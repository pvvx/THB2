/**
    \file appl_generic_power_onoff_server.c

    Illustration of usage of following model combinations
    - Generic Power OnOff Server
    - Generic Power OnOff Setup Server
    - Generic OnOff Server
    - Generic Default Transition Time Server
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_generic_power_onoff_server.h"
#include "appl_model_state_handler.h"
#include "MS_generic_power_onoff_api.h"
#include "MS_generic_onoff_api.h"


/* --------------------------------------------- Global Definitions */
void appl_model_power_cycle(void);

/* --------------------------------------------- Static Global Variables */
static const char main_generic_power_onoff_server_options[] = "\n\
======== Generic_Power_Onoff Server Menu ========\n\
     0. Exit. \n\
     1. Refresh. \n\
 \n\
     2. Power Cycle. \n\
 \n\
Your Option ? \0";


/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_generic_power_onoff_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_generic_power_onoff_setup_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_generic_onoff_server_model_handle;
#if 0
    static MS_ACCESS_MODEL_HANDLE   appl_generic_default_transition_time_server_model_handle;
#endif /* 0 */

/* --------------------------------------------- Function */
/* generic_power_onoff server application entry point */
void main_generic_power_onoff_server_operations(/* IN */ UINT8 have_menu)
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
        retval = MS_generic_power_onoff_server_init
                 (
                     element_handle,
                     &appl_generic_power_onoff_server_model_handle,
                     appl_generic_power_onoff_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Generic Power Onoff Server Initialized. Model Handle: 0x%04X\n",
                appl_generic_power_onoff_server_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Generic Power Onoff Server Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        retval = MS_generic_power_onoff_setup_server_init
                 (
                     element_handle,
                     &appl_generic_power_onoff_setup_server_model_handle,
                     appl_generic_power_onoff_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Generic Power Onoff Setup Server Initialized. Model Handle: 0x%04X\n",
                appl_generic_power_onoff_setup_server_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Generic Power Onoff Setup Server Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        retval = MS_generic_onoff_server_init
                 (
                     element_handle,
                     &appl_generic_onoff_server_model_handle,
                     appl_generic_onoff_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Generic Onoff Server Initialized. Model Handle: 0x%04X\n",
                appl_generic_onoff_server_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Generic Onoff Server Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        #if 0
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

        #endif /* 0 */
        appl_model_states_initialization();
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
        ("%s", main_generic_power_onoff_server_options);
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
            appl_model_power_cycle();
            break;
        }
    }
}

#if 0
/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Generic_Onoff server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_generic_onoff_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_STATE_GENERIC_ONOFF_STRUCT    param;
    MS_ACCESS_MODEL_STATE_PARAMS     current_state_params;
    API_RESULT                       retval;
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT("[GENERIC_ONOFF] GET Request.\n");
        appl_model_state_get(state_params->state_type, 0, &param, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = &param;
        /* Using same as target state and remaining time as 0 */
    }
    else if (MS_ACCESS_MODEL_REQ_MSG_T_SET == req_type->type)
    {
        CONSOLE_OUT("[GENERIC_ONOFF] SET Request.\n");
        retval = appl_model_state_set(state_params->state_type, 0, (MS_STATE_GENERIC_ONOFF_STRUCT*)state_params->state, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = (MS_STATE_GENERIC_ONOFF_STRUCT*)state_params->state;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT("[GENERIC_ONOFF] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_generic_onoff_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    return retval;
}

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Generic_Power_Onoff server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_generic_power_onoff_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_STATE_GENERIC_ONPOWERUP_STRUCT param;
    MS_ACCESS_MODEL_STATE_PARAMS                    current_state_params;
    API_RESULT retval;
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[GENERIC_POWER_ONOFF] GET Request.\n");
        appl_model_state_get(state_params->state_type, 0, &param, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = &param;
    }
    else if (MS_ACCESS_MODEL_REQ_MSG_T_SET == req_type->type)
    {
        CONSOLE_OUT(
            "[GENERIC_POWER_ONOFF] SET Request.\n");
        appl_model_state_set(state_params->state_type, 0, (MS_STATE_GENERIC_ONPOWERUP_STRUCT*)state_params->state, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = (MS_STATE_GENERIC_ONPOWERUP_STRUCT*)state_params->state;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[GENERIC_POWER_ONOFF] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_generic_power_onoff_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    return retval;
}

#endif /* 0 */
