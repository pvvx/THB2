/**
    \file appl_time_server.c
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_time_server.h"
#include "appl_model_state_handler.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
static const char main_time_server_options[] = "\n\
======== Time Server Menu ========\n\
     0. Exit. \n\
     1. Refresh. \n\
 \n\
 \n\
Your Option ? \0";


/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_time_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_time_setup_server_model_handle;


/* --------------------------------------------- Function */
/* time server application entry point */
void main_time_server_operations(/* IN */ UINT8 have_menu)
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
        retval = MS_time_server_init
                 (
                     element_handle,
                     &appl_time_server_model_handle,
                     &appl_time_setup_server_model_handle,
                     appl_time_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Time Server Initialized. Model Handle: 0x%04X\n",
                appl_time_server_model_handle);
            CONSOLE_OUT(
                "Time Setup Server Initialized. Model Handle: 0x%04X\n",
                appl_time_setup_server_model_handle);
            appl_model_states_initialization();
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Time Server Initialization Failed. Result: 0x%04X\n",
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
        ("%s", main_time_server_options);
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
        }
    }
}

#if 0
/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Time server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_time_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_STATE_TIME_STRUCT appl_time_param;
    MS_STATE_TIME_TAI_UTC_DELTA_STRUCT appl_time_delta_param;
    MS_STATE_TIME_ZONE_STRUCT appl_time_zone_param;
    MS_STATE_TIME_ROLE_STRUCT appl_time_role_param;
    MS_ACCESS_MODEL_STATE_PARAMS                    current_state_params;
    void* param;
    API_RESULT retval;
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[TIME] GET Request.\n");

        switch (state_params->state_type)
        {
        case MS_STATE_TIME_T:
        {
            param = &appl_time_param;
        }
        break;

        case MS_STATE_TIME_ZONE_T:
        {
            param = &appl_time_zone_param;
        }
        break;

        case MS_STATE_TIME_TAI_UTC_DELTA_T:
        {
            param = &appl_time_delta_param;
        }
        break;

        case MS_STATE_TIME_ROLE_T:
        {
            param = &appl_time_role_param;
        }
        break;

        default:
            break;
        }

        appl_model_state_get(state_params->state_type, 0, param, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = param;
    }
    else if (MS_ACCESS_MODEL_REQ_MSG_T_SET == req_type->type)
    {
        CONSOLE_OUT(
            "[TIME] SET Request.\n");
        appl_model_state_set(state_params->state_type, 0, state_params->state, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = state_params->state;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[TIME] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_time_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    return retval;
}

#endif /* 0 */
