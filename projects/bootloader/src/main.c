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

// --- defines ---------------------------------------------------------------------------------------------------------
#define FLASH_APP_START_ADDR 0x8008000 // Bootloader reserves the first 32KB

// --- typedefs --------------------------------------------------------------------------------------------------------
typedef void (*bl_func_ptr)(void);

// --- static variable definitions -------------------------------------------------------------------------------------
CRC_HandleTypeDef  hcrc;
UART_HandleTypeDef huart2; // Change from huart1 to huart2

// --- static function declarations ------------------------------------------------------------------------------------
static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void); // Change from MX_USART1_UART_Init to MX_USART2_UART_Init
static void MX_CRC_Init(void);
static void Error_Handler(void);
static void boot_application(void);

// --- static function definitions -------------------------------------------------------------------------------------
/**
 * @brief System Clock Configuration
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
        Error_Handler();
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
        Error_Handler();
    }
}

/**
 * @brief CRC Initialization Function
 * @param None
 * @retval None
 */
static void
MX_CRC_Init(void)
{
    hcrc.Instance = CRC;
    if (HAL_CRC_Init(&hcrc) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void
MX_USART2_UART_Init(void) // Change from MX_USART1_UART_Init to MX_USART2_UART_Init
{
    huart2.Instance          = USART2; // Change from USART1 to USART2
    huart2.Init.BaudRate     = 115200;
    huart2.Init.WordLength   = UART_WORDLENGTH_8B;
    huart2.Init.StopBits     = UART_STOPBITS_1;
    huart2.Init.Parity       = UART_PARITY_NONE;
    huart2.Init.Mode         = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_8;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void
MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin : B1_Pin */
    GPIO_InitStruct.Pin  = B1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : USART_TX_Pin USART_RX_Pin */
    GPIO_InitStruct.Pin       = USART_TX_Pin | USART_RX_Pin;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pin : LD2_Pin */
    GPIO_InitStruct.Pin   = LD2_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);
}

int
_write(int file, char *ptr, int len)
{
    (void)file;
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        HAL_UART_Transmit(&huart2, (uint8_t *)ptr++, 1, 100);
    }
    return len;
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
static void
Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

/**
 * @brief Function that boots the application
 *
 */
static void
boot_application(void)
{
    uint32_t    JumpAddress;
    bl_func_ptr Jump_To_Application;
    printf("BOOTLOADER Start \r\n");

    // check if there is something "installed" in the app FLASH region
    // TODO: GPA: need to find a specific pattern to identify our application
    if (((*(uint32_t *)FLASH_APP_START_ADDR) & 0x2FFE0000) == 0x20000000)
    {
        printf("APP Start ...\r\n");
        HAL_Delay(100);
        // jump to the application
        JumpAddress         = *(uint32_t *)(FLASH_APP_START_ADDR + 4);
        Jump_To_Application = (bl_func_ptr)JumpAddress;
        // initialize application's stack pointer
        __set_MSP(*(uint32_t *)FLASH_APP_START_ADDR);
        Jump_To_Application();
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
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init(); // Change from MX_USART1_UART_Init to MX_USART2_UART_Init
    MX_CRC_Init();

    // Try to find and boot the application
    boot_application();
    while (1)
    {
        printf("Bootloader loop...\r\n");
        HAL_Delay(1000);
    }
}
