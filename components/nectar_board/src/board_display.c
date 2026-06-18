#include "board_display.h"

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "lvgl_port.h"

static const char *TAG = "nectar_board";

#define NECTAR_I2C_PORT I2C_NUM_0
#define NECTAR_I2C_SCL GPIO_NUM_9
#define NECTAR_I2C_SDA GPIO_NUM_8
#define NECTAR_I2C_FREQ_HZ 400000
#define NECTAR_I2C_TIMEOUT_MS 1000

#define NECTAR_TOUCH_RESET_GATE_GPIO GPIO_NUM_4
#define NECTAR_TOUCH_IO_EXPANDER_ADDR 0x24
#define NECTAR_TOUCH_CTRL_ADDR 0x38

#define NECTAR_LCD_PIXEL_CLOCK_HZ (16 * 1000 * 1000)

#define NECTAR_LCD_GPIO_VSYNC GPIO_NUM_3
#define NECTAR_LCD_GPIO_HSYNC GPIO_NUM_46
#define NECTAR_LCD_GPIO_DE GPIO_NUM_5
#define NECTAR_LCD_GPIO_PCLK GPIO_NUM_7

static esp_lcd_panel_handle_t s_panel_handle;
static esp_lcd_touch_handle_t s_touch_handle;
static i2c_master_bus_handle_t s_i2c_bus_handle;
static i2c_master_dev_handle_t s_touch_gate_dev_handle;
static i2c_master_dev_handle_t s_touch_ctrl_dev_handle;
static bool s_i2c_ready;
static bool s_display_ready;

IRAM_ATTR static bool board_display_on_vsync(esp_lcd_panel_handle_t panel,
                                             const esp_lcd_rgb_panel_event_data_t *event_data,
                                             void *user_ctx)
{
    (void)panel;
    (void)event_data;
    (void)user_ctx;
    return lvgl_port_notify_rgb_vsync();
}

static esp_err_t board_i2c_write(i2c_master_dev_handle_t device_handle, uint8_t value)
{
    return i2c_master_transmit(device_handle, &value, 1, NECTAR_I2C_TIMEOUT_MS);
}

static esp_err_t board_i2c_init(void)
{
    if (s_i2c_ready) {
        return ESP_OK;
    }

    const i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .i2c_port = NECTAR_I2C_PORT,
        .sda_io_num = NECTAR_I2C_SDA,
        .scl_io_num = NECTAR_I2C_SCL,
        .flags.enable_internal_pullup = true,
    };
    const i2c_device_config_t touch_gate_device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = NECTAR_TOUCH_IO_EXPANDER_ADDR,
        .scl_speed_hz = NECTAR_I2C_FREQ_HZ,
    };
    const i2c_device_config_t touch_ctrl_device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = NECTAR_TOUCH_CTRL_ADDR,
        .scl_speed_hz = NECTAR_I2C_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &s_i2c_bus_handle));
    ESP_ERROR_CHECK(i2c_master_bus_add_device(s_i2c_bus_handle, &touch_gate_device_config, &s_touch_gate_dev_handle));
    ESP_ERROR_CHECK(i2c_master_bus_add_device(s_i2c_bus_handle, &touch_ctrl_device_config, &s_touch_ctrl_dev_handle));

    s_i2c_ready = true;
    return ESP_OK;
}

static esp_err_t board_touch_gate_init(void)
{
    const gpio_config_t config = {
        .pin_bit_mask = 1ULL << NECTAR_TOUCH_RESET_GATE_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    return gpio_config(&config);
}

static esp_err_t board_touch_reset_sequence(void)
{
    uint8_t value = 0x01;

    ESP_ERROR_CHECK(board_i2c_write(s_touch_gate_dev_handle, value));

    value = 0x2C;
    ESP_ERROR_CHECK(board_i2c_write(s_touch_ctrl_dev_handle, value));
    esp_rom_delay_us(100 * 1000);

    gpio_set_level(NECTAR_TOUCH_RESET_GATE_GPIO, 0);
    esp_rom_delay_us(100 * 1000);

    value = 0x2E;
    ESP_ERROR_CHECK(board_i2c_write(s_touch_ctrl_dev_handle, value));
    esp_rom_delay_us(200 * 1000);
    return ESP_OK;
}

static esp_err_t board_touch_init(void)
{
    esp_lcd_panel_io_handle_t touch_io_handle = NULL;
    const esp_lcd_panel_io_i2c_config_t touch_io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
    const esp_lcd_touch_config_t touch_config = {
        .x_max = BOARD_DISPLAY_H_RES,
        .y_max = BOARD_DISPLAY_V_RES,
        .rst_gpio_num = -1,
        .int_gpio_num = -1,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };

    ESP_ERROR_CHECK(board_i2c_init());
    ESP_ERROR_CHECK(board_touch_gate_init());
    ESP_ERROR_CHECK(board_touch_reset_sequence());

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(
        s_i2c_bus_handle,
        &touch_io_config,
        &touch_io_handle
    ));

    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(touch_io_handle, &touch_config, &s_touch_handle));
    return ESP_OK;
}

static esp_err_t board_panel_init(void)
{
    const esp_lcd_rgb_panel_config_t panel_config = {
        .data_width = 16,
        .dma_burst_size = 64,
        .num_fbs = LVGL_PORT_LCD_RGB_BUFFER_NUMS,
        /* Bounce buffer (10 lignes) — config officielle Waveshare pour ce panneau :
         * le LCD lit depuis un petit buffer SRAM réalimenté par DMA depuis la PSRAM,
         * ce qui stabilise le flux pixel et, combiné à la synchro on_frame_buf_complete,
         * supprime le tearing. Le signal de fin de frame est alors on_frame_buf_complete
         * (cf. board_display_init), pas on_vsync. */
        .bounce_buffer_size_px = BOARD_DISPLAY_H_RES * 10,
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .timings = {
            .pclk_hz = NECTAR_LCD_PIXEL_CLOCK_HZ,
            .h_res = BOARD_DISPLAY_H_RES,
            .v_res = BOARD_DISPLAY_V_RES,
            .hsync_pulse_width = 4,
            .hsync_back_porch = 8,
            .hsync_front_porch = 8,
            .vsync_pulse_width = 4,
            .vsync_back_porch = 8,
            .vsync_front_porch = 8,
            .flags = {
                .pclk_active_neg = 1,
            },
        },
        .hsync_gpio_num = NECTAR_LCD_GPIO_HSYNC,
        .vsync_gpio_num = NECTAR_LCD_GPIO_VSYNC,
        .de_gpio_num = NECTAR_LCD_GPIO_DE,
        .pclk_gpio_num = NECTAR_LCD_GPIO_PCLK,
        .disp_gpio_num = -1,
        .data_gpio_nums = {
            GPIO_NUM_14,
            GPIO_NUM_38,
            GPIO_NUM_18,
            GPIO_NUM_17,
            GPIO_NUM_10,
            GPIO_NUM_39,
            GPIO_NUM_0,
            GPIO_NUM_45,
            GPIO_NUM_48,
            GPIO_NUM_47,
            GPIO_NUM_21,
            GPIO_NUM_1,
            GPIO_NUM_2,
            GPIO_NUM_42,
            GPIO_NUM_41,
            GPIO_NUM_40,
        },
        .flags = {
            .fb_in_psram = 1,
        },
    };

    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &s_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(s_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_panel_handle));
    return ESP_OK;
}

esp_err_t board_display_init(void)
{
    if (s_display_ready) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing Waveshare 7-inch display stack");
    ESP_ERROR_CHECK(board_panel_init());
    ESP_ERROR_CHECK(board_touch_init());
    ESP_ERROR_CHECK(lvgl_port_init(s_panel_handle, s_touch_handle));

    /* Bounce buffer actif => le signal "frame entièrement transmise au LCD" est
     * on_frame_buf_complete (API ESP-IDF v6 ; et NON on_vsync). C'est lui qui
     * débloque le flush LVGL pour la synchro anti-tearing (cf. lvgl_port.c). */
    const esp_lcd_rgb_panel_event_callbacks_t callbacks = {
        .on_frame_buf_complete = board_display_on_vsync,
    };
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_register_event_callbacks(s_panel_handle, &callbacks, NULL));

    board_display_backlight_set(true);
    s_display_ready = true;
    return ESP_OK;
}

bool board_display_lock(int timeout_ms)
{
    return lvgl_port_lock(timeout_ms);
}

void board_display_unlock(void)
{
    lvgl_port_unlock();
}

void board_display_backlight_set(bool enabled)
{
    if (!s_i2c_ready) {
        return;
    }

    ESP_ERROR_CHECK(board_i2c_write(s_touch_gate_dev_handle, 0x01));
    ESP_ERROR_CHECK(board_i2c_write(s_touch_ctrl_dev_handle, enabled ? 0x1E : 0x1A));
}

uint16_t board_display_width(void)
{
    return BOARD_DISPLAY_H_RES;
}

uint16_t board_display_height(void)
{
    return BOARD_DISPLAY_V_RES;
}