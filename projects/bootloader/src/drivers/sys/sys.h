/**
 * @file sys_init.h
 * @author George Pappas (pappasgeorge12@gmail.com)
 * @brief This header file is the low level system init driver layer.
 * @version 0.1
 * @date 2024-05-22
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef SYS_H
#define SYS_H

// --- includes --------------------------------------------------------------------------------------------------------
#include <stdint.h>
#include <stddef.h>

// --- function declarations -------------------------------------------------------------------------------------------
void sys_delay_ms(uint32_t delay);
void set_msp(size_t addr);

#endif // SYS_H
