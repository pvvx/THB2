/**
    \file appl_sensor_server.c
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_sensor_server.h"
#include "appl_model_state_handler.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
static const char main_sensor_server_options[] = "\n\
======== Sensor Server Menu ========\n\
     0. Exit. \n\
     1. Refresh. \n\
 \n\
 \n\
Your Option ? \0";


/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_sensor_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_sensor_setup_server_model_handle;

/* --------------------------------------------- Function */
/* sensor server application entry point */
void main_sensor_server_operations(/* IN */ UINT8 have_menu)
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
        retval = MS_sensor_server_init
                 (
                     element_handle,
                     &appl_sensor_server_model_handle,
                     appl_sensor_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Sensor Server Initialized. Model Handle: 0x%04X\n",
                appl_sensor_server_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Sensor Server Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        retval = MS_sensor_setup_server_init
                 (
                     element_handle,
                     &appl_sensor_setup_server_model_handle,
                     appl_sensor_setup_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Sensor Setup Server Initialized. Model Handle: 0x%04X\n",
                appl_sensor_setup_server_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Sensor Setup Server Initialization Failed. Result: 0x%04X\n",
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
        ("%s", main_sensor_server_options);
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
    Sensor server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_sensor_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_STATE_SENSOR_PROPERTY_ID_STRUCT param;
    MS_ACCESS_MODEL_STATE_PARAMS                    current_state_params;
    void* param_p;
    UINT16  u16[16];
    API_RESULT retval;
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[SENSOR] GET Request.\n");
        param_p = state_params->state;

        switch (state_params->state_type)
        {
        case MS_STATE_SENSOR_DESCRIPTOR_T:
            break;

        case MS_STATE_SENSOR_CADENCE_T:
            break;

        case MS_STATE_SENSOR_SETTINGS_T:
            ((MS_STATE_SENSOR_SETTINGS_STRUCT*)param_p)->setting_property_ids = u16;
            ((MS_STATE_SENSOR_SETTINGS_STRUCT*)param_p)->setting_property_ids_count = sizeof(u16) / sizeof(UINT16);
            break;

        case MS_STATE_SENSOR_SETTING_T:
            break;

        case MS_STATE_SENSOR_DATA_T:
            break;

        case MS_STATE_SENSOR_SERIES_COLUMN_T:
            break;
        }

        appl_model_state_get(state_params->state_type, 0, param_p, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = param_p;
    }
    else if (MS_ACCESS_MODEL_REQ_MSG_T_SET == req_type->type)
    {
        CONSOLE_OUT(
            "[SENSOR] SET Request.\n");
        appl_model_state_set(state_params->state_type, 0, state_params->state, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = state_params->state;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[SENSOR] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_sensor_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    return retval;
}

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Sensor server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_sensor_setup_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_STATE_SENSOR_PROPERTY_ID_STRUCT param;
    MS_ACCESS_MODEL_STATE_PARAMS                    current_state_params;
    void* param_p;
    UINT16  u16[16];
    API_RESULT retval;
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[SENSOR] GET Request.\n");
        param_p = state_params->state;

        switch (state_params->state_type)
        {
        case MS_STATE_SENSOR_DESCRIPTOR_T:
            break;

        case MS_STATE_SENSOR_CADENCE_T:
            break;

        case MS_STATE_SENSOR_SETTINGS_T:
            ((MS_STATE_SENSOR_SETTINGS_STRUCT*)param_p)->setting_property_ids = u16;
            ((MS_STATE_SENSOR_SETTINGS_STRUCT*)param_p)->setting_property_ids_count = sizeof(u16) / sizeof(UINT16);
            break;

        case MS_STATE_SENSOR_SETTING_T:
            break;
        }

        appl_model_state_get(state_params->state_type, 0, param_p, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = param_p;
    }
    else if (MS_ACCESS_MODEL_REQ_MSG_T_SET == req_type->type)
    {
        CONSOLE_OUT(
            "[SENSOR] SET Request.\n");
        appl_model_state_set(state_params->state_type, 0, state_params->state, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = state_params->state;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[SENSOR] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_sensor_setup_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    return retval;
}

#endif /* 0 */
