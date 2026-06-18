#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"


void controlPompe(bool);
void setPompeDuration(uint32_t duration_ms);
void servirBoisson(uint32_t secondes);
bool checkVerre(void);

float lirePoid(void);
float lireTensionPoidI2C(void);


