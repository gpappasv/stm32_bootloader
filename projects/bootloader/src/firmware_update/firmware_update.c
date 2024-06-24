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
#include "firmware_update.h"

#include <stdbool.h>
#include <stdint.h>
#include "flash/flash_apis.h"
#include "com_protocol/com_protocol.h"

// --- static variable definitions -------------------------------------------------------------------------------------
static struct firmware_update_state_s firmware_update_state;

// --- function definitions --------------------------------------------------------------------------------------------
/**
 * @brief Function to start the firmware update process.
 *
 * @return true if erase was successful, false otherwise.
 */
bool
firmware_update_start(void)
{
    // Check if the update process has already started
    if (firmware_update_state.is_update_started)
    {
        return false;
    }

    // Initialize the firmware update state
    firmware_update_state.is_update_started = true;
    firmware_update_state.packets_received  = 0;

    // Erase the secondary space, to make room for the new firmware
    bool ret = flash_api_erase_secondary_space();
    return ret;
}

/**
 * @brief Function to process a firmware update packet.
 *
 * @param packet Pointer to the packet data
 * @param size Size of the packet data
 * @return true if the packet was processed successfully, false otherwise.
 */
bool
firmware_update_process_packet(uint8_t *packet_data, uint32_t packet_number)
{
    // Check if the update process has started
    if (!firmware_update_state.is_update_started)
    {
        return false;
    }

    /* Check if the packet number is correct: packets_received is 1-based. Packet number is 0-based.
       E.g. first packet has packet_number 0. After receiving it, packets_received is 1.
       On the next packet, packet_number should be 1 and will be checked against packets_received. */
    if (packet_number != firmware_update_state.packets_received)
    {
        return false;
    }

    // Calculate the flash address offset
    uint32_t flash_address_offset = firmware_update_state.packets_received * FIRMWARE_UPDATE_PACKET_SIZE;

    // Write the packet data to the secondary space
    bool ret = flash_api_write_firmware_update_packet(packet_data, FIRMWARE_UPDATE_PACKET_SIZE, flash_address_offset);
    if (ret)
    {
        firmware_update_state.packets_received++;
    }

    return ret;
}

/**
 * @brief This function is used to get the current status of the firmware update process. Will be used as a response to
 *        any firmware update message. (COM_PROTO_MSG_TYPE_FWUG_STATUS)
 *
 * @param state
 * @return true
 * @return false
 */
void
firmware_update_status(struct firmware_update_state_s *state)
{
    if (state == NULL)
    {
        return;
    }

    // Just copy the firmware update state
    state->is_update_started = firmware_update_state.is_update_started;
    state->packets_received  = firmware_update_state.packets_received;

    return;
}

/**
 * @brief Function to cancel the firmware update process.
 *
 * @return true if the update process was successful, false otherwise.
 */
bool
firmware_update_cancel(void)
{
    // Reset the firmware update state
    firmware_update_state.is_update_started = false;
    firmware_update_state.packets_received  = 0;

    return true;
}