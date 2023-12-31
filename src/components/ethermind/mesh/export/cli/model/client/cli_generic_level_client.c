/**
    \file cli_generic_level_client.c

    \brief This file defines the Mesh Generic Level Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "cli_generic_level_client.h"

#ifdef CLI_GENERICS_LEVEL_CLIENT_MODEL
/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
DECL_CONST CLI_COMMAND cli_modelc_generic_level_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root },

    /* Setup */
    { "setup", "Model Generic_Level Setup", cli_modelc_generic_level_setup },

    /* Get Model Handle */
    { "gethandle", "Get Model Handle", cli_modelc_generic_level_get_model_handle },

    /* Set Publish Address */
    { "publishaddr", "Set Publish Address  <Peer Server Address (HEX 16-bit)> <Appkey Index (HEX 16-bit)>", cli_modelc_generic_level_set_publish_address },

    /* Send Generic Delta Set */
    { "deltaset", "Send Generic Delta Set", cli_modelc_generic_delta_set},

    /* Send Generic Delta Set Unacknowledged */
    { "deltasetun", "Send Generic Delta Set Unacknowledged", cli_modelc_generic_delta_set_unacknowledged},

    /* Send Generic Level Get */
    { "levelget", "Send Generic Level Get", cli_modelc_generic_level_get},

    /* Send Generic Level Set */
    { "levelset", "Send Generic Level Set", cli_modelc_generic_level_set},

    /* Send Generic Level Set Unacknowledged */
    { "levelsetun", "Send Generic Level Set Unacknowledged", cli_modelc_generic_level_set_unacknowledged},

    /* Send Generic Move Set */
    { "moveset", "Send Generic Move Set", cli_modelc_generic_move_set},

    /* Send Generic Move Set Unacknowledged */
    { "movesetun", "Send Generic Move Set Unacknowledged", cli_modelc_generic_move_set_unacknowledged}
};



/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_generic_level_client_model_handle;


/* --------------------------------------------- Function */
API_RESULT cli_modelc_generic_level(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT("In Model Client - Generic_Level\n");
    cli_cmd_stack_push((CLI_COMMAND*)cli_modelc_generic_level_cmd_list, sizeof(cli_modelc_generic_level_cmd_list) / sizeof(CLI_COMMAND));
    retval = cli_help(argc, argv);
    return retval;
}

/* generic_level client CLI entry point */
API_RESULT cli_modelc_generic_level_setup(UINT32 argc, UCHAR* argv[])
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
        retval = MS_generic_level_client_init
                 (
                     element_handle,
                     &appl_generic_level_client_model_handle,
                     cli_generic_level_client_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Generic Level Client Initialized. Model Handle: 0x%04X\n",
                appl_generic_level_client_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Generic Level Client Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    return retval;
}

/* Send Generic Delta Set */
API_RESULT cli_modelc_generic_delta_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_DELTA_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Delta Set\n");

    if (2 <= argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.delta_level = (UINT32)choice;
        CONSOLE_OUT("Delta Level (32-bit in HEX): 0x%08X\n", param.delta_level);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.tid = (UCHAR)choice;
        CONSOLE_OUT("TID (8-bit in HEX): 0x%02X\n", param.tid);
    }

    if (4 == argc)
    {
        param.optional_fields_present = 0x01;
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.transition_time = (UCHAR)choice;
        CONSOLE_OUT("Transition Time (8-bit in HEX): 0x%02X\n", param.transition_time);
        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.delay = (UCHAR)choice;
        CONSOLE_OUT("Delay (8-bit in HEX): 0x%02X\n", param.delay);
    }
    else
    {
        param.optional_fields_present = 0x00;
    }

    retval = MS_generic_delta_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Generic Delta Set Unacknowledged */
API_RESULT cli_modelc_generic_delta_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_DELTA_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Delta Set Unacknowledged\n");

    if (2 <= argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.delta_level = (UINT32)choice;
        CONSOLE_OUT("Delta Level (32-bit in HEX): 0x%08X\n", param.delta_level);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.tid = (UCHAR)choice;
        CONSOLE_OUT("TID (8-bit in HEX): 0x%02X\n", param.tid);
    }

    if (4 == argc)
    {
        param.optional_fields_present = 0x01;
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.transition_time = (UCHAR)choice;
        CONSOLE_OUT("Transition Time (8-bit in HEX): 0x%02X\n", param.transition_time);
        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.delay = (UCHAR)choice;
        CONSOLE_OUT("Delay (8-bit in HEX): 0x%02X\n", param.delay);
    }
    else
    {
        param.optional_fields_present = 0x00;
    }

    retval = MS_generic_delta_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Generic Level Get */
API_RESULT cli_modelc_generic_level_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Generic Level Get\n");
    retval = MS_generic_level_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Generic Level Set */
API_RESULT cli_modelc_generic_level_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_LEVEL_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Level Set\n");

    if (2 <= argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.level = (UINT16)choice;
        CONSOLE_OUT("Level (16-bit in HEX): 0x%04X\n", param.level);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.tid = (UCHAR)choice;
        CONSOLE_OUT("TID (8-bit in HEX): 0x%02X\n", param.tid);
    }

    if (4 == argc)
    {
        param.optional_fields_present = 0x01;
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.transition_time = (UCHAR)choice;
        CONSOLE_OUT("Transition Time (8-bit in HEX): 0x%02X\n", param.transition_time);
        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.delay = (UCHAR)choice;
        CONSOLE_OUT("Delay (8-bit in HEX): 0x%02X\n", param.delay);
    }
    else
    {
        param.optional_fields_present = 0x00;
    }

    retval = MS_generic_level_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Generic Level Set Unacknowledged */
API_RESULT cli_modelc_generic_level_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_LEVEL_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Level Set Unacknowledged\n");

    if (2 <= argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.level = (UINT16)choice;
        CONSOLE_OUT("Level (16-bit in HEX): 0x%04X\n", param.level);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.tid = (UCHAR)choice;
        CONSOLE_OUT("TID (8-bit in HEX): 0x%02X\n", param.tid);
    }

    if (4 == argc)
    {
        param.optional_fields_present = 0x01;
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.transition_time = (UCHAR)choice;
        CONSOLE_OUT("Transition Time (8-bit in HEX): 0x%02X\n", param.transition_time);
        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.delay = (UCHAR)choice;
        CONSOLE_OUT("Delay (8-bit in HEX): 0x%02X\n", param.delay);
    }
    else
    {
        param.optional_fields_present = 0x00;
    }

    retval = MS_generic_level_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Generic Move Set */
API_RESULT cli_modelc_generic_move_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_MOVE_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Move Set\n");

    if (2 <= argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.delta_level = (UINT16)choice;
        CONSOLE_OUT("Delta Level (16-bit in HEX): 0x%04X\n", param.delta_level);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.tid = (UCHAR)choice;
        CONSOLE_OUT("TID (8-bit in HEX): 0x%02X\n", param.tid);
    }

    if (4 == argc)
    {
        param.optional_fields_present = 0x01;
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.transition_time = (UCHAR)choice;
        CONSOLE_OUT("Transition Time (8-bit in HEX): 0x%02X\n", param.transition_time);
        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.delay = (UCHAR)choice;
        CONSOLE_OUT("Delay (8-bit in HEX): 0x%02X\n", param.delay);
    }
    else
    {
        param.optional_fields_present = 0x00;
    }

    retval = MS_generic_move_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Generic Move Set Unacknowledged */
API_RESULT cli_modelc_generic_move_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_MOVE_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Move Set Unacknowledged\n");

    if (2 <= argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.delta_level = (UINT16)choice;
        CONSOLE_OUT("Delta Level (16-bit in HEX): 0x%04X\n", param.delta_level);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.tid = (UCHAR)choice;
        CONSOLE_OUT("TID (8-bit in HEX): 0x%02X\n", param.tid);
    }

    if (4 == argc)
    {
        param.optional_fields_present = 0x01;
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.transition_time = (UCHAR)choice;
        CONSOLE_OUT("Transition Time (8-bit in HEX): 0x%02X\n", param.transition_time);
        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.delay = (UCHAR)choice;
        CONSOLE_OUT("Delay (8-bit in HEX): 0x%02X\n", param.delay);
    }
    else
    {
        param.optional_fields_present = 0x00;
    }

    retval = MS_generic_move_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}


/* Get Model Handle */
API_RESULT cli_modelc_generic_level_get_model_handle(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    #if 0
    MS_ACCESS_MODEL_HANDLE model_handle;
    retval = MS_generic_level_get_model_handle(&model_handle);

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
API_RESULT cli_modelc_generic_level_set_publish_address(UINT32 argc, UCHAR* argv[])
{
    int  choice;
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    MS_ACCESS_PUBLISH_INFO  publish_info;
    /* Set Publish Information */
    EM_mem_set(&publish_info, 0, sizeof(publish_info));
    publish_info.addr.use_label = MS_FALSE;
    model_handle = appl_generic_level_client_model_handle;
    CONSOLE_OUT("Model Handle (16-bit in HEX): 0x%04X\n", model_handle);

    /* Check Number of Arguments */
    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        publish_info.addr.addr = (UINT16)choice;
        CONSOLE_OUT("Generic_Level Server Address (16-bit in HEX): 0x%04X\n", publish_info.addr.addr);
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
    Generic_Level client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_generic_level_client_cb
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
        "[GENERIC_LEVEL_CLIENT] Callback. Opcode 0x%04X\n", opcode);
    appl_dump_bytes(data_param, data_len);

    switch(opcode)
    {
    case MS_ACCESS_GENERIC_LEVEL_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_GENERIC_LEVEL_STATUS_OPCODE\n");
    }
    break;
    }

    return retval;
}

#endif /* CLI_GENERICS_LEVEL_CLIENT_MODEL */
