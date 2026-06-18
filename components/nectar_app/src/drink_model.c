#include "drink_model.h"

#include "allergen_model.h"

static const drink_model_t s_drinks[] = {
    {
        .name = "Citrus soda",
        .min_ml = 20,
        .max_ml = 160,
        .default_ml = 60,
        .stock_percent = 84,
        .allergens = {ALLERGEN_CITRUS},
        .allergen_count = 1,
    },
    {
        .name = "Dry tonic",
        .min_ml = 20,
        .max_ml = 160,
        .default_ml = 40,
        .stock_percent = 57,
        .allergens = {ALLERGEN_QUININE},
        .allergen_count = 1,
    },
    {
        .name = "Ginger ale",
        .min_ml = 20,
        .max_ml = 160,
        .default_ml = 20,
        .stock_percent = 28,
        .allergens = {ALLERGEN_GINGER},
        .allergen_count = 1,
    },
};

size_t drink_model_count(void)
{
    return sizeof(s_drinks) / sizeof(s_drinks[0]);
}

const drink_model_t *drink_model_at(size_t index)
{
    if (index >= drink_model_count()) {
        return NULL;
    }

    return &s_drinks[index];
}