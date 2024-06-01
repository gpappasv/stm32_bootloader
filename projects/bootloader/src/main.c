/**
 * @file main.c
 * @author George Pappas (pappasgeorge12@gmail.com)
 * @brief This source file is the main bootloader code.
 * @version 0.1
 * @date 2024-05-22
 *
 * @copyright Copyright (c) 2024
 *
 */

// --- includes --------------------------------------------------------------------------------------------------------
#include "main.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sys_init.h"
#include "sys.h"
#include "uart_driver.h"
#include "crc_apis.h"
#include "crc_driver.h"
#include "flash_apis.h"
#include "drivers/common.h"

// --- typedefs --------------------------------------------------------------------------------------------------------
typedef void (*bl_func_ptr)(void);

// --- static function declarations ------------------------------------------------------------------------------------
static void boot_application(void);

// --- static function definitions -------------------------------------------------------------------------------------
/**
 * @brief Function that boots the application
 *
 */
static void
boot_application(void)
{
    uint32_t    jump_address;
    bl_func_ptr jump_to_application;
    printf("BOOTLOADER Start \r\n");

    // check if there is something "installed" in the app FLASH region
    // TODO: GPA: need to find a specific pattern to identify our application
    if (((*(uint32_t *)((uint32_t) & __flash_app_start__)) & 0x2FFE0000) == 0x20000000)
    {
        // TODO: GPA: we can check here if the stack pointer is within valid RAM region
        // TODO: GPA: we can check here if the reset handler is a valid address, within the FLASH region
        printf("APP Start ...\r\n");
        // jump to the application
        jump_address        = *(uint32_t *)(((uint32_t) & __flash_app_start__) + 4);
        jump_to_application = (bl_func_ptr)jump_address;
        // initialize application's stack pointer
        sys_set_msp(((uint32_t) & __flash_app_start__));
        // prepare for the application
        sys_prepare_for_application();
        jump_to_application();
    }
    else
    {
        // there is no application installed
        printf("No APP found\r\n");
    }
}

// --- function definitions --------------------------------------------------------------------------------------------
/**
 * @brief  The application entry point.
 * @retval int
 */
int
main(void)
{
    sys_init();
    uart_driver_init();
    crc_driver_init();

    // Try to find and boot the application
    boot_application();
    while (1)
    {
        printf("Bootloader loop...\r\n");
        sys_delay_ms(1000);
    }
}
