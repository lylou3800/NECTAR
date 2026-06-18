#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void app_services_init(void);
void app_services_reset_glass_sequence(void);
void app_services_reset_prepare_sequence(void);
void app_services_tick(uint32_t delta_ms);
bool app_services_glass_detected(void);
uint8_t app_services_prepare_progress(void);
bool app_services_prepare_complete(void);
const char *app_services_prepare_step(void);
uint8_t app_services_reservoir_level(size_t index);

#ifdef __cplusplus
}
#endif