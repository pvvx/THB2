/**
    \file cli_generic_default_transition_time_client.c

    \brief This file defines the Mesh Generic Default Transition Time Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "cli_generic_default_transition_time_client.h"

#ifdef CLI_GENERICS_TRANSITIONTIME_CLIENT_MODEL

/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
DECL_CONST CLI_COMMAND cli_modelc_generic_default_transition_time_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root },

    /* Setup */
    { "setup", "Model Generic_Default_Transition_Time Setup", cli_modelc_generic_default_transition_time_setup },

    /* Get Model Handle */
    { "gethandle", "Get Model Handle", cli_modelc_generic_default_transition_time_get_model_handle },

    /* Set Publish Address */
    { "publishaddr", "Set Publish Address  <Peer Server Address (HEX 16-bit)> <Appkey Index (HEX 16-bit)>", cli_modelc_generic_default_transition_time_set_publish_address },

    /* Send Generic Default Transition Time Get */
    { "get", "Send Generic Default Transition Time Get", cli_modelc_generic_default_transition_time_get},

    /* Send Generic Default Transition Time Set */
    { "set", "Send Generic Default Transition Time Set", cli_modelc_generic_default_transition_time_set},

    /* Send Generic Default Transition Time Set Unacknowledged */
    { "setun", "Send Generic Default Transition Time Set Unacknowledged", cli_modelc_generic_default_transition_time_set_unacknowledged},

};


/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_generic_default_transition_time_client_model_handle;


/* --------------------------------------------- Function */
API_RESULT cli_modelc_generic_default_transition_time(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT("In Model Client - Generic_Default_Transition_Time\n");
    cli_cmd_stack_push((CLI_COMMAND*)cli_modelc_generic_default_transition_time_cmd_list, sizeof(cli_modelc_generic_default_transition_time_cmd_list) / sizeof(CLI_COMMAND));
    retval = cli_help(argc, argv);
    return retval;
}

/* generic_default_transition_time client CLI entry point */
API_RESULT cli_modelc_generic_default_transition_time_setup(UINT32 argc, UCHAR* argv[])
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
        retval = MS_generic_default_transition_time_client_init
                 (
                     element_handle,
                     &appl_generic_default_transition_time_client_model_handle,
                     cli_generic_default_transition_time_client_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Generic Default Transition Time Client Initialized. Model Handle: 0x%04X\n",
                appl_generic_default_transition_time_client_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Generic Default Transition Time Client Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    return retval;
}

/* Send Generic Default Transition Time Get */
API_RESULT cli_modelc_generic_default_transition_time_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Generic Default Transition Time Get\n");
    retval = MS_generic_default_transition_time_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Generic Default Transition Time Set */
API_RESULT cli_modelc_generic_default_transition_time_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_DEFAULT_TRANSITION_TIME_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Default Transition Time Set\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.transition_number_of_steps = (UCHAR)choice;
        CONSOLE_OUT("Transition Number of Steps (6-bit in HEX): 0x%02X\n", param.transition_number_of_steps);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.transition_step_resolution = (UCHAR)choice;
        CONSOLE_OUT("Transition Step Resolution (2-bit in HEX): 0x%02X\n", param.transition_step_resolution);
    }

    retval = MS_generic_default_transition_time_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Generic Default Transition Time Set Unacknowledged */
API_RESULT cli_modelc_generic_default_transition_time_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_DEFAULT_TRANSITION_TIME_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Default Transition Time Set Unacknowledged\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.transition_number_of_steps = (UCHAR)choice;
        CONSOLE_OUT("Transition Number of Steps (6-bit in HEX): 0x%02X\n", param.transition_number_of_steps);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.transition_step_resolution = (UCHAR)choice;
        CONSOLE_OUT("Transition Step Resolution (2-bit in HEX): 0x%02X\n", param.transition_step_resolution);
    }

    retval = MS_generic_default_transition_time_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Get Model Handle */
API_RESULT cli_modelc_generic_default_transition_time_get_model_handle(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    #if 0
    MS_ACCESS_MODEL_HANDLE model_handle;
    retval = MS_generic_default_transition_time_get_model_handle(&model_handle);

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
API_RESULT cli_modelc_generic_default_transition_time_set_publish_address(UINT32 argc, UCHAR* argv[])
{
    int  choice;
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    MS_ACCESS_PUBLISH_INFO  publish_info;
    /* Set Publish Information */
    EM_mem_set(&publish_info, 0, sizeof(publish_info));
    publish_info.addr.use_label = MS_FALSE;
    model_handle = appl_generic_default_transition_time_client_model_handle;
    CONSOLE_OUT("Model Handle (16-bit in HEX): 0x%04X\n", model_handle);

    /* Check Number of Arguments */
    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        publish_info.addr.addr = (UINT16)choice;
        CONSOLE_OUT("Generic_Default_Transition_Time Server Address (16-bit in HEX): 0x%04X\n", publish_info.addr.addr);
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
    Generic_Default_Transition_Time client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_generic_default_transition_time_client_cb
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
        "[GENERIC_DEFAULT_TRANSITION_TIME_CLIENT] Callback. Opcode 0x%04X\n", opcode);
    appl_dump_bytes(data_param, data_len);

    switch(opcode)
    {
    case MS_ACCESS_GENERIC_DEFAULT_TRANSITION_TIME_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_GENERIC_DEFAULT_TRANSITION_TIME_STATUS_OPCODE\n");
    }
    break;
    }

    return retval;
}

#endif /* CLI_GENERICS_TRANSITIONTIME_CLIENT_MODEL */
