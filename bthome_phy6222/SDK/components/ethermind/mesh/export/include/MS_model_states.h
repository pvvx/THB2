/**
    \file MS_model_states.h
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_MODEL_STATES_
#define _H_MS_MODEL_STATES_


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */
/** Model State Type Defines */
#define MS_STATE_GENERIC_ONOFF_T                      0
#define MS_STATE_GENERIC_LEVEL_T                      1
#define MS_STATE_GENERIC_DEFAULT_TRANSITION_TIME_T    2
#define MS_STATE_GENERIC_ONPOWERUP_T                  3
#define MS_STATE_GENERIC_POWER_ACTUAL_T               4
#define MS_STATE_GENERIC_POWER_LAST_T                 5
#define MS_STATE_GENERIC_POWER_DEFAULT_T              6
#define MS_STATE_GENERIC_POWER_RANGE_T                7
#define MS_STATE_GENERIC_POWER_LEVEL_T                8
#define MS_STATE_GENERIC_BATTERY_T                    9
#define MS_STATE_GENERIC_LOCATION_GLOBAL_T            10
#define MS_STATE_GENERIC_LOCATION_LOCAL_T             11
#define MS_STATE_GENERIC_LOCATION_T                   12
#define MS_STATE_GENERIC_USER_PROPERTY_T              13
#define MS_STATE_GENERIC_ADMIN_PROPERTY_T             14
#define MS_STATE_GENERIC_MANUFACTURER_PROPERTY_T      15
#define MS_STATE_GENERIC_PROPERTY_ID_T                16
#define MS_STATE_GENERIC_PROPERTY_IDS_T               17
#define MS_STATE_SENSOR_PROPERTY_ID_T                 18
#define MS_STATE_SENSOR_DESCRIPTOR_T                  19
#define MS_STATE_SENSOR_SETTINGS_T                    20
#define MS_STATE_SENSOR_SETTING_T                     21
#define MS_STATE_SENSOR_CADENCE_T                     22
#define MS_STATE_SENSOR_DATA_PROPERTY_ID_T            23
#define MS_STATE_SENSOR_DATA_T                        24
#define MS_STATE_SENSOR_COLUMN_STATUS_T               25
#define MS_STATE_SENSOR_SERIES_COLUMN_T               26

#define MS_STATE_TIME_T                               27
#define MS_STATE_TIME_ZONE_T                          28
#define MS_STATE_TIME_TAI_UTC_DELTA_T                 29
#define MS_STATE_TIME_ROLE_T                          30
#define MS_STATE_SCENE_NUMBER_T                       31
#define MS_STATE_SCENE_STATUS_T                       32
#define MS_STATE_SCENE_REGISTER_STATUS_T              33
#define MS_STATE_SCHEDULER_SCHEDULES_T                34
#define MS_STATE_SCHEDULER_ENTRY_INDEX_T              35
#define MS_STATE_SCHEDULER_ENTRY_T                    36
#define MS_STATE_LIGHT_LIGHTNESS_LINEAR_T             37
#define MS_STATE_LIGHT_LIGHTNESS_ACTUAL_T             38
#define MS_STATE_LIGHT_LIGHTNESS_LAST_T               39
#define MS_STATE_LIGHT_LIGHTNESS_DEFAULT_T            40
#define MS_STATE_LIGHT_LIGHTNESS_RANGE_T              41
#define MS_STATE_LIGHT_LIGHTNESS_T                    42
#define MS_STATE_LIGHT_CTL_T                          43
#define MS_STATE_LIGHT_CTL_DEFAULT_T                  44
#define MS_STATE_LIGHT_CTL_TEMPERATURE_T              45
#define MS_STATE_LIGHT_CTL_TEMPERATURE_RANGE_T        46
#define MS_STATE_LIGHT_HSL_T                          47
#define MS_STATE_LIGHT_HSL_TARGET_T                   48
#define MS_STATE_LIGHT_HSL_DEFAULT_T                  49
#define MS_STATE_LIGHT_HSL_HUE_T                      50
#define MS_STATE_LIGHT_HSL_SATURATION_T               51
#define MS_STATE_LIGHT_HSL_RANGE_T                    52
#define MS_STATE_LIGHT_XYL_T                          53
#define MS_STATE_LIGHT_XYL_TARGET_T                   54
#define MS_STATE_LIGHT_XYL_DEFAULT_T                  55
#define MS_STATE_LIGHT_XYL_RANGE_T                    56
#define MS_STATE_LIGHT_LC_MODE_T                      57
#define MS_STATE_LIGHT_LC_OM_T                        58
#define MS_STATE_LIGHT_LC_LIGHT_ONOFF_T               59
#define MS_STATE_LIGHT_LC_PROPERTY_ID_T               60
#define MS_STATE_LIGHT_LC_PROPERTY_T                  61

#define MS_STATE_GENERIC_USER_PROPERTY_IDS_T          62
#define MS_STATE_GENERIC_ADMIN_PROPERTY_IDS_T         63
#define MS_STATE_GENERIC_MANUFACTURER_PROPERTY_IDS_T  64
#define MS_STATE_GENERIC_CLIENT_PROPERTY_IDS_T        65

#define MS_STATE_SCENE_STORE_T                        66
#define MS_STATE_SCENE_RECALL_T                       67
#define MS_STATE_SCENE_DELETE_T                       68

#define MS_STATE_DELTA_LEVEL_T                        69
#define MS_STATE_MOVE_LEVEL_T                         70

/* Additional supporting structure type defines */
#define MS_EXT_TID_AND_TRANSITION_STRUCT_T            128
#define MS_EXT_STATUS_STRUCT_T                        129
#define MS_STATE_SENSOR_SERIES_T                      130

/**
    Generic OnOff state is a Boolean value that represents the state of an
    element
*/
typedef struct MS_state_generic_onoff_struct
{
    /** Generic On/Off */
    UINT8  onoff;

    /** Target On/Off - Used in response path */
    UINT8  target_onoff;

    /** Last On/Off */
    UINT8  last_onoff;

    /** TID - Used in request path */
    UINT8 tid;

    /**
        Transition Time - Used in request path.
        Used as remaining time in response path.
    */
    UINT8 transition_time;

    /** Delay - Used in request path */
    UINT8 delay;

    /** Transition Timer Handle */
    UINT16  transition_time_handle;

} MS_STATE_GENERIC_ONOFF_STRUCT;

/**
    Generic Level state is a 16-bit signed integer (2-s complement) representing
    the state of an element
*/
typedef struct MS_state_generic_level_struct
{
    /** Generic Level */
    UINT16  level;

    /** Delta Level */
    UINT32  delta_level;

    /** Move Level */
    UINT16  move_level;

    /* TID - Used in received path */
    UINT8   tid;

    /* Optional */
    /* Transition Time */
    UINT8   transition_time;

    /* Delay */
    UINT8   delay;

    /** Target Level */
    UINT16  target_level;

} MS_STATE_GENERIC_LEVEL_STRUCT;

/**
    Generic Default Transition Time state determines how long an element shall
    take to transition from a present state to a new state
*/
typedef struct MS_state_generic_default_transition_time_struct
{
    /** The number of Steps */
    UCHAR  default_transition_number_of_steps;

    /** The resolution of the Default Transition Number of Steps field */
    UCHAR  default_transition_step_resolution;
} MS_STATE_GENERIC_DEFAULT_TRANSITION_TIME_STRUCT;

/**
    Generic OnPowerUp state is an enumeration representing the behavior of an
    element when powered up
*/
typedef struct MS_state_generic_onpowerup_struct
{
    /** Generic OnPowerUp */
    UCHAR  onpowerup;
} MS_STATE_GENERIC_ONPOWERUP_STRUCT;

/**
    Generic Power Actual state determines the linear percentage of the maximum
    power level of an element, representing a range from 0 percent through 100
    percent
*/
typedef struct MS_state_generic_power_actual_struct
{
    /** Generic Power Actual */
    UINT16 power_actual;

    /* TID - Used only in request path */
    UINT8  tid;

    /** Generic Power Target - Used only in response path */
    UINT16 power_target;

    /**
        Transition Time - Used in request path
        Used as Remaining Time in response path
    */
    UINT8  transition_time;

    /* Delay - Used only in request path */
    UINT8  delay;

    /** Transition Timer Handle */
    UINT16  transition_time_handle;

} MS_STATE_GENERIC_POWER_ACTUAL_STRUCT;

/**
    Generic Power Last state is a 16-bit value representing a percentage ranging
    from (1/65535) percent to 100 percent
*/
typedef struct MS_state_generic_power_last_struct
{
    /** Generic Power Last */
    UINT16 power_last;
} MS_STATE_GENERIC_POWER_LAST_STRUCT;

/** Generic Power Default state is a 16-bit value ranging from 0 through 65535 */
typedef struct MS_state_generic_power_default_struct
{
    /** Generic Power Default */
    UINT16 power_default;
} MS_STATE_GENERIC_POWER_DEFAULT_STRUCT;

/**
    Generic Power Range state determines the minimum and maximum power levels of
    an element relative to the maximum power level an element can output
*/
typedef struct MS_state_generic_power_range_struct
{
    /** Generic Power Range - Minimum */
    UINT16 power_range_min;

    /** Generic Power Range - Maximum */
    UINT16 power_range_max;

    /** Status - Used only in the response path */
    UINT8  status;

} MS_STATE_GENERIC_POWER_RANGE_STRUCT;

/**
    Generic Power Level state is a composite state that includes a Generic Power
    Actual state, a Generic Power Last state, a Generic Power Default state, and
    a Generic Power Range state
*/
typedef struct MS_state_generic_power_level_struct
{
    /** Generic Power Actual */
    MS_STATE_GENERIC_POWER_ACTUAL_STRUCT  generic_power_actual;

    /** Generic Power Last */
    MS_STATE_GENERIC_POWER_LAST_STRUCT  generic_power_last;

    /** Generic Power Default */
    MS_STATE_GENERIC_POWER_DEFAULT_STRUCT  generic_power_default;

    /** Generic Power Range */
    MS_STATE_GENERIC_POWER_RANGE_STRUCT  generic_power_range;
} MS_STATE_GENERIC_POWER_LEVEL_STRUCT;

/**
    Generic Battery state is a set of four values representing the state of a
    battery
*/
typedef struct MS_state_generic_battery_struct
{
    /**
        Generic Battery Level state is a value ranging from 0 percent through 100
        percent
    */
    UCHAR  generic_battery_level;

    /**
        Generic Battery Time to Discharge state is a 24-bit unsigned value ranging
        from 0 through 0xFFFFFF
    */
    UINT32 generic_battery_time_to_discharge;

    /**
        Generic Battery Time to Charge state is a 24-bit unsigned value ranging from
        0 through 0xFFFFFF
    */
    UINT32 generic_battery_time_to_charge;

    /**
        Generic Battery Flags state is a concatenation of four 2-bit bit fields:
        Presence, Indicator, Charging, and Serviceability
    */
    UCHAR  generic_battery_flags;
} MS_STATE_GENERIC_BATTERY_STRUCT;

/** Generic Global Location state defines location information of an element */
typedef struct MS_state_generic_location_global_struct
{
    /** Global Coordinates (Latitude) */
    UINT32 global_latitude;

    /** Global Coordinates (Longitude) */
    UINT32 global_longitude;

    /** Global Altitude */
    UINT16 global_altitude;
} MS_STATE_GENERIC_LOCATION_GLOBAL_STRUCT;

/** Generic Local Location state defines location information of an element */
typedef struct MS_state_generic_location_local_struct
{
    /** Local Coordinates (North) */
    UINT16 local_north;

    /** Local Coordinates (East) */
    UINT16 local_east;

    /** Local Altitude */
    UINT16 local_altitude;

    /** Floor Number */
    UCHAR  floor_number;

    /** Uncertainty */
    UINT16 uncertainty;
} MS_STATE_GENERIC_LOCATION_LOCAL_STRUCT;

/**
    Generic Location state is a composite state that includes a Generic Location
    Global state and a Generic Location Local state
*/
typedef struct MS_state_generic_location_struct
{
    /** Global Location */
    MS_STATE_GENERIC_LOCATION_GLOBAL_STRUCT global_location;

    /** Local Location */
    MS_STATE_GENERIC_LOCATION_LOCAL_STRUCT local_location;
} MS_STATE_GENERIC_LOCATION_STRUCT;


/**
    Generic Property is a state representing a device property of an element.
    The properties can be one of the following
    - Manufacturer Properties
    - Admin Properties
    - User Properties
*/
typedef struct MS_state_generic_property_struct
{
    /**
        User Property ID field is a 2-octet Assigned Number value referencing a
        property
    */
    UINT16 property_id;

    /** Property Type - Manufacturer/Admin/User */
    UINT8  property_type;

    /**
        User Access field is an enumeration indicating whether the device property
        can be read or written as a Generic Admin/User Property
    */
    UCHAR  access;

    /** User Property Value field is a conditional field */
    UCHAR* property_value;
    UINT16 property_value_len;

} MS_STATE_GENERIC_PROPERTY_STRUCT;


/** Generic User Property is a state representing a device property of an element */
typedef struct MS_state_generic_user_property_struct
{
    /**
        User Property ID field is a 2-octet Assigned Number value referencing a
        device property
    */
    UINT16 property_id;

    /**
        User Access field is an enumeration indicating whether the device property
        can be read or written as a Generic User Property
    */
    UCHAR  user_access;

    /** User Property Value field is a conditional field */
    UCHAR* property_value;
    UINT16 property_value_len;
} MS_STATE_GENERIC_USER_PROPERTY_STRUCT;

/**
    Generic Admin Property is a state representing a device property of an
    element that can be read or written
*/
typedef struct MS_state_generic_admin_property_struct
{
    /**
        Admin Property ID field is a 2-octet Assigned Number value referencing a
        device property
    */
    UINT16 property_id;

    /**
        Admin User Access field is an enumeration indicating whether the device
        property can be read or written as a Generic User Property
    */
    UCHAR  user_access;

    /** Admin Property Value field is a conditional field */
    UCHAR* property_value;
    UINT16  property_value_len;
} MS_STATE_GENERIC_ADMIN_PROPERTY_STRUCT;

/**
    Generic Manufacturer Property is a state representing a device property of an
    element that is programmed by a manufacturer and can be read
*/
typedef struct MS_state_generic_manufacturer_property_struct
{
    /**
        Manufacturer Property ID field is a 2-octet Assigned Number value that
        references a device property
    */
    UINT16 property_id;

    /**
        Manufacturer User Access field is an enumeration indicating whether or not
        the device property can be read as a Generic User Property
    */
    UCHAR  user_access;

    /** Manufacturer Property Value field is a conditional field */
    UCHAR* property_value;
    UINT16  property_value_len;
} MS_STATE_GENERIC_MANUFACTURER_PROPERTY_STRUCT;

/**
    Generic Property ID a read-only state representing a device property that an
    element supports
*/
typedef struct MS_state_generic_property_id_struct
{
    /**
        Property ID field is a 2-octet Assigned Number value that references a device
        property
    */
    UINT16 property_id;
} MS_STATE_GENERIC_PROPERTY_ID_STRUCT;

/**
    Generic Property IDs a state representing a set of device properties that an
    element supports
*/
typedef struct MS_state_generic_property_ids_struct
{
    /**
        Property IDs field is a set of 2-octet Assigned Number value that references
        a set of device properties
    */
    UINT16* property_ids;
    UINT16 property_ids_count;
} MS_STATE_GENERIC_PROPERTY_IDS_STRUCT;

/**
    Sensor Property ID field is a 2-octet value referencing a device property
    that describes the meaning and the format of data reported by a sensor
*/
typedef struct MS_state_sensor_property_id_struct
{
    /**
        Sensor Property ID field is a 2-octet value referencing a device property
        that describes the meaning and the format of data reported by a sensor
    */
    UINT16 property_id;
} MS_STATE_SENSOR_PROPERTY_ID_STRUCT;

/** Sensor Descriptor state represents the attributes describing the sensor data */
typedef struct MS_state_sensor_descriptor_struct
{
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

    /** Status - used in response to indicate if other fields to be included */
    UINT8  status;

} MS_STATE_SENSOR_DESCRIPTOR_STRUCT;

/** Sensor Settings state controls parameters of a sensor */
typedef struct MS_state_sensor_settings_struct
{
    /** Property ID of a sensor */
    UINT16 sensor_property_id;

    /** Property ID of a setting within a sensor */
    UINT16* setting_property_ids;
    UINT16 setting_property_ids_count;

} MS_STATE_SENSOR_SETTINGS_STRUCT;

/** Sensor Setting state controls parameters of a sensor */
typedef struct MS_state_sensor_setting_struct
{
    /** Property ID of a sensor */
    UINT16 sensor_property_id;

    /** Property ID of a setting within a sensor */
    UINT16 sensor_setting_property_id;

    /** Read/Write access rights for the setting */
    UCHAR  sensor_setting_access;

    /** Raw value of a setting within a sensor */
    UCHAR* sensor_setting_raw;
    UINT16 sensor_setting_raw_len;

    /* Status - used in response path */
    UINT8  status;

} MS_STATE_SENSOR_SETTING_STRUCT;

/** Sensor Cadence state controls the cadence of sensor reports */
typedef struct MS_state_sensor_cadence_struct
{
    /** Property ID of a sensor */
    UINT16 sensor_property_id;

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

    /** Status - used in response path */
    UINT8  status;

} MS_STATE_SENSOR_CADENCE_STRUCT;

/**
    The Sensor Data state is a sequence of one or more pairs of Sensor Property
    ID and Raw Value fields, with each Raw Value field size and representation
    defined by the characteristics referenced by the Sensor Property ID
*/
typedef struct MS_state_sensor_data_struct
{
    /** ID of the 1st device property of the sensor */
    UINT16 property_id_1;

    /**
        Raw Value field with a size and representation defined by the 1st device
        property
    */
    UCHAR* raw_value_1;
    UINT16 raw_value_1_len;

    /** ID of the 2nd device property of the sensor */
    UINT16 property_id_2;

    /**
        Raw Value field with a size and representation defined by the 2nd device
        property
    */
    UCHAR* raw_value_2;
    UINT16 raw_value_2_len;

    /** ID of the nth device property of the sensor */
    UINT16 property_id_n;

    /**
        Raw Value field with a size and representation defined by the nth device
        property
    */
    UCHAR* raw_value_n;
    UINT16 raw_value_n_len;

    /** Status - used in response path */
    UINT8  status;

} MS_STATE_SENSOR_DATA_STRUCT;

/**
    Values measured by sensors may be organized as arrays (and represented as
    series of columns, such as histograms
*/
typedef struct MS_state_sensor_series_column_struct
{
    /** Property describing the data series of the sensor */
    UINT16 sensor_property_id;

    /** Raw value representing the left corner of a column on the X axis */
    UCHAR* sensor_raw_value_x;
    UINT16 sensor_raw_value_x_len;

    /** Raw value representing the width of the column */
    UCHAR* sensor_column_width;
    UINT16 sensor_column_width_len;

    /** Raw value representing the height of the column on the Y axis */
    UCHAR* sensor_raw_value_y;
    UINT16 sensor_raw_value_y_len;

    /** Status - used in response path */
    UINT8  status;

} MS_STATE_SENSOR_SERIES_COLUMN_STRUCT;

/**
    Mesh defines times based on International Atomic Time (TAI). The base
    representation of times is the number of seconds after 00:00:00 TAI on
    2000-01-01 (that is, 1999-12-31T23:59:28 UTC)
*/
typedef struct MS_state_time_struct
{
    /** Current TAI time in seconds since the epoch. */
    UCHAR  tai_seconds[5];

    /** The sub-second time in units of 1/256s. */
    UCHAR  subsecond;

    /** Estimated uncertainty in 10-millisecond steps. */
    UCHAR  uncertainty;

    /**
        0 = No Time Authority. The element does not have a trusted OOB source of
        time, such as GPS or NTP. 1 = Time Authority. The element has a trusted OOB
        source of time, such as GPS or NTP or a battery-backed, properly initialized
        RTC.
    */
    UCHAR  time_authority;

    /** Current difference between TAI and UTC in seconds */
    UINT16 tai_utc_delta;

    /** The local time zone offset in 15-minute increments */
    UCHAR  time_zone_offset;
} MS_STATE_TIME_STRUCT;

/** Time Zone */
typedef struct MS_state_time_zone_struct
{
    /**
        Current local time zone offset.
        Meaningful only in 'Time Zone Status' response.
    */
    UCHAR  time_zone_offset_current;

    /** Upcoming local time zone offset. */
    UCHAR  time_zone_offset_new;

    /** Absolute TAI time when the Time Zone Offset will change from Current to New. */
    UCHAR  tai_of_zone_change[5];
} MS_STATE_TIME_ZONE_STRUCT;

/** TAI-UTC Delta */
typedef struct MS_state_time_tai_utc_delta_struct
{
    /**
        Current difference between TAI and UTC in seconds.
        Meaningful only in 'TAI-UTC Delta Status' response.
    */
    UINT16 tai_utc_delta_current;

    /** Upcoming difference between TAI and UTC in seconds. */
    UINT16 tai_utc_delta_new;

    /** Always 0b0. Other values are Prohibited. */
    UCHAR  padding;

    /** TAI Seconds time of the upcoming TAI-UTC Delta change */
    UCHAR  tai_of_delta_change[5];
} MS_STATE_TIME_TAI_UTC_DELTA_STRUCT;

/** The Time Role state of an element */
typedef struct MS_state_time_role_struct
{
    /** Time Role */
    UCHAR  role;
} MS_STATE_TIME_ROLE_STRUCT;

/** The state to identify a scene */
typedef struct MS_state_scene_number_struct
{
    /** The number to identify a scene */
    UINT16 number;
} MS_STATE_SCENE_NUMBER_STRUCT;

/** The current status of a currently active scene */
typedef struct MS_state_scene_status_struct
{
    /** Status Code */
    UCHAR  status_code;

    /** Scene Number of a current scene. */
    UINT16 current_scene;

    /** Scene Number of a target scene. */
    UINT16 target_scene;

    /* Remaining Time */
    UINT8  remaining_time;

} MS_STATE_SCENE_STATUS_STRUCT;

/** The current status of scene register */
typedef struct MS_state_scene_register_status_struct
{
    /** Status Code */
    UCHAR  status_code;

    /** Scene Number of a current scene. */
    UINT16 current_scene;

    /** A list of scenes stored within an element */
    UINT16* scenes;
    UINT16 scenes_count;
} MS_STATE_SCENE_REGISTER_STATUS_STRUCT;

/** The current Schedule Register state of an element. */
typedef struct MS_state_scheduler_schedules_struct
{
    /** Bit field indicating defined Actions in the Schedule Register */
    UINT16 schedules;
} MS_STATE_SCHEDULER_SCHEDULES_STRUCT;

/** The entry index of the Schedule Register state */
typedef struct MS_state_scheduler_entry_index_struct
{
    /** Index of the Schedule Register entry */
    UCHAR  index;
} MS_STATE_SCHEDULER_ENTRY_INDEX_STRUCT;

/** The entry of the Schedule Register state */
typedef struct MS_state_scheduler_entry_struct
{
    /** Index of the Schedule Register entry */
    UCHAR  index;

    /** Scheduled year for the action */
    UCHAR  year;

    /** Scheduled month for the action */
    UINT16 month;

    /** Scheduled day of the month for the action */
    UCHAR  day;

    /** Scheduled hour for the action */
    UCHAR  hour;

    /** Scheduled minute for the action */
    UCHAR  minute;

    /** Scheduled second for the action */
    UCHAR  second;

    /** Schedule days of the week for the action */
    UCHAR  dayofweek;

    /** Action to be performed at the scheduled time */
    UCHAR  action;

    /** Transition time for this action */
    UCHAR  transition_time;

    /** Scene number to be used for some actions */
    UINT16 scene_number;

} MS_STATE_SCHEDULER_ENTRY_STRUCT;

/**
    Light Lightness Linear state represents the lightness of a light on a linear
    scale
*/
typedef struct MS_state_light_lightness_linear_struct
{
    /** Light Lightness Linear */
    UINT16 lightness_linear;

    /** Light Lightness Target - Used in response path. */
    UINT16 lightness_target;

    /** TID - Used in request path */
    UINT8  tid;

    /**
        Transition Time - Used in request path.
        Used as remaining time in response path.
    */
    UINT8 transition_time;

    /** Delay - Used in request path */
    UINT8  delay;

} MS_STATE_LIGHT_LIGHTNESS_LINEAR_STRUCT;

/**
    Light Lightness Actual state represents the lightness of a light on a
    perceptually uniform lightness scale
*/
typedef struct MS_state_light_lightness_actual_struct
{
    /** Light Lightness Actual */
    UINT16 lightness_actual;

    /** Light Lightness Target - Used in response path. */
    UINT16 lightness_target;

    /** TID - Used in request path */
    UINT8  tid;

    /**
        Transition Time - Used in request path.
        Used as remaining time in response path.
    */
    UINT8 transition_time;

    /** Delay - Used in request path */
    UINT8  delay;

    /** Transition Timer Handle */
    UINT16  transition_time_handle;

} MS_STATE_LIGHT_LIGHTNESS_ACTUAL_STRUCT;

/**
    Light Lightness Last state represents the lightness of a light on a
    perceptually uniform lightness scale
*/
typedef struct MS_state_light_lightness_last_struct
{
    /** Light Lightness Last */
    UINT16 lightness_last;
} MS_STATE_LIGHT_LIGHTNESS_LAST_STRUCT;

/**
    Light Lightness Default state is a value ranging from 0x0000 to 0xFFFF,
    representing a default lightness level for the Light Lightness Actual state
*/
typedef struct MS_state_light_lightness_default_struct
{
    /** Light Lightness Default */
    UINT16 lightness_default;
} MS_STATE_LIGHT_LIGHTNESS_DEFAULT_STRUCT;

/**
    Light Lightness Range state determines the minimum and maximum lightness of
    an element
*/
typedef struct MS_state_light_lightness_range_struct
{
    /** Light Lightness Range Min */
    UINT16 lightness_range_min;

    /** Light Lightness Range Max */
    UINT16 lightness_range_max;
} MS_STATE_LIGHT_LIGHTNESS_RANGE_STRUCT;

/**
    Light Lightness state is a composite state that includes the Light Lightness
    Linear, the Light Lightness Actual, the Light Lightness Last, and the Light
    Lightness Default states
*/
typedef struct MS_state_light_lightness_struct
{
    /**
        Light Lightness Linear state represents the lightness of a light on a linear
        scale
    */
    MS_STATE_LIGHT_LIGHTNESS_LINEAR_STRUCT  light_lightness_linear;

    /**
        Light Lightness Actual state represents the lightness of a light on a
        perceptually uniform lightness scale
    */
    MS_STATE_LIGHT_LIGHTNESS_ACTUAL_STRUCT  light_lightness_actual;

    /**
        Light Lightness Last state represents the lightness of a light on a
        perceptually uniform lightness scale
    */
    MS_STATE_LIGHT_LIGHTNESS_LAST_STRUCT  light_lightness_last;

    /**
        Light Lightness Default state is a value ranging from 0x0000 to 0xFFFF,
        representing a default lightness level for the Light Lightness Actual state
    */
    MS_STATE_LIGHT_LIGHTNESS_DEFAULT_STRUCT  light_lightness_default;

    /**
        Light Lightness Range state.
    */
    MS_STATE_LIGHT_LIGHTNESS_RANGE_STRUCT    light_lightness_range;

    /** Status field used only for the Range Status */
    UINT8                                    range_status;

} MS_STATE_LIGHT_LIGHTNESS_STRUCT;

/**
    Light CTL state is a composite state that includes the Light CTL Lightness,
    the Light CTL Temperature and the Light CTL Delta UV states
*/
typedef struct MS_state_light_ctl_struct
{
    /** Light CTL Lightness */
    UINT16 ctl_lightness;

    /** Target Light CTL Lightness - Used in response path */
    UINT16 target_ctl_lightness;

    /** Light CTL Temperature */
    UINT16 ctl_temperature;

    /** Terget Light CTL Temperature - Used in response path */
    UINT16 target_ctl_temperature;

    /** Light CTL Delta UV */
    UINT16 ctl_delta_uv;

    /** TID - Used in request path */
    UINT8  tid;

    /**
        Transition Time - Used in request path.
        Used as remaining time in response path.
    */
    UINT8 transition_time;

    /** Delay - Used in request path */
    UINT8  delay;

} MS_STATE_LIGHT_CTL_STRUCT;

/**
    Light CTL Default state is a composite state that includes the Light CTL
    Lightness, the Light CTL Temperature and the Light CTL Delta UV states
*/
typedef struct MS_state_light_ctl_default_struct
{
    /** Light CTL Lightness */
    UINT16 ctl_lightness;

    /** Light CTL Temperature */
    UINT16 ctl_temperature;

    /** Light CTL Delta UV */
    UINT16 ctl_delta_uv;
} MS_STATE_LIGHT_CTL_DEFAULT_STRUCT;

/**
    Light CTL Temperature state is a composite state that includes the Light CTL
    Temperature and the Light CTL Delta UV states
*/
typedef struct MS_state_light_ctl_temperature_struct
{
    /** Light CTL Temperature */
    UINT16 ctl_temperature;

    /** Target Light CTL Temperature - Used in response path */
    UINT16 target_ctl_temperature;

    /** Light CTL Delta UV */
    UINT16 ctl_delta_uv;

    /** Target Light CTL Delta UV - Used in response path */
    UINT16 target_ctl_delta_uv;

    /** TID - Used in request path */
    UINT8  tid;

    /**
        Transition Time - Used in request path.
        Used as remaining time in response path.
    */
    UINT8 transition_time;

    /** Delay - Used in request path */
    UINT8  delay;

} MS_STATE_LIGHT_CTL_TEMPERATURE_STRUCT;

/**
    Light CTL Temperature Range state determines the minimum and maximum color
    temperatures of tunable white light an element is capable of emitting
*/
typedef struct MS_state_light_ctl_temperature_range_struct
{
    /** CTL Temperature Range Min */
    UINT16 ctl_temperature_range_min;

    /** CTL Temperature Range Max */
    UINT16 ctl_temperature_range_max;

    /** Status - Used in response path */
    UINT8  status;

} MS_STATE_LIGHT_CTL_TEMPERATURE_RANGE_STRUCT;

/**
    Light HSL state is a composite state that includes the Light HSL Lighness,
    the Light HSL Hue and the Light HSL Saturation states
*/
typedef struct MS_state_light_hsl_struct
{
    /** The perceived lightness of a light emitted by the element */
    UINT16 hsl_lightness;

    /** Target Perceived lightness - used in the response path */
    UINT16 target_hsl_lightness;

    /** The 16-bit value representing the hue */
    UINT16 hsl_hue;

    /** Target hue - used in the response path */
    UINT16 target_hsl_hue;

    /** The saturation of a color light */
    UINT16 hsl_saturation;

    /** Target saturation - used in the response path */
    UINT16 target_hsl_saturation;

    /** TID - Used in request path */
    UINT8  tid;

    /**
        Transition Time - Used in request path.
        Used as remaining time in response path.
    */
    UINT8 transition_time;

    /** Delay - Used in request path */
    UINT8  delay;

    /** Transition Timer Handle */
    UINT16  transition_time_handle;

} MS_STATE_LIGHT_HSL_STRUCT;

/**
    Light HSL Target state is a composite state that includes the Light HSL
    Lighness, the Light HSL Hue and the Light HSL Saturation states
*/
typedef struct MS_state_light_hsl_target_struct
{
    /** The perceived lightness of a light emitted by the element */
    UINT16 hsl_lightness;

    /** The 16-bit value representing the hue */
    UINT16 hsl_hue;

    /** The saturation of a color light */
    UINT16 hsl_saturation;
} MS_STATE_LIGHT_HSL_TARGET_STRUCT;

/**
    Light HSL Default state is a composite state that includes the Light HSL
    Lighness, the Light HSL Hue and the Light HSL Saturation states
*/
typedef struct MS_state_light_hsl_default_struct
{
    /** The perceived lightness of a light emitted by the element */
    UINT16 hsl_lightness;

    /** The 16-bit value representing the hue */
    UINT16 hsl_hue;

    /** The saturation of a color light */
    UINT16 hsl_saturation;
} MS_STATE_LIGHT_HSL_DEFAULT_STRUCT;

/** Light HSL Hue */
typedef struct MS_state_light_hsl_hue_struct
{
    /** The 16-bit value representing the hue */
    UINT16 hsl_hue;
} MS_STATE_LIGHT_HSL_HUE_STRUCT;

/** Light HSL Saturation */
typedef struct MS_state_light_hsl_saturation_struct
{
    /** The saturation of a color light */
    UINT16 hsl_saturation;
} MS_STATE_LIGHT_HSL_SATURATION_STRUCT;

/**
    Light HSL Range state is a composite state that includes Minimum and Maximum
    of the Light HSL Hue and the Light HSL Saturation states
*/
typedef struct MS_state_light_hsl_range_struct
{
    /** The value of the Hue Range Min field of the Light HSL Hue Range state */
    UINT16 hue_range_min;

    /** The value of the Hue Range Max field of the Light HSL Hue Range state */
    UINT16 hue_range_max;

    /**
        The value of the Saturation Range Min field of the Light HSL Saturation Range
        state
    */
    UINT16 saturation_range_min;

    /**
        The value of the Saturation Range Max field of the Light HSL Saturation Range
        state
    */
    UINT16 saturation_range_max;

    /** Status - Used only in response path */
    UINT8  status;

} MS_STATE_LIGHT_HSL_RANGE_STRUCT;

/**
    Light xyL state is a composite state that includes the xyL Lightness, the
    Light xyL x and the Light xyL y states
*/
typedef struct MS_state_light_xyl_struct
{
    /** The perceived lightness of a light emitted by the element */
    UINT16 xyl_lightness;

    /** Target perceived lightness - used in response path */
    UINT16 target_xyl_lightness;

    /** The 16-bit value representing the x coordinate of a CIE1931 color light */
    UINT16 xyl_x;

    /** Target x coordinate - used in response path */
    UINT16 target_xyl_x;

    /** The 16-bit value representing the y coordinate of a CIE1931 color light */
    UINT16 xyl_y;

    /** Target y coordinate - used in response path */
    UINT16 target_xyl_y;

    /** TID - Used in request path */
    UINT8  tid;

    /**
        Transition Time - Used in request path.
        Used as remaining time in response path.
    */
    UINT8 transition_time;

    /** Delay - Used in request path */
    UINT8  delay;

    /** Transition Timer Handle */
    UINT16  transition_time_handle;

} MS_STATE_LIGHT_XYL_STRUCT;

/**
    Light xyL target state is a composite state that includes the xyL Lightness,
    the Light xyL x and the Light xyL y states
*/
typedef struct MS_state_light_xyl_target_struct
{
    /** The perceived lightness of a light emitted by the element */
    UINT16 xyl_lightness;

    /** The 16-bit value representing the x coordinate of a CIE1931 color light */
    UINT16 xyl_x;

    /** The 16-bit value representing the y coordinate of a CIE1931 color light */
    UINT16 xyl_y;
} MS_STATE_LIGHT_XYL_TARGET_STRUCT;

/**
    Light xyL default state is a composite state that includes the xyL Lightness,
    the Light xyL x and the Light xyL y states
*/
typedef struct MS_state_light_xyl_default_struct
{
    /** The perceived lightness of a light emitted by the element */
    UINT16 xyl_lightness;

    /** The 16-bit value representing the x coordinate of a CIE1931 color light */
    UINT16 xyl_x;

    /** The 16-bit value representing the y coordinate of a CIE1931 color light */
    UINT16 xyl_y;
} MS_STATE_LIGHT_XYL_DEFAULT_STRUCT;

/**
    Light xyL Range state determines the minimum and maximum values of the Light
    xyL x and syL y state of an element
*/
typedef struct MS_state_light_xyl_range_struct
{
    /** The minimum value of a Light xyL x state of an element */
    UINT16 xyl_x_range_min;

    /** The maximum value of a Light xyL x state of an element */
    UINT16 xyl_x_range_max;

    /** The minimum  value of a Light xyL y state of an element */
    UINT16 xyl_y_range_min;

    /** The maximum  value of a Light xyL y state of an element */
    UINT16 xyl_y_range_max;

    /** Status - Used in the response path */
    UINT8  status;

} MS_STATE_LIGHT_XYL_RANGE_STRUCT;

/** Light LC Mode state */
typedef struct MS_state_light_lc_mode_struct
{
    /** Light LC Mode state - present */
    UCHAR  present_mode;

    /** Light LC Mode state - target */
    UCHAR  target_mode;

} MS_STATE_LIGHT_LC_MODE_STRUCT;

/** Light LC Occupancy Mode state */
typedef struct MS_state_light_lc_om_struct
{
    /** Light LC Occupancy Mode state - present */
    UCHAR  present_mode;

    /** Light LC Occupancy Mode state - target */
    UCHAR  target_mode;

} MS_STATE_LIGHT_LC_OM_STRUCT;

/** Light LC Light OnOff State */
typedef struct MS_state_light_lc_light_onoff_struct
{
    /** Light LC Light OnOff State */
    UCHAR  present_light_onoff;

    /** Light LC Light OnOff State */
    UCHAR  target_light_onoff;

    /** TID - Used in request path */
    UINT8  tid;

    /**
        Transition Time - Used in request path.
        Used as remaining time in response path.
    */
    UINT8 transition_time;

    /** Delay - Used in request path */
    UINT8  delay;

} MS_STATE_LIGHT_LC_LIGHT_ONOFF_STRUCT;

/** Property ID identifying a Light LC Property */
typedef struct MS_state_light_lc_property_id_struct
{
    /** Property ID identifying a Light LC Property */
    UINT16 property_id;
} MS_STATE_LIGHT_LC_PROPERTY_ID_STRUCT;

/** Light LC Property state */
typedef struct MS_state_light_lc_property_struct
{
    /** Property ID identifying a Light LC Property */
    UINT16 property_id;

    /** Raw value for the Light LC Property */
    UCHAR* property_value;
    UINT16 property_value_len;
} MS_STATE_LIGHT_LC_PROPERTY_STRUCT;


/* Additional supporting structure defines */

/**
    TID and Transition is a structure which contains Transaction ID (TID) as mandatory field.
    Other two fields, Transition Time and Delay are optional.

    TID field is a transaction identifier indicating whether the message is a new message or
    a retransmission of a previously sent message.

    If present, the Transition Time field identifies the time that an element will take
    to transition to the target state from the present state.

    The Delay field shall be present when the Transition Time field is present.
    It identifies the message execution delay, representing a time interval between receiving
    the message by a model and executing the associated model behaviors.
*/
typedef struct MS_ext_tid_and_transition_struct
{
    UCHAR  tid;
    UCHAR  transition_time;
    UCHAR  delay;
    UCHAR  optional_fields_present;

} MS_EXT_TID_AND_TRANSITION_STRUCT;

/**
    The Status Code field identifies the Status Code for the last operation.
*/
typedef struct MS_ext_status_struct
{
    UCHAR  status;

} MS_EXT_STATUS_STRUCT;

/* State Transition data structure */
typedef struct _MS_ACCESS_STATE_TRANSITION_TYPE
{
    /* Transition Timer */
    EM_timer_handle  transition_timer_handle;

    /* Transition State. Initial/delay/transition */
    UINT8 transition_state;

    /* Delay */
    UINT8 delay;

    /* Transition Time */
    UINT8 transition_time;

    /* Transition Start Callback */
    void (* transition_start_cb)(void*);

    /* Transition Complete Callback */
    void (* transition_complete_cb)(void*);

    /* Blob/Context */
    void* blob;

} MS_ACCESS_STATE_TRANSITION_TYPE;

/* --------------------------------------------- Function */
#endif /*_H_MS_MODEL_STATES_ */
