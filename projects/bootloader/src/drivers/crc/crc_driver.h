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
uint32_t crc_driver_calculate(uint32_t *data, uint32_t size);

#endif // CRC_DRIVER_H
