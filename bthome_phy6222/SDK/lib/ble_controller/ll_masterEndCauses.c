/*******************************************************************************
    Filename:       ll_MasterEndCauses.c
    Revised:        $
    Revision:       $

    Description:    This file contains the Link Layer (LL) handlers for the
                  various master end causes that result from a PHY task
                  completion. The file also contains the master end causes
                  related to connection termination, as well as common
                  termination routines used by the master.

 SDK_LICENSE

*******************************************************************************/

/*******************************************************************************
    INCLUDES
*/
#include "bcomdef.h"

#include "ll.h"
#include "ll_common.h"
#include "ll_enc.h"
#include "mcu.h"
#include "bus_dev.h"
#include "jump_function.h"
#include "global_config.h"
#include "ll_hw_drv.h"

/*******************************************************************************
    MACROS
*/

/*******************************************************************************
    CONSTANTS
*/

/*******************************************************************************
    TYPEDEFS
*/

/*******************************************************************************
    LOCAL VARIABLES
*/
int32   connUpdateTimer = 0;                     // adjustment timer for conn update, could be negative if new interval greate than old one. Unit : 625us
/*******************************************************************************
    GLOBAL VARIABLES
*/

extern perStatsByChan_t* p_perStatsByChan;
extern  uint8    g_conn_taskID;
extern  uint16   g_conn_taskEvent;


/*******************************************************************************
    Prototypes
*/
uint8 llProcessMasterControlProcedures( llConnState_t* connPtr );
uint8 llSetupNextMasterEvent( void );

/*******************************************************************************
    Functions
*/

/*******************************************************************************
    @fn          llMasterEvt_TaskEndOk

    @brief       This function is used to handle the PHY task done end cause
                TASK_ENDOK that can result from one of three causes. First, a
                a packet was successfully received with MD=0 (i.e. no more Slave
                data) after having transmitted a packet with MD=0. Second, a
                received packet did not fit in the RX FIFO after transmitting
                a packet with MD=0. Third, a packet was received from the Slave
                while BLE_L_CONF.ENDC is true or after Timer 2 Event 2 occurs.

                Note: The TASK_ENDOK end cause will also handle the TASK_NOSYNC,
                      TASK_RXERR, and TASK_MAXNACK end causes as well.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
void llMasterEvt_TaskEndOk0( void )
{
    llConnState_t* connPtr;
    uint16         numPkts;
    int        i;
    uint32_t   T2, schedule_time;
    // get connection information
    connPtr = &conn_param[g_ll_conn_ctx.currentConn];
    // advance the connection event count
    connPtr->currentEvent = connPtr->nextEvent;
    // get the total number of received packets
    // Note: Since Auto-Flush is enabled, numRxFifoFull is incremented instead of
    //       numRxOk when there's no room in the FIFO. When Auto-Flush is
    //       disabled and there's no room in the FIFO, only numRxFifoFull is
    //       incremented for any kind of received packet.
    numPkts = ( rfCounters.numRxOk       +
                rfCounters.numRxNotOk    +
                rfCounters.numRxEmpty    +
                rfCounters.numRxIgnored  +
                rfCounters.numRxFifoFull );
    // collect packet error information
    connPtr->perInfo.numPkts   += numPkts;
    connPtr->perInfo.numCrcErr += rfCounters.numRxNotOk;
    //
    connPtr->perInfo.numEvents++;

//  // check if PER by Channel is enabled
//  if ( connPtr->perInfoByChan != NULL )
//  {
//     connPtr->perInfoByChan->numPkts[ PHY_GET_DATA_CHAN() ]   += numPkts;
//     connPtr->perInfoByChan->numCrcErr[ PHY_GET_DATA_CHAN() ] += rfCounters.numRxNotOk;
//  }

    // check if any data has been received
    // Note: numRxOk includes numRxCtrl
    // Note: numRxNotOk removed as 4.5.2 of spec says the timer is reset upon
    //       receipt of a "valid packet", which is taken to mean no CRC error.
    if ( rfCounters.numRxOk    || rfCounters.numRxIgnored ||
            rfCounters.numRxEmpty || rfCounters.numRxFifoFull
            || connPtr->rx_crcok != 0)     // ever Rx CRC OK packet
    {
        // yes, so update the supervision expiration count
        connPtr->expirationEvent = connPtr->currentEvent + connPtr->expirationValue;
        // clear flag that indicates we received first packet
        // Note: The first packet only really needs to be signalled when a new
        //       connection is formed. However, there's no harm in resetting it
        //       every time in order to simplify the control logic.
        // Note: True-Low logic is used here to be consistent with nR's language.
        connPtr->firstPacket = 0;

        //20181206 ZQ add phy change nofity
        //receiver ack notifty the host
        if(connPtr->llPhyModeCtrl.isChanged==TRUE)
        {
            connPtr->llPhyModeCtrl.isChanged = FALSE;
            llPhyModeCtrlUpdateNotify(connPtr,LL_STATUS_SUCCESS);
        }
    }
    else // no data received, or packet received with CRC error
    {
        // check if we received any packets with a CRC error
        if ( rfCounters.numRxNotOk )
        {
            // clear flag that indicates we received first packet
            // Note: The first packet only really needs to be signalled when a new
            //       connection is formed. However, there's no harm in resetting it
            //       every time in order to simplify the control logic.
            // Note: True-Low logic is used here to be consistent with nR's language.
            connPtr->firstPacket = 0;
        }
        else // no packet was received
        {
            // collect packet error information, TI use HCI ext to get this information. No used by PHY+ now
            connPtr->perInfo.numMissedEvts++;
        }

        // check if we have a Supervision Timeout
        if ( connPtr->expirationEvent <= connPtr->currentEvent )     // 2019-7-17, change from '==' to '<='
        {
            // check if the connection has already been established
            if ( connPtr->firstPacket == 0 )
            {
                // yes, so terminate with LSTO
                llConnTerminate( connPtr, LL_SUPERVISION_TIMEOUT_TERM );
            }
            else // no, so this is a failure to establish the connection
            {
                // so terminate immediately with failure to establish connection
                llConnTerminate( connPtr, LL_CONN_ESTABLISHMENT_FAILED_TERM );
            }

//#ifdef MULTI_ROLE
            ll_scheduler(LL_INVALID_TIME);           // link is terminated, update scheduler info
//#endif
            return;
        }
    }

    /*
    ** Process RX Data Packets
    */
    // check if there is any data in the Rx FIFO
    uint8_t  buffer_size;
    buffer_size = getRxBufferSize(connPtr);

    for ( i = 0; i < buffer_size; i ++)     // note: i < getRxBufferSize()  will fail the loop
    {
        // there is, so process it; check if data was processed
        if ( llProcessRxData() == FALSE )
        {
            // it wasn't, so we're done
//        ll_scheduler(LL_INVALID_TIME);
            break;
        }
    }

    // check if this connection was terminated
    if ( !connPtr->active )
    {
//#ifdef MULTI_ROLE
        ll_scheduler(LL_INVALID_TIME);
//#endif
        return;
    }

    /*
    ** Check Control Procedure Processing
    */
    if ( llProcessMasterControlProcedures( connPtr ) == LL_CTRL_PROC_STATUS_TERMINATE )
    {
//#ifdef MULTI_ROLE
        ll_scheduler(LL_INVALID_TIME);           // link is termainte, update schedle info
//#endif
        return;
    }

    /*
    ** Process TX Data Packets
    */
    // copy any pending data to the TX FIFO
    llProcessTxData( connPtr, LL_TX_DATA_CONTEXT_POST_PROCESSING );
    // if any fragment l2cap pkt, copy to TX FIFO
    l2capPocessFragmentTxData((uint16)connPtr->connId);

    /*
    ** Setup Next Slave Event Timing
    */

    // update next event, calculate time to next event, calculate timer drift,
    // update anchor points, setup NR T2E1 and T2E2 events
    if ( llSetupNextMasterEvent() == LL_SETUP_NEXT_LINK_STATUS_TERMINATE )            // PHY+ always return success here
    {
        // this connection is terminated, so nothing to schedule
//#ifdef MULTI_ROLE
        ll_scheduler(LL_INVALID_TIME);
//#endif
        return;
    }

    /*
    ** Schedule Next Task
    */
//#ifdef MULTI_ROLE
//  schedule_time = ll_get_next_timer(g_ll_conn_ctx.currentConn);
    schedule_time = (connPtr->curParam.connInterval + connUpdateTimer) * 625;
    T2 = read_current_fine_time();
    // TODO: don't know the cause, here need add 32us to gain accurate timing
    ll_scheduler(schedule_time - 10 - LL_TIME_DELTA(g_ll_conn_ctx.timerExpiryTick, T2) + 32);    // 10us: rough delay from timer expire to timer ISR
//#endif
    return;
}




/*******************************************************************************
    @fn          llSetupNextMasterEvent

    @brief       This function is used to checks if an Update Parameters and/or an
                Update Data Channel has occurred, and sets the next data
                channel.

                Side Effects:
                t2e1
                t2e2

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      boolean - Indicates whether a link terminate occurred:
                          LL_SETUP_NEXT_LINK_STATUS_TERMINATE: Terminate due to a LSTO.
                          LL_SETUP_NEXT_LINK_STATUS_SUCCESS: Do not terminate.
*/
uint8 llSetupNextMasterEvent0( void )
{
    llConnState_t* connPtr;
    //uint32        timeToNextEvt;      // TODO: use this parameter to update timer1 setting
    connUpdateTimer = 0;                // for connUpdate case, the value is (winoffset + delta interval) , otherwise, it is zero
    // get pointer to connection info
    connPtr = &conn_param[g_ll_conn_ctx.currentConn];
    // update the next connection event count
    connPtr->nextEvent = (uint16)(connPtr->currentEvent + 1);

    /*
    ** Check for a Parameter Update
    */

    // check if there's a connection parameter udpate to the connection, and if
    // so, check if the update event is before or equal to the next active event
    if ((connPtr->pendingParamUpdate == TRUE)  &&
            (connPtr->nextEvent == connPtr->paramUpdateEvent))
    {
        // convert the new LSTO from time to an expiration connection event count
        llConvertLstoToEvent( connPtr, &connPtr->paramUpdate );
        // update the LSTO expiration count
        // Note: This is needed in case the new connection fails to receive a
        //       packet and a RXTIMEOUT results. The expiration count must
        //       already be initialized based on the new event values.
        connPtr->expirationEvent = connPtr->nextEvent + connPtr->expirationValue;
        #if 0
        // find the number of events between this event and the update parameter
        // event, based on the original connection interval
        // Note: The old connection interval must be used!
        timeToNextEvt = (uint32)connPtr->curParam.connInterval +
                        (uint32)connPtr->paramUpdate.winOffset;
        #endif
//#ifdef MULTI_ROLE
        // schedule next timer after processing the connection event, no re-schedule required
        // for conn update case, schedule time should be (old interval + offset), in llMasterEvt_TaskEndOk, sheduler using new interval
        connUpdateTimer = connPtr->paramUpdate.winOffset + (connPtr->curParam.connInterval - connPtr->paramUpdate.connInterval);
//#endif
        // update the current parameters from the new parameters
        // Note: The new parameter connTimeout is not needed since once it is
        //       converted to connection events, its value is maintained by
        //       expirationValue. The new parameter winSize is only needed in case
        //       there's a RX timeout.
        connPtr->curParam.connInterval = connPtr->paramUpdate.connInterval;
        connPtr->curParam.winSize      = connPtr->paramUpdate.winSize;
        connPtr->curParam.slaveLatency = connPtr->paramUpdate.slaveLatency;
        // convert the Control Procedure timeout into connection event count
        llConvertCtrlProcTimeoutToEvent( connPtr );
        // clear the pending flag
        connPtr->pendingParamUpdate = FALSE;
        // notify the Host
        // Note: The HCI spec says the LE Connection Update Complete event shall
        //       be generated after the connection parameters have been applied
        //       by the Controller. This is different from the Slave, which only
        //       reports this event if the connection interval, slave latency,
        //       and/or connection timeout are changed.
        // Note: The values are kept in units of 625us, so they must be
        //       converted back to their what's used at the interface.
        LL_ConnParamUpdateCback( (uint16)connPtr->connId,
                                 connPtr->paramUpdate.connInterval >> 1,
                                 connPtr->paramUpdate.slaveLatency,
                                 connPtr->paramUpdate.connTimeout >> 4 );
//#ifdef MULTI_ROLE
//#else
//    // next conn event timer start when trigger current conn event, if conn parameter update is scheduled, update the timer
//    uint32 elapse_time, next_time, calibrate;
//
//    next_time = timeToNextEvt * 625;
//    calibrate = 10;     // consider re-schedule timer1 cost
//    elapse_time = (CP_TIM1->LoadCount - CP_TIM1->CurrentCount) >> 2;
//
//    next_time = next_time - elapse_time - calibrate;
//
//    // re-schedule timer1
//    ll_schedule_next_event(next_time);
//#endif
    }
    else // no parameter update, so...
    {
        // time to next event continues as usual
        // timeToNextEvt = connPtr->curParam.connInterval;     // to remove
    }

    // check for a Data Channel Update and calculate and set next data channel
    // Note: The Data Channel Update must come after the Parameters Update in
    //       case the latter updates the next event count.
    llSetNextDataChan( connPtr );
    llSetNextPhyMode( connPtr );
    return( LL_SETUP_NEXT_LINK_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          llProcessMasterControlPacket

    @brief       This routine is used to process incoming Control packet.

    input parameters

    @param       connPtr - Pointer to BLE LL Connection.
    @param       pBuf    - Pointer to Control packet payload.

    output parameters

    @param       None.

    @return      None.
*/
void llProcessMasterControlPacket0( llConnState_t* connPtr,
                                    uint8*         pBuf )
{
    uint8 i;
    uint8 opcode = *pBuf++;
    uint8 iqCnt = 0;

    // check the type of control packet
    switch( opcode )
    {
    // Encryption Response
    case LL_CTRL_ENC_RSP:
        // concatenate slave's SKDs with SKDm
        // Note: The SKDs MSO is the MSO of the SKD.
        //PHY_READ_BYTE( (uint8 *)&connPtr->encInfo.SKD[LL_ENC_SKD_S_OFFSET], LL_ENC_SKD_S_LEN );
        pBuf = llMemCopySrc( (uint8*)&connPtr->encInfo.SKD[LL_ENC_SKD_S_OFFSET], pBuf, LL_ENC_SKD_S_LEN );
        // bytes are received LSO..MSO, but need to be maintained as
        // MSO..LSO, per FIPS 197 (AES), so reverse the bytes
        LL_ENC_ReverseBytes( &connPtr->encInfo.SKD[LL_ENC_SKD_S_OFFSET], LL_ENC_SKD_S_LEN );
        // concatenate the slave's IVs with IVm
        // Note: The IVs MSO is the MSO of the IV.
        //PHY_READ_BYTE( (uint8 *)&connPtr->encInfo.IV[LL_ENC_IV_S_OFFSET], LL_ENC_IV_S_LEN );
        pBuf = llMemCopySrc( (uint8*)&connPtr->encInfo.IV[LL_ENC_IV_S_OFFSET], pBuf, LL_ENC_IV_S_LEN );
        // bytes are received LSO..MSO, but need to be maintained as
        // MSO..LSO, per FIPS 197 (AES), so reverse the bytes
        // ALT: POSSIBLE TO MAINTAIN THE IV IN LSO..MSO ORDER SINCE THE NONCE
        //      IS FORMED THAT WAY.
        LL_ENC_ReverseBytes( &connPtr->encInfo.IV[LL_ENC_IV_S_OFFSET], LL_ENC_IV_S_LEN );

        // place the IV into the Nonce to be used for this connection
        // Note: If a Pause Encryption control procedure is started, the
        //       old Nonce value will be used until encryption is disabled.
        // Note: The IV is sequenced LSO..MSO within the Nonce.
        // ALT: POSSIBLE TO MAINTAIN THE IV IN LSO..MSO ORDER SINCE THE NONCE
        //      IS FORMED THAT WAY.
        for (i=0; i<LL_ENC_IV_LEN; i++)
        {
            connPtr->encInfo.nonce[ LL_END_NONCE_IV_OFFSET+i ] =
                connPtr->encInfo.IV[ (LL_ENC_IV_LEN-i)-1 ];
        }

        // generate the Session Key (i.e. SK = AES128(LTK, SKD))
        LL_ENC_GenerateSK( connPtr->encInfo.LTK,
                           connPtr->encInfo.SKD,
                           connPtr->encInfo.SK );
//      LOG("LTK: %x\r\n", connPtr->encInfo.LTK);
//      LOG("SKD: %x\r\n", connPtr->encInfo.SKD);
//      LOG("SK: %x\r\n", connPtr->encInfo.SK[0], connPtr->encInfo.SK[1], connPtr->encInfo.SK[],connPtr->encInfo.SK[0],
//      connPtr->encInfo.SK[0],connPtr->encInfo.SK[0],connPtr->encInfo.SK[0]);
        // Note: Done for now; the slave will send LL_CTRL_START_ENC_REQ.
//LOG("ENC_RSP ->");
        break;

    // Start Encryption Request
    case LL_CTRL_START_ENC_REQ:
        // set a flag to indicate we've received this packet
        connPtr->encInfo.startEncReqRcved = TRUE;
        break;

    // Start Encryption Response
    case LL_CTRL_START_ENC_RSP:
        // set flag to allow outgoing data transmissions
        connPtr->txDataEnabled = TRUE;
        // okay to receive data again
        connPtr->rxDataEnabled = TRUE;
        // indicate we've received the start encryption response
        connPtr->encInfo.startEncRspRcved = TRUE;

        // notify the Host
        if ( connPtr->encInfo.encRestart == TRUE )
        {
            // a key change was requested
            LL_EncKeyRefreshCback( connPtr->connId,
                                   LL_ENC_KEY_REQ_ACCEPTED );
        }
        else
        {
            // a new encryption was requested
            LL_EncChangeCback( connPtr->connId,
                               LL_ENC_KEY_REQ_ACCEPTED,
                               LL_ENCRYPTION_ON );
        }

        // clear the restart flag in case of another key change request
        // Note: But in reality, there isn't a disable encryption in BLE,
        //       so once encryption is enabled, any call to LL_StartEncrypt
        //       will result in an encryption key change callback.
        connPtr->encInfo.encRestart = FALSE;
//LOG("START_ENC_RSP ->");
        break;

    // Pause Encryption Response
    case LL_CTRL_PAUSE_ENC_RSP:
        // set a flag to indicate we have received LL_START_ENC_RSP
        connPtr->encInfo.pauseEncRspRcved = TRUE;
        break;

    // Reject Encryption Indication
    case LL_CTRL_REJECT_IND:
        // either the slave's Host has failed to provide an LTK, or
        // the encryption feature is not supported by the slave, so read
        // the rejection indication error code
        //connPtr->encInfo.encRejectErrCode = PHY_READ_BYTE_VAL();
        connPtr->encInfo.encRejectErrCode = *pBuf;
        // and end the start encryption procedure
        connPtr->encInfo.rejectIndRcved = TRUE;
        break;

    // Controller Feature Setup   --> should be LL_CTRL_SLAVE_FEATURE_REQ
//    case LL_CTRL_FEATURE_REQ:                // new for BLE4.2, to test

//      for (i=0; i<LL_MAX_FEATURE_SET_SIZE; i++)
//      {
//        connPtr->featureSetInfo.featureSet[i] = deviceFeatureSet.featureSet[i];
//      }

//      // logical-AND with master's feature set to indicate which of the
//      // controller features in the master the slave requests to be used
//      for (i=0; i<LL_MAX_FEATURE_SET_SIZE; i++)
//      {
//        connPtr->featureSetInfo.featureSet[i] =
//          *pBuf++ & deviceFeatureSet.featureSet[i];
//      }

//      // schedule the output of the control packet
//      // Note: Features to be used will be taken on the next connection
//      //       event after the response is successfully transmitted.
//      llEnqueueCtrlPkt( connPtr, LL_CTRL_FEATURE_RSP );

//      break;

    case LL_CTRL_FEATURE_RSP:
    {
        uint8 peerFeatureSet[ LL_MAX_FEATURE_SET_SIZE ];
        // get the peer's device Feature Set
        //for (i=0; i<LL_MAX_FEATURE_SET_SIZE; i++)
        //{
        //  peerFeatureSet[i] = PHY_READ_BYTE_VAL();
        //}
        pBuf = llMemCopySrc( peerFeatureSet, pBuf, LL_MAX_FEATURE_SET_SIZE );

        // read this device's Feature Set
        // Note: Must re-read the device feature set as it may have been
        //       changed by the Host with Set Local Feature Set.
        // Note: It is not clear this should be done. If the host sets
        //       the Local Set, then reads the Remote Set, but before
        //       that is done, sets the Local Set a second time, then
        //       perhaps re-reading this device's feature set makes the
        //       Local Set inconsistent. On the other hand, it will be
        //       consistent with the Local Set before the Remote Set
        //       read. Note sure.
        for (i=0; i<LL_MAX_FEATURE_SET_SIZE; i++)
        {
            connPtr->featureSetInfo.featureSet[i] = deviceFeatureSet.featureSet[i];
        }

        // logical-AND with slave's feature set to indicate which of the
        // controller features in the master the slave requests to be
        // used
        // Note: For now, there is only one feature that is supported
        //       controller-to-controller.
        // Note: If the peer supports the feature, then our setting is
        //       the controller-to-controller setting, so no action
        //       is required.
        if ( !(peerFeatureSet[0] & LL_FEATURE_ENCRYPTION) )
        {
            // this feature is not supported by the peer, so it doesn't
            // matter if we support it or not, it should not be supported
            connPtr->featureSetInfo.featureSet[0] &= ~LL_FEATURE_ENCRYPTION;
        }
    }

        // set flag to indicate the response has been received
    connPtr->featureSetInfo.featureRspRcved = TRUE;
    break;

    // Version Information Indication
    case LL_CTRL_VERSION_IND:

        // check if the peer's version information has already been obtained
        if ( connPtr->verExchange.peerInfoValid == TRUE )
        {
            // it has, so something is wrong as the spec indicates that
            // only one version indication should be sent for a connection
            // unknown data PDU control packet received so save the type
            connPtr->unknownCtrlType = opcode;
            // schedule the output of the control packet
            llEnqueueCtrlPkt( connPtr, LL_CTRL_UNKNOWN_RSP );
        }
        else // the peer version info is invalid, so make it valid
        {
            // get the peer's version information and save it
            //PHY_READ_BYTE( (uint8 *)&peerInfo.verInfo.verNum, 1 );
            connPtr->verInfo.verNum = *pBuf++;
            //PHY_READ_BYTE( (uint8 *)&peerInfo.verInfo.comId, 2 );
            pBuf = llMemCopySrc( (uint8*)&connPtr->verInfo.comId, pBuf, 2 );
            //PHY_READ_BYTE( (uint8 *)&peerInfo.verInfo.subverNum, 2 );
            pBuf = llMemCopySrc( (uint8*)&connPtr->verInfo.subverNum, pBuf, 2 );
            // set a flag to indicate it is now valid
            connPtr->verExchange.peerInfoValid = TRUE;

            // check if a version indication has been sent
            if ( connPtr->verExchange.verInfoSent == FALSE )
            {
                // no, so this is a peer's request for our version information
                llEnqueueCtrlPkt( connPtr, LL_CTRL_VERSION_IND );
            }
        }

        break;

    // Terminate Indication
    case LL_CTRL_TERMINATE_IND:
        // read the reason code
        connPtr->termInfo.reason = *pBuf;
        // set flag to indicate a termination indication was received
        connPtr->termInfo.termIndRcvd = TRUE;
        // received a terminate from peer host, so terminate after
        // confirming we have sent an ACK
        // Note: For the master, we have to ensure that this control
        //       packet was ACK'ed. For that, the nR has a new flag that
        //       is set when the control packet is received, and cleared
        //       when the control packet received is ACK'ed.
        // Note: This is not an issue as a slave because the terminate
        //       packet will re-transmit until the slave ACK's.
        // ALT: COULD REPLACE THIS CONTROL PROCEDURE AT THE HEAD OF THE
        //      QUEUE SO TERMINATE CAN TAKE PLACE ASAP.
        //llReplaceCtrlPkt( connPtr, LL_CTRL_TERMINATE_RX_WAIT_FOR_TX_ACK );
        llEnqueueCtrlPkt( connPtr, LL_CTRL_TERMINATE_RX_WAIT_FOR_TX_ACK );
        break;

// LL PDU Data Length Req
    case LL_CTRL_LENGTH_REQ:

        // check if the feature response procedure has already been performed
        // on this connection
        if ( connPtr->featureSetInfo.featureRspRcved == FALSE )
        {
            // it hasn't so re-load this device's local Feature Set to the
            // connection as it may have been changed by the Host with HCI
            // extenstion Set Local Feature Set command
            for (i=0; i<LL_MAX_FEATURE_SET_SIZE; i++)
            {
                connPtr->featureSetInfo.featureSet[i] = deviceFeatureSet.featureSet[i];
            }
        }

        // check if supported DLE
        if ( (connPtr->featureSetInfo.featureSet[0] & LL_FEATURE_DATA_LENGTH_EXTENSION)
                != LL_FEATURE_DATA_LENGTH_EXTENSION )
        {
            // unknown data PDU control packet received so save the type
            connPtr->unknownCtrlType = opcode;
            // schedule the output of the control packet
            llEnqueueCtrlPkt( connPtr, LL_CTRL_UNKNOWN_RSP );
        }
        else
        {
            if(connPtr->llPduLen.isProcessingReq==FALSE)
            {
                pBuf = llMemCopySrc( (uint8*)& (connPtr->llPduLen.remote.MaxRxOctets), pBuf, 2 );
                pBuf = llMemCopySrc( (uint8*)& (connPtr->llPduLen.remote.MaxRxTime), pBuf, 2 );
                pBuf = llMemCopySrc( (uint8*)& (connPtr->llPduLen.remote.MaxTxOctets), pBuf, 2 );
                pBuf = llMemCopySrc( (uint8*)& (connPtr->llPduLen.remote.MaxTxTime), pBuf, 2 );
                connPtr->llPduLen.isProcessingReq=TRUE;
                llEnqueueCtrlPkt( connPtr, LL_CTRL_LENGTH_RSP );
            }
        }

        break;

    // LL PDU Data Length RSP
    case LL_CTRL_LENGTH_RSP:

        // check if supported DLE
        if ( (connPtr->featureSetInfo.featureSet[0] & LL_FEATURE_DATA_LENGTH_EXTENSION)
                != LL_FEATURE_DATA_LENGTH_EXTENSION )
        {
            // unknown data PDU control packet received so save the type
            connPtr->unknownCtrlType = opcode;
            // schedule the output of the control packet
            llEnqueueCtrlPkt( connPtr, LL_CTRL_UNKNOWN_RSP );
        }
        else
        {
            if(connPtr->llPduLen.isWatingRsp==TRUE )
            {
                pBuf = llMemCopySrc( (uint8*)& (connPtr->llPduLen.remote.MaxRxOctets), pBuf, 2 );
                pBuf = llMemCopySrc( (uint8*)& (connPtr->llPduLen.remote.MaxRxTime), pBuf, 2 );
                pBuf = llMemCopySrc( (uint8*)& (connPtr->llPduLen.remote.MaxTxOctets), pBuf, 2 );
                pBuf = llMemCopySrc( (uint8*)& (connPtr->llPduLen.remote.MaxTxTime), pBuf, 2 );
                llPduLengthUpdate((uint16)connPtr->connId);
                connPtr->llPduLen.isWatingRsp=FALSE;
            }
        }

        break;

    // LL PHY UPDATE REQ
    case LL_CTRL_PHY_REQ:

        // check if the feature response procedure has already been performed
        // on this connection
        if ( connPtr->featureSetInfo.featureRspRcved == FALSE )
        {
            // it hasn't so re-load this device's local Feature Set to the
            // connection as it may have been changed by the Host with HCI
            // extenstion Set Local Feature Set command
            for (i=0; i<LL_MAX_FEATURE_SET_SIZE; i++)
            {
                connPtr->featureSetInfo.featureSet[i] = deviceFeatureSet.featureSet[i];
            }
        }

        // check if supported PHY MODE UPDATE
        if (   (connPtr->featureSetInfo.featureSet[1] & LL_FEATURE_2M_PHY) != LL_FEATURE_2M_PHY
                && (connPtr->featureSetInfo.featureSet[1] & LL_FEATURE_CODED_PHY) != LL_FEATURE_CODED_PHY)
        {
            // unknown data PDU control packet received so save the type
            connPtr->unknownCtrlType = opcode;
            // schedule the output of the control packet
            llEnqueueCtrlPkt( connPtr, LL_CTRL_UNKNOWN_RSP );
        }
        else
        {
            //process for the protocol collision
            //2018-11-10 by ZQ
            if(connPtr->llPhyModeCtrl.isWatingRsp==TRUE ||
                    connPtr->pendingChanUpdate==TRUE  ||
                    connPtr->pendingParamUpdate==TRUE   )
            {
                connPtr->isCollision=TRUE;
                connPtr->rejectOpCode = LL_CTRL_PHY_REQ;
                // schedule the output of the control packet
                llEnqueueCtrlPkt( connPtr, LL_CTRL_REJECT_EXT_IND );
            }
            else
            {
                if(connPtr->llPhyModeCtrl.isProcessingReq==FALSE)
                {
                    connPtr->llPhyModeCtrl.req.txPhy=*pBuf++;
                    connPtr->llPhyModeCtrl.req.rxPhy=*pBuf++;
                    connPtr->llPhyModeCtrl.req.allPhy=connPtr->llPhyModeCtrl.def.allPhy;
                    connPtr->llPhyModeCtrl.rsp.txPhy=connPtr->llPhyModeCtrl.def.txPhy;
                    connPtr->llPhyModeCtrl.rsp.rxPhy=connPtr->llPhyModeCtrl.def.rxPhy;
                    //rsp and req will be used to determine the next phy mode
                    LL_PhyUpdate((uint16) connPtr->connId);
                    connPtr->llPhyModeCtrl.isProcessingReq=TRUE;
                }
                else
                {
                    //should no be here
                }
            }
        }

        break;

    // LL_CTRL_PHY_RSP
    case LL_CTRL_PHY_RSP:

        // check if supported PHY MODE UPDATE
        if (   (connPtr->featureSetInfo.featureSet[1] & LL_FEATURE_2M_PHY) != LL_FEATURE_2M_PHY
                && (connPtr->featureSetInfo.featureSet[1] & LL_FEATURE_CODED_PHY) != LL_FEATURE_CODED_PHY)
        {
            // unknown data PDU control packet received so save the type
            connPtr->unknownCtrlType = opcode;
            // schedule the output of the control packet
            llEnqueueCtrlPkt( connPtr, LL_CTRL_UNKNOWN_RSP );
        }
        else
        {
            if(connPtr->llPhyModeCtrl.isWatingRsp==TRUE)
            {
                connPtr->llPhyModeCtrl.rsp.txPhy=*pBuf++;
                connPtr->llPhyModeCtrl.rsp.rxPhy=*pBuf++;
                LL_PhyUpdate((uint16) connPtr->connId);
                connPtr->llPhyModeCtrl.isWatingRsp=FALSE;
            }
            else
            {
                //should no be here
            }
        }

        break;

    case LL_CTRL_CTE_REQ:

        // check if the feature response procedure has already been performed
        // on this connection
        if ( connPtr->featureSetInfo.featureRspRcved == FALSE )
        {
            // it hasn't so re-load this device's local Feature Set to the
            // connection as it may have been changed by the Host with HCI
            // extenstion Set Local Feature Set command
            for (i=0; i<LL_MAX_FEATURE_SET_SIZE; i++)
            {
                connPtr->featureSetInfo.featureSet[i] = deviceFeatureSet.featureSet[i];
            }
        }

        // check if supported CTE Response Feature
//        if( connPtr->featureSetInfo.featureSet[LL_CTE_FEATURE_IDX] & LL_CONN_CTE_RSP)
        if(( ( connPtr->featureSetInfo.featureSet[LL_CTE_FEATURE_IDX] & LL_CONN_CTE_RSP) != LL_CONN_CTE_RSP) || \
                ( connPtr->llCTE_RspFlag != TRUE ))
        {
            // unknown data PDU control packet received so save the type
            connPtr->unknownCtrlType = opcode;
            // schedule the output of the control packet
            llEnqueueCtrlPkt( connPtr, LL_CTRL_UNKNOWN_RSP );
        }
        else
        {
            // process for the protocol collision
            // if other ctrl command procedure in processing , then reject
            if(connPtr->llCTEModeCtrl.isWatingRsp==TRUE)
            {
                connPtr->isCollision=TRUE;
                connPtr->rejectOpCode = LL_CTRL_CTE_REQ;
                // schedule the output of the control packet
                llEnqueueCtrlPkt( connPtr, LL_CTRL_REJECT_EXT_IND );
            }
            else
            {
                if(connPtr->llCTEModeCtrl.isProcessingReq==FALSE)
                {
                    uint8 CTE_tmp;
                    CTE_tmp = *pBuf++;
                    connPtr->llConnCTE.CTE_Length = CTE_tmp & 0x1F;
                    connPtr->llConnCTE.CTE_Type = CTE_tmp & 0xC0;
                    connPtr->llCTEModeCtrl.isProcessingReq=TRUE;

                    if( ( connPtr->llConnCTE.enable ) && ( connPtr->llRfPhyPktFmt < LL_PHY_CODE ))
                    {
                        llEnqueueCtrlPkt( connPtr, LL_CTRL_CTE_RSP );
                    }
                    else
                    {
                        if( connPtr->llRfPhyPktFmt >= LL_PHY_CODE )
                        {
                            connPtr->llCTEModeCtrl.errorCode = LL_STATUS_ERROR_INVALID_LMP_LL_PARAMETER;
                        }
                        else
                        {
                            connPtr->llCTEModeCtrl.errorCode = LL_STATUS_ERROR_UNSUPPORT_LMP_LL_PARAMETER;
                        }

                        connPtr->rejectOpCode = LL_CTRL_CTE_REQ;
                        // schedule the output of the control packet
                        llEnqueueCtrlPkt( connPtr, LL_CTRL_REJECT_EXT_IND );
                    }
                }
            }
        }

        break;

    case LL_CTRL_CTE_RSP:
        if( connPtr->llCTEModeCtrl.isWatingRsp == TRUE )
        {
            if( ( g_pLLcteISample != NULL ) && ( g_pLLcteQSample != NULL) )
                iqCnt = ll_hw_get_iq_RawSample( g_pLLcteISample, g_pLLcteQSample );

            if( iqCnt > 0)
            {
                LL_ConnectionIQReportCback( connPtr->connId,
                                            connPtr->llRfPhyPktFmt,
                                            connPtr->currentChan,
                                            connPtr->lastRssi,
                                            // before CTE Transmit and sampling , no Antenna change , default 0
                                            0,
                                            connPtr->llConnCTE.CTE_Type,
                                            connPtr->llConnCTE.slot_Duration,
                                            // Packet_Status=0, CRC success,cause only CRC Correctly that can run here
                                            0,
                                            connPtr->currentEvent,
                                            iqCnt,
                                            g_pLLcteISample,
                                            g_pLLcteQSample);
            }
            else
            {
                // packet contain LL_CTE_RSP , but did not contain CTE field
                // status = 0x0 : LL_CTE_RSP received successful , but without a CTE field
                LL_CTE_Report_FailedCback(  0x0,connPtr->connId);
            }

            connPtr->llCTEModeCtrl.isWatingRsp = FALSE;
        }

        break;

    // Peer Device Received an Unknown Control Type
    case LL_CTRL_UNKNOWN_RSP:

        // Note: There doesn't appear to be any action for this message,
        //       other than to ACK it.
        if(connPtr->llPduLen.isWatingRsp)
        {
            llPduLengthUpdate((uint16)connPtr->connId);
            connPtr->llPduLen.isWatingRsp=FALSE;//not support DLE
        }

        if(connPtr->llPhyModeCtrl.isWatingRsp)
        {
            llPhyModeCtrlUpdateNotify(connPtr,LL_STATUS_ERROR_UNSUPPORTED_REMOTE_FEATURE);
            connPtr->llPhyModeCtrl.isWatingRsp=FALSE;//not support PHY_UPDATE
        }

        // 2020-01-23 add for CTE
        if( connPtr->llCTEModeCtrl.isWatingRsp )
        {
            connPtr->llCTEModeCtrl.isWatingRsp = FALSE;
        }

        break;

    // Our Device Received an Unknown Control Type
    default:
        // unknown data PDU control packet received so save the type
        connPtr->unknownCtrlType = opcode;
        // schedule the output of the control packet
        llEnqueueCtrlPkt( connPtr, LL_CTRL_UNKNOWN_RSP );
        break;
    }

    return;
}



/*******************************************************************************
    @fn          llProcessMasterControlProcedures

    @brief       This function is used to process any control procedures that
                may be active.

                Note: There can only be one active control procedure at a time.

                Note: It is assumed the NR counters have been updated at the
                      end of the task before calling this routine.

    input parameters

    @param       connPtr - Pointer to the current connection.

    output parameters

    @param       None.

    @return      uint8 - Status of control procedure processing, which can be:
                        LL_CTRL_PROC_STATUS_SUCCESS: Continue normally.
                        LL_CTRL_PROC_STATUS_TERMINATE: We have terminated.
*/
uint8 llProcessMasterControlProcedures0( llConnState_t* connPtr )
{
    // check if there are any control packets ready for processing
    while ( connPtr->ctrlPktInfo.ctrlPktCount > 0 )
    {
        // processing based on control packet type at the head of the queue
        switch( connPtr->ctrlPktInfo.ctrlPkts[ 0 ] )
        {
        case LL_CTRL_TERMINATE_IND:

            // check if the control packet procedure is is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // we have already place packet on TX FIFO, so check if its been ACK'ed
                if ( rfCounters.numTxCtrlAck )
                {
                    // done with this control packet, so remove from the processing queue
                    llDequeueCtrlPkt( connPtr );
                    // yes, so process the termination
                    // Note: No need to cleanup control packet info as we are done.
                    llConnTerminate( connPtr, LL_HOST_REQUESTED_TERM );
                    return( LL_CTRL_PROC_STATUS_TERMINATE );
                }
                else // no done yet
                {
                    // check if a termination control procedure timeout has occurred
                    // Note: No need to cleanup control packet info as we are done.
                    if ( --connPtr->ctrlPktInfo.ctrlTimeout == 0 )
                    {
                        // we're done waiting, so end it all
                        // Note: No need to cleanup control packet info as we are done.
                        llConnTerminate( connPtr, LL_CTRL_PKT_TIMEOUT_HOST_TERM );
                        return( LL_CTRL_PROC_STATUS_TERMINATE );
                    }
                    else
                    {
                        //  control packet stays at head of queue, so exit here
                        return( LL_CTRL_PROC_STATUS_SUCCESS );
                    }
                }
            }
            else // control packet has not been put on the TX FIFO yet
            {
                // so try to put it there; being active depends on a success
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupTermInd( connPtr );
                // Note: Two cases are possible:
                //       a) We successfully placed the packet in the TX FIFO.
                //       b) We did not.
                //
                //       In case (a), it may be possible that a previously just
                //       completed control packet happened to complete based on
                //       rfCounters.numTxCtrlAck. Since the current control
                //       procedure is now active, it could falsely detect
                //       rfCounters.numTxCtrlAck, when in fact this was from the
                //       previous control procedure. Consequently, return.
                //
                //       In case (b), the control packet stays at the head of the
                //       queue, and there's nothing more to do. Consequently, return.
                //
                //       So, in either case, return.
                return( LL_CTRL_PROC_STATUS_SUCCESS );
            }

        // Note: Unreachable statement generates compiler warning!
        //break;

        /*
        ** Connection Update Request
        */
        case LL_CTRL_CONNECTION_UPDATE_REQ:

//      LOG("CONN UPD");
            // check if the control packet procedure is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // we have already placed a packet on TX FIFO, so check if its been ACK'ed
                if ( rfCounters.numTxCtrlAck )
                {
                    // yes, so adjust all time values to units of 625us
                    connPtr->paramUpdate.winSize      <<= 1;
                    connPtr->paramUpdate.winOffset    <<= 1;
                    connPtr->paramUpdate.connInterval <<= 1;
                    connPtr->paramUpdate.connTimeout  <<= 4;
                    // and activate the update
                    connPtr->pendingParamUpdate = TRUE;
                    // done with this control packet, so remove from the processing queue
                    llDequeueCtrlPkt( connPtr );
                }
                else // no done yet
                {
                    // Core Spec V4.0 now indicates there is no control procedure
                    // timeout. However, it still seems prudent to monitor for the
                    // instant while waiting for the slave's ACK.
                    if ( connPtr->nextEvent == connPtr->paramUpdateEvent )
                    {
                        // this event is the instant, and the control procedure still
                        // has not been ACK'ed, we the instant has passed
                        // Note: No need to cleanup control packet info as we are done.
                        llConnTerminate( connPtr, LL_CTRL_PKT_INSTANT_PASSED_HOST_TERM );
                        return( LL_CTRL_PROC_STATUS_TERMINATE );
                    }
                    else // continue waiting for the slave's ACK
                    {
                        //  control packet stays at head of queue, so exit here
                        return( LL_CTRL_PROC_STATUS_SUCCESS );
                    }
                }
            }
            else // control packet has not been put on the TX FIFO yet
            {
                // so try to put it there; being active depends on a success
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupUpdateParamReq( connPtr );
                // Note: Two cases are possible:
                //       a) We successfully placed the packet in the TX FIFO.
                //       b) We did not.
                //
                //       In case (a), it may be possible that a previously just
                //       completed control packet happened to complete based on
                //       rfCounters.numTxCtrlAck. Since the current control
                //       procedure is now active, it could falsely detect
                //       rfCounters.numTxCtrlAck, when in fact this was from the
                //       previous control procedure. Consequently, return.
                //
                //       In case (b), the control packet stays at the head of the
                //       queue, and there's nothing more to do. Consequently, return.
                //
                //       So, in either case, return.
                return( LL_CTRL_PROC_STATUS_SUCCESS );
            }

            break;

        /*
        ** Channel Map Update Request
        */
        case LL_CTRL_CHANNEL_MAP_REQ:

            // check if the control packet procedure is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // we have already placed a packet on TX FIFO, so check if its been ACK'ed
                if ( rfCounters.numTxCtrlAck )
                {
                    // yes, so activate the update
                    connPtr->pendingChanUpdate = TRUE;
                    // done with this control packet, so remove from the processing queue
                    llDequeueCtrlPkt( connPtr );
                }
                else // no done yet
                {
                    // Core Spec V4.0 now indicates there is no control procedure
                    // timeout. However, it still seems prudent to monitor for the
                    // instant while waiting for the slave's ACK.
                    if ( connPtr->nextEvent == connPtr->chanMapUpdateEvent )
                    {
                        // this event is the instant, and the control procedure still
                        // has not been ACK'ed, we the instant has passed
                        // Note: No need to cleanup control packet info as we are done.
                        llConnTerminate( connPtr, LL_CTRL_PKT_INSTANT_PASSED_HOST_TERM );
                        return( LL_CTRL_PROC_STATUS_TERMINATE );
                    }
                    else // continue waiting for the slave's ACK
                    {
                        //  control packet stays at head of queue, so exit here
                        return( LL_CTRL_PROC_STATUS_SUCCESS );
                    }
                }
            }
            else // control packet has not been put on the TX FIFO yet
            {
                // so try to put it there; being active depends on a success
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupUpdateChanReq( connPtr );
                // Note: Two cases are possible:
                //       a) We successfully placed the packet in the TX FIFO.
                //       b) We did not.
                //
                //       In case (a), it may be possible that a previously just
                //       completed control packet happened to complete based on
                //       rfCounters.numTxCtrlAck. Since the current control
                //       procedure is now active, it could falsely detect
                //       rfCounters.numTxCtrlAck, when in fact this was from the
                //       previous control procedure. Consequently, return.
                //
                //       In case (b), the control packet stays at the head of the
                //       queue, and there's nothing more to do. Consequently, return.
                //
                //       So, in either case, return.
                return( LL_CTRL_PROC_STATUS_SUCCESS );
            }

            break;

        /*
        ** Encryption Request
        */
        case LL_CTRL_ENC_REQ:

//          LOG("1 ENC_REQ->");
            // check if the control packet procedure is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // yes, so check if it has been transmitted yet
                // Note: This does not mean this packet has been ACK'ed or NACK'ed.
                if ( rfCounters.numTxCtrl )
                {
                    // set flag to discard all incoming data transmissions
                    connPtr->rxDataEnabled = FALSE;
                }

                // we have already placed a packet on TX FIFO, so wait now until we
                // get the slave's LL_START_ENC_REQ
                if ( connPtr->encInfo.startEncReqRcved == TRUE )
                {
                    // clear packet counters
                    connPtr->encInfo.txPktCount = 0;
                    connPtr->encInfo.rxPktCount = 0;
                    // enable encryption
                    connPtr->encEnabled = TRUE;
                    // replace control procedure at head of queue to prevent interleaving
                    llReplaceCtrlPkt( connPtr, LL_CTRL_START_ENC_RSP );
                }
                else if ( connPtr->encInfo.rejectIndRcved  == TRUE )
                {
                    // the slave's Host has failed to provide an LTK, so the encryption
                    // setup has been rejected; end the start encryption procedure
                    // done with this control packet, so remove from the processing queue
                    llDequeueCtrlPkt( connPtr );
                    // disable encryption
                    // Note: Not really necessary as no data is supposed to be sent
                    //       or received.
                    connPtr->encEnabled = FALSE;
                    // set flag to allow outgoing transmissions again
                    connPtr->txDataEnabled = TRUE;
                    // set flag to allow all incoming data transmissions
                    connPtr->rxDataEnabled = TRUE;

                    // check the rejection indication error code
                    if ( connPtr->encInfo.encRejectErrCode == LL_STATUS_ERROR_PIN_OR_KEY_MISSING )
                    {
                        // notify the Host
                        LL_EncChangeCback( connPtr->connId,
                                           LL_ENC_KEY_REQ_REJECTED,
                                           LL_ENCRYPTION_OFF );
                    }
                    else // LL_STATUS_ERROR_UNSUPPORTED_REMOTE_FEATURE
                    {
                        // notify the Host
                        LL_EncChangeCback( connPtr->connId,
                                           LL_ENC_KEY_REQ_UNSUPPORTED_FEATURE,
                                           LL_ENCRYPTION_OFF );
                    }
                }
                else if ( connPtr->termInfo.termIndRcvd == TRUE )
                {
                    // the slave's Host has failed to provide an LTK, so the encryption
                    // setup has been rejected; end the start encryption procedure
                    // done with this control packet, so remove from the processing queue
                    llDequeueCtrlPkt( connPtr );
                }
                else // no done yet
                {
                    // check if a update param req control procedure timeout has occurred
                    // Note: No need to cleanup control packet info as we are done.
                    if ( --connPtr->ctrlPktInfo.ctrlTimeout == 0 )
                    {
                        // notify the Host
                        if ( connPtr->encInfo.encRestart == TRUE )
                        {
                            // a key change was requested
                            LL_EncKeyRefreshCback( connPtr->connId,
                                                   LL_CTRL_PKT_TIMEOUT_TERM );
                        }
                        else
                        {
                            // a new encryption was requested
                            LL_EncChangeCback( connPtr->connId,
                                               LL_CTRL_PKT_TIMEOUT_TERM,
                                               LL_ENCRYPTION_OFF );
                        }

                        // we're done waiting, so end it all
                        // Note: No need to cleanup control packet info as we are done.
                        llConnTerminate( connPtr, LL_CTRL_PKT_TIMEOUT_HOST_TERM );
                        return( LL_CTRL_PROC_STATUS_TERMINATE );
                    }
                    else
                    {
                        //  control packet stays at head of queue, so exit here
                        return( LL_CTRL_PROC_STATUS_SUCCESS );
                    }
                }
            }
            else // control packet has not been put on the TX FIFO yet
            {
                // so try to put it there; being active depends on a success
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupEncReq( connPtr );
                // set a flag to indicate we have received LL_START_ENC_REQ
                // Note: The LL_ENC_RSP will be received first, which will result in
                //       the master calculating its IVm and SKDm, concatenating it
                //       with the slave's IVs and SKDs, and calculating the SK from
                //       the LTK and SKD. After that, we will receive the
                //       LL_START_ENC_REQ from the slave. So, it is okay to stay in
                //       this control procedure until LL_START_ENC_REQ is received.
                // Note: It is okay to repeatedly set this flag in the event the
                //       setup routine hasn't completed yet (e.g. if the TX FIFO
                //       has not yet become empty).
                connPtr->encInfo.startEncReqRcved = FALSE;
                connPtr->encInfo.rejectIndRcved   = FALSE;
                // Note: Two cases are possible:
                //       a) We successfully placed the packet in the TX FIFO.
                //       b) We did not.
                //
                //       In case (a), it may be possible that a previously just
                //       completed control packet happened to complete based on
                //       rfCounters.numTxCtrlAck. Since the current control
                //       procedure is now active, it could falsely detect
                //       rfCounters.numTxCtrlAck, when in fact this was from the
                //       previous control procedure. Consequently, return.
                //
                //       In case (b), the control packet stays at the head of the
                //       queue, and there's nothing more to do. Consequently, return.
                //
                //       So, in either case, return.
                return( LL_CTRL_PROC_STATUS_SUCCESS );
            }

            break;

        /*
        ** Encryption Start Response
        */
        case LL_CTRL_START_ENC_RSP:

//          LOG("1 START_ENC_RSP->");
            // check if the control packet procedure is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // we have already placed a packet on TX FIFO, so wait now until we
                // get the slave's LL_START_ENC_RSP
                if ( connPtr->encInfo.startEncRspRcved == TRUE )
                {
                    // done with this control packet, so remove from the processing queue
                    llDequeueCtrlPkt( connPtr );
                    // we're done with encryption procedure, so clear flags
                    connPtr->encInfo.encReqRcved      = FALSE;
                    connPtr->encInfo.pauseEncRspRcved = FALSE;
                    connPtr->encInfo.startEncReqRcved = FALSE;
                    connPtr->encInfo.startEncRspRcved = FALSE;
                    connPtr->encInfo.rejectIndRcved   = FALSE;
                }
                else // no done yet
                {
                    // check if a update param req control procedure timeout has occurred
                    // Note: No need to cleanup control packet info as we are done.
                    if ( --connPtr->ctrlPktInfo.ctrlTimeout == 0 )
                    {
                        // notify the Host
                        if ( connPtr->encInfo.encRestart == TRUE )
                        {
                            // a key change was requested
                            LL_EncKeyRefreshCback( connPtr->connId,
                                                   LL_CTRL_PKT_TIMEOUT_TERM );
                        }
                        else
                        {
                            // a new encryption was requested
                            LL_EncChangeCback( connPtr->connId,
                                               LL_CTRL_PKT_TIMEOUT_TERM,
                                               LL_ENCRYPTION_OFF );
                        }

                        // we're done waiting, so end it all
                        // Note: No need to cleanup control packet info as we are done.
                        llConnTerminate( connPtr, LL_CTRL_PKT_TIMEOUT_HOST_TERM );
                        return( LL_CTRL_PROC_STATUS_TERMINATE );
                    }
                    else
                    {
                        //  control packet stays at head of queue, so exit here
                        return( LL_CTRL_PROC_STATUS_SUCCESS );
                    }
                }
            }
            else // control packet has not been put on the TX FIFO yet
            {
                // so try to put it there; being active depends on a success
                // Note: The llSetupStartEncRsp routine will *not* reset the control
                //       timeout value since the entire encryption procedure starts
                //       with the master sending the LL_ENC_REQ, and ends when the
                //       master receives the LL_START_ENC_RSP from the slave.
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupStartEncRsp( connPtr );
                // set a flag to indicate we have received LL_START_ENC_RSP
                // Note: It is okay to repeatedly set this flag in the event the
                //       setup routine hasn't completed yet (e.g. if the TX FIFO
                //       has not yet become empty).
                connPtr->encInfo.startEncRspRcved = FALSE;
                // Note: Two cases are possible:
                //       a) We successfully placed the packet in the TX FIFO.
                //       b) We did not.
                //
                //       In case (a), it may be possible that a previously just
                //       completed control packet happened to complete based on
                //       rfCounters.numTxCtrlAck. Since the current control
                //       procedure is now active, it could falsely detect
                //       rfCounters.numTxCtrlAck, when in fact this was from the
                //       previous control procedure. Consequently, return.
                //
                //       In case (b), the control packet stays at the head of the
                //       queue, and there's nothing more to do. Consequently, return.
                //
                //       So, in either case, return.
                return( LL_CTRL_PROC_STATUS_SUCCESS );
            }

            break;

        /*
        ** Encryption Pause Request
        */
        case LL_CTRL_PAUSE_ENC_REQ:

            // check if the control packet procedure is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // we have already placed a packet on TX FIFO, so wait now until we
                // get the slave's LL_PAUSE_ENC_RSP
                if ( connPtr->encInfo.pauseEncRspRcved == TRUE )
                {
                    // disable encryption
                    connPtr->encEnabled = FALSE;
                    // replace control procedure at head of queue to prevent interleaving
                    llReplaceCtrlPkt( connPtr, LL_CTRL_PAUSE_ENC_RSP );
                }
                else // no done yet
                {
                    // check if a update param req control procedure timeout has occurred
                    // Note: No need to cleanup control packet info as we are done.
                    if ( --connPtr->ctrlPktInfo.ctrlTimeout == 0 )
                    {
                        // notify the Host
                        if ( connPtr->encInfo.encRestart == TRUE )
                        {
                            // a key change was requested
                            LL_EncKeyRefreshCback( connPtr->connId,
                                                   LL_CTRL_PKT_TIMEOUT_TERM );
                        }
                        else
                        {
                            // a new encryption was requested
                            LL_EncChangeCback( connPtr->connId,
                                               LL_CTRL_PKT_TIMEOUT_TERM,
                                               LL_ENCRYPTION_OFF );
                        }

                        // we're done waiting, so end it all
                        // Note: No need to cleanup control packet info as we are done.
                        llConnTerminate( connPtr, LL_CTRL_PKT_TIMEOUT_HOST_TERM );
                        return( LL_CTRL_PROC_STATUS_TERMINATE );
                    }
                    else
                    {
                        //  control packet stays at head of queue, so exit here
                        return( LL_CTRL_PROC_STATUS_SUCCESS );
                    }
                }
            }
            else // control packet has not been put on the TX FIFO yet
            {
                // so try to put it there; being active depends on a success
                // Note: The llSetupStartEncRsp routine will *not* reset the control
                //       timeout value since the entire encryption procedure starts
                //       with the master sending the LL_ENC_REQ, and ends when the
                //       master receives the LL_START_ENC_RSP from the slave.
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupPauseEncReq( connPtr );
                // set a flag to indicate we have received LL_START_ENC_RSP
                // Note: It is okay to repeatedly set this flag in the event the
                //       setup routine hasn't completed yet (e.g. if the TX FIFO
                //       has not yet become empty).
                connPtr->encInfo.pauseEncRspRcved = FALSE;
                // Note: Two cases are possible:
                //       a) We successfully placed the packet in the TX FIFO.
                //       b) We did not.
                //
                //       In case (a), it may be possible that a previously just
                //       completed control packet happened to complete based on
                //       rfCounters.numTxCtrlAck. Since the current control
                //       procedure is now active, it could falsely detect
                //       rfCounters.numTxCtrlAck, when in fact this was from the
                //       previous control procedure. Consequently, return.
                //
                //       In case (b), the control packet stays at the head of the
                //       queue, and there's nothing more to do. Consequently, return.
                //
                //       So, in either case, return.
                return( LL_CTRL_PROC_STATUS_SUCCESS );
            }

            break;

        /*
        ** Encryption Pause Response
        */
        case LL_CTRL_PAUSE_ENC_RSP:

            // check if the control packet procedure is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // yes, so check if it has been transmitted yet
                // Note: This only means the packet has been transmitted, not that it
                //       has been ACK'ed or NACK'ed.
                if ( rfCounters.numTxCtrl )
                {
                    // replace control procedure at head of queue to prevent interleaving
                    llReplaceCtrlPkt( connPtr, LL_CTRL_ENC_REQ );
                }
                else // no done yet
                {
                    // check if a update param req control procedure timeout has occurred
                    // Note: No need to cleanup control packet info as we are done.
                    if ( --connPtr->ctrlPktInfo.ctrlTimeout == 0 )
                    {
                        // notify the Host
                        if ( connPtr->encInfo.encRestart == TRUE )
                        {
                            // a key change was requested
                            LL_EncKeyRefreshCback( connPtr->connId,
                                                   LL_CTRL_PKT_TIMEOUT_TERM );
                        }
                        else
                        {
                            // a new encryption was requested
                            LL_EncChangeCback( connPtr->connId,
                                               LL_CTRL_PKT_TIMEOUT_TERM,
                                               LL_ENCRYPTION_OFF );
                        }

                        // we're done waiting, so end it all
                        // Note: No need to cleanup control packet info as we are done.
                        llConnTerminate( connPtr, LL_CTRL_PKT_TIMEOUT_HOST_TERM );
                        return( LL_CTRL_PROC_STATUS_TERMINATE );
                    }
                    else
                    {
                        //  control packet stays at head of queue, so exit here
                        return( LL_CTRL_PROC_STATUS_SUCCESS );
                    }
                }
            }
            else // control packet has not been put on the TX FIFO yet
            {
                // so try to put it there; being active depends on a success
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupPauseEncRsp( connPtr );
                // Note: Two cases are possible:
                //       a) We successfully placed the packet in the TX FIFO.
                //       b) We did not.
                //
                //       In case (a), it may be possible that a previously just
                //       completed control packet happened to complete based on
                //       rfCounters.numTxCtrlAck. Since the current control
                //       procedure is now active, it could falsely detect
                //       rfCounters.numTxCtrlAck, when in fact this was from the
                //       previous control procedure. Consequently, return.
                //
                //       In case (b), the control packet stays at the head of the
                //       queue, and there's nothing more to do. Consequently, return.
                //
                //       So, in either case, return.
                return( LL_CTRL_PROC_STATUS_SUCCESS );
            }

            break;

        /*
        ** Feature Set Request
        */
        case LL_CTRL_FEATURE_REQ:

            // check if the control packet procedure is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // we have already placed a packet on TX FIFO, so wait now until we
                // get the slave's LL_CTRL_FEATURE_RSP
                if ( connPtr->featureSetInfo.featureRspRcved == TRUE )
                {
                    // notify the Host
                    LL_ReadRemoteUsedFeaturesCompleteCback( LL_STATUS_SUCCESS,
                                                            connPtr->connId,
                                                            connPtr->featureSetInfo.featureSet );
                    // done with this control packet, so remove from the processing queue
                    llDequeueCtrlPkt( connPtr );
                }
                else // no done yet
                {
                    // check if a update param req control procedure timeout has occurred
                    // Note: No need to cleanup control packet info as we are done.
                    if ( --connPtr->ctrlPktInfo.ctrlTimeout == 0 )
                    {
                        // indicate a control procedure timeout on this request
                        // Note: The parameters are not valid.
                        LL_ReadRemoteUsedFeaturesCompleteCback( LL_CTRL_PKT_TIMEOUT_TERM,
                                                                connPtr->connId,
                                                                connPtr->featureSetInfo.featureSet );
                        // we're done waiting, so end it all
                        // Note: No need to cleanup control packet info as we are done.
                        llConnTerminate( connPtr, LL_CTRL_PKT_TIMEOUT_HOST_TERM );
                        return( LL_CTRL_PROC_STATUS_TERMINATE );
                    }
                    else
                    {
                        //  control packet stays at head of queue, so exit here
                        return( LL_CTRL_PROC_STATUS_SUCCESS );
                    }
                }
            }
            else // control packet has not been put on the TX FIFO yet
            {
                // add by HZF, read device feature set
                for (int i=0; i<LL_MAX_FEATURE_SET_SIZE; i++)
                {
                    connPtr->featureSetInfo.featureSet[i] = deviceFeatureSet.featureSet[i];
                }

                // so try to put it there; being active depends on a success
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupFeatureSetReq( connPtr );
                // set flag while we wait for response
                // Note: It is okay to repeatedly set this flag in the event the
                //       setup routine hasn't completed yet (e.g. if the TX FIFO
                //       has not yet become empty).
                connPtr->featureSetInfo.featureRspRcved = FALSE;
                // Note: Two cases are possible:
                //       a) We successfully placed the packet in the TX FIFO.
                //       b) We did not.
                //
                //       In case (a), it may be possible that a previously just
                //       completed control packet happened to complete based on
                //       rfCounters.numTxCtrlAck. Since the current control
                //       procedure is now active, it could falsely detect
                //       rfCounters.numTxCtrlAck, when in fact this was from the
                //       previous control procedure. Consequently, return.
                //
                //       In case (b), the control packet stays at the head of the
                //       queue, and there's nothing more to do. Consequently, return.
                //
                //       So, in either case, return.
                return( LL_CTRL_PROC_STATUS_SUCCESS );
            }

            break;

        case LL_CTRL_FEATURE_RSP:            // new for BLE4.2, feature req could be init by slave

            // check if the control packet procedure is is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // yes, so check if it has been transmitted yet
                // Note: This does not mean this packet has been ACK'ed or NACK'ed.
                if ( rfCounters.numTxCtrl )
                {
                    // packet TX'ed, so use this flag on the Slave to indicate that
                    // the feature response procedure has already taken place on this
                    // connection
                    // Note: This is being done to support the HCI extension command
                    //       LL_EXT_SetLocalSupportedFeatures so that the user can
                    //       update the local supported features even after a connection
                    //       is formed. This update will be used as long as a feature
                    //       response feature has not been performed by the Master. Once
                    //       performed, the connection feature set is fixed!
                    connPtr->featureSetInfo.featureRspRcved = TRUE;
                    // ALT: COULD RE-ACTIVATE SL (IF ENABLED) RIGHT HERE.
//            connPtr->slaveLatency = connPtr->slaveLatencyValue;
                    // remove control packet from processing queue and drop through
                    llDequeueCtrlPkt( connPtr );
                }
                else // not done yet
                {
                    // check if a start enc req control procedure timeout has occurred
                    // Note: No need to cleanup control packet info as we are done.
                    if ( --connPtr->ctrlPktInfo.ctrlTimeout == 0 )
                    {
                        // we're done waiting, so end it all
                        // Note: No need to cleanup control packet info as we are done.
                        llConnTerminate( connPtr, LL_CTRL_PKT_TIMEOUT_PEER_TERM );
                        return( LL_CTRL_PROC_STATUS_TERMINATE );
                    }
                    else
                    {
                        //  control packet stays at head of queue, so exit here
                        return( LL_CTRL_PROC_STATUS_SUCCESS );
                    }
                }
            }
            else // control packet has not been put on the TX FIFO yet
            {
                // so try to put it there; being active depends on a success
                // Note: There is no control procedure timeout associated with this
                //       control packet.
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupFeatureSetRsp( connPtr );
                // Note: Two cases are possible:
                //       a) We successfully placed the packet in the TX FIFO.
                //       b) We did not.
                //
                //       In case (a), it may be possible that a previously just
                //       completed control packet happened to complete based on
                //       rfCounters.numTxCtrlAck. Since the current control
                //       procedure is now active, it could falsely detect
                //       rfCounters.numTxCtrlAck, when in fact this was from the
                //       previous control procedure. Consequently, return.
                //
                //       In case (b), the control packet stays at the head of the
                //       queue, and there's nothing more to do. Consequently, return.
                //
                //       So, in either case, return.
                return( LL_CTRL_PROC_STATUS_SUCCESS );
            }

            break;

        /*
        ** Vendor Information Exchange (Request or Reply)
        */
        case LL_CTRL_VERSION_IND:

            // check if the control packet procedure is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // yes, so check if the peer's version information is valid
                if ( connPtr->verExchange.peerInfoValid == TRUE )
                {
                    // yes, so check if the host has requested this information
                    if ( connPtr->verExchange.hostRequest == TRUE )
                    {
                        // yes, so provide it
                        LL_ReadRemoteVersionInfoCback( LL_STATUS_SUCCESS,
                                                       connPtr->connId,
                                                       connPtr->verInfo.verNum,
                                                       connPtr->verInfo.comId,
                                                       connPtr->verInfo.subverNum );
                    }

                    // in any case, dequeue this control procedure
                    llDequeueCtrlPkt( connPtr );
                }
                else // no done yet
                {
                    // check if a update param req control procedure timeout has occurred
                    // Note: No need to cleanup control packet info as we are done.
                    if ( --connPtr->ctrlPktInfo.ctrlTimeout == 0 )
                    {
                        // we're done waiting, so complete the callback with error
                        LL_ReadRemoteVersionInfoCback( LL_CTRL_PKT_TIMEOUT_TERM,
                                                       connPtr->connId,
                                                       connPtr->verInfo.verNum,
                                                       connPtr->verInfo.comId,
                                                       connPtr->verInfo.subverNum );
                        // and end it all
                        // Note: No need to cleanup control packet info as we are done.
                        llConnTerminate( connPtr, LL_CTRL_PKT_TIMEOUT_HOST_TERM );
                        return( LL_CTRL_PROC_STATUS_TERMINATE );
                    }
                    else
                    {
                        //  control packet stays at head of queue, so exit here
                        return( LL_CTRL_PROC_STATUS_SUCCESS );
                    }
                }
            }
            else // control packet has not been put on the TX FIFO yet
            {
                // since we are in the process of sending the version indication,
                // it is okay to set this flag here even if it is set repeatedly
                // in the of llSetupVersionIndReq failures
                connPtr->verExchange.verInfoSent = TRUE;
//          // so try to put it there; being active depends on a success
//          connPtr->ctrlPktInfo.ctrlPktActive = llSetupPingReq(connPtr);// llSetupVersionIndReq( connPtr );
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupVersionIndReq( connPtr );
                // Note: Two cases are possible:
                //       a) We successfully placed the packet in the TX FIFO.
                //       b) We did not.
                //
                //       In case (a), it may be possible that a previously just
                //       completed control packet happened to complete based on
                //       rfCounters.numTxCtrlAck. Since the current control
                //       procedure is now active, it could falsely detect
                //       rfCounters.numTxCtrlAck, when in fact this was from the
                //       previous control procedure. Consequently, return.
                //
                //       In case (b), the control packet stays at the head of the
                //       queue, and there's nothing more to do. Consequently, return.
                //
                //       So, in either case, return.
                return( LL_CTRL_PROC_STATUS_SUCCESS );
            }

            break;

        case LL_CTRL_LENGTH_REQ:

            // check if the control packet procedure is is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // yes, so check if it has been transmitted yet
                // Note: This does not mean this packet has been ACK'ed or NACK'ed.
                if ( rfCounters.numTxCtrl )
                {
                    connPtr->llPduLen.isWatingRsp=TRUE;
                    // remove control packet from processing queue and drop through
                    llDequeueCtrlPkt( connPtr );
                }
                else // not done yet
                {
                    // check if a start enc req control procedure timeout has occurred
                    // Note: No need to cleanup control packet info as we are done.
                    if ( --connPtr->ctrlPktInfo.ctrlTimeout == 0 )
                    {
                        // we're done waiting, so end it all
                        // Note: No need to cleanup control packet info as we are done.
                        llConnTerminate( connPtr, LL_CTRL_PKT_TIMEOUT_PEER_TERM );
                        return( LL_CTRL_PROC_STATUS_TERMINATE );
                    }
                    else
                    {
                        //  control packet stays at head of queue, so exit here
                        return( LL_CTRL_PROC_STATUS_SUCCESS );
                    }
                }
            }
            else // control packet has not been put on the TX FIFO yet
            {
                // so try to put it there; being active depends on a success
                // Note: There is no control procedure timeout associated with this
                //       control packet.
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupDataLenghtReq( connPtr );
                connPtr->llPduLen.isWatingRsp=FALSE;
                // Note: Two cases are possible:
                //       a) We successfully placed the packet in the TX FIFO.
                //       b) We did not.
                //
                //       In case (a), it may be possible that a previously just
                //       completed control packet happened to complete based on
                //       rfCounters.numTxCtrlAck. Since the current control
                //       procedure is now active, it could falsely detect
                //       rfCounters.numTxCtrlAck, when in fact this was from the
                //       previous control procedure. Consequently, return.
                //
                //       In case (b), the control packet stays at the head of the
                //       queue, and there's nothing more to do. Consequently, return.
                //
                //       So, in either case, return.
                return( LL_CTRL_PROC_STATUS_SUCCESS );
            }

            break;

        case LL_CTRL_LENGTH_RSP:

            // check if the control packet procedure is is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // yes, so check if it has been transmitted yet
                // Note: This does not mean this packet has been ACK'ed or NACK'ed.
                if ( rfCounters.numTxCtrl )
                {
                    connPtr->llPduLen.isProcessingReq=FALSE;
                    llPduLengthUpdate((uint16)connPtr->connId);
                    // remove control packet from processing queue and drop through
                    llDequeueCtrlPkt( connPtr );
                }
                else // not done yet
                {
                    // check if a start enc req control procedure timeout has occurred
                    // Note: No need to cleanup control packet info as we are done.
                    if ( --connPtr->ctrlPktInfo.ctrlTimeout == 0 )
                    {
                        // we're done waiting, so end it all
                        // Note: No need to cleanup control packet info as we are done.
                        llConnTerminate( connPtr, LL_CTRL_PKT_TIMEOUT_PEER_TERM );
                        return( LL_CTRL_PROC_STATUS_TERMINATE );
                    }
                    else
                    {
                        //  control packet stays at head of queue, so exit here
                        return( LL_CTRL_PROC_STATUS_SUCCESS );
                    }
                }
            }
            else // control packet has not been put on the TX FIFO yet
            {
                // so try to put it there; being active depends on a success
                // Note: There is no control procedure timeout associated with this
                //       control packet.
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupDataLenghtRsp( connPtr );
                // Note: Two cases are possible:
                //       a) We successfully placed the packet in the TX FIFO.
                //       b) We did not.
                //
                //       In case (a), it may be possible that a previously just
                //       completed control packet happened to complete based on
                //       rfCounters.numTxCtrlAck. Since the current control
                //       procedure is now active, it could falsely detect
                //       rfCounters.numTxCtrlAck, when in fact this was from the
                //       previous control procedure. Consequently, return.
                //
                //       In case (b), the control packet stays at the head of the
                //       queue, and there's nothing more to do. Consequently, return.
                //
                //       So, in either case, return.
                return( LL_CTRL_PROC_STATUS_SUCCESS );
            }

            break;

        // LL PHY UPDATE REQ
        case LL_CTRL_PHY_REQ:

            // check if the control packet procedure is is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // yes, so check if it has been transmitted yet
                // Note: This does not mean this packet has been ACK'ed or NACK'ed.
                if ( rfCounters.numTxCtrl )
                {
                    connPtr->llPhyModeCtrl.isWatingRsp=TRUE;
                    // remove control packet from processing queue and drop through
                    llDequeueCtrlPkt( connPtr );
                }
                else // not done yet
                {
                    // check if a start enc req control procedure timeout has occurred
                    // Note: No need to cleanup control packet info as we are done.
                    if ( --connPtr->ctrlPktInfo.ctrlTimeout == 0 )
                    {
                        // we're done waiting, so end it all
                        // Note: No need to cleanup control packet info as we are done.
                        llConnTerminate( connPtr, LL_CTRL_PKT_TIMEOUT_PEER_TERM );
                        return( LL_CTRL_PROC_STATUS_TERMINATE );
                    }
                    else
                    {
                        //  control packet stays at head of queue, so exit here
                        return( LL_CTRL_PROC_STATUS_SUCCESS );
                    }
                }
            }
            else // control packet has not been put on the TX FIFO yet
            {
                // so try to put it there; being active depends on a success
                // Note: There is no control procedure timeout associated with this
                //       control packet.
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupPhyReq( connPtr );
                connPtr->llPhyModeCtrl.isWatingRsp=FALSE;
                // Note: Two cases are possible:
                //       a) We successfully placed the packet in the TX FIFO.
                //       b) We did not.
                //
                //       In case (a), it may be possible that a previously just
                //       completed control packet happened to complete based on
                //       rfCounters.numTxCtrlAck. Since the current control
                //       procedure is now active, it could falsely detect
                //       rfCounters.numTxCtrlAck, when in fact this was from the
                //       previous control procedure. Consequently, return.
                //
                //       In case (b), the control packet stays at the head of the
                //       queue, and there's nothing more to do. Consequently, return.
                //
                //       So, in either case, return.
                return( LL_CTRL_PROC_STATUS_SUCCESS );
            }

            break;

        case LL_CTRL_PHY_UPDATE_IND:

            // check if the control packet procedure is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // we have already placed a packet on TX FIFO, so check if its been ACK'ed
                if ( rfCounters.numTxCtrlAck )
                {
                    //20181206 ZQ phy update no change case
                    if(     connPtr->phyUpdateInfo.m2sPhy== 0
                            &&  connPtr->phyUpdateInfo.s2mPhy== 0)
                    {
                        connPtr->phyUpdateInfo.m2sPhy=connPtr->llPhyModeCtrl.local.txPhy;
                        connPtr->phyUpdateInfo.s2mPhy=connPtr->llPhyModeCtrl.local.rxPhy;
                        llPhyModeCtrlUpdateNotify(connPtr,LL_STATUS_SUCCESS);
                    }
                    else
                    {
                        // yes, so activate the update
                        connPtr->pendingPhyModeUpdate = TRUE;
                    }

                    connPtr->llPhyModeCtrl.isWatingRsp=FALSE;
                    connPtr->llPhyModeCtrl.isProcessingReq=FALSE;
                    // done with this control packet, so remove from the processing queue
                    llDequeueCtrlPkt( connPtr );
                }
                else // no done yet
                {
                    // Core Spec V4.0 now indicates there is no control procedure
                    // timeout. However, it still seems prudent to monitor for the
                    // instant while waiting for the slave's ACK.
                    if ( connPtr->nextEvent == connPtr->phyModeUpdateEvent )
                    {
                        // this event is the instant, and the control procedure still
                        // has not been ACK'ed, we the instant has passed
                        // Note: No need to cleanup control packet info as we are done.
                        llConnTerminate( connPtr, LL_CTRL_PKT_INSTANT_PASSED_HOST_TERM );
                        return( LL_CTRL_PROC_STATUS_TERMINATE );
                    }
                    else // continue waiting for the slave's ACK
                    {
                        //  control packet stays at head of queue, so exit here
                        return( LL_CTRL_PROC_STATUS_SUCCESS );
                    }
                }
            }
            else // control packet has not been put on the TX FIFO yet
            {
                // so try to put it there; being active depends on a success
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupPhyUpdateInd( connPtr );
                // Note: Two cases are possible:
                //       a) We successfully placed the packet in the TX FIFO.
                //       b) We did not.
                //
                //       In case (a), it may be possible that a previously just
                //       completed control packet happened to complete based on
                //       rfCounters.numTxCtrlAck. Since the current control
                //       procedure is now active, it could falsely detect
                //       rfCounters.numTxCtrlAck, when in fact this was from the
                //       previous control procedure. Consequently, return.
                //
                //       In case (b), the control packet stays at the head of the
                //       queue, and there's nothing more to do. Consequently, return.
                //
                //       So, in either case, return.
                return( LL_CTRL_PROC_STATUS_SUCCESS );
            }

            break;

        // REJECT EXT IND --> PHY UPDATE COLLSION
        case LL_CTRL_REJECT_EXT_IND:

            // check if the control packet procedure is is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // yes, so check if it has been transmitted yet
                // Note: This does not mean this packet has been ACK'ed or NACK'ed.
                if ( rfCounters.numTxCtrl )
                {
                    connPtr->isCollision=TRUE;
                    // remove control packet from processing queue and drop through
                    llDequeueCtrlPkt( connPtr );
                }
                else // not done yet
                {
                    // check if a start enc req control procedure timeout has occurred
                    // Note: No need to cleanup control packet info as we are done.
                    if ( --connPtr->ctrlPktInfo.ctrlTimeout == 0 )
                    {
                        // we're done waiting, so end it all
                        // Note: No need to cleanup control packet info as we are done.
                        llConnTerminate( connPtr, LL_CTRL_PKT_TIMEOUT_PEER_TERM );
                        return( LL_CTRL_PROC_STATUS_TERMINATE );
                    }
                    else
                    {
                        //  control packet stays at head of queue, so exit here
                        return( LL_CTRL_PROC_STATUS_SUCCESS );
                    }
                }
            }
            else // control packet has not been put on the TX FIFO yet
            {
                if(connPtr->llPhyModeCtrl.isWatingRsp==TRUE)
                {
                    connPtr->ctrlPktInfo.ctrlPktActive = llSetupRejectExtInd( connPtr,LL_STATUS_ERROR_LL_PROCEDURE_COLLISION);
                }
                else if(connPtr->pendingChanUpdate==TRUE ||
                        connPtr->pendingParamUpdate==TRUE )
                {
                    connPtr->ctrlPktInfo.ctrlPktActive = llSetupRejectExtInd( connPtr,LL_STATUS_ERROR_DIFF_TRANSACTION_COLLISION);
                }
                else if( connPtr->llCTEModeCtrl.isWatingRsp == TRUE)
                {
                    // 2020-01-23 add for CTE
                    connPtr->ctrlPktInfo.ctrlPktActive = llSetupRejectExtInd( connPtr,connPtr->llCTEModeCtrl.errorCode );
                    connPtr->llCTEModeCtrl.errorCode = LL_STATUS_SUCCESS;
                }
                else
                {
                    //should not be here
                }

                return( LL_CTRL_PROC_STATUS_SUCCESS );
            }

            break;

        case LL_CTRL_CTE_REQ:

            // check if the control packet procedure is is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // yes, so check if it has been transmitted yet
                // Note: This does not mean this packet has been ACK'ed or NACK'ed.
                if ( rfCounters.numTxCtrl )
                {
                    //  connPtr->llCTEModeCtrl.isWatingRsp = TRUE;
                    // remove control packet from processing queue and drop through
                    llDequeueCtrlPkt( connPtr );
                }
                else // not done yet
                {
                    //
                    if ( --connPtr->ctrlPktInfo.ctrlTimeout == 0 )
                    {
                        osal_memset( &(connPtr->llCTEModeCtrl), 0, sizeof( connPtr->llCTEModeCtrl ));
                        // we're done waiting, so end it all
                        // Note: No need to cleanup control packet info as we are done.
                        llConnTerminate( connPtr, LL_CTRL_PKT_TIMEOUT_PEER_TERM );
                        return( LL_CTRL_PROC_STATUS_TERMINATE );
                    }
                    else
                    {
                        //  control packet stays at head of queue, so exit here
                        return( LL_CTRL_PROC_STATUS_SUCCESS );
                    }
                }
            }
            else // control packet has not been put on the TX FIFO yet
            {
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupCTEReq( connPtr );
                connPtr->llCTEModeCtrl.isWatingRsp = TRUE;
                return( LL_CTRL_PROC_STATUS_SUCCESS );
            }

            break;

        case LL_CTRL_CTE_RSP:

            // check if the control packet procedure is is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // yes, so check if it has been transmitted yet
                // Note: This does not mean this packet has been ACK'ed or NACK'ed.
                if ( rfCounters.numTxCtrl )
                {
                    connPtr->llCTEModeCtrl.isProcessingReq = FALSE;
                    // remove control packet from processing queue and drop through
                    llDequeueCtrlPkt( connPtr );
                }
                else // not done yet
                {
                    return( LL_CTRL_PROC_STATUS_TERMINATE );
                }
            }
            else // control packet has not been put on the TX FIFO yet
            {
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupCTERsp( connPtr );
                return( LL_CTRL_PROC_STATUS_SUCCESS );
            }

            break;

        /*
        ** Unknown Control Type Response
        */
        case LL_CTRL_UNKNOWN_RSP:

            // try to place control packet in the TX FIFO
            // Note: Since there are no dependencies for this control packet, we
            //       do not have to bother with the active flag.
            if ( llSetupUnknownRsp( connPtr ) == TRUE )
            {
                // all we have to do is put this control packet on the TX FIFO, so
                // remove control packet from the processing queue and drop through
                llDequeueCtrlPkt( connPtr );
            }
            else // not done yet
            {
                // control packet stays at head of queue, so exit here
                return( LL_CTRL_PROC_STATUS_SUCCESS );
            }

            break;

        /*
        ** Control Internal - Wait for Control ACK
        */
        case LL_CTRL_TERMINATE_RX_WAIT_FOR_TX_ACK:

            // check if the control packet has been ACK'ed (i.e. is not pending)
            // Note: Normally this routine is used for control procedures where
            //       control packets are sent by this role. This is a special case
            //       where a terminate indication was received, but we must as a
            //       master wait for our ACK to be sent before terminating.
            if ( rfCounters.numTxCtrlAck == 1)              // ctrl packet has been acked
            {
                // yes, so terminate
                // Note: No need to cleanup control packet info as we are done.
                llConnTerminate( connPtr, connPtr->termInfo.reason );
                return( LL_CTRL_PROC_STATUS_TERMINATE );
            }

            // control packet stays at head of queue, so exit here
            return( LL_CTRL_PROC_STATUS_SUCCESS );

        // Note: Unreachable statement generates compiler warning!
        //break;

        // Dummy Place Holder
        //case LL_CTRL_DUMMY_PLACE_HOLDER:
        //  //  dummy packet stays at head of queue, so exit here
        //  return( LL_CTRL_PROC_STATUS_SUCCESS );
        // Note: Unreachable statement generates compiler warning!
        //break;
        default:
            #ifdef DEBUG
            // fatal error - a unknown control procedure value was used
            LL_ASSERT( FALSE );
            #endif // DEBUG
            break;
        }
    }

    return( LL_CTRL_PROC_STATUS_SUCCESS );
}

//#pragma O0
// A2 multi-connection
uint8 ll_processMissMasterEvt(uint8 connId)
{
    llConnState_t* connPtr;
//    LOG("-M ");
    connPtr = &conn_param[connId];
    connPtr->rx_crcok = 0;
    connPtr->rx_timeout = 1;
    connPtr->pmCounter.ll_conn_event_cnt ++;
    connPtr->pmCounter.ll_conn_event_timeout_cnt ++;
    connPtr->pmCounter.ll_miss_master_evt_cnt ++;

    if(p_perStatsByChan!=NULL)
        p_perStatsByChan->rxToCnt[connPtr->currentChan]++;

    // Tx done OK counter
    rfCounters.numTxDone = 0;
    // update the numTxCtrlAck counter, add on 2017-11-15
    rfCounters.numTxCtrlAck = 0;
    // this counter is set in function LL_master_conn_event() used in function llProcessMasterControlProcedures()
    rfCounters.numTxCtrl = 0;
    // advance the connection event count
    connPtr->currentEvent = connPtr->nextEvent;
    connPtr->perInfo.numEvents++;
    connPtr->perInfo.numMissedEvts++;

    // check if we have a Supervision Timeout
    if ( connPtr->expirationEvent <= connPtr->currentEvent )     // 2019-7-17, change from '==' to '<='
    {
        // check if the connection has already been established
        if ( connPtr->firstPacket == 0 )
        {
            // yes, so terminate with LSTO
            llConnTerminate( connPtr, LL_SUPERVISION_TIMEOUT_TERM );
        }
        else // no, so this is a failure to establish the connection
        {
            // so terminate immediately with failure to establish connection
            llConnTerminate( connPtr, LL_CONN_ESTABLISHMENT_FAILED_TERM );
        }

        return LL_PROC_LINK_TERMINATE;
    }

    // no Rx packet

    /*
    ** Check Control Procedure Processing
    */
    if ( llProcessMasterControlProcedures( connPtr ) == LL_CTRL_PROC_STATUS_TERMINATE )
    {
        // this connection is terminated, so nothing to schedule
        return LL_PROC_LINK_TERMINATE;
    }

    /*
    ** Process TX Data Packets, no new data for miss event
    */

    /*
    ** Setup Next master Event Timing
    */

    // update next event, calculate time to next event, calculate timer drift,
    // update anchor points, setup NR T2E1 and T2E2 events
    if ( llSetupNextMasterEvent() == LL_SETUP_NEXT_LINK_STATUS_TERMINATE )  // PHY+ always return success here
    {
        // this connection is terminated, so nothing to schedule
        return LL_PROC_LINK_TERMINATE;
    }

    // update scheduler information
    g_ll_conn_ctx.scheduleInfo[connId].remainder += (connPtr->curParam.connInterval + connUpdateTimer) * 625;

    // connection event notify
    if (g_conn_taskID != 0)
        osal_set_event(g_conn_taskID, g_conn_taskEvent);

    return LL_PROC_LINK_KEEP;
}




/*******************************************************************************
*/
