/*************************************************************************************************
    Filename:       sm_task.c
    Revised:
    Revision:

    Description:    This file contains the SM Task.

	SDK_LICENSE

**************************************************************************************************/

#include "bcomdef.h"
#include "hci_tl.h"
#include "osal_bufmgr.h"
#include "gap_internal.h"
#include "linkdb.h"
#include "l2cap.h"
#include "sm.h"
#include "sm_internal.h"
#include "smp.h"
#include "jump_function.h"

/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/

/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    GLOBAL VARIABLES
*/
uint8 smTaskID;  // SM task ID

/*********************************************************************
    EXTERNAL VARIABLES
*/

/*********************************************************************
    EXTERNAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/

/*********************************************************************
    LOCAL FUNCTIONS
*/
static uint8 smProcessOSALMsg( osal_event_hdr_t* pMsg );
static uint8 smProcessHCICmdCompleteEvt( hciEvt_CmdComplete_t* pMsg );
static uint8 smProcessHCIBLEEventCode( hciEvt_CmdComplete_t* pMsg );

/*********************************************************************
    API FUNCTIONS
*/

/*********************************************************************
    @fn          SM_Init0

    @brief       SM Task initialization function.

    @param       taskID - SM task ID.

    @return      void
*/
void SM_Init( uint8 task_id )
{
    smTaskID = task_id;
    // Register to receive all security related HCI events
    HCI_SMPTaskRegister( smTaskID );
    // Register with L2CAP SMP channel
    VOID L2CAP_RegisterApp( smTaskID, L2CAP_CID_SMP );
    // link database callback
    VOID linkDB_Register( smLinkCheck );
}

/*********************************************************************
    @fn          SM_ProcessEvent0

    @brief       SM Task event processing function.

    @param       taskID - SM task ID.
    @param       events - SM events.

    @return      events not processed
*/
uint16 SM_ProcessEvent( uint8 task_id, uint16 events )
{
	(void) task_id;
    if ( events & SYS_EVENT_MSG )
    {
        uint8* pMsg;  // pointer to incoming message

        // Process incoming OSAL SM messages
        if ( (pMsg = osal_msg_receive( smTaskID )) != NULL )
        {
            uint8 dealloc = TRUE;

            if ( !smProcessOSALMsg( (osal_event_hdr_t*)pMsg ) )
            {
                // The message wasn't processed by SM
                if ( gapUnwantedTaskID != INVALID_TASK_ID )
                {
                    // send it to the registered application to process
                    if ( osal_msg_send( gapUnwantedTaskID, pMsg ) == SUCCESS )
                    {
                        dealloc = FALSE;
                    }
                }
            }

            if ( dealloc )
            {
                // Release the OSAL message
                VOID osal_msg_deallocate( pMsg );
            }
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

//  if ( events & SM_TIMEOUT_EVT )
//  {
//    // Pairing Timeout
//
////    smTimedOut();
//
//    return ( events ^ SM_TIMEOUT_EVT );
//  }
//
//  if ( events & SM_PAIRING_STATE_EVT )
//  {
//    // Trigger the next Pairing State
//
//
//    return ( events ^ SM_PAIRING_STATE_EVT );
//  }
    // If reach here, the events are unknown
    // Discard or make more handlers
    return 0;
}

/*********************************************************************
    @fn      smProcessOSALMsg

    @brief   Process an incoming task message.

    @param   pMsg - message to process

    @return  TRUE if processed and safe to deallocate, FALSE if passed
            off to another task.
*/
static uint8 smProcessOSALMsg( osal_event_hdr_t* pMsg )
{
    uint8 safeToDealloc = TRUE;   // Assume that we are expecting the message

    switch ( pMsg->event )
    {
    case HCI_SMP_EVENT_EVENT:
    {
        switch( pMsg->status )
        {
        case HCI_COMMAND_COMPLETE_EVENT_CODE:
            safeToDealloc = smProcessHCICmdCompleteEvt( (hciEvt_CmdComplete_t*)pMsg );
            break;

        case HCI_LE_EVENT_CODE:
            safeToDealloc = smProcessHCIBLEEventCode( (hciEvt_CmdComplete_t*)pMsg );
            break;

        default:
            safeToDealloc = FALSE;
            break;
        }
    }
    break;

    case L2CAP_DATA_EVENT:
    {
        l2capDataEvent_t* pPkt = (l2capDataEvent_t*)pMsg;
        smProcessDataMsg( pPkt );

        // Free the buffer - payload
        if ( pPkt->pkt.pPayload )
        {
            osal_bm_free( pPkt->pkt.pPayload );
        }
    }
    break;

    default:
        safeToDealloc = FALSE;
        break;
    }

    return ( safeToDealloc );
}

/*********************************************************************
    @fn      smProcessHCICmdCompleteEvt

    @brief   Process an incoming OSAL HCI Command Complete Event.

    @param   pMsg - message to process

    @return  TRUE if processed and safe to deallocate, FALSE if passed
            off to another task.
*/
static uint8 smProcessHCICmdCompleteEvt( hciEvt_CmdComplete_t* pMsg )
{
    uint8 safeToDealloc; // return value

    switch ( pMsg->cmdOpcode )
    {
    case HCI_LE_RAND:
        safeToDealloc = smProcessRandComplete( pMsg->pReturnParam[0], &(pMsg->pReturnParam[1]) );
        break;

    default:
        safeToDealloc = FALSE;  // send this message to the app
        break;
    }

    return ( safeToDealloc );
}

/*********************************************************************
    @fn      smProcessHCIBLEEventCode

    @brief   Process an incoming OSAL HCI BLE Events.

    @param   pMsg - message to process

    @return  TRUE if processed and safe to deallocate, FALSE if passed
            off to another task.
*/
static uint8 smProcessHCIBLEEventCode( hciEvt_CmdComplete_t* pMsg )
{
    uint8 safeToDealloc = TRUE; // Assume that the message will be processed by SM
    uint8 eventCode = ((hciEvt_BLELTKReq_t*)pMsg)->BLEEventCode;

    switch ( eventCode )
    {
    case HCI_BLE_LTK_REQUESTED_EVENT:

        // Are we the responder?
        if ( gapProfileRole & GAP_PROFILE_PERIPHERAL )
        {
            if ( pfnResponderCBs && pfnResponderCBs->pfnProcessLTKReq )
            {
                hciEvt_BLELTKReq_t* pPkt = (hciEvt_BLELTKReq_t*)pMsg;
                safeToDealloc = pfnResponderCBs->pfnProcessLTKReq( pPkt->connHandle,
                                                                   pPkt->random,
                                                                   pPkt->encryptedDiversifier );
            }
        }

        break;

    case HCI_ENCRYPTION_CHANGE_EVENT_CODE:
    {
        uint8 reason;         // Reason field
        uint16 connHandle;    // Connection handle
        hciEvt_EncryptChange_t* pPkt = (hciEvt_EncryptChange_t*)pMsg;
        connHandle = pPkt->connHandle;
        reason = pPkt->reason;
        // pkt->encEnable isn't needed, if pkt->reason == SUCCESS or LL_ENC_KEY_REQ_ACCEPTED
        // then the link is encrypted, otherwise it isn't.
        safeToDealloc = smProcessEncryptChange( connHandle, reason );
    }
    break;

    default:
        safeToDealloc = FALSE;  // send this message to the app
        break;
    }

    return ( safeToDealloc );
}

void smTo_timerCB( uint8* pData )
{
    uint16 connHandle = (uint16 )pData[0];
    smTimedOut( connHandle );
}

void smState_timerCB( uint8* pData )
{
    uint16 connHandle = (uint16 )pData[0];
    smNextPairingState( connHandle );
}


