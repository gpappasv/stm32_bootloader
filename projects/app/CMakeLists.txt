# CMake file for Pulse tester
cmake_minimum_required(VERSION 3.15.3)

# Optional: print out extra messages to see what is going on. Comment it to have less verbose messages
set(CMAKE_VERBOSE_MAKEFILE ON)
# Build configuration adjustment - Release/Debug: Default is debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
 endif()

# Get the git root path
find_package(Git)

if(GIT_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --show-toplevel
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_ROOT_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
else()
  message(FATAL_ERROR "Git not found! Please install Git and try again.")
  return()
endif()

message(STATUS "Git root directory: ${GIT_ROOT_DIR}")

# Path to toolchain file. This one has to be before 'project()' below
set(CMAKE_TOOLCHAIN_FILE ${GIT_ROOT_DIR}/projects/arm-none-eabi-gcc.cmake)
set(LINKER_FILE ${GIT_ROOT_DIR}/projects/app/boards/stm32f401re/STM32F401RETx_FLASH.ld)
project(app)
# Setup project, output and linker file
set(EXECUTABLE ${PROJECT_NAME}.elf)

enable_language(C ASM)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Enable clang tidy for C language
#set(CMAKE_C_CLANG_TIDY "clang-tidy;-checks=*")

# issue a message to be sure it uses the correct toolchain file.
message(STATUS "CMAKE_TOOLCHAIN_FILE is: ${CMAKE_TOOLCHAIN_FILE}")

# List of source files
set(THIRD_PARTY_SRC_FILES
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
)

add_library(third_party ${THIRD_PARTY_SRC_FILES})

# List of compiler defines, prefix with -D compiler option
target_compile_definitions(third_party PUBLIC
        #-DUSE_HAL_DRIVER
        -DSTM32F401xE
        )

target_include_directories(third_party PUBLIC
    ${GIT_ROOT_DIR}/projects/app/boards/stm32f401re/Inc
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Inc
    ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    ${GIT_ROOT_DIR}/third_party/Drivers/CMSIS/Device/ST/STM32F4xx/Include
    ${GIT_ROOT_DIR}/third_party/Drivers/CMSIS/Include
)

target_compile_options(third_party PRIVATE -Wno-unused-parameter -mthumb
        -mcpu=cortex-m4
        -mfloat-abi=soft
        -Wall
        -Wextra
        -fdata-sections
        -ffunction-sections
        -fstack-usage)
        
target_link_options(third_party PRIVATE
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


set(SRC_FILES
    ${GIT_ROOT_DIR}/projects/app/src/main.c
    ${GIT_ROOT_DIR}/projects/app/boards/stm32f401re/Src/stm32f4xx_it.c
    ${GIT_ROOT_DIR}/projects/app/boards/stm32f401re/Src/stm32f4xx_hal_msp.c
    ${GIT_ROOT_DIR}/projects/app/boards/stm32f401re/Src/system_stm32f4xx.c
    ${GIT_ROOT_DIR}/projects/app/boards/stm32f401re/startup_stm32f401xe.s
)

# Build the executable based on the source files
add_executable(${EXECUTABLE} ${SRC_FILES})

target_link_libraries(${EXECUTABLE} third_party)

# List of compiler defines, prefix with -D compiler option
target_compile_definitions(${EXECUTABLE} PRIVATE
        )

# List of includ directories
target_include_directories(${EXECUTABLE} PRIVATE
        ${GIT_ROOT_DIR}/projects/app/boards/stm32f401re/Inc
        ${GIT_ROOT_DIR}/projects/app/src
        ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Inc
        ${GIT_ROOT_DIR}/third_party/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
        ${GIT_ROOT_DIR}/third_party/Drivers/CMSIS/Device/ST/STM32F4xx/Include
        ${GIT_ROOT_DIR}/third_party/Drivers/CMSIS/Include
        )

# Compiler options
target_compile_options(${EXECUTABLE} PRIVATE
        -mthumb
        -mcpu=cortex-m4
        -mfloat-abi=soft
        -Wall
        -Wextra
        -fdata-sections
        -ffunction-sections
        -fstack-usage
        ${DEBUG_COMPILER_OPTIONS}
        )

get_target_property(COMPILE_OPTIONS ${EXECUTABLE} COMPILE_OPTIONS)
message("Compile options for ${EXECUTABLE}: ${COMPILE_OPTIONS}")

# Linker options
target_link_options(${EXECUTABLE} PRIVATE
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
        -lnosys
        )

add_custom_command(TARGET ${EXECUTABLE}
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O srec --srec-len=64 ${EXECUTABLE} ${CMAKE_SOURCE_DIR}/build/${PROJECT_NAME}.s19
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE} ${CMAKE_SOURCE_DIR}/build/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE} ${CMAKE_SOURCE_DIR}/build/${PROJECT_NAME}.bin
    )