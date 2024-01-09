/*************************************************************************************************
    Filename:       l2cap_task.c
    Revised:
    Revision:

    Description:    This file contains the L2CAP Task. It also includes the
                  L2CAP Channel Manager and Resource Manager components.

	SDK_LICENSE

**************************************************************************************************/


/*******************************************************************************
    INCLUDES
*/

#include "bcomdef.h"
#include "osal_bufmgr.h"
#include "osal_cbtimer.h"

#include "hci.h"

#include "l2cap_internal.h"

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
// L2CAP task id
uint8 l2capTaskID;

// L2CAP Fixed Channels table
l2capFixedChannel_t l2capFixedChannels[L2CAP_NUM_FIXED_CHANNELS];

// L2CAP Dynamic Channels table
l2capChannel_t l2capChannels[L2CAP_NUM_CHANNELS];

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
static void l2capProcessOSALMsg( osal_event_hdr_t* pMsg );
static void l2capProcessRxData( hciDataEvent_t* pHciMsg );
static void l2capProcessSignal( uint16 connHandle, l2capPacket_t* pPkt );
static bStatus_t l2capProcessRsp( uint16 connHandle, l2capSignalHdr_t* pHdr, uint8* pData );
static bStatus_t l2capProcessReq( uint16 connHandle, l2capSignalHdr_t* pHdr, uint8* pData );

/*********************************************************************
    API FUNCTIONS
*/

/*********************************************************************
    @fn      L2CAP_Init()

    @brief   Initialize the L2CAP layer.

    @param   taskId - Task identifier for the desired task

    @return  none
*/
void L2CAP_Init( uint8 taskId )
{
    uint8 i;
    l2capTaskID = taskId;

    // Initialize all fixed channel structures
    for ( i = 0; i < L2CAP_NUM_FIXED_CHANNELS; i++ )
    {
        l2capFixedChannels[i].CID = L2CAP_CID_NULL;
    }

    // Initialize all dynamic channel structures
    for ( i = 0; i < L2CAP_NUM_CHANNELS; i++ )
    {
        l2capChannels[i].CID = L2CAP_CID_NULL;
        l2capChannels[i].state = L2CAP_CLOSED;
        l2capChannels[i].timerId = INVALID_TIMER_ID;
    }

    // Register with HCI to receive data message
    HCI_L2CAPTaskRegister( l2capTaskID );
    // resert SAR buffers
    l2capSarBufReset();
    // Register with Link DB to receive link status change callback
    //VOID linkDB_Register( pfnLinkDBCB pFunc ); // needed for dynamic channels
}

/*********************************************************************
    @fn      L2CAP_ProcessEvent()

    @brief   L2CAP Task event processing function. This function should
            be called at periodic intervals when event occur.

    @param   taskId - Task ID
    @param   events  - Bitmap of events

    @return  none
*/
uint16 L2CAP_ProcessEvent( uint8 taskId, uint16 events )
{
    uint8* pMsg;
    VOID taskId; // required by OSAL but not used here

    if ( events & SYS_EVENT_MSG )
    {
        if ( (pMsg = osal_msg_receive( l2capTaskID )) != NULL )
        {
            l2capProcessOSALMsg( (osal_event_hdr_t*)pMsg );
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
    @fn      l2capProcessOSALMsg

    @brief   Process an incoming OSAL task message.

    @param   pMsg - message to process

    @return  none
*/
static void l2capProcessOSALMsg( osal_event_hdr_t* pMsg )
{
    switch ( pMsg->event )
    {
    case HCI_DATA_EVENT:
        // Process incoming HCI Data message
        l2capProcessRxData( (hciDataEvent_t*)pMsg );
        break;

    default:
        // Unknown message - drop it.
        break;
    }
}

/*********************************************************************
    @fn      l2capProcessRxData

    @brief   Process an incoming HCI data message.

    @param   pHciMsg - message to process

    @return  none
*/
static void l2capProcessRxData( hciDataEvent_t* pHciMsg )
{
    l2capPacket_t pkt;
    uint8 free;
    uint16 connHandle = pHciMsg->connHandle;
    // First parse the packet
    free = l2capParsePacket( &pkt, pHciMsg );

    if ( pkt.pPayload != NULL )
    {
        // We received the entire packet; try to process it
        if ( pkt.CID == L2CAP_CID_SIG )
        {
            // Signaling protocol
            l2capProcessSignal( connHandle, &pkt );
        }
        else if ( ( pkt.CID == L2CAP_CID_ATT )
                  || ( pkt.CID == L2CAP_CID_SMP )
                  || ( pkt.CID == L2CAP_CID_GENERIC )  )
        {
            // Application fixed channel data
            if ( FIX_CHANNEL_REC( pkt.CID ).CID != L2CAP_CID_NULL )
            {
                // Forward the data packet up to the application
                // the pkt will be free in high layer-->ATT-->GATT
                if ( l2capNotifyData( FIX_CHANNEL_REC( pkt.CID ).taskId, connHandle, &pkt ) == SUCCESS )
                {
                    // Consider received packet processed unless it's an ATT packet
                    if ( pkt.CID != L2CAP_CID_ATT )
                    {
                        L2CAP_HostNumCompletedPkts( connHandle, 1 );
                    }

                    if ( free==TRUE)
                    {
                        // No need to hold on the received buffer any longer
                        if(pHciMsg->pData != NULL)
                        {
                            osal_bm_free( pHciMsg->pData );
                        }
                    }

                    return; // We're done here
                }
            }
        }
        else
        {
            // Unknown CID
        }

        // We're done processing the packet
        free = TRUE;
    }

    if ( free == TRUE )
    {
        // No need to hold on the received buffer any longer
        if ( pHciMsg->pData != NULL )
        {
            osal_bm_free( pHciMsg->pData );
        }
    }

    // Consider received packet processed
    L2CAP_HostNumCompletedPkts( connHandle, 1 );
}

/*********************************************************************
    @fn      l2capProcessSignal

    @brief   Process an incoming L2CAP signaling packet.

    @param   connHandle - connection packet was received on
    @param   pPkt - packet to process

    @return  none
*/
static void l2capProcessSignal( uint16 connHandle, l2capPacket_t* pPkt )
{
    bStatus_t status = SUCCESS;

    // Make sure we've received enough data to proceed
    if ( pPkt->len >= L2CAP_HDR_SIZE )
    {
        l2capSignalHdr_t hdr;
        // Parse Signaling header
        l2capParseSignalHdr( &hdr, pPkt->pPayload );

        // Make sure the information pPayload length doesn't exceed our Signaling MTU
        if ( hdr.len <= L2CAP_SIG_MTU_SIZE )
        {
            // Multiple commands in Signaling packet is not supported
            if ( pPkt->len == L2CAP_HDR_SIZE + hdr.len )
            {
                // Try to proccess the packet
                if ( hdr.opcode & 0x01 )
                {
                    // It's a response message (odd opcode)
                    status = l2capProcessRsp( connHandle, &hdr, &(pPkt->pPayload[SIGNAL_HDR_SIZE]) );
                }
                else
                {
                    // It's a request message (even opcode)
                    status = l2capProcessReq( connHandle, &hdr, &(pPkt->pPayload[SIGNAL_HDR_SIZE]) );
                }
            }
            else
            {
                // Don't process the packet
                status = FAILURE;
            }
        }
        else
        {
            l2capCmdReject_t cmdReject;
            // Send a Command Reject containing the supported Signaling MTU
            cmdReject.reason = L2CAP_REJECT_SIGNAL_MTU_EXCEED;
            cmdReject.maxSignalMTU = L2CAP_SIG_MTU_SIZE;
            VOID L2CAP_CmdReject( connHandle, hdr.id, &cmdReject );
        }
    }

    if ( status != SUCCESS )
    {
        // The request or response message was received with an error
        l2capHandleRxError( connHandle );
    }
}

/*********************************************************************
    @fn      l2capProcessRsp

    @brief   Process an incoming response message.

    @param   connHandle - connection command was received on
    @param   pHdr - pointer to signaling header
    @param   pData - pointer to command data

    @return  SUCCESS: Command was processed successfully.
            INVALIDPARAMETER: Command length is invalid.
*/
static bStatus_t l2capProcessRsp( uint16 connHandle, l2capSignalHdr_t* pHdr, uint8* pData )
{
    l2capSignalCmd_t cmd;
    l2capChannel_t* pChannel;
    bStatus_t status = FAILURE;
    // We sent the request; find the channel using the local identifier
    pChannel = l2capFindLocalId( pHdr->id );

    if ( pChannel == NULL )
    {
        // Invalid response or channel has been closed
        return ( SUCCESS );
    }

    // Reset the request identifier
    pChannel->id = 0;

    // Parse and process the signaling command
    switch ( pHdr->opcode )
    {
    case L2CAP_CMD_REJECT:
        // Parse the command
        status = l2capParseCmdReject( &cmd, pData, pHdr->len );
        break;
        #if 0 // No longer supported

    case L2CAP_ECHO_RSP:
        if ( pChannel->state != L2CAP_W4_ECHO_RSP )
        {
            // We're not expecting an Echo Response.
            return ( SUCCESS );
        }

        // Parse the response
        status = l2capParseEchoRsp( &cmd, pData, pHdr->len );
        break;

    case L2CAP_INFO_RSP:
        if ( pChannel->state != L2CAP_W4_INFO_RSP )
        {
            // We're not expecting an Information Response.
            return ( SUCCESS );
        }

        // Parse the response
        status = l2capParseInfoRsp( &cmd, pData, pHdr->len ) ;
        break;
        #endif

    case L2CAP_PARAM_UPDATE_RSP:
        if ( pChannel->state != L2CAP_W4_PARAM_UPDATE_RSP )
        {
            // We're not expecting a Connection Parameter Update Response.
            return ( SUCCESS );
        }

        // Parse the response
        status = l2capParseParamUpdateRsp( &cmd, pData, pHdr->len );
        break;

    default:
        // Unknown command
        return ( SUCCESS );
    }

    // Stop the timeout timer for this channel
    l2capStopTimer( pChannel );
    // Forward the response to the application
    l2capNotifySignal( pChannel->taskId, connHandle, status, pHdr->opcode, 0, &cmd );
    // Free the channel
    l2capFreeChannel( pChannel );
    return ( status );
}

/*********************************************************************
    @fn      l2capProcessReq

    @brief   Process an incoming request message.

    @param   connHandle - connection command was received on
    @param   pHdr - pointer to signaling header
    @param   pData - pointer to command data

    @return  SUCCESS: Command was processed successfully.
            INVALIDPARAMETER: Command length is invalid.
*/
static bStatus_t l2capProcessReq( uint16 connHandle, l2capSignalHdr_t* pHdr, uint8* pData )
{
    bStatus_t status = SUCCESS;

    // Parse and process the signaling command
    switch ( pHdr->opcode )
    {
        #if 0 // No longer supported

    case L2CAP_ECHO_REQ:
        // Nothing to parse; just echo back the received data
    {
        l2capEchoRsp_t echoRsp;
        // Send an Echo Response.
        echoRsp.pData = pData;
        echoRsp.len = pHdr->len;
        VOID l2capEchoRsp( connHandle, pHdr->id, &echoRsp );
    }
    break;

    case L2CAP_INFO_REQ:
    {
        l2capSignalCmd_t cmd;
        status = L2CAP_ParseInfoReq( &cmd, pData, pHdr->len ) ;

        if ( status == SUCCESS )
        {
            l2capInfoRsp_t infoRsp;
            // Send an Info Response.
            infoRsp.infoType = cmd.infoReq.infoType;

            if ( infoRsp.infoType == L2CAP_INFO_EXTENDED_FEATURES )
            {
                // Indicate that we support Fixed Channels feature
                infoRsp.info.extendedFeatures = L2CAP_FIXED_CHANNELS;
                infoRsp.result = L2CAP_INFO_SUCCESS;
            }
            else if ( infoRsp.infoType == L2CAP_INFO_FIXED_CHANNELS )
            {
                VOID osal_memset( infoRsp.info.fixedChannels, 0, L2CAP_FIXED_CHANNELS_SIZE );
                // Indicate Fixed Channels that we support
                infoRsp.info.fixedChannels[0] = ( L2CAP_FIXED_CHANNELS_ATT |
                                                  L2CAP_FIXED_CHANNELS_SIG |
                                                  L2CAP_FIXED_CHANNELS_SMP );
                infoRsp.result = L2CAP_INFO_SUCCESS;
            }
            else
            {
                infoRsp.result = L2CAP_INFO_NOT_SUPPORTED;
            }

            VOID l2capInfoRsp( connHandle, pHdr->id, &infoRsp );
        }
    }
    break;
    #endif

    case L2CAP_PARAM_UPDATE_REQ:
    {
        l2capSignalCmd_t cmd;
        status = L2CAP_ParseParamUpdateReq( &cmd, pData, pHdr->len );

        if ( status == SUCCESS )
        {
            // Send the request up to the application for processing
            if ( FIX_CHANNEL_REC( L2CAP_CID_SIG ).CID != L2CAP_CID_NULL )
            {
                l2capNotifySignal( FIX_CHANNEL_REC( L2CAP_CID_SIG ).taskId, connHandle,
                                   SUCCESS, pHdr->opcode, pHdr->id, &cmd );
            }
        }
    }
    break;

    default:
        // Unsupported command -- send a Command Reject back
    {
        l2capCmdReject_t cmdReject;
        cmdReject.reason = L2CAP_REJECT_CMD_NOT_UNDERSTOOD;
        VOID L2CAP_CmdReject( connHandle, pHdr->id, &cmdReject );
    }
    break;
    }

    return ( status );
}



/****************************************************************************
****************************************************************************/
