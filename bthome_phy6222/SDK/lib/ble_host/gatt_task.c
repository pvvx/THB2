/**************************************************************************************************
    Filename:       gatt_task.c
    Revised:
    Revision:

    Description:    This file contains Generic Attribute Profile (GATT) task.

	SDK_LICENSE

**************************************************************************************************/


/*********************************************************************
    INCLUDES
*/
#include "comdef.h"
#include "OSAL.h"
#include "osal_bufmgr.h"
//#include "OnBoard.h"
#include "bus_dev.h"
#include "mcu.h"

#include "gatt.h"
#include "gatt_internal.h"


/*********************************************************************
    MACROS
*/
#define WRITE_CMD( method, cmd )  ( ( (method) == ATT_WRITE_REQ ) && ( (cmd) == TRUE ) )

/*********************************************************************
    CONSTANTS
*/

/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    GLOBAL VARIABLES
*/

/*********************************************************************
    GLOBAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/
uint8 gattTaskID;

// Function to call to process the Attribute Server messages.
static gattProcessMsg_t pfnServerProcessMsgCB = NULL;

// Function to call to process the Attribute Client messages.
static gattProcessMsg_t pfnClientProcessMsgCB = NULL;

/*********************************************************************
    LOCAL FUNCTIONS
*/
static void gattProcessOSALMsg( osal_event_hdr_t* pMsg );
static void gattProcessRxData( l2capDataEvent_t* pL2capMsg );

/*********************************************************************
    API FUNCTIONS
*/

/*********************************************************************
    @fn      gattRegisterServer

    @brief   Register the server's processing function with the GATT task.

    @param   pfnProcessMsg - pointer to processing function

    @return  none
*/
void gattRegisterServer( gattProcessMsg_t pfnProcessMsg )
{
    pfnServerProcessMsgCB = pfnProcessMsg;
}

/*********************************************************************
    @fn      gattRegisterClient

    @brief   Register the client's processing function with the GATT task.

    @param   pfnProcessMsg - pointer to processing function

    @return  none
*/
void gattRegisterClient( gattProcessMsg_t pfnProcessMsg )
{
    pfnClientProcessMsgCB = pfnProcessMsg;
}

/*********************************************************************
    @fn      GATT_SetHostToAppFlowCtrl

    @brief   This API is used by the Application to turn flow control on
            or off for GATT messages sent from the Host to the Application.

            Note: If the flow control is enabled then the Application must
                  call the GATT_AppCompletedMsg() API when it completes
                  processing an incoming GATT message.

    @param   flowCtrlMode - flow control mode: TRUE or FALSE

    @return  none
*/
void GATT_SetHostToAppFlowCtrl( uint16_t hostBufSize,uint8 flowCtrlMode )
{
    // The Host buffer size is constrained to 1/3 the maximum heap size
    L2CAP_SetControllerToHostFlowCtrl_DLE( hostBufSize/3, flowCtrlMode );
}

/*********************************************************************
    @fn      GATT_AppCompletedMsg

    @brief   This API is used by the Application to notify GATT that
            the processing of a message has been completed.

    @param   pMsg - processed GATT message

    @return  none
*/
void GATT_AppCompletedMsg( gattMsgEvent_t* pMsg )
{
    // Make sure the processed message is an ATT Write Command or Notification.
    // All ATT Requests and Indication have a built-in flow control (i.e.,
    // have coressponding ATT Responses and Confirmation respectively), therefore
    // a buffer credit was given to the Controller for all other ATT message
    // types when they were processed by the GATT layer.
    if ( ( pMsg->hdr.event == GATT_MSG_EVENT )               &&
            ( pMsg->hdr.status == SUCCESS )                     &&
            ( WRITE_CMD( pMsg->method, pMsg->msg.writeReq.cmd ) ||
              ( pMsg->method == ATT_HANDLE_VALUE_NOTI ) ) )
    {
        L2CAP_HostNumCompletedPkts( pMsg->connHandle, 1 );
    }
}

/*********************************************************************
    @fn      GATT_Init

    @brief   GATT Task initialization function.

    @param   taskId - GATT task ID.

    @return  none
*/
void GATT_Init( uint8 taskId )
{
    gattTaskID = taskId;
    // Register with L2CAP
    VOID L2CAP_RegisterApp( gattTaskID, L2CAP_CID_ATT );
    // Initialize GATT Server; it's mandatory on every device
    VOID GATT_InitServer();
    VOID ATT_InitMtuSize();
}

/*********************************************************************
    @fn      GATT_ProcessEvent

    @brief   GATT Task event processing function.

    @param   taskId - GATT task ID
    @param   events - GATT events.

    @return  events not processed
*/
uint16 GATT_ProcessEvent( uint8 taskId, uint16 events )
{
    uint8* pMsg;
    VOID taskId; // OSAL required parameter that isn't used in this function

    if ( events & SYS_EVENT_MSG )
    {
        if ( (pMsg = osal_msg_receive( gattTaskID )) != NULL )
        {
            gattProcessOSALMsg( (osal_event_hdr_t*)pMsg );
            // Release the OSAL message
            VOID osal_msg_deallocate( pMsg );
        }

        // Return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    // Discard unknown events
    return 0;
}

/*********************************************************************
    @fn      gattProcessOSALMsg

    @brief   Process an incoming task message.

    @param   pMsg - message to process

    @return  none
*/
static void gattProcessOSALMsg( osal_event_hdr_t* pMsg )
{
    switch ( pMsg->event )
    {
    case L2CAP_DATA_EVENT:
        // Process incoming L2CAP Data message
        gattProcessRxData( (l2capDataEvent_t*)pMsg );
        break;

    default:
        // Unknown message - drop it.
        break;
    }
}

/******************************************************************************
    @fn          gattProcessRxData

    @brief       Process data packet received from the L2CAP layer

    @param       pL2capMsg ? pointer to received L2CAP message

    @return      none
*/
static void gattProcessRxData( l2capDataEvent_t* pL2capMsg )
{
    attPacket_t pkt;
    uint8 status;
    // First parse the packet
    status = ATT_ParsePacket( pL2capMsg, &pkt );

    if ( status == SUCCESS )
    {
        // Try to proccess the packet
        if ( pkt.method & 0x01 )
        {
            // It's a client message (odd method)
            if ( pfnClientProcessMsgCB != NULL )
            {
                status = (*pfnClientProcessMsgCB)( pL2capMsg->connHandle, &pkt );
            }
            else
            {
                // Unsupported request
                status = ATT_ERR_UNSUPPORTED_REQ;
            }
        }
        else
        {
            // It's a server message (even method)
            if ( pfnServerProcessMsgCB != NULL )
            {
                status = (*pfnServerProcessMsgCB)( pL2capMsg->connHandle, &pkt );
            }
            else
            {
                // Unsupported request
                status = ATT_ERR_UNSUPPORTED_REQ;
            }

            // Do not send an error response back for any command
            if ( ( status != SUCCESS ) && ( pkt.cmd == FALSE ) )
            {
                attErrorRsp_t errorRsp;
                // Send an Error Response back to the client
                errorRsp.reqOpcode = pkt.method;
                errorRsp.handle = GATT_INVALID_HANDLE;
                // Status should be either Invalid PDU or Unsupported request
                errorRsp.errCode = status;
                VOID ATT_ErrorRsp( pL2capMsg->connHandle, &errorRsp );
            }
        }
    }

    if ( pL2capMsg->pkt.pPayload != NULL )
    {
        // Received buffer is processed so it's safe to free it
        osal_bm_free( pL2capMsg->pkt.pPayload );
    }

    // Consider received message processed unless it's an ATT Write Command
    // or Notification passed up to the App. Only these two ATT messages don't
    // have a built-in flow control (i.e., don't have corresponding ATT Response
    // or Confirmation respectively), therefore a buffer credit is given to the
    // Controller here for all other ATT message types.
    if ( ( status != SUCCESS )               ||
            ( !WRITE_CMD( pkt.method, pkt.cmd ) &&
              ( pkt.method != ATT_HANDLE_VALUE_NOTI ) ) )
    {
        L2CAP_HostNumCompletedPkts( pL2capMsg->connHandle, 1 );
    }
}

/*********************************************************************
    @fn          gattNotifyEvent

    @brief       Send an event to upper layer application/protocol.

    @param       taskId - application task
    @param       connHandle - connection event belongs to
    @param       status - status
    @param       method - type of message
    @param       pMsg - pointer to message to be sent

    @return      SUCCESS or bleMemAllocError
*/
bStatus_t gattNotifyEvent( uint8 taskId, uint16 connHandle, uint8 status,
                           uint8 method, gattMsg_t* pMsg )
{
    gattMsgEvent_t* pEvent;
    pEvent = (gattMsgEvent_t*)osal_msg_allocate( sizeof( gattMsgEvent_t ) );

    if ( pEvent != NULL )
    {
        // Set up the OSAL message header
        pEvent->hdr.event = GATT_MSG_EVENT;
        pEvent->hdr.status = status;
        pEvent->connHandle = connHandle;
        pEvent->method = method;

        if ( pMsg != NULL )
        {
            VOID osal_memcpy( &(pEvent->msg), pMsg, sizeof( gattMsg_t ) );
        }
        else
        {
            VOID osal_memset( &(pEvent->msg), 0, sizeof( gattMsg_t ) );
        }

        // send message through task message
        VOID osal_msg_send( taskId, (uint8*)pEvent );
        return ( SUCCESS );
    }

    return ( bleMemAllocError );
}

/*********************************************************************
    @fn      gattStartTimer

    @brief   Start a timer to expire in n seconds.

    @param   pfnCbTimer - callback function to be called when timer expires
    @param   pData - data to be passed in to callback function
    @param   timeout - in seconds.
    @param   pTimerId - will point to new timer Id (if not null)

    @return  none
*/
void gattStartTimer( pfnCbTimer_t pfnCbTimer, uint8* pData, uint16 timeout, uint8* pTimerId )
{
    // Timeout is in msec
    VOID osal_CbTimerStart( pfnCbTimer, pData, (timeout * 1000), pTimerId );
}

/*********************************************************************
    @fn      gattStopTimer

    @brief   Stop an active timer for a given channel.

    @param   pTimerId - pointer to timer id

    @return  none
*/
void gattStopTimer( uint8* pTimerId )
{
    if ( ( pTimerId != NULL ) && TIMER_VALID( *pTimerId ) )
    {
        // Stop the timer
        VOID osal_CbTimerStop( *pTimerId );
        // Reset timer id
        *pTimerId = INVALID_TIMER_ID;
    }
}


/****************************************************************************
****************************************************************************/
