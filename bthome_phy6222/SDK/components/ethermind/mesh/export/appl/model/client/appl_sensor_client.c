/**
    \file appl_sensor_client.c

    \brief This file defines the Mesh Sensor Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_sensor_client.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
static const char main_sensor_client_options[] = "\n\
======== Sensor Client Menu ========\n\
     0. Exit. \n\
     1. Refresh. \n\
 \n\
    10. Send Sensor Cadence Get. \n\
    11. Send Sensor Cadence Set. \n\
    12. Send Sensor Cadence Set Unacknowledged. \n\
    13. Send Sensor Column Get. \n\
    14. Send Sensor Descriptor Get. \n\
    15. Send Sensor Get. \n\
    16. Send Sensor Series Get. \n\
    17. Send Sensor Setting Get. \n\
    18. Send Sensor Setting Set. \n\
    19. Send Sensor Setting Set Unacknowledged. \n\
    20. Send Sensor Settings Get. \n\
 \n\
    21. Get Model Handle. \n\
    22. Set Publish Address. \n\
 \n\
Your Option ? \0";


/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_sensor_client_model_handle;


/* --------------------------------------------- Function */
/* sensor client application entry point */
void main_sensor_client_operations(void)
{
    int choice;
    MS_ACCESS_ELEMENT_HANDLE element_handle;
    static UCHAR model_initialized = 0x00;

    /**
        Register with Access Layer.
    */
    if (0x00 == model_initialized)
    {
        API_RESULT retval;
        /* Use Default Element Handle. Index 0 */
        element_handle = MS_ACCESS_DEFAULT_ELEMENT_HANDLE;
        retval = MS_sensor_client_init
                 (
                     element_handle,
                     &appl_sensor_client_model_handle,
                     appl_sensor_client_cb
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

    MS_LOOP_FOREVER()
    {
        CONSOLE_OUT
        ("%s", main_sensor_client_options);
        CONSOLE_IN
        ("%d", &choice);

        if (choice < 0)
        {
            CONSOLE_OUT
            ("*** Invalid Choice. Try Again.\n");
            continue;
        }

        switch (choice)
        {
        case 0:
            return;

        case 1:
            break;

        case 10: /* Send Sensor Cadence Get */
            appl_send_sensor_cadence_get();
            break;

        case 11: /* Send Sensor Cadence Set */
            appl_send_sensor_cadence_set();
            break;

        case 12: /* Send Sensor Cadence Set Unacknowledged */
            appl_send_sensor_cadence_set_unacknowledged();
            break;

        case 13: /* Send Sensor Column Get */
            appl_send_sensor_column_get();
            break;

        case 14: /* Send Sensor Descriptor Get */
            appl_send_sensor_descriptor_get();
            break;

        case 15: /* Send Sensor Get */
            appl_send_sensor_get();
            break;

        case 16: /* Send Sensor Series Get */
            appl_send_sensor_series_get();
            break;

        case 17: /* Send Sensor Setting Get */
            appl_send_sensor_setting_get();
            break;

        case 18: /* Send Sensor Setting Set */
            appl_send_sensor_setting_set();
            break;

        case 19: /* Send Sensor Setting Set Unacknowledged */
            appl_send_sensor_setting_set_unacknowledged();
            break;

        case 20: /* Send Sensor Settings Get */
            appl_send_sensor_settings_get();
            break;

        case 21: /* Get Model Handle */
            appl_sensor_client_get_model_handle();
            break;

        case 22: /* Set Publish Address */
            appl_sensor_client_set_publish_address();
            break;
        }
    }
}

/* Send Sensor Cadence Get */
void appl_send_sensor_cadence_get(void)
{
    API_RESULT retval;
    int  choice;
    MS_SENSOR_CADENCE_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Sensor Cadence Get\n");
    CONSOLE_OUT
    ("Enter Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.property_id = (UINT16)choice;
    retval = MS_sensor_cadence_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Sensor Cadence Set */
void appl_send_sensor_cadence_set(void)
{
    API_RESULT retval;
    int  choice;
    MS_SENSOR_CADENCE_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Sensor Cadence Set\n");
    CONSOLE_OUT
    ("Enter Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.property_id = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Fast Cadence Period Divisor (7-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.fast_cadence_period_divisor = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Status Trigger Type (1-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.status_trigger_type = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Status Trigger Delta Down Length (in decimal)\n");
    CONSOLE_IN
    ("%d", &choice);
    CONSOLE_OUT
    ("Enter Status Trigger Delta Down (%d-octets in HEX)\n", (UINT16)choice);
    {
        UINT16 index, input_len;
        input_len = (UINT16)choice;
        param.status_trigger_delta_down = EM_alloc_mem(input_len);

        if(NULL == param.status_trigger_delta_down)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Status Trigger Delta Down. Returning\n");
            return;
        }

        param.status_trigger_delta_down_len = (UINT16) input_len;

        for(index = 0; index < input_len; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.status_trigger_delta_down[index] = (UCHAR)choice;
        }
    }
    CONSOLE_OUT
    ("Enter Status Trigger Delta Up Length (in decimal)\n");
    CONSOLE_IN
    ("%d", &choice);
    CONSOLE_OUT
    ("Enter Status Trigger Delta Up (%d-octets in HEX)\n", (UINT16)choice);
    {
        UINT16 index, input_len;
        input_len = (UINT16)choice;
        param.status_trigger_delta_up = EM_alloc_mem(input_len);

        if(NULL == param.status_trigger_delta_up)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Status Trigger Delta Up. Returning\n");
            return;
        }

        param.status_trigger_delta_up_len = (UINT16) input_len;

        for(index = 0; index < input_len; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.status_trigger_delta_up[index] = (UCHAR)choice;
        }
    }
    CONSOLE_OUT
    ("Enter Status Min Interval (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.status_min_interval = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Fast Cadence Low Length (in decimal)\n");
    CONSOLE_IN
    ("%d", &choice);
    CONSOLE_OUT
    ("Enter Fast Cadence Low (%d-octets in HEX)\n", (UINT16)choice);
    {
        UINT16 index, input_len;
        input_len = (UINT16)choice;
        param.fast_cadence_low = EM_alloc_mem(input_len);

        if(NULL == param.fast_cadence_low)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Fast Cadence Low. Returning\n");
            return;
        }

        param.fast_cadence_low_len = (UINT16) input_len;

        for(index = 0; index < input_len; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.fast_cadence_low[index] = (UCHAR)choice;
        }
    }
    CONSOLE_OUT
    ("Enter Fast Cadence High Length (in decimal)\n");
    CONSOLE_IN
    ("%d", &choice);
    CONSOLE_OUT
    ("Enter Fast Cadence High (%d-octets in HEX)\n", (UINT16)choice);
    {
        UINT16 index, input_len;
        input_len = (UINT16)choice;
        param.fast_cadence_high = EM_alloc_mem(input_len);

        if(NULL == param.fast_cadence_high)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Fast Cadence High. Returning\n");
            return;
        }

        param.fast_cadence_high_len = (UINT16) input_len;

        for(index = 0; index < input_len; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.fast_cadence_high[index] = (UCHAR)choice;
        }
    }
    retval = MS_sensor_cadence_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.fast_cadence_high)
    {
        EM_free_mem(param.fast_cadence_high);
    }
}

/* Send Sensor Cadence Set Unacknowledged */
void appl_send_sensor_cadence_set_unacknowledged(void)
{
    API_RESULT retval;
    int  choice;
    MS_SENSOR_CADENCE_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Sensor Cadence Set Unacknowledged\n");
    CONSOLE_OUT
    ("Enter Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.property_id = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Fast Cadence Period Divisor (7-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.fast_cadence_period_divisor = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Status Trigger Type (1-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.status_trigger_type = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Status Trigger Delta Down Length (in decimal)\n");
    CONSOLE_IN
    ("%d", &choice);
    CONSOLE_OUT
    ("Enter Status Trigger Delta Down (%d-octets in HEX)\n", (UINT16)choice);
    {
        UINT16 index, input_len;
        input_len = (UINT16)choice;
        param.status_trigger_delta_down = EM_alloc_mem(input_len);

        if(NULL == param.status_trigger_delta_down)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Status Trigger Delta Down. Returning\n");
            return;
        }

        param.status_trigger_delta_down_len = (UINT16) input_len;

        for(index = 0; index < input_len; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.status_trigger_delta_down[index] = (UCHAR)choice;
        }
    }
    CONSOLE_OUT
    ("Enter Status Trigger Delta Up Length (in decimal)\n");
    CONSOLE_IN
    ("%d", &choice);
    CONSOLE_OUT
    ("Enter Status Trigger Delta Up (%d-octets in HEX)\n", (UINT16)choice);
    {
        UINT16 index, input_len;
        input_len = (UINT16)choice;
        param.status_trigger_delta_up = EM_alloc_mem(input_len);

        if(NULL == param.status_trigger_delta_up)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Status Trigger Delta Up. Returning\n");
            return;
        }

        param.status_trigger_delta_up_len = (UINT16) input_len;

        for(index = 0; index < input_len; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.status_trigger_delta_up[index] = (UCHAR)choice;
        }
    }
    CONSOLE_OUT
    ("Enter Status Min Interval (8-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.status_min_interval = (UCHAR)choice;
    CONSOLE_OUT
    ("Enter Fast Cadence Low Length (in decimal)\n");
    CONSOLE_IN
    ("%d", &choice);
    CONSOLE_OUT
    ("Enter Fast Cadence Low (%d-octets in HEX)\n", (UINT16)choice);
    {
        UINT16 index, input_len;
        input_len = (UINT16)choice;
        param.fast_cadence_low = EM_alloc_mem(input_len);

        if(NULL == param.fast_cadence_low)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Fast Cadence Low. Returning\n");
            return;
        }

        param.fast_cadence_low_len = (UINT16) input_len;

        for(index = 0; index < input_len; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.fast_cadence_low[index] = (UCHAR)choice;
        }
    }
    CONSOLE_OUT
    ("Enter Fast Cadence High Length (in decimal)\n");
    CONSOLE_IN
    ("%d", &choice);
    CONSOLE_OUT
    ("Enter Fast Cadence High (%d-octets in HEX)\n", (UINT16)choice);
    {
        UINT16 index, input_len;
        input_len = (UINT16)choice;
        param.fast_cadence_high = EM_alloc_mem(input_len);

        if(NULL == param.fast_cadence_high)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Fast Cadence High. Returning\n");
            return;
        }

        param.fast_cadence_high_len = (UINT16) input_len;

        for(index = 0; index < input_len; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.fast_cadence_high[index] = (UCHAR)choice;
        }
    }
    retval = MS_sensor_cadence_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.fast_cadence_high)
    {
        EM_free_mem(param.fast_cadence_high);
    }
}

/* Send Sensor Column Get */
void appl_send_sensor_column_get(void)
{
    API_RESULT retval;
    int  choice;
    MS_SENSOR_COLUMN_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Sensor Column Get\n");
    CONSOLE_OUT
    ("Enter Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.property_id = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Raw Value X Length (in decimal)\n");
    CONSOLE_IN
    ("%d", &choice);
    CONSOLE_OUT
    ("Enter Raw Value X (%d-octets in HEX)\n", (UINT16)choice);
    {
        UINT16 index, input_len;
        input_len = (UINT16)choice;
        param.raw_value_x = EM_alloc_mem(input_len);

        if(NULL == param.raw_value_x)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Raw Value X. Returning\n");
            return;
        }

        param.raw_value_x_len = (UINT16) input_len;

        for(index = 0; index < input_len; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.raw_value_x[index] = (UCHAR)choice;
        }
    }
    retval = MS_sensor_column_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.raw_value_x)
    {
        EM_free_mem(param.raw_value_x);
    }
}

/* Send Sensor Descriptor Get */
void appl_send_sensor_descriptor_get(void)
{
    API_RESULT retval;
    int  choice;
    MS_SENSOR_DESCRIPTOR_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Sensor Descriptor Get\n");
    CONSOLE_OUT
    ("Enter if optional fields to be send (1 - Yes, 0 - No)\n");
    CONSOLE_IN
    ("%d", &choice);

    if(0 == choice)
    {
        param.optional_fields_present = 0x00;
    }
    else
    {
        param.optional_fields_present = 0x01;
        CONSOLE_OUT
        ("Enter Property ID (16-bit in HEX)\n");
        CONSOLE_IN
        ("%x", &choice);
        param.property_id = (UINT16)choice;
    }

    retval = MS_sensor_descriptor_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Sensor Get */
void appl_send_sensor_get(void)
{
    API_RESULT retval;
    int  choice;
    MS_SENSOR_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Sensor Get\n");
    CONSOLE_OUT
    ("Enter if optional fields to be send (1 - Yes, 0 - No)\n");
    CONSOLE_IN
    ("%d", &choice);

    if(0 == choice)
    {
        param.optional_fields_present = 0x00;
    }
    else
    {
        param.optional_fields_present = 0x01;
        CONSOLE_OUT
        ("Enter Property ID (16-bit in HEX)\n");
        CONSOLE_IN
        ("%x", &choice);
        param.property_id = (UINT16)choice;
    }

    retval = MS_sensor_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Sensor Series Get */
void appl_send_sensor_series_get(void)
{
    API_RESULT retval;
    int  choice;
    MS_SENSOR_SERIES_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Sensor Series Get\n");
    CONSOLE_OUT
    ("Enter Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.property_id = (UINT16)choice;
    CONSOLE_OUT
    ("Enter if optional fields to be send (1 - Yes, 0 - No)\n");
    CONSOLE_IN
    ("%d", &choice);

    if(0 == choice)
    {
        param.optional_fields_present = 0x00;
    }
    else
    {
        param.optional_fields_present = 0x01;
        CONSOLE_OUT
        ("Enter Raw Value X1 Length (in decimal)\n");
        CONSOLE_IN
        ("%d", &choice);
        CONSOLE_OUT
        ("Enter Raw Value X1 (%d-octets in HEX)\n", (UINT16)choice);
        {
            UINT16 index, input_len;
            input_len = (UINT16)choice;
            param.raw_value_x1 = EM_alloc_mem(input_len);

            if(NULL == param.raw_value_x1)
            {
                CONSOLE_OUT
                ("Memory allocation failed for Raw Value X1. Returning\n");
                return;
            }

            param.raw_value_x1_len = (UINT16) input_len;

            for(index = 0; index < input_len; index++)
            {
                CONSOLE_IN
                ("%x", &choice);
                param.raw_value_x1[index] = (UCHAR)choice;
            }
        }
        CONSOLE_OUT
        ("Enter Raw Value X2 Length (in decimal)\n");
        CONSOLE_IN
        ("%d", &choice);
        CONSOLE_OUT
        ("Enter Raw Value X2 (%d-octets in HEX)\n", (UINT16)choice);
        {
            UINT16 index, input_len;
            input_len = (UINT16)choice;
            param.raw_value_x2 = EM_alloc_mem(input_len);

            if(NULL == param.raw_value_x2)
            {
                CONSOLE_OUT
                ("Memory allocation failed for Raw Value X2. Returning\n");
                return;
            }

            param.raw_value_x2_len = (UINT16) input_len;

            for(index = 0; index < input_len; index++)
            {
                CONSOLE_IN
                ("%x", &choice);
                param.raw_value_x2[index] = (UCHAR)choice;
            }
        }
    }

    retval = MS_sensor_series_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.raw_value_x2)
    {
        EM_free_mem(param.raw_value_x2);
    }
}

/* Send Sensor Setting Get */
void appl_send_sensor_setting_get(void)
{
    API_RESULT retval;
    int  choice;
    MS_SENSOR_SETTING_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Sensor Setting Get\n");
    CONSOLE_OUT
    ("Enter Sensor Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.sensor_property_id = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Sensor Setting Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.sensor_setting_property_id = (UINT16)choice;
    retval = MS_sensor_setting_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}

/* Send Sensor Setting Set */
void appl_send_sensor_setting_set(void)
{
    API_RESULT retval;
    int  choice;
    MS_SENSOR_SETTING_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Sensor Setting Set\n");
    CONSOLE_OUT
    ("Enter Sensor Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.sensor_property_id = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Sensor Setting Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.sensor_setting_property_id = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Sensor Setting Raw Length (in decimal)\n");
    CONSOLE_IN
    ("%d", &choice);
    CONSOLE_OUT
    ("Enter Sensor Setting Raw (%d-octets in HEX)\n", (UINT16)choice);
    {
        UINT16 index, input_len;
        input_len = (UINT16)choice;
        param.sensor_setting_raw = EM_alloc_mem(input_len);

        if(NULL == param.sensor_setting_raw)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Sensor Setting Raw. Returning\n");
            return;
        }

        param.sensor_setting_raw_len = (UINT16) input_len;

        for(index = 0; index < input_len; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.sensor_setting_raw[index] = (UCHAR)choice;
        }
    }
    retval = MS_sensor_setting_set(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.sensor_setting_raw)
    {
        EM_free_mem(param.sensor_setting_raw);
    }
}

/* Send Sensor Setting Set Unacknowledged */
void appl_send_sensor_setting_set_unacknowledged(void)
{
    API_RESULT retval;
    int  choice;
    MS_SENSOR_SETTING_SET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Sensor Setting Set Unacknowledged\n");
    CONSOLE_OUT
    ("Enter Sensor Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.sensor_property_id = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Sensor Setting Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.sensor_setting_property_id = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Sensor Setting Raw Length (in decimal)\n");
    CONSOLE_IN
    ("%d", &choice);
    CONSOLE_OUT
    ("Enter Sensor Setting Raw (%d-octets in HEX)\n", (UINT16)choice);
    {
        UINT16 index, input_len;
        input_len = (UINT16)choice;
        param.sensor_setting_raw = EM_alloc_mem(input_len);

        if(NULL == param.sensor_setting_raw)
        {
            CONSOLE_OUT
            ("Memory allocation failed for Sensor Setting Raw. Returning\n");
            return;
        }

        param.sensor_setting_raw_len = (UINT16) input_len;

        for(index = 0; index < input_len; index++)
        {
            CONSOLE_IN
            ("%x", &choice);
            param.sensor_setting_raw[index] = (UCHAR)choice;
        }
    }
    retval = MS_sensor_setting_set_unacknowledged(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);

    if(NULL != param.sensor_setting_raw)
    {
        EM_free_mem(param.sensor_setting_raw);
    }
}

/* Send Sensor Settings Get */
void appl_send_sensor_settings_get(void)
{
    API_RESULT retval;
    int  choice;
    MS_SENSOR_SETTINGS_GET_STRUCT  param;
    CONSOLE_OUT
    (">> Send Sensor Settings Get\n");
    CONSOLE_OUT
    ("Enter Sensor Property ID (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    param.sensor_property_id = (UINT16)choice;
    retval = MS_sensor_settings_get(&param);
    CONSOLE_OUT
    ("retval = 0x%04X\n", retval);
}


/* Get Model Handle */
void appl_sensor_client_get_model_handle(void)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    retval = MS_sensor_client_get_model_handle(&model_handle);

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

    return;
}


/* Set Publish Address */
void appl_sensor_client_set_publish_address(void)
{
    int  choice;
    API_RESULT retval;
    MS_ACCESS_MODEL_HANDLE model_handle;
    MS_ACCESS_PUBLISH_INFO  publish_info;
    /* Set Publish Information */
    EM_mem_set(&publish_info, 0, sizeof(publish_info));
    publish_info.addr.use_label = MS_FALSE;
    CONSOLE_OUT
    ("Enter Model Handle (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    model_handle = (UINT16)choice;
    CONSOLE_OUT
    ("Enter Sensor client Server Address (16-bit in HEX)\n");
    CONSOLE_IN
    ("%x", &choice);
    publish_info.addr.addr = (UINT16)choice;
    CONSOLE_OUT
    ("Enter AppKey Index\n");
    CONSOLE_IN
    ("%x", &choice);
    publish_info.appkey_index = choice;
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

    return;
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
API_RESULT appl_sensor_client_cb
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

