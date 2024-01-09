/*************************************************************************************************
    Filename:       gap_centdevmgr.c
    Revised:
    Revision:

    Description:    This file contains the GAP Central Device Manager.

	SDK_LICENSE

**************************************************************************************************/


/*******************************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "hci_tl.h"
#include "l2cap.h"
#include "gap.h"
#include "gap_internal.h"
#include "global_config.h"

#include "log.h"
/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/

// GAP Advertisement Report types - same as HCI values except EMPTY
//  others are defined in gap.h (ie. GAP_ADVERTISEMENT_REPORT_TYPE_DEFINES)
#define GAP_ADRPT_EMPTY                   0xFF // Used only internally

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

// Scan records: advertisements and scan responses
static gapAdvertRec_t* pGapScanRecs = NULL;

// Number of scan responses to accept
static uint8 gapMaxScanResponses = 0;

/*********************************************************************
    LOCAL FUNCTIONS
*/

static uint8 gapCentProcessHCICmdEvt( uint16 cmdOpcode, hciEvt_CmdComplete_t* pMsg );
static void gapProcessScanningEvt( hciEvt_BLEAdvPktReport_t* pPkt );
static uint8 gapProcessConnEvt( uint16 cmdOpcode, hciEvt_CommandStatus_t* pMsg );

static void gapProcessScanDurationTimeout( void );
static gapAdvertRec_t* gapFindEmptyScanRec( void );
static gapAdvertRec_t* gapFindScanRec( uint8 addrType, uint8* pAddr );
static void gapFreeScanRecs( uint8 numScanRecs, uint8 freeRecs );
static bStatus_t gapAllocScanRecs( void );
static uint8 gapProcessAdvertDevInfo( hciEvt_DevInfo_t* pDevInfo );
static void gapSendDevDiscEvent( bStatus_t status );
static void gapSendDeviceInfoEvent( hciEvt_DevInfo_t* pDevInfo );
static bStatus_t gapSendScanEnable( uint8 enable );

static void gapProcessAdvertisementReport( hciEvt_BLEAdvPktReport_t* pPkt );

/*********************************************************************
    CENTRAL CALLBACKS
*/

// GAP Central Callbacks
static gapCentralCBs_t gapCentralCBs =
{
    gapCentProcessHCICmdEvt, // Process HCI Command Event Callback
    gapProcessScanningEvt,   // Process Scanning Event Callback
};

// GAP Central Connection-Related Callbacks
static gapCentralConnCBs_t gapCentralConnCBs =
{
    gapCancelLinkReq,  // Process Cancel Connection Initiation Callback
    gapProcessConnEvt, // Process Connection-Related Event Callback
};

/*********************************************************************
    PUBLIC FUNCTIONS
*/

/*********************************************************************
    Start a device discovery scan.

    Public function defined in gap.h.
*/
bStatus_t GAP_DeviceDiscoveryRequest( gapDevDiscReq_t* pParams )
{
    if ( pGapDiscReq )
    {
        // We only support one scan at a time
        return ( bleAlreadyInRequestedMode );
    }

    if ( (gapProfileRole & (GAP_PROFILE_CENTRAL | GAP_PROFILE_OBSERVER)) == 0 )
    {
        // This is only allowed for Central or Observer profile roles
        return ( bleIncorrectMode );
    }

    // Save off the parameters
    pGapDiscReq = (gapDevDiscReq_t*)osal_mem_alloc( (uint16)sizeof ( gapDevDiscReq_t ) );

    if ( pGapDiscReq )
    {
        uint16 scanInterval;  // Selected scan interval
        uint16 scanWindow;    // Selected scan window
        // Copy parameters
        VOID osal_memcpy( pGapDiscReq, pParams, (unsigned int)(sizeof (  gapDevDiscReq_t )) );

        // Select the LL parameters
        if ( GAP_NumActiveConnections() )
        {
            // We are already in a connection
            scanInterval = GAP_GetParamValue( TGAP_CONN_SCAN_INT );
            scanWindow = GAP_GetParamValue( TGAP_CONN_SCAN_WIND );
        }
        else if ( pParams->mode == DEVDISC_MODE_LIMITED )
        {
            scanInterval = GAP_GetParamValue( TGAP_LIM_DISC_SCAN_INT );
            scanWindow = GAP_GetParamValue( TGAP_LIM_DISC_SCAN_WIND );
        }
        else    // Assume General Discovery Mode
        {
            scanInterval = GAP_GetParamValue( TGAP_GEN_DISC_SCAN_INT );
            scanWindow = GAP_GetParamValue( TGAP_GEN_DISC_SCAN_WIND );
        }

        return ( HCI_LE_SetScanParamCmd( pParams->activeScan, scanInterval, scanWindow,
                                         ((gapDeviceAddrMode == ADDRTYPE_PUBLIC) ? ADDRTYPE_PUBLIC : ADDRTYPE_RANDOM),
                                         ((pParams->whiteList) ? TRUE : FALSE) ) );
    }
    else
    {
        return ( bleMemAllocError );
    }
}

/*********************************************************************
    Cancel an existing device discovery request.

    Public function defined in gap.h.
*/
bStatus_t GAP_DeviceDiscoveryCancel( uint8 taskID )
{
    // Make sure we are discovering.
    if ( pGapDiscReq == NULL )
    {
        return ( bleIncorrectMode );
    }

    // Is this the same task that started the scan?
    if ( pGapDiscReq->taskID != taskID )
    {
        return ( bleInvalidTaskID );
    }

    // Turn off the scan timer
    VOID osal_stop_timerEx( gapTaskID, GAP_OSAL_TIMER_SCAN_DURATION_EVT );
    // Send the user the Device Discovery Event message
    gapSendDevDiscEvent( bleGAPUserCanceled );
    // Cancel LL Scan
    return ( gapSendScanEnable( FALSE ) );
}

/*********************************************************************
    PUBLIC INTERNAL GAP FUNCTIONS
*/

/*********************************************************************
    @fn          GAP_CentDevMgrInit

    @brief       Initialize the GAP Central Dev Manager.

    @param       profileRole - GAP Profile Roles
    @param       maxScanResponses - maximum number to scan responses
                  can receive during a device discovery

    @return      SUCCESS or bleMemAllocError
*/
bStatus_t GAP_CentDevMgrInit( uint8 maxScanResponses )
{
    // Free the space first
    gapFreeScanRecs( gapMaxScanResponses, TRUE );

    if ( gapProfileRole & (GAP_PROFILE_CENTRAL | GAP_PROFILE_OBSERVER) )
    {
        // Set up Central's processing functions
        gapMaxScanResponses = maxScanResponses;
        gapRegisterCentral( &gapCentralCBs );
        return ( gapAllocScanRecs() );
    }

    gapMaxScanResponses = 0;
    gapRegisterCentral( NULL );
    return ( SUCCESS );
}

/*********************************************************************
    @fn          GAP_CentConnRegister

    @brief       Register the GAP Central Connection processing functions.

    @param       none

    @return      none
*/
void GAP_CentConnRegister( void )
{
    if ( gapProfileRole & GAP_PROFILE_CENTRAL )
    {
        // Set up Central's Connection processing functions
        gapRegisterCentralConn( &gapCentralConnCBs );
    }
    else
    {
        gapRegisterCentralConn( NULL );
    }
}

/*********************************************************************
    @fn      gapCentProcessHCICmdEvt

    @brief   Process an incoming GAP Central Event.

    @param   pMsg - message to process

    @return  TRUE if processed and safe to deallocate, FALSE if passed
            off to another task.
*/
static uint8 gapCentProcessHCICmdEvt( uint16 cmdOpcode, hciEvt_CmdComplete_t* pMsg )
{
    uint8 safeToDealloc = TRUE;

    switch ( cmdOpcode )
    {
    case HCI_LE_SET_SCAN_ENABLE:
        if ( *(pMsg->pReturnParam) == SUCCESS )
            break;
        safeToDealloc = gapSetScanParamStatus( *(pMsg->pReturnParam) );
        break;
    /*lint --fallthrough */
    case HCI_LE_SET_SCAN_PARAM:
        safeToDealloc = gapSetScanParamStatus( *(pMsg->pReturnParam) );
        break;
    default:
        safeToDealloc = FALSE;  // send this message to the app
        break;
    }

    return ( safeToDealloc );
}

/*********************************************************************
    @fn      gapProcessConnEvt

    @brief   Process an incoming GAP Central Connection Event.

    @param   pMsg - message to process

    @return  TRUE if processed and safe to deallocate, FALSE if passed
            off to another task.
*/
static uint8 gapProcessConnEvt( uint16 cmdOpcode, hciEvt_CommandStatus_t* pMsg )
{
    uint8 safeToDealloc = TRUE;

    switch ( cmdOpcode )
    {
    case HCI_LE_CREATE_CONNECTION_CANCEL:
        // this is a command complete event (hciEvt_CmdComplete_t)
        gapTerminateConnComplete();
        break;

    case HCI_LE_CREATE_CONNECTION:
        gapProcessCreateLLConnCmdStatus( pMsg->cmdStatus );
        break;

    case HCI_LE_CONNECTION_UPDATE:
        gapProcessConnUpdateCmdStatus( pMsg->cmdStatus );
        break;

    case L2CAP_PARAM_UPDATE:
        gapProcessL2CAPSignalEvt( (l2capSignalEvent_t*)pMsg );
        break;

    default:
        safeToDealloc = FALSE;  // send this message to the app
        break;
    }

    return ( safeToDealloc );
}

/*********************************************************************
    @fn          gapSetScanParamStatus

    @brief       Process HCI Command Complete Event status for
                the call to HCI_BLESetScanParamCmd().

    @param       status - HCI Set Scan Parameters status

    @return      TRUE if expected, FALSE if not.
*/
uint8 gapSetScanParamStatus( uint8 status )
{
    if ( pGapDiscReq )
    {
        if ( status == SUCCESS )
        {
            uint32 timeout;

            // Set the Scan duration
            if ( pGapDiscReq->mode == DEVDISC_MODE_LIMITED )
            {
                timeout = GAP_GetParamValue( TGAP_LIM_DISC_SCAN );
            }
            else
            {
                timeout = GAP_GetParamValue( TGAP_GEN_DISC_SCAN );
            }

            VOID osal_start_timerEx( gapTaskID,
                                     GAP_OSAL_TIMER_SCAN_DURATION_EVT, timeout );
            // Enable the LL Scan Mode
            status = gapSendScanEnable( TRUE );
        }

        if ( status != SUCCESS )
        {
            // Turn off the timer
            VOID osal_stop_timerEx( gapTaskID, GAP_OSAL_TIMER_SCAN_DURATION_EVT );
            // Send back Device Discovery Event
            gapSendDevDiscEvent( status );
        }

        return ( TRUE );
    }
    else
    {
        return ( FALSE );
    }
}

/*********************************************************************
    @fn          gapProcessScanningEvt

    @brief       Process Scanning event.

    @param       pPkt - Advertisement packet, NULL if Device Discovery timeout

    @return      none
*/
static void gapProcessScanningEvt( hciEvt_BLEAdvPktReport_t* pPkt )
{
    if ( pPkt == NULL )
    {
        gapProcessScanDurationTimeout();
    }
    else
    {
        gapProcessAdvertisementReport( pPkt );
    }
}

/*********************************************************************
    @fn          gapProcessScanDurationTimeout

    @brief       Scan Duration timer expired. Terminate the scan.

    @param       none

    @return      none
*/
static void gapProcessScanDurationTimeout( void )
{
    // Are we doing a scan?
    if ( pGapDiscReq )
    {
        // Send the user the Device Discovery Event message
        gapSendDevDiscEvent( SUCCESS );
        // Turn off the scan
        VOID gapSendScanEnable( FALSE );
    }
}

/*********************************************************************
    @fn          gapProcessAdvertisementReport

    @brief       Process HCI BLE Advertisement Report Event.

    @param       pkt - Advertisement packet

    @return      none
*/
static void gapProcessAdvertisementReport( hciEvt_BLEAdvPktReport_t* pPkt )
{
    // Parameter check
    if ( pPkt->numDevices && pPkt->devInfo )
    {
        hciEvt_DevInfo_t* pDevInfo = pPkt->devInfo;
        int8 minRssi = (int8)GAP_GetParamValue( TGAP_SCAN_RSP_RSSI_MIN );

        // Loop through the device advertisements
        for ( uint8 x = 0; x < pPkt->numDevices; x++ )
        {
            if ( ( pDevInfo->rssi >= minRssi ) && gapProcessAdvertDevInfo( pDevInfo ) )
            {
                // Send a report to the app
                gapSendDeviceInfoEvent( pDevInfo );
            }

            pDevInfo++;  // Move to next device
        }
    }
}

/*********************************************************************
    LOCAL FUNCTIONS
*/

/*********************************************************************
    @fn          gapFreeScanRecs

    @brief       Free the scan records.

    @param       numScanRecs - number of scan records
    @param       freeRecs - TRUE to deallocate,
                           FALSE to just clear the memory.

    @return      none
*/
static void gapFreeScanRecs( uint8 numScanRecs, uint8 freeRecs )
{
    if ( pGapScanRecs )
    {
        uint8 x;

        for ( x = 0; x < numScanRecs; x++ )
        {
            if ( pGapScanRecs[x].pAdData )
            {
                osal_mem_free( pGapScanRecs[x].pAdData );
                pGapScanRecs[x].pAdData = NULL;
            }

            if ( pGapScanRecs[x].pScanData )
            {
                osal_mem_free( pGapScanRecs[x].pScanData );
                pGapScanRecs[x].pScanData = NULL;
            }
        }

        if ( freeRecs )
        {
            osal_mem_free( pGapScanRecs );
            pGapScanRecs = NULL;
        }
        else
        {
            // Clear the memory
            uint16 blockSize = (uint16)(sizeof ( gapAdvertRec_t ) * numScanRecs);

            if ( pGapScanRecs )
            {
                VOID osal_memset( pGapScanRecs, 0, blockSize );

                for ( x = 0; x < numScanRecs; x++ )
                {
                    pGapScanRecs[x].eventType = GAP_ADRPT_EMPTY;
                }
            }
        }
    }
}

/*********************************************************************
    @fn          gapAllocScanRecs

    @brief       Allocate and initialize the scan records.

    @param       none

    @return      SUCCESS or bleMemAllocError
*/
static bStatus_t gapAllocScanRecs( void )
{
    if ( pGapScanRecs != NULL )
    {
        return ( bleMemAllocError );
    }

    // Check scan responses are needed
    if ( gapMaxScanResponses )
    {
        // Calculate the block size (bytes)
        uint16 blockSize = (uint16)(sizeof ( gapAdvertRec_t ) * gapMaxScanResponses);
        // Allocate one block for all the scan records
        pGapScanRecs = (gapAdvertRec_t*)osal_mem_alloc( blockSize );

        if ( pGapScanRecs )
        {
            uint8 x; // loop counter
            // Initialize the entire space
            VOID osal_memset( pGapScanRecs, 0, (int)blockSize );

            // Loop through all records and mark them empty
            for ( x = 0; x < gapMaxScanResponses; x++ )
            {
                pGapScanRecs[x].eventType = GAP_ADRPT_EMPTY;
            }
        }
        else
        {
            return ( bleMemAllocError );
        }
    }

    return ( SUCCESS );
}

/*********************************************************************
    @fn          gapFindEmptyScanRec

    @brief       Find an empty scan record slot.

    @param       none

    @return      empty record pointer or NULL if no empty slots
*/
static gapAdvertRec_t* gapFindEmptyScanRec( void )
{
    if ( pGapScanRecs )
    {
        for ( uint8 x = 0; x < gapMaxScanResponses; x++ )
        {
            // Look for empty slot
            if ( pGapScanRecs[x].eventType == GAP_ADRPT_EMPTY )
            {
                // Empty slot found
                return ( &(pGapScanRecs[x]) );
            }
        }
    }

    // No empty slots
    return ( (gapAdvertRec_t*)NULL );
}

/*********************************************************************
    @fn          gapFindScanRec

    @brief       Find an existing scan record.

    @param       addrType - ADDRTYPE_PUBLIC, ADDRTYPE_STATIC,
                          ADDRTYPE_PRIVATE_NONRESOLVE
                          or ADDRTYPE_PRIVATE_RESOLVE
    @param       pAddr - pointer to address

    @return      pointer to record or NULL if not found
*/
static gapAdvertRec_t* gapFindScanRec( uint8 addrType, uint8* pAddr )
{
    if ( pGapScanRecs )
    {
        // Look for a record that matches the address
        for ( uint8 x = 0; x < gapMaxScanResponses; x++ )
        {
            if ( pGapScanRecs[x].eventType != GAP_ADRPT_EMPTY )
            {
                if ( (pGapScanRecs[x].addrType == addrType)
                        && (osal_memcmp( pGapScanRecs[x].addr, pAddr, B_ADDR_LEN )) )
                {
                    // Record found
                    return ( &(pGapScanRecs[x]) );
                }
            }
        }
    }

    // Address not found
    return ( (gapAdvertRec_t*)NULL );
}

/*********************************************************************
    @fn          gapProcessAdvertDevInfo

    @brief       Process a Device Information structure of a
                HCI BLE Advertisement Report Event.

    @param       pDevInfo - Device Information Structure

    @return      TRUE if added to report or information changed.
*/
static uint8 gapProcessAdvertDevInfo( hciEvt_DevInfo_t* pDevInfo )
{
    uint8 stateFlags = 0;     // State flag (Flags AD type)
    uint8* ADToken = NULL;    // Holds a advertisement data token
    gapAdvertRec_t* pRec;      // advertisement record pointer

    // Are we in scan mode?
    if ( pGapDiscReq == NULL )
    {
        // Ignore the record
        return ( FALSE );
    }

    // Don't process the data field of a directed advertisement
    if ( pDevInfo->eventType != GAP_ADRPT_ADV_DIRECT_IND )
    {
        uint8 ADLen;   // Token length
        // What kind of advertisement is this?
        ADToken = gapFindADType( GAP_ADTYPE_FLAGS, &ADLen,
                                 pDevInfo->dataLen,  pDevInfo->rspData );
    }

    // Was the Flags token found
    if ( ADToken )
    {
        stateFlags = *ADToken;
    }

    // Filter type based on filter mode, only filter Advertisement reports
    if ( pDevInfo->eventType != GAP_ADRPT_SCAN_RSP )
    {
        uint8 passedFilter = FALSE;

        if ( pGapDiscReq->mode == DEVDISC_MODE_ALL )
        {
            // Send all through
            passedFilter = TRUE;
        }
        else if ( (pGapDiscReq->mode == DEVDISC_MODE_NONDISCOVERABLE)
                  && (stateFlags == 0) )
        {
            // no discovery bits
            passedFilter = TRUE;
        }
        else if ( ADToken )
        {
            if ( (pGapDiscReq->mode == DEVDISC_MODE_GENERAL)
                    && (stateFlags & (GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_LIMITED)) )
            {
                // Must have general discovery bit set
                passedFilter = TRUE;
            }
            else if ( (pGapDiscReq->mode == DEVDISC_MODE_LIMITED)
                      && (stateFlags & GAP_ADTYPE_FLAGS_LIMITED) )
            {
                // Must have limited discovery bit set
                passedFilter = TRUE;
            }
        }

        if ( passedFilter == FALSE )
        {
            // Ignore the record, didn't pass the filter
            return ( FALSE );
        }
    }

    // Convert from LL Address type to Host Address type
    pDevInfo->addrType = gapDetermineAddrType( pDevInfo->addrType, pDevInfo->addr );
    // Find an existing advertisement record
    pRec = gapFindScanRec( pDevInfo->addrType, pDevInfo->addr );

    if ( pRec )
    {
        gapAdvertRecData_t* pRecData;  // Data field pointer

        if ( pDevInfo->eventType != GAP_ADRPT_SCAN_RSP )
        {
            pRecData = pRec->pAdData;
        }
        else
        {
            pRecData = pRec->pScanData;
        }

        if ( pRecData
                && !(pGlobal_config[LL_SWITCH] & GAP_DUP_RPT_FILTER_DISALLOW))     // A2 add: duplicate adv report filter could be switch off due to Mindtree mesh request
        {
            if ( (pRecData->dataLen == pDevInfo->dataLen)
                    && (osal_memcmp( pRecData->dataField, pDevInfo->rspData, pDevInfo->dataLen ) == TRUE) )
            {
                // Same, ignore this report
                pRecData = (gapAdvertRecData_t*)NULL;
                pRec = (gapAdvertRec_t*)NULL;
            }
        }

        if ( pRecData )
        {
            if ( pDevInfo->eventType != GAP_ADRPT_SCAN_RSP )
            {
                // Clear the Advertisement data
                if ( (pRec) && (pRec->pAdData) )
                {
                    osal_mem_free( pRec->pAdData );
                    pRec->pAdData = NULL;
                }
            }
            else
            {
                // Clear the scan report data
                if ( (pRec) && (pRec->pScanData) )
                {
                    osal_mem_free( pRec->pScanData );
                    pRec->pScanData = NULL;
                }
            }
        }
    }
    // Only make a new record for Advertisment Records
    else if ( pDevInfo->eventType != GAP_ADRPT_SCAN_RSP )
    {
        // Find an empty spot
        pRec = gapFindEmptyScanRec();

        if ( pRec )
        {
            // Copy the new information
            pRec->eventType = pDevInfo->eventType;
            pRec->addrType  = pDevInfo->addrType;
            VOID osal_memcpy( pRec->addr, pDevInfo->addr, B_ADDR_LEN );
        }
    }

    // save the new advertisement data or scan response data
    if ( pRec )
    {
        gapAdvertRecData_t* pRecData = (gapAdvertRecData_t*)osal_mem_alloc( (uint16)(pDevInfo->dataLen + 1) );

        if ( pRecData )
        {
            if ( pDevInfo->eventType != GAP_ADRPT_SCAN_RSP )
            {
                pRec->pAdData = pRecData;
            }
            else
            {
                pRec->pScanData = pRecData;
            }

            pRecData->dataLen = pDevInfo->dataLen;
            VOID osal_memcpy( pRecData->dataField, pDevInfo->rspData, pDevInfo->dataLen );
        }

        if ( pDevInfo->dataLen )
        {
            return ( TRUE );
        }
    }

    return ( FALSE );
}

/*********************************************************************
    @fn          gapSendDevDiscEvent

    @brief       Send the GAP_DEVICE_DISCOVERY_EVENT.

    @param       status - Reason for sending this message.
                         SUCCESS if all went well.

    @return      none
*/
static void gapSendDevDiscEvent( bStatus_t status )
{
    gapDevDiscEvent_t* pMsg;  // Message pointer
    uint8 x;                  // Loop counter
    uint8 numDevs = 0;        // Number of devices found
    uint16 totalMemSize;      // Total buffer size needed

    // Mode check, make sure we are doing a scan
    if ( pGapDiscReq == NULL || pGapScanRecs == NULL )
    {
        return;
    }

    // calculate the size of the message needed
    if ( status == SUCCESS )
    {
        for ( x = 0; x < gapMaxScanResponses; x++ )
        {
            if ( pGapScanRecs[x].eventType != GAP_ADRPT_EMPTY )
            {
                numDevs++;
            }
        }
    }

    // Calculate total size of buffer needed
    totalMemSize = (uint16)(sizeof ( gapDevDiscEvent_t ));
    totalMemSize += (uint16)(sizeof ( gapDevRec_t ) * numDevs);
    // Allocate the message to send
    pMsg = (gapDevDiscEvent_t*)osal_msg_allocate( totalMemSize );

    if ( pMsg == NULL )
    {
        // There's not enough memory to allocate the whole buffer, so try to
        // allocate enough for a error status message
        pMsg = (gapDevDiscEvent_t*)osal_msg_allocate( (uint16)(sizeof ( gapDevDiscEvent_t )) );
        numDevs = 0;
        status = bleMemAllocError;
    }

    if ( pMsg )
    {
        // Build the message
        pMsg->hdr.event = GAP_MSG_EVENT;
        pMsg->hdr.status = status;
        pMsg->opcode = GAP_DEVICE_DISCOVERY_EVENT;
        pMsg->numDevs = numDevs;

        if ( numDevs > 0 )
        {
            gapDevRec_t* pDevList;
            // Setup pointer for device list
            pDevList = (gapDevRec_t*)(pMsg+1);
            pMsg->pDevList = pDevList;

            // Copy the scan records into the message
            for ( x = 0; (x < gapMaxScanResponses) && (numDevs > 0); x++ )
            {
                if ( pGapScanRecs[x].eventType != GAP_ADRPT_EMPTY )
                {
                    pDevList->eventType = pGapScanRecs[x].eventType;
                    pDevList->addrType  = pGapScanRecs[x].addrType;
                    VOID osal_memcpy( pDevList->addr, pGapScanRecs[x].addr, B_ADDR_LEN );
                    pDevList++;
                    numDevs--;
                }
            }
        }
        else
        {
            pMsg->pDevList = NULL;  // No records
        }

        // Send the message to the app
        VOID osal_msg_send( pGapDiscReq->taskID, (uint8*)pMsg );
    }

    // Clear the scan record.  Not freed - just cleared.
    gapFreeScanRecs( gapMaxScanResponses, FALSE );
    // Free all Device Discovery memory
    osal_mem_free( pGapDiscReq );
    pGapDiscReq = NULL;
}

/*********************************************************************
    @fn          gapSendScanEnable

    @brief       Send the HCI message to turn enable/disable the
                device discovery scan.

    @param       enable - TRUE to turn on scan, FALSE to turn off scan.

    @return      HCI return value
*/
static bStatus_t gapSendScanEnable( uint8 enable )
{
    uint8 filterDuplicates = GAP_GetParamValue( TGAP_FILTER_ADV_REPORTS ) ? TRUE : FALSE;
    // Continue with scan
    return ( HCI_LE_SetScanEnableCmd( enable, filterDuplicates ) );
}

/*********************************************************************
    @fn          gapSendDeviceInfoEvent

    @brief       Send the GAP_DEVICE_INFO_EVENT.

    @param       pDevInfo - Pointer to the HCI Advertisement or Scan Rsp.

    @return      none
*/
static void gapSendDeviceInfoEvent( hciEvt_DevInfo_t* pDevInfo )
{
    if ( pGapDiscReq )
    {
        gapDeviceInfoEvent_t* pRsp;
        // Allocate structure and extra space to put the data buffer at the end
        pRsp = (gapDeviceInfoEvent_t*)osal_msg_allocate(
                   ((uint16)(sizeof( gapDeviceInfoEvent_t )) + pDevInfo->dataLen) );

        if ( pRsp )
        {
            // Build the message
            pRsp->hdr.event = GAP_MSG_EVENT;
            pRsp->hdr.status = SUCCESS;
            pRsp->opcode = GAP_DEVICE_INFO_EVENT;
            pRsp->eventType = pDevInfo->eventType;
            pRsp->rssi = pDevInfo->rssi;
            pRsp->addrType = pDevInfo->addrType;
            VOID osal_memcpy( pRsp->addr, pDevInfo->addr, B_ADDR_LEN );
            pRsp->dataLen = pDevInfo->dataLen;

            if ( pRsp->dataLen )
            {
                // Point to the extra space at the end of the allocated memory
                pRsp->pEvtData = (uint8*)(pRsp + 1);
                VOID osal_memcpy( pRsp->pEvtData, pDevInfo->rspData, pRsp->dataLen );
            }
            else
            {
                pRsp->pEvtData = NULL;
            }

            VOID osal_msg_send( pGapDiscReq->taskID, (uint8*)pRsp );
        }
    }
}


/****************************************************************************
****************************************************************************/

