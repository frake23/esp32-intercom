#ifndef INDICATORS_H
#define INDICATORS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"

void init_flash();

void init_led();

void led_show(int ms);

void led_start_blinking();

void led_stop_blinking();

#endif // INDICATORS_H
