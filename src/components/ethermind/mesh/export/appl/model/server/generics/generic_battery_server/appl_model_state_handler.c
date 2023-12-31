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

static MS_STATE_GENERIC_BATTERY_STRUCT appl_generic_battery[MS_MAX_NUM_STATES];




/* --------------------------------------------- Function */
void appl_model_states_initialization(void)
{
    EM_mem_set(appl_generic_battery, 0, sizeof(appl_generic_battery));
}

void appl_model_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
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
}

void appl_model_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
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
}
