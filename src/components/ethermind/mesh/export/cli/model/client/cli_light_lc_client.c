/**
    \file cli_light_lc_client.c

    \brief This file defines the Mesh Light Lc Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "cli_light_lc_client.h"

#ifdef CLI_LIGHTINGS_LC_CLIENT_MODEL

/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
DECL_CONST CLI_COMMAND cli_modelc_light_lc_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root },

    /* Setup */
    { "setup", "Model Light_Lc Setup", cli_modelc_light_lc_setup },

    /* Get Model Handle */
    { "gethandle", "Get Model Handle", cli_modelc_light_lc_get_model_handle },

    /* Set Publish Address */
    { "publishaddr", "Set Publish Address  <Peer Server Address (HEX 16-bit)> <Appkey Index (HEX 16-bit)>", cli_modelc_light_lc_set_publish_address },

    /* Send Light Lc Light Onoff Get */
    { "lightonoffget", "Send Light Lc Light Onoff Get", cli_modelc_light_lc_light_onoff_get},

    /* Send Light Lc Light Onoff Set */
    { "lightonoffset", "Send Light Lc Light Onoff Set", cli_modelc_light_lc_light_onoff_set},

    /* Send Light Lc Light Onoff Set Unacknowledged */
    { "lightonoffsetun", "Send Light Lc Light Onoff Set Unacknowledged", cli_modelc_light_lc_light_onoff_set_unacknowledged},

    /* Send Light Lc Mode Get */
    { "modeget", "Send Light Lc Mode Get", cli_modelc_light_lc_mode_get},

    /* Send Light Lc Mode Set */
    { "modeset", "Send Light Lc Mode Set", cli_modelc_light_lc_mode_set},

    /* Send Light Lc Mode Set Unacknowledged */
    { "modesetun", "Send Light Lc Mode Set Unacknowledged", cli_modelc_light_lc_mode_set_unacknowledged},

    /* Send Light Lc Om Get */
    { "omget", "Send Light Lc Om Get", cli_modelc_light_lc_om_get},

    /* Send Light Lc Om Set */
    { "omset", "Send Light Lc Om Set", cli_modelc_light_lc_om_set},

    /* Send Light Lc Om Set Unacknowledged */
    { "omsetun", "Send Light Lc Om Set Unacknowledged", cli_modelc_light_lc_om_set_unacknowledged},

    /* Send Light Lc Property Get */
    { "propertyget", "Send Light Lc Property Get", cli_modelc_light_lc_property_get},

    /* Send Light Lc Property Set */
    { "propertyset", "Send Light Lc Property Set", cli_modelc_light_lc_property_set},

    /* Send Light Lc Property Set Unacknowledged */
    { "propertysetun", "Send Light Lc Property Set Unacknowledged", cli_modelc_light_lc_property_set_unacknowledged}
};



/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_light_lc_client_model_handle;


/* --------------------------------------------- Function */
API_RESULT cli_modelc_light_lc(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT("In Model Client - Light_Lc\n");
    cli_cmd_stack_push((CLI_COMMAND*)cli_modelc_light_lc_cmd_list, sizeof(cli_modelc_light_lc_cmd_list) / sizeof(CLI_COMMAND));
    retval = cli_help(argc, argv);
    return retval;
}

/* light_lc client CLI entry point */
API_RESULT cli_modelc_light_lc_setup(UINT32 argc, UCHAR* argv[])
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
        retval = MS_light_lc_client_init
                 (
                     element_handle,
                     &appl_light_lc_client_model_handle,
                     cli_light_lc_client_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Light Lc Client Initialized. Model Handle: 0x%04X\n",
                appl_light_lc_client_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Light Lc Client Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    return retval;
}

/* Send Light Lc Light Onoff Get */
API_RESULT cli_modelc_light_lc_light_onoff_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Light Lc Light Onoff Get\n");
    retval = MS_light_lc_light_onoff_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Lc Light Onoff Set */
API_RESULT cli_modelc_light_lc_light_onoff_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_LC_LIGHT_ONOFF_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Lc Light Onoff Set\n");

    if (2 <= argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.light_onoff = (UCHAR)choice;
        CONSOLE_OUT("Light OnOff (8-bit in HEX): 0x%02X\n", param.light_onoff);
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

    retval = MS_light_lc_light_onoff_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Lc Light Onoff Set Unacknowledged */
API_RESULT cli_modelc_light_lc_light_onoff_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_LC_LIGHT_ONOFF_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Lc Light Onoff Set Unacknowledged\n");

    if (2 <= argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.light_onoff = (UCHAR)choice;
        CONSOLE_OUT("Light OnOff (8-bit in HEX): 0x%02X\n", param.light_onoff);
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

    retval = MS_light_lc_light_onoff_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Lc Mode Get */
API_RESULT cli_modelc_light_lc_mode_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Light Lc Mode Get\n");
    retval = MS_light_lc_mode_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Lc Mode Set */
API_RESULT cli_modelc_light_lc_mode_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_LC_MODE_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Lc Mode Set\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.mode = (UCHAR)choice;
        CONSOLE_OUT("Mode (8-bit in HEX): 0x%02X\n", param.mode);
    }

    retval = MS_light_lc_mode_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Lc Mode Set Unacknowledged */
API_RESULT cli_modelc_light_lc_mode_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_LC_MODE_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Lc Mode Set Unacknowledged\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.mode = (UCHAR)choice;
        CONSOLE_OUT("Mode (8-bit in HEX): 0x%02X\n", param.mode);
    }

    retval = MS_light_lc_mode_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Lc Om Get */
API_RESULT cli_modelc_light_lc_om_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Light Lc Om Get\n");
    retval = MS_light_lc_om_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Lc Om Set */
API_RESULT cli_modelc_light_lc_om_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_LC_OM_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Lc Om Set\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.mode = (UCHAR)choice;
        CONSOLE_OUT("Mode (8-bit in HEX): 0x%02X\n", param.mode);
    }

    retval = MS_light_lc_om_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Lc Om Set Unacknowledged */
API_RESULT cli_modelc_light_lc_om_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_LC_OM_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Lc Om Set Unacknowledged\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.mode = (UCHAR)choice;
        CONSOLE_OUT("Mode (8-bit in HEX): 0x%02X\n", param.mode);
    }

    retval = MS_light_lc_om_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Lc Property Get */
API_RESULT cli_modelc_light_lc_property_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_LC_PROPERTY_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Lc Property Get\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.light_lc_property_id = (UINT16)choice;
        CONSOLE_OUT("Light LC Property ID (16-bit in HEX): 0x%04X\n", param.light_lc_property_id);
    }

    retval = MS_light_lc_property_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}
/* Send Light Lc Property Set */
API_RESULT cli_modelc_light_lc_property_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_LC_PROPERTY_SET_STRUCT  param;
    retval = API_FAILURE;
    CONSOLE_OUT
    (">> Send Light Lc Property Set\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.light_lc_property_id = (UINT16)choice;
        CONSOLE_OUT("Light LC Property ID (16-bit in HEX): 0x%04X\n", param.light_lc_property_id);
        choice = CLI_strlen(argv[1]);
        {
            UINT16 input_len;
            input_len = (UINT16)choice;
            param.light_lc_property_value = EM_alloc_mem(input_len);

            if(NULL == param.light_lc_property_value)
            {
                CONSOLE_OUT
                ("Memory allocation failed for Light LC Property Value. Returning\n");
                return retval;
            }

            param.light_lc_property_value_len = (UINT16) input_len;
            CLI_strtoarray
            (
                argv[1],
                CLI_strlen(argv[1]),
                &param.light_lc_property_value[0],
                input_len
            );
        }
    }

    retval = MS_light_lc_property_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.light_lc_property_value)
    {
        EM_free_mem(param.light_lc_property_value);
    }

    return retval;
}


/* Send Light Lc Property Set Unacknowledged */
API_RESULT cli_modelc_light_lc_property_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_LC_PROPERTY_SET_STRUCT  param;
    retval = API_FAILURE;
    CONSOLE_OUT
    (">> Send Light Lc Property Set Unacknowledged\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.light_lc_property_id = (UINT16)choice;
        CONSOLE_OUT("Light LC Property ID (16-bit in HEX): 0x%04X\n", param.light_lc_property_id);
        choice = CLI_strlen(argv[1]);
        {
            UINT16 input_len;
            input_len = (UINT16)choice;
            param.light_lc_property_value = EM_alloc_mem(input_len);

            if(NULL == param.light_lc_property_value)
            {
                CONSOLE_OUT
                ("Memory allocation failed for Light LC Property Value. Returning\n");
                return retval;
            }

            param.light_lc_property_value_len = (UINT16) input_len;
            CLI_strtoarray
            (
                argv[1],
                CLI_strlen(argv[1]),
                &param.light_lc_property_value[0],
                input_len
            );
        }
    }

    retval = MS_light_lc_property_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.light_lc_property_value)
    {
        EM_free_mem(param.light_lc_property_value);
    }

    return retval;
}

/* Get Model Handle */
API_RESULT cli_modelc_light_lc_get_model_handle(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    #if 0
    MS_ACCESS_MODEL_HANDLE model_handle;
    retval = MS_light_lc_get_model_handle(&model_handle);

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
API_RESULT cli_modelc_light_lc_set_publish_address(UINT32 argc, UCHAR* argv[])
{
    int  choice;
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    MS_ACCESS_PUBLISH_INFO  publish_info;
    /* Set Publish Information */
    EM_mem_set(&publish_info, 0, sizeof(publish_info));
    publish_info.addr.use_label = MS_FALSE;
    model_handle = appl_light_lc_client_model_handle;
    CONSOLE_OUT("Model Handle (16-bit in HEX): 0x%04X\n", model_handle);

    /* Check Number of Arguments */
    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        publish_info.addr.addr = (UINT16)choice;
        CONSOLE_OUT("Light_Lc Server Address (16-bit in HEX): 0x%04X\n", publish_info.addr.addr);
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
    Light_Lc client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_light_lc_client_cb
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
        "[LIGHT_LC_CLIENT] Callback. Opcode 0x%04X\n", opcode);
    appl_dump_bytes(data_param, data_len);

    switch(opcode)
    {
    case MS_ACCESS_LIGHT_LC_LIGHT_ONOFF_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_LIGHT_LC_LIGHT_ONOFF_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_LIGHT_LC_MODE_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_LIGHT_LC_MODE_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_LIGHT_LC_OM_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_LIGHT_LC_OM_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_LIGHT_LC_PROPERTY_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_LIGHT_LC_PROPERTY_STATUS_OPCODE\n");
    }
    break;
    }

    return retval;
}

#endif /* CLI_LIGHTINGS_LC_CLIENT_MODEL */
