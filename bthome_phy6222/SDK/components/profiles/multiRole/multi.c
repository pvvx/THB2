/******************************************************************************

    @file       multi.c

    @brief multi GAPRole profile code

    Group: CMCU, SCS

 SDK_LICENSE

 ******************************************************************************/

/*********************************************************************
    INCLUDES
*/
//#include <string.h>
#include "bcomdef.h"
#include "OSAL.h"
#include "hci_tl.h"
#include "l2cap.h"
#include "gap.h"
#include "linkdb.h"
#include "gatt.h"
#include "osal_snv.h"
#include "gapbondmgr.h"

/* This Header file contains all BLE API and icall structure definition */
#include "multi.h"
#include "multi_role.h"
#include "log.h"
#include "flash.h"
#include "multi_timer.h"
#include "multi_schedule.h"

/*********************************************************************
    MACROS
*/
#define MULTI_ROLE_CENTRAL_HANDLER      0xFE

// Length of bd addr as a string
#define B_ADDR_STR_LEN                  15

// default Exchange MTU SIZE
#define DEFAULT_EXCHANGE_MTU_LEN        23


/*********************************************************************
    GLOBAL VARIABLES
*/

// Link DB maximum number of connections
uint8 linkDBNumConns = MAX_NUM_LL_CONN;      // hardcoded,

/*********************************************************************
    EXTERNAL VARIABLES
*/
extern GAPMultiRoleLinkCtrl_t* g_multiLinkInfo;

/*********************************************************************
    EXTERNAL FUNCTIONS
*/
extern uint8 gapBond_PairingMode[];

/*********************************************************************
    LOCAL VARIABLES
*/
uint8 gapMultiRole_TaskID;
// Application callbacks
static gapMultiRolesCBs_t* pGapRoles_AppCGs = NULL;

/*********************************************************************
    Profile Parameters - reference GAPMULTIROLE_PROFILE_PARAMETERS for
    descriptions
*/
#if (MAX_CONNECTION_MASTER_NUM > 0 )
    // scanner list
    static GAPMultiRolScanner_t* g_scanlist = NULL;
    GAPMultiRoleCentralDev_t* g_cfgSlaveList = NULL;        // devices want to be connected
    GAPMultiRoleCentralDev_t* g_mlinkingList = NULL;        // devices
    GAPMultiRoleCentralAction_t* g_centralAction = NULL;

    multiTimer* g_centralActionTimer = NULL;

    GAPMultiRole_CentralSDP_t* g_centralSDPlist = NULL;
#endif

// multi-role common info
static GAPMultiRoleParam_t g_multiRoleParam;

#if(MAX_CONNECTION_SLAVE_NUM > 0 )
    // multi-role as peripheral Info
    // max support DEFAULT_SLAVE_CNT slave
    multiTimer g_peri_conn_update_timer[MAX_CONNECTION_SLAVE_NUM];
    multiTimer g_pcu_no_success_timer[MAX_CONNECTION_SLAVE_NUM];

    static uint8 paramUpdateNoSuccessOption = MULTIROLE_NO_ACTION;
#endif

// parameter update no success actions

/*********************************************************************
    Profile Attributes - variables
*/

/*********************************************************************
    Profile Attributes - Table
*/

/*********************************************************************
    EXTERN FUNCTIONS
*/
#if( MAX_CONNECTION_SLAVE_NUM > 0 )
    extern void multiConfigSchAdv_param(uint8 opcode,uint8 status,uint8 advType );
#endif
#if( MAX_CONNECTION_MASTER_NUM > 0 )
    extern void multiConfigSchScan_param(void);
#endif
extern GAPMultiLinkInfo_t multiConfigLink_status(uint8 opcode,void* pkt);

/*********************************************************************
    LOCAL FUNCTIONS
*/
static void MultiPeriodProcessEvent(void);
#if( MAX_CONNECTION_SLAVE_NUM > 0 )
    static void Multi_peripheralUpdateParam(uint16 idx);
    static void MultiRole_PeripheralstartConnUpdate( uint8 idx,uint8 handleFailure );
    static void MultiRole_HandleParamUpdateNoSuccess( uint16 idx );
#endif
#if( MAX_CONNECTION_MASTER_NUM > 0 )
    static void* multiListCreate(GAPMultiListMode_t mode );
    static void* multiListFindTail(GAPMultiListMode_t mode, void** ppnode );
    static void multiListDelNode(GAPMultiListMode_t mode, void** ppnode, void* pvalue );
    static uint8 multiListInsertTail(GAPMultiListMode_t mode, void** ppnode,void* vnode);
    static void* multiList_inside(GAPMultiListMode_t mode,void** ppnode, void* pvalue);
    static void multiListMemoryFree(GAPMultiListMode_t mode );
    void multi_ScannerInsertDev(gapDeviceInfoEvent_t* pMsg);
    static uint8 MultiRole_CancelConn(void);
    static void Multi_centralAction(uint16 idx);
    static void  MultiRole_ProcessSDPInfo(gattMsgEvent_t* pMsg);
    static void multilist_free_central_action_actimerlist(uint16 connHandle);
    static void multilist_free_sdplist(uint16 connHandle);

#endif

static void MultiRole_ProcessParamUpdateInfo(gapLinkUpdateEvent_t* pPkt);
static void MultiRole_ProcessOSALMsg( osal_event_hdr_t* pMsg );
static uint8 MultiRole_processGAPMsg(gapEventHdr_t* pMsg);
static void multiRoleProcessGATTMsg( gattMsgEvent_t* pMsg );
static void MultiRole_SetupGAP(uint8* numConns);
static void multiRolePasscodeCB( uint8* deviceAddr, uint16 connectionHandle,
                                 uint8 uiInputs, uint8 uiOutputs );
static void multiRolePairStateCB( uint16 connHandle, uint8 state, uint8 status );



// Bond Manager Callbacks
static const gapBondCBs_t multiRoleBondCB =
{
    multiRolePasscodeCB,
    multiRolePairStateCB
};

/*********************************************************************
    @brief   Set a GAP Role parameter.

    Public function defined in peripheral.h.
*/
bStatus_t GAPMultiRole_SetParameter(uint16 param, uint8 len, void* pValue)
{
    bStatus_t ret = SUCCESS;

    switch (param)
    {
    case GAPMULTIROLE_PROFILEROLE:
    {
        if (len == sizeof(uint8))
        {
            g_multiRoleParam.common.profileRole = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case GAPMULTIROLE_IRK:
    {
        if (len == KEYLEN)
        {
            osal_memcpy(g_multiRoleParam.common.IRK, pValue, KEYLEN);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case GAPMULTIROLE_SRK:
    {
        if (len == KEYLEN)
        {
            osal_memcpy(g_multiRoleParam.common.SRK, pValue, KEYLEN);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case GAPMULTIROLE_SIGNCOUNTER:
    {
        if (len == sizeof (uint32))
        {
            g_multiRoleParam.common.signCounter = *((uint32*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case GAPMULTIROLE_MIN_CONN_INTERVAL:
    {
        if ( len == sizeof (uint16) )
        {
            g_multiRoleParam.common.ExConnIntvMIN = *((uint16*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case GAPMULTIROLE_MAX_CONN_INTERVAL:
    {
        if ( len == sizeof (uint16)  )
        {
            g_multiRoleParam.common.ExConnIntvMAX = *((uint16*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case GAPMULTIROLE_SLAVE_LATENCY:
    {
        if ( len == sizeof (uint16) )
        {
            g_multiRoleParam.common.ExLatency = *((uint16*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case GAPMULTIROLE_TIMEOUT_MULTIPLIER:
    {
        if ( len == sizeof (uint16)  )
        {
            g_multiRoleParam.common.ExTimeOut = *((uint16*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;
    #if(MAX_CONNECTION_SLAVE_NUM > 0 )

    case GAPMULTIROLE_ADV_EVENT_TYPE:
    {
        if ((len == sizeof (uint8)) && (*((uint8*)pValue) <= GAP_ADTYPE_ADV_LDC_DIRECT_IND) )
        {
            g_multiRoleParam.adv.EventType = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case GAPMULTIROLE_ADV_CHANNEL_MAP:
    {
        if ((len == sizeof (uint8)) && (*((uint8*)pValue) <= 0x07))
        {
            g_multiRoleParam.adv.ChanMap = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case GAPMULTIROLE_ADV_FILTER_POLICY:
    {
        if ((len == sizeof (uint8)) && (*((uint8*)pValue) <= GAP_FILTER_POLICY_WHITE) )
        {
            g_multiRoleParam.adv.FilterPolicy = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case GAPMULTIROLE_PARAM_UPDATE_ENABLE:
    {
        if ( len == sizeof (uint8) )
        {
            g_multiRoleParam.adv.UpdateEnable = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;
    #endif
    #if (MAX_CONNECTION_MASTER_NUM > 0 )

    case GAPMULTIROLE_MAX_SCAN_RES:
    {
        if (len == sizeof (uint8))
        {
            g_multiRoleParam.scan.maxScanRes = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case GAPMULTIROLE_SCAN_MODE:
    {
        if (len == sizeof (uint8))
        {
            g_multiRoleParam.scan.scanMode = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case GAPMULTIROLE_ACTIVE_SCAN:
    {
        if (len == sizeof (uint8))
        {
            g_multiRoleParam.scan.activeScan = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case GAPMULTIROLE_SCAN_WHITELIST:
    {
        if (len == sizeof (uint8))
        {
            g_multiRoleParam.scan.whitelist = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case GAPMULTIROLE_LINK_HIGHDUTYCYCLE:
    {
        if (len == sizeof (uint8))
        {
            g_multiRoleParam.link.highDutyCycle = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case GAPMULTIROLE_LINK_WHITELIST:
    {
        if (len == sizeof (uint8))
        {
            g_multiRoleParam.link.whitelist = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;

    case GAPMULTIROLE_ACTION_AFTER_LINK:
    {
        if (len == sizeof (uint16))
        {
            g_multiRoleParam.link.actionAfterLink = *((uint16*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    break;
    #endif

    default:
    {
        // The param value isn't part of this profile, try the GAP.
        if ((param < TGAP_PARAMID_MAX) && (len == sizeof (uint16)))
        {
            ret = GAP_SetParamValue(param, *((uint16*)pValue));
        }
        else
        {
            ret = INVALIDPARAMETER;
        }
    }
    break;
    }

    return (ret);
}

/*********************************************************************
    @brief   Get a GAP Role parameter.

    Public function defined in peripheral.h.
*/
bStatus_t GAPMultiRole_GetParameter(uint16 param, void* pValue)
{
    bStatus_t ret = SUCCESS;

    switch (param)
    {
    case GAPMULTIROLE_PROFILEROLE:
        *((uint8*)pValue) = g_multiRoleParam.common.profileRole;
        break;

    case GAPMULTIROLE_IRK:
        osal_memcpy(pValue, g_multiRoleParam.common.IRK, KEYLEN);
        break;

    case GAPMULTIROLE_SRK:
        osal_memcpy(pValue, g_multiRoleParam.common.SRK, KEYLEN);
        break;

    case GAPMULTIROLE_SIGNCOUNTER:
        *((uint32*)pValue) = g_multiRoleParam.common.signCounter;
        break;

    case GAPMULTIROLE_BD_ADDR:
        osal_memcpy(pValue, g_multiRoleParam.common.bdAddr, B_ADDR_LEN);
        break;

    case GAPMULTIROLE_MIN_CONN_INTERVAL:
        *((uint16*)pValue) = g_multiRoleParam.common.ExConnIntvMIN;
        break;

    case GAPMULTIROLE_MAX_CONN_INTERVAL:
        *((uint16*)pValue) = g_multiRoleParam.common.ExConnIntvMAX;
        break;

    case GAPMULTIROLE_SLAVE_LATENCY:
        *((uint16*)pValue) = g_multiRoleParam.common.ExLatency;
        break;

    case GAPMULTIROLE_TIMEOUT_MULTIPLIER:
        *((uint16*)pValue) = g_multiRoleParam.common.ExTimeOut;
        break;
        #if(MAX_CONNECTION_SLAVE_NUM > 0 )

    case GAPMULTIROLE_ADV_EVENT_TYPE:
        *((uint8*)pValue) = g_multiRoleParam.adv.EventType;
        break;

    case GAPMULTIROLE_ADV_CHANNEL_MAP:
        *((uint8*)pValue) = g_multiRoleParam.adv.ChanMap;
        break;

    case GAPMULTIROLE_ADV_FILTER_POLICY:
        *((uint8*)pValue) = g_multiRoleParam.adv.FilterPolicy;
        break;

    case GAPMULTIROLE_PARAM_UPDATE_ENABLE:
        *((uint8*)pValue) = g_multiRoleParam.adv.UpdateEnable;
        break;
        #endif
        #if (MAX_CONNECTION_MASTER_NUM > 0 )

    case GAPMULTIROLE_MAX_SCAN_RES:
        *((uint8*)pValue) = g_multiRoleParam.scan.maxScanRes;
        break;
        #endif

    default:
    {
        // The param value isn't part of this profile, try the GAP.
        if (param < TGAP_PARAMID_MAX)
        {
            *((uint16*)pValue) = GAP_GetParamValue(param);
        }
        else
        {
            ret = INVALIDPARAMETER;
        }
    }
    break;
    }

    return (ret);
}

/*********************************************************************
    @brief   Does the device initialization.

    Public function defined in peripheral.h.
*/
bStatus_t GAPMultiRole_StartDevice(gapMultiRolesCBs_t* pAppCallbacks, uint8* numConns)
{
    // Clear all of the Application callbacks
    if (pAppCallbacks)
    {
        pGapRoles_AppCGs = pAppCallbacks;
    }

    // Start the GAP
    MultiRole_SetupGAP(numConns);
    return (SUCCESS);
}

/*********************************************************************
    @brief   Terminates the existing connection.

    Public function defined in peripheral.h.
*/
bStatus_t GAPMultiRole_TerminateConnection(uint16_t Handler)
{
    return (GAP_TerminateLinkReq(gapMultiRole_TaskID, Handler,
                                 HCI_DISCONNECT_REMOTE_USER_TERM));
}



/*********************************************************************
    LOCAL FUNCTION PROTOTYPES
*/
/*********************************************************************
    @fn      bdAddr2Str

    @brief   Convert Bluetooth address to string. Only needed when
           LCD display is used.

    @return  none
*/
char* bdAddr2Str( uint8* pAddr )
{
    uint8       i;
    char        hex[] = "0123456789ABCDEF";
    static char str[B_ADDR_STR_LEN];
    char*        pStr = str;
    *pStr++ = '0';
    *pStr++ = 'x';
    // Start from end of addr
    pAddr += B_ADDR_LEN;

    for ( i = B_ADDR_LEN; i > 0; i-- )
    {
        *pStr++ = hex[*--pAddr >> 4];
        *pStr++ = hex[*pAddr & 0x0F];
    }

    *pStr = 0;
    return str;
}

/*********************************************************************
    @fn      gapRole_init

    @brief   Initialization function for the GAP Role Task.

    @param   none

    @return  none
*/
void GAPMultiRole_Init(uint8 taskId)
{
    gapMultiRole_TaskID = taskId;
    multiSchedule_init( gapMultiRole_TaskID );
    osal_memset(&g_multiRoleParam,0,sizeof(GAPMultiRoleParam_t));
    #if(MAX_CONNECTION_MASTER_NUM > 0 )
    // Initialize GATT Client
    GATT_InitClient();
    // Register to receive incoming ATT Indications/Notifications
    GATT_RegisterForInd( gapMultiRole_TaskID );
    #endif
//     Register with bond manager after starting device
    GAPBondMgr_Register( (gapBondCBs_t*) &multiRoleBondCB );
//    GAP_RegisterForHCIMsgs( taskId );
//    HCI_PPLUS_AdvEventDoneNoticeCmd(gapMultiRole_TaskID,MULTI_ADV_EVENT_DONE_EVT);
    osal_start_reload_timer(gapMultiRole_TaskID, MULTI_PERIOD_EVT,  MULTI_PERIOD_TIMING);
}

/**
    @brief   Establish a link to a peer device.

    Public function defined in central.h.
*/
bStatus_t GAPMultiRole_EstablishLink(uint8 highDutyCycle, uint8 whiteList,
                                     uint8 addrTypePeer, uint8* peerAddr)
{
    gapEstLinkReq_t params;
    params.taskID = gapMultiRole_TaskID;
    params.highDutyCycle = highDutyCycle;
    params.whiteList = whiteList;
    params.addrTypePeer = addrTypePeer;
    osal_memcpy(params.peerAddr, peerAddr, B_ADDR_LEN);
    return GAP_EstablishLinkReq(&params);
}


/*********************************************************************
    @fn      gapRole_taskFxn

    @brief   Task entry point for the GAP Peripheral Role.

    @param   a0 - first argument
    @param   a1 - second argument

    @return  none
*/
uint16 GAPMultiRole_ProcessEvent( uint8 task_id, uint16 events )
{
    if ( events & SYS_EVENT_MSG )
    {
        uint8* pMsg;

        if ( (pMsg = osal_msg_receive( gapMultiRole_TaskID )) != NULL )
        {
            MultiRole_ProcessOSALMsg( (osal_event_hdr_t*)pMsg );
            // Release the OSAL message
            VOID osal_msg_deallocate( pMsg );
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if( events & MULTI_SCHEDULE_EVT)
    {
        multiScheduleProcess();
        return ( events ^ MULTI_SCHEDULE_EVT );
    }

    if( events & MULTI_ADV_EVENT_DONE_EVT )
    {
//        MultiProcessAdvEvent();
        return ( events ^ MULTI_ADV_EVENT_DONE_EVT );
    }

    #if( MAX_CONNECTION_MASTER_NUM > 0 )

    if( events & CONN_TIMEOUT_EVT )
    {
        MultiRole_CancelConn();
        return ( events ^ CONN_TIMEOUT_EVT );
    }

    #endif

    if( events & MULTI_PERIOD_EVT )
    {
        MultiPeriodProcessEvent();
        return ( events ^ MULTI_PERIOD_EVT );
    }

    return 0;
}

/*********************************************************************
    @fn      gapRole_processStackMsg

    @brief   Process an incoming task message.

    @param   pMsg - message to process

    @return  none
*/
//static uint8 gapRole_processStackMsg(ICall_Hdr *pMsg)
static void MultiRole_ProcessOSALMsg( osal_event_hdr_t* pMsg )
{
    //  uint8 safeToDealloc = TRUE;
    switch (pMsg->event)
    {
    case GAP_MSG_EVENT:
        MultiRole_processGAPMsg((gapEventHdr_t*)pMsg);
        break;

    case L2CAP_SIGNAL_EVENT:
    {
    }
    break;

    case GATT_MSG_EVENT:
        multiRoleProcessGATTMsg( (gattMsgEvent_t*) pMsg );
        break;

    default:
        break;
    }
}


/*********************************************************************
    @fn      gapRole_processGAPMsg

    @brief   Process an incoming task message.

    @param   pMsg - message to process

    @return  none
*/
static uint8 MultiRole_processGAPMsg(gapEventHdr_t* pMsg)
{
    switch (pMsg->opcode)
    {
    // Device initialized
    case GAP_DEVICE_INIT_DONE_EVENT:
    {
        gapDeviceInitDoneEvent_t* pPkt = (gapDeviceInitDoneEvent_t*)pMsg;
        bStatus_t stat = pPkt->hdr.status;

        if (stat == SUCCESS)
        {
            LOG("Device Init Done \n");
            LOG("BLE Multi-Role Address:" );
            LOG( bdAddr2Str( pPkt->devAddr ) );
            LOG("\n");
            LOG("HCI_LE PKT LEN %d\n",pPkt->dataPktLen);
            LOG("HCI_LE NUM TOTAL %d\n",pPkt->numDataPkts);
        }
        else
        {
            LOG("Device Init Done ERR_Code 0x%02X\n",stat);
        }
    }
    break;
    #if( MAX_CONNECTION_SLAVE_NUM > 0 )

    // Update advertising done
    case GAP_ADV_DATA_UPDATE_DONE_EVENT:
    {
        gapAdvDataUpdateEvent_t* pPkt = (gapAdvDataUpdateEvent_t*)pMsg;
        multiConfigSchAdv_param( pMsg->opcode,pPkt->hdr.status, pPkt->adType);
    }
    break;

    case GAP_MAKE_DISCOVERABLE_DONE_EVENT:
    {
        gapMakeDiscoverableRspEvent_t* pPkt = (gapMakeDiscoverableRspEvent_t*)pMsg;
        multiConfigSchAdv_param( pMsg->opcode,pPkt->hdr.status, NULL);
    }
    break;

    case GAP_END_DISCOVERABLE_DONE_EVENT:
    {
        gapEndDiscoverableRspEvent_t* pPkt = (gapEndDiscoverableRspEvent_t*)pMsg;
        multiConfigSchAdv_param( pMsg->opcode,pPkt->hdr.status, NULL);
    }
    break;
    #endif

    // Connection formed
    case GAP_LINK_ESTABLISHED_EVENT:
    {
        GAPMultiLinkInfo_t info;
        gapEstLinkReqEvent_t* pPkt = (gapEstLinkReqEvent_t*)pMsg;

        if( pPkt->hdr.status == SUCCESS )
        {
            info = multiConfigLink_status(pMsg->opcode,pMsg);
            #if( MAX_CONNECTION_SLAVE_NUM > 0 )

            if( info.value.role == Slave_Role )
            {
//								gapBond_PairingMode[ pPkt->connectionHandle ] = GAPBOND_PAIRING_MODE_INITIATE ;
								gapBond_PairingMode[ pPkt->connectionHandle ] = GAPBOND_PAIRING_MODE_NO_PAIRING ;
                uint8 bondret = GAPBondMgr_LinkEst( pPkt->devAddrType, pPkt->devAddr, pPkt->connectionHandle, GAP_PROFILE_PERIPHERAL );
                LOG("GAPBondMgr_LinkEst SLAVE bondret %d\n",bondret);
                // varify if enable connection parameter update?
                uint8 param_update_enable = FALSE;
                GAPMultiRole_GetParameter(GAPMULTIROLE_PARAM_UPDATE_ENABLE, &param_update_enable );

                if( param_update_enable )
                {
                    // enable connection parameter update procedure
                    uint16 Conndelay = 0;
                    GAPMultiRole_GetParameter(TGAP_CONN_PAUSE_PERIPHERAL, &Conndelay );
                    LOG("multitimer_init perIdx %d\n",info.value.perIdx);
                    multitimer_init(&g_peri_conn_update_timer[info.value.perIdx], Multi_peripheralUpdateParam, \
                                    Conndelay*1000, 0,info.value.perIdx);
                    g_peri_conn_update_timer[info.value.perIdx].valid = TRUE;
                    int ret = multitimer_start(&g_peri_conn_update_timer[info.value.perIdx]);
                    LOG("multitimer_start RET %d\n",ret);
                }
            }

            #endif
            #if( MAX_CONNECTION_MASTER_NUM > 0 )

            if( info.value.role == Master_Role )
            {
                gapBond_PairingMode[ pPkt->connectionHandle ] = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ ;
                osal_stop_timerEx(gapMultiRole_TaskID, CONN_TIMEOUT_EVT );
                multiDelCurrentConnNode();
//              uint8 bondret = GAPBondMgr_LinkEst( pPkt->devAddrType, pPkt->devAddr, pPkt->connectionHandle, GAP_PROFILE_CENTRAL );
//              LOG("GAPBondMgr_LinkEst MASTER bondret %d\n",bondret);
                // TODO : read user area flash to check the action after establish success
                // action_readFlash shall according different address
                uint16 action_readFlash = 0;
                uint16 action = g_multiRoleParam.link.actionAfterLink;
                uint16 tAction = action ^ action_readFlash;

                // xor value tAction not null , means there's action(s) to be run
                if( tAction )
                {
                    GAPMultiRoleCentralAction_t node;
                    node.connHandle = info.value.connHandle;
                    node.action = tAction;
                    node.busy = FALSE;
                    node.next = NULL;

                    if(multiListInsertTail(MULTI_CONFIG_MASTER_ACTION_MODE,(void**)&g_centralAction,&node) )
                    {
                        multiTimer tim_node;
                        osal_memset(&tim_node,0,sizeof(multiTimer));
                        tim_node.next = NULL;
                        tim_node.id = info.value.connHandle;

                        if(multiListInsertTail( MULTI_CENTRAL_ACTION_TIMER_MODE,(void**)&g_centralActionTimer,&tim_node ) )
                        {
                            multiTimer* entry = multiListFindTail(MULTI_CENTRAL_ACTION_TIMER_MODE,(void**)&g_centralActionTimer);
                            multitimer_init(entry, Multi_centralAction,500, 1,info.value.connHandle);
                            multitimer_start(entry);
                        }
                    }
                }
            }

            #endif
        }

        if( pGapRoles_AppCGs->pfnEstablish )
            pGapRoles_AppCGs->pfnEstablish(pPkt->hdr.status,info.value.connHandle,\
                                           ((Master_Role == info.value.role) ? Master_Role:Slave_Role),info.value.perIdx,pPkt->devAddr);
    }
    break;

    case GAP_LINK_TERMINATED_EVENT:
    {
        GAPMultiLinkInfo_t info;
        gapTerminateLinkEvent_t* pPkt = (gapTerminateLinkEvent_t*)pMsg;

        if( pPkt->hdr.status == SUCCESS )
        {
            info = multiConfigLink_status(pMsg->opcode,pMsg);
            #if( MAX_CONNECTION_MASTER_NUM > 0 )
            multilist_free_central_action_actimerlist(pPkt->connectionHandle);
            // if sdp not success
            multilist_free_sdplist(pPkt->connectionHandle);
            #endif
            #if( MAX_CONNECTION_SLAVE_NUM > 0 )
            multitimer_stop(&g_peri_conn_update_timer[info.value.perIdx]);
            multitimer_stop(&g_pcu_no_success_timer[ info.value.perIdx ]);
            #endif
        }

        if( pGapRoles_AppCGs->pfnTerminate )
            pGapRoles_AppCGs->pfnTerminate(info.value.connHandle,\
                                           ((Master_Role == info.value.role) ? Master_Role:Slave_Role),info.value.perIdx,pPkt->reason);
    }
    break;

    // Connection parameter update
    case GAP_LINK_PARAM_UPDATE_EVENT:
    {
        gapLinkUpdateEvent_t* pPkt = (gapLinkUpdateEvent_t*)pMsg;
        MultiRole_ProcessParamUpdateInfo(pPkt);
    }
    break;
    #if( MAX_CONNECTION_MASTER_NUM > 0 )

    case GAP_DEVICE_INFO_EVENT:
    {
        gapDeviceInfoEvent_t* pPkt = (gapDeviceInfoEvent_t*)pMsg;
        multi_ScannerInsertDev( pPkt );

        if( pGapRoles_AppCGs && pGapRoles_AppCGs->pfnEachScan )
            pGapRoles_AppCGs->pfnEachScan(pPkt);
    }
    break;

    case GAP_DEVICE_DISCOVERY_EVENT:
    {
        if( pGapRoles_AppCGs && pGapRoles_AppCGs->pfnScanDone )
            pGapRoles_AppCGs->pfnScanDone(g_scanlist);

        multiListMemoryFree(MULTI_SCAN_MODE);
        multiConfigSchScan_param();
    }
    break;

    // Security request received from slave
    case GAP_SLAVE_REQUESTED_SECURITY_EVENT:
    {
        LOG("GAP slave requested security event \n");
        // copy from central.c --> temporary walkaround
        GAPBondMgr_ProcessGAPMsg( pMsg );
    }
    break;
    #endif

    default:
        break;
    }

    return TRUE;
}


/*********************************************************************
    @fn      multiRoleProcessGATTMsg

    @brief   Process GATT messages

    @return  none
*/
static void multiRoleProcessGATTMsg( gattMsgEvent_t* pMsg )
{
    #if( MAX_CONNECTION_MASTER_NUM > 0 )
    uint16 handle = pMsg->connHandle;

    switch( pMsg->method )
    {
    case ATT_EXCHANGE_MTU_RSP:
    {
        LOG("connHand %d,exchangeMTU status %d,rxMTU %d\n", pMsg->connHandle,\
            pMsg->hdr.status,\
            pMsg->msg.exchangeMTURsp.serverRxMTU);
        GAPMultiRoleCentralAction_t* action_node = NULL;
        action_node =   multiList_inside(MULTI_CONFIG_MASTER_ACTION_MODE,(void**)&g_centralAction,&handle);

        if( action_node )
            action_node->busy = FALSE;
    }
    break;

    case ATT_READ_RSP:
        if( pMsg->hdr.status == SUCCESS )
        {
            LOG("Read Success Handle %d,len %d ",pMsg->connHandle,pMsg->msg.readRsp.len );

            for( uint8 i=0; i < pMsg->msg.readRsp.len; i++)
                LOG("0x%02X,",pMsg->msg.readRsp.value[i]);

            LOG("\n");
        }

        break;

    case ATT_WRITE_REQ:
        if( pMsg->hdr.status == SUCCESS )
        {
            LOG( "Write sent connHandle %d\n",pMsg->connHandle );
        }

        break;

    case ATT_HANDLE_VALUE_NOTI:
        if( pMsg->hdr.status == SUCCESS )
        {
            if( pGapRoles_AppCGs && pGapRoles_AppCGs->dataNotify )
                pGapRoles_AppCGs->dataNotify( pMsg->connHandle, pMsg->msg.handleValueNoti.len, pMsg->msg.handleValueNoti.value );
        }

        break;

    case ATT_READ_BY_GRP_TYPE_RSP:
    case ATT_READ_BY_TYPE_RSP:
        MultiRole_ProcessSDPInfo(pMsg);
        break;

    case ATT_ERROR_RSP:
        switch( pMsg->msg.errorRsp.reqOpcode )
        {
        case ATT_READ_REQ:
            LOG( "Read Error %d\n", pMsg->msg.errorRsp.errCode );
            break;

        case ATT_WRITE_REQ:
            LOG( "Write Error: %d\n", pMsg->msg.errorRsp.errCode );
            break;

        case ATT_HANDLE_VALUE_NOTI:
            LOG( "Notify Error: %d\n", pMsg->msg.errorRsp.errCode );
            break;

        default:
            LOG( "connHandle %d,ATT_ERROR_RSP %d\n", handle,pMsg->msg.errorRsp.reqOpcode );
            break;
        }

        break;

    default:
        break;
    }

    #endif
}

/*********************************************************************
    @fn      MultiRole_SetupGAP

    @brief   Call the GAP Device Initialization function using the
           Profile Parameters. Negotiate the maximum number
           of simultaneous connections with the stack.

    @param   numConns - desired number of simultaneous connections

    @return  none
*/
static void MultiRole_SetupGAP(uint8* numConns)
{
    // Set number of possible simultaneous connections
    *numConns = linkDBNumConns;
    uint8 maxScanNum = 0;
    #if (MAX_CONNECTION_MASTER_NUM > 0 )
    maxScanNum = g_multiRoleParam.scan.maxScanRes;
    LOG("maxScanNum %d\n",maxScanNum);
    #endif
    bStatus_t ret= GAP_DeviceInit(  gapMultiRole_TaskID, g_multiRoleParam.common.profileRole, maxScanNum,
                                    g_multiRoleParam.common.IRK, g_multiRoleParam.common.SRK,
                                    (uint32*)&g_multiRoleParam.common.signCounter);
    LOG("%s,ret %d\n",__func__,ret);
}

void multiRolePasscodeCB( uint8* deviceAddr, uint16 connectionHandle,
                          uint8 uiInputs, uint8 uiOutputs )
{
    uint32 passcode = 0;
    LOG("pass code CB connHandle 0x%02X,uiInputs:0x%x,uiOutputs:0x%x\n",connectionHandle,uiInputs,uiOutputs);
    GAPBondMgr_GetParameter( GAPBOND_DEFAULT_PASSCODE,&passcode);
    LOG("passcode %d\n",passcode);
//    GAP_PasscodeUpdate(passcode,connectionHandle);
    GAPBondMgr_PasscodeRsp(connectionHandle,SUCCESS,passcode);
}
void multiRolePairStateCB( uint16 connHandle, uint8 state, uint8 status )
{
    LOG("PairStateCB handle 0x%02X,status %d,state 0x%X\n",connHandle,status,state);

    if ( state == GAPBOND_PAIRING_STATE_STARTED )
    {
        LOG( "Pairing started connHandle %d\n",connHandle);
    }
    else if (( state == GAPBOND_PAIRING_STATE_COMPLETE ) || ( state == GAPBOND_PAIRING_STATE_BONDED ) )
    {
        if ( status == SUCCESS )
        {
            #if( MAX_CONNECTION_MASTER_NUM > 0 )
            LOG( "Pairing & Bonding success,connHandle %d , and start SDP \n",connHandle);
            GAPMultiRole_CentralSDP_t sdp_node;
            osal_memset(&sdp_node, 0,sizeof(GAPMultiRole_CentralSDP_t));
            sdp_node.connHandle = connHandle;
            multiListInsertTail(MULTI_CENTRAL_SDP_MODE,(void**)&g_centralSDPlist,&sdp_node);
            #endif
            #if( MAX_CONNECTION_SLAVE_NUM > 0 )
            #endif
        }
        else
        {
            LOG( "Pairing fail connHandle %d\n",connHandle);
        }
    }
}

static void MultiPeriodProcessEvent(void)
{
//  LOG("%s\n",__func__);
    multitimer_ticks( MULTI_PERIOD_TIMING );
    multitimer_loop();
}

#if( MAX_CONNECTION_SLAVE_NUM > 0 )

static void Multi_peripheralUpdateParam(uint16 idx)
{
    LOG("%s idx %d\n",__func__,idx);
    MultiRole_PeripheralstartConnUpdate(idx,paramUpdateNoSuccessOption );
}

static void MultiRole_PeripheralstartConnUpdate( uint8 idx,uint8 handleFailure )
{
    LOG("%s idx %d\n",__func__,idx);
    l2capParamUpdateReq_t updateReq;
    uint16 timeout = GAP_GetParamValue( TGAP_CONN_PARAM_TIMEOUT );
    uint16 desired_min_interval;
    uint16 desired_max_interval;
    uint16 desired_slave_latency;
    uint16 desired_conn_timeout ;
    GAPMultiRole_GetParameter( GAPMULTIROLE_MIN_CONN_INTERVAL, &desired_min_interval );
    GAPMultiRole_GetParameter( GAPMULTIROLE_MAX_CONN_INTERVAL, &desired_max_interval );
    GAPMultiRole_GetParameter( GAPMULTIROLE_SLAVE_LATENCY,&desired_slave_latency );
    GAPMultiRole_GetParameter( GAPMULTIROLE_TIMEOUT_MULTIPLIER, &desired_conn_timeout );
    updateReq.intervalMin = desired_min_interval;
    updateReq.intervalMax = desired_max_interval;
    updateReq.slaveLatency = desired_slave_latency;
    updateReq.timeoutMultiplier = desired_conn_timeout;
    uint16 connHandle = multiLinkStatusGetSlaveConnHandle( idx );
    LOG("idx %d,connHandle %d\n",idx,connHandle);
    L2CAP_ConnParamUpdateReq( connHandle, &updateReq, gapMultiRole_TaskID );
    multitimer_init(&g_pcu_no_success_timer[idx], MultiRole_HandleParamUpdateNoSuccess, \
                    timeout, 0,idx);
    multitimer_start(&g_pcu_no_success_timer[idx]);
}

static void MultiRole_HandleParamUpdateNoSuccess( uint16 idx )
{
    LOG("%s\n",__func__);

    // See which option was choosen for unsuccessful updates
    switch ( paramUpdateNoSuccessOption )
    {
    case MULTIROLE_RESEND_PARAM_UPDATE:
        Multi_peripheralUpdateParam( idx );
        break;

    case MULTIROLE_TERMINATE_LINK:
        break;

    case MULTIROLE_NO_ACTION:

    // fall through
    default:
        //do nothing
        break;
    }
}
#endif

static void MultiRole_ProcessParamUpdateInfo(gapLinkUpdateEvent_t* pPkt)
{
    if ( pPkt->hdr.status == SUCCESS )
    {
        LOG("handle %d,Intv %d,latency %d,TO %d\n",     pPkt->connectionHandle,\
            pPkt->connInterval,\
            pPkt->connLatency,\
            pPkt->connTimeout);
        #if( MAX_CONNECTION_SLAVE_NUM > 0 )
        // stop no success timer
        uint8 perIdx = multiLinkConnParamUpdate( pPkt );

        if( perIdx < MAX_CONNECTION_SLAVE_NUM )
            multitimer_stop(&g_pcu_no_success_timer[ perIdx ]);

        #endif
        #if( MAX_CONNECTION_MASTER_NUM > 0 )
        // for multi-role as master not support parameter update
        GAPMultiRoleLinkCtrl_t* LinkInfoTmp = g_multiLinkInfo;

        while( LinkInfoTmp != NULL )
        {
            if( ( Master_Role == LinkInfoTmp->RoleState ) && (( LinkInfoTmp->connectionHandle & 0x0fff) == pPkt->connectionHandle ))
            {
                uint8 bondret = GAPBondMgr_LinkEst( LinkInfoTmp->peerDevAddrType,LinkInfoTmp->peerDevAddr, pPkt->connectionHandle, GAP_PROFILE_CENTRAL );
                break;
            }

            LinkInfoTmp = LinkInfoTmp->next;
        }

        #endif
    }
    else
    {
        LOG("ParamUpdateInfo error status %d\n",pPkt->hdr.status);
    }
}


#if( MAX_CONNECTION_MASTER_NUM > 0 )
/**
    @brief   Start a device discovery scan.

    Public function defined in central.h.
*/
bStatus_t GAPMultiRole_StartDiscovery(uint8 mode, uint8 activeScan, uint8 whiteList)
{
    gapDevDiscReq_t params;
    params.taskID = gapMultiRole_TaskID;
    params.mode = mode;
    params.activeScan = activeScan;
    params.whiteList = whiteList;
    return GAP_DeviceDiscoveryRequest(&params);
}

/**
    @brief   Cancel a device discovery scan.

    Public function defined in central.h.
*/
bStatus_t GAPMultiRole_CancelDiscovery(void)
{
    return GAP_DeviceDiscoveryCancel(gapMultiRole_TaskID);
}


void multiGetScanStrategy(uint8* param,uint8 len)
{
    if( len == 3 )
    {
        param[0] = g_multiRoleParam.scan.scanMode;
        param[1] = g_multiRoleParam.scan.activeScan;
        param[2] = g_multiRoleParam.scan.whitelist;
    }
}

void multiGetLinkStrategy(uint8* param,uint8 len)
{
    if( len == 2 )
    {
        param[0] = g_multiRoleParam.link.highDutyCycle;
        param[1] = g_multiRoleParam.link.whitelist;
    }
}

void multi_ScannerInsertDev(gapDeviceInfoEvent_t* pMsg)
{
    GAPMultiRolScanner_t* node = (GAPMultiRolScanner_t*)multiList_inside(MULTI_SCAN_MODE,(void**)&g_scanlist, pMsg->addr);

    if( node )
    {
        // already in g_scanlist
        // update rssi
        node->rssi = pMsg->rssi;

        // TODO: there's osal_mem_free will result memory leak ?
        if( pMsg->eventType == LL_ADV_RPT_SCAN_RSP )
        {
            // update scanner scan response data
            if( node->rspData )
                osal_mem_free( node->rspData );

            node->scanRsplen = pMsg->dataLen;
            node->rspData = osal_mem_alloc( node->scanRsplen );

            if( node->rspData )
            {
                osal_memset(node->rspData, 0, node->scanRsplen );
                osal_memcpy( node->rspData,pMsg->pEvtData,node->scanRsplen);
            }
        }
        else
        {
            // update scanner advertising data
            if( node->advData )
                osal_mem_free( node->advData );

            node->advDatalen = pMsg->dataLen;
            node->advData = osal_mem_alloc( node->advDatalen );

            if( node->advData )
            {
                osal_memset(node->advData, 0, node->advDatalen );
                osal_memcpy( node->advData,pMsg->pEvtData,node->advDatalen);
            }
        }
    }
    else
    {
        GAPMultiRolScanner_t pVnode;
        osal_memset( &pVnode,0,sizeof(GAPMultiRolScanner_t));
        pVnode.rssi = pMsg->rssi;
        pVnode.addrtype = pMsg->addrType;
        osal_memcpy(pVnode.addr, pMsg->addr,B_ADDR_LEN);

        // for ADV_IND, shall not necessary ,
        // consider another type advertising
        if( pMsg->eventType == LL_ADV_RPT_SCAN_RSP )
        {
            pVnode.scanRsplen = pMsg->dataLen;
            pVnode.rspData = osal_mem_alloc( pVnode.scanRsplen );

            if( pVnode.rspData )
            {
                osal_memset(pVnode.rspData, 0, pVnode.scanRsplen );
                osal_memcpy( pVnode.rspData,pMsg->pEvtData,pVnode.scanRsplen);
            }
        }
        else
        {
            pVnode.advDatalen = pMsg->dataLen;
            pVnode.advData = osal_mem_alloc( pVnode.advDatalen );

            if( pVnode.advData )
            {
                osal_memset(pVnode.advData, 0, pVnode.advDatalen );
                osal_memcpy( pVnode.advData,pMsg->pEvtData,pVnode.advDatalen);
            }
        }

        multiListInsertTail(MULTI_SCAN_MODE,(void**)&g_scanlist,&pVnode);
    }
}

uint8 multiConfigSlaveList( uint8 addrType,uint8* addr)
{
    uint8 resault = FALSE;

    if( NULL == multiList_inside(MULTI_CONFIG_SLAVE_DEV_MODE,(void**)&g_cfgSlaveList,addr) )
    {
        GAPMultiRoleCentralDev_t node;
        osal_memset(&node,0,sizeof(GAPMultiRoleCentralDev_t));
        node.next = NULL;
        node.addrType = addrType;
        osal_memcpy(node.addr,addr,B_ADDR_LEN );
        resault = multiListInsertTail(MULTI_CONFIG_SLAVE_DEV_MODE,(void**)&g_cfgSlaveList,&node);
    }

    return resault;
}
uint8 multiClearSlaveList( uint8 addrType,uint8* addr)
{
    if( multiList_inside(MULTI_CONFIG_SLAVE_DEV_MODE,(void**)&g_cfgSlaveList,addr) )
    {
        multiListDelNode(MULTI_CONFIG_SLAVE_DEV_MODE,(void**)&g_cfgSlaveList,addr);
    }

    return TRUE;
}

void multiClearAllSlaveList(void)
{
    GAPMultiRoleCentralDev_t* node = g_cfgSlaveList;

    while( node != NULL )
    {
        g_cfgSlaveList = node->next;
        osal_mem_free( node );
        node = g_cfgSlaveList;
    }
}

GAPMultiRoleCentralDev_t* multiGetConfigSlaveList(void)
{
    return g_cfgSlaveList;
}

uint8 multiDevInConfSlaveList(uint8* addr)
{
    uint8 inFlag = FALSE;
    GAPMultiRoleCentralDev_t* node = (GAPMultiRoleCentralDev_t*)multiList_inside(MULTI_CONFIG_SLAVE_DEV_MODE,(void**)&g_cfgSlaveList,addr);

    if( node )
        inFlag = TRUE;

    return inFlag;
}

void multiAddSlaveConnList( uint8 addrType,uint8* addr)
{
    GAPMultiRoleCentralDev_t node;
    osal_memset(&node,0,sizeof(GAPMultiRoleCentralDev_t));
    node.next = NULL;
    node.addrType = addrType;
    osal_memcpy(node.addr,addr,B_ADDR_LEN );
    multiListInsertTail(MULTI_CONFIG_MASTER_LINKING_MODE,(void**)&g_mlinkingList,&node);
}

void multiDelCurrentConnNode(void)
{
    // use first node
    if( g_mlinkingList )
    {
        GAPMultiRoleCentralDev_t* node = g_mlinkingList->next;
        osal_memset(g_mlinkingList,0,sizeof(GAPMultiRoleCentralDev_t));
        osal_mem_free(g_mlinkingList);

        if( node == NULL )
        {
            // only one node
            g_mlinkingList = NULL;
        }
        else
        {
            g_mlinkingList = node;
        }
    }
}

uint8 multi_devInLinkList(uint8* addr)
{
    uint8 inFlag = FALSE;
    GAPMultiRoleCentralDev_t* node = multiList_inside(MULTI_CONFIG_MASTER_LINKING_MODE,(void**)&g_mlinkingList,addr);

    if( node )
        inFlag = TRUE;

    return inFlag;
}

GAPMultiRoleCentralDev_t* multiGetSlaveConnList(void)
{
    return g_mlinkingList;
}

uint8 MultiRole_CancelConn(void)
{
    uint8 status = GAPMultiRole_TerminateConnection(GAP_CONNHANDLE_INIT);
    return SUCCESS;
}

static void Multi_centralAction(uint16 idx)
{
    multiTimer* atim_node = multiList_inside(MULTI_CENTRAL_ACTION_TIMER_MODE,(void**)&g_centralActionTimer,&idx);
    GAPMultiRoleCentralAction_t* action_node =  multiList_inside(MULTI_CONFIG_MASTER_ACTION_MODE,(void**)&g_centralAction,&idx);

    if( atim_node && action_node )
    {
        if( action_node->busy )
            return;

        if( action_node->action & GAPMULTI_CENTRAL_MTU_EXCHANGE )
        {
            ATT_SetMTUSizeMax( DEFAULT_EXCHANGE_MTU_LEN );
            attExchangeMTUReq_t pReq;
            pReq.clientRxMTU = DEFAULT_EXCHANGE_MTU_LEN;
            uint8 status =GATT_ExchangeMTU( idx,&pReq, gapMultiRole_TaskID );

            if( status == SUCCESS )
            {
                action_node->busy = TRUE;
                action_node->action &= ~GAPMULTI_CENTRAL_MTU_EXCHANGE;
            }
        }
        else if( action_node->action & GAPMULTI_CENTRAL_DLE_EXCHANGE  )
        {
            uint8 status = HCI_LE_SetDataLengthCmd(idx,251, 2120);

            if( status == SUCCESS )
            {
                action_node->action &= ~GAPMULTI_CENTRAL_DLE_EXCHANGE;
            }
        }
        else if( action_node->action & GAPMULTI_CENTRAL_SDP  )
        {
            GAPMultiRole_CentralSDP_t* sdp_node =  multiList_inside(MULTI_CENTRAL_SDP_MODE,(void**)&g_centralSDPlist,&idx);

            if( sdp_node )
            {
                if( sdp_node->state == DISC_STATE_IDLE )
                {
                    bStatus_t ret = GATT_DiscAllPrimaryServices(idx,gapMultiRole_TaskID);

                    if( ret == SUCCESS  )
                    {
                        action_node->busy = TRUE;
                        sdp_node->state = DISC_STATE_SVC;
                    }
                }
            }
        }
    }
}

static void  MultiRole_ProcessSDPInfo(gattMsgEvent_t* pMsg)
{
    GAPMultiRole_CentralSDP_t* sdp_node =  multiList_inside(MULTI_CENTRAL_SDP_MODE,(void**)&g_centralSDPlist,&pMsg->connHandle);

    if( sdp_node )
    {
        switch ( pMsg->method )
        {
        case ATT_READ_BY_GRP_TYPE_RSP:
        {
            if( pMsg->msg.readByGrpTypeRsp.numGrps > 0 )
            {
                for(uint8 i = 0 ; i < pMsg->msg.readByGrpTypeRsp.numGrps; i++)
                {
                    GATTReadGroupRsp primserv_node;
                    primserv_node.StHandle = BUILD_UINT16(   pMsg->msg.readByGrpTypeRsp.dataList[pMsg->msg.readByGrpTypeRsp.len * i], \
                                                             pMsg->msg.readByGrpTypeRsp.dataList[pMsg->msg.readByGrpTypeRsp.len * i + 1]);
                    primserv_node.EGHandle = BUILD_UINT16(   pMsg->msg.readByGrpTypeRsp.dataList[pMsg->msg.readByGrpTypeRsp.len * i+2], \
                                                             pMsg->msg.readByGrpTypeRsp.dataList[pMsg->msg.readByGrpTypeRsp.len * i + 3]);
                    primserv_node.uuid_Len = pMsg->msg.readByGrpTypeRsp.len - 4;
                    osal_memcpy(primserv_node.uuid,\
                                &(pMsg->msg.readByGrpTypeRsp.dataList[pMsg->msg.readByGrpTypeRsp.len * i + 4]),\
                                primserv_node.uuid_Len);
                    multiListInsertTail(MULTI_CENTRAL_SDP_PRIMSERV_MODE,(void**)&sdp_node->service.PrimServ,&primserv_node);
                }
            }
            else if( pMsg->hdr.status == bleProcedureComplete )
            {
                GATT_DiscAllChars(  pMsg->connHandle,sdp_node->service.PrimServ->StHandle,\
                                    sdp_node->service.PrimServ->EGHandle,gapMultiRole_TaskID );
            }
            else if( pMsg->method == ATT_ERROR_RSP )
            {
                LOG("SDP primary service error connHandle %d\n",pMsg->connHandle);
            }
        }
        break;

        case ATT_READ_BY_TYPE_RSP:
        {
            if ( pMsg->msg.readByTypeRsp.numPairs > 0 )
            {
                for(uint8 i = 0; i < pMsg->msg.readByTypeRsp.numPairs ; i++)
                {
                    Characteristic char_node;
                    char_node.charHandle = BUILD_UINT16(   pMsg->msg.readByTypeRsp.dataList[pMsg->msg.readByTypeRsp.len * i], \
                                                           pMsg->msg.readByTypeRsp.dataList[pMsg->msg.readByTypeRsp.len * i + 1]);
                    char_node.Properties = pMsg->msg.readByTypeRsp.dataList[pMsg->msg.readByTypeRsp.len * i + 2];
                    char_node.valueHandle = BUILD_UINT16(   pMsg->msg.readByTypeRsp.dataList[pMsg->msg.readByTypeRsp.len * i+3], \
                                                            pMsg->msg.readByTypeRsp.dataList[pMsg->msg.readByTypeRsp.len * i + 4]);
                    char_node.uuid_Len = pMsg->msg.readByTypeRsp.len - 5;
                    osal_memcpy(char_node.uuid,&(pMsg->msg.readByTypeRsp.dataList[pMsg->msg.readByTypeRsp.len * i + 5]),char_node.uuid_Len);
                    multiListInsertTail(MULTI_CENTRAL_SDP_CHARAC_MODE,(void**)&sdp_node->service.Charac,&char_node);
                    sdp_node->sdpCharHandle = char_node.valueHandle;
                }
            }
            else if( pMsg->hdr.status == bleProcedureComplete )
            {
                GATTReadGroupRsp* next_prim_node = multiList_inside(MULTI_CENTRAL_SDP_PRIMSERV_MODE,(void**)&sdp_node->service.PrimServ, &sdp_node->sdpCharHandle);

                if( next_prim_node )
                {
                    GATT_DiscAllChars(  pMsg->connHandle,next_prim_node->StHandle,\
                                        next_prim_node->EGHandle,gapMultiRole_TaskID );
                }
//                  else if( sdp_node->sdpCharHandle < 0xFFFF)
                else
                {
                    LOG("All Characteristic Discover Success pMsg->connHandle %d\n",pMsg->connHandle);

                    if( pGapRoles_AppCGs && pGapRoles_AppCGs->SDPNotify )
                        pGapRoles_AppCGs->SDPNotify( sdp_node );

                    multilist_free_central_action_actimerlist( pMsg->connHandle );
                    multilist_free_sdplist(pMsg->connHandle);
                }
            }
            else if( pMsg->method == ATT_ERROR_RSP )
            {
                LOG("SDP Characteristic error connHandle %d\n",pMsg->connHandle);
            }
        }

        default:
            break;
        }
    }
}

static void multilist_free_central_action_actimerlist( uint16 connHandle )
{
    multiListDelNode( MULTI_CONFIG_MASTER_ACTION_MODE, (void**)&g_centralAction,&connHandle );
    //
    multiTimer* entry = multiList_inside(MULTI_CENTRAL_ACTION_TIMER_MODE,(void**)&g_centralActionTimer, &connHandle );
    multitimer_stop( entry );
    multiListDelNode( MULTI_CENTRAL_ACTION_TIMER_MODE, (void**)&g_centralActionTimer,&connHandle );
}

static void multilist_free_sdplist(uint16 connHandle)
{
    multiListDelNode( MULTI_CENTRAL_SDP_MODE, (void**)&g_centralSDPlist,&connHandle);
}

static void* multiListCreate(GAPMultiListMode_t mode )
{
    void* node = NULL ;

    switch( mode )
    {
    case MULTI_SCAN_MODE:
    {
        GAPMultiRolScanner_t* tnode = (GAPMultiRolScanner_t*)osal_mem_alloc( sizeof( GAPMultiRolScanner_t ) );

        if( tnode )
        {
            osal_memset( tnode, 0, sizeof( sizeof( GAPMultiRolScanner_t ) ) );
            // TODO: tnode->next can not clear to zero after osal_memset, why?
            tnode->next = NULL;
        }

        node = (GAPMultiRolScanner_t*)tnode;
    }
    break;

    case MULTI_CONFIG_SLAVE_DEV_MODE:
    case MULTI_CONFIG_MASTER_LINKING_MODE:
    {
        GAPMultiRoleCentralDev_t* tnode = (GAPMultiRoleCentralDev_t*)osal_mem_alloc( sizeof( GAPMultiRoleCentralDev_t ) );

        if( tnode )
        {
            osal_memset( tnode, 0, sizeof( sizeof( GAPMultiRoleCentralDev_t ) ) );
            // TODO: tnode->next can not clear to zero after osal_memset, why?
            tnode->next = NULL;
        }

        node = (GAPMultiRoleCentralDev_t*)tnode;
    }
    break;

    case MULTI_CONFIG_MASTER_ACTION_MODE:
    {
        GAPMultiRoleCentralAction_t* tnode = (GAPMultiRoleCentralAction_t*)osal_mem_alloc( sizeof( GAPMultiRoleCentralAction_t ) );

        if( tnode )
        {
            osal_memset( tnode, 0, sizeof( sizeof( GAPMultiRoleCentralAction_t ) ) );
            // TODO: tnode->next can not clear to zero after osal_memset, why?
            tnode->next = NULL;
        }

        node = (GAPMultiRoleCentralAction_t*)tnode;
    }
    break;

    case MULTI_CENTRAL_ACTION_TIMER_MODE:
    {
        multiTimer* tnode = (multiTimer*)osal_mem_alloc( sizeof( multiTimer ) );

        if( tnode )
        {
            osal_memset( tnode, 0, sizeof( sizeof( multiTimer ) ) );
            // TODO: tnode->next can not clear to zero after osal_memset, why?
            tnode->next = NULL;
        }

        node = (multiTimer*)tnode;
    }
    break;

    case MULTI_CENTRAL_SDP_MODE:
    {
        GAPMultiRole_CentralSDP_t* tnode = (GAPMultiRole_CentralSDP_t*)osal_mem_alloc( sizeof( GAPMultiRole_CentralSDP_t ) );

        if( tnode )
        {
            osal_memset( tnode, 0, sizeof( sizeof( GAPMultiRole_CentralSDP_t ) ) );
            // TODO: tnode->next can not clear to zero after osal_memset, why?
            tnode->next = NULL;
        }

        node = (GAPMultiRole_CentralSDP_t*)tnode;
    }
    break;

    case MULTI_CENTRAL_SDP_PRIMSERV_MODE:
    {
        GATTReadGroupRsp* tnode = (GATTReadGroupRsp*)osal_mem_alloc( sizeof( GATTReadGroupRsp ) );

        if( tnode )
        {
            osal_memset( tnode, 0, sizeof( sizeof( GATTReadGroupRsp ) ) );
            // TODO: tnode->next can not clear to zero after osal_memset, why?
            tnode->next = NULL;
        }

        node = (GATTReadGroupRsp*)tnode;
    }
    break;

    case MULTI_CENTRAL_SDP_CHARAC_MODE:
    {
        Characteristic* tnode = (Characteristic*)osal_mem_alloc( sizeof( Characteristic ) );

        if( tnode )
        {
            osal_memset( tnode, 0, sizeof( sizeof( Characteristic ) ) );
            // TODO: tnode->next can not clear to zero after osal_memset, why?
            tnode->next = NULL;
        }

        node = (Characteristic*)tnode;
    }
    break;

    default:
        break;
    }

    return node;
}

static void* multiListFindTail(GAPMultiListMode_t mode, void** ppnode)
{
    // TODO : ppnode shall not be NULL ,
    void* node = NULL ;

    switch( mode )
    {
    case MULTI_SCAN_MODE:
    {
        GAPMultiRolScanner_t* entry = *(GAPMultiRolScanner_t**)ppnode;

        while( entry )
        {
            if(entry->next == NULL )
                break;

            entry = entry->next;
        }

        node = entry;
    }
    break;

    case MULTI_CONFIG_SLAVE_DEV_MODE:
    case MULTI_CONFIG_MASTER_LINKING_MODE:
    {
        GAPMultiRoleCentralDev_t* entry = *(GAPMultiRoleCentralDev_t**)ppnode;

        while( entry )
        {
            if(entry->next == NULL )
                break;

            entry = entry->next;
        }

        node = entry;
    }
    break;

    case MULTI_CONFIG_MASTER_ACTION_MODE:
    {
        GAPMultiRoleCentralAction_t* entry = *(GAPMultiRoleCentralAction_t**)ppnode;

        while( entry )
        {
            if(entry->next == NULL )
                break;

            entry = entry->next;
        }

        node = entry;
    }
    break;

    case MULTI_CENTRAL_ACTION_TIMER_MODE:
    {
        multiTimer* entry = *(multiTimer**)ppnode;

        while( entry )
        {
            if(entry->next == NULL )
                break;

            entry = entry->next;
        }

        node = entry;
    }
    break;

    case MULTI_CENTRAL_SDP_MODE:
    {
        GAPMultiRole_CentralSDP_t* entry = *(GAPMultiRole_CentralSDP_t**)ppnode;

        while( entry )
        {
            if(entry->next == NULL )
                break;

            entry = entry->next;
        }

        node = entry;
    }
    break;

    case MULTI_CENTRAL_SDP_PRIMSERV_MODE:
    {
        GATTReadGroupRsp* entry = *(GATTReadGroupRsp**)ppnode;

        while( entry )
        {
            if(entry->next == NULL )
                break;

            entry = entry->next;
        }

        node = entry;
    }
    break;

    case MULTI_CENTRAL_SDP_CHARAC_MODE:
    {
        Characteristic* entry = *(Characteristic**)ppnode;

        while( entry )
        {
            if(entry->next == NULL )
                break;

            entry = entry->next;
        }

        node = entry;
    }
    break;

    default:
        break;
    }

    return node;
}

static void multiListDelNode(GAPMultiListMode_t mode, void** ppnode, void* pvalue )
{
    switch( mode )
    {
    case MULTI_CONFIG_SLAVE_DEV_MODE:
    {
        for( GAPMultiRoleCentralDev_t** curr = (GAPMultiRoleCentralDev_t**)ppnode; *curr;)
        {
            GAPMultiRoleCentralDev_t* entry = *curr;

            if( osal_memcmp(entry->addr,(uint8*)pvalue,B_ADDR_LEN) == TRUE )
            {
                *curr = entry->next;
                osal_mem_free( entry );
                break;
            }
            else
                curr = &entry->next;
        }
    }
    break;

    case MULTI_CONFIG_MASTER_ACTION_MODE:
    {
        for( GAPMultiRoleCentralAction_t** curr = (GAPMultiRoleCentralAction_t**)ppnode; *curr;)
        {
            GAPMultiRoleCentralAction_t* entry = *curr;

            if( entry == NULL )
                break;

            if( entry->connHandle == *(uint16*)pvalue )
            {
                *curr = entry->next;
                osal_mem_free( entry );
                break;
            }
            else
                curr = &entry->next;
        }
    }
    break;

    case MULTI_CENTRAL_ACTION_TIMER_MODE:
    {
        for( multiTimer** curr = (multiTimer**)ppnode; *curr;)
        {
            multiTimer* entry = *curr;

            if( entry == NULL )
                break;

            if(  entry->id == *(uint16*)pvalue )
            {
                *curr = entry->next;
                osal_mem_free( entry );
                break;
            }
            else
                curr = &entry->next;
        }
    }
    break;

    case MULTI_CENTRAL_SDP_MODE:
    {
        for( GAPMultiRole_CentralSDP_t** curr = (GAPMultiRole_CentralSDP_t**)ppnode; *curr;)
        {
            GAPMultiRole_CentralSDP_t* entry = *curr;

            if( entry == NULL )
                break;

            if( entry->connHandle == *(uint16*)pvalue )
            {
                *curr = entry->next;
                GATTReadGroupRsp* primservice = entry->service.PrimServ;

                while( primservice )
                {
                    osal_mem_free( primservice );
                    primservice = primservice->next;
                }

                Characteristic* charac = entry->service.Charac;

                while( charac )
                {
                    osal_mem_free( charac );
                    charac = charac->next;
                }

                osal_mem_free( entry );
                break;
            }
            else
                curr = &entry->next;
        }
    }
    break;

    default:
        break;
    }
}

static uint8 multiListInsertTail(GAPMultiListMode_t mode, void** ppnode,void* vnode)
{
    volatile uint8 ret = FALSE;

    switch( mode )
    {
    case MULTI_SCAN_MODE:
    {
        GAPMultiRolScanner_t** curr = (GAPMultiRolScanner_t**)ppnode;
        GAPMultiRolScanner_t* new_node = multiListCreate( mode );

        if( new_node )
        {
            osal_memcpy( new_node, (GAPMultiRolScanner_t*)vnode, sizeof( GAPMultiRolScanner_t ) );
            new_node->next = NULL;
            GAPMultiRolScanner_t* entry = multiListFindTail(mode,(void**)ppnode);

            if( entry == NULL )
            {
                *curr = new_node;
            }
            else
            {
                entry->next = new_node;
            }

            ret = TRUE;
        }
    }
    break;

    case MULTI_CONFIG_SLAVE_DEV_MODE:
    case MULTI_CONFIG_MASTER_LINKING_MODE:
    {
        GAPMultiRoleCentralDev_t** curr = (GAPMultiRoleCentralDev_t**)ppnode;
        GAPMultiRoleCentralDev_t* new_node = multiListCreate( mode );

        if( new_node )
        {
            osal_memcpy( new_node, (GAPMultiRoleCentralDev_t*)vnode, sizeof( GAPMultiRoleCentralDev_t ) );
            new_node->next = NULL;
            GAPMultiRoleCentralDev_t* entry = multiListFindTail(mode,(void**)ppnode);

            if( entry == NULL )
            {
                *curr = new_node;
            }
            else
            {
                entry->next = new_node;
            }

            ret = TRUE;
        }
    }
    break;

    case MULTI_CONFIG_MASTER_ACTION_MODE:
    {
        GAPMultiRoleCentralAction_t** curr = (GAPMultiRoleCentralAction_t**)ppnode;
        GAPMultiRoleCentralAction_t* new_node = multiListCreate( mode );

        if( new_node )
        {
            osal_memcpy( new_node, (GAPMultiRoleCentralAction_t*)vnode, sizeof( GAPMultiRoleCentralAction_t ) );
            new_node->next = NULL;
            GAPMultiRoleCentralAction_t* entry = multiListFindTail(mode,(void**)ppnode);

            if( entry == NULL )
            {
                *curr = new_node;
            }
            else
            {
                entry->next = new_node;
            }

            ret = TRUE;
        }
    }
    break;

    case MULTI_CENTRAL_ACTION_TIMER_MODE:
    {
        multiTimer** curr = (multiTimer**)ppnode;
        multiTimer* new_node = multiListCreate( mode );

        if( new_node )
        {
            osal_memcpy( new_node, (multiTimer*)vnode, sizeof( multiTimer ) );
            new_node->next = NULL;
            multiTimer* entry = multiListFindTail(mode,(void**)ppnode);

            if( entry == NULL )
            {
                *curr = new_node;
            }
            else
            {
                entry->next = new_node;
            }

            ret = TRUE;
        }
    }
    break;

    case MULTI_CENTRAL_SDP_MODE:
    {
        GAPMultiRole_CentralSDP_t** curr = (GAPMultiRole_CentralSDP_t**)ppnode;
        GAPMultiRole_CentralSDP_t* new_node = multiListCreate( mode );

        if( new_node )
        {
            osal_memcpy( new_node, (GAPMultiRole_CentralSDP_t*)vnode, sizeof( GAPMultiRole_CentralSDP_t ) );
            new_node->next = NULL;
            GAPMultiRole_CentralSDP_t* entry = multiListFindTail(mode,(void**)ppnode);

            if( entry == NULL )
            {
                *curr = new_node;
            }
            else
            {
                entry->next = new_node;
            }

            ret = TRUE;
        }
    }
    break;

    case MULTI_CENTRAL_SDP_PRIMSERV_MODE:
    {
        GATTReadGroupRsp** curr = (GATTReadGroupRsp**)ppnode;
        GATTReadGroupRsp* new_node = multiListCreate( mode );

        if( new_node )
        {
            osal_memcpy( new_node, (GATTReadGroupRsp*)vnode, sizeof( GATTReadGroupRsp ) );
            new_node->next = NULL;
            GATTReadGroupRsp* entry = multiListFindTail(mode,(void**)ppnode);

            if( entry == NULL )
            {
                *curr = new_node;
            }
            else
            {
                entry->next = new_node;
            }

            ret = TRUE;
        }
    }
    break;

    case MULTI_CENTRAL_SDP_CHARAC_MODE:
    {
        Characteristic** curr = (Characteristic**)ppnode;
        Characteristic* new_node = multiListCreate( mode );

        if( new_node )
        {
            osal_memcpy( new_node, (Characteristic*)vnode, sizeof( Characteristic ) );
            new_node->next = NULL;
            Characteristic* entry = multiListFindTail(mode,(void**)ppnode);

            if( entry == NULL )
            {
                *curr = new_node;
            }
            else
            {
                entry->next = new_node;
            }

            ret = TRUE;
        }
    }
    break;

    default:
        break;
    }

    return ret;
}

static void* multiList_inside(GAPMultiListMode_t mode,void** ppnode, void* pvalue)
{
    void* node = NULL;

    switch( mode )
    {
    case MULTI_SCAN_MODE:
    {
        GAPMultiRolScanner_t* entry = *(GAPMultiRolScanner_t**)ppnode;

        while( entry )
        {
            // scan mode, detect MAC addr
            if( osal_memcmp(entry->addr,(uint8*)pvalue,B_ADDR_LEN) == TRUE )
                break;

            entry = entry->next;
        }

        // not in scan list , tail node tnode = NULL
        node =  (GAPMultiRolScanner_t*)entry;
    }
    break;

    case MULTI_CONFIG_SLAVE_DEV_MODE:
    case MULTI_CONFIG_MASTER_LINKING_MODE:
    {
        GAPMultiRoleCentralDev_t* entry = *(GAPMultiRoleCentralDev_t**)ppnode;

        while( entry )
        {
            // scan mode, detect MAC addr
            if( osal_memcmp(entry->addr,(uint8*)pvalue,B_ADDR_LEN) == TRUE )
                break;

            entry = entry->next;
        }

        node =  (GAPMultiRoleCentralDev_t*)entry;
    }
    break;

    case MULTI_CONFIG_MASTER_ACTION_MODE:
    {
        GAPMultiRoleCentralAction_t* entry = *(GAPMultiRoleCentralAction_t**)ppnode;

        while( entry )
        {
            if( entry->connHandle== *(uint16*)pvalue )
                break;

            entry = entry->next;
        }

        node =  (GAPMultiRoleCentralAction_t*)entry;
    }
    break;

    case MULTI_CENTRAL_ACTION_TIMER_MODE:
    {
        multiTimer* entry = *(multiTimer**)ppnode;

        while( entry )
        {
            if( entry->id == *(uint8*)pvalue )
                break;

            entry = entry->next;
        }

        node =  (multiTimer*)entry;
    }
    break;

    case MULTI_CENTRAL_SDP_MODE:
    {
        GAPMultiRole_CentralSDP_t* entry = *(GAPMultiRole_CentralSDP_t**)ppnode;

        while( entry )
        {
            if( entry->connHandle == *(uint16*)pvalue )
                break;

            entry = entry->next;
        }

        node =  (GAPMultiRole_CentralSDP_t*)entry;
    }
    break;

    case MULTI_CENTRAL_SDP_PRIMSERV_MODE:
    {
        GATTReadGroupRsp* entry = *(GATTReadGroupRsp**)ppnode;

        while( entry )
        {
            if( entry->StHandle > *(uint16*)pvalue )
                break;

            entry = entry->next;
        }

        node =  (GATTReadGroupRsp*)entry;
    }
    break;

    default:
        node = (uint8*)NULL;
        break;
    }

    return node;
}

static void multiListMemoryFree(GAPMultiListMode_t mode )
{
    switch( mode )
    {
    case MULTI_SCAN_MODE:
    {
        #if( MAX_CONNECTION_MASTER_NUM > 0 )
        GAPMultiRolScanner_t* node = g_scanlist;

        while( node != NULL )
        {
            g_scanlist = node->next;

            if( node->advData != NULL )
                osal_mem_free(node->advData);

            if( node->rspData != NULL )
                osal_mem_free(node->rspData);

            osal_mem_free( node );
            node = g_scanlist;
        }

        #endif
    }
    break;

    default:
        break;
    }
}
#endif




