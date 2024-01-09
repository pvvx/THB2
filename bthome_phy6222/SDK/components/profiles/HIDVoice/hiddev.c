/*************
 hiddev.c
 SDK_LICENSE
***************/
/*********************************************************************
    INCLUDES
*/

#include "OSAL.h"
#include "gatt.h"
#include "hci.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "gatt_uuid.h"
#include "gatt_profile_uuid.h"
#include "linkdb.h"
#include "peripheral.h"
#include "gapbondmgr.h"
#include "devinfoservice.h"
#include "battservice.h"
#include "scanparamservice.h"


#include "hiddev.h"
#include "hidkbd.h"

#include "hidkbdservice.h"
//#include "touch_key.h"
#include "log.h"

/*********************************************************************
    MACROS
*/

// Battery measurement period in ms
#define DEFAULT_BATT_PERIOD                   15000

// TRUE to run scan parameters refresh notify test
#define DEFAULT_SCAN_PARAM_NOTIFY_TEST        TRUE

// Advertising intervals (units of 625us, 160=100ms)
#define HID_INITIAL_ADV_INT_MIN               48
#define HID_INITIAL_ADV_INT_MAX               80
#define HID_HIGH_ADV_INT_MIN                  32
#define HID_HIGH_ADV_INT_MAX                  48
#define HID_LOW_ADV_INT_MIN                   1600
#define HID_LOW_ADV_INT_MAX                   1600

// Advertising timeouts in sec
#define HID_INITIAL_ADV_TIMEOUT               60
#define HID_HIGH_ADV_TIMEOUT                  5
#define HID_LOW_ADV_TIMEOUT                   0

// Heart Rate Task Events
#define START_DEVICE_EVT                      0x0001
#define BATT_PERIODIC_EVT                     0x0002
#define HID_IDLE_EVT                          0x0004
#define HID_SEND_REPORT_EVT                   0x0008



#define reportQEmpty()                        ( firstQIdx == lastQIdx )

/*********************************************************************
    CONSTANTS
*/

#define HID_DEV_DATA_LEN                      8

#ifdef HID_DEV_RPT_QUEUE_LEN
    #define HID_DEV_REPORT_Q_SIZE               (HID_DEV_RPT_QUEUE_LEN+1)
#else
    #define HID_DEV_REPORT_Q_SIZE               (10+1)
#endif

/*********************************************************************
    TYPEDEFS
*/

typedef struct
{
    uint8 id;
    uint8 type;
    uint8 len;
    uint8 data[HID_DEV_DATA_LEN];
} hidDevReport_t;

/*********************************************************************
    GLOBAL VARIABLES
*/

// Task ID
uint8 hidDevTaskId;

/*********************************************************************
    EXTERNAL VARIABLES
*/

/*********************************************************************
    EXTERNAL FUNCTIONS
*/

// HID report mapping table
extern hidRptMap_t  hidRptMap[];


/*********************************************************************
    LOCAL VARIABLES
*/

// GAP State
gaprole_States_t hidDevGapState = GAPROLE_INIT;

// TRUE if connection is secure
static uint8 hidDevConnSecure = FALSE;

// GAP connection handle
uint16 gapConnHandle;

// TRUE if pairing in progress
static uint8 hidDevPairingStarted = FALSE;

// Status of last pairing
static uint8 pairingStatus = SUCCESS;

static hidRptMap_t* pHidDevRptTbl;

static uint8 hidDevRptTblLen;

static hidDevCB_t* pHidDevCB;

static hidDevCfg_t* pHidDevCfg;

// Whether to change to the preferred connection parameters
static uint8 updateConnParams = TRUE;

// Pending reports
static uint8 firstQIdx = 0;
static uint8 lastQIdx = 0;
static hidDevReport_t hidDevReportQ[HID_DEV_REPORT_Q_SIZE];

// Last report sent out
static attHandleValueNoti_t lastNoti = { 0 };

/*********************************************************************
    LOCAL FUNCTIONS
*/

static void hidDev_ProcessOSALMsg( osal_event_hdr_t* pMsg );
static void hidDevProcessGattMsg( gattMsgEvent_t* pMsg );
static void hidDevDisconnected( void );
static void hidDevGapStateCB( gaprole_States_t newState );
static void hidDevPairStateCB( uint16 connHandle, uint8 state, uint8 status );
static void hidDevPasscodeCB( uint8* deviceAddr, uint16 connectionHandle,
                              uint8 uiInputs, uint8 uiOutputs );
void hidDevBattCB( uint8 event );
void hidDevScanParamCB( uint8 event );
static void hidDevBattPeriodicTask( void );
static hidRptMap_t* hidDevRptByHandle( uint16 handle );
static hidRptMap_t* hidDevRptById( uint8 id, uint8 type );
static hidRptMap_t* hidDevRptByCccdHandle( uint16 handle );
static void hidDevEnqueueReport( uint8 id, uint8 type, uint8 len, uint8* pData );
static hidDevReport_t* hidDevDequeueReport( void );
static void hidDevSendReport( uint8 id, uint8 type, uint8 len, uint8* pData );
static void hidDevHighAdvertising( void );
static void hidDevLowAdvertising( void );
static void hidDevInitialAdvertising( void );
static uint8 hidDevBondCount( void );
static void hidDevStartIdleTimer( void );
static void hidDevStopIdleTimer( void );
static void HidDev_scanParamCB(uint8_t event);

/*********************************************************************
    PROFILE CALLBACKS
*/
// GAP Role Callbacks
static gapRolesCBs_t hidDev_PeripheralCBs =
{
    hidDevGapStateCB,   // Profile State Change Callbacks
    NULL                // When a valid RSSI is read from controller
};

// Bond Manager Callbacks
static const gapBondCBs_t hidDevBondCB =
{
    hidDevPasscodeCB,
    hidDevPairStateCB
};




/*********************************************************************
    PUBLIC FUNCTIONS
*/

/*********************************************************************
    @fn      HidDev_Init

    @brief   Initialization function for the Hid Dev Task.
            This is called during initialization and should contain
            any application specific initialization (ie. hardware
            initialization/setup, table initialization, power up
            notificaiton ... ).

    @param   task_id - the ID assigned by OSAL.  This ID should be
                      used to send messages and set timers.

    @return  none
*/
void HidDev_Init( uint8 task_id )
{
    hidDevTaskId = task_id;
    // Setup the GAP Bond Manager
    {
        uint8 syncWL = FALSE;//TRUE;
        // If a bond is created, the HID Device should write the address of the
        // HID Host in the HID Device controller's white list and set the HID
        // Device controller's advertising filter policy to 'process scan and
        // connection requests only from devices in the White List'.
        VOID GAPBondMgr_SetParameter( GAPBOND_AUTO_SYNC_WL, sizeof( uint8 ), &syncWL );
    }
    // Set up services
    GGS_AddService( GATT_ALL_SERVICES );         // GAP
    GATTServApp_AddService( GATT_ALL_SERVICES ); // GATT attributes
    DevInfo_AddService( );
    Batt_AddService();
    ScanParam_AddService();
    Batt_Register(NULL);
    // Register for Scan Parameters service callback.
    ScanParam_Register(HidDev_scanParamCB);
    //touch_init(on_key);
    // Setup a delayed profile startup
    osal_set_event( hidDevTaskId, START_DEVICE_EVT );
}

/*********************************************************************
    @fn      HidDev_ProcessEvent

    @brief   Hid Dev Task event processor.  This function
            is called to process all events for the task.  Events
            include timers, messages and any other user defined events.

    @param   task_id  - The OSAL assigned task ID.
    @param   events - events to process.  This is a bit map and can
                     contain more than one event.

    @return  events not processed
*/
uint16 HidDev_ProcessEvent( uint8 task_id, uint16 events )
{
    VOID task_id; // OSAL required parameter that isn't used in this function
    LOG("%s\n",__FUNCTION__);

    if ( events & SYS_EVENT_MSG )
    {
        uint8* pMsg;

        if ( (pMsg = osal_msg_receive( hidDevTaskId )) != NULL )
        {
            hidDev_ProcessOSALMsg( (osal_event_hdr_t*)pMsg );
            // Release the OSAL message
            VOID osal_msg_deallocate( pMsg );
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if ( events & START_DEVICE_EVT )
    {
        // Start the Device
        VOID GAPRole_StartDevice( &hidDev_PeripheralCBs );
        // Register with bond manager after starting device
        GAPBondMgr_Register( (gapBondCBs_t*) &hidDevBondCB );
        GAPBondMgr_SetParameter(GAPBOND_ERASE_ALLBONDS,0,NULL);
        LOG("start Device EVT\n\r");
        return ( events ^ START_DEVICE_EVT );
    }

    if ( events & HID_IDLE_EVT )
    {
        if ( hidDevGapState == GAPROLE_CONNECTED )
        {
            // if pairing in progress then restart timer
            if ( hidDevPairingStarted )
            {
                hidDevStartIdleTimer();
                LOG("hidDevStartIdleTimer \n\r");
            }
            // else disconnect
            else
            {
                GAPRole_TerminateConnection();
            }
        }

        return ( events ^ HID_IDLE_EVT );
    }

    if ( events & BATT_PERIODIC_EVT )
    {
        // Perform periodic battery task
        hidDevBattPeriodicTask();
        return ( events ^ BATT_PERIODIC_EVT );
    }

    if ( events & HID_SEND_REPORT_EVT )
    {
        // if connection is secure
        if ( hidDevConnSecure )
        {
            LOG("Send Hid Report\n\r");
            hidDevReport_t* pReport = hidDevDequeueReport();

            if ( pReport != NULL )
            {
                // Send report
                hidDevSendReport( pReport->id, pReport->type, pReport->len, pReport->data );
            }

            return ( reportQEmpty() ? events^ HID_SEND_REPORT_EVT : events );
        }

        return ( events ^ HID_SEND_REPORT_EVT );
    }

    return 0;
}

/*********************************************************************
    @fn      HidDev_Register

    @brief   Register a callback function with HID Dev.

    @param   pCfg - Parameter configuration.
    @param   pfnServiceCB - Callback function.

    @return  None.
*/
void HidDev_Register( hidDevCfg_t* pCfg, hidDevCB_t* pCBs )
{
    pHidDevCB = pCBs;
    pHidDevCfg = pCfg;
}

/*********************************************************************
    @fn      HidDev_RegisterReports

    @brief   Register the report table with HID Dev.

    @param   numReports - Length of report table.
    @param   pRpt - Report table.

    @return  None.
*/
void HidDev_RegisterReports( uint8 numReports, hidRptMap_t* pRpt )
{
    pHidDevRptTbl = pRpt;
    hidDevRptTblLen = numReports;
}

/*********************************************************************
    @fn      HidDev_Report

    @brief   Send a HID report.

    @param   id - HID report ID.
    @param   type - HID report type.
    @param   len - Length of report.
    @param   pData - Report data.

    @return  None.
*/
void HidDev_Report( uint8 id, uint8 type, uint8 len, uint8* pData )
{
    // if connected
    if ( hidDevGapState == GAPROLE_CONNECTED )
    {
        // if connection is secure
        if ( hidDevConnSecure )
        {
            // Make sure there're no pending reports
            if ( reportQEmpty() )
            {
                // send report
                hidDevSendReport( id, type, len, pData );
                LOG("send key action\n\r");
                return; // we're done
            }
        }
    }
    // else if not already advertising
    else if ( hidDevGapState != GAPROLE_ADVERTISING )
    {
        // if bonded
        if ( hidDevBondCount() > 0 )
        {
            // start high duty cycle advertising
            hidDevHighAdvertising();
        }
        // else not bonded
        else
        {
            // start initial advertising
            hidDevInitialAdvertising();
        }
    }

    // hidDev task will send report when secure connection is established
    hidDevEnqueueReport( id, type, len, pData );
}

/*********************************************************************
    @fn      HidDev_Close

    @brief   Close the connection or stop advertising.

    @return  None.
*/
void HidDev_Close( void )
{
    uint8 param;

    // if connected then disconnect
    if ( hidDevGapState == GAPROLE_CONNECTED )
    {
        GAPRole_TerminateConnection();
    }
    // else stop advertising
    else
    {
        param = FALSE;
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &param );
    }
}

/*********************************************************************
    @fn      HidDev_SetParameter

    @brief   Set a HID Dev parameter.

    @param   param - Profile parameter ID
    @param   len - length of data to right
    @param   pValue - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  bStatus_t
*/
bStatus_t HidDev_SetParameter( uint8 param, uint8 len, void* pValue )
{
    bStatus_t ret = SUCCESS;

    switch ( param )
    {
    case HIDDEV_ERASE_ALLBONDS:
        if ( len == 0 )
        {
            // See if the last report sent out wasn't a release key
            if ( osal_isbufset( lastNoti.value, 0x00, lastNoti.len ) == FALSE )
            {
                // Send a release report before disconnecting, otherwise
                // the last pressed key would get 'stuck' on the HID Host.
                osal_memset( lastNoti.value, 0x00, lastNoti.len );
                GATT_Notification( gapConnHandle, &lastNoti, FALSE );
            }

            // Drop connection
            if ( hidDevGapState == GAPROLE_CONNECTED )
            {
                GAPRole_TerminateConnection();
            }

            // Flush report queue
            firstQIdx = lastQIdx = 0;
            // Erase bonding info
            GAPBondMgr_SetParameter( GAPBOND_ERASE_ALLBONDS, 0, NULL );
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

    return ( ret );
}

/*********************************************************************
    @fn      HidDev_GetParameter

    @brief   Get a HID Dev parameter.

    @param   param - Profile parameter ID
    @param   pValue - pointer to data to get.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  bStatus_t
*/
bStatus_t HidDev_GetParameter( uint8 param, void* pValue )
{
    bStatus_t ret = SUCCESS;

    switch ( param )
    {
    default:
        ret = INVALIDPARAMETER;
        break;
    }

    return ( ret );
}

/*********************************************************************
    @fn      HidDev_PasscodeRsp

    @brief   Respond to a passcode request.

    @param   status - SUCCESS if passcode is available, otherwise
                     see @ref SMP_PAIRING_FAILED_DEFINES.
    @param   passcode - integer value containing the passcode.

    @return  none
*/
void HidDev_PasscodeRsp( uint8 status, uint32 passcode )
{
    // Send passcode response
    GAPBondMgr_PasscodeRsp( gapConnHandle, status, passcode );
}

/*********************************************************************
    @fn          HidDev_ReadAttrCB

    @brief       HID Dev attribute read callback.

    @param       connHandle - connection message was received on
    @param       pAttr - pointer to attribute
    @param       pValue - pointer to data to be read
    @param       pLen - length of data to be read
    @param       offset - offset of the first octet to be read
    @param       maxLen - maximum length of data to be read
    @param       method - type of read message

    @return      SUCCESS, blePending or Failure
*/
uint8 HidDev_ReadAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                         uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen )
{
    bStatus_t   status = SUCCESS;
    hidRptMap_t* pRpt;
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

    // Only report map is long
    if ( offset > 0 && uuid != REPORT_MAP_UUID )
    {
        return ( ATT_ERR_ATTR_NOT_LONG );
    }

    if ( uuid == REPORT_UUID ||
            uuid == BOOT_KEY_INPUT_UUID ||
            uuid == BOOT_KEY_OUTPUT_UUID ||
            uuid == BOOT_MOUSE_INPUT_UUID )
    {
        // find report ID in table
        if ( (pRpt = hidDevRptByHandle(pAttr->handle)) != NULL )
        {
            // execute report callback
            status  = (*pHidDevCB->reportCB)( pRpt->id, pRpt->type, uuid,
                                              HID_DEV_OPER_READ, pLen, pValue );
        }
        else
        {
            *pLen = 0;
        }
    }
    else if ( uuid == REPORT_MAP_UUID )
    {
        // verify offset
        if ( offset >= hidReportMapLen )
        {
            status = ATT_ERR_INVALID_OFFSET;
        }
        else
        {
            // determine read length
            *pLen = MIN( maxLen, (hidReportMapLen - offset) );
            // copy data
            osal_memcpy( pValue, pAttr->pValue + offset, *pLen );
        }
    }
    else if ( uuid == HID_INFORMATION_UUID )
    {
        *pLen = HID_INFORMATION_LEN;
        osal_memcpy( pValue, pAttr->pValue, HID_INFORMATION_LEN );
    }
    else if ( uuid == GATT_REPORT_REF_UUID )
    {
        *pLen = HID_REPORT_REF_LEN;
        osal_memcpy( pValue, pAttr->pValue, HID_REPORT_REF_LEN );
    }
    else if ( uuid == PROTOCOL_MODE_UUID )
    {
        *pLen = HID_PROTOCOL_MODE_LEN;
        pValue[0] = pAttr->pValue[0];
    }
    else if ( uuid == GATT_EXT_REPORT_REF_UUID )
    {
        *pLen = HID_EXT_REPORT_REF_LEN;
        osal_memcpy( pValue, pAttr->pValue, HID_EXT_REPORT_REF_LEN );
    }

    // restart idle timer
    if ( status == SUCCESS )
    {
        hidDevStartIdleTimer();
    }

    return ( status );
}

/*********************************************************************
    @fn      HidDev_WriteAttrCB

    @brief   HID Dev attribute read callback.

    @param   connHandle - connection message was received on
    @param   pAttr - pointer to attribute
    @param   pValue - pointer to data to be written
    @param   len - length of data
    @param   offset - offset of the first octet to be written

    @return  Success or Failure
*/
bStatus_t HidDev_WriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                              uint8* pValue, uint16 len, uint16 offset )
{
    bStatus_t status = SUCCESS;
    hidRptMap_t* pRpt;

    // Make sure it's not a blob operation (no attributes in the profile are long)
    if ( offset > 0 )
    {
        return ( ATT_ERR_ATTR_NOT_LONG );
    }

    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

    if ( uuid == REPORT_UUID ||
            uuid == BOOT_KEY_OUTPUT_UUID )
    {
        // find report ID in table
        if ((pRpt = hidDevRptByHandle(pAttr->handle)) != NULL)
        {
            // execute report callback
            status  = (*pHidDevCB->reportCB)( pRpt->id, pRpt->type, uuid,
                                              HID_DEV_OPER_WRITE, &len, pValue );
        }
    }
    else if ( uuid == HID_CTRL_PT_UUID )
    {
        // Validate length and value range
        if ( len == 1 )
        {
            if ( pValue[0] == HID_CMD_SUSPEND ||  pValue[0] == HID_CMD_EXIT_SUSPEND )
            {
                // execute HID app event callback
                (*pHidDevCB->evtCB)( (pValue[0] == HID_CMD_SUSPEND) ?
                                     HID_DEV_SUSPEND_EVT : HID_DEV_EXIT_SUSPEND_EVT );
            }
            else
            {
                status = ATT_ERR_INVALID_VALUE;
            }
        }
        else
        {
            status = ATT_ERR_INVALID_VALUE_SIZE;
        }
    }
    else if ( uuid == GATT_CLIENT_CHAR_CFG_UUID )
    {
        status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                 offset, GATT_CLIENT_CFG_NOTIFY );

        if ( status == SUCCESS )
        {
            uint16 charCfg = BUILD_UINT16( pValue[0], pValue[1] );

            // find report ID in table
            if ( (pRpt = hidDevRptByCccdHandle(pAttr->handle)) != NULL )
            {
                if(hidRptMap[5].cccdHandle==pRpt->cccdHandle)
                {
                    LOG("Audio cfg%4X\n\r",charCfg);
                }

                // execute report callback
                (*pHidDevCB->reportCB)( pRpt->id, pRpt->type, uuid,
                                        (charCfg == GATT_CLIENT_CFG_NOTIFY) ?
                                        HID_DEV_OPER_ENABLE : HID_DEV_OPER_DISABLE,
                                        &len, pValue );
            }
        }
    }
    else if ( uuid == PROTOCOL_MODE_UUID )
    {
        if ( len == HID_PROTOCOL_MODE_LEN )
        {
            if ( pValue[0] == HID_PROTOCOL_MODE_BOOT ||
                    pValue[0] == HID_PROTOCOL_MODE_REPORT )
            {
                pAttr->pValue[0] = pValue[0];
                // execute HID app event callback
                (*pHidDevCB->evtCB)( (pValue[0] == HID_PROTOCOL_MODE_BOOT) ?
                                     HID_DEV_SET_BOOT_EVT : HID_DEV_SET_REPORT_EVT );
            }
            else
            {
                status = ATT_ERR_INVALID_VALUE;
            }
        }
        else
        {
            status = ATT_ERR_INVALID_VALUE_SIZE;
        }
    }

    // restart idle timer
    if (status == SUCCESS)
    {
        hidDevStartIdleTimer();
    }

    return ( status );
}

/*********************************************************************
    @fn      hidDev_ProcessOSALMsg

    @brief   Process an incoming task message.

    @param   pMsg - message to process

    @return  none
*/
static void hidDev_ProcessOSALMsg( osal_event_hdr_t* pMsg )
{
    switch ( pMsg->event )
    {
    case GATT_MSG_EVENT:
        hidDevProcessGattMsg( (gattMsgEvent_t*) pMsg );
        break;

    default:
        break;
    }
}

/*********************************************************************
    @fn      hidDevProcessGattMsg

    @brief   Process GATT messages

    @return  none
*/
static void hidDevProcessGattMsg( gattMsgEvent_t* pMsg )
{
}

/*********************************************************************
    @fn          hidDevHandleConnStatusCB

    @brief       Reset client char config.

    @param       connHandle - connection handle
    @param       changeType - type of change

    @return      none
*/
static void hidDevHandleConnStatusCB( uint16 connHandle, uint8 changeType )
{
    uint8           i;
    hidRptMap_t*     p = pHidDevRptTbl;
    uint16          retHandle;
    gattAttribute_t* pAttr;

    // Make sure this is not loopback connection
    if ( connHandle != LOOPBACK_CONNHANDLE )
    {
        if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )      ||
                ( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) &&
                  ( !linkDB_Up( connHandle ) ) ) )
        {
            for ( i = hidDevRptTblLen; i > 0; i--, p++ )
            {
                if ( p->cccdHandle != 0 )
                {
                    if ( (pAttr = GATT_FindHandle(p->cccdHandle, &retHandle)) != NULL )
                    {
                        GATTServApp_InitCharCfg( connHandle, (gattCharCfg_t*) pAttr->pValue );
                    }
                }
            }
        }
    }
}

/*********************************************************************
    @fn      hidDevDisconnected

    @brief   Handle disconnect.

    @return  none
*/
static void hidDevDisconnected( void )
{
    // Stop idle timer
    hidDevStopIdleTimer();
    // Reset client characteristic configuration descriptors
    Batt_HandleConnStatusCB( gapConnHandle, LINKDB_STATUS_UPDATE_REMOVED );
    //ScanParam_HandleConnStatusCB( gapConnHandle, LINKDB_STATUS_UPDATE_REMOVED );
    hidDevHandleConnStatusCB( gapConnHandle, LINKDB_STATUS_UPDATE_REMOVED );
    // Reset state variables
    hidDevConnSecure = FALSE;
    hidProtocolMode = HID_PROTOCOL_MODE_REPORT;
    hidDevPairingStarted = FALSE;
    // Reset last report sent out
    osal_memset( &lastNoti, 0, sizeof( attHandleValueNoti_t ) );

    // if bonded and normally connectable start advertising
    if ( ( hidDevBondCount() > 0 ) &&
            ( pHidDevCfg->hidFlags & HID_FLAGS_NORMALLY_CONNECTABLE ) )
    {
        hidDevLowAdvertising();
    }
}

/*********************************************************************
    @fn      hidDevGapStateCB

    @brief   Notification from the profile of a state change.

    @param   newState - new state

    @return  none
*/
void hidDevGapStateCB( gaprole_States_t newState )
{
    LOG("%s, %d\n",__FUNCTION__, newState);

    // if connected
    if ( newState == GAPROLE_CONNECTED )
    {
        // get connection handle
        GAPRole_GetParameter( GAPROLE_CONNHANDLE, &gapConnHandle );
        // connection not secure yet
        hidDevConnSecure = FALSE;
        // don't start advertising when connection is closed
        uint8 param = FALSE;
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &param );
        uint8 peerAddress[B_ADDR_LEN];
        GAPRole_GetParameter(GAPROLE_CONN_BD_ADDR, peerAddress);
        LOG("Master Mac:%02X,%02X,%02X,%02X,%02X,%02X\n\r",peerAddress[5],peerAddress[4],peerAddress[3],peerAddress[2],peerAddress[1],peerAddress[0]);
        // start idle timer
        hidDevStartIdleTimer();
    }
    // if disconnected
    else if ( hidDevGapState == GAPROLE_CONNECTED &&
              newState != GAPROLE_CONNECTED )
    {
//  GAPBondMgr_SetParameter(GAPBOND_ERASE_ALLBONDS,0,NULL);
        hidDevDisconnected();
        updateConnParams = TRUE;

        if ( pairingStatus == SMP_PAIRING_FAILED_CONFIRM_VALUE )
        {
            // bonding failed due to mismatched confirm values
            hidDevInitialAdvertising();
            pairingStatus = SUCCESS;
        }
    }
    // if started
    else if ( newState == GAPROLE_STARTED )
    {
        // nothing to do for now!
    }

    hidDevGapState = newState;
}

/*********************************************************************
    @fn      hidDevPairStateCB

    @brief   Pairing state callback.

    @return  none
*/
static void hidDevPairStateCB( uint16 connHandle, uint8 state, uint8 status )
{
    if ( state == GAPBOND_PAIRING_STATE_STARTED )
    {
        hidDevPairingStarted = TRUE;
    }
    else if ( state == GAPBOND_PAIRING_STATE_COMPLETE )
    {
        hidDevPairingStarted = FALSE;

        if ( status == SUCCESS )
        {
            hidDevConnSecure = TRUE;
            LOG("Pair Success\n\r");
        }
        else
        {
            LOG("Pair Fail\n\r");
        }

        pairingStatus = status;
    }
    else if ( state == GAPBOND_PAIRING_STATE_BONDED )
    {
        if ( status == SUCCESS )
        {
            hidDevConnSecure = TRUE;
        }
    }

    if ( !reportQEmpty() && hidDevConnSecure )
    {
        LOG("Set Send Report EVENT\n\r");
        // Notify our task to send out pending reports
        osal_set_event( hidDevTaskId, HID_SEND_REPORT_EVT );
    }
}

/*********************************************************************
    @fn      hidDevPasscodeCB

    @brief   Passcode callback.

    @param   deviceAddr - address of device to pair with, and could be either public or random.
    @param   connectionHandle - connection handle
    @param   uiInputs - pairing User Interface Inputs - Ask user to input passcode
    @param   uiOutputs - pairing User Interface Outputs - Display passcode

    @return  none
*/
void hidDevPasscodeCB( uint8* deviceAddr, uint16 connectionHandle,
                       uint8 uiInputs, uint8 uiOutputs )
{
    if ( pHidDevCB && pHidDevCB->passcodeCB )
    {
        // execute HID app passcode callback
        (*pHidDevCB->passcodeCB)( deviceAddr, connectionHandle, uiInputs, uiOutputs );
    }
    else
    {
        // Send passcode response
        GAPBondMgr_PasscodeRsp( connectionHandle, SUCCESS, 0 );
    }
}

/*********************************************************************
    @fn      hidDevBattCB

    @brief   Callback function for battery service.

    @param   event - service event

    @return  none
*/
void hidDevBattCB( uint8 event )
{
    if ( event == BATT_LEVEL_NOTI_ENABLED )
    {
        // if connected start periodic measurement
        if ( hidDevGapState == GAPROLE_CONNECTED )
        {
            osal_start_timerEx( hidDevTaskId, BATT_PERIODIC_EVT, DEFAULT_BATT_PERIOD );
        }
    }
    else if ( event == BATT_LEVEL_NOTI_DISABLED )
    {
        // stop periodic measurement
        osal_stop_timerEx( hidDevTaskId, BATT_PERIODIC_EVT );
    }
}

/*********************************************************************
    @fn      hidDevScanParamCB

    @brief   Callback function for scan parameter service.

    @param   event - service event

    @return  none
*/
void hidDevScanParamCB( uint8 event )
{
}

/*********************************************************************
    @fn      hidDevBattPeriodicTask

    @brief   Perform a periodic task for battery measurement.

    @param   none

    @return  none
*/
static void hidDevBattPeriodicTask( void )
{
    if ( hidDevGapState == GAPROLE_CONNECTED )
    {
        // perform battery level check
        Batt_MeasLevel( );
        // Restart timer
        osal_start_timerEx( hidDevTaskId, BATT_PERIODIC_EVT, DEFAULT_BATT_PERIOD );
    }
}

/*********************************************************************
    @fn      hidDevRptByHandle

    @brief   Find the HID report structure for the given handle.

    @param   handle - ATT handle

    @return  Pointer to HID report structure
*/
static hidRptMap_t* hidDevRptByHandle( uint16 handle )
{
    uint8       i;
    hidRptMap_t* p = pHidDevRptTbl;

    for ( i = hidDevRptTblLen; i > 0; i--, p++ )
    {
        if ( p->handle == handle && p->mode == hidProtocolMode)
        {
            return p;
        }
    }

    return NULL;
}

/*********************************************************************
    @fn      hidDevRptByCccdHandle

    @brief   Find the HID report structure for the given CCC handle.

    @param   handle - ATT handle

    @return  Pointer to HID report structure
*/
static hidRptMap_t* hidDevRptByCccdHandle( uint16 handle )
{
    uint8       i;
    hidRptMap_t* p = pHidDevRptTbl;

    for ( i = hidDevRptTblLen; i > 0; i--, p++ )
    {
        if ( p->cccdHandle == handle)
        {
            if(i==HID_VOICE_START_IN_CCCD_IDX)
            {
                LOG("Voice Notify Enable!!!!!\n\r");
            }

            return p;
        }
    }

    return NULL;
}

/*********************************************************************
    @fn      hidDevRptById

    @brief   Find the HID report structure for the Report ID and type.

    @param   id - HID report ID
    @param   type - HID report type

    @return  Pointer to HID report structure
*/
static hidRptMap_t* hidDevRptById( uint8 id, uint8 type )
{
    uint8       i;
    hidRptMap_t* p = pHidDevRptTbl;

    for ( i = hidDevRptTblLen; i > 0; i--, p++ )
    {
        if ( p->id == id && p->type == type && p->mode == hidProtocolMode )
        {
            return p;
        }
    }

    return NULL;
}

/*********************************************************************
    @fn      hidDevSendReport

    @brief   Send a HID report.

    @param   id - HID report ID.
    @param   type - HID report type.
    @param   len - Length of report.
    @param   pData - Report data.

    @return  None.
*/
static void hidDevSendReport( uint8 id, uint8 type, uint8 len, uint8* pData )
{
    hidRptMap_t*           pRpt;
    gattAttribute_t*       pAttr;
    uint16                retHandle;
    LOG("%s\n",__FUNCTION__);

    // Get ATT handle for report
    if ( (pRpt = hidDevRptById(id, type)) != NULL )
    {
        // if notifications are enabled
        if ( (pAttr = GATT_FindHandle(pRpt->cccdHandle, &retHandle)) != NULL )
        {
            uint16 value;
            value  = GATTServApp_ReadCharCfg( gapConnHandle, (gattCharCfg_t*) pAttr->pValue );

            if ( value & GATT_CLIENT_CFG_NOTIFY )
            {
                // After service discovery and encryption, the HID Device should request to
                // change to the preferred connection parameters that best suit its use case.
                if ( updateConnParams )
                {
                    GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_REQ, sizeof( uint8 ), &updateConnParams );
                    updateConnParams = FALSE;
                }

                // send notification
                lastNoti.handle = pRpt->handle;
                lastNoti.len = len;
                osal_memcpy(lastNoti.value, pData, len);
                GATT_Notification( gapConnHandle, &lastNoti, FALSE );
                // start idle timer
                hidDevStartIdleTimer();
            }
            else
            {
                LOG("notify fail\n\r");
            }
        }
    }
}

/*********************************************************************
    @fn      hidDevEnqueueReport

    @brief   Enqueue a HID report to be sent later.

    @param   id - HID report ID.
    @param   type - HID report type.
    @param   len - Length of report.
    @param   pData - Report data.

    @return  None.
*/
static void hidDevEnqueueReport( uint8 id, uint8 type, uint8 len, uint8* pData )
{
    // Enqueue only if bonded
    if ( hidDevBondCount() > 0 )
    {
        // Update last index
        lastQIdx = ( lastQIdx + 1 ) % HID_DEV_REPORT_Q_SIZE;

        if ( lastQIdx == firstQIdx )
        {
            // Queue overflow; discard oldest report
            firstQIdx = ( firstQIdx + 1 ) % HID_DEV_REPORT_Q_SIZE;
        }

        // Save report
        hidDevReportQ[lastQIdx].id = id;
        hidDevReportQ[lastQIdx].type = type;
        hidDevReportQ[lastQIdx].len = len;
        osal_memcpy( hidDevReportQ[lastQIdx].data, pData, len );

        if ( hidDevConnSecure )
        {
            // Notify our task to send out pending reports
            osal_set_event( hidDevTaskId, HID_SEND_REPORT_EVT );
        }
    }
}

/*********************************************************************
    @fn      hidDevDequeueReport

    @brief   Dequeue a HID report to be sent out.

    @param   id - HID report ID.
    @param   type - HID report type.
    @param   len - Length of report.
    @param   pData - Report data.

    @return  None.
*/
static hidDevReport_t* hidDevDequeueReport( void )
{
    if ( reportQEmpty() )
    {
        return NULL;
    }

    // Update first index
    firstQIdx = ( firstQIdx + 1 ) % HID_DEV_REPORT_Q_SIZE;
    return ( &(hidDevReportQ[firstQIdx]) );
}

/*********************************************************************
    @fn      hidDevHighAdvertising

    @brief   Start advertising at a high duty cycle.

    @param   None.

    @return  None.
*/
static void hidDevHighAdvertising( void )
{
    uint8 param;
    VOID GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, HID_HIGH_ADV_INT_MIN );
    VOID GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, HID_HIGH_ADV_INT_MAX );
    VOID GAP_SetParamValue( TGAP_LIM_ADV_TIMEOUT, HID_HIGH_ADV_TIMEOUT );
    // Setup adverstising filter policy first
    param = GAP_FILTER_POLICY_WHITE;
    VOID GAPRole_SetParameter( GAPROLE_ADV_FILTER_POLICY, sizeof( uint8 ), &param );
    param = TRUE;
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &param );
}

/*********************************************************************
    @fn      hidDevLowAdvertising

    @brief   Start advertising at a low duty cycle.

    @param   None.

    @return  None.
*/
static void hidDevLowAdvertising( void )
{
    uint8 param;
    VOID GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, HID_LOW_ADV_INT_MIN );
    VOID GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, HID_LOW_ADV_INT_MAX );
    VOID GAP_SetParamValue( TGAP_LIM_ADV_TIMEOUT, HID_LOW_ADV_TIMEOUT );
    // Setup adverstising filter policy first
    param = GAP_FILTER_POLICY_ALL;//GAP_FILTER_POLICY_WHITE teddy modify
    VOID GAPRole_SetParameter( GAPROLE_ADV_FILTER_POLICY, sizeof( uint8 ), &param );
    param = TRUE;
    VOID GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &param );
}

/*********************************************************************
    @fn      hidDevInitialAdvertising

    @brief   Start advertising for initial connection

    @return  None.
*/
static void hidDevInitialAdvertising( void )
{
    uint8 param;
    VOID GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, HID_INITIAL_ADV_INT_MIN );
    VOID GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, HID_INITIAL_ADV_INT_MAX );
    VOID GAP_SetParamValue( TGAP_LIM_ADV_TIMEOUT, HID_INITIAL_ADV_TIMEOUT );
    // Setup adverstising filter policy first
    param = GAP_FILTER_POLICY_ALL;
    VOID GAPRole_SetParameter( GAPROLE_ADV_FILTER_POLICY, sizeof( uint8 ), &param );
    param = TRUE;
    VOID GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &param );
}

/*********************************************************************
    @fn      hidDevBondCount

    @brief   Gets the total number of bonded devices.

    @param   None.

    @return  number of bonded devices.
*/
static uint8 hidDevBondCount( void )
{
    uint8 bondCnt = 0;
    VOID GAPBondMgr_GetParameter( GAPBOND_BOND_COUNT, &bondCnt );
    return ( bondCnt );
}

/*********************************************************************
    @fn      hidDevStartIdleTimer

    @brief   Start the idle timer.

    @return  None.
*/
static void hidDevStartIdleTimer( void )
{
    if ( pHidDevCfg->idleTimeout > 0 )
    {
        osal_start_timerEx( hidDevTaskId, HID_IDLE_EVT, pHidDevCfg->idleTimeout );
    }
}

/*********************************************************************
    @fn      hidDevStopIdleTimer

    @brief   Stop the idle timer.

    @return  None.
*/
static void hidDevStopIdleTimer( void )
{
    osal_stop_timerEx( hidDevTaskId, HID_IDLE_EVT );
}


/*********************************************************************
    @fn      HidDev_scanParamCB

    @brief   Callback function for scan parameter service.

    @param   event - service event

    @return  none
*/
static void HidDev_scanParamCB(uint8_t event)
{
    // Do nothing.
}


/*********************************************************************
*********************************************************************/
