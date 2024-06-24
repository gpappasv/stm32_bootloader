/**
 * @file firmware_update.h
 * @brief This module is about handling the firmware update process
 * @version 0.1
 * @date 2024-06-23
 *
 * @copyright Copyright (c) 2024
 *
 */

// --- includes --------------------------------------------------------------------------------------------------------
#include <stdbool.h>
#include <stdint.h>

// --- structs ---------------------------------------------------------------------------------------------------------
/**
 * @brief Struct to remember the firmware update state
 */
struct firmware_update_state_s
{
    bool    is_update_started; /**< Flag to indicate if the update process has started */
    uint32_t packets_received;  /**< Number of packets received */
};

// --- function declarations -------------------------------------------------------------------------------------------
bool firmware_update_start(void);
bool firmware_update_process_packet(uint8_t *packet, uint32_t size);
bool firmware_update_cancel(void);
void firmware_update_status(struct firmware_update_state_s *state);