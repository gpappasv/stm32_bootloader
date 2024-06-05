/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

void Error_Handler(void);
// --- defines ---------------------------------------------------------------------------------------------------------
// Flash sector start addresses and sizes
#define FLASH_SECTOR_0_START 0x08000000
#define FLASH_SECTOR_0_SIZE  (16 * 1024)
#define FLASH_SECTOR_1_START 0x08004000
#define FLASH_SECTOR_1_SIZE  (16 * 1024)
#define FLASH_SECTOR_2_START 0x08008000
#define FLASH_SECTOR_2_SIZE  (16 * 1024)
#define FLASH_SECTOR_3_START 0x0800C000
#define FLASH_SECTOR_3_SIZE  (16 * 1024)
#define FLASH_SECTOR_4_START 0x08010000
#define FLASH_SECTOR_4_SIZE  (64 * 1024)
#define FLASH_SECTOR_5_START 0x08020000
#define FLASH_SECTOR_5_SIZE  (128 * 1024)
#define FLASH_SECTOR_6_START 0x08040000
#define FLASH_SECTOR_6_SIZE  (128 * 1024)
#define FLASH_SECTOR_7_START 0x08060000
#define FLASH_SECTOR_7_SIZE  (128 * 1024)

// --- typedefs --------------------------------------------------------------------------------------------------------
typedef struct
{
    uint32_t start_address;
    uint32_t size;
} flash_sector;

// --- static variable definitions -------------------------------------------------------------------------------------
// STM32F401RE supports flash erase in sectors.
// clang-format off
/*
    |sec idx  |size kB|start addr|
    |Sector 0 | 16kB  |0x08000000| Bootloader memory
    |Sector 1 | 16kB  |0x08004000| Bootloader memory
    |Sector 2 | 16kB  |0x08008000| App memory primary
    |Sector 3 | 16kB  |0x0800C000| App memory primary
    |Sector 4 | 64kB  |0x08010000| App memory primary
    |Sector 5 | 128kB |0x08020000| App memory primary - total 224kB available for app primary
    |Sector 6 | 128kB |0x08040000| App secondary
    |Sector 7 | 128kB |0x08060000| App secondary - end address 0x08077FFF - app primary size == app secondary size
*/

// TODO: GPA: Need to properly split flash to allow application have a dedicated flash space for persistent storage.
static const flash_sector flash_sectors[] = {
    {FLASH_SECTOR_0_START, FLASH_SECTOR_0_SIZE},
    {FLASH_SECTOR_1_START, FLASH_SECTOR_1_SIZE},
    {FLASH_SECTOR_2_START, FLASH_SECTOR_2_SIZE},
    {FLASH_SECTOR_3_START, FLASH_SECTOR_3_SIZE},
    {FLASH_SECTOR_4_START, FLASH_SECTOR_4_SIZE},
    {FLASH_SECTOR_5_START, FLASH_SECTOR_5_SIZE},
    {FLASH_SECTOR_6_START, FLASH_SECTOR_6_SIZE},
    {FLASH_SECTOR_7_START, FLASH_SECTOR_7_SIZE},
};
// clang-format on

// --- defines ---------------------------------------------------------------------------------------------------------
#define FLASH_APP_START_SECTOR_IDX           2
#define FLASH_APP_END_SECTOR_IDX             5
#define FLASH_APP_SECONDARY_START_SECTOR_IDX 6
#define FLASH_APP_SECONDARY_END_SECTOR_IDX   7

#define FLASH_SECTOR_COUNT (sizeof(flash_sectors) / sizeof(flash_sectors[0]))

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

