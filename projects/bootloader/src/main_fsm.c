/**
 * @file main_fsm.c
 * @brief Main functionality of the code
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
#include "drivers/common.h"

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
 * @brief Enumeration of the possible results of each state machine states.
 *
 */
typedef enum
{
    BL_FSM_ERR_OR_NONE_EVT = 0,
    BL_FSM_BUTTON_PRESSED_EVT,
    BL_FSM_NEWER_VER_EVT,
    BL_FSM_CHECK_PASS_EVT,
    BL_FSM_CHECK_FAIL_EVT,
    BL_FSM_EVT_END,
} bl_fsm_evts_e;
#define BL_FSM_EVT_COUNT (BL_FSM_EVT_END)

typedef struct bl_fsm_ctx_t
{
    // state machine info
    bl_fsm_states_e curr_state : 3;
    bl_fsm_evts_e   evt_produced : 3;

    // Status flags
    uint8_t is_button_pressed : 1;
    uint8_t newer_ver_on_backup : 1;
    uint8_t recover_main_img : 1;

    // Unused bits
    uint8_t unused : 7;
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
static const bl_fsm_handler bl_fsm_map[BL_FSM_STATE_COUNT][BL_FSM_EVT_COUNT] = {
                              /*  | BL_FSM_ERR_OR_NONE_EVT | BL_FSM_CHECK_PASS_EVT | BL_FSM_CHECK_FAIL_EVT | BL_FSM_BUTTON_PRESSED_EVT | BL_FSM_NEWER_VER_EVT */
/* BL_FSM_NONE_STATE           */ { fsm_init_hdl,            NULL,                   NULL,                   NULL,                       NULL },
/* BL_FSM_INIT_STATE           */ { fsm_crc_check_hdl,       NULL,                   NULL,                   fsm_bootloop_hdl,           fsm_crc_check_hdl },
/* BL_FSM_CRC_CHECK_STATE      */ { fsm_bootloop_hdl,        fsm_auth_hdl,           fsm_crc_check_hdl,      NULL,                       NULL },
/* BL_FSM_AUTH_STATE           */ { fsm_bootloop_hdl,        fsm_boot_app_hdl,       NULL,                   NULL,                       NULL },
/* BL_FSM_BOOT_APP_STATE       */ { NULL,                    fsm_bootloop_hdl,       NULL,                   NULL,                       NULL },
/* BL_FSM_BOOTLOOP_STATE       */ { fsm_bootloop_hdl,        NULL,                   NULL,                   NULL,                       NULL },\
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
        printf("APP Start ...\r\n");
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
        printf("No APP found\r\n");
    }
}

/**
 * @brief State handler for initialization
 *
 */
static bl_fsm_evts_e
fsm_init_hdl(bl_fsm_ctx_s * const ctx)
{

}

/**
 * @brief State handler for CRC check
 *
 */
static bl_fsm_evts_e
fsm_crc_check_hdl(bl_fsm_ctx_s * const ctx)
{
    printf("Checking CRC...\r\n");
}

/**
 * @brief State handler for authentication
 *
 */
static bl_fsm_evts_e
fsm_auth_hdl(bl_fsm_ctx_s * const ctx)
{
    printf("Authenticating...\r\n");
}

/**
 * @brief State handler for booting the application
 *
 */
static bl_fsm_evts_e
fsm_boot_app_hdl(bl_fsm_ctx_s * const ctx)
{
    printf("Booting application...\r\n");
    boot_application();
    return BL_FSM_ERR_OR_NONE_EVT; // Boot process finished, loop back if needed
}

/**
 * @brief State handler for boot loop
 *
 */
static bl_fsm_evts_e
fsm_bootloop_hdl(bl_fsm_ctx_s * const ctx)
{
    printf("Bootloader loop...\r\n");
    // TODO: GPA: we need to enable the uart communication here for serial dfu support
    return BL_FSM_ERR_OR_NONE_EVT; // Stay in bootloop
}

// --- function definitions --------------------------------------------------------------------------------------------
