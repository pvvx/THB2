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

static MS_STATE_GENERIC_LOCATION_STRUCT appl_generic_location[MS_MAX_NUM_STATES];


/* --------------------------------------------- Function */
void appl_model_states_initialization(void)
{
    EM_mem_set (appl_generic_location, 0, sizeof(appl_generic_location));
}

void appl_model_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
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
}


void appl_model_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
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
}


