/**
 * @file main.h
 * @author George Pappas (pappasgeorge12@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2024-05-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __MAIN_H
#define __MAIN_H

// --- includes --------------------------------------------------------------------------------------------------------
#include "stm32f4xx_hal.h"

// --- defines ---------------------------------------------------------------------------------------------------------
#define B1_Pin             GPIO_PIN_13
#define B1_GPIO_Port       GPIOC
#define USART_TX_Pin       GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin       GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin            GPIO_PIN_5
#define LD2_GPIO_Port      GPIOA
#define TMS_Pin            GPIO_PIN_13
#define TMS_GPIO_Port      GPIOA
#define TCK_Pin            GPIO_PIN_14
#define TCK_GPIO_Port      GPIOA
#define SWO_Pin            GPIO_PIN_3
#define SWO_GPIO_Port      GPIOB

#endif /* __MAIN_H */
