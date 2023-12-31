/**
    \file appl_model_server_callback.c

    \brief
    This file contains reference implementation of model server callbacks.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/


/* --------------------------------------------- Header File Inclusion */
#include "appl_main.h"
#include "MS_scene_api.h"
#include "MS_generic_onoff_api.h"
#include "MS_generic_level_api.h"
#include "MS_generic_power_onoff_api.h"
#include "MS_generic_power_level_api.h"
#include "MS_generic_battery_api.h"
#include "MS_generic_location_api.h"
#include "MS_generic_property_api.h"
#include "MS_light_lightness_api.h"
#include "MS_light_ctl_api.h"
#include "MS_light_hsl_api.h"
#include "MS_light_xyl_api.h"
#include "MS_light_lc_api.h"

void* appl_scene_save_current_state(/* IN */ UINT32 scene_index);
void* appl_scene_delete_saved_state(/* IN */ UINT32 scene_index, /* IN */ void* context);
void* appl_scene_recall_saved_state(/* IN */ UINT32 scene_index, /* IN */ void* context);
API_RESULT appl_model_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction);
API_RESULT appl_model_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction);

/* --------------------------------------------- Global Definitions */
#define MS_MODEL_SERVER_APPL_CB_DECL(cb) \
    API_RESULT (cb) \
    ( \
      MS_ACCESS_MODEL_REQ_MSG_CONTEXT    * ctx, \
      MS_ACCESS_MODEL_REQ_MSG_RAW        * msg_raw, \
      MS_ACCESS_MODEL_REQ_MSG_T          * req_type, \
      MS_ACCESS_MODEL_STATE_PARAMS       * state_params, \
      MS_ACCESS_MODEL_EXT_PARAMS         * ext_params \
    );

MS_MODEL_SERVER_APPL_CB_DECL(appl_generic_onoff_server_cb);
MS_MODEL_SERVER_APPL_CB_DECL(appl_generic_level_server_cb);
MS_MODEL_SERVER_APPL_CB_DECL(appl_generic_power_onoff_server_cb);
MS_MODEL_SERVER_APPL_CB_DECL(appl_generic_power_level_server_cb);
MS_MODEL_SERVER_APPL_CB_DECL(appl_generic_battery_server_cb);
MS_MODEL_SERVER_APPL_CB_DECL(appl_generic_location_server_cb);
MS_MODEL_SERVER_APPL_CB_DECL(appl_generic_user_property_server_cb);
MS_MODEL_SERVER_APPL_CB_DECL(appl_generic_admin_property_server_cb);
MS_MODEL_SERVER_APPL_CB_DECL(appl_generic_manufacturer_property_server_cb);
MS_MODEL_SERVER_APPL_CB_DECL(appl_generic_client_property_server_cb);
MS_MODEL_SERVER_APPL_CB_DECL(appl_light_lightness_server_cb);
MS_MODEL_SERVER_APPL_CB_DECL(appl_light_ctl_server_cb);
MS_MODEL_SERVER_APPL_CB_DECL(appl_light_ctl_temperature_server_cb);
MS_MODEL_SERVER_APPL_CB_DECL(appl_light_hsl_server_cb);
MS_MODEL_SERVER_APPL_CB_DECL(appl_light_hsl_hue_server_cb);
MS_MODEL_SERVER_APPL_CB_DECL(appl_light_hsl_saturation_server_cb);
MS_MODEL_SERVER_APPL_CB_DECL(appl_light_xyl_server_cb);
MS_MODEL_SERVER_APPL_CB_DECL(appl_light_lc_server_cb);

void* appl_scene_server_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE*              handle,
    /* IN */ UINT8                                event_type,
    /* IN */ void*                                event_param,
    /* IN */ UINT16                               event_length,
    /* IN */ void*                                context
);

/* --------------------------------------------- Static Global Variables */


/* --------------------------------------------- External Global Variables */


/* --------------------------------------------- Function */
/**
    \brief Generic_Onoff Server Get Model Handle.

    \par Description
    Function to get Generic_Onoff server Model Handle.

    \param [out] model_handle        Model Handle.
    \param [in]  state_inst          State Instance.
*/
API_RESULT appl_generic_onoff_server_get_model_handle
(
    /* OUT */ MS_ACCESS_MODEL_HANDLE* model_handle,
    /* IN */  UINT16                   state_inst
)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_ID model_id;
    MS_ACCESS_ELEMENT_HANDLE elem_handle;
    /* TODO: Not always true */
    elem_handle = (MS_ACCESS_ELEMENT_HANDLE)state_inst;
    model_id.id = MS_MODEL_ID_GENERIC_ONOFF_SERVER;
    model_id.type = MS_ACCESS_MODEL_TYPE_SIG;
    *model_handle = 0x0000;
    retval = MS_access_get_model_handle
             (
                 elem_handle,
                 model_id,
                 model_handle
             );
    return retval;
}

void appl_generic_onoff_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
)
{
    MS_STATE_GENERIC_ONOFF_STRUCT     param;
    MS_ACCESS_MODEL_STATE_PARAMS      current_state_params;
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT   ctx;
    /* Modle Handle of Generic OnOff */
    appl_generic_onoff_server_get_model_handle(&ctx.handle, state_inst);
    ctx.saddr = 0; /* Unassigned address */
    appl_model_state_get(state_type, 0, &param, 0);
    current_state_params.state_type = state_type;
    current_state_params.state = &param;
    CONSOLE_OUT(
        "[GENERIC_ONOFF] Sending Status.\n");
    MS_generic_onoff_server_state_update(&ctx, &current_state_params, NULL, 0, NULL);
}


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
    MS_IGNORE_UNUSED_PARAM(msg_raw);
    MS_IGNORE_UNUSED_PARAM(ext_params);
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
    \brief Generic_Level Server Get Model Handle.

    \par Description
    Function to get Generic_Level server Model Handle.

    \param [out] model_handle        Model Handle.
    \param [in]  state_inst          State Instance.
*/
API_RESULT appl_generic_level_server_get_model_handle
(
    /* OUT */ MS_ACCESS_MODEL_HANDLE* model_handle,
    /* IN */  UINT16                   state_inst
)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_ID model_id;
    MS_ACCESS_ELEMENT_HANDLE elem_handle;
    /* TODO: Not always true */
    elem_handle = (MS_ACCESS_ELEMENT_HANDLE)state_inst;
    model_id.id = MS_MODEL_ID_GENERIC_LEVEL_SERVER;
    model_id.type = MS_ACCESS_MODEL_TYPE_SIG;
    *model_handle = 0x0000;
    retval = MS_access_get_model_handle
             (
                 elem_handle,
                 model_id,
                 model_handle
             );
    return retval;
}

void appl_generic_level_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
)
{
    MS_STATE_GENERIC_LEVEL_STRUCT     param;
    MS_ACCESS_MODEL_STATE_PARAMS      current_state_params;
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT   ctx;
    /* Modle Handle of Generic level */
    appl_generic_level_server_get_model_handle(&ctx.handle, state_inst);
    ctx.saddr = 0; /* Unassigned address */
    appl_model_state_get(state_type, 0, &param, 0);
    current_state_params.state_type = state_type;
    current_state_params.state = &param;
    CONSOLE_OUT(
        "[GENERIC_LEVEL] Sending Status.\n");
    /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
    MS_generic_level_server_state_update(&ctx, &current_state_params, NULL, 0, NULL);
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
    MS_STATE_GENERIC_LEVEL_STRUCT    param;
    MS_ACCESS_MODEL_STATE_PARAMS     current_state_params;
    API_RESULT                       retval;
    MS_IGNORE_UNUSED_PARAM(ext_params);
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
    \brief Generic_Power_Onoff Server Get Model Handle.

    \par Description
    Function to get Generic_Power_Onoff server Model Handle.

    \param [out] model_handle        Model Handle.
    \param [in]  state_inst          State Instance.
*/
API_RESULT appl_generic_power_onoff_server_get_model_handle
(
    /* OUT */ MS_ACCESS_MODEL_HANDLE* model_handle,
    /* IN */  UINT16                   state_inst
)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_ID model_id;
    MS_ACCESS_ELEMENT_HANDLE elem_handle;
    /* TODO: Not always true */
    elem_handle = (MS_ACCESS_ELEMENT_HANDLE)state_inst;
    model_id.id = MS_MODEL_ID_GENERIC_POWER_ONOFF_SERVER;
    model_id.type = MS_ACCESS_MODEL_TYPE_SIG;
    *model_handle = 0x0000;
    retval = MS_access_get_model_handle
             (
                 elem_handle,
                 model_id,
                 model_handle
             );
    return retval;
}

void appl_generic_power_onoff_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
)
{
    MS_STATE_GENERIC_ONPOWERUP_STRUCT param;
    MS_ACCESS_MODEL_STATE_PARAMS      current_state_params;
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT   ctx;
    /* Modle Handle of Generic OnOff */
    appl_generic_power_onoff_server_get_model_handle(&ctx.handle, state_inst);
    ctx.saddr = 0; /* Unassigned address */
    appl_model_state_get(state_type, 0, &param, 0);
    current_state_params.state_type = state_type;
    current_state_params.state = &param;
    CONSOLE_OUT(
        "[GENERIC_POWER_ONOFF] Sending Status.\n");
    MS_generic_power_onoff_server_state_update(&ctx, &current_state_params, NULL, 0, NULL);
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
    MS_STATE_GENERIC_ONPOWERUP_STRUCT    param;
    MS_ACCESS_MODEL_STATE_PARAMS         current_state_params;
    API_RESULT                           retval;
    MS_IGNORE_UNUSED_PARAM(msg_raw);
    MS_IGNORE_UNUSED_PARAM(ext_params);
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
    \brief Generic_Power_Level Server Get Model Handle.

    \par Description
    Function to get Generic_Power_Level server Model Handle.

    \param [out] model_handle        Model Handle.
    \param [in]  state_inst          State Instance.
*/
API_RESULT appl_generic_power_level_server_get_model_handle
(
    /* OUT */ MS_ACCESS_MODEL_HANDLE* model_handle,
    /* IN */  UINT16                   state_inst
)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_ID model_id;
    MS_ACCESS_ELEMENT_HANDLE elem_handle;
    /* TODO: Not always true */
    elem_handle = (MS_ACCESS_ELEMENT_HANDLE)state_inst;
    model_id.id = MS_MODEL_ID_GENERIC_POWER_LEVEL_SERVER;
    model_id.type = MS_ACCESS_MODEL_TYPE_SIG;
    *model_handle = 0x0000;
    retval = MS_access_get_model_handle
             (
                 elem_handle,
                 model_id,
                 model_handle
             );
    return retval;
}


void appl_generic_power_level_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
)
{
    MS_STATE_GENERIC_POWER_LEVEL_STRUCT param;
    MS_ACCESS_MODEL_STATE_PARAMS        current_state_params;
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT     ctx;
    void*                               param_p;
    /* Modle Handle of Generic Power Level */
    appl_generic_power_level_server_get_model_handle(&ctx.handle, state_inst);
    ctx.saddr = 0; /* Unassigned address */

    switch (state_type)
    {
    case MS_STATE_GENERIC_POWER_ACTUAL_T:
    {
        param_p = (MS_STATE_GENERIC_POWER_ACTUAL_STRUCT*)&param.generic_power_actual;
    }
    break;

    case MS_STATE_GENERIC_POWER_LAST_T:
    {
        param_p = (MS_STATE_GENERIC_POWER_LAST_STRUCT*)&param.generic_power_last;
    }
    break;

    case MS_STATE_GENERIC_POWER_DEFAULT_T:
    {
        param_p = (MS_STATE_GENERIC_POWER_DEFAULT_STRUCT*)&param.generic_power_default;
    }
    break;

    case MS_STATE_GENERIC_POWER_RANGE_T:
    {
        param_p = (MS_STATE_GENERIC_POWER_RANGE_STRUCT*)&param.generic_power_range;
    }
    break;
    }

    appl_model_state_get(state_type, 0, param_p, 0);
    current_state_params.state_type = state_type;
    current_state_params.state = param_p;
    CONSOLE_OUT(
        "[GENERIC_POWER_LEVEL] Sending Status.\n");
    MS_generic_power_level_server_state_update(&ctx, &current_state_params, NULL, 0, NULL);
}

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Generic_Power_Level server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_generic_power_level_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_STATE_GENERIC_POWER_LEVEL_STRUCT param;
    MS_ACCESS_MODEL_STATE_PARAMS        current_state_params;
    MS_ACCESS_MODEL_EXT_PARAMS          ext_param;
    MS_EXT_STATUS_STRUCT                status;
    void*                               param_p;
    API_RESULT                          retval;
    MS_IGNORE_UNUSED_PARAM(msg_raw);
    MS_IGNORE_UNUSED_PARAM(ext_params);
    retval = API_SUCCESS;
    param_p = NULL;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[GENERIC_POWER_LEVEL] GET Request.\n");

        switch (state_params->state_type)
        {
        case MS_STATE_GENERIC_POWER_ACTUAL_T:
        {
            param_p = (MS_STATE_GENERIC_POWER_ACTUAL_STRUCT*)&param.generic_power_actual;
        }
        break;

        case MS_STATE_GENERIC_POWER_LAST_T:
        {
            param_p = (MS_STATE_GENERIC_POWER_LAST_STRUCT*)&param.generic_power_last;
        }
        break;

        case MS_STATE_GENERIC_POWER_DEFAULT_T:
        {
            param_p = (MS_STATE_GENERIC_POWER_DEFAULT_STRUCT*)&param.generic_power_default;
        }
        break;

        case MS_STATE_GENERIC_POWER_RANGE_T:
        {
            param_p = (MS_STATE_GENERIC_POWER_RANGE_STRUCT*)&param.generic_power_range;
        }
        break;
        }

        appl_model_state_get(state_params->state_type, 0, param_p, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = param_p;
    }
    else if (MS_ACCESS_MODEL_REQ_MSG_T_SET == req_type->type)
    {
        CONSOLE_OUT(
            "[GENERIC_POWER_LEVEL] SET Request.\n");
        appl_model_state_set(state_params->state_type, 0, (MS_STATE_GENERIC_POWER_LEVEL_STRUCT*)state_params->state, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = (MS_STATE_GENERIC_POWER_LEVEL_STRUCT*)state_params->state;
        /* Assign Status */
        ext_param.ext_type = MS_EXT_STATUS_STRUCT_T;
        ext_param.ext = &status;
        status.status = 0x00;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[GENERIC_POWER_LEVEL] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_generic_power_level_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    (void)ext_param;
    return retval;
}

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Generic_Battery server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_generic_battery_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_STATE_GENERIC_BATTERY_STRUCT    param;
    MS_ACCESS_MODEL_STATE_PARAMS       current_state_params;
    API_RESULT                         retval;
    MS_IGNORE_UNUSED_PARAM(msg_raw);
    MS_IGNORE_UNUSED_PARAM(ext_params);
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[GENERIC_BATTERY] GET Request.\n");
        appl_model_state_get(state_params->state_type, 0, &param, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = &param;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[GENERIC_BATTERY] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_generic_battery_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    return retval;
}

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Generic_Location or Generic_Location_Setup server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_generic_location_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_STATE_GENERIC_LOCATION_STRUCT    param;
    MS_ACCESS_MODEL_STATE_PARAMS        current_state_params;
    API_RESULT                          retval;
    MS_IGNORE_UNUSED_PARAM(msg_raw);
    MS_IGNORE_UNUSED_PARAM(ext_params);
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[GENERIC_LOCATION] GET Request.\n");
        appl_model_state_get(state_params->state_type, 0, &param, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = &param;
    }
    else if (MS_ACCESS_MODEL_REQ_MSG_T_SET == req_type->type)
    {
        CONSOLE_OUT(
            "[GENERIC_LOCATION] SET Request.\n");
        appl_model_state_set(state_params->state_type, 0, (MS_STATE_GENERIC_LOCATION_STRUCT*)state_params->state, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = (MS_STATE_GENERIC_LOCATION_STRUCT*)state_params->state;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[GENERIC_LOCATION] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_generic_location_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    return retval;
}

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
    MS_STATE_GENERIC_USER_PROPERTY_STRUCT  user_prop_param;
    MS_STATE_GENERIC_PROPERTY_IDS_STRUCT   prop_ids;
    void*                                  param = NULL;
    /* As sample example. The array size need to defined, based on the final configuration */
    UINT16                                 prop_ids_list[10];
    MS_ACCESS_MODEL_STATE_PARAMS           current_state_params;
    API_RESULT                             retval;
    MS_IGNORE_UNUSED_PARAM(msg_raw);
    MS_IGNORE_UNUSED_PARAM(ext_params);
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

        if(param != NULL)
        {
            appl_model_state_get(state_params->state_type, 0, param, 0);
            current_state_params.state_type = state_params->state_type;
            current_state_params.state = param;
        }
    }
    else if (MS_ACCESS_MODEL_REQ_MSG_T_SET == req_type->type)
    {
        CONSOLE_OUT(
            "[GENERIC_USER_PROPERTY] SET Request.\n");
        retval = appl_model_state_set(state_params->state_type, 0, (MS_STATE_GENERIC_USER_PROPERTY_STRUCT*)state_params->state, 0);

        if (API_SUCCESS != retval)
        {
            req_type->to_be_acked = 0x00;
        }
        else
        {
            current_state_params.state_type = state_params->state_type;
            current_state_params.state = (MS_STATE_GENERIC_USER_PROPERTY_STRUCT*)state_params->state;
        }
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
    \brief Generic_Admin_Property Server Get Model Handle.

    \par Description
    Function to get Generic_Admin_Property server Model Handle.

    \param [out] model_handle        Model Handle.
    \param [in]  state_inst          State Instance.
*/
API_RESULT appl_generic_admin_property_server_get_model_handle
(
    /* OUT */ MS_ACCESS_MODEL_HANDLE* model_handle,
    /* IN */  UINT16                   state_inst
)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_ID model_id;
    MS_ACCESS_ELEMENT_HANDLE elem_handle;
    /* TODO: Not always true */
    elem_handle = (MS_ACCESS_ELEMENT_HANDLE)state_inst;
    model_id.id = MS_MODEL_ID_GENERIC_ADMIN_PROPERTY_SERVER;
    model_id.type = MS_ACCESS_MODEL_TYPE_SIG;
    *model_handle = 0x0000;
    retval = MS_access_get_model_handle
             (
                 elem_handle,
                 model_id,
                 model_handle
             );
    return retval;
}

void appl_generic_admin_property_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst,
    /* IN */  UINT16                   property_id
)
{
    MS_STATE_GENERIC_ADMIN_PROPERTY_STRUCT  prop_param;
    MS_ACCESS_MODEL_STATE_PARAMS      current_state_params;
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT   ctx;
    /* Modle Handle of Generic Admin Property */
    appl_generic_admin_property_server_get_model_handle(&ctx.handle, state_inst);
    ctx.saddr = 0; /* Unassigned address */
    prop_param.property_id = property_id;
    appl_model_state_get(state_type, 0, &prop_param, 0);
    current_state_params.state_type = state_type;
    current_state_params.state = &prop_param;
    CONSOLE_OUT(
        "[GENERIC_ADMIN_PROPERTY] Sending Status.\n");
    MS_generic_admin_property_server_state_update(&ctx, &current_state_params, NULL, 0, NULL);
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
    MS_STATE_GENERIC_ADMIN_PROPERTY_STRUCT  prop_param;
    MS_STATE_GENERIC_PROPERTY_IDS_STRUCT    prop_ids;
    void*                                   param = NULL;
    /* As sample example. The array size need to defined, based on the final configuration */
    UINT16                                  prop_ids_list[10];
    MS_ACCESS_MODEL_STATE_PARAMS            current_state_params;
    API_RESULT                              retval;
    MS_IGNORE_UNUSED_PARAM(msg_raw);
    MS_IGNORE_UNUSED_PARAM(ext_params);
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

        if(param != NULL)
        {
            appl_model_state_get(state_params->state_type, 0, param, 0);
            current_state_params.state_type = state_params->state_type;
            current_state_params.state = param;
        }
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
    \brief Generic_Manufacturer_Property Server Get Model Handle.

    \par Description
    Function to get Generic_Manufacturer_Property server Model Handle.

    \param [out] model_handle        Model Handle.
    \param [in]  state_inst          State Instance.
*/
API_RESULT appl_generic_manufacturer_property_server_get_model_handle
(
    /* OUT */ MS_ACCESS_MODEL_HANDLE* model_handle,
    /* IN */  UINT16                   state_inst
)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_ID model_id;
    MS_ACCESS_ELEMENT_HANDLE elem_handle;
    /* TODO: Not always true */
    elem_handle = (MS_ACCESS_ELEMENT_HANDLE)state_inst;
    model_id.id = MS_MODEL_ID_GENERIC_MANUFACTURER_PROPERTY_SERVER;
    model_id.type = MS_ACCESS_MODEL_TYPE_SIG;
    *model_handle = 0x0000;
    retval = MS_access_get_model_handle
             (
                 elem_handle,
                 model_id,
                 model_handle
             );
    return retval;
}

void appl_generic_manufacturer_property_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst,
    /* IN */  UINT16                   property_id
)
{
    MS_STATE_GENERIC_MANUFACTURER_PROPERTY_STRUCT  prop_param;
    MS_ACCESS_MODEL_STATE_PARAMS      current_state_params;
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT   ctx;
    /* Modle Handle of Generic Manufacturer Property */
    appl_generic_manufacturer_property_server_get_model_handle(&ctx.handle, state_inst);
    ctx.saddr = 0; /* Unassigned address */
    prop_param.property_id = property_id;
    appl_model_state_get(state_type, 0, &prop_param, 0);
    current_state_params.state_type = state_type;
    current_state_params.state = &prop_param;
    CONSOLE_OUT(
        "[GENERIC_MANUFACTURER_PROPERTY] Sending Status.\n");
    MS_generic_manufacturer_property_server_state_update(&ctx, &current_state_params, NULL, 0, NULL);
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
    MS_STATE_GENERIC_MANUFACTURER_PROPERTY_STRUCT   prop_param;
    MS_STATE_GENERIC_PROPERTY_IDS_STRUCT            prop_ids;
    void*                                           param = NULL;
    /* As sample example. The array size need to defined, based on the final configuration */
    UINT16                                          prop_ids_list[10];
    MS_ACCESS_MODEL_STATE_PARAMS                    current_state_params;
    API_RESULT                                      retval;
    MS_IGNORE_UNUSED_PARAM(msg_raw);
    MS_IGNORE_UNUSED_PARAM(ext_params);
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

        if(param != NULL)
        {
            appl_model_state_get(state_params->state_type, 0, param, 0);
            current_state_params.state_type = state_params->state_type;
            current_state_params.state = param;
        }
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
    void*                                 param = NULL;
    /* As sample example. The array size need to defined, based on the final configuration */
    UINT16                                prop_ids_list[10];
    MS_ACCESS_MODEL_STATE_PARAMS          current_state_params;
    API_RESULT                            retval;
    MS_IGNORE_UNUSED_PARAM(msg_raw);
    MS_IGNORE_UNUSED_PARAM(ext_params);
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

        if(param != NULL)
        {
            appl_model_state_get(state_params->state_type, 0, param, 0);
            current_state_params.state_type = state_params->state_type;
            current_state_params.state = param;
        }
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
    MS_IGNORE_UNUSED_PARAM(handle);
    MS_IGNORE_UNUSED_PARAM(event_length);
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
    \brief Light_Lightness Server Get Model Handle.

    \par Description
    Function to get Light_Lightness server Model Handle.

    \param [out] model_handle        Model Handle.
    \param [in]  state_inst          State Instance.
*/
API_RESULT appl_light_lightness_server_get_model_handle
(
    /* OUT */ MS_ACCESS_MODEL_HANDLE* model_handle,
    /* IN */  UINT16                   state_inst
)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_ID model_id;
    MS_ACCESS_ELEMENT_HANDLE elem_handle;
    /* TODO: Not always true */
    elem_handle = (MS_ACCESS_ELEMENT_HANDLE)state_inst;
    model_id.id = MS_MODEL_ID_LIGHT_LIGHTNESS_SERVER;
    model_id.type = MS_ACCESS_MODEL_TYPE_SIG;
    *model_handle = 0x0000;
    retval = MS_access_get_model_handle
             (
                 elem_handle,
                 model_id,
                 model_handle
             );
    return retval;
}

void appl_light_lightness_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
)
{
    MS_STATE_LIGHT_LIGHTNESS_STRUCT   param;
    MS_ACCESS_MODEL_STATE_PARAMS      current_state_params;
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT   ctx;
    /* Modle Handle of Light_Lightness */
    appl_light_lightness_server_get_model_handle(&ctx.handle, state_inst);
    ctx.saddr = 0; /* Unassigned address */
    appl_model_state_get(state_type, 0, &param, 0);
    current_state_params.state_type = state_type;
    current_state_params.state = &param;
    CONSOLE_OUT(
        "[LIGHT_LIGHTNESS] Sending Status.\n");
    MS_light_lightness_server_state_update(&ctx, &current_state_params, NULL, 0, NULL);
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
    MS_STATE_LIGHT_LIGHTNESS_STRUCT    param;
    MS_ACCESS_MODEL_STATE_PARAMS       current_state_params;
    API_RESULT                         retval;
    MS_IGNORE_UNUSED_PARAM(msg_raw);
    MS_IGNORE_UNUSED_PARAM(ext_params);
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
    \brief Light_Ctl Server Get Model Handle.

    \par Description
    Function to get Light_Ctl server Model Handle.

    \param [out] model_handle        Model Handle.
    \param [in]  state_inst          State Instance.
*/
API_RESULT appl_light_ctl_server_get_model_handle
(
    /* OUT */ MS_ACCESS_MODEL_HANDLE* model_handle,
    /* IN */  UINT16                   state_inst
)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_ID model_id;
    MS_ACCESS_ELEMENT_HANDLE elem_handle;
    /* TODO: Not always true */
    elem_handle = (MS_ACCESS_ELEMENT_HANDLE)state_inst;
    model_id.id = MS_MODEL_ID_LIGHT_CTL_SERVER;
    model_id.type = MS_ACCESS_MODEL_TYPE_SIG;
    *model_handle = 0x0000;
    retval = MS_access_get_model_handle
             (
                 elem_handle,
                 model_id,
                 model_handle
             );
    return retval;
}

void appl_light_ctl_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
)
{
    MS_STATE_LIGHT_CTL_STRUCT                      light_ctl_params;
    MS_STATE_LIGHT_CTL_TEMPERATURE_RANGE_STRUCT    light_ctl_temperature_range_params;
    MS_STATE_LIGHT_CTL_DEFAULT_STRUCT              light_ctl_default_params;
    MS_STATE_LIGHT_CTL_TEMPERATURE_STRUCT          light_ctl_temperature_params;
    MS_ACCESS_MODEL_STATE_PARAMS                   current_state_params;
    void*                                          param_p;
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT                ctx;
    /* Modle Handle of Light_Ctl */
    appl_light_ctl_server_get_model_handle(&ctx.handle, state_inst);
    ctx.saddr = 0; /* Unassigned address */
    /* Check state type */
    param_p = NULL;

    switch (state_type)
    {
    case MS_STATE_LIGHT_CTL_DEFAULT_T:
    {
        param_p = &light_ctl_default_params;
    }
    break;

    case MS_STATE_LIGHT_CTL_T:
    {
        param_p = &light_ctl_params;
    }
    break;

    case MS_STATE_LIGHT_CTL_TEMPERATURE_RANGE_T:
    {
        param_p = &light_ctl_temperature_range_params;
    }
    break;

    case MS_STATE_LIGHT_CTL_TEMPERATURE_T:
    {
        param_p = &light_ctl_temperature_params;
    }
    break;

    default:
        break;
    }

    if (NULL != param_p)
    {
        appl_model_state_get(state_type, 0, param_p, 0);
        current_state_params.state_type = state_type;
        current_state_params.state = param_p;
        CONSOLE_OUT(
            "[LIGHT_CTL] Sending Status.\n");
        MS_light_ctl_server_state_update(&ctx, &current_state_params, NULL, 0, NULL);
    }
}


/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Light_Ctl server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_light_ctl_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_ACCESS_MODEL_STATE_PARAMS                   current_state_params;
    MS_STATE_LIGHT_CTL_STRUCT                      light_ctl_params;
    MS_STATE_LIGHT_CTL_TEMPERATURE_RANGE_STRUCT    light_ctl_temperature_range_params;
    MS_STATE_LIGHT_CTL_DEFAULT_STRUCT              light_ctl_default_params;
    MS_STATE_LIGHT_CTL_TEMPERATURE_STRUCT          light_ctl_temperature_params;
    void*                                          param_p;
    API_RESULT                                     retval;
    MS_IGNORE_UNUSED_PARAM(msg_raw);
    MS_IGNORE_UNUSED_PARAM(ext_params);
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[LIGHT_CTL] GET Request.\n");

        switch (state_params->state_type)
        {
        case MS_STATE_LIGHT_CTL_DEFAULT_T:
        {
            param_p = &light_ctl_default_params;
        }
        break;

        case MS_STATE_LIGHT_CTL_T:
        {
            param_p = &light_ctl_params;
        }
        break;

        case MS_STATE_LIGHT_CTL_TEMPERATURE_RANGE_T:
        {
            param_p = &light_ctl_temperature_range_params;
        }
        break;

        case MS_STATE_LIGHT_CTL_TEMPERATURE_T:
        {
            param_p = &light_ctl_temperature_params;
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
            "[LIGHT_CTL] SET Request.\n");
        appl_model_state_set(state_params->state_type, 0, state_params->state, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = state_params->state;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[LIGHT_CTL] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_light_ctl_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    return retval;
}

/**
    \brief Light_Ctl_Temperature Server Get Model Handle.

    \par Description
    Function to get Light_Ctl_Temperature server Model Handle.

    \param [out] model_handle        Model Handle.
    \param [in]  state_inst          State Instance.
*/
API_RESULT appl_light_ctl_temperature_server_get_model_handle
(
    /* OUT */ MS_ACCESS_MODEL_HANDLE* model_handle,
    /* IN */  UINT16                   state_inst
)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_ID model_id;
    MS_ACCESS_ELEMENT_HANDLE elem_handle;
    /* TODO: Not always true */
    elem_handle = (MS_ACCESS_ELEMENT_HANDLE)state_inst;
    model_id.id = MS_MODEL_ID_LIGHT_CTL_TEMPERATURE_SERVER;
    model_id.type = MS_ACCESS_MODEL_TYPE_SIG;
    *model_handle = 0x0000;
    retval = MS_access_get_model_handle
             (
                 elem_handle,
                 model_id,
                 model_handle
             );
    return retval;
}

void appl_light_ctl_temperature_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
)
{
    MS_STATE_LIGHT_CTL_TEMPERATURE_STRUCT    param;
    MS_ACCESS_MODEL_STATE_PARAMS             current_state_params;
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT          ctx;
    /* Modle Handle of Light_Ctl_Temperature */
    appl_light_ctl_temperature_server_get_model_handle(&ctx.handle, state_inst);
    ctx.saddr = 0; /* Unassigned address */
    appl_model_state_get(state_type, 0, &param, 0);
    current_state_params.state_type = state_type;
    current_state_params.state = &param;
    CONSOLE_OUT(
        "[LIGHT_CTL_TEMPERATURE] Sending Status.\n");
    MS_light_ctl_temperature_server_state_update(&ctx, &current_state_params, NULL, 0, NULL);
}

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Light_Ctl_Temperature server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_light_ctl_temperature_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_STATE_LIGHT_CTL_TEMPERATURE_STRUCT    param;
    MS_ACCESS_MODEL_STATE_PARAMS             current_state_params;
    API_RESULT                               retval;
    MS_IGNORE_UNUSED_PARAM(msg_raw);
    MS_IGNORE_UNUSED_PARAM(ext_params);
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[LIGHT_CTL_TEMPERATURE] GET Request.\n");
        appl_model_state_get(state_params->state_type, 0, &param, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = &param;
    }
    else if (MS_ACCESS_MODEL_REQ_MSG_T_SET == req_type->type)
    {
        CONSOLE_OUT(
            "[LIGHT_CTL_TEMPERATURE] SET Request.\n");
        appl_model_state_set(state_params->state_type, 0, (MS_STATE_LIGHT_CTL_TEMPERATURE_STRUCT*)state_params->state, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = (MS_STATE_LIGHT_CTL_TEMPERATURE_STRUCT*)state_params->state;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[LIGHT_CTL_TEMPERATURE] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_light_ctl_temperature_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    return retval;
}

/**
    \brief Light_HSL Server Get Model Handle.

    \par Description
    Function to get Light_HSL server Model Handle.

    \param [out] model_handle        Model Handle.
    \param [in]  state_inst          State Instance.
*/
API_RESULT appl_light_hsl_server_get_model_handle
(
    /* OUT */ MS_ACCESS_MODEL_HANDLE* model_handle,
    /* IN */  UINT16                   state_inst
)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_ID model_id;
    MS_ACCESS_ELEMENT_HANDLE elem_handle;
    /* TODO: Not always true */
    elem_handle = (MS_ACCESS_ELEMENT_HANDLE)state_inst;
    model_id.id = MS_MODEL_ID_LIGHT_HSL_SERVER;
    model_id.type = MS_ACCESS_MODEL_TYPE_SIG;
    *model_handle = 0x0000;
    retval = MS_access_get_model_handle
             (
                 elem_handle,
                 model_id,
                 model_handle
             );
    return retval;
}

void appl_light_hsl_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
)
{
    MS_ACCESS_MODEL_STATE_PARAMS             current_state_params;
    MS_STATE_LIGHT_HSL_STRUCT            param;
    MS_STATE_LIGHT_HSL_RANGE_STRUCT      param_range;
    MS_STATE_LIGHT_HSL_DEFAULT_STRUCT    param_default;
    void*                                param_p;
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT          ctx;
    /* Modle Handle of Light_HSL */
    appl_light_hsl_server_get_model_handle(&ctx.handle, state_inst);
    ctx.saddr = 0; /* Unassigned address */
    param_p = NULL;

    switch (state_type)
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

    if (NULL != param_p)
    {
        appl_model_state_get(state_type, 0, param_p, 0);
        current_state_params.state_type = state_type;
        current_state_params.state = param_p;
        CONSOLE_OUT(
            "[LIGHT_HSL] Sending Status.\n");
        MS_light_hsl_server_state_update(&ctx, &current_state_params, NULL, 0, NULL);
    }
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
    API_RESULT                           retval;
    MS_IGNORE_UNUSED_PARAM(msg_raw);
    MS_IGNORE_UNUSED_PARAM(ext_params);
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
    \brief Light_HSL_HUE Server Get Model Handle.

    \par Description
    Function to get Light_HSL_HUE server Model Handle.

    \param [out] model_handle        Model Handle.
    \param [in]  state_inst          State Instance.
*/
API_RESULT appl_light_hsl_hue_server_get_model_handle
(
    /* OUT */ MS_ACCESS_MODEL_HANDLE* model_handle,
    /* IN */  UINT16                   state_inst
)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_ID model_id;
    MS_ACCESS_ELEMENT_HANDLE elem_handle;
    /* TODO: Not always true */
    elem_handle = (MS_ACCESS_ELEMENT_HANDLE)state_inst;
    model_id.id = MS_MODEL_ID_LIGHT_HSL_HUE_SERVER;
    model_id.type = MS_ACCESS_MODEL_TYPE_SIG;
    *model_handle = 0x0000;
    retval = MS_access_get_model_handle
             (
                 elem_handle,
                 model_id,
                 model_handle
             );
    return retval;
}

void appl_light_hsl_hue_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
)
{
    MS_ACCESS_MODEL_STATE_PARAMS       current_state_params;
    MS_STATE_LIGHT_HSL_STRUCT          param;
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT    ctx;
    /* Modle Handle of Light_HSL_HUE */
    appl_light_hsl_hue_server_get_model_handle(&ctx.handle, state_inst);
    ctx.saddr = 0; /* Unassigned address */
    appl_model_state_get(state_type, 0, &param, 0);
    current_state_params.state_type = state_type;
    current_state_params.state = &param;
    CONSOLE_OUT(
        "[LIGHT_HSL_HUE] Sending Status.\n");
    MS_light_hsl_hue_server_state_update(&ctx, &current_state_params, NULL, 0, NULL);
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
    MS_STATE_LIGHT_HSL_STRUCT       param;
    MS_ACCESS_MODEL_STATE_PARAMS    current_state_params;
    API_RESULT                      retval;
    MS_IGNORE_UNUSED_PARAM(msg_raw);
    MS_IGNORE_UNUSED_PARAM(ext_params);
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
    \brief LIGHT_HSL_SATURATION Server Get Model Handle.

    \par Description
    Function to get LIGHT_HSL_SATURATION server Model Handle.

    \param [out] model_handle        Model Handle.
    \param [in]  state_inst          State Instance.
*/
API_RESULT appl_light_hsl_saturation_server_get_model_handle
(
    /* OUT */ MS_ACCESS_MODEL_HANDLE* model_handle,
    /* IN */  UINT16                   state_inst
)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_ID model_id;
    MS_ACCESS_ELEMENT_HANDLE elem_handle;
    /* TODO: Not always true */
    elem_handle = (MS_ACCESS_ELEMENT_HANDLE)state_inst;
    model_id.id = MS_MODEL_ID_LIGHT_HSL_SATURATION_SERVER;
    model_id.type = MS_ACCESS_MODEL_TYPE_SIG;
    *model_handle = 0x0000;
    retval = MS_access_get_model_handle
             (
                 elem_handle,
                 model_id,
                 model_handle
             );
    return retval;
}

void appl_light_hsl_saturation_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
)
{
    MS_ACCESS_MODEL_STATE_PARAMS       current_state_params;
    MS_STATE_LIGHT_HSL_STRUCT          param;
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT    ctx;
    /* Modle Handle of Light_HSL_SATURATION */
    appl_light_hsl_saturation_server_get_model_handle(&ctx.handle, state_inst);
    ctx.saddr = 0; /* Unassigned address */
    appl_model_state_get(state_type, 0, &param, 0);
    current_state_params.state_type = state_type;
    current_state_params.state = &param;
    CONSOLE_OUT(
        "[LIGHT_HSL_SATURATION] Sending Status.\n");
    MS_light_hsl_saturation_server_state_update(&ctx, &current_state_params, NULL, 0, NULL);
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
    MS_STATE_LIGHT_HSL_STRUCT      param;
    MS_ACCESS_MODEL_STATE_PARAMS   current_state_params;
    API_RESULT                     retval;
    MS_IGNORE_UNUSED_PARAM(msg_raw);
    MS_IGNORE_UNUSED_PARAM(ext_params);
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

/**
    \brief Light_Xyl Server Get Model Handle.

    \par Description
    Function to get Light_Xyl server Model Handle.

    \param [out] model_handle        Model Handle.
    \param [in]  state_inst          State Instance.
*/
API_RESULT appl_light_xyl_server_get_model_handle
(
    /* OUT */ MS_ACCESS_MODEL_HANDLE* model_handle,
    /* IN */  UINT16                   state_inst
)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_ID model_id;
    MS_ACCESS_ELEMENT_HANDLE elem_handle;
    /* TODO: Not always true */
    elem_handle = (MS_ACCESS_ELEMENT_HANDLE)state_inst;
    model_id.id = MS_MODEL_ID_LIGHT_XYL_SERVER;
    model_id.type = MS_ACCESS_MODEL_TYPE_SIG;
    *model_handle = 0x0000;
    retval = MS_access_get_model_handle
             (
                 elem_handle,
                 model_id,
                 model_handle
             );
    return retval;
}

void appl_light_xyl_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst
)
{
    MS_ACCESS_MODEL_STATE_PARAMS       current_state_params;
    MS_STATE_LIGHT_XYL_STRUCT            param;
    MS_STATE_LIGHT_XYL_RANGE_STRUCT      param_range;
    MS_STATE_LIGHT_XYL_DEFAULT_STRUCT    param_default;
    void*                                param_p;
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT    ctx;
    /* Modle Handle of Light_Xyl */
    appl_light_xyl_server_get_model_handle(&ctx.handle, state_inst);
    ctx.saddr = 0; /* Unassigned address */

    switch (state_type)
    {
    case MS_STATE_LIGHT_XYL_T:
    case MS_STATE_LIGHT_XYL_TARGET_T:
    {
        param_p = &param;
    }
    break;

    case MS_STATE_LIGHT_XYL_DEFAULT_T:
    {
        param_p = &param_default;
    }
    break;

    case MS_STATE_LIGHT_XYL_RANGE_T:
    {
        param_p = &param_range;
    }
    break;

    default:
        break;
    }

    appl_model_state_get(state_type, 0, param_p, 0);
    current_state_params.state_type = state_type;
    current_state_params.state = param_p;
    CONSOLE_OUT(
        "[LIGHT_Xyl] Sending Status.\n");
    MS_light_xyl_server_state_update(&ctx, &current_state_params, NULL, 0, NULL);
}

/**
    \brief Server Application Asynchronous Notification Callback.

    \par Description
    Light_Xyl server calls the registered callback to indicate events occurred to the application.

    \param [in] ctx           Context of message received for a specific model instance.
    \param [in] msg_raw       Uninterpreted/raw received message.
    \param [in] req_type      Requested message type.
    \param [in] state_params  Model specific state parameters.
    \param [in] ext_params    Additional parameters.
*/
API_RESULT appl_light_xyl_server_cb
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*     ctx,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_RAW*         msg_raw,
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_T*           req_type,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*        state_params,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*          ext_params
)
{
    MS_ACCESS_MODEL_STATE_PARAMS         current_state_params;
    MS_STATE_LIGHT_XYL_STRUCT            param;
    MS_STATE_LIGHT_XYL_RANGE_STRUCT      param_range;
    MS_STATE_LIGHT_XYL_DEFAULT_STRUCT    param_default;
    void*                                param_p;
    API_RESULT                           retval;
    MS_IGNORE_UNUSED_PARAM(msg_raw);
    MS_IGNORE_UNUSED_PARAM(ext_params);
    retval = API_SUCCESS;

    /* Check message type */
    if (MS_ACCESS_MODEL_REQ_MSG_T_GET == req_type->type)
    {
        CONSOLE_OUT(
            "[LIGHT_XYL] GET Request.\n");

        switch (state_params->state_type)
        {
        case MS_STATE_LIGHT_XYL_T:
        case MS_STATE_LIGHT_XYL_TARGET_T:
        {
            param_p = &param;
        }
        break;

        case MS_STATE_LIGHT_XYL_DEFAULT_T:
        {
            param_p = &param_default;
        }
        break;

        case MS_STATE_LIGHT_XYL_RANGE_T:
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
            "[LIGHT_XYL] SET Request.\n");
        appl_model_state_set(state_params->state_type, 0, state_params->state, 0);
        current_state_params.state_type = state_params->state_type;
        current_state_params.state = state_params->state;
    }

    /* See if to be acknowledged */
    if (0x01 == req_type->to_be_acked)
    {
        CONSOLE_OUT(
            "[LIGHT_XYL] Sending Response.\n");
        /* Parameters: Request Context, Current State, Target State (NULL: to be ignored), Remaining Time (0: to be ignored), Additional Parameters (NULL: to be ignored) */
        retval = MS_light_xyl_server_state_update(ctx, &current_state_params, NULL, 0, NULL);
    }

    return retval;
}

/**
    \brief Light_LC Server Get Model Handle.

    \par Description
    Function to get Light_LC server Model Handle.

    \param [out] model_handle        Model Handle.
    \param [in]  state_inst          State Instance.
*/
API_RESULT appl_light_lc_server_get_model_handle
(
    /* OUT */ MS_ACCESS_MODEL_HANDLE* model_handle,
    /* IN */  UINT32                   id,
    /* IN */  UINT16                   state_inst
)
{
    API_RESULT retval;
    MS_ACCESS_MODEL_ID model_id;
    MS_ACCESS_ELEMENT_HANDLE elem_handle;
    /* TODO: Not always true */
    elem_handle = (MS_ACCESS_ELEMENT_HANDLE)state_inst;
    model_id.id = id;
    model_id.type = MS_ACCESS_MODEL_TYPE_SIG;
    *model_handle = 0x0000;
    retval = MS_access_get_model_handle
             (
                 elem_handle,
                 model_id,
                 model_handle
             );
    return retval;
}

void appl_light_lc_server_publish
(
    /* IN */  UINT8                    state_type,
    /* IN */  UINT16                   state_inst,
    /* IN */  UINT16                   property_id
)
{
    MS_ACCESS_MODEL_STATE_PARAMS            current_state_params;
    MS_STATE_LIGHT_LC_MODE_STRUCT           param_mode;
    MS_STATE_LIGHT_LC_OM_STRUCT             param_om;
    MS_STATE_LIGHT_LC_LIGHT_ONOFF_STRUCT    param_onoff;
    MS_STATE_LIGHT_LC_PROPERTY_STRUCT       param_prop;
    void*                                   param_p;
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT        ctx;

    /* Modle Handle of Light_LC */
    if (MS_STATE_LIGHT_LC_PROPERTY_T == state_type)
    {
        appl_light_lc_server_get_model_handle(&ctx.handle, MS_MODEL_ID_LIGHT_LC_SETUP_SERVER, state_inst);
    }
    else
    {
        appl_light_lc_server_get_model_handle(&ctx.handle, MS_MODEL_ID_LIGHT_LC_SERVER, state_inst);
    }

    ctx.saddr = 0; /* Unassigned address */
    param_p = NULL;

    switch (state_type)
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
        param_prop.property_id = property_id;
        param_p = &param_prop;
        break;

    default:
        break;
    }

    if (NULL != param_p)
    {
        appl_model_state_get(state_type, 0, param_p, 0);
        current_state_params.state_type = state_type;
        current_state_params.state = param_p;
        CONSOLE_OUT(
            "[LIGHT_LC] Sending Status.\n");
        MS_light_lc_server_state_update(&ctx, &current_state_params, NULL, 0, NULL);
    }
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
    MS_ACCESS_MODEL_STATE_PARAMS            current_state_params;
    void*                                   param_p;
    API_RESULT                              retval;
    MS_IGNORE_UNUSED_PARAM(msg_raw);
    MS_IGNORE_UNUSED_PARAM(ext_params);
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

