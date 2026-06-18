#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DRINK_MODEL_MAX_ALLERGENS 2

typedef struct {
    const char *name;
    uint16_t min_ml;
    uint16_t max_ml;
    uint16_t default_ml;
    uint8_t stock_percent;
    const char *allergens[DRINK_MODEL_MAX_ALLERGENS];
    uint8_t allergen_count;
} drink_model_t;

size_t drink_model_count(void);
const drink_model_t *drink_model_at(size_t index);

#ifdef __cplusplus
}
#endif