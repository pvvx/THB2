/*************************************************************************************************
    Filename:       gap_linkmgr.c
    Revised:
    Revision:

    Description:    This file contains the GAP Link Manager and the GAP Link
                  Database.

	SDK_LICENSE

**************************************************************************************************/



/*******************************************************************************
    INCLUDES
*/

#include "bcomdef.h"
#include "gap.h"
#include "gap_internal.h"
#include "linkdb.h"
#include "sm.h"
#include "sm_internal.h"
#include "smp.h"
#include "jump_function.h"
#include "att.h"

/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/

// Authentication States
#define AUTHSTATE_PAIRING                   2

/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    GLOBAL VARIABLES
*/

// Created when an establish connections requested and freed when the
// connection is made.
gapEstLinkReq_t* pEstLink = NULL;

// Created when a pairing is requested and freed when the pairing is complete.
gapAuthStateParams_t* pAuthLink[MAX_NUM_LL_CONN] = {NULL};

// Callback function pointers for Central
gapCentralConnCBs_t* pfnCentralConnCBs = NULL;

/*********************************************************************
    EXTERNAL VARIABLES
*/

/*********************************************************************
    EXTERNAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/

// Flag to disconnect all connections when requested.
static uint8 terminateAllTaskID = 0;
static uint8 terminateReason;

/*********************************************************************
    LOCAL FUNCTIONS
*/

static void sendTerminateEvent( uint8 status, uint8 taskID,
                                uint16 connectionHandle, uint8 reason );
static bStatus_t disconnectNext( uint8 taskID, uint8 reason );
static void sendAuthEvent( uint8 status, uint16 connectionHandle,
                           uint8 authState,
                           smSecurityInfo_t* pDevEncParams );
static void gapFreeAuthLink( uint16 connectionHandle );

/*********************************************************************
    API FUNCTIONS
*/

/*********************************************************************
    Terminate a link connection.

    Public function defined in gap.h.
*/
bStatus_t GAP_TerminateLinkReq( uint8 taskID, uint16 connectionHandle, uint8 reason )
{
    bStatus_t stat = bleIncorrectMode;   // Return status

    // Are we trying to terminate a Establish Connection Request?
    if ( connectionHandle == GAP_CONNHANDLE_INIT )
    {
        if ( pfnCentralConnCBs && pfnCentralConnCBs->pfnCancelLinkReq )
        {
            stat = pfnCentralConnCBs->pfnCancelLinkReq( taskID, connectionHandle );
        }
    }
    // Terminate All Connections?
    else if ( connectionHandle == GAP_CONNHANDLE_ALL )
    {
        // First are we currently trying to establish a connection?
        if ( pfnCentralConnCBs && pfnCentralConnCBs->pfnCancelLinkReq )
        {
            stat = pfnCentralConnCBs->pfnCancelLinkReq( taskID, connectionHandle );
        }

        if ( stat == SUCCESS )
        {
            terminateAllTaskID = taskID;
            terminateReason = reason;
        }
        else
        {
            stat = disconnectNext( taskID, reason );

            if ( stat == SUCCESS )
            {
                terminateAllTaskID = taskID;
                terminateReason = reason;
            }
        }
    }
    else
    {
        // First check for an existing connection
        linkDBItem_t* pItem = linkDB_Find( connectionHandle );

        if ( pItem )
        {
            if ( pItem->taskID == taskID )
            {
                stat = HCI_DisconnectCmd( connectionHandle, reason );
            }
            else
            {
                // Not the correct taskID
                stat = bleInvalidTaskID;
            }
        }
    }

    return ( stat );
}

/*********************************************************************
    Set up the connection to accept signed data.

    Public function defined in gap.h.
*/
bStatus_t GAP_Signable( uint16 connectionHandle, uint8 authenticated, smSigningInfo_t* pParams )
{
    linkDBItem_t* pLinkItem;

    // Check the GAP Role
    if ( (gapProfileRole == GAP_PROFILE_OBSERVER) || (gapProfileRole == GAP_PROFILE_BROADCASTER) )
    {
        return ( bleIncorrectMode );
    }

    // Check the parameters
    if ( pParams == NULL )
    {
        return ( INVALIDPARAMETER );
    }

    // Check if the connection is real
    pLinkItem = linkDB_Find( connectionHandle );

    if ( pLinkItem == NULL )
    {
        return ( bleNotConnected );
    }

    // Is this signing information authenticated?
    if ( authenticated )
    {
        // Mark the link as authenticated.
        pLinkItem->stateFlags |= LINK_AUTHENTICATED;
    }

    // update the link's signing info
    VOID osal_memcpy( pLinkItem->sec.srk, pParams->srk, KEYLEN );
    pLinkItem->sec.signCounter = pParams->signCounter;
    return ( SUCCESS );
}

/*********************************************************************
    Start the Authentication process with the requested device.
    This function is used to Initiate/Allow pairing.
    Called by both master and slave device (Central and Peripheral).

    NOTE:        This function is called after the link is established.

    Public function defined in gap.h.
*/
bStatus_t GAP_Authenticate( gapAuthParams_t* pParams, gapPairingReq_t* pPairReq )
{
    linkDBItem_t* pLinkItem;
    bStatus_t ret;

    // Check the GAP Role, also the pairReq parameter should only be used by a Peripheral.
    if ( ((gapProfileRole == GAP_PROFILE_OBSERVER) || (gapProfileRole == GAP_PROFILE_BROADCASTER))
            || ((gapProfileRole == GAP_PROFILE_CENTRAL) && (pPairReq)) )
    {
        return ( bleIncorrectMode );
    }

    // Check the parameter
    if ( pParams == NULL )
    {
        return ( INVALIDPARAMETER );
    }

    // Make sure we aren't already in the pairing mode.
    if ( pAuthLink[pParams->connectionHandle] )
    {
        return ( bleAlreadyInRequestedMode );
    }

    // Check for a valid connection handle
    pLinkItem = linkDB_Find( pParams->connectionHandle );

    if ( pLinkItem == NULL )
    {
        return ( bleNotConnected );
    }

    // Allocate the pairing state variables
    pAuthLink[pParams->connectionHandle] = (gapAuthStateParams_t*)osal_mem_alloc( (uint16)(sizeof ( gapAuthStateParams_t )) );

    if ( pAuthLink[pParams->connectionHandle] == NULL )
    {
        return ( bleMemAllocError );
    }

    VOID osal_memset( pAuthLink[pParams->connectionHandle], 0, (int)(sizeof( gapAuthStateParams_t )) );
    // Copy existing items
    pAuthLink[pParams->connectionHandle]->connectionHandle = pParams->connectionHandle;
    VOID osal_memcpy( &(pAuthLink[pParams->connectionHandle]->secReqs), &(pParams->secReqs), sizeof ( smLinkSecurityReq_t ) );
    // Start authentication
    pAuthLink[pParams->connectionHandle]->state = AUTHSTATE_PAIRING;
    // Let's try pairing
//  ret = SM_StartPairing( ((gapProfileRole & GAP_PROFILE_CENTRAL) ? TRUE : FALSE),
    ret = SM_StartPairing(    ((pLinkItem->role == LL_LINK_CONNECT_COMPLETE_MASTER ) ? TRUE : FALSE),
                              gapTaskID,
                              pParams->connectionHandle,
                              &(pAuthLink[pParams->connectionHandle]->secReqs) );

    if ( pPairReq )
    {
        // We already received the Pairing Request, so force the processing of it now.
        smProcessPairingReq( pLinkItem, pPairReq );
    }

    // If something went wrong, clear the auth memory
    if ( ret != SUCCESS )
    {
        gapFreeAuthLink(pParams->connectionHandle);
    }

    return ( ret );
}

/*********************************************************************
    Terminate an authentication/pairing process.
    Called to send a pairing failure and stop the pairing process.

    Public function defined in gap.h.
*/
bStatus_t GAP_TerminateAuth( uint16 connectionHandle, uint8 reason )
{
    // Check for valid connection handle
    if ( linkDB_Find( connectionHandle ) == NULL )
    {
        return ( bleNotConnected );
    }

    // Simulate a pairing failed message
    smpPairingFailed_t failedMsg;
    failedMsg.reason = reason;
    return ( smSendFailAndEnd( connectionHandle, &failedMsg ) );
}

/*********************************************************************
    Update the passkey.  This function is called by the
    application/profile in response to receiving the GAP_PASSKEY_NEEDED_EVENT
    message.

    Public function defined in gap.h.
*/
bStatus_t GAP_PasskeyUpdate( uint8* pPasskey, uint16 connectionHandle )
{
    uint8 x;                // string index - starting from the end
    uint32 y;               // 10's place hold: 1, 10, 100, 1000 ...
    uint32 key = 0;         // Numeric Key Value

    // Check the parameter
    if ( pPasskey == NULL )
    {
        return ( INVALIDPARAMETER );
    }

    // Convert the string to key value
    for ( y = 1, x = PASSKEY_LEN; x > 0; x-- )
    {
        // Get the string character
        uint8 c = pPasskey[x-1];

        // Numeric check '0' - '9'
        if ( c <= '9' && c >= '0' )
        {
            // Convert character to decimal and multiply it
            // times the 10's place holder, then add it to
            // the key.
            key += (c - '0') * y;
        }
        else
        {
            return ( INVALIDPARAMETER );
        }

        // Move to the next 10's place:
        //   1, 10, 100, 1000 ...
        y = y * 10;
    }

    return ( GAP_PasscodeUpdate( key, connectionHandle ) );
}

/*********************************************************************
    Update the passkey.  This function is called by the
    application/profile in response to receiving the GAP_PASSKEY_NEEDED_EVENT
    message.

    Public function defined in gap.h.
*/
bStatus_t GAP_PasscodeUpdate( uint32 passcode, uint16 connectionHandle )
{
    uint8 tk[KEYLEN];   // typical key format

    // Check for valid connection handle
    if ( linkDB_Find( connectionHandle ) == NULL )
    {
        return ( bleIncorrectMode );
    }

    // Make sure the passcode is within range
    if ( passcode > GAP_PASSCODE_MAX )
    {
        return ( INVALIDPARAMETER );
    }

    // Convert passcode to key
    VOID osal_memset( tk, 0, KEYLEN );
    tk[0] = BREAK_UINT32( passcode, 0 );
    tk[1] = BREAK_UINT32( passcode, 1 );
    tk[2] = BREAK_UINT32( passcode, 2 );
    tk[3] = BREAK_UINT32( passcode, 3 );
    // Pass key to SM
    return ( SM_PasskeyUpdate( tk, connectionHandle ) );
}

/*********************************************************************
    Set up the connection's bound paramaters.

    NOTE:        This function is called after the link is established.

    Public function defined in gap.h.
*/
bStatus_t GAP_Bond( uint16 connectionHandle, uint8 authenticated,
                    smSecurityInfo_t* pParams, uint8 startEncryption )
{
    linkDBItem_t* pLinkItem;   // Pointer to connection information
    bStatus_t ret;            // return value

    // Check GAP Role
    if ( (gapProfileRole == GAP_PROFILE_OBSERVER) || (gapProfileRole == GAP_PROFILE_BROADCASTER) )
    {
        return ( bleIncorrectMode );
    }

    // Check security parameter
    if ( pParams == NULL )
    {
        return ( INVALIDPARAMETER );
    }

    // Check for valid connection
    pLinkItem = linkDB_Find( connectionHandle );

    if ( pLinkItem == NULL )
    {
        return ( bleNotConnected );
    }

    // Copy over Bound information
    if ( pLinkItem->pEncParams )
    {
        // If bound information already exists, free it
        osal_mem_free( pLinkItem->pEncParams );
    }

    // Duplicate the params into the link information
    pLinkItem->pEncParams = (encParams_t*)osal_memdup( pParams, sizeof( encParams_t ) );

    if ( pLinkItem->pEncParams == NULL )
    {
        // Report memory error
        return ( bleMemAllocError );
    }

    // Central device?
    if ( ( gapProfileRole & GAP_PROFILE_CENTRAL ) && startEncryption )
    {
        // Start the key loading
        ret = smStartEncryption( connectionHandle, pLinkItem->pEncParams->ltk,
                                 pLinkItem->pEncParams->div, pLinkItem->pEncParams->rand,
                                 pLinkItem->pEncParams->keySize );
    }
    else
    {
        ret = SUCCESS;
    }

    // Mark the link as bound
    pLinkItem->stateFlags |= LINK_BOUND;

    // Is this bond authenticated?
    if ( authenticated )
    {
        // Mark the link as authenticated.
        pLinkItem->stateFlags |= LINK_AUTHENTICATED;
    }

    return ( ret );
}

/*********************************************************************
    PUBLIC INTERNAL GAP FUNCTIONS
*/

/*********************************************************************
    @fn          gapProcessConnectionCompleteEvt

    @brief       Process the LL HCI Connection Complete Event for the
                call to HCI_BLECreateLLConnCmd().

    @param       pkt - HCI packet

    @return      none
*/
/*
    bugfix for multi-role llConnState_t.llTbd1 for link role
*/
#include "ll.h"
#include "ll_def.h"
void gapProcessConnectionCompleteEvt( hciEvt_BLEConnComplete_t* pPkt )
{
    uint8 appTaskID;
    /*bugfix for multi-role */
    llConnState_t* connPtr;
    connPtr = &conn_param[ pPkt->connectionHandle ];
    connPtr->llTbd1 = pPkt->role;
    /*bugfix end */

    // Check parameter
    if ( pPkt == NULL )
    {
        return;
    }

    // Determine who gets the event
    if ( pEstLink )
    {
        appTaskID = pEstLink->taskID;  // Task that requested the connection
    }
    else
    {
        appTaskID = gapAppTaskID;   // Task controlling the GAP
    }

    // The LL stopped advertising, clean up the advertising state.
    if ( pfnPeripheralCBs && pfnPeripheralCBs->pfnProcessAdvertisingEvt )
    {
        pfnPeripheralCBs->pfnProcessAdvertisingEvt( FALSE );
    }

    // Convert from LL address type to Host address type
    pPkt->peerAddrType = gapDetermineAddrType( pPkt->peerAddrType, pPkt->peerAddr );
    // Send the connection notification to the app
    sendEstLinkEvent( pPkt->status, appTaskID, pPkt->peerAddrType,
                      pPkt->peerAddr, pPkt->connectionHandle,
                      pPkt->connInterval, pPkt->connLatency,
                      pPkt->connTimeout, pPkt->clockAccuracy );

    if ( pPkt->status == SUCCESS )
    {
        // Make link database entry
        pPkt->status = linkDB_Add( appTaskID, pPkt->connectionHandle, LINK_CONNECTED,pPkt->role,
                                   pPkt->peerAddrType, pPkt->peerAddr, pPkt->connInterval );
    }

    if ( pEstLink )
    {
        // Get rid of the establish link information
        gapFreeEstLink();

        // Are we in the middle of a disconnect all
        if ( terminateAllTaskID )
        {
            // disconnect the next one
            if ( disconnectNext( terminateAllTaskID, terminateReason ) != SUCCESS )
            {
                // No more, send terminate message
                sendTerminateEvent( pPkt->status, terminateAllTaskID,
                                    pPkt->connectionHandle, pPkt->status );
                terminateAllTaskID = 0;
            }
        }
    }

    ATT_UpdateMtuSize(pPkt->connectionHandle, ATT_MTU_SIZE_MIN);
    L2CAP_ReassemblePkt_Reset(pPkt->connectionHandle);
    L2CAP_SegmentPkt_Reset(pPkt->connectionHandle);
}

/*********************************************************************
    @fn          gapProcessConnUpdateCompleteEvt

    @brief       Process the LL HCI Connection Update Complete Event for the
                call to HCI_BLEUpdateLLConnCmd().

    @param       pkt - HCI packet

    @return      none
*/
void gapProcessConnUpdateCompleteEvt( hciEvt_BLEConnUpdateComplete_t* pPkt )
{
    // Send the link update notification to the app
    gapSendLinkUpdateEvent( pPkt->status, pPkt->connectionHandle,
                            pPkt->connInterval, pPkt->connLatency,
                            pPkt->connTimeout );
}

/*********************************************************************
    @fn          gapProcessDisconnectCompleteEvt

    @brief       Process the LL Disconnection Complete Event for the
                call to HCI_DisconnectCmd().

    @param       pkt - HCI packet

    @return      none
*/
void gapProcessDisconnectCompleteEvt( hciEvt_DisconnComplete_t* pPkt )
{
    uint8 taskID = 0;
    linkDBItem_t* pItem = linkDB_Find( pPkt->connHandle ); // Find connection information

    if ( pItem )
    {
        // if found, delete the link database entry
        taskID = pItem->taskID;
        VOID linkDB_Remove( pPkt->connHandle );
    }

    // Are we deleting all connections for a task ID?
    if ( (terminateAllTaskID) && (taskID == terminateAllTaskID) )
    {
        // Disconnect the next connection that matches the task ID
        if ( disconnectNext( terminateAllTaskID, terminateReason ) == SUCCESS )
        {
            // Don't send message now, we have more to terminate
            taskID = 0;
        }
        else
        {
            // No more to disconnect
            terminateAllTaskID = 0;
        }
    }

    if ( taskID )
    {
        // Send the event to the app/profile
        sendTerminateEvent( pPkt->status, taskID, pPkt->connHandle, pPkt->reason );
    }

    // If we are in the middle of a pairing?
    if ( pAuthLink[pPkt->connHandle] )
    {
        // Just remove the item
        gapFreeAuthLink(pPkt->connHandle);
    }

    ATT_UpdateMtuSize(pPkt->connHandle, ATT_MTU_SIZE_MIN);
    L2CAP_ReassemblePkt_Reset(pPkt->connHandle);
    L2CAP_SegmentPkt_Reset(pPkt->connHandle);
}

/*********************************************************************
    @fn          sendEstLinkEvent

    @brief       Build and send the GAP_LINK_ESTABLISHED_EVENT to the app.

    @param       status - connection status
    @param       taskID - where to send message
    @param       devAddrType - connected device's device address type
    @param       pDevAddr - connected device's device address
    @param       connectionHandle - controller link connection handle.
    @param       connInterval - connection interval
    @param       connTimeout - connection timeout
    @param       clockAccuracy - LL clock accuracy

    @return      none
*/
void sendEstLinkEvent( uint8 status, uint8 taskID, uint8 devAddrType,
                       uint8* pDevAddr, uint16 connectionHandle,
                       uint16 connInterval, uint16 connLatency,
                       uint16 connTimeout, uint16 clockAccuracy )
{
    gapEstLinkReqEvent_t* pRsp;
    pRsp = (gapEstLinkReqEvent_t*)osal_msg_allocate( (uint16)(sizeof ( gapEstLinkReqEvent_t )) );

    if ( pRsp )
    {
        // Build the message
        pRsp->hdr.event = GAP_MSG_EVENT;
        pRsp->hdr.status = status;
        pRsp->opcode = GAP_LINK_ESTABLISHED_EVENT;
        pRsp->devAddrType = devAddrType;

        // If the address parameter is valid
        if ( pDevAddr )
        {
            VOID osal_memcpy( pRsp->devAddr, pDevAddr, B_ADDR_LEN );
        }
        else
        {
            // Not valid, zero field
            VOID osal_memset( pRsp->devAddr, 0, B_ADDR_LEN );
        }

        pRsp->connectionHandle = connectionHandle;
        pRsp->connInterval = connInterval;
        pRsp->connLatency = connLatency;
        pRsp->connTimeout = connTimeout;
        pRsp->clockAccuracy = clockAccuracy;
        VOID osal_msg_send( taskID, (uint8*)pRsp );
    }
}

/*********************************************************************
    @fn          sendTerminateEvent

    @brief       Build and send the GAP_LINK_TERMINATED_EVENT to the app.

    @param       status - terminate status
    @param       taskID - where to send message
    @param       connectionHandle - controller link connection handle.
    @param       reason - termination reason - (LL)

    @return      none
*/
static void sendTerminateEvent( uint8 status, uint8 taskID,
                                uint16 connectionHandle, uint8 reason )
{
    // Allocate, build and send Terminate event
    gapTerminateLinkEvent_t* pRsp;
    pRsp = (gapTerminateLinkEvent_t*)osal_msg_allocate( (uint16)(sizeof ( gapTerminateLinkEvent_t )) );

    if ( pRsp )
    {
        pRsp->hdr.event = GAP_MSG_EVENT;
        pRsp->hdr.status = status;
        pRsp->opcode = GAP_LINK_TERMINATED_EVENT;
        pRsp->connectionHandle = connectionHandle;
        pRsp->reason = reason;
        VOID osal_msg_send( taskID, (uint8*)pRsp );
    }
}

/*********************************************************************
    @fn          gapSendLinkUpdateEvent

    @brief       Build and send the GAP_LINK_PARAM_UPDATE_EVENT to the app.

    @param       status - update status
    @param       connectionHandle - controller link connection handle
    @param       connInterval - connection interval
    @param       connLatency - conenction latency
    @param       connTimeout - connection timeout

    @return      none
*/
void gapSendLinkUpdateEvent( uint8 status, uint16 connectionHandle,
                             uint16 connInterval, uint16 connLatency,
                             uint16 connTimeout )
{
    gapLinkUpdateEvent_t* rsp;
    // Allocate, build and send Link Update event
    rsp = (gapLinkUpdateEvent_t*)osal_msg_allocate( (uint16)(sizeof ( gapLinkUpdateEvent_t )) );

    if ( rsp )
    {
        uint8 appTaskID;
        // Determine who gets the event
        linkDBItem_t* pItem = linkDB_Find( connectionHandle );

        if ( pItem )
        {
            appTaskID = pItem->taskID;
        }
        else
        {
            appTaskID = gapAppTaskID;
        }

        // Build the message
        rsp->hdr.event = GAP_MSG_EVENT;
        rsp->hdr.status = status;
        rsp->opcode = GAP_LINK_PARAM_UPDATE_EVENT;
        rsp->connectionHandle = connectionHandle;
        rsp->connInterval = connInterval;
        rsp->connLatency = connLatency;
        rsp->connTimeout = connTimeout;
        // Send the connection update to the app
        VOID osal_msg_send( appTaskID, (uint8*)rsp );
    }
}

/*********************************************************************
    @fn          disconnectNext

    @brief       if we are in the middle of a disconnect all command
                this function will send a disconnect for the next
                connection for an app.

    @param       taskID - task ID of the app/profile that created the task
    @param       reason - disconnect reason

    @return      SUCCESS if initiated
                bleIncorrectMode if connection not found
*/
static bStatus_t disconnectNext( uint8 taskID, uint8 reason )
{
    linkDBItem_t* pItem = linkDB_FindFirst( taskID );

    if ( pItem )
    {
        return ( HCI_DisconnectCmd( pItem->connectionHandle, reason ) );
    }
    else
    {
        return ( bleIncorrectMode );
    }
}

/*********************************************************************
    @fn          sendAuthEvent

    @brief       Build and send the GAP_AUTHENTICATION_COMPLETE_EVENT to the app.

    @param       status - authentication status
    @param       connectionHandle - controller link connection handle.
    @param       authState - TRUE if the pairing was authenticated, FALSE otherwise
    @param       devEncParams - pointer to the connected device's encryption parameters

    @return      none
*/
static void sendAuthEvent( uint8 status, uint16 connectionHandle,
                           uint8 authState,
                           smSecurityInfo_t* pDevEncParams )
{
    linkDBItem_t* pLink; // pointer to connection information
    // Find the connection information
    pLink = linkDB_Find( connectionHandle );

    // Was the connection information found and we are currently
    // performing a pairing process?
    if ( (pLink != NULL) && (pAuthLink[connectionHandle] != NULL) )
    {
        gapAuthCompleteEvent_t* pRsp;   // pointer to Event message
        uint16 len;                     // Length of Event message
        // Calculate the length of the event message
        len = sizeof ( gapAuthCompleteEvent_t );

        if ( pAuthLink[connectionHandle]->pSecurityInfo )
        {
            len += sizeof ( smSecurityInfo_t );
        }

        if ( pAuthLink[connectionHandle]->pSigningInfo )
        {
            len += sizeof ( smSigningInfo_t );
        }

        if ( pDevEncParams )
        {
            len += sizeof ( smSecurityInfo_t );
        }

        if ( pAuthLink[connectionHandle]->pIdentityInfo )
        {
            len += sizeof ( smIdentityInfo_t );
        }

        // Allocate the message with the calculated length
        pRsp = (gapAuthCompleteEvent_t*)osal_msg_allocate( len );

        if ( pRsp )
        {
            uint16 taskID;                // Where to send the message
            uint8* pBuf;
            // Clear the message
            VOID osal_memset( pRsp, 0, sizeof ( gapAuthCompleteEvent_t ) );
            // Assume that GAP's App/Profile controller ID
            taskID = GAP_GetParamValue( TGAP_AUTH_TASK_ID );

            if ( taskID == 0 )
            {
                // default to the connection's task ID
                taskID = pLink->taskID;
            }

            // Build and send
            pRsp->hdr.event = GAP_MSG_EVENT;
            pRsp->hdr.status = status;
            pRsp->opcode = GAP_AUTHENTICATION_COMPLETE_EVENT;
            pRsp->connectionHandle = connectionHandle;
            pRsp->authState = authState;
            pBuf = (uint8*)(pRsp + 1);

            // If it exists, copy security information generated by this device
            if ( pAuthLink[connectionHandle]->pSecurityInfo )
            {
                pRsp->pSecurityInfo = (smSecurityInfo_t*)pBuf;
                pBuf = (uint8*)(pRsp->pSecurityInfo + 1);
                VOID osal_memcpy( pRsp->pSecurityInfo, pAuthLink[connectionHandle]->pSecurityInfo, sizeof ( smSecurityInfo_t ) );
            }

            // If it exists, copy the connected device's signing information
            if ( pAuthLink[connectionHandle]->pSigningInfo )
            {
                pRsp->pSigningInfo = (smSigningInfo_t*)pBuf;
                pBuf = (uint8*)(pRsp->pSigningInfo + 1);
                VOID osal_memcpy( pRsp->pSigningInfo, pAuthLink[connectionHandle]->pSigningInfo, sizeof ( smSigningInfo_t ) );
            }

            // if it exists, copy security information for the connected device
            if ( pDevEncParams )
            {
                pRsp->pDevSecInfo = (smSecurityInfo_t*)pBuf;
                pBuf = (uint8*)(pRsp->pDevSecInfo + 1);
                VOID osal_memcpy( pRsp->pDevSecInfo, pDevEncParams, sizeof ( smSecurityInfo_t ) );
            }

            // If it exists, copy the connected device's identification information
            if ( pAuthLink[connectionHandle]->pIdentityInfo )
            {
                pRsp->pIdentityInfo = (smIdentityInfo_t*)pBuf;
                VOID osal_memcpy( pRsp->pIdentityInfo, pAuthLink[connectionHandle]->pIdentityInfo, sizeof ( smIdentityInfo_t ) );
            }

            VOID osal_msg_send( taskID, (uint8*)pRsp );
            // Remove the pAuthLink[connectionHandle]
            gapFreeAuthLink( connectionHandle);
        }
    }
}

/*********************************************************************
    @fn          gapFreeAuthLink

    @brief       Free the allocated memory used in pAuthLink[connectionHandle].

    @param       none

    @return      none
*/
static void gapFreeAuthLink( uint16 connectionHandle )
{
    if ( pAuthLink[connectionHandle] )
    {
        if ( pAuthLink[connectionHandle]->pSecurityInfo )
        {
            osal_mem_free( pAuthLink[connectionHandle]->pSecurityInfo );
        }

        if ( pAuthLink[connectionHandle]->pIdentityInfo )
        {
            osal_mem_free( pAuthLink[connectionHandle]->pIdentityInfo );
        }

        if ( pAuthLink[connectionHandle]->pSigningInfo )
        {
            osal_mem_free( pAuthLink[connectionHandle]->pSigningInfo );
        }

        osal_mem_free( pAuthLink[connectionHandle] );
        pAuthLink[connectionHandle] = NULL;
    }
}

/*********************************************************************
    @fn          gapFreeEstLink

    @brief       Free the establish link memory.

    @param       status - authentication status
    @param       connectionHandle - controller link connection handle.

    @return      none
*/
void gapFreeEstLink( void )
{
    if ( pEstLink )
    {
        // Clear the connection information
        osal_mem_free( pEstLink );
        pEstLink = NULL;
    }
}

/*********************************************************************
    @fn          gapSendBondCompleteEvent

    @brief       Build and send the GAP_BOND_COMPLETE_EVENT to the app.

    @param       status - authentication status
    @param       connectionHandle - controller link connection handle.

    @return      none
*/
void gapSendBondCompleteEvent( uint8 status, uint16 connectionHandle )
{
    linkDBItem_t* pLink;
    pLink = linkDB_Find( connectionHandle );

    if ( pLink )
    {
        uint16 taskID;
        gapBondCompleteEvent_t* pRsp;
        taskID = GAP_GetParamValue( TGAP_AUTH_TASK_ID );

        if ( taskID == 0 )
        {
            taskID = pLink->taskID;
        }

        pRsp = (gapBondCompleteEvent_t*)osal_msg_allocate( sizeof ( gapBondCompleteEvent_t ) );

        if ( pRsp )
        {
            VOID osal_memset( (uint8*)pRsp, 0, sizeof ( gapBondCompleteEvent_t ) );
            pRsp->hdr.event = GAP_MSG_EVENT;
            pRsp->hdr.status = status;
            pRsp->opcode = GAP_BOND_COMPLETE_EVENT;
            pRsp->connectionHandle = connectionHandle;
            VOID osal_msg_send( taskID, (uint8*)pRsp );
        }
    }
}

/*********************************************************************
    @fn          gapSendPairingReqEvent

    @brief       Build and send the GAP_PAIRING_REQ_EVENT to the app.

    @param       status - authentication status
    @param       connectionHandle - controller link connection handle.

    @return      none
*/
void gapSendPairingReqEvent( uint8 status, uint16 connectionHandle,
                             uint8 ioCap,
                             uint8 oobDataFlag,
                             uint8 authReq,
                             uint8 maxEncKeySize,
                             keyDist_t keyDist )
{
    linkDBItem_t* pLink;
    pLink = linkDB_Find( connectionHandle );

    if ( pLink )
    {
        gapPairingReqEvent_t* pRsp;
        pRsp = (gapPairingReqEvent_t*)osal_msg_allocate( sizeof ( gapPairingReqEvent_t ) );

        if ( pRsp )
        {
            uint16 taskID;
            VOID osal_memset( (uint8*)pRsp, 0, sizeof ( gapPairingReqEvent_t ) );
            taskID = GAP_GetParamValue( TGAP_AUTH_TASK_ID );

            if ( taskID == 0 )
            {
                taskID = pLink->taskID;
            }

            pRsp->hdr.event = GAP_MSG_EVENT;
            pRsp->hdr.status = status;
            pRsp->opcode = GAP_PAIRING_REQ_EVENT;
            pRsp->connectionHandle = connectionHandle;
            pRsp->pairReq.ioCap = ioCap;
            pRsp->pairReq.oobDataFlag = oobDataFlag;
            pRsp->pairReq.authReq = authReq;
            pRsp->pairReq.maxEncKeySize = maxEncKeySize;
            pRsp->pairReq.keyDist = keyDist;
            VOID osal_msg_send( taskID, (uint8*)pRsp );
        }
    }
}

/*********************************************************************
    @fn          gapPasskeyNeededCB

    @brief       Send GAP_PASSKEY_NEEDED_EVENT to app

    @param       connectionHandle - connection needing the passkey
    @param       type - SM_PASSKEY_TYPE_INPUT and/or SM_PASSKEY_TYPE_DISPLAY

    @return      none
*/
void gapPasskeyNeededCB( uint16 connectionHandle, uint8 type )
{
    linkDBItem_t* pLinkItem;       // Connection information
    // Find the connection information
    pLinkItem = linkDB_Find( connectionHandle );

    if ( pLinkItem )
    {
        gapPasskeyNeededEvent_t* pRsp; // Pointer to event message
        pRsp = (gapPasskeyNeededEvent_t*)osal_msg_allocate( (uint16)(sizeof ( gapPasskeyNeededEvent_t )) );

        if ( pRsp )
        {
            // Figure out where to send the event
            uint16 taskID = GAP_GetParamValue( TGAP_AUTH_TASK_ID );

            if ( taskID == 0 )
            {
                // default to the connection's task ID
                taskID = pLinkItem->taskID;
            }

            // Build and send
            pRsp->hdr.event = GAP_MSG_EVENT;
            pRsp->hdr.status = SUCCESS;
            pRsp->opcode = GAP_PASSKEY_NEEDED_EVENT;
            VOID osal_memcpy( pRsp->deviceAddr, pLinkItem->addr, B_ADDR_LEN );
            pRsp->connectionHandle = connectionHandle;

            // Set the input field
            if ( type & SM_PASSKEY_TYPE_INPUT )
            {
                pRsp->uiInputs = TRUE;
            }
            else
            {
                pRsp->uiInputs = FALSE;
            }

            // Set the output field
            if ( type & SM_PASSKEY_TYPE_DISPLAY )
            {
                pRsp->uiOutputs = TRUE;
            }
            else
            {
                pRsp->uiOutputs = FALSE;
            }

            VOID osal_msg_send( taskID, (uint8*)pRsp );
        }
    }
}

/*********************************************************************
    @fn          gapPairingCompleteCB

    @brief       Callback function from SM to inform that the Pairing Process
                is complete

    @param       connectionHandle - connection needing the passkey
    @param       type - SM_PASSKEY_TYPE_INPUT and/or SM_PASSKEY_TYPE_DISPLAY

    @return      none
*/
void gapPairingCompleteCB( uint8 status, uint8 initiatorRole,
                           uint16 connectionHandle,
                           uint8 authState,
                           smSecurityInfo_t* pEncParams,
                           smSecurityInfo_t* pDevEncParams,
                           smIdentityInfo_t* pIdInfo,
                           smSigningInfo_t*  pSigningInfo )
{
    linkDBItem_t* pLinkItem; // pointer to the connection information
    // Find the connection information
    pLinkItem = linkDB_Find( connectionHandle );

    if ( pLinkItem )
    {
        // Update the link if encrypted
        if ( status == SUCCESS )
        {
            // Update the encrypted state flag
            pLinkItem->stateFlags |= LINK_ENCRYPTED;

            // Was the pairing authenticated?
            if ( authState & SM_AUTH_STATE_AUTHENTICATED )
            {
                pLinkItem->stateFlags |= LINK_AUTHENTICATED;
            }
            else
            {
                pLinkItem->stateFlags &= ~LINK_AUTHENTICATED;
            }

            // Were there any keys exchanged?
            if ( pEncParams || pDevEncParams || pIdInfo || pSigningInfo )
            {
                pLinkItem->stateFlags |= LINK_BOUND;
            }
            else
            {
                pLinkItem->stateFlags &= ~LINK_BOUND;
            }

            if ( pSigningInfo )
            {
                // Copy the signing information
                pLinkItem->sec.signCounter = pSigningInfo->signCounter;
                VOID osal_memcpy( pLinkItem->sec.srk, pSigningInfo->srk, KEYLEN );
            }

            // Copy off the LTK if slave/responder and LTK exists
            if ( (pEncParams) && (initiatorRole == FALSE) )
            {
                if ( pLinkItem->pEncParams )
                {
                    osal_mem_free( pLinkItem->pEncParams );
                }

                pLinkItem->pEncParams = (encParams_t*)osal_memdup( pEncParams, sizeof ( encParams_t ) );
            }
            else if ( (pDevEncParams) && (initiatorRole == TRUE) )
            {
                if ( pLinkItem->pEncParams )
                {
                    osal_mem_free( pLinkItem->pEncParams );
                }

                pLinkItem->pEncParams = (encParams_t*)osal_memdup( pDevEncParams, sizeof ( encParams_t ) );
            }
        }
    }

    if ( pAuthLink[connectionHandle] )
    {
        // Copy the new Auth data
        if ( pEncParams )
        {
            if ( pAuthLink[connectionHandle]->pSecurityInfo == NULL )
            {
                pAuthLink[connectionHandle]->pSecurityInfo = (smSecurityInfo_t*)osal_mem_alloc( (uint16)(sizeof ( smSecurityInfo_t )) );
            }

            if ( pAuthLink[connectionHandle]->pSecurityInfo )
            {
                VOID osal_memcpy( pAuthLink[connectionHandle]->pSecurityInfo, pEncParams, sizeof( smSecurityInfo_t ) );
            }
        }

        // Copy the Identity information
        if ( pIdInfo )
        {
            if (pAuthLink[connectionHandle]->pIdentityInfo == NULL )
            {
                pAuthLink[connectionHandle]->pIdentityInfo = (smIdentityInfo_t*)osal_mem_alloc( (uint16)(sizeof ( smIdentityInfo_t )) );
            }

            if ( pAuthLink[connectionHandle]->pIdentityInfo )
            {
                VOID osal_memcpy( pAuthLink[connectionHandle]->pIdentityInfo, pIdInfo, sizeof( smIdentityInfo_t ) );
            }
        }

        // Copy the signing information
        if ( pSigningInfo )
        {
            if ( pAuthLink[connectionHandle]->pSigningInfo == NULL )
            {
                pAuthLink[connectionHandle]->pSigningInfo = (smSigningInfo_t*)osal_mem_alloc( (uint16)(sizeof ( smSigningInfo_t )) );
            }

            if ( pAuthLink[connectionHandle]->pSigningInfo )
            {
                VOID osal_memcpy( pAuthLink[connectionHandle]->pSigningInfo, pSigningInfo, sizeof( smSigningInfo_t ) );
            }
        }

        // Send the auth Complete Event message to app
        sendAuthEvent( status, connectionHandle, authState, pDevEncParams );
    }
}

/*********************************************************************
    @fn      gapRegisterCentralConn

    @brief   Register Central's connection-related processing function
            with GAP task.

    @param   pfnCBs - pointer to Central's connection-related processing function

    @return  none
*/
void gapRegisterCentralConn( gapCentralConnCBs_t* pfnCBs )
{
    pfnCentralConnCBs = pfnCBs;
}



/****************************************************************************
****************************************************************************/
