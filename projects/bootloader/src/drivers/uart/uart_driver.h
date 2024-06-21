/**
 * @file uart_driver.h
 * @brief This header file is the low level uart driver layer.
 * @version 0.1
 * @date 2024-05-22
 *
 * @copyright Copyright (c) 2024
 *
 */

// --- defines ---------------------------------------------------------------------------------------------------------
#define USART_TX_Pin GPIO_PIN_2
#define USART_RX_Pin GPIO_PIN_3

// --- function declarations -------------------------------------------------------------------------------------------
void uart_driver_init(void);