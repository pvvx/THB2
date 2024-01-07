/**
    \file phy_model_client.c

    \brief This file defines the Mesh Configuration Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/





/* --------------------------------------------- Header File Inclusion */
#include "vendormodel_client.h"

/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Global Definitions */





/* --------------------------------------------- Static Global Variables */

static DECL_CONST UINT32 vendormodel_client_opcode_list[] =
{
    MS_ACCESS_VENDORMODEL_STATUS_OPCODE,
    MS_ACCESS_VENDORMODEL_INDICATION_OPCODE,
    MS_ACCESS_VENDORMODEL_WRITECMD_OPCODE,
    MS_ACCESS_VENDORMODEL_NOTIFY_OPCODE
};

static MS_ACCESS_MODEL_HANDLE           vendormodel_client_model_handle;
static MS_VENDORMODEL_CLIENT_CB        vendormodel_client_UI_cb;




/* --------------------------------------------- External Global Variables */


/* --------------------------------------------- Function */

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
API_RESULT vendormodel_client_cb
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
    MS_ACCESS_VENDORMODEL_STATE_PARAMS        state_params;
    UINT16  marker = 0;
    API_RESULT    retval;
    retval = API_SUCCESS;
    ext_params_p = NULL;
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

//    printf(
//    "[PHY_MODEL_CLIENT] Callback. Opcode 0x%04X\n", opcode);

//    appl_dump_bytes(data_param, data_len);

    switch(opcode)
    {
    case MS_ACCESS_VENDORMODEL_STATUS_OPCODE:
    {
//            printf(
//            "MS_ACCESS_PHY_MODEL_STATUS_MSG\n");
        MODEL_OPCODE_HANDLER_CALL(vendor_example_transparent_msg_handler);
        /* Set Request Type */
        req_type.type = MS_ACCESS_MODEL_REQ_MSG_T_OTHERS;
        req_type.to_be_acked = 0x00;
    }
    break;

    case MS_ACCESS_VENDORMODEL_INDICATION_OPCODE:
    {
//            printf(
//            "MS_ACCESS_PHY_MODEL_INDICATION_OPCODE_MSG\n");
        marker = 1;
        MS_UNPACK_LE_2_BYTE(&state_params.vendormodel_type, data_param+marker);
        marker += 2;
        state_params.vendormodel_param = &data_param[marker];
        /* Set Request Type */
        req_type.type = MS_ACCESS_MODEL_REQ_MSG_T_OTHERS;
        req_type.to_be_acked = 0x01;
    }
    break;

    case MS_ACCESS_VENDORMODEL_WRITECMD_OPCODE:
    {
//            printf(
//            "MS_ACCESS_PHY_MODEL_WRITECMD_OPCODE\n");
//            UCHAR *_data_pram;
        marker = 1;
        MS_UNPACK_LE_2_BYTE(&state_params.vendormodel_type, data_param+marker);
        marker += 2;
//            if(data_len>3)
//            {
//                _data_pram =  EM_alloc_mem(data_len - 3);
//                EM_mem_copy(_data_pram, data_param+marker, data_len-3);
//            }
//            EM_mem_copy((UCHAR *)state_params.state,data_param+marker,data_len-3);
        state_params.vendormodel_param = &data_param[marker];
//            EM_free_mem(_data_pram);
        /* Set Request Type */
        req_type.type = MS_ACCESS_MODEL_REQ_MSG_T_OTHERS;
        req_type.to_be_acked = 0x00;
    }
    break;

    case MS_ACCESS_VENDORMODEL_NOTIFY_OPCODE:
    {
//            UCHAR *_data_pram;
        state_params.vendormodel_type = MS_STATE_VENDORMODEL_NOTIFY_T;
        marker = 1;
//            if(data_len > 1)
//            {
//                _data_pram =  EM_alloc_mem(data_len - 1);
//                EM_mem_copy(_data_pram, data_param+marker, data_len-1);
//            }
        state_params.vendormodel_param = &data_param[marker];
//            EM_free_mem(_data_pram);
        /* Set Request Type */
        req_type.type = MS_ACCESS_MODEL_REQ_MSG_T_OTHERS;
        req_type.to_be_acked = 0x00;
    }
    break;

    default:
//            printf(
//            "MS_ACCESS_PHY_MODEL_NONE_OPCODE\n");
        break;
    }

    /* Application callback */
    if (NULL != vendormodel_client_UI_cb)
    {
        vendormodel_client_UI_cb(&req_context, &req_raw, &req_type, &state_params, ext_params_p);
    }

    return retval;
}

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
API_RESULT MS_vendormodel_client_state_update
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
        opcode = MS_ACCESS_VENDORMODEL_CONFIRMATION_OPCODE;
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
    \brief API to send acknowledged commands

    \par Description
    This is to initialize sending acknowledged commands.

    \param [in] req_opcode    Request Opcode.
    \param [in] param         Parameter associated with Request Opcode.
    \param [in] rsp_opcode    Response Opcode.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_vendormodel_client_send_reliable_pdu
(
    /* IN */ UINT32    req_opcode,
    /* IN */ void*     param,
    /* IN */ MS_NET_ADDR dst_addr
)
{
    API_RESULT retval;
    /* TODO: Check what should be maximum length */
    UCHAR      buffer[256];
    UCHAR*     pdu_ptr;
    UINT16     marker;
    MS_APPKEY_HANDLE key_handle;
    retval = API_FAILURE;
    marker = 0;

//    printf(
//    "[VENDOR_MODEL_CLIENT] Send Reliable PDU. Req Opcode 0x%08X\n",
//    req_opcode);

    switch(req_opcode)
    {
    case MS_ACCESS_VENDORMODEL_GET_OPCODE:
    {
        MS_ACCESS_VENDORMODEL_STATE_PARAMS* param_p;
        buffer[marker] = ++vendor_tid;
        marker++;
        param_p = (MS_ACCESS_VENDORMODEL_STATE_PARAMS*) param;
        MS_PACK_LE_2_BYTE_VAL(&buffer[marker], param_p->vendormodel_type);
        marker += 2;
    }
    break;

    case MS_ACCESS_VENDORMODEL_SET_OPCODE:
    case MS_ACCESS_VENDORMODEL_SET_UNACKNOWLEDGED_OPCODE:
    {
        MS_ACCESS_VENDORMODEL_STATE_PARAMS* param_p;
        buffer[marker] = ++vendor_tid;
        marker++;
        param_p = (MS_ACCESS_VENDORMODEL_STATE_PARAMS*) param;
        MS_PACK_LE_2_BYTE_VAL(&buffer[marker], param_p->vendormodel_type);
        marker += 2;

        switch(param_p->vendormodel_type)
        {
        case MS_STATE_VENDORMODEL_ONOFF_T:
        {
            EM_mem_copy(&buffer[marker], param_p->vendormodel_param, 1);
            marker += 1;
        }
        break;

        case MS_STATE_VENDORMODEL_HSL_T:
        {
            EM_mem_copy(&buffer[marker], param_p->vendormodel_param, 6);
            marker += 6;
        }
        break;

        default :
            break;
        }
    }
    break;

    case MS_ACCESS_VENDORMODEL_NOTIFY_OPCODE:
    {
        UINT16 len;
        MS_ACCESS_VENDORMODEL_STATE_PARAMS* param_p;
        param_p = (MS_ACCESS_VENDORMODEL_STATE_PARAMS*) param;
        MS_IGNORE_UNUSED_PARAM(param_p->vendormodel_type);
        MS_UNPACK_LE_2_BYTE(&len, param_p->vendormodel_param);
        buffer[marker] = ++vendor_tid;
        marker ++;

        if(len)
        {
            EM_mem_copy(&buffer[marker], &param_p->vendormodel_param[2], len);
            marker += len;
        }
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

    MS_access_get_appkey_handle
    (
        &vendormodel_client_model_handle,
        &key_handle
    );
    retval = MS_access_raw_data
             (
                 &vendormodel_client_model_handle,
                 req_opcode,
                 dst_addr,
                 key_handle,
                 pdu_ptr,
                 marker,
                 MS_FALSE
             );
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
API_RESULT MS_vendormodel_client_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*       model_handle,
    /* IN */    MS_VENDORMODEL_CLIENT_CB      UI_cb
)
{
    API_RESULT retval;
    MS_ACCESS_NODE_ID        node_id;
    MS_ACCESS_MODEL          model;
    /* TBD: Initialize MUTEX and other data structures */
    /* Using default node ID */
    node_id = MS_ACCESS_DEFAULT_NODE_ID;
//    printf(
//        "[PHY_MODEL] Registered Element Handle 0x%02X\n", element_handle);
    /* Configure Model */
    model.model_id.id = MS_MODEL_ID_VENDORMODEL_CLIENT;
    model.model_id.type = MS_ACCESS_MODEL_TYPE_VENDOR;
    model.elem_handle = element_handle;
    /* Register Callback */
    model.cb = vendormodel_client_cb;
    /* List of Opcodes */
    model.opcodes = vendormodel_client_opcode_list;
    model.num_opcodes = sizeof(vendormodel_client_opcode_list) / sizeof(UINT32);
    retval = MS_access_register_model
             (
                 node_id,
                 &model,
                 model_handle
             );
    /* Save Application Callback */
    vendormodel_client_UI_cb = UI_cb;
    vendormodel_client_model_handle = *model_handle;
    return retval;
}



