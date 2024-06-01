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

// --- static function declarations ------------------------------------------------------------------------------------
static uint32_t compute_crc32(const uint8_t *data, uint32_t length);

// --- static function definitions -------------------------------------------------------------------------------------
static uint32_t
compute_crc32(const uint8_t *data, uint32_t length)
{
    uint32_t crc = 0xFFFFFFFF;

    for (uint32_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }

    return crc ^ 0xFFFFFFFF;
}

// --- function definitions --------------------------------------------------------------------------------------------
uint32_t
crc_driver_calculate(uint32_t *data, uint32_t size)
{
    uint32_t crc = 0;
    printf("Calculating CRC of %lu bytes from address %p to %p\r\n", size, data, data + size);
    crc = compute_crc32((uint8_t *)data, size);
    return crc;
}