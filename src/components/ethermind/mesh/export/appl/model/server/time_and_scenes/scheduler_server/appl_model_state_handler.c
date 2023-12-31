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
#define MS_MAX_NUM_STATES    2

static MS_STATE_SCHEDULER_SCHEDULES_STRUCT appl_scheduler_current[MS_MAX_NUM_STATES];
static MS_STATE_SCHEDULER_ENTRY_STRUCT     appl_scheduler[MS_MAX_NUM_STATES];


/* --------------------------------------------- Function */
void appl_model_states_initialization(void)
{
    EM_mem_set(appl_scheduler_current, 0, sizeof(appl_scheduler_current));
    EM_mem_set (appl_scheduler, 0, sizeof(appl_scheduler));
}


void appl_model_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    switch(state_t)
    {
    case MS_STATE_SCHEDULER_ENTRY_INDEX_T:
    {
        MS_STATE_SCHEDULER_ENTRY_STRUCT* param_p;
        param_p = (MS_STATE_SCHEDULER_ENTRY_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_scheduler[0];
    }
    break;

    case MS_STATE_SCHEDULER_SCHEDULES_T:
    {
        MS_STATE_SCHEDULER_SCHEDULES_STRUCT* param_p;
        param_p = (MS_STATE_SCHEDULER_SCHEDULES_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_scheduler_current[0];
    }
    break;

    default:
        break;
    }
}


void appl_model_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    switch(state_t)
    {
    case MS_STATE_SCHEDULER_ENTRY_T:
    {
        MS_STATE_SCHEDULER_ENTRY_STRUCT* param_p;
        param_p = (MS_STATE_SCHEDULER_ENTRY_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_scheduler[0] = *param_p;
    }
    break;

    default:
        break;
    }
}


