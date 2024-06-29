/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32f4xx_it.c
 * @brief   Interrupt Service Routines.
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
#include "stm32f4xx_it.h"
#include "stm32f401xe.h"
#include <stdio.h>

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart2;

/**
 * @brief This function handles Non maskable interrupt.
 */
void
NMI_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void
HardFault_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief This function handles Memory management fault.
 */
void
MemManage_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief This function handles Pre-fetch fault, memory access fault.
 */
void
BusFault_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void
UsageFault_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief This function handles System service call via SWI instruction.
 */
void
SVC_Handler(void)
{
}

/**
 * @brief This function handles Debug monitor.
 */
void
DebugMon_Handler(void)
{
}

/**
 * @brief This function handles Pendable request for system service.
 */
void
PendSV_Handler(void)
{
}

/**
 * @brief This function handles System tick timer.
 */
void
SysTick_Handler(void)
{
    HAL_IncTick();
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles USART2 global interrupt.
 */
void
USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
    uart_driver_feed_wdg();
}

void
TIM1_UP_TIM10_IRQHandler(void)
{
    if (TIM1->SR & TIM_SR_UIF) // If update interrupt flag is set
    {
        TIM1->SR &= ~TIM_SR_UIF; // Clear the interrupt flag
        uart_driver_rx_recover();
    }
}