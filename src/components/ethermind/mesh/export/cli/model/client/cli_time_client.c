/**
    \file cli_time_client.c

    \brief This file defines the Mesh Time Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "cli_time_client.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
DECL_CONST CLI_COMMAND cli_modelc_time_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root },

    /* Setup */
    { "setup", "Model Time Setup", cli_modelc_time_setup },

    /* Get Model Handle */
    { "gethandle", "Get Model Handle", cli_modelc_time_get_model_handle },

    /* Set Publish Address */
    { "publishaddr", "Set Publish Address  <Peer Server Address (HEX 16-bit)> <Appkey Index (HEX 16-bit)>", cli_modelc_time_set_publish_address },

    /* Send Tai Utc Delta Get */
    { "taideltaget", "Send Tai Utc Delta Get", cli_modelc_tai_utc_delta_get},

    /* Send Tai Utc Delta Set */
    { "taideltaset", "Send Tai Utc Delta Set", cli_modelc_tai_utc_delta_set},

    /* Send Time Get */
    { "timeget", "Send Time Get", cli_modelc_time_get},

    /* Send Time Role Get */
    { "timeroleget", "Send Time Role Get", cli_modelc_time_role_get},

    /* Send Time Role Set */
    { "timeroleset", "Send Time Role Set", cli_modelc_time_role_set},

    /* Send Time Set */
    { "timeset", "Send Time Set", cli_modelc_time_set},

    /* Send Time Zone Get */
    { "timezoneget", "Send Time Zone Get", cli_modelc_time_zone_get},

    /* Send Time Zone Set */
    { "timezoneset", "Send Time Zone Set", cli_modelc_time_zone_set}

};



/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_time_client_model_handle;


/* --------------------------------------------- Function */
API_RESULT cli_modelc_time(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT("In Model Client - Time\n");
    cli_cmd_stack_push((CLI_COMMAND*)cli_modelc_time_cmd_list, sizeof(cli_modelc_time_cmd_list) / sizeof(CLI_COMMAND));
    retval = cli_help(argc, argv);
    return retval;
}

/* time client CLI entry point */
API_RESULT cli_modelc_time_setup(UINT32 argc, UCHAR* argv[])
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
        retval = MS_time_client_init
                 (
                     element_handle,
                     &appl_time_client_model_handle,
                     cli_time_client_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Time Client Initialized. Model Handle: 0x%04X\n",
                appl_time_client_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Time Client Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    return retval;
}

/* Send Tai Utc Delta Get */
API_RESULT cli_modelc_tai_utc_delta_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Tai Utc Delta Get\n");
    retval = MS_tai_utc_delta_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Tai Utc Delta Set */
API_RESULT cli_modelc_tai_utc_delta_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_TAI_UTC_DELTA_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Tai Utc Delta Set\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.tai_utc_delta_new = (UINT16)choice;
        CONSOLE_OUT("TAI UTC Delta New (15-bit in HEX): 0x%04X\n", param.tai_utc_delta_new);
        /* choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16); */
        param.padding = (UCHAR)0x00;
        CONSOLE_OUT("Padding (1-bit in HEX): 0x%02X\n", param.padding);
        CLI_strtoarray_le
        (
            argv[1],
            CLI_strlen(argv[1]),
            &param.tai_of_delta_change[0],
            5
        );
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_tai_utc_delta_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Time Get */
API_RESULT cli_modelc_time_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Time Get\n");
    retval = MS_time_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Time Role Get */
API_RESULT cli_modelc_time_role_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Time Role Get\n");
    retval = MS_time_role_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Time Role Set */
API_RESULT cli_modelc_time_role_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_TIME_ROLE_STRUCT  param;
    CONSOLE_OUT
    (">> Send Time Role Set\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.time_role = (UCHAR)choice;
        CONSOLE_OUT("Time Role (8-bit in HEX): 0x%02X\n", param.time_role);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_time_role_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}


/* Send Time Set */
API_RESULT cli_modelc_time_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_TIME_STRUCT  param;
    CONSOLE_OUT
    (">> Send Time Set\n");

    if (6 == argc)
    {
        CLI_strtoarray_le
        (
            argv[0],
            CLI_strlen(argv[0]),
            &param.tai_seconds[0],
            5
        );
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.subsecond = (UCHAR)choice;
        CONSOLE_OUT("Subsecond (8-bit in HEX): 0x%02X\n", param.subsecond);
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.uncertainty = (UCHAR)choice;
        CONSOLE_OUT("Uncertainty (8-bit in HEX): 0x%02X\n", param.uncertainty);
        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.time_authority = (UCHAR)choice;
        CONSOLE_OUT("Time Authority (1-bit in HEX): 0x%02X\n", param.time_authority);
        choice = CLI_strtoi(argv[4], CLI_strlen(argv[4]), 16);
        param.tai_utc_delta = (UINT16)choice;
        CONSOLE_OUT("TAI UTC Delta (15-bit in HEX): 0x%04X\n", param.tai_utc_delta);
        choice = CLI_strtoi(argv[5], CLI_strlen(argv[5]), 16);
        param.time_zone_offset = (UCHAR)choice;
        CONSOLE_OUT("Time Zone Offset (8-bit in HEX): 0x%02X\n", param.time_zone_offset);
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_time_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Time Zone Get */
API_RESULT cli_modelc_time_zone_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Time Zone Get\n");
    retval = MS_time_zone_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Time Zone Set */
API_RESULT cli_modelc_time_zone_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_TIME_ZONE_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Time Zone Set\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.time_zone_offset_new = (UCHAR)choice;
        CONSOLE_OUT("Time Zone Offset New (8-bit in HEX): 0x%02X\n", param.time_zone_offset_new);
        CLI_strtoarray_le
        (
            argv[1],
            CLI_strlen(argv[1]),
            &param.tai_of_zone_change[0],
            5
        );
    }
    else
    {
        CONSOLE_OUT("Invalid Number of Arguments:0x%04X. Returning.\n", argc);
        return API_FAILURE;
    }

    retval = MS_time_zone_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Get Model Handle */
API_RESULT cli_modelc_time_get_model_handle(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    #if 0
    MS_ACCESS_MODEL_HANDLE model_handle;
    retval = MS_time_get_model_handle(&model_handle);

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
API_RESULT cli_modelc_time_set_publish_address(UINT32 argc, UCHAR* argv[])
{
    int  choice;
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    MS_ACCESS_PUBLISH_INFO  publish_info;
    /* Set Publish Information */
    EM_mem_set(&publish_info, 0, sizeof(publish_info));
    publish_info.addr.use_label = MS_FALSE;
    model_handle = appl_time_client_model_handle;
    CONSOLE_OUT("Model Handle (16-bit in HEX): 0x%04X\n", model_handle);

    /* Check Number of Arguments */
    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        publish_info.addr.addr = (UINT16)choice;
        CONSOLE_OUT("Time Server Address (16-bit in HEX): 0x%04X\n", publish_info.addr.addr);
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
    Time client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_time_client_cb
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
        "[TIME_CLIENT] Callback. Opcode 0x%04X\n", opcode);
    appl_dump_bytes(data_param, data_len);

    switch(opcode)
    {
    case MS_ACCESS_TAI_UTC_DELTA_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_TAI_UTC_DELTA_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_TIME_ROLE_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_TIME_ROLE_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_TIME_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_TIME_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_TIME_ZONE_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_TIME_ZONE_STATUS_OPCODE\n");
    }
    break;
    }

    return retval;
}

