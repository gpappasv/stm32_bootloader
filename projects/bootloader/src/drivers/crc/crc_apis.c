/**
 * @file crc_apis.c
 * @author George Pappas (pappasgeorge12@gmail.com)
 * @brief This source file is the higher level crc calculation layer, providing an API to the application.
 * @version 0.1
 * @date 2024-05-22
 *
 * @copyright Copyright (c) 2024
 *
 */

// --- includes --------------------------------------------------------------------------------------------------------
#include "crc_apis.h"

#include <stdbool.h>
#include <stdint.h>
#include "crc_driver.h"
#include "common.h"

// --- function definitions --------------------------------------------------------------------------------------------
bool
crc_api_check_primary_app(void)
{
    // Calculate the CRC of the primary application
    uint32_t crc = crc_driver_calculate((uint32_t *)((uint32_t)&__flash_app_start__),
                                        ((uint32_t)&__flash_app_end__) - ((uint32_t)&__flash_app_start__) + 1);

    // Compare the calculated CRC with the stored CRC
    return true;
}

bool
crc_api_check_secondary_app(void)
{
    // Calculate the CRC of the secondary application
    uint32_t crc = crc_driver_calculate((uint32_t *)((uint32_t)&__flash_app_secondary_start__),
                                        ((uint32_t)&__flash_app_secondary_end__) - ((uint32_t)&__flash_app_secondary_start__) + 1);

    // Compare the calculated CRC with the stored CRC
    return true;
}