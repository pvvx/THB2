/*************************************************************************************************
    Filename:       gap_centlinkmgr.c
    Revised:
    Revision:

    Description:    This file contains the GAP Celntral Link Manager.


	SDK_LICENSE

**************************************************************************************************/


/*******************************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "gap.h"
#include "gap_internal.h"
#include "sm.h"
#include "sm_internal.h"
#include "smp.h"

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

/*********************************************************************
    API FUNCTIONS
*/

/*********************************************************************
    Establish a link to a slave device.

    Public function defined in gap.h.
*/
bStatus_t GAP_EstablishLinkReq( gapEstLinkReq_t* pParams )
{
    bStatus_t stat;   // Return status

    // This function is only good for Central devices
    if ( (gapProfileRole & GAP_PROFILE_CENTRAL) == 0 )
    {
        return ( bleIncorrectMode );
    }

    // Are are currently scanning?
    if ( gapIsScanning() )
    {
        return ( bleNotReady );
    }

    // Are we already connected to the max number of devices?
    if ( GAP_NumActiveConnections() >= MAX_NUM_LL_CONN )
    {
        return ( bleNoResources );
    }

    // We can only perform one establish connection request at a time.
    if ( pEstLink )
    {
        return ( bleAlreadyInRequestedMode );
    }

    // Make a copy of the link establishment request
    pEstLink = (gapEstLinkReq_t*)osal_mem_alloc( (uint16)(sizeof ( gapEstLinkReq_t )) );

    if ( pEstLink  )
    {
        uint16 scanInterval;
        uint16 scanWindow;
        uint8  ownAddrType;
        VOID osal_memcpy( pEstLink, pParams, (unsigned int)(sizeof ( gapEstLinkReq_t )) );

        // Request LL to make the connection
        if ( pEstLink->highDutyCycle )
        {
            scanInterval = GAP_GetParamValue( TGAP_CONN_HIGH_SCAN_INT );
            scanWindow = GAP_GetParamValue( TGAP_CONN_HIGH_SCAN_WIND );
        }
        else
        {
            scanInterval = GAP_GetParamValue( TGAP_CONN_SCAN_INT );
            scanWindow = GAP_GetParamValue( TGAP_CONN_SCAN_WIND );
        }

        if ( gapDeviceAddrMode == ADDRTYPE_PUBLIC )
        {
            ownAddrType = ADDRTYPE_PUBLIC;
        }
        else
        {
            ownAddrType = ADDRTYPE_RANDOM;
        }

        // Request the LL to make the connection
        stat = HCI_LE_CreateConnCmd( scanInterval, scanWindow, pEstLink->whiteList,
                                     gapAddAddrAdj( pEstLink->addrTypePeer, pEstLink->peerAddr ),
                                     pEstLink->peerAddr, ownAddrType,
                                     GAP_GetParamValue( TGAP_CONN_EST_INT_MIN ),
                                     GAP_GetParamValue( TGAP_CONN_EST_INT_MAX ),
                                     GAP_GetParamValue( TGAP_CONN_EST_LATENCY ),
                                     GAP_GetParamValue( TGAP_CONN_EST_SUPERV_TIMEOUT ),
                                     GAP_GetParamValue( TGAP_CONN_EST_MIN_CE_LEN ),
                                     GAP_GetParamValue( TGAP_CONN_EST_MAX_CE_LEN ) );

        if ( stat != SUCCESS )
        {
            gapFreeEstLink();
        }
    }
    else
    {
        stat = bleMemAllocError;
    }

    return ( stat );
}

/*********************************************************************
    @fn          gapCancelLinkReq

    @brief       Cancel a connection create request.

    @param       taskID - requesting app's task id
    @param       connectionHandle - connection handle of link to terminate

    @return      SUCCESS, bleIncorrectMode, or bleInvalidTaskID
*/
bStatus_t gapCancelLinkReq( uint8 taskID, uint16 connectionHandle )
{
    bStatus_t stat = bleIncorrectMode;   // Return status

    // Are we trying to terminate a Establish Connection Request?
    if ( connectionHandle == GAP_CONNHANDLE_INIT )
    {
        // Does the task ID match the task ID of the Connection Request?
        if ( pEstLink && pEstLink->taskID == taskID )
        {
            // Connection Cancel Cmd
            stat = HCI_LE_CreateConnCancelCmd();
        }
        else
        {
            if ( pEstLink )
            {
                // Not the correct taskID
                stat = bleInvalidTaskID;
            }
        }
    }
    // Terminate All Connections?
    else if ( connectionHandle == GAP_CONNHANDLE_ALL )
    {
        // First are we currently trying to establish a connection
        if ( pEstLink )
        {
            // Connection Cancel Cmd
            stat = HCI_LE_CreateConnCancelCmd();
        }
    }

    return ( stat );
}

/*********************************************************************
    Update the link parameters to a slave device.

    Public function defined in gap.h.
*/
bStatus_t GAP_UpdateLinkParamReq( gapUpdateLinkParamReq_t* pParams )
{
    // This function is only good for Central devices
    if ( (gapProfileRole & GAP_PROFILE_CENTRAL) == 0 )
    {
        return ( bleIncorrectMode );
    }

    // Check parameters
    if ( !HCI_ValidConnTimeParams( pParams->intervalMin,
                                   pParams->intervalMax,
                                   pParams->connLatency,
                                   pParams->connTimeout ) )
    {
        return ( INVALIDPARAMETER );
    }

    // Make sure the physical connection is up
    if ( !linkDB_Up( pParams->connectionHandle ) )
    {
        return ( bleNotConnected );
    }

    return ( HCI_LE_ConnUpdateCmd( pParams->connectionHandle, pParams->intervalMin,
                                   pParams->intervalMax, pParams->connLatency,
                                   pParams->connTimeout, 0, 0 ) );
}

/*********************************************************************
    @fn          gapProcessCreateLLConnCmdStatus

    @brief       Process the status for the HCI_BLECreateLLConnCmd().

    @param       status - create connection command status

    @return      none
*/
void gapProcessCreateLLConnCmdStatus( uint8 status )
{
    if ( (status != SUCCESS) && (pEstLink != NULL) )
    {
        // Send the connection notification to the app
        sendEstLinkEvent( status, pEstLink->taskID,
                          pEstLink->addrTypePeer, pEstLink->peerAddr, 0,
                          // Fill with 0's for now, otherwise fill in what was asked for.
                          0, 0, 0, 0 );
        // Clear the connection information
        gapFreeEstLink();
    }
}

/*********************************************************************
    @fn          gapTerminateConnComplete

    @brief       Process the status for the HCI_BLECreateLLConnCancelCmd().

    @param       none

    @return      none
*/
void gapTerminateConnComplete( void )
{
    // Free the establish link variables
    gapFreeEstLink();
}

/*********************************************************************
    @fn          gapProcessConnUpdateCmdStatus

    @brief       Process the status for the HCI_LE_ConnUpdateCmd().

    @param       status - connection update command status

    @return      none
*/
void gapProcessConnUpdateCmdStatus( uint8 status )
{
    if ( status != SUCCESS )
    {
        // Send the link update notification to the app
        gapSendLinkUpdateEvent( status, 0, 0, 0, 0 );
    }
}

/*********************************************************************
    @fn      gapProcessL2CAPSignalEvent

    @brief   Process L2CAP Signaling messages.

    @param   pCmd - pointer to received command

    @return  none
 *********************************************************************/
void gapProcessL2CAPSignalEvt( l2capSignalEvent_t* pCmd )
{
    #if defined ( TESTMODES )

    if ( GAP_GetParamValue( TGAP_GAP_TESTCODE ) == GAP_TESTMODE_NO_RESPONSE )
    {
        // just ignore the messages
        return;
    }

    #endif // TESTMODES

    // Process the signaling command
    if ( pCmd->opcode == L2CAP_PARAM_UPDATE_REQ )
    {
        // Depending on the parameters of other connections, the master's
        // Host may accept the requested parameters and deliver them to the
        // local Controller or reject the request. If the master's Host accepts
        // the request, it shall send the Connection Parameter Update Response
        // with result 0x00 (Parameters accepted) otherwise it shall set the
        // result to 0x01 (request rejected).
        l2capParamUpdateReq_t* pReq = &(pCmd->cmd.updateReq);
        l2capParamUpdateRsp_t updateRsp;

        if ( HCI_ValidConnTimeParams( pReq->intervalMin,
                                      pReq->intervalMax,
                                      pReq->slaveLatency,
                                      pReq->timeoutMultiplier )
                && ( GAP_GetParamValue( TGAP_REJECT_CONN_PARAMS ) == FALSE ) )
        {
            updateRsp.result = L2CAP_CONN_PARAMS_ACCEPTED;
        }
        else
        {
            updateRsp.result = L2CAP_CONN_PARAMS_REJECTED;
        }

        // First send response back...
        VOID L2CAP_ConnParamUpdateRsp( pCmd->connHandle, pCmd->id, &updateRsp );

        // ...then update Connection Parameters
        if ( updateRsp.result == L2CAP_CONN_PARAMS_ACCEPTED )
        {
            VOID HCI_LE_ConnUpdateCmd( pCmd->connHandle, pReq->intervalMin,
                                       pReq->intervalMax, pReq->slaveLatency,
                                       pReq->timeoutMultiplier, 0, 0 );
        }
    }
}


/****************************************************************************
****************************************************************************/
