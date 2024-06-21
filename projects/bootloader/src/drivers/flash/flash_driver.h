/**
 * @file flash_driver.h
 * @brief This header file is the low level flash driver layer.
 * @version 0.1
 * @date 2024-05-22
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef FLASH_DRIVER_H
#define FLASH_DRIVER_H

// --- includes --------------------------------------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>

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

// --- function declarations -------------------------------------------------------------------------------------------
void flash_driver_read(uint8_t *p_dest, const uint8_t *p_src, uint32_t length_bytes);
bool flash_driver_erase(uint32_t start_address, uint32_t end_address);
bool flash_driver_program(const uint8_t *p_src_ram, uint32_t flash_address, uint32_t length_bytes);

#endif // FLASH_DRIVER_H