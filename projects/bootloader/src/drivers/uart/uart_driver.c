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
#include "stm32f401xe.h" // stm32f401re
#include "sys_init.h"

// --- defines ---------------------------------------------------------------------------------------------------------
#define TIM1_COUNTDOWN_SEC 15 // 15 seconds timeout for the uart reception watchdog

// --- static variable definitions -------------------------------------------------------------------------------------
// This is the structure that will store the received buffer and the size of it. This will be used by the upper layers
// to receive the data.
static uint8_t uart_rx_buffer[RX_CHUNK_SIZE_BYTES] = { 0 };
static struct uart_driver_data_s uart_buf = { .data_buffer = uart_rx_buffer, .len = RX_CHUNK_SIZE_BYTES };

static process_rx_data data_rx_cb = NULL;

// --- variable definitions --------------------------------------------------------------------------------------------
UART_HandleTypeDef huart2;

// --- static function declarations ------------------------------------------------------------------------------------
static void uart_recv_it_init_wdg(void);
static void MX_USART2_UART_Init(void);

// --- static function definitions -------------------------------------------------------------------------------------
/**
 * @brief This function initializes the timer1 peripheral to be used as a watchdog for the uart reception.
 *        TODO: GPA: We can set the timer to one-pulse mode, so that it will stop after the timeout. The timer can be
 * re-enabled by the uart isr, only when needed.
 *
 */
static void
uart_recv_it_init_wdg(void)
{
    // Enable the TIM1 clock
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    TIM1->SMCR = 0;
    TIM1->DIER = TIM_DIER_UIE; // Enable update interrupt
    TIM1->SR   = 0;            // Clear status register
    TIM1->EGR  = TIM_EGR_UG;   // Generate an update event to reload the prescaler
    // Configure the prescaler such is to generate 1ms ticks. The systemclock is 84mhz
    TIM1->PSC = 21000 - 1; // this means 1ms -> 4000 ticks

    // Set the auto-reload value for a 15-second countdown
    TIM1->ARR = TIM1_COUNTDOWN_SEC * 4000 - 1;
    TIM1->CNT = TIM1->ARR; // Set the counter to the auto-reload value
    TIM1->CR2 = 0;
    // CR1 config: URS = 1, DIR = 1, CEN = 1
    TIM1->CR1 = (TIM_CR1_URS | TIM_CR1_DIR | TIM_CR1_CEN);

    // Clear any pending update interrupt flags
    TIM1->SR &= ~TIM_SR_UIF;
    // Enable the update interrupt for TIM1 in NVIC
    NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 0); // Set interrupt priority
    NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
}

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
    huart2.Init.BaudRate     = 1000000;
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
    // Init the uart watchdog
    uart_recv_it_init_wdg();
}

// --- function definitions --------------------------------------------------------------------------------------------
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

// --- application specific functions ----------------------------------------------------------------------------------
/**
 * @brief Function to feed the uart watchdog. Will be called by the uart isr, every time something is being received.
 *
 */
void
uart_driver_feed_wdg(void)
{
    // Reset the uart wdg timer to not trigger the buffer reset.
    TIM1->CNT = TIM1->ARR;
}

/**
 * @brief Function to recover the uart reception.
 *
 */
void
uart_driver_rx_recover(void)
{
    HAL_UART_DeInit(&huart2);
    HAL_UART_Init(&huart2);
    // Restart the reception
    HAL_UART_Receive_IT(&huart2, uart_buf.data_buffer, uart_buf.len);
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
 * @brief Error callback function, for uart errors
 *
 * @param huart
 */
void
HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        uart_driver_rx_recover();
    }
}