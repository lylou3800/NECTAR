idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES nectar_board nectar_ui nvs_flash
)