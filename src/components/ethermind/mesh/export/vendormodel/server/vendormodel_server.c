/**
    \file phy_model_server.c

    \brief This file defines the Mesh Configuration Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/



/* --------------------------------------------- Header File Inclusion */
#include "vendormodel_server.h"


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */
static DECL_CONST UINT32 vendormodel_server_opcode_list[] =
{
    MS_ACCESS_VENDORMODEL_GET_OPCODE,
    MS_ACCESS_VENDORMODEL_SET_OPCODE,
    MS_ACCESS_VENDORMODEL_SET_UNACKNOWLEDGED_OPCODE,
    MS_ACCESS_VENDORMODEL_WRITECMD_OPCODE,
    MS_ACCESS_VENDORMODEL_CONFIRMATION_OPCODE,
    MS_ACCESS_VENDORMODEL_NOTIFY_OPCODE
};

//static MS_ACCESS_MODEL_HANDLE       phy_model_server_model_handle;
static MS_VENDORMODEL_SERVER_CB       vendormodel_server_UI_cb;

/* ----------------------------------------- Functions */
/**
    \brief API to send reply or to update state change

    \par Description
    This is to send reply for a request or to inform change in state.

    \param [in] ctx                     Context of the message.
    \param [in] current_state_params    Model specific current state parameters.
    \param [in] target_state_params     Model specific target state parameters (NULL: to be ignored).
    \param [in] remaining_time          Time from current state to target state (0: to be ignored).
    \param [in] ext_params              Additional parameters (NULL: to be ignored).

    \return API_SUCCESS or an error code indicating reason for failure
*/
__ATTR_SECTION_XIP__ API_RESULT MS_vendormodel_server_state_update
(
    /* IN */ MS_ACCESS_MODEL_REQ_MSG_CONTEXT*      ctx,
    /* IN */ MS_ACCESS_VENDORMODEL_STATE_PARAMS*     current_state_params,
    /* IN */ MS_ACCESS_MODEL_STATE_PARAMS*         target_state_params,
    /* IN */ UINT16                              remaining_time,
    /* IN */ MS_ACCESS_MODEL_EXT_PARAMS*           ext_params,
    /* IN */ UINT16                              data_length
)
{
    API_RESULT retval;
    /* TODO: Check what should be maximum length */
    UCHAR      buffer[256];
    UCHAR*     pdu_ptr;
    UINT16     marker;
    UINT32     opcode;
//    UINT16       ttl;
//    ACCESS_CM_GET_RX_TTL(ttl);
    retval = API_FAILURE;
    marker = 0;

//    printf(
//    "current_state_params->state_type 0x%04X.\n",current_state_params->phy_mode_type);

    switch (current_state_params->vendormodel_type)
    {
    case MS_STATE_VENDORMODEL_ONOFF_T:
    {
        buffer[marker] = ++vendor_tid;
        marker++;
        MS_PACK_LE_2_BYTE_VAL(&buffer[marker], current_state_params->vendormodel_type);
        marker += 2;
        EM_mem_copy(&buffer[marker], current_state_params->vendormodel_param, 1);
        marker++;
        /* Set Opcode */
        opcode = MS_ACCESS_VENDORMODEL_STATUS_OPCODE;
    }
    break;

    case MS_STATE_VENDORMODEL_NOTIFY_T:
    {
        EM_mem_copy(&buffer[marker], current_state_params->vendormodel_param, data_length);
        marker += data_length;
        /* Set Opcode */
        opcode = MS_ACCESS_VENDORMODEL_NOTIFY_OPCODE;
    }
    break;

    default:
        break;
    }

    /* Publish - reliable */
    if (0 == marker)
    {
        pdu_ptr = NULL;
    }
    else
    {
        pdu_ptr = buffer;
    }

    retval = MS_access_reply
             (
                 &ctx->handle,
                 ctx->daddr,
                 ctx->saddr,
                 ctx->subnet_handle,
                 ctx->appkey_handle,
                 ACCESS_INVALID_DEFAULT_TTL,
                 opcode,
                 pdu_ptr,
                 marker
             );
    return retval;
}

/**
    \brief Access Layer Application Asynchronous Notification Callback.

    \par Description
    Access Layer calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] saddr         16 bit Source Address.
    \param [in] daddr         16 bit Destination Address.
    \param [in] appkey_handle AppKey Handle.
    \param [in] subnet_handle Subnet Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
__ATTR_SECTION_XIP__ API_RESULT vendormodel_server_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ MS_NET_ADDR              saddr,
    /* IN */ MS_NET_ADDR              daddr,
    /* IN */ MS_SUBNET_HANDLE         subnet_handle,
    /* IN */ MS_APPKEY_HANDLE         appkey_handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
)
{
    MS_ACCESS_MODEL_REQ_MSG_CONTEXT         req_context;
    MS_ACCESS_MODEL_REQ_MSG_RAW             req_raw;
    MS_ACCESS_MODEL_REQ_MSG_T               req_type;
    MS_ACCESS_MODEL_EXT_PARAMS*               ext_params_p;
    MS_ACCESS_VENDORMODEL_STATE_PARAMS     state_params;
    UINT16        marker;
    API_RESULT    retval;
    retval = API_SUCCESS;
    ext_params_p = NULL;
    marker = 0;
    /* Request Context */
    req_context.handle = *handle;
    req_context.saddr  = saddr;
    req_context.daddr  = daddr;
    req_context.subnet_handle = subnet_handle;
    req_context.appkey_handle = appkey_handle;
    /* Request Raw */
    req_raw.opcode = opcode;
    req_raw.data_param = data_param;
    req_raw.data_len = data_len;
    state_params.vendormodel_param = NULL;

    switch(opcode)
    {
    case MS_ACCESS_VENDORMODEL_GET_OPCODE:
    {
//            printf(
//            "MS_ACCESS_PHY_MODEL_GET_OPCODE\n");
        MODEL_OPCODE_HANDLER_CALL(vendor_example_get_handler);
        marker = 1;
        MS_UNPACK_LE_2_BYTE(&state_params.vendormodel_type, data_param+marker);
        marker += 2;
        /* Get Request Type */
        req_type.type = MS_ACCESS_MODEL_REQ_MSG_T_GET;
        req_type.to_be_acked = 0x01;
        /* Assign reqeusted state type to the application */
    }
    break;

    case MS_ACCESS_VENDORMODEL_SET_OPCODE:
    case MS_ACCESS_VENDORMODEL_SET_UNACKNOWLEDGED_OPCODE:
    {
//            printf(
//            "MS_ACCESS_PHY_MODEL_SET_OPCODE\n");
        MODEL_OPCODE_HANDLER_CALL(vendor_example_set_handler);
        marker = 1;
        MS_UNPACK_LE_2_BYTE(&state_params.vendormodel_type, data_param+marker);
        marker += 2;
        state_params.vendormodel_param = &data_param[marker];
        /* Set Request Type */
        req_type.type = MS_ACCESS_MODEL_REQ_MSG_T_SET;

        if(MS_ACCESS_VENDORMODEL_SET_OPCODE == opcode)
        {
            req_type.to_be_acked = 0x01;
        }
        else
        {
            req_type.to_be_acked = 0x00;
        }
    }
    break;

    case MS_ACCESS_VENDORMODEL_STATUS_OPCODE:
    {
//            printf(
//            "MS_ACCESS_PHY_MODEL_STATUS\n");
        MODEL_OPCODE_HANDLER_CALL(vendor_example_status_handler);
        /* Set Request Type */
        req_type.type = MS_ACCESS_MODEL_REQ_MSG_T_OTHERS;
        req_type.to_be_acked = 0x00;
    }
    break;

    case MS_ACCESS_VENDORMODEL_CONFIRMATION_OPCODE:
    {
//            printf(
//            "MS_ACCESS_PHY_MODEL_CONFIRMATION\n");
        MODEL_OPCODE_HANDLER_CALL(vendor_example_confirmation_handler);
        /* Set Request Type */
        req_type.type = MS_ACCESS_MODEL_REQ_MSG_T_OTHERS;
        req_type.to_be_acked = 0x00;
    }
    break;

    case MS_ACCESS_VENDORMODEL_WRITECMD_OPCODE:
    {
//            printf(
//            "MS_ACCESS_PHY_MODEL_WRITECMD_OPCODE\n");
        marker = 1;
        MS_UNPACK_LE_2_BYTE(&state_params.vendormodel_type, data_param+marker);
        marker += 2;
        state_params.vendormodel_param = &data_param[marker];
        /* Set Request Type */
        req_type.type = MS_ACCESS_MODEL_REQ_MSG_T_OTHERS;
        req_type.to_be_acked = 0x00;
    }
    break;

    case MS_ACCESS_VENDORMODEL_NOTIFY_OPCODE:
    {
//        printf(
//            "MS_ACCESS_PHY_MODEL_NOTIFY_OPCODE\n");
        state_params.vendormodel_type = MS_STATE_VENDORMODEL_NOTIFY_T;
        marker = 1;
        state_params.vendormodel_param = &data_param[marker];
        /* Set Request Type */
        req_type.type = MS_ACCESS_MODEL_REQ_MSG_T_OTHERS;
        req_type.to_be_acked = 0x00;
    }
    break;

    default:
        printf(
            "MS_ACCESS_VENDORMODEL_NONE_OPCODE\n");
        break;
    }

    /* Application callback */
    if (NULL != vendormodel_server_UI_cb)
    {
        vendormodel_server_UI_cb(&req_context, &req_raw, &req_type, &state_params, ext_params_p);
    }

    return retval;
}



/**
    \brief API to initialize Vendor_Example_1 Server model

    \par Description
    This is to initialize Vendor_Example_1 Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] UI_cb    Application Callback to be used by the Vendor_Example_1 Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
__ATTR_SECTION_XIP__ API_RESULT MS_vendormodel_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*       model_handle,
    /* IN */    MS_VENDORMODEL_SERVER_CB      UI_cb
)
{
    API_RESULT retval;
    MS_ACCESS_NODE_ID        node_id;
    MS_ACCESS_MODEL          model;
    /* TBD: Initialize MUTEX and other data structures */
    /* Using default node ID */
    node_id = MS_ACCESS_DEFAULT_NODE_ID;
    /* Configure Model */
    model.model_id.id = MS_MODEL_ID_VENDORMODEL_SERVER;
    model.model_id.type = MS_ACCESS_MODEL_TYPE_VENDOR;
    model.elem_handle = element_handle;
    /* Register Callback */
    model.cb = vendormodel_server_cb;
    /* List of Opcodes */
    model.opcodes = vendormodel_server_opcode_list;
    model.num_opcodes = sizeof(vendormodel_server_opcode_list) / sizeof(UINT32);
    retval = MS_access_register_model
             (
                 node_id,
                 &model,
                 model_handle
             );
    /* Save Application Callback */
    vendormodel_server_UI_cb = UI_cb;
    //    /* TODO: Remove */
//    phy_model_server_model_handle = *model_handle;
    return retval;
}


