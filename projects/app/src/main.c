/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
UART_HandleTypeDef huart1;
extern uint32_t __flash_app_start__;
extern uint32_t __flash_app_end__;
extern uint32_t __flash_app_secondary_start__;
extern uint32_t __flash_app_secondary_end__;

extern uint32_t __header_size_bytes__;
extern uint32_t __header_crc_size_bytes__;
extern uint32_t __header_fw_ver_size_bytes__;
extern uint32_t __header_hash_size_bytes__;

extern uint32_t __header_app_start__;
extern uint32_t __header_app_end__;
extern uint32_t __header_app_crc_start__;
extern uint32_t __header_app_fw_version_start__;
extern uint32_t __header_app_hash_start__;

extern uint32_t __header_app_secondary_start__;
extern uint32_t __header_app_secondary_end__;
extern uint32_t __header_app_secondary_crc_start__;
extern uint32_t __header_app_secondary_fw_version_start__;
extern uint32_t __header_app_secondary_hash_start__;
#define FLASH_ERASE_NO_ERROR (0xFFFFFFFF)

void        SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void UART1_SendString(const char *str);
static bool
flash_driver_write_enable(void)
{
    if (HAL_FLASH_Unlock() != HAL_OK)
    {
        printf("Flash write enable: failed\r\n");
        return false;
    }

    return true;
}

static bool
flash_driver_write_disable(void)
{
    if (HAL_FLASH_Lock() != HAL_OK)
    {
        printf("Flash write disable: failed\r\n");
        return false;
    }

    return true;
}

void
flash_driver_read(uint8_t *p_dest, const uint8_t *p_src, uint32_t length_bytes)
{
    while (length_bytes != 0)
    {
        *(p_dest++) = *(p_src++);
        length_bytes--;
    };
}

bool
flash_driver_erase(uint32_t start_address, uint32_t end_address)
{
    uint32_t               sector_error = 0;
    FLASH_EraseInitTypeDef erase;
    uint32_t               start_sector = FLASH_SECTOR_COUNT;
    uint32_t               end_sector   = FLASH_SECTOR_COUNT;

    // Calculate start and end sectors
    for (uint32_t i = 0; i < FLASH_SECTOR_COUNT; i++)
    {
        if (start_address >= flash_sectors[i].start_address
            && start_address < flash_sectors[i].start_address + flash_sectors[i].size)
        {
            start_sector = i;
        }
        if (end_address >= flash_sectors[i].start_address
            && end_address < flash_sectors[i].start_address + flash_sectors[i].size)
        {
            end_sector = i;
        }
    }

    // If the start or end sector is not found, return false
    if (start_sector == FLASH_SECTOR_COUNT || end_sector == FLASH_SECTOR_COUNT)
    {
        return false;
    }

    erase.TypeErase    = FLASH_TYPEERASE_SECTORS;
    erase.Sector       = start_sector;
    erase.NbSectors    = end_sector - start_sector + 1;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    if (flash_driver_write_enable() == false)
    {
        return false;
    }

    if (HAL_FLASHEx_Erase(&erase, &sector_error) == HAL_OK)
    {
        if (sector_error == FLASH_ERASE_NO_ERROR)
        {
            flash_driver_write_disable();
            return true;
        }
    }
    flash_driver_write_disable();

    return false;
}
bool
flash_driver_program(const uint8_t *p_src_ram, uint32_t flash_address, uint32_t length_bytes)
{
    uint32_t i;

    if (p_src_ram == NULL)
    {
        printf("Flash write: null pointer input\n");
        UART1_SendString("Flash write: null pointer input\r\n");
    }

    // // check if data will be written in a valid address
    // if ((flash_address < ((uint32_t)&__flash_app_start__))
    //     || ((flash_address + length_bytes) > ((uint32_t)&__flash_app_secondary_end__)))
    // {
    //     printf("Flash write: failed\n");
    //     UART1_SendString("Flash write: failed\r\n");
    //     return false;
    // }

    flash_driver_write_enable();
    // Programming flash only when address is valid
    for (i = 0; i < length_bytes; i++)
    {
        // Write data to flash
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, flash_address, (uint64_t)p_src_ram[i]) != HAL_OK)
        {
            printf("Flash program: failed\n");
            UART1_SendString("Flash program: failed\r\n");
            flash_driver_write_disable();
            return false;
        }

        // Move to the next byte
        flash_address += 1;
    }
    flash_driver_write_disable();
    return true;
}

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
    MX_USART1_UART_Init();
    // Send a hello message via UART1
    UART1_SendString("Hello from STM32!\r\n");

    // Counter variable
    int counter = 0;

    while (1)
    {
        // Send the counter value via UART1
        char buffer[20];
        sprintf(buffer, "Counter: %d\r\n", counter);
        UART1_SendString(buffer);

        // Increment the counter
        counter++;

        // Blink the stm32f401re nucleo board LED, after initing
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        // // Read flash memory address content on 0x08000000
        uint8_t flash_data[10];
        flash_driver_read(flash_data, (uint8_t *)0x08000000, 10);
        //flash_driver_program((uint8_t *)"Hello", 0x08000000, 5);
        // Delay for a short period
        HAL_Delay(1000); // Delay for 1 second
    }
}

static void
UART1_SendString(const char *str)
{
    // Transmit the string via UART1
    HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void
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
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void
MX_USART1_UART_Init(void)
{
    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = 115200;
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_8;
    if (HAL_UART_Init(&huart1) != HAL_OK)
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

    /* Enable GPIO clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configure USART1 Tx and Rx pins */
    GPIO_InitStruct.Pin       = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;      // Alternate function, push-pull
    GPIO_InitStruct.Pull      = GPIO_PULLUP;          // Pull-up resistor enabled
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH; // High-speed output
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;      // USART1 alternate function
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Configure PA5 pin as output
    GPIO_InitStruct.Pin   = GPIO_PIN_5;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void
Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}
