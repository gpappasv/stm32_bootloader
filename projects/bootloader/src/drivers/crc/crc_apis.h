/**
 * @file crc_apis.h
 * @author George Pappas (pappasgeorge12@gmail.com)
 * @brief This header file is the higher level crc apis layer.
 * @version 0.1
 * @date 2024-05-22
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef CRC_APIS_H
#define CRC_APIS_H

// --- includes --------------------------------------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>

// --- function declarations -------------------------------------------------------------------------------------------
bool crc_api_check_primary_app(void);
bool crc_api_check_secondary_app(void);

#endif // CRC_APIS_H
