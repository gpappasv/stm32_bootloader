/**
 * @file user_input.c
 * @brief This source file is about creating an api to get a user input. In the current implementation, the input is the
 *        default button on the stm32f401re nucleo board.
 * @version 0.1
 * @date 2024-06-24
 *
 * @copyright Copyright (c) 2024
 *
 */

// --- includes --------------------------------------------------------------------------------------------------------
#include "user_input.h"

#include "stm32f4xx_hal.h"
#include <stdbool.h>

// --- defines ---------------------------------------------------------------------------------------------------------
#define BUTTON_GPIO_PORT GPIOC
#define BUTTON_GPIO_PIN  GPIO_PIN_13

// --- static function declarations ------------------------------------------------------------------------------------
static void user_input_init(void);

// --- static function definitions -------------------------------------------------------------------------------------
static void
user_input_init(void)
{
    static bool is_init = false;

    if (is_init) {
        return;
    }
    // Enable the GPIOA clock
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // Configure the GPIO pin as input with pull-up
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    GPIO_InitStruct.Pin              = BUTTON_GPIO_PIN;
    GPIO_InitStruct.Mode             = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull             = GPIO_PULLUP;
    GPIO_InitStruct.Speed            = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BUTTON_GPIO_PORT, &GPIO_InitStruct);

    is_init = true;
}

// --- function definitions --------------------------------------------------------------------------------------------
/**
 * @brief This function checks if the user input is pressed.
 *
 * @return true
 * @return false
 */
bool
user_input_is_pressed(void)
{
    user_input_init();

    return HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, BUTTON_GPIO_PIN) == GPIO_PIN_RESET;
}