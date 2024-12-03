#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdint.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/**
 * @brief Callback type for the number entry completion event.
 *
 * This callback is called when a sequence of numbers has been entered,
 * and the user presses '*' or after a specified inactivity timeout.
 *
 * @param number The string of numbers entered by the user.
 */
typedef void (*keypad_number_entry_callback_t)(const char* number);

/**
 * @brief Callback type for the cancellation event.
 *
 * This callback is called when the user presses the '#' key.
 */
typedef void (*keypad_cancel_callback_t)(void);

/**
 * @brief Initialize the keypad module.
 *
 * @param row_pins              Array of 4 GPIO numbers used for row pins.
 * @param col_pins              Array of 3 GPIO numbers used for column pins.
 * @param inactivity_timeout_ms Inactivity timeout in milliseconds.
 */
void init_keypad(uint32_t inactivity_timeout_ms);

/**
 * @brief Register a callback for the number entry completion event.
 *
 * This event is triggered when the '*' key is pressed or after inactivity.
 * Only one listener can be registered at a time; subsequent calls overwrite
 * the previous listener.
 *
 * @param callback The callback function to register.
 */
void keypad_register_number_entry_callback(keypad_number_entry_callback_t callback);

/**
 * @brief Register a callback for the cancellation event.
 *
 * This event is triggered when the '#' key is pressed.
 * Only one listener can be registered at a time; subsequent calls overwrite
 * the previous listener.
 *
 * @param callback The callback function to register.
 */
void keypad_register_cancel_callback(keypad_cancel_callback_t callback);

void lock_keypad();

void release_keypad();

#endif // KEYPAD_H