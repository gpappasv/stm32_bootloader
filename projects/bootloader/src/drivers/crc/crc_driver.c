/**
 * @file crc_driver.c
 * @author George Pappas (pappasgeorge12@gmail.com)
 * @brief This source file is the low level crc driver layer.
 * @version 0.1
 * @date 2024-05-22
 *
 * @copyright Copyright (c) 2024
 *
 */

// --- includes --------------------------------------------------------------------------------------------------------
#include "crc_driver.h"

#include <stdio.h>
#include "stm32f4xx_hal.h"

// --- static variable definitions -------------------------------------------------------------------------------------
CRC_HandleTypeDef hcrc;

// --- static function declarations ------------------------------------------------------------------------------------
static void MX_CRC_Init(void);

// --- static function definitions -------------------------------------------------------------------------------------
/**
 * @brief CRC Initialization Function
 * @param None
 * @retval None
 */
static void
MX_CRC_Init(void)
{
    hcrc.Instance = CRC;
    if (HAL_CRC_Init(&hcrc) != HAL_OK)
    {
        printf("Error initializing crc module\n");
    }
}

// --- function definitions --------------------------------------------------------------------------------------------
void crc_driver_init(void)
{
    MX_CRC_Init();
}