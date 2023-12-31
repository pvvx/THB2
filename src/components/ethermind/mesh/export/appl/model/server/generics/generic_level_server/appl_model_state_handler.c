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
    /* Generic Level state */
    UINT16     level;

} APPL_STATE_INFO_VAULT;

typedef struct _APPL_GENENRIC_LEVEL_INFO
{
    MS_STATE_GENERIC_LEVEL_STRUCT generic_level;

    /* Operation Type : 0xFF as initial value (invalid) */
    UINT8    operation_type;

    UINT16   transition_time_handle;

} APPL_GENERIC_LEVEL_INFO;

/* static MS_STATE_GENERIC_LEVEL_STRUCT appl_generic_level[MS_MAX_NUM_STATES]; */
static APPL_GENERIC_LEVEL_INFO appl_generic_level_info[MS_MAX_NUM_STATES];

/* For simplicity keeping same number of state info, as the number of max scences */
static APPL_STATE_INFO_VAULT   appl_state_info_vault[16];

void appl_set_move_level(UINT16 state_inst, UINT16 move, UINT8 immediate);
void appl_set_level(UINT16 state_inst, UINT16 level);
void appl_set_delta_level(UINT16 state_inst, INT32 delta, UINT8 immediate);

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

    EM_mem_set(appl_state_info_vault, 0, sizeof(appl_state_info_vault));
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
    appl_state_info_vault[scene_index].level = appl_generic_level_info[0].generic_level.level;
    return (void*)scene_index;
}

void* appl_scene_delete_saved_state(/* IN */ UINT32 scene_index, /* IN */ void* context)
{
    /* Free context - if any */
    /* Empty right now - dummy returning NULL */
    /* Invalid value */
    appl_state_info_vault[scene_index].level = 0xFFFF;
    return (void*)NULL;
}

void* appl_scene_recall_saved_state(/* IN */ UINT32 scene_index, /* IN */ void* context)
{
    /* Recall context - if any */
    /* Empty right now - dummy returning NULL */
    appl_generic_level_info[0].generic_level.level = appl_state_info_vault[scene_index].level;
    return (void*)NULL;
}

API_RESULT appl_model_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    retval = API_SUCCESS;

    switch (state_t)
    {
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

void appl_set_level(UINT16 state_inst, UINT16 level)
{
    appl_generic_level_info[state_inst].generic_level.level = level;
    appl_generic_level_info[0].generic_level.target_level = 0;
    appl_generic_level_info[0].generic_level.delta_level = 0;
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

API_RESULT appl_model_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
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
