#ifndef INDICATORS_H
#define INDICATORS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"

void init_flash();

#endif // INDICATORS_H
