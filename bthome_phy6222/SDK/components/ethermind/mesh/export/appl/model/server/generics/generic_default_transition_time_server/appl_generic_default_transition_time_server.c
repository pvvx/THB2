/**
    \file appl_generic_default_transition_time_server.c
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_generic_default_transition_time_server.h"
#include "MS_model_states.h"

/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
static const char main_generic_default_transition_time_server_options[] = "\n\
======== Generic_Default_Transition_Time Server Menu ========\n\
     0. Exit. \n\
     1. Refresh. \n\
 \n\
     2. Get Default Transition Time. \n\
 \n\
Your Option ? \0";


/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_generic_default_transition_time_server_model_handle;


/* --------------------------------------------- Function */
/* generic_default_transition_time server application entry point */
void main_generic_default_transition_time_server_operations(/* IN */ UINT8 have_menu)
{
    int choice;
    MS_ACCESS_ELEMENT_HANDLE element_handle;
    static UCHAR model_initialized = 0x00;

    /**
        Register with Access Layer.
    */
    if (0x00 == model_initialized)
    {
        API_RESULT retval;
        /* Use Default Element Handle. Index 0 */
        element_handle = MS_ACCESS_DEFAULT_ELEMENT_HANDLE;
        retval = MS_generic_default_transition_time_server_init
                 (
                     element_handle,
                     &appl_generic_default_transition_time_server_model_handle,
                     NULL
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Generic Default Transition Time Server Initialized. Model Handle: 0x%04X\n",
                appl_generic_default_transition_time_server_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Generic Default Transition Time Server Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        model_initialized = 0x01;
    }

    if (MS_TRUE != have_menu)
    {
        CONSOLE_OUT("Not to use menu options\n");
        return;
    }

    MS_LOOP_FOREVER()
    {
        CONSOLE_OUT
        ("%s", main_generic_default_transition_time_server_options);
        CONSOLE_IN
        ("%d", &choice);

        if (choice < 0)
        {
            CONSOLE_OUT
            ("*** Invalid Choice. Try Again.\n");
            continue;
        }

        switch (choice)
        {
        case 0:
            return;

        case 1:
            break;

        case 2:
        {
            MS_STATE_GENERIC_DEFAULT_TRANSITION_TIME_STRUCT   default_time;
            MS_generic_default_transition_time_server_get_time
            (
                &default_time
            );
            CONSOLE_OUT("Default Transition Steps Resolution: 0x%02X\n", default_time.default_transition_step_resolution);
            CONSOLE_OUT("Default Transition Number of Steps: 0x%02X\n", default_time.default_transition_number_of_steps);
        }
        break;
        }
    }
}

