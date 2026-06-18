idf_component_register(
    SRCS
        "src/admin_auth.c"
        "src/app_controller.c"
        "src/app_services_mock.c"
        "src/ui_state_model.c"
        "src/recipe_model.c"
        "src/drink_model.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_common esp_timer nvs_flash nectar_board
)