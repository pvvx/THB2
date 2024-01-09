/*************
 SDK_LICENSE
***************/
/*********************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "OSAL.h"
#include "hci_tl.h"

#include "gap.h"

#include "broadcaster.h"

/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/
// Profile Events
#define START_ADVERTISING_EVT         0x0001

#define DEFAULT_ADVERT_OFF_TIME       30000   // 30 seconds

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
static uint8  gapRole_bdAddr[B_ADDR_LEN];
static uint8  gapRole_AdvEnabled = TRUE;
static uint16 gapRole_AdvertOffTime = DEFAULT_ADVERT_OFF_TIME;
static uint8  gapRole_AdvertDataLen = 3;
static uint8  gapRole_AdvertData[B_MAX_ADV_LEN] =
{
    0x02,             // length of this data
    GAP_ADTYPE_FLAGS, // AD Type = Flags
    // BR/EDR not supported
    GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
static uint8  gapRole_ScanRspDataLen = 0;
static uint8  gapRole_ScanRspData[B_MAX_ADV_LEN] = {0};
static uint8  gapRole_AdvEventType;
static uint8  gapRole_AdvDirectType;
static uint8  gapRole_AdvDirectAddr[B_ADDR_LEN] = {0};
static uint8  gapRole_AdvChanMap;
static uint8  gapRole_AdvFilterPolicy;

// Application callbacks
static gapRolesCBs_t* pGapRoles_AppCGs = NULL;

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

/*********************************************************************
    NETWORK LAYER CALLBACKS
*/

/*********************************************************************
    PUBLIC FUNCTIONS
*/

/*********************************************************************
    @brief   Set a GAP Role parameter.

    Public function defined in broadcaster.h.
*/
bStatus_t GAPRole_SetParameter( uint16 param, uint8 len, void* pValue )
{
    bStatus_t ret = SUCCESS;

    switch ( param )
    {
    case GAPROLE_ADVERT_ENABLED:
        if ( len == sizeof( uint8 ) )
        {
            uint8 oldAdvEnabled = gapRole_AdvEnabled;
            gapRole_AdvEnabled = *((uint8*)pValue);

            if ( (oldAdvEnabled) && (gapRole_AdvEnabled == FALSE) )
            {
                // Turn off Advertising
                if ( gapRole_state == GAPROLE_ADVERTISING )
                {
                    VOID GAP_EndDiscoverable( gapRole_TaskID );
                }
            }
            else if ( (oldAdvEnabled == FALSE) && (gapRole_AdvEnabled) )
            {
                // Turn on Advertising
                if ( (gapRole_state == GAPROLE_STARTED)
                        || (gapRole_state == GAPROLE_WAITING) )
                {
                    VOID osal_set_event( gapRole_TaskID, START_ADVERTISING_EVT );
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

    Public function defined in broadcaster.h.
*/
bStatus_t GAPRole_GetParameter( uint16 param, void* pValue )
{
    bStatus_t ret = SUCCESS;

    switch ( param )
    {
    case GAPROLE_PROFILEROLE:
        *((uint8*)pValue) = gapRole_profileRole;
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

    Public function defined in broadcaster.h.
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
    LOCAL FUNCTION PROTOTYPES
*/

/*********************************************************************
    @brief   Task Initialization function.

    Internal function defined in broadcaster.h.
*/
void GAPRole_Init( uint8 task_id )
{
    gapRole_TaskID = task_id;
    gapRole_state = GAPROLE_INIT;
    GAP_RegisterForHCIMsgs( gapRole_TaskID );
    // Initialize the Profile Advertising and Connection Parameters
    gapRole_profileRole = GAP_PROFILE_BROADCASTER;
    gapRole_AdvEventType = GAP_ADTYPE_ADV_NONCONN_IND;
    gapRole_AdvDirectType = ADDRTYPE_PUBLIC;
    gapRole_AdvChanMap = GAP_ADVCHAN_ALL;
    gapRole_AdvFilterPolicy = GAP_FILTER_POLICY_ALL;
}

/*********************************************************************
    @brief   Task Event Processor function.

    Internal function defined in broadcaster.h.
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

    if ( events & START_ADVERTISING_EVT )
    {
        if ( gapRole_AdvEnabled )
        {
            gapAdvertisingParams_t params;
            // Setup advertisement parameters
            params.eventType = gapRole_AdvEventType;
            params.initiatorAddrType = gapRole_AdvDirectType;
            VOID osal_memcpy( params.initiatorAddr, gapRole_AdvDirectAddr, B_ADDR_LEN );
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
            //hciEvt_CmdComplete_t *pPkt = (hciEvt_CmdComplete_t *)pMsg;
        }

        break;

    case GAP_MSG_EVENT:
        gapRole_ProcessGAPMsg( (gapEventHdr_t*)pMsg );
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
            // Save off the information
            VOID osal_memcpy( gapRole_bdAddr, pPkt->devAddr, B_ADDR_LEN );
            gapRole_state = GAPROLE_STARTED;
            // Update the advertising data
            stat = GAP_UpdateAdvertisingData( gapRole_TaskID, TRUE,
                                              gapRole_AdvertDataLen,
                                              gapRole_AdvertData );
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
                gapRole_state = GAPROLE_ADVERTISING;
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
                         NULL, NULL, NULL );
}

/*********************************************************************
*********************************************************************/
