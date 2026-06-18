#include "lvgl_port.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "board_display.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"

static const char *TAG = "nectar_lvgl";

#define LVGL_PORT_TICK_PERIOD_MS 2
#define LVGL_PORT_TASK_MAX_DELAY_MS 500
#define LVGL_PORT_TASK_MIN_DELAY_MS 1
#define LVGL_PORT_TASK_STACK_SIZE (6 * 1024)
#define LVGL_PORT_TASK_PRIORITY 4
#define LVGL_PORT_TASK_CORE 1

static SemaphoreHandle_t s_lvgl_mutex;
static TaskHandle_t s_lvgl_task_handle;

/* Synchronisation anti-tearing — schéma officiel Espressif pour panneau RGB en
 * DOUBLE framebuffer (sans bounce buffer). Poignée de main à deux sémaphores
 * entre le flush LVGL (contexte tâche) et l'interruption vsync du panneau :
 *  - le flush signale "GUI prête" (s_sem_gui_ready) puis ATTEND la fin du vsync
 *    (s_sem_vsync_end) avant de basculer le framebuffer affiché ;
 *  - l'ISR vsync, si une GUI est prête, libère s_sem_vsync_end.
 * Garantit que chaque image affichée est complète et que le swap est aligné au
 * vsync => aucun tearing/tremblement pendant le mouvement. */
static SemaphoreHandle_t s_sem_vsync_end;
static SemaphoreHandle_t s_sem_gui_ready;

static void flush_callback(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;

    (void)area;

    xSemaphoreGive(s_sem_gui_ready);
    xSemaphoreTake(s_sem_vsync_end, portMAX_DELAY);

    /* full_refresh => color_map est l'un des deux framebuffers plein écran ;
     * le panneau bascule dessus au prochain vsync (swap zéro-copie). */
    esp_lcd_panel_draw_bitmap(
        panel_handle,
        0,
        0,
        BOARD_DISPLAY_H_RES,
        BOARD_DISPLAY_V_RES,
        color_map
    );

    lv_disp_flush_ready(drv);
}

static lv_disp_t *display_init(esp_lcd_panel_handle_t panel_handle)
{
    static lv_disp_draw_buf_t draw_buffer;
    static lv_disp_drv_t display_driver;

    void *buf1 = NULL;
    void *buf2 = NULL;
    const uint32_t buffer_size = BOARD_DISPLAY_H_RES * BOARD_DISPLAY_V_RES;

    ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, &buf1, &buf2));

    lv_disp_draw_buf_init(&draw_buffer, buf1, buf2, buffer_size);

    lv_disp_drv_init(&display_driver);
    display_driver.hor_res = BOARD_DISPLAY_H_RES;
    display_driver.ver_res = BOARD_DISPLAY_V_RES;
    display_driver.flush_cb = flush_callback;
    display_driver.draw_buf = &draw_buffer;
    display_driver.user_data = panel_handle;
    /* Double buffering plein écran (les 2 buffers = les 2 framebuffers PSRAM) :
     * full_refresh=1 + direct_mode=0 => le flush_cb fait un simple swap d'adresse
     * de framebuffer (mode "traditional double buffering" de la doc LVGL). */
    display_driver.direct_mode = 0;
    display_driver.full_refresh = 1;

    return lv_disp_drv_register(&display_driver);
}

static void touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    esp_lcd_touch_handle_t touch_handle = (esp_lcd_touch_handle_t)indev_drv->user_data;
    uint16_t touch_x = 0;
    uint16_t touch_y = 0;
    uint8_t touch_count = 0;

    esp_lcd_touch_read_data(touch_handle);

    if (esp_lcd_touch_get_coordinates(touch_handle, &touch_x, &touch_y, NULL, &touch_count, 1) &&
        (touch_count > 0)) {
        data->point.x = touch_x;
        data->point.y = touch_y;
        data->state = LV_INDEV_STATE_PRESSED;
        return;
    }

    data->state = LV_INDEV_STATE_RELEASED;
}

static void indev_init(esp_lcd_touch_handle_t touch_handle)
{
    static lv_indev_drv_t indev_driver;

    lv_indev_drv_init(&indev_driver);
    indev_driver.type = LV_INDEV_TYPE_POINTER;
    indev_driver.read_cb = touchpad_read;
    indev_driver.user_data = touch_handle;
    lv_indev_drv_register(&indev_driver);
}

static void tick_increment(void *arg)
{
    (void)arg;
    lv_tick_inc(LVGL_PORT_TICK_PERIOD_MS);
}

static esp_err_t tick_init(void)
{
    esp_timer_handle_t tick_timer = NULL;
    const esp_timer_create_args_t timer_args = {
        .callback = tick_increment,
        .name = "nectar_lvgl_tick",
    };

    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &tick_timer));
    return esp_timer_start_periodic(tick_timer, LVGL_PORT_TICK_PERIOD_MS * 1000);
}

static void lvgl_port_task(void *arg)
{
    (void)arg;

    uint32_t task_delay_ms = LVGL_PORT_TASK_MAX_DELAY_MS;
    while (true) {
        /* lv_timer_handler() appelle flush_callback() qui se bloque sur la
         * poignée de main vsync : le rythme de rendu se cale donc sur le vsync. */
        if (lvgl_port_lock(-1)) {
            task_delay_ms = lv_timer_handler();
            lvgl_port_unlock();
        }

        if (task_delay_ms > LVGL_PORT_TASK_MAX_DELAY_MS) {
            task_delay_ms = LVGL_PORT_TASK_MAX_DELAY_MS;
        } else if (task_delay_ms < LVGL_PORT_TASK_MIN_DELAY_MS) {
            task_delay_ms = LVGL_PORT_TASK_MIN_DELAY_MS;
        }

        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}

esp_err_t lvgl_port_init(esp_lcd_panel_handle_t lcd_handle, esp_lcd_touch_handle_t tp_handle)
{
    lv_init();
    ESP_ERROR_CHECK(tick_init());

    s_lvgl_mutex = xSemaphoreCreateRecursiveMutex();
    if (s_lvgl_mutex == NULL) {
        return ESP_ERR_NO_MEM;
    }

    s_sem_vsync_end = xSemaphoreCreateBinary();
    s_sem_gui_ready = xSemaphoreCreateBinary();
    if ((s_sem_vsync_end == NULL) || (s_sem_gui_ready == NULL)) {
        return ESP_ERR_NO_MEM;
    }

    if (display_init(lcd_handle) == NULL) {
        return ESP_FAIL;
    }

    if (tp_handle != NULL) {
        indev_init(tp_handle);
    }

    if (xTaskCreatePinnedToCore(
            lvgl_port_task,
            "nectar_lvgl",
            LVGL_PORT_TASK_STACK_SIZE,
            NULL,
            LVGL_PORT_TASK_PRIORITY,
            &s_lvgl_task_handle,
            LVGL_PORT_TASK_CORE) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create LVGL task");
        return ESP_FAIL;
    }

    return ESP_OK;
}

bool lvgl_port_lock(int timeout_ms)
{
    if (s_lvgl_mutex == NULL) {
        return false;
    }

    const TickType_t timeout_ticks = (timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTakeRecursive(s_lvgl_mutex, timeout_ticks) == pdTRUE;
}

void lvgl_port_unlock(void)
{
    if (s_lvgl_mutex != NULL) {
        xSemaphoreGiveRecursive(s_lvgl_mutex);
    }
}

IRAM_ATTR bool lvgl_port_notify_rgb_vsync(void)
{
    BaseType_t high_task_awoken = pdFALSE;

    if ((s_sem_gui_ready != NULL) && (s_sem_vsync_end != NULL)) {
        if (xSemaphoreTakeFromISR(s_sem_gui_ready, &high_task_awoken) == pdTRUE) {
            xSemaphoreGiveFromISR(s_sem_vsync_end, &high_task_awoken);
        }
    }

    return high_task_awoken == pdTRUE;
}
