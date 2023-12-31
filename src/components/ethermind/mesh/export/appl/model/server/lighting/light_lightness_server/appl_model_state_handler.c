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

/* --------------------------------------------- Data Types/ Structures */
#define MS_MAX_NUM_STATES    2

typedef struct _APPL_STATE_INFO_VAULT
{
    /* OnOff state */
    UINT8    onoff;

    /* Generic Level state */
    UINT16     level;

    /* Light Lightness Actual */
    UINT16     lightness_actual;

    /* Light Lightness Linear */
    UINT16     lightness_linear;

} APPL_STATE_INFO_VAULT;

typedef struct _APPL_GENENRIC_LEVEL_INFO
{
    MS_STATE_GENERIC_LEVEL_STRUCT generic_level;

    /* Operation Type : 0xFF as initial value (invalid) */
    UINT8    operation_type;

    UINT16   transition_time_handle;

} APPL_GENERIC_LEVEL_INFO;

static MS_STATE_LIGHT_LIGHTNESS_STRUCT appl_light_lightness[MS_MAX_NUM_STATES];
static MS_STATE_LIGHT_LIGHTNESS_STRUCT appl_light_lightness_setup[MS_MAX_NUM_STATES];

static MS_STATE_GENERIC_ONPOWERUP_STRUCT appl_generic_onpower[MS_MAX_NUM_STATES];

static MS_STATE_GENERIC_ONOFF_STRUCT appl_generic_onoff[MS_MAX_NUM_STATES];

/* static MS_STATE_GENERIC_LEVEL_STRUCT appl_generic_level[MS_MAX_NUM_STATES]; */
static APPL_GENERIC_LEVEL_INFO appl_generic_level_info[MS_MAX_NUM_STATES];

/* For simplicity keeping same number of state info, as the number of max scences */
static APPL_STATE_INFO_VAULT   appl_state_info_vault[16];
static UINT8 appl_in_transtion;

void appl_light_lightness_set_actual(UINT16 state_inst, UINT16 actual);

void appl_set_move_level(UINT16 state_inst, UINT16 move, UINT8 immediate);
void appl_set_level(UINT16 state_inst, UINT16 level);
void appl_set_delta_level(UINT16 state_inst, INT32 delta, UINT8 immediate);

API_RESULT appl_generic_level_model_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction);
void appl_set_generic_onoff(UINT16 state_inst, UINT8 onoff);
void appl_light_lightness_set_linear(UINT16 state_inst, UINT16 linear);

/* --------------------------------------------- Function */

void appl_model_states_initialization(void)
{
    UINT32 index;

    for (index = 0; index < MS_MAX_NUM_STATES; index++)
    {
        EM_mem_set(&appl_generic_level_info[index], 0, sizeof(APPL_GENERIC_LEVEL_INFO));
        appl_generic_level_info[index].operation_type = 0xFF;
        appl_generic_level_info[index].transition_time_handle = 0xFFFF;
    }

    EM_mem_set (appl_light_lightness, 0, sizeof(appl_light_lightness));
    EM_mem_set (appl_light_lightness_setup, 0, sizeof(appl_light_lightness_setup));
    appl_light_lightness[0].light_lightness_last.lightness_last = 0xFFFF;
    appl_light_lightness[1].light_lightness_last.lightness_last = 0xFFFF;
    EM_mem_set(appl_generic_onpower, 0, sizeof(appl_generic_onpower));
    EM_mem_set(appl_generic_onoff, 0, sizeof(appl_generic_onoff));
    EM_mem_set(appl_state_info_vault, 0, sizeof(appl_state_info_vault));
    appl_in_transtion = 0x00;
}

void appl_model_power_cycle(void)
{
    appl_generic_onoff[0].transition_time = 0x00;
    appl_generic_onoff[0].target_onoff = 0x00;

    /* */
    if (0x01 == appl_generic_onpower[0].onpowerup)
    {
        appl_set_generic_onoff(0, 0x01);
    }
    else if (0x00 == appl_generic_onpower[0].onpowerup)
    {
        appl_set_generic_onoff(0, 0x00);
    }
    /* else 0x02. Keep OnOff as before power down */
    /* TODO: Hack */
    else
    {
        appl_generic_onoff[0].onoff = 0x00;
    }

    /* TODO: Hack */
    if (0x00 != appl_light_lightness[0].light_lightness_actual.transition_time)
    {
        appl_light_lightness[0].light_lightness_actual.lightness_actual = appl_light_lightness[0].light_lightness_actual.lightness_target;
        appl_light_lightness[0].light_lightness_actual.transition_time = 0x00;
        appl_light_lightness[0].light_lightness_actual.lightness_target = 0x00;
    }
}


/* ---- State Transition Handlers */

static void appl_generic_onoff_transition_start_cb(void* blob)
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

static void appl_generic_onoff_transition_complete_cb(void* blob)
{
    /* State Transition Complete */
    appl_generic_onoff[0].transition_time = 0;
    /* appl_generic_onoff[0].onoff = appl_generic_onoff[0].target_onoff; */
    appl_set_generic_onoff(0, appl_generic_onoff[0].target_onoff);
    appl_generic_onoff[0].target_onoff = 0;
}

static void appl_generic_level_transition_start_cb(void* blob)
{
    UINT16 state_t;
    state_t = (UINT16)blob;
}

static void appl_generic_level_transition_complete_cb(void* blob)
{
    UINT16 state_t;
    state_t = (UINT16)blob;
    appl_generic_level_info[0].generic_level.transition_time = 0x00;
    appl_generic_level_info[0].transition_time_handle = 0xFFFF;

    switch (state_t)
    {
    case MS_STATE_GENERIC_LEVEL_T:
    {
        appl_set_level(0, appl_generic_level_info[0].generic_level.target_level);
    }
    break;

    case MS_STATE_DELTA_LEVEL_T:
    {
        appl_set_level(0, appl_generic_level_info[0].generic_level.target_level);
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

static void appl_light_lightness_transition_start_cb(void* blob)
{
}

static void appl_light_lightness_transition_complete_cb(void* blob)
{
    /* State Transition Complete */
    appl_light_lightness[0].light_lightness_actual.transition_time = 0;
    /* appl_generic_onoff[0].onoff = appl_generic_onoff[0].target_onoff; */
    appl_light_lightness_set_actual(0, appl_light_lightness[0].light_lightness_actual.lightness_target);
    appl_light_lightness[0].light_lightness_actual.lightness_target = 0;
}

static void appl_light_lightness_linear_transition_start_cb(void* blob)
{
}

static void appl_light_lightness_linear_transition_complete_cb(void* blob)
{
    /* State Transition Complete */
    appl_light_lightness[0].light_lightness_linear.transition_time = 0;
    /* appl_generic_onoff[0].onoff = appl_generic_onoff[0].target_onoff; */
    appl_light_lightness_set_linear(0, appl_light_lightness[0].light_lightness_linear.lightness_target);
    appl_light_lightness[0].light_lightness_linear.lightness_target = 0;
}

void* appl_scene_save_current_state(/* IN */ UINT32 scene_index)
{
    /* Free previous context - if any */
    /* Empty right now - dummy returning the same scene index itself */
    /* Store current state */
    appl_state_info_vault[scene_index].onoff = appl_generic_onoff[0].onoff;
    appl_state_info_vault[scene_index].level = appl_generic_level_info[0].generic_level.level;
    appl_state_info_vault[scene_index].lightness_actual = appl_light_lightness[0].light_lightness_actual.lightness_actual;
    appl_state_info_vault[scene_index].lightness_linear = appl_light_lightness[0].light_lightness_linear.lightness_linear;
    return (void*)scene_index;
}

void* appl_scene_delete_saved_state(/* IN */ UINT32 scene_index, /* IN */ void* context)
{
    /* Free context - if any */
    /* Empty right now - dummy returning NULL */
    /* Invalid value */
    appl_state_info_vault[scene_index].onoff = 0x02;
    appl_state_info_vault[scene_index].level = 0xFFFF;
    appl_state_info_vault[scene_index].lightness_actual = 0x0000;
    appl_state_info_vault[scene_index].lightness_linear = 0x0000;
    return (void*)NULL;
}

void* appl_scene_recall_saved_state(/* IN */ UINT32 scene_index, /* IN */ void* context)
{
    /* Recall context - if any */
    /* Empty right now - dummy returning NULL */
    /* appl_generic_onoff[0].onoff = appl_state_info_vault[scene_index].onoff; */
    appl_set_generic_onoff(0, appl_state_info_vault[scene_index].onoff);
    appl_generic_level_info[0].generic_level.level = appl_state_info_vault[scene_index].level;
    appl_light_lightness[0].light_lightness_actual.lightness_actual = appl_state_info_vault[scene_index].lightness_actual;
    appl_light_lightness[0].light_lightness_linear.lightness_linear = appl_state_info_vault[scene_index].lightness_linear;
    return (void*)NULL;
}

API_RESULT appl_model_light_lightness_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    MS_STATE_LIGHT_LIGHTNESS_STRUCT* param_p;
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
    }
    break;

    default:
        break;
    }

    return API_SUCCESS;
}

API_RESULT appl_model_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    retval = API_SUCCESS;

    switch (state_t)
    {
    case MS_STATE_GENERIC_ONPOWERUP_T:
    {
        MS_STATE_GENERIC_ONPOWERUP_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_ONPOWERUP_STRUCT*)param;
        /* Ignoting Instance and direction right now */
        *param_p = appl_generic_onpower[0];
    }
    break;

    case MS_STATE_GENERIC_ONOFF_T:
    {
        MS_STATE_GENERIC_ONOFF_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_ONOFF_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_generic_onoff[0];
    }
    break;

    case MS_STATE_GENERIC_LEVEL_T:
    {
        MS_STATE_GENERIC_LEVEL_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_LEVEL_STRUCT*)param;
        /* Ignoting Instance and direction right now */
        *param_p = appl_generic_level_info[0].generic_level;
        param_p->level = appl_generic_level_info[0].generic_level.level + appl_generic_level_info[0].generic_level.delta_level;

        /* TODO: hack */
        if (0xFE == appl_generic_level_info[0].generic_level.transition_time)
        {
            param_p->transition_time = 0x3F;
        }
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_DEFAULT_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_RANGE_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_LINEAR_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_LAST_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_ACTUAL_T:
        retval = appl_model_light_lightness_state_get(state_t, state_inst, param, direction);
        break;

    default:
        break;
    }

    return retval;
}

void appl_light_lightness_set_actual(UINT16 state_inst, UINT16 actual)
{
    UINT16 min, max;

    /* Generic OnOff binding */
    if (0x0000 == actual)
    {
        appl_generic_onoff[state_inst].onoff= 0x00;
    }
    else
    {
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

        appl_generic_onoff[state_inst].onoff = 0x01;
        appl_light_lightness[state_inst].light_lightness_last.lightness_last = actual;
    }

    appl_light_lightness[state_inst].light_lightness_actual.lightness_actual = actual;
    /* Light Lightness Linear = ((Actual)^2) / 65535 */
    appl_light_lightness[state_inst].light_lightness_linear.lightness_linear = ((actual * actual) + 65534) / 65535;
    appl_generic_level_info[state_inst].generic_level.level = actual - 32768;
}

void appl_light_lightness_set_range(UINT16 state_inst, UINT16 min, UINT16 max)
{
    UINT16 actual;
    appl_light_lightness[state_inst].light_lightness_range.lightness_range_min = min;
    appl_light_lightness[state_inst].light_lightness_range.lightness_range_max = max;
    /* Check if actual to be updated */
    actual = appl_light_lightness[state_inst].light_lightness_actual.lightness_actual;

    if (actual < min)
    {
        appl_light_lightness[state_inst].light_lightness_actual.lightness_actual = min;
    }
    else if (actual > max)
    {
        appl_light_lightness[state_inst].light_lightness_actual.lightness_actual = max;
    }
}
void appl_set_level(UINT16 state_inst, UINT16 level)
{
    appl_generic_level_info[state_inst].generic_level.level = level;
    appl_generic_level_info[state_inst].generic_level.target_level = 0;
    appl_generic_level_info[state_inst].generic_level.delta_level = 0;
    appl_light_lightness_set_actual(0, level + 32768);
}

void appl_set_delta_level(UINT16 state_inst, INT32 delta, UINT8 immediate)
{
    /* TODO: See if this to be stored */
    if (0x01 == immediate)
    {
        appl_set_level(state_inst, (appl_generic_level_info[state_inst].generic_level.level + delta));
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

void appl_set_move_level(UINT16 state_inst, UINT16 move, UINT8 immediate)
{
    /* TODO: See if this to be stored */
    appl_generic_level_info[state_inst].generic_level.move_level = move;
}

/* Todo: Remove the dependency */
#include "math.h"

void appl_light_lightness_set_linear(UINT16 state_inst, UINT16 linear)
{
    UINT16 actual;
    UINT32 mul_val;
    long double d;
    /* appl_light_lightness[state_inst].light_lightness_linear.lightness_linear = linear; */
    mul_val = linear * 65535;
    actual = (UINT16)sqrt(mul_val);
    /* Light Lightness actual = sqrt(Linear * 65535) */
    appl_light_lightness_set_actual(state_inst, actual);
}


void appl_set_generic_onoff(UINT16 state_inst, UINT8 onoff)
{
    UINT16 actual;
    /* TODO: See if this to be stored */
    appl_generic_onoff[state_inst].onoff = onoff;

    /* Binding */
    if (onoff == 0x00)
    {
        appl_light_lightness_set_actual(state_inst, 0x00);
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

        appl_light_lightness_set_actual(state_inst, actual);
    }
}

API_RESULT appl_model_light_lightness_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
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
            #if 0
            appl_light_lightness[0].light_lightness_range = param_p->light_lightness_range;
            param_p->light_lightness_range.lightness_range_min = 0x0000;
            param_p->light_lightness_range.lightness_range_max = 0x0000;
            #endif
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
        if (0 != param_p->light_lightness_actual.transition_time)
        {
            MS_ACCESS_STATE_TRANSITION_TYPE   transition;
            UINT16                            transition_time_handle;
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
                &transition_time_handle
            );
            /* Return value to indicate not sending status right now */
            retval = API_FAILURE;
        }
        else
        {
            /* Instantaneous Change */
            /* appl_generic_onoff[0].onoff = param_p->onoff; */
            appl_light_lightness_set_actual(0, param_p->light_lightness_actual.lightness_actual);
        }

        *param_p = appl_light_lightness[0];
        CONSOLE_OUT("[state] current: 0x%02X\n", param_p->light_lightness_actual.lightness_actual);
        CONSOLE_OUT("[state] target: 0x%02X\n", param_p->light_lightness_actual.lightness_target);
        CONSOLE_OUT("[state] remaining_time: 0x%02X\n", param_p->light_lightness_actual.transition_time);
        /* Ignoring Instance and direction right now */
        /* param_p->light_lightness_actual.lightness_actual = appl_light_lightness[0].light_lightness_actual.lightness_actual; */
    }
    break;

    default:
        break;
    }

    return retval;
}


API_RESULT appl_model_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    retval = API_SUCCESS;

    switch (state_t)
    {
    case MS_STATE_GENERIC_ONPOWERUP_T:
    {
        MS_STATE_GENERIC_ONPOWERUP_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_ONPOWERUP_STRUCT*)param;
        /* Ignoting Instance and direction right now */
        appl_generic_onpower[0] = *param_p;
    }
    break;

    case MS_STATE_GENERIC_ONOFF_T:
    {
        MS_STATE_GENERIC_ONOFF_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_ONOFF_STRUCT*)param;

        /* Check if state transition is specified */
        if (0 != param_p->transition_time)
        {
            MS_ACCESS_STATE_TRANSITION_TYPE   transition;
            UINT16                            transition_time_handle;
            appl_generic_onoff[0].target_onoff = param_p->onoff;
            appl_generic_onoff[0].transition_time = param_p->transition_time;
            transition.delay = param_p->delay;
            transition.transition_time = param_p->transition_time;
            transition.blob = NULL;
            transition.transition_start_cb = appl_generic_onoff_transition_start_cb;
            transition.transition_complete_cb = appl_generic_onoff_transition_complete_cb;
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
            appl_set_generic_onoff(0, param_p->onoff);
        }

        *param_p = appl_generic_onoff[0];
        CONSOLE_OUT("[state] current: 0x%02X\n", appl_generic_onoff[0].onoff);
        CONSOLE_OUT("[state] target: 0x%02X\n", appl_generic_onoff[0].target_onoff);
        CONSOLE_OUT("[state] remaining_time: 0x%02X\n", appl_generic_onoff[0].transition_time);
        /* Ignoring Instance and direction right now */
    }
    break;

    case MS_STATE_GENERIC_LEVEL_T: /* Fall Through */
    case MS_STATE_DELTA_LEVEL_T:   /* Fall Through */
    case MS_STATE_MOVE_LEVEL_T:
    {
        retval = appl_generic_level_model_state_set(state_t, state_inst, param, direction);
    }
    break;

    case MS_STATE_LIGHT_LIGHTNESS_DEFAULT_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_RANGE_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_LINEAR_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_LAST_T: /* Fall Through */
    case MS_STATE_LIGHT_LIGHTNESS_ACTUAL_T:
        retval = appl_model_light_lightness_state_set(state_t, state_inst, param, direction);
        break;

    default:
        break;
    }

    return retval;
}

API_RESULT appl_generic_level_model_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
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
        /* Ignoting Instance and direction right now */
        /* TODO: Not handling transaction state */
        {
            MS_common_stop_transition_timer(appl_generic_level_info[0].transition_time_handle);
            appl_generic_level_info[0].transition_time_handle = 0xFFFF;
            appl_generic_level_info[0].generic_level.transition_time = 0x00;

            if (0 == param_p->transition_time)
            {
                appl_set_level(0, param_p->level);
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

        /* Ignoting Instance and direction right now */
        if (0 == param_p->transition_time)
        {
            if (0x02 == transaction_state)
            {
                appl_set_delta_level(0, (INT32)appl_generic_level_info[0].generic_level.delta_level, 0x01);
            }

            /* Only update delta */
            appl_set_delta_level(0, (INT32)param_p->delta_level, 0x00);
        }
        else
        {
            appl_set_delta_level(0, (INT32)param_p->delta_level, 0x00);
        }
    }
    break;

    case MS_STATE_MOVE_LEVEL_T:
    {
        /* Ignoting Instance and direction right now */
        if (0 == param_p->transition_time)
        {
            appl_set_move_level(0, param_p->move_level, 0x01);
        }
        else
        {
            appl_set_move_level(0, param_p->move_level, 0x00);
            /* appl_generic_level_info[0].generic_level.target_level = 0x00; */
            appl_generic_level_info[0].generic_level.target_level = param_p->move_level * (param_p->transition_time & 0x3F);
            /* TODO: Hardcoding */
            appl_generic_level_info[0].generic_level.target_level = 0x7FFF;
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
        transition.blob = (void*)state_t;
        transition.transition_start_cb = appl_generic_level_transition_start_cb;
        transition.transition_complete_cb = appl_generic_level_transition_complete_cb;
        MS_common_start_transition_timer
        (
            &transition,
            &appl_generic_level_info[0].transition_time_handle
        );
    }

    *param_p = appl_generic_level_info[0].generic_level;
    param_p->level = appl_generic_level_info[0].generic_level.level + appl_generic_level_info[0].generic_level.delta_level;

    /* TODO: hack */
    if (0xFE == appl_generic_level_info[0].generic_level.transition_time)
    {
        param_p->transition_time = 0x3F;
    }

    return API_SUCCESS;
}

