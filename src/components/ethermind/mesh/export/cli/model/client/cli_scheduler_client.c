/**
    \file cli_scheduler_client.c

    \brief This file defines the Mesh Scheduler Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "cli_scheduler_client.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
DECL_CONST CLI_COMMAND cli_modelc_scheduler_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root },

    /* Setup */
    { "setup", "Model Scheduler Setup", cli_modelc_scheduler_setup },

    /* Get Model Handle */
    { "gethandle", "Get Model Handle", cli_modelc_scheduler_get_model_handle },

    /* Set Publish Address */
    { "publishaddr", "Set Publish Address  <Peer Server Address (HEX 16-bit)> <Appkey Index (HEX 16-bit)>", cli_modelc_scheduler_set_publish_address },

    /* Send Scheduler Action Get */
    { "actionget", "Send Scheduler Action Get", cli_modelc_scheduler_action_get},

    /* Send Scheduler Action Set */
    { "actionset", "Send Scheduler Action Set", cli_modelc_scheduler_action_set},

    /* Send Scheduler Action Set Unacknowledged */
    { "actionsetun", "Send Scheduler Action Set Unacknowledged", cli_modelc_scheduler_action_set_unacknowledged},

    /* Send Scheduler Get */
    { "get", "Send Scheduler Get", cli_modelc_scheduler_get}
};



/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_scheduler_client_model_handle;


/* --------------------------------------------- Function */
API_RESULT cli_modelc_scheduler(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT("In Model Client - Scheduler\n");
    cli_cmd_stack_push((CLI_COMMAND*)cli_modelc_scheduler_cmd_list, sizeof(cli_modelc_scheduler_cmd_list) / sizeof(CLI_COMMAND));
    retval = cli_help(argc, argv);
    return retval;
}

/* scheduler client CLI entry point */
API_RESULT cli_modelc_scheduler_setup(UINT32 argc, UCHAR* argv[])
{
    MS_ACCESS_ELEMENT_HANDLE element_handle;
    API_RESULT retval;
    static UCHAR model_initialized = 0x00;
    MS_IGNORE_UNUSED_PARAM(argc);
    MS_IGNORE_UNUSED_PARAM(argv);
    /**
        Register with Access Layer.
    */
    retval = API_FAILURE;

    if (0x00 == model_initialized)
    {
        /* Use Default Element Handle. Index 0 */
        element_handle = MS_ACCESS_DEFAULT_ELEMENT_HANDLE;
        retval = MS_scheduler_client_init
                 (
                     element_handle,
                     &appl_scheduler_client_model_handle,
                     cli_scheduler_client_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Scheduler Client Initialized. Model Handle: 0x%04X\n",
                appl_scheduler_client_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Scheduler Client Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    return retval;
}

/* Send Scheduler Action Get */
API_RESULT cli_modelc_scheduler_action_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_SCHEDULER_ACTION_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Scheduler Action Get\n");

    /* Check Number of Arguments */
    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.index = (UCHAR)choice;
        CONSOLE_OUT("Index (8-bit in HEX): 0x%02X\n", param.index);
    }

    retval = MS_scheduler_action_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Scheduler Action Set */
API_RESULT cli_modelc_scheduler_action_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_SCHEDULER_ACTION_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Scheduler Action Set\n");

    /* Check Number of Arguments */
    if (11 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.index = (UCHAR)choice;
        CONSOLE_OUT("Index (4-bit in HEX): 0x%02X\n", param.index);
        CONSOLE_OUT
        ("Scheduled year for the action (7-bit in HEX)\n");
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.year = (UINT8)choice;
        CONSOLE_OUT
        ("Scheduled month for the action (12-bit in HEX)\n");
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.month = (UINT16)choice;
        CONSOLE_OUT
        ("Scheduled day of the month for the action (5-bit in HEX)\n");
        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.day = (UINT8)choice;
        CONSOLE_OUT
        ("Scheduled hour for the action (5-bit in HEX)\n");
        choice = CLI_strtoi(argv[4], CLI_strlen(argv[4]), 16);
        param.hour = (UINT8)choice;
        CONSOLE_OUT
        ("Scheduled minute for the action (6-bit in HEX)\n");
        choice = CLI_strtoi(argv[5], CLI_strlen(argv[5]), 16);
        param.minute = (UINT8)choice;
        CONSOLE_OUT
        ("Scheduled second for the action (6-bit in HEX)\n");
        choice = CLI_strtoi(argv[6], CLI_strlen(argv[6]), 16);
        param.second = (UINT8)choice;
        CONSOLE_OUT
        ("Scheduled days of the week for the action (7-bit in HEX)\n");
        choice = CLI_strtoi(argv[7], CLI_strlen(argv[7]), 16);
        param.day_of_week = (UINT8)choice;
        CONSOLE_OUT
        ("Action to be performed at the scheduled time (4-bit in HEX)\n");
        choice = CLI_strtoi(argv[8], CLI_strlen(argv[8]), 16);
        param.action = (UINT8)choice;
        CONSOLE_OUT
        ("Scene number to be used for some actions (16-bit in HEX)\n");
        choice = CLI_strtoi(argv[9], CLI_strlen(argv[9]), 16);
        param.scene_number = (UINT16)choice;
        CONSOLE_OUT
        ("Transition time for this action (8-bit in HEX)\n");
        choice = CLI_strtoi(argv[10], CLI_strlen(argv[10]), 16);
        param.transition_time = (UINT8)choice;
    }

    retval = MS_scheduler_action_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Scheduler Action Set Unacknowledged */
API_RESULT cli_modelc_scheduler_action_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_SCHEDULER_ACTION_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Scheduler Action Set Unacknowledged\n");

    /* Check Number of Arguments */
    if (11 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.index = (UCHAR)choice;
        CONSOLE_OUT("Index (4-bit in HEX): 0x%02X\n", param.index);
        CONSOLE_OUT
        ("Scheduled year for the action (7-bit in HEX)\n");
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.year = (UINT8)choice;
        CONSOLE_OUT
        ("Scheduled month for the action (12-bit in HEX)\n");
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.month = (UINT16)choice;
        CONSOLE_OUT
        ("Scheduled day of the month for the action (5-bit in HEX)\n");
        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.day = (UINT8)choice;
        CONSOLE_OUT
        ("Scheduled hour for the action (5-bit in HEX)\n");
        choice = CLI_strtoi(argv[4], CLI_strlen(argv[4]), 16);
        param.hour = (UINT8)choice;
        CONSOLE_OUT
        ("Scheduled minute for the action (6-bit in HEX)\n");
        choice = CLI_strtoi(argv[5], CLI_strlen(argv[5]), 16);
        param.minute = (UINT8)choice;
        CONSOLE_OUT
        ("Scheduled second for the action (6-bit in HEX)\n");
        choice = CLI_strtoi(argv[6], CLI_strlen(argv[6]), 16);
        param.second = (UINT8)choice;
        CONSOLE_OUT
        ("Scheduled days of the week for the action (7-bit in HEX)\n");
        choice = CLI_strtoi(argv[7], CLI_strlen(argv[7]), 16);
        param.day_of_week = (UINT8)choice;
        CONSOLE_OUT
        ("Action to be performed at the scheduled time (4-bit in HEX)\n");
        choice = CLI_strtoi(argv[8], CLI_strlen(argv[8]), 16);
        param.action = (UINT8)choice;
        CONSOLE_OUT
        ("Scene number to be used for some actions (16-bit in HEX)\n");
        choice = CLI_strtoi(argv[9], CLI_strlen(argv[9]), 16);
        param.scene_number = (UINT16)choice;
        CONSOLE_OUT
        ("Transition time for this action (8-bit in HEX)\n");
        choice = CLI_strtoi(argv[10], CLI_strlen(argv[10]), 16);
        param.transition_time = (UINT8)choice;
    }

    retval = MS_scheduler_action_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Scheduler Get */
API_RESULT cli_modelc_scheduler_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Scheduler Get\n");
    retval = MS_scheduler_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}


/* Get Model Handle */
API_RESULT cli_modelc_scheduler_get_model_handle(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    #if 0
    MS_ACCESS_MODEL_HANDLE model_handle;
    retval = MS_scheduler_get_model_handle(&model_handle);

    if (API_SUCCESS == retval)
    {
        CONSOLE_OUT
        (">> Model Handle 0x%04X\n", model_handle);
    }
    else
    {
        CONSOLE_OUT
        (">> Get Model Handle Failed. Status 0x%04X\n", retval);
    }

    #else
    retval = API_FAILURE;
    CONSOLE_OUT("To be implemented\n");
    #endif /* 0 */
    return retval;
}


/* Set Publish Address */
API_RESULT cli_modelc_scheduler_set_publish_address(UINT32 argc, UCHAR* argv[])
{
    int  choice;
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    MS_ACCESS_PUBLISH_INFO  publish_info;
    /* Set Publish Information */
    EM_mem_set(&publish_info, 0, sizeof(publish_info));
    publish_info.addr.use_label = MS_FALSE;
    model_handle = appl_scheduler_client_model_handle;
    CONSOLE_OUT("Model Handle (16-bit in HEX): 0x%04X\n", model_handle);

    /* Check Number of Arguments */
    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        publish_info.addr.addr = (UINT16)choice;
        CONSOLE_OUT("Scheduler Server Address (16-bit in HEX): 0x%04X\n", publish_info.addr.addr);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        publish_info.appkey_index = (UINT16)choice;
        CONSOLE_OUT("AppKey Index: 0x%04X\n", publish_info.appkey_index);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    publish_info.remote = MS_FALSE;
    retval = MS_access_cm_set_model_publication
             (
                 model_handle,
                 &publish_info
             );

    if (API_SUCCESS == retval)
    {
        CONSOLE_OUT
        (">> Publish Address is set Successfully.\n");
    }
    else
    {
        CONSOLE_OUT
        (">> Failed to set publish address. Status 0x%04X\n", retval);
    }

    return retval;
}

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Scheduler client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_scheduler_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
)
{
    API_RESULT retval;
    retval = API_SUCCESS;
    CONSOLE_OUT (
        "[SCHEDULER_CLIENT] Callback. Opcode 0x%04X\n", opcode);
    appl_dump_bytes(data_param, data_len);

    switch(opcode)
    {
    case MS_ACCESS_SCHEDULER_ACTION_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_SCHEDULER_ACTION_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_SCHEDULER_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_SCHEDULER_STATUS_OPCODE\n");
    }
    break;
    }

    return retval;
}

