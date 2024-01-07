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

    /* Generic Power Level */
    UINT16     power_level;

} APPL_STATE_INFO_VAULT;

typedef struct _APPL_GENENRIC_LEVEL_INFO
{
    MS_STATE_GENERIC_LEVEL_STRUCT generic_level;

    /* Operation Type : 0xFF as initial value (invalid) */
    UINT8    operation_type;

    UINT16   transition_time_handle;

} APPL_GENERIC_LEVEL_INFO;

static MS_STATE_GENERIC_POWER_LEVEL_STRUCT appl_generic_power_level[MS_MAX_NUM_STATES];

static MS_STATE_GENERIC_ONPOWERUP_STRUCT appl_generic_onpower[MS_MAX_NUM_STATES];

static MS_STATE_GENERIC_ONOFF_STRUCT appl_generic_onoff[MS_MAX_NUM_STATES];

/* static MS_STATE_GENERIC_LEVEL_STRUCT appl_generic_level[MS_MAX_NUM_STATES]; */
static APPL_GENERIC_LEVEL_INFO appl_generic_level_info[MS_MAX_NUM_STATES];

/* For simplicity keeping same number of state info, as the number of max scences */
static APPL_STATE_INFO_VAULT   appl_state_info_vault[16];

void appl_set_move_level(UINT16 state_inst, UINT16 move, UINT8 immediate);
void appl_set_level(UINT16 state_inst, UINT16 level);
void appl_set_delta_level(UINT16 state_inst, INT32 delta, UINT8 immediate);

API_RESULT appl_generic_level_model_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction);
void appl_set_generic_onoff(UINT16 state_inst, UINT8 onoff);
void appl_power_level_set_actual(UINT16 state_inst, UINT16 actual, UINT8 flag);

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

    EM_mem_set(appl_generic_power_level, 0, sizeof(appl_generic_power_level));
    EM_mem_set(appl_generic_onpower, 0, sizeof(appl_generic_onpower));
    EM_mem_set(appl_generic_onoff, 0, sizeof(appl_generic_onoff));
    EM_mem_set(appl_state_info_vault, 0, sizeof(appl_state_info_vault));
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
    if (0x00 != appl_generic_power_level[0].generic_power_actual.transition_time)
    {
        appl_generic_power_level[0].generic_power_actual.power_actual = appl_generic_power_level[0].generic_power_actual.power_target;
        appl_generic_power_level[0].generic_power_actual.transition_time = 0x00;
        appl_generic_power_level[0].generic_power_actual.power_target = 0x00;
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
    /* appl_generic_onoff[0].onoff = appl_generic_onoff[0].target_onoff; */
    appl_power_level_set_actual(0, appl_generic_power_level[0].generic_power_actual.power_target, 1);
    appl_generic_power_level[0].generic_power_actual.power_target = 0;
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

void* appl_scene_save_current_state(/* IN */ UINT32 scene_index)
{
    /* Free previous context - if any */
    /* Empty right now - dummy returning the same scene index itself */
    /* Store current state */
    appl_state_info_vault[scene_index].onoff = appl_generic_onoff[0].onoff;
    appl_state_info_vault[scene_index].level = appl_generic_level_info[0].generic_level.level;
    appl_state_info_vault[scene_index].power_level = appl_generic_power_level[0].generic_power_actual.power_actual;
    return (void*)scene_index;
}

void* appl_scene_delete_saved_state(/* IN */ UINT32 scene_index, /* IN */ void* context)
{
    /* Free context - if any */
    /* Empty right now - dummy returning NULL */
    /* Invalid value */
    appl_state_info_vault[scene_index].onoff = 0x02;
    appl_state_info_vault[scene_index].level = 0xFFFF;
    appl_state_info_vault[scene_index].power_level = 0xFFFF;
    return (void*)NULL;
}

void* appl_scene_recall_saved_state(/* IN */ UINT32 scene_index, /* IN */ void* context)
{
    /* Recall context - if any */
    /* Empty right now - dummy returning NULL */
    /* appl_generic_onoff[0].onoff = appl_state_info_vault[scene_index].onoff; */
    appl_set_generic_onoff(0, appl_state_info_vault[scene_index].onoff);
    appl_generic_level_info[0].generic_level.level = appl_state_info_vault[scene_index].level;
    appl_generic_power_level[0].generic_power_actual.power_actual = appl_state_info_vault[0].power_level;
    return (void*)NULL;
}

API_RESULT appl_model_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    retval = API_SUCCESS;

    switch (state_t)
    {
    case MS_STATE_GENERIC_POWER_ACTUAL_T:
    {
        MS_STATE_GENERIC_POWER_ACTUAL_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_POWER_ACTUAL_STRUCT*)param;
        /* Ignoting Instance and direction right now */
        *param_p = appl_generic_power_level[0].generic_power_actual;
    }
    break;

    case MS_STATE_GENERIC_POWER_LAST_T:
    {
        MS_STATE_GENERIC_POWER_LAST_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_POWER_LAST_STRUCT*)param;
        /* Ignoting Instance and direction right now */
        *param_p = appl_generic_power_level[0].generic_power_last;
    }
    break;

    case MS_STATE_GENERIC_POWER_DEFAULT_T:
    {
        MS_STATE_GENERIC_POWER_DEFAULT_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_POWER_DEFAULT_STRUCT*)param;
        /* Ignoting Instance and direction right now */
        *param_p = appl_generic_power_level[0].generic_power_default;
    }
    break;

    case MS_STATE_GENERIC_POWER_RANGE_T:
    {
        MS_STATE_GENERIC_POWER_RANGE_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_POWER_RANGE_STRUCT*)param;
        /* Ignoting Instance and direction right now */
        *param_p = appl_generic_power_level[0].generic_power_range;
    }
    break;

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

    default:
        break;
    }

    return retval;
}

void appl_power_level_set_actual(UINT16 state_inst, UINT16 actual, UINT8 flag)
{
    UINT16 min, max;

    /* Generic OnOff binding */
    if (0x0000 == actual)
    {
        appl_generic_onoff[state_inst].onoff = 0x00;
    }
    else
    {
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

        appl_generic_onoff[state_inst].onoff = 0x01;
        appl_generic_power_level[state_inst].generic_power_last.power_last = actual;
    }

    appl_generic_power_level[state_inst].generic_power_actual.power_actual = actual;

    if (0x00 != flag)
    {
        appl_generic_level_info[state_inst].generic_level.level = actual - 32768;
    }
}

void appl_generic_power_level_set_range(UINT16 state_inst, UINT16 min, UINT16 max)
{
    UINT16 actual;
    appl_generic_power_level[state_inst].generic_power_range.power_range_min = min;
    appl_generic_power_level[state_inst].generic_power_range.power_range_max = max;
    /* Check if actual to be updated */
    actual = appl_generic_power_level[state_inst].generic_power_actual.power_actual;
    #if 1
    appl_power_level_set_actual(state_inst, actual, 1);
    #else

    if (actual < min)
    {
        appl_generic_power_level[state_inst].generic_power_actual.power_actual = min;
    }
    else if (actual > max)
    {
        appl_generic_power_level[state_inst].generic_power_actual.power_actual = max;
    }

    #endif /* 0 */
}
void appl_set_level(UINT16 state_inst, UINT16 level)
{
    appl_generic_level_info[state_inst].generic_level.level = level;
    appl_generic_level_info[state_inst].generic_level.target_level = 0;
    appl_generic_level_info[state_inst].generic_level.delta_level = 0;
    appl_power_level_set_actual(0, level + 32768, 0);
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

void appl_set_generic_onoff(UINT16 state_inst, UINT8 onoff)
{
    /* TODO: See if this to be stored */
    appl_generic_onoff[state_inst].onoff = onoff;

    /* Binding */
    if (onoff == 0x00)
    {
        appl_power_level_set_actual(state_inst, 0x00, 0x01);
    }
    else
    {
        if (0x0000 == appl_generic_power_level[state_inst].generic_power_default.power_default)
        {
            appl_power_level_set_actual(state_inst, appl_generic_power_level[state_inst].generic_power_last.power_last, 0x01);
        }
        else
        {
            appl_power_level_set_actual(state_inst, appl_generic_power_level[state_inst].generic_power_default.power_default, 0x01);
        }
    }
}

API_RESULT appl_model_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
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
            UINT16                            transition_time_handle;
            appl_generic_power_level[0].generic_power_actual.power_target = param_p->power_actual;
            appl_generic_power_level[0].generic_power_actual.transition_time = param_p->transition_time;
            transition.delay = param_p->delay;
            transition.transition_time = param_p->transition_time;
            transition.blob = NULL;
            transition.transition_start_cb = appl_generic_power_level_transition_start_cb;
            transition.transition_complete_cb = appl_generic_power_level_transition_complete_cb;
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
            appl_power_level_set_actual(0, param_p->power_actual, 1);
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
        /* Ignoting Instance and direction right now */
        appl_generic_power_level[0].generic_power_default.power_default = param_p->power_default;
    }
    break;

    case MS_STATE_GENERIC_POWER_RANGE_T:
    {
        MS_STATE_GENERIC_POWER_RANGE_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_POWER_RANGE_STRUCT*)param;
        /* Ignoting Instance and direction right now */
        appl_generic_power_level[0].generic_power_range.power_range_min = param_p->power_range_min;
        appl_generic_power_level[0].generic_power_range.power_range_max = param_p->power_range_max;
        appl_generic_power_level_set_range(0, param_p->power_range_min, param_p->power_range_max);
        param_p->status = 0x00;
    }
    break;

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

