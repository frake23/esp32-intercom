#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

/**
 * @brief Initialize Wi-Fi as station and connect to the specified SSID and password.
 *
 * @param ssid      The SSID of the Wi-Fi network to connect to.
 * @param password  The password for the Wi-Fi network.
 */
void wifi_init_sta(const char *ssid, const char *password);

#endif // WIFI_MANAGER_H
