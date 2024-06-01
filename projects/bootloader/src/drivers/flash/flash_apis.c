/**
 * @file flash_apis.c
 * @author George Pappas (pappasgeorge12@gmail.com)
 * @brief This source file provides the important flash apis to the bootloader code.
 * @version 0.1
 * @date 2024-05-22
 *
 * @copyright Copyright (c) 2024
 *
 */

// --- includes --------------------------------------------------------------------------------------------------------
#include "flash_apis.h"

#include <stdio.h>
#include <cmsis_compiler.h>
#include <stdbool.h>
#include <stdint.h>
#include "common.h"

// --- static function declarations ------------------------------------------------------------------------------------
static bool flash_api_erase_primary_space(void);
static bool flash_api_erase_secondary_space(void);

// --- static function definitions -------------------------------------------------------------------------------------
/**
 * @brief Function to erase the flash space that the app primary resides in.
 *
 * @return true
 * @return false
 */
static bool
flash_api_erase_primary_space(void)
{
    bool ret = true;
    // Erase the selected sectors
    ret = flash_driver_erase(((uint32_t)&__flash_app_start__), ((uint32_t)&__flash_app_end__));
    if (!ret)
    {
        printf("Error while erasing app primary flash data\r\n");
    }

    return ret;
}

/**
 * @brief Function to erase the flash space that the app secondary resides in.
 *
 * @return true
 * @return false
 */
static bool
flash_api_erase_secondary_space(void)
{
    bool ret = true;
    // Erase the selected sectors
    ret = flash_driver_erase(((uint32_t)&__flash_app_secondary_start__), ((uint32_t)&__flash_app_secondary_end__));

    if (!ret)
    {
        printf("Error while erasing app secondary flash data\n");
    }
    return ret;
}

// --- function definitions --------------------------------------------------------------------------------------------
/**
 * @brief Function to transfer the app data from secondary space to primary.
 *
 * @return true
 * @return false
 */
bool
flash_api_transfer_secondary_to_primary(void)
{
    bool ret = true;

    uint32_t secondary_start_addr = ((uint32_t)&__flash_app_secondary_start__);
    uint32_t secondary_img_size_bytes
        = ((uint32_t)&__flash_app_secondary_end__) - ((uint32_t)&__flash_app_secondary_start__) + 1;

    uint32_t primary_start_addr     = ((uint32_t)&__flash_app_start__);
    uint32_t primary_img_size_bytes = ((uint32_t)&__flash_app_end__) - ((uint32_t)&__flash_app_start__) + 1;

    printf("Attempt to transfer secondary to primary...\r\n");
    // Just make sure that primary img size is equal to secondary img size
    if (primary_img_size_bytes != secondary_img_size_bytes)
    {
        printf("Primary img size != secondary img size: check configuration\n");
        return false;
    }

    __disable_irq();
    // erase the primary space
    ret = flash_api_erase_primary_space();
    if (!ret)
    {
        printf("Error while erasing app primary space\r\n");
        return false;
    }

    bool rv = flash_driver_program((uint8_t *)secondary_start_addr, primary_start_addr, secondary_img_size_bytes);
    if (rv == false)
    {
        printf("Failed while transfering secondary slot to primary...\r\n");
        __enable_irq();
        return false;
    }
    __enable_irq();
    return true;
}
