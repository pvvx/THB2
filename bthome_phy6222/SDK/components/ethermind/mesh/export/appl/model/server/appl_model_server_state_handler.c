/**
    \file appl_model_server_state_handler.c

    \brief
    This file contains function, which handles model state set/get operations.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

/* --------------------------------------------- Header File Inclusion */
#include "MS_common.h"
#include "MS_generic_property_api.h"

#include "appl_model_state_handler.h"
#include "model_state_handler_pl.h"

#ifdef MS_STORAGE
    #include "nvsto.h"
#endif /* MS_STORAGE */

/* -------------------------------------- External Functions */
extern void appl_generic_power_level_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
);
extern void appl_generic_level_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
);
extern void appl_generic_onoff_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
);
extern void appl_generic_power_onoff_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
);
extern void appl_light_lightness_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
);
extern void appl_light_ctl_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
);
extern void appl_light_ctl_temperature_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
);
extern void appl_light_hsl_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
);
extern void appl_light_hsl_hue_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
);
extern void appl_light_hsl_saturation_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
);
extern void appl_light_xyl_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
);
extern void appl_light_lc_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst,
    /* IN */  UINT16                   property_id
);

extern void appl_generic_admin_property_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst,
    /* IN */  UINT16                   property_id
);
extern void appl_generic_manufacturer_property_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst,
    /* IN */  UINT16                   property_id
);
/* --------------------------------------------- Global Definitions */
API_RESULT appl_model_light_lc_server_set_default_trans_timeout_in_ms(/* IN */ UINT32 time_in_ms);

/* --------------------------------------------- Data Types/ Structures */
#define MS_MAX_NUM_STATES    2

/* States stored/restored as Scene */
typedef struct _APPL_STATE_INFO_VAULT
{
    /* Generic OnOff state */
    UINT8    onoff;

    /* Generic Level state */
    UINT16     level;

    /* Generic Power Level */
    UINT16     power_level;

    /* Light Lightness Actual */
    UINT16     lightness_actual;

    /* Light Lightness Linear */
    UINT16     lightness_linear;

    /** Light CTL Lightness */
    UINT16 ctl_lightness;

    /** Light CTL Temperature */
    UINT16 ctl_temperature;

    /** Light CTL Delta UV */
    UINT16 ctl_delta_uv;

    /** The perceived lightness of a light emitted by the element */
    UINT16 hsl_lightness;

    /** The 16-bit value representing the hue */
    UINT16 hsl_hue;

    /** The saturation of a color light */
    UINT16 hsl_saturation;

    /** The perceived lightness of a light emitted by the element */
    UINT16 xyl_lightness;

    /** The 16-bit value representing the x coordinate of a CIE1931 color light */
    UINT16 xyl_x;

    /** The 16-bit value representing the y coordinate of a CIE1931 color light */
    UINT16 xyl_y;

    /* LC Mode */
    UINT8    lc_mode;

    /* LC Property */
    MS_STATE_LIGHT_LC_PROPERTY_STRUCT lc_property;

} APPL_STATE_INFO_VAULT;

typedef struct _APPL_GENENRIC_LEVEL_INFO
{
    MS_STATE_GENERIC_LEVEL_STRUCT generic_level;

    /* Operation Type : 0xFF as initial value (invalid) */
    UINT8    operation_type;

    UINT16   transition_time_handle;

} APPL_GENERIC_LEVEL_INFO;

/** -- Generics - OnOff State */
#define APPL_GENERIC_ONOFF_SET(inst, OnOff) \
    appl_generic_onoff[(inst)].onoff = (OnOff); \
    appl_generic_onoff[(inst)].last_onoff = (OnOff)

static MS_STATE_GENERIC_ONOFF_STRUCT appl_generic_onoff[MS_MAX_NUM_STATES];

/** -- Generics - Level State */
static APPL_GENERIC_LEVEL_INFO appl_generic_level_info[MS_MAX_NUM_STATES];

/** -- Generics - Power OnOff State */
static MS_STATE_GENERIC_ONPOWERUP_STRUCT appl_generic_onpower[MS_MAX_NUM_STATES];

/** -- Generics - Power Level State */
static MS_STATE_GENERIC_POWER_LEVEL_STRUCT appl_generic_power_level[MS_MAX_NUM_STATES];

/** -- Generics - Battery */
static MS_STATE_GENERIC_BATTERY_STRUCT appl_generic_battery[MS_MAX_NUM_STATES];

/** -- Generics - Location */
static MS_STATE_GENERIC_LOCATION_STRUCT appl_generic_location[MS_MAX_NUM_STATES];

#define MS_MAX_CLIENT_PROPERTIES          1 /* 3 */

/* Example codebase. One Property each for Manufacturer, Admin and User */
#define MS_MAX_PROPERTIES                 3

static MS_STATE_GENERIC_PROPERTY_STRUCT appl_generic_property[MS_MAX_NUM_STATES][MS_MAX_PROPERTIES];

static MS_STATE_GENERIC_USER_PROPERTY_STRUCT appl_generic_client_property[MS_MAX_NUM_STATES][MS_MAX_CLIENT_PROPERTIES];

/** -- Light - Lightness */
static MS_STATE_LIGHT_LIGHTNESS_STRUCT appl_light_lightness[MS_MAX_NUM_STATES];

/** -- Light - CTL */
#define LIGHT_CTL_TEMPERATURE_T_MIN  0x0320
#define LIGHT_CTL_TEMPERATURE_T_MAX  0x4E20

static MS_STATE_LIGHT_CTL_STRUCT appl_light_ctl[MS_MAX_NUM_STATES];
static MS_STATE_LIGHT_CTL_TEMPERATURE_RANGE_STRUCT appl_light_ctl_temperature_range[MS_MAX_NUM_STATES];
static MS_STATE_LIGHT_CTL_DEFAULT_STRUCT appl_light_ctl_default[MS_MAX_NUM_STATES];
static MS_STATE_LIGHT_CTL_TEMPERATURE_STRUCT appl_light_ctl_temperature[MS_MAX_NUM_STATES];

/** -- Light - HSL */
static MS_STATE_LIGHT_HSL_STRUCT appl_light_hsl[MS_MAX_NUM_STATES];
static MS_STATE_LIGHT_HSL_RANGE_STRUCT appl_light_hsl_range[MS_MAX_NUM_STATES];
static MS_STATE_LIGHT_HSL_DEFAULT_STRUCT appl_light_hsl_default[MS_MAX_NUM_STATES];

/** -- Light - xyL */
static MS_STATE_LIGHT_XYL_STRUCT appl_light_xyl[MS_MAX_NUM_STATES];
static MS_STATE_LIGHT_XYL_DEFAULT_STRUCT appl_light_xyl_default[MS_MAX_NUM_STATES];
static MS_STATE_LIGHT_XYL_RANGE_STRUCT appl_light_xyl_range[MS_MAX_NUM_STATES];

/** -- Light - LC */
#define MS_MAX_LC_PROPERTIES 10
static MS_STATE_LIGHT_LC_MODE_STRUCT appl_light_lc_mode[MS_MAX_NUM_STATES];
static MS_STATE_LIGHT_LC_OM_STRUCT appl_light_lc_om[MS_MAX_NUM_STATES];
static MS_STATE_LIGHT_LC_LIGHT_ONOFF_STRUCT appl_light_lc_onoff[MS_MAX_NUM_STATES];
static MS_STATE_LIGHT_LC_PROPERTY_STRUCT appl_light_lc_property[MS_MAX_NUM_STATES][MS_MAX_LC_PROPERTIES];
static UINT8       appl_current_property_id_index;

/* For simplicity keeping same number of state info, as the number of max scences */
/* Last one is to store Power OnOff related value */
static APPL_STATE_INFO_VAULT   appl_state_info_vault[16 + 1];
static UINT8 appl_in_transtion;

static void appl_light_lightness_set_actual(UINT16 state_inst, UINT16 actual, UCHAR forced_publish);
static void appl_light_ctl_lightness_set_actual(UINT16 state_inst, UINT16 lightness, UINT16 temperature, UINT16 delta_uv, UCHAR forced_publish);
static void appl_light_ctl_temp_set_actual(UINT16 state_inst, UINT16 actual, UINT16 delta_uv, UCHAR is_calculated);
static void appl_light_xyl_set_actual(UINT16 state_inst, UINT16 xyl_lightness, UINT16 xyl_x, UINT16 xyl_y);

static void appl_light_hsl_set_actual(UINT16 state_inst, UINT16 lightness, UINT16 hue, UINT16 saturation, UCHAR forced_publish);

static void appl_light_lc_set_actual(UINT16 state_inst, UINT16 state_t, UCHAR onoff, UCHAR mode);
static API_RESULT appl_light_lc_set_actual_propery(UINT16 state_inst, UINT16 property_id, UCHAR* property_value, UINT16 property_value_len);

static void appl_set_generic_onoff(UINT16 state_inst, UINT8 onoff, UINT8 forced_publish);
void appl_light_lightness_set_linear(UINT16 state_inst, UINT16 linear);

void* appl_scene_save_current_state(/* IN */ UINT32 scene_index);
void* appl_scene_delete_saved_state(/* IN */ UINT32 scene_index, /* IN */ void* context);
void* appl_scene_recall_saved_state(/* IN */ UINT32 scene_index, /* IN */ void* context);

#ifdef MS_STORAGE

    /** Maximum number of Application Records in Persistent Storage */
    #define MS_PS_APPL_MAX_RECORDS       2

    /** Bitmask for all Application PS Access Record types */
    #define MS_PS_APPL_ALL_RECORDS       ((1 << MS_PS_APPL_MAX_RECORDS) - 1)

    /** Persistet Storage Application Records (bitfield values) */
    #define MS_PS_RECORD_APPL_SCENE           0x00000001
    #define MS_PS_RECORD_APPL_ON_POWER        0x00000002

    static void appl_ps_init (void);
    static void appl_ps_store(/* IN */ UINT32 records);
    static void appl_ps_load(/* IN */ UINT32 records);
#endif /* MS_STORAGE */

/* --------------------------------------------- Function */
static API_RESULT appl_model_state_is_diff_in_range
(
    /* IN */ UINT32 value_a,
    /* IN */ UINT32 value_b,
    /* IN */ UINT32 range
)
{
    API_RESULT retval;
    UINT32     value_diff;
    retval = API_FAILURE;

    if (value_a < value_b)
    {
        value_diff = value_b - value_a;
    }
    else
    {
        value_diff = value_a - value_b;
    }

    if (value_diff <= range)
    {
        retval = API_SUCCESS;
    }

    return retval;
}

/** ---------------- Model Binding ---- */
static void appl_set_generic_level(UINT16 state_inst, UINT16 level, UINT8 propagate, UCHAR is_calculated);

/**
    Binding with Power Actual.

    Generic Power Actual = Generic Level + 32768
    Light Lightness Actual = Generic Level + 32768
    Light HSL Hue = Generic Level + 32768
    Light HSL Saturation = Generic Level + 32768

    Light CTL Temperature = T_MIN + (Generic Level + 32768) * (T_MAX - T_MIN) / 65535
*/
static void appl_set_generic_power_level_actual(UINT16 state_inst, UINT16 actual, UINT8 propagate)
{
    UINT16 temperature;
    CONSOLE_OUT("[Set Generic Power Level] Level: 0x%04X. Propagate: 0x%02X\n", actual, propagate);

    if (actual != appl_generic_power_level[state_inst].generic_power_actual.power_actual)
    {
        appl_generic_power_level[state_inst].generic_power_last.power_last = actual;
        appl_generic_power_level[state_inst].generic_power_actual.power_actual = actual;
        /* Publish State Change */
        appl_generic_power_level_server_publish(MS_STATE_GENERIC_POWER_ACTUAL_T, state_inst);
        appl_light_lightness_set_actual(state_inst, actual, MS_FALSE);
        /* Light HSL Lightness = Light Lightness Actual */
        appl_light_hsl[state_inst].hsl_lightness = actual;
        /* Light Lightness Linear = ((Actual)^2) / 65535 */
        /* appl_light_lightness[state_inst].light_lightness_linear.lightness_linear = ((actual * actual) + 65534) / 65535; */
        #if 0
        /* Light CTL Temperature = T_MIN + (Generic Level + 32768) * (T_MAX - T_MIN) / 65535 */
        appl_light_ctl_temperature[state_inst].ctl_temperature =
            appl_light_ctl_temperature_range[state_inst].ctl_temperature_range_min +
            ((actual) * (appl_light_ctl_temperature_range[state_inst].ctl_temperature_range_max -
                         appl_light_ctl_temperature_range[state_inst].ctl_temperature_range_min)) / 65535;
        appl_light_ctl[state_inst].ctl_lightness = actual;
        #else
        /* Light CTL Temperature = T_MIN + (Generic Level + 32768) * (T_MAX - T_MIN) / 65535 */
        temperature = LIGHT_CTL_TEMPERATURE_T_MIN + (((actual) * (LIGHT_CTL_TEMPERATURE_T_MAX - LIGHT_CTL_TEMPERATURE_T_MIN)) / 65535);
        appl_light_ctl_temp_set_actual
        (
            state_inst,
            temperature,
            appl_light_ctl[state_inst].ctl_delta_uv,
            MS_TRUE
        );
        #endif /* 0 */
        #if 0
        appl_light_hsl[state_inst].hsl_hue = actual;
        appl_light_hsl[state_inst].hsl_saturation = actual;
        #endif /* 0 */

        /* Binding with Generic OnOff */
        if (0x0000 == actual)
        {
            appl_set_generic_onoff(state_inst, 0x00, MS_TRUE);
        }
        else
        {
            appl_set_generic_onoff(state_inst, 0x01, MS_TRUE);
        }

        /* Set Power Actual and rest will be set */
        if (MS_TRUE == propagate)
        {
            appl_set_generic_level(state_inst, actual - 32768, MS_FALSE, MS_FALSE);
        }
    }
}

static void appl_set_generic_power_level_default(UINT16 state_inst, UINT16 actual, UINT8 propagate)
{
    CONSOLE_OUT("[Set Generic Power Level Default] Level: 0x%04X. Propagate: 0x%02X\n", actual, propagate);

    if (actual != appl_generic_power_level[state_inst].generic_power_default.power_default)
    {
        appl_generic_power_level[state_inst].generic_power_default.power_default = actual;
        appl_generic_power_level_server_publish(MS_STATE_GENERIC_POWER_DEFAULT_T, state_inst);
        /* Propagate to Generic Power Actual */
        /* appl_set_generic_power_level_actual(state_inst, actual, MS_TRUE); */
    }
}

/**
    Binding with Generic Level.

    Generic Power Actual = Generic Level + 32768
    Light Lightness Actual = Generic Level + 32768
    Light HSL Hue = Generic Level + 32768
    Light HSL Saturation = Generic Level + 32768

    Light CTL Temperature = T_MIN + (Generic Level + 32768) * (T_MAX - T_MIN) / 65535
*/
static void appl_set_generic_level(UINT16 state_inst, UINT16 level, UINT8 propagate, UCHAR is_calculated)
{
    CONSOLE_OUT("[Set Generic Level] New Level: 0x%04X. Propagate: 0x%02X\n", level, propagate);
    CONSOLE_OUT("[Set Generic Level] Current Level: 0x%04X.\n",
                appl_generic_level_info[state_inst].generic_level.level);

    if ((level == appl_generic_level_info[state_inst].generic_level.level) ||
            /* If calculated and difference is only one */
            ((MS_TRUE == is_calculated) && (API_SUCCESS == appl_model_state_is_diff_in_range(level, appl_generic_level_info[state_inst].generic_level.level, 0x05)))
       )
    {
    }
    else
    {
        appl_generic_level_info[state_inst].generic_level.level = level;
        appl_generic_level_info[state_inst].generic_level.target_level = 0;
        appl_generic_level_info[state_inst].generic_level.delta_level = 0;
        appl_generic_level_server_publish(MS_STATE_GENERIC_LEVEL_T, state_inst);

        /* Set Power Actual and rest will be set */
        if (MS_TRUE == propagate)
        {
            appl_set_generic_power_level_actual(state_inst, level + 32768, MS_FALSE);
        }
    }
}

/** --------------------- Generics - OnOff ---- */
/* Initialization */
static void appl_generic_onoff_states_initialization(void)
{
    UINT32 index;
    EM_mem_set(appl_generic_onoff, 0, sizeof(appl_generic_onoff));

    for (index = 0; index < MS_MAX_NUM_STATES; index++)
    {
        appl_generic_onoff[index].transition_time_handle = 0xFFFF;
    }
}

/* Transition Timer Handlers */
static void appl_generic_onoff_transition_start_cb(void* blob)
{
    /**
        Because binary states cannot support transitions, when changing to 0x01 (On),
        the Generic OnOff state shall change immediately when the transition starts,
        and when changing to 0x00, the state shall change when the transition finishes.
    */
    if (0 == appl_generic_onoff[0].onoff)
    {
        /* appl_generic_onoff[0].onoff = appl_generic_onoff[0].target_onoff; */
        appl_set_generic_onoff(0, appl_generic_onoff[0].target_onoff, MS_FALSE);
    }
}

static void appl_generic_onoff_transition_complete_cb(void* blob)
{
    CONSOLE_OUT("Generic OnOff Transition Complete Callback\n");
    /* State Transition Complete */
    appl_generic_onoff[0].transition_time_handle = 0xFFFF;
    appl_generic_onoff[0].transition_time = 0;
    appl_set_generic_onoff(0, appl_generic_onoff[0].target_onoff, MS_TRUE);
    appl_generic_onoff[0].target_onoff = 0;
}

/* Generic OnOff Model Get Handlers */
static API_RESULT appl_model_generic_onoff_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    retval = API_SUCCESS;

    switch(state_t)
    {
    case MS_STATE_GENERIC_ONOFF_T:
    {
        MS_STATE_GENERIC_ONOFF_STRUCT* param_p;
        API_RESULT                      ret;
        UINT8                           transition_time;
        param_p = (MS_STATE_GENERIC_ONOFF_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_generic_onoff[0];

        if (0 != param_p->transition_time)
        {
            ret = MS_common_get_remaining_transition_time
                  (
                      appl_generic_onoff[0].transition_time_handle,
                      &transition_time
                  );

            if (API_SUCCESS == ret)
            {
                param_p->transition_time = transition_time;
            }
        }

        CONSOLE_OUT("Generic OnOff State Get. Returning: 0x%02X\n", param_p->onoff);
    }
    break;

    default:
        break;
    }

    return retval;
}

/* Generic OnOff Model Set Handlers */
static API_RESULT appl_model_generic_onoff_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    retval = API_SUCCESS;

    switch (state_t)
    {
    case MS_STATE_GENERIC_ONOFF_T:
    {
        MS_STATE_GENERIC_ONOFF_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_ONOFF_STRUCT*)param;
        CONSOLE_OUT("Generic OnOff State Set: 0x%02X\n", param_p->onoff);

        /* Check if state transition is specified */
        if (0 != param_p->transition_time)
        {
            MS_ACCESS_STATE_TRANSITION_TYPE   transition;
            appl_generic_onoff[0].target_onoff = param_p->onoff;
            appl_generic_onoff[0].transition_time = param_p->transition_time;
            appl_generic_onoff[0].last_onoff = param_p->onoff;
            transition.delay = param_p->delay;
            transition.transition_time = param_p->transition_time;
            transition.blob = NULL;
            transition.transition_start_cb = appl_generic_onoff_transition_start_cb;
            transition.transition_complete_cb = appl_generic_onoff_transition_complete_cb;
            *param_p = appl_generic_onoff[0];
            MS_common_start_transition_timer
            (
                &transition,
                &appl_generic_onoff[0].transition_time_handle
            );
            /* Return value to indicate not sending status right now */
            retval = API_FAILURE;
        }
        else
        {
            /* Instantaneous Change */
            appl_set_generic_onoff(state_inst, param_p->onoff, MS_FALSE);
            *param_p = appl_generic_onoff[0];
        }

        CONSOLE_OUT("[state] current: 0x%02X\n", appl_generic_onoff[0].onoff);
        CONSOLE_OUT("[state] target: 0x%02X\n", appl_generic_onoff[0].target_onoff);
        CONSOLE_OUT("[state] remaining_time: 0x%02X\n", appl_generic_onoff[0].transition_time);
        /* Ignoring Instance and direction right now */
        /* Call platform specific handling for generic set */
        generic_onoff_set_pl (appl_generic_onoff[0].onoff);
    }
    break;

    default:
        break;
    }

    return retval;
}

/** --------------------- Generics - Level ---- */
/* Initialization */
static void appl_model_generic_level_states_initialization(void)
{
    UINT32 index;

    for (index = 0; index < MS_MAX_NUM_STATES; index++)
    {
        EM_mem_set(&appl_generic_level_info[index], 0, sizeof(APPL_GENERIC_LEVEL_INFO));
        appl_generic_level_info[index].operation_type = 0xFF;
        appl_generic_level_info[index].transition_time_handle = 0xFFFF;
    }
}

static void appl_set_delta_level(UINT16 state_inst, INT32 delta, UINT8 immediate)
{
    /* TODO: See if this to be stored */
    if (0x01 == immediate)
    {
        appl_set_generic_level(state_inst, (appl_generic_level_info[state_inst].generic_level.level + delta), MS_TRUE, MS_FALSE);
    }
    else if (0x00 == immediate)
    {
        appl_generic_level_info[0].generic_level.target_level = (appl_generic_level_info[state_inst].generic_level.level + delta);
        appl_generic_level_info[state_inst].generic_level.delta_level = delta;
    }
    else
    {
        /* Do nothing right now */
    }
}

static void appl_set_move_level(UINT16 state_inst, UINT16 move, UINT8 immediate)
{
    /* TODO: See if this to be stored */
    appl_generic_level_info[state_inst].generic_level.move_level = move;
}

/* Transition Timer Handlers */
static void appl_generic_level_transition_start_cb(void* blob)
{
    UINT16 state_t;
    state_t = (UINT16)((intptr_t)(blob));
    (void)state_t;
}

static void appl_generic_level_transition_complete_cb(void* blob)
{
    UINT16 state_t;
    state_t = (UINT16)((intptr_t)(blob));
    appl_generic_level_info[0].generic_level.transition_time = 0x00;
    appl_generic_level_info[0].transition_time_handle = 0xFFFF;

    switch (state_t)
    {
    case MS_STATE_GENERIC_LEVEL_T:
    {
        appl_set_generic_level(0, appl_generic_level_info[0].generic_level.target_level, MS_TRUE, MS_FALSE);
    }
    break;

    case MS_STATE_DELTA_LEVEL_T:
    {
        appl_set_generic_level(0, appl_generic_level_info[0].generic_level.target_level, MS_TRUE, MS_FALSE);
    }
    break;

    case MS_STATE_MOVE_LEVEL_T:
    {
        appl_set_move_level(0, appl_generic_level_info[0].generic_level.delta_level, 0x01);
        /* TODO: Remove Bad Logic */
        appl_generic_level_info[0].generic_level.move_level = 0;
    }
    break;
    }
}

static API_RESULT appl_model_generic_level_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    retval = API_SUCCESS;

    switch (state_t)
    {
    case MS_STATE_GENERIC_LEVEL_T:
    {
        MS_STATE_GENERIC_LEVEL_STRUCT* param_p;
        API_RESULT                      ret;
        UINT8                           transition_time;
        param_p = (MS_STATE_GENERIC_LEVEL_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_generic_level_info[0].generic_level;
        param_p->level = appl_generic_level_info[0].generic_level.level + appl_generic_level_info[0].generic_level.delta_level;

        if (0 != param_p->transition_time)
        {
            ret = MS_common_get_remaining_transition_time
                  (
                      appl_generic_level_info[0].transition_time_handle,
                      &transition_time
                  );

            if (API_SUCCESS == ret)
            {
                param_p->transition_time = transition_time;
            }
        }
    }
    break;

    default:
        break;
    }

    return retval;
}

static API_RESULT appl_model_generic_level_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    MS_STATE_GENERIC_LEVEL_STRUCT* param_p;
    /**
        Transaction State:
        0 : Start of Transaction
        1 : Continue
        2 : End of Transaction
    */
    UINT8                           transaction_state;
    param_p = (MS_STATE_GENERIC_LEVEL_STRUCT*)param;

    switch (state_t)
    {
    case MS_STATE_GENERIC_LEVEL_T:
    {
        /* Ignoring Instance and direction right now */
        /* TODO: Not handling transaction state */
        {
            MS_common_stop_transition_timer(appl_generic_level_info[0].transition_time_handle);
            appl_generic_level_info[0].transition_time_handle = 0xFFFF;
            appl_generic_level_info[0].generic_level.transition_time = 0x00;

            if (0 == param_p->transition_time)
            {
                appl_set_generic_level(0, param_p->level, MS_TRUE, MS_FALSE);
            }
            else
            {
                appl_generic_level_info[0].generic_level.target_level = param_p->level;
                param_p->target_level = param_p->level;
            }
        }
    }
    break;

    case MS_STATE_DELTA_LEVEL_T:
    {
        /* Check if a new transaction */
        if (appl_generic_level_info[0].operation_type != state_t)
        {
            transaction_state = 0x00;
            /* Stop associated Transaction Timer, if any */
            MS_common_stop_transition_timer(appl_generic_level_info[0].transition_time_handle);
            appl_generic_level_info[0].transition_time_handle = 0xFFFF;
            appl_generic_level_info[0].generic_level.transition_time = 0x00;
            appl_generic_level_info[0].operation_type = state_t;
            appl_generic_level_info[0].generic_level.tid = param_p->tid;
        }
        else if (param_p->tid == appl_generic_level_info[0].generic_level.tid)
        {
            transaction_state = 0x01;
        }
        else
        {
            transaction_state = 0x02;
            /* Stop associated Transaction Timer, if any */
            MS_common_stop_transition_timer(appl_generic_level_info[0].transition_time_handle);
            appl_generic_level_info[0].transition_time_handle = 0xFFFF;
            appl_generic_level_info[0].generic_level.transition_time = 0x00;
            appl_generic_level_info[0].operation_type = state_t;
            appl_generic_level_info[0].generic_level.tid = param_p->tid;
        }

        /* Ignoring Instance and direction right now */
        if (0x02 == transaction_state)
        {
            appl_set_delta_level(0, (INT32)appl_generic_level_info[0].generic_level.delta_level, 0x01);
        }

        /* Only update delta */
        appl_set_delta_level(0, (INT32)param_p->delta_level, 0x00);
    }
    break;

    case MS_STATE_MOVE_LEVEL_T:
    {
        /* Ignoring Instance and direction right now */
        if (0 == param_p->transition_time)
        {
            appl_set_move_level(0, param_p->move_level, 0x01);
        }
        else
        {
            appl_set_move_level(0, param_p->move_level, 0x00);
            appl_generic_level_info[0].generic_level.target_level = param_p->move_level * (param_p->transition_time & 0x3F);
            /* TODO: Hardcoding */
            /* appl_generic_level_info[0].generic_level.target_level = 0x7FFF; */
        }
    }
    break;

    default:
        break;
    }

    /* TODO: Do we need to check if this is a new transaction? */
    if (0 != param_p->transition_time)
    {
        MS_ACCESS_STATE_TRANSITION_TYPE   transition;
        appl_generic_level_info[0].generic_level.transition_time = param_p->transition_time;
        transition.delay = param_p->delay;
        transition.transition_time = param_p->transition_time;
        transition.blob = (void*)(intptr_t)state_t;
        transition.transition_start_cb = appl_generic_level_transition_start_cb;
        transition.transition_complete_cb = appl_generic_level_transition_complete_cb;
        MS_common_start_transition_timer
        (
            &transition,
            &appl_generic_level_info[0].transition_time_handle
        );
    }

    *param_p = appl_generic_level_info[0].generic_level;

    if (0 == param_p->transition_time)
    {
        param_p->level = appl_generic_level_info[0].generic_level.level + appl_generic_level_info[0].generic_level.delta_level;
    }

    return API_SUCCESS;
}

/** --------------------- Generics - Default Transition Time ---- */

/** --------------------- Generics - Power OnOff ---- */
static void appl_model_generic_power_onoff_states_initialization(void)
{
    EM_mem_set(appl_generic_onpower, 0, sizeof(appl_generic_onpower));
}

static API_RESULT appl_model_generic_power_onoff_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    retval = API_SUCCESS;

    switch(state_t)
    {
    case MS_STATE_GENERIC_ONPOWERUP_T:
    {
        MS_STATE_GENERIC_ONPOWERUP_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_ONPOWERUP_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_generic_onpower[0];
    }
    break;

    default:
        break;
    }

    return retval;
}

static API_RESULT appl_model_generic_power_onoff_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    retval = API_SUCCESS;

    switch (state_t)
    {
    case MS_STATE_GENERIC_ONPOWERUP_T:
    {
        MS_STATE_GENERIC_ONPOWERUP_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_ONPOWERUP_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_generic_onpower[0] = *param_p;
    }
    break;

    default:
        break;
    }

    return retval;
}

/** --------------------- Generics - Power Level ---- */
static void appl_model_power_level_states_initialization(void)
{
    EM_mem_set(appl_generic_power_level, 0, sizeof(appl_generic_power_level));
}

static void appl_power_level_set_actual(UINT16 state_inst, UINT16 actual)
{
    UINT16 min, max;
    min = appl_generic_power_level[state_inst].generic_power_range.power_range_min;
    max = appl_generic_power_level[state_inst].generic_power_range.power_range_max;

    if ((0 != min) && (actual < min))
    {
        actual = min;
    }
    else if ((0 != max) && (actual > max))
    {
        actual = max;
    }

    appl_set_generic_power_level_actual(state_inst, actual, MS_TRUE);
}

static void appl_generic_power_level_set_range(UINT16 state_inst, UINT16 min, UINT16 max)
{
    UINT16 actual;
    appl_generic_power_level[state_inst].generic_power_range.power_range_min = min;
    appl_generic_power_level[state_inst].generic_power_range.power_range_max = max;
    /* Check if actual to be updated */
    actual = appl_generic_power_level[state_inst].generic_power_actual.power_actual;
    appl_power_level_set_actual(state_inst, actual);
}

static void appl_generic_power_level_transition_start_cb(void* blob)
{
    /**
        Because binary states cannot support transitions, when changing to 0x01 (On),
        the Generic OnOff state shall change immediately when the transition starts,
        and when changing to 0x00, the state shall change when the transition finishes.
    */
    #if 0
    if (0 == appl_generic_onoff[0].onoff)
    {
        appl_generic_onoff[0].onoff = appl_generic_onoff[0].target_onoff;
    }

    #endif
}

static void appl_generic_power_level_transition_complete_cb(void* blob)
{
    /* State Transition Complete */
    appl_generic_power_level[0].generic_power_actual.transition_time = 0;
    appl_power_level_set_actual(0, appl_generic_power_level[0].generic_power_actual.power_target);
    appl_generic_power_level[0].generic_power_actual.power_target = 0;
}

static API_RESULT appl_model_generic_power_level_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    retval = API_SUCCESS;

    switch (state_t)
    {
    case MS_STATE_GENERIC_POWER_ACTUAL_T:
    {
        MS_STATE_GENERIC_POWER_ACTUAL_STRUCT* param_p;
        API_RESULT                      ret;
        UINT8                           transition_time;
        param_p = (MS_STATE_GENERIC_POWER_ACTUAL_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_generic_power_level[0].generic_power_actual;

        if (0 != param_p->transition_time)
        {
            ret = MS_common_get_remaining_transition_time
                  (
                      appl_generic_power_level[0].generic_power_actual.transition_time_handle,
                      &transition_time
                  );

            if (API_SUCCESS == ret)
            {
                param_p->transition_time = transition_time;
            }
        }
    }
    break;

    case MS_STATE_GENERIC_POWER_LAST_T:
    {
        MS_STATE_GENERIC_POWER_LAST_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_POWER_LAST_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_generic_power_level[0].generic_power_last;
    }
    break;

    case MS_STATE_GENERIC_POWER_DEFAULT_T:
    {
        MS_STATE_GENERIC_POWER_DEFAULT_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_POWER_DEFAULT_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_generic_power_level[0].generic_power_default;
    }
    break;

    case MS_STATE_GENERIC_POWER_RANGE_T:
    {
        MS_STATE_GENERIC_POWER_RANGE_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_POWER_RANGE_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_generic_power_level[0].generic_power_range;
    }
    break;

    default:
        break;
    }

    return retval;
}

static API_RESULT appl_model_generic_power_level_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    retval = API_SUCCESS;

    switch (state_t)
    {
    case MS_STATE_GENERIC_POWER_ACTUAL_T:
    {
        MS_STATE_GENERIC_POWER_ACTUAL_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_POWER_ACTUAL_STRUCT*)param;

        /* Check if state transition is specified */
        if (0 != param_p->transition_time)
        {
            MS_ACCESS_STATE_TRANSITION_TYPE   transition;
            appl_generic_power_level[state_inst].generic_power_actual.power_target = param_p->power_actual;
            appl_generic_power_level[state_inst].generic_power_actual.transition_time = param_p->transition_time;
            transition.delay = param_p->delay;
            transition.transition_time = param_p->transition_time;
            transition.blob = NULL;
            transition.transition_start_cb = appl_generic_power_level_transition_start_cb;
            transition.transition_complete_cb = appl_generic_power_level_transition_complete_cb;
            MS_common_start_transition_timer
            (
                &transition,
                &appl_generic_power_level[state_inst].generic_power_actual.transition_time_handle
            );
            /* Return value to indicate not sending status right now */
            retval = API_FAILURE;
        }
        else
        {
            /* Instantaneous Change */
            /* Ignoring Instance and direction right now */
            appl_power_level_set_actual(0, param_p->power_actual);
        }

        *param_p = appl_generic_power_level[0].generic_power_actual;
        CONSOLE_OUT("[state] current: 0x%02X\n", appl_generic_power_level[0].generic_power_actual.power_actual);
        CONSOLE_OUT("[state] target: 0x%02X\n", appl_generic_power_level[0].generic_power_actual.power_target);
        CONSOLE_OUT("[state] remaining_time: 0x%02X\n", appl_generic_power_level[0].generic_power_actual.transition_time);
    }
    break;

    case MS_STATE_GENERIC_POWER_DEFAULT_T:
    {
        MS_STATE_GENERIC_POWER_DEFAULT_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_POWER_DEFAULT_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_set_generic_power_level_default(0, param_p->power_default, MS_TRUE);
    }
    break;

    case MS_STATE_GENERIC_POWER_RANGE_T:
    {
        MS_STATE_GENERIC_POWER_RANGE_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_POWER_RANGE_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_generic_power_level_set_range(0, param_p->power_range_min, param_p->power_range_max);
        param_p->status = 0x00;
    }
    break;

    default:
        break;
    }

    return retval;
}

/** --------------------- Generics - Battery ---- */
static void appl_model_generic_battery_states_initialization(void)
{
    EM_mem_set(appl_generic_battery, 0, sizeof(appl_generic_battery));
}

static API_RESULT appl_model_generic_battery_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    switch (state_t)
    {
    case MS_STATE_GENERIC_BATTERY_T:
    {
        MS_STATE_GENERIC_BATTERY_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_BATTERY_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_generic_battery[0];
    }
    break;

    default:
        break;
    }

    return API_SUCCESS;
}

static API_RESULT appl_model_generic_battery_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    switch (state_t)
    {
    case MS_STATE_GENERIC_BATTERY_T:
    {
        MS_STATE_GENERIC_BATTERY_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_BATTERY_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_generic_battery[0] = *param_p;
    }
    break;

    default:
        break;
    }

    return API_SUCCESS;
}

/** --------------------- Generics - Location ---- */
static void appl_model_generic_location_states_initialization(void)
{
    EM_mem_set (appl_generic_location, 0, sizeof(appl_generic_location));
}

static API_RESULT appl_model_generic_location_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    switch(state_t)
    {
    case MS_STATE_GENERIC_LOCATION_GLOBAL_T:
    {
        MS_STATE_GENERIC_LOCATION_GLOBAL_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_LOCATION_GLOBAL_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_generic_location[0].global_location;
    }
    break;

    case MS_STATE_GENERIC_LOCATION_LOCAL_T:
    {
        MS_STATE_GENERIC_LOCATION_LOCAL_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_LOCATION_LOCAL_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_generic_location[0].local_location;
    }
    break;

    default:
        break;
    }

    return API_SUCCESS;
}

static API_RESULT appl_model_generic_location_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    switch(state_t)
    {
    case MS_STATE_GENERIC_LOCATION_GLOBAL_T:
    {
        MS_STATE_GENERIC_LOCATION_GLOBAL_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_LOCATION_GLOBAL_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_generic_location[0].global_location = *param_p;
    }
    break;

    case MS_STATE_GENERIC_LOCATION_LOCAL_T:
    {
        MS_STATE_GENERIC_LOCATION_LOCAL_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_LOCATION_LOCAL_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_generic_location[0].local_location = *param_p;
    }
    break;

    default:
        break;
    }

    return API_SUCCESS;
}

/** --------------------- Generics - Property ---- */
static void appl_model_generic_property_states_initialization(void)
{
    UINT32 index;
    UINT8  access_type;
    EM_mem_set(appl_generic_property, 0, sizeof(appl_generic_property));
    /* One Manufacturer Property */
    index = 0;
    /* Motion Sensed */
    appl_generic_property[0][index].property_id = MS_DEV_PROP_MOTION_SENSED;
    appl_generic_property[0][index].property_type = MS_GENERIC_PROP_TYPE_MANUFACTURER;
    appl_generic_property[0][index].access = MS_GENERIC_USER_ACCESS_READ;
    appl_generic_property[0][index].property_value_len = 1;
    appl_generic_property[0][index].property_value = EM_alloc_mem(appl_generic_property[0][index].property_value_len);
    EM_mem_set(appl_generic_property[0][index].property_value, (index + 1), appl_generic_property[0][index].property_value_len);
    /* One Admin Property */
    index ++;
    /* People Count */
    appl_generic_property[0][index].property_id = MS_DEV_PROP_PEOPLE_COUNT;
    appl_generic_property[0][index].property_type = MS_GENERIC_PROP_TYPE_ADMIN;
    appl_generic_property[0][index].access = MS_GENERIC_USER_ACCESS_READ;
    appl_generic_property[0][index].property_value_len = 1;
    appl_generic_property[0][index].property_value = EM_alloc_mem(appl_generic_property[0][index].property_value_len);
    EM_mem_set(appl_generic_property[0][index].property_value, (index + 1), appl_generic_property[0][index].property_value_len);
    /* One USer Property */
    index++;
    /* Presence Detected */
    appl_generic_property[0][index].property_id = MS_DEV_PROP_PRESENCE_DETECTED;
    appl_generic_property[0][index].property_type = MS_GENERIC_PROP_TYPE_USER;
    appl_generic_property[0][index].access = MS_GENERIC_USER_ACCESS_READ;
    appl_generic_property[0][index].property_value_len = 1;
    appl_generic_property[0][index].property_value = EM_alloc_mem(appl_generic_property[0][index].property_value_len);
    EM_mem_set(appl_generic_property[0][index].property_value, (index + 1), appl_generic_property[0][index].property_value_len);
    EM_mem_set(appl_generic_client_property, 0, sizeof(appl_generic_client_property));
    /* Client Properties */
    /* Manufacturer Properties */
    access_type = MS_GENERIC_USER_ACCESS_READ;

    for (index = 0; index < MS_MAX_CLIENT_PROPERTIES; index++)
    {
        appl_generic_client_property[0][index].property_id = index + 1;
        appl_generic_client_property[0][index].user_access = access_type;
        /* Allocate and keep some property value */
        appl_generic_client_property[0][index].property_value_len = index + 1;
        appl_generic_client_property[0][index].property_value = EM_alloc_mem(index + 1);
        /* TODO: Not checking for memeory allocation failure */
        EM_mem_set(appl_generic_client_property[0][index].property_value, (index + 1), (index + 1));

        if (MS_GENERIC_USER_ACCESS_READ_WRITE == access_type)
        {
            access_type = MS_GENERIC_USER_ACCESS_READ;
        }
        else
        {
            access_type++;
        }
    }
}

static API_RESULT appl_model_generic_property_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    UINT32 index;

    switch(state_t)
    {
    case MS_STATE_GENERIC_USER_PROPERTY_IDS_T:
    {
        MS_STATE_GENERIC_PROPERTY_IDS_STRUCT* param_p;
        UINT8 count;
        param_p = (MS_STATE_GENERIC_PROPERTY_IDS_STRUCT*)param;
        count = 0;

        for (index = 0; index < MS_MAX_PROPERTIES; index++)
        {
            if (((MS_GENERIC_PROP_TYPE_MANUFACTURER == appl_generic_property[0][index].property_type) &&
                    (MS_GENERIC_USER_ACCESS_READ == appl_generic_property[0][index].access)) ||
                    ((MS_GENERIC_PROP_TYPE_ADMIN == appl_generic_property[0][index].property_type) &&
                     (MS_GENERIC_USER_ACCESS_PROHIBITED != appl_generic_property[0][index].access)) ||
                    ((MS_GENERIC_PROP_TYPE_USER == appl_generic_property[0][index].property_type) &&
                     (MS_GENERIC_USER_ACCESS_PROHIBITED != appl_generic_property[0][index].access)))
            {
                param_p->property_ids[count] = appl_generic_property[0][index].property_id;
                count++;
            }
        }

        /* TODO: Not checking 'property_ids_count' in input */
        param_p->property_ids_count = count;
    }
    break;

    case MS_STATE_GENERIC_ADMIN_PROPERTY_IDS_T:
    {
        MS_STATE_GENERIC_PROPERTY_IDS_STRUCT* param_p;
        UINT8 count;
        param_p = (MS_STATE_GENERIC_PROPERTY_IDS_STRUCT*)param;
        count = 0;

        for (index = 0; index < MS_MAX_PROPERTIES; index++)
        {
            if (/* (MS_GENERIC_PROP_TYPE_MANUFACTURER == appl_generic_property[0][index].property_type) || */
                (MS_GENERIC_PROP_TYPE_ADMIN == appl_generic_property[0][index].property_type))
            {
                param_p->property_ids[count] = appl_generic_property[0][index].property_id;
                count++;
            }
        }

        /* TODO: Not checking 'property_ids_count' in input */
        param_p->property_ids_count = count;
    }
    break;

    case MS_STATE_GENERIC_MANUFACTURER_PROPERTY_IDS_T:
    {
        MS_STATE_GENERIC_PROPERTY_IDS_STRUCT* param_p;
        UINT8 count;
        param_p = (MS_STATE_GENERIC_PROPERTY_IDS_STRUCT*)param;
        count = 0;

        for (index = 0; index < MS_MAX_PROPERTIES; index++)
        {
            if (MS_GENERIC_PROP_TYPE_MANUFACTURER == appl_generic_property[0][index].property_type)
            {
                param_p->property_ids[count] = appl_generic_property[0][index].property_id;
                count++;
            }
        }

        /* TODO: Not checking 'property_ids_count' in input */
        param_p->property_ids_count = count;
    }
    break;

    case MS_STATE_GENERIC_CLIENT_PROPERTY_IDS_T:
    {
        MS_STATE_GENERIC_PROPERTY_IDS_STRUCT* param_p;
        UINT8 count;
        param_p = (MS_STATE_GENERIC_PROPERTY_IDS_STRUCT*)param;
        count = 0;

        for (index = 0; index < MS_MAX_CLIENT_PROPERTIES; index++)
        {
            /* Check if Admin Property is not disabled */
            /* if (MS_GENERIC_USER_ACCESS_PROHIBITED != appl_generic_client_property[0][index].user_access) */
            {
                param_p->property_ids[index] = appl_generic_client_property[0][index].property_id;
                count++;
            }
        }

        /* TODO: Not checking 'property_ids_count' in input */
        param_p->property_ids_count = count;
    }
    break;

    case MS_STATE_GENERIC_USER_PROPERTY_T:
    {
        MS_STATE_GENERIC_USER_PROPERTY_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_USER_PROPERTY_STRUCT*)param;
        /* Check for property ID match */
        /* Mark the access as prohibited, to indicate invalid Property ID */
        param_p->user_access = MS_GENERIC_USER_ACCESS_PROHIBITED;

        for (index = 0; index < MS_MAX_PROPERTIES; index++)
        {
            if (appl_generic_property[0][index].property_id == param_p->property_id)
            {
                if (MS_GENERIC_USER_ACCESS_PROHIBITED != appl_generic_property[0][index].access)
                {
                    param_p->user_access = appl_generic_property[0][index].access;

                    if (MS_GENERIC_USER_ACCESS_WRITE == param_p->user_access)
                    {
                        param_p->property_value_len = 0;
                    }
                    else
                    {
                        param_p->property_value = appl_generic_property[0][index].property_value;
                        param_p->property_value_len = appl_generic_property[0][index].property_value_len;
                    }

                    printf("GGGGEEETTTTTT::::USER::::: [%d] ID:0x%04X. Access:0x%02X. Len:%d\n",
                           index, appl_generic_property[0][index].property_id,
                           appl_generic_property[0][index].access,
                           appl_generic_property[0][index].property_value_len);
                }

                break;
            }
        }
    }
    break;

    case MS_STATE_GENERIC_ADMIN_PROPERTY_T:
    {
        MS_STATE_GENERIC_ADMIN_PROPERTY_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_ADMIN_PROPERTY_STRUCT*)param;
        /* Check for property ID match */
        /* Mark the access as prohibited, to indicate invalid Property ID */
        param_p->user_access = MS_GENERIC_USER_ACCESS_INVALID_PROPERTY_ID;
        param_p->property_value_len = 0;

        for (index = 0; index < MS_MAX_PROPERTIES; index++)
        {
            if (appl_generic_property[0][index].property_id == param_p->property_id)
            {
                if (/* (MS_GENERIC_PROP_TYPE_MANUFACTURER == appl_generic_property[0][index].property_type) || */
                    (MS_GENERIC_PROP_TYPE_ADMIN == appl_generic_property[0][index].property_type))
                {
                    param_p->user_access = appl_generic_property[0][index].access;
                    #if 0

                    if (MS_GENERIC_USER_ACCESS_WRITE == param_p->user_access)
                    {
                        param_p->property_value_len = 0;
                    }
                    else
                    #endif /* 0 */
                    {
                        param_p->property_value = appl_generic_property[0][index].property_value;
                        param_p->property_value_len = appl_generic_property[0][index].property_value_len;
                    }

                    printf("GGGGEEETTTTTT::::ADMIN::::: [%d] ID:0x%04X. Access:0x%02X. Len:%d\n",
                           index, appl_generic_property[0][index].property_id,
                           appl_generic_property[0][index].access,
                           appl_generic_property[0][index].property_value_len);
                    break;
                }
            }
        }
    }
    break;

    case MS_STATE_GENERIC_MANUFACTURER_PROPERTY_T:
    {
        MS_STATE_GENERIC_MANUFACTURER_PROPERTY_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_MANUFACTURER_PROPERTY_STRUCT*)param;

        /* Check for property ID match */

        for (index = 0; index < MS_MAX_PROPERTIES; index++)
        {
            if (appl_generic_property[0][index].property_id == param_p->property_id)
            {
                if (MS_GENERIC_PROP_TYPE_MANUFACTURER == appl_generic_property[0][index].property_type)
                {
                    param_p->user_access = appl_generic_property[0][index].access;
                    #if 0

                    if (MS_GENERIC_USER_ACCESS_WRITE == param_p->user_access)
                    {
                        param_p->property_value_len = 0;
                    }
                    else
                    #endif /* 0 */
                    {
                        param_p->property_value = appl_generic_property[0][index].property_value;
                        param_p->property_value_len = appl_generic_property[0][index].property_value_len;
                    }

                    printf("GGGGEEETTTTTT::::MANU::::: [%d] ID:0x%04X. Access:0x%02X. Len:%d\n",
                           index, appl_generic_property[0][index].property_id,
                           appl_generic_property[0][index].access,
                           appl_generic_property[0][index].property_value_len);
                    break;
                }
            }
        }

        /* Mark the access as prohibited, to indicate invalid Property ID */
        if (MS_MAX_PROPERTIES == index)
        {
            param_p->user_access = MS_GENERIC_USER_ACCESS_INVALID_PROPERTY_ID;
            param_p->property_value_len = 0;
        }
    }
    break;
    }

    return API_SUCCESS;
}

static API_RESULT appl_model_generic_property_check_format(MS_STATE_GENERIC_USER_PROPERTY_STRUCT* param)
{
    API_RESULT retval;
    retval = API_FAILURE;

    /* Check for all the supported properties - currently only for 0x006B */
    switch (param->property_id)
    {
    case MS_DEV_PROP_MOTION_SENSED: /* Motion Sensed */
    case MS_DEV_PROP_PRESENCE_DETECTED: /* Presence Detected */

        /* Check if 8 bit value is received */
        if (1 == param->property_value_len)
        {
            retval = API_SUCCESS;
        }

        break;

    case MS_DEV_PROP_PEOPLE_COUNT: /* People Count */

        /* Check if 16 bit value is received */
        if (2 == param->property_value_len)
        {
            retval = API_SUCCESS;
        }

        break;

    default:
        break;
    }

    return retval;
}

static API_RESULT appl_model_generic_property_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    UINT32 index;

    switch(state_t)
    {
    case MS_STATE_GENERIC_USER_PROPERTY_T:
    {
        MS_STATE_GENERIC_USER_PROPERTY_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_USER_PROPERTY_STRUCT*)param;
        /* Check for property ID match */
        /* Mark the access as prohibited, to indicate invalid Property ID */
        param_p->user_access = MS_GENERIC_USER_ACCESS_PROHIBITED;

        for (index = 0; index < MS_MAX_PROPERTIES; index++)
        {
            if (appl_generic_property[0][index].property_id == param_p->property_id)
            {
                /* Check if the property value is of correct format */
                if (API_SUCCESS != appl_model_generic_property_check_format(param_p))
                {
                    printf("Invalid format for property ID: 0x%04X\n", param_p->property_id);
                    return API_FAILURE;
                }

                if (MS_GENERIC_USER_ACCESS_PROHIBITED != appl_generic_property[0][index].access)
                {
                    /* Check properties - write is permitted */
                    if (MS_GENERIC_USER_ACCESS_READ == appl_generic_property[0][index].access)
                    {
                        printf("Can not set read only Generic User Property\n");
                        param_p->property_value = NULL;
                        param_p->property_value_len = 0;
                    }
                    else
                    {
                        /* Update Value */
                        /* Free Memory */
                        if (NULL != appl_generic_property[0][index].property_value)
                        {
                            EM_free_mem(appl_generic_property[0][index].property_value);
                        }

                        /* Allocate Memory */
                        appl_generic_property[0][index].property_value_len = param_p->property_value_len;
                        appl_generic_property[0][index].property_value = EM_alloc_mem(param_p->property_value_len);
                        /* TODO: Not checking memory allocation failure */
                        /* Copy */
                        EM_mem_copy(appl_generic_property[0][index].property_value, param_p->property_value, param_p->property_value_len);
                    }

                    param_p->user_access = appl_generic_property[0][index].access;
                    printf("SSSEEETTTTTT::::USER::::: [%d] ID:0x%04X. Access:0x%02X. Len:%d\n",
                           index, appl_generic_property[0][index].property_id,
                           appl_generic_property[0][index].access,
                           appl_generic_property[0][index].property_value_len);
                }

                break;
            }
        }
    }
    break;

    case MS_STATE_GENERIC_ADMIN_PROPERTY_T:
    {
        MS_STATE_GENERIC_ADMIN_PROPERTY_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_ADMIN_PROPERTY_STRUCT*)param;

        /* Check for property ID match */

        for (index = 0; index < MS_MAX_PROPERTIES; index++)
        {
            if (appl_generic_property[0][index].property_id == param_p->property_id)
            {
                if (/* (MS_GENERIC_PROP_TYPE_MANUFACTURER == appl_generic_property[0][index].property_type) || */
                    (MS_GENERIC_PROP_TYPE_ADMIN == appl_generic_property[0][index].property_type))
                {
                    appl_generic_property[0][index].access = param_p->user_access;
                    /* Check properties - write is permitted */
                    /* if (MS_GENERIC_USER_ACCESS_READ != appl_generic_property[0][index].access) */
                    {
                        /* Update Value */
                        /* Free Memory */
                        if (NULL != appl_generic_property[0][index].property_value)
                        {
                            EM_free_mem(appl_generic_property[0][index].property_value);
                        }

                        /* Allocate Memory */
                        appl_generic_property[0][index].property_value_len = param_p->property_value_len;
                        appl_generic_property[0][index].property_value = EM_alloc_mem(param_p->property_value_len);
                        /* TODO: Not checking memory allocation failure */
                        /* Copy */
                        EM_mem_copy(appl_generic_property[0][index].property_value, param_p->property_value, param_p->property_value_len);
                        appl_generic_admin_property_server_publish(MS_STATE_GENERIC_ADMIN_PROPERTY_T, 0x00, param_p->property_id);
                    }
                    param_p->property_value = appl_generic_property[0][index].property_value;
                    param_p->property_value_len = appl_generic_property[0][index].property_value_len;
                    printf("SSSSSSSEEETTTTTT::::ADMIN::::: [%d] ID:0x%04X. Access:0x%02X. Len:%d\n",
                           index, appl_generic_property[0][index].property_id,
                           appl_generic_property[0][index].access,
                           appl_generic_property[0][index].property_value_len);
                    break;
                }
            }
        }

        /* Check if match found. Else set prohibited access */
        if (MS_MAX_PROPERTIES == index)
        {
            /* Mark the access as prohibited, to indicate invalid Property ID */
            param_p->user_access = MS_GENERIC_USER_ACCESS_INVALID_PROPERTY_ID;
            param_p->property_value_len = 0;
        }
    }
    break;

    case MS_STATE_GENERIC_MANUFACTURER_PROPERTY_T:
    {
        MS_STATE_GENERIC_MANUFACTURER_PROPERTY_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_MANUFACTURER_PROPERTY_STRUCT*)param;

        /* Check for property ID match */
        /* Mark the access as prohibited, to indicate invalid Property ID */
        for (index = 0; index < MS_MAX_PROPERTIES; index++)
        {
            if (appl_generic_property[0][index].property_id == param_p->property_id)
            {
                if (MS_GENERIC_PROP_TYPE_MANUFACTURER == appl_generic_property[0][index].property_type)
                {
                    /* Check properties - write is permitted */
                    if (MS_GENERIC_USER_ACCESS_WRITE == (MS_GENERIC_USER_ACCESS_WRITE & (appl_generic_property[0][index].access | param_p->user_access)))
                    {
                        /* Update Value */
                        /* Free Memory */
                        if ((0 != param_p->property_value_len) && (NULL != appl_generic_property[0][index].property_value))
                        {
                            EM_free_mem(appl_generic_property[0][index].property_value);
                            /* Allocate Memory */
                            appl_generic_property[0][index].property_value_len = param_p->property_value_len;
                            appl_generic_property[0][index].property_value = EM_alloc_mem(param_p->property_value_len);
                            /* TODO: Not checking memory allocation failure */
                            /* Copy */
                            EM_mem_copy(appl_generic_property[0][index].property_value, param_p->property_value, param_p->property_value_len);
                        }
                    }

                    appl_generic_property[0][index].access = param_p->user_access;
                    appl_generic_manufacturer_property_server_publish(MS_STATE_GENERIC_MANUFACTURER_PROPERTY_T, 0x00, param_p->property_id);
                    param_p->user_access = appl_generic_property[0][index].access;
                    param_p->property_value = appl_generic_property[0][index].property_value;
                    param_p->property_value_len = appl_generic_property[0][index].property_value_len;
                    printf("SSSSSSSEEETTTTTT::::MANUFACTURER::::: [%d] ID:0x%04X. Access:0x%02X. Len:%d\n",
                           index, appl_generic_property[0][index].property_id,
                           appl_generic_property[0][index].access,
                           appl_generic_property[0][index].property_value_len);
                    break;
                }
            }
        }

        /* Mark the access as prohibited, to indicate invalid Property ID */
        if (MS_MAX_PROPERTIES == index)
        {
            param_p->user_access = MS_GENERIC_USER_ACCESS_INVALID_PROPERTY_ID;
            param_p->property_value_len = 0;
        }
    }
    break;
    }

    return API_SUCCESS;
}

/** --------------------- Scene ---- */
void* appl_scene_save_current_state(/* IN */ UINT32 scene_index)
{
    /* Store current state */
    appl_state_info_vault[scene_index].onoff = appl_generic_onoff[0].onoff;
    appl_state_info_vault[scene_index].level = appl_generic_level_info[0].generic_level.level;
    appl_state_info_vault[scene_index].power_level = appl_generic_power_level[0].generic_power_actual.power_actual;
    appl_state_info_vault[scene_index].lightness_actual = appl_light_lightness[0].light_lightness_actual.lightness_actual;
    appl_state_info_vault[scene_index].lightness_linear = appl_light_lightness[0].light_lightness_linear.lightness_linear;
    appl_state_info_vault[scene_index].ctl_lightness = appl_light_ctl[0].ctl_lightness;
    appl_state_info_vault[scene_index].ctl_temperature = appl_light_ctl[0].ctl_temperature;
    appl_state_info_vault[scene_index].ctl_delta_uv = appl_light_ctl[0].ctl_delta_uv;
    appl_state_info_vault[scene_index].hsl_lightness = appl_light_hsl[0].hsl_lightness;
    appl_state_info_vault[scene_index].hsl_hue = appl_light_hsl[0].hsl_hue;
    appl_state_info_vault[scene_index].hsl_saturation = appl_light_hsl[0].hsl_saturation;
    appl_state_info_vault[scene_index].xyl_lightness = appl_light_xyl[0].xyl_lightness;
    appl_state_info_vault[scene_index].xyl_x = appl_light_xyl[0].xyl_x;
    appl_state_info_vault[scene_index].xyl_y = appl_light_xyl[0].xyl_y;
    appl_state_info_vault[scene_index].lc_mode = appl_light_lc_mode[0].present_mode;
    /* Allocate memory for LC Property Value */
    CONSOLE_OUT("[Store] Prop ID: 0x%04X. Len: 0x%04X\n",
                appl_light_lc_property[0][appl_current_property_id_index].property_id,
                appl_light_lc_property[0][appl_current_property_id_index].property_value_len);
    appl_state_info_vault[scene_index].lc_property.property_id =
        appl_light_lc_property[0][appl_current_property_id_index].property_id;

    if (0 != appl_light_lc_property[0][appl_current_property_id_index].property_value_len)
    {
        /* Free memory */
        if (NULL != appl_state_info_vault[scene_index].lc_property.property_value)
        {
            EM_free_mem(appl_state_info_vault[scene_index].lc_property.property_value);
        }

        appl_state_info_vault[scene_index].lc_property.property_value =
            EM_alloc_mem(appl_light_lc_property[0][appl_current_property_id_index].property_value_len);

        if (NULL != appl_state_info_vault[scene_index].lc_property.property_value)
        {
            EM_mem_copy
            (
                appl_state_info_vault[scene_index].lc_property.property_value,
                appl_light_lc_property[0][appl_current_property_id_index].property_value,
                appl_light_lc_property[0][appl_current_property_id_index].property_value_len
            );
            CONSOLE_OUT("Storing property value\n");
            appl_dump_bytes
            (
                appl_state_info_vault[scene_index].lc_property.property_value,
                appl_light_lc_property[0][appl_current_property_id_index].property_value_len
            );
        }
    }

    appl_state_info_vault[scene_index].lc_property.property_value_len =
        appl_light_lc_property[0][appl_current_property_id_index].property_value_len;
    /* TODO: optimize by not writing all the 16 scenes */
    appl_ps_store(MS_PS_RECORD_APPL_SCENE);
    return (void*)scene_index;
}

void* appl_scene_save_current_state_ex(/* IN */ UINT32 scene_index)
{
    /* Store current state */
    /* Check if transition ongoing */
    if (0x00 != appl_generic_onoff[0].transition_time)
    {
        appl_state_info_vault[scene_index].onoff = appl_generic_onoff[0].target_onoff;
    }
    else
    {
        appl_state_info_vault[scene_index].onoff = appl_generic_onoff[0].onoff;
    }

    appl_state_info_vault[scene_index].level = appl_generic_level_info[0].generic_level.level;
    appl_state_info_vault[scene_index].power_level = appl_generic_power_level[0].generic_power_actual.power_actual;
    appl_state_info_vault[scene_index].lightness_actual = appl_light_lightness[0].light_lightness_actual.lightness_actual;
    appl_state_info_vault[scene_index].lightness_linear = appl_light_lightness[0].light_lightness_linear.lightness_linear;
    appl_state_info_vault[scene_index].ctl_lightness = appl_light_ctl[0].ctl_lightness;
    appl_state_info_vault[scene_index].ctl_temperature = appl_light_ctl[0].ctl_temperature;
    appl_state_info_vault[scene_index].ctl_delta_uv = appl_light_ctl[0].ctl_delta_uv;
    appl_state_info_vault[scene_index].hsl_lightness = appl_light_hsl[0].hsl_lightness;
    appl_state_info_vault[scene_index].hsl_hue = appl_light_hsl[0].hsl_hue;
    appl_state_info_vault[scene_index].hsl_saturation = appl_light_hsl[0].hsl_saturation;
    appl_state_info_vault[scene_index].xyl_lightness = appl_light_xyl[0].xyl_lightness;
    appl_state_info_vault[scene_index].xyl_x = appl_light_xyl[0].xyl_x;
    appl_state_info_vault[scene_index].xyl_y = appl_light_xyl[0].xyl_y;
    appl_state_info_vault[scene_index].lc_mode = appl_light_lc_mode[0].present_mode;
    /* Allocate memory for LC Property Value */
    CONSOLE_OUT("[Store] Prop ID: 0x%04X. Len: 0x%04X\n",
                appl_light_lc_property[0][appl_current_property_id_index].property_id,
                appl_light_lc_property[0][appl_current_property_id_index].property_value_len);
    appl_state_info_vault[scene_index].lc_property.property_id =
        appl_light_lc_property[0][appl_current_property_id_index].property_id;

    if (0 != appl_light_lc_property[0][appl_current_property_id_index].property_value_len)
    {
        /* Free memory */
        if (NULL != appl_state_info_vault[scene_index].lc_property.property_value)
        {
            EM_free_mem(appl_state_info_vault[scene_index].lc_property.property_value);
        }

        appl_state_info_vault[scene_index].lc_property.property_value =
            EM_alloc_mem(appl_light_lc_property[0][appl_current_property_id_index].property_value_len);

        if (NULL != appl_state_info_vault[scene_index].lc_property.property_value)
        {
            EM_mem_copy
            (
                appl_state_info_vault[scene_index].lc_property.property_value,
                appl_light_lc_property[0][appl_current_property_id_index].property_value,
                appl_light_lc_property[0][appl_current_property_id_index].property_value_len
            );
            CONSOLE_OUT("Storing property value\n");
            appl_dump_bytes
            (
                appl_state_info_vault[scene_index].lc_property.property_value,
                appl_light_lc_property[0][appl_current_property_id_index].property_value_len
            );
        }
    }

    appl_state_info_vault[scene_index].lc_property.property_value_len =
        appl_light_lc_property[0][appl_current_property_id_index].property_value_len;

    /* TODO: optimize by not writing all the 16 scenes */
    if (16 > scene_index)
    {
        appl_ps_store(MS_PS_RECORD_APPL_SCENE);
    }

    return (void*)scene_index;
}

void* appl_scene_delete_saved_state(/* IN */ UINT32 scene_index, /* IN */ void* context)
{
    /* Free context - if any */
    /* Invalid value */
    appl_state_info_vault[scene_index].onoff = 0x02;
    appl_state_info_vault[scene_index].level = 0xFFFF;
    appl_state_info_vault[scene_index].power_level = 0xFFFF;
    appl_state_info_vault[scene_index].lightness_actual = 0x0000;
    appl_state_info_vault[scene_index].lightness_linear = 0x0000;
    appl_state_info_vault[scene_index].ctl_lightness = 0x0000;
    appl_state_info_vault[scene_index].ctl_temperature = 0x0000;
    appl_state_info_vault[scene_index].ctl_delta_uv = 0x0000;
    appl_state_info_vault[scene_index].hsl_lightness = 0x0000;
    appl_state_info_vault[scene_index].hsl_hue = 0x0000;
    appl_state_info_vault[scene_index].hsl_saturation = 0x0000;
    appl_state_info_vault[scene_index].xyl_lightness = 0x0000;
    appl_state_info_vault[scene_index].xyl_x = 0x0000;
    appl_state_info_vault[scene_index].xyl_y = 0x0000;
    appl_state_info_vault[scene_index].lc_mode = 0x00;
    appl_state_info_vault[scene_index].lc_property.property_id = 0x0000;

    if (NULL != appl_state_info_vault[scene_index].lc_property.property_value)
    {
        EM_free_mem(appl_state_info_vault[scene_index].lc_property.property_value);
        appl_state_info_vault[scene_index].lc_property.property_value = NULL;
    }

    appl_state_info_vault[scene_index].lc_property.property_value_len = 0x0000;
    /* TODO: optimize by not writing all the 16 scenes */
    appl_ps_store(MS_PS_RECORD_APPL_SCENE);
    return (void*)NULL;
}

void* appl_scene_recall_saved_state(/* IN */ UINT32 scene_index, /* IN */ void* context)
{
//    UINT32 index;
    /* Recall context - if any */
    appl_set_generic_onoff(0, appl_state_info_vault[scene_index].onoff, MS_FALSE);
    appl_set_generic_level(0, appl_state_info_vault[scene_index].level, MS_FALSE, MS_FALSE);
    appl_power_level_set_actual(0, appl_state_info_vault[scene_index].power_level);
    appl_light_lightness_set_actual(0, appl_state_info_vault[scene_index].lightness_actual, MS_TRUE);
    appl_light_lightness[0].light_lightness_linear.lightness_linear = appl_state_info_vault[scene_index].lightness_linear;
    appl_light_ctl_lightness_set_actual
    (
        0,
        appl_state_info_vault[scene_index].ctl_lightness,
        appl_state_info_vault[scene_index].ctl_temperature,
        appl_state_info_vault[scene_index].ctl_delta_uv,
        MS_TRUE
    );
    #if 0
    appl_light_ctl[0].ctl_lightness = appl_state_info_vault[scene_index].ctl_lightness;
    appl_light_ctl[0].ctl_temperature = appl_state_info_vault[scene_index].ctl_temperature;
    appl_light_ctl[0].ctl_delta_uv = appl_state_info_vault[scene_index].ctl_delta_uv;
    #endif /* 0 */
    appl_light_hsl_set_actual
    (
        0,
        appl_state_info_vault[scene_index].hsl_lightness,
        appl_state_info_vault[scene_index].hsl_hue,
        appl_state_info_vault[scene_index].hsl_saturation,
        MS_TRUE
    );
    #if 0
    appl_light_hsl[0].hsl_lightness = appl_state_info_vault[scene_index].hsl_lightness;
    appl_light_hsl[0].hsl_hue = appl_state_info_vault[scene_index].hsl_hue;
    appl_light_hsl[0].hsl_saturation = appl_state_info_vault[scene_index].hsl_saturation;
    #endif /* 0 */
    #if 0
    appl_light_xyl[0].xyl_lightness = appl_state_info_vault[scene_index].xyl_lightness;
    appl_light_xyl[0].xyl_x = appl_state_info_vault[scene_index].xyl_x;
    appl_light_xyl[0].xyl_y = appl_state_info_vault[scene_index].xyl_y;
    #else
    appl_light_xyl_set_actual
    (
        0,
        appl_state_info_vault[scene_index].xyl_lightness,
        appl_state_info_vault[scene_index].xyl_x,
        appl_state_info_vault[scene_index].xyl_y
    );
    #endif /* 0 */
    appl_light_lc_set_actual(0, MS_STATE_LIGHT_LC_MODE_T, 0x00, appl_state_info_vault[scene_index].lc_mode);

    if (16 < scene_index)
    {
        CONSOLE_OUT("[Restore] Prop ID: 0x%04X. Len: 0x%04X\n",
                    appl_state_info_vault[scene_index].lc_property.property_id,
                    appl_state_info_vault[scene_index].lc_property.property_value_len);
        appl_light_lc_set_actual_propery
        (
            0,
            appl_state_info_vault[scene_index].lc_property.property_id,
            appl_state_info_vault[scene_index].lc_property.property_value,
            appl_state_info_vault[scene_index].lc_property.property_value_len
        );
    }

    return (void*)NULL;
}


/** --------------------- Light - Lightness ---- */
static void appl_model_light_lightness_states_initialization(void)
{
    EM_mem_set (appl_light_lightness, 0, sizeof(appl_light_lightness));
    appl_light_lightness[0].light_lightness_last.lightness_last = 0x0000; /* 0xFFFF; */
    appl_light_lightness[1].light_lightness_last.lightness_last = 0x0000; /* 0xFFFF; */
}

static void appl_light_lightness_transition_start_cb(void* blob)
{
}

static void appl_light_lightness_transition_complete_cb(void* blob)
{
    /* State Transition Complete */
    appl_light_lightness[0].light_lightness_actual.transition_time = 0;
    appl_light_lightness_set_actual(0, appl_light_lightness[0].light_lightness_actual.lightness_target, MS_FALSE);
    appl_light_lightness[0].light_lightness_actual.lightness_target = 0;
}

static void appl_light_lightness_linear_transition_start_cb(void* blob)
{
}

static void appl_light_lightness_linear_transition_complete_cb(void* blob)
{
    /* State Transition Complete */
    appl_light_lightness[0].light_lightness_linear.transition_time = 0;
    appl_light_lightness_set_linear(0, appl_light_lightness[0].light_lightness_linear.lightness_target);
    appl_light_lightness[0].light_lightness_linear.lightness_target = 0;
}

static void appl_light_lightness_set_actual(UINT16 state_inst, UINT16 actual, UCHAR forced_publish)
{
    UINT16 min, max;
    printf("appl_light_lightness_set_actual: Actual: 0x%04X\n", actual);
    /* Generic OnOff binding */
    min = appl_light_lightness[state_inst].light_lightness_range.lightness_range_min;
    max = appl_light_lightness[state_inst].light_lightness_range.lightness_range_max;

    if ((0 != min) && (actual < min))
    {
        actual = min;
    }
    else if ((0 != max) && (actual > max))
    {
        actual = max;
    }

    if (actual != appl_light_lightness[state_inst].light_lightness_actual.lightness_actual)
    {
        appl_light_lightness[state_inst].light_lightness_actual.lightness_actual = actual;
        /* Publish State Change */
        appl_light_lightness_server_publish(MS_STATE_LIGHT_LIGHTNESS_ACTUAL_T, state_inst);

        /* If Lightness Actual is non-zero, save as Lightness Last */
        if (0x0000 != actual)
        {
            appl_light_lightness[state_inst].light_lightness_last.lightness_last = actual;
            appl_light_hsl_set_actual(state_inst, actual, appl_light_hsl[state_inst].hsl_hue, appl_light_hsl[state_inst].hsl_saturation, MS_FALSE);
        }

        /* Light Lightness Linear = ((Actual)^2) / 65535 */
        appl_light_lightness[state_inst].light_lightness_linear.lightness_linear = ((actual * actual) + 65534) / 65535;
        appl_set_generic_level(state_inst, actual - 32768, MS_TRUE, MS_FALSE);
    }
    else if (MS_TRUE == forced_publish)
    {
        /* Publish State Change */
        appl_light_lightness_server_publish(MS_STATE_LIGHT_LIGHTNESS_ACTUAL_T, state_inst);
    }
}

#if 0
/* Not used */
static void appl_light_lightness_set_range(UINT16 state_inst, UINT16 min, UINT16 max)
{
    UINT16 actual;
    appl_light_lightness[state_inst].light_lightness_range.lightness_range_min = min;
    appl_light_lightness[state_inst].light_lightness_range.lightness_range_max = max;
    /* Check if actual to be updated */
    actual = appl_light_lightness[state_inst].light_lightness_actual.lightness_actual;
    appl_light_lightness_set_actual(state_inst, actual);
}
#endif /* 0 */

/* Todo: Remove the dependency */
#include "math.h"

/* TODO: Make the below function static */
void appl_light_lightness_set_linear(UINT16 state_inst, UINT16 linear)
{
    UINT16 actual;
    UINT32 mul_val;
    mul_val = linear * 65535;
    actual = (UINT16)sqrt(mul_val);
    /* Light Lightness actual = sqrt(Linear * 65535) */
    appl_light_lightness_set_actual(state_inst, actual, MS_FALSE);
}

static void appl_set_generic_onoff(UINT16 state_inst, UINT8 onoff, UINT8 forced_publish)
{
    UINT16 actual;

    /* Check if there is change in state */
    if (appl_generic_onoff[state_inst].onoff != onoff)
    {
        /* TODO: See if this to be stored */
        APPL_GENERIC_ONOFF_SET(state_inst, onoff);
        /* Publish State Change */
        appl_generic_onoff_server_publish(MS_STATE_GENERIC_ONOFF_T, 0x00);
        appl_light_lc_onoff[state_inst].present_light_onoff = onoff;

        /* Binding */
        if (onoff == 0x00)
        {
            appl_light_lightness_set_actual(state_inst, 0x00, MS_FALSE);
            appl_power_level_set_actual(state_inst, 0x00);
        }
        else
        {
            if (0x0000 == appl_light_lightness[state_inst].light_lightness_default.lightness_default)
            {
                actual = appl_light_lightness[state_inst].light_lightness_last.lightness_last;
            }
            else
            {
                actual = appl_light_lightness[state_inst].light_lightness_default.lightness_default;
            }

            if (0x0000 != actual)
            {
                appl_light_lightness_set_actual(state_inst, actual, MS_FALSE);
            }

            if (0x0000 == appl_generic_power_level[state_inst].generic_power_default.power_default)
            {
                actual = appl_generic_power_level[state_inst].generic_power_last.power_last;
            }
            else
            {
                actual = appl_generic_power_level[state_inst].generic_power_default.power_default;
            }

            if (0x0000 != actual)
            {
                appl_power_level_set_actual(state_inst, actual);
            }
        }
    }
    else if (MS_TRUE == forced_publish)
    {
        /* Publish State Change */
        appl_generic_onoff_server_publish(MS_STATE_GENERIC_ONOFF_T, 0x00);
    }
}

static API_RESULT appl_model_light_lightness_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    MS_STATE_LIGHT_LIGHTNESS_STRUCT* param_p;
    API_RESULT                        ret;
    UINT8                             transition_time;
    param_p = (MS_STATE_LIGHT_LIGHTNESS_STRUCT*)param;

    switch (state_t)
    {
    case MS_STATE_LIGHT_LIGHTNESS_DEFAULT_T:
    {
        /* Ignoring Instance and direction right now */
        param_p->light_lightness_default = appl_light_lightness[0].light_lightness_default;
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_RANGE_T:
    {
        /* Ignoring Instance and direction right now */
        param_p->light_lightness_range = appl_light_lightness[0].light_lightness_range;
        param_p->range_status = 0x00;
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_LINEAR_T:
    {
        /* Ignoring Instance and direction right now */
        param_p->light_lightness_linear = appl_light_lightness[0].light_lightness_linear;
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_LAST_T:
    {
        /* Ignoring Instance and direction right now */
        param_p->light_lightness_last = appl_light_lightness[0].light_lightness_last;
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_ACTUAL_T:
    {
        /* Ignoring Instance and direction right now */
        param_p->light_lightness_actual = appl_light_lightness[0].light_lightness_actual;

        if (0 != param_p->light_lightness_actual.transition_time)
        {
            ret = MS_common_get_remaining_transition_time
                  (
                      param_p->light_lightness_actual.transition_time_handle,
                      &transition_time
                  );

            if (API_SUCCESS == ret)
            {
                param_p->light_lightness_actual.transition_time = transition_time;
            }
        }

        CONSOLE_OUT("Light Lightness Actual State Get.\n");
    }
    break;

    default:
        break;
    }

    return API_SUCCESS;
}

static API_RESULT appl_model_light_lightness_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    MS_STATE_LIGHT_LIGHTNESS_STRUCT* param_p;
    param_p = (MS_STATE_LIGHT_LIGHTNESS_STRUCT*)param;
    retval = API_SUCCESS;

    switch(state_t)
    {
    case MS_STATE_LIGHT_LIGHTNESS_DEFAULT_T:
    {
        /* Ignoring Instance and direction right now */
        appl_light_lightness[0].light_lightness_default = param_p->light_lightness_default;
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_RANGE_T:
    {
        /* Check range min and max */
        if (param_p->light_lightness_range.lightness_range_min > param_p->light_lightness_range.lightness_range_max)
        {
            /* TODO: add macro define */
            /**
                Table 7.2:
                0x00 - Success
                0x01 - Cannot Set Range Min
                0x02 - Cannot Set Range Max
            */
            param_p->range_status = 0x01;
        }
        else
        {
            /* Ignoring Instance and direction right now */
            appl_light_lightness[0].light_lightness_range = param_p->light_lightness_range;
            param_p->range_status = 0x00;
        }
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_LINEAR_T:
    {
        if (0 != param_p->light_lightness_linear.transition_time)
        {
            MS_ACCESS_STATE_TRANSITION_TYPE   transition;
            UINT16                            transition_time_handle;
            appl_light_lightness[0].light_lightness_linear.lightness_target = param_p->light_lightness_linear.lightness_linear;
            appl_light_lightness[0].light_lightness_linear.transition_time = param_p->light_lightness_linear.transition_time;
            transition.delay = param_p->light_lightness_linear.delay;
            transition.transition_time = param_p->light_lightness_linear.transition_time;
            transition.blob = NULL;
            transition.transition_start_cb = appl_light_lightness_linear_transition_start_cb;
            transition.transition_complete_cb = appl_light_lightness_linear_transition_complete_cb;
            MS_common_start_transition_timer
            (
                &transition,
                &transition_time_handle
            );
            /* Return value to indicate not sending status right now */
            retval = API_FAILURE;
        }
        else
        {
            /* Instantaneous Change */
            /* appl_generic_onoff[0].onoff = param_p->onoff; */
            appl_light_lightness_set_linear(0, param_p->light_lightness_linear.lightness_linear);
        }

        *param_p = appl_light_lightness[0];
        CONSOLE_OUT("[state] current: 0x%02X\n", param_p->light_lightness_linear.lightness_linear);
        CONSOLE_OUT("[state] target: 0x%02X\n", param_p->light_lightness_linear.lightness_target);
        CONSOLE_OUT("[state] remaining_time: 0x%02X\n", param_p->light_lightness_linear.transition_time);
        /* Ignoring Instance and direction right now */
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_LAST_T:
    {
        /* Ignoring Instance and direction right now */
        appl_light_lightness[0].light_lightness_last = param_p->light_lightness_last;
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_ACTUAL_T:
    {
        printf("**** MS_STATE_LIGHT_LIGHTNESS_ACTUAL_T ***\n");

        if (0 != param_p->light_lightness_actual.transition_time)
        {
            MS_ACCESS_STATE_TRANSITION_TYPE   transition;
            appl_light_lightness[0].light_lightness_actual.lightness_target = param_p->light_lightness_actual.lightness_actual;
            appl_light_lightness[0].light_lightness_actual.transition_time = param_p->light_lightness_actual.transition_time;
            transition.delay = param_p->light_lightness_actual.delay;
            transition.transition_time = param_p->light_lightness_actual.transition_time;
            transition.blob = NULL;
            transition.transition_start_cb = appl_light_lightness_transition_start_cb;
            transition.transition_complete_cb = appl_light_lightness_transition_complete_cb;
            MS_common_start_transition_timer
            (
                &transition,
                &appl_light_lightness[0].light_lightness_actual.transition_time_handle
            );
            /* Return value to indicate not sending status right now */
            retval = API_FAILURE;
        }
        else
        {
            /* Instantaneous Change */
            /* appl_generic_onoff[0].onoff = param_p->onoff; */
            appl_light_lightness_set_actual(0, param_p->light_lightness_actual.lightness_actual, MS_FALSE);
        }

        *param_p = appl_light_lightness[0];
        CONSOLE_OUT("[state] current: 0x%02X\n", param_p->light_lightness_actual.lightness_actual);
        CONSOLE_OUT("[state] target: 0x%02X\n", param_p->light_lightness_actual.lightness_target);
        CONSOLE_OUT("[state] remaining_time: 0x%02X\n", param_p->light_lightness_actual.transition_time);
    }
    break;

    default:
        break;
    }

    return retval;
}

/** --------------------- Light - CTL ---- */
static void appl_model_light_ctl_states_initialization(void)
{
    UINT16 level;
    EM_mem_set (appl_light_ctl, 0, sizeof(appl_light_ctl));
    /**
        Color temperature of white light in Kelvin
        (0x0320 = 800 Kelvin, 0x4E20 = 20000 Kelvin)
    */
    /* EM_mem_set(appl_light_ctl_temperature_range, 0, sizeof(appl_light_ctl_temperature_range)); */
    appl_light_ctl_temperature_range[0].ctl_temperature_range_min = LIGHT_CTL_TEMPERATURE_T_MIN;
    appl_light_ctl_temperature_range[0].ctl_temperature_range_max = LIGHT_CTL_TEMPERATURE_T_MAX;
    EM_mem_set(appl_light_ctl_default, 0, sizeof(appl_light_ctl_default));
    EM_mem_set (appl_light_ctl_temperature, 0, sizeof(appl_light_ctl_temperature));
    level = appl_generic_level_info[0].generic_level.level;
    /* Light CTL Temperature = T_MIN + (Generic Level + 32768) * (T_MAX - T_MIN) / 65535 */
    appl_light_ctl_temperature[0].ctl_temperature =
        appl_light_ctl_temperature_range[0].ctl_temperature_range_min +
        ((level + 32768) * (appl_light_ctl_temperature_range[0].ctl_temperature_range_max -
                            appl_light_ctl_temperature_range[0].ctl_temperature_range_min)) / 65535;
}

static void appl_light_ctl_lightness_set_actual(UINT16 state_inst, UINT16 lightness, UINT16 temperature, UINT16 delta_uv, UCHAR forced_publish)
{
    printf("appl_light_ctl_lightness_set_actual: lightness:0x%04X, temp:0x%04X, delta_uv:0x%04X\n",
           lightness, temperature, delta_uv);

    if (lightness != appl_light_ctl[state_inst].ctl_lightness)
    {
        appl_light_ctl[state_inst].ctl_lightness = lightness;
        appl_light_ctl_temp_set_actual(state_inst, temperature, delta_uv, MS_FALSE);
        /* Publish State Change */
        appl_light_ctl_server_publish(MS_STATE_LIGHT_CTL_T, state_inst);
        appl_light_lightness_set_actual(state_inst, lightness, MS_FALSE);
    }
    else if (MS_TRUE == forced_publish)
    {
        /* Publish State Change */
        appl_light_ctl_server_publish(MS_STATE_LIGHT_CTL_T, state_inst);
    }
}

static void appl_light_ctl_temp_set_actual(UINT16 state_inst, UINT16 actual, UINT16 delta_uv, UCHAR is_calculated)
{
    UINT16 min, max;
    printf("appl_light_ctl_temp_set_actual: actual:0x%04X, is_calculated:%02X\n", actual, is_calculated);
    min = appl_light_ctl_temperature_range[state_inst].ctl_temperature_range_min;
    max = appl_light_ctl_temperature_range[state_inst].ctl_temperature_range_max;

    if ((0 != min) && (actual < min))
    {
        actual = min;
    }
    else if ((0 != max) && (actual > max))
    {
        actual = max;
    }

    if ((actual == appl_light_ctl[state_inst].ctl_temperature) ||
            /* If calculated and difference is only one */
            ((MS_TRUE == is_calculated) && (API_SUCCESS == appl_model_state_is_diff_in_range(actual, appl_light_ctl[state_inst].ctl_temperature, 0x01)))
       )
    {
    }
    else
    {
        printf("appl_light_ctl_temp_set_actual: Setting CTL Temperature:0x%04X, Delta UV:0x%04X\n",
               actual, delta_uv);
        appl_light_ctl[state_inst].ctl_temperature = actual;
        appl_light_ctl[state_inst].ctl_delta_uv = delta_uv;
        appl_light_ctl_temperature[state_inst].ctl_temperature = actual;
        appl_light_ctl_temperature[state_inst].ctl_delta_uv = delta_uv;
        appl_light_ctl_temperature_server_publish(MS_STATE_LIGHT_CTL_TEMPERATURE_T, state_inst);

        /* Check min and max are not the same */
        if (max != min)
        {
            appl_set_generic_level
            (
                state_inst,
                (((actual - LIGHT_CTL_TEMPERATURE_T_MIN) * 0xFFFF) / (LIGHT_CTL_TEMPERATURE_T_MAX - LIGHT_CTL_TEMPERATURE_T_MIN)) - 32768,
                MS_TRUE,
                MS_TRUE
            );
        }
    }
}

static void appl_light_ctl_temp_set_range(UINT16 state_inst, UINT16 min, UINT16 max)
{
    UINT16 actual;
    appl_light_ctl_temperature_range[state_inst].ctl_temperature_range_min = min;
    appl_light_ctl_temperature_range[state_inst].ctl_temperature_range_max = max;
    /* Check if actual to be updated */
    actual = appl_light_ctl[state_inst].ctl_temperature;

    if (actual < min)
    {
        appl_light_ctl[state_inst].ctl_temperature = min;
    }
    else if (actual > max)
    {
        appl_light_ctl[state_inst].ctl_temperature = max;
    }
}

static void appl_light_ctl_transition_start_cb(void* blob)
{
}

static void appl_light_ctl_transition_complete_cb(void* blob)
{
    appl_light_ctl[0].transition_time = 0x0000;
    appl_light_ctl_temp_set_actual(0, appl_light_ctl[0].target_ctl_temperature, appl_light_ctl[0].ctl_delta_uv, MS_FALSE);
    appl_light_ctl[0].ctl_lightness = appl_light_ctl[0].target_ctl_lightness;
    appl_light_ctl[0].target_ctl_temperature = 0x0000;
    appl_light_ctl[0].target_ctl_lightness = 0x0000;
}

static void appl_light_ctl_temperature_transition_start_cb(void* blob)
{
}

static void appl_light_ctl_temperature_transition_complete_cb(void* blob)
{
    appl_light_ctl_temp_set_actual(0, appl_light_ctl_temperature[0].target_ctl_temperature, appl_light_ctl_temperature[0].target_ctl_delta_uv, MS_FALSE);
    /* appl_light_ctl_temperature[0].ctl_delta_uv = appl_light_ctl_temperature[0].target_ctl_delta_uv; */
    appl_light_ctl_temperature[0].target_ctl_temperature = 0x0000;
    appl_light_ctl_temperature[0].target_ctl_delta_uv= 0x0000;
    appl_light_ctl_temperature[0].transition_time = 0x0000;
}

static API_RESULT appl_model_light_ctl_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    switch(state_t)
    {
    case MS_STATE_LIGHT_CTL_DEFAULT_T:
    {
        MS_STATE_LIGHT_CTL_DEFAULT_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_CTL_DEFAULT_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_light_ctl_default[0];
    }
    break;

    case MS_STATE_LIGHT_CTL_T:
    {
        MS_STATE_LIGHT_CTL_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_CTL_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_light_ctl[0];
    }
    break;

    case MS_STATE_LIGHT_CTL_TEMPERATURE_RANGE_T:
    {
        MS_STATE_LIGHT_CTL_TEMPERATURE_RANGE_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_CTL_TEMPERATURE_RANGE_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_light_ctl_temperature_range[0];
        param_p->status = 0x00;
    }
    break;

    case MS_STATE_LIGHT_CTL_TEMPERATURE_T:
    {
        MS_STATE_LIGHT_CTL_TEMPERATURE_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_CTL_TEMPERATURE_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_light_ctl_temperature[0];
    }
    break;

    default:
        break;
    }

    return API_SUCCESS;
}

static API_RESULT appl_model_light_ctl_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    retval = API_SUCCESS;

    switch(state_t)
    {
    case MS_STATE_LIGHT_CTL_DEFAULT_T:
    {
        MS_STATE_LIGHT_CTL_DEFAULT_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_CTL_DEFAULT_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_light_ctl_default[0] = *param_p;
    }
    break;

    case MS_STATE_LIGHT_CTL_T:
    {
        MS_STATE_LIGHT_CTL_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_CTL_STRUCT*)param;

        /* Check if state transition is specified */
        if (0 != param_p->transition_time)
        {
            MS_ACCESS_STATE_TRANSITION_TYPE   transition;
            UINT16                            transition_time_handle;
            appl_light_ctl[0].target_ctl_lightness = param_p->ctl_lightness;
            appl_light_ctl[0].target_ctl_temperature = param_p->ctl_temperature;
            appl_light_ctl[0].ctl_delta_uv = param_p->ctl_delta_uv;
            appl_light_ctl[0].transition_time = param_p->transition_time;
            transition.delay = param_p->delay;
            transition.transition_time = param_p->transition_time;
            transition.blob = NULL;
            transition.transition_start_cb = appl_light_ctl_transition_start_cb;
            transition.transition_complete_cb = appl_light_ctl_transition_complete_cb;
            MS_common_start_transition_timer
            (
                &transition,
                &transition_time_handle
            );
            /* Return value to indicate not sending status right now */
            retval = API_FAILURE;
        }
        else
        {
            /* Instantaneous Change */
            /* appl_light_ctl[0] = *param_p; */
            appl_light_ctl_lightness_set_actual(0, param_p->ctl_lightness, param_p->ctl_temperature, param_p->ctl_delta_uv, MS_FALSE);
            /* appl_light_ctl_temp_set_actual(0, param_p->ctl_temperature); */
            appl_light_ctl[0].ctl_delta_uv = param_p->ctl_delta_uv;
            appl_light_ctl[0].tid = param_p->tid;
            appl_light_lightness[state_inst].light_lightness_actual.lightness_actual = param_p->ctl_lightness;
            /* appl_light_ctl[state_inst].ctl_lightness = param_p->ctl_lightness; */
        }

        *param_p = appl_light_ctl[0];
        CONSOLE_OUT("[state] current Lightness: 0x%02X\n", appl_light_ctl[0].ctl_lightness);
        CONSOLE_OUT("[state] target Lightness: 0x%02X\n", appl_light_ctl[0].target_ctl_lightness);
        CONSOLE_OUT("[state] current Temperature: 0x%02X\n", appl_light_ctl[0].ctl_temperature);
        CONSOLE_OUT("[state] target Temperature: 0x%02X\n", appl_light_ctl[0].target_ctl_temperature);
        CONSOLE_OUT("[state] remaining_time: 0x%02X\n", appl_light_ctl[0].transition_time);
        /* Ignoring Instance and direction right now */
    }
    break;

    case MS_STATE_LIGHT_CTL_TEMPERATURE_RANGE_T:
    {
        MS_STATE_LIGHT_CTL_TEMPERATURE_RANGE_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_CTL_TEMPERATURE_RANGE_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_light_ctl_temp_set_range(0, param_p->ctl_temperature_range_min, param_p->ctl_temperature_range_max);
        param_p->status = 0x00;
    }
    break;

    case MS_STATE_LIGHT_CTL_TEMPERATURE_T:
    {
        MS_STATE_LIGHT_CTL_TEMPERATURE_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_CTL_TEMPERATURE_STRUCT*)param;
        printf("Set MS_STATE_LIGHT_CTL_TEMPERATURE_T: 0x%04X\n", param_p->ctl_temperature);

        /* Check if state transition is specified */
        if (0 != param_p->transition_time)
        {
            MS_ACCESS_STATE_TRANSITION_TYPE   transition;
            UINT16                            transition_time_handle;
            appl_light_ctl_temperature[0].target_ctl_temperature = param_p->ctl_temperature;
            appl_light_ctl_temperature[0].target_ctl_delta_uv = param_p->ctl_delta_uv;
            appl_light_ctl_temperature[0].transition_time = param_p->transition_time;
            transition.delay = param_p->delay;
            transition.transition_time = param_p->transition_time;
            transition.blob = NULL;
            transition.transition_start_cb = appl_light_ctl_temperature_transition_start_cb;
            transition.transition_complete_cb = appl_light_ctl_temperature_transition_complete_cb;
            MS_common_start_transition_timer
            (
                &transition,
                &transition_time_handle
            );
            /* Return value to indicate not sending status right now */
            retval = API_FAILURE;
        }
        else
        {
            /* Instantaneous Change */
            /* appl_light_ctl_temperature[0] = *param_p; */
            appl_light_ctl_temp_set_actual(0, param_p->ctl_temperature, param_p->ctl_delta_uv, MS_FALSE);
        }

        *param_p = appl_light_ctl_temperature[0];
        CONSOLE_OUT("[state] current Temperature: 0x%02X\n", appl_light_ctl_temperature[0].ctl_temperature);
        CONSOLE_OUT("[state] target Temperature: 0x%02X\n", appl_light_ctl_temperature[0].target_ctl_temperature);
        CONSOLE_OUT("[state] current Delta UV: 0x%02X\n", appl_light_ctl_temperature[0].ctl_delta_uv);
        CONSOLE_OUT("[state] target Delta UV: 0x%02X\n", appl_light_ctl_temperature[0].ctl_delta_uv);
        CONSOLE_OUT("[state] remaining_time: 0x%02X\n", appl_light_ctl_temperature[0].transition_time);
    }
    break;

    default:
        break;
    }

    return retval;
}

/** --------------------- Light - HSL ---- */
static void appl_model_light_hsl_states_initialization(void)
{
    UINT32 index;
    EM_mem_set (appl_light_hsl, 0, sizeof(appl_light_hsl));
    EM_mem_set(appl_light_hsl_range, 0, sizeof(appl_light_hsl_range));
    EM_mem_set(appl_light_hsl_default, 0, sizeof(appl_light_hsl_default));

    for (index = 0; index < MS_MAX_NUM_STATES; index++)
    {
        /* Light HSL Hue = Generic Level + 32768 */
        appl_light_hsl[index].hsl_hue = 32768;
        /* Light HSL Saturation = Generic Level + 32768 */
        appl_light_hsl[index].hsl_saturation = 32768;
    }

    #if 0

    for (index = 0; index < MS_MAX_NUM_STATES; index++)
    {
        appl_light_hsl_set_hue(index, 0x0000);
        appl_light_hsl_set_saturation(index, 0x0000);
    }

    #endif /* 0 */
}

static void appl_light_hsl_set_actual(UINT16 state_inst, UINT16 lightness, UINT16 hue, UINT16 saturation, UCHAR forced_publish)
{
    printf("appl_light_hsl_set_actual: L:0x%04X, H:0x%04X, S:0x%04X, F:%d\n",
           lightness, hue, saturation, forced_publish);

    if (lightness != appl_light_hsl[state_inst].hsl_lightness)
    {
        appl_light_hsl[state_inst].hsl_lightness = lightness;
        appl_light_hsl[state_inst].hsl_hue = hue;
        appl_light_hsl[state_inst].hsl_saturation = saturation;
        appl_light_hsl_server_publish(MS_STATE_LIGHT_HSL_T, state_inst);
        /* appl_light_lightness[state_inst].light_lightness_actual.lightness_actual = param_p->hsl_lightness; */
        appl_light_lightness_set_actual(state_inst, lightness, MS_FALSE);
    }
    else if (MS_TRUE == forced_publish)
    {
        appl_light_hsl[state_inst].hsl_lightness = lightness;
        appl_light_hsl[state_inst].hsl_hue = hue;
        appl_light_hsl[state_inst].hsl_saturation = saturation;
        appl_light_hsl_server_publish(MS_STATE_LIGHT_HSL_T, state_inst);
    }
}

static void appl_light_hsl_set_hue(UINT16 state_inst, UINT16 actual)
{
    UINT16 min, max;
    CONSOLE_OUT("[Set Light HSL HUE] Actual: 0x%04X\n", actual);
    min = appl_light_hsl_range[state_inst].hue_range_min;
    max = appl_light_hsl_range[state_inst].hue_range_max;

    if ((0 != min) && (actual < min))
    {
        actual = min;
    }
    else if ((0 != max) && (actual > max))
    {
        actual = max;
    }

    /* Generic Level = (Light CTL Temperature - T _MIN) * 65535 / (T_MAX - T_MIN) - 32768 */
    #if 0
    appl_set_generic_level(state_inst, actual - 32768, MS_TRUE, MS_FALSE);
    #else

    if (actual != appl_light_hsl[state_inst].hsl_hue)
    {
        appl_light_hsl[state_inst].hsl_hue = actual;
        appl_light_hsl_hue_server_publish(MS_STATE_LIGHT_HSL_HUE_T, state_inst);
        appl_set_generic_level(state_inst, actual - 32768, MS_TRUE, MS_FALSE);
    }

    #endif /* 0 */
}

static void appl_light_hsl_set_saturation(UINT16 state_inst, UINT16 actual)
{
    UINT16 min, max;
    min = appl_light_hsl_range[state_inst].saturation_range_min;
    max = appl_light_hsl_range[state_inst].saturation_range_max;

    if ((0 != min) && (actual < min))
    {
        actual = min;
    }
    else if ((0 != max) && (actual > max))
    {
        actual = max;
    }

    if (actual != appl_light_hsl[state_inst].hsl_saturation)
    {
        appl_light_hsl[state_inst].hsl_saturation = actual;
        appl_light_hsl_saturation_server_publish(MS_STATE_LIGHT_HSL_SATURATION_T, state_inst);
        /* Generic Level = (Light CTL Temperature - T _MIN) * 65535 / (T_MAX - T_MIN) - 32768 */
        appl_set_generic_level(state_inst, actual - 32768, MS_TRUE, MS_FALSE);
    }
}

static void appl_light_hsl_set_range(UINT16 state_inst, UINT16 hue_min, UINT16 hue_max, UINT16 saturation_min, UINT16 saturation_max)
{
    UINT16 actual_hue, actual_saturation;
    appl_light_hsl_range[state_inst].hue_range_min = hue_min;
    appl_light_hsl_range[state_inst].hue_range_max = hue_max;
    appl_light_hsl_range[state_inst].saturation_range_min = saturation_min;
    appl_light_hsl_range[state_inst].saturation_range_max = saturation_max;
    /* Check if actual to be updated */
    actual_hue = appl_light_hsl[state_inst].hsl_hue;
    appl_light_hsl_set_hue(state_inst, actual_hue);
    actual_saturation = appl_light_hsl[state_inst].hsl_hue;
    appl_light_hsl_set_saturation(state_inst, actual_saturation);
}

static void appl_light_hsl_transition_start_cb(void* blob)
{
}

static void appl_light_hsl_transition_complete_cb(void* blob)
{
    UINT16 state_t;
    UINT16 lightness, hue, saturation;
    state_t = (UINT16)((intptr_t)(blob));
    appl_light_hsl[0].transition_time = 0x00;

    switch(state_t)
    {
    case MS_STATE_LIGHT_HSL_T:
    {
        #if 0
        appl_light_hsl[0].hsl_hue = appl_light_hsl[0].target_hsl_hue;
        appl_light_hsl[0].hsl_saturation = appl_light_hsl[0].target_hsl_saturation;
        appl_light_hsl[0].hsl_lightness = appl_light_hsl[0].target_hsl_lightness;
        #else
        hue = appl_light_hsl[0].target_hsl_hue;
        saturation = appl_light_hsl[0].target_hsl_saturation;
        lightness = appl_light_hsl[0].target_hsl_lightness;
        #endif /* 0 */
        appl_light_lightness[0].light_lightness_actual.lightness_actual = appl_light_hsl[0].hsl_lightness;
        appl_light_hsl[0].target_hsl_hue = 0x0000;
        appl_light_hsl[0].target_hsl_saturation = 0x0000;
        appl_light_hsl[0].target_hsl_lightness = 0x0000;
        appl_light_hsl_set_actual(0, lightness, hue, saturation, MS_TRUE);
    }
    break;

    case MS_STATE_LIGHT_HSL_SATURATION_T:
    {
        appl_light_hsl_set_saturation(0, appl_light_hsl[0].target_hsl_saturation);
        appl_light_hsl[0].target_hsl_saturation = 0x0000;
    }
    break;

    case MS_STATE_LIGHT_HSL_HUE_T:
    {
        appl_light_hsl_set_hue(0, appl_light_hsl[0].target_hsl_hue);
        appl_light_hsl[0].target_hsl_hue = 0x0000;
    }
    break;
    }
}

static API_RESULT appl_model_light_hsl_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT                  ret;
    UINT8                       transition_time;

    switch(state_t)
    {
    case MS_STATE_LIGHT_HSL_T:
    {
        MS_STATE_LIGHT_HSL_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_HSL_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_light_hsl[0];

        if (0 != param_p->transition_time)
        {
            ret = MS_common_get_remaining_transition_time
                  (
                      param_p->transition_time_handle,
                      &transition_time
                  );

            if (API_SUCCESS == ret)
            {
                param_p->transition_time = transition_time;
            }
        }
    }
    break;

    case MS_STATE_LIGHT_HSL_TARGET_T:
    {
        MS_STATE_LIGHT_HSL_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_HSL_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_light_hsl[0];
    }
    break;

    case MS_STATE_LIGHT_HSL_RANGE_T:
    {
        MS_STATE_LIGHT_HSL_RANGE_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_HSL_RANGE_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_light_hsl_range[0];
    }
    break;

    case MS_STATE_LIGHT_HSL_DEFAULT_T:
    {
        MS_STATE_LIGHT_HSL_DEFAULT_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_HSL_DEFAULT_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_light_hsl_default[0];
    }
    break;

    case MS_STATE_LIGHT_HSL_HUE_T:
    {
        MS_STATE_LIGHT_HSL_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_HSL_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_light_hsl[0];

        if (0 != param_p->transition_time)
        {
            ret = MS_common_get_remaining_transition_time
                  (
                      param_p->transition_time_handle,
                      &transition_time
                  );

            if (API_SUCCESS == ret)
            {
                param_p->transition_time = transition_time;
            }
        }
    }
    break;

    case MS_STATE_LIGHT_HSL_SATURATION_T:
    {
        MS_STATE_LIGHT_HSL_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_HSL_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_light_hsl[0];

        if (0 != param_p->transition_time)
        {
            ret = MS_common_get_remaining_transition_time
                  (
                      param_p->transition_time_handle,
                      &transition_time
                  );

            if (API_SUCCESS == ret)
            {
                param_p->transition_time = transition_time;
            }
        }
    }
    break;

    default:
        break;
    }

    return API_SUCCESS;
}

static API_RESULT appl_model_light_hsl_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    retval = API_SUCCESS;

    switch(state_t)
    {
    case MS_STATE_LIGHT_HSL_T:
    {
        MS_STATE_LIGHT_HSL_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_HSL_STRUCT*)param;

        /* Check if state transition is specified */
        if (0 != param_p->transition_time)
        {
            MS_ACCESS_STATE_TRANSITION_TYPE   transition;
            appl_light_hsl[0].target_hsl_hue = param_p->hsl_hue;
            appl_light_hsl[0].target_hsl_saturation = param_p->hsl_saturation;
            appl_light_hsl[0].target_hsl_lightness = param_p->hsl_lightness;
            appl_light_hsl[0].transition_time = param_p->transition_time;
            transition.delay = param_p->delay;
            transition.transition_time = param_p->transition_time;
            transition.blob = (void*)((intptr_t)state_t);
            transition.transition_start_cb = appl_light_hsl_transition_start_cb;
            transition.transition_complete_cb = appl_light_hsl_transition_complete_cb;
            MS_common_start_transition_timer
            (
                &transition,
                &appl_light_hsl[0].transition_time_handle
            );
            /* Return value to indicate not sending status right now */
            retval = API_FAILURE;
        }
        else
        {
            CONSOLE_OUT("[prior state] current Hue: 0x%04X\n", appl_light_hsl[0].hsl_hue);
            CONSOLE_OUT("[prior state] current Saturation: 0x%04X\n", appl_light_hsl[0].hsl_saturation);
            CONSOLE_OUT("[prior state] current Lightness: 0x%04X\n", appl_light_hsl[0].hsl_lightness);
            #if 0
            appl_light_hsl[0].hsl_lightness = param_p->hsl_lightness;
            appl_light_hsl[0].hsl_hue = param_p->hsl_hue;
            appl_light_hsl[0].hsl_saturation = param_p->hsl_saturation;
            #else
            appl_light_hsl_set_actual(0, param_p->hsl_lightness, param_p->hsl_hue, param_p->hsl_saturation, MS_FALSE);
            #endif
            appl_light_hsl[0].tid = param_p->tid;
            /* appl_light_lightness[state_inst].light_lightness_actual.lightness_actual = param_p->hsl_lightness; */
            appl_light_xyl[state_inst].xyl_lightness = param_p->hsl_lightness;
        }

        *param_p = appl_light_hsl[0];
        CONSOLE_OUT("[state] current Hue: 0x%04X\n", appl_light_hsl[0].hsl_hue);
        CONSOLE_OUT("[state] current Saturation: 0x%04X\n", appl_light_hsl[0].hsl_saturation);
        CONSOLE_OUT("[state] current Lightness: 0x%04X\n", appl_light_hsl[0].hsl_lightness);
        CONSOLE_OUT("[state] target Hue: 0x%04X\n", appl_light_hsl[0].target_hsl_hue);
        CONSOLE_OUT("[state] target Saturation: 0x%04X\n", appl_light_hsl[0].target_hsl_saturation);
        CONSOLE_OUT("[state] target Lightness: 0x%04X\n", appl_light_hsl[0].target_hsl_lightness);
        CONSOLE_OUT("[state] remaining_time: 0x%02X\n", appl_light_hsl[0].transition_time);
        /* Ignoring Instance and direction right now */
    }
    break;

    case MS_STATE_LIGHT_HSL_RANGE_T:
    {
        MS_STATE_LIGHT_HSL_RANGE_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_HSL_RANGE_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_light_hsl_set_range(0, param_p->hue_range_min, param_p->hue_range_max, param_p->saturation_range_min, param_p->saturation_range_max);
        param_p->status = 0x00;
    }
    break;

    case MS_STATE_LIGHT_HSL_DEFAULT_T:
    {
        MS_STATE_LIGHT_HSL_DEFAULT_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_HSL_DEFAULT_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_light_hsl_default[0] = *param_p;
    }
    break;

    case MS_STATE_LIGHT_HSL_HUE_T:
    {
        MS_STATE_LIGHT_HSL_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_HSL_STRUCT*)param;

        if (0 != param_p->transition_time)
        {
            MS_ACCESS_STATE_TRANSITION_TYPE   transition;
            appl_light_hsl[0].target_hsl_hue = param_p->hsl_hue;
            appl_light_hsl[0].transition_time = param_p->transition_time;
            transition.delay = param_p->delay;
            transition.transition_time = param_p->transition_time;
            transition.blob = (void*)((intptr_t)state_t);
            transition.transition_start_cb = appl_light_hsl_transition_start_cb;
            transition.transition_complete_cb = appl_light_hsl_transition_complete_cb;
            MS_common_start_transition_timer
            (
                &transition,
                &appl_light_hsl[0].transition_time_handle
            );
            /* Return value to indicate not sending status right now */
            retval = API_FAILURE;
        }
        else
        {
            /* Ignoring Instance and direction right now */
            appl_light_hsl_set_hue(0, param_p->hsl_hue);
        }

        *param_p = appl_light_hsl[0];
        CONSOLE_OUT("[state] current Hue: 0x%04X\n", appl_light_hsl[0].hsl_hue);
        CONSOLE_OUT("[state] target Hue: 0x%04X\n", appl_light_hsl[0].target_hsl_hue);
        CONSOLE_OUT("[state] remaining_time: 0x%02X\n", appl_light_hsl[0].transition_time);
    }
    break;

    case MS_STATE_LIGHT_HSL_SATURATION_T:
    {
        MS_STATE_LIGHT_HSL_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_HSL_STRUCT*)param;

        /* Check if state transition is specified */
        if (0 != param_p->transition_time)
        {
            MS_ACCESS_STATE_TRANSITION_TYPE   transition;
            UINT16                            transition_time_handle;
            appl_light_hsl[0].target_hsl_saturation = param_p->hsl_saturation;
            appl_light_hsl[0].transition_time = param_p->transition_time;
            transition.delay = param_p->delay;
            transition.transition_time = param_p->transition_time;
            transition.blob = (void*)((intptr_t)state_t);
            transition.transition_start_cb = appl_light_hsl_transition_start_cb;
            transition.transition_complete_cb = appl_light_hsl_transition_complete_cb;
            MS_common_start_transition_timer
            (
                &transition,
                &transition_time_handle
            );
            /* Return value to indicate not sending status right now */
            retval = API_FAILURE;
        }
        else
        {
            /* Ignoring Instance and direction right now */
            appl_light_hsl_set_saturation(0, param_p->hsl_saturation);
        }

        *param_p = appl_light_hsl[0];
        CONSOLE_OUT("[state] current Saturation: 0x%04X\n", appl_light_hsl[0].hsl_saturation);
        CONSOLE_OUT("[state] target Saturation: 0x%04X\n", appl_light_hsl[0].target_hsl_saturation);
        CONSOLE_OUT("[state] remaining_time: 0x%02X\n", appl_light_hsl[0].transition_time);
    }
    break;

    default:
        break;
    }

    return retval;
}

/** --------------------- Light - xyL ---- */
static void appl_model_light_xyl_states_initialization(void)
{
    #if 0
    UINT32 index;
    #endif /* 0 */
    EM_mem_set (appl_light_xyl, 0, sizeof(appl_light_xyl));
    EM_mem_set(appl_light_xyl_default, 0, sizeof(appl_light_xyl_default));
    EM_mem_set(appl_light_xyl_range, 0, sizeof(appl_light_xyl_range));
    #if 0

    for (index = 0; index < MS_MAX_NUM_STATES; index++)
    {
        appl_light_xyl_set_x(index, 0x0000);
        appl_light_xyl_set_y(index, 0x0000);
    }

    #endif /* 0 */
}

static void appl_light_xyl_set_x(UINT16 state_inst, UINT16 actual)
{
    #if 0
    UINT16 min, max;
    min = appl_light_xyl_range[state_inst].xyl_x_range_min;
    max = appl_light_xyl_range[state_inst].xyl_x_range_max;

    if ((0 != min) && (actual < min))
    {
        actual = min;
    }
    else if ((0 != max) && (actual > max))
    {
        actual = max;
    }

    #endif /* 0 */
    appl_light_xyl[state_inst].xyl_x = actual;
}

static void appl_light_xyl_set_y(UINT16 state_inst, UINT16 actual)
{
    #if 0
    UINT16 min, max;
    min = appl_light_xyl_range[state_inst].xyl_y_range_min;
    max = appl_light_xyl_range[state_inst].xyl_y_range_max;

    if ((0 != min) && (actual < min))
    {
        actual = min;
    }
    else if ((0 != max) && (actual > max))
    {
        actual = max;
    }

    #endif /* 0 */
    appl_light_xyl[state_inst].xyl_y = actual;
}

static void appl_light_xyl_set_actual(UINT16 state_inst, UINT16 xyl_lightness, UINT16 xyl_x, UINT16 xyl_y)
{
    /* if (xyl_lightness != appl_light_xyl[state_inst].xyl_lightness) */
    {
        appl_light_xyl[state_inst].xyl_lightness = xyl_lightness;
        appl_light_xyl_set_x(state_inst, xyl_x);
        appl_light_xyl_set_y(state_inst, xyl_y);
        /* Publish */
        appl_light_xyl_server_publish(MS_STATE_LIGHT_XYL_T, 0);
    }
}

static void appl_light_xyl_set_range(UINT16 state_inst, UINT16 x_min, UINT16 x_max, UINT16 y_min, UINT16 y_max)
{
    UINT16 actual_x, actual_y;
    appl_light_xyl_range[state_inst].xyl_x_range_min = x_min;
    appl_light_xyl_range[state_inst].xyl_x_range_max = x_max;
    appl_light_xyl_range[state_inst].xyl_y_range_min = y_min;
    appl_light_xyl_range[state_inst].xyl_y_range_max= y_max;
    /* Check if actual to be updated */
    actual_x = appl_light_xyl[state_inst].xyl_x;
    appl_light_xyl_set_x(state_inst, actual_x);
    actual_y = appl_light_xyl[state_inst].xyl_y;
    appl_light_xyl_set_y(state_inst, actual_y);
}

static void appl_light_xyl_transition_start_cb(void* blob)
{
}

static void appl_light_xyl_transition_complete_cb(void* blob)
{
    UINT16 state_t;
    UINT16 xyl_lightness, xyl_x, xyl_y;
    state_t = (UINT16)((intptr_t)(blob));
    appl_light_xyl[0].transition_time = 0x00;

    switch(state_t)
    {
    case MS_STATE_LIGHT_XYL_T:
    {
        xyl_x = appl_light_xyl[0].target_xyl_x;
        xyl_y = appl_light_xyl[0].target_xyl_y;
        xyl_lightness = appl_light_xyl[0].target_xyl_lightness;
        appl_light_hsl[0].hsl_lightness = appl_light_xyl[0].xyl_lightness;
        appl_light_xyl[0].target_xyl_x = 0x0000;
        appl_light_xyl[0].target_xyl_y = 0x0000;
        appl_light_xyl[0].target_xyl_lightness = 0x0000;
        appl_light_xyl_set_actual(0, xyl_lightness, xyl_x, xyl_y);
    }
    break;
    }
}

static API_RESULT appl_model_light_xyl_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT                  ret;
    UINT8                       transition_time;

    switch(state_t)
    {
    case MS_STATE_LIGHT_XYL_TARGET_T:
    {
        MS_STATE_LIGHT_XYL_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_XYL_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_light_xyl[0];
    }
    break;

    case MS_STATE_LIGHT_XYL_T:
    {
        MS_STATE_LIGHT_XYL_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_XYL_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_light_xyl[0];

        if (0 != param_p->transition_time)
        {
            ret = MS_common_get_remaining_transition_time
                  (
                      param_p->transition_time_handle,
                      &transition_time
                  );

            if (API_SUCCESS == ret)
            {
                param_p->transition_time = transition_time;
            }
        }
    }
    break;

    case MS_STATE_LIGHT_XYL_DEFAULT_T:
    {
        MS_STATE_LIGHT_XYL_DEFAULT_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_XYL_DEFAULT_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_light_xyl_default[0];
    }
    break;

    case MS_STATE_LIGHT_XYL_RANGE_T:
    {
        MS_STATE_LIGHT_XYL_RANGE_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_XYL_RANGE_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_light_xyl_range[0];
        param_p->status = 0x00;
    }
    break;

    default:
        break;
    }

    return API_SUCCESS;
}

static API_RESULT appl_model_light_xyl_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    retval = API_SUCCESS;

    switch(state_t)
    {
    case MS_STATE_LIGHT_XYL_T:
    {
        MS_STATE_LIGHT_XYL_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_XYL_STRUCT*)param;

        /* Check if state transition is specified */
        if (0 != param_p->transition_time)
        {
            MS_ACCESS_STATE_TRANSITION_TYPE   transition;
            appl_light_xyl[0].target_xyl_lightness = param_p->xyl_lightness;
            appl_light_xyl[0].target_xyl_x = param_p->xyl_x;
            appl_light_xyl[0].target_xyl_y = param_p->xyl_y;
            appl_light_xyl[0].transition_time = param_p->transition_time;
            transition.delay = param_p->delay;
            transition.transition_time = param_p->transition_time;
            transition.blob = (void*)((intptr_t)(state_t));
            transition.transition_start_cb = appl_light_xyl_transition_start_cb;
            transition.transition_complete_cb = appl_light_xyl_transition_complete_cb;
            MS_common_start_transition_timer
            (
                &transition,
                &appl_light_xyl[0].transition_time_handle
            );
            /* Return value to indicate not sending status right now */
            retval = API_FAILURE;
        }
        else
        {
            appl_light_xyl_set_actual(0, param_p->xyl_lightness, param_p->xyl_x, param_p->xyl_y);
            appl_light_hsl[0].hsl_lightness = param_p->xyl_lightness;
        }

        *param_p = appl_light_xyl[0];
        CONSOLE_OUT("[state] current X: 0x%04X\n", appl_light_xyl[0].xyl_x);
        CONSOLE_OUT("[state] current Y: 0x%04X\n", appl_light_xyl[0].xyl_y);
        CONSOLE_OUT("[state] current Lightness: 0x%04X\n", appl_light_xyl[0].xyl_lightness);
        CONSOLE_OUT("[state] target X: 0x%04X\n", appl_light_xyl[0].target_xyl_x);
        CONSOLE_OUT("[state] target Y: 0x%04X\n", appl_light_xyl[0].target_xyl_y);
        CONSOLE_OUT("[state] target Lightness: 0x%04X\n", appl_light_xyl[0].target_xyl_lightness);
        CONSOLE_OUT("[state] remaining_time: 0x%02X\n", appl_light_xyl[0].transition_time);
        /* Ignoring Instance and direction right now */
    }
    break;

    case MS_STATE_LIGHT_XYL_DEFAULT_T:
    {
        MS_STATE_LIGHT_XYL_DEFAULT_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_XYL_DEFAULT_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_light_xyl_default[0] = *param_p;
    }
    break;

    case MS_STATE_LIGHT_XYL_RANGE_T:
    {
        MS_STATE_LIGHT_XYL_RANGE_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_XYL_RANGE_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_light_xyl_set_range(0, param_p->xyl_x_range_min, param_p->xyl_x_range_max, param_p->xyl_y_range_min, param_p->xyl_y_range_max);
        param_p->status = 0x00;
    }
    break;

    default:
        break;
    }

    return retval;
}

/** --------------------- Light - LC ---- */
static void appl_model_light_lc_states_initialization(void)
{
    UINT32 index;
    EM_mem_set(appl_light_lc_mode, 0, sizeof(appl_light_lc_mode));
    EM_mem_set(appl_light_lc_om, 0, sizeof(appl_light_lc_om));
    EM_mem_set(appl_light_lc_onoff, 0, sizeof(appl_light_lc_onoff));
    EM_mem_set(appl_light_lc_property, 0, sizeof(appl_light_lc_property));

    for (index = 0; index < MS_MAX_LC_PROPERTIES; index++)
    {
        /* TODO: For PTS */
        appl_light_lc_property[0][index].property_id = index + 0x36 /* 1 */;
        /* Allocate and keep some property value */
        appl_light_lc_property[0][index].property_value_len = 3 /* 1 */;
        appl_light_lc_property[0][index].property_value = EM_alloc_mem(3 /* 1 */);
        /* TODO: Not checking for memeory allocation failure */
        EM_mem_set(appl_light_lc_property[0][index].property_value, 0x00, (3 /* 1 */));
    }

    appl_current_property_id_index = 0;
}

static void appl_light_lc_set_actual(UINT16 state_inst, UINT16 state_t, UCHAR onoff, UCHAR mode)
{
    /* if (xyl_lightness != appl_light_xyl[state_inst].xyl_lightness) */
    switch(state_t)
    {
    case MS_STATE_LIGHT_LC_MODE_T:
    {
        appl_light_lc_mode[state_inst].present_mode = mode;
    }
    break;

    case MS_STATE_LIGHT_LC_OM_T:
    {
        appl_light_lc_om[state_inst].present_mode = mode;
    }
    break;

    case MS_STATE_LIGHT_LC_LIGHT_ONOFF_T:
    {
        appl_light_lc_onoff[state_inst].present_light_onoff = onoff;
    }
    break;

    default:
        return;
    }

    /* Publish */
    appl_light_lc_server_publish(state_t, state_inst, 0x0000);
}

static API_RESULT appl_light_lc_set_actual_propery(UINT16 state_inst, UINT16 property_id, UCHAR* property_value, UINT16 property_value_len)
{
    UINT32 index;
    API_RESULT retval;
    retval = API_FAILURE;

    for (index = 0; index < MS_MAX_LC_PROPERTIES; index++)
    {
        if (property_id == appl_light_lc_property[state_inst][index].property_id)
        {
            if (appl_light_lc_property[state_inst][index].property_value_len == property_value_len)
            {
                EM_mem_copy
                (
                    appl_light_lc_property[state_inst][index].property_value,
                    property_value,
                    property_value_len
                );
                appl_current_property_id_index = index;
                retval = API_SUCCESS;
            }

            break;
        }
    }

    if (API_SUCCESS == retval)
    {
        /* Publish */
        appl_light_lc_server_publish(MS_STATE_LIGHT_LC_PROPERTY_T, state_inst, property_id);
    }

    return retval;
}

static void appl_light_lc_onoff_transition_start_cb(void* blob)
{
    /**
        Because binary states cannot support transitions, when changing to 0x01 (On),
        the Generic OnOff state shall change immediately when the transition starts,
        and when changing to 0x00, the state shall change when the transition finishes.
    */
    if (0 == appl_light_lc_onoff[0].present_light_onoff)
    {
        appl_light_lc_onoff[0].present_light_onoff = appl_light_lc_onoff[0].target_light_onoff;
    }
}

static void appl_light_lc_onoff_transition_complete_cb(void* blob)
{
    /* State Transition Complete */
    appl_light_lc_onoff[0].transition_time = 0;
    appl_light_lc_onoff[0].present_light_onoff = appl_light_lc_onoff[0].target_light_onoff;
    appl_light_lc_onoff[0].target_light_onoff = 0;
}

API_RESULT appl_model_light_lc_server_set_default_trans_timeout_in_ms(/* IN */ UINT32 time_in_ms)
{
    /* Covert as octet stream and save as Light Control Time Fade property */
    if (MS_DEV_PROP_LIGHT_CONTROL_TIME_FADE == appl_light_lc_property[0][0].property_id)
    {
        MS_PACK_LE_3_BYTE_VAL(appl_light_lc_property[0][0].property_value, time_in_ms);
    }

    return API_SUCCESS;
}

static API_RESULT appl_model_light_lc_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    retval = API_SUCCESS;

    switch(state_t)
    {
    case MS_STATE_LIGHT_LC_MODE_T:
    {
        MS_STATE_LIGHT_LC_MODE_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_LC_MODE_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        param_p->present_mode = appl_light_lc_mode[0].present_mode;
    }
    break;

    case MS_STATE_LIGHT_LC_OM_T:
    {
        MS_STATE_LIGHT_LC_OM_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_LC_OM_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        param_p->present_mode = appl_light_lc_om[0].present_mode;
    }
    break;

    case MS_STATE_LIGHT_LC_LIGHT_ONOFF_T:
    {
        MS_STATE_LIGHT_LC_LIGHT_ONOFF_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_LC_LIGHT_ONOFF_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_light_lc_onoff[0];
    }
    break;

    case MS_STATE_LIGHT_LC_PROPERTY_T:
    {
        MS_STATE_LIGHT_LC_PROPERTY_STRUCT*   param_p;
        UINT32 index;
        param_p = (MS_STATE_LIGHT_LC_PROPERTY_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        /* Search for matching property ID */
        param_p->property_value_len = 0x00;

        for (index = 0; index < MS_MAX_LC_PROPERTIES; index++)
        {
            if (param_p->property_id == appl_light_lc_property[0][index].property_id)
            {
                param_p->property_value_len = appl_light_lc_property[0][index].property_value_len;
                param_p->property_value = appl_light_lc_property[0][index].property_value;
                break;
            }
        }

        /* Else Return Error */
    }
    break;

    default:
        break;
    }

    return retval;
}

static API_RESULT appl_model_light_lc_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
//    UINT32 index;
    API_RESULT retval;
    retval = API_SUCCESS;

    switch(state_t)
    {
    case MS_STATE_LIGHT_LC_MODE_T:
    {
        MS_STATE_LIGHT_LC_MODE_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_LC_MODE_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_light_lc_set_actual(0, MS_STATE_LIGHT_LC_MODE_T, 0x00, param_p->target_mode);
        param_p->present_mode = appl_light_lc_mode[0].present_mode;
    }
    break;

    case MS_STATE_LIGHT_LC_OM_T:
    {
        MS_STATE_LIGHT_LC_OM_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_LC_OM_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_light_lc_set_actual(0, MS_STATE_LIGHT_LC_OM_T, 0x00, param_p->target_mode);
        param_p->present_mode = appl_light_lc_om[0].present_mode;
    }
    break;

    case MS_STATE_LIGHT_LC_LIGHT_ONOFF_T:
    {
        MS_STATE_LIGHT_LC_LIGHT_ONOFF_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_LC_LIGHT_ONOFF_STRUCT*)param;

        if (0 == param_p->transition_time)
        {
            UINT32 time_in_ms;
            UINT8  transition_time;

            /* Check Light Control Time Fade property */
            /* Convert to Transition Time */
            if (MS_DEV_PROP_LIGHT_CONTROL_TIME_FADE == appl_light_lc_property[0][0].property_id)
            {
                MS_UNPACK_LE_3_BYTE(&time_in_ms, appl_light_lc_property[0][0].property_value);
                MS_common_get_transition_time_from_ms
                (
                    time_in_ms,
                    &transition_time
                );
                param_p->transition_time = transition_time;
                param_p->delay = 0;
            }
        }

        /* Check if state transition is specified */
        if (0 != param_p->transition_time)
        {
            MS_ACCESS_STATE_TRANSITION_TYPE   transition;
            UINT16                            transition_time_handle;
            appl_light_lc_onoff[0].target_light_onoff = param_p->target_light_onoff;
            appl_light_lc_onoff[0].transition_time = param_p->transition_time;
            transition.delay = param_p->delay;
            transition.transition_time = param_p->transition_time;
            transition.blob = NULL;
            transition.transition_start_cb = appl_light_lc_onoff_transition_start_cb;
            transition.transition_complete_cb = appl_light_lc_onoff_transition_complete_cb;
            MS_common_start_transition_timer
            (
                &transition,
                &transition_time_handle
            );
            /* Return value to indicate not sending status right now */
            retval = API_FAILURE;
        }
        else
        {
            /* Instantaneous Change */
            /* Ignoring Instance and direction right now */
            appl_light_lc_set_actual(0, MS_STATE_LIGHT_LC_LIGHT_ONOFF_T, param_p->target_light_onoff, 0x00);
            appl_set_generic_onoff(state_inst, param_p->target_light_onoff, MS_FALSE);
            appl_light_lightness_set_linear(0, param_p->target_light_onoff);
        }

        *param_p = appl_light_lc_onoff[0];
        CONSOLE_OUT("[state] current: 0x%02X\n", appl_light_lc_onoff[0].present_light_onoff);
        CONSOLE_OUT("[state] target: 0x%02X\n", appl_light_lc_onoff[0].target_light_onoff);
        CONSOLE_OUT("[state] remaining_time: 0x%02X\n", appl_light_lc_onoff[0].transition_time);
        /* TODO: Check for timer and start */
    }
    break;

    case MS_STATE_LIGHT_LC_PROPERTY_T:
    {
        MS_STATE_LIGHT_LC_PROPERTY_STRUCT* param_p;
        param_p = (MS_STATE_LIGHT_LC_PROPERTY_STRUCT*)param;
        retval = appl_light_lc_set_actual_propery(state_inst, param_p->property_id, param_p->property_value, param_p->property_value_len);

        if (API_SUCCESS != retval)
        {
            param_p->property_value_len = 0x00;
        }

        /* TODO: */
    }
    break;

    default:
        break;
    }

    return retval;
}

/** --------------------------------------------------------------------------------- State Initilization ---- */
void appl_model_states_initialization(void)
{
    /* Generics - OnOff */
    appl_generic_onoff_states_initialization();
    /* Generics - Level */
    appl_model_generic_level_states_initialization();
    /* Generics - Power OnOff */
    appl_model_generic_power_onoff_states_initialization();
    /* Generics - Power Level */
    appl_model_power_level_states_initialization();
    /* Generics - Battery */
    appl_model_generic_battery_states_initialization();
    /* Generics - Location */
    appl_model_generic_location_states_initialization();
    /* Generics - Properties */
    appl_model_generic_property_states_initialization();
    /* Scene - States */
    EM_mem_set(appl_state_info_vault, 0, sizeof(appl_state_info_vault));
    /* Light - Lightness */
    appl_model_light_lightness_states_initialization();
    /* Light - CTL */
    appl_model_light_ctl_states_initialization();
    /* Light - HSL */
    appl_model_light_hsl_states_initialization();
    /* Light - xyL */
    appl_model_light_xyl_states_initialization();
    /* Light - LC */
    appl_model_light_lc_states_initialization();
    /* Initialize Persistent Storage for application */
    appl_ps_init();
    /* Load from Persistent Storage */
    appl_ps_load(MS_PS_APPL_ALL_RECORDS);
    appl_in_transtion = 0x00;
    (void)appl_in_transtion;
}

/** ------------------------------------------------------------------------ Power Cycle - Emulation ---- */
void appl_model_power_cycle(void)
{
    /* Save the OnPowerUp state and if required other states */
    #if 1
    appl_ps_store(MS_PS_RECORD_APPL_ON_POWER);
    #else
    appl_ps_store(MS_PS_APPL_ALL_RECORDS);
    #endif /* 0 */
    #if 0
    appl_generic_onoff[0].transition_time = 0x00;
    appl_generic_onoff[0].target_onoff = 0x00;

    /* */
    if (0x01 == appl_generic_onpower[0].onpowerup)
    {
        appl_set_generic_onoff(0, 0x01, MS_FALSE);
    }
    else if (0x00 == appl_generic_onpower[0].onpowerup)
    {
        appl_set_generic_onoff(0, 0x00, MS_FALSE);
    }
    else
    {
        appl_generic_onoff[0].onoff = appl_generic_onoff[0].last_onoff;
    }

    /* TODO: Hack */
    if (0x00 != appl_light_lightness[0].light_lightness_actual.transition_time)
    {
        appl_light_lightness[0].light_lightness_actual.lightness_actual = appl_light_lightness[0].light_lightness_actual.lightness_target;
        appl_light_lightness[0].light_lightness_actual.transition_time = 0x00;
        appl_light_lightness[0].light_lightness_actual.lightness_target = 0x00;
    }

    #endif /* 0 */
}

/** ------------------------------------------------------------------------ State Get - Handler ---- */
API_RESULT appl_model_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    CONSOLE_OUT("[State Get] State_T:0x%04X, State_Inst:0x%04X\n",
                state_t, state_inst);
    retval = API_SUCCESS;

    switch (state_t)
    {
    case MS_STATE_GENERIC_ONOFF_T:
    {
        retval = appl_model_generic_onoff_state_get(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_GENERIC_LEVEL_T:
    {
        retval = appl_model_generic_level_state_get(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_GENERIC_ONPOWERUP_T:
    {
        retval = appl_model_generic_power_onoff_state_get(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_GENERIC_POWER_ACTUAL_T:
    case MS_STATE_GENERIC_POWER_LAST_T:
    case MS_STATE_GENERIC_POWER_DEFAULT_T:
    case MS_STATE_GENERIC_POWER_RANGE_T:
    {
        retval = appl_model_generic_power_level_state_get(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_GENERIC_BATTERY_T:
    {
        retval = appl_model_generic_battery_state_get(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_GENERIC_LOCATION_GLOBAL_T:
    case MS_STATE_GENERIC_LOCATION_LOCAL_T:
    {
        retval = appl_model_generic_location_state_get(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_GENERIC_USER_PROPERTY_IDS_T:
    case MS_STATE_GENERIC_ADMIN_PROPERTY_IDS_T:
    case MS_STATE_GENERIC_MANUFACTURER_PROPERTY_IDS_T:
    case MS_STATE_GENERIC_CLIENT_PROPERTY_IDS_T:
    case MS_STATE_GENERIC_USER_PROPERTY_T:
    case MS_STATE_GENERIC_ADMIN_PROPERTY_T:
    case MS_STATE_GENERIC_MANUFACTURER_PROPERTY_T:
    {
        retval = appl_model_generic_property_state_get(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_DEFAULT_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_RANGE_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_LINEAR_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_LAST_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_ACTUAL_T:
    {
        retval = appl_model_light_lightness_state_get(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_LIGHT_CTL_DEFAULT_T: /* Fall Through */
    case MS_STATE_LIGHT_CTL_T: /* Fall Through */
    case MS_STATE_LIGHT_CTL_TEMPERATURE_RANGE_T: /* Fall Through */
    case MS_STATE_LIGHT_CTL_TEMPERATURE_T:
    {
        retval = appl_model_light_ctl_state_get(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_LIGHT_HSL_T: /* Fall Through */
    case MS_STATE_LIGHT_HSL_TARGET_T: /* Fall Through */
    case MS_STATE_LIGHT_HSL_RANGE_T: /* Fall Through */
    case MS_STATE_LIGHT_HSL_DEFAULT_T: /* Fall Through */
    case MS_STATE_LIGHT_HSL_HUE_T: /* Fall Through */
    case MS_STATE_LIGHT_HSL_SATURATION_T:
    {
        retval = appl_model_light_hsl_state_get(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_LIGHT_XYL_T: /* Fall Through */
    case MS_STATE_LIGHT_XYL_TARGET_T: /* Fall Through */
    case MS_STATE_LIGHT_XYL_DEFAULT_T: /* Fall Through */
    case MS_STATE_LIGHT_XYL_RANGE_T:
    {
        retval = appl_model_light_xyl_state_get(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_LIGHT_LC_MODE_T:
    case MS_STATE_LIGHT_LC_OM_T:
    case MS_STATE_LIGHT_LC_LIGHT_ONOFF_T:
    case MS_STATE_LIGHT_LC_PROPERTY_T:
    {
        retval = appl_model_light_lc_state_get(state_t, state_inst, param, direction);
    }
    break;

    default:
        break;
    }

    return retval;
}

/** ------------------------------------------------------------------------ State Set - Handler ---- */
API_RESULT appl_model_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    retval = API_SUCCESS;

    switch (state_t)
    {
    case MS_STATE_GENERIC_ONOFF_T:
    {
        retval = appl_model_generic_onoff_state_set(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_GENERIC_LEVEL_T: /* Fall Through */
    case MS_STATE_DELTA_LEVEL_T:   /* Fall Through */
    case MS_STATE_MOVE_LEVEL_T:
    {
        retval = appl_model_generic_level_state_set(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_GENERIC_ONPOWERUP_T:
    {
        retval = appl_model_generic_power_onoff_state_set(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_GENERIC_POWER_ACTUAL_T:
    case MS_STATE_GENERIC_POWER_LAST_T:
    case MS_STATE_GENERIC_POWER_DEFAULT_T:
    case MS_STATE_GENERIC_POWER_RANGE_T:
    {
        retval = appl_model_generic_power_level_state_set(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_GENERIC_BATTERY_T:
    {
        retval = appl_model_generic_battery_state_set(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_GENERIC_LOCATION_GLOBAL_T:
    case MS_STATE_GENERIC_LOCATION_LOCAL_T:
    {
        retval = appl_model_generic_location_state_set(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_GENERIC_USER_PROPERTY_T:
    case MS_STATE_GENERIC_ADMIN_PROPERTY_T:
    case MS_STATE_GENERIC_MANUFACTURER_PROPERTY_T:
    {
        retval = appl_model_generic_property_state_set(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_DEFAULT_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_RANGE_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_LINEAR_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_LAST_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_ACTUAL_T:
    {
        retval = appl_model_light_lightness_state_set(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_LIGHT_CTL_DEFAULT_T: /* Fall Through */
    case MS_STATE_LIGHT_CTL_T: /* Fall Through */
    case MS_STATE_LIGHT_CTL_TEMPERATURE_RANGE_T: /* Fall Through */
    case MS_STATE_LIGHT_CTL_TEMPERATURE_T:
    {
        retval = appl_model_light_ctl_state_set(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_LIGHT_HSL_T: /* Fall Through */
    case MS_STATE_LIGHT_HSL_TARGET_T: /* Fall Through */
    case MS_STATE_LIGHT_HSL_RANGE_T: /* Fall Through */
    case MS_STATE_LIGHT_HSL_DEFAULT_T: /* Fall Through */
    case MS_STATE_LIGHT_HSL_HUE_T: /* Fall Through */
    case MS_STATE_LIGHT_HSL_SATURATION_T:
    {
        retval = appl_model_light_hsl_state_set(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_LIGHT_XYL_T: /* Fall Through */
    case MS_STATE_LIGHT_XYL_TARGET_T: /* Fall Through */
    case MS_STATE_LIGHT_XYL_RANGE_T: /* Fall Through */
    case MS_STATE_LIGHT_XYL_DEFAULT_T: /* Fall Through */
    {
        retval = appl_model_light_xyl_state_set(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_LIGHT_LC_MODE_T:
    case MS_STATE_LIGHT_LC_OM_T:
    case MS_STATE_LIGHT_LC_LIGHT_ONOFF_T:
    case MS_STATE_LIGHT_LC_PROPERTY_T:
    {
        retval = appl_model_light_lc_state_set(state_t, state_inst, param, direction);
    }
    break;

    default:
        break;
    }

    return retval;
}

/** PS Storage Interface for the applicaiton */
#ifdef MS_STORAGE

/** Data structure related to persistent storage (PS) load/store */
typedef struct _MS_PS_APPL_RECORDS
{
    /** Information to load/store */
    void* info;

    /** Start Offset in PS */
    UINT32 start_offset;

    /** Size of Information (in bytes) */
    UINT32 size;

    /** Function Pointer to Store */
    void(*store) (void);

    /** Function Pointer to Load */
    void(*load) (void);

} MS_PS_APPL_RECORDS;

/* Store Scene States */
static void ms_store_appl_scenes(void);
/* Store OnPower States */
static void ms_store_appl_on_power(void);

/* Load Scene States */
static void ms_load_appl_scenes(void);
/* Load OnPower States */
static void ms_load_appl_on_power(void);

/**
    List of persistent store records.
*/
static MS_PS_APPL_RECORDS ms_ps_appl_records[] =
{
    /** Information to load/store, Start Offset, Size, Store, Load */
    {NULL, 0, 0, ms_store_appl_scenes, ms_load_appl_scenes},
    {NULL, 0, 0, ms_store_appl_on_power,   ms_load_appl_on_power}
};

static NVSTO_HANDLE appl_nvsto_handle;

/* Initialize the access ps store */
static void appl_ps_init (void)
{
    UINT32       offset;
    UINT32       scene_size;
    MS_access_ps_get_handle_and_offset(&appl_nvsto_handle, &offset);
    scene_size = sizeof(appl_state_info_vault);
    /** Information to load/store, Start Offset, Size, Store, Load */
    ms_ps_appl_records[0].start_offset = 0;
    ms_ps_appl_records[0].size = scene_size;
    ms_ps_appl_records[1].start_offset = scene_size;
    ms_ps_appl_records[1].size = 100; /* TODO: See the impact of giving a random value for the size of the last blob in the PS */
    /* Register with the NV Storage */
    nvsto_register_ps
    (
        scene_size + 100,
        &appl_nvsto_handle
    );
}

/* Store Scene States */
static void ms_store_appl_scenes(void)
{
    /* Write Scenes */
    nvsto_write_ps
    (
        appl_nvsto_handle,
        &appl_state_info_vault[0],
        sizeof(appl_state_info_vault)
    );
}

/* Store OnPower States */
static void ms_store_appl_on_power(void)
{
    /* Write OnPowerUp state */
    CONSOLE_OUT
    ("[APPL_PS]: ms_store_appl_on_power. OnPowerUp 0x%02X\n",
     appl_generic_onpower[0].onpowerup);
    nvsto_write_ps
    (
        appl_nvsto_handle,
        &appl_generic_onpower[0].onpowerup,
        sizeof(appl_generic_onpower[0].onpowerup)
    );

    /* Write other States, only if OnPowerUp is 0x02 */
    if (0x02 == appl_generic_onpower[0].onpowerup)
    {
        /* TODO */
        appl_scene_save_current_state_ex(16);
        nvsto_write_ps
        (
            appl_nvsto_handle,
            &appl_state_info_vault[16],
            sizeof(appl_state_info_vault[16])
        );
    }
}

static void appl_ps_store(/* IN */ UINT32 records)
{
    UINT32 index;
    INT16 ret;
    CONSOLE_OUT
    ("[APPL_PS]: PS Store. Records 0x%08X\n", records);
    /* Open the storage */
    ret = nvsto_open_pswrite(appl_nvsto_handle);

    if (0 > ret)
    {
        CONSOLE_OUT
        ("[APPL_PS]: PS Open Failed\n");
        return;
    }

    /** Store information in listed order */
    for (index = 0; index < MS_PS_APPL_MAX_RECORDS; index++)
    {
        if (0 != (records & (1 << index)))
        {
            /* Seek to the corresponding location in PS */
            nvsto_seek_ps(appl_nvsto_handle, ms_ps_appl_records[index].start_offset);
            /* Only using the load/store function pointers */
            ms_ps_appl_records[index].store();
        }
    }

    /* Close the storage */
    nvsto_close_ps(appl_nvsto_handle);
    return;
}

/* Load Scene States */
static void ms_load_appl_scenes(void)
{
    /* Read Scenes */
    nvsto_read_ps
    (
        appl_nvsto_handle,
        &appl_state_info_vault[0],
        sizeof(appl_state_info_vault)
    );
}

/* Load OnPower States */
static void ms_load_appl_on_power(void)
{
    /* Read OnPowerUp state */
    nvsto_read_ps
    (
        appl_nvsto_handle,
        &appl_generic_onpower[0].onpowerup,
        sizeof(appl_generic_onpower[0].onpowerup)
    );
    CONSOLE_OUT
    ("[APPL_PS]: ms_load_appl_on_power. OnPowerUp 0x%02X\n",
     appl_generic_onpower[0].onpowerup);

    /* Check the status of GenericOnPowerUP */
    /* TODO: Take care of multiple elements */
    /**
        Generic OnPowerUp states:
        0x00: Off.
             After being powered up, the element is in an off state.
        0x01: Default.
             After being powered up, the element is in an On state and uses default state values.
        0x02: Restore.
             If a transition was in progress when powered down, the element restores the target state
             when powered up. Otherwise the element restores the state it was in when powered down.
    */
    switch(appl_generic_onpower[0].onpowerup)
    {
    case 0x00:
        /* Do Nothing */
        break;

    case 0x01:
        /* Default State */
        /* Generic OnOff as ON */
        appl_set_generic_onoff(0, 0x01, MS_FALSE);
        break;

    case 0x02:
        /* Restore */
        nvsto_read_ps
        (
            appl_nvsto_handle,
            &appl_state_info_vault[16],
            sizeof(appl_state_info_vault[16])
        );
        appl_scene_recall_saved_state(16, NULL);
        break;
    }
}

static void appl_ps_load(/* IN */ UINT32 records)
{
    UINT32 index;
    INT16 ret;
    CONSOLE_OUT
    ("[APPL_PS]: PS Load. Records 0x%08X\n", records);
    /* Open the storage */
    ret = nvsto_open_psread(appl_nvsto_handle);

    if (0 > ret)
    {
        CONSOLE_OUT
        ("[APPL_PS]: PS Open Failed\n");
        return;
    }

    /** Load information in listed order */
    for (index = 0; index < MS_PS_APPL_MAX_RECORDS; index++)
    {
        if (0 != (records & (1 << index)))
        {
            /* Seek to the corresponding location in PS */
            nvsto_seek_ps(appl_nvsto_handle, ms_ps_appl_records[index].start_offset);
            /* Only using the load/store function pointers */
            ms_ps_appl_records[index].load();
        }
    }

    /* Close the storage */
    nvsto_close_ps(appl_nvsto_handle);
    return;
}
#endif /* MS_STORAGE */

