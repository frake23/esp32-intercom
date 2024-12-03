#include "tcp_client.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "esp_err.h"

#define MAX_COMMAND_CALLBACKS 10 // Maximum number of commands you can register

static const char *TAG = "tcp_client";

static int sock = -1;
static struct sockaddr_in server_addr;

// Structure to hold a command and its associated callback
typedef struct
{
    char command[32]; // Adjust the size according to your maximum command length
    tcp_client_command_callback_t callback;
} tcp_client_command_entry_t;

// Array to store registered commands and their callbacks
static tcp_client_command_entry_t command_callbacks[MAX_COMMAND_CALLBACKS];
static int command_callback_count = 0;

static tcp_client_disconnect_callback_t disconnect_callback = NULL;

esp_err_t tcp_client_register_disconnect_callback(tcp_client_disconnect_callback_t callback)
{
    disconnect_callback = callback;
    return ESP_OK;
}

static SemaphoreHandle_t wait_mutex = NULL;

void tcp_client_wait_task()
{
    xSemaphoreTake(wait_mutex, portMAX_DELAY);
    char rx_buffer[128];

    int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
    if (len < 0)
    {
        ESP_LOGE(TAG, "recv failed: errno %d", errno);
        xSemaphoreGive(wait_mutex);
        vTaskDelete(NULL);
        return;
    }
    else if (len == 0)
    {
        ESP_LOGW(TAG, "Connection closed");
        if (disconnect_callback != NULL)
        {
            disconnect_callback();
        }
        xSemaphoreGive(wait_mutex);
        vTaskDelete(NULL);
        return;
    }
    rx_buffer[len] = 0; // Null-terminate the string
    ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);

    // Check for registered command callbacks
    for (int i = 0; i < command_callback_count; i++)
    {
        if (strcmp(rx_buffer, command_callbacks[i].command) == 0)
        {
            if (command_callbacks[i].callback != NULL)
            {
                command_callbacks[i].callback(rx_buffer);
            }
            xSemaphoreGive(wait_mutex);
            vTaskDelete(NULL);
            return;
        }
    }
    ESP_LOGW(TAG, "No callback registered for command: %s", rx_buffer);
    xSemaphoreGive(wait_mutex);
    vTaskDelete(NULL);
}

void tcp_client_wait_for_msg()
{
    xTaskCreate(tcp_client_wait_task, "tcp_client_wait_task", 4096, NULL, 5, NULL);
}

esp_err_t tcp_client_connect(const char *server_ip, uint16_t server_port)
{
    if (wait_mutex == NULL)
    {
        wait_mutex = xSemaphoreCreateMutex();
    }

    if (sock != -1)
    {
        ESP_LOGE(TAG, "Socket already connected");
        return ESP_FAIL;
    }

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return ESP_FAIL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    int err = inet_pton(AF_INET, server_ip, &server_addr.sin_addr.s_addr);
    if (err != 1)
    {
        ESP_LOGE(TAG, "Invalid server IP address");
        close(sock);
        sock = -1;
        return ESP_FAIL;
    }

    err = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (err != 0)
    {
        ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
        close(sock);
        sock = -1;
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Successfully connected");

    return ESP_OK;
}

esp_err_t tcp_client_register_command_callback(const char *command, tcp_client_command_callback_t callback)
{
    if (command_callback_count >= MAX_COMMAND_CALLBACKS)
    {
        ESP_LOGE(TAG, "Maximum number of command callbacks reached");
        return ESP_ERR_NO_MEM;
    }

    // Store the command and callback
    strncpy(command_callbacks[command_callback_count].command, command, sizeof(command_callbacks[command_callback_count].command) - 1);
    command_callbacks[command_callback_count].command[sizeof(command_callbacks[command_callback_count].command) - 1] = 0; // Ensure null-terminated

    command_callbacks[command_callback_count].callback = callback;
    command_callback_count++;

    ESP_LOGI(TAG, "Registered callback for command: %s", command);

    return ESP_OK;
}

esp_err_t tcp_client_send_string(const char *message)
{
    if (sock == -1)
    {
        ESP_LOGE(TAG, "Socket not connected");
        return ESP_FAIL;
    }

    int err = send(sock, message, strlen(message), 0);
    if (err < 0)
    {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t tcp_client_send_photo()
{
    if (sock == -1)
    {
        ESP_LOGE(TAG, "Socket not connected");
        return ESP_FAIL;
    }

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
        ESP_LOGE(TAG, "Camera capture failed");
        return ESP_FAIL;
    }

    // Send the size of the image first
    uint32_t image_size = fb->len;
    uint32_t size_network_order = htonl(image_size);

    int err = send(sock, &size_network_order, sizeof(size_network_order), 0);
    if (err < 0)
    {
        ESP_LOGE(TAG, "Error occurred during sending image size: errno %d", errno);
        esp_camera_fb_return(fb);
        return ESP_FAIL;
    }

    // Send the image data
    size_t to_send = fb->len;
    size_t sent = 0;
    while (sent < to_send)
    {
        int bytes = send(sock, fb->buf + sent, to_send - sent, 0);
        if (bytes < 0)
        {
            ESP_LOGE(TAG, "Error occurred during sending image data: errno %d", errno);
            esp_camera_fb_return(fb);
            return ESP_FAIL;
        }
        sent += bytes;
    }

    esp_camera_fb_return(fb);
    return ESP_OK;
}

esp_err_t tcp_client_disconnect()
{
    if (disconnect_callback != NULL)
    {
        disconnect_callback();
    }
    if (sock != -1)
    {
        shutdown(sock, 0);
        close(sock);
        sock = -1;
    }
    return ESP_OK;
}
