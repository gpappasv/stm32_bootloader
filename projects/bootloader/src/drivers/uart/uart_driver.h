/**
 * @file uart_driver.h
 * @brief This header file is the low level uart driver layer.
 * @version 0.1
 * @date 2024-05-22
 *
 * @copyright Copyright (c) 2024
 *
 */

// --- includes --------------------------------------------------------------------------------------------------------
#include <stdint.h>

// --- defines ---------------------------------------------------------------------------------------------------------
#define USART_TX_Pin        GPIO_PIN_2
#define USART_RX_Pin        GPIO_PIN_3
#define RX_CHUNK_SIZE_BYTES 256 // TODO: GPA: Right now, we need to send a buffer of 256 bytes, even if the effective payload is smaller.

// --- structs ---------------------------------------------------------------------------------------------------------
struct uart_driver_data_s
{
    uint8_t  data_buffer[RX_CHUNK_SIZE_BYTES];
    uint16_t len;
} __attribute__((packed));

// --- typedefs --------------------------------------------------------------------------------------------------------
typedef void (*process_rx_data)(struct uart_driver_data_s * const rx_data);

// --- function declarations -------------------------------------------------------------------------------------------
void uart_driver_init(void);
void uart_driver_feed_wdg(void);
void uart_driver_rx_recover(void);
void uart_driver_register_rx_callback(process_rx_data rx_cb);
void uart_tx_data(uint8_t *buffer, uint16_t length);