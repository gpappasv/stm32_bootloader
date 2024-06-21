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
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// --- defines ---------------------------------------------------------------------------------------------------------
// TODO: GPA: this must be used also on uart rx somehow.
#define COM_PROTO_CHUNK_SIZE 256

// --- enums -----------------------------------------------------------------------------------------------------------
// clang-format off
/**
 * @brief Com protocol operation results. Will be the response to message type COM_PROTO_MSG_TYPE_OP_RESULT
 * 
 */
enum com_protocol_op_results_e {
    COM_PROTO_OP_RESULT_NO_ERR          = 0x00,
    COM_PROTO_OP_RESULT_GENERIC_ERR     = 0xE1,
    COM_PROTO_OP_RESULT_CRC_ERR         = 0xE2,
    COM_PROTO_OP_RESULT_AUTH_ERR        = 0xE3,
    COM_PROTO_OP_RESULT_UNKNOWN_MSG_ERR = 0xE4,
};

/**
 * @brief Com protocol data types. COM_PROTO_MSG_TYPE_REQ_DATA / COM_PROTO_MSG_TYPE_DATA
 * 
 */
enum com_protocol_data_types_e {
    COM_PROTO_DATA_TYPE_DEBUG_INF = 0xD0, /* Data type: debug statistics */
};

/**
 * @brief Com protocol command types (COM_PROTO_MSG_TYPE_CMD)
 * 
 */
enum com_protocol_cmd_types_e {
    COM_PROTO_CMD_CONFIRM_BACKUP_IMG  = 0xC0,
    COM_PROTO_CMD_TEST_BACKUP_IMG     = 0xC1,
    COM_PROTO_CMD_VALIDATE_BACKUP_IMG = 0xC2,
    COM_PROTO_CMD_ERASE_BACKUP_IMG    = 0xC3,
};

/**
 * @brief Com protocol msg types. NOTE: Keep incrementing this properly! This will index an array.
 * 
 */
enum com_protocol_msg_types_e {
    /* None */
    COM_PROTO_MSG_TYPE_NONE        = 0x00,
    /* Firmware update msg types */
    COM_PROTO_MSG_TYPE_FWUG_START  = 0x01, /* Message to start a firmware update */
    COM_PROTO_MSG_TYPE_FWUG_DATA   = 0x02, /* Message that contains firmware update data */
    COM_PROTO_MSG_TYPE_FWUG_STATUS = 0x03, /* Message to be forwarded to the fwug client. Will contain fwug status. */
    COM_PROTO_MSG_TYPE_FWUG_CANCEL = 0x04, /* Message to stop a firmware update */
    /* Generic data msg types */
    COM_PROTO_MSG_TYPE_REQ_DATA    = 0x05, /* Message that will be used to request data */
    COM_PROTO_MSG_TYPE_DATA        = 0x06, /* Message that can contain different data, as a response to REQ_DATA. */
    /* Command msg */
    COM_PROTO_MSG_TYPE_CMD         = 0x07, /* Message that will contain a command for the bootloader */
    /* Operation result */
    COM_PROTO_MSG_TYPE_OP_RESULT   = 0x08, /* Message that contains the operation result (error codes) */
};
// clang-format on

// --- structs ---------------------------------------------------------------------------------------------------------
/**
 * @brief Each msg type:
 *        Might, or might not, be encrypted.
 *        Provides a specific response msg type.
 *        Has a specific range that is encrypted.
 *        Has a specific range to calculate crc32.
 *
 * This structure provides that information for each msg type
 *
 */
struct com_proto_msg_type_settings_s
{
    bool    is_encrypted;
    uint8_t enc_start_byte;
    uint8_t enc_end_byte;
    uint8_t crc_end_byte;
    uint8_t response_msg_type;
};

// TODO: GPA: pack the structures!!!
/**
 * @brief Header structure for every msg type.
 *
 */
struct com_proto_msg_header_s
{
    uint8_t type;
    uint8_t len;
} __attribute__((packed));

/**
 * @brief Footer structure for every msg type. Auth can be empty. Each msg type might be encrypted, or not.
 *
 */
struct com_proto_msg_footer_s
{
    uint8_t auth;
    uint8_t crc32;
} __attribute__((packed));

/**
 * @brief Structure of COM_PROTO_MSG_TYPE_FWUG_START
 *
 */
struct com_proto_fwug_start_s
{
    struct com_proto_msg_header_s msg_header;
    struct com_proto_msg_footer_s msg_footer;
} __attribute__((packed));

/**
 * @brief Structure of COM_PROTO_MSG_TYPE_FWUG_DATA
 *
 */
struct com_proto_fwug_data_s
{
    struct com_proto_msg_header_s msg_header;
    uint8_t                       payload[128];
    struct com_proto_msg_footer_s msg_footer;
} __attribute__((packed));

/**
 * @brief Structure of COM_PROTO_MSG_TYPE_FWUG_STATUS
 *
 */
struct com_proto_fwug_status_s
{
    struct com_proto_msg_header_s msg_header;
    uint8_t                       is_active;
    uint16_t                      packets_received;
    struct com_proto_msg_footer_s msg_footer;
} __attribute__((packed));

/**
 * @brief Structure of COM_PROTO_MSG_TYPE_FWUG_CANCEL
 *
 */
struct com_proto_fwug_cancel_s
{
    struct com_proto_msg_header_s msg_header;
    struct com_proto_msg_footer_s msg_footer;
} __attribute__((packed));

// --- function declarations -------------------------------------------------------------------------------------------
const struct com_proto_msg_type_settings_s *get_msg_type_settings(enum com_protocol_msg_types_e msg_type);