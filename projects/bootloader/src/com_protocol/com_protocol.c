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

#include <stdio.h>
#include "uart/uart_driver.h"
#include "crc/crc_driver.h"
#include "firmware_update/firmware_update.h"

// --- defines ---------------------------------------------------------------------------------------------------------
#define MSG_TYPE_POS 0
#define MSG_LEN_POS  1

#define IS_COM_PROTO_MSG_TYPE_FWUG_START_ENC false
#define IS_COM_PROTO_MSG_TYPE_FWUG_DATA_ENC true
#define IS_COM_PROTO_MSG_TYPE_FWUG_STATUS_ENC false
#define IS_COM_PROTO_MSG_TYPE_FWUG_CANCEL_ENC false
#define IS_COM_PROTO_MSG_TYPE_REQ_DATA_ENC false
#define IS_COM_PROTO_MSG_TYPE_DATA_ENC true
#define IS_COM_PROTO_MSG_TYPE_CMD_ENC true
#define IS_COM_PROTO_MSG_TYPE_OP_RESULT_ENC false


// --- typedefs --------------------------------------------------------------------------------------------------------
typedef void (*com_proto_msg_handler_t)(void *data);
typedef void (*com_proto_msg_response_handler_t)(void);

// --- static function declarations ------------------------------------------------------------------------------------
static void com_process_rx_data(struct uart_driver_data_s * const rx_data);
static bool is_msg_type_valid(enum com_protocol_msg_types_e msg_type);
static bool is_msg_len_valid(uint8_t msg_len);
static bool is_crc_valid(uint8_t const * const rx_buffer, uint8_t msg_len);

static void fwug_start_handler(void *data);
static void fwug_data_handler(void *data);
static void fwug_cancel_handler(void *data);
static void unsupported_msg_handler(void *data);

static void fwug_status_response_handler(void);
static void data_response_handler(void);
static void op_result_response_handler(void);

static void com_proto_message_handle_encryption(uint8_t *rx_buffer, enum com_protocol_msg_types_e msg_type);

// --- static variable definitions -------------------------------------------------------------------------------------
// Define an array of structures to map message types to their settings
static const struct com_proto_msg_type_settings_s com_proto_msg_type_settings[] = {
    // FWUG_START
    [COM_PROTO_MSG_TYPE_FWUG_START] = {
        .is_encrypted = IS_COM_PROTO_MSG_TYPE_FWUG_START_ENC,
        .enc_start_byte = 0,
        .enc_end_byte = 0,
        .response_msg_type = COM_PROTO_MSG_TYPE_FWUG_STATUS
    },
    // FWUG_DATA
    [COM_PROTO_MSG_TYPE_FWUG_DATA] = {
        .is_encrypted = IS_COM_PROTO_MSG_TYPE_FWUG_DATA_ENC,
        .enc_start_byte = offsetof(struct com_proto_fwug_data_s, payload),
        .enc_end_byte = offsetof(struct com_proto_fwug_data_s, payload) + sizeof(((struct com_proto_fwug_data_s*)0)->payload) - 1,
        .response_msg_type = COM_PROTO_MSG_TYPE_FWUG_STATUS
    },
    // FWUG_STATUS
    [COM_PROTO_MSG_TYPE_FWUG_STATUS] = {
        .is_encrypted = IS_COM_PROTO_MSG_TYPE_FWUG_STATUS_ENC,
        .enc_start_byte = 0,
        .enc_end_byte = 0,
        .response_msg_type = COM_PROTO_MSG_TYPE_NONE // No response expected
    },
    // FWUG_CANCEL
    [COM_PROTO_MSG_TYPE_FWUG_CANCEL] = {
        .is_encrypted = IS_COM_PROTO_MSG_TYPE_FWUG_CANCEL_ENC,
        .enc_start_byte = 0,
        .enc_end_byte = 0,
        .response_msg_type = COM_PROTO_MSG_TYPE_FWUG_STATUS
    },
    // REQ_DATA
    [COM_PROTO_MSG_TYPE_REQ_DATA] = { // TODO: GPA: TBD
        .is_encrypted = IS_COM_PROTO_MSG_TYPE_REQ_DATA_ENC,
        .enc_start_byte = 0,
        .enc_end_byte = 0,
        .response_msg_type = COM_PROTO_MSG_TYPE_DATA
    },
    // DATA
    [COM_PROTO_MSG_TYPE_DATA] = { // TODO: GPA: TBD
        .is_encrypted = IS_COM_PROTO_MSG_TYPE_DATA_ENC,
        .enc_start_byte = offsetof(struct com_proto_fwug_data_s, payload),
        .enc_end_byte = offsetof(struct com_proto_fwug_data_s, payload) + sizeof(((struct com_proto_fwug_data_s*)0)->payload) - 1,
        .response_msg_type = COM_PROTO_MSG_TYPE_NONE // No response expected
    },
    // CMD
    [COM_PROTO_MSG_TYPE_CMD] = { // TODO: GPA: TBD
        .is_encrypted = IS_COM_PROTO_MSG_TYPE_CMD_ENC,
        .enc_start_byte = 0,
        .enc_end_byte = sizeof(struct com_proto_msg_header_s) - 1,
        .response_msg_type = COM_PROTO_MSG_TYPE_OP_RESULT
    },
    // OP_RESULT
    [COM_PROTO_MSG_TYPE_OP_RESULT] = { // TODO: GPA: TBD
        .is_encrypted = IS_COM_PROTO_MSG_TYPE_OP_RESULT_ENC,
        .enc_start_byte = 0,
        .enc_end_byte = 0,
        .response_msg_type = COM_PROTO_MSG_TYPE_NONE // No response expected
    }
};

// clang-format off
/**
 * @brief Map of the message type to the handler function
 *
 * @param
 */
static const com_proto_msg_handler_t com_proto_msg_handler_map[] = {
/* 0 */[COM_PROTO_MSG_TYPE_NONE]        = NULL,                    /* No handler for this message type */
/* 1 */[COM_PROTO_MSG_TYPE_FWUG_START]  = fwug_start_handler,
/* 2 */[COM_PROTO_MSG_TYPE_FWUG_DATA]   = fwug_data_handler,
/* 3 */[COM_PROTO_MSG_TYPE_FWUG_STATUS] = NULL,                    /* No handler for this message type */
/* 4 */[COM_PROTO_MSG_TYPE_FWUG_CANCEL] = fwug_cancel_handler,
/* 5 */[COM_PROTO_MSG_TYPE_REQ_DATA]    = unsupported_msg_handler,
/* 6 */[COM_PROTO_MSG_TYPE_DATA]        = NULL,                    /* No handler for this message type */
/* 7 */[COM_PROTO_MSG_TYPE_CMD]         = unsupported_msg_handler,
/* 8 */[COM_PROTO_MSG_TYPE_OP_RESULT]   = NULL,                    /* No handler for this message type */
};

/**
 * @brief Map of the response message type and the handler function
 *
 */
static const com_proto_msg_response_handler_t com_proto_response_msg_handler_map[] = {
/* 0 */[COM_PROTO_MSG_TYPE_NONE]        = NULL,                    /* No handler for this message type */
/* 1 */[COM_PROTO_MSG_TYPE_FWUG_START]  = NULL,                    /* No handler for this message type */
/* 2 */[COM_PROTO_MSG_TYPE_FWUG_DATA]   = NULL,                    /* No handler for this message type */
/* 3 */[COM_PROTO_MSG_TYPE_FWUG_STATUS] = fwug_status_response_handler,
/* 4 */[COM_PROTO_MSG_TYPE_FWUG_CANCEL] = NULL,                    /* No handler for this message type */
/* 5 */[COM_PROTO_MSG_TYPE_REQ_DATA]    = NULL,                    /* No handler for this message type */
/* 6 */[COM_PROTO_MSG_TYPE_DATA]        = data_response_handler,
/* 7 */[COM_PROTO_MSG_TYPE_CMD]         = NULL,                    /* No handler for this message type */
/* 8 */[COM_PROTO_MSG_TYPE_OP_RESULT]   = op_result_response_handler,
};
// clang-format on

static uint8_t                        op_result_status = 0;
static struct firmware_update_state_s firmware_update_state;

// --- static function definitions -------------------------------------------------------------------------------------
/**
 * @brief Function to check if the message type is valid
 *
 * @param msg_type
 * @return true
 * @return false
 */
static bool
is_msg_type_valid(enum com_protocol_msg_types_e msg_type)
{
    return (msg_type > COM_PROTO_MSG_TYPE_NONE && msg_type < COM_PROTO_MSG_TYPE_END);
}

/**
 * @brief Function to check if the message length is valid
 *
 * @param msg_len
 * @return true
 * @return false
 */
static bool
is_msg_len_valid(uint8_t msg_len)
{
    return (msg_len >= sizeof(struct com_proto_msg_header_s) + sizeof(struct com_proto_msg_footer_s));
}

/**
 * @brief Function to check if the CRC is valid
 *
 * @param rx_buffer
 * @param msg_len
 * @return true
 * @return false
 */
static bool
is_crc_valid(uint8_t const * const rx_buffer, uint8_t msg_len)
{
    uint16_t crc_16_calc = 0;
    uint16_t crc_16_msg  = 0;
    // Each message has a header (2 bytes: msg type and msg len), payload and CRC (2 bytes)
    crc_16_calc = crc16_driver_calculate(rx_buffer, msg_len - 2);
    crc_16_msg  = (rx_buffer[msg_len - 2] << 8) | rx_buffer[msg_len - 1];
    return (crc_16_calc == crc_16_msg);
}

/**
 * @brief Function to process a received message
 *
 * @param rx_data
 */
static void
com_process_rx_data(struct uart_driver_data_s * const rx_data)
{
    // Input check
    if ((rx_data == NULL) || (rx_data->len < sizeof(struct com_proto_msg_header_s)))
    {
#ifdef DEBUG_LOG
        printf("Received empty buffer\r\n");
#endif
        return;
    }
    uint8_t *rx_buffer = rx_data->data_buffer;
    uint8_t  msg_type  = rx_buffer[MSG_TYPE_POS];
    uint8_t  msg_len   = rx_buffer[MSG_LEN_POS]; /* Message len includes: header + payload + crc16 size */

    // Check if the message type and len are valid
    if (!is_msg_type_valid(msg_type) || !is_msg_len_valid(msg_len))
    {
#ifdef DEBUG_LOG
        printf("Invalid msg type or msg len\r\n");
#endif
        return;
    }

    // Get the settings based on the message type
    struct com_proto_msg_type_settings_s const *settings;
    settings = get_msg_type_settings(msg_type);
    if (settings == NULL)
    {
#ifdef DEBUG_LOG
        printf("Unsupported msg type\r\n");
#endif
        op_result_response_handler();
        return;
    }

    // 1. --- Decrypt buffer if needed ---
    if (settings->is_encrypted)
    {
        // TODO: GPA: Decrypt msg, inside the rx_data.rx_buffer
    }

    // 2. --- CRC validation ---
    if (!is_crc_valid(rx_buffer, msg_len))
    {
#ifdef DEBUG_LOG
        printf("CRC validation failed\r\n");
#endif
        op_result_response_handler();
        return;
    }

    // 3. --- Handle received message ---
    com_proto_msg_handler_t handler = com_proto_msg_handler_map[msg_type];
    if (handler != NULL)
    {
        handler(rx_buffer);
    }
    else
    {
#ifdef DEBUG_LOG
        printf("No handler for this message type\r\n");
#endif
        op_result_response_handler();
        return;
    }

    // 4. Provide a response
    // TODO: GPA: Based on the settings, we know what is the response message. Utilize the response map
    com_proto_msg_response_handler_t response_handler = com_proto_response_msg_handler_map[settings->response_msg_type];
    if (response_handler != NULL)
    {
        response_handler();
    }
    else
    {
#ifdef DEBUG_LOG
        printf("No response handler for this message type\r\n");
#endif
    }

    return;
}

// --- MESSAGE HANDLERS ---
/**
 * @brief Handler for the FWUG_START message
 *
 * @param data
 */
static void
fwug_start_handler(void *data)
{
    // Check if the received msg data len is correct
    if (((uint8_t *)data)[MSG_LEN_POS] != sizeof(struct com_proto_fwug_start_s))
    {
        op_result_status = COM_PROTO_OP_RESULT_GENERIC_ERR;
        return;
    }

    bool ret         = firmware_update_start();
    op_result_status = ret ? COM_PROTO_OP_RESULT_NO_ERR : COM_PROTO_OP_RESULT_GENERIC_ERR;

    // Get the status of the firmware update process
    firmware_update_status(&firmware_update_state);
}

/**
 * @brief Handler for the FWUG_DATA message
 *
 * @param data
 */
static void
fwug_data_handler(void *data)
{
    // Check if the received msg data len is correct. This also checks, implicitly, if the fwug packet size is correct
    if (((uint8_t *)data)[MSG_LEN_POS] != sizeof(struct com_proto_fwug_data_s))
    {
        op_result_status = COM_PROTO_OP_RESULT_GENERIC_ERR;
        return;
    }

    struct com_proto_fwug_data_s *fwug_data = (struct com_proto_fwug_data_s *)data;
    bool                          ret = firmware_update_process_packet(fwug_data->payload, fwug_data->packet_number);

    op_result_status = ret ? COM_PROTO_OP_RESULT_NO_ERR : COM_PROTO_OP_RESULT_GENERIC_ERR;

    // Get the status of the firmware update process
    firmware_update_status(&firmware_update_state);
}

/**
 * @brief Handler for the FWUG_CANCEL message
 *
 * @param data
 */
static void
fwug_cancel_handler(void *data)
{
    // Check if the received msg data len is correct
    if (((uint8_t *)data)[MSG_LEN_POS] != sizeof(struct com_proto_fwug_cancel_s))
    {
        op_result_status = COM_PROTO_OP_RESULT_GENERIC_ERR;
        return;
    }

    bool ret         = firmware_update_cancel();
    op_result_status = ret ? COM_PROTO_OP_RESULT_NO_ERR : COM_PROTO_OP_RESULT_GENERIC_ERR;

    // Get the status of the firmware update process
    firmware_update_status(&firmware_update_state);
}

/**
 * @brief Handler for the unsupported message type
 *
 * @param data
 */
static void
unsupported_msg_handler(void *data)
{
    (void)data;
    // Empty function
}

// --- RESPONSE HANDLERS ---
/**
 * @brief Handler for the FWUG_STATUS response message
 *
 * @param data
 */
static void
fwug_status_response_handler(void)
{
    // Prepare the response message
    struct com_proto_fwug_status_s response_msg = {
        .msg_header.type  = COM_PROTO_MSG_TYPE_FWUG_STATUS,
        .msg_header.len   = sizeof(struct com_proto_fwug_status_s),
        .op_result        = op_result_status,
        .is_active        = firmware_update_state.is_update_started,
        .packets_received = firmware_update_state.packets_received,
        .msg_footer.crc16 = 0,
    };

#if IS_COM_PROTO_MSG_TYPE_FWUG_STATUS_ENC
    // Handle possible encryption of the response message
    com_proto_message_handle_encryption((uint8_t *)&response_msg, COM_PROTO_MSG_TYPE_FWUG_STATUS);
#endif

    // Calculate the CRC16
    response_msg.msg_footer.crc16
        = crc16_driver_calculate((uint8_t *)&response_msg, sizeof(struct com_proto_fwug_status_s) - 1);

    // Send the response message
    uart_tx_data((uint8_t *)&response_msg, response_msg.msg_header.len);
}

/**
 * @brief Handler for the DATA response message
 *
 * @param data
 */
static void
data_response_handler(void)
{
    // Empty function
}

/**
 * @brief Handler for the OP_RESULT response message
 *
 * @param data
 */
static void
op_result_response_handler(void)
{
    struct com_proto_op_result_s response_msg = {
        .msg_header.type  = COM_PROTO_MSG_TYPE_OP_RESULT,
        .msg_header.len   = sizeof(struct com_proto_op_result_s),
        .op_result        = op_result_status,
        .msg_footer.crc16 = 0,
    };

#if IS_COM_PROTO_MSG_TYPE_OP_RESULT_ENC
    // Handle possible encryption of the response message
    com_proto_message_handle_encryption((uint8_t *)&response_msg, COM_PROTO_MSG_TYPE_OP_RESULT);
#endif
    response_msg.msg_footer.crc16
        = crc16_driver_calculate((uint8_t *)&response_msg, sizeof(struct com_proto_op_result_s) - 1);
    uart_tx_data((uint8_t *)&response_msg, response_msg.msg_header.len);
}

/**
 * @brief Function to handle the encryption of the message. This will only be used for response messages.
 *        We might need to encrypt the response message before sending it back to the host.
 *
 * @param rx_buffer
 * @param msg_type
 */
static void
com_proto_message_handle_encryption(uint8_t *rx_buffer, enum com_protocol_msg_types_e msg_type)
{
    struct com_proto_msg_type_settings_s const *settings;
    settings = get_msg_type_settings(msg_type);
    if (settings == NULL)
    {
        return;
    }

    if (settings->is_encrypted)
    {
        (void)rx_buffer; // Avoid unused variable warning
        /* TODO: GPA: Encrypt the message. Modify the rx_buffer directly. We should be able to add the cryptographic
        information directly inside the rx_buffer, based on the settings. We will check the settings to know where to
        start, and where to end the encryption and where to add the cryptographic information. */
    }

    return;
}

// --- function definitions --------------------------------------------------------------------------------------------
/**
 * @brief Initializes the communication protocol by registering the UART RX callback.
 */
void
com_protocol_init(void)
{
    // Register the RX callback function with the UART driver
    uart_driver_register_rx_callback(com_process_rx_data);
}

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
    if (msg_type <= COM_PROTO_MSG_TYPE_NONE || msg_type >= COM_PROTO_MSG_TYPE_END)
    {
        // Return NULL if the msg_type is invalid
        return NULL;
    }

    return &com_proto_msg_type_settings[msg_type];
}