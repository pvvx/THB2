/**************************************************************************************************
 SDK_LICENSE
**************************************************************************************************/


#include "bcomdef.h"
#include "linkdb.h"
#include "types.h"
#include "gatt.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "gatt_uuid.h"
#include "peripheral.h"
#include "gapbondmgr.h"
#include "gatt_profile_uuid.h"
#include "ble_ancs.h"
#include "ancs_attr.h"
#include "log.h"


static uint8_t Ancs_CCCDConfig(uint16_t attrHdl, uint8_t isEnable);

static ancs_ctx_t s_ancs_ctx;

bStatus_t ble_ancs_attr_add(const ancs_notif_attr_id id, uint8_t* p_data, const uint16_t len)
{
    ancs_attr_list_t* p_ancs_notif_attr_list = s_ancs_ctx.notif_attr_list;

    if(!p_data)
        return INVALIDPARAMETER;

    if((len == 0) || (len > ANCS_ATTR_DATA_MAX))
    {
        return INVALID_MEM_SIZE;
    }

    p_ancs_notif_attr_list[id].en         = true;
    p_ancs_notif_attr_list[id].attr_len    = len;
    p_ancs_notif_attr_list[id].p_attr_data = p_data;
    return SUCCESS;
}

static uint8_t figure_out_chars_end_hdl(void)
{
    uint8_t errorcode = 0;
    ancs_ctx_t* pctx = &s_ancs_ctx;
    ancs_service_t* pservie = &(s_ancs_ctx.ancs_service);
    uint16_t* p_chars_hdl = pservie->chars_hdl;
    uint16_t ntf, cp, dt;
    ntf = p_chars_hdl[ANCS_NOTIF_SCR_HDL_START];
    cp = p_chars_hdl[ANCS_CTRL_POINT_HDL_START];
    dt = p_chars_hdl[ANCS_DATA_SRC_HDL_START];

    if(ntf < ((cp < dt) ? cp :dt))
        p_chars_hdl[ANCS_NOTIF_SCR_HDL_END] = cp > dt ? dt : cp;
    else if(ntf < ((cp < dt) ? dt :cp))
        p_chars_hdl[ANCS_NOTIF_SCR_HDL_END] = cp > dt ? cp : dt;
    else
        p_chars_hdl[ANCS_NOTIF_SCR_HDL_END] = pservie->service_hdl[1];

    if(cp < ((ntf < dt) ? ntf:dt))
        p_chars_hdl[ANCS_CTRL_POINT_HDL_END] = ntf > dt ? dt : ntf;
    else if(cp < ((ntf < dt) ? dt :ntf))
        p_chars_hdl[ANCS_CTRL_POINT_HDL_END] = ntf > dt ? ntf : dt;
    else
        p_chars_hdl[ANCS_CTRL_POINT_HDL_END] = pservie->service_hdl[1];

    if(dt < ((ntf < cp) ? ntf :cp))
        p_chars_hdl[ANCS_DATA_SRC_HDL_END] = ntf > cp ? cp : ntf;
    else if(dt < ((ntf < cp) ? cp :ntf))
        p_chars_hdl[ANCS_DATA_SRC_HDL_END] = ntf > cp ? ntf : cp;
    else
        p_chars_hdl[ANCS_DATA_SRC_HDL_END] = pservie->service_hdl[1];

    // Sanity check to ensure that each start handle is valid and
    // less than each respective end handle.
    if(p_chars_hdl[ANCS_NOTIF_SCR_HDL_START]  != 0 &&
            p_chars_hdl[ANCS_CTRL_POINT_HDL_START] != 0 &&
            p_chars_hdl[ANCS_DATA_SRC_HDL_START]   != 0)
    {
        if(p_chars_hdl[ANCS_NOTIF_SCR_HDL_START]  < p_chars_hdl[ANCS_NOTIF_SCR_HDL_END]  &&
                p_chars_hdl[ANCS_CTRL_POINT_HDL_START] < p_chars_hdl[ANCS_CTRL_POINT_HDL_END] &&
                p_chars_hdl[ANCS_DATA_SRC_HDL_START]   < p_chars_hdl[ANCS_DATA_SRC_HDL_END])
        {
            LOG("All chars discoveried\n");
        }
        else
        {
            LOG("ANCS_STORE_CHARS_HANDLES FAILURE\n");
            pctx->disc_state = ANCS_DISC_FAILED;
            errorcode = 4;
        }
    }
    // Throw an error if the handles are invalid.
    else
    {
        LOG("ANCS_STORE_CHARS_HANDLES FAILURE\n");
        pctx->disc_state = ANCS_DISC_FAILED;
        errorcode = 5;
    }

    return errorcode;
}


static bStatus_t ble_disc_service(gattMsgEvent_t* pMsg)
{
    ancs_ctx_t* pctx = &s_ancs_ctx;
    ancs_service_t* pservie = &(pctx->ancs_service);
    // Stores the error code, should the discovery process fail at any state.
    uint8_t errorcode = 0;

    //if(pMsg){
    //  LOG("ble_disc_service->pMsg->method: 0x%x\n",pMsg->method);
    //}
    // Enter the state machine.
    switch (pctx->disc_state)
    {
    case ANCS_UNINIT:
        LOG("Discovery Progress:\t1\n");
        return FAILURE;

    // Perform a GATT Discover Primary Service By Service UUID to located the ANCS
    // handles.
    case ANCS_DISC_SERVICE:
    {
        LOG("Discovery Progress:\t2\n");
        LOG("Discovery State:\tDiscover the ANCS\n");
        // Initialize the ANCS handles to zero.
        pservie->service_hdl[0] = 0;
        pservie->service_hdl[1] = 0;
        // Store the ANCS UUID for GATT request.
        uint8_t uuid[ATT_UUID_SIZE] = {ANCSAPP_ANCS_SVC_UUID};
        // Discover the ANCS by UUID.
        bStatus_t ret = GATT_DiscPrimaryServiceByUUID(pservie->conn_hdl, uuid, ATT_UUID_SIZE, pctx->app_task_ID);

        // If successfully discovered proceed, throw error if not.
        if(ret == SUCCESS)
        {
            pctx->disc_state = ANCS_STORE_SERVICE_HANDLES;
            pservie->expect_type_value_num = 0;
            //ancsAppState = ANCS_STATE_READY;
        }
        else
        {
            LOG("ANCS_DISC_SERVICE FAILURE, Error code:\t%d\n", ret);
            pctx->disc_state = ANCS_DISC_FAILED;
            errorcode = 1;
        }
    }
    break;

    // Store the ANCS handles requested in the previous state.
    case ANCS_STORE_SERVICE_HANDLES:
    {
        LOG("Discovery Progress:\t3\n");
        LOG("Discovery State:\tStore the ANCS handles %d\n",pMsg->method);

        // Did the application receive a response from the GATT Disc Primary Service?
        if (pMsg->method == ATT_FIND_BY_TYPE_VALUE_RSP )
        {
            LOG("found :%d\n",pMsg->msg.findByTypeValueRsp.numInfo);

            // Check if the ANCS was found.
            if (pMsg->msg.findByTypeValueRsp.numInfo > 0)
            {
                // Found the ANCS, so store the handles and proceed.
                pservie->service_hdl[0]  = pMsg->msg.findByTypeValueRsp.handlesInfo[0].handle;
                pservie->service_hdl[1]  = pMsg->msg.findByTypeValueRsp.handlesInfo[0].grpEndHandle;
                pctx->disc_state = ANCS_DISC_CHARS;
                //for(i = 0; i< pMsg->msg.findByTypeValueRsp.numInfo; i++){
                //  pservie->service_hdl[pservie->expect_type_value_num]  = pMsg->msg.findByTypeValueRsp.handlesInfo[i].handle;
                //  pservie->expect_type_value_num ++;
                //  if(pservie->expect_type_value_num == 2){
                //    pctx->disc_state = ANCS_DISC_CHARS;
                //    break;
                //  }
                //}
            }
            else
            {
                // The ANCS was not found.
                LOG("ANCS_STORE_SERVICE_HANDLES FAILURE\n");
                pctx->disc_state = ANCS_DISC_FAILED;
                errorcode = 2;
            }
        }
        else
        {
            LOG("ble_disc_service->pMsg->method: 0x%x\n",pMsg->method);
            LOG("ANCS_STORE_SERVICE_HANDLES FAILURE\n");
            pctx->disc_state = ANCS_DISC_FAILED;
            errorcode = 2;
        }
    }
    break;

    // Use the ANCS handles to discovery the ANCS's characteristics' handles.
    case ANCS_DISC_CHARS:
    {
        LOG("Discovery Progress:\t4\n");
        LOG("Discovery State:\tDiscover the ANCS characteristics\n");

        // Check if service handle discovery event has completed.
        if (pMsg->method == ATT_FIND_BY_TYPE_VALUE_RSP )
        {
            LOG("ble_disc_service->pMsg->method: 0x%x\n",pMsg->method);

            if(pMsg->hdr.status == bleProcedureComplete)
            {
                // Sanity check to make sure the handle is valid before proceeding.
                if (pservie->service_hdl[0] != 0 && pservie->service_hdl[1] != 0 )
                {
                    // Discover all characteristics of the ANCS.
                    bStatus_t ret = GATT_DiscAllChars(
                                        pservie->conn_hdl,
                                        pservie->service_hdl[0],
                                        pservie->service_hdl[1],
                                        pctx->app_task_ID);
                    pservie->chars_disc_num = 0;

                    // If the request was successfully sent, proceed with the discovery process.
                    if (ret == SUCCESS)
                    {
                        pctx->disc_state = ANCS_STORE_CHARS_HANDLES;
                    }
                    // If not, throw an error.
                    else
                    {
                        LOG("ANCS_DISC_CHARS FAILURE, Error code:\t%d\n",ret);
                        pctx->disc_state = ANCS_DISC_FAILED;
                        errorcode = 3;
                    }
                }
            }
        }
        else
        {
            LOG("ble_disc_service->pMsg->method: 0x%x\n",pMsg->method);
        }
    }
    break;

    // Store the retrieved ANCS characteristic handles.
    case ANCS_STORE_CHARS_HANDLES:
    {
        LOG("Discovery Progress:\t5\n");
        LOG("Discovery State:\tStore the ANCS characteristics' handles\n");

        // Wait until GATT "Read by type response" is received, then confirm that the correct number of
        // pairs are present, and that their length is correct
        if (pMsg->method == ATT_READ_BY_TYPE_RSP )
        {
            //if ( (pMsg->msg.readByTypeRsp.numPairs == NUMBER_OF_ANCS_CHARS) && (pMsg->msg.readByTypeRsp.len == CHAR_DESC_HDL_UUID128_LEN) )
            if ((pMsg->msg.readByTypeRsp.len == CHAR_DESC_HDL_UUID128_LEN) )
            {
                // Pointer to the pair list data in the GATT response.
                uint8_t*   pCharPairList;
                // Will store the start and end handles of the current pair.
                uint16_t  charStartHandle;
                // Stores to the UUID of the current pair.
                uint16_t  charUuid;
                // Stores what pair the loop is currently processing.
                uint8_t   currentCharIndex;
                // Set the pair pointer to the first pair.
                pCharPairList = pMsg->msg.readByTypeRsp.dataList;

                // Iterate through all three pairs found.
                for(currentCharIndex = 0; currentCharIndex < pMsg->msg.readByTypeRsp.numPairs ; currentCharIndex++)
                {
                    uint16_t* p_chars_hdl = pservie->chars_hdl;
                    // Extract the starting handle, ending handle, and UUID of the current characteristic.
                    charStartHandle = BUILD_UINT16(pCharPairList[3], pCharPairList[4]);
                    charUuid        = BUILD_UINT16(pCharPairList[5], pCharPairList[6]);
                    LOG("Chars found handle is is %d, uuid is %x\n", charStartHandle, charUuid);

                    // Store the start and end handles in the handle cache corresponding to
                    // their UUID.
                    switch (charUuid)
                    {
                    // If it's the Notification Source.
                    case ANCSAPP_NOTIF_SRC_CHAR_UUID:
                        p_chars_hdl[ANCS_NOTIF_SCR_HDL_START] = charStartHandle;
                        p_chars_hdl[ANCS_NOTIF_SCR_HDL_END]   = 0;
                        break;

                    // If it's the Control Point.
                    case ANCSAPP_CTRL_PT_CHAR_UUID:
                        p_chars_hdl[ANCS_CTRL_POINT_HDL_START] = charStartHandle;
                        p_chars_hdl[ANCS_CTRL_POINT_HDL_END]   = 0;
                        break;

                    // If it's the Data Source.
                    case ANCSAPP_DATA_SRC_CHAR_UUID:
                        p_chars_hdl[ANCS_DATA_SRC_HDL_START] = charStartHandle;
                        p_chars_hdl[ANCS_DATA_SRC_HDL_END]   = 0;
                        break;

                    default:
                        break;
                    }

                    pservie->chars_disc_num ++;

                    // If this is the final characteristic found in the response,
                    // reset its end handle to the ANCS's end handle. This is because
                    // there is no next staring handle to use as a reference and subtract one
                    // from, so instead the ending handle of the ANCS must be used.
                    if(pservie->chars_disc_num == NUMBER_OF_ANCS_CHARS)
                    {
                        errorcode = figure_out_chars_end_hdl();
                        pctx->disc_state = (errorcode == 0) ? ANCS_DISC_NS_DESCS : ANCS_DISC_FAILED;
                    }

                    // Increment the pair pointer to the next pair.
                    pCharPairList += CHAR_DESC_HDL_UUID128_LEN;
                }
            }
            // Throw an error if the length or number of pairs is incorrect.
            else
            {
                LOG("ANCS_STORE_CHARS_HANDLES FAILURE\n");
                pctx->disc_state = ANCS_DISC_FAILED;
                errorcode = 6;
            }
        }
        else
        {
            LOG("ble_disc_service->pMsg->method: 0x%x\n",pMsg->method);

            if(pMsg->method == ATT_ERROR_RSP)
            {
                attErrorRsp_t* perr = &(pMsg->msg.errorRsp);
                LOG("ATT_ERROR_RSP 0x%x, 0x%x, 0x%x\n", perr->errCode, perr->handle, perr->reqOpcode);
            }
        }
    }
    break;

    // Discover the Notification Source's descriptors (namely, the CCCD) using the start
    // and end handle stored in the handle cache.
    case ANCS_DISC_NS_DESCS:
    {
        LOG("Discovery Progress:\t6\n");
        LOG("Discovery State:\tDiscover the Notification Source's CCCD\n");

        // Wait until the characteristic handle discovery has finished.
        if ( (pMsg->method == ATT_READ_BY_TYPE_RSP) && (pMsg->hdr.status == bleProcedureComplete) )
        {
            // Discover the ANCS Notification Source descriptors.
            bStatus_t ret = GATT_DiscAllCharDescs(pservie->conn_hdl,
                                                  pservie->chars_hdl[ANCS_NOTIF_SCR_HDL_START],
                                                  pservie->chars_hdl[ANCS_NOTIF_SCR_HDL_END]-1,
                                                  pctx->app_task_ID);

            // If the discovery was successful, proceed.
            if ( ret == SUCCESS )
                pctx->disc_state = ANCS_STORE_NS_DESCS_HANDLES;
            // If not, throw an error and invalidate the CCCD handle in the handle cache.
            else
            {
                LOG("ANCS_DISC_NS_DESCS FAILURE\n");
                pservie->chars_hdl[ANCS_NOTIF_SCR_HDL_START] = 0;
                pctx->disc_state = ANCS_DISC_FAILED;
                errorcode = 7;
            }
        }
    }
    break;

    // Store the retrieved Notification Source descriptors (namely, the CCCD).
    case ANCS_STORE_NS_DESCS_HANDLES:
    {
        LOG("Discovery Progress:\t7\n");
        LOG("Discovery State:\tStore the Notification Source's CCCD handle\n");

        // Wait for the discovery response.
        if (pMsg->method == ATT_FIND_INFO_RSP )
        {
            // Sanity check to validate that at least one descriptors pair was found,
            // and that the pair length is correct.
            if ( (pMsg->msg.findInfoRsp.numInfo > 0) &&
                    (pMsg->msg.findInfoRsp.format == ATT_HANDLE_BT_UUID_TYPE) )
            {
                // This will keep track of the current pair being processed.
                uint8_t currentPair;

                // Iterate through the pair list.
                for(currentPair = 0; currentPair < pMsg->msg.findInfoRsp.numInfo; currentPair++)
                {
                    // Check if the pair is a CCCD.
                    uint16_t uuid = BUILD_UINT16(pMsg->msg.findInfoRsp.info.btPair[currentPair].uuid[0], pMsg->msg.findInfoRsp.info.btPair[currentPair].uuid[1]);

                    if (uuid == GATT_CLIENT_CHAR_CFG_UUID)
                    {
                        // If so, store the handle in the handle cache, and proceed.
                        pservie->chars_hdl[ANCS_NOTIF_SCR_HDL_CCCD] = pMsg->msg.findInfoRsp.info.btPair[currentPair].handle;
                        pctx->disc_state = ANCS_DISC_DS_DESCS;
                    }
                }
            }
        }
    }
    break;

    // Discover the Data Source's descriptors (namely, the CCCD) using the start
    // and end handle stored in the handle cache.
    case ANCS_DISC_DS_DESCS:
    {
        LOG("Discovery Progress:\t8\n");
        LOG("Discovery State:\tDiscover the Data Source's CCCD\n");

        // Wait until the Notification Source descriptors discovery has finished.
        if ( (pMsg->method == ATT_FIND_INFO_RSP) && (pMsg->hdr.status == bleProcedureComplete) )
        {
            // Discover ANCS Notification Source CCCD
            uint8_t discCheck = GATT_DiscAllCharDescs(pservie->conn_hdl,
                                                      pservie->chars_hdl[ANCS_DATA_SRC_HDL_START] + 1,
                                                      pservie->chars_hdl[ANCS_DATA_SRC_HDL_END],
                                                      pctx->app_task_ID);

            // If the discovery was successful, proceed.
            if (discCheck == SUCCESS )
                pctx->disc_state = ANCS_STORE_DS_DESCS_HANDLES;
            // If not, throw an error and invalidate the CCCD handle in the handle cache.
            else
            {
                LOG("ANCS_DISC_DS_DESCS FAILURE\n");
                pservie->chars_hdl[ANCS_DATA_SRC_HDL_CCCD] = 0;
                pctx->disc_state = ANCS_DISC_FAILED;
                errorcode = 8;
            }
        }
    }
    break;

    // Discover the Data Source's descriptors (namely, the CCCD) using the start
    // and end handle stored in the handle cache.
    case ANCS_STORE_DS_DESCS_HANDLES:
    {
        LOG("Discovery Progress:\t9\n");
        LOG("Discovery State:\tStore the Data Source's CCCD handle\n");

        // Wait for the discovery response.
        if (pMsg->method == ATT_FIND_INFO_RSP )
        {
            bool flg = FALSE;

            // Sanity check to validate that at least one descriptors pair was found,
            // and that the pair length is correct.
            if ( (pMsg->msg.findInfoRsp.numInfo > 0) && (pMsg->msg.findInfoRsp.format == ATT_HANDLE_BT_UUID_TYPE) )
            {
                // This will keep track of the current pair being processed.
                uint8_t currentPair;

                // Iterate through the pair list.
                for(currentPair = 0; currentPair < pMsg->msg.findInfoRsp.numInfo; currentPair++)
                {
                    // Check if the pair is a CCCD.
                    uint16_t uuid = BUILD_UINT16(pMsg->msg.findInfoRsp.info.btPair[currentPair].uuid[0], pMsg->msg.findInfoRsp.info.btPair[currentPair].uuid[1]);

                    if (uuid == GATT_CLIENT_CHAR_CFG_UUID)
                    {
                        // If so, store the handle in the handle cache, and proceed to the subscription process.
                        pservie->chars_hdl[ANCS_DATA_SRC_HDL_CCCD] = pMsg->msg.findInfoRsp.info.btPair[currentPair].handle;
                        flg = TRUE;
                    }
                }

                if(flg == FALSE)
                {
                    LOG("ANCS_STORE_DS_DESCS_HANDLES FAILURE\n");
                    pservie->chars_hdl[ANCS_DATA_SRC_HDL_CCCD] = 0;
                    pctx->disc_state = ANCS_DISC_FAILED;
                    errorcode = 9;
                }
                else
                {
                    pctx->disc_state = ANCS_ENABLE_NS_CCCD;
                }
            }
        }
    }
    break;

    case ANCS_ENABLE_NS_CCCD:
    {
        bStatus_t ret;

        if (pMsg->method == ATT_FIND_INFO_RSP && (pMsg->hdr.status == bleProcedureComplete))
        {
            ret = Ancs_CCCDConfig(pservie->chars_hdl[ANCS_NOTIF_SCR_HDL_CCCD], TRUE);

            if(ret != SUCCESS)
            {
                LOG("ANCS_DATA_SRC_HDL_CCCD FAILURE\n");
                pservie->chars_hdl[ANCS_DATA_SRC_HDL_CCCD] = 0;
                pctx->disc_state = ANCS_DISC_FAILED;
                errorcode = 10;
                break;
            }

            pctx->disc_state = ANCS_ENABLE_DS_CCCD;
        }
    }

    case ANCS_ENABLE_DS_CCCD:
    {
        bStatus_t ret;

        if (pMsg->method == ATT_WRITE_RSP )
        {
            ret = Ancs_CCCDConfig(pservie->chars_hdl[ANCS_DATA_SRC_HDL_CCCD], TRUE);

            if(ret != SUCCESS)
            {
                LOG("ANCS_DATA_SRC_HDL_CCCD FAILURE\n");
                pservie->chars_hdl[ANCS_DATA_SRC_HDL_CCCD] = 0;
                pctx->disc_state = ANCS_DISC_FAILED;
                errorcode = 10;
                break;
            }

            pctx->disc_state = ANCS_DISC_FINISH;
            pctx->app_state = ANCS_STATE_READY;
            //pctx->disc_state = ANCS_WAIT_CCCD_READY;
        }
    }
    break;

    case ANCS_WAIT_CCCD_READY:
        if (pMsg->method == ATT_WRITE_RSP )
        {
            pctx->disc_state = ANCS_DISC_FINISH;
            pctx->app_state = ANCS_STATE_READY;
            LOG("Discovery Progress:\t12\n");
            LOG("Discovery State:\tProcessing notification data\n");
            break;
        }

    default:
    {
        pctx->disc_state = ANCS_DISC_FAILED;
        errorcode = 11;
    }
    break;
    }

    if(errorcode != 0)
    {
        LOG("Discovery State:\tDiscovery Error: %d\n",errorcode);
    }

    return errorcode;
}

static uint8_t Ancs_CCCDConfig(uint16_t attrHdl, uint8_t isEnable)
{
    ancs_ctx_t* pctx = &s_ancs_ctx;
    // Declare return variable status.
    uint8_t status;
    // Stores the GATT write request parameters.
    attWriteReq_t req;
    LOG("Ancs_CCCDConfig handle %d\n", attrHdl);
    // Else, prepare the request.
    // Set the data length to 2 ("01" = 2 bytes).
    req.len = 2;

    // If we are enabling notifications, set the write data to "01".
    if (isEnable == TRUE)
    {
        req.value[0] = LO_UINT16(GATT_CLIENT_CFG_NOTIFY);
        req.value[1] = HI_UINT16(GATT_CLIENT_CFG_NOTIFY);
    }
    // Else, disable notifications, thus set the write data to "00".
    else
    {
        req.value[0] = 0x00;
        req.value[1] = 0x00;
    }

    // Signature and command must be set to zero.
    req.sig = 0;
    req.cmd = 0;
    // Set the handle to the passed value (either the Notification Source's CCCD handle
    // or the Data Source's CCCD handle).
    req.handle = attrHdl;
    // Send write request. If it fails, free the memory allocated and
    // return a failure.
    status = GATT_WriteCharValue(pctx->ancs_service.conn_hdl, &req, pctx->app_task_ID);

    if ( status != SUCCESS)
    {
        LOG("Ancs_CCCDConfig %d\n", status);
    }

    return status;
}

bStatus_t ble_ancs_start_descovery(uint16_t conn_handle)
{
    ancs_ctx_t* pctx = &s_ancs_ctx;
    ancs_service_t* pservie = &(pctx->ancs_service);
    pservie->conn_hdl = conn_handle;
    pctx->disc_state = ANCS_DISC_SERVICE;
    pctx->app_state = ANCS_STATE_DISCOVERY;
    ble_disc_service(NULL);
    return SUCCESS;
}
static void ble_ancs_process_ds_notify(gattMsgEvent_t* pMsg)
{
    ancs_ctx_t* pctx = &s_ancs_ctx;
    ancs_parse_get_attrs_response(pctx, (const uint8_t*)pMsg->msg.handleValueNoti.value, pMsg->msg.handleValueNoti.len);
    return;
}



void ble_ancs_process_ns_notify(gattMsgEvent_t* pMsg)
{
    ancs_ctx_t* pctx = &s_ancs_ctx;
    ancs_evt_t evt;
    ancs_notify_evt_t notify_msg;
    uint8_t len = pMsg->msg.handleValueNoti.len;

    if (len != 8)
    {
        LOG("\n");
        LOG("Error evt len\n");
        return;
    }

    // Create pointer to GATT notification data.
    uint8_t* packetData = pMsg->msg.handleValueNoti.value;
    // Store the ANCS notification's eventID
    notify_msg.eventID = packetData[0];
    // Store the ANCS notification's eventFlag
    notify_msg.eventFlag = packetData[1];
    // Store the ANCS notification's categoryID
    notify_msg.categoryID = packetData[2];
    notify_msg.categoryCount = packetData[3];
    // Notification UID from packetData[4] to packetData[7]
    notify_msg.notifUID[0] = packetData[ANCS_NOTIF_UID_LENGTH];
    notify_msg.notifUID[1] = packetData[ANCS_NOTIF_UID_LENGTH+1];
    notify_msg.notifUID[2] = packetData[ANCS_NOTIF_UID_LENGTH+2];
    notify_msg.notifUID[3] = packetData[ANCS_NOTIF_UID_LENGTH+3];
    evt.msg = (void*)(&notify_msg);
    evt.len = sizeof(notify_msg);
    evt.type = BLE_ANCS_EVT_NOTIF;

    if(pctx->callback)
    {
        pctx->callback(&evt);
    }
}



bStatus_t ble_ancs_get_notif_attrs(const uint8_t* pNotificationUID)
{
    return notif_attrs_get(&s_ancs_ctx, pNotificationUID);
}


bStatus_t ble_ancs_get_app_attrs(const uint8_t* p_app_id, uint8_t app_id_len)
{
    return app_attrs_get(&s_ancs_ctx, p_app_id, app_id_len);
}


bStatus_t ble_ancs_handle_gatt_event(gattMsgEvent_t* pMsg)
{
    ancs_ctx_t* pctx = &s_ancs_ctx;
    ancs_service_t* pservie = &(pctx->ancs_service);

    if(pctx->app_state == ANCS_STATE_DISCOVERY)
    {
        ble_disc_service(pMsg);
    }
    else if (pMsg->method == ATT_HANDLE_VALUE_NOTI || pMsg->method == ATT_HANDLE_VALUE_IND)
    {
        // If we receive a GATT notification, we can assume it pertains to ANCS
        // because we only subscribe to notifications from the Notification Source
        // ancs Data Source.
        uint8_t notifHandle = pMsg->msg.handleValueNoti.handle;

        if ( notifHandle == pservie->chars_hdl[ANCS_NOTIF_SCR_HDL_START])
        {
            ble_ancs_process_ns_notify(pMsg);
        }
        else if ( notifHandle == pservie->chars_hdl[ANCS_DATA_SRC_HDL_START])
        {
            ble_ancs_process_ds_notify(pMsg);
        }
    }
    //If we have received a read or write response, assume that it is related to
    //CCCD configuration
    else if (pMsg->method == ATT_WRITE_RSP)
    {
    }

    // It's safe to free the incoming message
    return (TRUE);
}



bStatus_t ble_ancs_disconnect(void)
{
    ancs_ctx_t* pctx = &s_ancs_ctx;
    uint8_t               app_task_ID = pctx->app_task_ID;
    ancs_evt_hdl_t        callback = pctx->callback;
    osal_memset(&s_ancs_ctx, 0, sizeof(s_ancs_ctx));
    pctx->app_task_ID = app_task_ID;
    pctx->callback = callback;
    return SUCCESS;
}

bStatus_t ble_ancs_init(ancs_evt_hdl_t evt_hdl, uint8_t task_ID)
{
    ancs_ctx_t* pctx = &s_ancs_ctx;
    osal_memset(&s_ancs_ctx, 0, sizeof(s_ancs_ctx));
    pctx->app_task_ID = task_ID;
    pctx->callback = evt_hdl;
    return SUCCESS;
}

