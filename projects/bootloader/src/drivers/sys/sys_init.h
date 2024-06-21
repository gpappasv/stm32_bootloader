/**
 * @file sys_init.h
 * @brief This header file is the low level system init driver layer.
 * @version 0.1
 * @date 2024-05-22
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef SYS_INIT_H
#define SYS_INIT_H

// --- includes --------------------------------------------------------------------------------------------------------
#include "stm32f4xx_hal.h"

// --- function declarations -------------------------------------------------------------------------------------------
void sys_init(void);
void sys_prepare_for_application(void);

#endif // SYS_INIT_H