set(UECC_SRC_FILES
    ${GIT_ROOT_DIR}/third_party/uECC/uECC.c
)

add_library(uECC ${UECC_SRC_FILES})

target_include_directories(uECC PUBLIC
    ${GIT_ROOT_DIR}/third_party/uECC
)

target_compile_options(uECC PRIVATE -Wno-unused-parameter -mthumb
        -mcpu=cortex-m4
        -mfloat-abi=soft
        -Wall
        -Wextra
        -fdata-sections
        -ffunction-sections
        -fstack-usage
        -Os
        )

target_link_options(uECC PRIVATE
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
