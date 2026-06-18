file(GLOB_RECURSE nectar_ui_sources "src/*.c")

idf_component_register(
    SRCS ${nectar_ui_sources}
    INCLUDE_DIRS "include"
    REQUIRES nectar_board nectar_app lvgl
)