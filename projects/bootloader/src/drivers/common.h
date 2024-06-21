/**
 * @file common.h
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

#endif // COMMON_H