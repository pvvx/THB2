/**
    \file cli_sensor_client.c

    \brief This file defines the Mesh Sensor Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "cli_sensor_client.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
DECL_CONST CLI_COMMAND cli_modelc_sensor_cmd_list[] =
{
    /* Help */
    { "help", "Help", cli_help },

    /* Back */
    { "back", "One Level Up", cli_back },

    /* Root */
    { "root", "Back to Root", cli_root },

    /* Setup */
    { "setup", "Model Sensor Setup", cli_modelc_sensor_setup },

    /* Get Model Handle */
    { "gethandle", "Get Model Handle", cli_modelc_sensor_get_model_handle },

    /* Set Publish Address */
    { "publishaddr", "Set Publish Address  <Peer Server Address (HEX 16-bit)> <Appkey Index (HEX 16-bit)>", cli_modelc_sensor_set_publish_address },

    /* Send Sensor Cadence Get */
    { "cadenceget", "Send Sensor Cadence Get", cli_modelc_sensor_cadence_get},

    /* Send Sensor Cadence Set */
    { "cadenceset", "Send Sensor Cadence Set", cli_modelc_sensor_cadence_set},

    /* Send Sensor Cadence Set Unacknowledged */
    { "cadencesetun", "Send Sensor Cadence Set Unacknowledged", cli_modelc_sensor_cadence_set_unacknowledged},

    /* Send Sensor Column Get */
    { "columnget", "Send Sensor Column Get", cli_modelc_sensor_column_get},

    /* Send Sensor Descriptor Get */
    { "descriptorget", "Send Sensor Descriptor Get", cli_modelc_sensor_descriptor_get},

    /* Send Sensor Get */
    { "get", "Send Sensor Get", cli_modelc_sensor_get},

    /* Send Sensor Series Get */
    { "seriesget", "Send Sensor Series Get", cli_modelc_sensor_series_get},

    /* Send Sensor Setting Get */
    { "settingget", "Send Sensor Setting Get", cli_modelc_sensor_setting_get},

    /* Send Sensor Setting Set */
    { "settingset", "Send Sensor Setting Set", cli_modelc_sensor_setting_set},

    /* Send Sensor Setting Set Unacknowledged */
    { "settingsetun", "Send Sensor Setting Set Unacknowledged", cli_modelc_sensor_setting_set_unacknowledged},

    /* Send Sensor Settings Get */
    { "settingsget", "Send Sensor Settings Get", cli_modelc_sensor_settings_get}
};



/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_sensor_client_model_handle;


/* --------------------------------------------- Function */
API_RESULT cli_modelc_sensor(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    CONSOLE_OUT("In Model Client - Sensor\n");
    cli_cmd_stack_push((CLI_COMMAND*)cli_modelc_sensor_cmd_list, sizeof(cli_modelc_sensor_cmd_list) / sizeof(CLI_COMMAND));
    retval = cli_help(argc, argv);
    return retval;
}

/* sensor client CLI entry point */
API_RESULT cli_modelc_sensor_setup(UINT32 argc, UCHAR* argv[])
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
        retval = MS_sensor_client_init
                 (
                     element_handle,
                     &appl_sensor_client_model_handle,
                     cli_sensor_client_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Sensor Client Initialized. Model Handle: 0x%04X\n",
                appl_sensor_client_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Sensor Client Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    return retval;
}

/* Send Sensor Cadence Get */
API_RESULT cli_modelc_sensor_cadence_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_SENSOR_CADENCE_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Sensor Cadence Get\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], (UINT16)CLI_strlen(argv[0]), 16);
        param.property_id = (UINT16)choice;
        CONSOLE_OUT("Property ID (16-bit in HEX): 0x%04X\n", param.property_id);
    }

    retval = MS_sensor_cadence_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Sensor Cadence Set */
API_RESULT cli_modelc_sensor_cadence_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    INT32 choice;
    UINT16 argv_index;
    UINT16 input_len;
    MS_SENSOR_CADENCE_SET_STRUCT  param;
    argv_index = 0;
    CONSOLE_OUT
    (">> Send Sensor Cadence Set\n");

    if (8 == argc)
    {
        CONSOLE_OUT
        ("Property ID (16-bit in HEX)\n");
        choice = CLI_strtoi(argv[argv_index], (UINT16)CLI_strlen(argv[argv_index]), 16);
        argv_index++;
        param.property_id = (UINT16)choice;
        CONSOLE_OUT
        ("Fast Cadence Period Divisor (7-bit in HEX)\n");
        choice = CLI_strtoi(argv[argv_index], (UINT16)CLI_strlen(argv[argv_index]), 16);
        argv_index++;
        param.fast_cadence_period_divisor = (UCHAR)choice;
        CONSOLE_OUT
        ("Status Trigger Type (1-bit in HEX)\n");
        choice = CLI_strtoi(argv[argv_index], (UINT16)CLI_strlen(argv[argv_index]), 16);
        argv_index++;
        param.status_trigger_type = (UCHAR)choice;
        choice = CLI_strlen(argv[argv_index]);
        choice = (choice + 1) / 2;
        CONSOLE_OUT
        ("Status Trigger Delta Down (%d-octets in HEX)\n", (UINT16)choice);
        input_len = (UINT16)choice;
        param.status_trigger_delta_down = EM_alloc_mem(input_len);

        if (NULL == param.status_trigger_delta_down)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Status Trigger Delta Down. Returning\n");
            return API_FAILURE;
        }

        param.status_trigger_delta_down_len = (UINT16)input_len;
        CLI_strtoarray
        (
            argv[argv_index],
            (UINT16)CLI_strlen(argv[argv_index]),
            param.status_trigger_delta_down,
            input_len
        );
        argv_index++;
        choice = CLI_strlen(argv[argv_index]);
        choice = (choice + 1) / 2;
        CONSOLE_OUT
        ("Status Trigger Delta Up (%d-octets in HEX)\n", (UINT16)choice);
        input_len = (UINT16)choice;
        param.status_trigger_delta_up = EM_alloc_mem(input_len);

        if (NULL == param.status_trigger_delta_up)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Status Trigger Delta Up. Returning\n");
            return API_FAILURE;
        }

        param.status_trigger_delta_up_len = (UINT16)input_len;
        CLI_strtoarray
        (
            argv[argv_index],
            (UINT16)CLI_strlen(argv[argv_index]),
            param.status_trigger_delta_up,
            input_len
        );
        argv_index++;
        CONSOLE_OUT
        ("Status Min Interval (8-bit in HEX)\n");
        choice = CLI_strtoi(argv[argv_index], (UINT16)CLI_strlen(argv[argv_index]), 16);
        argv_index++;
        param.status_min_interval = (UCHAR)choice;
        choice = CLI_strlen(argv[argv_index]);
        choice = (choice + 1) / 2;
        CONSOLE_OUT
        ("Fast Cadence Low (%d-octets in HEX)\n", (UINT16)choice);
        input_len = (UINT16)choice;
        param.fast_cadence_low = EM_alloc_mem(input_len);

        if (NULL == param.fast_cadence_low)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Fast Cadence Low. Returning\n");
            return API_FAILURE;
        }

        param.fast_cadence_low_len = (UINT16)input_len;
        CLI_strtoarray
        (
            argv[argv_index],
            (UINT16)CLI_strlen(argv[argv_index]),
            param.fast_cadence_low,
            input_len
        );
        argv_index++;
        choice = CLI_strlen(argv[argv_index]);
        choice = (choice + 1) / 2;
        CONSOLE_OUT
        ("Fast Cadence High (%d-octets in HEX)\n", (UINT16)choice);
        input_len = (UINT16)choice;
        param.fast_cadence_high = EM_alloc_mem(input_len);

        if (NULL == param.fast_cadence_high)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Fast Cadence High. Returning\n");
            return API_FAILURE;
        }

        param.fast_cadence_high_len = (UINT16)input_len;
        CLI_strtoarray
        (
            argv[argv_index],
            (UINT16)CLI_strlen(argv[argv_index]),
            param.fast_cadence_high,
            input_len
        );
        argv_index++;
    }

    retval = MS_sensor_cadence_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.fast_cadence_high)
    {
        EM_free_mem(param.fast_cadence_high);
    }

    if(NULL != param.fast_cadence_low)
    {
        EM_free_mem(param.fast_cadence_low);
    }

    if(NULL != param.status_trigger_delta_up)
    {
        EM_free_mem(param.status_trigger_delta_up);
    }

    if(NULL != param.status_trigger_delta_down)
    {
        EM_free_mem(param.status_trigger_delta_down);
    }

    return retval;
}

/* Send Sensor Cadence Set Unacknowledged */
API_RESULT cli_modelc_sensor_cadence_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    UINT16 argv_index;
    UINT16 input_len;
    MS_SENSOR_CADENCE_SET_STRUCT  param;
    argv_index = 0;
    CONSOLE_OUT
    (">> Send Sensor Cadence Set Unacknowledged\n");

    if (8 == argc)
    {
        CONSOLE_OUT
        ("Property ID (16-bit in HEX)\n");
        choice = CLI_strtoi(argv[argv_index], (UINT16)CLI_strlen(argv[argv_index]), 16);
        argv_index++;
        param.property_id = (UINT16)choice;
        CONSOLE_OUT
        ("Fast Cadence Period Divisor (7-bit in HEX)\n");
        choice = CLI_strtoi(argv[argv_index], (UINT16)CLI_strlen(argv[argv_index]), 16);
        argv_index++;
        param.fast_cadence_period_divisor = (UCHAR)choice;
        CONSOLE_OUT
        ("Status Trigger Type (1-bit in HEX)\n");
        choice = CLI_strtoi(argv[argv_index], (UINT16)CLI_strlen(argv[argv_index]), 16);
        argv_index++;
        param.status_trigger_type = (UCHAR)choice;
        choice = CLI_strlen(argv[argv_index]);
        choice = (choice + 1) / 2;
        CONSOLE_OUT
        ("Status Trigger Delta Down (%d-octets in HEX)\n", (UINT16)choice);
        input_len = (UINT16)choice;
        param.status_trigger_delta_down = EM_alloc_mem(input_len);

        if (NULL == param.status_trigger_delta_down)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Status Trigger Delta Down. Returning\n");
            return API_FAILURE;
        }

        param.status_trigger_delta_down_len = (UINT16)input_len;
        CLI_strtoarray
        (
            argv[argv_index],
            (UINT16)CLI_strlen(argv[argv_index]),
            param.status_trigger_delta_down,
            input_len
        );
        argv_index++;
        choice = CLI_strlen(argv[argv_index]);
        choice = (choice + 1) / 2;
        CONSOLE_OUT
        ("Status Trigger Delta Up (%d-octets in HEX)\n", (UINT16)choice);
        input_len = (UINT16)choice;
        param.status_trigger_delta_up = EM_alloc_mem(input_len);

        if (NULL == param.status_trigger_delta_up)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Status Trigger Delta Up. Returning\n");
            return API_FAILURE;
        }

        param.status_trigger_delta_up_len = (UINT16)input_len;
        CLI_strtoarray
        (
            argv[argv_index],
            (UINT16)CLI_strlen(argv[argv_index]),
            param.status_trigger_delta_up,
            input_len
        );
        argv_index++;
        CONSOLE_OUT
        ("Status Min Interval (8-bit in HEX)\n");
        choice = CLI_strtoi(argv[argv_index], (UINT16)CLI_strlen(argv[argv_index]), 16);
        argv_index++;
        param.status_min_interval = (UCHAR)choice;
        choice = CLI_strlen(argv[argv_index]);
        choice = (choice + 1) / 2;
        CONSOLE_OUT
        ("Fast Cadence Low (%d-octets in HEX)\n", (UINT16)choice);
        input_len = (UINT16)choice;
        param.fast_cadence_low = EM_alloc_mem(input_len);

        if (NULL == param.fast_cadence_low)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Fast Cadence Low. Returning\n");
            return API_FAILURE;
        }

        param.fast_cadence_low_len = (UINT16)input_len;
        CLI_strtoarray
        (
            argv[argv_index],
            (UINT16)CLI_strlen(argv[argv_index]),
            param.fast_cadence_low,
            input_len
        );
        argv_index++;
        choice = CLI_strlen(argv[argv_index]);
        choice = (choice + 1) / 2;
        CONSOLE_OUT
        ("Fast Cadence High (%d-octets in HEX)\n", (UINT16)choice);
        input_len = (UINT16)choice;
        param.fast_cadence_high = EM_alloc_mem(input_len);

        if (NULL == param.fast_cadence_high)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Fast Cadence High. Returning\n");
            return API_FAILURE;
        }

        param.fast_cadence_high_len = (UINT16)input_len;
        CLI_strtoarray
        (
            argv[argv_index],
            (UINT16)CLI_strlen(argv[argv_index]),
            param.fast_cadence_high,
            input_len
        );
        argv_index++;
    }

    retval = MS_sensor_cadence_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.fast_cadence_high)
    {
        EM_free_mem(param.fast_cadence_high);
    }

    if(NULL != param.fast_cadence_low)
    {
        EM_free_mem(param.fast_cadence_low);
    }

    if(NULL != param.status_trigger_delta_up)
    {
        EM_free_mem(param.status_trigger_delta_up);
    }

    if(NULL != param.status_trigger_delta_down)
    {
        EM_free_mem(param.status_trigger_delta_down);
    }

    return retval;
}

/* Send Sensor Column Get */
API_RESULT cli_modelc_sensor_column_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    UINT16 argv_index;
    UINT16 input_len;
    MS_SENSOR_COLUMN_GET_STRUCT  param;
    argv_index = 0;
    CONSOLE_OUT
    (">> Send Sensor Column Get\n");

    if (2 == argc)
    {
        CONSOLE_OUT
        ("Property ID (16-bit in HEX)\n");
        choice = CLI_strtoi(argv[argv_index], (UINT16)CLI_strlen(argv[argv_index]), 16);
        argv_index++;
        param.property_id = (UINT16)choice;
        choice = CLI_strlen(argv[argv_index]);
        choice = (choice + 1) / 2;
        CONSOLE_OUT
        ("Raw Value X (%d-octets in HEX)\n", (UINT16)choice);
        input_len = (UINT16)choice;
        param.raw_value_x = EM_alloc_mem(input_len);

        if (NULL == param.raw_value_x)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Raw Value X. Returning\n");
            return API_FAILURE;
        }

        param.raw_value_x_len = (UINT16)input_len;
        CLI_strtoarray
        (
            argv[argv_index],
            (UINT16)CLI_strlen(argv[argv_index]),
            param.raw_value_x,
            input_len
        );
        argv_index++;
    }

    retval = MS_sensor_column_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.raw_value_x)
    {
        EM_free_mem(param.raw_value_x);
    }

    return retval;
}

/* Send Sensor Descriptor Get */
API_RESULT cli_modelc_sensor_descriptor_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_SENSOR_DESCRIPTOR_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Sensor Descriptor Get\n");

    /* Check Number of Arguments */
    if (1 == argc)
    {
        param.optional_fields_present = 0x01;
        choice = CLI_strtoi(argv[0], (UINT16) CLI_strlen(argv[0]), 16);
        param.property_id = (UINT16)choice;
        CONSOLE_OUT("Property ID (16-bit in HEX): 0x%04X\n", param.property_id);
    }
    else
    {
        param.optional_fields_present = 0x00;
    }

    retval = MS_sensor_descriptor_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Sensor Get */
API_RESULT cli_modelc_sensor_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_SENSOR_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Sensor Get\n");

    if (1 == argc)
    {
        param.optional_fields_present = 0x01;
        CONSOLE_OUT
        ("Property ID (16-bit in HEX)\n");
        choice = CLI_strtoi(argv[0], (UINT16)CLI_strlen(argv[0]), 16);
        param.property_id = (UINT16)choice;
    }
    else
    {
        param.optional_fields_present = 0x00;
    }

    retval = MS_sensor_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Sensor Series Get */
API_RESULT cli_modelc_sensor_series_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    UINT16 argv_index;
    UINT16 input_len;
    MS_SENSOR_SERIES_GET_STRUCT  param;
    argv_index = 0;
    CONSOLE_OUT
    (">> Send Sensor Series Get\n");
    CONSOLE_OUT
    ("Property ID (16-bit in HEX)\n");
    choice = CLI_strtoi(argv[argv_index], (UINT16)CLI_strlen(argv[argv_index]), 16);
    argv_index++;
    param.property_id = (UINT16)choice;

    if(1 == argc)
    {
        param.optional_fields_present = 0x00;
    }
    else
    {
        param.optional_fields_present = 0x01;
        choice = CLI_strlen(argv[argv_index]);
        choice = (choice + 1) / 2;
        CONSOLE_OUT
        ("Raw Value X1 (%d-octets in HEX)\n", (UINT16)choice);
        input_len = (UINT16)choice;
        param.raw_value_x1 = EM_alloc_mem(input_len);

        if(NULL == param.raw_value_x1)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Raw Value X1. Returning\n");
            return API_FAILURE;
        }

        param.raw_value_x1_len = (UINT16) input_len;
        CLI_strtoarray
        (
            argv[argv_index],
            (UINT16)CLI_strlen(argv[argv_index]),
            param.raw_value_x1,
            input_len
        );
        argv_index++;
        choice = CLI_strlen(argv[argv_index]);
        choice = (choice + 1) / 2;
        CONSOLE_OUT
        ("Raw Value X2 (%d-octets in HEX)\n", (UINT16)choice);
        input_len = (UINT16)choice;
        param.raw_value_x2 = EM_alloc_mem(input_len);

        if(NULL == param.raw_value_x2)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Raw Value X2. Returning\n");
            return API_FAILURE;
        }

        param.raw_value_x2_len = (UINT16) input_len;
        CLI_strtoarray
        (
            argv[argv_index],
            (UINT16)CLI_strlen(argv[argv_index]),
            param.raw_value_x2,
            input_len
        );
        argv_index++;
    }

    retval = MS_sensor_series_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.raw_value_x1)
    {
        EM_free_mem(param.raw_value_x1);
    }

    if(NULL != param.raw_value_x2)
    {
        EM_free_mem(param.raw_value_x2);
    }

    return retval;
}

/* Send Sensor Setting Get */
API_RESULT cli_modelc_sensor_setting_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_SENSOR_SETTING_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Sensor Setting Get\n");

    if (2 == argc)
    {
        CONSOLE_OUT
        ("Sensor Property ID (16-bit in HEX)\n");
        choice = CLI_strtoi(argv[0], (UINT16)CLI_strlen(argv[0]), 16);
        param.sensor_property_id = (UINT16)choice;
        CONSOLE_OUT
        ("Sensor Setting Property ID (16-bit in HEX)\n");
        choice = CLI_strtoi(argv[1], (UINT16)CLI_strlen(argv[1]), 16);
        param.sensor_setting_property_id = (UINT16)choice;
    }

    retval = MS_sensor_setting_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Send Sensor Setting Set */
API_RESULT cli_modelc_sensor_setting_set(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    UINT16 argv_index;
    UINT16 input_len;
    MS_SENSOR_SETTING_SET_STRUCT  param;
    argv_index = 0;
    CONSOLE_OUT
    (">> Send Sensor Setting Set\n");

    if (3 == argc)
    {
        CONSOLE_OUT
        ("Sensor Property ID (16-bit in HEX)\n");
        choice = CLI_strtoi(argv[argv_index], (UINT16)CLI_strlen(argv[argv_index]), 16);
        argv_index++;
        param.sensor_property_id = (UINT16)choice;
        CONSOLE_OUT
        ("Sensor Setting Property ID (16-bit in HEX)\n");
        choice = CLI_strtoi(argv[argv_index], (UINT16)CLI_strlen(argv[argv_index]), 16);
        argv_index++;
        param.sensor_setting_property_id = (UINT16)choice;
        choice = CLI_strlen(argv[argv_index]);
        choice = (choice + 1) / 2;
        CONSOLE_OUT
        ("Sensor Setting Raw (%d-octets in HEX)\n", (UINT16)choice);
        input_len = (UINT16)choice;
        param.sensor_setting_raw = EM_alloc_mem(input_len);

        if (NULL == param.sensor_setting_raw)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Sensor Setting Raw. Returning\n");
            return API_FAILURE;
        }

        param.sensor_setting_raw_len = (UINT16)input_len;
        CLI_strtoarray
        (
            argv[argv_index],
            (UINT16)CLI_strlen(argv[argv_index]),
            param.sensor_setting_raw,
            input_len
        );
        argv_index++;
    }

    retval = MS_sensor_setting_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.sensor_setting_raw)
    {
        EM_free_mem(param.sensor_setting_raw);
    }

    return retval;
}

/* Send Sensor Setting Set Unacknowledged */
API_RESULT cli_modelc_sensor_setting_set_unacknowledged(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    UINT16 argv_index;
    UINT16 input_len;
    MS_SENSOR_SETTING_SET_STRUCT  param;
    argv_index = 0;
    CONSOLE_OUT
    (">> Send Sensor Setting Set Unacknowledged\n");

    if (3 == argc)
    {
        CONSOLE_OUT
        ("Sensor Property ID (16-bit in HEX)\n");
        choice = CLI_strtoi(argv[argv_index], (UINT16)CLI_strlen(argv[argv_index]), 16);
        argv_index++;
        param.sensor_property_id = (UINT16)choice;
        CONSOLE_OUT
        ("Sensor Setting Property ID (16-bit in HEX)\n");
        choice = CLI_strtoi(argv[argv_index], (UINT16)CLI_strlen(argv[argv_index]), 16);
        argv_index++;
        param.sensor_setting_property_id = (UINT16)choice;
        choice = CLI_strlen(argv[argv_index]);
        choice = (choice + 1) / 2;
        CONSOLE_OUT
        ("Sensor Setting Raw (%d-octets in HEX)\n", (UINT16)choice);
        input_len = (UINT16)choice;
        param.sensor_setting_raw = EM_alloc_mem(input_len);

        if (NULL == param.sensor_setting_raw)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Sensor Setting Raw. Returning\n");
            return API_FAILURE;
        }

        param.sensor_setting_raw_len = (UINT16)input_len;
        CLI_strtoarray
        (
            argv[argv_index],
            (UINT16)CLI_strlen(argv[argv_index]),
            param.sensor_setting_raw,
            input_len
        );
        argv_index++;
    }

    retval = MS_sensor_setting_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.sensor_setting_raw)
    {
        EM_free_mem(param.sensor_setting_raw);
    }

    return retval;
}


/* Send Sensor Settings Get */
API_RESULT cli_modelc_sensor_settings_get(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    int  choice;
    MS_SENSOR_SETTINGS_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Sensor Settings Get\n");

    if (1 == argc)
    {
        choice = CLI_strtoi(argv[0], (UINT16)CLI_strlen(argv[0]), 16);
        param.sensor_property_id = (UINT16)choice;
        CONSOLE_OUT("Sensor Property ID (16-bit in HEX): 0x%04X\n", param.sensor_property_id);
    }

    retval = MS_sensor_settings_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
    return retval;
}

/* Get Model Handle */
API_RESULT cli_modelc_sensor_get_model_handle(UINT32 argc, UCHAR* argv[])
{
    API_RESULT retval;
    MS_IGNORE_UNUSED_PARAM(argc);
    MS_IGNORE_UNUSED_PARAM(argv);
    #if 0
    MS_ACCESS_MODEL_HANDLE model_handle;
    retval = MS_sensor_get_model_handle(&model_handle);

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
API_RESULT cli_modelc_sensor_set_publish_address(UINT32 argc, UCHAR* argv[])
{
    int  choice;
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    MS_ACCESS_PUBLISH_INFO  publish_info;
    /* Set Publish Information */
    EM_mem_set(&publish_info, 0, sizeof(publish_info));
    publish_info.addr.use_label = MS_FALSE;
    model_handle = appl_sensor_client_model_handle;
    CONSOLE_OUT("Model Handle (16-bit in HEX): 0x%04X\n", model_handle);

    /* Check Number of Arguments */
    if (2 == argc)
    {
        choice = CLI_strtoi(argv[0], (UINT16)CLI_strlen(argv[0]), 16);
        publish_info.addr.addr = (UINT16)choice;
        CONSOLE_OUT("Sensor Server Address (16-bit in HEX): 0x%04X\n", publish_info.addr.addr);
        choice = CLI_strtoi(argv[1], (UINT16)CLI_strlen(argv[1]), 16);
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
    Sensor client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_sensor_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
)
{
    API_RESULT retval;
    MS_IGNORE_UNUSED_PARAM(handle);
    retval = API_SUCCESS;
    CONSOLE_OUT (
        "[SENSOR_CLIENT] Callback. Opcode 0x%04X\n", opcode);
    appl_dump_bytes(data_param, data_len);

    switch(opcode)
    {
    case MS_ACCESS_SENSOR_CADENCE_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_SENSOR_CADENCE_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_SENSOR_COLUMN_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_SENSOR_COLUMN_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_SENSOR_DESCRIPTOR_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_SENSOR_DESCRIPTOR_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_SENSOR_SERIES_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_SENSOR_SERIES_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_SENSOR_SETTING_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_SENSOR_SETTING_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_SENSOR_SETTINGS_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_SENSOR_SETTINGS_STATUS_OPCODE\n");
    }
    break;

    case MS_ACCESS_SENSOR_STATUS_OPCODE:
    {
        CONSOLE_OUT(
            "MS_ACCESS_SENSOR_STATUS_OPCODE\n");
    }
    break;
    }

    return retval;
}

