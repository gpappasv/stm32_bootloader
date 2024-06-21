/**
 * @file com_protocol.h
 * @brief Under this module, the communication protocol exists.
 * @version 0.1
 * @date 2024-06-21
 *
 * @copyright Copyright (c) 2024
 *
 */

// --- includes --------------------------------------------------------------------------------------------------------
#include "com_protocol.h"

// --- static variable definitions -------------------------------------------------------------------------------------
// Define an array of structures to map message types to their settings
static const struct com_proto_msg_type_settings_s com_proto_msg_type_settings[] = {
    // FWUG_START
    [COM_PROTO_MSG_TYPE_FWUG_START] = {
        .is_encrypted = false,
        .enc_start_byte = 0,
        .enc_end_byte = 0,
        .crc_end_byte = sizeof(struct com_proto_fwug_start_s) - 1,
        .response_msg_type = COM_PROTO_MSG_TYPE_OP_RESULT
    },
    // FWUG_DATA
    [COM_PROTO_MSG_TYPE_FWUG_DATA] = {
        .is_encrypted = true,
        .enc_start_byte = offsetof(struct com_proto_fwug_data_s, payload),
        .enc_end_byte = offsetof(struct com_proto_fwug_data_s, payload) + sizeof(((struct com_proto_fwug_data_s*)0)->payload) - 1,
        .crc_end_byte = sizeof(struct com_proto_fwug_data_s) - 1,
        .response_msg_type = COM_PROTO_MSG_TYPE_FWUG_STATUS
    },
    // FWUG_STATUS
    [COM_PROTO_MSG_TYPE_FWUG_STATUS] = {
        .is_encrypted = false,
        .enc_start_byte = 0,
        .enc_end_byte = 0,
        .crc_end_byte = sizeof(struct com_proto_fwug_status_s) - 1,
        .response_msg_type = COM_PROTO_MSG_TYPE_NONE // No response expected
    },
    // FWUG_CANCEL
    [COM_PROTO_MSG_TYPE_FWUG_CANCEL] = {
        .is_encrypted = false,
        .enc_start_byte = 0,
        .enc_end_byte = 0,
        .crc_end_byte = sizeof(struct com_proto_fwug_cancel_s) - 1,
        .response_msg_type = COM_PROTO_MSG_TYPE_OP_RESULT
    },
    // REQ_DATA
    [COM_PROTO_MSG_TYPE_REQ_DATA] = { // TODO: GPA: TBD
        .is_encrypted = false,
        .enc_start_byte = 0,
        .enc_end_byte = 0,
        .crc_end_byte = sizeof(struct com_proto_msg_header_s) - 1, // Assuming only header
        .response_msg_type = COM_PROTO_MSG_TYPE_DATA
    },
    // DATA
    [COM_PROTO_MSG_TYPE_DATA] = { // TODO: GPA: TBD
        .is_encrypted = true,
        .enc_start_byte = offsetof(struct com_proto_fwug_data_s, payload),
        .enc_end_byte = offsetof(struct com_proto_fwug_data_s, payload) + sizeof(((struct com_proto_fwug_data_s*)0)->payload) - 1,
        .crc_end_byte = sizeof(struct com_proto_fwug_data_s) - 1,
        .response_msg_type = COM_PROTO_MSG_TYPE_NONE // No response expected
    },
    // CMD
    [COM_PROTO_MSG_TYPE_CMD] = { // TODO: GPA: TBD
        .is_encrypted = true,
        .enc_start_byte = 0,
        .enc_end_byte = sizeof(struct com_proto_msg_header_s) - 1,
        .crc_end_byte = sizeof(struct com_proto_msg_header_s) - 1,
        .response_msg_type = COM_PROTO_MSG_TYPE_OP_RESULT
    },
    // OP_RESULT
    [COM_PROTO_MSG_TYPE_OP_RESULT] = { // TODO: GPA: TBD
        .is_encrypted = false,
        .enc_start_byte = 0,
        .enc_end_byte = 0,
        .crc_end_byte = sizeof(struct com_proto_msg_header_s) - 1, // Assuming only header
        .response_msg_type = COM_PROTO_MSG_TYPE_NONE // No response expected
    }
};

/**
 * @brief Get the msg type settings object
 *
 * @param msg_type
 * @return const struct com_proto_msg_type_settings_s*
 */
const struct com_proto_msg_type_settings_s *
get_msg_type_settings(enum com_protocol_msg_types_e msg_type)
{
    // Check if the msg_type is within the valid range
    if (msg_type <= COM_PROTO_MSG_TYPE_NONE
        || msg_type >= sizeof(com_proto_msg_type_settings) / sizeof(com_proto_msg_type_settings[0]))
    {
        // Return NULL if the msg_type is invalid
        return NULL;
    }

    return &com_proto_msg_type_settings[msg_type];
}