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
#define LVGL_PORT_TASK_MIN_DELAY_MS 5
#define LVGL_PORT_TASK_STACK_SIZE (6 * 1024)
#define LVGL_PORT_TASK_PRIORITY 4
#define LVGL_PORT_TASK_CORE 1

static SemaphoreHandle_t s_lvgl_mutex;
static TaskHandle_t s_lvgl_task_handle;
static volatile bool s_flush_pending = false;
static lv_disp_drv_t *s_flush_drv = NULL;

static void flush_callback(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;

    (void)area;

    /* Enregistre le driver pour que le vsync puisse appeler lv_disp_flush_ready.
     * Ne PAS appeler lv_disp_flush_ready ici : le buffer swap n'est effectif
     * qu'au prochain vsync ; l'appeler avant causerait du tearing/flickering. */
    s_flush_drv = drv;
    s_flush_pending = true;

    esp_lcd_panel_draw_bitmap(
        panel_handle,
        0,
        0,
        BOARD_DISPLAY_H_RES,
        BOARD_DISPLAY_V_RES,
        color_map
    );
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
    /* direct_mode = 1 : LVGL écrit directement dans les frame buffers PSRAM.
     * Combiné au vsync-synchronized flush, cela élimine tout tearing. */
    display_driver.direct_mode = 1;
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
        TickType_t wait_ticks;

        /* Attendre le vsync (ou le timeout) AVANT de rendre.
         * Cela garantit que lv_disp_flush_ready() n'est appelé qu'après
         * que le LCD a effectivement basculé sur le nouveau frame buffer. */
        wait_ticks = pdMS_TO_TICKS(task_delay_ms);
        if (wait_ticks == 0U) {
            wait_ticks = 1U;
        }
        (void)ulTaskNotifyTake(pdTRUE, wait_ticks);

        if (lvgl_port_lock(-1)) {
            /* Si un flush est en attente de confirmation vsync, le signaler
             * maintenant que le swap de buffer est effectif. */
            if (s_flush_pending && s_flush_drv != NULL) {
                lv_disp_flush_ready(s_flush_drv);
                s_flush_drv = NULL;
                s_flush_pending = false;
            }

            task_delay_ms = lv_timer_handler();
            lvgl_port_unlock();
        }

        if (task_delay_ms > LVGL_PORT_TASK_MAX_DELAY_MS) {
            task_delay_ms = LVGL_PORT_TASK_MAX_DELAY_MS;
        } else if (task_delay_ms < LVGL_PORT_TASK_MIN_DELAY_MS) {
            task_delay_ms = LVGL_PORT_TASK_MIN_DELAY_MS;
        }
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
    BaseType_t need_yield = pdFALSE;

    if (s_lvgl_task_handle != NULL) {
        xTaskNotifyFromISR(s_lvgl_task_handle, 1U, eIncrement, &need_yield);
    }

    return need_yield == pdTRUE;
}