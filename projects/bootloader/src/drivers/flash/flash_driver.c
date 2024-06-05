/**
 * @file flash_driver.c
 * @author George Pappas (pappasgeorge12@gmail.com)
 * @brief This source file is the low level flash driver layer.
 * @version 0.1
 * @date 2024-05-22
 *
 * @copyright Copyright (c) 2024
 *
 */

// --- includes --------------------------------------------------------------------------------------------------------
#include "flash_driver.h"

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "common.h"

// --- defines ---------------------------------------------------------------------------------------------------------
#define FLASH_ERASE_NO_ERROR (0xFFFFFFFF)

// --- static function declarations ------------------------------------------------------------------------------------
static bool flash_driver_write_enable(void);
static bool flash_driver_write_disable(void);

// --- static function definitions -------------------------------------------------------------------------------------
static bool
flash_driver_write_enable(void)
{
    if (HAL_FLASH_Unlock() != HAL_OK)
    {
        printf("Flash write enable: failed\r\n");
        return false;
    }

    return true;
}

static bool
flash_driver_write_disable(void)
{
    if (HAL_FLASH_Lock() != HAL_OK)
    {
        printf("Flash write disable: failed\r\n");
        return false;
    }

    return true;
}

// --- function definitions --------------------------------------------------------------------------------------------
/**
 * @brief Function to erase the flash content in the specified address range. NOTE: the stm32f401re does not support
 * erasing a range of addresses, so the function erases the whole sectors that the range belongs to.
 *
 * @param start_address
 * @param end_address
 * @return true
 * @return false
 */
bool
flash_driver_erase(uint32_t start_address, uint32_t end_address)
{
    uint32_t               sector_error = 0;
    FLASH_EraseInitTypeDef erase;
    uint32_t               start_sector = FLASH_SECTOR_COUNT;
    uint32_t               end_sector   = FLASH_SECTOR_COUNT;

    // Calculate start and end sectors
    for (uint32_t i = 0; i < FLASH_SECTOR_COUNT; i++)
    {
        if (start_address >= flash_sectors[i].start_address
            && start_address < flash_sectors[i].start_address + flash_sectors[i].size)
        {
            start_sector = i;
        }
        if (end_address >= flash_sectors[i].start_address
            && end_address < flash_sectors[i].start_address + flash_sectors[i].size)
        {
            end_sector = i;
        }
    }

    // If the start or end sector is not found, return false
    if (start_sector == FLASH_SECTOR_COUNT || end_sector == FLASH_SECTOR_COUNT)
    {
        return false;
    }

    erase.TypeErase    = FLASH_TYPEERASE_SECTORS;
    erase.Sector       = start_sector;
    erase.NbSectors    = end_sector - start_sector + 1;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    if (flash_driver_write_enable() == false)
    {
        return false;
    }

    if (HAL_FLASHEx_Erase(&erase, &sector_error) == HAL_OK)
    {
        if (sector_error == FLASH_ERASE_NO_ERROR)
        {
            flash_driver_write_disable();
            return true;
        }
    }
    flash_driver_write_disable();

    return false;
}

/**
 * @brief Function to read the flash content from the specified address to the destination buffer.
 *
 * @param p_dest
 * @param p_src
 * @param length_bytes
 */
void
flash_driver_read(uint8_t *p_dest, const uint8_t *p_src, uint32_t length_bytes)
{
    while (length_bytes != 0)
    {
        *(p_dest++) = *(p_src++);
        length_bytes--;
    };
}

/**
 * @brief Function to write the data from the source RAM to the flash memory. The data is written to the specified
 * address. The function writes the data byte by byte. The function returns true if the write is successful, otherwise
 * false.
 *
 * @param p_src_ram
 * @param flash_address
 * @param length_bytes
 * @return true
 * @return false
 */
bool
flash_driver_program(const uint8_t *p_src_ram, uint32_t flash_address, uint32_t length_bytes)
{
    uint32_t i;

    if (p_src_ram == NULL)
    {
        printf("Flash write: null pointer input\n");
    }

    // check if data will be written in a valid address
    if ((flash_address < ((uint32_t)&__flash_app_start__))
        || ((flash_address + length_bytes) > ((uint32_t)&__flash_app_secondary_end__)))
    {
        printf("Flash write: failed\n");
        return false;
    }

    flash_driver_write_enable();
    // Programming flash only when address is valid
    for (i = 0; i < length_bytes; i++)
    {
        // Write data to flash
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, flash_address, (uint64_t)p_src_ram[i]) != HAL_OK)
        {
            printf("Flash program: failed\n");
            flash_driver_write_disable();
            return false;
        }

        // Move to the next byte
        flash_address += 1;
    }
    flash_driver_write_disable();
    return true;
}
