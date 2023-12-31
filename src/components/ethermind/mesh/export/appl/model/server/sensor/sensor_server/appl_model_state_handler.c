/**
    \file appl_model_state_handler.c
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_model_state_handler.h"
#include "MS_common.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */


/* --------------------------------------------- Data Types/Structures */
#define MS_MAX_NUM_STATES            2
#define MS_MAX_SENSORS               1
#define MS_MAX_SENSOR_SETTINGS       1

#define MS_PROP_ID_FOR_TEST          0x0042 /* 0x1234 */

/* UINT16 value of People Count */
#define MS_PROP_ID_PEOPLE_COUNT     0x004C

typedef struct _MS_SENSOR_STRUCT
{
    /** -- Sensor Descriptor -- **/
    /**
        Sensor Property ID field is a 2-octet value referencing a device property
        that describes the meaning and the format of data reported by a sensor
    */
    UINT16 sensor_property_id;

    /**
        Sensor Positive Tolerance field is a 12-bit value representing the magnitude
        of a possible positive error associated with the measurements that the sensor
        is reporting
    */
    UINT16 sensor_positive_tolerance;

    /**
        Sensor Negative Tolerance field is a 12-bit value representing the magnitude
        of a possible negative error associated with the measurements that the sensor
        is reporting
    */
    UINT16 sensor_negative_tolerance;

    /**
        Sensor Sampling Function field specifies the averaging operation or type of
        sampling function applied to the measured value
    */
    UCHAR  sensor_sampling_function;

    /**
        Sensor Measurement Period field specifies a uint8 value n that represents the
        averaging time span, accumulation time, or measurement period in seconds over
        which the measurement is taken
    */
    UCHAR  sensor_measurement_period;

    /**
        measurement reported by a sensor is internally refreshed at the frequency
        indicated in the Sensor Update Interval field
    */
    UCHAR  sensor_update_interval;


    /** -- Sensor Cadence -- **/
    /** Divisor for the Publish Period */
    UCHAR  fast_cadence_period_divisor;

    /** Defines the unit and format of the Status Trigger Delta fields */
    UCHAR  status_trigger_type;

    /** Delta down value that triggers a status message */
    UCHAR* status_trigger_delta_down;
    UINT16 status_trigger_delta_down_len;

    /** Delta up value that triggers a status message */
    UCHAR* status_trigger_delta_up;
    UINT16 status_trigger_delta_up_len;

    /** Minimum interval between two consecutive Status messages */
    UCHAR  status_min_interval;

    /** Low value for the fast cadence range */
    UCHAR* fast_cadence_low;
    UINT16 fast_cadence_low_len;

    /** High value for the fast cadence range */
    UCHAR* fast_cadence_high;
    UINT16 fast_cadence_high_len;

    /** -- Sensor Settings -- **/
    /* Start Index of associated Sensor Settings */
    UINT16  sensor_settings_start_index;

    /* Count of Sensor Settings */
    UINT8  sensor_settings_count;

    /** -- Sensor Data -- **/

    /* Sensor Data */
    UINT8* sensor_data;

    /* Sensor Data Length */
    UINT8  sensor_data_length;

} MS_SENSOR_STRUCT;

typedef struct _MS_SENSOR_SETTING_STRUCT
{
    /** Index of Property of a sensor */
    UINT16 sensor_property_index;

    /** Property ID of a setting within a sensor */
    UINT16 sensor_setting_property_id;

    /** Read/Write access rights for the setting */
    UCHAR  sensor_setting_access;

    /** Raw value of a setting within a sensor */
    UCHAR* sensor_setting_raw;
    UINT16 sensor_setting_raw_len;

} MS_SENSOR_SETTING_STRUCT;


#if 0
    static MS_STATE_SENSOR_PROPERTY_ID_STRUCT      appl_sensor[MS_MAX_NUM_STATES];
    static MS_STATE_SENSOR_DESCRIPTOR_STRUCT       appl_sensor_desc[MS_MAX_NUM_STATES];
    static MS_STATE_SENSOR_SERIES_COLUMN_STRUCT    appl_sensor_series[MS_MAX_NUM_STATES];
    static MS_STATE_SENSOR_DATA_STRUCT             appl_sensor_data[MS_MAX_NUM_STATES];
    static MS_STATE_SENSOR_CADENCE_STRUCT          appl_sensor_cadence[MS_MAX_NUM_STATES];
    static MS_STATE_SENSOR_SETTING_STRUCT          appl_sensor_setting[MS_MAX_NUM_STATES];
    static MS_STATE_SENSOR_SETTINGS_STRUCT         appl_sensor_settings[MS_MAX_NUM_STATES];
#endif /* 0 */

static MS_SENSOR_STRUCT                        appl_sensors[MS_MAX_NUM_STATES][MS_MAX_SENSORS];
static MS_SENSOR_SETTING_STRUCT                appl_sensor_settings[MS_MAX_NUM_STATES][MS_MAX_SENSOR_SETTINGS];

/* For Cadence */
static UCHAR appl_status_trigger_delta_down[MS_MAX_NUM_STATES][MS_MAX_SENSORS][16];
static UCHAR appl_status_trigger_delta_up[MS_MAX_NUM_STATES][MS_MAX_SENSORS][16];
static UCHAR appl_fast_cadence_low[MS_MAX_NUM_STATES][MS_MAX_SENSORS][16];
static UCHAR appl_fast_cadence_high[MS_MAX_NUM_STATES][MS_MAX_SENSORS][16];

/* For Settings */
static UCHAR appl_sensor_setting_raw[MS_MAX_NUM_STATES][MS_MAX_SENSOR_SETTINGS][16];

/* For Data */
static UCHAR appl_sensor_data[MS_MAX_NUM_STATES][MS_MAX_SENSORS][16];

/* Raw Column/Series Values */
static UINT8 appl_raw_x = 0x00;
static UINT8 appl_raw_width = 0x0F;
static UINT8 appl_raw_y = 0x10;

/* --------------------------------------------- Function */
void appl_model_states_initialization(void)
{
    UINT32 index;
    #if 0
    EM_mem_set (appl_sensor, 0, sizeof(appl_sensor));
    EM_mem_set(appl_sensor_desc, 0, sizeof(appl_sensor_desc));
    EM_mem_set(appl_sensor_series, 0, sizeof(appl_sensor_series));
    EM_mem_set(appl_sensor_data, 0, sizeof(appl_sensor_data));
    EM_mem_set(appl_sensor_cadence, 0, sizeof(appl_sensor_cadence));
    EM_mem_set(appl_sensor_setting, 0, sizeof(appl_sensor_setting));
    EM_mem_set(appl_sensor_settings, 0, sizeof(appl_sensor_settings));
    #else
    EM_mem_set(appl_sensors, 0, sizeof(appl_sensors));
    EM_mem_set(appl_sensor_settings, 0, sizeof(appl_sensor_settings));

    for (index = 0; index < MS_MAX_SENSOR_SETTINGS; index++)
    {
        appl_sensor_settings[0][index].sensor_property_index = index;
        /* Read + Write Permission */
        appl_sensor_settings[0][index].sensor_setting_access = 0x03;
        appl_sensor_settings[0][index].sensor_setting_property_id = index + 0x1234;
        appl_sensor_settings[0][index].sensor_setting_raw = &appl_sensor_setting_raw[0][index][0];
        appl_sensor_settings[0][index].sensor_setting_raw_len = 1;
    }

    /* TODO: only initializing 0-th instance */
    for (index = 0; index < MS_MAX_SENSORS; index++)
    {
        appl_sensors[0][index].sensor_property_id = MS_PROP_ID_FOR_TEST;
        appl_status_trigger_delta_down[0][index][0] = 0x01;
        appl_sensors[0][index].status_trigger_delta_down = &appl_status_trigger_delta_down[0][index][0];
        appl_sensors[0][index].status_trigger_delta_down_len = 0x01;
        appl_status_trigger_delta_up[0][index][0] = 0xA0;
        appl_sensors[0][index].status_trigger_delta_up = &appl_status_trigger_delta_up[0][index][0];
        appl_sensors[0][index].status_trigger_delta_up_len = 0x01;
        appl_fast_cadence_low[0][index][0] = 0x00;
        appl_sensors[0][index].fast_cadence_low = &appl_fast_cadence_low[0][index][0];
        appl_sensors[0][index].fast_cadence_low_len = 0x01;
        appl_fast_cadence_high[0][index][0] = 0xA0;
        appl_sensors[0][index].fast_cadence_high = &appl_fast_cadence_high[0][index][0];
        appl_sensors[0][index].fast_cadence_high_len = 0x01;
        /* TODO: update example to use more than one settings per sensor property */
        appl_sensors[0][index].sensor_settings_start_index = index;
        appl_sensors[0][index].sensor_settings_count = 1;
        appl_sensors[0][index].sensor_data = &appl_sensor_data[0][index][0];
        appl_sensors[0][index].sensor_data_length = 1;
        appl_sensors[0][index].fast_cadence_period_divisor = 0x01;
        appl_sensors[0][index].status_trigger_type = 0x00;
        appl_sensors[0][index].status_min_interval = 0x01;
    }

    #endif /* 0 */
}

API_RESULT appl_search_sensor_property_id(/* IN */ UINT16 state_inst, /* IN */ UINT16 prop_id, /* OUT */ UINT16* prop_index)
{
    UINT16 index;
    API_RESULT retval;
    retval = API_FAILURE;

    for (index = 0; index < MS_MAX_SENSORS; index ++)
    {
        if (appl_sensors[state_inst][index].sensor_property_id == prop_id)
        {
            *prop_index = (UINT16)index;
            retval = API_SUCCESS;
            break;
        }
    }

    return retval;
}

API_RESULT appl_search_sensor_setting_property_id(/* IN */ UINT16 state_inst, /* IN */ UINT16 prop_id, /* IN */ UINT16 setting_prop_id, /* OUT */ UINT16* setting_index)
{
    UINT16 index;
    UINT16 sensor_propery_index;
    API_RESULT retval;
    retval = appl_search_sensor_property_id(state_inst, prop_id, &sensor_propery_index);

    if (API_SUCCESS == retval)
    {
        UINT32 count;
        retval = API_FAILURE;

        for (count = 0; count < appl_sensors[state_inst][sensor_propery_index].sensor_settings_count; count++)
        {
            if (setting_prop_id == appl_sensor_settings[state_inst][appl_sensors[state_inst][sensor_propery_index].sensor_settings_start_index + count].sensor_setting_property_id)
            {
                *setting_index = appl_sensors[state_inst][sensor_propery_index].sensor_settings_start_index + count;
                retval = API_SUCCESS;
                break;
            }
        }
    }

    return retval;
}

void appl_model_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    UINT16     sensor_propery_index;
    UINT16     setting_propery_index;

    switch(state_t)
    {
    case MS_STATE_SENSOR_DESCRIPTOR_T:
    {
        MS_STATE_SENSOR_DESCRIPTOR_STRUCT* param_p;
        param_p = (MS_STATE_SENSOR_DESCRIPTOR_STRUCT*)param;

        if (0x0000 != param_p->sensor_property_id)
        {
            retval = appl_search_sensor_property_id(state_inst, param_p->sensor_property_id, &sensor_propery_index);
        }
        else
        {
            retval = API_SUCCESS;
            sensor_propery_index = 0x00;
        }

        if (API_SUCCESS == retval)
        {
            param_p->sensor_property_id = appl_sensors[state_inst][sensor_propery_index].sensor_property_id;
            param_p->sensor_measurement_period = appl_sensors[state_inst][sensor_propery_index].sensor_measurement_period;
            param_p->sensor_negative_tolerance = appl_sensors[state_inst][sensor_propery_index].sensor_negative_tolerance;
            param_p->sensor_positive_tolerance = appl_sensors[state_inst][sensor_propery_index].sensor_positive_tolerance;
            param_p->sensor_sampling_function = appl_sensors[state_inst][sensor_propery_index].sensor_sampling_function;
            param_p->sensor_update_interval = appl_sensors[state_inst][sensor_propery_index].sensor_update_interval;
            param_p->status = 0x01;
        }
        else
        {
            param_p->status = 0x00;
        }
    }
    break;

    case MS_STATE_SENSOR_CADENCE_T:
    {
        MS_STATE_SENSOR_CADENCE_STRUCT* param_p;
        param_p = (MS_STATE_SENSOR_CADENCE_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        retval = appl_search_sensor_property_id(state_inst, param_p->sensor_property_id, &sensor_propery_index);
        param_p->status = 0x00;

        if (API_SUCCESS == retval)
        {
            param_p->sensor_property_id = appl_sensors[state_inst][sensor_propery_index].sensor_property_id;
            param_p->fast_cadence_high = appl_sensors[state_inst][sensor_propery_index].fast_cadence_high;
            param_p->fast_cadence_high_len = appl_sensors[state_inst][sensor_propery_index].fast_cadence_high_len;
            param_p->fast_cadence_low = appl_sensors[state_inst][sensor_propery_index].fast_cadence_low;
            param_p->fast_cadence_low_len = appl_sensors[state_inst][sensor_propery_index].fast_cadence_low_len;
            param_p->fast_cadence_period_divisor = appl_sensors[state_inst][sensor_propery_index].fast_cadence_period_divisor;
            param_p->status_min_interval = appl_sensors[state_inst][sensor_propery_index].status_min_interval;
            param_p->status_trigger_delta_down = appl_sensors[state_inst][sensor_propery_index].status_trigger_delta_down;
            param_p->status_trigger_delta_down_len = appl_sensors[state_inst][sensor_propery_index].status_trigger_delta_down_len;
            param_p->status_trigger_delta_up = appl_sensors[state_inst][sensor_propery_index].status_trigger_delta_up;
            param_p->status_trigger_delta_up_len = appl_sensors[state_inst][sensor_propery_index].status_trigger_delta_up_len;
            param_p->status_trigger_type = appl_sensors[state_inst][sensor_propery_index].status_trigger_type;
            param_p->status = 0x001;
        }
    }
    break;

    case MS_STATE_SENSOR_SETTINGS_T:
    {
        MS_STATE_SENSOR_SETTINGS_STRUCT* param_p;
        param_p = (MS_STATE_SENSOR_SETTINGS_STRUCT*)param;
        param_p->setting_property_ids_count = 0;
        retval = appl_search_sensor_property_id(state_inst, param_p->sensor_property_id, &sensor_propery_index);

        if (API_SUCCESS == retval)
        {
            UINT32 count;
            param_p->sensor_property_id = appl_sensors[state_inst][sensor_propery_index].sensor_property_id;

            for (count = 0; count < appl_sensors[state_inst][sensor_propery_index].sensor_settings_count; count ++)
            {
                param_p->setting_property_ids[count] = appl_sensor_settings[state_inst][appl_sensors[state_inst][sensor_propery_index].sensor_settings_start_index + count].sensor_setting_property_id;
            }

            param_p->setting_property_ids_count = appl_sensors[state_inst][sensor_propery_index].sensor_settings_count;
        }
    }
    break;

    case MS_STATE_SENSOR_SETTING_T:
    {
        MS_STATE_SENSOR_SETTING_STRUCT* param_p;
        param_p = (MS_STATE_SENSOR_SETTING_STRUCT*)param;
        param_p->sensor_setting_raw_len = 0x00;
        retval = appl_search_sensor_setting_property_id(state_inst, param_p->sensor_property_id, param_p->sensor_setting_property_id, &setting_propery_index);

        if (API_SUCCESS == retval)
        {
            UINT32 count;
            /* param_p->sensor_property_id = appl_sensors[state_inst][sensor_propery_index].sensor_property_id; */
            param_p->sensor_setting_access = appl_sensor_settings[state_inst][setting_propery_index].sensor_setting_access;
            param_p->sensor_setting_raw = appl_sensor_settings[state_inst][setting_propery_index].sensor_setting_raw;
            param_p->sensor_setting_raw_len = appl_sensor_settings[state_inst][setting_propery_index].sensor_setting_raw_len;
            param_p->status = 0x01;
        }
        else
        {
            param_p->status = 0x00;
        }
    }
    break;

    case MS_STATE_SENSOR_DATA_T:
    {
        MS_STATE_SENSOR_DATA_STRUCT* param_p;
        param_p = (MS_STATE_SENSOR_DATA_STRUCT*)param;
        param_p->raw_value_1 = NULL;
        param_p->raw_value_1_len = 0;
        param_p->status = 0x00;

        if (0x0000 != param_p->property_id_1)
        {
            retval = appl_search_sensor_property_id(state_inst, param_p->property_id_1, &sensor_propery_index);

            if (API_SUCCESS != retval)
            {
                /* param_p->property_id_1 = 0x1234; */
                /* param_p->property_id_1 = 0x78; */
                return retval;
            }
            else
            {
                param_p->status = 0x01;
            }
        }
        else
        {
            retval = API_SUCCESS;
            sensor_propery_index = 0x00;
            param_p->status = 0x01;
        }

        param_p->property_id_1 = appl_sensors[state_inst][sensor_propery_index].sensor_property_id;
        param_p->raw_value_1 = appl_sensors[state_inst][sensor_propery_index].sensor_data;
        param_p->raw_value_1_len = appl_sensors[state_inst][sensor_propery_index].sensor_data_length;
    }
    break;

    case MS_STATE_SENSOR_SERIES_COLUMN_T:
    {
        MS_STATE_SENSOR_SERIES_COLUMN_STRUCT* param_p;
        param_p = (MS_STATE_SENSOR_SERIES_COLUMN_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        /* *param_p = appl_sensor_series[0]; */
        retval = appl_search_sensor_property_id(state_inst, param_p->sensor_property_id, &sensor_propery_index);
        param_p->status = 0x00;

        if (API_SUCCESS == retval)
        {
            param_p->sensor_raw_value_x = &appl_raw_x;
            param_p->sensor_raw_value_x_len = sizeof(appl_raw_x);
            param_p->sensor_column_width = &appl_raw_width;
            param_p->sensor_column_width_len = sizeof(appl_raw_width);
            param_p->sensor_raw_value_y = &appl_raw_y;
            param_p->sensor_raw_value_y_len = sizeof(appl_raw_y);
            param_p->status = 0x001;
        }
    }
    break;

    case MS_STATE_SENSOR_SERIES_T:
    {
        MS_STATE_SENSOR_SERIES_COLUMN_STRUCT* param_p;
        param_p = (MS_STATE_SENSOR_SERIES_COLUMN_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        /* *param_p = appl_sensor_series[0]; */
        retval = appl_search_sensor_property_id(state_inst, param_p->sensor_property_id, &sensor_propery_index);
        param_p->status = 0x00;

        if (API_SUCCESS == retval)
        {
            param_p->sensor_raw_value_x = &appl_raw_x;
            param_p->sensor_raw_value_x_len = sizeof(appl_raw_x);
            param_p->sensor_column_width = &appl_raw_width;
            param_p->sensor_column_width_len = sizeof(appl_raw_width);
            param_p->sensor_raw_value_y = &appl_raw_y;
            param_p->sensor_raw_value_y_len = sizeof(appl_raw_y);
            param_p->status = 0x001;
        }
    }
    break;

    default:
        break;
    }
}


void appl_model_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    UINT16     sensor_propery_index;
    UINT16     setting_propery_index;

    switch(state_t)
    {
    case MS_STATE_SENSOR_SETTING_T:
    {
        MS_STATE_SENSOR_SETTING_STRUCT* param_p;
        param_p = (MS_STATE_SENSOR_SETTING_STRUCT*)param;
        retval = appl_search_sensor_setting_property_id(state_inst, param_p->sensor_property_id, param_p->sensor_setting_property_id, &setting_propery_index);

        if (API_SUCCESS == retval)
        {
            /* param_p->sensor_property_id = appl_sensors[state_inst][sensor_propery_index].sensor_property_id; */
            param_p->sensor_setting_access = appl_sensor_settings[state_inst][setting_propery_index].sensor_setting_access;

            if (0 != param_p->sensor_setting_raw_len)
            {
                EM_mem_copy
                (
                    &appl_sensor_settings[state_inst][setting_propery_index].sensor_setting_raw[0],
                    param_p->sensor_setting_raw,
                    param_p->sensor_setting_raw_len
                );
                appl_sensor_settings[state_inst][setting_propery_index].sensor_setting_raw_len = param_p->sensor_setting_raw_len;
            }

            /* param_p->sensor_setting_raw = appl_sensor_settings[state_inst][setting_propery_index].sensor_setting_raw; */
            /* param_p->sensor_setting_raw_len = appl_sensor_settings[state_inst][setting_propery_index].sensor_setting_raw_len; */
            param_p->status = 0x01;
        }
        else
        {
            param_p->status = 0x00;
            param_p->sensor_setting_raw_len = 0x00;
        }
    }
    break;

    case MS_STATE_SENSOR_CADENCE_T:
    {
        MS_STATE_SENSOR_CADENCE_STRUCT* param_p;
        param_p = (MS_STATE_SENSOR_CADENCE_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        retval = appl_search_sensor_property_id(state_inst, param_p->sensor_property_id, &sensor_propery_index);
        param_p->status = 0x00;

        if (API_SUCCESS == retval)
        {
            /* Check Status Min Interval is in range 0-26 */
            if (26 < param_p->status_trigger_delta_down[2])
            {
                param_p->status = 0x02;
                break;
            }

            /* Copy */
            appl_sensors[state_inst][sensor_propery_index].fast_cadence_period_divisor = param_p->fast_cadence_period_divisor;
            appl_sensors[state_inst][sensor_propery_index].status_trigger_type = param_p->status_trigger_type;
            appl_sensors[state_inst][sensor_propery_index].status_trigger_delta_down[0] = param_p->status_trigger_delta_down[0];
            appl_sensors[state_inst][sensor_propery_index].status_trigger_delta_up[0] = param_p->status_trigger_delta_down[1];
            appl_sensors[state_inst][sensor_propery_index].status_min_interval = param_p->status_trigger_delta_down[2];
            appl_sensors[state_inst][sensor_propery_index].fast_cadence_low[0] = param_p->status_trigger_delta_down[3];
            appl_sensors[state_inst][sensor_propery_index].fast_cadence_high[0] = param_p->status_trigger_delta_down[4];
            /* Reassign */
            param_p->sensor_property_id = appl_sensors[state_inst][sensor_propery_index].sensor_property_id;
            param_p->fast_cadence_high = appl_sensors[state_inst][sensor_propery_index].fast_cadence_high;
            param_p->fast_cadence_high_len = appl_sensors[state_inst][sensor_propery_index].fast_cadence_high_len;
            param_p->fast_cadence_low = appl_sensors[state_inst][sensor_propery_index].fast_cadence_low;
            param_p->fast_cadence_low_len = appl_sensors[state_inst][sensor_propery_index].fast_cadence_low_len;
            param_p->fast_cadence_period_divisor = appl_sensors[state_inst][sensor_propery_index].fast_cadence_period_divisor;
            param_p->status_min_interval = appl_sensors[state_inst][sensor_propery_index].status_min_interval;
            param_p->status_trigger_delta_down = appl_sensors[state_inst][sensor_propery_index].status_trigger_delta_down;
            param_p->status_trigger_delta_down_len = appl_sensors[state_inst][sensor_propery_index].status_trigger_delta_down_len;
            param_p->status_trigger_delta_up = appl_sensors[state_inst][sensor_propery_index].status_trigger_delta_up;
            param_p->status_trigger_delta_up_len = appl_sensors[state_inst][sensor_propery_index].status_trigger_delta_up_len;
            param_p->status_trigger_type = appl_sensors[state_inst][sensor_propery_index].status_trigger_type;
            param_p->status = 0x01;
        }
    }
    break;
    #if 0

    case MS_STATE_SENSOR_DESCRIPTOR_T:
    {
        MS_STATE_SENSOR_DESCRIPTOR_STRUCT* param_p;
        param_p = (MS_STATE_SENSOR_DESCRIPTOR_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_sensor_desc[0] = *param_p;
    }
    break;

    case MS_STATE_SENSOR_SERIES_COLUMN_T:
    {
        MS_STATE_SENSOR_SERIES_COLUMN_STRUCT* param_p;
        param_p = (MS_STATE_SENSOR_SERIES_COLUMN_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_sensor_series[0] = *param_p;
    }
    break;

    case MS_STATE_SENSOR_DATA_T:
    {
        MS_STATE_SENSOR_DATA_STRUCT* param_p;
        param_p = (MS_STATE_SENSOR_DATA_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_sensor_data[0] = *param_p;
    }
    break;

    case MS_STATE_SENSOR_SETTING_T:
    {
        MS_STATE_SENSOR_SETTING_STRUCT* param_p;
        param_p = (MS_STATE_SENSOR_SETTING_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_sensor_setting[0] = *param_p;
    }
    break;

    case MS_STATE_SENSOR_SETTINGS_T:
    {
        MS_STATE_SENSOR_SETTINGS_STRUCT* param_p;
        param_p = (MS_STATE_SENSOR_SETTINGS_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_sensor_settings[0] = *param_p;
    }
    break;
        #endif /* 0 */

    default:
        break;
    }
}

