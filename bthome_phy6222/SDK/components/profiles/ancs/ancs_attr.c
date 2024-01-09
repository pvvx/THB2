/**************************************************************************************************
 SDK_LICENSE
**************************************************************************************************/


#include "bcomdef.h"
#include "linkdb.h"
#include "osal.h"
#include "gatt.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "peripheral.h"
#include "gapbondmgr.h"
#include "gatt_profile_uuid.h"
#include "ble_ancs.h"
#include "ancs_attr.h"
#include "log.h"




static bool all_req_attrs_parsed(ancs_ctx_t* p_ancs)
{
    if (p_ancs->parse_info.expected_number_of_attrs == 0)
    {
        return TRUE;
    }

    return FALSE;
}

static bool attr_is_requested(ancs_ctx_t* p_ancs, ancs_attr_evt_t* pattr)
{
    if(p_ancs->parse_info.p_attr_list[pattr->attr_id].en == TRUE)
    {
        return TRUE;
    }

    return FALSE;
}


static ancs_parse_state_t command_id_parse(ancs_ctx_t* p_ancs, const uint8_t* p_data_src, uint32_t* index)
{
    ancs_parse_state_t parse_state;
    p_ancs->parse_info.command_id = (ancs_cmd_id_val_t) p_data_src[(*index)++];
    p_ancs->attr_rsp_evt.msg = (void*)(&(p_ancs->attr_evt_msg));

    switch (p_ancs->parse_info.command_id)
    {
    case ANCS_COMMAND_ID_GET_NOTIF_ATTRIBUTES:
        p_ancs->attr_rsp_evt.type = BLE_ANCS_EVT_NOTIF_ATTRIBUTE;
        p_ancs->parse_info.p_attr_list  = p_ancs->notif_attr_list;
        p_ancs->parse_info.nb_of_attr   = ANCS_NB_OF_NOTIF_ATTR;
        parse_state                     = NOTIF_UID;
        break;

    case ANCS_COMMAND_ID_GET_APP_ATTRIBUTES:
        p_ancs->attr_rsp_evt.type = BLE_ANCS_EVT_APP_ATTRIBUTE;
        p_ancs->parse_info.p_attr_list  = p_ancs->app_attr_list;
        p_ancs->parse_info.nb_of_attr   = ANCS_NB_OF_APP_ATTR;
        parse_state                     = APP_ID;
        break;

    default:
        //no valid command_id, abort the rest of the parsing procedure.
        LOG("Invalid Command ID");
        parse_state = DONE;
        break;
    }

    return parse_state;
}


static ancs_parse_state_t notif_uid_parse(ancs_ctx_t*   p_ancs,
                                          const uint8_t* p_data_src,
                                          uint32_t*       index)
{
    p_ancs->attr_evt_msg.notif_uid = BUILD_UINT32(p_data_src[*index],p_data_src[*index+1],p_data_src[*index+2],p_data_src[*index+3]);
    *index               += sizeof(uint32_t);
    return ATTR_ID;
}

static ancs_parse_state_t app_id_parse(ancs_ctx_t*   p_ancs,
                                       const uint8_t* p_data_src,
                                       uint32_t*       index)
{
    p_ancs->attr_evt_msg.app_id[p_ancs->parse_info.current_app_id_index] = p_data_src[(*index)++];

    if(p_ancs->attr_evt_msg.app_id[p_ancs->parse_info.current_app_id_index] != '\0')
    {
        p_ancs->parse_info.current_app_id_index++;
        return APP_ID;
    }
    else
    {
        return ATTR_ID;
    }
}

static ancs_parse_state_t attr_id_parse(ancs_ctx_t*   p_ancs,
                                        const uint8_t* p_data_src,
                                        uint32_t*       index)
{
    p_ancs->attr_evt_msg.attr_id     = p_data_src[(*index)++];

    if (p_ancs->attr_evt_msg.attr_id >= p_ancs->parse_info.nb_of_attr)
    {
        LOG("Attribute ID Invalid.\r\n");
        return DONE;
    }

    p_ancs->attr_evt_msg.p_attr_data = p_ancs->parse_info.p_attr_list[p_ancs->attr_evt_msg.attr_id].p_attr_data;

    if (all_req_attrs_parsed(p_ancs))
    {
        LOG("All requested attributes received. \r\n");
        return DONE;
    }
    else
    {
        if (attr_is_requested(p_ancs, &(p_ancs->attr_evt_msg)))
        {
            p_ancs->parse_info.expected_number_of_attrs--;
        }

        LOG("Attribute ID %i \r\n", p_ancs->attr_evt_msg.attr_id);
        return ATTR_LEN1;
    }
}


static ancs_parse_state_t attr_len1_parse(ancs_ctx_t* p_ancs, const uint8_t* p_data_src, uint32_t* index)
{
    p_ancs->attr_evt_msg.attr_len = p_data_src[(*index)++];
    return ATTR_LEN2;
}

static ancs_parse_state_t attr_len2_parse(ancs_ctx_t* p_ancs, const uint8_t* p_data_src, uint32_t* index)
{
    p_ancs->attr_evt_msg.attr_len |= (p_data_src[(*index)++] << 8);
    p_ancs->parse_info.current_attr_index = 0;

    if (p_ancs->attr_evt_msg.attr_len != 0)
    {
        //If the attribute has a length but there is no allocated space for this attribute
        if((p_ancs->parse_info.p_attr_list[p_ancs->attr_evt_msg.attr_id].attr_len == 0) ||
                (p_ancs->parse_info.p_attr_list[p_ancs->attr_evt_msg.attr_id].p_attr_data == NULL))
        {
            return ATTR_SKIP;
        }
        else
        {
            return ATTR_DATA;
        }
    }
    else
    {
        LOG("Attribute LEN %i \r\n", p_ancs->attr_evt_msg.attr_len);

        if(attr_is_requested(p_ancs, &(p_ancs->attr_evt_msg)))
        {
            p_ancs->callback(&p_ancs->attr_rsp_evt);
        }

        if(all_req_attrs_parsed(p_ancs))
        {
            return DONE;
        }
        else
        {
            return ATTR_ID;
        }
    }
}


static ancs_parse_state_t attr_data_parse(ancs_ctx_t*   p_ancs,
                                          const uint8_t* p_data_src,
                                          uint32_t*       index)
{
    // We have not reached the end of the attribute, nor our max allocated internal size.
    // Proceed with copying data over to our buffer.
    if (   (p_ancs->parse_info.current_attr_index < p_ancs->parse_info.p_attr_list[p_ancs->attr_evt_msg.attr_id].attr_len)
            && (p_ancs->parse_info.current_attr_index < p_ancs->attr_evt_msg.attr_len))
    {
        //LOG("Byte copied to buffer: %c\r\n", p_data_src[(*index)]); // Un-comment this line to see every byte of an attribute as it is parsed. Commented out by default since it can overflow the uart buffer.
        p_ancs->attr_evt_msg.p_attr_data[p_ancs->parse_info.current_attr_index++] = p_data_src[(*index)++];
    }

    // We have reached the end of the attribute, or our max allocated internal size.
    // Stop copying data over to our buffer. NUL-terminate at the current index.
    if ( (p_ancs->parse_info.current_attr_index == p_ancs->attr_evt_msg.attr_len) ||
            (p_ancs->parse_info.current_attr_index == p_ancs->parse_info.p_attr_list[p_ancs->attr_evt_msg.attr_id].attr_len - 1))
    {
        if (attr_is_requested(p_ancs, &(p_ancs->attr_evt_msg)))
        {
            p_ancs->attr_evt_msg.p_attr_data[p_ancs->parse_info.current_attr_index] = '\0';
        }

        // If our max buffer size is smaller than the remaining attribute data, we must
        // increase index to skip the data until the start of the next attribute.
        if (p_ancs->parse_info.current_attr_index < p_ancs->attr_evt_msg.attr_len)
        {
            return ATTR_SKIP;
        }

        LOG("Attribute finished!\r\n");

        if(attr_is_requested(p_ancs, &(p_ancs->attr_evt_msg)))
        {
            p_ancs->callback(&p_ancs->attr_rsp_evt);
        }

        if(all_req_attrs_parsed(p_ancs))
        {
            return DONE;
        }
        else
        {
            return ATTR_ID;
        }
    }

    return ATTR_DATA;
}


static ancs_parse_state_t attr_skip(ancs_ctx_t* p_ancs, const uint8_t* p_data_src, uint32_t* index)
{
    // We have not reached the end of the attribute, nor our max allocated internal size.
    // Proceed with copying data over to our buffer.
    if (p_ancs->parse_info.current_attr_index < p_ancs->attr_evt_msg.attr_len)
    {
        p_ancs->parse_info.current_attr_index++;
        (*index)++;
    }

    // At the end of the attribute, determine if it should be passed to event handler and
    // continue parsing the next attribute ID if we are not done with all the attributes.
    if (p_ancs->parse_info.current_attr_index == p_ancs->attr_evt_msg.attr_len)
    {
        if(attr_is_requested(p_ancs, &(p_ancs->attr_evt_msg)))
        {
            p_ancs->callback(&p_ancs->attr_rsp_evt);
        }

        if(all_req_attrs_parsed(p_ancs))
        {
            return DONE;
        }
        else
        {
            return ATTR_ID;
        }
    }

    return ATTR_SKIP;
}

static void print_hex (const uint8* data, uint16 len)
{
    uint16 i;

    for (i = 0; i < len - 1; i++)
    {
        LOG("%x,",data[i]);
        LOG(" ");
    }

    LOG("%d\n",data[i]);
}

void ancs_parse_get_attrs_response(ancs_ctx_t*   p_ancs, const uint8_t* p_data_src, uint8_t  hvx_data_len)
{
    uint32_t index;
    LOG("ancs_parse_get_attrs_response:\n");
    print_hex(p_data_src, hvx_data_len);

    for (index = 0; index < hvx_data_len;)
    {
        switch (p_ancs->parse_info.parse_state)
        {
        case COMMAND_ID:
            p_ancs->parse_info.parse_state = command_id_parse(p_ancs, p_data_src, &index);
            break;

        case NOTIF_UID:
            p_ancs->parse_info.parse_state = notif_uid_parse(p_ancs, p_data_src, &index);
            break;

        case APP_ID:
            p_ancs->parse_info.parse_state = app_id_parse(p_ancs, p_data_src, &index);
            break;

        case ATTR_ID:
            p_ancs->parse_info.parse_state = attr_id_parse(p_ancs, p_data_src, &index);
            break;

        case ATTR_LEN1:
            p_ancs->parse_info.parse_state = attr_len1_parse(p_ancs, p_data_src, &index);
            break;

        case ATTR_LEN2:
            p_ancs->parse_info.parse_state = attr_len2_parse(p_ancs, p_data_src, &index);
            break;

        case ATTR_DATA:
            p_ancs->parse_info.parse_state = attr_data_parse(p_ancs, p_data_src, &index);
            break;

        case ATTR_SKIP:
            p_ancs->parse_info.parse_state = attr_skip(p_ancs, p_data_src, &index);
            break;

        case DONE:
            LOG("Parse state: Done %s\r\n", p_ancs->attr_evt_msg.p_attr_data);
            index = hvx_data_len;
            break;

        default:
            // Default case will never trigger intentionally. Go to the DONE state to minimize the consequences.
            p_ancs->parse_info.parse_state = DONE;
            break;
        }
    }
}






static bool app_attr_is_requested(ancs_ctx_t* p_ancs, uint32_t attr_id)
{
    if(p_ancs->app_attr_list[attr_id].en == TRUE)
    {
        return TRUE;
    }

    return false;
}


static uint32_t app_attr_nb_to_get(ancs_ctx_t* p_ancs)
{
    uint32_t attr_nb_to_get = 0;

    for(uint32_t i = 0; i < (sizeof(p_ancs->app_attr_list)/sizeof(ancs_attr_list_t)); i++)
    {
        if(app_attr_is_requested(p_ancs,i))
        {
            attr_nb_to_get++;
        }
    }

    return attr_nb_to_get;
}



static encode_app_attr_t app_attr_encode_cmd_id(ancs_ctx_t*   p_ancs,
                                                uint32_t*       index,
                                                uint8_t*   tx_buf)
{
    LOG("Encoding Command ID\r\n");
    // Encode Command ID.
    tx_buf[(*index)++] = ANCS_COMMAND_ID_GET_APP_ATTRIBUTES;
    return APP_ATTR_APP_ID;
}


static encode_app_attr_t app_attr_encode_app_id(ancs_ctx_t*   p_ancs,
                                                uint32_t*       p_index,
                                                uint16_t*       p_offset,
                                                uint8_t*        tx_buf,
                                                const uint8_t* p_app_id,
                                                const uint32_t  app_id_len,
                                                uint32_t*       p_app_id_bytes_encoded_count)
{
    LOG("Encoding APP ID\r\n");

    //Encode App Identifier.
    if(*p_app_id_bytes_encoded_count == app_id_len)
    {
        tx_buf[(*p_index)++] = '\0';
        (*p_app_id_bytes_encoded_count)++;
    }

    LOG("%c\r\n", p_app_id[(*p_app_id_bytes_encoded_count)]);

    if(*p_app_id_bytes_encoded_count < app_id_len)
    {
        tx_buf[(*p_index)++] = p_app_id[(*p_app_id_bytes_encoded_count)++];
    }

    if(*p_app_id_bytes_encoded_count > app_id_len)
    {
        return APP_ATTR_ATTR_ID;
    }

    return APP_ATTR_APP_ID;
}


static encode_app_attr_t app_attr_encode_attr_id(ancs_ctx_t*   p_ancs,
                                                 uint32_t*        p_index,
                                                 uint16_t*        p_offset,
                                                 uint8_t*         tx_buf,
                                                 uint32_t*        p_attr_count,
                                                 uint32_t*        attr_get_total_nb)
{
    LOG("Encoding Attribute ID\r\n");

    //Encode Attribute ID.
    if (*p_attr_count < ANCS_NB_OF_APP_ATTR)
    {
        if (app_attr_is_requested(p_ancs, *p_attr_count))
        {
            tx_buf[(*p_index)] = *p_attr_count;
            (*p_index)++;
            LOG("offset %i\r\n", *p_offset);
        }

        (*p_attr_count)++;
    }

    if (*p_attr_count == ANCS_NB_OF_APP_ATTR)
    {
        return APP_ATTR_DONE;
    }

    return APP_ATTR_APP_ID;
}



bStatus_t app_attrs_get(ancs_ctx_t*   p_ancs, const uint8_t* p_app_id, uint8_t app_id_len)
{
    ancs_service_t* pservice = &(p_ancs->ancs_service);
    uint32_t          index                      = 0;
    uint32_t          attr_bytes_encoded_count   = 0;
    uint16_t          offset                     = 0;
    uint32_t          app_id_bytes_encoded_count = 0;
    encode_app_attr_t state                      = APP_ATTR_COMMAND_ID;
    bStatus_t         status;
    p_ancs->parse_info.parse_state = COMMAND_ID;
    uint32_t     attr_get_total_nb = app_attr_nb_to_get(p_ancs);
    uint8_t* tx_buf = p_ancs->app_attr_tx_buf;

    if(app_id_len == 0)
    {
        return INVALIDPARAMETER;
    }

    if(p_app_id[app_id_len] != '\0') // App id to be requestes must be NULL terminated
    {
        return INVALIDPARAMETER;
    }

    osal_memset(tx_buf, 0, sizeof(ANCS_APP_ATTR_TX_SIZE));

    while(state != APP_ATTR_DONE)
    {
        switch(state)
        {
        case APP_ATTR_COMMAND_ID:
            state = app_attr_encode_cmd_id(p_ancs,
                                           &index,
                                           tx_buf);
            break;

        case APP_ATTR_APP_ID:
            state = app_attr_encode_app_id(p_ancs,
                                           &index,
                                           &offset,
                                           tx_buf,
                                           p_app_id,
                                           app_id_len,
                                           &app_id_bytes_encoded_count);
            break;

        case APP_ATTR_ATTR_ID:
            state = app_attr_encode_attr_id(p_ancs,
                                            &index,
                                            &offset,
                                            tx_buf,
                                            &attr_bytes_encoded_count,
                                            &attr_get_total_nb);
            break;

        case APP_ATTR_DONE:
            break;

        default:
            break;
        }
    }

    p_ancs->parse_info.expected_number_of_attrs = ANCS_NB_OF_APP_ATTR;

    if(index > 20)
    {
        gattPrepareWriteReq_t lreq;
        lreq.pValue = osal_mem_alloc(index);
        osal_memcpy(lreq.pValue, tx_buf, index);
        lreq.handle = pservice->chars_hdl[ANCS_CTRL_POINT_HDL_START];
        lreq.len = index;
        status = GATT_WriteLongCharValue(pservice->conn_hdl, &lreq, p_ancs->app_task_ID);

        if (status != SUCCESS)
        {
            // If it fails free the message.
            LOG("CP WRITE ERROR:\t%d\n",status);
            osal_mem_free(lreq.pValue);
        }
    }
    else
    {
        attWriteReq_t req;
        osal_memcpy(req.value, tx_buf, index);
        req.sig = 0;
        req.cmd = 0;
        req.handle = pservice->chars_hdl[ANCS_CTRL_POINT_HDL_START];
        req.len = index;
        status = GATT_WriteCharValue(pservice->conn_hdl, &req, p_ancs->app_task_ID);

        if (status != SUCCESS)
        {
            // If it fails free the message.
            LOG("CP WRITE ERROR:\t%d\n",status);
        }
    }

    return SUCCESS;
}



bStatus_t notif_attrs_get(ancs_ctx_t*   p_ancs,const uint8_t* pNotificationUID)
{
    ancs_service_t* pservice = &(p_ancs->ancs_service);
    ancs_attr_list_t* pattrlist = p_ancs->notif_attr_list;
    bStatus_t         status;
    uint8_t* tx_buf = p_ancs->app_attr_tx_buf;
    uint8_t number_of_requested_attr = 0;
    p_ancs->parse_info.parse_state = COMMAND_ID;
    osal_memset(tx_buf, 0, sizeof(ANCS_APP_ATTR_TX_SIZE));
    uint32_t index                   = 0;
    //Encode Command ID.
    tx_buf[index++] = ANCS_COMMAND_ID_GET_NOTIF_ATTRIBUTES;
    //Encode Notification UID.
    tx_buf[index++] = pNotificationUID[0];
    tx_buf[index++] = pNotificationUID[1];
    tx_buf[index++] = pNotificationUID[2];
    tx_buf[index++] = pNotificationUID[3];

    //Encode Attribute ID.
    for (uint32_t attr = 0; attr < ANCS_NB_OF_NOTIF_ATTR; attr++)
    {
        if (pattrlist[attr].en == TRUE)
        {
            tx_buf[index++] = attr;

            if ((attr == BLE_ANCS_NOTIF_ATTR_ID_TITLE) ||
                    (attr == BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE) ||
                    (attr == BLE_ANCS_NOTIF_ATTR_ID_MESSAGE))
            {
                //Encode Length field, only applicable for Title, Subtitle and Message
                tx_buf[index++] = (uint8_t)(pattrlist[attr].attr_len & 0xff);
                tx_buf[index++] = (uint8_t)((pattrlist[attr].attr_len >> 8)&0xff);
            }

            number_of_requested_attr++;
        }
    }

    LOG("notif_attrs_get:\n");
    print_hex(tx_buf, index);
    p_ancs->parse_info.expected_number_of_attrs = number_of_requested_attr;

    if(index > 20)
    {
        gattPrepareWriteReq_t lreq;
        lreq.pValue = osal_mem_alloc(index);
        osal_memcpy(lreq.pValue, tx_buf, index);
        lreq.handle = pservice->chars_hdl[ANCS_CTRL_POINT_HDL_START];
        lreq.len = index;
        status = GATT_WriteLongCharValue(pservice->conn_hdl, &lreq, p_ancs->app_task_ID);

        if (status != SUCCESS)
        {
            // If it fails free the message.
            LOG("CP WRITE ERROR:\t%d\n",status);
            osal_mem_free(lreq.pValue);
        }
    }
    else
    {
        attWriteReq_t req;
        osal_memcpy(req.value, tx_buf, index);
        req.sig = 0;
        req.cmd = 0;
        req.handle = pservice->chars_hdl[ANCS_CTRL_POINT_HDL_START];
        req.len = index;
        status = GATT_WriteCharValue(pservice->conn_hdl, &req, p_ancs->app_task_ID);

        if (status != SUCCESS)
        {
            // If it fails free the message.
            LOG("CP WRITE ERROR:\t%d\n",status);
        }
    }

    return SUCCESS;
}


