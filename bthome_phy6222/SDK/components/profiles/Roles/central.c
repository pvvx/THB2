/*************
 SDK_LICENSE
***************/
/*********************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "OSAL.h"
#include "osal_cbtimer.h"
#include "osal_snv.h"
#include "hci_tl.h"
#include "l2cap.h"
#include "linkdb.h"
#include "gap.h"
#include "gapbondmgr.h"
#include "central.h"

/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/

// Profile Events
#define START_ADVERTISING_EVT         0x0001
#define RSSI_READ_EVT                 0x0002
#define UPDATE_PARAMS_TIMEOUT_EVT     0x0004

// Profile OSAL Message IDs
#define GAPCENTRALROLE_RSSI_MSG_EVT   0xE0

/*********************************************************************
    TYPEDEFS
*/

// RSSI read data structure
typedef struct
{
    uint16        period;
    uint16        connHandle;
    uint8         timerId;
} gapCentralRoleRssi_t;

// OSAL event structure for RSSI timer events
typedef struct
{
    osal_event_hdr_t      hdr;
    gapCentralRoleRssi_t*  pRssi;
} gapCentralRoleRssiEvent_t;

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

// Task ID
static uint8 gapCentralRoleTaskId;

// App callbacks
static gapCentralRoleCB_t* pGapCentralRoleCB;

// Array of RSSI read structures
static gapCentralRoleRssi_t gapCentralRoleRssi[GAPCENTRALROLE_NUM_RSSI_LINKS];

/*********************************************************************
    Profile Parameters - reference GAPCENTRALROLE_PROFILE_PARAMETERS for
    descriptions
*/

static uint8  gapCentralRoleIRK[KEYLEN];
static uint8  gapCentralRoleSRK[KEYLEN];
static uint32 gapCentralRoleSignCounter;
static uint8  gapCentralRoleBdAddr[B_ADDR_LEN];
static uint8  gapCentralRoleMaxScanRes = 0;

/*********************************************************************
    LOCAL FUNCTIONS
*/
static void gapCentralRole_ProcessOSALMsg( osal_event_hdr_t* pMsg );
static void gapCentralRole_ProcessGAPMsg( gapEventHdr_t* pMsg );
static gapCentralRoleRssi_t* gapCentralRole_RssiAlloc( uint16 connHandle );
static gapCentralRoleRssi_t* gapCentralRole_RssiFind( uint16 connHandle );
static void gapCentralRole_RssiFree( uint16 connHandle );
static void gapCentralRole_timerCB( uint8* pData );

/*********************************************************************
    PUBLIC FUNCTIONS
*/

/**
    @brief   Start the device in Central role.  This function is typically
            called once during system startup.

    Public function defined in central.h.
*/
bStatus_t GAPCentralRole_StartDevice( gapCentralRoleCB_t* pAppCallbacks )
{
    if ( pAppCallbacks )
    {
        pGapCentralRoleCB = pAppCallbacks;
    }

    return GAP_DeviceInit( gapCentralRoleTaskId, GAP_PROFILE_CENTRAL,
                           gapCentralRoleMaxScanRes, gapCentralRoleIRK,
                           gapCentralRoleSRK, &gapCentralRoleSignCounter );
}

/**
    @brief   Set a parameter in the Central Profile.

    Public function defined in central.h.
*/
bStatus_t GAPCentralRole_SetParameter( uint16 param, uint8 len, void* pValue )
{
    bStatus_t ret = SUCCESS;

    switch ( param )
    {
    case GAPCENTRALROLE_IRK:
        if ( len == KEYLEN )
        {
            VOID osal_memcpy( gapCentralRoleIRK, pValue, KEYLEN ) ;
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPCENTRALROLE_SRK:
        if ( len == KEYLEN )
        {
            VOID osal_memcpy( gapCentralRoleSRK, pValue, KEYLEN ) ;
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPCENTRALROLE_SIGNCOUNTER:
        if ( len == sizeof ( uint32 ) )
        {
            gapCentralRoleSignCounter = *((uint32*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPCENTRALROLE_MAX_SCAN_RES:
        if ( len == sizeof ( uint8 ) )
        {
            gapCentralRoleMaxScanRes = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    default:
        ret = INVALIDPARAMETER;
        break;
    }

    return ret;
}

/**
    @brief   Get a parameter in the Central Profile.

    Public function defined in central.h.
*/
bStatus_t GAPCentralRole_GetParameter( uint16 param, void* pValue )
{
    bStatus_t ret = SUCCESS;

    switch ( param )
    {
    case GAPCENTRALROLE_IRK:
        VOID osal_memcpy( pValue, gapCentralRoleIRK, KEYLEN ) ;
        break;

    case GAPCENTRALROLE_SRK:
        VOID osal_memcpy( pValue, gapCentralRoleSRK, KEYLEN ) ;
        break;

    case GAPCENTRALROLE_SIGNCOUNTER:
        *((uint32*)pValue) = gapCentralRoleSignCounter;
        break;

    case GAPCENTRALROLE_BD_ADDR:
        VOID osal_memcpy( pValue, gapCentralRoleBdAddr, B_ADDR_LEN ) ;
        break;

    case GAPCENTRALROLE_MAX_SCAN_RES:
        *((uint8*)pValue) = gapCentralRoleMaxScanRes;
        break;

    default:
        ret = INVALIDPARAMETER;
        break;
    }

    return ret;
}

/**
    @brief   Terminate a link.

    Public function defined in central.h.
*/
bStatus_t GAPCentralRole_TerminateLink( uint16 connHandle )
{
    return GAP_TerminateLinkReq( gapCentralRoleTaskId, connHandle, HCI_DISCONNECT_REMOTE_USER_TERM ) ;
}

/**
    @brief   Establish a link to a peer device.

    Public function defined in central.h.
*/
bStatus_t GAPCentralRole_EstablishLink( uint8 highDutyCycle, uint8 whiteList,
                                        uint8 addrTypePeer, uint8* peerAddr )
{
    gapEstLinkReq_t params;
    params.taskID = gapCentralRoleTaskId;
    params.highDutyCycle = highDutyCycle;
    params.whiteList = whiteList;
    params.addrTypePeer = addrTypePeer;
    VOID osal_memcpy( params.peerAddr, peerAddr, B_ADDR_LEN );
    return GAP_EstablishLinkReq( &params );
}

/**
    @brief   Update the link connection parameters.

    Public function defined in central.h.
*/
bStatus_t GAPCentralRole_UpdateLink( uint16 connHandle, uint16 connIntervalMin,
                                     uint16 connIntervalMax, uint16 connLatency,
                                     uint16 connTimeout )
{
    return (bStatus_t) HCI_LE_ConnUpdateCmd( connHandle, connIntervalMin,
                                             connIntervalMax, connLatency,
                                             connTimeout, 0, 0 );
}

/**
    @brief   Start a device discovery scan.

    Public function defined in central.h.
*/
bStatus_t GAPCentralRole_StartDiscovery( uint8 mode, uint8 activeScan, uint8 whiteList )
{
    gapDevDiscReq_t params;
    params.taskID = gapCentralRoleTaskId;
    params.mode = mode;
    params.activeScan = activeScan;
    params.whiteList = whiteList;
    return GAP_DeviceDiscoveryRequest( &params );
}

/**
    @brief   Cancel a device discovery scan.

    Public function defined in central.h.
*/
bStatus_t GAPCentralRole_CancelDiscovery( void )
{
    return GAP_DeviceDiscoveryCancel( gapCentralRoleTaskId );
}

/**
    @brief   Start periodic RSSI reads on a link.

    Public function defined in central.h.
*/
bStatus_t GAPCentralRole_StartRssi( uint16 connHandle, uint16 period )
{
    gapCentralRoleRssi_t*  pRssi;

    // Verify link is up
    if (!linkDB_Up(connHandle))
    {
        return bleIncorrectMode;
    }

    // If already allocated
    if ((pRssi = gapCentralRole_RssiFind( connHandle )) != NULL)
    {
        // Stop timer
        osal_CbTimerStop( pRssi->timerId );
    }
    // Allocate structure
    else if ((pRssi = gapCentralRole_RssiAlloc( connHandle )) != NULL)
    {
        pRssi->period = period;
    }
    // Allocate failed
    else
    {
        return bleNoResources;
    }

    // Start timer
    osal_CbTimerStart( gapCentralRole_timerCB, (uint8*) pRssi,
                       period, &pRssi->timerId );
    return SUCCESS;
}

/**
    @brief   Cancel periodic RSSI reads on a link.

    Public function defined in central.h.
*/
bStatus_t GAPCentralRole_CancelRssi(uint16 connHandle )
{
    gapCentralRoleRssi_t*  pRssi;

    if ((pRssi = gapCentralRole_RssiFind( connHandle )) != NULL)
    {
        // Stop timer
        osal_CbTimerStop( pRssi->timerId );
        // Free RSSI structure
        gapCentralRole_RssiFree( connHandle );
        return SUCCESS;
    }

    // Not found
    return bleIncorrectMode;
}

/**
    @brief   Central Profile Task initialization function.

    @param   taskId - Task ID.

    @return  void
*/
void GAPCentralRole_Init( uint8 taskId )
{
    uint8 i;
    gapCentralRoleTaskId = taskId;

    // Initialize internal data
    for ( i = 0; i < GAPCENTRALROLE_NUM_RSSI_LINKS; i++ )
    {
        gapCentralRoleRssi[i].connHandle = GAP_CONNHANDLE_ALL;
        gapCentralRoleRssi[i].timerId = INVALID_TIMER_ID;
    }

    // Initialize parameters
    // Retore items from NV
//  VOID osal_snv_read( BLE_NVID_IRK, KEYLEN, gapCentralRoleIRK );
//  VOID osal_snv_read( BLE_NVID_CSRK, KEYLEN, gapCentralRoleSRK );
//  VOID osal_snv_read( BLE_NVID_SIGNCOUNTER, sizeof( uint32 ), &gapCentralRoleSignCounter );
    // Register for HCI messages (for RSSI)
    GAP_RegisterForHCIMsgs( taskId );
}

/**
    @brief   Central Profile Task event processing function.

    @param   taskId - Task ID
    @param   events - Events.

    @return  events not processed
*/
uint16 GAPCentralRole_ProcessEvent( uint8 taskId, uint16 events )
{
    if ( events & SYS_EVENT_MSG )
    {
        uint8* pMsg;

        if ( (pMsg = osal_msg_receive( gapCentralRoleTaskId )) != NULL )
        {
            gapCentralRole_ProcessOSALMsg( (osal_event_hdr_t*) pMsg );
            // Release the OSAL message
            VOID osal_msg_deallocate( pMsg );
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if ( events & GAP_EVENT_SIGN_COUNTER_CHANGED )
    {
        // Sign counter changed, save it to NV
        //  VOID osal_snv_write( BLE_NVID_SIGNCOUNTER, sizeof( uint32 ), &gapCentralRoleSignCounter );
        return ( events ^ GAP_EVENT_SIGN_COUNTER_CHANGED );
    }

    // Discard unknown events
    return 0;
}

/*********************************************************************
    @fn      gapCentralRole_ProcessOSALMsg

    @brief   Process an incoming task message.

    @param   pMsg - message to process

    @return  none
*/
static void gapCentralRole_ProcessOSALMsg( osal_event_hdr_t* pMsg )
{
    switch ( pMsg->event )
    {
    case HCI_GAP_EVENT_EVENT:
        if ( pMsg->status == HCI_COMMAND_COMPLETE_EVENT_CODE )
        {
            hciEvt_CmdComplete_t* pPkt = (hciEvt_CmdComplete_t*) pMsg;

            if ( pPkt->cmdOpcode == HCI_READ_RSSI )
            {
                uint16 connHandle = BUILD_UINT16( pPkt->pReturnParam[1], pPkt->pReturnParam[2] );
                int8 rssi = (int8) pPkt->pReturnParam[3];

                // Report RSSI to app
                if ( pGapCentralRoleCB && pGapCentralRoleCB->rssiCB )
                {
                    pGapCentralRoleCB->rssiCB( connHandle, rssi );
                }
            }
        }

        break;

    case GAP_MSG_EVENT:
        gapCentralRole_ProcessGAPMsg( (gapEventHdr_t*) pMsg );
        break;

    case GAPCENTRALROLE_RSSI_MSG_EVT:
    {
        gapCentralRoleRssi_t* pRssi = ((gapCentralRoleRssiEvent_t*) pMsg)->pRssi;

        // If link is up and RSSI reads active
        if (pRssi->connHandle != GAP_CONNHANDLE_ALL &&
                linkDB_Up(pRssi->connHandle))
        {
            // Restart timer
            osal_CbTimerStart( gapCentralRole_timerCB, (uint8*) pRssi,
                               pRssi->period, &pRssi->timerId );
            // Read RSSI
            VOID HCI_ReadRssiCmd( pRssi->connHandle );
        }
    }
    break;

    default:
        break;
    }
}

/*********************************************************************
    @fn      gapCentralRole_ProcessGAPMsg

    @brief   Process an incoming task message from GAP.

    @param   pMsg - message to process

    @return  none
*/
static void gapCentralRole_ProcessGAPMsg( gapEventHdr_t* pMsg )
{
    switch ( pMsg->opcode )
    {
    case GAP_DEVICE_INIT_DONE_EVENT:
    {
        gapDeviceInitDoneEvent_t* pPkt = (gapDeviceInitDoneEvent_t*) pMsg;

        if ( pPkt->hdr.status == SUCCESS )
        {
            // Save off the generated keys
            // VOID osal_snv_write( BLE_NVID_IRK, KEYLEN, gapCentralRoleIRK );
            // VOID osal_snv_write( BLE_NVID_CSRK, KEYLEN, gapCentralRoleSRK );
            // Save off the information
            VOID osal_memcpy( gapCentralRoleBdAddr, pPkt->devAddr, B_ADDR_LEN );
        }
    }
    break;

    case GAP_LINK_ESTABLISHED_EVENT:
    {
        gapEstLinkReqEvent_t* pPkt = (gapEstLinkReqEvent_t*) pMsg;

        if (pPkt->hdr.status == SUCCESS)
        {
            // Notify the Bond Manager of the connection
            VOID GAPBondMgr_LinkEst( pPkt->devAddrType, pPkt->devAddr,
                                     pPkt->connectionHandle, GAP_PROFILE_CENTRAL );
        }
    }
    break;

    case GAP_LINK_TERMINATED_EVENT:
    {
        uint16 connHandle = ((gapTerminateLinkEvent_t*) pMsg)->connectionHandle;
        VOID GAPBondMgr_ProcessGAPMsg( (gapEventHdr_t*)pMsg );
        // Cancel RSSI reads
        GAPCentralRole_CancelRssi( connHandle );
    }
    break;

    // temporary workaround
    case GAP_SLAVE_REQUESTED_SECURITY_EVENT:
        VOID GAPBondMgr_ProcessGAPMsg( pMsg );
        break;

    default:
        break;
    }

    // Pass event to app
    if ( pGapCentralRoleCB && pGapCentralRoleCB->eventCB )
    {
        pGapCentralRoleCB->eventCB( (gapCentralRoleEvent_t*) pMsg );
    }
}

/*********************************************************************
    @fn      gapCentralRole_RssiAlloc

    @brief   Allocate an RSSI structure.

    @param   connHandle - Connection handle

    @return  pointer to structure or NULL if allocation failed.
*/
static gapCentralRoleRssi_t* gapCentralRole_RssiAlloc( uint16 connHandle )
{
    uint8 i;

    // Find free RSSI structure
    for ( i = 0; i < GAPCENTRALROLE_NUM_RSSI_LINKS; i++ )
    {
        if ( gapCentralRoleRssi[i].connHandle == GAP_CONNHANDLE_ALL )
        {
            gapCentralRoleRssi[i].connHandle = connHandle;
            return &gapCentralRoleRssi[i];
        }
    }

    // No free structure found
    return NULL;
}

/*********************************************************************
    @fn      gapCentralRole_RssiFind

    @brief   Find an RSSI structure.

    @param   connHandle - Connection handle

    @return  pointer to structure or NULL if not found.
*/
static gapCentralRoleRssi_t* gapCentralRole_RssiFind( uint16 connHandle )
{
    uint8 i;

    // Find free RSSI structure
    for ( i = 0; i < GAPCENTRALROLE_NUM_RSSI_LINKS; i++ )
    {
        if ( gapCentralRoleRssi[i].connHandle == connHandle )
        {
            return &gapCentralRoleRssi[i];
        }
    }

    // Not found
    return NULL;
}

/*********************************************************************
    @fn      gapCentralRole_RssiFree

    @brief   Free an RSSI structure.

    @param   connHandle - Connection handle

    @return  none
*/
static void gapCentralRole_RssiFree( uint16 connHandle )
{
    uint8 i;

    // Find RSSI structure
    for ( i = 0; i < GAPCENTRALROLE_NUM_RSSI_LINKS; i++ )
    {
        if ( gapCentralRoleRssi[i].connHandle == connHandle )
        {
            gapCentralRoleRssi[i].connHandle = GAP_CONNHANDLE_ALL;
            break;
        }
    }
}

/*********************************************************************
    @fn      gapCentralRole_timerCB

    @brief   OSAL timer callback function

    @param   pData - Data pointer

    @return  none
*/
static void gapCentralRole_timerCB( uint8* pData )
{
    gapCentralRoleRssiEvent_t* pMsg;
    // Timer has expired so clear timer ID
    ((gapCentralRoleRssi_t*) pData)->timerId = INVALID_TIMER_ID;
    // Send OSAL message
    pMsg = (gapCentralRoleRssiEvent_t*) osal_msg_allocate( sizeof(gapCentralRoleRssiEvent_t) );

    if ( pMsg )
    {
        pMsg->hdr.event = GAPCENTRALROLE_RSSI_MSG_EVT;
        pMsg->pRssi = (gapCentralRoleRssi_t*) pData;
        osal_msg_send ( gapCentralRoleTaskId, (uint8*) pMsg );
    }
}

/*********************************************************************
*********************************************************************/
