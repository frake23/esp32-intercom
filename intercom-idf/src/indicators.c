#include <indicators.h>

static const char *TAG = "indicators";

void init_flash()
{
    // Configure the LEDC timer
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_8_BIT, // Resolution of PWM duty
        .freq_hz = 40000,                    // Frequency of PWM signal
        .speed_mode = LEDC_LOW_SPEED_MODE,   // LEDC speed mode
        .timer_num = LEDC_TIMER_1,           // Timer index
        .clk_cfg = LEDC_AUTO_CLK             // Auto select the source clock
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Configure the LEDC channel
    ledc_channel_config_t ledc_channel = {
        .channel = LEDC_CHANNEL_0,         // LEDC channel index
        .duty = 0,                         // Initial duty cycle
        .gpio_num = GPIO_NUM_4,            // GPIO number (Flashlight pin)
        .speed_mode = LEDC_LOW_SPEED_MODE, // LEDC speed mode
        .hpoint = 0,                       // LEDC high point
        .timer_sel = LEDC_TIMER_1         // Select the timer for this channel
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    // Calculate the duty cycle value for 10% brightness
    uint32_t max_duty = (1 << LEDC_TIMER_8_BIT) - 1; // Max duty based on resolution
    uint32_t duty = max_duty / 25;                   // 5% of max duty

    // Set the duty cycle to 10%
    ESP_ERROR_CHECK(ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, duty));
    // Update duty cycle to apply the new value
    ESP_ERROR_CHECK(ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel));
    
    ESP_LOGI(TAG, "Flash initialized");
}