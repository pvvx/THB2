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

} APPL_STATE_INFO_VAULT;


static MS_STATE_GENERIC_ONPOWERUP_STRUCT appl_generic_onpower[MS_MAX_NUM_STATES];

static MS_STATE_GENERIC_ONOFF_STRUCT appl_generic_onoff[MS_MAX_NUM_STATES];
static APPL_STATE_INFO_VAULT   appl_state_info_vault[16];


/* --------------------------------------------- Function */
void appl_model_states_initialization(void)
{
    EM_mem_set(appl_generic_onpower, 0, sizeof(appl_generic_onpower));
    EM_mem_set(appl_generic_onoff, 0, sizeof(appl_generic_onoff));
    EM_mem_set(appl_state_info_vault, 0, sizeof(appl_state_info_vault));
    /* For Test MMDL/SR/GPOO/BV-03-C */
    appl_generic_onoff[0].onoff = 0x01;
}

void appl_model_power_cycle(void)
{
    appl_generic_onoff[0].transition_time = 0x00;
    appl_generic_onoff[0].target_onoff = 0x00;

    /* */
    if (0x01 == appl_generic_onpower[0].onpowerup)
    {
        appl_generic_onoff[0].onoff = 0x01;
    }
    else if (0x00 == appl_generic_onpower[0].onpowerup)
    {
        appl_generic_onoff[0].onoff = 0x00;
    }
    /* else 0x02. Keep OnOff as before power down */
    /* TODO: Hack */
    else
    {
        appl_generic_onoff[0].onoff = 0x00;
    }
}


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
    appl_generic_onoff[0].onoff = appl_generic_onoff[0].target_onoff;
    appl_generic_onoff[0].target_onoff = 0;
}

void* appl_scene_save_current_state(/* IN */ UINT32 scene_index)
{
    /* Free previous context - if any */
    /* Empty right now - dummy returning the same scene index itself */
    /* Store current state */
    appl_state_info_vault[scene_index].onoff = appl_generic_onoff[0].onoff;
    return (void*)scene_index;
}

void* appl_scene_delete_saved_state(/* IN */ UINT32 scene_index, /* IN */ void* context)
{
    /* Free context - if any */
    /* Empty right now - dummy returning NULL */
    /* Invalid value */
    appl_state_info_vault[scene_index].onoff = 0x02;
    return (void*)NULL;
}

void* appl_scene_recall_saved_state(/* IN */ UINT32 scene_index, /* IN */ void* context)
{
    /* Recall context - if any */
    /* Empty right now - dummy returning NULL */
    appl_generic_onoff[0].onoff = appl_state_info_vault[scene_index].onoff;
    return (void*)NULL;
}

API_RESULT appl_model_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    API_RESULT retval;
    retval = API_SUCCESS;

    switch(state_t)
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
            appl_generic_onoff[0].onoff = param_p->onoff;
        }

        *param_p = appl_generic_onoff[0];
        CONSOLE_OUT("[state] current: 0x%02X\n", appl_generic_onoff[0].onoff);
        CONSOLE_OUT("[state] target: 0x%02X\n", appl_generic_onoff[0].target_onoff);
        CONSOLE_OUT("[state] remaining_time: 0x%02X\n", appl_generic_onoff[0].transition_time);
        /* Ignoring Instance and direction right now */
    }
    break;

    default:
        break;
    }

    return retval;
}

