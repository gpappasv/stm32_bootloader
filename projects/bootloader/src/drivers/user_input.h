/**
 * @file user_input.h
 * @brief This source file is about creating an api to get a user input. In the current implementation, the input is the
 *        default button on the stm32f401re nucleo board.
 * @version 0.1
 * @date 2024-06-24
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef USER_INPUT_H
#define USER_INPUT_H

// --- includes --------------------------------------------------------------------------------------------------------
#include <stdbool.h>

// --- function declarations -------------------------------------------------------------------------------------------
bool user_input_is_pressed(void);

#endif // USER_INPUT_H