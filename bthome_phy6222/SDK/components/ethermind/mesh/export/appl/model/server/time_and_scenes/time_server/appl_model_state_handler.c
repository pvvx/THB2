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

static MS_STATE_TIME_STRUCT appl_time[MS_MAX_NUM_STATES];
static MS_STATE_TIME_TAI_UTC_DELTA_STRUCT appl_time_delta[MS_MAX_NUM_STATES];
static MS_STATE_TIME_ZONE_STRUCT appl_time_zone[MS_MAX_NUM_STATES];
static MS_STATE_TIME_ROLE_STRUCT appl_time_role[MS_MAX_NUM_STATES];

/* --------------------------------------------- Function */
void appl_model_states_initialization(void)
{
    EM_mem_set (appl_time, 0, sizeof(appl_time));
    EM_mem_set(appl_time_delta, 0, sizeof(appl_time_delta));
    EM_mem_set(appl_time_zone, 0, sizeof(appl_time_zone));
    EM_mem_set(appl_time_role, 0, sizeof(appl_time_role));
}


void appl_model_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    switch(state_t)
    {
    case MS_STATE_TIME_T:
    {
        MS_STATE_TIME_STRUCT* param_p;
        param_p = (MS_STATE_TIME_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_time[0];
    }
    break;

    case MS_STATE_TIME_ZONE_T:
    {
        MS_STATE_TIME_ZONE_STRUCT* param_p;
        param_p = (MS_STATE_TIME_ZONE_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_time_zone[0];
    }
    break;

    case MS_STATE_TIME_TAI_UTC_DELTA_T:
    {
        MS_STATE_TIME_TAI_UTC_DELTA_STRUCT* param_p;
        param_p = (MS_STATE_TIME_TAI_UTC_DELTA_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_time_delta[0];
    }
    break;

    case MS_STATE_TIME_ROLE_T:
    {
        MS_STATE_TIME_ROLE_STRUCT* param_p;
        param_p = (MS_STATE_TIME_ROLE_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        *param_p = appl_time_role[0];
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
    case MS_STATE_TIME_T:
    {
        MS_STATE_TIME_STRUCT* param_p;
        param_p = (MS_STATE_TIME_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_time[0] = *param_p;
    }
    break;

    case MS_STATE_TIME_ZONE_T:
    {
        MS_STATE_TIME_ZONE_STRUCT* param_p;
        param_p = (MS_STATE_TIME_ZONE_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        param_p->time_zone_offset_current = appl_time_zone[0].time_zone_offset_current;
        appl_time_zone[0].time_zone_offset_current = param_p->time_zone_offset_new;
        appl_time_zone[0].time_zone_offset_new = appl_time_zone[0].time_zone_offset_current;
        EM_mem_copy(&appl_time_zone[0].tai_of_zone_change[0], &param_p->tai_of_zone_change[0], 5);
    }
    break;

    case MS_STATE_TIME_TAI_UTC_DELTA_T:
    {
        MS_STATE_TIME_TAI_UTC_DELTA_STRUCT* param_p;
        param_p = (MS_STATE_TIME_TAI_UTC_DELTA_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        param_p->tai_utc_delta_current = appl_time_delta[0].tai_utc_delta_current;
        appl_time_delta[0].tai_utc_delta_current = param_p->tai_utc_delta_new;
        appl_time_delta[0].tai_utc_delta_new = appl_time_delta[0].tai_utc_delta_current;
        EM_mem_copy(&appl_time_delta[0].tai_of_delta_change[0], &param_p->tai_of_delta_change[0], 5);
    }
    break;

    case MS_STATE_TIME_ROLE_T:
    {
        MS_STATE_TIME_ROLE_STRUCT* param_p;
        param_p = (MS_STATE_TIME_ROLE_STRUCT*)param;
        /* Ignoring Instance and direction right now */
        appl_time_role[0] = *param_p;
    }
    break;

    default:
        break;
    }
}


