/**
    \file cli_health_client.c

    \brief This file defines the Mesh Health Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "cli_health_client.h"

#ifdef CLI_HEALTH_CLIENT_MODEL

/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
DECL_CONST CLI_COMMAND cli_modelc_health_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root },

    /* Setup */
    { "setup", "Model Health Setup", cli_modelc_health_setup },

    /* Get Model Handle */
    { "gethandle", "Get Model Handle", cli_modelc_health_get_model_handle },

    /* Set Publish Address */
    { "publishaddr", "Set Publish Address  <Peer Server Address (HEX 16-bit)> <Appkey Index (HEX 16-bit)>", cli_modelc_health_set_publish_address },

    /* Send Health Attention Get */
    { "attentionget", "Send Health Attention Get", cli_modelc_health_attention_get},

    /* Send Health Attention Set */
    { "attentionset", "Send Health Attention Set", cli_modelc_health_attention_set},

    /* Send Health Attention Set Unacknowledged */
    { "attentionsetun", "Send Health Attention Set Unacknowledged", cli_modelc_health_attention_set_unacknowledged},

    /* Send Health Fault Clear */
    { "faultclear", "Send Health Fault Clear", cli_modelc_health_fault_clear},

    /* Send Health Fault Clear Unacknowledged */
    { "faultclearun", "Send Health Fault Clear Unacknowledged", cli_modelc_health_fault_clear_unacknowledged},

    /* Send Health Fault Get */
    { "faultget", "Send Health Fault Get", cli_modelc_health_fault_get},

    /* Send Health Fault Test */
    { "faulttest", "Send Health Fault Test", cli_modelc_health_fault_test},

    /* Send Health Fault Test Unacknowledged */
    { "faulttestun", "Send Health Fault Test Unacknowledged", cli_modelc_health_fault_test_unacknowledged},

    /* Send Health Period Get */
    { "periodget", "Send Health Period Get", cli_modelc_health_period_get},

    /* Send Health Period Set */
    { "periodset", "Send Health Period Set", cli_modelc_health_period_set},

    /* Send Health Period Set Unacknowledged */
    { "periodsetun", "Send Health Period Set Unacknowledged", cli_modelc_health_period_set_unacknowledged}
};



/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_health_client_model_handle;


/* --------------------------------------------- Function */
API_RESULT cli_modelc_health(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT("In Model Client - Health\n");
    cli_cmd_stack_push((CLI_COMMAND*)cli_modelc_health_cmd_list, sizeof(cli_modelc_health_cmd_list) / sizeof(CLI_COMMAND));
    retval = cli_help(argc, argv);
    return retval;
}

/* health client CLI entry point */
API_RESULT cli_modelc_health_setup(UINT32 argc, UCHAR* argv[])
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
        retval = MS_health_client_init
                 (
                     element_handle,
                     &appl_health_client_model_handle,
                     cli_health_client_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Health Client Initialized. Model Handle: 0x%04X\n",
                appl_health_client_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Health Client Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    return retval;
}

/* Send Health Attention Get */
API_RESULT cli_modelc_health_attention_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Health Attention Get\n");
    retval = MS_health_attention_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Health Attention Set */
API_RESULT cli_modelc_health_attention_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_HEALTH_ATTENTION_STRUCT  param;
    CONSOLE_OUT
    (">> Send Health Attention Set\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.attention = (UCHAR)choice;
        CONSOLE_OUT("Attention (8-bit in HEX): 0x%02X\n", param.attention);
    }

    retval = MS_health_attention_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Health Attention Set Unacknowledged */
API_RESULT cli_modelc_health_attention_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_HEALTH_ATTENTION_STRUCT  param;
    CONSOLE_OUT
    (">> Send Health Attention Set Unacknowledged\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.attention = (UCHAR)choice;
        CONSOLE_OUT("Attention (8-bit in HEX): 0x%02X\n", param.attention);
    }

    retval = MS_health_attention_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Health Fault Clear */
API_RESULT cli_modelc_health_fault_clear(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_HEALTH_FAULT_GET_CLEAR_STRUCT  param;
    CONSOLE_OUT
    (">> Send Health Fault Clear\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.company_id = (UINT16)choice;
        CONSOLE_OUT("Company ID (16-bit in HEX): 0x%04X\n", param.company_id);
    }

    retval = MS_health_fault_clear(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Health Fault Clear Unacknowledged */
API_RESULT cli_modelc_health_fault_clear_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_HEALTH_FAULT_GET_CLEAR_STRUCT  param;
    CONSOLE_OUT
    (">> Send Health Fault Clear Unacknowledged\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.company_id = (UINT16)choice;
        CONSOLE_OUT("Company ID (16-bit in HEX): 0x%04X\n", param.company_id);
    }

    retval = MS_health_fault_clear_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Health Fault Get */
API_RESULT cli_modelc_health_fault_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_HEALTH_FAULT_GET_CLEAR_STRUCT  param;
    CONSOLE_OUT
    (">> Send Health Fault Get\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.company_id = (UINT16)choice;
        CONSOLE_OUT("Company ID (16-bit in HEX): 0x%04X\n", param.company_id);
    }

    retval = MS_health_fault_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Health Fault Test */
API_RESULT cli_modelc_health_fault_test(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_HEALTH_FAULT_TEST_STRUCT  param;
    CONSOLE_OUT
    (">> Send Health Fault Test\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.test_id = (UCHAR)choice;
        CONSOLE_OUT("Test ID (8-bit in HEX): 0x%02X\n", param.test_id);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.company_id = (UINT16)choice;
        CONSOLE_OUT("Company ID (16-bit in HEX): 0x%04X\n", param.company_id);
    }

    retval = MS_health_fault_test(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Health Fault Test Unacknowledged */
API_RESULT cli_modelc_health_fault_test_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_HEALTH_FAULT_TEST_STRUCT  param;
    CONSOLE_OUT
    (">> Send Health Fault Test Unacknowledged\n");

    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.test_id = (UCHAR)choice;
        CONSOLE_OUT("Test ID (8-bit in HEX): 0x%02X\n", param.test_id);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.company_id = (UINT16)choice;
        CONSOLE_OUT("Company ID (16-bit in HEX): 0x%04X\n", param.company_id);
    }

    retval = MS_health_fault_test_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Health Period Get */
API_RESULT cli_modelc_health_period_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Health Period Get\n");
    retval = MS_health_period_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Health Period Set */
API_RESULT cli_modelc_health_period_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_HEALTH_PERIOD_STRUCT  param;
    CONSOLE_OUT
    (">> Send Health Period Set\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.fastperioddivisor = (UCHAR)choice;
        CONSOLE_OUT("FastPeriodDivisor (8-bit in HEX): 0x%02X\n", param.fastperioddivisor);
    }

    retval = MS_health_period_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Health Period Set Unacknowledged */
API_RESULT cli_modelc_health_period_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_HEALTH_PERIOD_STRUCT  param;
    CONSOLE_OUT
    (">> Send Health Period Set Unacknowledged\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.fastperioddivisor = (UCHAR)choice;
        CONSOLE_OUT("FastPeriodDivisor (8-bit in HEX): 0x%02X\n", param.fastperioddivisor);
    }

    retval = MS_health_period_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Get Model Handle */
API_RESULT cli_modelc_health_get_model_handle(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    #if 0
    MS_ACCESS_MODEL_HANDLE model_handle;
    retval = MS_health_get_model_handle(&model_handle);

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
API_RESULT cli_modelc_health_set_publish_address(UINT32 argc, UCHAR* argv[])
{
    int  choice;
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    MS_ACCESS_PUBLISH_INFO  publish_info;
    /* Set Publish Information */
    EM_mem_set(&publish_info, 0, sizeof(publish_info));
    publish_info.addr.use_label = MS_FALSE;
    model_handle = appl_health_client_model_handle;
    CONSOLE_OUT("Model Handle (16-bit in HEX): 0x%04X\n", model_handle);

    /* Check Number of Arguments */
    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        publish_info.addr.addr = (UINT16)choice;
        CONSOLE_OUT("Health Server Address (16-bit in HEX): 0x%04X\n", publish_info.addr.addr);
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
    Health client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_health_client_cb
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
        "[HEALTH_CLIENT] Callback. Opcode 0x%04X\n", opcode);
    appl_dump_bytes(data_param, data_len);

    switch(opcode)
    {
    case MS_ACCESS_HEALTH_ATTENTION_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_HEALTH_ATTENTION_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_HEALTH_CURRENT_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_HEALTH_CURRENT_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_HEALTH_FAULT_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_HEALTH_FAULT_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_HEALTH_PERIOD_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_HEALTH_PERIOD_STATUS_OPCODE\n");
    }
    break;
    }

    return retval;
}

#endif /* CLI_HEALTH_CLIENT_MODEL */
