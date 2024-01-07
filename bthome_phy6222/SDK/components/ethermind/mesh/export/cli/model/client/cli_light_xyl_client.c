/**
    \file cli_light_xyl_client.c

    \brief This file defines the Mesh Light Xyl Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "cli_light_xyl_client.h"

#ifdef CLI_LIGHTINGS_XYL_CLIENT_MODEL

/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
DECL_CONST CLI_COMMAND cli_modelc_light_xyl_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root },

    /* Setup */
    { "setup", "Model Light_Xyl Setup", cli_modelc_light_xyl_setup },

    /* Get Model Handle */
    { "gethandle", "Get Model Handle", cli_modelc_light_xyl_get_model_handle },

    /* Set Publish Address */
    { "publishaddr", "Set Publish Address  <Peer Server Address (HEX 16-bit)> <Appkey Index (HEX 16-bit)>", cli_modelc_light_xyl_set_publish_address },

    /* Send Light Xyl Default Get */
    { "defaultget", "Send Light Xyl Default Get", cli_modelc_light_xyl_default_get},

    /* Send Light Xyl Default Set */
    { "defaultset", "Send Light Xyl Default Set", cli_modelc_light_xyl_default_set},

    /* Send Light Xyl Default Set Unacknowledged */
    { "defaultsetun", "Send Light Xyl Default Set Unacknowledged", cli_modelc_light_xyl_default_set_unacknowledged},

    /* Send Light Xyl Get */
    { "xylget", "Send Light Xyl Get", cli_modelc_light_xyl_get},

    /* Send Light Xyl Range Get */
    { "rangeget", "Send Light Xyl Range Get", cli_modelc_light_xyl_range_get},

    /* Send Light Xyl Range Set */
    { "rangeset", "Send Light Xyl Range Set", cli_modelc_light_xyl_range_set},

    /* Send Light Xyl Range Set Unacknowledged */
    { "rangesetun", "Send Light Xyl Range Set Unacknowledged", cli_modelc_light_xyl_range_set_unacknowledged},

    /* Send Light Xyl Set */
    { "xylset", "Send Light Xyl Set", cli_modelc_light_xyl_set},

    /* Send Light Xyl Set Unacknowledged */
    { "xylsetun", "Send Light Xyl Set Unacknowledged", cli_modelc_light_xyl_set_unacknowledged},

    /* Send Light Xyl Target Get */
    { "targetget", "Send Light Xyl Target Get", cli_modelc_light_xyl_target_get}
};



/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_light_xyl_client_model_handle;


/* --------------------------------------------- Function */
API_RESULT cli_modelc_light_xyl(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT("In Model Client - Light_Xyl\n");
    cli_cmd_stack_push((CLI_COMMAND*)cli_modelc_light_xyl_cmd_list, sizeof(cli_modelc_light_xyl_cmd_list) / sizeof(CLI_COMMAND));
    retval = cli_help(argc, argv);
    return retval;
}

/* light_xyl client CLI entry point */
API_RESULT cli_modelc_light_xyl_setup(UINT32 argc, UCHAR* argv[])
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
        retval = MS_light_xyl_client_init
                 (
                     element_handle,
                     &appl_light_xyl_client_model_handle,
                     cli_light_xyl_client_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Light Xyl Client Initialized. Model Handle: 0x%04X\n",
                appl_light_xyl_client_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Light Xyl Client Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    return retval;
}

/* Send Light Xyl Default Get */
API_RESULT cli_modelc_light_xyl_default_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Light Xyl Default Get\n");
    retval = MS_light_xyl_default_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Xyl Default Set */
API_RESULT cli_modelc_light_xyl_default_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_XYL_DEFAULT_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Xyl Default Set\n");

    if (3 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.lightness = (UINT16)choice;
        CONSOLE_OUT("Lightness (16-bit in HEX): 0x%04X\n", param.lightness);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.xyl_x = (UINT16)choice;
        CONSOLE_OUT("xyL x (16-bit in HEX): 0x%04X\n", param.xyl_x);
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.xyl_y = (UINT16)choice;
        CONSOLE_OUT("xyL y (16-bit in HEX): 0x%04X\n", param.xyl_y);
    }

    retval = MS_light_xyl_default_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Xyl Default Set Unacknowledged */
API_RESULT cli_modelc_light_xyl_default_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_XYL_DEFAULT_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Xyl Default Set Unacknowledged\n");

    if (3 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.lightness = (UINT16)choice;
        CONSOLE_OUT("Lightness (16-bit in HEX): 0x%04X\n", param.lightness);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.xyl_x = (UINT16)choice;
        CONSOLE_OUT("xyL x (16-bit in HEX): 0x%04X\n", param.xyl_x);
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.xyl_y = (UINT16)choice;
        CONSOLE_OUT("xyL y (16-bit in HEX): 0x%04X\n", param.xyl_y);
    }

    retval = MS_light_xyl_default_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Xyl Get */
API_RESULT cli_modelc_light_xyl_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Light Xyl Get\n");
    retval = MS_light_xyl_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Xyl Range Get */
API_RESULT cli_modelc_light_xyl_range_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Light Xyl Range Get\n");
    retval = MS_light_xyl_range_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Xyl Range Set */
API_RESULT cli_modelc_light_xyl_range_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_XYL_RANGE_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Xyl Range Set\n");

    if (4 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.xyl_x_range_min = (UINT16)choice;
        CONSOLE_OUT("xyL x Range Min (16-bit in HEX): 0x%04X\n", param.xyl_x_range_min);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.xyl_x_range_max = (UINT16)choice;
        CONSOLE_OUT("xyL x Range Max (16-bit in HEX): 0x%04X\n", param.xyl_x_range_max);
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.xyl_y_range_min = (UINT16)choice;
        CONSOLE_OUT("xyL y Range Min (16-bit in HEX): 0x%04X\n", param.xyl_y_range_min);
        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.xyl_y_range_max = (UINT16)choice;
        CONSOLE_OUT("xyL y Range Max (16-bit in HEX): 0x%04X\n", param.xyl_y_range_max);
    }

    retval = MS_light_xyl_range_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Xyl Range Set Unacknowledged */
API_RESULT cli_modelc_light_xyl_range_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_XYL_RANGE_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Xyl Range Set Unacknowledged\n");

    if (4 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.xyl_x_range_min = (UINT16)choice;
        CONSOLE_OUT("xyL x Range Min (16-bit in HEX): 0x%04X\n", param.xyl_x_range_min);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.xyl_x_range_max = (UINT16)choice;
        CONSOLE_OUT("xyL x Range Max (16-bit in HEX): 0x%04X\n", param.xyl_x_range_max);
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.xyl_y_range_min = (UINT16)choice;
        CONSOLE_OUT("xyL y Range Min (16-bit in HEX): 0x%04X\n", param.xyl_y_range_min);
        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.xyl_y_range_max = (UINT16)choice;
        CONSOLE_OUT("xyL y Range Max (16-bit in HEX): 0x%04X\n", param.xyl_y_range_max);
    }

    retval = MS_light_xyl_range_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Xyl Set */
API_RESULT cli_modelc_light_xyl_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_XYL_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Xyl Set\n");

    if (4 <= argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.xyl_lightness = (UINT16)choice;
        CONSOLE_OUT("xyL Lightness (16-bit in HEX): 0x%04X\n", param.xyl_lightness);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.xyl_x = (UINT16)choice;
        CONSOLE_OUT("xyL x (16-bit in HEX): 0x%04X\n", param.xyl_x);
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.xyl_y = (UINT16)choice;
        CONSOLE_OUT("xyL y (16-bit in HEX): 0x%04X\n", param.xyl_y);
        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.tid = (UCHAR)choice;
        CONSOLE_OUT("TID (8-bit in HEX): 0x%02X\n", param.tid);
    }

    if (6 == argc)
    {
        param.optional_fields_present = 0x01;
        choice = CLI_strtoi(argv[4], CLI_strlen(argv[4]), 16);
        param.transition_time = (UCHAR)choice;
        CONSOLE_OUT("Transition Time (8-bit in HEX): 0x%02X\n", param.transition_time);
        choice = CLI_strtoi(argv[5], CLI_strlen(argv[5]), 16);
        param.delay = (UCHAR)choice;
        CONSOLE_OUT("Delay (8-bit in HEX): 0x%02X\n", param.delay);
    }
    else
    {
        param.optional_fields_present = 0x00;
    }

    retval = MS_light_xyl_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Xyl Set Unacknowledged */
API_RESULT cli_modelc_light_xyl_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_XYL_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Xyl Set Unacknowledged\n");

    if (4 <= argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.xyl_lightness = (UINT16)choice;
        CONSOLE_OUT("xyL Lightness (16-bit in HEX): 0x%04X\n", param.xyl_lightness);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.xyl_x = (UINT16)choice;
        CONSOLE_OUT("xyL x (16-bit in HEX): 0x%04X\n", param.xyl_x);
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.xyl_y = (UINT16)choice;
        CONSOLE_OUT("xyL y (16-bit in HEX): 0x%04X\n", param.xyl_y);
        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.tid = (UCHAR)choice;
        CONSOLE_OUT("TID (8-bit in HEX): 0x%02X\n", param.tid);
    }

    if (6 == argc)
    {
        param.optional_fields_present = 0x01;
        choice = CLI_strtoi(argv[4], CLI_strlen(argv[4]), 16);
        param.transition_time = (UCHAR)choice;
        CONSOLE_OUT("Transition Time (8-bit in HEX): 0x%02X\n", param.transition_time);
        choice = CLI_strtoi(argv[5], CLI_strlen(argv[5]), 16);
        param.delay = (UCHAR)choice;
        CONSOLE_OUT("Delay (8-bit in HEX): 0x%02X\n", param.delay);
    }
    else
    {
        param.optional_fields_present = 0x00;
    }

    retval = MS_light_xyl_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Xyl Target Get */
API_RESULT cli_modelc_light_xyl_target_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Light Xyl Target Get\n");
    retval = MS_light_xyl_target_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Get Model Handle */
API_RESULT cli_modelc_light_xyl_get_model_handle(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    #if 0
    MS_ACCESS_MODEL_HANDLE model_handle;
    retval = MS_light_xyl_get_model_handle(&model_handle);

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
API_RESULT cli_modelc_light_xyl_set_publish_address(UINT32 argc, UCHAR* argv[])
{
    int  choice;
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    MS_ACCESS_PUBLISH_INFO  publish_info;
    /* Set Publish Information */
    EM_mem_set(&publish_info, 0, sizeof(publish_info));
    publish_info.addr.use_label = MS_FALSE;
    model_handle = appl_light_xyl_client_model_handle;
    CONSOLE_OUT("Model Handle (16-bit in HEX): 0x%04X\n", model_handle);

    /* Check Number of Arguments */
    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        publish_info.addr.addr = (UINT16)choice;
        CONSOLE_OUT("Light_Xyl Server Address (16-bit in HEX): 0x%04X\n", publish_info.addr.addr);
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
    Light_Xyl client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_light_xyl_client_cb
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
        "[LIGHT_XYL_CLIENT] Callback. Opcode 0x%04X\n", opcode);
    appl_dump_bytes(data_param, data_len);

    switch(opcode)
    {
    case MS_ACCESS_LIGHT_XYL_DEFAULT_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_LIGHT_XYL_DEFAULT_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_LIGHT_XYL_RANGE_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_LIGHT_XYL_RANGE_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_LIGHT_XYL_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_LIGHT_XYL_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_LIGHT_XYL_TARGET_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_LIGHT_XYL_TARGET_STATUS_OPCODE\n");
    }
    break;
    }

    return retval;
}

#endif /* CLI_LIGHTINGS_XYL_CLIENT_MODEL */
