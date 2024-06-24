/**
 * @file crc_apis.c
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
#include <stdio.h>
#include "crc_driver.h"
#include "common.h"

// --- function definitions --------------------------------------------------------------------------------------------
/**
 * @brief Function that checks the primary application's CRC. It calculates the CRC of the primary application and
 * compares it with the stored CRC.
 *
 * @return true
 * @return false
 */
bool
crc_api_check_primary_app(void)
{
    // Calculate the CRC of the primary application
    uint32_t crc = crc32_driver_calculate((uint8_t *)((uint32_t)&__flash_app_start__),
                                        ((uint32_t)&__flash_app_end__) - ((uint32_t)&__flash_app_start__)
                                            - ((uint32_t)&__header_size_bytes__) + 1);

    // Compare the calculated CRC with the stored CRC
    uint32_t stored_crc = *((uint32_t *)(&__header_app_crc_start__));
    if (crc != stored_crc)
    {
#ifdef DEBUG_LOG
        printf("CRC mismatch for primary app: calculated 0x%08lX, stored 0x%08lX\r\n", crc, stored_crc);
#endif
        return false;
    }
#ifdef DEBUG_LOG
    printf("CRC match for primary app: calculated 0x%08lX, stored 0x%08lX\r\n", crc, stored_crc);
#endif
    return true;
}

/**
 * @brief Function that checks the secondary application's CRC. It calculates the CRC of the secondary application and
 * compares it with the stored CRC.
 *
 * @return true
 * @return false
 */
bool
crc_api_check_secondary_app(void)
{
    // Calculate the CRC of the secondary application
    uint32_t crc
        = crc32_driver_calculate((uint8_t *)((uint32_t)&__flash_app_secondary_start__),
                               ((uint32_t)&__flash_app_secondary_end__) - ((uint32_t)&__flash_app_secondary_start__)
                                   - ((uint32_t)&__header_size_bytes__) + 1);

    // Compare the calculated CRC with the stored CRC
    uint32_t stored_crc = *((uint32_t *)(&__header_app_secondary_crc_start__));
    if (crc != stored_crc)
    {
#ifdef DEBUG_LOG
        printf("CRC mismatch for secondary app: calculated 0x%08lX, stored 0x%08lX\r\n", crc, stored_crc);
#endif
        return false;
    }
#ifdef DEBUG_LOG
    printf("CRC match for secondary app: calculated 0x%08lX, stored 0x%08lX\r\n", crc, stored_crc);
#endif
    return true;
}