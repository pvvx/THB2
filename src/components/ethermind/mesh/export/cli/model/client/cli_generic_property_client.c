/**
    \file cli_generic_property_client.c

    \brief This file defines the Mesh Generic Property Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "cli_generic_property_client.h"

#ifdef CLI_GENERICS_PROPERTY_CLIENT_MODEL

/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
DECL_CONST CLI_COMMAND cli_modelc_generic_property_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root },

    /* Setup */
    { "setup", "Model Generic_Property Setup", cli_modelc_generic_property_setup },

    /* Get Model Handle */
    { "gethandle", "Get Model Handle", cli_modelc_generic_property_get_model_handle },

    /* Set Publish Address */
    { "publishaddr", "Set Publish Address  <Peer Server Address (HEX 16-bit)> <Appkey Index (HEX 16-bit)>", cli_modelc_generic_property_set_publish_address },

    /* Send Generic Admin Properties Get */
    { "adminpropertiesget", "Send Generic Admin Properties Get", cli_modelc_generic_admin_properties_get},

    /* Send Generic Admin Property Get */
    { "adminpropertyget", "Send Generic Admin Property Get", cli_modelc_generic_admin_property_get},

    /* Send Generic Admin Property Set */
    { "adminpropertyset", "Send Generic Admin Property Set", cli_modelc_generic_admin_property_set},

    /* Send Generic Admin Property Set Unacknowledged */
    { "adminpropertysetun", "Send Generic Admin Property Set Unacknowledged", cli_modelc_generic_admin_property_set_unacknowledged},

    /* Send Generic Client Properties Get */
    { "clientpropertiesget", "Send Generic Client Properties Get", cli_modelc_generic_client_properties_get},

    /* Send Generic Manufacturer Properties Get */
    { "manufacturerpropertiesget", "Send Generic Manufacturer Properties Get", cli_modelc_generic_manufacturer_properties_get},

    /* Send Generic Manufacturer Property Get */
    { "manufacturerpropertyget", "Send Generic Manufacturer Property Get", cli_modelc_generic_manufacturer_property_get},

    /* Send Generic Manufacturer Property Set */
    { "manufacturerpropertyset", "Send Generic Manufacturer Property Set", cli_modelc_generic_manufacturer_property_set},

    /* Send Generic Manufacturer Property Set Unacknowledged */
    { "manufacturerpropertysetun", "Send Generic Manufacturer Property Set Unacknowledged", cli_modelc_generic_manufacturer_property_set_unacknowledged},

    /* Send Generic User Properties Get */
    { "userpropertiesget", "Send Generic User Properties Get", cli_modelc_generic_user_properties_get},

    /* Send Generic User Property Get */
    { "userpropertyget", "Send Generic User Property Get", cli_modelc_generic_user_property_get},

    /* Send Generic User Property Set */
    { "userpropertyset", "Send Generic User Property Set", cli_modelc_generic_user_property_set},

    /* Send Generic User Property Set Unacknowledged */
    { "userpropertysetun", "Send Generic User Property Set Unacknowledged", cli_modelc_generic_user_property_set_unacknowledged}
};



/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_generic_property_client_model_handle;


/* --------------------------------------------- Function */
API_RESULT cli_modelc_generic_property(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT("In Model Client - Generic_Property\n");
    cli_cmd_stack_push((CLI_COMMAND*)cli_modelc_generic_property_cmd_list, sizeof(cli_modelc_generic_property_cmd_list) / sizeof(CLI_COMMAND));
    retval = cli_help(argc, argv);
    return retval;
}

/* generic_property client CLI entry point */
API_RESULT cli_modelc_generic_property_setup(UINT32 argc, UCHAR* argv[])
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
        retval = MS_generic_property_client_init
                 (
                     element_handle,
                     &appl_generic_property_client_model_handle,
                     cli_generic_property_client_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Generic Property Client Initialized. Model Handle: 0x%04X\n",
                appl_generic_property_client_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Generic Property Client Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    return retval;
}

/* Send Generic Admin Properties Get */
API_RESULT cli_modelc_generic_admin_properties_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Generic Admin Properties Get\n");
    retval = MS_generic_admin_properties_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Generic Admin Property Get */
API_RESULT cli_modelc_generic_admin_property_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_ADMIN_PROPERTY_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Admin Property Get\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.admin_property_id = (UINT16)choice;
        CONSOLE_OUT("Admin Property ID (16-bit in HEX): 0x%04X\n", param.admin_property_id);
    }

    retval = MS_generic_admin_property_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Generic Admin Property Set */
API_RESULT cli_modelc_generic_admin_property_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_ADMIN_PROPERTY_SET_STRUCT  param;
    retval = API_FAILURE;
    CONSOLE_OUT
    (">> Send Generic Admin Property Set\n");

    if (3 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.admin_property_id = (UINT16)choice;
        CONSOLE_OUT("Admin Property ID (16-bit in HEX): 0x%04X\n", param.admin_property_id);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.admin_user_access = (UCHAR)choice;
        CONSOLE_OUT("Admin User Access (8-bit in HEX): 0x%02X\n", param.admin_user_access);
        choice = CLI_strlen(argv[2]);
        {
            UINT16 input_len;
            input_len = (UINT16)(choice + 1)/2;
            param.admin_property_value = EM_alloc_mem(input_len);

            if(NULL == param.admin_property_value)
            {
                CONSOLE_OUT
                ("Memory allocation failed for Admin Property Value. Returning\n");
                return retval;
            }

            param.admin_property_value_len = (UINT16) input_len;
            CLI_strtoarray
            (
                argv[2],
                CLI_strlen(argv[2]),
                &param.admin_property_value[0],
                input_len
            );
        }
    }

    retval = MS_generic_admin_property_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.admin_property_value)
    {
        EM_free_mem(param.admin_property_value);
    }

    return retval;
}

/* Send Generic Admin Property Set Unacknowledged */
API_RESULT cli_modelc_generic_admin_property_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_ADMIN_PROPERTY_SET_STRUCT  param;
    retval = API_FAILURE;
    CONSOLE_OUT
    (">> Send Generic Admin Property Set Unacknowledged\n");

    if (3 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.admin_property_id = (UINT16)choice;
        CONSOLE_OUT("Admin Property ID (16-bit in HEX): 0x%04X\n", param.admin_property_id);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.admin_user_access = (UCHAR)choice;
        CONSOLE_OUT("Admin User Access (8-bit in HEX): 0x%02X\n", param.admin_user_access);
        choice = CLI_strlen(argv[2]);
        {
            UINT16 input_len;
            input_len = (UINT16)(choice + 1)/2;
            param.admin_property_value = EM_alloc_mem(input_len);

            if(NULL == param.admin_property_value)
            {
                CONSOLE_OUT
                ("Memory allocation failed for Admin Property Value. Returning\n");
                return retval;
            }

            param.admin_property_value_len = (UINT16) input_len;
            CLI_strtoarray
            (
                argv[2],
                CLI_strlen(argv[2]),
                &param.admin_property_value[0],
                input_len
            );
        }
    }

    retval = MS_generic_admin_property_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.admin_property_value)
    {
        EM_free_mem(param.admin_property_value);
    }

    return retval;
}

/* Send Generic Client Properties Get */
API_RESULT cli_modelc_generic_client_properties_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_CLIENT_PROPERTIES_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Client Properties Get\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.client_property_id = (UINT16)choice;
        CONSOLE_OUT("Client Property ID (16-bit in HEX): 0x%04X\n", param.client_property_id);
    }

    retval = MS_generic_client_properties_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Generic Manufacturer Properties Get */
API_RESULT cli_modelc_generic_manufacturer_properties_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Generic Manufacturer Properties Get\n");
    retval = MS_generic_manufacturer_properties_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Generic Manufacturer Property Get */
API_RESULT cli_modelc_generic_manufacturer_property_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_MANUFACTURER_PROPERTY_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Manufacturer Property Get\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.manufacturer_property_id = (UINT16)choice;
        CONSOLE_OUT("Manufacturer Property ID (16-bit in HEX): 0x%04X\n", param.manufacturer_property_id);
    }

    retval = MS_generic_manufacturer_property_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Generic Manufacturer Property Set */
API_RESULT cli_modelc_generic_manufacturer_property_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_MANUFACTURER_PROPERTY_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Manufacturer Property Set\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.manufacturer_property_id = (UINT16)choice;
        CONSOLE_OUT("Manufacturer Property ID (16-bit in HEX): 0x%04X\n", param.manufacturer_property_id);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.manufacturer_user_access = (UCHAR)choice;
        CONSOLE_OUT("Manufacturer User Access (8-bit in HEX): 0x%02X\n", param.manufacturer_user_access);
    }

    retval = MS_generic_manufacturer_property_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Generic Manufacturer Property Set Unacknowledged */
API_RESULT cli_modelc_generic_manufacturer_property_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_MANUFACTURER_PROPERTY_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic Manufacturer Property Set Unacknowledged\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.manufacturer_property_id = (UINT16)choice;
        CONSOLE_OUT("Manufacturer Property ID (16-bit in HEX): 0x%04X\n", param.manufacturer_property_id);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.manufacturer_user_access = (UCHAR)choice;
        CONSOLE_OUT("Manufacturer User Access (8-bit in HEX): 0x%02X\n", param.manufacturer_user_access);
    }

    retval = MS_generic_manufacturer_property_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Generic User Properties Get */
API_RESULT cli_modelc_generic_user_properties_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Generic User Properties Get\n");
    retval = MS_generic_user_properties_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Generic User Property Get */
API_RESULT cli_modelc_generic_user_property_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_USER_PROPERTY_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Generic User Property Get\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.user_property_id = (UINT16)choice;
        CONSOLE_OUT("User Property ID (16-bit in HEX): 0x%04X\n", param.user_property_id);
    }

    retval = MS_generic_user_property_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Generic User Property Set */
API_RESULT cli_modelc_generic_user_property_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_USER_PROPERTY_SET_STRUCT  param;
    retval = API_FAILURE;
    CONSOLE_OUT
    (">> Send Generic User Property Set\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.user_property_id = (UINT16)choice;
        CONSOLE_OUT("User Property ID (16-bit in HEX): 0x%04X\n", param.user_property_id);
        choice = CLI_strlen(argv[1]);
        {
            UINT16 input_len;
            input_len = (UINT16)(choice + 1)/2;
            param.user_property_value = EM_alloc_mem(input_len);

            if(NULL == param.user_property_value)
            {
                CONSOLE_OUT
                ("Memory allocation failed for User Property Value. Returning\n");
                return retval;
            }

            param.user_property_value_len = (UINT16) input_len;
            CLI_strtoarray
            (
                argv[1],
                CLI_strlen(argv[1]),
                &param.user_property_value[0],
                input_len
            );
        }
    }

    retval = MS_generic_user_property_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.user_property_value)
    {
        EM_free_mem(param.user_property_value);
    }

    return retval;
}


/* Send Generic User Property Set Unacknowledged */
API_RESULT cli_modelc_generic_user_property_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_GENERIC_USER_PROPERTY_SET_STRUCT  param;
    retval = API_FAILURE;
    CONSOLE_OUT
    (">> Send Generic User Property Set Unacknowledged\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.user_property_id = (UINT16)choice;
        CONSOLE_OUT("User Property ID (16-bit in HEX): 0x%04X\n", param.user_property_id);
        choice = CLI_strlen(argv[1]);
        {
            UINT16 input_len;
            input_len = (UINT16)(choice + 1)/2;
            param.user_property_value = EM_alloc_mem(input_len);

            if(NULL == param.user_property_value)
            {
                CONSOLE_OUT
                ("Memory allocation failed for User Property Value. Returning\n");
                return retval;
            }

            param.user_property_value_len = (UINT16) input_len;
            CLI_strtoarray
            (
                argv[1],
                CLI_strlen(argv[1]),
                &param.user_property_value[0],
                input_len
            );
        }
    }

    retval = MS_generic_user_property_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.user_property_value)
    {
        EM_free_mem(param.user_property_value);
    }

    return retval;
}

/* Get Model Handle */
API_RESULT cli_modelc_generic_property_get_model_handle(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    #if 0
    MS_ACCESS_MODEL_HANDLE model_handle;
    retval = MS_generic_property_get_model_handle(&model_handle);

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
API_RESULT cli_modelc_generic_property_set_publish_address(UINT32 argc, UCHAR* argv[])
{
    int  choice;
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    MS_ACCESS_PUBLISH_INFO  publish_info;
    /* Set Publish Information */
    EM_mem_set(&publish_info, 0, sizeof(publish_info));
    publish_info.addr.use_label = MS_FALSE;
    model_handle = appl_generic_property_client_model_handle;
    CONSOLE_OUT("Model Handle (16-bit in HEX): 0x%04X\n", model_handle);

    /* Check Number of Arguments */
    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        publish_info.addr.addr = (UINT16)choice;
        CONSOLE_OUT("Generic_Property Server Address (16-bit in HEX): 0x%04X\n", publish_info.addr.addr);
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
    Generic_Property client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_generic_property_client_cb
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
        "[GENERIC_PROPERTY_CLIENT] Callback. Opcode 0x%04X\n", opcode);
    appl_dump_bytes(data_param, data_len);

    switch(opcode)
    {
    case MS_ACCESS_GENERIC_ADMIN_PROPERTIES_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_GENERIC_ADMIN_PROPERTIES_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_GENERIC_ADMIN_PROPERTY_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_GENERIC_ADMIN_PROPERTY_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_GENERIC_CLIENT_PROPERTIES_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_GENERIC_CLIENT_PROPERTIES_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_GENERIC_MANUFACTURER_PROPERTIES_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_GENERIC_MANUFACTURER_PROPERTIES_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_GENERIC_MANUFACTURER_PROPERTY_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_GENERIC_MANUFACTURER_PROPERTY_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_GENERIC_USER_PROPERTIES_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_GENERIC_USER_PROPERTIES_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_GENERIC_USER_PROPERTY_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_GENERIC_USER_PROPERTY_STATUS_OPCODE\n");
    }
    break;
    }

    return retval;
}

#endif /* CLI_GENERICS_PROPERTY_CLIENT_MODEL */
