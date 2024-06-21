/**
 * @file mpu_driver.c
 * @brief This source file is the low level mpu driver layer.
 * @version 0.1
 * @date 2024-05-22
 *
 * @copyright Copyright (c) 2024
 *
 */

// --- includes --------------------------------------------------------------------------------------------------------
#include "mpu_driver.h"

#include "stm32f4xx.h"
#include "stm32f4xx_ll_cortex.h"
#include "cmsis_version.h"
#include "common.h"
#include <stdio.h>

// --- function definitions --------------------------------------------------------------------------------------------
void
mpu_config_lock(void)
{
    // Disable the MPU
    LL_MPU_Disable();
    // Configure the MPU region 0: 32KB of flash memory as read-only for privileged and unprivileged access
    // TODO: GPA: Later, we will need to also have a space for the security keys. That space should be non-accessible.
    // The non-accessible configuration will take place here, after the bootloader has already accessed the keys to
    // authenticate the primary application. Here, the key region will be completely locked.
    LL_MPU_ConfigRegion(LL_MPU_REGION_NUMBER0,
                        0,
                        0x08000000,
                        LL_MPU_REGION_PRIV_RO_URO | LL_MPU_INSTRUCTION_ACCESS_ENABLE | LL_MPU_REGION_SIZE_32KB);
    // Enable Region 0
    LL_MPU_EnableRegion(0);
    // Enable the MPU with default memory map as background region
    LL_MPU_Enable(LL_MPU_CTRL_PRIVILEGED_DEFAULT);
}