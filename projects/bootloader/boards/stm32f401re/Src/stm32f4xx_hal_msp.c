/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file         stm32f4xx_hal_msp.c
 * @brief        This file provides code for the MSP Initialization
 *               and de-Initialization codes.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "uart_driver.h"
#include "stm32f4xx_hal.h"

/**
 * Initializes the Global MSP.
 */
void
HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);
}

/**
 * @brief CRC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hcrc: CRC handle pointer
 * @retval None
 */
void
HAL_CRC_MspInit(CRC_HandleTypeDef *hcrc)
{
    if (hcrc->Instance == CRC)
    {
        __HAL_RCC_CRC_CLK_ENABLE();
    }
}

/**
 * @brief CRC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hcrc: CRC handle pointer
 * @retval None
 */
void
HAL_CRC_MspDeInit(CRC_HandleTypeDef *hcrc)
{
    if (hcrc->Instance == CRC)
    {
        __HAL_RCC_CRC_CLK_DISABLE();
    }
}

/**
 * @brief UART MSP Initialization
 * This function configures the hardware resources used in this example
 * @param huart: UART handle pointer
 * @retval None
 */
void
HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    if (huart->Instance == USART2)
    {
        __HAL_RCC_USART2_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10     ------> USART1_RX
        */
        GPIO_InitStruct.Pin       = USART_TX_Pin | USART_RX_Pin;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART1 interrupt Init */
        HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
}

/**
 * @brief UART MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param huart: UART handle pointer
 * @retval None
 */
void
HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        /* Peripheral clock disable */
        __HAL_RCC_USART1_CLK_DISABLE();

        /**USART2 GPIO Configuration
        PA9     ------> USART1_TX
        PA10     ------> USART1_RX
        */
        HAL_GPIO_DeInit(GPIOA, USART_TX_Pin | USART_RX_Pin);

        /* USART1 interrupt DeInit */
        HAL_NVIC_DisableIRQ(USART2_IRQn);
    }
}
