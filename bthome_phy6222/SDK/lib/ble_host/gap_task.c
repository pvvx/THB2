/*************************************************************************************************
    Filename:       gap_task.c
    Revised:
    Revision:

    Description:    This file contains the GAP Task.

	SDK_LICENSE

**************************************************************************************************/



/*******************************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "OSAL.h"
#include "hci_tl.h"
#include "l2cap.h"
#include "gatt.h"
#include "gap.h"
#include "gap_internal.h"
#include "linkdb.h"

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
uint8 gapTaskID;                           // The GAP's Task ID
uint8 gapUnwantedTaskID = INVALID_TASK_ID; // The task ID of an app/profile that
// wants unexpected HCI messages.

// Callback function pointers for Peripheral
gapPeripheralCBs_t* pfnPeripheralCBs = NULL;

/*********************************************************************
    EXTERNAL VARIABLES
*/

/*********************************************************************
    EXTERNAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/

// Callback function pointers for Central
static const gapCentralCBs_t* pfnCentralCBs = NULL;

/*********************************************************************
    LOCAL FUNCTIONS
*/
static uint8 gapProcessOSALMsg( osal_event_hdr_t* pMsg );
static uint8 gapProcessBLEEvents( osal_event_hdr_t* pMsg );
static uint8 gapProcessHCICmdCompleteEvt( hciEvt_CmdComplete_t* pMsg );
static uint8 gapProcessCommandStatusEvt( hciEvt_CommandStatus_t* pMsg );

/*********************************************************************
    API FUNCTIONS
*/

/*********************************************************************
    Register your task ID to receive extra (unwanted) HCI status and
    complete events.

    Public function defined in gap.h.
*/
void GAP_RegisterForHCIMsgs( uint8 taskID )
{
    gapUnwantedTaskID = taskID;
}

/*********************************************************************
    @fn          GAP_Init

    @brief       GAP Task initialization function.

    @param       taskID - GAP task ID.

    @return      void
*/
void GAP_Init( uint8 task_id )
{
    // Save our own Task ID
    gapTaskID = task_id;
    // Initialize the Link Database
    linkDB_Init();
    // Register with HCI to receive events
    HCI_GAPTaskRegister( gapTaskID );
    // Register with L2CAP to receive unprocessed Signaling messages
    VOID L2CAP_RegisterApp( gapTaskID, L2CAP_CID_SIG );
}

/*********************************************************************
    @fn          GAP_ProcessEvent

    @brief       GAP Task event processing function.

    @param       taskID - GAP task ID
    @param       events - GAP events.

    @return      events not processed
*/
uint16 GAP_ProcessEvent( uint8 task_id, uint16 events )
{
    VOID task_id; // OSAL required parameter that isn't used in this function

    if ( events & SYS_EVENT_MSG )
    {
        uint8* pMsg;

        if ( (pMsg = osal_msg_receive( gapTaskID )) != NULL )
        {
            if ( !gapProcessOSALMsg( (osal_event_hdr_t*)pMsg ) )
            {
                // Send it to the registered application.
                if ( gapUnwantedTaskID != INVALID_TASK_ID )
                {
                    // This message wasn't processed in the GAP, send it
                    // to an app/profile that wants it.
                    if ( osal_msg_send( gapUnwantedTaskID, pMsg ) == SUCCESS )
                    {
                        // return unprocessed events and don't dealloc
                        // the message because it was sent elsewhere
                        return (events ^ SYS_EVENT_MSG);
                    }
                }
            }

            // Release the OSAL message
            VOID osal_msg_deallocate( pMsg );
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if ( events & GAP_OSAL_TIMER_SCAN_DURATION_EVT )
    {
        // Device Discovery time expired, stop the scan
        if ( pfnCentralCBs && pfnCentralCBs->pfnProcessScanningEvt )
        {
            pfnCentralCBs->pfnProcessScanningEvt( NULL );
        }

        return ( events ^ GAP_OSAL_TIMER_SCAN_DURATION_EVT );
    }

    if ( events & GAP_END_ADVERTISING_EVT )
    {
        // Advertising time expired, stop advertising
        if ( pfnPeripheralCBs && pfnPeripheralCBs->pfnProcessAdvertisingEvt )
        {
            pfnPeripheralCBs->pfnProcessAdvertisingEvt( TRUE );
        }

        return ( events ^ GAP_END_ADVERTISING_EVT );
    }

    if ( events & GAP_CHANGE_RESOLVABLE_PRIVATE_ADDR_EVT )
    {
        // Are we in Addr Change mode?
        if ( gapPrivateAddrChangeTimeout )
        {
            // Decrement the seconds in the timeout value
            gapPrivateAddrChangeTimeout--;

            // Time to change the Resolvable Private Address?
            if ( gapPrivateAddrChangeTimeout == 0 )
            {
                uint8 newAddr[B_ADDR_LEN];  // space for the new address

                // Are we advertising now?
                if ( gapIsAdvertising() )
                {
                    gapAutoAdvPrivateAddrChange = TRUE;

                    // Stop any advertising
                    if ( pfnPeripheralCBs && pfnPeripheralCBs->pfnProcessAdvertisingEvt )
                    {
                        pfnPeripheralCBs->pfnProcessAdvertisingEvt( TRUE );
                    }
                }

                // Calculate a new address
                VOID SM_CalcRandomAddr( gapGetIRK(), newAddr );
                // Send the new address to the app/profile
                VOID gapProcessNewAddr( newAddr );
                // Reset the timer
                gapPrivateAddrChangeTimeout = GAP_GetParamValue( TGAP_PRIVATE_ADDR_INT );
            }
        }
        else
        {
            // It shouldn't be set, so stop the timer
            VOID osal_stop_timerEx( gapTaskID, GAP_CHANGE_RESOLVABLE_PRIVATE_ADDR_EVT );
        }

        return ( events ^ GAP_CHANGE_RESOLVABLE_PRIVATE_ADDR_EVT );
    }

    // If reach here, the events are unknown
    // Discard or make more handlers
    return 0;
}

/*********************************************************************
    @fn          GAP_NumActiveConnections

    @brief       Returns the number of active connections.

    @param       none

    @return      number of active connections
*/
uint8 GAP_NumActiveConnections( void )
{
    return ( linkDB_NumActive() );
}

/*********************************************************************
    @fn      gapProcessOSALMsg

    @brief   Process an incoming task message.

    @param   pMsg - message to process

    @return  TRUE if processed and safe to deallocate, FALSE if passed
            off to another task.
*/
static uint8 gapProcessOSALMsg( osal_event_hdr_t* pMsg )
{
    uint8 safeToDealloc = TRUE;

    switch ( pMsg->event )
    {
    case HCI_GAP_EVENT_EVENT:
    {
        switch( pMsg->status )
        {
        case HCI_COMMAND_COMPLETE_EVENT_CODE:
            safeToDealloc = gapProcessHCICmdCompleteEvt( (hciEvt_CmdComplete_t*)pMsg );
            break;

        case HCI_DISCONNECTION_COMPLETE_EVENT_CODE:
            gapProcessDisconnectCompleteEvt( (hciEvt_DisconnComplete_t*)pMsg );
            break;

        case HCI_COMMAND_STATUS_EVENT_CODE:
            safeToDealloc = gapProcessCommandStatusEvt( (hciEvt_CommandStatus_t*)pMsg );
            break;

        case HCI_LE_EVENT_CODE:
            // BLE Events
            safeToDealloc = gapProcessBLEEvents( pMsg );
            break;

        default:
            safeToDealloc = FALSE;  // Send to app
            break;
        }
    }
    break;

    case L2CAP_SIGNAL_EVENT:

        // & --> == bugfix for multi-role
        if ( gapProfileRole == GAP_PROFILE_PERIPHERAL )
        {
            l2capSignalEvent_t* pCmd = (l2capSignalEvent_t*)pMsg;
            l2capCmdReject_t cmdReject;
            // If the slave's Host receives a Connection Parameter Update Request
            // packet it shall respond with a Command Reject packet with reason
            // 0x0000 (Command not understood).
            cmdReject.reason = L2CAP_REJECT_CMD_NOT_UNDERSTOOD;
            VOID L2CAP_CmdReject( pCmd->connHandle, pCmd->id, &cmdReject );
        }
        else if ( pfnCentralConnCBs && pfnCentralConnCBs->pfnProcessConnEvt )
        {
            VOID pfnCentralConnCBs->pfnProcessConnEvt( L2CAP_PARAM_UPDATE,
                                                       (hciEvt_CommandStatus_t*)pMsg );
        }

        break;

    default:
        safeToDealloc = FALSE;  // Send to app
        break;
    }

    return ( safeToDealloc );
}

/*********************************************************************
    @fn      gapProcessBLEEvents

    @brief   Process an incoming OSAL HCI BLE specific events.

    @param   pMsg - message to process

    @return  TRUE if processed and safe to deallocate, FALSE if passed
            off to another task.
*/
static uint8 gapProcessBLEEvents( osal_event_hdr_t* pMsg )
{
    uint8 safeToDealloc = TRUE;

    switch ( ((hciEvt_BLEAdvPktReport_t*)(pMsg))->BLEEventCode )
    {
    case HCI_BLE_ADV_REPORT_EVENT:
        if ( pfnCentralCBs && pfnCentralCBs->pfnProcessScanningEvt )
        {
            pfnCentralCBs->pfnProcessScanningEvt( (hciEvt_BLEAdvPktReport_t*)pMsg );
        }

        break;

    case HCI_BLE_CONNECTION_COMPLETE_EVENT:
        gapProcessConnectionCompleteEvt( (hciEvt_BLEConnComplete_t*)pMsg );
        break;

    case HCI_BLE_CONN_UPDATE_COMPLETE_EVENT:
        gapProcessConnUpdateCompleteEvt( (hciEvt_BLEConnUpdateComplete_t*)pMsg );
        break;

    case HCI_LE_CONNECTIONLESS_IQ_REPORT_EVENT:
        LOG("gapProcessBLEEvents HCI_LE_CONNECTIONLESS_IQ_REPORT_EVENT \n");
        break;

    case HCI_LE_CONNECTION_IQ_REPORT_EVENT:
        LOG("gapProcessBLEEvents HCI_LE_CONNECTION_IQ_REPORT_EVENT \n");
        break;

    default:
        safeToDealloc = FALSE;
        break;
    }

    return ( safeToDealloc );
}

/*********************************************************************
    @fn      gapProcessHCICmdCompleteEvt

    @brief   Process an incoming OSAL HCI Command Complete Event.

    @param   pMsg - message to process

    @return  TRUE if processed and safe to deallocate, FALSE if passed
            off to another task.
*/
static uint8 gapProcessHCICmdCompleteEvt( hciEvt_CmdComplete_t* pMsg )
{
    uint8 safeToDealloc = TRUE;

    switch ( pMsg->cmdOpcode )
    {
    case HCI_LE_SET_RANDOM_ADDR:
        gapProcessRandomAddrComplete( *(pMsg->pReturnParam) );
        break;

    case HCI_LE_READ_BUFFER_SIZE:
        safeToDealloc = gapReadBufSizeCmdStatus(
                            (hciRetParam_LeReadBufSize_t*)pMsg->pReturnParam );
        break;

    case HCI_READ_BDADDR:
        safeToDealloc = gapReadBD_ADDRStatus( pMsg->pReturnParam[0],
                                              &(pMsg->pReturnParam[1]) );
        break;

    case HCI_LE_SET_SCAN_ENABLE:
        if ( *(pMsg->pReturnParam) == SUCCESS )
            break;
        if ( pfnCentralCBs && pfnCentralCBs->pfnProcessHCICmdEvt )
        {
            safeToDealloc = pfnCentralCBs->pfnProcessHCICmdEvt( pMsg->cmdOpcode, pMsg );
        }

        break;
    /*lint --fallthrough */
    case HCI_LE_SET_SCAN_PARAM:
        if ( pfnCentralCBs && pfnCentralCBs->pfnProcessHCICmdEvt )
        {
            safeToDealloc = pfnCentralCBs->pfnProcessHCICmdEvt( pMsg->cmdOpcode, pMsg );
        }

        break;

    case HCI_LE_CREATE_CONNECTION_CANCEL:
        if ( pfnCentralConnCBs && pfnCentralConnCBs->pfnProcessConnEvt )
        {
            safeToDealloc = pfnCentralConnCBs->pfnProcessConnEvt( pMsg->cmdOpcode,
                                                                  (hciEvt_CommandStatus_t*)pMsg );
        }

        break;

    case HCI_LE_SET_ADV_PARAM:
    case HCI_LE_SET_ADV_ENABLE:
    case HCI_LE_SET_SCAN_RSP_DATA:
    case HCI_LE_SET_ADV_DATA:
        if ( pfnPeripheralCBs && pfnPeripheralCBs->pfnProcessHCICmdCompleteEvt )
        {
            safeToDealloc = pfnPeripheralCBs->pfnProcessHCICmdCompleteEvt( pMsg );
        }

        break;

    // 2020-01-16 add for CTE related
    case HCI_LE_SET_CONNLESS_CTE_TRANS_PARAMETER:
    case HCI_LE_SET_CONNLESS_CTE_TRANS_ENABLE:
    case HCI_LE_SET_CONNLESS_IQ_SAMPLE_ENABLE:
    case HCI_LE_SET_CONNCTE_RECV_PARAMETER:
    case HCI_LE_SET_CONN_CTE_TRANSMIT_PARAMETER:
    case HCI_LE_CONN_CTE_REQUEST_ENABLE:
    case HCI_LE_CONN_CTE_RESPONSE_ENABLE:
    case HCI_LE_READ_ANTENNA_INFO:
        if ( pfnPeripheralCBs && pfnPeripheralCBs->pfnProcessHCICmdCompleteEvt )
        {
            safeToDealloc = pfnPeripheralCBs->pfnProcessHCICmdCompleteEvt( pMsg );
        }

        break;

    default:
        safeToDealloc = FALSE;  // send this message to the app
        break;
    }

    return ( safeToDealloc );
}

/*********************************************************************
    @fn      gapProcessCommandStatusEvt

    @brief   Process an incoming OSAL HCI Command Status Event.

    @param   pMsg - message to process

    @return  TRUE if processed and safe to deallocate, FALSE if passed
            off to another task.
*/
static uint8 gapProcessCommandStatusEvt( hciEvt_CommandStatus_t* pMsg )
{
    uint8 safeToDealloc = TRUE;

    switch ( pMsg->cmdOpcode )
    {
    case HCI_LE_CREATE_CONNECTION:
    case HCI_LE_CONNECTION_UPDATE:
        if ( pfnCentralConnCBs && pfnCentralConnCBs->pfnProcessConnEvt )
        {
            safeToDealloc = pfnCentralConnCBs->pfnProcessConnEvt( pMsg->cmdOpcode, pMsg );
        }

        break;

    default:
        safeToDealloc = FALSE;  // Send to app
        break;
    }

    return ( safeToDealloc );
}

/*********************************************************************
    @fn      gapRegisterCentral

    @brief   Register Central's processing function with GAP task.

    @param   pfnCBs - pointer to Central's processing function

    @return  none
*/
void gapRegisterCentral( gapCentralCBs_t* pfnCBs )
{
    pfnCentralCBs = pfnCBs;
}

/*********************************************************************
    @fn      gapRegisterPeripheral

    @brief   Register Peripheral's processing function with GAP task.

    @param   pfnCBs - pointer to Peripheral's processing function

    @return  none
*/
void gapRegisterPeripheral( gapPeripheralCBs_t* pfnCBs )
{
    pfnPeripheralCBs = pfnCBs;
}



/****************************************************************************
****************************************************************************/

