/*************
 SDK_LICENSE
***************/
/*********************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "OSAL.h"
#include "hci.h"
#include "l2cap.h"
#include "gap.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "osal_snv.h"
#include "hci_tl.h"

#include "peripheralBroadcaster.h"
#include "gapbondmgr.h"

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

#define DEFAULT_ADVERT_OFF_TIME       30000   // 30 seconds

#define RSSI_NOT_AVAILABLE            127

#define DEFAULT_MIN_CONN_INTERVAL     0x0006  // 100 milliseconds
#define DEFAULT_MAX_CONN_INTERVAL     0x0C80  // 4 seconds

#define MIN_CONN_INTERVAL             0x0006
#define MAX_CONN_INTERVAL             0x0C80

#define DEFAULT_SLAVE_LATENCY         0
#define DEFAULT_TIMEOUT_MULTIPLIER    1000

#define CONN_INTERVAL_MULTIPLIER      6

#define MAX_SLAVE_LATENCY             500
#define MIN_TIMEOUT_MULTIPLIER        0x000a
#define MAX_TIMEOUT_MULTIPLIER        0x0c80

#define MAX_TIMEOUT_VALUE             0xFFFF

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
static uint8 gapRole_TaskID;   // Task ID for internal task/event processing

static gaprole_States_t gapRole_state;

/*********************************************************************
    Profile Parameters - reference GAPROLE_PROFILE_PARAMETERS for
    descriptions
*/

static uint8  gapRole_profileRole;
static uint8  gapRole_IRK[KEYLEN];
static uint8  gapRole_SRK[KEYLEN];
static uint32 gapRole_signCounter;
static uint8  gapRole_bdAddr[B_ADDR_LEN];
static uint8  gapRole_AdvEnabled = TRUE;
static uint16 gapRole_AdvertOffTime = DEFAULT_ADVERT_OFF_TIME;
static uint8  gapRole_AdvertDataLen = 3;
static uint8  gapRole_AdvertData[B_MAX_ADV_LEN] =
{
    0x02,   // length of this data
    GAP_ADTYPE_FLAGS,   // AD Type = Flags
    // Limited Discoverable & BR/EDR not supported
    (GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED),
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
static uint8  gapRole_ScanRspDataLen = 0;
static uint8  gapRole_ScanRspData[B_MAX_ADV_LEN] = {0};
static uint8  gapRole_AdvEventType;
static uint8  gapRole_AdvDirectType;
static uint8  gapRole_AdvDirectAddr[B_ADDR_LEN] = {0};
static uint8  gapRole_AdvChanMap;
static uint8  gapRole_AdvFilterPolicy;

static uint16 gapRole_ConnectionHandle = INVALID_CONNHANDLE;
static uint16 gapRole_RSSIReadRate = 0;

static gapRolesCBs_t* pGapRoles_AppCGs = NULL;
static uint8  gapRole_ConnectedDevAddr[B_ADDR_LEN] = {0};

static uint8  gapRole_ParamUpdateEnable = FALSE;
static uint16 gapRole_MinConnInterval = DEFAULT_MIN_CONN_INTERVAL;
static uint16 gapRole_MaxConnInterval = DEFAULT_MAX_CONN_INTERVAL;
static uint16 gapRole_SlaveLatency = DEFAULT_SLAVE_LATENCY;
static uint16 gapRole_TimeoutMultiplier = DEFAULT_TIMEOUT_MULTIPLIER;


/*********************************************************************
    Profile Attributes - variables
*/

/*********************************************************************
    Profile Attributes - Table
*/

/*********************************************************************
    LOCAL FUNCTIONS
*/
static void gapRole_ProcessOSALMsg( osal_event_hdr_t* pMsg );
static void gapRole_ProcessGAPMsg( gapEventHdr_t* pMsg );
static void gapRole_SetupGAP( void );
static void gapRole_SendUpdateParam( uint16 connInterval, uint16 connLatency );

/*********************************************************************
    NETWORK LAYER CALLBACKS
*/

/*********************************************************************
    PUBLIC FUNCTIONS
*/

/*********************************************************************
    @brief   Set a GAP Role parameter.

    Public function defined in peripheral.h.
*/
bStatus_t GAPRole_SetParameter( uint16 param, uint8 len, void* pValue )
{
    bStatus_t ret = SUCCESS;

    switch ( param )
    {
    case GAPROLE_IRK:
        if ( len == KEYLEN )
        {
            VOID osal_memcpy( gapRole_IRK, pValue, KEYLEN ) ;
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPROLE_SRK:
        if ( len == KEYLEN )
        {
            VOID osal_memcpy( gapRole_SRK, pValue, KEYLEN ) ;
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPROLE_SIGNCOUNTER:
        if ( len == sizeof ( uint32 ) )
        {
            gapRole_signCounter = *((uint32*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPROLE_ADVERT_ENABLED:
        if ( len == sizeof( uint8 ) )
        {
            if ( (gapRole_state == GAPROLE_CONNECTED) || (gapRole_state == GAPROLE_CONNECTED_ADV) )
            {
                uint8 advEnabled = *((uint8*)pValue);

                if ( (gapRole_state == GAPROLE_CONNECTED) && (advEnabled == TRUE) )
                {
                    // Turn on advertising
                    osal_set_event( gapRole_TaskID, START_ADVERTISING_EVT );
                }
                else if ( (gapRole_state == GAPROLE_CONNECTED_ADV) && (advEnabled == FALSE) )
                {
                    // Turn off Advertising
                    GAP_EndDiscoverable( gapRole_TaskID );
                }
            }
            else
            {
                uint8 oldAdvEnabled = gapRole_AdvEnabled;
                gapRole_AdvEnabled = *((uint8*)pValue);

                if ( (oldAdvEnabled) && (gapRole_AdvEnabled == FALSE) )
                {
                    // Turn off Advertising
                    VOID GAP_EndDiscoverable( gapRole_TaskID );
                }
                else if ( (oldAdvEnabled == FALSE) && (gapRole_AdvEnabled) )
                {
                    // Turn on Advertising
                    if ( (gapRole_state == GAPROLE_STARTED)
                            || (gapRole_state == GAPROLE_WAITING )
                            || (gapRole_state == GAPROLE_WAITING_AFTER_TIMEOUT) )
                    {
                        VOID osal_set_event( gapRole_TaskID, START_ADVERTISING_EVT );
                    }
                }
            }
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPROLE_ADVERT_OFF_TIME:
        if ( len == sizeof ( uint16 ) )
        {
            gapRole_AdvertOffTime = *((uint16*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPROLE_ADVERT_DATA:
        if ( len <= B_MAX_ADV_LEN )
        {
            VOID osal_memset( gapRole_AdvertData, 0, B_MAX_ADV_LEN );
            VOID osal_memcpy( gapRole_AdvertData, pValue, len );
            gapRole_AdvertDataLen = len;
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPROLE_SCAN_RSP_DATA:
        if ( len <= B_MAX_ADV_LEN )
        {
            VOID osal_memset( gapRole_ScanRspData, 0, B_MAX_ADV_LEN );
            VOID osal_memcpy( gapRole_ScanRspData, pValue, len );
            gapRole_ScanRspDataLen = len;
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPROLE_ADV_EVENT_TYPE:
        if ( (len == sizeof ( uint8 )) && (*((uint8*)pValue) <= GAP_ADTYPE_ADV_LDC_DIRECT_IND) )
        {
            gapRole_AdvEventType = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPROLE_ADV_DIRECT_TYPE:
        if ( (len == sizeof ( uint8 )) && (*((uint8*)pValue) <= ADDRTYPE_PRIVATE_RESOLVE) )
        {
            gapRole_AdvDirectType = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPROLE_ADV_DIRECT_ADDR:
        if ( len == B_ADDR_LEN )
        {
            VOID osal_memcpy( gapRole_AdvDirectAddr, pValue, B_ADDR_LEN ) ;
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPROLE_ADV_CHANNEL_MAP:
        if ( (len == sizeof ( uint8 )) && (*((uint8*)pValue) <= 0x07) )
        {
            gapRole_AdvChanMap = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPROLE_ADV_FILTER_POLICY:
        if ( (len == sizeof ( uint8 )) && (*((uint8*)pValue) <= GAP_FILTER_POLICY_WHITE) )
        {
            gapRole_AdvFilterPolicy = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPROLE_RSSI_READ_RATE:
        if ( len == sizeof ( uint16 ) )
        {
            gapRole_RSSIReadRate = *((uint16*)pValue);

            if ( (gapRole_RSSIReadRate) && (gapRole_state == GAPROLE_CONNECTED) )
            {
                // Start the RSSI Reads
                VOID osal_start_timerEx( gapRole_TaskID, RSSI_READ_EVT, gapRole_RSSIReadRate );
            }
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPROLE_PARAM_UPDATE_ENABLE:
        if ( (len == sizeof ( uint8 )) && (*((uint8*)pValue) <= TRUE) )
        {
            gapRole_ParamUpdateEnable = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPROLE_MIN_CONN_INTERVAL:
    {
        uint16 newInterval = *((uint16*)pValue);

        if (   len == sizeof ( uint16 )           &&
                ( newInterval >= MIN_CONN_INTERVAL ) &&
                ( newInterval <= MAX_CONN_INTERVAL ) )
        {
            gapRole_MinConnInterval = newInterval;
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case GAPROLE_MAX_CONN_INTERVAL:
    {
        uint16 newInterval = *((uint16*)pValue);

        if (   len == sizeof ( uint16 )          &&
                ( newInterval >= MIN_CONN_INTERVAL) &&
                ( newInterval <= MAX_CONN_INTERVAL) )
        {
            gapRole_MaxConnInterval = newInterval;
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case GAPROLE_SLAVE_LATENCY:
    {
        uint16 latency = *((uint16*)pValue);

        if ( len == sizeof ( uint16 ) && (latency < MAX_SLAVE_LATENCY) )
        {
            gapRole_SlaveLatency = latency;
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case GAPROLE_TIMEOUT_MULTIPLIER:
    {
        uint16 newTimeout = *((uint16*)pValue);

        if ( len == sizeof ( uint16 )
                && (newTimeout >= MIN_TIMEOUT_MULTIPLIER) && (newTimeout <= MAX_TIMEOUT_MULTIPLIER) )
        {
            gapRole_TimeoutMultiplier = newTimeout;
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    default:

        // The param value isn't part of this profile, try the GAP.
        if ( (param < TGAP_PARAMID_MAX) && (len == sizeof ( uint16 )) )
        {
            ret = GAP_SetParamValue( param, *((uint16*)pValue) );
        }
        else
        {
            ret = INVALIDPARAMETER;
        }

        break;
    }

    return ( ret );
}

/*********************************************************************
    @brief   Get a GAP Role parameter.

    Public function defined in peripheral.h.
*/
bStatus_t GAPRole_GetParameter( uint16 param, void* pValue )
{
    bStatus_t ret = SUCCESS;

    switch ( param )
    {
    case GAPROLE_PROFILEROLE:
        *((uint8*)pValue) = gapRole_profileRole;
        break;

    case GAPROLE_IRK:
        VOID osal_memcpy( pValue, gapRole_IRK, KEYLEN ) ;
        break;

    case GAPROLE_SRK:
        VOID osal_memcpy( pValue, gapRole_SRK, KEYLEN ) ;
        break;

    case GAPROLE_SIGNCOUNTER:
        *((uint32*)pValue) = gapRole_signCounter;
        break;

    case GAPROLE_BD_ADDR:
        VOID osal_memcpy( pValue, gapRole_bdAddr, B_ADDR_LEN ) ;
        break;

    case GAPROLE_ADVERT_ENABLED:
        *((uint8*)pValue) = gapRole_AdvEnabled;
        break;

    case GAPROLE_ADVERT_OFF_TIME:
        *((uint16*)pValue) = gapRole_AdvertOffTime;
        break;

    case GAPROLE_ADVERT_DATA:
        VOID osal_memcpy( pValue, gapRole_AdvertData, gapRole_AdvertDataLen );
        break;

    case GAPROLE_SCAN_RSP_DATA:
        VOID osal_memcpy( pValue, gapRole_ScanRspData, gapRole_ScanRspDataLen ) ;
        break;

    case GAPROLE_ADV_EVENT_TYPE:
        *((uint8*)pValue) = gapRole_AdvEventType;
        break;

    case GAPROLE_ADV_DIRECT_TYPE:
        *((uint8*)pValue) = gapRole_AdvDirectType;
        break;

    case GAPROLE_ADV_DIRECT_ADDR:
        VOID osal_memcpy( pValue, gapRole_AdvDirectAddr, B_ADDR_LEN ) ;
        break;

    case GAPROLE_ADV_CHANNEL_MAP:
        *((uint8*)pValue) = gapRole_AdvChanMap;
        break;

    case GAPROLE_ADV_FILTER_POLICY:
        *((uint8*)pValue) = gapRole_AdvFilterPolicy;
        break;

    case GAPROLE_CONNHANDLE:
        *((uint16*)pValue) = gapRole_ConnectionHandle;
        break;

    case GAPROLE_RSSI_READ_RATE:
        *((uint16*)pValue) = gapRole_RSSIReadRate;
        break;

    case GAPROLE_PARAM_UPDATE_ENABLE:
        *((uint16*)pValue) = gapRole_ParamUpdateEnable;
        break;

    case GAPROLE_MIN_CONN_INTERVAL:
        *((uint16*)pValue) = gapRole_MinConnInterval;
        break;

    case GAPROLE_MAX_CONN_INTERVAL:
        *((uint16*)pValue) = gapRole_MaxConnInterval;
        break;

    case GAPROLE_SLAVE_LATENCY:
        *((uint16*)pValue) = gapRole_SlaveLatency;
        break;

    case GAPROLE_TIMEOUT_MULTIPLIER:
        *((uint16*)pValue) = gapRole_TimeoutMultiplier;
        break;

    case GAPROLE_CONN_BD_ADDR:
        VOID osal_memcpy( pValue, gapRole_ConnectedDevAddr, B_ADDR_LEN ) ;
        break;

    default:

        // The param value isn't part of this profile, try the GAP.
        if ( param < TGAP_PARAMID_MAX )
        {
            *((uint16*)pValue) = GAP_GetParamValue( param );
        }
        else
        {
            ret = INVALIDPARAMETER;
        }

        break;
    }

    return ( ret );
}

/*********************************************************************
    @brief   Does the device initialization.

    Public function defined in peripheral.h.
*/
bStatus_t GAPRole_StartDevice( gapRolesCBs_t* pAppCallbacks )
{
    if ( gapRole_state == GAPROLE_INIT )
    {
        // Clear all of the Application callbacks
        if ( pAppCallbacks )
        {
            pGapRoles_AppCGs = pAppCallbacks;
        }

        // Start the GAP
        gapRole_SetupGAP();
        return ( SUCCESS );
    }
    else
    {
        return ( bleAlreadyInRequestedMode );
    }
}

/*********************************************************************
    @brief   Terminates the existing connection.

    Public function defined in peripheral.h.
*/
bStatus_t GAPRole_TerminateConnection( void )
{
    if ( (gapRole_state == GAPROLE_CONNECTED)
            || (gapRole_state == GAPROLE_CONNECTED_ADV) )
    {
        return ( GAP_TerminateLinkReq( gapRole_TaskID, gapRole_ConnectionHandle,
                                       HCI_DISCONNECT_REMOTE_USER_TERM ) );
    }
    else
    {
        return ( bleIncorrectMode );
    }
}

/*********************************************************************
    LOCAL FUNCTION PROTOTYPES
*/

/*********************************************************************
    @brief   Task Initialization function.

    Internal function defined in peripheral.h.
*/
void GAPRole_Init( uint8 task_id )
{
    gapRole_TaskID = task_id;
    gapRole_state = GAPROLE_INIT;
    gapRole_ConnectionHandle = INVALID_CONNHANDLE;
    GAP_RegisterForHCIMsgs( gapRole_TaskID );
    // Initialize the Profile Advertising and Connection Parameters
    gapRole_profileRole = (GAP_PROFILE_PERIPHERAL | GAP_PROFILE_BROADCASTER);
    VOID osal_memset( gapRole_IRK, 0, KEYLEN );
    VOID osal_memset( gapRole_SRK, 0, KEYLEN );
    gapRole_signCounter = 0;
    gapRole_AdvEventType = GAP_ADTYPE_ADV_IND;
    gapRole_AdvDirectType = ADDRTYPE_PUBLIC;
    gapRole_AdvChanMap = GAP_ADVCHAN_ALL;
    gapRole_AdvFilterPolicy = GAP_FILTER_POLICY_ALL;
    // Restore Items from NV
    VOID osal_snv_read( BLE_NVID_IRK, KEYLEN, gapRole_IRK );
    VOID osal_snv_read( BLE_NVID_CSRK, KEYLEN, gapRole_SRK );
    VOID osal_snv_read( BLE_NVID_SIGNCOUNTER, sizeof( uint32 ), &gapRole_signCounter );
}

/*********************************************************************
    @brief   Task Event Processor function.

    Internal function defined in peripheral.h.
*/
uint16 GAPRole_ProcessEvent( uint8 task_id, uint16 events )
{
    VOID task_id; // OSAL required parameter that isn't used in this function

    if ( events & SYS_EVENT_MSG )
    {
        uint8* pMsg;

        if ( (pMsg = osal_msg_receive( gapRole_TaskID )) != NULL )
        {
            gapRole_ProcessOSALMsg( (osal_event_hdr_t*)pMsg );
            // Release the OSAL message
            VOID osal_msg_deallocate( pMsg );
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if ( events & GAP_EVENT_SIGN_COUNTER_CHANGED )
    {
        // Sign counter changed, save it to NV
        VOID osal_snv_write( BLE_NVID_SIGNCOUNTER, sizeof( uint32 ), &gapRole_signCounter );
        return ( events ^ GAP_EVENT_SIGN_COUNTER_CHANGED );
    }

    if ( events & START_ADVERTISING_EVT )
    {
        if ( gapRole_AdvEnabled )
        {
            gapAdvertisingParams_t params;

            // Setup advertisement parameters
            if ( gapRole_state == GAPROLE_CONNECTED )
            {
                // While in a connection, we can only advertise non-connectable undirected.
                params.eventType = GAP_ADTYPE_ADV_NONCONN_IND;
            }
            else
            {
                params.eventType = gapRole_AdvEventType;
                params.initiatorAddrType = gapRole_AdvDirectType;
                VOID osal_memcpy( params.initiatorAddr, gapRole_AdvDirectAddr, B_ADDR_LEN );
            }

            params.channelMap = gapRole_AdvChanMap;
            params.filterPolicy = gapRole_AdvFilterPolicy;

            if ( GAP_MakeDiscoverable( gapRole_TaskID, &params ) != SUCCESS )
            {
                gapRole_state = GAPROLE_ERROR;

                if ( pGapRoles_AppCGs && pGapRoles_AppCGs->pfnStateChange )
                {
                    pGapRoles_AppCGs->pfnStateChange( gapRole_state );
                }
            }
        }

        return ( events ^ START_ADVERTISING_EVT );
    }

    if ( events & RSSI_READ_EVT )
    {
        // Only get RSSI when in a connection
        if ( (gapRole_state == GAPROLE_CONNECTED)
                || (gapRole_state == GAPROLE_CONNECTED_ADV) )
        {
            // Ask for RSSI
            VOID HCI_ReadRssiCmd( gapRole_ConnectionHandle );

            // Setup next event
            if ( gapRole_RSSIReadRate )
            {
                VOID osal_start_timerEx( gapRole_TaskID, RSSI_READ_EVT, gapRole_RSSIReadRate );
            }
        }

        return ( events ^ RSSI_READ_EVT );
    }

    if ( events & UPDATE_PARAMS_TIMEOUT_EVT )
    {
        // Clear an existing timeout
        if ( osal_get_timeoutEx( gapRole_TaskID, UPDATE_PARAMS_TIMEOUT_EVT ) )
        {
            VOID osal_stop_timerEx( gapRole_TaskID, UPDATE_PARAMS_TIMEOUT_EVT );
        }

        // The Update Parameters Timeout occurred - Terminate connection
        VOID GAP_TerminateLinkReq( gapRole_TaskID, gapRole_ConnectionHandle,
                                   HCI_DISCONNECT_REMOTE_USER_TERM );
        return ( events ^ UPDATE_PARAMS_TIMEOUT_EVT );
    }

    // Discard unknown events
    return 0;
}

/*********************************************************************
    @fn      gapRole_ProcessOSALMsg

    @brief   Process an incoming task message.

    @param   pMsg - message to process

    @return  none
*/
static void gapRole_ProcessOSALMsg( osal_event_hdr_t* pMsg )
{
    switch ( pMsg->event )
    {
    case HCI_GAP_EVENT_EVENT:
        if ( pMsg->status == HCI_COMMAND_COMPLETE_EVENT_CODE )
        {
            hciEvt_CmdComplete_t* pPkt = (hciEvt_CmdComplete_t*)pMsg;

            if ( pPkt->cmdOpcode == HCI_READ_RSSI )
            {
                int8 rssi = (int8)pPkt->pReturnParam[3];

                if ( ((gapRole_state == GAPROLE_CONNECTED)
                        || (gapRole_state == GAPROLE_CONNECTED_ADV))
                        && (rssi != RSSI_NOT_AVAILABLE) )
                {
                    // Report RSSI to app
                    if ( pGapRoles_AppCGs && pGapRoles_AppCGs->pfnRssiRead )
                    {
                        pGapRoles_AppCGs->pfnRssiRead( rssi );
                    }
                }
            }
        }

        break;

    case GAP_MSG_EVENT:
        gapRole_ProcessGAPMsg( (gapEventHdr_t*)pMsg );
        break;

    case L2CAP_SIGNAL_EVENT:
    {
        l2capSignalEvent_t* pPkt = (l2capSignalEvent_t*)pMsg;

        // Process the Parameter Update Response
        if ( pPkt->opcode == L2CAP_PARAM_UPDATE_RSP )
        {
            l2capParamUpdateRsp_t* pRsp = (l2capParamUpdateRsp_t*)&(pPkt->cmd.updateRsp);

            if ( pRsp->result == SUCCESS )
            {
                // All is good stop Update Parameters timeout
                VOID osal_stop_timerEx( gapRole_TaskID, UPDATE_PARAMS_TIMEOUT_EVT );
            }
        }
    }
    break;

    default:
        break;
    }
}

/*********************************************************************
    @fn      gapRole_ProcessGAPMsg

    @brief   Process an incoming task message.

    @param   pMsg - message to process

    @return  none
*/
static void gapRole_ProcessGAPMsg( gapEventHdr_t* pMsg )
{
    uint8 notify = FALSE;   // State changed notify the app? (default no)

    switch ( pMsg->opcode )
    {
    case GAP_DEVICE_INIT_DONE_EVENT:
    {
        gapDeviceInitDoneEvent_t* pPkt = (gapDeviceInitDoneEvent_t*)pMsg;
        bStatus_t stat = pPkt->hdr.status;

        if ( stat == SUCCESS )
        {
            // Save off the generated keys
            VOID osal_snv_write( BLE_NVID_IRK, KEYLEN, gapRole_IRK );
            VOID osal_snv_write( BLE_NVID_CSRK, KEYLEN, gapRole_SRK );
            // Save off the information
            VOID osal_memcpy( gapRole_bdAddr, pPkt->devAddr, B_ADDR_LEN );
            gapRole_state = GAPROLE_STARTED;
            // Update the advertising data
            stat = GAP_UpdateAdvertisingData( gapRole_TaskID,
                                              TRUE, gapRole_AdvertDataLen, gapRole_AdvertData );
        }

        if ( stat != SUCCESS )
        {
            gapRole_state = GAPROLE_ERROR;
        }

        notify = TRUE;
    }
    break;

    case GAP_ADV_DATA_UPDATE_DONE_EVENT:
    {
        gapAdvDataUpdateEvent_t* pPkt = (gapAdvDataUpdateEvent_t*)pMsg;

        if ( pPkt->hdr.status == SUCCESS )
        {
            if ( pPkt->adType )
            {
                // Setup the Response Data
                pPkt->hdr.status = GAP_UpdateAdvertisingData( gapRole_TaskID,
                                                              FALSE, gapRole_ScanRspDataLen, gapRole_ScanRspData );
            }
            else
            {
                // Start advertising
                VOID osal_set_event( gapRole_TaskID, START_ADVERTISING_EVT );
            }
        }

        if ( pPkt->hdr.status != SUCCESS )
        {
            // Set into Error state
            gapRole_state = GAPROLE_ERROR;
            notify = TRUE;
        }
    }
    break;

    case GAP_MAKE_DISCOVERABLE_DONE_EVENT:
    case GAP_END_DISCOVERABLE_DONE_EVENT:
    {
        gapMakeDiscoverableRspEvent_t* pPkt = (gapMakeDiscoverableRspEvent_t*)pMsg;

        if ( pPkt->hdr.status == SUCCESS )
        {
            if ( pMsg->opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT )
            {
                if ( gapRole_state == GAPROLE_CONNECTED )
                {
                    gapRole_state = GAPROLE_CONNECTED_ADV;
                }
                else
                {
                    gapRole_state = GAPROLE_ADVERTISING;
                }
            }
            else // GAP_END_DISCOVERABLE_DONE_EVENT
            {
                if ( gapRole_AdvertOffTime != 0 )
                {
                    if ( ( gapRole_AdvEnabled ) )
                    {
                        VOID osal_start_timerEx( gapRole_TaskID, START_ADVERTISING_EVT, gapRole_AdvertOffTime );
                    }
                }
                else
                {
                    // Since gapRole_AdvertOffTime is set to 0, the device should not
                    // automatically become discoverable again after a period of time.
                    // Set enabler to FALSE; device will become discoverable again when
                    // this value gets set to TRUE
                    gapRole_AdvEnabled = FALSE;
                }

                // In the Advertising Off period
                gapRole_state = GAPROLE_WAITING;
            }
        }
        else
        {
            gapRole_state = GAPROLE_ERROR;
        }

        notify = TRUE;
    }
    break;

    case GAP_LINK_ESTABLISHED_EVENT:
    {
        gapEstLinkReqEvent_t* pPkt = (gapEstLinkReqEvent_t*)pMsg;

        if ( pPkt->hdr.status == SUCCESS )
        {
            VOID osal_memcpy( gapRole_ConnectedDevAddr, pPkt->devAddr, B_ADDR_LEN );
            gapRole_ConnectionHandle = pPkt->connectionHandle;
            gapRole_state = GAPROLE_CONNECTED;

            if ( gapRole_RSSIReadRate )
            {
                // Start the RSSI Reads
                VOID osal_start_timerEx( gapRole_TaskID, RSSI_READ_EVT, gapRole_RSSIReadRate );
            }

            // Check whether update parameter request is enabled, and check the connection parameters
            if ( ( gapRole_ParamUpdateEnable == TRUE ) &&
                    ( (pPkt->connInterval < gapRole_MinConnInterval) ||
                      (pPkt->connInterval > gapRole_MaxConnInterval) ||
                      (pPkt->connLatency != gapRole_SlaveLatency)    ||
                      (pPkt->connTimeout != gapRole_TimeoutMultiplier) ))
            {
                gapRole_SendUpdateParam( pPkt->connInterval, pPkt->connLatency );
            }

            // Notify the Bond Manager to the connection
            VOID GAPBondMgr_LinkEst( pPkt->devAddrType, pPkt->devAddr, pPkt->connectionHandle, GAP_PROFILE_PERIPHERAL );
        }
        else
        {
            gapRole_state = GAPROLE_ERROR;
        }

        notify = TRUE;
    }
    break;

    case GAP_LINK_TERMINATED_EVENT:
    {
        VOID GAPBondMgr_ProcessGAPMsg( (gapEventHdr_t*)pMsg );
        osal_memset( gapRole_ConnectedDevAddr, 0, B_ADDR_LEN );

        if ( gapRole_state == GAPROLE_CONNECTED_ADV )
        {
            // End the non-connectable advertising
            GAP_EndDiscoverable( gapRole_TaskID );
            gapRole_state = GAPROLE_CONNECTED;
        }
        else
        {
            gapTerminateLinkEvent_t* pPkt = (gapTerminateLinkEvent_t*)pMsg;

            if( pPkt->reason == LL_SUPERVISION_TIMEOUT_TERM )
            {
                gapRole_state = GAPROLE_WAITING_AFTER_TIMEOUT;
            }
            else
            {
                gapRole_state = GAPROLE_WAITING;
            }

            notify = TRUE;
            VOID osal_set_event( gapRole_TaskID, START_ADVERTISING_EVT );
        }

        gapRole_ConnectionHandle = INVALID_CONNHANDLE;
    }
    break;

    case GAP_LINK_PARAM_UPDATE_EVENT:
    {
        gapLinkUpdateEvent_t* pPkt = (gapLinkUpdateEvent_t*)pMsg;

        if ( (pPkt->hdr.status != SUCCESS)
                || (pPkt->connInterval < gapRole_MinConnInterval)
                || (pPkt->connInterval > gapRole_MaxConnInterval) )
        {
            // Ask to change the interval
            gapRole_SendUpdateParam( pPkt->connInterval, pPkt->connLatency );
        }
        else
        {
            // All is good stop Update Parameters timeout
            VOID osal_stop_timerEx( gapRole_TaskID, UPDATE_PARAMS_TIMEOUT_EVT );
        }
    }
    break;

    default:
        break;
    }

    if ( notify == TRUE )
    {
        // Notify the application
        if ( pGapRoles_AppCGs && pGapRoles_AppCGs->pfnStateChange )
        {
            pGapRoles_AppCGs->pfnStateChange( gapRole_state );
        }
    }
}

/*********************************************************************
    @fn      gapRole_SetupGAP

    @brief   Call the GAP Device Initialization function using the
            Profile Parameters.

    @param   none

    @return  none
*/
static void gapRole_SetupGAP( void )
{
    VOID GAP_DeviceInit( gapRole_TaskID,
                         gapRole_profileRole, 0,
                         gapRole_IRK, gapRole_SRK,
                         &gapRole_signCounter );
}

/*********************************************************************
    @fn      gapRole_SendUpdateParam

    @brief   Send an Update Connection Parameters.

    @param   connInterval - current connection interval
    @param   connLatency - current connection latency

    @return  none
*/
static void gapRole_SendUpdateParam( uint16 connInterval, uint16 connLatency )
{
    l2capParamUpdateReq_t updateReq;  // Space for Conn Update parameters
    uint32 timeout;                   // Calculated response timeout
    // Calculate the current interval
    uint16 effectiveOldInterval = (connInterval * (connLatency + 1));
    // Calculate the interval we want
    uint16 effectiveNewMaxInterval = (gapRole_MaxConnInterval * (gapRole_SlaveLatency + 1));
    // Fill in the wanted parameters
    updateReq.intervalMin = gapRole_MinConnInterval;
    updateReq.intervalMax = gapRole_MaxConnInterval;
    updateReq.slaveLatency = gapRole_SlaveLatency;
    updateReq.timeoutMultiplier = gapRole_TimeoutMultiplier;
    VOID L2CAP_ConnParamUpdateReq( gapRole_ConnectionHandle, &updateReq, gapRole_TaskID );

    // Set up the timeout for expected response
    if( effectiveOldInterval > effectiveNewMaxInterval )
    {
        timeout = (uint32)(effectiveOldInterval) * CONN_INTERVAL_MULTIPLIER;
    }
    else
    {
        timeout = (uint32)(effectiveNewMaxInterval) * CONN_INTERVAL_MULTIPLIER;
    }

    if( timeout > MAX_TIMEOUT_VALUE )
    {
        timeout = MAX_TIMEOUT_VALUE;
    }

    VOID osal_start_timerEx( gapRole_TaskID, UPDATE_PARAMS_TIMEOUT_EVT, (uint16)(timeout) );
}

/*********************************************************************
*********************************************************************/
