/**
 * @file crc_driver.h
 * @brief This header file is the low level crc driver layer.
 * @version 0.1
 * @date 2024-05-22
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef CRC_DRIVER_H
#define CRC_DRIVER_H

// --- includes --------------------------------------------------------------------------------------------------------
#include <stdint.h>

// --- function declarations -------------------------------------------------------------------------------------------
uint32_t crc32_driver_calculate(uint8_t const *data, uint32_t size);
uint16_t crc16_driver_calculate(uint8_t const *data, uint32_t size);

#endif // CRC_DRIVER_H
