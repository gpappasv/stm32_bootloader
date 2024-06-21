/**
 * @file uart_driver.c
 * @author George Pappas (pappasgeorge12@gmail.com)
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
#include "stm32f4xx_hal.h"
#include "sys_init.h"

// --- defines ---------------------------------------------------------------------------------------------------------
#define RX_CHUNK_SIZE_BYTES 256

// --- static variable definitions -------------------------------------------------------------------------------------
uint8_t uart_rx_buf[RX_CHUNK_SIZE_BYTES];
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
        printf("Error initializing uart\n");
    }
    HAL_UART_Receive_IT(&huart2, uart_rx_buf, RX_CHUNK_SIZE_BYTES); // Start reception
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
        // Handle received data
        // TODO: GPA: At this point handle the received data and restart the receptions
        HAL_UART_Receive_IT(&huart2, uart_rx_buf, RX_CHUNK_SIZE_BYTES);
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
        HAL_UART_Receive_IT(&huart2, uart_rx_buf, RX_CHUNK_SIZE_BYTES);
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
