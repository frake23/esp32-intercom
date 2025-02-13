#include "keypad.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include <string.h>
#include "esp_log.h"
#include "pcf8574.h"

#define KEYPAD_TASK_STACK_SIZE 4096
#define KEYPAD_TASK_PRIORITY 5
#define KEYPAD_SCAN_INTERVAL_MS 100 // Scan the keypad every 50 ms
#define KEYPAD_MAX_BUFFER_SIZE 32   // Maximum number size

static const char *TAG = "keypad";

// Buffers and variables for number accumulation
static char s_number_buffer[KEYPAD_MAX_BUFFER_SIZE];
static size_t s_number_index = 0;

// Callback function pointers
static keypad_number_entry_callback_t s_number_entry_callback = NULL;
static keypad_cancel_callback_t s_cancel_callback = NULL;

// Timer handle for inactivity timeout
static TimerHandle_t s_inactivity_timer = NULL;
static uint32_t s_inactivity_timeout_ms = 0;

// Mutex to protect shared resources
static SemaphoreHandle_t s_mutex = NULL;

static bool scan = true;

// Forward declarations of static functions
static void keypad_scan_task(void *arg);
static void keypad_inactivity_timer_callback(TimerHandle_t xTimer);
static char keypad_get_key_pressed(void);

/**
 * @brief Initialize the keypad module.
 */
void init_keypad(uint32_t inactivity_timeout_ms)
{
    s_inactivity_timeout_ms = inactivity_timeout_ms;

    // Create a mutex for shared resources
    s_mutex = xSemaphoreCreateMutex();

    // Create inactivity timer
    s_inactivity_timer = xTimerCreate("keypad_inactivity_timer",
                                      pdMS_TO_TICKS(s_inactivity_timeout_ms),
                                      pdFALSE,
                                      NULL,
                                      keypad_inactivity_timer_callback);

    // Create keypad scanning task
    xTaskCreate(keypad_scan_task, "keypad_scan_task", KEYPAD_TASK_STACK_SIZE, NULL, KEYPAD_TASK_PRIORITY, NULL);

    ESP_LOGI(TAG, "Keypad initialized");
}

/**
 * @brief Register the number entry completion callback.
 */
void keypad_register_number_entry_callback(keypad_number_entry_callback_t callback)
{
    s_number_entry_callback = callback;
}

/**
 * @brief Register the cancellation callback.
 */
void keypad_register_cancel_callback(keypad_cancel_callback_t callback)
{
    s_cancel_callback = callback;
}

/**
 * @brief Inactivity timer callback function.
 *
 * This function is called when the inactivity timer expires.
 */
static void keypad_inactivity_timer_callback(TimerHandle_t xTimer)
{
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if (s_number_index > 0)
    {
        // Inactivity timeout occurred, trigger number entry callback if set
        if (s_number_entry_callback)
        {
            s_number_buffer[s_number_index] = '\0'; // Null-terminate the string
            s_number_entry_callback(s_number_buffer);
        }
        s_number_index = 0; // Reset the buffer
    }
    xSemaphoreGive(s_mutex);
}

/**
 * @brief Keypad scanning task.
 *
 * This task continually scans the keypad for key presses.
 */
static void keypad_scan_task(void *arg)
{
    while (1)
    {
        char key = keypad_get_key_pressed();

        if (!scan && key != '#')
        {
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        if (key != '\0')
        {
            // A key has been pressed
            ESP_LOGI(TAG, "Key pressed: %c", key);

            xSemaphoreTake(s_mutex, portMAX_DELAY);

            if (key == '#')
            {
                // Cancellation button pressed
                s_number_index = 0; // Clear buffer
                if (s_cancel_callback)
                {
                    s_cancel_callback();
                }
            }
            else if (key == '*')
            {
                // Number entry completion
                if (s_number_index > 0)
                {
                    s_number_buffer[s_number_index] = '\0'; // Null-terminate the string
                    if (s_number_entry_callback)
                    {
                        s_number_entry_callback(s_number_buffer);
                    }
                    s_number_index = 0; // Reset the buffer
                }
            }
            else if (key >= '0' && key <= '9')
            {
                // Accumulate digits
                if (s_number_index < KEYPAD_MAX_BUFFER_SIZE - 1)
                {
                    s_number_buffer[s_number_index++] = key;
                }
                else
                {
                    // Buffer is full, ignore further input
                    ESP_LOGW(TAG, "Number buffer full");
                }
            }

            if (key != '#' && key != '*')
            {
                // Restart inactivity timer
                xTimerStop(s_inactivity_timer, 0);
                xTimerStart(s_inactivity_timer, 0);
            }
            xSemaphoreGive(s_mutex);

            // Debounce delay
            vTaskDelay(pdMS_TO_TICKS(200)); // Wait 200 ms before scanning again
        }
        else
        {
            // No key pressed, scan at regular interval
            vTaskDelay(pdMS_TO_TICKS(KEYPAD_SCAN_INTERVAL_MS));
        }
    }
}

/**
 * @brief Scan the keypad and return the key that is pressed.
 *
 * @return The character of the key pressed, or '\0' if no key is pressed.
 */
static char keypad_get_key_pressed(void)
{
    if (!scan)
    {
        return '\0';
    }

    const char keymap[4][3] = {
        {'1', '2', '3'},
        {'4', '5', '6'},
        {'7', '8', '9'},
        {'*', '0', '#'}};

    uint8_t row_val = 0xF0; // Rows as input (1) and columns as output (0)
    uint8_t col_val, data;

    for (int col = 1; col < 4; col++)
    {
        col_val = ~(1 << col) & 0x0E; // Set one column low

        data = read_pcf8574();
        write_pcf8574((row_val | col_val));

        // Read again
        data = read_pcf8574() & 0xF0;
        for (int row = 0; row < 4; row++)
        {
            if (!(data & (1 << (row + 4))))
            {
                // Debounce and validate the key press
                vTaskDelay(pdMS_TO_TICKS(10));
                data = read_pcf8574() & 0xF0;
                if (!(data & (1 << (row + 4))))
                {
                    // Wait until key is released
                    while (!(read_pcf8574() & (1 << (row + 4))))
                    {
                        vTaskDelay(pdMS_TO_TICKS(10));
                    }
                    return keymap[3 - row][3 - col];
                }
            }
        }
    }
    return '\0';
}

void lock_keypad()
{
    scan = false;
}

void release_keypad()
{
    scan = true;
}
