/**
 * @file main.c
 * @brief Main functionality of the bootloader.
 * @version 0.1
 * @date 2024-06-09
 *
 * @copyright Copyright (c) 2024
 *
 */

// --- includes --------------------------------------------------------------------------------------------------------
#include "main.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sys_init.h"
#include "sys.h"
#include "uart_driver.h"
#include "crc_apis.h"
#include "crc_driver.h"
#include "flash_apis.h"
#include "common.h"
#include "com_protocol.h"
#include "user_input.h"

// --- typedefs --------------------------------------------------------------------------------------------------------
/**
 * @brief Enumeration of the different state machine states.
 *
 */
typedef enum
{
    BL_FSM_NONE_STATE = 0,
    BL_FSM_INIT_STATE,
    BL_FSM_CRC_CHECK_STATE,
    BL_FSM_AUTH_STATE,
    BL_FSM_BOOT_APP_STATE,
    BL_FSM_BOOTLOOP_STATE,
    BL_FSM_STATE_END,
} bl_fsm_states_e;
#define BL_FSM_STATE_COUNT (BL_FSM_STATE_END)

/**
 * @brief Enumeration of the possible results of each state machine states. This is the return value of each handler,
 * that will determine the next transition.
 *
 */
typedef enum
{
    BL_FSM_ERR_OR_NONE_EVT = 0,
    BL_FSM_CHECK_PASS_EVT,
    BL_FSM_CHECK_FAIL_EVT,
    BL_FSM_BUTTON_PRESSED_EVT,
    BL_FSM_EVT_END,
} bl_fsm_evts_e;
#define BL_FSM_EVT_COUNT (BL_FSM_EVT_END)

typedef struct bl_fsm_ctx_t
{
    // state machine info
    bl_fsm_states_e curr_state : 3;
    bl_fsm_evts_e   evt_produced : 3;

    // Status flags
    uint8_t newer_ver_on_backup : 1;
    uint8_t recover_main_img : 1;
} bl_fsm_ctx_s;

/**
 * @brief This is the fsm handler fp. It gets as an input the state machine context. It outputs the result of the state
 * execution. Based on the output, the next state machine handler is selected.
 *
 */
typedef bl_fsm_evts_e (*bl_fsm_handler)(bl_fsm_ctx_s * const ctx);
typedef void (*bl_func_ptr)(void);

// --- static function declarations ------------------------------------------------------------------------------------
static bl_fsm_evts_e fsm_init_hdl(bl_fsm_ctx_s * const ctx);
static bl_fsm_evts_e fsm_crc_check_hdl(bl_fsm_ctx_s * const ctx);
static bl_fsm_evts_e fsm_auth_hdl(bl_fsm_ctx_s * const ctx);
static bl_fsm_evts_e fsm_boot_app_hdl(bl_fsm_ctx_s * const ctx);
static bl_fsm_evts_e fsm_bootloop_hdl(bl_fsm_ctx_s * const ctx);

// State machine map
// clang-format off
/**
 * @brief Based on the 'Last executed state' (current state - bl_fsm_states_e) and the output of the relevant handler (bl_fsm_evts_e)
 * the next handler to be executed will be triggered.
 * E.g. At first, the (current_state == BL_FSM_NONE_STATE and the hdl_output(evt) == BL_FSM_NONE_STATE) -> fsm_init_hdl will be executed.
 *      The fsm_init_hdl, will set the current_state == BL_FSM_INIT_STATE and if (for example) hdl_output(evt) ==  BL_FSM_ERR_OR_NONE_EVT,
 *      -> fsm_crc_check_hdl will be called on the next state machine cycle.
 *
 * The purpose of this fsm is to follow a path, from the init (boot) phase to either booting the application, or entering the recovery
 * mode.
 *
 */
static const bl_fsm_handler bl_fsm_map[BL_FSM_STATE_COUNT][BL_FSM_EVT_COUNT] = {
                               /* | BL_FSM_ERR_OR_NONE_EVT | BL_FSM_CHECK_PASS_EVT | BL_FSM_CHECK_FAIL_EVT | BL_FSM_BUTTON_PRESSED_EVT */
/* BL_FSM_NONE_STATE           */ { fsm_init_hdl,            NULL,                   NULL,                   NULL },
/* BL_FSM_INIT_STATE           */ { fsm_crc_check_hdl,       NULL,                   NULL,                   fsm_bootloop_hdl },
/* BL_FSM_CRC_CHECK_STATE      */ { fsm_bootloop_hdl,        fsm_auth_hdl,           fsm_crc_check_hdl,      NULL },
/* BL_FSM_AUTH_STATE           */ { fsm_bootloop_hdl,        fsm_boot_app_hdl,       fsm_crc_check_hdl,      NULL },
/* BL_FSM_BOOT_APP_STATE       */ { fsm_bootloop_hdl,        fsm_bootloop_hdl,       fsm_bootloop_hdl,       fsm_bootloop_hdl },
/* BL_FSM_BOOTLOOP_STATE       */ { fsm_bootloop_hdl,        NULL,                   NULL,                   NULL },\
};
// clang-format on

static void boot_application(void);

// --- static function definitions -------------------------------------------------------------------------------------
/**
 * @brief Function that boots the application
 *
 */
static void
boot_application(void)
{
    uint32_t    jump_address;
    bl_func_ptr jump_to_application;

    // check if there is something "installed" in the app FLASH region
    // TODO: GPA: need to find a specific pattern to identify our application
    if (((*(uint32_t *)((uint32_t)&__flash_app_start__)) & 0x2FFE0000) == 0x20000000)
    {
        // TODO: GPA: we can check here if the stack pointer is within valid RAM region
        // TODO: GPA: we can check here if the reset handler is a valid address, within the FLASH region
#ifdef DEBUG_LOG
        printf("APP Start ...\r\n");
#endif
        // jump to the application
        jump_address        = *(uint32_t *)(((uint32_t)&__flash_app_start__) + 4);
        jump_to_application = (bl_func_ptr)jump_address;
        // initialize application's stack pointer
        sys_set_msp(((uint32_t)&__flash_app_start__));
        // prepare for the application
        sys_prepare_for_application();
        jump_to_application();
    }
    else
    {
        // there is no application installed
#ifdef DEBUG_LOG
        printf("No APP found\r\n");
#endif
    }
}

/**
 * @brief State handler for initialization. Takes care of system initialization and performs the following checks:
 *        1. If there is an image of newer version in secondary image slot.
 *        2. If the serial recovery button is pressed (should transition to bootloop state with serial communication on)
 *
 */
static bl_fsm_evts_e
fsm_init_hdl(bl_fsm_ctx_s * const ctx)
{
    if (ctx == NULL)
    {
        return -1;
    }

    // Initialize the context
    memset(ctx, 0, sizeof(bl_fsm_ctx_s));

    // Store the current state
    ctx->curr_state = BL_FSM_INIT_STATE;

    // Initialize the system
    sys_init();
    // Init the uart peripheral
    uart_driver_init();
    // Initialize the com protocol
    com_protocol_init();
#ifdef DEBUG_LOG
        printf(" --- BOOTLOADER Start --- \r\n");
#endif

    if (user_input_is_pressed())
    {
        return BL_FSM_BUTTON_PRESSED_EVT;
    }

    if (flash_api_is_secondary_newer())
    {
        ctx->newer_ver_on_backup = true;
    }

    return BL_FSM_ERR_OR_NONE_EVT;
}

/**
 * @brief State handler for CRC check. This state is responsible for performing CRC checks on either the primary or the
 *        secondary image slot.
 *        NOTES: 1. If newer version found flag is on: check the CRC validity of the secondary image.
 *               2. If primary img recovery flag is on: Check the CRC validity of the secondary image.
 *               3. On other cases, check the CRC validity of the main image.
 *
 */
static bl_fsm_evts_e
fsm_crc_check_hdl(bl_fsm_ctx_s * const ctx)
{
    if (ctx == NULL)
    {
        return -1;
    }
    // Set the current state
    ctx->curr_state = BL_FSM_CRC_CHECK_STATE;

    bool is_crc_ok = false;
    // First handle the case where an image of newer version is found in the backup/seconday region.
    if (ctx->newer_ver_on_backup)
    {
#ifdef DEBUG_LOG
        printf("Checking CRC of backup slot (newer version found)\r\n");
#endif
        is_crc_ok = crc_api_check_secondary_app();
        if (is_crc_ok)
        {
            // If newer version is found and CRC is ok, mark the check as passed.
            return BL_FSM_CHECK_PASS_EVT;
        }

        // If newer version is found but CRC is not ok, mark the check as failed.
        ctx->newer_ver_on_backup = false;
        return BL_FSM_CHECK_FAIL_EVT;
    }

    // If no newer version is found in the backup region, proceed with normal image CRC checks.
    // First check if we need to recover the primary image.
    if (!ctx->recover_main_img)
    {
#ifdef DEBUG_LOG
        printf("Checking CRC of main application \r\n");
#endif
        is_crc_ok = crc_api_check_primary_app();
        if (is_crc_ok)
        {
            // If CRC is ok, mark the check as passed.
            return BL_FSM_CHECK_PASS_EVT;
        }

        // If CRC is not ok, mark the check as failed and we should recover the primary image, so raise the flag.
        ctx->recover_main_img = true;
        return BL_FSM_CHECK_FAIL_EVT;
    }

    // Then check if we need to recover the main image, from the secondary image.
#ifdef DEBUG_LOG
    printf("Checking CRC of secondary image\r\n");
#endif
    is_crc_ok = crc_api_check_secondary_app();
    if (is_crc_ok)
    {
        // If CRC is ok, mark the check as passed.
        return BL_FSM_CHECK_PASS_EVT;
    }

    // If again CRC is not ok, bootloader is stuck in a bootloop.
    return BL_FSM_ERR_OR_NONE_EVT;
}

/**
 * @brief State handler for authentication. The authentication stage is the last stage before booting the application.
 *        NOTES: 1. If the newer version found flag is on: Check the auth of the secondary image.
 *               2. If the main img recovery flag is on: Check the auth of the secondary image.
 *               3. On other cases, check the auth of the primary image.
 *
 */
static bl_fsm_evts_e
fsm_auth_hdl(bl_fsm_ctx_s * const ctx)
{
    if (ctx == NULL)
    {
        return -1;
    }
    ctx->curr_state = BL_FSM_AUTH_STATE;

    bool is_auth_ok = false;
    // First handle the case where an image of newer version is found in the backup/seconday region.
    if (ctx->newer_ver_on_backup)
    {
#ifdef DEBUG_LOG
        printf("Checking AUTH for secondary slot (newer version found)");
#endif
        // Either the auth step will succeed or fail for the newer version in the backup region.
        // Either way, the bootloader will not re-try to boot the newer version, if the first try fails.
        ctx->newer_ver_on_backup = false;
        // If newer version is found and auth is ok, mark the check as passed.
        if (/*authentication is ok*/ 1)
        {
            // Newer version on backup + CRC ok + Auth ok = transfer the secondary to primary
            bool is_transfer_ok = flash_api_transfer_secondary_to_primary();
            if (is_transfer_ok)
            {
                // If transfer is successful, mark the check as passed.
                return BL_FSM_CHECK_PASS_EVT;
            }

            // If transfer failed, mark the check as failed.
            return BL_FSM_CHECK_FAIL_EVT;
        }

        // If authentication failed, mark the check as failed.
        return BL_FSM_CHECK_FAIL_EVT;
    }

    // If no newer version is found in the backup region, proceed with normal image authentication.
    // First check if we need to recover the primary image.
    if (ctx->recover_main_img)
    {
#ifdef DEBUG_LOG
        printf("Checking AUTH for secondary image slot\r\n");
#endif
        ctx->recover_main_img = false;
        // Check the auth of the secondary image.
        if (/*authentication check of secondary is ok*/ 1)
        {
            // If auth is ok, mark the check, first transfer the secondary to primary.
            bool is_transfer_ok = flash_api_transfer_secondary_to_primary();
            if (is_transfer_ok)
            {
                // If transfer is successful, mark the check as passed.
                return BL_FSM_CHECK_PASS_EVT;
            }

            // If transfer failed, recovery failed, so raise an error.
            return BL_FSM_ERR_OR_NONE_EVT;
        }

        // If authentication failed, mark the check as failed.
        return BL_FSM_ERR_OR_NONE_EVT;
    }
#ifdef DEBUG_LOG
    printf("Checking AUTH for primary image slot\r\n");
#endif
    // Then check if primary image is ok.
    if (/*authentication check of primary is ok*/ 1)
    {
        // If auth is ok, mark the check as passed.
        return BL_FSM_CHECK_PASS_EVT;
    }

    // If authentication failed, mark the check as failed.
    ctx->recover_main_img = true;
    return BL_FSM_CHECK_FAIL_EVT;
}

/**
 * @brief State handler for booting the application. This handler is responsible for preparing the system to jump to the
 *        main application and eventually jump. It always boots the primary slot. During the boot process the secondary
 *        image has already been transfered to main partition, if needed (a. newer version, b. main img is damaged).
 *        This state will only be skipped when there is no valid application found on either main or secondary slot to
 *        boot.
 *
 */
static bl_fsm_evts_e
fsm_boot_app_hdl(bl_fsm_ctx_s * const ctx)
{
    if (ctx == NULL)
    {
        return -1;
    }
    ctx->curr_state = BL_FSM_BOOT_APP_STATE;
#ifdef DEBUG_LOG
    printf("Booting application...\r\n");
#endif
    boot_application();
    return BL_FSM_ERR_OR_NONE_EVT; // Boot process finished, loop back if needed.
}

/**
 * @brief State handler for boot loop. This state is either entered when there is no valid application found to boot, or
 *        when the serial recovery button is pressed during boot.
 *
 */
static bl_fsm_evts_e
fsm_bootloop_hdl(bl_fsm_ctx_s * const ctx)
{
    if (ctx == NULL)
    {
        return -1;
    }
    ctx->curr_state = BL_FSM_BOOTLOOP_STATE;
#ifdef DEBUG_LOG
    printf("Bootloader loop...\r\n");
#endif
    // TODO: GPA: we need to enable the uart communication here for serial dfu support
    sys_delay_ms(1000);
    return BL_FSM_ERR_OR_NONE_EVT; // Stay in bootloop
}

// --- function definitions --------------------------------------------------------------------------------------------
int
main(void)
{
    bl_fsm_ctx_s  fsm_ctx = { 0 };
    bl_fsm_evts_e evt     = BL_FSM_ERR_OR_NONE_EVT;

    while (1)
    {
        // Get the current state
        bl_fsm_states_e curr_state = fsm_ctx.curr_state;
        // Check if current state, event or the selected state handler is valid
        if (curr_state >= BL_FSM_STATE_COUNT || evt >= BL_FSM_EVT_COUNT || bl_fsm_map[curr_state][evt] == NULL)
        {
            // Invalid state
#ifdef DEBUG_LOG
            printf("Current state: %d - event: %d\r\n", curr_state, evt);
#endif
            break;
        }

        // Execute the state handler
        evt = bl_fsm_map[curr_state][evt](&fsm_ctx);
    }
#ifdef DEBUG_LOG
    printf("Bootloader: fatal error... Terminating.");
#endif
    return 0;
}