/**
 * @file sys_init.c
 * @author George Pappas (pappasgeorge12@gmail.com)
 * @brief This source file is the low level system driver layer. Here will reside whatever is needed for the system to
 *        start (e.g. clock configurations)
 * @version 0.1
 * @date 2024-05-22
 *
 * @copyright Copyright (c) 2024
 *
 */

// --- includes --------------------------------------------------------------------------------------------------------
#include "sys_init.h"

#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "common.h"
#include "mpu/mpu_driver.h"

// --- static function declarations ------------------------------------------------------------------------------------
static void SystemClock_Config(void);
static void set_unprivileged_mode(void);

// --- static function definitions -------------------------------------------------------------------------------------
/**
 * @brief System Clock Configuration. Given by STM32CubeMX.
 *
 *
 * @retval None
 */
static void
SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM            = 16;
    RCC_OscInitStruct.PLL.PLLN            = 336;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ            = 7;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        printf("Error initializing RCC\n");
    }
    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        printf("Error initializing RCC\n");
    }
}

static void
set_unprivileged_mode(void)
{
    // Set CONTROL register to switch to unprivileged mode
    __asm__("MOV R0, #1");      // Move the value of CONTROL register into R0
    __asm__("MSR CONTROL, R0"); // Move the modified value back to CONTROL register
    __DSB();                    // Data Synchronization Barrier
    __ISB();                    // Instruction Synchronization Barrier
}

// --- function definitions --------------------------------------------------------------------------------------------
/**
 * @brief Function that encapsulates the system initialization process
 *
 */
void
sys_init(void)
{
    SystemClock_Config();
    HAL_Init();
}

/**
 * @brief Prepare the system for the application to run. This means deinitializing peripherals and preparing the vector
 *       table for the application.
 *
 */
void
sys_prepare_for_application(void)
{
    printf("Deinitializing peripherals and preparing for application start\r\n");
    // Deinitialize peripherals to their reset state
    HAL_RCC_DeInit();
    HAL_DeInit();

    // Disable all interrupts
    __disable_irq();
    // Clear pending interrupts
    for (int i = 0; i < 8; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    // Set the vector table to the application's vector table
    SCB->VTOR = (uint32_t)&__flash_app_start__;
    // Enable the MPU, to protect the bootloader.
    mpu_config_lock();
    // TODO: GPA: It is important to set the unprivileged mode after the MPU is enabled, to protect bootloader.
    // Right now, setting the unprivileged mode is commented out, because it will cause the bootloader to crash.
    //set_unprivileged_mode();
    __enable_irq();
}