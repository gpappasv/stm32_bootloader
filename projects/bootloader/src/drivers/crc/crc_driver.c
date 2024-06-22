/**
 * @file crc_driver.c
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
static uint16_t compute_crc16(const uint8_t *data, uint32_t length);

// --- static function definitions -------------------------------------------------------------------------------------
/**
 * @brief Function to compute the CRC32 of the given data. It uses the CRC32 software implementation. The data is passed
 * as a pointer to the start of the data and the length of the data. Returns the calculated CRC.
 *
 * @param data
 * @param length
 * @return uint32_t
 */
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

/**
 * @brief Function to compute the CRC16 of the given data. It uses the CRC16-CCITT (0x1021) polynomial.
 * The data is passed as a pointer to the start of the data and the length of the data. Returns the calculated CRC.
 *
 * @param data
 * @param length
 * @return uint16_t
 */
static uint16_t
compute_crc16(const uint8_t *data, uint32_t length)
{
    uint16_t crc = 0xFFFF;

    for (uint32_t i = 0; i < length; i++)
    {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }

    return crc;
}

// --- function definitions --------------------------------------------------------------------------------------------
/**
 * @brief Function to calculate the CRC of the given data. It uses the CRC32 software implementation. The data is
 * passed as a pointer to the start of the data and the size of the data. Returns the calculated CRC.
 *
 * @param data
 * @param size
 * @return uint32_t calculated CRC
 */
uint32_t
crc32_driver_calculate(uint8_t const *data, uint32_t size)
{
    uint32_t crc = 0;
#ifdef DEBUG_LOG
    printf("Calculating CRC32 of %lu bytes from address %p to %p\r\n", size, data, data + size);
#endif
    crc = compute_crc32(data, size);
    return crc;
}

/**
 * @brief Function to calculate the CRC16 of the given data. The data is passed as a pointer to the start of the data
 * and the size of the data. Returns the calculated CRC.
 *
 * @param data
 * @param size
 * @return uint16_t calculated CRC
 */
uint16_t
crc16_driver_calculate(uint8_t const *data, uint32_t size)
{
    uint16_t crc = 0;
#ifdef DEBUG_LOG
    printf("Calculating CRC16 of %lu bytes from address %p to %p\r\n", size, data, data + size);
#endif
    crc = compute_crc16(data, size);
    return crc;
}