/**
 * @file sys_init.c
 * @author George Pappas (pappasgeorge12@gmail.com)
 * @brief This source file is the low level system driver layer. Here will reside whatever is needed for the system to
 *        start (e.g. clock configurations)
 * @version 0.1
 * @date 2024-05-22
 *
 * @copyright Copyright (c) 2024
 *
 */

// --- includes --------------------------------------------------------------------------------------------------------
#include "sys.h"

#include "stm32f4xx_hal.h"

// --- function definitions --------------------------------------------------------------------------------------------
void
sys_delay_ms(uint32_t delay)
{
    HAL_Delay(delay);
}

void
set_msp(size_t addr)
{
    __set_MSP(*(uint32_t *)addr);
}
