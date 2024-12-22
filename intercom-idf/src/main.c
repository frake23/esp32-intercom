#include <indicators.h>
#include <tcp_client.h>
#include <wifi_manager.h>
#include <keypad.h>
#include <cam.h>
#include <pcf8574.h>


const char* TAG = "APP_MAIN";


void number_callback(char *s) {
    ESP_LOGE(TAG, "%s", s);
    esp_err_t ret = tcp_client_connect("89.169.155.3", 3001); // Replace with your server's IP and port
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize TCP client");
        return;
    }
    tcp_client_send_string("start");
    vTaskDelay(pdMS_TO_TICKS(500));
    tcp_client_send_string(s);
    tcp_client_wait_for_msg();
}

void cancel_callback() {
    tcp_client_send_string("cancel");
    vTaskDelay(pdMS_TO_TICKS(500));
    tcp_client_send_string("\n");
    tcp_client_disconnect();
}

void photo_command(char *cmd) {
    tcp_client_send_string("photo");
    vTaskDelay(pdMS_TO_TICKS(500));
    tcp_client_send_photo();
    tcp_client_wait_for_msg();
}

void reject_command(char *cmd) {
    // show led
    tcp_client_send_string("reject_ok");
    vTaskDelay(pdMS_TO_TICKS(500));
    tcp_client_send_string("\n");
    tcp_client_disconnect();
}

void accept_command(char *cmd) {
    tcp_client_send_string("accept_ok");
    vTaskDelay(pdMS_TO_TICKS(500));
    tcp_client_send_string("\n");
    tcp_client_disconnect();
    
    gpio_set_level(GPIO_NUM_3, 0);
    
    lock_keypad();
    uint8_t data = read_pcf8574();
    write_pcf8574(data - 1);
    vTaskDelay(pdMS_TO_TICKS(3000));
    data = read_pcf8574();
    write_pcf8574(data + 1);
    release_keypad();
}

void not_found_command(char *cmd) {
    // show led
    tcp_client_disconnect();
}

void disconnect_callback() {
    // release_keypad();
}

void app_main()
{
    const char *ssid = "Dima";
    const char *password = "bebriksex";
    wifi_init_sta(ssid, password);
    i2c_master_init();
    write_pcf8574(0xFF);

    camera_init();
    init_flash();

    init_keypad(3000);
    keypad_register_number_entry_callback(*number_callback);
    keypad_register_cancel_callback(*cancel_callback);

    tcp_client_register_command_callback("accept", *accept_command);
    tcp_client_register_command_callback("photo", *photo_command);
    tcp_client_register_command_callback("reject", *reject_command);
    tcp_client_register_command_callback("not_found", *not_found_command);
    tcp_client_register_disconnect_callback(*disconnect_callback);

    while (1)
    {
        vTaskDelay(portMAX_DELAY);
    }
}