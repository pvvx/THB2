/**
    \file appl_generic_property_server.c
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_generic_property_server.h"
#include "appl_model_state_handler.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
static const char main_generic_property_server_options[] = "\n\
======== Generic_Property Server Menu ========\n\
     0. Exit. \n\
     1. Refresh. \n\
 \n\
 \n\
Your Option ? \0";


/* --------------------------------------------- External Global Variables */
static MS_ACCESS_MODEL_HANDLE   appl_generic_user_property_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_generic_admin_property_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_generic_manufacturer_property_server_model_handle;
static MS_ACCESS_MODEL_HANDLE   appl_generic_client_property_server_model_handle;

/* --------------------------------------------- Function */
/* generic_property server application entry point */
void main_generic_property_server_operations(/* IN */ UINT8 have_menu)
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
        retval = MS_generic_user_property_server_init
                 (
                     element_handle,
                     &appl_generic_user_property_server_model_handle,
                     appl_generic_user_property_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Generic User Property Server Initialized. Model Handle: 0x%04X\n",
                appl_generic_user_property_server_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Generic User Property Server Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        retval = MS_generic_admin_property_server_init
                 (
                     element_handle,
                     &appl_generic_admin_property_server_model_handle,
                     appl_generic_admin_property_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Generic Admin Property Server Initialized. Model Handle: 0x%04X\n",
                appl_generic_admin_property_server_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Generic Admin Property Server Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        retval = MS_generic_manufacturer_property_server_init
                 (
                     element_handle,
                     &appl_generic_manufacturer_property_server_model_handle,
                     appl_generic_manufacturer_property_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Generic Manufacturer Property Server Initialized. Model Handle: 0x%04X\n",
                appl_generic_manufacturer_property_server_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Generic Manufacturer Property Server Initialization Failed. Result: 0x%04X\n",
                retval);
        }

        retval = MS_generic_client_property_server_init
                 (
                     element_handle,
                     &appl_generic_client_property_server_model_handle,
                     appl_generic_client_property_server_cb
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT(
                "Generic Client Property Server Initialized. Model Handle: 0x%04X\n",
                appl_generic_client_property_server_model_handle);
        }
        else
        {
            CONSOLE_OUT(
                "[ERR] Generic Client Property Server Initialization Failed. Result: 0x%04X\n",
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
        ("%s", main_generic_property_server_options);
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
    Generic_User_Property server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_generic_user_property_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_STATE_GENERIC_USER_PROPERTY_STRUCT user_prop_param;
    MS_STATE_GENERIC_PROPERTY_IDS_STRUCT  prop_ids;
    void* param;
    /* As sample example. The array size need to defined, based on the final configuration */
    UINT16                                prop_ids_list[10];
    MS_ACCESS_MODEL_STATE_PARAMS                    current_state_params;
    API_RESULT retval;
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[GENERIC_USER_PROPERTY] GET Request. Type: 0x%02X\n", state_params->state_type);

        if (MS_STATE_GENERIC_USER_PROPERTY_T == state_params->state_type)
        {
            user_prop_param.property_id = ((MS_STATE_GENERIC_PROPERTY_ID_STRUCT*)state_params->state)->property_id;
            param = &user_prop_param;
        }
        else if (MS_STATE_GENERIC_USER_PROPERTY_IDS_T == state_params->state_type)
        {
            param = &prop_ids;
            prop_ids.property_ids = prop_ids_list;
            prop_ids.property_ids_count = sizeof(prop_ids_list) / sizeof(UINT16);
        }

        appl_model_state_get(state_params->state_type, 0, param, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = param;
    }
    else if (MS_ACCESS_MODEL_REQ_MSG_T_SET == req_type->type)
    {
        CONSOLE_OUT(
            "[GENERIC_USER_PROPERTY] SET Request.\n");
        appl_model_state_set(state_params->state_type, 0, (MS_STATE_GENERIC_USER_PROPERTY_STRUCT*)state_params->state, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = (MS_STATE_GENERIC_USER_PROPERTY_STRUCT*)state_params->state;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[GENERIC_USER_PROPERTY] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_generic_user_property_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    return retval;
}

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Generic_Admin_Property server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_generic_admin_property_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_STATE_GENERIC_ADMIN_PROPERTY_STRUCT prop_param;
    MS_STATE_GENERIC_PROPERTY_IDS_STRUCT   prop_ids;
    void* param;
    /* As sample example. The array size need to defined, based on the final configuration */
    UINT16                                prop_ids_list[10];
    MS_ACCESS_MODEL_STATE_PARAMS                    current_state_params;
    API_RESULT retval;
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[GENERIC_ADMIN_PROPERTY] GET Request. Type: 0x%02X\n", state_params->state_type);

        if (MS_STATE_GENERIC_ADMIN_PROPERTY_T == state_params->state_type)
        {
            prop_param.property_id = ((MS_STATE_GENERIC_PROPERTY_ID_STRUCT*)state_params->state)->property_id;
            param = &prop_param;
        }
        else if (MS_STATE_GENERIC_ADMIN_PROPERTY_IDS_T == state_params->state_type)
        {
            param = &prop_ids;
            prop_ids.property_ids = prop_ids_list;
            prop_ids.property_ids_count = sizeof(prop_ids_list) / sizeof(UINT16);
        }

        appl_model_state_get(state_params->state_type, 0, param, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = param;
    }
    else if (MS_ACCESS_MODEL_REQ_MSG_T_SET == req_type->type)
    {
        CONSOLE_OUT(
            "[GENERIC_ADMIN_PROPERTY] SET Request.\n");
        appl_model_state_set(state_params->state_type, 0, (MS_STATE_GENERIC_ADMIN_PROPERTY_STRUCT*)state_params->state, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = (MS_STATE_GENERIC_ADMIN_PROPERTY_STRUCT*)state_params->state;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[GENERIC_ADMIN_PROPERTY] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_generic_admin_property_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    return retval;
}

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Generic_Manufacturer_Property server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_generic_manufacturer_property_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_STATE_GENERIC_MANUFACTURER_PROPERTY_STRUCT prop_param;
    MS_STATE_GENERIC_PROPERTY_IDS_STRUCT   prop_ids;
    void* param;
    /* As sample example. The array size need to defined, based on the final configuration */
    UINT16                                prop_ids_list[10];
    MS_ACCESS_MODEL_STATE_PARAMS                    current_state_params;
    API_RESULT retval;
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[GENERIC_MANUFACTURER_PROPERTY] GET Request. Type: 0x%02X\n", state_params->state_type);

        if (MS_STATE_GENERIC_MANUFACTURER_PROPERTY_T == state_params->state_type)
        {
            prop_param.property_id = ((MS_STATE_GENERIC_PROPERTY_ID_STRUCT*)state_params->state)->property_id;
            param = &prop_param;
        }
        else if (MS_STATE_GENERIC_MANUFACTURER_PROPERTY_IDS_T == state_params->state_type)
        {
            param = &prop_ids;
            prop_ids.property_ids = prop_ids_list;
            prop_ids.property_ids_count = sizeof(prop_ids_list) / sizeof(UINT16);
        }

        appl_model_state_get(state_params->state_type, 0, param, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = param;
    }
    else if (MS_ACCESS_MODEL_REQ_MSG_T_SET == req_type->type)
    {
        CONSOLE_OUT(
            "[GENERIC_MANUFACTURER_PROPERTY] SET Request.\n");
        appl_model_state_set(state_params->state_type, 0, (MS_STATE_GENERIC_MANUFACTURER_PROPERTY_STRUCT*)state_params->state, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = (MS_STATE_GENERIC_MANUFACTURER_PROPERTY_STRUCT*)state_params->state;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[GENERIC_MANUFACTURER_PROPERTY] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_generic_manufacturer_property_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    return retval;
}

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Generic_Client_Property server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_generic_client_property_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_STATE_GENERIC_PROPERTY_IDS_STRUCT  prop_ids;
    void* param;
    /* As sample example. The array size need to defined, based on the final configuration */
    UINT16                                prop_ids_list[10];
    MS_ACCESS_MODEL_STATE_PARAMS                    current_state_params;
    API_RESULT retval;
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[GENERIC_CLIENT_PROPERTY] GET Request. Type: 0x%02X\n", state_params->state_type);

        if (MS_STATE_GENERIC_CLIENT_PROPERTY_IDS_T == state_params->state_type)
        {
            param = &prop_ids;
            prop_ids.property_ids = prop_ids_list;
            prop_ids.property_ids_count = sizeof(prop_ids_list) / sizeof(UINT16);
        }

        appl_model_state_get(state_params->state_type, 0, param, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = param;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[GENERIC_CLIENT_PROPERTY] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_generic_client_property_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    return retval;
}


#endif /* 0 */
