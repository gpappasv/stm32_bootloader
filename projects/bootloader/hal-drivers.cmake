# List of HAL source files
set(HAL_DRIVERS_SRC_FILES
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_crc.c
)

add_library(hal_drivers ${HAL_DRIVERS_SRC_FILES})

# List of compiler defines, prefix with -D compiler option
target_compile_definitions(hal_drivers PUBLIC
        #-DUSE_HAL_DRIVER
        -DSTM32F401xE
        )

target_include_directories(hal_drivers PUBLIC
    ${GIT_ROOT_DIR}/projects/bootloader/boards/stm32f401re/Inc
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Inc
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    ${GIT_ROOT_DIR}/third_party/Drivers/CMSIS/Device/ST/STM32F4xx/Include
    ${GIT_ROOT_DIR}/third_party/Drivers/CMSIS/Include
)

target_compile_options(hal_drivers PRIVATE -Wno-unused-parameter -mthumb
        -mcpu=cortex-m4
        -mfloat-abi=soft
        -Wall
        -Wextra
        -fdata-sections
        -ffunction-sections
        -fstack-usage
        -Os
        )
        
target_link_options(hal_drivers PRIVATE
        -T${LINKER_FILE}
        -mthumb
        -mcpu=cortex-m4
        -mfloat-abi=soft
        -specs=nosys.specs
        -specs=nano.specs
        -Wl,-Map=${PROJECT_NAME}.map,--cref
        -Wl,--gc-sections
        -Wl,--print-memory-usage
        -lc
        -lnosys)