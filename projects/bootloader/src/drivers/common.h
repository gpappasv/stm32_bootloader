/**
 * @file common.h
 * @author George Pappas (pappasgeorge12@gmail.com)
 * @brief This header file is a common header file for all drivers.
 * @version 0.1
 * @date 2024-05-30
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef COMMON_H
#define COMMON_H

// --- includes --------------------------------------------------------------------------------------------------------
#include <stdint.h>

// --- external variables ----------------------------------------------------------------------------------------------
extern uint32_t __flash_app_start__;
extern uint32_t __flash_app_end__;
extern uint32_t __flash_app_secondary_start__;
extern uint32_t __flash_app_secondary_end__;

#endif // COMMON_H