/**
    \file appl_light_lc_server.c
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_light_lc_server.h"
#include "appl_model_state_handler.h"

#include "MS_scene_api.h"
#include "MS_light_lightness_api.h"
#include "MS_generic_onoff_api.h"

/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
static const char main_light_lc_server_options[] = "\n\
======== Light_Lc Server Menu ========\n\
     0. Exit. \n\
     1. Refresh. \n\
 \n\
     2. Set Light Linear \n\
 \n\
Your Option ? \0";


/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_light_lc_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_light_lc_setup_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_light_lightness_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_generic_onoff_server_model_handle;

static MS_ACCESS_MODEL_HANDLE   appl_scene_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_scene_setup_server_model_handle;

void appl_light_lightness_set_linear(UINT16 state_inst, UINT16 linear);;

/* --------------------------------------------- Function */
/* light_lc server application entry point */
void main_light_lc_server_operations(/* IN */ UINT8 have_menu)
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
        retval = MS_light_lc_server_init
                 (
                     element_handle,
                     &appl_light_lc_server_model_handle,
                     &appl_light_lc_setup_server_model_handle,
                     appl_light_lc_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Light LC Server Initialized. Model Handle: 0x%04X\n",
                appl_light_lc_server_model_handle);
            CONSOLE_OUT(
                "Light LC Setup Server Initialized. Model Handle: 0x%04X\n",
                appl_light_lc_setup_server_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Light Lc Server Initialization Failed. Result: 0x%04X\n",
                retval);
        }

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
        ("%s", main_light_lc_server_options);
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
            CONSOLE_OUT
            ("Enter Light Linerar Value (in HEX)\n");
            CONSOLE_IN("%x", &choice);
            appl_light_lightness_set_linear(0, (UINT16)choice);;
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
    Light_Lc server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_light_lc_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_STATE_LIGHT_LC_MODE_STRUCT           param_mode;
    MS_STATE_LIGHT_LC_OM_STRUCT             param_om;
    MS_STATE_LIGHT_LC_LIGHT_ONOFF_STRUCT    param_onoff;
    MS_ACCESS_MODEL_STATE_PARAMS                    current_state_params;
    void* param_p;
    API_RESULT retval;
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[LIGHT_LC] GET Request.\n");

        switch (state_params->state_type)
        {
        case MS_STATE_LIGHT_LC_MODE_T:
            param_p = &param_mode;
            break;

        case MS_STATE_LIGHT_LC_OM_T:
            param_p = &param_om;
            break;

        case MS_STATE_LIGHT_LC_LIGHT_ONOFF_T:
            param_p = &param_onoff;
            break;

        case MS_STATE_LIGHT_LC_PROPERTY_T:
            param_p = state_params->state;
            break;
        }

        appl_model_state_get(state_params->state_type, 0, param_p, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = param_p;
    }
    else if (MS_ACCESS_MODEL_REQ_MSG_T_SET == req_type->type)
    {
        CONSOLE_OUT(
            "[LIGHT_LC] SET Request.\n");
        appl_model_state_set(state_params->state_type, 0, state_params->state, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = state_params->state;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[LIGHT_LC] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_light_lc_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    return retval;
}

#endif /* 0 */
