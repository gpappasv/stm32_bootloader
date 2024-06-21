/**
 * @file flash_apis.h
 * @brief This source file provides the important flash apis to the bootloader code.
 * @version 0.1
 * @date 2024-05-22
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef FLASH_APIS_H
#define FLASH_APIS_H

// --- include ---------------------------------------------------------------------------------------------------------
#include "flash_driver.h"
#include <stdbool.h>

// --- function declarations -------------------------------------------------------------------------------------------
bool flash_api_transfer_secondary_to_primary(void);

#endif // FLASH_APIS_H