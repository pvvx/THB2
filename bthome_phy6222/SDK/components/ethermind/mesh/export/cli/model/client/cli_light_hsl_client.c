/**
    \file cli_light_hsl_client.c

    \brief This file defines the Mesh Light Hsl Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "cli_light_hsl_client.h"

#ifdef CLI_LIGHTINGS_HSL_CLIENT_MODEL

/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
DECL_CONST CLI_COMMAND cli_modelc_light_hsl_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root },

    /* Setup */
    { "setup", "Model Light_Hsl Setup", cli_modelc_light_hsl_setup },

    /* Get Model Handle */
    { "gethandle", "Get Model Handle", cli_modelc_light_hsl_get_model_handle },

    /* Set Publish Address */
    { "publishaddr", "Set Publish Address  <Peer Server Address (HEX 16-bit)> <Appkey Index (HEX 16-bit)>", cli_modelc_light_hsl_set_publish_address },

    /* Send Light Hsl Default Get */
    { "defaultget", "Send Light Hsl Default Get", cli_modelc_light_hsl_default_get},

    /* Send Light Hsl Default Set */
    { "defaultset", "Send Light Hsl Default Set", cli_modelc_light_hsl_default_set},

    /* Send Light Hsl Default Set Unacknowledged */
    { "defaultsetun", "Send Light Hsl Default Set Unacknowledged", cli_modelc_light_hsl_default_set_unacknowledged},

    /* Send Light Hsl Get */
    { "hslget", "Send Light Hsl Get", cli_modelc_light_hsl_get},

    /* Send Light Hsl Hue Get */
    { "hueget", "Send Light Hsl Hue Get", cli_modelc_light_hsl_hue_get},

    /* Send Light Hsl Hue Set */
    { "hueset", "Send Light Hsl Hue Set", cli_modelc_light_hsl_hue_set},

    /* Send Light Hsl Hue Set Unacknowledged */
    { "huesetun", "Send Light Hsl Hue Set Unacknowledged", cli_modelc_light_hsl_hue_set_unacknowledged},

    /* Send Light Hsl Range Get */
    { "rangeget", "Send Light Hsl Range Get", cli_modelc_light_hsl_range_get},

    /* Send Light Hsl Range Set */
    { "rangeset", "Send Light Hsl Range Set", cli_modelc_light_hsl_range_set},

    /* Send Light Hsl Range Set Unacknowledged */
    { "rangesetun", "Send Light Hsl Range Set Unacknowledged", cli_modelc_light_hsl_range_set_unacknowledged},

    /* Send Light Hsl Saturation Get */
    { "saturget", "Send Light Hsl Saturation Get", cli_modelc_light_hsl_saturation_get},

    /* Send Light Hsl Saturation Set */
    { "saturset", "Send Light Hsl Saturation Set", cli_modelc_light_hsl_saturation_set},

    /* Send Light Hsl Saturation Set Unacknowledged */
    { "satursetun", "Send Light Hsl Saturation Set Unacknowledged", cli_modelc_light_hsl_saturation_set_unacknowledged},

    /* Send Light Hsl Set */
    { "hslset", "Send Light Hsl Set", cli_modelc_light_hsl_set},

    /* Send Light Hsl Set Unacknowledged */
    { "hslsetun", "Send Light Hsl Set Unacknowledged", cli_modelc_light_hsl_set_unacknowledged},

    /* Send Light Hsl Target Get */
    { "targetget", "Send Light Hsl Target Get", cli_modelc_light_hsl_target_get}
};



/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_light_hsl_client_model_handle;


/* --------------------------------------------- Function */
API_RESULT cli_modelc_light_hsl(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT("In Model Client - Light_Hsl\n");
    cli_cmd_stack_push((CLI_COMMAND*)cli_modelc_light_hsl_cmd_list, sizeof(cli_modelc_light_hsl_cmd_list) / sizeof(CLI_COMMAND));
    retval = cli_help(argc, argv);
    return retval;
}

/* light_hsl client CLI entry point */
API_RESULT cli_modelc_light_hsl_setup(UINT32 argc, UCHAR* argv[])
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
        retval = MS_light_hsl_client_init
                 (
                     element_handle,
                     &appl_light_hsl_client_model_handle,
                     cli_light_hsl_client_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Light Hsl Client Initialized. Model Handle: 0x%04X\n",
                appl_light_hsl_client_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Light Hsl Client Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    return retval;
}

/* Send Light Hsl Default Get */
API_RESULT cli_modelc_light_hsl_default_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Light Hsl Default Get\n");
    retval = MS_light_hsl_default_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Hsl Default Set */
API_RESULT cli_modelc_light_hsl_default_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_HSL_DEFAULT_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Hsl Default Set\n");

    if (3 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.lightness = (UINT16)choice;
        CONSOLE_OUT("Lightness (16-bit in HEX): 0x%04X\n", param.lightness);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.hue = (UINT16)choice;
        CONSOLE_OUT("Hue (16-bit in HEX): 0x%04X\n", param.hue);
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.saturation = (UINT16)choice;
        CONSOLE_OUT("Saturation (16-bit in HEX): 0x%04X\n", param.saturation);
    }

    retval = MS_light_hsl_default_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Hsl Default Set Unacknowledged */
API_RESULT cli_modelc_light_hsl_default_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_HSL_DEFAULT_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Hsl Default Set Unacknowledged\n");

    if (3 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.lightness = (UINT16)choice;
        CONSOLE_OUT("Lightness (16-bit in HEX): 0x%04X\n", param.lightness);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.hue = (UINT16)choice;
        CONSOLE_OUT("Hue (16-bit in HEX): 0x%04X\n", param.hue);
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.saturation = (UINT16)choice;
        CONSOLE_OUT("Saturation (16-bit in HEX): 0x%04X\n", param.saturation);
    }

    retval = MS_light_hsl_default_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Hsl Get */
API_RESULT cli_modelc_light_hsl_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Light Hsl Get\n");
    retval = MS_light_hsl_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Hsl Hue Get */
API_RESULT cli_modelc_light_hsl_hue_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Light Hsl Hue Get\n");
    retval = MS_light_hsl_hue_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Hsl Hue Set */
API_RESULT cli_modelc_light_hsl_hue_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_HSL_HUE_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Hsl Hue Set\n");

    if (2 <= argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.hue = (UINT16)choice;
        CONSOLE_OUT("Hue (16-bit in HEX): 0x%04X\n", param.hue);
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

    retval = MS_light_hsl_hue_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Hsl Hue Set Unacknowledged */
API_RESULT cli_modelc_light_hsl_hue_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_HSL_HUE_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Hsl Hue Set Unacknowledged\n");

    if (2 <= argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.hue = (UINT16)choice;
        CONSOLE_OUT("Hue (16-bit in HEX): 0x%04X\n", param.hue);
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

    retval = MS_light_hsl_hue_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Hsl Range Get */
API_RESULT cli_modelc_light_hsl_range_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Light Hsl Range Get\n");
    retval = MS_light_hsl_range_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Hsl Range Set */
API_RESULT cli_modelc_light_hsl_range_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_HSL_RANGE_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Hsl Range Set\n");

    if (4 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.hue_range_min = (UINT16)choice;
        CONSOLE_OUT("Hue Range Min (16-bit in HEX): 0x%04X\n", param.hue_range_min);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.hue_range_max = (UINT16)choice;
        CONSOLE_OUT("Hue Range Max (16-bit in HEX): 0x%04X\n", param.hue_range_max);
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.saturation_range_min = (UINT16)choice;
        CONSOLE_OUT("Saturation Range Min (16-bit in HEX): 0x%04X\n", param.saturation_range_min);
        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.saturation_range_max = (UINT16)choice;
        CONSOLE_OUT("Saturation Range Max (16-bit in HEX): 0x%04X\n", param.saturation_range_max);
    }

    retval = MS_light_hsl_range_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Hsl Range Set Unacknowledged */
API_RESULT cli_modelc_light_hsl_range_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_HSL_RANGE_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Hsl Range Set Unacknowledged\n");

    if (4 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.hue_range_min = (UINT16)choice;
        CONSOLE_OUT("Hue Range Min (16-bit in HEX): 0x%04X\n", param.hue_range_min);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.hue_range_max = (UINT16)choice;
        CONSOLE_OUT("Hue Range Max (16-bit in HEX): 0x%04X\n", param.hue_range_max);
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.saturation_range_min = (UINT16)choice;
        CONSOLE_OUT("Saturation Range Min (16-bit in HEX): 0x%04X\n", param.saturation_range_min);
        choice = CLI_strtoi(argv[3], CLI_strlen(argv[3]), 16);
        param.saturation_range_max = (UINT16)choice;
        CONSOLE_OUT("Saturation Range Max (16-bit in HEX): 0x%04X\n", param.saturation_range_max);
    }

    retval = MS_light_hsl_range_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Hsl Saturation Get */
API_RESULT cli_modelc_light_hsl_saturation_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Light Hsl Saturation Get\n");
    retval = MS_light_hsl_saturation_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Hsl Saturation Set */
API_RESULT cli_modelc_light_hsl_saturation_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_HSL_SATURATION_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Hsl Saturation Set\n");

    if (2 <= argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.saturation = (UINT16)choice;
        CONSOLE_OUT("Saturation (16-bit in HEX): 0x%04X\n", param.saturation);
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

    retval = MS_light_hsl_saturation_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Hsl Saturation Set Unacknowledged */
API_RESULT cli_modelc_light_hsl_saturation_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_HSL_SATURATION_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Hsl Saturation Set Unacknowledged\n");

    if (2 <= argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.saturation = (UINT16)choice;
        CONSOLE_OUT("Saturation (16-bit in HEX): 0x%04X\n", param.saturation);
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

    retval = MS_light_hsl_saturation_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}


/* Send Light Hsl Set */
API_RESULT cli_modelc_light_hsl_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_HSL_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Hsl Set\n");

    if (4 <= argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.hsl_lightness = (UINT16)choice;
        CONSOLE_OUT("HSL Lightness (16-bit in HEX): 0x%04X\n", param.hsl_lightness);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.hsl_hue = (UINT16)choice;
        CONSOLE_OUT("HSL Hue (16-bit in HEX): 0x%04X\n", param.hsl_hue);
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.hsl_saturation = (UINT16)choice;
        CONSOLE_OUT("HSL Saturation (16-bit in HEX): 0x%04X\n", param.hsl_saturation);
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

    retval = MS_light_hsl_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Hsl Set Unacknowledged */
API_RESULT cli_modelc_light_hsl_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_LIGHT_HSL_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Light Hsl Set Unacknowledged\n");

    if (4 <= argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        param.hsl_lightness = (UINT16)choice;
        CONSOLE_OUT("HSL Lightness (16-bit in HEX): 0x%04X\n", param.hsl_lightness);
        choice = CLI_strtoi(argv[1], CLI_strlen(argv[1]), 16);
        param.hsl_hue = (UINT16)choice;
        CONSOLE_OUT("HSL Hue (16-bit in HEX): 0x%04X\n", param.hsl_hue);
        choice = CLI_strtoi(argv[2], CLI_strlen(argv[2]), 16);
        param.hsl_saturation = (UINT16)choice;
        CONSOLE_OUT("HSL Saturation (16-bit in HEX): 0x%04X\n", param.hsl_saturation);
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

    retval = MS_light_hsl_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Light Hsl Target Get */
API_RESULT cli_modelc_light_hsl_target_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT
    (">> Send Light Hsl Target Get\n");
    retval = MS_light_hsl_target_get();
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Get Model Handle */
API_RESULT cli_modelc_light_hsl_get_model_handle(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    #if 0
    MS_ACCESS_MODEL_HANDLE model_handle;
    retval = MS_light_hsl_get_model_handle(&model_handle);

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
API_RESULT cli_modelc_light_hsl_set_publish_address(UINT32 argc, UCHAR* argv[])
{
    int  choice;
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    MS_ACCESS_PUBLISH_INFO  publish_info;
    /* Set Publish Information */
    EM_mem_set(&publish_info, 0, sizeof(publish_info));
    publish_info.addr.use_label = MS_FALSE;
    model_handle = appl_light_hsl_client_model_handle;
    CONSOLE_OUT("Model Handle (16-bit in HEX): 0x%04X\n", model_handle);

    /* Check Number of Arguments */
    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], CLI_strlen(argv[0]), 16);
        publish_info.addr.addr = (UINT16)choice;
        CONSOLE_OUT("Light_Hsl Server Address (16-bit in HEX): 0x%04X\n", publish_info.addr.addr);
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
    Light_Hsl client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_light_hsl_client_cb
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
        "[LIGHT_HSL_CLIENT] Callback. Opcode 0x%04X\n", opcode);
    appl_dump_bytes(data_param, data_len);

    switch(opcode)
    {
    case MS_ACCESS_LIGHT_HSL_DEFAULT_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_LIGHT_HSL_DEFAULT_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_LIGHT_HSL_HUE_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_LIGHT_HSL_HUE_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_LIGHT_HSL_RANGE_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_LIGHT_HSL_RANGE_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_LIGHT_HSL_SATURATION_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_LIGHT_HSL_SATURATION_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_LIGHT_HSL_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_LIGHT_HSL_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_LIGHT_HSL_TARGET_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_LIGHT_HSL_TARGET_STATUS_OPCODE\n");
    }
    break;
    }

    return retval;
}

#endif /* CLI_LIGHTINGS_HSL_CLIENT_MODEL */
