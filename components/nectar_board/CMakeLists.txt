idf_component_register(
    SRCS
        "src/board.c"
        "src/board_display.c"
        "src/gpio.c"
        "src/lvgl_port.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_gpio esp_driver_i2c esp_lcd esp_timer freertos lvgl esp_lcd_touch_gt911 esp_lcd_touch
)