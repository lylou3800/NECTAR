#include "app_services.h"

#include <stddef.h>

typedef struct {
    bool glass_sequence_active;
    bool glass_detected;
    uint32_t glass_elapsed_ms;
    bool prepare_sequence_active;
    uint32_t prepare_elapsed_ms;
    uint8_t prepare_progress;
    const char *prepare_step;
} app_services_mock_state_t;

static app_services_mock_state_t s_state;
static const uint8_t s_reservoir_levels[] = {84, 57, 28};

void app_services_init(void)
{
    s_state.glass_sequence_active = false;
    s_state.glass_detected = false;
    s_state.glass_elapsed_ms = 0;
    s_state.prepare_sequence_active = false;
    s_state.prepare_elapsed_ms = 0;
    s_state.prepare_progress = 0;
    s_state.prepare_step = "Idle";
}

void app_services_reset_glass_sequence(void)
{
    s_state.glass_sequence_active = true;
    s_state.glass_detected = false;
    s_state.glass_elapsed_ms = 0;
}

void app_services_reset_prepare_sequence(void)
{
    s_state.prepare_sequence_active = true;
    s_state.prepare_elapsed_ms = 0;
    s_state.prepare_progress = 0;
    s_state.prepare_step = "Dose check";
}

void app_services_tick(uint32_t delta_ms)
{
    if (s_state.glass_sequence_active && !s_state.glass_detected) {
        s_state.glass_elapsed_ms += delta_ms;
        if (s_state.glass_elapsed_ms >= 1200) {
            s_state.glass_detected = true;
            s_state.glass_sequence_active = false;
        }
    }

    if (s_state.prepare_sequence_active) {
        s_state.prepare_elapsed_ms += delta_ms;
        if (s_state.prepare_elapsed_ms >= 4000) {
            s_state.prepare_elapsed_ms = 4000;
        }

        s_state.prepare_progress = (uint8_t)((s_state.prepare_elapsed_ms * 100U) / 4000U);

        if (s_state.prepare_progress < 25) {
            s_state.prepare_step = "Recipe check";
        } else if (s_state.prepare_progress < 65) {
            s_state.prepare_step = "Controlled pour";
        } else if (s_state.prepare_progress < 90) {
            s_state.prepare_step = "Final balance";
        } else {
            s_state.prepare_step = "Almost ready";
        }

        if (s_state.prepare_elapsed_ms >= 4000) {
            s_state.prepare_sequence_active = false;
            s_state.prepare_progress = 100;
            s_state.prepare_step = "Drink ready";
        }
    }
}

bool app_services_glass_detected(void)
{
    return s_state.glass_detected;
}

uint8_t app_services_prepare_progress(void)
{
    return s_state.prepare_progress;
}

bool app_services_prepare_complete(void)
{
    return s_state.prepare_progress >= 100;
}

const char *app_services_prepare_step(void)
{
    return s_state.prepare_step;
}

uint8_t app_services_reservoir_level(size_t index)
{
    if (index >= (sizeof(s_reservoir_levels) / sizeof(s_reservoir_levels[0]))) {
        return 0;
    }

    return s_reservoir_levels[index];
}