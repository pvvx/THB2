/*******************************************************************************
    Filename:       ll_SlaveEndCauses.c
    Revised:
    Revision:

    Description:    This file contains the Link Layer (LL) handlers for the
                  various slave end causes that result from a PHY task
                  completion. The file also contains the slave end causes
                  related to connection termination, as well as common
                  termination routines used by the slave.

 SDK_LICENSE

*******************************************************************************/

/*******************************************************************************
    INCLUDES
*/



#include "ll_def.h"
#include "ll.h"
#include "ll_common.h"
#include "timer.h"

#include "bus_dev.h"
#include "ll_enc.h"
#include "jump_function.h"
#include "global_config.h"
#include "ll_hw_drv.h"


/*******************************************************************************
    MACROS
*/

/*******************************************************************************
    CONSTANTS
*/

#define MAX_LSTO_IN_COARSE_TICKS  51200  // 32s in 625us
#define MIN_CI_IN_COARSE_TICKS    12     // 7.5ms in 625us
#define MAX_LSTO_NUM_OF_EVENTS    (MAX_LSTO_IN_COARSE_TICKS / MIN_CI_IN_COARSE_TICKS)

/*******************************************************************************
    EXTERNAL VARIABLES
*/
extern uint32_t ISR_entry_time;
extern int slave_conn_event_recv_delay;

extern  uint32_t  g_smartWindowSize;
extern  uint8_t   g_smartWindowRTOCnt;
extern  uint8    g_conn_taskID;
extern  uint16   g_conn_taskEvent;
extern perStatsByChan_t* p_perStatsByChan;

///// EXTERNAL FUNCTION


/*******************************************************************************
    Prototypes
*/


/*******************************************************************************
    Functions
*/
void ll_adptive_adj_next_time(uint32_t nextTime);

/*******************************************************************************
    @fn          llSlaveEvt_TaskEndOk0

    @brief       This function is used to handle the PHY task done end cause
                TASK_ENDOK that can result from one of two causes. First, a
                normal connection event closure occurred since the Slave
                transmitted a packet with MD=0 (i.e. no more Slave data) after
                having successfully received a packet from the Master with MD=0
                (i.e. no more Master data). Second, the Slave transmitted a
                packet and the BLE_L_CONF.ENDC=1. Since the ENDC bit is
                currently not used, only the first case is handled.

                This function needs to read and save the anchor point, which is
                used for timing the next connection event. All RX FIFO data
                (data and control packets) are to be counted (for encyrtion)
                and processed. All data that needs to be sent are placed in the
                TX FIFO. The connectin event is counted, and the start of slave
                latency is checked. If the connection hasn't been terminated
                via a control packet request, the next data channel is selected,
                and the timing for the start of the next Slave tasks it
                determined based on the connection interval, relative to the
                anchor point.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
void llSlaveEvt_TaskEndOk0( void )
{
    llConnState_t* connPtr;
    uint32_t next_time;
    uint32   sw_delay, T2;
    int      calibra_time, i;                          // this parameter will be calibrate provided by global_config

    // check if the connection is still valid
    if (FALSE == conn_param[g_ll_conn_ctx.currentConn].active)
    {
        // connection may have already been ended by a reset
        // HZF: if other procedure terminate the link, it should schedule the next event
        return;
    }

    // get connection information
    connPtr = &conn_param[g_ll_conn_ctx.currentConn];
    // advance the connection event count
    connPtr->currentEvent = connPtr->nextEvent;

    // check if any data has been received
    // Note: numRxOk includes numRxCtrl
    // Note: numRxNotOk removed as 4.5.2 of spec says the LSTO is reset upon
    //       receipt of a "valid packet", which is taken to mean no CRC error.
    if ( rfCounters.numRxOk  || rfCounters.numRxIgnored ||        // we have only "numRxOk"
            rfCounters.numRxEmpty || rfCounters.numRxFifoFull
            || connPtr->rx_crcok != 0)     // ever Rx CRC OK packet
    {
        // yes, so update the supervision expiration count
        connPtr->expirationEvent = connPtr->currentEvent + connPtr->expirationValue;
        // clear flag that indicates we received first packet
        // Note: The first packet only really needs to be signalled when a new
        //       connection is formed or a connection's parameters are updated.
        //       However, there's no harm in resetting it every time in order to
        //       simplify the control logic.
        // Note: True-Low logic is used here to be consistent with nR's language.
        connPtr->firstPacket = 0;

        // slave latency may have been disabled because a packet from the master
        // was not received (see Core spec V4.0, Vol 6, Section 4.5.1), so restore
        // the slave latency value; but note that if this is the beginning of a
        // connection, you can not allow slave latency until the first NESN change
        // has occurred from the master (i.e. until master ACK's slave)
        if ( connPtr->slaveLatencyAllowed == TRUE )
        {
            // activate slave latency
            connPtr->slaveLatency = connPtr->slaveLatencyValue;
        }

        //receiver ack notifty the host
        if(connPtr->llPhyModeCtrl.isChanged==TRUE)
        {
            connPtr->llPhyModeCtrl.isChanged = FALSE;
            llPhyModeCtrlUpdateNotify(connPtr,LL_STATUS_SUCCESS);
        }
    }
    else // either no packets received, or packets received with CRC error
    {
        // check if we received any packets with a CRC error
        // Note: Spec change (section 4.5.5) indicates any packet received,
        //       regardless of CRC result, determines the anchor point.
        if (connPtr->rx_timeout)     // no packet was received from the master
        {
            // collect packet error information
            connPtr->perInfo.numMissedEvts++;
            // so listen to every event until a packet is received
            connPtr->slaveLatency = 0;
        }
        else   //   ( rfCounters.numRxNotOk )  // different to TI sequence
        {
            // clear flag that indicates we received first packet
            // Note: The first packet only really needs to be signalled when a new
            //       connection is formed or a connection's parameters are updated.
            //       However, there's no harm in resetting it every time in order to
            //       simplify the control logic.
            // Note: True-Low logic is used here to be consistent with nR's language.
            connPtr->firstPacket = 0;

            // slave latency may have been disabled because a packet from the master
            // was not received (see Core spec V4.0, Vol 6, Section 4.5.1), so restore
            // the slave latency value; but note that if this is the beginning of a
            // connection, you can not allow slave latency until the first NESN change
            // has occurred from the master (i.e. until master ACK's slave)
            if ( connPtr->slaveLatencyAllowed == TRUE )
            {
                // activate slave latency
                connPtr->slaveLatency = connPtr->slaveLatencyValue;
            }
        }

        // check if we have a Supervision Timeout
        if ( connPtr->expirationEvent == connPtr->currentEvent )
        {
            // check if either we already got the first packet in a connection or
            // if this isn't the quick LSTO associated with connection establishment
            // Note: The Slave reuses firstPacket when a Update Parameter control
            //       procedure is started, but in that case, the expiration event
            //       will be well past the event associated with connection
            //       establishement.
            if ( (connPtr->firstPacket == 0) ||
                    (connPtr->currentEvent != LL_LINK_SETUP_TIMEOUT) )
            {
                // yes, so terminate with LSTO
                llConnTerminate( connPtr, LL_SUPERVISION_TIMEOUT_TERM );
                g_pmCounters.ll_link_lost_cnt ++;
            }
            else // this is a failure to establish the connection
            {
                // so terminate immediately with failure to establish connection
                llConnTerminate( connPtr, LL_CONN_ESTABLISHMENT_FAILED_TERM );
                g_pmCounters.ll_link_estab_fail_cnt ++;
            }

//#ifdef MULTI_ROLE
            ll_scheduler(LL_INVALID_TIME);
//#endif
            return;
        }
    }

    // for a new connection, slave latency isn't enabled until the master's NESN
    // bit changes, which is equivalent to receiving a TX ACK; if this is an
    // update parameters, slave latency is also disabled until any first packet arrives,
    // however, to keep things simple for now, we will use the same ACK constraint
    if (rfCounters.numTxAck > 0)//connPtr->firstPacket == 0)   /// TODO: test the scenario
    {
        // set a flag to indicates it is now okay to use slave latency on this
        // connection, if specified
        // Note: This is now needed due to a change in spec (section 4.5.1) which
        //       requires that slave latency be disabled when a packet is not
        //       received from the master. Since this routine is common for END_OK
        //       and RX_TIMEOUT, there's no way to know if the first Master NESN
        //       bit change has taken place. This flag will indicate it has, so if
        //       a master packet is received, the slave latency value can be
        //       restored.
        if (pGlobal_config[LL_SWITCH] & SLAVE_LATENCY_ALLOW)
            connPtr->slaveLatencyAllowed = TRUE;

        // only update slave latency if no control procedure is active
        // Note: When a control procedure is active, slave latency has to be
        //       disabled in case it exceeds the control procedure timeout.
        // Note: Even when a control procedure is active, but a control transaction
        //       timeout isn't used, we can still skip setting SL since that kind
        //       of control procedure wouldn't have disabled slave latency to begin
        //       with.
        // ALT: COULD RESET SL IN llProcessSlaveControlProcedures.
        if ( (connPtr->ctrlPktInfo.ctrlPktActive == FALSE) &&
                (connPtr->pendingParamUpdate == FALSE)        &&
                (connPtr->pendingChanUpdate  == FALSE)        &&
                (connPtr->pendingPhyModeUpdate  == FALSE) )
        {
            // at least one ACK, so Slave Latency is operational
            // Note: This really only happens once for the very first connection
            //       interval, and when an update parameters procedure is taking place.
            connPtr->slaveLatency = connPtr->slaveLatencyValue;
        }
    }

    // check if the nR performed a anchor point capture
    // Note: This bit is cleared at the start of a new task.
    // Note: The anchor capture will occur even if the RX FIFO was too full to
    //       accept the packet.
//  if (connPtr->connected == 0)//rx_timeout ==1 )      // update by HZF 05-03, if not connected, disable slave latency
    if (connPtr->firstPacket)                             // update by HZF 12-18, if not receive 1st packet(CRC OK or not), disable slave latency
    {
        connPtr->slaveLatency = 0;
    }

    /*
    ** Process RX Data Packets
    */
    uint8_t  buffer_size;
    buffer_size = getRxBufferSize(connPtr);

    if (buffer_size > 0)                 // 2018-10-22, disable slave latency if receives some data/ctrl packet
        connPtr->slaveLatencyAllowed = FALSE;

    for ( i = 0; i < buffer_size; i ++)     // note: i < getRxBufferSize()  will fail the loop
    {
        // there is, so process it; check if data was processed
        if ( llProcessRxData() == FALSE )
        {
            // it wasn't, so we're done
            break;
        }
    }

    // check if this connection was terminated
    // HZF: when llProcessRxData(), the link may be terminated
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
    if ( llProcessSlaveControlProcedures( connPtr ) == LL_CTRL_PROC_STATUS_TERMINATE )
    {
//#ifdef MULTI_ROLE
        ll_scheduler(LL_INVALID_TIME);
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

    if (connPtr->rx_timeout)
        connPtr->accuTimerDrift += connPtr->timerDrift;
    else
        connPtr->accuTimerDrift = 0;

    /*
    ** Setup Next Slave Event Timing
    */

    // update next event, calculate time to next event, calculate timer drift,
    // update anchor points, setup NR T2E1 and T2E2 events
    if ( llSetupNextSlaveEvent() == LL_SETUP_NEXT_LINK_STATUS_TERMINATE )
    {
//#ifdef MULTI_ROLE
        ll_scheduler(LL_INVALID_TIME);
//#endif
        return;
    }

    // schedule next connection event
    // calculate the timer drift due to slave latency
//        llCalcTimerDrift(conn_param[connId].curParam.connInterval,
//                           conn_param [connId].slaveLatency,
//                           conn_param [connId].scaFactor,
//                           (uint32 *)&conn_param [connId].timerDrift);       //
    // calibrate time:
    // 100 for SW process, 100 for timing advance, 60 for hw engine startup
    calibra_time = 100 + 100 + 60 ;
    // calculate the delay
    T2 = read_current_fine_time();
    sw_delay = (T2 > ISR_entry_time) ? (T2 - ISR_entry_time) : (BASE_TIME_UNITS - ISR_entry_time + T2);
    // schedule next connection event time
    //  <start time> ------> <anchor point> -----> <loop time>(Interrupt trigger) ------> <next conn event time>
    //  note that slaveLatency should be 0,i.e. no latency if there are data to send in slave
    // should not use conn_param[connId].curParam.connInterval, lastTimeToNextEvt also cover conn parameter update case
    next_time = connPtr->lastTimeToNextEvt * (connPtr->lastSlaveLatency + 1) * 625;
    // rx -> anchor: 110us
    //next_time = next_time - slave_conn_event_recv_delay - sw_delay - 110 - calibra_time - conn_param[connId].timerDrift ;
    //used to adj next time for multi-link
    ll_adptive_adj_next_time(next_time);
    int32_t adj_time = slave_conn_event_recv_delay + sw_delay + 110 + calibra_time + connPtr->timerDrift;
    next_time = (next_time > adj_time || adj_time < 0) ? (next_time - adj_time) : 200;
//#ifdef MULTI_ROLE
    ll_scheduler(next_time);
//#else
//    ll_schedule_next_event(next_time);
//#endif
    return;
}




/*******************************************************************************
    @fn          llSetupNextSlaveEvent0

    @brief       This function is used to setup the next Timer 2 Event 1 and
                Event 2 times, checks if an Update Parameters and/or an
                Update Data Channel has occurred, and sets the next data
                channel.


    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      Indicates whether a link terminate occurred:
                LL_SETUP_NEXT_LINK_STATUS_TERMINATE: Terminate due to a LSTO.
                LL_SETUP_NEXT_LINK_STATUS_SUCCESS: Do not terminate.
*/
uint8 llSetupNextSlaveEvent0( void )
{
    llConnState_t* connPtr;
    uint32         timeToNextEvt;
    // get pointer to connection info
    connPtr = &conn_param[g_ll_conn_ctx.currentConn];

    // check if there's any TX data pending, and if so, disable slave latency
    if ( (getTxBufferSize(connPtr) > 0)                        // data buffer
            || (connPtr->txDataQ.head != NULL)              // data queue
            || connPtr->ll_buf.tx_not_ack_pkt->valid                     // not ack buffer
            || (connPtr->ll_buf.ntrm_cnt != 0)                         // not transmit buffer
            || (connPtr->rx_timeout)                         // add 2018-8-21, disable slave latency in Timeout case
            || (connPtr->slaveLatencyAllowed == FALSE))      // 2018-10-22, if slave latency not allow, set latency = 0
    {
        connPtr->slaveLatency = 0;
    }
    else // restore slave latency
    {
        // but only if enabled and no updates or control procedures are pending
        // and the first NESN change has occurred from the master (i.e. the Master
        // has ACK'ed the Slave)
        if ( (connPtr->ctrlPktInfo.ctrlPktActive == FALSE) &&
                (connPtr->pendingParamUpdate == FALSE)        &&
                (connPtr->pendingChanUpdate  == FALSE)        &&
                (connPtr->pendingPhyModeUpdate  == FALSE)     &&
                (connPtr->slaveLatencyAllowed == TRUE) )
        {
            // activate slave latency
            connPtr->slaveLatency = connPtr->slaveLatencyValue;
        }
    }

    // update the next connection event count, taking slave latency into account
    connPtr->nextEvent = connPtr->currentEvent + connPtr->slaveLatency + (uint8)1;

    // check if the supervision timeout is going to occur while we are in slave
    // latency, or the user has suspended slave latency
    // Note: The check for LSTO during slave latency requires that nextEvent was
    //       already updated based on slave latency!
    if ( (llCheckForLstoDuringSL( connPtr ) == TRUE)  )
    {
        // yes, so disable slave latency
        connPtr->slaveLatency = 0;
        // update the next connection event count without slave latency
        connPtr->nextEvent = connPtr->currentEvent + (uint8)1;
    }

    /*
    ** Check for a Parameter Update
    **
    ** Note: The Parameters Update must come before the Data Channel Update in
    **       case the next event count is changed.
    */

    // check if there's a connection parameter udpate to the connection, and if
    // so, check if the update event is before or equal to the next active event
    if ( (connPtr->pendingParamUpdate == TRUE)  &&
            (llEventInRange( connPtr->currentEvent,
                             connPtr->nextEvent,
                             connPtr->paramUpdateEvent)) )
    {
        // override the next active event to be the update event
        connPtr->nextEvent = connPtr->paramUpdateEvent;
        // convert the new LSTO from time to an expiration connection event count
        llConvertLstoToEvent( connPtr, &connPtr->paramUpdate );
        // make the update slave latency the next latency to use
        connPtr->slaveLatencyValue = connPtr->paramUpdate.slaveLatency;
        // slave latency is not allowed until the first data packet is received
        connPtr->slaveLatencyAllowed = FALSE;
        // for update parameters, deactivate slave latency until first packet arrives
        // Note: The spec isn't clear if slave latency is to be used immediately or
        //       not, but it seems awkward to allow it before we've even received
        //       the first packet.
        // Note: Assuming we wait for the first packet to re-enable slave latency
        //       at the possibly new value, the spec places no restrictions on
        //       requiring an ACK from the Master. However, to keep the TaskDone
        //       processing generic, the same rules will be applied. This does not
        //       violate the spec as the Slave is not required to use slave latency.
        connPtr->slaveLatency = 0;
        // set a flag that indicates when a first data packet was received
        // Note: This is used for RX timeouts in llSetupNextSlaveEvent to add the
        //       window size to the receive window until the first packet is
        //       received.
        connPtr->firstPacket = 1;
        // update the LSTO expiration count
        // Note: This is needed in case the new connection fails to receive a
        //       packet and a RXTIMEOUT results. The expiration count must
        //       already be initialized based on the new event values.
        connPtr->expirationEvent = connPtr->nextEvent + connPtr->expirationValue;
        // find the number of events between this event and the update parameter
        // event, based on the original connection interval
        // Note: The old connection interval must be used!
        timeToNextEvt = ( (uint32)llEventDelta( connPtr->paramUpdateEvent,
                                                connPtr->currentEvent ) *
                          (uint32)connPtr->curParam.connInterval ) +
                        (uint32)connPtr->paramUpdate.winOffset;

        // notify the Host if connInterval, connTimeout, or slaveLatency
        // has been changed by the master
        if ( (connPtr->paramUpdate.connInterval != connPtr->curParam.connInterval)  ||
                (connPtr->paramUpdate.connTimeout  != connPtr->curParam.connTimeout )  ||
                (connPtr->paramUpdate.slaveLatency != connPtr->curParam.slaveLatency) )
        {
            // notify the Host
            // Note: The values are kept in units of 625us, so they must be
            //       converted back to what's used at the interface.
            LL_ConnParamUpdateCback( (uint16)connPtr->connId,
                                     connPtr->paramUpdate.connInterval >> 1,
                                     connPtr->paramUpdate.slaveLatency,
                                     connPtr->paramUpdate.connTimeout >> 4 );
        }

        // update the current parameters from the new parameters
        // Note: The new parameter connTimeout is not needed since once it is
        //       converted to connection events, its value is maintained by
        //       expirationValue. However, in order to compare to the last received
        //       updates, this value must be copied as well. The new parameter
        //       winSize is only needed in case there's a RX timeout.
        connPtr->curParam.winSize   = connPtr->paramUpdate.winSize;
        connPtr->curParam.connInterval  = connPtr->paramUpdate.connInterval;
        connPtr->curParam.slaveLatency  = connPtr->paramUpdate.slaveLatency;
        connPtr->curParam.connTimeout   = connPtr->paramUpdate.connTimeout;
        llAdjSlaveLatencyValue(connPtr);
        // convert the Control Procedure timeout into connection event count
        llConvertCtrlProcTimeoutToEvent( connPtr );
        // clear the pending flag
        connPtr->pendingParamUpdate = FALSE;
    }
    else // no parameter update, so...
    {
        // time to next event is just the connection interval
        // Note: Slave latency will be taken into account below.
        timeToNextEvt = connPtr->curParam.connInterval;
        // update by HZF: consider slave latency
//        timeToNextEvt = connPtr->curParam.connInterval * (connPtr->slaveLatency + 1);
    }

    // calculate/recalculate timer drift conditionally
    // Note: It is only necessary to recalculate timer drift if the time to next
    //       event changes. This is based on the connection interval and slave
    //       latency. This can happen when the connection is formed, from an
    //       update parameter control procedure, or from anything that affects
    //       whether slave latency is to be used (assuming it is non-zero). For
    //       example, whether or not there is data to transmit.
    if ( ((connPtr->lastTimeToNextEvt != timeToNextEvt) ||
            (connPtr->lastSlaveLatency != connPtr->slaveLatency)) )
    {
        // calculate timer drift correction, in coarse and fine timer ticks
        // Note: This routine needs the time to the next event only. It will use
        //       the slave latency value to adjust for slave latency.
        // Note: If power saving is enabled, this routine will find the timer drift
        //       by also taking our own device's timer drift into account.
        llCalcTimerDrift(timeToNextEvt,
                         connPtr->slaveLatency,
                         connPtr->sleepClkAccuracy,
                         (uint32*)&(connPtr->timerDrift));
    }

    // save time to next event for next connection event
    connPtr->lastTimeToNextEvt = timeToNextEvt;
    // update last slave latency used
    connPtr->lastSlaveLatency = connPtr->slaveLatency;
    llSetNextDataChan( connPtr );
    llSetNextPhyMode( connPtr );
    return( LL_SETUP_NEXT_LINK_STATUS_SUCCESS );
}


/*******************************************************************************
    @fn          llCheckForLstoDuringSL0

    @brief       This function is used to determine if a link supervision timeout
                (LSTO) is going to occur during slave latency. Typically, the
                expiration event ought to remain ahead of the next event unless
                a LSTO is going to occur. This routine is used after the next
                event is updated, taking slave latency into account, to find out
                if the LSTO will result during the ignored events. This is done
                by checking if the next event advances past the expiration
                event, taking wrap into consideration.

                Note: Next event can not advance more than the max slave latency
                      value, and expiration event can not advance more than the
                      max LSTO. Since the event counter is 64K, rather than
                      finding the number of events between the two in
                      consecutive order, the difference between the two will be
                      found instead. If this difference is larger than max
                      possible difference, then a wrap has taken place. This
                      saves the extra calculation otherwise needed.

    input parameters

    @param       connPtr - Pointer to the current connection.

    output parameters

    @param       None.

    @return      Indicates whether a supervision timeout is going to occur
                during slave latency:
                TRUE:  Terminate due to a LSTO.
                FALSE: Do not terminate.
*/
uint8 llCheckForLstoDuringSL0( llConnState_t* connPtr )
{
    // first check if the next event is after the expiration event
    if ( connPtr->expirationEvent < connPtr->nextEvent )
    {
        // yes, but check if that's only because expiration event wrapped around
        // Note: The delta is taken instead of finding the number of events between
        //       as this calculation is faster.
        if ( (connPtr->nextEvent - connPtr->expirationEvent) < MAX_LSTO_NUM_OF_EVENTS )
        {
            // expiration event did not wrap and is actually behind next event
            return( TRUE );
        }
    }
    else // expiration event is ahead of or equal to the next event
    {
        // so check if expiration event is only ahead because next event wrapped
        if ( (connPtr->expirationEvent - connPtr->nextEvent) > MAX_LSTO_NUM_OF_EVENTS )
        {
            // next event did wrap and is actually ahead of expiration event
            return( TRUE );
        }
    }

    return( FALSE );
}

/*******************************************************************************
    @fn          llProcessSlaveControlProcedures0

    @brief       This function is used to process any control procedures that
                may be active.

                Note: There can only be one active control procedure at a time.

                Note: It is assumed the NR counters have been updated at the
                      end of the task before calling this routine.

    input parameters

    @param       connPtr - Pointer to the current connection.

    output parameters

    @param       None.

    @return      Status of control procedure processing, which can be:
                LL_CTRL_PROC_STATUS_SUCCESS: Continue normally.
                LL_CTRL_PROC_STATUS_TERMINATE: We have terminated.
*/
uint8 llProcessSlaveControlProcedures0( llConnState_t* connPtr )
{
    // check if there are any control packets ready for processing
    while ( connPtr->ctrlPktInfo.ctrlPktCount > 0 )
    {
        // processing based on control packet type at the head of the queue
        switch( connPtr->ctrlPktInfo.ctrlPkts[ 0 ] )
        {
        case LL_CTRL_TERMINATE_IND:

            // check if the control packet procedure is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // we have already place packet on TX FIFO, so check if its been ACK'ed
                if ( rfCounters.numTxCtrlAck )
                {
                    //  yes, so process the termination
                    // Note: No need to cleanup control packet info as we are done.
                    llConnTerminate( connPtr, LL_HOST_REQUESTED_TERM );
                    return( LL_CTRL_PROC_STATUS_TERMINATE );
                }
                else // no done yet
                {
                    // check if a termination control procedure timeout has occurred
                    if ( --connPtr->ctrlPktInfo.ctrlTimeout == 0 )
                    {
                        // we're done waiting, so end it all
                        // Note: No need to cleanup control packet info as we are done.
                        llConnTerminate( connPtr, LL_CTRL_PKT_TIMEOUT_HOST_TERM );
                        return( LL_CTRL_PROC_STATUS_TERMINATE );
                    }
                    else // no control procedure timeout yet
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

        case LL_CTRL_ENC_RSP:

            // check if the control packet procedure is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // yes, so check if it has been transmitted yet
                // Note: This does not mean this packet has been ACK'ed or NACK'ed.
                if ( rfCounters.numTxCtrl )
                {
                    // done with this control packet, so remove from the processing queue
                    // Note: By dequeueing here, it is possible to get another control
                    //       packet at the head of the queue. This is techincally not
                    //       supposed to happen if the spec is followed.
                    // ALT: COULD MAKE MORE BULLET PROOF. SINCE THE REPLACE ROUTINE
                    //      CAN'T BE USED UNTIL THE LTK IS RECEIVED BY THE HOST, A
                    //      DUMMY CONTROL PACKET THAT SITS AT THE HEAD UNTIL IT IS
                    //      REPLACE COULD BE USED INSTEAD.
                    //llReplaceCtrlPkt( connPtr, LL_CTRL_DUMMY_PLACE_HOLDER );
                    llDequeueCtrlPkt( connPtr );
                    // notify the Host with RAND and EDIV after sending the RSP
                    // Note: Need to wait for the Host reply to determine if the LTK
                    //       is available or not.
                    LL_EncLtkReqCback( connPtr->connId,
                                       connPtr->encInfo.RAND,
                                       connPtr->encInfo.EDIV );
                }
                else // not done yet
                {
                    // check if a update param req control procedure timeout has occurred
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
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupEncRsp( connPtr );
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

        case LL_CTRL_START_ENC_REQ:

            // check if the control packet procedure is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // yes, so check if it has been transmitted yet
                // Note: This only means the packet has been transmitted, not that it
                //       has been ACK'ed or NACK'ed.
                if ( rfCounters.numTxCtrl )
                {
                    // enable encryption once start encryption request is sent
                    // Note: We can not receive data once the encryption control
                    //       procedure has begun, so there is no risk of a race
                    //       condition here.
                    connPtr->encEnabled = TRUE;
                    // clear packet counters
                    connPtr->encInfo.txPktCount = 0;
                    connPtr->encInfo.rxPktCount = 0;
                }

                // not done until the LL_CTRL_START_ENC_RSP is received, so check it
                // Note: The following code can not be in the previous "if" statement
                //       since it is possible that numTxCtrl could be true, yet the
                //       flag startEncRspRcved isn't. Then on the next event,
                //       numTxCtrl wouldn't be true, and we would never check the
                //       startEncRspRcved flag again. Since we can't get the
                //       LL_START_ENC_RSP until we send the LL_CTRL_START_ENC_REQ,
                //       this isn't an issue.
                if ( connPtr->encInfo.startEncRspRcved == TRUE )
                {
                    // replace control procedure at head of queue to prevent interleaving
                    llReplaceCtrlPkt( connPtr, LL_CTRL_START_ENC_RSP );
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
                // first, check if the SK has been calculated
                if ( connPtr->encInfo.SKValid == TRUE )
                {
                    // so try to begin the last step of the encryption procedure
                    if ( llSetupStartEncReq( connPtr ) == TRUE )
                    {
                        // ready the flag that indicates that we've received the response
                        connPtr->encInfo.startEncRspRcved = FALSE;
                        // the control packet is now active
                        connPtr->ctrlPktInfo.ctrlPktActive = TRUE;
                    }

                    // Note: Two cases are possible:
                    //       a) We successfully placed the packet in the TX FIFO.
                    //       b) We did not.
                    //
                    //       In case (a), it may be possible that a previously just
                    //       completed control packet happened to complete based on
                    //       rfCounters.numTxCtrl. Since the current control
                    //       procedure is now active, it could falsely detect
                    //       rfCounters.numTxCtrl, when in fact this was from the
                    //       previous control procedure. Consequently, return.
                    //
                    //       In case (b), the control packet stays at the head of the
                    //       queue, and there's nothing more to do. Consequently, return.
                    //
                    //       So, in either case, return.
                    return( LL_CTRL_PROC_STATUS_SUCCESS );
                }
                else // SK isn't valid yet, so see if we've received the LTK yet
                {
                    if ( connPtr->encInfo.LTKValid )
                    {
                        // generate the Session Key (i.e. SK = AES128(LTK, SKD))
                        LL_ENC_GenerateSK( connPtr->encInfo.LTK,
                                           connPtr->encInfo.SKD,
                                           connPtr->encInfo.SK );
                        // indicate the SK is valid, and drop through
                        connPtr->encInfo.SKValid = TRUE;
                    }
                    else // not done yet
                    {
                        //  control packet stays at head of queue, so exit here
                        return( LL_CTRL_PROC_STATUS_SUCCESS );
                    }
                }
            }

            break;

        case LL_CTRL_START_ENC_RSP:

            // check if the control packet procedure is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // yes, so check if it has been transmitted yet
                // Note: This only means the packet has been transmitted, not that it
                //       has been ACK'ed or NACK'ed.
                if ( rfCounters.numTxCtrl )
                {
                    // packet TX'ed, so we are done with the encryption procedure
                    // re-activate slave latency
                    connPtr->slaveLatency = connPtr->slaveLatencyValue;
                    // remove control packet from processing queue and drop through
                    llDequeueCtrlPkt( connPtr );
                    // set flag to allow outgoing data transmissions
                    connPtr->txDataEnabled = TRUE;
                    // okay to receive data again
                    connPtr->rxDataEnabled = TRUE;

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

                    // clear the restart flag in case of another key change request,
                    // and all other encryption flags
                    // Note: But in reality, there isn't a disable encryption in BLE,
                    //       so once encryption is enabled, any call to LL_StartEncrypt
                    //       will result in an encryption key change callback.
                    connPtr->encInfo.encRestart       = FALSE;
                    connPtr->encInfo.encReqRcved      = FALSE;
                    connPtr->encInfo.pauseEncRspRcved = FALSE;
                    connPtr->encInfo.startEncRspRcved = FALSE;
                }
                else // not done yet
                {
                    // check if a update param req control procedure timeout has occurred
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
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupStartEncRsp( connPtr );
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

        case LL_CTRL_PAUSE_ENC_RSP:

            // check if the control packet procedure is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // not done until the LL_CTRL_PAUSE_ENC_RSP is received, so check it
                if ( connPtr->encInfo.pauseEncRspRcved == TRUE )
                {
                    // done with this control packet, so remove from the processing
                    // queue and drop through (so the encrypton response can be
                    // processed)
                    // ALT: COULD REPLACE HEAD OF QUEUE WITH DUMMY SO NO OTHER CONTROL
                    //      PROCEDURE CAN INTERLEAVE BEFORE THE ENC_REQ IS RECEIVED.
                    llDequeueCtrlPkt( connPtr );
                }
                else // not received yet, so decrement and check control procedure timeout
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
                // so try to put it there
                // Note: All pending transmissions must also be finished before this
                //       packet is placed in the TX FIFO.
                if ( llSetupPauseEncRsp( connPtr ) == TRUE )
                {
                    // clear the flag that indicates an Encryption Request has been
                    // received, which is used by this control procedure to restart the
                    // control procedure timeout
                    connPtr->encInfo.pauseEncRspRcved = FALSE;
                    // disable encryption
                    // Note: Not really necessary as no data is supposed to be sent
                    //       or received.
                    connPtr->encEnabled = FALSE;
                    // the control packet is now active; drop through
                    connPtr->ctrlPktInfo.ctrlPktActive = TRUE;
                }
                else // not done yet
                {
                    //  control packet stays at head of queue, so exit here
                    return( LL_CTRL_PROC_STATUS_SUCCESS );
                }
            }

            break;

        case LL_CTRL_REJECT_IND:

            // check if the control packet procedure is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // yes, so check if it has been transmitted yet
                // Note: This only means the packet has been transmitted, not that it
                //       has been ACK'ed or NACK'ed.
                // Note: The control procedure does not end until the Reject is ACKed.
                //       However, if the ACK is a data packet, it will be tossed
                //       unless data is allowed hereafter. So to avoid this, only
                //       the confirmed transmission of this will be used to qualify
                //       the related flags, but a new procedure will not be able to
                //       begin until this procedure completes, per the spec.
                if ( rfCounters.numTxCtrl )
                {
                    // disable encryption
                    // Note: Never really enabled so this isn't necessary.
                    connPtr->encEnabled = FALSE;
                    // set flag to allow outgoing data transmissions
                    connPtr->txDataEnabled = TRUE;
                    // okay to receive data again
                    connPtr->rxDataEnabled = TRUE;
                }

                // we have already place packet on TX FIFO, so check if its been ACK'ed
                if ( rfCounters.numTxCtrlAck )
                {
                    // done with this control packet, so remove from the processing
                    // queue and drop through
                    llDequeueCtrlPkt( connPtr );
                }
                else // not ack'ed yet
                {
                    // check if a control procedure timeout has occurred
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
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupRejectInd( connPtr,connPtr->encInfo.encRejectErrCode);
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

        // should be LL_CTRL_SLAVE_FEATURE_REQ
//      case LL_CTRL_FEATURE_REQ:    // for v4.2, slave may send LL_CTRL_FEATURE_REQ msg. to be test later.........  HZF
//        // check if the control packet procedure is active
//        if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
//        {
//          // we have already placed a packet on TX FIFO, so wait now until we
//          // get the slave's LL_CTRL_FEATURE_RSP
//          if ( connPtr->featureSetInfo.featureRspRcved == TRUE )
//          {
//            // notify the Host
//            LL_ReadRemoteUsedFeaturesCompleteCback( LL_STATUS_SUCCESS,
//                                                    connPtr->connId,
//                                                    connPtr->featureSetInfo.featureSet );

//            // done with this control packet, so remove from the processing queue
//            llDequeueCtrlPkt( connPtr );
//          }
//          else // no done yet
//          {
//            // check if a update param req control procedure timeout has occurred
//            // Note: No need to cleanup control packet info as we are done.
//            if ( --connPtr->ctrlPktInfo.ctrlTimeout == 0 )
//            {
//              // indicate a control procedure timeout on this request
//              // Note: The parameters are not valid.
//              LL_ReadRemoteUsedFeaturesCompleteCback( LL_CTRL_PKT_TIMEOUT_TERM,
//                                                      connPtr->connId,
//                                                      connPtr->featureSetInfo.featureSet );
//              // we're done waiting, so end it all
//              // Note: No need to cleanup control packet info as we are done.
//              llConnTerminate( connPtr, LL_CTRL_PKT_TIMEOUT_HOST_TERM );

//              return( LL_CTRL_PROC_STATUS_TERMINATE );
//            }
//            else
//            {
//              //  control packet stays at head of queue, so exit here
//              return( LL_CTRL_PROC_STATUS_SUCCESS );
//            }
//          }
//        }
//        else // control packet has not been put on the TX FIFO yet
//        {
//          // so try to put it there; being active depends on a success
//          connPtr->ctrlPktInfo.ctrlPktActive = llSetupFeatureSetReq( connPtr );

//          // set flag while we wait for response
//          // Note: It is okay to repeatedly set this flag in the event the
//          //       setup routine hasn't completed yet (e.g. if the TX FIFO
//          //       has not yet become empty).
//          connPtr->featureSetInfo.featureRspRcved = FALSE;

//          // Note: Two cases are possible:
//          //       a) We successfully placed the packet in the TX FIFO.
//          //       b) We did not.
//          //
//          //       In case (a), it may be possible that a previously just
//          //       completed control packet happened to complete based on
//          //       rfCounters.numTxCtrlAck. Since the current control
//          //       procedure is now active, it could falsely detect
//          //       rfCounters.numTxCtrlAck, when in fact this was from the
//          //       previous control procedure. Consequently, return.
//          //
//          //       In case (b), the control packet stays at the head of the
//          //       queue, and there's nothing more to do. Consequently, return.
//          //
//          //       So, in either case, return.
//          return( LL_CTRL_PROC_STATUS_SUCCESS );
//        }

//        break;

        case LL_CTRL_FEATURE_RSP:

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
                    connPtr->slaveLatency = connPtr->slaveLatencyValue;
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

        // Version Information Indication
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
                // so try to put it there; being active depends on a success
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

        case LL_CTRL_PHY_RSP:

            // check if the control packet procedure is is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // yes, so check if it has been transmitted yet
                // Note: This does not mean this packet has been ACK'ed or NACK'ed.
                if ( rfCounters.numTxCtrl )
                {
                    connPtr->llPhyModeCtrl.isProcessingReq=FALSE;
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
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupPhyRsp( connPtr );
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

        case LL_CTRL_CTE_REQ:

            // check if the control packet procedure is is active
            if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
            {
                // yes, so check if it has been transmitted yet
                // Note: This does not mean this packet has been ACK'ed or NACK'ed.
                if ( rfCounters.numTxCtrl )
                {
                    //                 connPtr->llPhyModeCtrl.isWatingRsp=TRUE;
                    // remove control packet from processing queue and drop through
                    llDequeueCtrlPkt( connPtr );
                }
                else // not done yet
                {
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
                    connPtr->llCTEModeCtrl.isWatingRsp = FALSE;
                    connPtr->llCTEModeCtrl.isProcessingReq = FALSE;
                    // remove control packet from processing queue and drop through
                    // 2020-02-12 comment:after send CONN CTE RSP , then clear txSupp
                    ll_hw_set_cte_txSupp( CTE_SUPP_NULL);
                    llDequeueCtrlPkt( connPtr );
                }
                else // not done yet
                {
                }
            }
            else // control packet has not been put on the TX FIFO yet
            {
                connPtr->ctrlPktInfo.ctrlPktActive = llSetupCTERsp( connPtr );
                return( LL_CTRL_PROC_STATUS_SUCCESS );
            }

            break;

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

        // Dummy Place Holder
        //case LL_CTRL_DUMMY_PLACE_HOLDER:
        //  //  dummy packet stays at head of queue, so exit here
        // Note: Unreachable statement generates compiler warning!
        //break;
        //  return( LL_CTRL_PROC_STATUS_SUCCESS );

        default:
            break;
        }
    }

    return( LL_CTRL_PROC_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          llProcessSlaveControlPacket0

    @brief       This routine is used to process incoming Control packet.

    input parameters

    @param       connPtr - Pointer to BLE LL Connection.
    @param       pBuf    - Pointer to Control packet payload.

    output parameters

    @param       None.

    @return      None.
*/
void llProcessSlaveControlPacket0( llConnState_t* connPtr,
                                   uint8*         pBuf )
{
    uint8 i;
    uint8 opcode = *pBuf++;
    uint8 iqCnt = 0;

//  uint16 iSample[LL_CTE_MAX_SUPP_LEN * LL_CTE_SUPP_LEN_UNIT];
//  uint16 qSample[LL_CTE_MAX_SUPP_LEN * LL_CTE_SUPP_LEN_UNIT];
    // Control Packet
    // check the type of control packet
    switch( opcode )
    {
    // Update Connection Parameters
    case LL_CTRL_CONNECTION_UPDATE_REQ:
        // Note: It is assumed that we have automatically ACK'ed this
        //       packet since the only time the nR sends a NACK is when
        //       the RX FIFO is too full to receive a packet. The fact
        //       that we received this packet means this wasn't the case.
        // Note: What we don't know here is whether or not the Master
        //       in fact received the ACK. The only way to know that is
        //       if the Master's next packet is an ACK. For now, we are
        //       going to assume we only have to verify that we sent an
        //       ACK to the Master, not that the Master actually received
        //       it.
        // Note: The spec limits the number of control procedures that
        //       the Slave has to handle to one. It is assumed that the
        //       Master will ensure this isn't violated, so the Slave
        //       only need keep track of control procedures it starts.
        // Note: Can also check if RX FIFO full counter numRxFifoFull is
        //       not zero.
        // save the connection udpate parameters
        connPtr->paramUpdate.winSize = *pBuf++;
        pBuf = llMemCopySrc( (uint8*)&connPtr->paramUpdate.winOffset, pBuf, 2 );
        pBuf = llMemCopySrc( (uint8*)&connPtr->paramUpdate.connInterval, pBuf, 2 );
        pBuf = llMemCopySrc( (uint8*) &connPtr->paramUpdate.slaveLatency, pBuf, 2 );
        pBuf = llMemCopySrc( (uint8*)&connPtr->paramUpdate.connTimeout, pBuf, 2 );
        // convert this data into units of 625us
        connPtr->paramUpdate.winSize      <<= 1;
        connPtr->paramUpdate.winOffset    <<= 1;
        connPtr->paramUpdate.connInterval <<= 1;
        connPtr->paramUpdate.connTimeout  <<= 4;
        // connection event when update is activated (i.e. Instant)
        pBuf = llMemCopySrc( (uint8*)&connPtr->paramUpdateEvent, pBuf, 2 );

        // check if update event count is still valid
        // Note: The spec indicates the connection should be termainted when
        //       the instant is in the past.
        if ( (uint16)(connPtr->paramUpdateEvent - connPtr->currentEvent) >= LL_MAX_UPDATE_COUNT_RANGE )  // bug fixed 2018-09-25
        {
            // instant past, so terminate connection immediately
            llConnTerminate( connPtr, LL_CTRL_PKT_INSTANT_PASSED_PEER_TERM );
            return;
        }

        // check that the LSTO is valid (i.e. meets the requirements)
        // Note: LSTO > (1 + Slave Latency) * (Connection Interval * 2)
        // Note: The CI * 2 requirement based on ESR05 V1.0, Erratum 3904.
        // Note: All times are in 625us.
        if ( (uint32)connPtr->paramUpdate.connTimeout <=
                ((uint32)(1 + connPtr->paramUpdate.slaveLatency) *
                 (uint32)(connPtr->paramUpdate.connInterval << 1)) )
        {
            // it isn't, so terminate
            llConnTerminate( connPtr, LL_UNACCEPTABLE_CONN_INTERVAL_TERM );
            return;
        }

        //process for the protocol collision
        //2018-11-10 by ZQ
        if(connPtr->llPhyModeCtrl.isWatingRsp==TRUE)
        {
            connPtr->isCollision=TRUE;
            connPtr->rejectOpCode = LL_CTRL_CONNECTION_UPDATE_REQ;
        }

        // set flag in current connection to indicate an param update is valid
        connPtr->pendingParamUpdate = TRUE;
        // disable slave latency so slave can listen to every connection
        // event, per the spec
        // Note: Only required until ACK of this update is confirmed, and the
        //       connection event of the instant, and the connection event
        //       before that.
        connPtr->slaveLatency = 0;
        break;

    // Update Data Channel Map
    case LL_CTRL_CHANNEL_MAP_REQ:
        // Note: It is assumed that we have automatically ACK'ed this
        //       packet since the only time the NR sends a NACK is when
        //       the RX FIFO is too full to receive a packet. The fact
        //       that we received this packet means this wasn't the case.
        // Note: What we don't know here is whether or not the Master
        //       in fact received the ACK. The only way to know that is
        //       if the Master's next packet is an ACK. For now, we are
        //       going to assume we only have to verify that we sent an
        //       ACK to the Master, not that the Master actually received
        //       it.
        // Note: The spec limits the number of control procedures that
        //       the Slave has to handle to one. It is assumed that the
        //       Master will ensure this isn't violated, so the Slave
        //       only need keep track of control procedures it starts.
        // Note: Can also check if RX FIFO full error bit is set.
        // save the connection udpate data channel parameters
        //ZQ 20200207
        //stored the chanMapUpdate in conn_cxt
        //pBuf = llMemCopySrc( chanMapUpdate.chanMap, pBuf, LL_NUM_BYTES_FOR_CHAN_MAP );
        pBuf = llMemCopySrc( (uint8*)&connPtr->chanMapUpdate.chanMap, pBuf, LL_NUM_BYTES_FOR_CHAN_MAP );
        // connection event when update is activated
        pBuf = llMemCopySrc( (uint8*)&connPtr->chanMapUpdateEvent, pBuf, 2 );

        // check if update event count is still valid
        // Note: The spec indicates the connection should be termainted when
        //       the instant is in the past.
        if ( (uint16)(connPtr->chanMapUpdateEvent - connPtr->currentEvent) >= LL_MAX_UPDATE_COUNT_RANGE )      // bug fixed 2018-09-25
        {
            // instant past, so terminate connection immediately
            llConnTerminate( connPtr, LL_CTRL_PKT_INSTANT_PASSED_PEER_TERM );
            return;
        }

        //process for the protocol collision
        //2018-11-10 by ZQ
        if(connPtr->llPhyModeCtrl.isWatingRsp==TRUE)
        {
            connPtr->isCollision=TRUE;
            connPtr->rejectOpCode = LL_CTRL_CHANNEL_MAP_REQ;
        }

        // set flag in current connection to indicate an data channel update is valid
        connPtr->pendingChanUpdate = TRUE;
        // disable slave latency so slave can listen to every connection
        // event, per the spec
        // Note: Only required until ACK of this update is confirmed, and the
        //       connection event of the instant, and the connection event
        //       before that.
        connPtr->slaveLatency = 0;
        #if ( OSALMEM_METRICS )
        writeLog(TEMP_LOG, 0x333333);
        #endif
        break;

    // Encryption Request - Slave Only; Sent by the Master
    case LL_CTRL_ENC_REQ:

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

        // check if encryption is a supported feature set item
        if ( (connPtr->featureSetInfo.featureSet[0] & LL_FEATURE_ENCRYPTION) != LL_FEATURE_ENCRYPTION )
        {
            // set the encryption rejection error code
            connPtr->encInfo.encRejectErrCode = LL_STATUS_ERROR_UNSUPPORTED_REMOTE_FEATURE;
            // so reject the encryption request
            // ALT: IF THE HEAD OF THE QUEUE HAS A DUMMY TO PREVENT INTERLEAVING
            //      WHEN THE PAUSE ENC PROCEDURE HAS BEEN STARTED, THEN WE COULD
            //      USE A REPLACE HERE INSTEAD OF ENQUEUE. BUT NOTE THAT IF THIS
            //      IS A START ENC (INSTEAD OF RESTART) THEN WE CAN'T REPLACE AT
            //      THE HEAD AS SOMETHING VALID MAY ALREADY BE THERE.
            //      WE WOULD HAVE TO ENQUEUE.
            llEnqueueCtrlPkt( connPtr, LL_CTRL_REJECT_IND );
            break;
        }

        // set flag to stop all outgoing transmissions
        connPtr->txDataEnabled = FALSE;
        // set flag to discard all incoming data transmissions
        connPtr->rxDataEnabled = FALSE;
        // indicate an Encryption Request has been received
        // Note: This is used in a Pause Encryption procedure to allow
        //       the control procedure to restart the timer. In a normal
        //       encryption procedure, this flag is not used.
        connPtr->encInfo.encReqRcved = TRUE;
        // copy the random vector
        // Note: The RAND will be left in LSO..MSO order as this is
        //       assumed to be the order of the bytes at the Host API
        //       interface.
        pBuf = llMemCopySrc( &connPtr->encInfo.RAND[0], pBuf, LL_ENC_RAND_LEN );
        // copy the encrypted diversifier
        // Note: The EDIV will be left in LSO..MSO order as this is
        //       assumed to be the order of the bytes at the Host API
        //       interface.
        pBuf = llMemCopySrc( &connPtr->encInfo.EDIV[0], pBuf, LL_ENC_EDIV_LEN );
        // copy the master's portion of the session key identifier
        // Note: The SKDm LSO is the LSO of the SKD.
        pBuf = llMemCopySrc( &connPtr->encInfo.SKD[LL_ENC_SKD_M_OFFSET], pBuf, LL_ENC_SKD_M_LEN );
        // bytes are received LSO..MSO, but need to be maintained as
        // MSO..LSO, per FIPS 197 (AES), so reverse the bytes
        LL_ENC_ReverseBytes( &connPtr->encInfo.SKD[LL_ENC_SKD_M_OFFSET], LL_ENC_SKD_M_LEN );
        // copy the master's portion of the initialization vector
        // Note: The IVm LSO is the LSO of the IV.
        pBuf = llMemCopySrc( &connPtr->encInfo.IV[LL_ENC_IV_M_OFFSET], pBuf, LL_ENC_IV_M_LEN );
        // bytes are received LSO..MSO, but need to be maintained as
        // MSO..LSO, per FIPS 197 (AES), so reverse the bytes
        // ALT: POSSIBLE TO MAINTAIN THE IV IN LSO..MSO ORDER SINCE THE NONCE
        //      IS FORMED THAT WAY.
        LL_ENC_ReverseBytes( &connPtr->encInfo.IV[LL_ENC_IV_M_OFFSET], LL_ENC_IV_M_LEN );
        // generate SKDs
        // Note: The SKDs MSO is the MSO of the SKD.
        // Note: Placement of result forms concatenation of SKDm and SKDs.
        // Note: Leave the SKDs in LSO..MSO order for now since it has to
        //       be sent OTA in this way. Reservse bytes after that.
        LL_ENC_GenDeviceSKD( &connPtr->encInfo.SKD[ LL_ENC_SKD_S_OFFSET ] );
        // generate IVs
        // Note: The IVs MSO is the MSO of the IV.
        // Note: Placement of result forms concatenation of IVm and IVs.
        // Note: Leave the SKDs in LSO..MSO order for now since it has to
        //       be sent OTA in this way. Reservse bytes after that.
        LL_ENC_GenDeviceIV( &connPtr->encInfo.IV[ LL_ENC_IV_S_OFFSET ] );
        //  A1 ROM metal change, add update cachedTRNGdata
        //  a cache update of FIPS TRNG values for next SKD/IV usage
        (void)LL_ENC_GenerateTrueRandNum( cachedTRNGdata, LL_ENC_TRUE_RAND_BUF_SIZE );
        // schedule the output of the control packet
        llEnqueueCtrlPkt( connPtr, LL_CTRL_ENC_RSP );
        break;

    // FIRST MASTER THEN SLAVE SENDS
    case LL_CTRL_START_ENC_RSP:
        // indicate we've received the start encryption response
        connPtr->encInfo.startEncRspRcved = TRUE;
        break;

    // ONLY MASTER SENDS, ONLY SLAVE RECEIVES
    case LL_CTRL_PAUSE_ENC_REQ:
        // set flag to stop all outgoing transmissions
        connPtr->txDataEnabled = FALSE;
        // set flag to discard all incoming data transmissions
        connPtr->rxDataEnabled = FALSE;
        // invalidate the existing session key
        connPtr->encInfo.SKValid = FALSE;
        // set a flag to indicate this is a restart
        connPtr->encInfo.encRestart = TRUE;
        // indicate the LTK is no longer valid
        connPtr->encInfo.LTKValid = FALSE;
        // schedule the pause encryption response control packet
        llEnqueueCtrlPkt( connPtr, LL_CTRL_PAUSE_ENC_RSP );
        break;

    // FIRST MASTER THEN SLAVE
    case LL_CTRL_PAUSE_ENC_RSP:
        // set a flag to indicate the pause encryption response arrived
        connPtr->encInfo.pauseEncRspRcved = TRUE;
        break;

    case LL_CTRL_FEATURE_RSP:
        // TODO:  add the process in Version 4.2
        break;

    // Controller Feature Setup
    case LL_CTRL_FEATURE_REQ:

        // read this device's Feature Set
        // Note: Must re-read the device feature set as it may have been
        //       changed by the Host with Set Local Feature Set.
        for (i=0; i<LL_MAX_FEATURE_SET_SIZE; i++)
        {
            connPtr->featureSetInfo.featureSet[i] = deviceFeatureSet.featureSet[i];
        }

        // logical-AND with master's feature set to indicate which of the
        // controller features in the master the slave requests to be used
        for (i=0; i<LL_MAX_FEATURE_SET_SIZE; i++)
        {
            connPtr->featureSetInfo.featureSet[i] =
                *pBuf++ & deviceFeatureSet.featureSet[i];
        }

        // schedule the output of the control packet
        // Note: Features to be used will be taken on the next connection
        //       event after the response is successfully transmitted.
        llEnqueueCtrlPkt( connPtr, LL_CTRL_FEATURE_RSP );
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
            connPtr->verInfo.verNum = *pBuf++;
            pBuf = llMemCopySrc( (uint8*)&connPtr->verInfo.comId, pBuf, 2 );
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

    // LL PHY UPDATE Req
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
            if(connPtr->llPhyModeCtrl.isWatingRsp==TRUE)
            {
                connPtr->isCollision=TRUE;
                connPtr->rejectOpCode = LL_CTRL_PHY_REQ;
            }

            if(connPtr->llPhyModeCtrl.isProcessingReq==FALSE)
            {
                connPtr->llPhyModeCtrl.req.txPhy=*pBuf++;
                connPtr->llPhyModeCtrl.req.rxPhy=*pBuf++;
                connPtr->llPhyModeCtrl.req.allPhy=connPtr->llPhyModeCtrl.def.allPhy;
                connPtr->llPhyModeCtrl.isProcessingReq=TRUE;
                llEnqueueCtrlPkt( connPtr, LL_CTRL_PHY_RSP );
            }
        }

        break;

    case LL_CTRL_PHY_UPDATE_IND:

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
                connPtr->phyUpdateInfo.m2sPhy   =   *pBuf++;
                connPtr->phyUpdateInfo.s2mPhy   =   *pBuf++;

                //20181204 ZQ
                //process PHY no change case
                if(connPtr->phyUpdateInfo.m2sPhy==0 && connPtr->phyUpdateInfo.s2mPhy==0)
                {
                    connPtr->phyUpdateInfo.m2sPhy = connPtr->llPhyModeCtrl.local.rxPhy;
                    connPtr->phyUpdateInfo.s2mPhy = connPtr->llPhyModeCtrl.local.txPhy;
                    llPhyModeCtrlUpdateNotify(connPtr,LL_STATUS_SUCCESS);
                    connPtr->llPhyModeCtrl.isWatingRsp=FALSE;
                    return;
                }

                // connection event when update is activated (i.e. Instant)
                pBuf = llMemCopySrc( (uint8*)&connPtr->phyModeUpdateEvent, pBuf, 2 );

                // check if update event count is still valid
                // Note: The spec indicates the connection should be termainted when
                //       the instant is in the past.
                if ( (uint16)(connPtr->phyModeUpdateEvent - connPtr->currentEvent) >= LL_MAX_UPDATE_COUNT_RANGE )
                {
                    // instant past, so terminate connection immediately
                    llConnTerminate( connPtr, LL_CTRL_PKT_INSTANT_PASSED_PEER_TERM );
                    return;
                }

                connPtr->llPhyModeCtrl.isWatingRsp=FALSE;
                connPtr->pendingPhyModeUpdate=TRUE;
                // disable slave latency so slave can listen to every connection
                // event, per the spec
                // Note: Only required until ACK of this update is confirmed, and the
                //       connection event of the instant, and the connection event
                //       before that.
                connPtr->slaveLatency = 0;
            }
        }

        break;

    // Terminate Indication
    case LL_CTRL_TERMINATE_IND:
        // read the reason code
        connPtr->termInfo.reason = *pBuf++;
        // set flag to indicate a termination indication was received
        connPtr->termInfo.termIndRcvd = TRUE;
        // received a terminate from peer host, so terminate immediately
        // Note: It is assumed that we have automatically ACK'ed this
        //       packet since the terminate indication was correctly received
        //       (i.e. no NACK from either a CRC error or RX FIFO too full
        //       to receive error).
        // Note: What we don't know here is whether or not the Master
        //       in fact received the ACK. The only way to know that is
        //       if the Master's next packet is an ACK. But if the Master
        //       received the ACK, then it would terminate, so we may not
        //       ever receive another packet! In any case, the spec has been
        //       updated such that, when a terminate indication is received,
        //       we only need confirm we've ACK'ed the packet.
        // Note: The spec limits the number of control procedures that
        //       the Slave has to handle to one. It is assumed that the
        //       Master will ensure this isn't violated, so the Slave
        //       only need keep track of control procedures it starts.
        // terminate
        llConnTerminate( connPtr, connPtr->termInfo.reason );
        return;

    // Process Handling Protocol Collision
    case LL_REJECT_IND:
    case LL_REJECT_IND_EXT:
        if(connPtr->isCollision==TRUE)
        {
            if(connPtr->rejectOpCode== LL_CTRL_PHY_REQ)
            {
                llPhyModeCtrlUpdateNotify(connPtr,LL_STATUS_ERROR_DIFF_TRANSACTION_COLLISION);
            }
            else
            {
                llPhyModeCtrlUpdateNotify(connPtr,LL_STATUS_ERROR_LL_PROCEDURE_COLLISION);
            }
        }
        else
        {
            // do nothing
            if(opcode==LL_REJECT_IND_EXT)
            {
                llPhyModeCtrlUpdateNotify(connPtr,*pBuf++);
            }
            else
            {
                llPhyModeCtrlUpdateNotify(connPtr,LL_STATUS_ERROR_LL_PROCEDURE_COLLISION);
            }
        }

        connPtr->llPhyModeCtrl.isWatingRsp=FALSE;
        connPtr->isCollision=FALSE;
        break;

    // Peer Device Received an Unknown Control Type
    case LL_CTRL_UNKNOWN_RSP:

        // Note: There doesn't appear to be any action for this message,
        //       other than to ACK it.
        // if we support BLE 4.2/5.0, need consider the impact to BLE4.2/5.0 new ctrl messages ... HZF
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

        break;

    // Our Device Received an Unknown Control Type
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

        // check if supported CTE Response Feature and if HCI Command enable rsp
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
            //process for the protocol collision
            if(connPtr->llCTEModeCtrl.isWatingRsp==TRUE)
            {
                connPtr->isCollision=TRUE;
                connPtr->rejectOpCode = LL_CTRL_CTE_REQ;
            }

            if(connPtr->llCTEModeCtrl.isProcessingReq==FALSE)
            {
                uint8 CTE_tmp;
                CTE_tmp = *pBuf++;
                connPtr->llConnCTE.CTE_Length = CTE_tmp & 0x1F;
                connPtr->llConnCTE.CTE_Type = CTE_tmp & 0xC0;
                connPtr->llCTEModeCtrl.isProcessingReq=TRUE;
                llEnqueueCtrlPkt( connPtr, LL_CTRL_CTE_RSP );
            }
        }

        break;

    case LL_CTRL_CTE_RSP:
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

        break;

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
    @fn          llSlaveEvt_TaskAbort0

    @brief       This function is used to handle the PHY task done end cause
                TASK_ABORT that can result from one of two causes. First, a
                command was issued to start a new task while the hardware was
                already executing a task. Second, a CMD_SHUTDOWN command
                was received while executing a task. Since the former is
                controlled by the LL software it will never happen. Therefore,
                this handler is only for a hardware shutdown initiated by the
                LL software.

                Note: Issuing a CMD_SHUTDOWN when the hardware is not running
                      does not cause this end cause to occur, and in fact, has
                      no effect.

                Possible reasons for the LL issuing this command are:
                - A connection timeout occurred (i.e. a LSTO expiration).
                - This device's Host requested termination.

                If the LL was in the process of forming a connection (i.e. was
                in a Connection Setup procedure), then the LL connection is
                immediately terminated (i.e. everything in the RX and TX FIFO
                are forfeited, and the Host is notified). If the LL is in a
                connection, then a Connection Termination procedure is started
                in that the peer device is sent a TERMINATE_IND control packet.
                Then the connection is terminated.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
void llSlaveEvt_TaskAbort0( void )
{
    return;
}


/*******************************************************************************
    @fn          ll_adptive_adj_next_time

    @brief      will adptive modified following var to adjust the rx window in RTLP
                1.slave_conn_event_recv_delay
                2.pGlobal_config[LL_HW_RTLP_1ST_TIMEOUT

    input parameters
    @param       none

    output parameters

    @param       None

    @return      None.
*/
void ll_adptive_adj_next_time0(uint32 nextTime)
{
    //==================================================
    //DO NOT ADD LOG PRINTF In this FUNCTION
    //==================================================
    (void)(nextTime);
    llConnState_t* connPtr;
    // get connection information
    connPtr = &conn_param[g_ll_conn_ctx.currentConn];

    //no anche point
    if(connPtr->rx_timeout)
        connPtr->pmCounter.ll_tbd_cnt1++;
    else
        connPtr->pmCounter.ll_tbd_cnt1=0;

    //only adj for the 1st rxtimeout
    if(1==connPtr->pmCounter.ll_tbd_cnt1)
        slave_conn_event_recv_delay+=500;

    //adj for ntrm pkt, each pkt cost 50us in wt tfifo
    //if(connPtr->rx_timeout)
    slave_conn_event_recv_delay+=((connPtr->ll_buf.ntrm_cnt)*50);
}


// process missing slave event
uint8 ll_processMissSlaveEvt(uint8 connId)
{
    llConnState_t* connPtr;
//    LOG("-S ");
    uint32_t sche_time;
    // get connection information
    connPtr = &conn_param[connId];
    connPtr->rx_crcok = 0;
    connPtr->rx_timeout = 1;
    connPtr->pmCounter.ll_conn_event_cnt ++;
    connPtr->pmCounter.ll_conn_event_timeout_cnt ++;
    connPtr->pmCounter.ll_miss_slave_evt_cnt ++;

    if(p_perStatsByChan!=NULL)
        p_perStatsByChan->rxToCnt[connPtr->currentChan]++;

    rfCounters.numTxDone = 0;
    rfCounters.numRxNotOk    = 0;
    rfCounters.numRxIgnored  = 0;
    rfCounters.numRxEmpty    = 0;
    rfCounters.numRxFifoFull = 0;
    rfCounters.numTxAck      = 0;
    // update the numTxCtrlAck counter, add on 2017-11-15
    rfCounters.numTxCtrlAck = 0;
    // advance the connection event count
    connPtr->currentEvent = connPtr->nextEvent;
    // collect packet error information
    connPtr->perInfo.numMissedEvts++;
    // so listen to every event until a packet is received
    connPtr->slaveLatency = 0;

    // check if we have a Supervision Timeout
    if ( connPtr->expirationEvent == connPtr->currentEvent )
    {
        // check if either we already got the first packet in a connection or
        // if this isn't the quick LSTO associated with connection establishment
        // Note: The Slave reuses firstPacket when a Update Parameter control
        //       procedure is started, but in that case, the expiration event
        //       will be well past the event associated with connection
        //       establishement.
        if ( (connPtr->firstPacket == 0) ||
                (connPtr->currentEvent != LL_LINK_SETUP_TIMEOUT) )
        {
            // yes, so terminate with LSTO
            llConnTerminate( connPtr, LL_SUPERVISION_TIMEOUT_TERM );
            g_pmCounters.ll_link_lost_cnt ++;
            return LL_PROC_LINK_TERMINATE;
        }
        else // this is a failure to establish the connection
        {
            // so terminate immediately with failure to establish connection
            llConnTerminate( connPtr, LL_CONN_ESTABLISHMENT_FAILED_TERM );
            g_pmCounters.ll_link_estab_fail_cnt ++;
            return LL_PROC_LINK_TERMINATE;
        }
    }

    connPtr->slaveLatency = 0;

    /*
    ** Process RX Data Packets, no Rx data for missing event
    */

    /*
    ** Check Control Procedure Processing
    */
    if ( llProcessSlaveControlProcedures( connPtr ) == LL_CTRL_PROC_STATUS_TERMINATE )
    {
        // this connection is terminated, so nothing to schedule
        return LL_PROC_LINK_TERMINATE;
    }

    /*
    ** Process TX Data Packets, no Tx data for missing event
    */
    connPtr->accuTimerDrift += connPtr->timerDrift;

    /*
    ** Setup Next Slave Event Timing
    */

    // update next event, calculate time to next event, calculate timer drift,
    // update anchor points, setup NR T2E1 and T2E2 events
    if ( llSetupNextSlaveEvent() == LL_SETUP_NEXT_LINK_STATUS_TERMINATE )
    {
        // this connection is terminated, so nothing to schedule
        return LL_PROC_LINK_TERMINATE;
    }

    // schedule next connection event time
    // not really timeout case so not widen the receive windows
    sche_time = conn_param[connId].lastTimeToNextEvt * 625 - conn_param[connId].timerDrift;
    // update scheduler information
    g_ll_conn_ctx.scheduleInfo[connId].remainder += sche_time;

// =========
    // connection event notify
    if (g_conn_taskID != 0)
        osal_set_event(g_conn_taskID, g_conn_taskEvent);

    return LL_PROC_LINK_KEEP;
}


/*******************************************************************************
*/
