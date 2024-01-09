/*************************************************************************************************
    Filename:       gap_peridevmgr.c
    Revised:
    Revision:

    Description:    This file contains the GAP Peripheral Device Manager.

	SDK_LICENSE

**************************************************************************************************/



/*******************************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "hci_tl.h"
#include "gap.h"
#include "gap_internal.h"

/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/

#define GAP_LIMITED_ADVERTISING_RESOLUTION      0xEA60 // Timer resolution is 1 minute

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

// App Task ID for Updating Advertising Data
static uint8 gapAdvAppTaskID = INVALID_TASK_ID;

// GAP advertisement data types - tokens
static gapAdvToken_t* pGapAdvTokens  = NULL;

// Buffer to hold advertising data
static gapAdvertisingData_t* pGapAdData = NULL;

// Buffer to hold extra advertising data (SCAN_RSP)
static gapAdvertisingData_t* pGapScanRspData = NULL;

// Limited advertising timeout (in seconds)
static uint16 gapLimitedAdvertisingTimeout = 0;

/*********************************************************************
    LOCAL FUNCTIONS
*/

static uint8 gapPeriProcessHCICmdCompleteEvt( hciEvt_CmdComplete_t* pMsg );
static void gapProcessAdvertisingEvt( uint8 timeout );

static void gapProcessAdvertisingTimeout( void );
static void gapConnectedCleanUpAdvertising( void );
static void gapSendMakeDiscEvent( bStatus_t status, uint16 interval );
static void gapFreeAdvertState( void );
static bStatus_t gapAllocAdvRecs( void );
static void gapSendAdDataUpdateEvent( uint8 adType, uint8 status );
static void gapSendEndDiscoverableEvent( uint8 status );
static uint8 isLimitedDiscoverableMode( void );

/*********************************************************************
    PERIPHERAL CALLBACKS
*/

// GAP Peripheral Callbacks
static gapPeripheralCBs_t gapPeripheralCBs =
{
    gapPeriProcessHCICmdCompleteEvt, // Process HCI Command Complete Event Callback
    gapProcessAdvertisingEvt,        // Process Advertising Event Callback
    gapSetAdvParams                  // Set Advertisement Parameters Callback
};

/*********************************************************************
    PUBLIC FUNCTIONS
*/

/*********************************************************************
    Setup or change advertising.  Also starts advertising.

    Public function defined in gap.h.
*/
bStatus_t GAP_MakeDiscoverable( uint8 taskID, gapAdvertisingParams_t* pParams )
{
    bStatus_t stat;     // Return status

    // Are we already advertising?
    if ( pGapAdvertState )
    {
        return ( bleAlreadyInRequestedMode );
    }

    // Is the profile role set properly to do this function?
    if ( (gapProfileRole & (GAP_PROFILE_BROADCASTER | GAP_PROFILE_PERIPHERAL)) == 0 )
    {
        return ( bleIncorrectMode );
    }

    // Do we have advertising data?
    if ( (pGapAdData == NULL) || (pGapAdData->dataLen == 0) )
    {
        return ( bleNotReady );
    }

    // Is profile role Broadcaster only and trying to send something other than
    // a non-connectable advertisement or scannable event?  Also, make sure that
    // the advertisement type is correct while we are in a connection.
    if ( ((gapProfileRole & GAP_PROFILE_BROADCASTER) || (GAP_NumActiveConnections() > 0))
            && (pParams->eventType != GAP_ADTYPE_ADV_NONCONN_IND)
            && (pParams->eventType != GAP_ADTYPE_ADV_SCAN_IND)
            && (pParams->eventType != GAP_ADTYPE_ADV_IND))
    {
        return ( bleIncorrectMode );
    }

    // Setup the variables needed
    pGapAdvertState = (gapAdvertState_t*)osal_mem_alloc( (uint16)(sizeof ( gapAdvertState_t )) );

    if ( pGapAdvertState )
    {
        // Save parameters
        pGapAdvertState->taskID = taskID;
        pGapAdvertState->state = GAP_ADSTATE_SET_PARAMS;
        VOID osal_memcpy( &(pGapAdvertState->params),
                          pParams, (unsigned int)(sizeof( gapAdvertisingParams_t )) );
        stat = gapSetAdvParams();

        if ( stat != SUCCESS )
        {
            // Failure, free the advertisement state structure
            gapFreeAdvertState();
        }
        else
        {
            if ( gapDeviceAddrMode == ADDRTYPE_PRIVATE_RESOLVE )
            {
                // Setup the timer to change the Private Resolvable Address
                gapPrivateAddrChangeTimeout = GAP_GetParamValue( TGAP_PRIVATE_ADDR_INT );

                // Start a timer
                if ( gapPrivateAddrChangeTimeout )
                {
                    VOID osal_start_reload_timer( gapTaskID, GAP_CHANGE_RESOLVABLE_PRIVATE_ADDR_EVT,
                                                  (uint32)GAP_PRIVATE_ADDR_CHANGE_RESOLUTION );
                }
            }
        }
    }
    else
    {
        // Memory Error
        stat = bleMemAllocError;
    }

    return ( stat );
}

/*********************************************************************
    Setup or change advertising and scan response data.

      NOTE:  if the return status from this function is SUCCESS,
             the task isn't complete until the GAP_ADV_DATA_UPDATE_DONE_EVENT
             is sent to the calling application task.

    Public function defined in gap.h.
*/
bStatus_t GAP_UpdateAdvertisingData( uint8 taskID, uint8 adType,
                                     uint8 dataLen, uint8* pAdvertData )
{
    gapAdvertisingData_t* pDestAdData = NULL;
    bStatus_t stat;

    if ( (gapProfileRole & (GAP_PROFILE_PERIPHERAL | GAP_PROFILE_BROADCASTER)) == 0 )
    {
        // This is only allowed for Peripheral or Broadcaster profile roles
        return ( bleIncorrectMode );
    }

    if ( ((gapProfileRole & (GAP_PROFILE_PERIPHERAL | GAP_PROFILE_BROADCASTER)) == 0) &&
            (adType == FALSE) )
    {
        // Scan Response data is only allowed for Peripheral or Broadcaster profile roles
        return ( bleIncorrectMode );
    }

    // Check for advertisement data type
    if ( adType == TRUE )
    {
        // Advertisement data
        if ( pGapAdData )
        {
            pDestAdData = pGapAdData;
        }
    }
    else
    {
        // Scan Response data
        if ( pGapScanRspData )
        {
            pDestAdData = pGapScanRspData;
        }
    }

    if ( pDestAdData )
    {
        // Make sure data length isn't greater than the allowed size
        if ( dataLen > B_MAX_ADV_LEN )
        {
            return( INVALIDPARAMETER );
        }

        // Check if there's supposed to be data
        if ( ( dataLen > 0 ) && ( pAdvertData == NULL ) )
        {
            return( INVALIDPARAMETER );
        }

        gapAdvAppTaskID = taskID;
        VOID osal_memset( pDestAdData->dataField, 0, B_MAX_ADV_LEN );
        VOID osal_memcpy( pDestAdData->dataField, pAdvertData, dataLen );
        pDestAdData->dataLen = dataLen;

        if ( adType )
        {
            stat = HCI_LE_SetAdvDataCmd( pDestAdData->dataLen, pDestAdData->dataField );
        }
        else
        {
            stat = HCI_LE_SetScanRspDataCmd( pDestAdData->dataLen, pDestAdData->dataField );
        }
    }
    else
    {
        stat = bleIncorrectMode;
    }

    return ( stat );
}

/*********************************************************************
    Stops advertising.

    Public function defined in gap.h.
*/
bStatus_t GAP_EndDiscoverable( uint8 taskID )
{
    // Check for the correct state.
    if ( pGapAdvertState == NULL )
    {
        return ( bleIncorrectMode );
    }

    // Parameter check, make sure it came from the task that started the advertising
    if ( pGapAdvertState->taskID != taskID )
    {
        return ( bleInvalidTaskID );
    }

    // Stop the advertising timer
    VOID osal_stop_timerEx( gapTaskID, GAP_END_ADVERTISING_EVT );
    gapLimitedAdvertisingTimeout = 0;
    // Stop any address changing timers
    VOID osal_stop_timerEx( gapTaskID, GAP_CHANGE_RESOLVABLE_PRIVATE_ADDR_EVT );
    gapPrivateAddrChangeTimeout = 0;
    // Send the disable to HCI advertise enable
    pGapAdvertState->state = GAP_ADSTATE_ENDING;
    return ( HCI_LE_SetAdvEnableCmd( HCI_DISABLE_ADV ) );
}

/*********************************************************************
    Called to setup a GAP Advertisement token.

    Public function defined in gap.h.
*/
bStatus_t GAP_SetAdvToken( gapAdvDataToken_t* pToken )
{
    uint8 adLen;
    uint8 srLen;

    if ( (gapProfileRole & (GAP_PROFILE_PERIPHERAL |GAP_PROFILE_BROADCASTER)) == 0 )
    {
        // Not allowed for this device type
        return ( bleIncorrectMode );
    }

    // Check for an invalid parameter
    if ( (pToken == NULL) || ( gapValidADType( pToken->adType ) == FALSE ) )
    {
        return ( INVALIDPARAMETER );
    }

    // Make sure the token isn't too big
    if ( (pToken->attrLen + ADV_TOKEN_HDR) > B_MAX_ADV_LEN )
    {
        return ( INVALID_MEM_SIZE );
    }

    // Make sure each token is unique
    if ( gapFindAdvToken( pToken->adType ) )
    {
        return ( bleInvalidRange );
    }

    // See if there is enough space in the advertisement or Scan response for this token
    gapCalcAdvTokenDataLen( &adLen, &srLen );

    if ( srLen )
    {
        // Check if it will fit in the scan response data, since we are already
        // using scan response, the advertisement data is already filled.
        if ( (srLen + (pToken->attrLen + ADV_TOKEN_HDR)) > B_MAX_ADV_LEN )
        {
            return ( INVALID_MEM_SIZE );
        }
    }

    // Add token to the list
    return ( gapAddAdvToken( pToken ) );
}

/*********************************************************************
    Called to read a GAP Advertisement token.

    Public function defined in gap.h.
*/
gapAdvDataToken_t* GAP_GetAdvToken( uint8 adType )
{
    gapAdvToken_t* pAdItem;

    if ( (gapProfileRole & (GAP_PROFILE_PERIPHERAL |GAP_PROFILE_BROADCASTER)) == 0 )
    {
        // Not allowed for this device type
        return ((gapAdvDataToken_t*)NULL );
    }

    pAdItem = gapFindAdvToken( adType );

    if ( pAdItem )
    {
        // Found
        return ( pAdItem->pToken );
    }
    else
    {
        // Not found
        return ( (gapAdvDataToken_t*)NULL );
    }
}

/*********************************************************************
    Called to remove a GAP Advertisement token.

    Public function defined in gap.h.
*/
gapAdvDataToken_t* GAP_RemoveAdvToken( uint8 adType )
{
    if ( (gapProfileRole & (GAP_PROFILE_PERIPHERAL |GAP_PROFILE_BROADCASTER)) == 0 )
    {
        // Not allowed for this device type
        return ( (gapAdvDataToken_t*)NULL );
    }

    // Delete the token from the list, but don't free the token space
    // let the application/profile do that
    return ( gapDeleteAdvToken( adType ) );
}

/*********************************************************************
    Called to rebuild and load Advertisement and Scan Response data
    from existing GAP Advertisement Tokens.

    Public function defined in gap.h.
*/
bStatus_t GAP_UpdateAdvTokens( void )
{
    if ( (gapProfileRole & (GAP_PROFILE_PERIPHERAL |GAP_PROFILE_BROADCASTER)) == 0 )
    {
        // Not allowed for this device type
        return ( bleIncorrectMode );
    }

    // Build and update (send advertisement and scan response data to LL)
    return ( gapBuildADTokens() );
}

/*********************************************************************
    PUBLIC INTERNAL GAP FUNCTIONS
*/

/*********************************************************************
    @fn          GAP_PeriDevMgrInit

    @brief       Initialize the GAP Peripheral Dev Manager.

    @param       none

    @return      SUCCESS or bleMemAllocError
*/
bStatus_t GAP_PeriDevMgrInit( void )
{
    // Free the space first
    if ( pGapAdData )
    {
        osal_mem_free( pGapAdData );
        pGapAdData = NULL;
    }

    if ( pGapScanRspData )
    {
        osal_mem_free( pGapScanRspData );
        pGapScanRspData = NULL;
    }

    if ( gapProfileRole & (GAP_PROFILE_PERIPHERAL | GAP_PROFILE_BROADCASTER) )
    {
        // Set up Peripheral's processing functions
        gapRegisterPeripheral( &gapPeripheralCBs );
        return ( gapAllocAdvRecs() );
    }

    gapRegisterPeripheral( NULL );
    return ( SUCCESS );
}

/*********************************************************************
    @fn      gapPeriProcessHCICmdCompleteEvt

    @brief   Process an incoming GAP Peripheral Event.

    @param   pMsg - message to process

    @return  TRUE if processed and safe to deallocate, FALSE if passed
            off to another task.
*/
static uint8 gapPeriProcessHCICmdCompleteEvt( hciEvt_CmdComplete_t* pMsg )
{
    uint8 safeToDealloc = TRUE;

    switch ( pMsg->cmdOpcode )
    {
    case HCI_LE_SET_ADV_PARAM:
        safeToDealloc = gapSetAdvParamsStatus( *(pMsg->pReturnParam) );
        break;

    case HCI_LE_SET_ADV_ENABLE:
        safeToDealloc = gapWriteAdvEnableStatus( pMsg->pReturnParam[0],
                                                 BUILD_UINT16( pMsg->pReturnParam[1], pMsg->pReturnParam[2] ) );
        break;

    case HCI_LE_SET_SCAN_RSP_DATA:
    case HCI_LE_SET_ADV_DATA:
    {
        uint8 adType;

        if ( pMsg->cmdOpcode == HCI_LE_SET_ADV_DATA )
        {
            adType = TRUE;
        }
        else
        {
            adType = FALSE;
        }

        gapWriteAdvDataStatus( adType, *(pMsg->pReturnParam) );
    }
    break;

    case HCI_LE_SET_CONNLESS_CTE_TRANS_PARAMETER:
        LOG("CTE Param status 0x%02X\n",pMsg->pReturnParam[0]);
        break;

    case HCI_LE_SET_CONNLESS_CTE_TRANS_ENABLE:
        LOG("CTE enable status 0x%02X\n",pMsg->pReturnParam[0]);
        break;

    case HCI_LE_SET_CONNLESS_IQ_SAMPLE_ENABLE:
    case HCI_LE_SET_CONNCTE_RECV_PARAMETER:
    case HCI_LE_SET_CONN_CTE_TRANSMIT_PARAMETER:
    case HCI_LE_CONN_CTE_REQUEST_ENABLE:
    case HCI_LE_CONN_CTE_RESPONSE_ENABLE:
    case HCI_LE_READ_ANTENNA_INFO:
        LOG("gapPeriProcessHCICmdCompleteEvt Opcode 0x%X\n",pMsg->cmdOpcode);
        break;

    default:
        safeToDealloc = FALSE;  // send this message to the app
        break;
    }

    return ( safeToDealloc );
}

/*********************************************************************
    @fn          gapAllocAdvRecs

    @brief       Allocate and initialize the advertising data and
                extra advertising data (SCAN_RSP) records.

    @param       none

    @return      SUCCESS or bleMemAllocError
*/
static bStatus_t gapAllocAdvRecs( void )
{
    bStatus_t stat = SUCCESS;

    // Peripheral and Broadcaster's will advertise and support LL SCAN_RSP
    if ( gapProfileRole & (GAP_PROFILE_PERIPHERAL | GAP_PROFILE_BROADCASTER) )
    {
        pGapAdData = (gapAdvertisingData_t*)osal_mem_alloc( (uint16)(sizeof ( gapAdvertisingData_t )) );

        if ( pGapAdData )
        {
            VOID osal_memset( pGapAdData, 0, (int)(sizeof ( gapAdvertisingData_t )) );
            pGapScanRspData = (gapAdvertisingData_t*)osal_mem_alloc( (uint16)(sizeof ( gapAdvertisingData_t )) );

            if ( pGapScanRspData )
            {
                VOID osal_memset( pGapScanRspData, 0, (int)(sizeof ( gapAdvertisingData_t )) );
            }
            else
            {
                osal_mem_free( pGapAdData );
                pGapAdData = NULL;
                stat = bleMemAllocError;
            }
        }
        else
        {
            stat = bleMemAllocError;
        }
    }

    return ( stat );
}

/*********************************************************************
    @fn          gapSetAdvParams

    @brief       Write the Advertisement Parameters from the saved parameters
                in pGapAdvertState.

    @param       none

    @return      SUCCESS
                FAILURE
*/
bStatus_t gapSetAdvParams( void )
{
    if ( pGapAdvertState )
    {
        uint16 advIntervalMin;
        uint16 advIntervalMax;
        uint8  advType;
        uint8  ownAddrType;
        // Setup to send the Advertising parameters to the LL
        advType = pGapAdvertState->params.eventType;

        // Are we already in an active connection
        if ( GAP_NumActiveConnections() )
        {
            advIntervalMin = GAP_GetParamValue( TGAP_CONN_ADV_INT_MIN );
            advIntervalMax = GAP_GetParamValue( TGAP_CONN_ADV_INT_MAX );
        }
        else if ( isLimitedDiscoverableMode() )
        {
            advIntervalMin = GAP_GetParamValue( TGAP_LIM_DISC_ADV_INT_MIN );
            advIntervalMax = GAP_GetParamValue( TGAP_LIM_DISC_ADV_INT_MAX );
        }
        else
        {
            advIntervalMin = GAP_GetParamValue( TGAP_GEN_DISC_ADV_INT_MIN );
            advIntervalMax = GAP_GetParamValue( TGAP_GEN_DISC_ADV_INT_MAX );
        }

        if ( gapDeviceAddrMode == ADDRTYPE_PUBLIC )
        {
            ownAddrType = ADDRTYPE_PUBLIC;
        }
        else
        {
            ownAddrType = ADDRTYPE_RANDOM;
        }

        return ( HCI_LE_SetAdvParamCmd( advIntervalMin, advIntervalMax, advType,
                                        ownAddrType,
                                        gapAddAddrAdj( pGapAdvertState->params.initiatorAddrType,
                                                       pGapAdvertState->params.initiatorAddr ),
                                        pGapAdvertState->params.initiatorAddr,
                                        pGapAdvertState->params.channelMap,
                                        pGapAdvertState->params.filterPolicy ) );
    }
    else
    {
        return ( FAILURE );
    }
}

/*********************************************************************
    @fn          gapSetAdvParamsStatus

    @brief       Process HCI Command Complete Event status for
                the call to HCI_BLESetAdvParamCmd().

    @param       status - HCI Set Advertisement Parameters status

    @return      TRUE if expected, FALSE if not
*/
uint8 gapSetAdvParamsStatus( uint8 status )
{
    if ( pGapAdvertState )
    {
        if ( status == SUCCESS )
        {
            // Move on to the next stage - Enable advertising
            pGapAdvertState->state = GAP_ADSTATE_SET_MODE;
            status = HCI_LE_SetAdvEnableCmd( HCI_ENABLE_ADV );
        }

        if ( status != SUCCESS )
        {
            // End the Make Discoverable process
            gapSendMakeDiscEvent( status, 0 );
        }

        return ( TRUE );
    }
    else
    {
        return ( FALSE );
    }
}

/*********************************************************************
    @fn          gapWriteAdvEnableStatus

    @brief       Process HCI Command Complete Event status for
                the call to HCI_BLEWriteAdvEnableCmd().

    @param       status - HCI Write Advertisement Enable status
    @param       interval - Advertisement interval (0x0020 = 0x4000)
                        interval * 0.625 msec

    @return      TRUE if expected, FALSE if not
*/
uint8 gapWriteAdvEnableStatus( uint8 status, uint16 interval )
{
    if ( pGapAdvertState )
    {
        if ( pGapAdvertState->state == GAP_ADSTATE_SET_MODE )
        {
            uint32 timeout;

            // Setup the advertising timeout
            if ( isLimitedDiscoverableMode() )
            {
                // Use the Limited Discoverable Timeout [Tgap(lim_adv_timeout)]
                timeout = GAP_GetParamValue( TGAP_LIM_ADV_TIMEOUT );

                // Maximum timeout value is 65 seconds
                if ( timeout > 65 )
                {
                    // Start a reload timer with 1 minute resoultion
                    VOID osal_start_reload_timer( gapTaskID, GAP_END_ADVERTISING_EVT,
                                                  (uint32)GAP_LIMITED_ADVERTISING_RESOLUTION );
                    // Calculate the remaining timeout
                    gapLimitedAdvertisingTimeout = timeout - 60;
                    timeout = 0;
                }
                else
                {
                    // Convert it to mSec
                    timeout *= 1000;
                }
            }
            else
            {
                // assume the General Discoverable advertising timeout [Tgap(gen_disc_adv_min)]
                timeout = GAP_GetParamValue( TGAP_GEN_DISC_ADV_MIN );
            }

            // Start a timer
            if ( timeout > 0 )
            {
                VOID osal_start_timerEx( gapTaskID, GAP_END_ADVERTISING_EVT, timeout );
            }

            // Update the state
            pGapAdvertState->state = GAP_ADSTATE_ADVERTISING;

            // Send the Make Discoverable Event message to the app
            if ( gapAutoAdvPrivateAddrChange == FALSE )
            {
                gapSendMakeDiscEvent( status, interval );
            }
            else
            {
                gapAutoAdvPrivateAddrChange = FALSE;
            }
        }
        else
        {
            if ( gapAutoAdvPrivateAddrChange == FALSE )
            {
                // Send the End Discoverable Event message to the app
                gapSendEndDiscoverableEvent( status );
                // Free the Advertising state
                gapFreeAdvertState();
            }
        }

        return ( TRUE );
    }
    else
    {
        return ( FALSE );
    }
}

/*********************************************************************
    @fn          gapWriteAdvDataStatus

    @brief       Process HCI Command Complete Event status for
                the call to HCI_BLEWriteAdvDataCmd() or
                HCI_BLEWriteScanRspDataCmd().

    @param       adType - TRUE for Advertising Data Status
                         FALSE for SCAN_RSP Data status
    @param       status - HCI Write Advertisement Data
                          of Scan Response Data status

    @return      none
*/
void gapWriteAdvDataStatus( uint8 adType, uint8 status )
{
    // Send the Make Discoverable Event message to the app
    gapSendAdDataUpdateEvent( adType, status );
}

/*********************************************************************
    @fn          gapProcessAdvertisingEvt

    @brief       Process Advertising event.

    @param       timeout - TRUE if advertising timeout, FALSE otherwise

    @return      none
*/
static void gapProcessAdvertisingEvt( uint8 timeout )
{
    if ( timeout == TRUE )
    {
        // Time to stop (limited or general) advertising?
        if ( ( gapLimitedAdvertisingTimeout == 0 ) || gapAutoAdvPrivateAddrChange )
        {
            gapProcessAdvertisingTimeout();
        }
        else
        {
            // Calculate the Limited Advertising remaining timeout
            if ( gapLimitedAdvertisingTimeout >= 60 )
            {
                // Decrement the seconds in the timeout value
                gapLimitedAdvertisingTimeout -= 60;
            }
            else
            {
                // Cancel the reloaded timer since its resolution is 1 minute
                VOID osal_stop_timerEx( gapTaskID, GAP_END_ADVERTISING_EVT );
                // Start a new timer for the remaining timeout (in mSec)
                VOID osal_start_timerEx( gapTaskID, GAP_END_ADVERTISING_EVT,
                                         (uint32)(gapLimitedAdvertisingTimeout*1000) );
                gapLimitedAdvertisingTimeout = 0;
            }
        }
    }
    else
    {
        gapConnectedCleanUpAdvertising();
    }
}

/*********************************************************************
    @fn          gapProcessAdvertisingTimeout

    @brief       Advertising time expired, stop advertising.

    @param       none

    @return      none
*/
static void gapProcessAdvertisingTimeout( void )
{
    // Are we in adverising state?
    if ( pGapAdvertState )
    {
        // Advertising time expired, stop advertising
        VOID GAP_EndDiscoverable( pGapAdvertState->taskID );
    }
}

/*********************************************************************
    @fn          gapConnectedCleanUpAdvertising

    @brief       This will clean up the advertising state.

    @param       none

    @return      none
*/
static void gapConnectedCleanUpAdvertising( void )
{
    // Make sure we're in Peripheral role (we could be in Central & Broadcaster role)
    if ( pGapAdvertState && ( gapProfileRole & GAP_PROFILE_PERIPHERAL ) )
    {
        // Stop the timer
        VOID osal_stop_timerEx( gapTaskID, GAP_END_ADVERTISING_EVT );
        gapLimitedAdvertisingTimeout = 0;
        // Free the Advertising state
        gapFreeAdvertState();
    }
}

/*********************************************************************
    @fn          gapAddAdvToken

    @brief       Add token to the end of the list.

    @param       pToken - pointer to data token structure

    @return      SUCCESS - advertisement token added to the GAP list <BR>
                bleMemAllocError - Memory allocation error <BR>
*/
bStatus_t gapAddAdvToken( gapAdvDataToken_t* pToken )
{
    gapAdvToken_t* pItem;
    pItem = (gapAdvToken_t*)osal_mem_alloc( (uint16)sizeof ( gapAdvToken_t ) );

    if ( pItem )
    {
        VOID osal_memset( pItem, 0, (int)sizeof ( gapAdvToken_t ) );

        if ( pGapAdvTokens )
        {
            gapAdvToken_t* pList;
            // Start at the top of the list
            pList = pGapAdvTokens;

            while ( pList->pNext )
            {
                pList = pList->pNext;
            }

            // Make this one the last in the list
            pList->pNext = pItem;
        }
        else
        {
            // Make this one the first in the list
            pGapAdvTokens = pItem;
        }

        pItem->pToken = pToken;
        return ( SUCCESS );
    }
    else
    {
        return ( bleMemAllocError );
    }
}

/*********************************************************************
    @fn          gapDeleteAdvToken

    @brief       Remove a token from the list

    @param       ADType - Advertisement data type

    @return      Pointer to removed advertisement data token,
                NULL if not found
*/
gapAdvDataToken_t* gapDeleteAdvToken( uint8 ADType )
{
    gapAdvToken_t* pPrevItem = NULL;
    gapAdvToken_t* pList;
    // Start at the top of the list
    pList = pGapAdvTokens;

    while ( pList )
    {
        if ( pList->pToken->adType == ADType )
        {
            gapAdvDataToken_t* pToken = pList->pToken;

            // Remove from the list
            if ( pPrevItem )
            {
                pPrevItem->pNext = pList->pNext;
            }
            else
            {
                pGapAdvTokens = pList->pNext;
            }

            // Deallocate the item
            osal_mem_free( pList );
            // Return the token memory for someone else to release
            return ( pToken );
        }

        pPrevItem = pList;
        pList = pList->pNext;
    }

    return ( (gapAdvDataToken_t*)NULL );
}

/*********************************************************************
    @fn          gapFindAdvToken

    @brief       Find a Advertisement data token from the advertisement type.

    @param       ADType - Advertisement data type

    @return      Pointer to the advertisement token,
                NULL if not found
*/
gapAdvToken_t* gapFindAdvToken( uint8 ADType )
{
    gapAdvToken_t* pList;
    // Start at the top of the list
    pList = pGapAdvTokens;

    while ( pList )
    {
        if ( pList->pToken->adType == ADType )
        {
            // found
            return ( pList );
        }

        pList = pList->pNext;
    }

    return ( (gapAdvToken_t*)NULL );
}

/*********************************************************************
    @fn          gapCalcAdvTokenDataLen

    @brief       Find a Advertisement data token from the advertisement type.

    @param       pAdLen - pointer to advertisement data length used
    @param       pSrLen - pointer to scan response data length used

    @return      none
*/
void gapCalcAdvTokenDataLen( uint8* pAdLen, uint8* pSrLen )
{
    gapAdvToken_t* pList;
    uint8 scan_rsp = FALSE;
    *pAdLen = 0;
    *pSrLen = 0;
    // Start at the top of the list
    pList = pGapAdvTokens;

    while ( pList )
    {
        if ( scan_rsp == FALSE )
        {
            if ( (*pAdLen + (pList->pToken->attrLen + ADV_TOKEN_HDR)) > B_MAX_ADV_LEN )
            {
                // Switch to scan response
                scan_rsp = TRUE;
            }
        }

        if ( scan_rsp )
        {
            *pSrLen += (pList->pToken->attrLen + ADV_TOKEN_HDR);
        }
        else
        {
            *pAdLen += (pList->pToken->attrLen + ADV_TOKEN_HDR);
        }

        // Next in list
        pList = pList->pNext;
    }
}

/*********************************************************************
    @fn          gapBuildADTokens

    @brief       Is a Advertisement Data Type valid.

    @param       none

    @return      SUCCESS, bleIncorrectMode, or bleMemAllocError
*/
bStatus_t gapBuildADTokens( void )
{
    gapAdvToken_t* pList;
    uint8 scan_rsp = FALSE;

    if ( (gapProfileRole & (GAP_PROFILE_PERIPHERAL | GAP_PROFILE_BROADCASTER)) == 0 )
    {
        return ( bleIncorrectMode );
    }

    if ( pGapAdData == NULL )
    {
        pGapAdData = (gapAdvertisingData_t*)osal_mem_alloc( (uint16)(sizeof ( gapAdvertisingData_t )) );
    }

    if ( pGapScanRspData == NULL )
    {
        pGapScanRspData = (gapAdvertisingData_t*)osal_mem_alloc( (uint16)(sizeof ( gapAdvertisingData_t )) );
    }

    if ( pGapAdData == NULL || pGapScanRspData == NULL )
    {
        return ( bleMemAllocError );
    }

    VOID osal_memset( pGapAdData, 0, (int)(sizeof ( gapAdvertisingData_t )) );
    VOID osal_memset( pGapScanRspData, 0, (int)(sizeof ( gapAdvertisingData_t )) );
    // Start at the top of the list
    pList = pGapAdvTokens;

    while ( pList )
    {
        uint8* pDataBuf;

        if ( scan_rsp == FALSE )
        {
            if ( (pGapAdData->dataLen + (pList->pToken->attrLen + ADV_TOKEN_HDR)) > B_MAX_ADV_LEN )
            {
                // Switch to scan response
                scan_rsp = TRUE;
            }
        }

        if ( scan_rsp )
        {
            pDataBuf = &(pGapScanRspData->dataField[pGapScanRspData->dataLen]);
            pGapScanRspData->dataLen += (pList->pToken->attrLen + ADV_TOKEN_HDR);
        }
        else
        {
            pDataBuf = &(pGapAdData->dataField[pGapAdData->dataLen]);
            pGapAdData->dataLen += (pList->pToken->attrLen + ADV_TOKEN_HDR);
        }

        // Make the token in the data field.
        pDataBuf[0] = pList->pToken->attrLen + 1;   // Token length (data + adType)
        pDataBuf[1] = pList->pToken->adType;        // Advertisement Type
        VOID osal_memcpy( &pDataBuf[2], pList->pToken->pAttrData, pList->pToken->attrLen );
        // Next in list
        pList = pList->pNext;
    }

    VOID HCI_LE_SetAdvDataCmd( pGapAdData->dataLen, pGapAdData->dataField );
    VOID HCI_LE_SetScanRspDataCmd( pGapScanRspData->dataLen, pGapScanRspData->dataField );
    return ( SUCCESS );
}

/*********************************************************************
    LOCAL FUNCTIONS
*/

/*********************************************************************
    @fn          gapSendMakeDiscEvent

    @brief       Send the GAP_MAKE_DISCOVERY_EVENT.

    @param       status - Reason for sending this message.
                         SUCCESS if all went well.

    @return      none
*/
static void gapSendMakeDiscEvent( bStatus_t status, uint16 interval )
{
    // Check advertising state
    if ( pGapAdvertState )
    {
        gapMakeDiscoverableRspEvent_t* pRsp;
        pRsp = (gapMakeDiscoverableRspEvent_t*)osal_msg_allocate( (uint16)(sizeof( gapMakeDiscoverableRspEvent_t )) );

        if ( pRsp )
        {
            // Build the message
            pRsp->hdr.event = GAP_MSG_EVENT;
            pRsp->hdr.status = status;
            pRsp->opcode = GAP_MAKE_DISCOVERABLE_DONE_EVENT;
            pRsp->interval = interval;
            VOID osal_msg_send( pGapAdvertState->taskID, (uint8*)pRsp );
        }

        if ( status != SUCCESS )
        {
            // Release Advertising data
            gapFreeAdvertState();
        }
    }
}

/*********************************************************************
    @fn          gapSendAdDataUpdateEvent

    @brief       Send the GAP_ADV_DATA_UPDATE_DONE_EVENT.

    @param       adType - TRUE for Advertising Data Status
                         FALSE for SCAN_RSP Data status
    @param       status - HCI Write Advertisement Data
                          of Scan Response Data status

    @return      none
*/
static void gapSendAdDataUpdateEvent( uint8 adType, uint8 status )
{
    if ( gapAdvAppTaskID != INVALID_TASK_ID )
    {
        gapAdvDataUpdateEvent_t* pRsp;
        pRsp = (gapAdvDataUpdateEvent_t*)osal_msg_allocate( (uint16)(sizeof ( gapAdvDataUpdateEvent_t )) );

        if ( pRsp )
        {
            pRsp->hdr.event = GAP_MSG_EVENT;
            pRsp->hdr.status = status;
            pRsp->opcode = GAP_ADV_DATA_UPDATE_DONE_EVENT;
            pRsp->adType = adType;
            VOID osal_msg_send( gapAdvAppTaskID, (uint8*)pRsp );
        }
    }
}

/*********************************************************************
    @fn          gapSendEndDiscoverableEvent

    @brief       Send the GAP_END_DISCOVERABLE_DONE_EVENT.

    @param       status - HCI Advertisement Enable status

    @return      none
*/
static void gapSendEndDiscoverableEvent( uint8 status )
{
    if ( pGapAdvertState )
    {
        gapEndDiscoverableRspEvent_t* pRsp;
        pRsp = (gapEndDiscoverableRspEvent_t*)osal_msg_allocate( (uint16)(sizeof ( gapEndDiscoverableRspEvent_t )) );

        if ( pRsp )
        {
            pRsp->hdr.event = GAP_MSG_EVENT;
            pRsp->hdr.status = status;
            pRsp->opcode = GAP_END_DISCOVERABLE_DONE_EVENT;
            VOID osal_msg_send( pGapAdvertState->taskID, (uint8*)pRsp );
        }
    }
}

/*********************************************************************
    @fn          gapFreeAdvertState

    @brief       Free the memory associated with the Advertising state.

    @param       none

    @return      none
*/
static void gapFreeAdvertState( void )
{
    if ( pGapAdvertState )
    {
        osal_mem_free( pGapAdvertState );
        pGapAdvertState = NULL;
    }
}

/*********************************************************************
    @fn          isLimitedDiscoverableMode

    @brief       Looks through the advertising data to determine if
                the device is in Limited Discovery Mode.

    @param       none

    @return      TRUE if in Limited Discovery Mode, FALSE if not.
*/
static uint8 isLimitedDiscoverableMode( void )
{
    if ( pGapAdData )
    {
        uint8 ADLen;
        uint8* pADToken = gapFindADType( GAP_ADTYPE_FLAGS, &ADLen,
                                         pGapAdData->dataLen,  pGapAdData->dataField );

        if ( (pADToken) && (*pADToken & GAP_ADTYPE_FLAGS_LIMITED) )
        {
            return ( TRUE );
        }
    }

    return ( FALSE );
}


/****************************************************************************
****************************************************************************/
