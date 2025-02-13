#include <indicators.h>
#include <tcp_client.h>
#include <wifi_manager.h>
#include <keypad.h>
#include <cam.h>
#include <pcf8574.h>
#include <soc/gpio_periph.h>

const char *TAG = "APP_MAIN";

void number_callback(char *s)
{
    ESP_LOGE(TAG, "%s", s);
    esp_err_t ret = tcp_client_connect("89.169.155.3", 3001);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize TCP client");
        return;
    }
    led_start_blinking();
    tcp_client_send_string("start");
    vTaskDelay(pdMS_TO_TICKS(500));
    tcp_client_send_string(s);
    tcp_client_wait_for_msg();
}

void cancel_callback()
{
    led_stop_blinking();
    tcp_client_send_string("cancel");
    vTaskDelay(pdMS_TO_TICKS(500));
    tcp_client_send_string("\n");
    tcp_client_disconnect();
}

void photo_command(char *cmd)
{
    tcp_client_send_string("photo");
    vTaskDelay(pdMS_TO_TICKS(500));
    tcp_client_send_photo();
    tcp_client_wait_for_msg();
}

void reject_command(char *cmd)
{
    tcp_client_send_string("reject_ok");
    vTaskDelay(pdMS_TO_TICKS(500));
    tcp_client_send_string("\n");
    tcp_client_disconnect();
    led_stop_blinking();
    led_show(3000);
}

void accept_command(char *cmd)
{
    tcp_client_send_string("accept_ok");
    vTaskDelay(pdMS_TO_TICKS(500));
    tcp_client_send_string("\n");
    tcp_client_disconnect();

    led_stop_blinking();

    gpio_set_level(GPIO_NUM_1, 0);
    vTaskDelay(pdMS_TO_TICKS(3000));
    gpio_set_level(GPIO_NUM_1, 1);
}

void not_found_command(char *cmd)
{
    tcp_client_disconnect();
    led_stop_blinking();
    led_show(3000);
}

void app_main()
{
    const char *ssid = "Dima";
    const char *password = "bebriksex";
    wifi_init_sta(ssid, password);
    i2c_master_init();

    camera_init();
    init_led();
    init_flash();

    init_keypad(3000);
    keypad_register_number_entry_callback(*number_callback);
    keypad_register_cancel_callback(*cancel_callback);

    tcp_client_register_command_callback("accept", *accept_command);
    tcp_client_register_command_callback("photo", *photo_command);
    tcp_client_register_command_callback("reject", *reject_command);
    tcp_client_register_command_callback("not_found", *not_found_command);

    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[1], PIN_FUNC_GPIO);
    gpio_set_direction(GPIO_NUM_1, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_1, 1);

    while (1)
    {
        vTaskDelay(portMAX_DELAY);
    }
}