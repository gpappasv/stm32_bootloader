/**
 * @file uart_driver.c
 * @brief This source file is the low level uart driver layer.
 * @version 0.1
 * @date 2024-05-22
 *
 * @copyright Copyright (c) 2024
 *
 */

// --- includes --------------------------------------------------------------------------------------------------------
#include "uart_driver.h"

#include <stdio.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "sys_init.h"

// --- static variable definitions -------------------------------------------------------------------------------------
// This is the structure that will store the received buffer and the size of it. This will be used by the upper layers
// to receive the data.
static struct uart_driver_data_s uart_buf = { .data_buffer = { 0 }, .len = RX_CHUNK_SIZE_BYTES };

static process_rx_data data_rx_cb = NULL;

// --- variable definitions --------------------------------------------------------------------------------------------
UART_HandleTypeDef huart2;

// --- static function declarations ------------------------------------------------------------------------------------
static void MX_USART2_UART_Init(void);

// --- static function definitions -------------------------------------------------------------------------------------
/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void
MX_USART2_UART_Init(void) // Change from MX_USART1_UART_Init to MX_USART2_UART_Init
{
    // Init the relevant gpios
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pins : USART_TX_Pin USART_RX_Pin */
    GPIO_InitStruct.Pin       = USART_TX_Pin | USART_RX_Pin;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Init the uart peripheral
    huart2.Instance          = USART2;
    huart2.Init.BaudRate     = 115200;
    huart2.Init.WordLength   = UART_WORDLENGTH_8B;
    huart2.Init.StopBits     = UART_STOPBITS_1;
    huart2.Init.Parity       = UART_PARITY_NONE;
    huart2.Init.Mode         = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_8;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
#ifdef DEBUG_LOG
        printf("Error initializing uart\n");
#endif
    }
    HAL_UART_Receive_IT(&huart2, uart_buf.data_buffer, uart_buf.len); // Start reception
}

// --- function definitions --------------------------------------------------------------------------------------------
/**
 * @brief Function to forward the printf output to the uart peripheral.
 *
 * @param file
 * @param ptr
 * @param len
 * @return int
 */
int
_write(int file, char *ptr, int len)
{
    (void)file;
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        HAL_UART_Transmit(&huart2, (uint8_t *)ptr++, 1, 100);
    }
    return len;
}

/**
 * @brief Callback function that is being called automatically when the uart rx is finished. Processes the data and
 *        restarts the uart reception.
 *
 * @return int
 */
void
HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        // Call the register callback function if it is set
        if (data_rx_cb != NULL)
        {
            data_rx_cb(&uart_buf);
        }
        // Handle received data
        // TODO: GPA: At this point handle the received data and restart the receptions
        HAL_UART_Receive_IT(&huart2, uart_buf.data_buffer, uart_buf.len);
    }
}

/**
 * @brief Error callback function, for uart errors
 *
 * @param huart
 */
void
HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        // TODO: GPA: Handle the possible errors and restart the reception
        HAL_UART_Receive_IT(&huart2, uart_buf.data_buffer, uart_buf.len);
    }
}

/**
 * @brief Function to initialize the uart peripheral, of the stm32f401re.
 *
 */
void
uart_driver_init(void)
{
    MX_USART2_UART_Init();
}

/**
 * @brief Function to register an rx callback function. This will be used by the com_protocol to register a callback
 *        function and process the received message.
 *
 */
void
uart_driver_register_rx_callback(process_rx_data rx_cb)
{
    data_rx_cb = rx_cb;
}

/**
 * @brief Function to transmit a buffer of data via UART.
 *
 * @param buffer The data buffer to be transmitted.
 * @param length The length of the data buffer.
 * @return none.
 */
void
uart_tx_data(uint8_t *buffer, uint16_t length)
{
    HAL_UART_Transmit(&huart2, buffer, length, HAL_MAX_DELAY);
}