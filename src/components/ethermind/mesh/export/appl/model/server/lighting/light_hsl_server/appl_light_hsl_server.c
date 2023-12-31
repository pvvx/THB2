/**
    \file appl_light_hsl_server.c
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_light_hsl_server.h"
#include "appl_model_state_handler.h"
#include "MS_scene_api.h"
#include "MS_light_lightness_api.h"
#include "MS_generic_power_onoff_api.h"
#include "MS_generic_onoff_api.h"
#include "MS_generic_level_api.h"

/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
static const char main_light_hsl_server_options[] = "\n\
======== Light_Hsl Server Menu ========\n\
     0. Exit. \n\
     1. Refresh. \n\
 \n\
     2. Power Cycle. \n\
\n\
Your Option ? \0";


/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_light_hsl_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_light_hsl_setup_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_light_hsl_hue_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_light_hsl_saturation_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_light_lightness_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_light_lightness_setup_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_generic_power_onoff_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_generic_power_onoff_setup_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_generic_onoff_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_generic_level_server_model_handle;

#ifndef HSL_DONT_USE_MULTI_ELEMENTS
    static MS_ACCESS_MODEL_HANDLE   appl_generic_level_server_hue_model_handle;
    static MS_ACCESS_MODEL_HANDLE   appl_generic_level_server_saturation_model_handle;
#endif /* HSL_DONT_USE_MULTI_ELEMENTS */

static MS_ACCESS_MODEL_HANDLE   appl_scene_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_scene_setup_server_model_handle;

/* --------------------------------------------- Function */
#ifndef HSL_DONT_USE_MULTI_ELEMENTS
    extern MS_ACCESS_ELEMENT_HANDLE sec_element_handle;
    extern MS_ACCESS_ELEMENT_HANDLE ter_element_handle;
#endif /* HSL_DONT_USE_MULTI_ELEMENTS */

/* light_hsl server application entry point */
void main_light_hsl_server_operations(/* IN */ UINT8 have_menu)
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
        retval = MS_light_hsl_server_init
                 (
                     element_handle,
                     &appl_light_hsl_server_model_handle,
                     &appl_light_hsl_setup_server_model_handle,
                     appl_light_hsl_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Light Hsl Server Initialized. Model Handle: 0x%04X\n",
                appl_light_hsl_server_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Light Hsl Server Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        #ifndef HSL_DONT_USE_MULTI_ELEMENTS
        /* Hue Element */
        element_handle = sec_element_handle;
        #endif /* HSL_DONT_USE_MULTI_ELEMENTS */
        retval = MS_light_hsl_hue_server_init
                 (
                     element_handle,
                     &appl_light_hsl_hue_server_model_handle,
                     appl_light_hsl_hue_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Light Hsl Hue Server Initialized. Model Handle: 0x%04X\n",
                appl_light_hsl_hue_server_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Light Hsl Hue Server Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        #ifndef HSL_DONT_USE_MULTI_ELEMENTS
        retval = MS_generic_level_server_init
                 (
                     element_handle,
                     &appl_generic_level_server_hue_model_handle,
                     appl_generic_level_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Generic Level Server Initialized. Model Handle: 0x%04X\n",
                appl_generic_level_server_hue_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Generic Level Server Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        /* Saturation Element */
        element_handle = ter_element_handle;
        #endif /* HSL_DONT_USE_MULTI_ELEMENTS */
        retval = MS_light_hsl_saturation_server_init
                 (
                     element_handle,
                     &appl_light_hsl_saturation_server_model_handle,
                     appl_light_hsl_saturation_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Light Hsl Saturation Server Initialized. Model Handle: 0x%04X\n",
                appl_light_hsl_saturation_server_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Light Hsl Saturation Server Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        #ifndef HSL_DONT_USE_MULTI_ELEMENTS
        retval = MS_generic_level_server_init
                 (
                     element_handle,
                     &appl_generic_level_server_saturation_model_handle,
                     appl_generic_level_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Generic Level Server Initialized. Model Handle: 0x%04X\n",
                appl_generic_level_server_saturation_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Generic Level Server Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        /* Use Default Element Handle (main). Index 0 */
        element_handle = MS_ACCESS_DEFAULT_ELEMENT_HANDLE;
        #endif /* HSL_DONT_USE_MULTI_ELEMENTS */
        retval = MS_light_lightness_server_init
                 (
                     element_handle,
                     &appl_light_lightness_server_model_handle,
                     appl_light_lightness_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Light Lightness Server Initialized. Model Handle: 0x%04X\n",
                appl_light_lightness_server_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Light Lightness Server Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        retval = MS_light_lightness_setup_server_init
                 (
                     element_handle,
                     &appl_light_lightness_setup_server_model_handle,
                     appl_light_lightness_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Light Lightness Setup Server Initialized. Model Handle: 0x%04X\n",
                appl_light_lightness_setup_server_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Light Lightness Setup Server Initialization Failed. Result: 0x%04X\n",
                retval);
        }

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

        #endif
        retval = MS_generic_level_server_init
                 (
                     element_handle,
                     &appl_generic_level_server_model_handle,
                     appl_generic_level_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Generic Level Server Initialized. Model Handle: 0x%04X\n",
                appl_generic_level_server_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Generic Level Server Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        retval = MS_scene_server_init
                 (
                     element_handle,
                     &appl_scene_server_model_handle,
                     &appl_scene_setup_server_model_handle,
                     appl_scene_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Scene Server Initialized. Model Handle: 0x%04X\n",
                appl_scene_server_model_handle);
            CONSOLE_OUT(
                "Scene Setup Server Initialized. Model Handle: 0x%04X\n",
                appl_scene_setup_server_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Scene Server Initialization Failed. Result: 0x%04X\n",
                retval);
        }

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
        ("%s", main_light_hsl_server_options);
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
    Generic_Level server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_generic_level_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_STATE_GENERIC_LEVEL_STRUCT param;
    MS_ACCESS_MODEL_STATE_PARAMS                    current_state_params;
    API_RESULT retval;
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[GENERIC_LEVEL] GET Request.\n");
        appl_model_state_get(state_params->state_type, 0, &param, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = &param;
    }
    else if (MS_ACCESS_MODEL_REQ_MSG_T_SET == req_type->type)
    {
        CONSOLE_OUT(
            "[GENERIC_LEVEL] SET Request.\n");

        if ((MS_ACCESS_GENERIC_LEVEL_SET_OPCODE == msg_raw->opcode) ||
                (MS_ACCESS_GENERIC_LEVEL_SET_UNACKNOWLEDGED_OPCODE == msg_raw->opcode))
        {
            CONSOLE_OUT(
                "[GENERIC_LEVEL] Level SET Request.\n");
        }
        else if ((MS_ACCESS_GENERIC_DELTA_SET_OPCODE == msg_raw->opcode) ||
                 (MS_ACCESS_GENERIC_DELTA_SET_UNACKNOWLEDGED_OPCODE == msg_raw->opcode))
        {
            CONSOLE_OUT(
                "[GENERIC_LEVEL] Delta SET Request.\n");
        }
        else if ((MS_ACCESS_GENERIC_MOVE_SET_OPCODE == msg_raw->opcode) ||
                 (MS_ACCESS_GENERIC_MOVE_SET_UNACKNOWLEDGED_OPCODE == msg_raw->opcode))
        {
            CONSOLE_OUT(
                "[GENERIC_LEVEL] Move SET Request.\n");
        }

        /* TODO: Right now not handling different type of SET requests separately */
        appl_model_state_set(state_params->state_type, 0, (MS_STATE_GENERIC_LEVEL_STRUCT*)state_params->state, 0);
        current_state_params.state_type = MS_STATE_GENERIC_LEVEL_T;
        current_state_params.state = (MS_STATE_GENERIC_LEVEL_STRUCT*)state_params->state;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[GENERIC_LEVEL] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_generic_level_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
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

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Scene server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.

    TODO: Update
*/
void* appl_scene_server_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE*              handle,
    /* IN */ UINT8                                event_type,
    /* IN */ void*                                event_param,
    /* IN */ UINT16                               event_length,
    /* IN */ void*                                context
)
{
    void* param_p;
    param_p = NULL;

    switch(event_type)
    {
    case MS_SCENE_EVENT_STORE:
    {
        param_p = appl_scene_save_current_state(*(UINT32*)event_param);
    }
    break;

    case MS_SCENE_EVENT_DELETE:
    {
        param_p = appl_scene_delete_saved_state(*(UINT32*)event_param, context);
    }
    break;

    case MS_SCENE_EVENT_RECALL_START:
    {
    }
    break;

    case MS_SCENE_EVENT_RECALL_COMPLETE:
    {
        param_p = appl_scene_recall_saved_state(*(UINT32*)event_param, context);
    }
    break;

    case MS_SCENE_EVENT_RECALL_IMMEDIATE:
    {
        param_p = appl_scene_recall_saved_state(*(UINT32*)event_param, context);
    }
    break;
    }

    return param_p;
}

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Light_Lightness server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_light_lightness_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_STATE_LIGHT_LIGHTNESS_STRUCT param;
    MS_ACCESS_MODEL_STATE_PARAMS                    current_state_params;
    API_RESULT retval;
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[LIGHT_LIGHTNESS] GET Request.\n");
        appl_model_state_get(state_params->state_type, 0, &param, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = &param;
    }
    else if (MS_ACCESS_MODEL_REQ_MSG_T_SET == req_type->type)
    {
        CONSOLE_OUT(
            "[LIGHT_LIGHTNESS] SET Request.\n");
        appl_model_state_set(state_params->state_type, 0, (MS_STATE_LIGHT_LIGHTNESS_STRUCT*)state_params->state, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = (MS_STATE_LIGHT_LIGHTNESS_STRUCT*)state_params->state;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[LIGHT_LIGHTNESS] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_light_lightness_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    return retval;
}

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Light_Hsl server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_light_hsl_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_ACCESS_MODEL_STATE_PARAMS         current_state_params;
    MS_STATE_LIGHT_HSL_STRUCT            param;
    MS_STATE_LIGHT_HSL_RANGE_STRUCT      param_range;
    MS_STATE_LIGHT_HSL_DEFAULT_STRUCT    param_default;
    void*                                param_p;
    API_RESULT retval;
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[LIGHT_HSL] GET Request.\n");

        switch (state_params->state_type)
        {
        case MS_STATE_LIGHT_HSL_T:
        case MS_STATE_LIGHT_HSL_HUE_T:
        case MS_STATE_LIGHT_HSL_SATURATION_T:
        case MS_STATE_LIGHT_HSL_TARGET_T:
        {
            param_p = &param;
        }
        break;

        case MS_STATE_LIGHT_HSL_DEFAULT_T:
        {
            param_p = &param_default;
        }
        break;

        case MS_STATE_LIGHT_HSL_RANGE_T:
        {
            param_p = &param_range;
        }
        break;

        default:
            break;
        }

        appl_model_state_get(state_params->state_type, 0, param_p, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = param_p;
    }
    else if (MS_ACCESS_MODEL_REQ_MSG_T_SET == req_type->type)
    {
        CONSOLE_OUT(
            "[LIGHT_HSL] SET Request.\n");
        appl_model_state_set(state_params->state_type, 0, state_params->state, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = state_params->state;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[LIGHT_HSL] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_light_hsl_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    return retval;
}

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Light_Hsl_Hue server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_light_hsl_hue_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_STATE_LIGHT_HSL_STRUCT                   param;
    MS_ACCESS_MODEL_STATE_PARAMS                    current_state_params;
    API_RESULT retval;
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[LIGHT_HSL_HUE] GET Request.\n");
        appl_model_state_get(state_params->state_type, 0, &param, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = &param;
    }
    else if (MS_ACCESS_MODEL_REQ_MSG_T_SET == req_type->type)
    {
        CONSOLE_OUT(
            "[LIGHT_HSL_HUE] SET Request.\n");
        appl_model_state_set(state_params->state_type, 0, state_params->state, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = state_params->state;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[LIGHT_HSL_HUE] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_light_hsl_hue_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    return retval;
}

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Light_Hsl_Saturation server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_light_hsl_saturation_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_STATE_LIGHT_HSL_STRUCT param;
    MS_ACCESS_MODEL_STATE_PARAMS                    current_state_params;
    API_RESULT retval;
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[LIGHT_HSL_SATURATION] GET Request.\n");
        appl_model_state_get(state_params->state_type, 0, &param, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = &param;
    }
    else if (MS_ACCESS_MODEL_REQ_MSG_T_SET == req_type->type)
    {
        CONSOLE_OUT(
            "[LIGHT_HSL_SATURATION] SET Request.\n");
        appl_model_state_set(state_params->state_type, 0, state_params->state, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = state_params->state;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[LIGHT_HSL_SATURATION] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_light_hsl_saturation_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    return retval;
}

#endif /* 0 */
