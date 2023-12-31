
/**
    \file cli_main.c

    This File contains the "main" function for the CLI application,
    to exercise various functionalities of the Mindtree Mesh stack.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef DEMO

/* ------------------------------- Header File Inclusion */
#include "cli_main.h"
#include "nvs.h"

/* ------------------------------- Global Variables */
#ifndef CLI_NO_MAIN
    DECL_STATIC UCHAR r_buf[1024];
#else
    void cli_input (char* cmd, int size);
#endif /* CLI_NO_MAIN */

/* Level - Root */
DECL_CONST CLI_COMMAND cli_root_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Core */
    { "core", "Core Options", cli_core },

    /* Model */
    { "model", "Model Options", cli_model },

    /* Reset */
    { "reset", "Reset Node", cli_reset },

    /* Persistent Storage */
    { "ps", "Persistent Storage", cli_ps },

    /* Mesh ADV/GATT bearer Operations */
    { "brr", "Mesh bearer Operations", cli_brr },

    /* Set Log Level */
    { "loglevel", "Set Log Level <ERR/INF/TRC/ALL>", cli_set_log_level }
};

/* Command List Stack */
typedef struct _CLI_COMMAND_STACK_FRAME
{
    CLI_COMMAND* cmd_list;
    UINT16        cmd_list_len;

} CLI_COMMAND_STACK_FRAME;

#define  CLI_CMD_LIST_MAX_DEPTH     10
DECL_STATIC UINT16        cli_cmd_list_sp;
DECL_STATIC CLI_COMMAND_STACK_FRAME cli_cmd_list_stack[CLI_CMD_LIST_MAX_DEPTH];

/* ------------------------------- Functions */
/* Common Utility Routines */
void cli_cmd_stack_push(/* IN */ CLI_COMMAND* cmd_list, /* IN */ UINT16 cmd_list_len)
{
    cli_cmd_list_sp++;
    cli_cmd_list_stack[cli_cmd_list_sp].cmd_list = cmd_list;
    cli_cmd_list_stack[cli_cmd_list_sp].cmd_list_len = cmd_list_len;
}

void cli_cmd_stack_pop(void)
{
    /* Check if the Stack point in not 0. Decrement by one */
    if (0 != cli_cmd_list_sp)
    {
        cli_cmd_list_sp--;
    }
    else
    {
        CONSOLE_OUT("Already at Root Level\n");
    }
}

API_RESULT cli_reset(UINT32 argc, UCHAR* argv[])
{
    MS_IGNORE_UNUSED_PARAM(argc);
    MS_IGNORE_UNUSED_PARAM(argv);
    CONSOLE_OUT ("Clearing Storage\n");
    nvs_reset(NVS_BANK_PERSISTENT);
    return API_SUCCESS;
}
extern uint8 llState;
extern uint8 llSecondaryState;
extern UCHAR blebrr_state;
extern uint32 blebrr_advscan_timeout_count;

/* Help */
API_RESULT cli_help(UINT32 argc, UCHAR* argv[])
{
    UINT32 index;
    UINT32 sum = 0;
    MS_IGNORE_UNUSED_PARAM(argc);
    MS_IGNORE_UNUSED_PARAM(argv);
    CONSOLE_OUT("In Help\n");

    /* Print all the available commands */
    for (index = 0; index < cli_cmd_list_stack[cli_cmd_list_sp].cmd_list_len; index++)
    {
        CONSOLE_OUT("    %s: %s\n",
                    cli_cmd_list_stack[cli_cmd_list_sp].cmd_list[index].cmd,
                    cli_cmd_list_stack[cli_cmd_list_sp].cmd_list[index].desc);
    }

    printf("osal_memory sum = %d\r\n", sum);
    printf("\r\n===== internal status ============\r\n");
    printf("llState = %d, llSecondaryState = %d\r\n", llState, llSecondaryState);
    printf("blebrr_state = %d\r\n", blebrr_state);
    printf("blebrr_advscan_timeout_count = %d\r\n", blebrr_advscan_timeout_count);
    return API_SUCCESS;
}

/* Back */
API_RESULT cli_back(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("Going Back \n");
    cli_cmd_stack_pop();
    /* Print Help of the new level */
    cli_help(argc, argv);
    return API_SUCCESS;
}

/* Root */
API_RESULT cli_root(UINT32 argc, UCHAR* argv[])
{
    CONSOLE_OUT("Level - Root\n");
    /* Set Stack Pointer to Root */
    cli_cmd_list_sp = 0;
    cli_cmd_list_stack[cli_cmd_list_sp].cmd_list = (CLI_COMMAND*)cli_root_cmd_list;
    cli_cmd_list_stack[cli_cmd_list_sp].cmd_list_len = sizeof(cli_root_cmd_list) / sizeof(CLI_COMMAND);
    /* Print Help of the Root level */
    cli_help(argc, argv);
    return API_SUCCESS;
}

/* Log Level */
API_RESULT cli_set_log_level(UINT32 argc, UCHAR* argv[])
{
    UINT32  index;
    UCHAR* log_levels[4] = { "ERR", "TRC", "INF", "ALL" };
    CONSOLE_OUT("In Set Log Level\n");

    if (1 == argc)
    {
        for (index = 0; index < 4; index++)
        {
            if (0 == CLI_STR_COMPARE(argv[0], log_levels[index]))
            {
                break;
            }
        }

        if (index > 3)
        {
            CONSOLE_OUT("Invalid Argument:%s\n", argv[0]);
            return API_FAILURE;
        }

        if (index < 3)
        {
            index += 1;
        }

        EM_set_debug_level((UCHAR)index);
        CONSOLE_OUT("Set Log Level:0x%02X\n", (UCHAR)index);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X\n", argc);
    }

    return API_SUCCESS;
}

/* Module related Utility Routines */
void cli_gatt_bearer_iface_event_pl_cb
(
    UCHAR  ev_name,
    UCHAR  ev_param
)
{
    switch(ev_name)
    {
    /* GATT Bearer BLE Link Layer Disconnected */
    case BLEBRR_GATT_IFACE_DOWN:
        CONSOLE_OUT("\r\n >> GATT Bearer BLE Link Layer Disconnection Event Received!\r\n");
        CONSOLE_OUT("Invoke relevant CLI Commands! \r\n");
        break;

    /* GATT Bearer BLE Link Layer Connected */
    case BLEBRR_GATT_IFACE_UP:
        CONSOLE_OUT("\r\n >> GATT Bearer BLE Link Layer Connection Event Received!\r\n");
        /**
            TODO:
            Check if this only when DUT is Unprovisioned Device!
            also check if this needs to be done.
        */
        MS_prov_stop_interleave_timer();
        MS_brr_bcast_end(BRR_BCON_TYPE_UNPROV_DEVICE, BRR_BCON_ACTIVE);
        break;

    /* GATT Bearer Service Enabled for Communication */
    case BLEBRR_GATT_IFACE_ENABLE:
        CONSOLE_OUT("\r\n >> GATT Bearer Active Event Received!\r\n");
        {
            if (BLEBRR_GATT_PROV_MODE == ev_param)
            {
                /* Call to bind with the selected device */
                /**
                    TODO:
                    This needs to be made role Specific.
                    For Provisioner role, this below function needs to be
                    updated. Typically for Provisioner Role, "Bind" needs
                    to happen from the CLI command pointing to the desired
                    PROV_DEVICE_S of Remote Unprovisioned Device.
                */
                appl_prov_bind_device(PROV_BRR_GATT);
            }
        }
        break;

    /* GATT Bearer Service Disabled for Communication */
    case BLEBRR_GATT_IFACE_DISABLE:
        CONSOLE_OUT("\r\n >> GATT Bearer Inactive Event Received!\r\n");
        CONSOLE_OUT("Invoke relevant CLI Commands! \r\n");
        break;

    /* Unknown Event! */
    default:
        CONSOLE_OUT("\r\n >> GATT Bearer BLE Link Layer Unknown Event 0x%02X Received!\r\n", ev_name);
        break;
    }
}

#ifndef CLI_NO_MAIN
int main (int argc, char** argv)
{
    MS_IGNORE_UNUSED_PARAM(argc);
    MS_IGNORE_UNUSED_PARAM(argv);
    /* Initialize CLI */
    CLI_init();
    /* Set Root Command List and Length */
    cli_root(0, NULL);
    MS_LOOP_FOREVER()
    {
        /* */
        fgets(r_buf, sizeof(r_buf), stdin);
        CLI_process_line
        (
            r_buf,
            CLI_strlen(r_buf),
            cli_cmd_list_stack[cli_cmd_list_sp].cmd_list,
            cli_cmd_list_stack[cli_cmd_list_sp].cmd_list_len
        );
    }
    return 0;
}
#else /* CLI_NO_MAIN */
void cli_input (char* cmd, int size)
{
    CLI_process_line
    (
        (UCHAR*) cmd,
        size
    );
}
#endif /* CLI_NO_MAIN */
#endif /* DEMO */
