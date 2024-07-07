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
/**
 * @brief Generic structure to hold the data buffer and its length. This is used to pass the received data to the upper
 *        layers. The data_buffer is a pointer to the buffer, and the len is the length of the buffer. The data buffer
 *        might not be of fixed size, so the len is used to know how many bytes are in the buffer.
 * 
 */
struct uart_driver_data_s
{
    uint8_t  *data_buffer;
    uint16_t len;
} __attribute__((packed));

// --- typedefs --------------------------------------------------------------------------------------------------------
typedef void (*process_rx_data)(struct uart_driver_data_s * const rx_data);

// --- function declarations -------------------------------------------------------------------------------------------
/**
 * @brief NOTE: To port the bootloader to a different platform, this function must be implemented accordingly to initialize
 *       the uart peripheral.
 * 
 */
void uart_driver_init(void);

/**
 * @brief NOTE: This function is important, in order to register the callback function that will be called when uart
 *              driver decides that the reception is done. The com_protocol layer will use this callback to process the
 *              received data.
 * 
 */
void uart_driver_register_rx_callback(process_rx_data rx_cb);

/**
 * @brief This function is used to send data over the uart. The data is sent as a buffer of bytes.
 * 
 * @param buffer: The buffer containing the data to be sent.
 * @param length: The length of the buffer.
 */
void uart_tx_data(uint8_t *buffer, uint16_t length);

// --- application specific functions ----------------------------------------------------------------------------------
// NOTE: These functions are not necessary for the bootloader, but rather for the specific uart driver implementation.
void uart_driver_feed_wdg(void);
void uart_driver_rx_recover(void);