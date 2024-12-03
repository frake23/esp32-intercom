#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "esp_err.h"
#include "esp_camera.h"

// Callback type for handling commands
typedef void (*tcp_client_command_callback_t)(const char *cmd);
typedef void (*tcp_client_disconnect_callback_t)(void);


// Initialize the TCP client module and connect to the server
esp_err_t tcp_client_connect(const char *server_ip, uint16_t server_port);

// Send a raw string to the server
esp_err_t tcp_client_send_string(const char *message);

// Send a photo from the camera module to the server
esp_err_t tcp_client_send_photo();

// Register a callback function for a specific server command
esp_err_t tcp_client_register_command_callback(const char *command, tcp_client_command_callback_t callback);

esp_err_t tcp_client_register_disconnect_callback(tcp_client_disconnect_callback_t callback);

// Close the TCP client connection
esp_err_t tcp_client_disconnect();

void tcp_client_wait_for_msg();

#endif // TCP_CLIENT_H