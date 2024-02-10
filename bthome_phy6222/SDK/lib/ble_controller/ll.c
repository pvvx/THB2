
/*******************************************************************************
    Filename:       ll.c, updated base on ll.h
    Revised:
    Revision:

    Description:    This file contains the Link Layer (LL) API for the Bluetooth
                  Low Energy (BLE) Controller. It provides the defines, types,
                  and functions for all supported Bluetooth Low Energy (BLE)
                  commands.

                  This API is based on the Bluetooth Core Specification,
                  V4.0.0, Vol. 6.

 SDK_LICENSE

*******************************************************************************/
//#define DEBUG_LL

/*******************************************************************************
    INCLUDES
*/
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "ll_buf.h"
#include "ll.h"
#include "ll_def.h"
#include "ll_common.h"
#include "ll_hw_drv.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "osal_bufmgr.h"
#include "bus_dev.h"
#include "jump_function.h"
#include "global_config.h"
#include "ll_debug.h"
#include "ll_enc.h"
#include "rf_phy_driver.h"
#include "time.h"

//#define OWN_PUBLIC_ADDR_POS      0x11004000

extern void clear_timer(AP_TIM_TypeDef* TIMx);
extern uint32_t get_timer_count(AP_TIM_TypeDef* TIMx);
/*******************************************************************************
    MACROS
*/
#define LL_COPY_DEV_ADDR_LE( dstPtr, srcPtr )        {                          \
        (dstPtr)[0] = (srcPtr)[0];                                                   \
        (dstPtr)[1] = (srcPtr)[1];                                                   \
        (dstPtr)[2] = (srcPtr)[2];                                                   \
        (dstPtr)[3] = (srcPtr)[3];                                                   \
        (dstPtr)[4] = (srcPtr)[4];                                                   \
        (dstPtr)[5] = (srcPtr)[5];}
// ALT: COULD USE OSAL COPY.
// osal_memcpy( (dstPtr), (srcPtr), LL_DEVICE_ADDR_LEN );

#define LL_COPY_DEV_ADDR_BE( dstPtr, srcPtr )         {                         \
        (dstPtr)[0] = (srcPtr)[5];                                                   \
        (dstPtr)[1] = (srcPtr)[4];                                                   \
        (dstPtr)[2] = (srcPtr)[3];                                                   \
        (dstPtr)[3] = (srcPtr)[2];                                                   \
        (dstPtr)[4] = (srcPtr)[1];                                                   \
        (dstPtr)[5] = (srcPtr)[0];}
// ALT: COULD USE OSAL COPY.
// osal_memcpy( (dstPtr), (srcPtr), LL_DEVICE_ADDR_LEN );

#define BDADDR_VALID( bdAddr )                                                 \
    ( !(                                                                         \
                                                                                 ((bdAddr)[0] == 0xFF) &&                                                \
                                                                                 ((bdAddr)[1] == 0xFF) &&                                                \
                                                                                 ((bdAddr)[2] == 0xFF) &&                                                \
                                                                                 ((bdAddr)[3] == 0xFF) &&                                                \
                                                                                 ((bdAddr)[4] == 0xFF) &&                                                \
                                                                                 ((bdAddr)[5] == 0xFF)                                                   \
       )                                                                         \
    )

// See "Supported States Related" below.
#define LL_SET_SUPPORTED_STATES( state )                                       \
    states[ (state)>>4 ] |= (1<<((state) & 0x0F));

/*******************************************************************************
    CONSTANTS
*/
// Bluetooth Version Information
#define LL_VERSION_NUM                0x09      // BT Core Specification V5.0.0, refer to https://www.bluetooth.com/specifications/assigned-numbers/host-controller-interface
#define LL_COMPANY_ID                 0x0504    // Phyplus

// Major Version (8 bits) . Minor Version (4 bits) . SubMinor Version (4 bits)
#define LL_SUBVERSION_NUM             0x0208    //  Controller v1.0.0. Definition not found in BLE spec

// Connection Window Information
#define LL_WINDOW_SIZE                2         // 2.5ms in 1.25ms ticks
#define LL_WINDOW_OFFSET              0         // 1.25ms + 0

/*******************************************************************************
    TYPEDEFS
*/

/*******************************************************************************
    LOCAL VARIABLES
*/

/*
** Own Device Address
**
** Note that the device may have a permanently assigned BLE address located in
** the Information Page. However, this address could be overridden with either
** the build flag BDADDR_FROM_FLASH, or by the vendor specific command
** LL_EXT_SET_BDADDR.
*/
// Own Device Public Address
uint8 ownPublicAddr[ LL_DEVICE_ADDR_LEN ];     // index 0..5 is LSO..MSB

// Own Device Random Address
uint8 ownRandomAddr[ LL_DEVICE_ADDR_LEN ];     // index 0..5 is LSO..MSB

// Delay Sleep
static uint16       sleepDelay;          // delay sleep for XOSC stabilization upon reset

syncInfo_t        syncInfo;

scannerSyncInfo_t    scanSyncInfo;

extern uint32 llWaitingIrq;
extern struct buf_rx_desc g_rx_adv_buf;

extern uint8    g_currentLocalRpa[LL_DEVICE_ADDR_LEN];
extern uint8    g_currentPeerRpa[LL_DEVICE_ADDR_LEN];
extern uint8    g_currentPeerAddrType;
extern uint8    g_currentLocalAddrType;


/*******************************************************************************
    GLOBAL VARIABLES
*/

advInfo_t           adv_param;       // Advertiser info
// add for scanner
scanInfo_t          scanInfo;        // Scanner info
// add for initiator 2018-05-09
initInfo_t          initInfo;        // Initiator info

extScanInfo_t       extScanInfo;     // extended Scanner info
extInitInfo_t       extInitInfo;     // extended initiator info

// BBB new: the memory of LL connect context is allocated by APP
llConnState_t*       conn_param      = NULL;
uint8               g_maxConnNum     = 0;
uint8               g_maxPktPerEventTx = TYP_CONN_BUF_LEN;
uint8               g_maxPktPerEventRx = TYP_CONN_BUF_LEN;
uint8               g_blePktVersion  = BLE_PKT_VERSION_4_0;

//move to llConnStat_t
//preChanMapUpdate_t preChanMapUpdate[MAX_NUM_LL_CONN];

llPeriodicScannerInfo_t    g_llPeriodAdvSyncInfo[MAX_NUM_LL_PRD_ADV_SYNC];

// A2 multi-connection
llConns_t           g_ll_conn_ctx;


uint8               LL_TaskID;          // OSAL LL task ID
uint8_t             llState;            // state of LL                            ==> to move to per connection context ???
peerInfo_t          peerInfo;           // peer device's address and address type   ==> to move to per connection context???
chanMap_t           chanMapUpdate;      // channel map for updates
featureSet_t        deviceFeatureSet;   // feature set for this device
verInfo_t           verInfo;            // own version information
//llConns_t         llConns;            // LL connections table
uint8              numComplPkts;        // number of completed Tx buffers, use global beacuse we report HCI event when numComplPkts >= numComplPktsLimit
uint8              numComplPktsLimit;   // minimum number of completed Tx buffers before event
rfCounters_t       rfCounters;          // counters for LL RX/TX atomic operation in one connection event

uint8              fastTxRespTime;      // flag indicates if fast TX response time feature is enabled/disabled

//  ======= add by HZF, for whitelist
peerInfo_t     g_llWhitelist[LL_WHITELIST_ENTRY_NUM]; // whitelist table
uint8          g_llWlDeviceNum;                     // current device number in whitelist list, should not exceed LL_WHITELIST_ENTRY_NUM

// ======= add by HZF, for resolving list
resolvingListInfo_t  g_llResolvinglist[LL_RESOLVINGLIST_ENTRY_NUM]; // resolving list table, size to be consider
uint8          g_llRlDeviceNum;                     // current device number in resolving list, should not exceed LL_RESOLVINGLIST_ENTRY_NUM
uint8          g_llRlEnable = FALSE;
uint16         g_llRlTimeout = 900;                 // Unit: second


// ============== A1 ROM metal change add
uint32_t       g_llHdcDirAdvTime;      // for HDC direct adv
//==

//uint8         numComplPktsLimit;    // minimum number of completed Tx buffers before event
//uint8         numComplPktsFlush;    // flag to indicate send number of completed buffers at end of event
//uint8         fastTxRespTime;     // flag indicates if fast TX response time feature is enabled/disabled
//==

// RX Flow Control
uint8        rxFifoFlowCtrl;

//llLinkBuf_t  ll_buf;

llGlobalStatistics_t g_pmCounters;             // TODO: to divide into per connection counters & global counters

// =====   A2 metal change add
llPduLenManagment_t  g_llPduLen;    //for dle feature       ==> to move to per connection context
//llPhyModeManagment_t g_llPhyModeCtrl;// for phy update      ==> to move to per connection context
uint8_t             llSecondaryState;            // secondary state of LL
// ===== A2 add End
extern l2capSARDbugCnt_t g_sarDbgCnt;

extern struct buf_tx_desc g_tx_adv_buf;
extern struct buf_tx_desc g_tx_ext_adv_buf;

extern struct buf_tx_desc tx_scanRsp_desc;

extern uint32   g_interAuxPduDuration;

// ============ extended advertisement variables
extAdvInfo_t*  g_pExtendedAdvInfo = NULL;     // extended adv information, note that periodic adv also need it
periodicAdvInfo_t*  g_pPeriodicAdvInfo = NULL;

// TODO: add periodic adv set array
uint8          g_extAdvNumber;           // number of ext adv set
uint8          g_perioAdvNumber;           // number of periodic adv set

uint16         g_advSetMaximumLen;       // maximum length of adv data

// extended adv scheduler context, move to a structure?
llAdvScheduleInfo_t*  g_pAdvSchInfo;
uint8          g_schExtAdvNum;              // current schedule extended adv number
uint8          g_currentExtAdv;             // current schedule extended adv index
uint32         g_advPerSlotTick;            // us
uint32         g_advSlotPeriodic;           // us
uint32         g_currentAdvTimer;           // us

// ==== periodic adv scheduler context
llPeriodicAdvScheduleInfo_t*  g_pAdvSchInfo_periodic;        // periodic adv scheduler info
uint8          g_schExtAdvNum_periodic;              // current scheduler periodic adv number
uint8          g_currentExtAdv_periodic;             // current scheduler periodic adv index


//
uint8          g_currentTimerTask;          // scan or adv
uint32         g_timerExpiryTick;           // us

// 2020-02-15 add for connectionless IQ Sample buffer
uint16* g_pLLcteISample=NULL;
uint16* g_pLLcteQSample=NULL;

// according to BQB test case HCI/GEV/BV-02-C & HCI/GEV/BV-03-C,
// mixed legacy advertisement and extended advertisement is disallowed
// mixed legacy scan and extended scan is disallowed
#define LL_MODE_INVALID       0xFF
#define LL_MODE_LEGACY        0x00
#define LL_MODE_EXTENDED      0x01
uint8  g_llScanMode = LL_MODE_INVALID;
uint8  g_llAdvMode =  LL_MODE_INVALID;


// RF path compensation
extern int16  g_rfTxPathCompensation, g_rfRxPathCompensation;

// periodic advertiser device list
periodicAdvertiserListInfo_t g_llPeriodicAdvlist[LL_PRD_ADV_ENTRY_NUM];
uint8          g_llPrdAdvDeviceNum;                     // current periodic advertiser device number

//   LOCAL FUNCTIONS
void llPrdAdvDecideNextChn(extAdvInfo_t*  pAdvInfo, periodicAdvInfo_t* pPrdAdv);
void llSetupSyncInfo(extAdvInfo_t*  pAdvInfo, periodicAdvInfo_t* pPrdAdv);



/*******************************************************************************
    @fn          LL_Init0

    @brief       This is the Link Layer task initialization called by OSAL. It
                must be called once when the software system is started and
                before any other function in the LL API is called.

    input parameters

    @param       taskId - Task identifier assigned by OSAL.

    output parameters

    @param       None.

    @return      None.
*/
void LL_Init0( uint8 taskId )
{
    #ifndef HOST
    uint8* p;
    #endif
    LL_TaskID = taskId;
    // set BLE version information
    verInfo.verNum    = LL_VERSION_NUM;
    verInfo.comId     = LL_COMPANY_ID;
    verInfo.subverNum = LL_SUBVERSION_NUM;
    // =========== calculate whiten seed
    calculate_whiten_seed(); // must set!!!
    #ifdef HOST
    // hardcoded address
    ownPublicAddr[0] = 0xF3;
    ownPublicAddr[1] = 0xE4;
    ownPublicAddr[2] = 0xa2;
    ownPublicAddr[3] = 0x98;
    ownPublicAddr[4] = 0x58;
    ownPublicAddr[5] = 0xdf;
    #else
    // read flash driectly becasue HW has do the address mapping for read Flash operation
    p = (uint8*)pGlobal_config[MAC_ADDRESS_LOC];
    ownPublicAddr[3] = *(p++);
    ownPublicAddr[2] = *(p++);
    ownPublicAddr[1] = *(p++);
    ownPublicAddr[0] = *(p++);
    ownPublicAddr[5] = *(p++);
    ownPublicAddr[4] = *(p);
//    ownPublicAddr[3] = ReadFlash(address ++);
//    ownPublicAddr[2] = ReadFlash(address ++);
//    ownPublicAddr[1] = ReadFlash(address ++);
//    ownPublicAddr[0] = ReadFlash(address ++);
//
//    ownPublicAddr[5] = ReadFlash(address ++);
//    ownPublicAddr[4] = ReadFlash(address);
    #endif
    // set own random address as invalid until one is provided
    ownRandomAddr[0] = 0xFF;
    ownRandomAddr[1] = 0xFF;
    ownRandomAddr[2] = 0xFF;
    ownRandomAddr[3] = 0xFF;
    ownRandomAddr[4] = 0xFF;
    ownRandomAddr[5] = 0xFF;
    adv_param.ownAddr[0]     = 0xFF;
    adv_param.ownAddr[1]     = 0xFF;
    adv_param.ownAddr[2]     = 0xFF;
    adv_param.ownAddr[3]     = 0xFF;
    adv_param.ownAddr[4]     = 0xFF;
    adv_param.ownAddr[5]     = 0xFF;
    adv_param.ownAddrType    = LL_DEV_ADDR_TYPE_PUBLIC;
    adv_param.advMode        = LL_ADV_MODE_OFF;
    adv_param.advInterval    = LL_ADV_INTERVAL_DEFAULT;
    adv_param.advEvtType     = LL_ADV_NONCONNECTABLE_UNDIRECTED_EVT;
    adv_param.advChanMap     = LL_ADV_CHAN_MAP_DEFAULT;            // correct by HZF, 2-23
    adv_param.wlPolicy       = LL_ADV_WL_POLICY_ANY_REQ;
    adv_param.scaValue       = LL_SCA_SLAVE_DEFAULT;
    adv_param.advDataLen     = 0;
    adv_param.scanRspLen     = 0;

    for (int i = 0; i < g_maxConnNum; i++)
    {
        conn_param[i].connId    = i;
        conn_param[i].active    = FALSE;
        conn_param[i].allocConn = FALSE;
        memset((uint8_t*)&conn_param[i].pmCounter, 0, sizeof(llLinkStatistics_t) );
    }

    // set default Scan values
    scanInfo.ownAddr[0]    = 0xFF;
    scanInfo.ownAddr[1]    = 0xFF;
    scanInfo.ownAddr[2]    = 0xFF;
    scanInfo.ownAddr[3]    = 0xFF;
    scanInfo.ownAddr[4]    = 0xFF;
    scanInfo.ownAddr[5]    = 0xFF;
    scanInfo.ownAddrType   = LL_DEV_ADDR_TYPE_PUBLIC;
    scanInfo.initPending   = TRUE;
    scanInfo.scanMode      = LL_SCAN_STOP;
    scanInfo.scanType      = LL_SCAN_PASSIVE;
    scanInfo.scanInterval  = LL_SCAN_INTERVAL_DEFAULT;
    scanInfo.scanWindow    = LL_SCAN_INTERVAL_DEFAULT;
    scanInfo.wlPolicy      = LL_SCAN_WL_POLICY_ANY_ADV_PKTS;
    scanInfo.filterReports = TRUE;
    scanInfo.scanBackoffUL = 1;
    scanInfo.nextScanChan  = LL_SCAN_ADV_CHAN_37;
    scanInfo.numSuccess    = 0;
    scanInfo.numFailure    = 0;
    // set default Init values
    initInfo.ownAddr[0]   = 0xFF;
    initInfo.ownAddr[1]   = 0xFF;
    initInfo.ownAddr[2]   = 0xFF;
    initInfo.ownAddr[3]   = 0xFF;
    initInfo.ownAddr[4]   = 0xFF;
    initInfo.ownAddr[5]   = 0xFF;
    initInfo.ownAddrType  = LL_DEV_ADDR_TYPE_PUBLIC;
    initInfo.initPending  = TRUE;
    initInfo.scanMode     = LL_SCAN_STOP;
    initInfo.scanInterval = LL_SCAN_INTERVAL_DEFAULT;
    initInfo.scanWindow   = LL_SCAN_INTERVAL_DEFAULT;
    initInfo.nextScanChan = LL_SCAN_ADV_CHAN_37;
    initInfo.wlPolicy     = LL_INIT_WL_POLICY_USE_PEER_ADDR;
    initInfo.connId       = 0;
    initInfo.scaValue     = LL_SCA_MASTER_DEFAULT;
    // set default extended Init values
    extInitInfo.ownAddr[0]   = 0xFF;
    extInitInfo.ownAddr[1]   = 0xFF;
    extInitInfo.ownAddr[2]   = 0xFF;
    extInitInfo.ownAddr[3]   = 0xFF;
    extInitInfo.ownAddr[4]   = 0xFF;
    extInitInfo.ownAddr[5]   = 0xFF;
    extInitInfo.ownAddrType  = LL_DEV_ADDR_TYPE_PUBLIC;
//    extInitInfo.initPending  = TRUE;
    extInitInfo.scanMode     = LL_SCAN_STOP;

    for (int i = 0; i < LL_MAX_EXTENDED_SCAN_PHYS; i ++)   // bug correct 2020-03-09
    {
        extInitInfo.scanInterval[i] = LL_SCAN_INTERVAL_DEFAULT;
        extInitInfo.scanWindow[i]   = LL_SCAN_INTERVAL_DEFAULT;
    }

    extInitInfo.wlPolicy     = LL_INIT_WL_POLICY_USE_PEER_ADDR;
    extInitInfo.connId       = 0;
    extInitInfo.scaValue     = LL_SCA_MASTER_DEFAULT;
    extInitInfo.current_chn  = LL_SCAN_ADV_CHAN_37;
    scanSyncInfo.valid  = FALSE;
    // reset the Link Layer
    (void)LL_Reset();
    // generate true random number for AES-CCM
    (void)LL_ENC_GenerateTrueRandNum( cachedTRNGdata, LL_ENC_TRUE_RAND_BUF_SIZE );
    numComplPkts         = 0;
    numComplPktsLimit    = 1;
    // add by HZF, init globals
    fastTxRespTime = LL_EXT_DISABLE_FAST_TX_RESP_TIME;   // what is fast TX feature???
    llState = LL_STATE_IDLE;
    // add in A2
    llSecondaryState = LL_SEC_STATE_IDLE;
    // initialize this devices Feature Set
    // has been called in LL_Reset() , so comended by ZQ
    //llInitFeatureSet();
    (void)osal_pwrmgr_task_state( LL_TaskID, PWRMGR_CONSERVE );
    // default sleep delay
    sleepDelay = pGlobal_config[MIN_TIME_TO_STABLE_32KHZ_XOSC];
    // delay sleep to allow the 32kHz crystal to stablize
    osal_set_event( LL_TaskID, LL_EVT_START_32KHZ_XOSC_DELAY );
}


/*******************************************************************************
    @fn          LL_ProcessEvent0

    @brief       This is the Link Layer process event handler called by OSAL.

    input parameters

    @param       taskId - Task identifier assigned by OSAL.
                events - Event flags to be processed by this task.

    output parameters

    @param       None.

    @return      Unprocessed event flags.
*/

uint16 LL_ProcessEvent0( uint8 task_id, uint16 events )
{
	(void) task_id;
    if ( events & LL_EVT_NEXT_INTERVAL )
    {
        ll_debug_output(DEBUG_LL_TIMER_EXPIRY_ENTRY);
        LL_evt_schedule();
        ll_debug_output(DEBUG_LL_TIMER_EXPIRY_EXIT);
        return (events ^ LL_EVT_NEXT_INTERVAL );
    }

    /*
    ** Directed Advertising results in a Master Connection
    */
    if ( events & LL_EVT_MASTER_CONN_CREATED )
    {
        llConnState_t* connPtr;
        connPtr = &conn_param[initInfo.connId];

        // if own addr or peer addr using RPA, report enhance connection complete event
        // otherwise report legacy connection complete event.
        // note that it may not align to below words int the spec
        // "If this event is unmasked and the HCI_LE_Connection_Complete event is
        // unmasked, only the HCI_LE_Enhanced_Connection_Complete event is sent
        // when a new connection has been created"
        /*if (g_currentPeerAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC ||
                g_currentPeerAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM ||
                g_currentLocalAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC ||
                g_currentLocalAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM)
        {
            uint8* local, *peer;
            uint8  nil_addr[] = {0, 0, 0, 0, 0, 0};

            if (g_currentPeerAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC ||
                    g_currentPeerAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM )
                peer = g_currentPeerRpa;
            else
                peer = nil_addr;

            if (g_currentLocalAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC ||
                    g_currentLocalAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM )
                local = g_currentLocalRpa;
            else
                local = nil_addr;

            LL_EnhConnectionCompleteCback( LL_STATUS_SUCCESS,
                                           (uint16)connPtr->connId,                 // connection handle
                                           LL_LINK_CONNECT_COMPLETE_MASTER,          // role
                                           g_currentPeerAddrType,                   // peer's address type
                                           peerInfo.peerAddr,                       // peer's address
                                           local,
                                           peer,
                                           connPtr->curParam.connInterval >> 1,     // connection interval, back to 1.25ms units
                                           connPtr->curParam.slaveLatency,          // slave latency
                                           connPtr->curParam.connTimeout >> 4,      // connection timeout, back to 10ms units
                                           0 );                                     // sleep clock accurracy not valid for master
        }
        else */
        {
            LL_ConnectionCompleteCback( LL_STATUS_SUCCESS,                       // reasonCode
                                        (uint16)connPtr->connId,                 // connection handle
                                        LL_LINK_CONNECT_COMPLETE_MASTER,          // role
                                        peerInfo.peerAddrType,                   // peer's address type
                                        peerInfo.peerAddr,                       // peer's address
                                        connPtr->curParam.connInterval >> 1,     // connection interval, back to 1.25ms units
                                        connPtr->curParam.slaveLatency,              // slave latency
                                        connPtr->curParam.connTimeout >> 4,      // connection timeout, back to 10ms units
                                        0 );             // sleep clock accurracy
        }

        //if (connPtr->channel_selection == LL_CHN_SEL_ALGORITHM_2)
        //    LL_ChannelSelectionAlgorithmCback((uint16)connPtr->connId, LL_CHN_SEL_ALGORITHM_2);

        return (events ^ LL_EVT_MASTER_CONN_CREATED );
    }

    /*
    ** Create Connection Cancel
    */
    if ( events & LL_EVT_MASTER_CONN_CANCELLED )
    {
        // notify Host
        LL_ConnectionCompleteCback( LL_STATUS_ERROR_UNKNOWN_CONN_HANDLE,     // reasonCode
                                    (uint16)0,                               // connection handle
                                    LL_LINK_CONNECT_COMPLETE_MASTER,         // role
                                    peerInfo.peerAddrType,                   // peer's address type
                                    peerInfo.peerAddr,                       // peer's address
                                    0,                                       // connection interval, back to 1.25ms units
                                    0,                                       // slave latency
                                    0,                                       // connection timeout, back to 10ms units
                                    0 );                                     // sleep clock accurracy not valid for master
        return (events ^ LL_EVT_MASTER_CONN_CANCELLED );
    }

    /*
    ** Directed Advertising failed to connect
    */
    if ( events & LL_EVT_DIRECTED_ADV_FAILED )
    {
        // notify Host
        LL_ConnectionCompleteCback( LL_STATUS_ERROR_DIRECTED_ADV_TIMEOUT,    // reasonCode
                                    (uint16)0,                               // connection handle
                                    LL_LINK_CONNECT_COMPLETE_SLAVE,          // role
                                    peerInfo.peerAddrType,                   // peer's address type
                                    peerInfo.peerAddr,                       // peer's address
                                    0,                                       // connection interval, back to 1.25ms units
                                    0,                                       // slave latency
                                    0,                                       // connection timeout, back to 10ms units
                                    0 );                                     // sleep clock accurracy
        return (events ^ LL_EVT_DIRECTED_ADV_FAILED );
    }

    /*
    ** Directed Advertising results in a Slave Connection
    */
    if ( events & LL_EVT_SLAVE_CONN_CREATED )
    {
        llConnState_t* connPtr;
        connPtr = &conn_param[adv_param.connId];

        // if own addr or peer addr using RPA, report enhance connection complete event
        // otherwise report legacy connection complete event.
        // note that it may not align to below words int the spec
        // "If this event is unmasked and the HCI_LE_Connection_Complete event is
        // unmasked, only the HCI_LE_Enhanced_Connection_Complete event is sent
        // when a new connection has been created"
        /*if (g_currentPeerAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC ||
                g_currentPeerAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM ||
                g_currentLocalAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC ||
                g_currentLocalAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM)
        {
            uint8* local, *peer;
            uint8  nil_addr[] = {0, 0, 0, 0, 0, 0};

            if (g_currentPeerAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC ||
                    g_currentPeerAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM )
                peer = g_currentPeerRpa;
            else
                peer = nil_addr;

            if (g_currentLocalAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC ||
                    g_currentLocalAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM )
                local = g_currentLocalRpa;
            else
                local = nil_addr;

            LL_EnhConnectionCompleteCback( LL_STATUS_SUCCESS,
                                           (uint16)connPtr->connId,                 // connection handle
                                           LL_LINK_CONNECT_COMPLETE_SLAVE,          // role
                                           g_currentPeerAddrType,                   // peer's address type
                                           peerInfo.peerAddr,                       // peer's address
                                           local,
                                           peer,
                                           connPtr->curParam.connInterval >> 1,     // connection interval, back to 1.25ms units
                                           connPtr->curParam.slaveLatency,              // slave latency
                                           connPtr->curParam.connTimeout >> 4,      // connection timeout, back to 10ms units
                                           connPtr->sleepClkAccuracy );             // sleep clock accurracy
        }
        else*/
        {
            LL_ConnectionCompleteCback( LL_STATUS_SUCCESS,                       // reasonCode
                                        (uint16)connPtr->connId,                 // connection handle
                                        LL_LINK_CONNECT_COMPLETE_SLAVE,          // role
                                        peerInfo.peerAddrType,                   // peer's address type
                                        peerInfo.peerAddr,                       // peer's address
                                        connPtr->curParam.connInterval >> 1,     // connection interval, back to 1.25ms units
                                        connPtr->curParam.slaveLatency,              // slave latency
                                        connPtr->curParam.connTimeout >> 4,      // connection timeout, back to 10ms units
                                        connPtr->sleepClkAccuracy );             // sleep clock accurracy
        }

        //if (connPtr->channel_selection == LL_CHN_SEL_ALGORITHM_2)
        //    LL_ChannelSelectionAlgorithmCback((uint16)connPtr->connId, LL_CHN_SEL_ALGORITHM_2);

        return (events ^ LL_EVT_SLAVE_CONN_CREATED );
    }

    /*
    ** Advertising results in a Slave Connection with an Unacceptable
    ** Connection Interval
    */
    if ( events & LL_EVT_SLAVE_CONN_CREATED_BAD_PARAM )
    {
        // notify Host
        LL_ConnectionCompleteCback( LL_STATUS_ERROR_UNACCEPTABLE_CONN_INTERVAL, // reasonCode
                                    (uint16)0,                                  // connection handle
                                    LL_LINK_CONNECT_COMPLETE_SLAVE,             // role
                                    peerInfo.peerAddrType,                      // peer's address type
                                    peerInfo.peerAddr,                          // peer's address
                                    0,                                          // connection interval, back to 1.25ms units
                                    0,                                          // slave latency
                                    0,                                          // connection timeout, back to 10ms units
                                    0 );                                        // sleep clock accurracy
        return (events ^ LL_EVT_SLAVE_CONN_CREATED_BAD_PARAM );
    }

    /*
    ** Ensure the 32kHz crystal is stable after POR/External Reset/PM3
    ** before allowing sleep.
    ** Note: There is nothing in hardware to indicate this to software.
    */
    if ( events & LL_EVT_START_32KHZ_XOSC_DELAY )
    {
        // check if the delay hasn't been disabled
        if ( sleepDelay )
        {
            //osal_pwrmgr_device( PWRMGR_ALWAYS_ON );
            (void)osal_pwrmgr_task_state( LL_TaskID, PWRMGR_HOLD );
            osal_start_timerEx( LL_TaskID, LL_EVT_32KHZ_XOSC_DELAY, sleepDelay );
        }

        return (events ^ LL_EVT_START_32KHZ_XOSC_DELAY );
    }

    /*
    ** The minimum stabilization delay for the 32kHz crystal has expired.
    */
    if ( events & LL_EVT_32KHZ_XOSC_DELAY )
    {
        // delay over, so allow sleep
        //osal_pwrmgr_device( PWRMGR_BATTERY );
        (void)osal_pwrmgr_task_state( LL_TaskID, PWRMGR_CONSERVE );
        return (events ^ LL_EVT_32KHZ_XOSC_DELAY );
    }

    /*
    ** Issue Hard System Reset
    */
    if ( events & LL_EVT_RESET_SYSTEM_HARD )
    {
        // Note: This will break communication with USB dongle.
        // Note: No return here after reset.
        //SystemReset();
        // Note: Unreachable statement generates compiler warning!
        //return (events ^ LL_EVT_RESET_SYSTEM_HARD );
    }

    /*
    ** Issue Soft System Reset
    */
    if ( events & LL_EVT_RESET_SYSTEM_SOFT )
    {
        // Note: This will not break communication with USB dongle.
        // Note: No return here after reset.
        //SystemResetSoft();
        return (events ^ LL_EVT_RESET_SYSTEM_SOFT );
    }

    //====== add in A2, for simultaneous connect & adv/scan
    if (events & LL_EVT_SECONDARY_ADV)
    {
        #ifdef DEBUG_LL
        LOG("--->\n");
        #endif

        if (llSecondaryState == LL_SEC_STATE_IDLE
                || llSecondaryState == LL_SEC_STATE_IDLE_PENDING)    // adv may be cancel during waiting period, do nothing in this case
            ;
        else
        {
            // advertise allow decision
            if (llSecAdvAllow())
            {
                llSetupSecAdvEvt();
//                if (llSetupSecAdvEvt())
//                    llSecondaryState = LL_SEC_STATE_ADV;
//              else
//                    llSecondaryState = LL_SEC_STATE_ADV_PENDING;;
//#ifdef DEBUG_LL
//                LOG("setup secondary adv event\r\n");
//#endif
            }
            else
            {
                llSecondaryState = LL_SEC_STATE_ADV_PENDING;
            }
        }

        return (events ^ LL_EVT_SECONDARY_ADV );
    }

    // ===== scan
    if (events & LL_EVT_SECONDARY_SCAN)
    {
        if (llSecondaryState == LL_SEC_STATE_IDLE || scanInfo.scanMode == LL_SCAN_STOP)   // scan may be cancel during waiting period, do nothing in this case
            llSecondaryState = LL_SEC_STATE_IDLE;
        else
        {
            llSetupSecScan(scanInfo.nextScanChan);
        }

        return (events ^ LL_EVT_SECONDARY_SCAN );
    }

    // ======= add for A2 multi-conn, init
    if (events & LL_EVT_SECONDARY_INIT)
    {
        if (llSecondaryState == LL_SEC_STATE_IDLE || initInfo.scanMode == LL_SCAN_STOP)   // scan may be cancel during waiting period, do nothing in this case
            llSecondaryState = LL_SEC_STATE_IDLE;
        else
        {
            llSetupSecInit(initInfo.nextScanChan);
            // TODO
        }

        return (events ^ LL_EVT_SECONDARY_INIT );
    }

    // ======= RPA re-calculate
    if (events & LL_EVT_RPA_TIMEOUT)
    {
        uint8 resolve_address[6];
        uint8* localIrk;

        // calculate RPA & update adv_param
        if (peerInfo.peerAddrType == LL_DEV_ADDR_TYPE_PUBLIC || peerInfo.peerAddrType == LL_DEV_ADDR_TYPE_RANDOM)
//        if (g_currentLocalAddrType == LL_DEV_ADDR_TYPE_PUBLIC || g_currentLocalAddrType == LL_DEV_ADDR_TYPE_RANDOM)
        {
            // search the resolving list to get local IRK
            if ((adv_param.ownAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC || adv_param.ownAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM)
                    && (ll_readLocalIRK(&localIrk, peerInfo.peerAddr, peerInfo.peerAddrType) == TRUE))
            {
                if (!ll_isIrkAllZero(localIrk) &&
                        ll_CalcRandomAddr(localIrk, resolve_address) == SUCCESS)
                {
                    LL_COPY_DEV_ADDR_LE( adv_param.ownAddr, resolve_address );
                    SET_BITS(g_tx_adv_buf.txheader, LL_DEV_ADDR_TYPE_RANDOM, TX_ADD_SHIFT, TX_ADD_MASK);
                    osal_memcpy( &g_tx_adv_buf.data[0],  adv_param.ownAddr, 6);
                    SET_BITS(tx_scanRsp_desc.txheader, LL_DEV_ADDR_TYPE_RANDOM, TX_ADD_SHIFT, TX_ADD_MASK);
                    osal_memcpy( &tx_scanRsp_desc.data[0], adv_param.ownAddr, 6);
                    osal_memcpy( &g_currentLocalRpa[0],  resolve_address, 6);
//                for (int i = 0; i < 6; i ++)
//                  LOG("%x ", resolve_address[i]);
//                LOG("\n");
                }
            }

            // update initA(targetA) for direct ADV
            if (adv_param.advEvtType == LL_ADV_CONNECTABLE_LDC_DIRECTED_EVT ||
                    adv_param.advEvtType == LL_ADV_CONNECTABLE_HDC_DIRECTED_EVT)
            {
                uint8*  peerIrk;

                // search the resolving list to get local IRK
                if (ll_readPeerIRK(&peerIrk, peerInfo.peerAddr, peerInfo.peerAddrType) == TRUE)
                {
                    if (!ll_isIrkAllZero(peerIrk))            // for all-zero local IRK, not RPA used
                    {
                        if (ll_CalcRandomAddr(peerIrk, resolve_address) == SUCCESS)
                        {
                            osal_memcpy((uint8_t*) &(g_tx_adv_buf.data[6]), resolve_address, 6);
                            osal_memcpy( &g_currentPeerRpa[0],  resolve_address, 6);
                        }
                    }
                }
            }
        }
        else            // if generate RPA failed, do nothing.
        {
        }

        if (g_llRlDeviceNum > 0)
            osal_start_timerEx( LL_TaskID, LL_EVT_RPA_TIMEOUT, g_llRlTimeout * 1000 );

        return (events ^ LL_EVT_RPA_TIMEOUT );
    }

    return 0;
}


/*******************************************************************************
    LL API for HCI
*/

/*******************************************************************************
    @fn          LL_TX_bm_alloc API

    @brief       This API is used to allocate memory using buffer management.

                Note: This function should never be called by the application.
                      It is only used by HCI and L2CAP_bm_alloc.

    input parameters

    @param       size - Number of bytes to allocate from the heap.

    output parameters

    @param       None.

    @return      Pointer to buffer, or NULL.
*/
void* LL_TX_bm_alloc( uint16 size )
{
    uint8* pBuf;
    #if 0
    // Note: This is the lowest call for TX buffer management allocation.

    // provide padding for encryption
    // Note: Potentially wastes up to 15 bytes per packet, but speeds up
    //       execution.
    if ( size <= LL_ENC_BLOCK_LEN )
    {
        size = LL_ENC_BLOCK_LEN;
    }
    else
    {
        size = 2*LL_ENC_BLOCK_LEN;
    }

    #else
    //size =  (size+LL_ENC_BLOCK_LEN-1)&0xfff0;//align to 16Byte
    #endif
    size =  (size+LL_ENC_BLOCK_LEN-1)&0xfff0;//align to 16Byte
    pBuf = osal_bm_alloc( size             +
                          sizeof(txData_t) +
                          LL_PKT_HDR_LEN   +
                          LL_PKT_MIC_LEN );

    if ( pBuf != NULL )
    {
        // return pointer to user payload
        return( pBuf + ((uint16)sizeof(txData_t)+LL_PKT_HDR_LEN) );
    }

    return( (void*)NULL );
}


/*******************************************************************************
    This API is used to allocate memory using buffer management.

    Public function defined in ll.h.
*/
void* LL_RX_bm_alloc( uint16 size )
{
    uint8* pBuf;
    // Note: This is the lowest call for RX buffer management allocation.
    pBuf = osal_bm_alloc( size + HCI_RX_PKT_HDR_SIZE );

    if ( pBuf != NULL )
    {
        // return pointer to user payload
        return( pBuf + HCI_RX_PKT_HDR_SIZE );
    }

    return( (void*)NULL );
}

/*******************************************************************************
    This function is used by the HCI to reset and initialize the LL Controller.

    Public function defined in ll.h.
*/
llStatus_t LL_Reset0( void )
{
    // enter critical section
    HAL_ENTER_CRITICAL_SECTION();
    // clear the white list table
    g_llWlDeviceNum = 0;

    for (int i = 0; i < LL_WHITELIST_ENTRY_NUM; i++)
        g_llWhitelist[i].peerAddrType = 0xff;

    // clear resolving list
    g_llRlDeviceNum = 0;

    for (int i = 0; i < LL_RESOLVINGLIST_ENTRY_NUM; i++)
    {
        g_llResolvinglist[i].peerAddrType = 0xff;
        g_llResolvinglist[i].privacyMode  = NETWORK_PRIVACY_MODE;
    }

    g_llRlEnable = FALSE;
    g_llRlTimeout = 900;
    g_llPrdAdvDeviceNum = 0;

    for (int i = 0; i < LL_PRD_ADV_ENTRY_NUM; i++)
        g_llPeriodicAdvlist[i].addrType = 0xff;

    // set the peer's device address type, as allowed for connections by Host
    peerInfo.peerAddrType = LL_DEV_ADDR_TYPE_PUBLIC;
    // clear the peer's address
    peerInfo.peerAddr[0] = 0x00;
    peerInfo.peerAddr[1] = 0x00;
    peerInfo.peerAddr[2] = 0x00;
    peerInfo.peerAddr[3] = 0x00;
    peerInfo.peerAddr[4] = 0x00;
    peerInfo.peerAddr[5] = 0x00;
    // init the Adv parameters to their default values
    adv_param.advMode = LL_ADV_MODE_OFF;
    // clear the advertising interval
    adv_param.advInterval = LL_ADV_INTERVAL_DEFAULT;//  Ti set this parameter = 0;
    // Note that only the first three bits are used, and note that despite the
    // fact that the Host HCI call passes 5 bytes to specify which advertising
    // channels are to be used, only the first three bits of the first byte are
    // actually used. We'll save four bytes then, and default to all used.
    adv_param.advChanMap = LL_ADV_CHAN_ALL;
    // set a default type of advertising
    adv_param.advEvtType = LL_ADV_NONCONNECTABLE_UNDIRECTED_EVT;
    // payload length, including AdvA
    adv_param.advDataLen = 0;
    #ifdef DEBUG
    // clear memory
    (void)osal_memset( advInfo.advData, 0, LL_MAX_ADV_DATA_LEN );
    // clear memory
    (void)osal_memset( advInfo.scanRspData, 0, LL_MAX_SCAN_DATA_LEN );
    #endif // DEBUG
    // default to no scan response data
    adv_param.scanRspLen = 0;
    // ===================== for scan/init parameters
    // disable scanning
    scanInfo.scanMode = LL_SCAN_STOP;
    // disable init scanning
    initInfo.scanMode = LL_SCAN_STOP;
    // initialize this devices Feature Set
    llInitFeatureSet();
    // initialize default channel map
    chanMapUpdate.chanMap[0] = 0xFF;
    chanMapUpdate.chanMap[1] = 0xFF;
    chanMapUpdate.chanMap[2] = 0xFF;
    chanMapUpdate.chanMap[3] = 0xFF;
    chanMapUpdate.chanMap[4] = 0x1F;
    // set state/role
    llState = LL_STATE_IDLE;
    ll_debug_output(DEBUG_LL_STATE_IDLE);
    // add in A2
    llSecondaryState = LL_SEC_STATE_IDLE;
    numComplPkts         = 0;
    numComplPktsLimit    = 1;
    fastTxRespTime = LL_EXT_DISABLE_FAST_TX_RESP_TIME;      // TI default enable it, hzf
    rxFifoFlowCtrl   = LL_RX_FLOW_CONTROL_DISABLED;
    #if 0               // TODO: normally LL should not invoke upper layer function. And gap also invoke these operation???
    //add by ZQ 20181030 for DLE feature
    L2CAP_SegmentPkt_Reset();
    L2CAP_ReassemblePkt_Reset();
    osal_memset(&g_sarDbgCnt, 0, sizeof(g_sarDbgCnt));
    #endif
    llPduLengthManagmentReset();
    llPhyModeCtrlReset();

    for (int i = 0; i < g_maxConnNum; i ++)
    {
        conn_param[i].llPhyModeCtrl.def.txPhy = LE_1M_PHY | LE_2M_PHY;
        conn_param[i].llPhyModeCtrl.def.rxPhy = LE_1M_PHY | LE_2M_PHY;
        conn_param[i].llPhyModeCtrl.def.allPhy=0;
        llResetConnId(i);
        reset_conn_buf(i);
    }

    // HZF: add for multi-connection
    g_ll_conn_ctx.currentConn = LL_INVALID_CONNECTION_ID;
    g_ll_conn_ctx.numLLConns = 0;
    g_ll_conn_ctx.numLLMasterConns = 0;
    // =========== extended adv/extended scan relate reset
    g_llScanMode = LL_MODE_INVALID;
    g_llAdvMode =  LL_MODE_INVALID;

    // clear adv set (extended adv part)
    for (int i = 0; i < g_extAdvNumber; i ++)
    {
        uint8* scanRspData = g_pExtendedAdvInfo[i].scanRspData;
        uint8* advData     = g_pExtendedAdvInfo[i].data.advertisingData;
        memset(&g_pExtendedAdvInfo[i], 0, sizeof(extAdvInfo_t));
        g_pExtendedAdvInfo[i].scanRspData = scanRspData;
        g_pExtendedAdvInfo[i].data.advertisingData = advData;
        g_pExtendedAdvInfo[i].advHandle = LL_INVALID_ADV_SET_HANDLE;
    }

    // clear adv set (periodic adv part)
    for (int i = 0; i < g_perioAdvNumber; i ++)
    {
        uint8* advData     = g_pPeriodicAdvInfo[i].data.advertisingData;
        memset(&g_pPeriodicAdvInfo[i], 0, sizeof(periodicAdvInfo_t));
        g_pPeriodicAdvInfo[i].data.advertisingData = advData;
        g_pPeriodicAdvInfo[i].advHandle = LL_INVALID_ADV_SET_HANDLE;
    }

    // exit critical section
    HAL_EXIT_CRITICAL_SECTION();
    // reset all statistics counters
    osal_memset(&g_pmCounters, 0, sizeof(g_pmCounters));
    return( LL_STATUS_SUCCESS );
}



/*******************************************************************************
    @fn          LL_Disconnect0 API

    @brief       This API is called by the HCI to terminate a LL connection.

    input parameters

    @param       connId - The LL connection ID on which to send this data.
    @param       reason - The reason for the Host connection termination.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER,
                LL_STATUS_ERROR_INACTIVE_CONNECTION
                LL_STATUS_ERROR_CTRL_PROC_ALREADY_ACTIVE
*/
llStatus_t LL_Disconnect0( uint16 connId,
                           uint8  reason )
{
    llStatus_t    status;
    llConnState_t* connPtr;

    // check if the reason code is valid
    if ( (reason != LL_DISCONNECT_AUTH_FAILURE)                &&
            (reason != LL_DISCONNECT_REMOTE_USER_TERM)            &&
            (reason != LL_DISCONNECT_REMOTE_DEV_LOW_RESOURCES)    &&
            (reason != LL_DISCONNECT_REMOTE_DEV_POWER_OFF)        &&
            (reason != LL_DISCONNECT_UNSUPPORTED_REMOTE_FEATURE)  &&
            (reason != LL_DISCONNECT_KEY_PAIRING_NOT_SUPPORTED)   &&
            (reason != LL_DISCONNECT_UNACCEPTABLE_CONN_INTERVAL) )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // make sure connection ID is valid
    if ( (status = LL_ConnActive(connId)) != LL_STATUS_SUCCESS )
    {
        return( status );
    }

    // get connection info
    connPtr = &conn_param[connId];

    // check if any control procedure is already pending
    if ( connPtr->ctrlPktInfo.ctrlPktActive == TRUE )
    {
        // check if a terminate control procedure is already what's pending
        if ( connPtr->ctrlPktInfo.ctrlPkts[0] == LL_CTRL_TERMINATE_IND )
        {
            return( LL_STATUS_ERROR_CTRL_PROC_ALREADY_ACTIVE );
        }
        else // spec now says a terminate can happen any time
        {
            // indicate the peer requested this termination
            connPtr->termInfo.reason = reason;
            // de-activate slave latency to expedite termination
            connPtr->slaveLatency = 0;
            // override any control procedure that may be in progress
            llReplaceCtrlPkt( connPtr, LL_CTRL_TERMINATE_IND );
        }
    }
    else // no control procedure currently active, so set this one up
    {
        // indicate the peer requested this termination
        connPtr->termInfo.reason = reason;
        // de-activate slave latency to expedite termination
        connPtr->slaveLatency = 0;
        // queue control packet for processing
        llEnqueueCtrlPkt( connPtr, LL_CTRL_TERMINATE_IND );
    }

    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          LL_TxData0 API

    @brief       This API is called by the HCI to transmit a buffer of data on a
                given LL connection. If fragmentation is supported, the HCI must
                also indicate whether this is the first Host packet, or a
                continuation Host packet. When fragmentation is not supported,
                then a start packet should always specified. If the device is in
                a connection as a Master and the current connection ID is the
                connection for this data, or is in a connection as a Slave, then
                the data is written to the TX FIFO (even if the radio is
                curerntly active). If this is a Slave connection, and Fast TX is
                enabled and Slave Latency is being used, then the amount of time
                to the next event is checked. If there's at least a connection
                interval plus some overhead, then the next event is re-aligned
                to the next event boundary. Otherwise, in all cases, the buffer
                pointer will be retained for transmission, and the callback
                event LL_TxDataCompleteCback will be generated to the HCI when
                the buffer pointer is no longer needed by the LL.

                Note: If the return status is LL_STATUS_ERROR_OUT_OF_TX_MEM,
                      then the HCI must not release the buffer until it receives
                      the LL_TxDataCompleteCback callback, which indicates the
                      LL has copied the transmit buffer.

                Note: The HCI should not call this routine if a buffer is still
                      pending from a previous call. This is fatal!

                Note: If the connection should be terminated within the LL
                      before the Host knows, attempts by the HCI to send more
                      data (after receiving a LL_TxDataCompleteCback) will
                      fail (LL_STATUS_ERROR_INACTIVE_CONNECTION).

    input parameters

    @param       connId   - The LL connection ID on which to send this data.
    @param       *pBuf    - A pointer to the data buffer to transmit.
    @param       pktLen   - The number of bytes to transmit on this connection.
    @param       fragFlag - LL_DATA_FIRST_PKT_HOST_TO_CTRL:
                             Indicates buffer is the start of a
                             Host-to-Controller packet.
                           LL_DATA_CONTINUATION_PKT:
                             Indicates buffer is a continuation of a
                             Host-to-Controller packet.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER,
                LL_STATUS_ERROR_INACTIVE_CONNECTION,
                LL_STATUS_ERROR_OUT_OF_TX_MEM,
                LL_STATUS_ERROR_UNEXPECTED_PARAMETER
*/
llStatus_t LL_TxData0( uint16 connId,
                       uint8* pBuf,
                       uint8  pktLen,
                       uint8  fragFlag )
{
    llConnState_t* connPtr;
    txData_t*      pTxData;
    // get the connection info based on the connection ID
    connPtr = &conn_param[connId];

    // sanity check input parameters
    if ( (pBuf == NULL) || (pktLen > connPtr->llPduLen.local.MaxTxOctets/*LL_MAX_LINK_DATA_LEN*/) ||
            ((fragFlag != LL_DATA_FIRST_PKT_HOST_TO_CTRL) &&
             (fragFlag != LL_DATA_CONTINUATION_PKT)) )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // make sure connection ID is valid
    if ( !conn_param [connId].active )
    {
        //return( status );
        return( LL_STATUS_ERROR_INACTIVE_CONNECTION );
    }

    // check if the number of available data buffers has been exceeded
    if ( getTxBufferFree(connPtr) == 0)
    {
        return( LL_STATUS_ERROR_OUT_OF_TX_MEM );
    }

    // adjust pointer to start of packet (i.e. at the header)
    pBuf -= LL_PKT_HDR_LEN;
    // set the packet length field
    // Note: The LLID and Length fields are swapped for the nR (for DMA).
    pBuf[0] = pktLen;
    // set LLID fragmentation flag in header
    // Note: NESN=SN=MD=0 and is handled by RF.
    pBuf[1] = (fragFlag==LL_DATA_FIRST_PKT_HOST_TO_CTRL) ?
              LL_DATA_PDU_HDR_LLID_DATA_PKT_FIRST       : // first pkt
              LL_DATA_PDU_HDR_LLID_DATA_PKT_NEXT;         // continuation pkt
    // ALT: Place check if packet needs to be encrypted and encryption here,
    //      but be careful about control packets and getting them out of order!
    // point to Tx data entry
    pTxData = (txData_t*)(pBuf - sizeof(txData_t));
    // it does, so queue up this data
    llEnqueueDataQ( &connPtr->txDataQ, pTxData );

    // check that we are either the master or slave, and if so, that the current
    // connection is connId
    if ( !(((llState == LL_STATE_CONN_MASTER) || (llState == LL_STATE_CONN_SLAVE))))
        //(connId == g_ll_conn_ctx.currentConn)) )
    {
        // either we are not the master or the slave, or the we are but the connId
        // is not the current connection, so just return success as the data is
        // queued on the connection
        return( LL_STATUS_SUCCESS );
    }

    // copy any pending data to the TX FIFO
    llProcessTxData( connPtr, LL_TX_DATA_CONTEXT_SEND_DATA );

    // check if TX FIFO has anything in it
    if (getTxBufferSize(connPtr) > 0 )
    {
        // check if we are a master or slave (i.e. in a connection), and the current
        // current connection is connId; check if fast TX is enabled; check if
        // slave latency is in use
        if ( (fastTxRespTime == LL_EXT_ENABLE_FAST_TX_RESP_TIME) && connPtr->slaveLatency )
        {
            // ����Ŀǰ��֧��fast Tx resp time���feature�����Ҫ֧�����µĴ�����Ҫ�޸�
            #if 0
            HAL_ENTER_CRITICAL_SECTION(cs);

            //   taskInfo_t *curTask = llGetCurrentTask();
            // uint32 curTime      = llGetCurrentTime();

            // check if there's enough time before the next event to readjust the
            // event timing
            // Note: If T2E1 > CT+1, then there is at least two ticks before T2E1.
            // Note: TRUE means first parameter T2E1 is not <= second parameter CT,
            //       thus T2E1 > CT.
            if ( llTimeCompare( conn_param [0].next_event_base_time, conn_param [0].next_event_fine_time ) == TRUE )
            {
                // There is, and at this point, there could be events between now
                // and the next connection event. In order to wake on the next
                // event boundary, the number of elapsed events is found.

                // update current time, and adjust for one event plus some pad
                // Note: If we are just before the next event, then there's no
                //       point to an event re-alignment.
                //   curTime += connPtr->curParam.connInterval +
                //              LL_FAST_TX_TICKS_TO_EVT_PAD;

                // check if there's enough time before the next event
                // Note: If T2E1 > CT+CI+6, then there is at least six ticks
                //       before the N-1 event.
                // Note: TRUE means first parameter T2E1 is not <= second
                //       parameter CT, thus T2E1 > CT.
                if (llTimeCompare( conn_param [0].next_event_base_time, conn_param [0].next_event_fine_time ))
                {
                    // yes, so it is necessary to re-align on an earlier event
                    // then the next event
                    uint16 numEventsPast;
                    uint32 time;
                    // get the time delta between current time (CT) and last T2E1
                    // Note: The assumption here is that CT is always ahead of the
                    //       lastT2e1 because T2E1 is always ahead of lastT2e1 and
                    //       CT is at this point behind T2E1. If the call to this
                    //       function happens during a radio event (assuming the
                    //       MCU is not halted) or between the end of a radio
                    //       event and the start of LL processing, then CT will
                    //       still be greater than T2E1 (because T2E1 has not yet
                    //       been updated). If the call is made after LL
                    //       processing, then T2E1 will be updated to the next
                    //       radio event (i.e. ahead of CT), and lastT2e1 will be
                    //       the previous radio event (i.e. behind CT). So if CT
                    //       is behind T2E1, then it is always ahead of lastT2e1.
                    // Note: The only time CT could be equal to lastT2e1 is
                    //       if radio event ended in less than one software tick
                    //       and this routine is called before the rollover. This
                    //       should never happen given the current radio time and
                    //       post-processing time, but even if it does, this code
                    //       should still be fine as the delta would be zero.
                    time = calculateTimeDelta(conn_param[0].next_event_base_time, conn_param [0].next_event_fine_time );
                    // determine the number of events past the last T2E1
                    // Note: Only quotient in upper halfword needed.
                    //     time          = llDivide31By16To16( time, connPtr->curParam.connInterval );
                    numEventsPast = time  / (conn_param [0].curParam.connInterval * 625)+1;
                    // update next event counter
                    connPtr->currentEvent = connPtr->lastCurrentEvent;
                    connPtr->nextEvent    = connPtr->currentEvent + numEventsPast;
                    // update time to next event based on last adjusted AP
                    // Note: No need to re-calculate the receive window; can
                    //       re-use rxTimeout value previously set by Slave
                    //       post-processing.
                    //       curTask->t2e1.coarse =
                    //        (curTask->lastT2e1.coarse +
                    //        ((uint32)numEventsPast   *
                    //         (uint32)connPtr->curParam.connInterval)) & 0x00FFFFFF;
                    // adjust data channel
                    connPtr->nextChan = connPtr->currentChan;
                    llSetNextDataChan( connPtr );
                    // enable RF event
                    //      llScheduleTask( curTask );
                } // else amount of time to next event is less than CI+6
            } // else not enough time before next event to make realignment worthwhile

            HAL_EXIT_CRITICAL_SECTION();
            #endif
        }  // else delta correction active so queue packet

        // M/S, curConn==connID, fast Tx enabled, and SL in use
    } // TX FIFO empty

    // indicate the packet is sent of buffered
    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          LL_SetAdvParam0 API

    @brief       This API is called by the HCI to set the Advertiser's
                parameters.

    input parameters
    @param       advIntervalMin - The minimum Adv interval.
    @param       advIntervalMax - The maximum Adv interval.
    @param       advEvtType     - The type of advertisment event.
    @param       ownAddrType    - The Adv's address type of public or random.
    @param       peerAddrType   - BLE4.0: Only used for directed advertising.  BLE4.2: Peer address type
    @param       *peerAddr      - BLE4.0: Only used for directed advertising (NULL otherwise). BLE4.2: Peer address
    @param       advChanMap     - A byte containing 1 bit per advertising
                                 channel. A bit set to 1 means the channel is
                                 used. The bit positions define the advertising
                                 channels as follows:
                                 Bit 0: 37, Bit 1: 38, Bit 2: 39.
    @param       advWlPolicy    - The Adv white list filter policy.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER,
                LL_STATUS_ERROR_NO_ADV_CHAN_FOUND
*/
llStatus_t LL_SetAdvParam0( uint16 advIntervalMin,
                            uint16 advIntervalMax,
                            uint8  advEvtType,
                            uint8  ownAddrType,
                            uint8  peerAddrType,
                            uint8*  peerAddr,
                            uint8  advChanMap,
                            uint8  advWlPolicy )
{
    uint8 pduType;
    uint8 resolve_address[6];
    uint8* localIrk, *peerIrk;

    if (g_llAdvMode == LL_MODE_EXTENDED )
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llAdvMode = LL_MODE_LEGACY;

    // sanity check of parameters
    if ( ( (advEvtType != LL_ADV_CONNECTABLE_UNDIRECTED_EVT)     &&
            (advEvtType != LL_ADV_CONNECTABLE_HDC_DIRECTED_EVT)   &&
            (advEvtType != LL_ADV_NONCONNECTABLE_UNDIRECTED_EVT)  &&
            (advEvtType != LL_ADV_SCANNABLE_UNDIRECTED_EVT)       &&
            (advEvtType != LL_ADV_CONNECTABLE_LDC_DIRECTED_EVT) )      ||
            ( ((advEvtType == LL_ADV_CONNECTABLE_HDC_DIRECTED_EVT)  ||
               (advEvtType == LL_ADV_CONNECTABLE_LDC_DIRECTED_EVT)) &&
              ((peerAddr == NULL)                                 ||
               ((peerAddrType != LL_DEV_ADDR_TYPE_PUBLIC)         &&
                (peerAddrType != LL_DEV_ADDR_TYPE_RANDOM))) )          ||
            ( ((advEvtType == LL_ADV_NONCONNECTABLE_UNDIRECTED_EVT) ||
               (advEvtType == LL_ADV_SCANNABLE_UNDIRECTED_EVT))     &&
              // the minimum interval for nonconnectable Adv is 100ms
              ((advIntervalMin < LL_ADV_CONN_INTERVAL_MIN)      ||    // should use LL_ADV_NONCONN_INTERVAL_MIN after update it to 20ms
               (advIntervalMin > LL_ADV_NONCONN_INTERVAL_MAX)     ||
               (advIntervalMax < LL_ADV_CONN_INTERVAL_MIN)        ||    // should use LL_ADV_NONCONN_INTERVAL_MIN after update it to 20ms
               (advIntervalMax > LL_ADV_NONCONN_INTERVAL_MAX)) )        ||
            ( (advEvtType == LL_ADV_CONNECTABLE_UNDIRECTED_EVT)     &&
              // the minimum interval for connectable undirected Adv is 20ms
              ((advIntervalMin < LL_ADV_CONN_INTERVAL_MIN)         ||
               (advIntervalMin > LL_ADV_CONN_INTERVAL_MAX)         ||
               (advIntervalMax < LL_ADV_CONN_INTERVAL_MIN)         ||
               (advIntervalMax > LL_ADV_CONN_INTERVAL_MAX)) )      ||
            ( advIntervalMax < advIntervalMin )                 ||
            ( (ownAddrType != LL_DEV_ADDR_TYPE_PUBLIC)              &&
              (ownAddrType != LL_DEV_ADDR_TYPE_RANDOM)              &&
              (ownAddrType != LL_DEV_ADDR_TYPE_RPA_PUBLIC)          &&            // BLE 4.2
              (ownAddrType != LL_DEV_ADDR_TYPE_RPA_RANDOM))         ||            // BLE 4.2
            ( ((ownAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC)   ||
               (ownAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM))   &&
              (peerAddr == NULL))                                   ||
            ( (advWlPolicy != LL_ADV_WL_POLICY_ANY_REQ)        &&
              (advWlPolicy != LL_ADV_WL_POLICY_WL_SCAN_REQ)     &&
              (advWlPolicy != LL_ADV_WL_POLICY_WL_CONNECT_REQ)  &&
              (advWlPolicy != LL_ADV_WL_POLICY_WL_ALL_REQ) )         ||
            ( ((advChanMap & LL_ADV_CHAN_ALL) == 0) ) )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // check if advertising is active
    if ( adv_param.advMode == LL_ADV_MODE_ON )
    {
        // yes, so not allowed per the spec
        return( LL_STATUS_ERROR_COMMAND_DISALLOWED );
    }

    adv_param.advEvtType = advEvtType;
    adv_param.ownAddrType = ownAddrType;
    // save off the advertiser channel map
    adv_param.advChanMap = advChanMap & 0x07;

    // make sure there's at least one advertising channel that can be used
    if ( !adv_param.advChanMap )
    {
        // force all to be usable in case the error message is ignored
        adv_param.advChanMap = LL_ADV_CHAN_ALL;
        return( LL_STATUS_ERROR_NO_ADV_CHAN_FOUND );
    }

    // save the white list policy
    adv_param.wlPolicy = advWlPolicy;

    // set the advertiser address based on the HCI's address type preference
    if ( ownAddrType == LL_DEV_ADDR_TYPE_PUBLIC )
    {
        // get our address and address type
        g_currentLocalAddrType = LL_DEV_ADDR_TYPE_PUBLIC;
        LL_COPY_DEV_ADDR_LE( adv_param.ownAddr, ownPublicAddr );
    }
    else if ( ownAddrType == LL_DEV_ADDR_TYPE_RANDOM )
    {
        // get our address and address type
        g_currentLocalAddrType  = LL_DEV_ADDR_TYPE_RANDOM;
        LL_COPY_DEV_ADDR_LE( adv_param.ownAddr, ownRandomAddr );
    }
    // BBB ROM code add
    else if ( ownAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC ||
              ownAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM)
    {
        uint8 found = FALSE;

        // search the resolving list to get local IRK
        if (ll_readLocalIRK(&localIrk, peerAddr, peerAddrType) == TRUE)
        {
            if (!ll_isIrkAllZero(localIrk))            // for all-zero local IRK, not RPA used
            {
                if (ll_CalcRandomAddr(localIrk, resolve_address) == SUCCESS)
                {
                    LL_COPY_DEV_ADDR_LE( adv_param.ownAddr, resolve_address );
                    osal_memcpy( &g_currentLocalRpa[0],  resolve_address, 6);
                    found = TRUE;
                    g_currentLocalAddrType  = LL_DEV_ADDR_TYPE_RANDOM;
                }
            }
        }

        if (found == FALSE)
        {
            if (ownAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC)
            {
                LL_COPY_DEV_ADDR_LE( adv_param.ownAddr, ownPublicAddr );
                g_currentLocalAddrType = LL_DEV_ADDR_TYPE_PUBLIC;
            }
            else
            {
                LL_COPY_DEV_ADDR_LE( adv_param.ownAddr, ownRandomAddr );
                g_currentLocalAddrType  = LL_DEV_ADDR_TYPE_RANDOM;
            }
        }
    }

    // save peer address info, to consider whether we need it
    if (peerAddr != NULL)
        LL_COPY_DEV_ADDR_LE( peerInfo.peerAddr, peerAddr );

    peerInfo.peerAddrType = peerAddrType;

    // a Connectable Directed Adv event requires Init address info
    if ( advEvtType == LL_ADV_CONNECTABLE_HDC_DIRECTED_EVT )
    {
        // get the Init's address and address type as well
        LL_COPY_DEV_ADDR_LE( peerInfo.peerAddr, peerAddr );
        // the advertising interval and delay are not used
        adv_param.advInterval = 2;//0;    // set by HZF, 2 means 1.25ms, so 3 adv channel = 3.75ms, spec require < 3.75ms
    }
    else if ( advEvtType == LL_ADV_CONNECTABLE_LDC_DIRECTED_EVT )
    {
        // get the Init's address and address type as well
        LL_COPY_DEV_ADDR_LE( peerInfo.peerAddr, peerAddr );
        // calculate the advertising interface based on the max/min values
        // ALT: COULD UPDATE WITH ALGO IF NEED BE.
        adv_param.advInterval = advIntervalMin;
    }
    else // undirected, discoverable, or non-connectable
    {
        // calculate the advertising interface based on the max/min values
        // ALT: COULD UPDATE WITH ALGO IF NEED BE.
        adv_param.advInterval = advIntervalMin;
    }

    // mapping from adv event type to packet header
    switch (adv_param.advEvtType)
    {
    case LL_ADV_CONNECTABLE_UNDIRECTED_EVT:
        pduType = ADV_IND;
        break;

    case LL_ADV_CONNECTABLE_HDC_DIRECTED_EVT:
    case LL_ADV_CONNECTABLE_LDC_DIRECTED_EVT:
        pduType = ADV_DIRECT_IND;
        break;

    case LL_ADV_NONCONNECTABLE_UNDIRECTED_EVT:
        pduType = ADV_NONCONN_IND;
        break;

    case LL_ADV_SCANNABLE_UNDIRECTED_EVT:
        pduType = ADV_SCAN_IND;
        break;

    default:
        // should not come here, sanity check in the function start. set default value to suppress warning
        pduType = ADV_IND;
        break;
    }

    SET_BITS(g_tx_adv_buf.txheader, pduType, PDU_TYPE_SHIFT, PDU_TYPE_MASK);
    SET_BITS(g_tx_adv_buf.txheader, g_currentLocalAddrType, TX_ADD_SHIFT, TX_ADD_MASK);

//  SET_BITS(g_tx_adv_buf.txheader, peerInfo.peerAddrType, RX_ADD_SHIFT, RX_ADD_MASK);   // RxAdd need't set
    if ((advEvtType == LL_ADV_CONNECTABLE_UNDIRECTED_EVT
            || advEvtType == LL_ADV_CONNECTABLE_HDC_DIRECTED_EVT
            || advEvtType == LL_ADV_CONNECTABLE_LDC_DIRECTED_EVT)
            && pGlobal_config[LL_SWITCH] & CONN_CSA2_ALLOW)
        SET_BITS(g_tx_adv_buf.txheader, 1, CHSEL_SHIFT, CHSEL_MASK);

    osal_memcpy( g_tx_adv_buf.data,  adv_param.ownAddr, 6);
    SET_BITS(tx_scanRsp_desc.txheader, ADV_SCAN_RSP, PDU_TYPE_SHIFT, PDU_TYPE_MASK);
    SET_BITS(tx_scanRsp_desc.txheader, g_currentLocalAddrType, TX_ADD_SHIFT, TX_ADD_MASK);
    osal_memcpy( tx_scanRsp_desc.data, adv_param.ownAddr, 6);
    // adv length should be set for not direct adv type, 2018-04-05
    SET_BITS(g_tx_adv_buf.txheader, (adv_param.advDataLen + 6), LENGTH_SHIFT, LENGTH_MASK);

    // for direct adv, copy the peer address to PDU
    if(pduType == ADV_DIRECT_IND )
    {
        uint8 useRpa = FALSE;
        SET_BITS(g_tx_adv_buf.txheader, 12, LENGTH_SHIFT, LENGTH_MASK);
//    SET_BITS(tx_scanRsp_desc.txheader, peerInfo.peerAddrType, RX_ADD_SHIFT, RX_ADD_MASK);

        // search the resolving list to get local IRK
        if (ll_readPeerIRK(&peerIrk, peerAddr, peerAddrType) == TRUE)
        {
            if (!ll_isIrkAllZero(peerIrk))            // for all-zero local IRK, not RPA used
            {
                if (ll_CalcRandomAddr(peerIrk, resolve_address) == SUCCESS)
                {
                    useRpa = TRUE;
                    osal_memcpy((uint8_t*) &(g_tx_adv_buf.data[6]), resolve_address, 6);
//                    osal_memcpy( &g_currentPeerRpa[0],  resolve_address, 6);
                }
            }
        }

        if (useRpa == FALSE)
            osal_memcpy((uint8_t*) &(g_tx_adv_buf.data[6]), peerInfo.peerAddr, 6);
    }

    //     ======================== add by HZF, init ll state so that adv PDU could be changed
    if (llState != LL_STATE_CONN_MASTER && llState != LL_STATE_CONN_SLAVE)
    {
        switch(adv_param .advEvtType)
        {
        case LL_ADV_CONNECTABLE_UNDIRECTED_EVT:
            llState=LL_STATE_ADV_UNDIRECTED;
            ll_debug_output(DEBUG_LL_STATE_ADV_UNDIRECTED);
            break;

        case LL_ADV_CONNECTABLE_HDC_DIRECTED_EVT:
        case LL_ADV_CONNECTABLE_LDC_DIRECTED_EVT:
            llState=LL_STATE_ADV_DIRECTED;
            ll_debug_output(DEBUG_LL_STATE_ADV_DIRECTED);
            break;

        case LL_ADV_NONCONNECTABLE_UNDIRECTED_EVT:
            llState=LL_STATE_ADV_NONCONN;
            ll_debug_output(DEBUG_LL_STATE_ADV_NONCONN);
            break;

        case LL_ADV_SCANNABLE_UNDIRECTED_EVT:
            llState=LL_STATE_ADV_SCAN;
            ll_debug_output(DEBUG_LL_STATE_ADV_SCAN);
            break;

        default:
            llState=LL_STATE_IDLE;
            ll_debug_output(DEBUG_LL_STATE_IDLE);
            break;
        }
    }

    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          LL_SetAdvData0 API

    @brief       This API is called by the HCI to set the Advertiser's data.

                Note: If the Advertiser is restarted without intervening calls
                      to this routine to make updates, then the previously
                      defined data will be reused.

                Note: If the data happens to be changed while advertising, then
                      the new data will be sent on the next advertising event.

    input parameters

    @param       advDataLen - The number of scan response bytes: 0..31.
    @param       advData    - Pointer to the advertiser data, or NULL.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER
*/
llStatus_t LL_SetAdvData0( uint8  advDataLen,
                           uint8* advData )
{
    if (g_llAdvMode == LL_MODE_EXTENDED )
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llAdvMode = LL_MODE_LEGACY;

    // check that data length isn't greater than the max allowed size
    if ( advDataLen > LL_MAX_ADV_DATA_LEN )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // save advertiser data length
    adv_param.advDataLen = advDataLen;

    // check if there's supposed to be data
    if ( advDataLen > 0 )
    {
        // yes, so make sure we have a valid pointer
        if ( advData == NULL )
        {
            return( LL_STATUS_ERROR_BAD_PARAMETER );
        }
        else // okay to go
        {
            // save advertiser data
            //osal_memcpy( (uint8_t *)adv_param.advData, advData, adv_param.advDataLen );                   // save adv data
            osal_memcpy( (uint8_t*) &(g_tx_adv_buf.data[6]), advData, adv_param.advDataLen );     // write adv to tx buffer, change it ?? ... HZF
        }
    }

// set tx buffer, to be changed
//   SET_BITS(g_tx_adv_buf.txheader, peerInfo .peerAddrType, RX_ADD_SHIFT, RX_ADD_MASK);
    // osal_memcpy(g_tx_adv_buf.data,  adv_param.ownAddr, 6);
    SET_BITS(g_tx_adv_buf.txheader, (adv_param.advDataLen+6), LENGTH_SHIFT, LENGTH_MASK);
    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    This API is called by the HCI to request the Controller to start or stop
    advertising.

    Public function defined in ll.h.
*/
llStatus_t LL_SetAdvControl0( uint8 advMode )
{
    if (g_llAdvMode == LL_MODE_EXTENDED )
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llAdvMode = LL_MODE_LEGACY;

    // check if a direct test mode or modem test is in progress
    if ( (llState == LL_STATE_DIRECT_TEST_MODE_TX) ||
            (llState == LL_STATE_DIRECT_TEST_MODE_RX) ||
            (llState == LL_STATE_MODEM_TEST_TX)       ||
            (llState == LL_STATE_MODEM_TEST_RX)       ||
            (llState == LL_STATE_MODEM_TEST_TX_FREQ_HOPPING) )
    {
        return( LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE );
    }

    // sanity checks again to be sure we don't start with bad parameters
    if ( ( (adv_param.advEvtType != LL_ADV_CONNECTABLE_UNDIRECTED_EVT)     &&
            (adv_param.advEvtType != LL_ADV_CONNECTABLE_HDC_DIRECTED_EVT)   &&
            (adv_param.advEvtType != LL_ADV_NONCONNECTABLE_UNDIRECTED_EVT)  &&
            (adv_param.advEvtType != LL_ADV_SCANNABLE_UNDIRECTED_EVT)       &&
            (adv_param.advEvtType != LL_ADV_CONNECTABLE_LDC_DIRECTED_EVT) )        ||
            ( (adv_param.ownAddrType != LL_DEV_ADDR_TYPE_PUBLIC)              &&
              (adv_param.ownAddrType != LL_DEV_ADDR_TYPE_RANDOM)              &&
              (adv_param.ownAddrType != LL_DEV_ADDR_TYPE_RPA_PUBLIC)          &&
              (adv_param.ownAddrType != LL_DEV_ADDR_TYPE_RPA_RANDOM))                ||
            ( ((adv_param.advEvtType == LL_ADV_NONCONNECTABLE_UNDIRECTED_EVT)        ||
               (adv_param.advEvtType == LL_ADV_SCANNABLE_UNDIRECTED_EVT))     &&
              (adv_param.advInterval < LL_ADV_CONN_INTERVAL_MIN) ) )     // should use LL_ADV_NONCONN_INTERVAL_MIN after update it to 20ms
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    #ifdef DEBUG_LL
    LOG("llState = %d\n", llState);
    #endif

    // check if we should begin advertising
    switch( advMode )
    {
    // Advertisment Mode is On
    case LL_ADV_MODE_ON:

        // check if command makes sense
        if ( adv_param.advMode == LL_ADV_MODE_ON )
        {
            // this is unexpected; something is wrong
            return( LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE );
        }

        // llState changed when configure adv parameters
        if (llState == LL_STATE_ADV_UNDIRECTED
                || llState == LL_STATE_ADV_DIRECTED
                || llState == LL_STATE_ADV_NONCONN
                || llState == LL_STATE_ADV_SCAN )     // TODO: check this setting
        {
            g_llHdcDirAdvTime = 0;    // for HDC direct adv
            adv_param.advNextChan = LL_ADV_CHAN_LAST + 1;       // set adv channel invalid

            if ( llSetupAdv() != LL_STATUS_SUCCESS )
            {
                // indicate advertising is no longer active
                adv_param.advMode = LL_ADV_MODE_OFF;
                return( LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE );
            }
        }
        // add in A2, simultaneous conn event & scan/adv event
        else if((llState == LL_STATE_CONN_SLAVE
                 || llState == LL_STATE_CONN_MASTER)
                && (pGlobal_config[LL_SWITCH] & SIMUL_CONN_ADV_ALLOW))
        {
            #ifdef DEBUG_LL
            LOG("LL_SetAdvControl: start sec adv\r\n");
            #endif

            if (llSecondaryState != LL_SEC_STATE_IDLE)
                return( LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE );

            // adv event check
            if (adv_param.advEvtType  != LL_ADV_NONCONNECTABLE_UNDIRECTED_EVT
                    && adv_param.advEvtType != LL_ADV_SCANNABLE_UNDIRECTED_EVT
                    && adv_param.advEvtType != LL_ADV_CONNECTABLE_UNDIRECTED_EVT)
                return( LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE );

            // Note: we may need maximum slave number check here. If number of slave reach ceil,
            //       only no-connectable adv is allowed. The checking could be don't in host
            llSecondaryState = LL_SEC_STATE_ADV;
            adv_param.advNextChan = LL_ADV_CHAN_LAST + 1;        // set adv channel invalid
            osal_stop_timerEx( LL_TaskID, LL_EVT_SECONDARY_ADV );
            osal_set_event(LL_TaskID, LL_EVT_SECONDARY_ADV);     // set adv event
        }
        else           // other state
            return (LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE);

        // indicate advertising is no longer active
        adv_param.advMode = LL_ADV_MODE_ON;

        if (g_llRlDeviceNum > 0)
            osal_start_timerEx( LL_TaskID, LL_EVT_RPA_TIMEOUT, g_llRlTimeout * 1000 );

        break;

    case LL_ADV_MODE_OFF:
        // check if command makes sense
//            if ( adv_param.advMode == LL_ADV_MODE_OFF )
//            {
//                // this is unexpected; something is wrong
//                return( LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE );
//            }
        HAL_ENTER_CRITICAL_SECTION();
        // free the associated task block
        //llFreeTask( &advInfo.llTask );
        // indicate we are no longer actively advertising
        adv_param.advMode = LL_ADV_MODE_OFF;

        if (llState != LL_STATE_CONN_SLAVE &&
                llState != LL_STATE_CONN_MASTER)      // no conn + adv case
        {
            llState = LL_STATE_IDLE;     // if not in connect state, set idle to disable advertise
            //ZQ 20190912
            //stop ll timer when idle, considering the scan-adv interleve case
            extern void clear_timer(AP_TIM_TypeDef* TIMx);
            clear_timer(AP_TIM1);
            ll_debug_output(DEBUG_LL_STATE_IDLE);
        }
        else                       // conn + adv case
        {
            uint8 i;
            i = 0;

            while (!(adv_param.advChanMap & (1 << i)))   i ++;    // get the 1st adv channel in the adv channel map

            if ((llSecondaryState == LL_SEC_STATE_ADV)
                    && (adv_param.advNextChan != (LL_ADV_CHAN_FIRST + i)))      // last adv event is not finished
                llSecondaryState = LL_SEC_STATE_IDLE_PENDING;
            else
            {
                llSecondaryState = LL_SEC_STATE_IDLE;
                osal_stop_timerEx( LL_TaskID, LL_EVT_SECONDARY_ADV );    // stop timer
            }
        }

        HAL_EXIT_CRITICAL_SECTION();
        osal_stop_timerEx(LL_TaskID, LL_EVT_RPA_TIMEOUT);
        break;

    default:
        // we have an invalid value for advertisement mode
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    This API is called by the LL or HCI to check if a connection given by the
    connection handle is active.

    Public function defined in ll.h.
*/
llStatus_t LL_ConnActive( uint16 connId )
{
    // check if the connection handle is valid
    if (connId >= g_maxConnNum )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // check if the connection is active
    if ( conn_param[connId].active == FALSE )
    {
        return( LL_STATUS_ERROR_INACTIVE_CONNECTION );
    }

    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    This API is called by the HCI to read the peer controller's Version
    Information. If the peer's Version Information has already been received by
    its request for our Version Information, then this data is already cached and
    can be directly returned to the Host. If the peer's Version Information is
    not already cached, then it will be requested from the peer, and when
    received, returned to the Host via the LL_ReadRemoteVersionInfoCback
    callback.

    Note: Only one Version Indication is allowed for a connection.

    Public function defined in ll.h.
*/
llStatus_t LL_ReadRemoteVersionInfo( uint16 connId )
{
    llStatus_t    status;
    llConnState_t* connPtr;

    // make sure connection ID is valid
    if ( (status=LL_ConnActive(connId)) != LL_STATUS_SUCCESS )
    {
        return( status );
    }

    // get connection info
    connPtr = &conn_param[connId];

    // first, make sure the connection is still active
    if ( !connPtr->active )
    {
        return( LL_STATUS_ERROR_INACTIVE_CONNECTION );
    }

    // check if the peer's version information has already been obtained
    if ( connPtr->verExchange.peerInfoValid == TRUE )
    {
        // yes it has, so provide it to the host
        LL_ReadRemoteVersionInfoCback( LL_STATUS_SUCCESS,
                                       connId,
                                       connPtr->verInfo.verNum,
                                       connPtr->verInfo.comId,
                                       connPtr->verInfo.subverNum );
    }
    else // no it hasn't, so...
    {
        // ...check if the host has already requested this information
        if ( connPtr->verExchange.hostRequest == FALSE )
        {
            // no, so request it by queueing the control packet for processing
            llEnqueueCtrlPkt( connPtr, LL_CTRL_VERSION_IND );
            // set the flag to indicate the host has requested this information
            connPtr->verExchange.hostRequest = TRUE;
        }
        else // previously requested
        {
            return( LL_STATUS_ERROR_VER_INFO_REQ_ALREADY_PENDING );
        }
    }

    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          LL_ClearWhiteList0 API

    @brief       This API is called by the HCI to clear the White List.

                Note: If Scanning is enabled using filtering, and the white
                      list policy is "Any", then this command will be
                      disallowed.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS
*/
llStatus_t LL_ClearWhiteList0( void )
{
    llStatus_t status;
    int i, j;

    // check that it is okay to use the white list
    if ( (status = llCheckWhiteListUsage()) != LL_STATUS_SUCCESS )
    {
        return( status );
    }

    // clear number of entries, valid flags, address type flags, and entries
    for (i = 0; i < LL_WHITELIST_ENTRY_NUM; i ++)
    {
        g_llWhitelist[i].peerAddrType = 0xff;

        for (j = 0; j < LL_DEVICE_ADDR_LEN; j ++)
            g_llWhitelist[i].peerAddr[j] = 0;
    }

    // set white list number 0
    g_llWlDeviceNum = 0;
    return( LL_STATUS_SUCCESS );
}


/*******************************************************************************
    @fn          LL_AddWhiteListDevice0 API

    @brief       This API is called by the HCI to add a device address and its
                type to the White List.

    input parameters

    @param       devAddr      - Pointer to a 6 byte device address.
    @param       addrType     - Public or Random device address.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER,
                LL_STATUS_ERROR_WL_TABLE_FULL
*/
llStatus_t LL_AddWhiteListDevice0( uint8* devAddr,
                                   uint8 addrType )
{
    llStatus_t  status;
    int i, j;

    // check that it is okay to use the white list
    if ( (status = llCheckWhiteListUsage()) != LL_STATUS_SUCCESS )
    {
        return( status );
    }

    // check the WL device address type
    if ( (addrType != LL_DEV_ADDR_TYPE_PUBLIC) &&
            (addrType != LL_DEV_ADDR_TYPE_RANDOM) )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // check if there was room for the entry
    if (g_llWlDeviceNum >= LL_WHITELIST_ENTRY_NUM)
        return( LL_STATUS_ERROR_WL_TABLE_FULL );

    // add the device to a empty record
    for (i = 0; i < LL_WHITELIST_ENTRY_NUM; i++)
    {
        if (g_llWhitelist[i].peerAddrType == 0xff)       // empty record
        {
            g_llWhitelist[i].peerAddrType = addrType;

            for (j = 0; j < LL_DEVICE_ADDR_LEN; j++)
                g_llWhitelist[i].peerAddr[j] = devAddr[j];

            g_llWlDeviceNum ++;
            break;
        }
    }

    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          LL_RemoveWhiteListDevice0 API

    @brief       This API is called by the HCI to remove a device address and
                it's type from the White List.

    input parameters

    @param       devAddr  - Pointer to a 6 byte device address.
    @param       addrType - Public or Random device address.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER,
                LL_STATUS_ERROR_WL_TABLE_EMPTY,
                LL_STATUS_ERROR_WL_ENTRY_NOT_FOUND
*/
llStatus_t LL_RemoveWhiteListDevice0( uint8* devAddr,
                                      uint8 addrType )
{
    llStatus_t  status;
    int i, j;

    // check that it is okay to use the white list
    if ( (status = llCheckWhiteListUsage()) != LL_STATUS_SUCCESS )
    {
        return( status );
    }

    // check the WL device address type
    if ( (addrType != LL_DEV_ADDR_TYPE_PUBLIC) &&
            (addrType != LL_DEV_ADDR_TYPE_RANDOM) )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // check that there was at least one entry in the table
    if ( g_llWlDeviceNum == 0 )
    {
        return( LL_STATUS_ERROR_WL_TABLE_EMPTY );
    }

    for (i = 0; i < LL_WHITELIST_ENTRY_NUM; i++)
    {
        if (g_llWhitelist[i].peerAddrType == addrType)
        {
            for (j = 0; j < LL_DEVICE_ADDR_LEN; j++)    // check whether the address is the same
            {
                if (g_llWhitelist[i].peerAddr[j] != devAddr[j])
                    break;
            }

            if (j == LL_DEVICE_ADDR_LEN)    // found it
            {
                g_llWhitelist[i].peerAddrType = 0xff;
                g_llWlDeviceNum --;
                break;
            }
        }
    }

    if (i == LL_WHITELIST_ENTRY_NUM)
        return( LL_STATUS_ERROR_WL_ENTRY_NOT_FOUND );

    return( LL_STATUS_SUCCESS );
}


/*******************************************************************************
    This API is called by the HCI to update the Host data channels initiating an
    Update Data Channel control procedure.

    Note: While it isn't specified, it is assumed that the Host
         expects an update channel map on all active connections.

    Note: This LL currently only supports one connection.

    Public function defined in ll.h.
*/
llStatus_t LL_ChanMapUpdate( uint8* chanMap )
{
    uint8 i;

    // make sure we're in Master role
    if ( llState != LL_STATE_CONN_MASTER )
    {
        return( LL_STATUS_ERROR_COMMAND_DISALLOWED );
    }

    // parameter check
    if ( chanMap == NULL )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // ensure non-data channels 37..39 are not set and that the Core spec V4.0
    // requirement of a minimum of two data channels be used is met
    if ( (chanMap[LL_NUM_BYTES_FOR_CHAN_MAP-1] & ~0x1F) ||
            (llAtLeastTwoChans( chanMap ) != TRUE) )
    {
        return( LL_STATUS_ERROR_ILLEGAL_PARAM_COMBINATION );
    }

    // first need to check if any previous channel map update is still pending
    for (i = 0; i < g_maxConnNum; i++)
    {
        if (conn_param [i].active )
        {
            llConnState_t* connPtr = &conn_param[i];

            // check if an update channel map control procedure is already pending
            if ( ((connPtr->ctrlPktInfo.ctrlPktCount > 0) &&
                    (connPtr->ctrlPktInfo.ctrlPkts[0] == LL_CTRL_CHANNEL_MAP_REQ)) ||
                    (connPtr->pendingChanUpdate == TRUE) )
            {
                return( LL_STATUS_ERROR_CTRL_PROC_ALREADY_ACTIVE );
            }
        }
    }

    // save the Host's channel map
    for (i = 0; i < LL_NUM_BYTES_FOR_CHAN_MAP; i++)
    {
        chanMapUpdate.chanMap[i] = chanMap[i];
    }

    // need to issue an update on all active connections, if any
    for (i = 0; i < g_maxConnNum; i++)
    {
        if (conn_param [i].active )
        {
            llConnState_t* connPtr = &conn_param[i];
            // set the relative offset of the number of events for the channel update
            // Note: The absolute event number will be determined at the time the
            //       packet is placed in the TX FIFO.
            // Note: The master should allow a minimum of 6 connection events that the
            //       slave will be listening for before the instant occurs.
            connPtr->chanMapUpdateEvent = (connPtr->curParam.slaveLatency+1) +
                                          LL_INSTANT_NUMBER_MIN;
            // queue control packet for processing
            llEnqueueCtrlPkt( connPtr, LL_CTRL_CHANNEL_MAP_REQ );
        }
    }

    return( LL_STATUS_SUCCESS );
}

llStatus_t LL_PhyUpdate0( uint16 connId )
{
    llStatus_t    status;
    llConnState_t* connPtr;
    uint8 phyMode;

    // make sure connection ID is valid
    if ( (status=LL_ConnActive(connId)) != LL_STATUS_SUCCESS )
    {
        return( status );
    }

    // get connection info
    connPtr = &conn_param[connId ];

    // check if an update control procedure is already pending
    if ( ((connPtr->ctrlPktInfo.ctrlPktCount > 0) &&
            (connPtr->ctrlPktInfo.ctrlPkts[0] == LL_CTRL_PHY_UPDATE_IND)) ||
            (connPtr->pendingPhyModeUpdate == TRUE) )
    {
        return( LL_STATUS_ERROR_CTRL_PROC_ALREADY_ACTIVE );
    }

    // we only support symmetric connection
    // tx rx phy should be same
    if(connPtr->llPhyModeCtrl.req.allPhy==0)
    {
        phyMode = connPtr->llPhyModeCtrl.req.txPhy & connPtr->llPhyModeCtrl.rsp.txPhy;
        phyMode|= connPtr->llPhyModeCtrl.req.rxPhy & connPtr->llPhyModeCtrl.rsp.rxPhy;
    }
    else if(connPtr->llPhyModeCtrl.req.allPhy==1)
    {
        phyMode = connPtr->llPhyModeCtrl.req.rxPhy & connPtr->llPhyModeCtrl.rsp.rxPhy;
    }
    else if(connPtr->llPhyModeCtrl.req.allPhy==2)
    {
        phyMode = connPtr->llPhyModeCtrl.req.txPhy & connPtr->llPhyModeCtrl.rsp.txPhy;
    }
    else
    {
        phyMode=0;
    }

    if(phyMode==0)
    {
        //no change case
        connPtr->phyUpdateInfo.m2sPhy = 0;
        connPtr->phyUpdateInfo.s2mPhy = 0;
    }
    else if(phyMode&LE_2M_PHY)
    {
        connPtr->phyUpdateInfo.m2sPhy = LE_2M_PHY;
        connPtr->phyUpdateInfo.s2mPhy = LE_2M_PHY;
    }
    else if(phyMode&LE_CODED_PHY)
    {
        connPtr->phyUpdateInfo.m2sPhy = LE_CODED_PHY;
        connPtr->phyUpdateInfo.s2mPhy = LE_CODED_PHY;
    }
    else
    {
        //no perferce can not support the tx/rx same time
        connPtr->phyUpdateInfo.m2sPhy = LE_1M_PHY;
        connPtr->phyUpdateInfo.s2mPhy = LE_1M_PHY;
    }

    if(phyMode==0)
    {
        connPtr->phyModeUpdateEvent = 0;
        connPtr->phyUpdateInfo.instant =   connPtr->phyModeUpdateEvent;
    }
    else
    {
        connPtr->phyModeUpdateEvent = (connPtr->curParam.slaveLatency+1) +
                                      LL_INSTANT_NUMBER_MIN;
        connPtr->phyUpdateInfo.instant =   connPtr->phyModeUpdateEvent;
    }

    // queue control packet for processing
    llEnqueueCtrlPkt( connPtr, LL_CTRL_PHY_UPDATE_IND );
    return( LL_STATUS_SUCCESS );
}


/*******************************************************************************
    @fn          LL_ReadRemoteUsedFeatures API

    @brief       This API is called by the Master HCI to initiate a feature
                setup control process.

    input parameters

    @param       connId - The LL connection ID on which to send this data.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS
*/
llStatus_t LL_ReadRemoteUsedFeatures0( uint16 connId )
{
    llStatus_t    status;
    llConnState_t* connPtr;

//  // make sure we're in Master role            // in BLE4.2, it could be send in both master & slave
//  if ( llState != LL_STATE_CONN_MASTER )
//  {
//    return( LL_STATUS_ERROR_COMMAND_DISALLOWED );
//  }

    // make sure connection ID is valid
    if ( (status=LL_ConnActive(connId)) != LL_STATUS_SUCCESS )
    {
        return( status );
    }

    // get connection info
    connPtr = &conn_param[connId ];
    // initiate a Feature Set control procedure
    llEnqueueCtrlPkt( connPtr, LL_CTRL_FEATURE_REQ );
    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          LL_ReadWlSize0 API

    @brief       This API is called by the HCI to get the total number of white
                list entries that can be stored in the Controller.

    input parameters

    @param       None.

    output parameters

    @param       *numEntries - Total number of available White List entries.

    @return      LL_STATUS_SUCCESS
*/
llStatus_t LL_ReadWlSize0( uint8* numEntries )
{
    *numEntries = LL_WHITELIST_ENTRY_NUM;
    return (LL_STATUS_SUCCESS);
}


/*******************************************************************************
    @fn          LL_NumEmptyWlEntries API

    @brief       This API is called by the HCI to get the number of White List
                entries that are empty.

    input parameters

    @param       None.

    output parameters

    @param       *numEmptyEntries - number of empty entries in the White List.

    @return      LL_STATUS_SUCCESS
*/
//extern llStatus_t LL_NumEmptyWlEntries( uint8 *numEmptyEntries );  // Not used by TI code


/*******************************************************************************
    @fn          LL_Encrypt API

    @brief       This API is called by the HCI to request the LL to encrypt the
                data in the command using the key given in the command.

                Note: The parameters are byte ordered MSO to LSO.

    input parameters

    @param       *key           - A 128 bit key to be used to calculate the
                                 session key.
    @param       *plaintextData - A 128 bit block that is to be encrypted.

    output parameters

    @param       *encryptedData - A 128 bit block that is encrypted.

    @param       None.

    @return      LL_STATUS_SUCCESS
*/
llStatus_t LL_Encrypt0( uint8* key,
                        uint8* plaintextData,
                        uint8* encryptedData )
{
    // check parameters
    if ( (key == NULL )          ||
            (plaintextData == NULL) ||
            (encryptedData == NULL) )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // encrypt on behalf of the host
    LL_ENC_AES128_Encrypt( key, plaintextData, encryptedData );
    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    This API is called by the HCI to request the LL Controller to provide a data
    block with random content.

    Note: we use different scheme to TI. Below is from TI notes:
         The HCI spec indicates that the random number
         generation should adhere to one of those specified in FIPS
         PUB 140-2. The Core spec refers specifically to the
         algorithm specified in FIPS PUB 186-2, Appendix 3.1.
         Note that this software only uses the RF hardware to
         generate true random numbers. What's more, if the RF is
         already in use (i.e. overlapped execution), then the use
         of radio to generate true random numbers is prohibited.
         In this case, a pseudo-random blocks of numbers will be
         returned instead.

    Public function defined in ll.h.
*/
llStatus_t LL_Rand( uint8* randData,
                    uint8 dataLen )
{
    uint16 temp_rand;
    uint8* pData = randData;
    uint32 sysTick;

    // check if a DTM or Modem operation is in progress
    if ( (llState == LL_STATE_DIRECT_TEST_MODE_TX) ||
            (llState == LL_STATE_DIRECT_TEST_MODE_RX) ||
            (llState == LL_STATE_MODEM_TEST_TX)       ||
            (llState == LL_STATE_MODEM_TEST_RX)       ||
            (llState == LL_STATE_MODEM_TEST_TX_FREQ_HOPPING) )
    {
        // yes, so sorry, no true random number generation allowed as the radio
        // is in continuous use
        return( LL_STATUS_ERROR_COMMAND_DISALLOWED );
    }

    if (dataLen == 0)
    {
        return (LL_STATUS_ERROR_BAD_PARAMETER);
    }

    // now use timer3 counter
    sysTick = get_timer_count(AP_TIM3);
    srand(sysTick);

    //srand((unsigned)time(&t));
    while (dataLen > 1)
    {
        temp_rand = (uint16)(rand() & 0xffff);
        *(pData ++) = (uint8)((temp_rand & 0xff00) >> 8);
        *(pData ++) = (uint8)(temp_rand & 0xff) ;
        dataLen -= 2;
    }

    if (dataLen == 1)
    {
        temp_rand = (uint16)(rand() & 0xffff);
        *(pData) = (uint8)((temp_rand & 0xff00) >> 8);
    }

    return( LL_STATUS_SUCCESS );
}


/*******************************************************************************
    This API is a generic interface to get a block of pseudo-random numbers.

    Public function defined in ll.h.
*/
llStatus_t LL_PseudoRand( uint8* randData,
                          uint8 dataLen )
{
    return LL_Rand(randData, dataLen);
}



/*******************************************************************************
    @fn          LL_ReadSupportedStates API

    @brief       This function is used to provide the HCI with the Link Layer
                supported states and supported state/role combinations.

    input parameters

    @param       None.

    output parameters

    @param       *states - Eight byte Bit map of supported states/combos.

    @return      LL_STATUS_SUCCESS
*/
// the defination of LL_SET_SUPPORTED_STATES implicly using input parameter "states", not good, but it is TI's
llStatus_t LL_ReadSupportedStates( uint8* states )
{
    // provide supported state/role combinations
    // Note: Upper nibble is byte offset, lower nibble is bit offset.
    LL_SET_SUPPORTED_STATES( LL_SCAN_PASSIVE_STATE );
    LL_SET_SUPPORTED_STATES( LL_SCAN_ACTIVE_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_NONCONN_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_DISCOV_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_NONCONN_SCAN_PASSIVE_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_DISCOV_SCAN_PASSIVE_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_NONCONN_SCAN_ACTIVE_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_DISCOV_SCAN_ACTIVE_STATE );
    LL_SET_SUPPORTED_STATES( LL_INIT_STATE );
    LL_SET_SUPPORTED_STATES( LL_INIT_MASTER_STATE );
    LL_SET_SUPPORTED_STATES( LL_SCAN_PASSIVE_INIT_STATE );
    LL_SET_SUPPORTED_STATES( LL_SCAN_ACTIVE_INIT_STATE );
    LL_SET_SUPPORTED_STATES( LL_SCAN_PASSIVE_MASTER_STATE );
    LL_SET_SUPPORTED_STATES( LL_SCAN_ACTIVE_MASTER_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_NONCONN_INIT_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_DISCOV_INIT_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_NONCONN_MASTER_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_DISCOV_MASTER_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_UNDIRECT_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_HDC_DIRECT_STATE );
    LL_SET_SUPPORTED_STATES( LL_SLAVE_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_LDC_DIRECT_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_UNDIRECT_SCAN_PASSIVE_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_HDC_DIRECT_SCAN_PASSIVE_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_UNDIRECT_SCAN_ACTIVE_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_HDC_DIRECT_SCAN_ACTIVE_STATE );
    LL_SET_SUPPORTED_STATES( LL_SCAN_PASSIVE_SLAVE_STATE );
    LL_SET_SUPPORTED_STATES( LL_SCAN_ACTIVE_SLAVE_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_LDC_DIRECT_SCAN_PASSIVE_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_LDC_DIRECT_SCAN_ACTIVE_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_NONCONN_SLAVE_STATE );
    LL_SET_SUPPORTED_STATES( LL_ADV_DISCOV_SLAVE_STATE );
    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          LL_ReadLocalVersionInfo API

    @brief       This API is called by the HCI to read the controller's
                Version information.

    input parameters

    @param       None.

    output parameters

    @param       verNum    - Version of the Bluetooth Controller specification.
    @param       comId     - Company identifier of the manufacturer of the
                            Bluetooth Controller.
    @param       subverNum - A unique value for each implementation or revision
                            of an implementation of the Bluetooth Controller.

    @return      LL_STATUS_SUCCESS
*/
llStatus_t LL_ReadLocalVersionInfo( uint8*  verNum,
                                    uint16* comId,
                                    uint16* subverNum )
{
    // get the version of this BLE controller
    *verNum = verInfo.verNum;
    // get the company ID of this BLE controller
    *comId = verInfo.comId;
    // get the subversion of this BLE controller
    *subverNum = verInfo.subverNum;
    return( LL_STATUS_SUCCESS );
}


/*******************************************************************************
    @fn          LL_CtrlToHostFlowControl API

    @brief       This function is used to indicate if the LL enable/disable
                receive FIFO processing. This function provides support for
                Controller to Host flow control.

    input parameters

    @param       mode: LL_ENABLE_RX_FLOW_CONTROL, LL_DISABLE_RX_FLOW_CONTROL

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS
*/
llStatus_t LL_CtrlToHostFlowControl( uint8 mode )
{
    if ( mode == LL_ENABLE_RX_FLOW_CONTROL )
    {
        // set flag to indicate flow control is enabled
        rxFifoFlowCtrl = LL_RX_FLOW_CONTROL_ENABLED;
    }
    else if ( mode == LL_DISABLE_RX_FLOW_CONTROL )
    {
        // set flag to indicate flow control is disabled
        rxFifoFlowCtrl = LL_RX_FLOW_CONTROL_DISABLED;
    }
    else // error
    {
        return( LL_STATUS_ERROR_UNEXPECTED_PARAMETER );
    }

    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          LL_ReadTxPowerLevel

    @brief       This function is used to read a connection's current transmit
                power level or the maximum transmit power level.

    input parameters

    @param       connId   - The LL connection handle.
    @param       type     - LL_READ_CURRENT_TX_POWER_LEVEL or
                           LL_READ_MAX_TX_POWER_LEVEL
    @param       *txPower - A signed value from -30..+20, in dBm.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER,
                LL_STATUS_ERROR_PARAM_OUT_OF_RANGE,
                LL_STATUS_ERROR_INACTIVE_CONNECTION
*/
llStatus_t LL_ReadTxPowerLevel0( uint8 connId,
                                 uint8 type,
                                 int8*  txPower )
{
	(void) connId;
    uint8_t txPowerIdx=0;
    int8 txPowerMaping[18]=
    {
        RF_PHY_TX_POWER_EXTRA_MAX,  10,
        RF_PHY_TX_POWER_MAX,         7,
        RF_PHY_TX_POWER_5DBM,        5,
        RF_PHY_TX_POWER_0DBM,        0,
        RF_PHY_TX_POWER_N5DBM,      -5,
        RF_PHY_TX_POWER_N10DBM,     -10,
        RF_PHY_TX_POWER_N15DBM,     -15,
        RF_PHY_TX_POWER_N20DBM,     -20,
        RF_PHY_TX_POWER_MIN,        -40,//[lastIdx-1] should be zero to end mapping search

    };

    if ( txPower == NULL )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // determine which type of TX power level is required
    switch( type )
    {
    case LL_READ_CURRENT_TX_POWER_LEVEL:

        // search the mapping table
        while(   txPowerMaping[txPowerIdx]>0
                 &&   txPowerMaping[txPowerIdx]>g_rfPhyTxPower)
        {
            txPowerIdx=txPowerIdx+2;
        }

        // return the TX power level based on current setting
        *txPower =txPowerMaping[txPowerIdx+1];        // assume when g_rfPhyTxPower = 0x1f, tx power = 10dBm,
//      // check if Tx output power is valid
//      if ( *txPower == LL_TX_POWER_INVALID )
//      {
//        return( LL_STATUS_ERROR_PARAM_OUT_OF_RANGE );
//      }
        break;

    case LL_READ_MAX_TX_POWER_LEVEL:
        // return max data channel TX power level
        *txPower = RF_PHY_TX_POWER_MAX;
        break;

    default:
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          LL_SetTxPowerLevel

    @brief       This function is used to set transmit power level

    input parameters

    @param       txPower   - The transmit power level to be set

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER,
                LL_STATUS_ERROR_INACTIVE_CONNECTION
*/
llStatus_t LL_SetTxPowerLevel0( int8  txPower )
{
    // TODO: add tx power range check
    // TODO: add tx power mapping
    g_rfPhyTxPower = txPower;
    rf_phy_set_txPower(g_rfPhyTxPower);
    return LL_STATUS_SUCCESS;
}

/*******************************************************************************
    @fn          LL_ReadChanMap API

    @brief       This API is called by the HCI to read the channel map that the
                LL controller is using for the LL connection.

    input parameters

    @param       connId  - The LL connection handle.

    output parameters

    @param       chanMap - A five byte array containing one bit per data channel
                          where a 1 means the channel is "used" and a 0 means
                          the channel is "unused".

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER,
                LL_STATUS_ERROR_INACTIVE_CONNECTION
*/
llStatus_t LL_ReadChanMap( uint8 connId,
                           uint8* chanMap )
{
    llStatus_t    status;
    llConnState_t* connPtr;

    // make sure connection ID is valid
    if ( (status=LL_ConnActive(connId)) != LL_STATUS_SUCCESS )
    {
        return( status );
    }

    // get connection info
    connPtr = &conn_param[ connId ];
    // copy current channel map
    chanMap[0] = connPtr->chanMap[0];
    chanMap[1] = connPtr->chanMap[1];
    chanMap[2] = connPtr->chanMap[2];
    chanMap[3] = connPtr->chanMap[3];
    chanMap[4] = connPtr->chanMap[4];
    return( LL_STATUS_SUCCESS );
}



/*******************************************************************************
    @fn          LL_ReadRssi API

    @brief       This API is called by the HCI to request RSSI. If there is an
                active connection for the given connection ID, then the RSSI of
                the last received data packet in the LL will be returned. If a
                receiver Modem Test is running, then the RF RSSI for the last
                received data will be returned. If no valid RSSI value is
                available, then LL_RSSI_NOT_AVAILABLE will be returned.

    input parameters

    @param       connId - The LL connection ID on which to read last RSSI.

    output parameters

    @param       *lastRssi - The last data RSSI received.
                            Range: -127dBm..+20dBm, 127=Not Available.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER,
                LL_STATUS_ERROR_INACTIVE_CONNECTION
*/
llStatus_t LL_ReadRssi0( uint16 connId,
                         int8*  lastRssi )
{
    *lastRssi = conn_param[connId].lastRssi ;
    return( LL_STATUS_SUCCESS );
}

llStatus_t LL_ReadFoff( uint16 connId,
                        uint16*  foff )
{
    *foff = conn_param[connId].foff ;
    return( LL_STATUS_SUCCESS );
}

llStatus_t LL_ReadCarrSens( uint16 connId,
                            uint8*  carrSens )
{
    *carrSens = conn_param[connId].carrSens ;
    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    This function is used to initiate a BLE PHY level Transmit Test in Direct
    Test Mode where the DUT generates test reference packets at fixed intervals.
    This test will make use of the nanoRisc Raw Data Transmit and Receive task.

    Note: The BLE device will transmit at maximum power.

    Public function defined in ll.h.
*/
llStatus_t LL_DirectTestTxTest0( uint8 txFreq,
                                 uint8 payloadLen,
                                 uint8 payloadType )
{
	(void) txFreq;
	(void) payloadLen;
	(void) payloadType;
    return( LL_STATUS_SUCCESS );
}


/*******************************************************************************
    This function is used to initiate a BLE PHY level Receive Test in Direct Test
    Mode where the DUT receives test reference packets at fixed intervals. This
    test will make use of the nanoRisc Raw Data Transmit and Receive task. The
    received packets are verified based on the CRC, and metrics are kept.

    Public function defined in ll.h.
*/
llStatus_t LL_DirectTestRxTest0( uint8 rxFreq )
{
	(void) rxFreq;
    return( LL_STATUS_SUCCESS );
}


/*******************************************************************************
    This function is used to end the Direct Test Transmit or Direct Test Receive
    tests executing in Direct Test mode. When the raw task is ended, the
    LL_DirectTestEndDoneCback callback is called. If a Direct Test mode operation
    is not currently active, an error is returned.

    Public function defined in ll.h.
*/
llStatus_t LL_DirectTestEnd0( void )
{
    return( LL_STATUS_SUCCESS );
}



/*******************************************************************************
    @fn          LL_EXT_ConnEventNotice Vendor Specific API

    @brief       This API is called to enable or disable a notification to the
                specified task using the specified task event whenever a
                Connection event ends. A non-zero taskEvent value is taken to
                be "enable", while a zero valued taskEvent is taken to be
                "disable".

                Note: Currently, only a Slave connection is supported.

    input parameters

    @param       taskID    - User's task ID.
    @param       taskEvent - User's task event.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_INACTIVE_CONNECTION,
                LL_STATUS_ERROR_BAD_PARAMETER
*/
llStatus_t LL_EXT_ConnEventNotice( uint8 taskID, uint16 taskEvent )
{
	(void) taskID;
	(void) taskEvent;
    return 0;
}

/*******************************************************************************
    @fn          LL_EXT_DisconnectImmed Vendor Specific API

    @brief       This function is used to disconnect the connection immediately.

                Note: The connection (if valid) is immediately terminated
                      without notifying the remote device. The Host is still
                      notified.

    input parameters

    @param       connId - The LL connection ID on which to send this data.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_INACTIVE_CONNECTION
*/
llStatus_t LL_EXT_DisconnectImmed( uint16 connId )
{
	(void) connId;
    return 0;
}

/*******************************************************************************
    @fn          LL_EXT_NumComplPktsLimit Vendor Specific API

    @brief       This API is used to set the minimum number of
                completed packets which must be met before a Number of
                Completed Packets event is returned. If the limit is not
                reach by the end of the connection event, then a Number of
                Completed Packets event will be returned (if non-zero) based
                on the flushOnEvt flag.

    input parameters

    @param       limit      - From 1 to LL_MAX_NUM_DATA_BUFFERS.
    @param       flushOnEvt - LL_EXT_DISABLE_NUM_COMPL_PKTS_ON_EVENT |
                             LL_EXT_ENABLE_NUM_COMPL_PKTS_ON_EVENT

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_ERROR_CODE_INVALID_HCI_CMD_PARAMS
*/
llStatus_t LL_EXT_NumComplPktsLimit( uint8 limit,
                                     uint8 flushOnEvt )
{
	(void) flushOnEvt;
    if ( (limit == 0) || (limit > g_maxPktPerEventTx) )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // save the limit
    numComplPktsLimit = limit;
    return LL_STATUS_SUCCESS;
}

/*******************************************************************************
    @fn          LL_EXT_OnePacketPerEvent Vendor Specific API

    @brief       This function is used to enable or disable allowing only one
                packet per event.

    input parameters

    @param       control - LL_EXT_ENABLE_ONE_PKT_PER_EVT,
                          LL_EXT_DISABLE_ONE_PKT_PER_EVT

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER
*/
llStatus_t LL_EXT_OnePacketPerEvent( uint8 control )
{
	(void) control;
    return 0;
}

/*******************************************************************************
    @fn          LL_EXT_OverlappedProcessing Vendor Specific API

    @brief       This API is used to enable or disable overlapped processing.

    input parameters

    @param       mode - LL_EXT_ENABLE_OVERLAPPED_PROCESSING |
                       LL_EXT_DISABLE_OVERLAPPED_PROCESSING

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_ERROR_CODE_INVALID_HCI_CMD_PARAMS
*/
llStatus_t LL_EXT_OverlappedProcessing( uint8 mode )
{
	(void) mode;
    return 0;
}

/*******************************************************************************
    @fn          LL_EXT_PERbyChan Vendor Specific API

    @brief       This API is called by the HCI to start or end Packet Error Rate
                by Channel counter accumulation for a connection. If the
                pointer is not NULL, it is assumed there is sufficient memory
                for the PER data, per the type perByChan_t. If NULL, then
                the operation is considered disabled.

                Note: It is the user's responsibility to make sure there is
                      sufficient memory for the data, and that the counters
                      are cleared prior to first use.

                Note: The counters are only 16 bits. At the shortest connection
                      interval, this provides a bit over 8 minutes of data.

    input parameters

    @param       connId    - The LL connection ID on which to send this data.
    @param       perByChan - Pointer to PER by Channel data, or NULL.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_INACTIVE_CONNECTION
*/
llStatus_t LL_EXT_PERbyChan( uint16 connId, perByChan_t* perByChan )
{
	(void) connId;
	(void) perByChan;
    return 0;
}

/*******************************************************************************
    @fn          LL_ReadAdvChanTxPower

    @brief       This function is used to read the transmit power level used
                for BLE advertising channel packets. Currently, only two
                settings are possible, a standard setting of 0 dBm, and a
                maximum setting of 4 dBm.

    input parameters

    @param       *txPower - A non-null pointer.

    output parameters

    @param       *txPower - A signed value from -20..+10, in dBm.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_PARAM_OUT_OF_RANGE
*/
llStatus_t LL_ReadAdvChanTxPower0( int8* txPower )
{
	(void) txPower;
    if (g_llAdvMode == LL_MODE_EXTENDED )
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llAdvMode = LL_MODE_LEGACY;
    return 0;
}

/*******************************************************************************
    @fn          LL_SetScanParam API

    @brief       This API is called by the HCI to set the Scanner's parameters.

    input parameters

    @param       scanType     - Passive or Active scan type.
    @param       scanInterval - Time between scan events.
    @param       scanWindow   - Duration of a scan. When the same as the scan
                               interval, then scan continuously.
    @param       ownAddrType  - Address type (Public or Random) to use in the
                               SCAN_REQ packet.
    @param       advWlPolicy  - Either allow all Adv packets, or only those that
                               are in the white list.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER
*/
llStatus_t LL_SetScanParam0( uint8  scanType,
                             uint16 scanInterval,
                             uint16 scanWindow,
                             uint8  ownAddrType,
                             uint8  scanWlPolicy )
{
    if (g_llScanMode == LL_MODE_EXTENDED )
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llScanMode = LL_MODE_LEGACY;

    // sanity check of parameters
    if ( ( (scanType != LL_SCAN_PASSIVE)    &&
            (scanType != LL_SCAN_ACTIVE) )                         ||
            ( (ownAddrType != LL_DEV_ADDR_TYPE_PUBLIC)      &&
              (ownAddrType != LL_DEV_ADDR_TYPE_RANDOM)      &&
              (ownAddrType != LL_DEV_ADDR_TYPE_RPA_PUBLIC)  &&
              (ownAddrType != LL_DEV_ADDR_TYPE_RPA_RANDOM) )             ||
            ( (scanInterval < LL_SCAN_WINDOW_MIN)                    ||
              (scanInterval > LL_SCAN_WINDOW_MAX) )                  ||
            ( (scanWindow < LL_SCAN_WINDOW_MIN)                      ||
              (scanWindow > LL_SCAN_WINDOW_MAX) )                    ||
            ( (scanWindow > scanInterval) )                          ||
            ( (scanWlPolicy != LL_SCAN_WL_POLICY_ANY_ADV_PKTS)   &&
              (scanWlPolicy != LL_SCAN_WL_POLICY_USE_WHITE_LIST) ) )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // check if scan is active
    if ( scanInfo.scanMode == LL_SCAN_START )
    {
        // yes, so not allowed per the spec
        return( LL_STATUS_ERROR_COMMAND_DISALLOWED );
    }

    // set the scan type
    scanInfo.scanType = scanType;
    // save white list policy
    scanInfo.wlPolicy = scanWlPolicy;

    // set the scanner's address based on the HCI's address type preference
    if ( ownAddrType == LL_DEV_ADDR_TYPE_PUBLIC )
    {
        // get our address and address type
        scanInfo.ownAddrType  = LL_DEV_ADDR_TYPE_PUBLIC;
        LL_COPY_DEV_ADDR_LE( scanInfo.ownAddr, ownPublicAddr );
    }
    else if ( ownAddrType == LL_DEV_ADDR_TYPE_RANDOM )// LL_DEV_ADDR_TYPE_RANDOM
    {
        // get our address and address type
        scanInfo.ownAddrType  = LL_DEV_ADDR_TYPE_RANDOM;
        LL_COPY_DEV_ADDR_LE( scanInfo.ownAddr, ownRandomAddr );
    }
    else
    {
        // for RPAs, scan control not indicate using which RPA list entry, no copy scanA here
        scanInfo.ownAddrType = ownAddrType;
    }

    // set the scan interval
    scanInfo.scanInterval = scanInterval;
    // set the scan window
    scanInfo.scanWindow = scanWindow;
    // set the scan filter policy
//  if ( scanWlPolicy == LL_SCAN_WL_POLICY_ANY_ADV_PKTS )
//  {
////    PHY_SetScanWlPolicy( PHY_SCANNER_ALLOW_ALL_ADV_PKTS );
//  }
//  else if ( scanWlPolicy == LL_SCAN_WL_POLICY_USE_WHITE_LIST )
//  {
////    PHY_SetScanWlPolicy( PHY_SCANNER_USE_WHITE_LIST );
//  }
    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          LL_SetScanControl API

    @brief       This API is called by the HCI to start or stop the Scanner. It
                also specifies whether the LL will filter duplicate advertising
                reports to the Host, or generate a report for each packet
                received.

    input parameters

    @param       scanMode      - LL_SCAN_START or LL_SCAN_STOP.
    @param       filterReports - LL_FILTER_REPORTS_DISABLE or
                                LL_FILTER_REPORTS_ENABLE

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER,
                LL_STATUS_ERROR_UNEXPECTED_PARAMETER,
                LL_STATUS_ERROR_OUT_OF_TX_MEM,
                LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE
*/
llStatus_t LL_SetScanControl0( uint8 scanMode,
                               uint8 filterReports )
{
    if (g_llScanMode == LL_MODE_EXTENDED )
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llScanMode = LL_MODE_LEGACY;

    // check if a direct test mode or modem test is in progress
    if ( (llState == LL_STATE_DIRECT_TEST_MODE_TX) ||
            (llState == LL_STATE_DIRECT_TEST_MODE_RX) ||
            (llState == LL_STATE_MODEM_TEST_TX)       ||
            (llState == LL_STATE_MODEM_TEST_RX)       ||
            (llState == LL_STATE_MODEM_TEST_TX_FREQ_HOPPING) )
    {
        return( LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE );
    }

    // sanity checks again to be sure we don't start with bad parameters
    if ( ( (scanInfo.scanType != LL_SCAN_PASSIVE)   &&
            (scanInfo.scanType != LL_SCAN_ACTIVE))              ||
            ( (scanInfo.ownAddrType != LL_DEV_ADDR_TYPE_PUBLIC) &&
              (scanInfo.ownAddrType != LL_DEV_ADDR_TYPE_RANDOM))  ||
            ( (scanInfo.scanInterval < LL_SCAN_WINDOW_MIN)        ||
              (scanInfo.scanInterval > LL_SCAN_WINDOW_MAX))       ||
            ( (scanInfo.scanWindow < LL_SCAN_WINDOW_MIN)          ||
              (scanInfo.scanWindow > LL_SCAN_WINDOW_MAX))         ||
            ( (scanInfo.scanWindow > scanInfo.scanInterval) )     ||
            ( (filterReports != LL_FILTER_REPORTS_DISABLE)  &&
              (filterReports != LL_FILTER_REPORTS_ENABLE)) )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // check if we should begin scanning
    switch( scanMode )
    {
    // Scanning Mode is On
    case LL_SCAN_START:

        // check if command makes sense
        if ( scanInfo.scanMode == LL_SCAN_START )
        {
            // this is unexpected; something is wrong
            return( LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE );
        }

        // get a task block for this BLE state/role
        // Note: There will always be a valid pointer, so no NULL check required.
//          scanInfo.llTask = llAllocTask( LL_TASK_ID_SCANNER );

        // check if no other tasks are currently active
        if ( llState == LL_STATE_IDLE )
        {
            // indicate Scan has not already been initalized
            scanInfo.initPending = TRUE;
            // save the scan filtering flag
            scanInfo.filterReports = filterReports;
            // add by HZF
            scanInfo.nextScanChan  = LL_SCAN_ADV_CHAN_37;
            // set LL state
            llState = LL_STATE_SCAN;
            // Note: llState has been changed.
            LL_evt_schedule();
        }
        else if ((llState == LL_STATE_CONN_SLAVE
                  || llState == LL_STATE_CONN_MASTER)     // HZF: if we should support adv + scan, add more state here
                 && (pGlobal_config[LL_SWITCH] & SIMUL_CONN_SCAN_ALLOW))
        {
            if (llSecondaryState != LL_SEC_STATE_IDLE)
                return( LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE );

            scanInfo.nextScanChan  = LL_SCAN_ADV_CHAN_37;
            llSecondaryState = LL_SEC_STATE_SCAN;
            osal_set_event(LL_TaskID, LL_EVT_SECONDARY_SCAN);
        }
        else
            return( LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE );

        // indicate we are actively scanning
        scanInfo.scanMode = LL_SCAN_START;
        break;

    case LL_SCAN_STOP:
        HAL_ENTER_CRITICAL_SECTION();

        if (llState == LL_STATE_SCAN)      // no conn + scan case
        {
            llState = LL_STATE_IDLE;     // if not in connect state, set idle to disable scan
            //ZQ 20190912
            //stop ll timer when idle, considering the scan-adv interleve case
            clear_timer(AP_TIM1);
            ll_debug_output(DEBUG_LL_STATE_IDLE);
        }
        else if (llState == LL_STATE_CONN_SLAVE
                 || llState == LL_STATE_CONN_MASTER)                      // conn + scan case
        {
            llSecondaryState = LL_SEC_STATE_IDLE;
        }

        // indicate we are no longer actively scanning
        scanInfo.scanMode = LL_SCAN_STOP;
        // A2 multiconn, should we consider current LL state to avoid change master/slave configuration
        // now LL slave/master event use same parameter 88
        ll_hw_set_rx_timeout(88);
        // HZF: should we stop scan task immediately, or wait scan IRQ then stop? Now use option 2.
        HAL_EXIT_CRITICAL_SECTION();
        break;

    default:
        // we have an invalid value for advertisement mode
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    return( LL_STATUS_SUCCESS );
}


/*******************************************************************************
    This API is called by the HCI to set the Advertiser's Scan Response data.

    Public function defined in ll.h.
*/
llStatus_t LL_SetScanRspData( uint8  scanRspLen,
                              uint8* scanRspData )
{
    if (g_llAdvMode == LL_MODE_EXTENDED )
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llAdvMode = LL_MODE_LEGACY;

    // check that data length isn't greater than the max allowed size
    if ( scanRspLen > LL_MAX_SCAN_DATA_LEN )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // save scan response data length
    adv_param.scanRspLen = scanRspLen;

    // check if there is any scan response data
    if ( scanRspLen > 0 )
    {
        // yes, so make sure we have a valid pointer
        if ( scanRspData == NULL )
        {
            return( LL_STATUS_ERROR_BAD_PARAMETER );
        }
        else // okay to go
        {
            // save scan response data
            osal_memcpy( (uint8_t*) &(tx_scanRsp_desc.data[6]), scanRspData, adv_param.scanRspLen );
        }
    }

    SET_BITS(tx_scanRsp_desc.txheader,  (adv_param .scanRspLen+6), LENGTH_SHIFT, LENGTH_MASK);
    //osal_memcpy( tx_scanRsp_desc.data, adv_param.ownAddr, 6);
    return( LL_STATUS_SUCCESS );
}

#ifdef USE_UNPATCHED
/*******************************************************************************
    @fn          LL_EncLtkReply API

    @brief       This API is called by the HCI to provide the controller with
                the Long Term Key (LTK) for encryption. This command is
                actually a reply to the link layer's LL_EncLtkReqCback, which
                provided the random number and encryption diversifier received
                from the Master during an encryption setup.

                Note: The key parameter is byte ordered LSO to MSO.

    input parameters

    @param       connId - The LL connection ID on which to send this data.
    @param       *key   - A 128 bit key to be used to calculate the session key.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS
*/
llStatus_t LL_EncLtkReply( uint16 connId,
                           uint8*  key )
{
    uint8         i;
    llStatus_t    status;
    llConnState_t* connPtr;

    // make sure we're in Master role
    if ( llState != LL_STATE_CONN_SLAVE )
    {
        return( LL_STATUS_ERROR_COMMAND_DISALLOWED );
    }

    // check parameters
    if ( key == NULL )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // make sure connection ID is valid
    if ( (status=LL_ConnActive(connId)) != LL_STATUS_SUCCESS )
    {
        return( status );
    }

    // get connection info
    connPtr = &conn_param[ connId ];

    // ALT: COULD MAKE THIS PER CONNECTION.

    // save LTK
    for (i=0; i<LL_ENC_LTK_LEN; i++)
    {
        // store LTK in MSO..LSO byte order, per FIPS 197 (AES)
        connPtr->encInfo.LTK[(LL_ENC_LTK_LEN-i)-1] = key[i];
    }

    // indicate the host has provided the key
    connPtr->encInfo.LTKValid = TRUE;
    // got the LTK, so schedule the start of encryption
    // ALT: COULD MAKE THIS A REPLACE IF A DUMMY IS SITTING AT THE HEAD OF
    //      THE QUEUE.
    llEnqueueCtrlPkt( connPtr, LL_CTRL_START_ENC_REQ );
    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          LL_EncLtkNegReply API

    @brief       This API is called by the HCI to indicate to the controller
                that the Long Term Key (LTK) for encryption can not be provided.
                This command is actually a reply to the link layer's
                LL_EncLtkReqCback, which provided the random number and
                encryption diversifier received from the Master during an
                encryption setup. How the LL responds to the negative reply
                depends on whether this is part of a start encryption or a
                re-start encryption after a pause. For the former, an
                encryption request rejection is sent to the peer device. For
                the latter, the connection is terminated.

    input parameters

    @param       connId - The LL connection ID on which to send this data.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS
*/
llStatus_t LL_EncLtkNegReply( uint16 connId )
{
    llStatus_t    status;
    llConnState_t* connPtr;

    // make sure we're in Master role
    if ( llState != LL_STATE_CONN_SLAVE )
    {
        return( LL_STATUS_ERROR_COMMAND_DISALLOWED );
    }

    // make sure connection ID is valid
    if ( (status=LL_ConnActive(connId)) != LL_STATUS_SUCCESS )
    {
        return( status );
    }

    // get connection info
    connPtr = &conn_param[ connId ];

    // check if this is during a start or a re-start encryption procedure
    if ( connPtr->encInfo.encRestart == TRUE )
    {
        // indicate the peer requested this termination
        connPtr->termInfo.reason = LL_ENC_KEY_REQ_REJECTED;
        // queue control packet for processing
        // ALT: COULD MAKE THIS A REPLACE IF A DUMMY IS SITTING AT THE HEAD OF
        //      THE QUEUE.
        //llReplaceCtrlPkt( connPtr, LL_CTRL_TERMINATE_IND );
        llEnqueueCtrlPkt( connPtr, LL_CTRL_TERMINATE_IND );
    }
    else // during a start encryption
    {
        // set the encryption rejection error code
        connPtr->encInfo.encRejectErrCode = LL_STATUS_ERROR_PIN_OR_KEY_MISSING; // same as LL_ENC_KEY_REQ_REJECTED
        // and reject the encryption request
        // ALT: COULD MAKE THIS A REPLACE IF A DUMMY IS SITTING AT THE HEAD OF
        //      THE QUEUE.
        //llReplaceCtrlPkt( connPtr, LL_CTRL_REJECT_IND );
        llEnqueueCtrlPkt( connPtr, LL_CTRL_REJECT_IND );
    }

    return( LL_STATUS_SUCCESS );
}
#endif // USE_UNPATCHED

/*******************************************************************************
    This API is called by the HCI to read the controller's own public device
    address.

    Note: The device's address is stored in NV memory.

    Public function defined in ll.h.
*/
llStatus_t LL_ReadBDADDR( uint8* bdAddr )
{
    // return own public device address LSO..MSO
    bdAddr[0] = ownPublicAddr[0];
    bdAddr[1] = ownPublicAddr[1];
    bdAddr[2] = ownPublicAddr[2];
    bdAddr[3] = ownPublicAddr[3];
    bdAddr[4] = ownPublicAddr[4];
    bdAddr[5] = ownPublicAddr[5];
    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    This API is called by the HCI to read the controller's Feature Set. The
    Controller indicates which features it supports.

    Public function defined in ll.h.
*/
llStatus_t LL_ReadLocalSupportedFeatures( uint8* featureSet )
{
    uint8 i;

    // copy Feature Set
    for (i=0; i<LL_MAX_FEATURE_SET_SIZE; i++)
    {
        featureSet[i] = deviceFeatureSet.featureSet[i];
    }

    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    This function is used to save this device's random address. It is provided by
    the Host for devices that are unable to store an IEEE assigned public address
    in NV memory.

    Public function defined in ll.h.
*/
llStatus_t LL_SetRandomAddress( uint8* devAddr )
{
//    // add for BQB test 2018-9-20, LL/SEC/ADV/BV-01-C
//  // check if advertising is active
//  if ( adv_param.advMode == LL_ADV_MODE_ON )
//  {
//    // yes, so not allowed per the spec
//    return( LL_STATUS_ERROR_COMMAND_DISALLOWED );
//  }
    if ( (llState == LL_STATE_ADV_UNDIRECTED) ||
            (llState == LL_STATE_ADV_DIRECTED)   ||
            (llState == LL_STATE_ADV_SCAN)       ||
            (llState == LL_STATE_ADV_NONCONN)    ||
            (llState == LL_STATE_SCAN)           ||
            (llState == LL_STATE_INIT)           ||
            (llSecondaryState == LL_SEC_STATE_ADV)          ||
            (llSecondaryState == LL_SEC_STATE_ADV_PENDING)  ||
            (llSecondaryState == LL_SEC_STATE_SCAN)         ||
            (llSecondaryState == LL_SEC_STATE_SCAN_PENDING) ||
            (llSecondaryState == LL_SEC_STATE_INIT)         ||
            (llSecondaryState == LL_SEC_STATE_INIT_PENDING))
    {
        return( LL_STATUS_ERROR_COMMAND_DISALLOWED );
    }

    // store our random address LSO..MSO
    ownRandomAddr[0] = devAddr[0];
    ownRandomAddr[1] = devAddr[1];
    ownRandomAddr[2] = devAddr[2];
    ownRandomAddr[3] = devAddr[3];
    ownRandomAddr[4] = devAddr[4];
    ownRandomAddr[5] = devAddr[5];
    return( LL_STATUS_SUCCESS );
}


/*******************************************************************************
    This API is called by the HCI to update the connection parameters by
    initiating a connection update control procedure.

    Public function defined in ll.h.
*/
llStatus_t LL_ConnUpdate( uint16 connId,
                          uint16 connIntervalMin,
                          uint16 connIntervalMax,
                          uint16 connLatency,
                          uint16 connTimeout,
                          uint16 minLength,
                          uint16 maxLength )
{
    llStatus_t    status;
    llConnState_t* connPtr;
    // unused input parameter; PC-Lint error 715.
    (void)minLength;
    (void)maxLength;

    // make sure we're in Master role
    if ( llState != LL_STATE_CONN_MASTER )
    {
        return( LL_STATUS_ERROR_COMMAND_DISALLOWED );
    }

    // sanity checks again to be sure we don't start with bad parameters
    if ( LL_INVALID_CONN_TIME_PARAM( connIntervalMin,
                                     connIntervalMax,
                                     connLatency,
                                     connTimeout ) )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // make sure connection ID is valid
    if ( (status=LL_ConnActive(connId)) != LL_STATUS_SUCCESS )
    {
        return( status );
    }

    // get connection info
    connPtr = &conn_param[connId];

    // check if an updated parameters control procedure is already what's pending
    if ( ((connPtr->ctrlPktInfo.ctrlPktCount > 0) &&
            (connPtr->ctrlPktInfo.ctrlPkts[0] == LL_CTRL_CONNECTION_UPDATE_REQ)) ||
            (connPtr->pendingParamUpdate == TRUE) )
    {
        return( LL_STATUS_ERROR_CTRL_PROC_ALREADY_ACTIVE );
    }

    // check if CI/SL/LSTO is valid (i.e. meets the requirements)
    // Note: LSTO > (1 + Slave Latency) * (Connection Interval * 2)
    // Note: The CI * 2 requirement based on ESR05 V1.0, Erratum 3904.
    // Note: LSTO time is normalized to units of 1.25ms (i.e. 10ms = 8 * 1.25ms).
    if ( LL_INVALID_CONN_TIME_PARAM_COMBO(connIntervalMax, connLatency, connTimeout) )
    {
        return( LL_STATUS_ERROR_ILLEGAL_PARAM_COMBINATION );
    }

    // if there is at least one connection, make sure this connection interval
    // is a multiple/divisor of all other active connection intervals; also make
    // sure that this connection's interval is not less than the allowed maximum
    // connection interval as determined by the maximum number of allowed
    // connections times the number of slots per connection.
    if ( g_ll_conn_ctx.numLLMasterConns > 0 )        //   if ( g_ll_conn_ctx.numLLConns > 0 )
    {
        uint16 connInterval = (connIntervalMax << 1);      // convert to 625us ticks
        uint16 minCI        = g_ll_conn_ctx.connInterval;

        //    // first check if this connection interval is even legal
        //    // Note: The number of active connections is limited by the minCI.
        //    if ( (minCI / NUM_SLOTS_PER_MASTER) < llConns.numActiveConns )
        //    {
        //      return( LL_STATUS_ERROR_UNACCEPTABLE_CONN_INTERVAL );
        //    }

        //    // does the CI need to be checked as a multiple of the minCI?
        if ( connInterval >= minCI )
        {
            // check if this connection's CI is valid (i.e. a multiple of minCI)
            if ( connInterval % minCI )
            {
                return( LL_STATUS_ERROR_UNACCEPTABLE_CONN_INTERVAL );
            }
        }
        else
            return( LL_STATUS_ERROR_UNACCEPTABLE_CONN_INTERVAL );
    }

    // no control procedure currently active, so set this one up
    // set the window size (units of 1.25ms)
    connPtr->paramUpdate.winSize = LL_WINDOW_SIZE;
    // set the window offset (units of 1.25ms)
//  connPtr->paramUpdate.winOffset = LL_WINDOW_OFFSET;
    connPtr->paramUpdate.winOffset = 0;                     // multiconnection, this value could be 0 or x * old conn interval and should be less than new conn interval
    // set the relative offset of the number of events for the parameter update
    // Note: The absolute event number will be determined at the time the packet
    //       is placed in the TX FIFO.
    // Note: The master should allow a minimum of 6 connection events that the
    //       slave will be listening for before the instant occurs.
    connPtr->paramUpdateEvent = (connPtr->curParam.slaveLatency+1) +
                                LL_INSTANT_NUMBER_MIN;
    // determine the connection interval based on min and max values
    // Note: Range not used, so assume max value.
    // Note: minLength and maxLength are informational.
    connPtr->paramUpdate.connInterval = connIntervalMax;
    // save the new connection slave latency to be used by the peer
    connPtr->paramUpdate.slaveLatency = connLatency;
    // save the new connection supervisor timeout
    connPtr->paramUpdate.connTimeout  = connTimeout;
    // queue control packet for processing
    llEnqueueCtrlPkt( connPtr, LL_CTRL_CONNECTION_UPDATE_REQ );
    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    This API is called by the HCI to create a connection.

    Public function defined in ll.h.
*/
// TODO: check the usage of new enum value of  ownAddrType/peerAddrType
llStatus_t LL_CreateConn0( uint16 scanInterval,
                           uint16 scanWindow,
                           uint8  initWlPolicy,
                           uint8  peerAddrType,
                           uint8*  peerAddr,
                           uint8  ownAddrType,
                           uint16 connIntervalMin,
                           uint16 connIntervalMax,
                           uint16 connLatency,
                           uint16 connTimeout,
                           uint16 minLength,        //  minimum length of connection needed for this LE conn, no use now
                           uint16 maxLength )       //  maximum length of connection needed for this LE conn, no use now
{
    uint8          i;
    llConnState_t* connPtr;
    uint16         txHeader = 0x2205;           // header for CONNECT REQ message, length: 0x22, PDU type: 0x5, TxAdd & RxAdd to be set below

    if (g_llScanMode == LL_MODE_EXTENDED )
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llScanMode = LL_MODE_LEGACY;

    //---------------------------------------------------
    // protect for the LL_HW re-trigger
    // 20190115 ZQ
    //    if(llWaitingIrq==TRUE)                             // HZF: temp comment out
//    {
//        return(LL_STATUS_WARNING_WAITING_LLIRQ);
//    }
    // ================== sanity check =================
    // check if a direct test mode or modem test is in progress
    if ( (llState == LL_STATE_DIRECT_TEST_MODE_TX)           ||
            (llState == LL_STATE_DIRECT_TEST_MODE_RX)           ||
            (llState == LL_STATE_MODEM_TEST_TX)                 ||
            (llState == LL_STATE_MODEM_TEST_RX)                 ||
            (llState == LL_STATE_MODEM_TEST_TX_FREQ_HOPPING) )
    {
        return( LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE );
    }

    // sanity checks again to be sure we don't start with bad parameters
    if ( ( (scanInterval < LL_SCAN_INTERVAL_MIN)              ||
            (scanInterval > LL_SCAN_INTERVAL_MAX) )            ||
            ( (scanWindow < LL_SCAN_INTERVAL_MIN)                ||
              (scanWindow > LL_SCAN_INTERVAL_MAX) )              ||
            ( (scanWindow > scanInterval) )                      ||
            ( (initWlPolicy != LL_INIT_WL_POLICY_USE_PEER_ADDR)  &&
              (initWlPolicy != LL_INIT_WL_POLICY_USE_WHITE_LIST) ) ||
            ( (initWlPolicy == LL_INIT_WL_POLICY_USE_PEER_ADDR)  &&
              (peerAddr == NULL) )                               ||
            ( (peerAddrType != LL_DEV_ADDR_TYPE_PUBLIC)          &&
              (peerAddrType != LL_DEV_ADDR_TYPE_RANDOM)          &&
              (peerAddrType != LL_DEV_ADDR_TYPE_RPA_RANDOM)          &&
              (peerAddrType != LL_DEV_ADDR_TYPE_RPA_PUBLIC))        ||
            ( (ownAddrType != LL_DEV_ADDR_TYPE_PUBLIC)            &&
              (ownAddrType != LL_DEV_ADDR_TYPE_RANDOM)            &&
              (ownAddrType != LL_DEV_ADDR_TYPE_RPA_RANDOM)        &&
              (ownAddrType != LL_DEV_ADDR_TYPE_RPA_PUBLIC))         ||
            ( (maxLength < minLength) ) )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // The Host shall not set Peer_Address_Type to either 0x02 or 0x03 if both the Host and the Controller
    // support the HCI_LE_Set_Privacy_Mode command. If a Controller that supports the HCI_LE_Set_Privacy_Mode
    // command receives the HCI_LE_Create_Connection command with Peer_Address_Type set to either
    // 0x02 or 0x03, it may use either device privacy mode or network privacy mode for that peer device.
//    if(((peerAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM) ||
//           (peerAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC))  ||
//        ((ownAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM)  ||
//         (ownAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC)))
//    {         // these values shall only be used by the Host if either the Host or the Controller does not support the HCI_LE_Set_Privacy_Mode command
//        return( LL_STATUS_ERROR_BAD_PARAMETER );
//    }

    // sanity checks again to be sure we don't start with bad parameters
    if ( LL_INVALID_CONN_TIME_PARAM( connIntervalMin,
                                     connIntervalMax,
                                     connLatency,
                                     connTimeout ) )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // check if CI/SL/LSTO is valid (i.e. meets the requirements)
    // Note: LSTO > (1 + Slave Latency) * (Connection Interval * 2)
    // Note: The CI * 2 requirement based on ESR05 V1.0, Erratum 3904.
    // Note: LSTO time is normalized to units of 1.25ms (i.e. 10ms = 8 * 1.25ms).
    if ( LL_INVALID_CONN_TIME_PARAM_COMBO(connIntervalMax, connLatency, connTimeout) )
    {
        return( LL_STATUS_ERROR_ILLEGAL_PARAM_COMBINATION );
    }

    // multi-connction limits check
    if (g_ll_conn_ctx.numLLConns >= g_maxConnNum)
    {
        return( LL_STATUS_ERROR_OUT_OF_CONN_RESOURCES );
    }

    // if there is at least one connection, make sure this connection interval
    // is a multiple/divisor of all other active connection intervals; also make
    // sure that this connection's interval is not less than the allowed maximum
    // connection interval as determined by the maximum number of allowed
    // connections times the number of slots per connection.
    if ( g_ll_conn_ctx.numLLMasterConns > 0 )        //   if ( g_ll_conn_ctx.numLLConns > 0 )
    {
        uint16 connInterval = (connIntervalMax << 1);      // convert to 625us ticks
        uint16 minCI        = g_ll_conn_ctx.connInterval;

        //    // first check if this connection interval is even legal
        //    // Note: The number of active connections is limited by the minCI.
        //    if ( (minCI / NUM_SLOTS_PER_MASTER) < llConns.numActiveConns )
        //    {
        //      return( LL_STATUS_ERROR_UNACCEPTABLE_CONN_INTERVAL );
        //    }

        //    // does the CI need to be checked as a multiple of the minCI?
        if ( connInterval >= minCI )
        {
            // check if this connection's CI is valid (i.e. a multiple of minCI)
            if ( connInterval % minCI )
            {
                return( LL_STATUS_ERROR_UNACCEPTABLE_CONN_INTERVAL );
            }
        }
        else
            return( LL_STATUS_ERROR_UNACCEPTABLE_CONN_INTERVAL );
    }
    else
    {
        // TODO: should we consider if there is only slave connection, using the interval of slave as the standard interval of master?
        // how could application know the slave interval?
    }

//  // check if an update channel map control procedure is already pending
//  // Note: This is the case where the control procedure is enqueued, but the
//  //       control packet has not yet been sent OTA. At this point, it is
//  //       easier to simply reject the new connection until that happens. Once
//  //       the packet is sent, and the pendingParamUpdate flag is set, the
//  //       update connection interval is checked by llGetMinCI.
//  if ( llPendingUpdateParam() == TRUE )
//  {
//    return( LL_STATUS_ERROR_UPDATE_CTRL_PROC_PENDING );
//  }

    // check if the white list policy uses the peer address, it is valid
    if ( (initWlPolicy == LL_INIT_WL_POLICY_USE_PEER_ADDR) && (peerAddr == NULL) )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    if (llState == LL_STATE_CONN_MASTER || llState == LL_STATE_CONN_SLAVE)
    {
        if (llSecondaryState == LL_SEC_STATE_IDLE)
            llSecondaryState = LL_SEC_STATE_INIT;
        else
            return LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE;
    }

    // allocate a connection and assure it is valid
    if ( (connPtr = llAllocConnId()) == NULL )
    {
        llSecondaryState = LL_SEC_STATE_IDLE;      // recover llSecondaryState
        // exceeded the number of available connection structures
        return( LL_STATUS_ERROR_CONNECTION_LIMIT_EXCEEDED );
    }

    g_ll_conn_ctx.numLLMasterConns ++;
//    connId = 0;
//    connPtr = &conn_param[connId];
    // reset connection parameters
    LL_set_default_conn_params(connPtr);
    // clear the connection buffer
    reset_conn_buf(connPtr->connId);
    // save the connection ID with Init
    initInfo.connId = connPtr->connId;
    // set the scan interval
    initInfo.scanInterval = scanInterval;
    // set the scan window
    initInfo.scanWindow = scanWindow;
    // set the Init white list policy
    initInfo.wlPolicy = initWlPolicy;

    // set the connection channel map
    for (i = 0; i < LL_NUM_BYTES_FOR_CHAN_MAP; i++)
    {
        connPtr->chanMap[i] = chanMapUpdate.chanMap[i];
    }

    // process connection channel map into the data channel table
    llProcessChanMap( connPtr, connPtr->chanMap );

    // Core spec V4.0 requires that a minimum of two data channels be used
    if ( connPtr->numUsedChans < 2 )
    {
        // it isn't, so release the resource
        llReleaseConnId( connPtr );
        g_ll_conn_ctx.numLLMasterConns --;
        return( LL_STATUS_ERROR_ILLEGAL_PARAM_COMBINATION );
    }

    // save the peer address
    if (peerAddr != NULL)              // bug fixed
        LL_COPY_DEV_ADDR_LE( peerInfo.peerAddr, peerAddr );

    // save our address type
    initInfo.ownAddrType = ownAddrType;

    // check the type of own address
    if ( ownAddrType == LL_DEV_ADDR_TYPE_PUBLIC )
    {
        // save our address
        LL_COPY_DEV_ADDR_LE( initInfo.ownAddr, ownPublicAddr );
    }
    else // LL_DEV_ADDR_TYPE_RANDOM
    {
        // save our address
        LL_COPY_DEV_ADDR_LE( initInfo.ownAddr, ownRandomAddr );
    }

    // save the peer address type
    peerInfo.peerAddrType = peerAddrType;
    txHeader |=  (ownAddrType << TX_ADD_SHIFT & TX_ADD_MASK);
    txHeader |=  (peerAddrType << RX_ADD_SHIFT & RX_ADD_MASK);
    // ramdomly generate a valid 24 bit CRC value
    connPtr->initCRC = llGenerateCRC();
    // randomly generate a valid, previously unused, 32-bit access address
    connPtr->accessAddr = llGenerateValidAccessAddr();

//  g_ll_conn_ctx.scheduleInfo[connPtr->allocConn].linkRole = LL_ROLE_MASTER;    // will change the role in move_to_master_function

    if (g_ll_conn_ctx.numLLMasterConns == 1)    // A2 multi-connection, 1st connection, save the connection parameters
    {
        // determine the connection interval based on min and max values
        // Note: Range not used, so assume max value.
        // Note: minLength and maxLength are informational.
        connPtr->curParam.connInterval = connIntervalMax;
        // set the connection timeout
        // Note: The spec says this begins at the end of the CONNECT_REQ, but the
        //       LSTO will be converted into events.
        connPtr->curParam.connTimeout = connTimeout;
        // set the slave latency
        connPtr->curParam.slaveLatency = connLatency;
        // save connection parameter as global
        g_ll_conn_ctx.connInterval = connPtr->curParam.connInterval;                              // unit: 1.25ms
        g_ll_conn_ctx.slaveLatency = connPtr->curParam.slaveLatency;
        g_ll_conn_ctx.connTimeout  = connPtr->curParam.connTimeout;
        g_ll_conn_ctx.per_slot_time = connPtr->curParam.connInterval * 2 / g_maxConnNum;       // unit: 625us
    }
    else
    {
        // determine the connection interval based on min and max values
        // Note: Range not used, so assume max value.
        // Note: minLength and maxLength are informational.
        connPtr->curParam.connInterval = g_ll_conn_ctx.connInterval;
        // set the connection timeout
        // Note: The spec says this begins at the end of the CONNECT_REQ, but the
        //       LSTO will be converted into events.
        connPtr->curParam.connTimeout = g_ll_conn_ctx.connTimeout;
        // set the slave latency
        connPtr->curParam.slaveLatency = g_ll_conn_ctx.slaveLatency;
    }

    // set the master's SCA
    connPtr->sleepClkAccuracy = initInfo.scaValue;
    // set the window size (units of 1.25ms)
    // Note: Must be the lesser of 10ms and the connection interval - 1.25ms.
    connPtr->curParam.winSize = pGlobal_config[LL_CONN_REQ_WIN_SIZE];//5;//LL_WINDOW_SIZE;
    // set the window offset (units of 1.25ms). TO change if we support multiple connections
    // Note: Normally, the window offset is managed dynamically so that precise
    //       connection start times can be achieved (necessary for multiple
    //       connnections). However, sometimes it is useful to force the window
    //       offset to something specific for testing. This can be done by here
    //       when the project is built with the above define.
    // Note: This define should only be used for testing one connection and will
    //       NOT work when multiple connections are attempted!
    connPtr->curParam.winOffset = pGlobal_config[LL_CONN_REQ_WIN_OFFSET];//2;//LL_WINDOW_OFFSET;
    // set the channel map hop length (5..16)
    // Note: 0..255 % 12 = 0..11 + 5 = 5..16.
    connPtr->hop = (uint8)( (LL_ENC_GeneratePseudoRandNum() % 12) + 5);
    // track advertising channel for advancement at each scan window
    initInfo.nextScanChan = LL_SCAN_ADV_CHAN_37;
    // enable Init scan
    initInfo.scanMode = LL_SCAN_START;
    // check if this is the only task
    // Note: If there are one or more master connections already running, then
    //       the Init task will be scheduled when the next connection ends.
    {
        // ============= construct CONN REQ message payload in g_tx_adv_buf buffer. Using byte copy to avoid
        // hardfault cause by no word align reading
        uint8 offset = 0;
        g_tx_adv_buf.txheader = txHeader;
        // setup CONN REQ in connPtr->ll_buf
        LL_ReadBDADDR(&g_tx_adv_buf.data[offset]);                    // initA, Byte 0 ~ 5
        offset += 6;

        if (peerAddr != NULL)
            LL_COPY_DEV_ADDR_LE(&g_tx_adv_buf.data[offset], peerAddr)     // AdvA,  Byte 6 ~ 11
            offset += 6;

        // Access Address, Byte 12 ~ 15
        memcpy((uint8*)&g_tx_adv_buf.data[offset], (uint8*)&connPtr->accessAddr, 4);
        offset += 4;
        // CRC init, Byte 16 ~ 18
        memcpy((uint8*)&g_tx_adv_buf.data[offset], (uint8*)&connPtr->initCRC, 3);
        offset += 3;
        // WinSize, Byte 19
        g_tx_adv_buf.data[offset] = connPtr->curParam.winSize;
        offset += 1;
        // WinOffset, Byte 20 ~ 21
        memcpy((uint8*)&g_tx_adv_buf.data[offset], (uint8*)&connPtr->curParam.winOffset, 2);
        offset += 2;
        // Interval, Byte 22 ~ 23
        memcpy((uint8*)&g_tx_adv_buf.data[offset], (uint8*)&connPtr->curParam.connInterval, 2);
        offset += 2;
        // Latency, Byte 24 ~ 25
        memcpy((uint8*)&g_tx_adv_buf.data[offset], (uint8*)&connPtr->curParam.slaveLatency, 2);
        offset += 2;
        // Timeout, Byte 26 ~ 27
        memcpy((uint8*)&g_tx_adv_buf.data[offset], (uint8*)&connPtr->curParam.connTimeout, 2);
        offset += 2;
        // Channel Map, Byte 28 ~ 32
        memcpy((uint8*)&g_tx_adv_buf.data[offset], (uint8*)&connPtr->chanMap[0], 5);
        offset += 5;
        // Hop(5bit) + SCA(3bit), Byte 33
        g_tx_adv_buf.data[offset] = (connPtr->hop & 0x1f) | ((connPtr->sleepClkAccuracy & 0x7) << 5);
        // go ahead and start Init immediately
//        llSetupInit( connPtr->connId );
        // TODO: calc new connection start time and set dynamic window offset
        //llSetupConn();
    }

    if ( llState == LL_STATE_IDLE )
        // go ahead and start Init immediately
        llSetupInit( connPtr->connId );
    else
        osal_set_event(LL_TaskID, LL_EVT_SECONDARY_INIT);

    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    This API is called by the HCI to cancel a previously given LL connection
    creation command that is still pending. This command should only be used
    after the LL_CreateConn command as been issued, but before the
    LL_ConnComplete callback.

    Public function defined in ll.h.
*/
llStatus_t LL_CreateConnCancel0( void )
{
    uint8 cancel_now = FALSE;
    uint8 connId;

    // ensure Init is active
    if ( initInfo.scanMode == LL_SCAN_STOP && extInitInfo.scanMode == LL_SCAN_STOP)
    {
        // no create connection in progress
        return( LL_STATUS_ERROR_COMMAND_DISALLOWED );
    }

    // if scan has not trigger, release conn ID and report. Note: we assume only 1 type of scan could be enabled at the same time
    if (initInfo.scanMode == LL_SCAN_START)
        connId = initInfo.connId;
    else if (extInitInfo.scanMode == LL_SCAN_START)
        connId = extInitInfo.connId;
    else
        return LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE;

    if (llState == LL_STATE_INIT && !llWaitingIrq)      // primary LL state is init case
    {
        cancel_now = TRUE;
        llState = LL_STATE_IDLE;
    }
    else if( llSecondaryState == LL_SEC_STATE_INIT && !llWaitingIrq )
    {
        cancel_now = TRUE;
        llSecondaryState = LL_SEC_STATE_IDLE;
    }
    else if (llSecondaryState == LL_SEC_STATE_INIT_PENDING)  // secondary LL state is init case
    {
        cancel_now = TRUE;
        llSecondaryState = LL_SEC_STATE_IDLE;
    }

    if (extInitInfo.scanMode == LL_SCAN_START && llTaskState != LL_TASK_EXTENDED_INIT)   // extended init case
        cancel_now = TRUE;

    // indicate we are no longer actively scanning
    initInfo.scanMode = LL_SCAN_STOP;
    extInitInfo.scanMode = LL_SCAN_STOP;

    // if the scan is not ongoing, release conn ID
    if (cancel_now == TRUE)
    {
        llReleaseConnId(&conn_param[connId]);
        g_ll_conn_ctx.numLLMasterConns --;
        (void)osal_set_event( LL_TaskID, LL_EVT_MASTER_CONN_CANCELLED );         // inform high layer
    }

    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    This API is called by the Master HCI to setup encryption and to update
    encryption keys in the LL connection. If the connection is already in
    encryption mode, then this command will first pause the encryption before
    subsequently running the encryption setup.

    Public function defined in ll.h.
*/
llStatus_t LL_StartEncrypt0( uint16 connId,
                             uint8*  rand,
                             uint8*  eDiv,
                             uint8*  ltk )
{
    uint8         i;
    llStatus_t    status;
    llConnState_t* connPtr;

    // make sure we're in Master role
    if ( llState != LL_STATE_CONN_MASTER )
    {
        return( LL_STATUS_ERROR_COMMAND_DISALLOWED );
    }

    // check parameters
    if ( (rand == NULL) || (eDiv == NULL) || (ltk == NULL) )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // make sure connection ID is valid
    if ( (status=LL_ConnActive(connId)) != LL_STATUS_SUCCESS )
    {
        return( status );
    }

    // get connection info
    connPtr = &conn_param[connId];

    // check if a feature response control procedure has taken place
    if ( connPtr->featureSetInfo.featureRspRcved == FALSE )
    {
        // it hasn't so re-load this device's local Feature Set to the
        // connection as it may have been changed by the Host with HCI
        // extenstion Set Local Feature Set command
        for (i=0; i<LL_MAX_FEATURE_SET_SIZE; i++)
        {
            connPtr->featureSetInfo.featureSet[i] = deviceFeatureSet.featureSet[i];
        }
    }

    // check if encryption is a supported feature set item
    if ( (connPtr->featureSetInfo.featureSet[0] & LL_FEATURE_ENCRYPTION) != LL_FEATURE_ENCRYPTION )
    {
        return( LL_STATUS_ERROR_FEATURE_NOT_SUPPORTED );
    }

    // cache the master's random vector
    // Note: The RAND will be left in LSO..MSO order as this is assumed to be the
    //       order of the bytes that will be returned to the Host.
    for (i=0; i<LL_ENC_RAND_LEN; i++)
    {
        connPtr->encInfo.RAND[i] = rand[i];
    }

    // cache the master's encryption diversifier
    // Note: The EDIV will be left in LSO..MSO order as this is assumed to be the
    //       order of the bytes that will be returned to the Host.
    connPtr->encInfo.EDIV[0] = eDiv[0];
    connPtr->encInfo.EDIV[1] = eDiv[1];

    // cache the master's long term key
    // Note: The order of the bytes will be maintained as MSO..LSO
    //       per FIPS 197 (AES).
    for (i=0; i<LL_ENC_LTK_LEN; i++)
    {
        connPtr->encInfo.LTK[(LL_ENC_LTK_LEN-i)-1] = ltk[i];
    }

    // generate SKDm
    // Note: The SKDm LSO is the LSO of the SKD.
    // Note: Placement of result forms concatenation of SKDm and SKDs.
    // Note: The order of the bytes will be maintained as MSO..LSO
    //       per FIPS 197 (AES).
    LL_ENC_GenDeviceSKD( &connPtr->encInfo.SKD[ LL_ENC_SKD_M_OFFSET ] );
    // generate IVm
    // Note: The IVm LSO is the LSO of the IV.
    // Note: Placement of result forms concatenation of IVm and IVs.
    // Note: The order of the bytes will be maintained as MSO..LSO
    //       per FIPS 197 (AES).
    LL_ENC_GenDeviceIV( &connPtr->encInfo.IV[ LL_ENC_IV_M_OFFSET ] );
    // schedule a cache update of FIPS TRNG values for next SKD/IV usage
//  postRfOperations |= LL_POST_RADIO_CACHE_RANDOM_NUM;
    // ������������cachedTRNGdata����ʵ��masterʱ�����
    (void)LL_ENC_GenerateTrueRandNum( cachedTRNGdata, LL_ENC_TRUE_RAND_BUF_SIZE );
    // set flag to stop all outgoing transmissions
    connPtr->txDataEnabled = FALSE;
    // invalidate the existing session key, if any
    connPtr->encInfo.SKValid = FALSE;
    // indicate the LTK is not valid
    connPtr->encInfo.LTKValid = FALSE;

    // check if we are already in encryption mode
    if ( connPtr->encEnabled == TRUE )
    {
        // set a flag to indicate this is a restart (i.e. pause-then-start)
        connPtr->encInfo.encRestart = TRUE;
        // setup a pause encryption control procedure
        llEnqueueCtrlPkt( connPtr, LL_CTRL_PAUSE_ENC_REQ );
    }
    else // no, so...
    {
        // clear flag to indicate this is an encryption setup
        connPtr->encInfo.encRestart = FALSE;
        // setup an encryption control procedure
        llEnqueueCtrlPkt( connPtr, LL_CTRL_ENC_REQ );
    }

    return( LL_STATUS_SUCCESS );
}


llStatus_t LL_WriteSuggestedDefaultDataLength(uint16 TxOctets,uint16 TxTime)
{
    if(TxOctets >   LL_PDU_LENGTH_SUPPORTED_MAX_TX_OCTECTS
            || TxTime   >   LL_PDU_LENGTH_SUPPORTED_MAX_TX_TIME
            || TxOctets <   LL_PDU_LENGTH_INITIAL_MAX_TX_OCTECTS
            || TxTime   <   LL_PDU_LENGTH_INITIAL_MAX_TX_TIME)
    {
        return(LL_STATUS_ERROR_PARAM_OUT_OF_RANGE);
    }
    else
    {
        g_llPduLen.suggested.MaxTxOctets= TxOctets;
        g_llPduLen.suggested.MaxRxTime  = TxTime;
        return(LL_STATUS_SUCCESS);
    }
}

llStatus_t LL_SetDataLengh0( uint16 connId,uint16 TxOctets,uint16 TxTime )
{
    uint8         i;
    llStatus_t    status;
    llConnState_t* connPtr;

    if(TxOctets >   LL_PDU_LENGTH_SUPPORTED_MAX_TX_OCTECTS
            || TxTime   >   LL_PDU_LENGTH_SUPPORTED_MAX_TX_TIME
            || TxOctets <   LL_PDU_LENGTH_INITIAL_MAX_TX_OCTECTS
            || TxTime   <   LL_PDU_LENGTH_INITIAL_MAX_TX_TIME)
    {
        return(LL_STATUS_ERROR_PARAM_OUT_OF_RANGE);
    }

    // make sure connection ID is valid
    if ( (status=LL_ConnActive(connId)) != LL_STATUS_SUCCESS )
    {
        return( status );
    }

    // get connection info
    connPtr = &conn_param[connId];

    // check if a feature response control procedure has taken place
    if ( connPtr->featureSetInfo.featureRspRcved == FALSE )
    {
        // it hasn't so re-load this device's local Feature Set to the
        // connection as it may have been changed by the Host with HCI
        // extenstion Set Local Feature Set command
        for (i=0; i<LL_MAX_FEATURE_SET_SIZE; i++)
        {
            connPtr->featureSetInfo.featureSet[i] = deviceFeatureSet.featureSet[i];
        }
    }

    // check if dle is a supported feature set item
    if ( (connPtr->featureSetInfo.featureSet[0] & LL_FEATURE_DATA_LENGTH_EXTENSION) != LL_FEATURE_DATA_LENGTH_EXTENSION )
    {
        return( LL_STATUS_ERROR_FEATURE_NOT_SUPPORTED );
    }

    // check if an updated parameters control procedure is already what's pending
    if ( ((connPtr->ctrlPktInfo.ctrlPktCount > 0) &&
            (connPtr->ctrlPktInfo.ctrlPkts[0] == LL_CTRL_LENGTH_REQ)) ||
            (connPtr->llPduLen.isProcessingReq == TRUE) || (connPtr->llPduLen.isWatingRsp == TRUE) )
    {
        return( LL_STATUS_ERROR_CTRL_PROC_ALREADY_ACTIVE );
    }

//    g_llPduLen.suggested.MaxTxOctets = TxOctets;          // remove by HZF, suggested value is from host, should not change in controller
//    g_llPduLen.suggested.MaxTxTime   = TxTime;
    // setup an LL_LENGTH_REQ
    llEnqueueCtrlPkt( connPtr, LL_CTRL_LENGTH_REQ );
    return(LL_STATUS_SUCCESS);
}


llStatus_t LL_SetDefaultPhyMode( uint16 connId,uint8 allPhy,uint8 txPhy, uint8 rxPhy)
{
    conn_param[connId].llPhyModeCtrl.def.allPhy = allPhy;
    conn_param[connId].llPhyModeCtrl.def.txPhy  = txPhy;
    conn_param[connId].llPhyModeCtrl.def.rxPhy  = rxPhy;
    return( LL_STATUS_SUCCESS );
}


llStatus_t LL_SetPhyMode0( uint16 connId,uint8 allPhy,uint8 txPhy, uint8 rxPhy,uint16 phyOptions)
{
    uint8         i;
    llStatus_t    status;
    llConnState_t* connPtr;

    // make sure connection ID is valid
    if ( (status=LL_ConnActive(connId)) != LL_STATUS_SUCCESS )
    {
        return( status );
    }

    // get connection info
    connPtr = &conn_param[connId];

    // check if a feature response control procedure has taken place
    if ( connPtr->featureSetInfo.featureRspRcved == FALSE )
    {
        // it hasn't so re-load this device's local Feature Set to the
        // connection as it may have been changed by the Host with HCI
        // extenstion Set Local Feature Set command
        for (i=0; i<LL_MAX_FEATURE_SET_SIZE; i++)
        {
            connPtr->featureSetInfo.featureSet[i] = deviceFeatureSet.featureSet[i];
        }
    }

    // check if dle is a supported feature set item
    if(     ( (connPtr->featureSetInfo.featureSet[1] & LL_FEATURE_2M_PHY) != LL_FEATURE_2M_PHY )
            &&  ( (connPtr->featureSetInfo.featureSet[1] & LL_FEATURE_CODED_PHY) != LL_FEATURE_CODED_PHY ) )
    {
        return( LL_STATUS_ERROR_FEATURE_NOT_SUPPORTED );
    }

    // check if an updated parameters control procedure is already what's pending
    if ( ((connPtr->ctrlPktInfo.ctrlPktCount > 0) &&
            (connPtr->ctrlPktInfo.ctrlPkts[0] == LL_CTRL_PHY_REQ)) ||
            (connPtr->pendingPhyModeUpdate== TRUE) ||
            (connPtr->llPhyModeCtrl.isWatingRsp == TRUE) || (connPtr->llPhyModeCtrl.isProcessingReq == TRUE) )
    {
        return( LL_STATUS_ERROR_CTRL_PROC_ALREADY_ACTIVE );
    }

    //support Symmetric Only
    if(allPhy==0 &&(txPhy!=rxPhy))
    {
        return( LL_STATUS_ERROR_FEATURE_NOT_SUPPORTED );
    }

    // how to check the required param?
    //LL_TS_5.0.3 Table 4.43: PDU payload contents for each case variation for LE 2M PHY
    connPtr->llPhyModeCtrl.req.allPhy = allPhy;

    if(connPtr->llPhyModeCtrl.req.allPhy==0)
    {
        connPtr->llPhyModeCtrl.req.txPhy  = txPhy;
        connPtr->llPhyModeCtrl.req.rxPhy  = txPhy;
    }
    else if(connPtr->llPhyModeCtrl.req.allPhy==1)
    {
        connPtr->llPhyModeCtrl.req.txPhy  = 0;
        connPtr->llPhyModeCtrl.req.rxPhy  = txPhy;
    }
    else if(connPtr->llPhyModeCtrl.req.allPhy==2)
    {
        connPtr->llPhyModeCtrl.req.txPhy  = txPhy;
        connPtr->llPhyModeCtrl.req.rxPhy  = 0;
    }
    else
    {
        //no prefer on both phy
        connPtr->llPhyModeCtrl.req.txPhy  = 0;
        connPtr->llPhyModeCtrl.req.rxPhy  = 0;
    }

    connPtr->llPhyModeCtrl.phyOptions = phyOptions;
    // setup an LL_LENGTH_REQ
    llEnqueueCtrlPkt( connPtr, LL_CTRL_PHY_REQ );
    return(LL_STATUS_SUCCESS);
}


// add by HZF for resolving list
llStatus_t LL_AddResolvingListLDevice( uint8  addrType,
                                       uint8* devAddr,
                                       uint8* peerIrk,
                                       uint8* localIrk)
{
    int i;

    if ( (llState == LL_STATE_ADV_UNDIRECTED) ||
            (llState == LL_STATE_ADV_DIRECTED)   ||
            (llState == LL_STATE_ADV_SCAN)       ||
            (llState == LL_STATE_ADV_NONCONN)    ||
            (llState == LL_STATE_SCAN)           ||
            (llState == LL_STATE_INIT)           ||
            (llSecondaryState == LL_SEC_STATE_ADV)          ||
            (llSecondaryState == LL_SEC_STATE_ADV_PENDING)  ||
            (llSecondaryState == LL_SEC_STATE_SCAN)         ||
            (llSecondaryState == LL_SEC_STATE_SCAN_PENDING) ||
            (llSecondaryState == LL_SEC_STATE_INIT)         ||
            (llSecondaryState == LL_SEC_STATE_INIT_PENDING))
    {
        return( LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE );
    }

    if (extInitInfo.scanMode == LL_SCAN_START)
        return( LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE );

    // check the RL device address type
    if ( (addrType != LL_DEV_ADDR_TYPE_PUBLIC) &&
            (addrType != LL_DEV_ADDR_TYPE_RANDOM) )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // check if there was room for the entry
    if (g_llRlDeviceNum >= LL_RESOLVINGLIST_ENTRY_NUM)
        return( LL_STATUS_ERROR_RL_TABLE_FULL );

    // add the device to a empty record
    for (i = 0; i < LL_RESOLVINGLIST_ENTRY_NUM; i++)
    {
        if (g_llResolvinglist[i].peerAddrType == 0xff)       // empty record
        {
            g_llResolvinglist[i].peerAddrType = addrType;
            osal_memcpy(&g_llResolvinglist[i].peerAddr[0], &devAddr[0], LL_DEVICE_ADDR_LEN);
            // TODO: should we revert the byte order???
            osal_memcpy(&g_llResolvinglist[i].localIrk[0], &localIrk[0], LL_ENC_IRK_LEN);
            osal_memcpy(&g_llResolvinglist[i].peerIrk[0], &peerIrk[0], LL_ENC_IRK_LEN);
            g_llRlDeviceNum ++;
            break;
        }
    }

    return( LL_STATUS_SUCCESS );
}


llStatus_t LL_RemoveResolvingListDevice( uint8* devAddr,
                                         uint8 addrType )
{
    int i, j;

    if ( (llState == LL_STATE_ADV_UNDIRECTED) ||
            (llState == LL_STATE_ADV_DIRECTED)   ||
            (llState == LL_STATE_ADV_SCAN)       ||
            (llState == LL_STATE_ADV_NONCONN)    ||
            (llState == LL_STATE_SCAN)           ||
            (llState == LL_STATE_INIT)           ||
            (llSecondaryState == LL_SEC_STATE_ADV)          ||
            (llSecondaryState == LL_SEC_STATE_ADV_PENDING)  ||
            (llSecondaryState == LL_SEC_STATE_SCAN)         ||
            (llSecondaryState == LL_SEC_STATE_SCAN_PENDING) ||
            (llSecondaryState == LL_SEC_STATE_INIT)         ||
            (llSecondaryState == LL_SEC_STATE_INIT_PENDING))
    {
        return( LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE );
    }

    if (extInitInfo.scanMode == LL_SCAN_START)
        return( LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE );

    // check the RL device address type
    if ( (addrType != LL_DEV_ADDR_TYPE_PUBLIC) &&
            (addrType != LL_DEV_ADDR_TYPE_RANDOM) )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // check that there was at least one entry in the table
    if ( g_llRlDeviceNum == 0 )
    {
        return( LL_STATUS_ERROR_RL_TABLE_EMPTY );
    }

    for (i = 0; i < LL_RESOLVINGLIST_ENTRY_NUM; i++)
    {
        if (g_llResolvinglist[i].peerAddrType == addrType)
        {
            for (j = 0; j < LL_DEVICE_ADDR_LEN; j++)    // check whether the address is the same
            {
                if (g_llResolvinglist[i].peerAddr[j] != devAddr[j])
                    break;
            }

            if (j == LL_DEVICE_ADDR_LEN)    // found it
            {
                g_llResolvinglist[i].peerAddrType = 0xff;
                g_llResolvinglist[i].privacyMode  = NETWORK_PRIVACY_MODE;
                g_llRlDeviceNum --;
                break;
            }
        }
    }

    if (i == LL_RESOLVINGLIST_ENTRY_NUM)
        return( LL_STATUS_ERROR_RL_ENTRY_NOT_FOUND );

    return( LL_STATUS_SUCCESS );
}

llStatus_t LL_ClearResolvingList( void )
{
    int i;

    if ( (llState == LL_STATE_ADV_UNDIRECTED) ||
            (llState == LL_STATE_ADV_DIRECTED)   ||
            (llState == LL_STATE_ADV_SCAN)       ||
            (llState == LL_STATE_ADV_NONCONN)    ||
            (llState == LL_STATE_SCAN)           ||
            (llState == LL_STATE_INIT)           ||
            (llSecondaryState == LL_SEC_STATE_ADV)          ||
            (llSecondaryState == LL_SEC_STATE_ADV_PENDING)  ||
            (llSecondaryState == LL_SEC_STATE_SCAN)         ||
            (llSecondaryState == LL_SEC_STATE_SCAN_PENDING) ||
            (llSecondaryState == LL_SEC_STATE_INIT)         ||
            (llSecondaryState == LL_SEC_STATE_INIT_PENDING))
    {
        return( LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE );
    }

    if (extInitInfo.scanMode == LL_SCAN_START)
        return( LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE );

    // clear number of entries, valid flags, address type flags, and entries
    for (i = 0; i < LL_RESOLVINGLIST_ENTRY_NUM; i ++)
    {
        g_llResolvinglist[i].peerAddrType = 0xff;
        g_llResolvinglist[i].privacyMode  = NETWORK_PRIVACY_MODE;
        memset(&g_llResolvinglist[i].peerAddr[0], 0, LL_DEVICE_ADDR_LEN);
    }

    // set white list number 0
    g_llRlDeviceNum = 0;
    return( LL_STATUS_SUCCESS );
}


llStatus_t LL_ReadResolvingListSize( uint8* numEntries )
{
    *numEntries = LL_RESOLVINGLIST_ENTRY_NUM;
    return (LL_STATUS_SUCCESS);
}

llStatus_t LL_ReadPeerResolvableAddress( uint8* peerRpa )
{
    peerRpa[0] = g_currentPeerRpa[0];
    peerRpa[1] = g_currentPeerRpa[1];
    peerRpa[2] = g_currentPeerRpa[2];
    peerRpa[3] = g_currentPeerRpa[3];
    peerRpa[4] = g_currentPeerRpa[4];
    peerRpa[5] = g_currentPeerRpa[5];
    return (LL_STATUS_SUCCESS);
}

llStatus_t LL_ReadLocalResolvableAddress( uint8* localRpa )
{
    localRpa[0] = g_currentPeerRpa[0];
    localRpa[1] = g_currentPeerRpa[1];
    localRpa[2] = g_currentPeerRpa[2];
    localRpa[3] = g_currentPeerRpa[3];
    localRpa[4] = g_currentPeerRpa[4];
    localRpa[5] = g_currentPeerRpa[5];
    return (LL_STATUS_SUCCESS);
}

// enable : 1 - enable, 0 - disable
llStatus_t LL_SetAddressResolutionEnable( uint8 enable )
{
    if ( (llState == LL_STATE_ADV_UNDIRECTED) ||
            (llState == LL_STATE_ADV_DIRECTED)   ||
            (llState == LL_STATE_ADV_SCAN)       ||
            (llState == LL_STATE_ADV_NONCONN)    ||
            (llState == LL_STATE_SCAN)           ||
            (llState == LL_STATE_INIT)           ||
            (llSecondaryState == LL_SEC_STATE_ADV)          ||
            (llSecondaryState == LL_SEC_STATE_ADV_PENDING)  ||
            (llSecondaryState == LL_SEC_STATE_SCAN)         ||
            (llSecondaryState == LL_SEC_STATE_SCAN_PENDING) ||
            (llSecondaryState == LL_SEC_STATE_INIT)         ||
            (llSecondaryState == LL_SEC_STATE_INIT_PENDING))
    {
        return( LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE );
    }

    if (extInitInfo.scanMode == LL_SCAN_START)
        return( LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE );

    g_llRlEnable = enable;
//    if (g_llRlEnable == TRUE)
//        osal_start_timerEx( LL_TaskID, LL_EVT_RPA_TIMEOUT, g_llRlTimeout * 1000 );
//  else   // FALSE
//      osal_stop_timerEx(LL_TaskID, LL_EVT_RPA_TIMEOUT);
    return( LL_STATUS_SUCCESS );
}

llStatus_t LL_SetResolvablePrivateAddressTimeout( uint16 rpaTimeout )
{
    g_llRlTimeout = rpaTimeout;
//    if (g_llRlEnable == TRUE)
//    osal_start_timerEx( LL_TaskID, LL_EVT_RPA_TIMEOUT, g_llRlTimeout * 1000 );
    return( LL_STATUS_SUCCESS );
}

llStatus_t LL_Set_Privacy_Mode(uint8  peerIdType,
                               uint8* peerIdAddr,
                               uint8 privacyMode)
{
    int i, j;

    // search the peer address in the resolving address list
    if ( g_llRlDeviceNum == 0 )
    {
        return( LL_STATUS_ERROR_RL_TABLE_EMPTY );
    }

    for (i = 0; i < LL_RESOLVINGLIST_ENTRY_NUM; i++)
    {
        if (g_llResolvinglist[i].peerAddrType == peerIdType)
        {
            for (j = 0; j < LL_DEVICE_ADDR_LEN; j++)    // check whether the address is the same
            {
                if (g_llResolvinglist[i].peerAddr[j] != peerIdAddr[j])
                    break;
            }

            if (j == LL_DEVICE_ADDR_LEN)    // found it
                break;
        }
    }

    if (i == LL_RESOLVINGLIST_ENTRY_NUM)
        return( LL_STATUS_ERROR_UNKNOWN_CONN_HANDLE );

    g_llResolvinglist[i].privacyMode = privacyMode;
    return LL_STATUS_SUCCESS;
}

// extend advertisement function

/*******************************************************************************
    @fn          LL_SetExtAdvSetRandomAddress

    @brief       This function is used to set random address for a advertisement set

    input parameters

    @param       adv_handle - advertisement set handler
                random_address -  random address

    output parameters

    @param       none

    @return      LL_STATUS_SUCCESS, error code(TBD)
*/
llStatus_t LL_SetExtAdvSetRandomAddress( uint8 adv_handle,
                                         uint8* random_address
                                       )
{
    int i;

    // extended advertiser memory is allocate by application, return fail if not
    if (g_pExtendedAdvInfo == NULL)
        return LL_STATUS_ERROR_OUT_OF_HEAP;

    // search advertisement handler list
    for (i = 0; i < g_extAdvNumber; i ++)
    {
        if (g_pExtendedAdvInfo[i].advHandle == adv_handle)
            break;
    }

    // if not found, then adv parameter has not been set
    if (i == g_extAdvNumber)
        return LL_STATUS_ERROR_UNKNOWN_ADV_ID;

    g_pExtendedAdvInfo[i].parameter.isOwnRandomAddressSet = TRUE;
    memcpy(&g_pExtendedAdvInfo[i].parameter.ownRandomAddress[0], random_address, LL_DEVICE_ADDR_LEN);
    return( LL_STATUS_SUCCESS );
}


/*******************************************************************************
    @fn          LL_SetExtAdvParam

    @brief       This function is used to set extend advertiser parameters

    input parameters

    @param       adv_handle - advertisement set handler


    output parameters

    @param       none

    @return      LL_STATUS_SUCCESS, error code(TBD)
*/
llStatus_t LL_SetExtAdvParam( uint8 adv_handle,
                              uint16 adv_event_properties,
                              uint32 primary_advertising_interval_Min,          // 3 octets
                              uint32 primary_advertising_interval_Max,          // 3 octets
                              uint8  primary_advertising_channel_map,
                              uint8  own_address_type,
                              uint8  peer_address_type,
                              uint8* peer_address,
                              uint8  advertising_filter_policy,
                              int8   advertising_tx_power,
                              uint8  primary_advertising_PHY,
                              uint8  secondary_advertising_max_skip,
                              uint8  secondary_advertising_PHY,
                              uint8  advertising_SID,
                              uint8  scan_request_notification_enable,
                              int8*  selectTxPwr
                            )
{
    int   i;

    // TODO: add parameters range checking
    if (g_llAdvMode == LL_MODE_LEGACY)
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llAdvMode = LL_MODE_EXTENDED;

    if ((adv_event_properties & LE_ADV_PROP_LEGACY_BITMASK)           // Legacy Adv
            && (adv_event_properties != LL_EXT_ADV_PROP_ADV_IND)
            && (adv_event_properties != LL_EXT_ADV_PROP_ADV_LDC_ADV)
            && (adv_event_properties != LL_EXT_ADV_PROP_ADV_HDC_ADV)
            && (adv_event_properties != LL_EXT_ADV_PROP_ADV_SCAN_IND)
            && (adv_event_properties != LL_EXT_ADV_PROP_ADV_NOCONN_IND))
        return LL_STATUS_ERROR_BAD_PARAMETER;

    if (primary_advertising_interval_Max < primary_advertising_interval_Min
            || primary_advertising_interval_Min < 0x20
            || primary_advertising_interval_Max < 0x20)
        return LL_STATUS_ERROR_BAD_PARAMETER;

    // check whether aux adv pdu resource period is OK for max skip & adv interval setting
    if (!(adv_event_properties & LE_ADV_PROP_LEGACY_BITMASK)                    // not legacy Adv
            && primary_advertising_interval_Max * (secondary_advertising_max_skip + 1) * 625 < g_advSlotPeriodic)
        return LL_STATUS_ERROR_BAD_PARAMETER;

    // extended advertiser memory is allocate by application, return fail if not
    if (g_pExtendedAdvInfo == NULL)
        return LL_STATUS_ERROR_OUT_OF_HEAP;

    // search advertisement handler list
    for (i = 0; i < g_extAdvNumber; i ++)
    {
        if (g_pExtendedAdvInfo[i].advHandle == adv_handle)
            break;
    }

    // if not found, search the 1st available slot
    if (i == g_extAdvNumber)
    {
        for (i = 0; i < g_extAdvNumber; i ++)
        {
            if (g_pExtendedAdvInfo[i].advHandle == LL_INVALID_ADV_SET_HANDLE)
            {
                uint8* scanRspData = g_pExtendedAdvInfo[i].scanRspData;
                uint8* advData     = g_pExtendedAdvInfo[i].data.advertisingData;
                memset(&g_pExtendedAdvInfo[i], 0, sizeof(extAdvInfo_t));
                g_pExtendedAdvInfo[i].scanRspData = scanRspData;
                g_pExtendedAdvInfo[i].data.advertisingData = advData;
                break;
            }
        }
    }

    if (i == g_extAdvNumber)
        return LL_STATUS_ERROR_OUT_OF_HEAP;

    // save the advertisement parameters
    g_pExtendedAdvInfo[i].advHandle = adv_handle;
    g_pExtendedAdvInfo[i].parameter.advEventProperties = adv_event_properties;
    g_pExtendedAdvInfo[i].parameter.priAdvIntMin       = primary_advertising_interval_Min;
    g_pExtendedAdvInfo[i].parameter.priAdvgIntMax      = primary_advertising_interval_Max;
    g_pExtendedAdvInfo[i].parameter.priAdvChnMap       = primary_advertising_channel_map;
    g_pExtendedAdvInfo[i].parameter.ownAddrType        = own_address_type;
    g_pExtendedAdvInfo[i].parameter.peerAddrType       = peer_address_type;
    memcpy(g_pExtendedAdvInfo[i].parameter.peerAddress, peer_address, LL_DEVICE_ADDR_LEN);
    g_pExtendedAdvInfo[i].parameter.wlPolicy           = advertising_filter_policy;
    g_pExtendedAdvInfo[i].parameter.advTxPower         = advertising_tx_power;
    g_pExtendedAdvInfo[i].parameter.primaryAdvPHY      = primary_advertising_PHY;
    g_pExtendedAdvInfo[i].parameter.secondaryAdvPHY    = secondary_advertising_PHY;
    g_pExtendedAdvInfo[i].parameter.secondaryAdvMaxSkip= secondary_advertising_max_skip;
    g_pExtendedAdvInfo[i].parameter.advertisingSID     = advertising_SID;
    g_pExtendedAdvInfo[i].parameter.scanReqNotificationEnable  = scan_request_notification_enable;

    // =========== controller select parameters, TBD
    // decide primary advertising interval. The aux PDU period is decided by global_config, the primary adv interval
    // should be set considering the maximum skip AUX PDU requirement
    if (primary_advertising_interval_Min * (secondary_advertising_max_skip + 1) * 625 > g_advSlotPeriodic)
        g_pExtendedAdvInfo[i].primary_advertising_interval = primary_advertising_interval_Min * 625;    // bug fiex 04-08, * 1250 -> * 625
    else
        g_pExtendedAdvInfo[i].primary_advertising_interval = primary_advertising_interval_Max * 625;

    // select Tx power and return
    g_pExtendedAdvInfo[i].tx_power = advertising_tx_power;
    *selectTxPwr = advertising_tx_power;
    g_pExtendedAdvInfo[i].isPeriodic = FALSE;
    return( LL_STATUS_SUCCESS );
}


/*******************************************************************************
    @fn          LL_SetExtAdvData

    @brief       This function is used to set extend advertiser set data

    input parameters

    @param       adv_handle - advertisement set handler


    output parameters

    @param       none

    @return      LL_STATUS_SUCCESS, error code(TBD)
*/
llStatus_t LL_SetExtAdvData( uint8  adv_handle,
                             uint8  operation,
                             uint8  fragment_preference,
                             uint8  advertising_data_length,
                             uint8* advertising_data
                           )
{
    int   i;

    // TODO: add parameters range checking
    if (g_llAdvMode == LL_MODE_LEGACY)
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llAdvMode = LL_MODE_EXTENDED;

    if (operation > BLE_EXT_ADV_OP_UNCHANGED_DATA)
        return LL_STATUS_ERROR_BAD_PARAMETER;

    // extended advertiser memory is allocate by application, return fail if not
    if (g_pExtendedAdvInfo == NULL)
        return LL_STATUS_ERROR_OUT_OF_HEAP;

    // search advertisement handler list
    for (i = 0; i < g_extAdvNumber; i ++)
    {
        if (g_pExtendedAdvInfo[i].advHandle == adv_handle)
            break;
    }

    // if not found, then adv parameter has not been set
    if (i == g_extAdvNumber)
        return LL_STATUS_ERROR_UNKNOWN_ADV_ID;

    // comment out 04-10, set adv data should be received after set adv parameter
//    {
//        for (i = 0; i < g_extAdvNumber; i ++)
//        {
//            if (g_pExtendedAdvInfo[i].advHandle == LL_INVALID_ADV_SET_HANDLE)
//              break;
//        }
//        if (i == g_extAdvNumber)
//          return LL_STATUS_ERROR_OUT_OF_HEAP;
//
//        // set DID when 1st create the data set
//      LL_Rand((uint8*)&g_pExtendedAdvInfo[i].data.DIDInfo, 2);
//    }

    // check legacy ADV data length should <= 31
    if (ll_isLegacyAdv(&g_pExtendedAdvInfo[i])
            && (operation != BLE_EXT_ADV_OP_COMPLETE_DATA
                || advertising_data_length > 31))
        return LL_STATUS_ERROR_BAD_PARAMETER;

    if (operation == BLE_EXT_ADV_OP_FIRST_FRAG ||
            operation == BLE_EXT_ADV_OP_COMPLETE_DATA)
        g_pExtendedAdvInfo[i].data.advertisingDataLength = 0;

    if (g_pExtendedAdvInfo[i].data.advertisingDataLength + advertising_data_length > g_advSetMaximumLen)
    {
        g_pExtendedAdvInfo[i].data.advertisingDataLength = 0;
        return LL_STATUS_ERROR_OUT_OF_HEAP;
    }

    // fill advertising set
    g_pExtendedAdvInfo[i].advHandle          = adv_handle;
    g_pExtendedAdvInfo[i].data.fragmentPreference = fragment_preference;
    memcpy(&g_pExtendedAdvInfo[i].data.advertisingData[g_pExtendedAdvInfo[i].data.advertisingDataLength],
           advertising_data, advertising_data_length);
    g_pExtendedAdvInfo[i].data.advertisingDataLength += advertising_data_length;

    // last fragment or 1 segment adv data
    if (operation == BLE_EXT_ADV_OP_LAST_FRAG     ||
            operation == BLE_EXT_ADV_OP_COMPLETE_DATA )
        g_pExtendedAdvInfo[i].data.dataComplete = TRUE;

    if (operation == BLE_EXT_ADV_OP_LAST_FRAG     ||
            operation == BLE_EXT_ADV_OP_COMPLETE_DATA ||
            operation == BLE_EXT_ADV_OP_UNCHANGED_DATA )    // unchange data, just update DID
    {
        // update DID
        g_pExtendedAdvInfo[i].data.DIDInfo = ll_generateExtAdvDid(g_pExtendedAdvInfo[i].data.DIDInfo);
    }

    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          LL_SetExtScanRspData

    @brief       This function is used to set extend scan response data

    input parameters

    @param       adv_handle - advertisement set handler


    output parameters

    @param       none

    @return      LL_STATUS_SUCCESS, error code(TBD)
*/
llStatus_t LL_SetExtScanRspData( uint8 adv_handle,
                                 uint8 operation,
                                 uint8  fragment_preference,
                                 uint8  scan_rsp_data_length,
                                 uint8* scan_rsp_data
                               )
{
	(void) fragment_preference;
    int   i;

    if (g_llAdvMode == LL_MODE_LEGACY)
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llAdvMode = LL_MODE_EXTENDED;

    if (operation != BLE_EXT_ADV_OP_COMPLETE_DATA)
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    // extended advertiser memory is allocate by application, return fail if not
    if (g_pExtendedAdvInfo == NULL)
        return LL_STATUS_ERROR_OUT_OF_HEAP;

    // search advertisement handler list
    for (i = 0; i < g_extAdvNumber; i ++)
    {
        if (g_pExtendedAdvInfo[i].advHandle == adv_handle)
            break;
    }

    // if not found, then adv parameter has not been set
    if (i == g_extAdvNumber)
        return LL_STATUS_ERROR_UNKNOWN_ADV_ID;

    // check legacy ADV scan rsp data length should <= 31
    if (ll_isLegacyAdv(&g_pExtendedAdvInfo[i])
            && (operation != BLE_EXT_ADV_OP_COMPLETE_DATA
                || scan_rsp_data_length > 31))
        return LL_STATUS_ERROR_BAD_PARAMETER;

//  // if not found, search the 1st available slot
//  if (i == g_extAdvNumber)
//    {
//        for (i = 0; i < g_extAdvNumber; i ++)
//        {
//            if (g_pExtendedAdvInfo[i].advHandle == LL_INVALID_ADV_SET_HANDLE)
//              break;
//        }
//        if (i == g_extAdvNumber)
//          return LL_STATUS_ERROR_OUT_OF_HEAP;
//    }

    if (g_pExtendedAdvInfo[i].scanRspData == NULL || g_advSetMaximumLen < scan_rsp_data_length)
        return LL_STATUS_ERROR_OUT_OF_HEAP;

    memcpy(&g_pExtendedAdvInfo[i].scanRspData[0], scan_rsp_data, scan_rsp_data_length);
    g_pExtendedAdvInfo[i].scanRspMaxLength = scan_rsp_data_length;
    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          LL_SetExtAdvEnable

    @brief       This function is used to enable/disable extend advertise

    input parameters

    @param       enable - enable/disable


    output parameters

    @param       none

    @return      LL_STATUS_SUCCESS, error code(TBD)
*/
llStatus_t LL_SetExtAdvEnable( uint8  enable,
                               uint8  number_of_sets,
                               uint8*  advertising_handle,
                               uint16* duration,
                               uint8*  max_extended_advertising_events)
{
    int i, j;
    extAdvInfo_t*  pExtAdv[64];
    periodicAdvInfo_t* pPrdAdv[64];

    // TODO: add more sanity checking to align to the spec
    if (g_llAdvMode == LL_MODE_LEGACY)
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llAdvMode = LL_MODE_EXTENDED;

    if (number_of_sets == 0 && enable == TRUE)
        return LL_STATUS_ERROR_UNEXPECTED_PARAMETER;

    //  the adv parameter should be present for the adv_handler, otherwise, it will return Unknown Advertising Identifier(0x42)
    for (i = 0; i < number_of_sets; i++)
    {
        pExtAdv[i] = NULL;
        pPrdAdv[i] = NULL;

        for (j = 0; j < g_extAdvNumber; j++)
        {
            if (g_pExtendedAdvInfo[j].advHandle == advertising_handle[i])
            {
                pExtAdv[i] = &g_pExtendedAdvInfo[j];
                break;
            }
        }

        if (pExtAdv[i] == NULL)     // adv handle not found
            return LL_STATUS_ERROR_UNKNOWN_ADV_ID;

        if (ll_isLegacyAdv(pExtAdv[i]))
        {
            if (pExtAdv[i]->data.advertisingDataLength > 31)
                return LL_STATUS_ERROR_ILLEGAL_PARAM_COMBINATION;
        }

        if ((pExtAdv[i]->parameter.advEventProperties & LE_ADV_PROP_SCAN_BITMASK)
                && (pExtAdv[i]->scanRspData == NULL || pExtAdv[i]->scanRspMaxLength == 0))
            return  LL_STATUS_ERROR_COMMAND_DISALLOWED;

        if (pExtAdv[i]->parameter.ownAddrType == LL_DEV_ADDR_TYPE_RANDOM
                && pExtAdv[i]->parameter.isOwnRandomAddressSet == FALSE)
            return LL_STATUS_ERROR_BAD_PARAMETER;

        // TODO: If the advertising set's Own_Address_Type parameter is set to 0x03, the
        // controller's resolving list did not contain a matching entry, and the random
        // address for the advertising set has not been initialized, the Controller shall
        // return the error code Invalid HCI Command Parameters (0x12).

//        pExtAdv[i]->isPeriodic = FALSE;
//        // if periodic adv is support, check whether the adv set is periodic adv
//        if (g_pPeriodicAdvInfo != NULL)
//        {
//            for (j = 0; j < g_perioAdvNumber; j ++)
//            {
//                if (g_pPeriodicAdvInfo[j].advHandle == pExtAdv[i]->advHandle)
//                {
//                    pExtAdv[i]->isPeriodic = TRUE;
//                  index = j;
//                    break;
//                }
//            }
//        }

        // for periodic adv, search the period adv context
        if (pExtAdv[i]->isPeriodic == TRUE)
        {
            for (j = 0; j < g_perioAdvNumber; j ++)
            {
                if (g_pPeriodicAdvInfo[j].advHandle == pExtAdv[i]->advHandle)
                {
                    pPrdAdv[i] = &g_pPeriodicAdvInfo[j];
                    break;
                }
            }

            if (pPrdAdv[i] == NULL)
                return LL_STATUS_ERROR_UNKNOWN_ADV_ID;
        }

        // for enable adv case, the adv set data should be complete,
        //   otherwise, it shall return Command Disallowed(0x0C)
        if (enable == TRUE)
        {
            if ((pExtAdv[i]->isPeriodic == FALSE && pExtAdv[i]->data.dataComplete == FALSE)
                    || (pExtAdv[i]->isPeriodic == TRUE && pPrdAdv[i]->data.dataComplete == FALSE))
                return LL_STATUS_ERROR_COMMAND_DISALLOWED;
        }
    }

    // check OK, save the enable info
    for (i = 0; i < number_of_sets; i++)
    {
        pExtAdv[i]->active = enable;

        if (enable == TRUE)
        {
            // only save parameters in enable case
            pExtAdv[i]->duration = duration[i] * 10000;     // 10000: unit 10ms, convert to us
            pExtAdv[i]->maxExtAdvEvents = max_extended_advertising_events[i];
        }
    }

    // ====== update extend advertiser scheduler to apply new parameters
    if (enable == FALSE)
    {
        // case 1: number of set is 0, disable all adv case. Remove the adv from adv schduler
        if (number_of_sets == 0)
        {
            for (i = 0; i < g_extAdvNumber; i ++)      // bug fixed 04-07
            {
                if (g_pAdvSchInfo[i].adv_handler == LL_INVALID_ADV_SET_HANDLE)
                    continue;

                ll_delete_adv_task(i);
            }

            //  periodic adv case, disable all EXT_ADV_IND + AUX_ADV_IND of periodic adv. It should be OK
            //  by set the active flag to FALSE. TO BE TEST.
            for (i = 0; i < g_perioAdvNumber; i ++)
            {
                if (g_pAdvSchInfo_periodic[i].adv_handler == LL_INVALID_ADV_SET_HANDLE)
                    continue;

                g_pAdvSchInfo_periodic[i].pAdvInfo->active = FALSE;
            }
        }
        // case 2: disable the adv in the adv set list
        else
        {
            for (i = 0; i < number_of_sets; i ++)
            {
                // search the adv in the scheduler list and disable it
                for (j = 0; j < g_schExtAdvNum; j++)
                {
                    if (g_pAdvSchInfo[j].adv_handler == pExtAdv[i]->advHandle)
                    {
                        ll_delete_adv_task(j);
                        break;
                    }
                }

                // for periodic adv, only de-active the extended adv part.
                // for extended adv, the active flag have already been set by ll_delete_adv_task()
                // but it is harmless to set it again
                pExtAdv[i]->active = FALSE;
            }
        }
    }
    else     // enable case
    {
        for (i = 0; i < number_of_sets; i++)
        {
            for (j = 0; j < g_schExtAdvNum; j++)
            {
                // check whether the adv already started
                if (g_pAdvSchInfo[j].adv_handler == pExtAdv[i]->advHandle)    // the adv in the scheduler list
                {
                    // TODO: adv already enable, it should:
                    // If the HCI_LE_Set_Extended_Advertising_Enable command is sent again for
                    // an advertising set while that set is enabled, the timer used for the duration and
                    // the number of events counter are reset and any change to the random address
                    // shall take effect
                    break;
                }
            }

            // new extended adv case
            if (j == g_schExtAdvNum && pExtAdv[i]->isPeriodic == FALSE)
                ll_add_adv_task(pExtAdv[i]);

            // new periodic adv case
            if (j == g_schExtAdvNum && pExtAdv[i]->isPeriodic == TRUE)
            {
                // check whether the corresponding periodic adv is enable
                // 1. enable, start periodic adv
                // 2. disable, do nothing
                if (pPrdAdv[i]->active == TRUE)
                    ll_add_adv_task_periodic(pPrdAdv[i], pExtAdv[i]);
            }
        }
    }

    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          LL_ReadMaximumAdvDataLength

    @brief       This function is used to read the maximum adv set data length support by controller

    input parameters

    @param       adv_handle - advertisement set handler


    output parameters

    @param       length  -  pointer to the variable of maximum data length support

    @return      LL_STATUS_SUCCESS, error code(TBD)
*/
llStatus_t LL_ReadMaximumAdvDataLength( uint16* length )
{
    *length = LL_MAX_ADVERTISER_SET_LENGTH;            // TBD.

    if (g_llAdvMode == LL_MODE_LEGACY)
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llAdvMode = LL_MODE_EXTENDED;
    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          LL_ReadNumberOfSupportAdvSet

    @brief       This function is used to read number of adv set supported by controller

    input parameters

    @param       adv_handle - advertisement set handler


    output parameters

    @param       none

    @return      LL_STATUS_SUCCESS, error code(TBD)
*/
llStatus_t LL_ReadNumberOfSupportAdvSet( uint8* number )
{
    if (g_llAdvMode == LL_MODE_LEGACY)
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llAdvMode = LL_MODE_EXTENDED;
    *number = 240;              // TBD
    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          LL_RemoveAdvSet

    @brief       This function is used to remove advertisement set

    input parameters

    @param       adv_handle - advertisement set handler


    output parameters

    @param       none

    @return      LL_STATUS_SUCCESS, error code(TBD)
*/
llStatus_t LL_RemoveAdvSet( uint8 adv_handle)
{
    uint8 i;
    uint8 extIndex = 0xFF, prdIndex = 0xFF;

    if (g_llAdvMode == LL_MODE_LEGACY)
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llAdvMode = LL_MODE_EXTENDED;

    // search extendedadvertisement handler list
    for (i = 0; i < g_extAdvNumber; i ++)
    {
        if (g_pExtendedAdvInfo[i].advHandle == adv_handle)
        {
            extIndex = i;
            break;
        }
    }

    // search advertisement handler list
    for (i = 0; i < g_perioAdvNumber; i ++)
    {
        if (g_pPeriodicAdvInfo[i].advHandle == adv_handle)
        {
            prdIndex = i;
            break;
        }
    }

    if (extIndex == 0xFF && prdIndex == 0xFF)
        return LL_STATUS_ERROR_UNKNOWN_ADV_ID;

    if ((extIndex != 0xFF && g_pExtendedAdvInfo[extIndex].active == TRUE)
            || (prdIndex != 0xFF && g_pPeriodicAdvInfo[prdIndex].active == TRUE))
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    if (extIndex != 0xFF)
    {
        uint8* scanRspData = g_pExtendedAdvInfo[extIndex].scanRspData;
        uint8* advData     = g_pExtendedAdvInfo[extIndex].data.advertisingData;
        memset(&g_pExtendedAdvInfo[extIndex], 0, sizeof(extAdvInfo_t));
        g_pExtendedAdvInfo[extIndex].scanRspData = scanRspData;
        g_pExtendedAdvInfo[extIndex].data.advertisingData = advData;
        g_pExtendedAdvInfo[extIndex].advHandle = LL_INVALID_ADV_SET_HANDLE;
    }

    if (prdIndex != 0xFF)
    {
        uint8* advData     = g_pPeriodicAdvInfo[prdIndex].data.advertisingData;
        memset(&g_pPeriodicAdvInfo[prdIndex], 0, sizeof(periodicAdvInfo_t));
        g_pPeriodicAdvInfo[prdIndex].data.advertisingData = advData;
        g_pPeriodicAdvInfo[prdIndex].advHandle = LL_INVALID_ADV_SET_HANDLE;
    }

    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          LL_ClearAdvSets

    @brief       This function is used to clear the stored advertisement sets in controller

    input parameters

    @param       none


    output parameters

    @param       none

    @return      LL_STATUS_SUCCESS, error code(TBD)
*/
llStatus_t LL_ClearAdvSets(void)
{
    uint8 i;

    if (g_llAdvMode == LL_MODE_LEGACY)
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llAdvMode = LL_MODE_EXTENDED;

    // check extendedadvertisement handler list
    for (i = 0; i < g_extAdvNumber; i ++)
    {
        if (g_pExtendedAdvInfo[i].advHandle != LL_INVALID_ADV_SET_HANDLE
                && g_pExtendedAdvInfo[i].active == TRUE)
            return LL_STATUS_ERROR_COMMAND_DISALLOWED;
    }

    // check periodic advertisement handler list
    for (i = 0; i < g_perioAdvNumber; i ++)
    {
        if (g_pPeriodicAdvInfo[i].advHandle != LL_INVALID_ADV_SET_HANDLE
                && g_pPeriodicAdvInfo[i].active == TRUE)
            return LL_STATUS_ERROR_COMMAND_DISALLOWED;
    }

    // clear adv set (extended adv part)
    for (i = 0; i < g_extAdvNumber; i ++)
    {
        uint8* scanRspData = g_pExtendedAdvInfo[i].scanRspData;
        uint8* advData     = g_pExtendedAdvInfo[i].data.advertisingData;
        memset(&g_pExtendedAdvInfo[i], 0, sizeof(extAdvInfo_t));
        g_pExtendedAdvInfo[i].scanRspData = scanRspData;
        g_pExtendedAdvInfo[i].data.advertisingData = advData;
        g_pExtendedAdvInfo[i].advHandle = LL_INVALID_ADV_SET_HANDLE;
    }

    // clear adv set (periodic adv part)
    for (i = 0; i < g_perioAdvNumber; i ++)
    {
        uint8* advData     = g_pPeriodicAdvInfo[i].data.advertisingData;
        memset(&g_pPeriodicAdvInfo[i], 0, sizeof(periodicAdvInfo_t));
        g_pPeriodicAdvInfo[i].data.advertisingData = advData;
        g_pPeriodicAdvInfo[i].advHandle = LL_INVALID_ADV_SET_HANDLE;
    }

    return( LL_STATUS_SUCCESS );
}



// ===================== extended scan/init
llStatus_t LL_SetExtendedScanParameters(uint8 own_address_type,
                                        uint8 scanning_filter_policy,
                                        uint8 scanning_PHYs,
                                        uint8* scan_type,
                                        uint16* scan_interval,
                                        uint16* scan_window)
{
    uint8 number_phys, i;

    // TODO: sanity checking
    if (g_llScanMode == LL_MODE_LEGACY )
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llScanMode = LL_MODE_EXTENDED;

    // If the Host specifies a PHY that is not supported by the Controller, including a bit that is reserved for future use,
    // it should return the error code Unsupported Feature or Parameter Value(0x11).
    if ((scanning_PHYs & (~(LL_SCAN_PHY_1M_BITMASK | LL_SCAN_PHY_CODED_BITMASK))) != 0)
        return LL_STATUS_ERROR_FEATURE_NOT_SUPPORTED;

    extScanInfo.ownAddrType = own_address_type;
    extScanInfo.wlPolicy    = scanning_filter_policy;
    number_phys = 0;

    if (scanning_PHYs & LL_SCAN_PHY_1M_BITMASK)
    {
        extScanInfo.scanPHYs[number_phys] = PKT_FMT_BLE1M;
        number_phys ++;
    }

    if (scanning_PHYs & LL_SCAN_PHY_CODED_BITMASK)
    {
        extScanInfo.scanPHYs[number_phys] = PKT_FMT_BLR125K;
        number_phys ++;
    }

    for (i = 0; i < number_phys; i ++)
    {
        extScanInfo.scanType[i] = scan_type[i];
        extScanInfo.scanInterval[i] = scan_interval[i];
        extScanInfo.scanWindow[i] = scan_window[i];
    }

    extScanInfo.numOfScanPHY = number_phys;
    return LL_STATUS_SUCCESS;
}

llStatus_t LL_SetExtendedScanEnable(uint8 enable,
                                    uint8 filter_duplicates,
                                    uint16 duration,
                                    uint16 period)
{
    // TODO: sanity checking
    if (g_llScanMode == LL_MODE_LEGACY )
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llScanMode = LL_MODE_EXTENDED;
    extScanInfo.enable = enable;
    extScanInfo.filterDuplicate = filter_duplicates;
    extScanInfo.duration = duration;
    extScanInfo.period   = period;

    // trigger scan task
    if (enable == TRUE)
    {
        extScanInfo.current_index = 0;
        extScanInfo.current_chn = LL_ADV_CHAN_FIRST;
        extScanInfo.current_scan_PHY = extScanInfo.scanPHYs[extScanInfo.current_index];
        extScanInfo.adv_data_offset = 0;
        llSetupExtScan(extScanInfo.current_chn);
    }
    else
    {
    }

    return LL_STATUS_SUCCESS;
}

#define DBG_DISABLE_LATENCY 0
llStatus_t LL_PLUS_DisableSlaveLatency0(uint8 connId)
{
    llConnState_t* connPtr;
    uint16  next_event, elapse_event,chanMapDltEvt;
    uint32  remain_time1, remain_time2;
    uint32  old_timerDrift;
    uint32  time_advance;
    // get connection information
    connPtr = &conn_param[connId];        // only 1 connection now, HZF

    // check whether it should recover from latency
    if (FALSE == connPtr->active
            || llState != LL_STATE_CONN_SLAVE
            || llSecondaryState != LL_SEC_STATE_IDLE)
    {
        //LOG("==1==\n\r");
        return(LL_STATUS_DISABLE_LATENCY_INACTIVE_CONN);
    }

    // TODO: if latency already disable , return
    pGlobal_config[LL_SWITCH] &=  ~SLAVE_LATENCY_ALLOW;
    connPtr->slaveLatencyAllowed = FALSE;

    if ((AP_TIM1->ControlReg & 0x1) == 0     // timer1 not running
            || llWaitingIrq
            || connPtr->slaveLatency == 0)
    {
        //LOG("==2==%d,%d,%d\n\r",(CP_TIM1->ControlReg & 0x1),llWaitingIrq,connPtr->slaveLatency);
        return (LL_STATUS_DISABLE_LATENCY_DISABLED);
    }

    remain_time1 = read_LL_remainder_time();

    if (remain_time1 <=connPtr->lastTimeToNextEvt*625* 3)        // the timer will expiry soon, so not adjust it
    {
        //LOG("==3==\n\r");
        return(LL_STATUS_DISABLE_LATENCY_PENDING);
    }

    uint16 remain_event;
    remain_event=(remain_time1+connPtr->timerDrift)/(connPtr->lastTimeToNextEvt * 625);
    //20190805 ZQ:
    // considering the event wrap case
    elapse_event= llEventDelta(connPtr->nextEvent,connPtr->currentEvent)-remain_event-(uint8)1;
    //  elapse_time=elapse_event*(connPtr->lastTimeToNextEvt * 625);
    next_event= connPtr->currentEvent + elapse_event + (uint8)2;   // additional 2 connect event for some margin
    old_timerDrift = connPtr->timerDrift;
    llCalcTimerDrift(connPtr->lastTimeToNextEvt,
                     elapse_event + 1,
                     connPtr->sleepClkAccuracy,
                     (uint32*)&(connPtr->timerDrift));
    // timer should expiry early, consider: 1. less timer drift  2. less latency event
    //time_advance = connPtr->lastTimeToNextEvt * (connPtr->nextEvent - next_event) * 625 - (old_timerDrift - connPtr->timerDrift)+550;
    time_advance = connPtr->lastTimeToNextEvt * (llEventDelta(connPtr->nextEvent,next_event) ) * 625 - (old_timerDrift - connPtr->timerDrift)+550;
    // apply the timing advance
    remain_time2 = AP_TIM1->CurrentCount >> 2;

    if(remain_time2<time_advance)
    {
        //LOG("==4==\n\r");
//       pGlobal_config[LL_SWITCH] |=  SLAVE_LATENCY_ALLOW;
//       connPtr->slaveLatencyAllowed = TRUE;
        return (LL_STATUS_DISABLE_LATENCY_MISS_EVT);
    }

    set_timer(AP_TIM1,remain_time2 - time_advance);
//    LOG("currentEvent = %d\r\n", connPtr->currentEvent);
//    LOG("old next_event = %d\r\n", connPtr->nextEvent);
//    LOG("new next_event = %d\r\n", next_event);
    // update connection context
    connPtr->currentEvent += elapse_event;
    //connPtr->currentChan = old_chn;
    //ZQ 20191209 : should use unmap channel
    connPtr->currentChan = connPtr->lastCurrentChan;

    //ZQ 20190805
    // Max latency is 500, preChanMapUdate restore should be trigged within 500 evt
    if(connPtr->preChanMapUpdate.chanMapUpdated==TRUE)
    {
        chanMapDltEvt =  llEventDelta( connPtr->preChanMapUpdate.chanMapUpdateEvent, (connPtr->currentEvent + (uint8) 2));

        //20190806 ZQ:
        //only process the revert chanMap when chanMap Delta event with in (0 500)
        if(chanMapDltEvt>0 && chanMapDltEvt<500)
        {
            connPtr->pendingChanUpdate = TRUE;
            osal_memcpy(&(connPtr->chanMap[0]),&(connPtr->preChanMapUpdate.chanMap[0]),5);
            llProcessChanMap(connPtr, connPtr->chanMap );   // 16MHz clk, cost 116us!
            #if (DBG_DISABLE_LATENCY)
            LOG("[Hit preChnMap] %d\n\r",connPtr->preChanMapUpdate.chanMapUpdateEvent);
            #endif
        }
    }

    if (connPtr->channel_selection == LL_CHN_SEL_ALGORITHM_1)
        connPtr->currentChan = llGetNextDataChan(connPtr,  elapse_event + 2 );
    else
    {
        // channel selection algorithm 2
        connPtr->currentChan = llGetNextDataChanCSA2(next_event,
                                                     (( ( connPtr->accessAddr & 0xFFFF0000 )>> 16 ) ^ ( connPtr->accessAddr  & 0x0000FFFF)),
                                                     connPtr->chanMap,
                                                     connPtr->chanMapTable,
                                                     connPtr->numUsedChans);
    }

    //connPtr->slaveLatency -= (connPtr->nextEvent - next_event);
    connPtr->slaveLatency -= llEventDelta(connPtr->nextEvent, next_event);
    connPtr->nextEvent = next_event;
//
//  LOG("==5==\n\r");
//
//
    #if (DBG_DISABLE_LATENCY)
    LOG("new timer = %d\r\n", remain_time2 - time_advance);
    LOG("old Drift = %d\,new Drift = %d \r\n", old_timerDrift,connPtr->timerDrift);
    LOG("cur_event=%d,elapse_event=%d, next_event=%d,\r\n",connPtr->currentEvent, elapse_event,next_event);
    LOG("RFCHN n=%d o=%d\r\n",connPtr->currentChan,connPtr->lastCurrentChan);
    #endif
    return(LL_STATUS_SUCCESS);
}


llStatus_t LL_PLUS_EnableSlaveLatency0(uint8 connId)
{
    llConnState_t* connPtr;
    // get connection information
    connPtr = &conn_param[connId];        // only 1 connection now.

    // check whether it should enable from disable_latency
    if (FALSE == connPtr->active
            || llState != LL_STATE_CONN_SLAVE
            || llSecondaryState != LL_SEC_STATE_IDLE)
    {
        //LOG("==e1==\n\r");
        return(LL_STATUS_DISABLE_LATENCY_INACTIVE_CONN);
    }

//    // check whether it should enable from disable_latency
//    if (   connPtr->pendingChanUpdate == TRUE
//        || connPtr->pendingParamUpdate == TRUE
//        || connPtr->pendingPhyModeUpdate == TRUE)
//    {
//        LOG("==e2==\n\r");
//        return;
//    }
    pGlobal_config[LL_SWITCH] |=  SLAVE_LATENCY_ALLOW;
    connPtr->slaveLatencyAllowed = TRUE;
    return(LL_STATUS_SUCCESS);
}


llStatus_t LL_ExtendedCreateConnection(uint8 initiator_filter_policy,
                                       uint8 own_address_type,
                                       uint8 peer_address_type,
                                       uint8* peer_address,
                                       uint8 initiating_PHYs,
                                       uint16* scan_interval,
                                       uint16* scan_window,
                                       uint16* conn_interval_min,
                                       uint16* conn_interval_max,
                                       uint16* conn_latency,
                                       uint16* supervision_timeout,
                                       uint16* minimum_CE_length,
                                       uint16* maximum_CE_length)
{
    uint8 number_phys, i;
    llConnState_t* connPtr;
    uint16         txHeader = 0x2205;

    if (g_llScanMode == LL_MODE_LEGACY )
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llScanMode = LL_MODE_EXTENDED;

    // TODO: more sanity checking
    if (((initiating_PHYs & (~(LL_SCAN_PHY_1M_BITMASK | LL_CONN_PHY_2M_BITMASK | LL_SCAN_PHY_CODED_BITMASK))) != 0))
        return LL_STATUS_ERROR_FEATURE_NOT_SUPPORTED;

    // =====  allocate a connection and assure it is valid
    if ( (connPtr = llAllocConnId()) == NULL )
    {
        llSecondaryState = LL_SEC_STATE_IDLE;      // recover llSecondaryState
        // exceeded the number of available connection structures
        return( LL_STATUS_ERROR_CONNECTION_LIMIT_EXCEEDED );
    }

    // ==================  save init parameters
    extInitInfo.ownAddrType = own_address_type;
    extInitInfo.wlPolicy    = initiator_filter_policy;
    number_phys = 0;
    i = 0;

    if (initiating_PHYs & LL_SCAN_PHY_1M_BITMASK)
    {
        extInitInfo.initPHYs[number_phys] = PKT_FMT_BLE1M;
        extInitInfo.scanInterval[number_phys] = scan_interval[i];
        extInitInfo.scanWindow[number_phys] = scan_window[i];
        extInitInfo.conn_latency[number_phys] = conn_latency[i];
        extInitInfo.supervision_timeout[number_phys] = supervision_timeout[i];
        extInitInfo.conn_interval_min[number_phys] = conn_interval_min[i];
        extInitInfo.conn_interval_max[number_phys] = conn_interval_max[i];
        extInitInfo.minimum_CE_length[number_phys] = minimum_CE_length[i];
        extInitInfo.maximum_CE_length[number_phys] = maximum_CE_length[i];
        number_phys ++;
        i ++;
    }

    if (initiating_PHYs & LL_CONN_PHY_2M_BITMASK)
    {
        extInitInfo.is_2M_parameter_present = TRUE;
        // no scan parameters for 2M PHY, the HCI parameters are skipped
        extInitInfo.conn_latency_2Mbps = conn_latency[i];
        extInitInfo.supervision_timeout_2Mbps = supervision_timeout[i];
        extInitInfo.conn_interval_min_2Mbps = conn_interval_min[i];
        extInitInfo.conn_interval_max_2Mbps = conn_interval_max[i];
        extInitInfo.minimum_CE_length_2Mbps = minimum_CE_length[i];
        extInitInfo.maximum_CE_length_2Mbps = maximum_CE_length[i];
        i ++;
    }

    if (initiating_PHYs & LL_SCAN_PHY_CODED_BITMASK)
    {
        extInitInfo.initPHYs[number_phys] = PKT_FMT_BLR125K;
        extInitInfo.scanInterval[number_phys] = scan_interval[i];
        extInitInfo.scanWindow[number_phys] = scan_window[i];
        extInitInfo.conn_latency[number_phys] = conn_latency[i];
        extInitInfo.supervision_timeout[number_phys] = supervision_timeout[i];
        extInitInfo.conn_interval_min[number_phys] = conn_interval_min[i];
        extInitInfo.conn_interval_max[number_phys] = conn_interval_max[i];
        extInitInfo.minimum_CE_length[number_phys] = minimum_CE_length[i];
        extInitInfo.maximum_CE_length[number_phys] = maximum_CE_length[i];
        number_phys ++;
    }

    extInitInfo.numOfScanPHY = number_phys;
    // save the peer address type
    peerInfo.peerAddrType = peer_address_type;

    // save the peer address
    if (peer_address != NULL)              // bug fixed
        LL_COPY_DEV_ADDR_LE( peerInfo.peerAddr, peer_address );

    // save our address type
    extInitInfo.ownAddrType = own_address_type;

    // check the type of own address
    if ( own_address_type == LL_DEV_ADDR_TYPE_PUBLIC )
    {
        // save our address
        LL_COPY_DEV_ADDR_LE( extInitInfo.ownAddr, ownPublicAddr );
    }
    else // LL_DEV_ADDR_TYPE_RANDOM
    {
        // save our address
        LL_COPY_DEV_ADDR_LE( extInitInfo.ownAddr, ownRandomAddr );
    }

    // ========== update connection context
    g_ll_conn_ctx.numLLMasterConns ++;
    // reset connection parameters
    LL_set_default_conn_params(connPtr);
    // clear the connection buffer
    reset_conn_buf(connPtr->connId);
    // save the connection ID with Init
    extInitInfo.connId = connPtr->connId;

    // set the connection channel map
    for (i = 0; i < LL_NUM_BYTES_FOR_CHAN_MAP; i++)
    {
        connPtr->chanMap[i] = chanMapUpdate.chanMap[i];
    }

    // process connection channel map into the data channel table
    llProcessChanMap( connPtr, connPtr->chanMap );
    // ramdomly generate a valid 24 bit CRC value
    connPtr->initCRC = llGenerateCRC();
    // randomly generate a valid, previously unused, 32-bit access address
    connPtr->accessAddr = llGenerateValidAccessAddr();
//  ============= below connection context depends on PHY index, will be selected when scan OK
    #if 0

    if (g_ll_conn_ctx.numLLMasterConns == 1)    // A2 multi-connection, 1st connection, save the connection parameters
    {
        // determine the connection interval based on min and max values
        // Note: Range not used, so assume max value.
        // Note: minLength and maxLength are informational.
        connPtr->curParam.connInterval = conn_interval_max;
        // set the connection timeout
        // Note: The spec says this begins at the end of the CONNECT_REQ, but the
        //       LSTO will be converted into events.
        connPtr->curParam.connTimeout = supervision_timeout;
        // set the slave latency
        connPtr->curParam.slaveLatency = conn_latency;
        // save connection parameter as global
        g_ll_conn_ctx.connInterval = connPtr->curParam.connInterval;                              // unit: 1.25ms
        g_ll_conn_ctx.slaveLatency = connPtr->curParam.slaveLatency;
        g_ll_conn_ctx.connTimeout  = connPtr->curParam.connTimeout;
        g_ll_conn_ctx.per_slot_time = connPtr->curParam.connInterval * 2 / g_maxConnNum;       // unit: 625us
    }
    else
    {
        // determine the connection interval based on min and max values
        // Note: Range not used, so assume max value.
        // Note: minLength and maxLength are informational.
        connPtr->curParam.connInterval = g_ll_conn_ctx.connInterval;
        // set the connection timeout
        // Note: The spec says this begins at the end of the CONNECT_REQ, but the
        //       LSTO will be converted into events.
        connPtr->curParam.connTimeout = g_ll_conn_ctx.connTimeout;
        // set the slave latency
        connPtr->curParam.slaveLatency = g_ll_conn_ctx.slaveLatency;
    }

    #endif
    // set the master's SCA
    connPtr->sleepClkAccuracy = extInitInfo.scaValue;
    // set the window size (units of 1.25ms)
    // Note: Must be the lesser of 10ms and the connection interval - 1.25ms.
    connPtr->curParam.winSize = pGlobal_config[LL_CONN_REQ_WIN_SIZE];
    // set the window offset (units of 1.25ms). TO change if we support multiple connections
    // Note: Normally, the window offset is managed dynamically so that precise
    //       connection start times can be achieved (necessary for multiple
    //       connnections). However, sometimes it is useful to force the window
    //       offset to something specific for testing. This can be done by here
    //       when the project is built with the above define.
    // Note: This define should only be used for testing one connection and will
    //       NOT work when multiple connections are attempted!
    connPtr->curParam.winOffset = pGlobal_config[LL_CONN_REQ_WIN_OFFSET];//2;//LL_WINDOW_OFFSET;
    // set the channel map hop length (5..16)
    // Note: 0..255 % 12 = 0..11 + 5 = 5..16.
    connPtr->hop = (uint8)( (LL_ENC_GeneratePseudoRandNum() % 12) + 5);
    // ===============  fill AUX_CONNECT_REQ PDU, some connection parameters depend on init PHY
    {
        uint8 offset = 0;
        g_tx_adv_buf.txheader = txHeader;
        // setup CONN REQ in connPtr->ll_buf
        LL_ReadBDADDR(&g_tx_adv_buf.data[offset]);                    // initA, Byte 0 ~ 5
        offset += 6;

        if (peer_address != NULL)
            LL_COPY_DEV_ADDR_LE(&g_tx_adv_buf.data[offset], peer_address)     // AdvA,  Byte 6 ~ 11
            offset += 6;

        // Access Address, Byte 12 ~ 15
        memcpy((uint8*)&g_tx_adv_buf.data[offset], (uint8*)&connPtr->accessAddr, 4);
        offset += 4;
        // CRC init, Byte 16 ~ 18
        memcpy((uint8*)&g_tx_adv_buf.data[offset], (uint8*)&connPtr->initCRC, 3);
        offset += 3;
        // WinSize, Byte 19
        g_tx_adv_buf.data[offset] = connPtr->curParam.winSize;
        offset += 1;
        // WinOffset, Byte 20 ~ 21
        memcpy((uint8*)&g_tx_adv_buf.data[offset], (uint8*)&connPtr->curParam.winOffset, 2);
        offset += 2;
        // Interval, Byte 22 ~ 23
//        memcpy((uint8 *)&g_tx_adv_buf.data[offset], (uint8 *)&connPtr->curParam.connInterval, 2);
        offset += 2;
        // Latency, Byte 24 ~ 25
//        memcpy((uint8 *)&g_tx_adv_buf.data[offset], (uint8 *)&connPtr->curParam.slaveLatency, 2);
        offset += 2;
        // Timeout, Byte 26 ~ 27
//        memcpy((uint8 *)&g_tx_adv_buf.data[offset], (uint8 *)&connPtr->curParam.connTimeout, 2);
        offset += 2;
        // Channel Map, Byte 28 ~ 32
        memcpy((uint8*)&g_tx_adv_buf.data[offset], (uint8*)&connPtr->chanMap[0], 5);
        offset += 5;
        // Hop(5bit) + SCA(3bit), Byte 33
        g_tx_adv_buf.data[offset] = (connPtr->hop & 0x1f) | ((connPtr->sleepClkAccuracy & 0x7) << 5);
    }
    // =============== init scan
//  if ( llState == LL_STATE_IDLE )
//        // go ahead and start Init immediately
    extInitInfo.current_index = 0;
    extInitInfo.current_chn = LL_ADV_CHAN_FIRST;
    extInitInfo.current_scan_PHY = extInitInfo.initPHYs[extInitInfo.current_index];
    // enable Init scan
    extInitInfo.scanMode = LL_SCAN_START;
    llSetupExtInit();
//  else
//        osal_set_event(LL_TaskID, LL_EVT_SECONDARY_INIT);
    return LL_STATUS_SUCCESS;
}

// ========================= Periodic Advertiser
llStatus_t LL_SetPeriodicAdvParameter(uint8 adv_handle,
                                      uint16 interval_min,
                                      uint16 interval_max,
                                      uint16 adv_event_properties)
{
    int   i;

    // TODO: add parameters range checking
    if (g_llAdvMode == LL_MODE_LEGACY)
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llAdvMode = LL_MODE_EXTENDED;

    if (interval_max * 1250 < g_advSlotPeriodic)
        return LL_STATUS_ERROR_OUT_OF_CONN_RESOURCES;

    // extended advertiser memory is allocate by application, return fail if not
    if (g_pPeriodicAdvInfo == NULL)
        return LL_STATUS_ERROR_OUT_OF_HEAP;

    // search advertisement handler list
    for (i = 0; i < g_perioAdvNumber; i ++)
    {
        if (g_pPeriodicAdvInfo[i].advHandle == adv_handle)
            break;
    }

    // if not found, search the 1st available slot
    if (i == g_perioAdvNumber)
    {
        for (i = 0; i < g_perioAdvNumber; i ++)
        {
            if (g_pPeriodicAdvInfo[i].advHandle == LL_INVALID_ADV_SET_HANDLE)
            {
                memset(&g_pPeriodicAdvInfo[i], 0, sizeof(periodicAdvInfo_t));
                break;
            }
        }
    }

    if (i == g_perioAdvNumber)
        return LL_STATUS_ERROR_OUT_OF_HEAP;

    // search extended advertisement handler list
    for (i = 0; i < g_extAdvNumber; i ++)
    {
        if (g_pExtendedAdvInfo[i].advHandle == adv_handle)
            break;
    }

    // if not found, report error. refer Vol 4, Part E. 7.8.61 LE Set Periodic Advertising Parameters command
    // The Advertising_Handle parameter identifies the advertising set whose
    // periodic advertising parameters are being configured. If the corresponding
    // advertising set does not already exist, then the Controller shall return the error
    // code Unknown Advertising Identifier (0x42).
    if (i == g_extAdvNumber)
    {
        return LL_STATUS_ERROR_UNKNOWN_ADV_ID;
    }
    else
        g_pExtendedAdvInfo[i].isPeriodic = TRUE;

    // save the advertisement parameters
    g_pPeriodicAdvInfo[i].advHandle  = adv_handle;
    g_pPeriodicAdvInfo[i].adv_event_properties = adv_event_properties;
    g_pPeriodicAdvInfo[i].adv_interval_max     = interval_max;
    g_pPeriodicAdvInfo[i].adv_interval_min     = interval_min;
    // =========== controller select parameters, TBD
    // decide periodic advertising interval
    g_pPeriodicAdvInfo[i].adv_interval = g_advSlotPeriodic;
    return LL_STATUS_SUCCESS;
}


llStatus_t  LL_SetPeriodicAdvData(uint8 adv_handle,
                                  uint8 operation,
                                  uint8  advertising_data_length,
                                  uint8* advertising_data)
{
    int   i;

    // TODO: add parameters range checking
    if (g_llAdvMode == LL_MODE_LEGACY)
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llAdvMode = LL_MODE_EXTENDED;

    if (operation > BLE_EXT_ADV_OP_UNCHANGED_DATA)
        return LL_STATUS_ERROR_BAD_PARAMETER;

    // extended advertiser memory is allocate by application, return fail if not
    if (g_pPeriodicAdvInfo == NULL)
        return LL_STATUS_ERROR_OUT_OF_HEAP;

    // search advertisement handler list
    for (i = 0; i < g_perioAdvNumber; i ++)
    {
        if (g_pPeriodicAdvInfo[i].advHandle == adv_handle)
            break;
    }

    // if not found, return error
    if (i == g_perioAdvNumber)
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    if (operation == BLE_EXT_ADV_OP_FIRST_FRAG ||
            operation == BLE_EXT_ADV_OP_COMPLETE_DATA)
    {
        g_pPeriodicAdvInfo[i].data.advertisingDataLength = 0;
        g_pPeriodicAdvInfo[i].data.dataComplete = FALSE;
    }

    if (g_pPeriodicAdvInfo[i].data.advertisingDataLength + advertising_data_length > g_advSetMaximumLen)
    {
        g_pPeriodicAdvInfo[i].data.advertisingDataLength = 0;
        return LL_STATUS_ERROR_OUT_OF_HEAP;
    }

    // fill advertising set
    g_pPeriodicAdvInfo[i].advHandle          = adv_handle;
    memcpy(&g_pPeriodicAdvInfo[i].data.advertisingData[g_pPeriodicAdvInfo[i].data.advertisingDataLength],
           advertising_data, advertising_data_length);
    g_pPeriodicAdvInfo[i].data.advertisingDataLength += advertising_data_length;

    // last fragment or 1 segment adv data
    if (operation == BLE_EXT_ADV_OP_LAST_FRAG     ||
            operation == BLE_EXT_ADV_OP_COMPLETE_DATA )
        g_pPeriodicAdvInfo[i].data.dataComplete = TRUE;

    return LL_STATUS_SUCCESS;
}

llStatus_t LL_SetPeriodicAdvEnable(uint8   enable,
                                   uint8  advertising_handle)
{
    int i;
    periodicAdvInfo_t* pPrdAdv = NULL;
    extAdvInfo_t* pExtAdv = NULL;

    if (g_llAdvMode == LL_MODE_LEGACY)
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llAdvMode = LL_MODE_EXTENDED;

    if (g_pPeriodicAdvInfo == NULL)
        return LL_STATUS_ERROR_OUT_OF_HEAP;

    // search advertisement handler list
    for (i = 0; i < g_perioAdvNumber; i ++)
    {
        if (g_pPeriodicAdvInfo[i].advHandle == advertising_handle)
        {
            pPrdAdv = &g_pPeriodicAdvInfo[i];
            break;
        }
    }

    // if not found, return error
    if (i == g_perioAdvNumber)
        return LL_STATUS_ERROR_UNKNOWN_ADV_ID;

    if (g_pPeriodicAdvInfo[i].data.dataComplete == FALSE
            && enable == TRUE)
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    // TODO: check whether the adv data could be filled in the adv period(error code LL_STATUS_ERROR_PACKET_TOO_LONG)

    // ====== periodic advertiser scheduler process
    if (enable == FALSE)
    {
        if (pPrdAdv->active == FALSE)   // already disable, do nothing
            return  LL_STATUS_SUCCESS;

        // disable periodic adv process
        // case 1: extended adv is already disable
        // case 2: extended adv is not disable, it will also stop extended adv
        // search the adv in the scheduler list and disable it
        for (i = 0; i < g_schExtAdvNum_periodic; i++)
        {
            if (g_pAdvSchInfo_periodic[i].adv_handler == pPrdAdv[i].advHandle)
            {
                g_pAdvSchInfo_periodic[i].pAdvInfo->active = FALSE;
                ll_delete_adv_task_periodic(i);
                break;
            }
        }
    }
    else     // enable case
    {
        if (pPrdAdv->active == TRUE)
        {
            // TODO: Enabling periodic advertising when it is already enabled can cause the
            // random address to change
            return  LL_STATUS_SUCCESS;
        }

        // search advertisement handler list
        for (i = 0; i < g_extAdvNumber; i ++)
        {
            if (g_pExtendedAdvInfo[i].advHandle == advertising_handle)
            {
                pExtAdv = &g_pExtendedAdvInfo[i];
                break;
            }
        }

        // initial periodic adv sync info context
        // ramdomly generate a valid 24 bit CRC value
        pPrdAdv->crcInit = llGenerateCRC();
        // randomly generate a valid, previously unused, 32-bit access address
        pPrdAdv->AA = llGenerateValidAccessAddr();
        pPrdAdv->sca = LL_SCA_MASTER_DEFAULT;
        pPrdAdv->chn_map[0] =  0xFF;
        pPrdAdv->chn_map[1] =  0xFF;
        pPrdAdv->chn_map[2] =  0xFF;
        pPrdAdv->chn_map[3] =  0xFF;
        pPrdAdv->chn_map[4] =  0x1F;

        // 2020-1-7 add for CSA2
        // the used channel map uses 1 bit per data channel, or 5 bytes for 37 chans
        for (i = 0; i < LL_NUM_BYTES_FOR_CHAN_MAP; i++)
        {
            // replace the current channel map with the update channel map
            // Note: This needs to be done so that llGetNextDataChan can more
            //       efficiently determine if the next channel is used.
            //      connPtr->chanMap[i] = chanMap[i];

            // for each channel given by a bit in each of the bytes
            // Note: When i is on the last byte, only 5 bits need to be checked, but
            //       it is easier here to check all 8 with the assumption that the rest
            //       of the reserved bits are zero.
            for (uint8 j = 0; j < 8; j++)
            {
                // check if the channel is used; only interested in used channels
                if ( (pPrdAdv->chn_map[i] >> j) & 1 )
                {
                    // sequence used channels in ascending order
                    pPrdAdv->chanMapTable[pPrdAdv->numUsedChans] = (i * 8U) + j;
                    // count it
                    pPrdAdv->numUsedChans++;
                }
            }
        }

        // case 1: extended adv is not enable, not trigger periodic adv
        if (pExtAdv == NULL || pExtAdv->active == FALSE)
        {
        }
        // case 2: extende adv is enable, start extended adv + periodic adv
        else
        {
            pExtAdv->data.DIDInfo = ll_generateExtAdvDid(pExtAdv->data.DIDInfo);
            ll_add_adv_task_periodic(pPrdAdv, pExtAdv);
        }
    }

    pPrdAdv->active = enable;
    return( LL_STATUS_SUCCESS );
}

llStatus_t LL_PeriodicAdvertisingCreateSync(uint8 options,
                                            uint8 advertising_SID,
                                            uint8 advertiser_Address_Type,
                                            uint8* advertiser_Address,
                                            uint16 skip,
                                            uint16 sync_Timeout,
                                            uint8 sync_CTE_Type)
{
    if (g_llScanMode == LL_MODE_LEGACY )
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llScanMode = LL_MODE_EXTENDED;

    // TODO: add more sanity checking
    if (scanSyncInfo.valid == TRUE)
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    if (options & LL_PERIODIC_ADV_CREATE_SYNC_INIT_RPT_DISABLE_BITMASK)     // we not support this mode
        return LL_STATUS_ERROR_CONN_FAILED_TO_BE_ESTABLISHED;

    scanSyncInfo.options = options;
    scanSyncInfo.advertising_SID = advertising_SID;
    scanSyncInfo.advertiser_Address_Type = advertiser_Address_Type;

    if (advertiser_Address != NULL)
        memcpy(scanSyncInfo.advertiser_Address, advertiser_Address, LL_DEVICE_ADDR_LEN);

    scanSyncInfo.skip = skip;
    scanSyncInfo.sync_Timeout = sync_Timeout;
    scanSyncInfo.sync_CTE_Type = sync_CTE_Type;
    scanSyncInfo.valid = TRUE;
    return( LL_STATUS_SUCCESS );
}

llStatus_t LL_PeriodicAdvertisingCreateSyncCancel(void)
{
    if (g_llScanMode == LL_MODE_LEGACY )
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llScanMode = LL_MODE_EXTENDED;

    if (scanSyncInfo.valid == FALSE)
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    scanSyncInfo.valid = FALSE;
    return( LL_STATUS_SUCCESS );
}

// TODO
llStatus_t LL_PeriodicAdvertisingTerminateSync(          uint16 sync_handle)
{
    if (g_llScanMode == LL_MODE_LEGACY )
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llScanMode = LL_MODE_EXTENDED;
    // TODO: search periodic scanner list and remove it from schedule list
    llDeleteSyncHandle(sync_handle);
    return( LL_STATUS_SUCCESS );
}

// ==========
llStatus_t LL_AddDevToPeriodicAdvList(uint8  addrType,
                                      uint8* devAddr,
                                      uint8 sid)
{
    int i;

    if (g_llScanMode == LL_MODE_LEGACY )
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llScanMode = LL_MODE_EXTENDED;

    // create Sync pending
    if (scanSyncInfo.valid == TRUE)
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    // check the device address type
    if ( (addrType != LL_DEV_ADDR_TYPE_PUBLIC) &&
            (addrType != LL_DEV_ADDR_TYPE_RANDOM) )
    {
        return( LL_STATUS_ERROR_BAD_PARAMETER );
    }

    // check if there was room for the entry
    if (g_llPrdAdvDeviceNum >= LL_PRD_ADV_ENTRY_NUM)
        return( LL_STATUS_ERROR_PAL_TABLE_FULL );

    // if the entry already exist, return 0x12 error code
    for (i = 0; i < LL_PRD_ADV_ENTRY_NUM; i++)
    {
        if (g_llPeriodicAdvlist[i].addrType == addrType
                && g_llPeriodicAdvlist[i].sid == sid
                && (g_llPeriodicAdvlist[i].addr[0] == devAddr[0]
                    && g_llPeriodicAdvlist[i].addr[1] == devAddr[1]
                    && g_llPeriodicAdvlist[i].addr[2] == devAddr[2]
                    && g_llPeriodicAdvlist[i].addr[3] == devAddr[3]
                    && g_llPeriodicAdvlist[i].addr[4] == devAddr[4]
                    && g_llPeriodicAdvlist[i].addr[5] == devAddr[5]))
            return LL_STATUS_ERROR_BAD_PARAMETER;
    }

    // add the device to a empty record
    for (i = 0; i < LL_PRD_ADV_ENTRY_NUM; i++)
    {
        if (g_llPeriodicAdvlist[i].addrType == 0xff)       // empty record
        {
            g_llPeriodicAdvlist[i].addrType = addrType;
            osal_memcpy(&g_llPeriodicAdvlist[i].addr[0], &devAddr[0], LL_DEVICE_ADDR_LEN);
            g_llPeriodicAdvlist[i].sid  = sid;
            g_llPrdAdvDeviceNum ++;
            break;
        }
    }

    return( LL_STATUS_SUCCESS );
}

llStatus_t LL_RemovePeriodicAdvListDevice(uint8  addrType,
                                          uint8* devAddr,
                                          uint8 sid)
{
    int i;

    if (g_llScanMode == LL_MODE_LEGACY )
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llScanMode = LL_MODE_EXTENDED;

    // create Sync pending
    if (scanSyncInfo.valid == TRUE)
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    // search the device
    for (i = 0; i < LL_PRD_ADV_ENTRY_NUM; i++)
    {
        if (g_llPeriodicAdvlist[i].addrType == addrType
                && g_llPeriodicAdvlist[i].sid == sid
                && (g_llPeriodicAdvlist[i].addr[0] == devAddr[0]
                    && g_llPeriodicAdvlist[i].addr[1] == devAddr[1]
                    && g_llPeriodicAdvlist[i].addr[2] == devAddr[2]
                    && g_llPeriodicAdvlist[i].addr[3] == devAddr[3]
                    && g_llPeriodicAdvlist[i].addr[4] == devAddr[4]
                    && g_llPeriodicAdvlist[i].addr[5] == devAddr[5]))
            break;
    }

    if (i == LL_PRD_ADV_ENTRY_NUM
            || g_llPrdAdvDeviceNum == 0)
        return LL_STATUS_ERROR_UNKNOWN_ADV_ID;

    g_llPeriodicAdvlist[i].addrType = 0xff;
    memset(g_llPeriodicAdvlist[i].addr, 0, LL_DEVICE_ADDR_LEN);
    g_llPrdAdvDeviceNum --;
    return( LL_STATUS_SUCCESS );
}

llStatus_t LL_ClearPeriodicAdvList(void)
{
    int i;

    if (g_llScanMode == LL_MODE_LEGACY )
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llScanMode = LL_MODE_EXTENDED;

    // create Sync pending
    if (scanSyncInfo.valid == TRUE)
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    for (i = 0; i < LL_PRD_ADV_ENTRY_NUM; i++)
    {
        g_llPeriodicAdvlist[i].addrType = 0xff;
        memset(g_llPeriodicAdvlist[i].addr, 0, LL_DEVICE_ADDR_LEN);
    }

    g_llPrdAdvDeviceNum = 0;
    return( LL_STATUS_SUCCESS );
}

llStatus_t LL_ReadPeriodicAdvListSize(uint8* numEntries)
{
    if (g_llScanMode == LL_MODE_LEGACY )
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;

    g_llScanMode = LL_MODE_EXTENDED;
    *numEntries = LL_PRD_ADV_ENTRY_NUM;
    return( LL_STATUS_SUCCESS );
}

// ==================

llStatus_t LL_ConnectionlessCTE_TransmitParam0(              uint8 advertising_handle,
                                                             uint8 len,
                                                             uint8 type,
                                                             uint8 count,
                                                             uint8 Pattern_LEN,
                                                             uint8* AnaIDs)
{
    int i,j;
    uint8 suppCTEType=0;
    periodicAdvInfo_t* pPrdAdv = NULL;

    if (g_pPeriodicAdvInfo == NULL)
        return LL_STATUS_ERROR_OUT_OF_HEAP;

    // search advertisement handler list
    for (i = 0; i < g_perioAdvNumber; i ++)
    {
        if (g_pPeriodicAdvInfo[i].advHandle == advertising_handle)
        {
            pPrdAdv = &g_pPeriodicAdvInfo[i];
            break;
        }
    }

    // if period advertising set not cteate , return unknown advertising Identifier
    if( i == g_perioAdvNumber )
    {
        return LL_STATUS_ERROR_UNKNOWN_ADV_ID;
    }

    // return error code Command Disallowed when CTE hasve been enabled
    if( pPrdAdv->PrdCTEInfo.enable == TRUE )
    {
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;
    }

    // check Controller support CTE Type
    suppCTEType = deviceFeatureSet.featureSet[LL_CTE_FEATURE_IDX] & 0x60;
    pPrdAdv->PrdCTEInfo.CTE_Type = type;

    switch (type)
    {
    case CONNLESS_CTE_TYPE_AOA:

        // ignore Pattern_LEN and AnaIDs
        if( !( suppCTEType & LL_AOA_SUPPORT ) )
        {
            return LL_STATUS_ERROR_FEATURE_NOT_SUPPORTED;
        }

        break;

    case CONNLESS_CTE_TYPE_AOD_1us:
    case CONNLESS_CTE_TYPE_AOD_2us:

        // TODO
        // should check the Controller support AOD 1us or 2us
        if( !( suppCTEType & LL_AOD_SUPPORT ) || ( Pattern_LEN > LL_CTE_MAX_PATTERN_LEN ) )
        {
            return LL_STATUS_ERROR_FEATURE_NOT_SUPPORTED;
        }
        else
        {
            pPrdAdv->PrdCTEInfo.pattern_LEN = Pattern_LEN;

            for( j = 0; j < Pattern_LEN; j++)
            {
                if( AnaIDs[j] > LL_CTE_MAX_ANT_ID )
                    return LL_STATUS_ERROR_FEATURE_NOT_SUPPORTED;
            }

            osal_memcpy(pPrdAdv->PrdCTEInfo.AntID, AnaIDs, LL_CTE_MAX_PATTERN_LEN );
        }

        break;

    default:
        // unsupported CTE Type
//          return LL_STATUS_ERROR_FEATURE_NOT_SUPPORTED;
        break;
    }

    // parameter len in 8us units shall not greater than LL_CTE_MAX_SUPP_LEN
    // max CTE_Count check in each PA Event
    if(( len < LL_CTE_MIN_SUPP_LEN ) || ( len > LL_CTE_MAX_SUPP_LEN ) || ( count > LL_CTE_MAX_PA_INTV_CNT ))
    {
        return LL_STATUS_ERROR_FEATURE_NOT_SUPPORTED;
    }
    else
    {
        pPrdAdv->PrdCTEInfo.CTE_Length = len;
        pPrdAdv->PrdCTEInfo.CTE_Count   = count;
        pPrdAdv->PrdCTEInfo.CTE_Count_Idx = 0;
    }

    return LL_STATUS_SUCCESS;
}

llStatus_t LL_ConnectionlessCTE_TransmitEnable0(                uint8 advertising_handle,uint8 enable)
{
    int i,j;
    uint32 ant1=0,ant0=0;
    periodicAdvInfo_t* pPrdAdv = NULL;

    if (g_pPeriodicAdvInfo == NULL)
        return LL_STATUS_ERROR_OUT_OF_HEAP;

    // search advertisement handler list
    for (i = 0; i < g_perioAdvNumber; i ++)
    {
        // equal advertising_handle indicate that it has already issued LL_SetPeriodicAdvParameter command
        if (g_pPeriodicAdvInfo[i].advHandle == advertising_handle)
        {
            pPrdAdv = &g_pPeriodicAdvInfo[i];
            break;
        }
    }

    // advertising set not exist return error code
    if( i == g_perioAdvNumber )
    {
        return LL_STATUS_ERROR_UNKNOWN_ADV_ID;
    }

    // should enable periodic advertising first
    // check g_pPeriodicAdvInfo[i].active means : if periodic advertising set disabled that should not enable CTE Tx

    // 2020-02-10 comment
    // TODO :: only check if PA Parameter is set before CTE Transmit enable
//  if( ( g_pPeriodicAdvInfo[i].active != TRUE ) )
//  {
//      return LL_STATUS_ERROR_COMMAND_DISALLOWED;
//  }

    // means the host has not issue LL_ConnectionlessCTE_TransmitParam before this command
    if( 0 == pPrdAdv->PrdCTEInfo.CTE_Length )
    {
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;
    }

    // check secondary advertising PHY allow CTE
    if( pPrdAdv->secondaryAdvPHY > LL_SECOND_ADV_PHY_2M )
    {
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;
    }

    switch ( enable )
    {
    case LL_CTE_ENABLE:
        pPrdAdv->PrdCTEInfo.enable = LL_CTE_ENABLE;

        switch ( pPrdAdv->PrdCTEInfo.CTE_Type )
        {
        case CONNLESS_CTE_TYPE_AOA:
            // AOA transmit not switching Antenna , shall not config switch timing
//                  ll_hw_set_ant_switch_timing(8, 40);
            ll_hw_set_ant_switch_mode( LL_HW_ANT_SW_CTE_OFF );
            break;

        case CONNLESS_CTE_TYPE_AOD_1us:
        case CONNLESS_CTE_TYPE_AOD_2us:

            // combination ant pattern
            for( j = 0 ; j < pPrdAdv->PrdCTEInfo.pattern_LEN ; j++ )
            {
                // PHY SDK 4 bit represents an Antenna ID, so uint32 represents max 8 antenna
                if( j < 8 )
                {
                    ant0 |=  ( ((uint32)pPrdAdv->PrdCTEInfo.AntID[j]) << (4*j) );
                }
                else
                {
                    ant1 |=  ( ((uint32)pPrdAdv->PrdCTEInfo.AntID[j]) << (4*(j-8)) );
                }
            }

            ll_hw_set_ant_pattern( ant1, ant0 );
            // antWin : 8, 1us switching + 1us sampling 8 *0.25us unit = 2us
            // antWin : 16,2us switching + 2us sampling 16*0.25us unit = 4us
            ll_hw_set_ant_switch_timing((pPrdAdv->PrdCTEInfo.CTE_Type==CONNLESS_CTE_TYPE_AOD_1us?8:16), 40);
            ll_hw_set_ant_switch_mode( LL_HW_ANT_SW_TX_MANU );
            break;

        default:
            break;
        }

        // 2020-02-10 comment
        // TODO: cte_txSupp instruction executed here, whether all packet contain CTEInfo field ,
        // and those that shall not include CTEInfo filed , eg AUX_EXT_IND
//          ll_hw_set_cte_txSupp( CTE_SUPP_LEN_SET | g_pPeriodicAdvInfo[i].PrdCTEInfo.CTE_Length );
        break;

    case LL_CTE_DISABLE:
        pPrdAdv->PrdCTEInfo.enable = LL_CTE_DISABLE;
        break;

    default:
        break;
    }

    return LL_STATUS_SUCCESS;
}

llStatus_t LL_ConnectionlessIQ_SampleEnable0(            uint16 sync_handle,
                                                         uint8 enable,
                                                         uint8 slot_Duration,
                                                         uint8 MaxSampledCTEs,
                                                         uint8 pattern_len,
                                                         uint8* AnaIDs)
{
    int i,j;
    uint32 ant1=0,ant0=0;

    for( i = 0; i < MAX_NUM_LL_PRD_ADV_SYNC ; i++ )
    {
        if( g_llPeriodAdvSyncInfo[i].syncHandler == sync_handle )
            break;
    }

    // only support 1 periodic advertising , so only support 1 IQ Sampling
    // TODO
    // Logic issus : JIRA BBBBLESTAC-10 issue 4
    if( ( scanSyncInfo.valid != TRUE ) || ( i == MAX_NUM_LL_PRD_ADV_SYNC ) )
    {
        return LL_STATUS_ERROR_UNKNOWN_ADV_ID;
    }

    if( MaxSampledCTEs > LL_CTE_MAX_IQ_SAMP_CNT)
    {
        return LL_STATUS_ERROR_UNEXPECTED_PARAMETER;
    }
    else
    {
        g_llPeriodAdvSyncInfo[i].IQSampleInfo.CTE_Count = MaxSampledCTEs;
    }

    // PHY SDK default support 1us and 2us slot dureation, so no condition determination
    g_llPeriodAdvSyncInfo[i].IQSampleInfo.slot_Duration = slot_Duration;

    switch ( enable )
    {
    case LL_IQ_SAMP_ENABLE:
        if( g_llPeriodAdvSyncInfo[i].IQSampleInfo.enable )
        {
            return LL_STATUS_ERROR_COMMAND_DISALLOWED;
        }
        else
        {
            g_llPeriodAdvSyncInfo[i].IQSampleInfo.enable = LL_IQ_SAMP_ENABLE;

            switch ( g_llPeriodAdvSyncInfo[i].IQSampleInfo.CTE_Type )
            {
            case CONNLESS_CTE_TYPE_AOA:
                g_llPeriodAdvSyncInfo[i].IQSampleInfo.pattern_LEN = pattern_len;

                for( j = 0; j < pattern_len; j++)
                {
                    if( AnaIDs[j] > LL_CTE_MAX_ANT_ID )
                        return LL_STATUS_ERROR_FEATURE_NOT_SUPPORTED;
                }

                osal_memcpy(g_llPeriodAdvSyncInfo[i].IQSampleInfo.AntID, AnaIDs, LL_CTE_MAX_PATTERN_LEN );
                ll_hw_set_ant_switch_mode( LL_HW_ANT_SW_RX_MANU );

                // combination ant pattern
                for( j = 0 ; j < g_llPeriodAdvSyncInfo[i].IQSampleInfo.pattern_LEN ; j++ )
                {
                    // PHY SDK 4 bit represents an Antenna ID, so uint32 represents max 8 antenna
                    if( j < 8 )
                    {
                        ant0 |=  ( ((uint32)g_llPeriodAdvSyncInfo[i].IQSampleInfo.AntID[j]) << (4*j) );
                    }
                    else
                    {
                        ant1 |=  ( ((uint32)g_llPeriodAdvSyncInfo[i].IQSampleInfo.AntID[j]) << (4*(j-8)) );
                    }
                }

                ll_hw_set_ant_pattern( ant1, ant0 );
                break;

            case CONNLESS_CTE_TYPE_AOD_1us:
            case CONNLESS_CTE_TYPE_AOD_2us:
                // AOD receiver not switching Antenna
                ll_hw_set_ant_switch_mode( LL_HW_ANT_SW_CTE_OFF );
                ll_hw_set_ant_pattern( 0, 0 );
                break;

            default:
                break;
            }

            // antWin : 8, 1us switching + 1us sampling 8 *0.25us unit = 2us
            // antWin : 16,2us switching + 2us sampling 16*0.25us unit = 4us
            ll_hw_set_ant_switch_timing((slot_Duration==LL_IQ_SW_SAMP_1US?8:16), 40);
            ll_hw_set_cte_rxSupp( CTE_SUPP_LEN_SET | g_pPeriodicAdvInfo[i].PrdCTEInfo.CTE_Length );
        }

        break;

    case LL_IQ_SAMP_DISABLE:
        g_llPeriodAdvSyncInfo[i].IQSampleInfo.enable = LL_IQ_SAMP_ENABLE;
        break;

    default:
        break;
    }

    return LL_STATUS_SUCCESS;
}


llStatus_t LL_Set_ConnectionCTE_ReceiveParam0(                  uint16 connHandle,
                                                                uint8 enable,
                                                                uint8 slot_Duration,
                                                                uint8 pattern_len,
                                                                uint8* AnaIDs)
{
    uint8 i,j;
    uint32 ant1=0,ant0=0;
    // llConnState_t.connid means connHandle
    llConnState_t* connPtr;

    for( i = 0; i< g_maxConnNum; i++)
    {
        connPtr = &conn_param[i];

        if( connPtr->connId == connHandle )
        {
            break;
        }
    }

    if( g_maxConnNum == i )
    {
        return LL_STATUS_ERROR_INACTIVE_CONNECTION;
    }

    switch( enable )
    {
    case LL_CONN_IQSAMP_ENABLE:
        if( connPtr->llConnCTE.enable == LL_CONN_IQSAMP_ENABLE )
        {
            // current connection CTE already enabled, return error
            return LL_STATUS_ERROR_COMMAND_DISALLOWED;
        }
        else
        {
            connPtr->llConnCTE.slot_Duration = slot_Duration;
            connPtr->llConnCTE.enable = LL_CONN_IQSAMP_ENABLE;
            connPtr->llConnCTE.pattern_LEN = pattern_len;

            for( j = 0; j < pattern_len; j++)
            {
                if( AnaIDs[j] > LL_CTE_MAX_ANT_ID )
                    return LL_STATUS_ERROR_FEATURE_NOT_SUPPORTED;
            }

            osal_memcpy(connPtr->llConnCTE.AntID, AnaIDs, LL_CTE_MAX_PATTERN_LEN );

            for( j = 0 ; j < connPtr->llConnCTE.pattern_LEN ; j++ )
            {
                // PHY SDK 4 bit represents an Antenna ID, so uint32 represents max 8 antenna
                if( j < 8 )
                {
                    ant0 |=  ( ((uint32)connPtr->llConnCTE.AntID[j]) << (4*j) );
                }
                else
                {
                    ant1 |=  ( ((uint32)connPtr->llConnCTE.AntID[j]) << (4*(j-8)) );
                }
            }

            ll_hw_set_ant_pattern( ant1, ant0 );
            ll_hw_set_ant_switch_mode( LL_HW_ANT_SW_CTE_AUTO );
            // antWin : 8, 1us switching + 1us sampling 8 *0.25us unit = 2us
            // antWin : 16,2us switching + 2us sampling 16*0.25us unit = 4us
            ll_hw_set_ant_switch_timing((slot_Duration==LL_IQ_SW_SAMP_1US?8:16), 40);
            // 2020-02-12 comment receive parameter set rxSupp len = 0
            ll_hw_set_cte_rxSupp(CTE_SUPP_NULL);
//              ll_hw_set_cte_rxSupp( CTE_SUPP_LEN_SET | g_pPeriodicAdvInfo[i].PrdCTEInfo.CTE_Length );
        }

        break;

    case LL_CONN_IQSAMP_DISENABLE:
        connPtr->llConnCTE.enable = LL_CONN_IQSAMP_DISENABLE;
        break;

    default:
        break;
    }

    return LL_STATUS_SUCCESS;
}


llStatus_t LL_Connection_CTE_Request_Enable0(                  uint16 connHandle,
                                                               uint8 enable,
                                                               uint16 Interval,
                                                               uint8 len,
                                                               uint8 type)
{
    uint8 i;
//  uint32 ant1=0,ant0=0;
    // llConnState_t.connid means connHandle
    llConnState_t* connPtr;

    for( i = 0; i< g_maxConnNum; i++)
    {
        connPtr = &conn_param[i];

        if( connPtr->connId == connHandle )
        {
            break;
        }
    }

    if( g_maxConnNum == i )
    {
        return LL_STATUS_ERROR_INACTIVE_CONNECTION;
    }

    // check if peer device feature set support CTE Response
    if( ( connPtr->featureSetInfo.featureSet[LL_CTE_FEATURE_IDX] & LL_CONN_CTE_RSP ) !=  LL_CONN_CTE_RSP )
    {
        return LL_STATUS_ERROR_UNSUPPORTED_REMOTE_FEATURE;
    }

    switch ( enable )
    {
    case LL_CONN_CTE_REQ_ENABLE:

        // if Controller already enable ,return error code LL_STATUS_ERROR_COMMAND_DISALLOWED
        // if the Host issue this command before issue the LL_Set_ConnectionCTE_ReceiveParam command at least
        // once on the connection , the Controller shall return error code LL_STATUS_ERROR_COMMAND_DISALLOWED
        if( ( LL_CONN_CTE_REQ_ENABLE == connPtr->llCTE_ReqFlag ) || \
                !( LL_CONN_IQSAMP_ENABLE == connPtr->llConnCTE.enable ))
        {
            return LL_STATUS_ERROR_COMMAND_DISALLOWED;
        }

        connPtr->llCTE_ReqFlag = LL_CONN_CTE_REQ_ENABLE;

        if(( len < LL_CTE_MIN_SUPP_LEN ) || ( len > LL_CTE_MAX_SUPP_LEN ))
        {
            return LL_STATUS_ERROR_FEATURE_NOT_SUPPORTED;
        }
        else
        {
            connPtr->llConnCTE.CTE_Length = len;
        }

        connPtr->llConnCTE.CTE_Type = type;

        // TODO
        // check connection PHY support CTE ?
//          if( connPtr->llPhyModeCtrl.local )
//          {
//              return LL_STATUS_ERROR_COMMAND_DISALLOWED
//          }
        if( 0 == Interval)
        {
            // CTE Request Interval = 0 , means Initiate the CTE request procedure once
        }
        else
        {
            // TODO
            // shall verify when disabled slaveLatency case
            if( connPtr->slaveLatencyAllowed )
            {
                if( Interval <= connPtr->slaveLatency )
                {
                    return LL_STATUS_ERROR_COMMAND_DISALLOWED;
                }
            }
        }

        connPtr->llConnCTE.CTE_Request_Intv = Interval;
        llEnqueueCtrlPkt( connPtr, LL_CTRL_CTE_REQ );
        break;

    case LL_CONN_CTE_REQ_DISENABLE:
        connPtr->llCTE_ReqFlag = LL_CONN_CTE_REQ_DISENABLE;
        break;
    }

    return LL_STATUS_SUCCESS;
}


llStatus_t LL_Set_ConnectionCTE_TransmitParam0(                 uint16 connHandle,
                                                                uint8 type,
                                                                uint8 pattern_len,
                                                                uint8* AnaIDs)
{
    uint8 i,j;
    uint32 ant1=0,ant0=0;
    // llConnState_t.connid means connHandle
    llConnState_t* connPtr;

    for( i = 0; i< g_maxConnNum; i++)
    {
        connPtr = &conn_param[i];

        if( connPtr->connId == connHandle )
        {
            break;
        }
    }

    if( g_maxConnNum == i )
    {
        return LL_STATUS_ERROR_INACTIVE_CONNECTION;
    }

    if( connPtr->llConnCTE.enable )
    {
        return LL_STATUS_ERROR_COMMAND_DISALLOWED;
    }

    // TODO:
    // if CTE Type parameter has a bit set for CTE that the controller does not support ,
    // the controller controller shall return error code
    // return LL_STATUS_ERROR_FEATURE_NOT_SUPPORTED

    // pattern len check
    if( pattern_len > LL_CTE_MAX_PATTERN_LEN )
    {
        return LL_STATUS_ERROR_FEATURE_NOT_SUPPORTED;
    }
    else
    {
        connPtr->llConnCTE.pattern_LEN = pattern_len;

        for( j = 0; j < pattern_len; j++)
        {
            if( AnaIDs[j] > LL_CTE_MAX_ANT_ID )
                return LL_STATUS_ERROR_FEATURE_NOT_SUPPORTED;
        }

        osal_memcpy(connPtr->llConnCTE.AntID, AnaIDs, LL_CTE_MAX_PATTERN_LEN );
    }

    switch ( type )
    {
    case CONN_CTE_TYPE_AOA:
        ll_hw_set_ant_pattern( 0, 0 );
        ll_hw_set_ant_switch_mode( LL_HW_ANT_SW_CTE_OFF );
        break;

    case CONN_CTE_TYPE_AOD_1us:
    case CONN_CTE_TYPE_AOD_2us:

        // combination ant pattern
        for( j = 0 ; j < connPtr->llConnCTE.pattern_LEN ; j++ )
        {
            // PHY SDK 4 bit represents an Antenna ID, so uint32 represents max 8 antenna
            if( j < 8 )
            {
                ant0 |=  ( ((uint32)connPtr->llConnCTE.AntID[j]) << (4*j) );
            }
            else
            {
                ant1 |=  ( ((uint32)connPtr->llConnCTE.AntID[j]) << (4*(j-8)) );
            }
        }

        ll_hw_set_ant_pattern( ant1, ant0 );
        ll_hw_set_ant_switch_timing((type==CONN_CTE_TYPE_AOD_1us?8:16), 40);
        ll_hw_set_ant_switch_mode( LL_HW_ANT_SW_CTE_AUTO );
        break;

    default:
        break;
    }

//  ll_hw_set_cte_txSupp( CTE_SUPP_NULL);
//  ll_hw_set_cte_txSupp( CTE_SUPP_LEN_SET | connPtr->llConnCTE.pattern_LEN );
    // indicate Connection CTE Transmit parameter has been set
    connPtr->llConnCTE.enable = TRUE;
    return LL_STATUS_SUCCESS;
}

llStatus_t LL_Connection_CTE_Response_Enable0(                uint16 connHandle,uint8 enable)
{
    uint8 i;
//  uint32 ant1=0,ant0=0;
    // llConnState_t.connid means connHandle
    llConnState_t* connPtr;

    for( i = 0; i< g_maxConnNum; i++)
    {
        connPtr = &conn_param[i];

        if( connPtr->connId == connHandle )
        {
            break;
        }
    }

    if( g_maxConnNum == i )
    {
        return LL_STATUS_ERROR_INACTIVE_CONNECTION;
    }

    switch ( enable )
    {
    case LL_CONN_CTE_RSP_ENABLE:

        // if the host issue this command before LL_Set_ConnectionCTE_TransmitParam at least once on the connection
        // the controller shall return error code: LL_STATUS_ERROR_COMMAND_DISALLOWED
        if( !connPtr->llConnCTE.enable )
        {
            return LL_STATUS_ERROR_COMMAND_DISALLOWED;
        }

        // TODO
        // check connection PHY support CTE ?
//          if( connPtr->llPhyModeCtrl.local )
//          {
//              return LL_STATUS_ERROR_COMMAND_DISALLOWED
//          }
        connPtr->llCTE_RspFlag = LL_CONN_CTE_RSP_ENABLE;
        break;

    case LL_CONN_CTE_RSP_DISENABLE:
        connPtr->llCTE_RspFlag = LL_CONN_CTE_RSP_DISENABLE;
        break;

    default:
        break;
    }

    return LL_STATUS_SUCCESS;
}

llStatus_t LL_READ_Anatenna_Info(          uint8* param )
{
    // PHY SDK default support
    // switching_sampling_rate
    param[0] =  LL_CONTROLLER_SUPP_1US_AOD_TX       | \
                LL_CONTROLLER_SUPP_1US_AOD_SAMP     | \
                LL_CONTROLLER_SUPP_1US_AOA_TX_SAMP;
    // Antenna length
    param[1] = LL_CTE_MAX_ANTENNA_LEN;
    // MAX Length of switch pattern
    param[2] = LL_CTE_MAX_PATTERN_LEN;
    // MAX CTE Length
    param[3] = LL_CTE_MAX_SUPP_LEN;
    return LL_STATUS_SUCCESS;
}

// power compensation
llStatus_t LL_Read_Rf_Path_Compensation(uint8* param)
{
    param[0] = LO_UINT16(g_rfTxPathCompensation);
    param[1] = HI_UINT16(g_rfTxPathCompensation);
    param[2] = LO_UINT16(g_rfRxPathCompensation);
    param[3] = HI_UINT16(g_rfRxPathCompensation);
    return LL_STATUS_SUCCESS;
}

llStatus_t LL_Write_Rf_Path_Compensation(int16 tx_compensation, int16 rx_compensation)
{
    g_rfTxPathCompensation = tx_compensation;
    g_rfRxPathCompensation = rx_compensation;
    return LL_STATUS_SUCCESS;
}

llStatus_t LL_Read_Transmit_Power( uint8* param)
{
    int8   minTxPwr, maxTxPwr;
    // temporary set as protocol range
    minTxPwr = -20;
    maxTxPwr = 20;
    param[1] = LO_UINT16(minTxPwr);
    param[2] = HI_UINT16(maxTxPwr);
    return LL_STATUS_SUCCESS;
}
//////////////////////////////////////// Vendor specific command API //////////////////////////////////////
/*
** Vendor Specific Command API
*/

/*******************************************************************************
    @fn          LL_EXT_SetRxGain Vendor Specific API

    @brief       This function is used to to set the RF RX gain.

    input parameters

    @param       rxGain - LL_EXT_RX_GAIN_STD, LL_EXT_RX_GAIN_HIGH

    output parameters

    @param       cmdComplete - Boolean to indicate the command is still pending.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER
*/
llStatus_t LL_EXT_SetRxGain( uint8 rxGain,
                             uint8* cmdComplete )
{
	(void) rxGain;
	(void) cmdComplete;
    return 0;
}


/*******************************************************************************
    @fn          LL_EXT_SetTxPower0 Vendor Specific API

    @brief       This function is used to to set the RF TX power.

    input parameters

    @param       txPower - LL_EXT_TX_POWER_0_DBM, LL_EXT_TX_POWER_4_DBM

    output parameters

    @param       cmdComplete - Boolean to indicate the command is still pending.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER
*/
llStatus_t LL_EXT_SetTxPower0( uint8 txPower,
                               uint8* cmdComplete )
{
    uint32 temp;
    *cmdComplete = TRUE;
    temp = *(volatile uint32*)(0x400302b8);
    *(volatile uint32*)(0x400300b8) = (temp & 0xfffe0fff) | ((txPower & 0x1f) << 12);
    return ( LL_STATUS_SUCCESS );
}




/*******************************************************************************
    @fn          LL_EXT_ClkDivOnHalt Vendor Specific API

    @brief       This function is used to enable or disable dividing down the
                system clock while halted.

                Note: This command is disallowed if haltDuringRf is not defined.

    input parameters

    @param       control - LL_EXT_ENABLE_CLK_DIVIDE_ON_HALT,
                          LL_EXT_DISABLE_CLK_DIVIDE_ON_HALT

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_COMMAND_DISALLOWED
*/
llStatus_t LL_EXT_ClkDivOnHalt( uint8 control )
{
	(void) control;
    return 0;
}


/*******************************************************************************
    @fn          LL_EXT_DeclareNvUsage Vendor Specific API

    @brief       This HCI Extension API is used to indicate to the Controller
                whether or not the Host will be using the NV memory during BLE
                operations.

    input parameters

    @param       mode - HCI_EXT_NV_IN_USE, HCI_EXT_NV_NOT_IN_USE

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER,
                LL_STATUS_ERROR_COMMAND_DISALLOWED
*/
llStatus_t LL_EXT_DeclareNvUsage( uint8 mode )
{
	(void) mode;
    return 0;
}


/*******************************************************************************
    @fn          LL_EXT_Decrypt API

    @brief       This API is called by the HCI to request the LL to decrypt the
                data in the command using the key given in the command.

                Note: The parameters are byte ordered MSO to LSO.

    input parameters

    @param       *key           - A 128 bit key to be used to calculate the
                                 session key.
    @param       *encryptedData - A 128 bit block that is encrypted.

    output parameters

    @param       *plaintextData - A 128 bit block that is to be encrypted.

    @param       None.

    @return      LL_STATUS_SUCCESS
*/
llStatus_t LL_EXT_Decrypt( uint8* key,
                           uint8* encryptedData,
                           uint8* plaintextData )
{
	(void) key;
	(void) encryptedData;
	(void) plaintextData;
    return 0;
}


/*******************************************************************************
    @fn          LL_EXT_SetLocalSupportedFeatures API

    @brief       This API is called by the HCI to indicate to the Controller
                which features can or can not be used.

                Note: Not all features indicated by the Host to the Controller
                      are valid. If invalid, they shall be ignored.

    input parameters

    @param       featureSet  - A pointer to the Feature Set where each bit:
                              0: Feature shall not be used.
                              1: Feature can be used.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS
*/
llStatus_t LL_EXT_SetLocalSupportedFeatures( uint8* featureSet )
{
	(void) featureSet;
    return 0;
}



/*******************************************************************************
    @fn          LL_EXT_ModemTestTx

    @brief       This API is used start a continuous transmitter modem test,
                using either a modulated or unmodulated carrier wave tone, at
                the frequency that corresponds to the specified RF channel. Use
                LL_EXT_EndModemTest command to end the test.

                Note: A LL reset will be issued by LL_EXT_EndModemTest!
                Note: The BLE device will transmit at maximum power.
                Note: This API can be used to verify this device meets Japan's
                      TELEC regulations.

    input parameters

    @param       cwMode - LL_EXT_TX_MODULATED_CARRIER,
                         LL_EXT_TX_UNMODULATED_CARRIER
                txFreq - Transmit RF channel k=0..39, where BLE F=2402+(k*2MHz).

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER,
                LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE
*/
llStatus_t LL_EXT_ModemTestTx( uint8 cwMode,
                               uint8 txFreq )
{
	(void) cwMode;
	(void) txFreq;
    return 0;
}


/*******************************************************************************
    @fn          LL_EXT_ModemHopTestTx

    @brief       This API is used to start a continuous transmitter direct test
                mode test using a modulated carrier wave and transmitting a
                37 byte packet of Pseudo-Random 9-bit data. A packet is
                transmitted on a different frequency (linearly stepping through
                all RF channels 0..39) every 625us. Use LL_EXT_EndModemTest
                command to end the test.

                Note: A LL reset will be issued by LL_EXT_EndModemTest!
                Note: The BLE device will transmit at maximum power.
                Note: This API can be used to verify this device meets Japan's
                      TELEC regulations.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE
*/
llStatus_t LL_EXT_ModemHopTestTx( void )
{
    return 0;
}


/*******************************************************************************
    @fn          LL_EXT_ModemTestRx

    @brief       This API is used to start a continuous receiver modem test
                using a modulated carrier wave tone, at the frequency that
                corresponds to the specific RF channel. Any received data is
                discarded. Receiver gain may be adjusted using the
                LL_EXT_SetRxGain command. RSSI may be read during this test by
                using the LL_ReadRssi command. Use LL_EXT_EndModemTest command
                to end the test.

                Note: A LL reset will be issued by LL_EXT_EndModemTest!
                Note: The BLE device will transmit at maximum power.

    input parameters

    @param       rxFreq - Receiver RF channel k=0..39, where BLE F=2402+(k*2MHz).

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER,
                LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE
*/
llStatus_t LL_EXT_ModemTestRx( uint8 rxFreq )
{
	(void) rxFreq;
    return 0;
}


/*******************************************************************************
    @fn          LL_EXT_EndModemTest

    @brief       This API is used to shutdown a modem test. A complete link
                layer reset will take place.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_UNEXPECTED_STATE_ROLE
*/
llStatus_t LL_EXT_EndModemTest( void )
{
    return 0;
}


/*******************************************************************************
    @fn          LL_EXT_SetFreqTune

    @brief       This API is used to set the Frequncy Tuning up or down. If the
                current setting is already at the max/min value, then no
                update is performed.

                Note: This is a Production Test Mode only command!

    input parameters

    @param       step - LL_EXT_SET_FREQ_TUNE_UP or LL_EXT_SET_FREQ_TUNE_DOWN

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER
*/
llStatus_t LL_EXT_SetFreqTune( uint8 step )
{
	(void) step;
    return 0;
}


/*******************************************************************************
    @fn          LL_EXT_SaveFreqTune

    @brief       This API is used to save the current Frequency Tuning value to
                flash memory. It is restored on reboot or wake from sleep.

                Note: This is a Production Test Mode only command!

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_COMMAND_DISALLOWED
*/
llStatus_t LL_EXT_SaveFreqTune( void )
{
    return 0;
}


/*******************************************************************************
    @fn          LL_EXT_SetMaxDtmTxPower Vendor Specific API

    @brief       This function is used to set the max RF TX power to be used
                when using Direct Test Mode.

    input parameters

    @param       txPower - LL_EXT_TX_POWER_MINUS_23_DBM,
                          LL_EXT_TX_POWER_MINUS_6_DBM,
                          LL_EXT_TX_POWER_0_DBM,
                          LL_EXT_TX_POWER_4_DBM

    output parameters

    @param       cmdComplete - Boolean to indicate the command is still pending.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER
*/
llStatus_t LL_EXT_SetMaxDtmTxPower( uint8 txPower )
{
	(void) txPower;
    return 0;
}


/*******************************************************************************
    @fn          LL_EXT_MapPmIoPort Vendor Specific API

    @brief       This function is used to configure and map a CC254x I/O Port as
                a General Purpose I/O (GPIO) output signal that reflects the
                Power Management (PM) state of the CC254x device. The GPIO
                output will be High on Wake, and Low upon entering Sleep. This
                feature can be disabled by specifying LL_EXT_PM_IO_PORT_NONE for
                the ioPort (ioPin is then ignored). The system default value
                upon hardware reset is disabled. This command can be used to
                control an external DC-DC Converter (its actual intent) such has
                the TI TPS62730 (or any similar converter that works the same
                way). This command should be used with extreme care as it will
                override how the Port/Pin was previously configured! This
                includes the mapping of Port 0 pins to 32kHz clock output,
                Analog I/O, UART, Timers; Port 1 pins to Observables, Digital
                Regulator status, UART, Timers; Port 2 pins to an external 32kHz
                XOSC. The selected Port/Pin will be configured as an output GPIO
                with interrupts masked. Careless use can result in a
                reconfiguration that could disrupt the system. It is therefore
                the user's responsibility to ensure the selected Port/Pin does
                not cause any conflicts in the system.

                Note: Only Pins 0, 3 and 4 are valid for Port 2 since Pins 1
                      and 2 are mapped to debugger signals DD and DC.

                Note: Port/Pin signal change will only occur when Power Savings
                      is enabled.

    input parameters

    @param       ioPort - LL_EXT_PM_IO_PORT_P0,
                         LL_EXT_PM_IO_PORT_P1,
                         LL_EXT_PM_IO_PORT_P2,
                         LL_EXT_PM_IO_PORT_NONE

    @param       ioPin  - LL_EXT_PM_IO_PORT_PIN0,
                         LL_EXT_PM_IO_PORT_PIN1,
                         LL_EXT_PM_IO_PORT_PIN2,
                         LL_EXT_PM_IO_PORT_PIN3,
                         LL_EXT_PM_IO_PORT_PIN4,
                         LL_EXT_PM_IO_PORT_PIN5,
                         LL_EXT_PM_IO_PORT_PIN6,
                         LL_EXT_PM_IO_PORT_PIN7

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER,
                LL_STATUS_ERROR_COMMAND_DISALLOWED
*/
llStatus_t LL_EXT_MapPmIoPort( uint8 ioPort, uint8 ioPin )
{
	(void) ioPort;
	(void) ioPin;
    return 0;
}



/*******************************************************************************
    @fn          LL_EXT_ExtendRfRange Vendor Specific API

    @brief       This function is used to Extend Rf Range using the TI CC2590
                2.4 GHz RF Front End device.

    input parameters

    @param       cmdComplete - Pointer to get indicatin if command is done.

    output parameters

    @param       cmdComplete - Boolean to indicate the command is still pending.

    @return      LL_STATUS_SUCCESS
*/
//llStatus_t LL_EXT_ExtendRfRange( uint8 *cmdComplete )
//{
//  return 0;
//}


/*******************************************************************************
    @fn          LL_EXT_HaltDuringRf Vendor Specfic API

    @brief       This function is used to enable or disable halting the
                CPU during RF. The system defaults to enabled.

    input parameters

    @param       mode - LL_EXT_HALT_DURING_RF_ENABLE,
                       LL_EXT_HALT_DURING_RF_DISABLE

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_COMMAND_DISALLOWED,
                LL_STATUS_ERROR_BAD_PARAMETER
*/
llStatus_t LL_EXT_HaltDuringRf( uint8 mode )
{
	(void) mode;
    return 0;
}

/*******************************************************************************
    @fn          LL_EXT_BuildRevision Vendor Specific API

    @brief       This API is used to to set a user revision number or read the
                build revision number.

    input parameters

    @param       mode       - LL_EXT_SET_USER_REVISION |
                             LL_EXT_READ_BUILD_REVISION
    @param       userRevNum - A 16 bit value the user can set as their own
                             revision number

    output parameters

    @param       buildRev   - Pointer to returned build revision, if any.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER
*/
llStatus_t LL_EXT_BuildRevision( uint8 mode, uint16 userRevNum, uint8* buildRev )
{
	(void) mode;
	(void) userRevNum;
	(void) buildRev;
    return 0;
}


/*******************************************************************************
    @fn          LL_EXT_DelaySleep Vendor Specific API

    @brief       This API is used to to set the sleep delay.

    input parameters

    @param       delay - 0 .. 1000, in milliseconds.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER
*/
llStatus_t LL_EXT_DelaySleep( uint16 delay )
{
	(void) delay;
    return 0;
}


/*******************************************************************************
    @fn          LL_EXT_ResetSystem Vendor Specific API

    @brief       This API is used to to issue a soft or hard system reset.

    input parameters

    @param       mode - LL_EXT_RESET_SYSTEM_HARD | LL_EXT_RESET_SYSTEM_SOFT

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER
*/
llStatus_t LL_EXT_ResetSystem( uint8 mode )
{
	(void) mode;
    return 0;
}


/*******************************************************************************
    @fn          LL_EXT_SetFastTxResponseTime API

    @brief       This API is used to enable or disable the fast TX response
                time feature. This can be helpful when a short connection
                interval is used in combination with slave latency. In such
                a scenario, the response time for sending the TX data packet
                can effectively shorten or eliminate slave latency, thereby
                increasing power consumption. By disabling, this feature
                trades fast response time for less power consumption.

    input parameters

    @param       control - LL_EXT_ENABLE_FAST_TX_RESP_TIME,
                          LL_EXT_DISABLE_FAST_TX_RESP_TIME

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_COMMAND_DISALLOWED,
                LL_STATUS_ERROR_BAD_PARAMETER
*/
llStatus_t LL_EXT_SetFastTxResponseTime( uint8 control )
{
	(void) control;
    return 0;
}

/*******************************************************************************
    @fn          LL_EXT_SetSCA

    @brief       This API is used to set this device's Sleep Clock Accuracy.

                Note: For a slave device, this value is directly used, but only
                      if power management is enabled. For a master device, this
                      value is converted into one of eight ordinal values
                      representing a SCA range, as specified in Table 2.2,
                      Vol. 6, Part B, Section 2.3.3.1 of the Core specification.

                Note: This command is only allowed when the device is not in a
                      connection.

                Note: The device's SCA value remains unaffected by a HCI_Reset.

    input parameters

    @param       scaInPPM - This device's SCA in PPM from 0..500.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER,
                LL_STATUS_ERROR_COMMAND_DISALLOWED
*/
llStatus_t LL_EXT_SetSCA( uint16 scaInPPM )
{
	(void) scaInPPM;
    return 0;
}

/*******************************************************************************
    @fn          LL_EXT_SetSlaveLatencyOverride API

    @brief       This API is used to enable or disable the suspention of slave
                latency. This can be helpful when the Slave application knows
                it will soon receive something that needs to be handled without
                delay.

    input parameters

    @param       control - LL_EXT_DISABLE_SL_OVERRIDE,
                          LL_EXT_ENABLE_SL_OVERRIDE

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_COMMAND_DISALLOWED,
                LL_STATUS_ERROR_BAD_PARAMETER
*/
llStatus_t LL_EXT_SetSlaveLatencyOverride( uint8 control )
{
	(void) control;
    return 0;
}

/*******************************************************************************
    @fn          LL_EXT_AdvEventNotice Vendor Specific API

    @brief       This API is called to enable or disable a notification to the
                specified task using the specified task event whenever a Adv
                event ends. A non-zero taskEvent value is taken to be "enable",
                while a zero valued taskEvent is taken to be "disable".

    input parameters

    @param       taskID    - User's task ID.
    @param       taskEvent - User's task event.

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_BAD_PARAMETER
*/
llStatus_t LL_EXT_AdvEventNotice( uint8 taskID, uint16 taskEvent )
{
	(void) taskID;
	(void) taskEvent;
    return 0;
}

/*******************************************************************************
    @fn          LL_EXT_PacketErrorRate Vendor Specific API

    @brief       This function is used to Reset or Read the Packet Error Rate
                counters for a connection. When Reset, the counters are cleared;
                when Read, the total number of packets received, the number of
                packets received with a CRC error, the number of events, and the
                number of missed events are returned via a callback.

                Note: The counters are only 16 bits. At the shortest connection
                      interval, this provides a bit over 8 minutes of data.

    input parameters

    @param       connId  - The LL connection ID on which to send this data.
    @param       command - LL_EXT_PER_RESET, LL_EXT_PER_READ

    output parameters

    @param       None.

    @return      LL_STATUS_SUCCESS, LL_STATUS_ERROR_INACTIVE_CONNECTION
*/
llStatus_t LL_EXT_PacketErrorRate( uint16 connId, uint8 command )
{
	(void) connId;
	(void) command;
    return 0;
}


/*
**  LL Callbacks to HCI
*/

/*******************************************************************************
    @fn          LL_EXT_PacketErrorRateCback Callback

    @brief       This Callback is used by the LL to notify the HCI that the
                Packet Error Rate Read command has been completed.

                Note: The counters are only 16 bits. At the shortest connection
                      interval, this provides a bit over 8 minutes of data.

    input parameters

    @param       numPkts   - Number of Packets received.
    @param       numCrcErr - Number of Packets received with a CRC error.
    @param       numEvents - Number of Connection Events.
    @param       numPkts   - Number of Missed Connection Events.

    output parameters

    @param       None.

    @return      None.
*/
//void LL_EXT_PacketErrorRateCback( uint16 numPkts,
//                                         uint16 numCrcErr,
//                                         uint16 numEvents,
//                                         uint16 numMissedEvts )
//{
//  return;
//}

llStatus_t LL_EXT_SetBDADDR( uint8* bdAddr )
{
	(void) bdAddr;
    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          LL_InitConnectContext

    @brief       This function initialize the LL connection-orient context

    input parameters

    @param       pConnContext   - connection-orient context, the memory is allocated by application
                maxConnNum     - the size of connect-orient context
                maxPktPerEvent - number of packets transmit/receive per connection event

    output parameters

    @param       None.

    @return      None.
*/
llStatus_t LL_InitConnectContext(llConnState_t*    pConnContext,
                                 uint8* pConnBuffer,
                                 uint8 maxConnNum,
                                 uint8 maxPktPerEventTx,
                                 uint8 maxPktPerEventRx,
                                 uint8 blePktVersion)
{
    int i, j;
    int pktLen = BLE_PKT40_LEN;
    uint8* p;
    int total = 0;

    if (maxConnNum > MAX_NUM_LL_CONN)
        return LL_STATUS_ERROR_BAD_PARAMETER;

    if (pConnContext == NULL)
        return LL_STATUS_ERROR_BAD_PARAMETER;

    if (pConnBuffer == NULL)
        return LL_STATUS_ERROR_BAD_PARAMETER;

    if (blePktVersion == BLE_PKT_VERSION_4_0)         // BLE4.0
        pktLen = BLE_PKT40_LEN;
    else if (blePktVersion == BLE_PKT_VERSION_5_1)         // BLE5.1
        pktLen = BLE_PKT51_LEN;

    pktLen += 6;             // header
    g_maxConnNum = maxConnNum;
    conn_param   = pConnContext;
    g_maxPktPerEventTx = maxPktPerEventTx;
    g_maxPktPerEventRx = maxPktPerEventRx;
    g_blePktVersion = blePktVersion;
    p  = pConnBuffer;

    for (i = 0; i < maxConnNum; i++)
    {
        memset(&conn_param[i], 0, sizeof(llConnState_t));

        for (j = 0; j < maxPktPerEventTx; j++)
        {
            conn_param[i].ll_buf.tx_conn_desc[j] = (struct ll_pkt_desc*)p;
            p += pktLen;
            total += pktLen;
        }

        for (j = 0; j < maxPktPerEventRx; j++)
        {
            conn_param[i].ll_buf.rx_conn_desc[j] = (struct ll_pkt_desc*)p;
            p += pktLen;
            total += pktLen;
        }

        conn_param[i].ll_buf.tx_not_ack_pkt = (struct ll_pkt_desc*)p;
        p += pktLen;
        total += pktLen;

        for (j = 0; j < maxPktPerEventTx; j++)
        {
            conn_param[i].ll_buf.tx_ntrm_pkts[j] = (struct ll_pkt_desc*)p;
            p += pktLen;
            total += pktLen;
        }
    }

//  LOG("total = %d\n", total);
    return( LL_STATUS_SUCCESS );
}


// necessary?
/*******************************************************************************
    @fn          LL_InitExtendedScan

    @brief       This function initialize the extended scan context

    input parameters

    @param       scanDataBuffer   -
                scanDataBufferLength     -

    output parameters

    @param       None.

    @return      None.
*/
llStatus_t LL_InitExtendedScan(uint8* scanDataBuffer,
                               uint16 scanDataBufferLength)
{
    extScanInfo.adv_data = scanDataBuffer;
    extScanInfo.adv_data_buf_len = scanDataBufferLength;
    return( LL_STATUS_SUCCESS );
}

// TODO: need better interface for user
/*******************************************************************************
    @fn          LL_InitExtendedAdv

    @brief       The capability of extend advertiser is decided by application.
                Application should allocate the memory for extend adv and init
                LL ext advertiser



    input parameters

    @param       extAdvInfo   - pointer to memory block for extended adv info
    @param       extAdvNumber - size of the memory block for extended adv info
    @param       advSetMaxLen   - maximum adv data set length

    output parameters

    @param       None.

    @return      None.
*/
llStatus_t LL_InitExtendedAdv( extAdvInfo_t* extAdvInfo,
                               uint8         extAdvSetNumber,
                               uint16        advSetMaxLen)
{
    int i;
    g_pExtendedAdvInfo  = extAdvInfo;
    g_extAdvNumber      = extAdvSetNumber;
    g_advSetMaximumLen  = advSetMaxLen;

    for (i = 0; i < g_extAdvNumber; i ++)
    {
        g_pExtendedAdvInfo[i].advHandle                       = LL_INVALID_ADV_SET_HANDLE;
        g_pExtendedAdvInfo[i].parameter.isOwnRandomAddressSet = FALSE;
//      g_pExtendedAdvInfo[i].parameter.advertisingSID        = LL_INVALID_ADV_SET_HANDLE;
        g_pExtendedAdvInfo[i].adv_event_counter = 0;
        g_pExtendedAdvInfo[i].data.fragmentPreference = 0xFF;
        g_pExtendedAdvInfo[i].data.advertisingDataLength = 0;        // no adv data
        g_pExtendedAdvInfo[i].data.dataComplete = FALSE;
        g_pExtendedAdvInfo[i].scanRspMaxLength = 0;                  // no scan rsp data
        g_pExtendedAdvInfo[i].isPeriodic = FALSE;
    }

    // init adv scheduler
    g_pAdvSchInfo = (llAdvScheduleInfo_t*)osal_mem_alloc(sizeof(llAdvScheduleInfo_t) * g_extAdvNumber);

    if (g_pAdvSchInfo == NULL)
        return (LL_STATUS_ERROR_OUT_OF_HEAP);

    for (i = 0; i < g_extAdvNumber; i ++)
    {
        g_pAdvSchInfo[i].adv_handler = LL_INVALID_ADV_SET_HANDLE;
        g_pAdvSchInfo[i].pAdvInfo = NULL;
    }

    g_schExtAdvNum  = 0;
    g_currentExtAdv = 0xFF;     // invalid
    // temp set, change to global_config
    g_advSlotPeriodic = pGlobal_config[LL_EXT_ADV_RSC_PERIOD];          // 1 second
    g_advPerSlotTick  =  pGlobal_config[LL_EXT_ADV_RSC_SLOT_DURATION];           // 10ms
    llTaskState = LL_TASK_INVALID;
    g_interAuxPduDuration = pGlobal_config[LL_EXT_ADV_INTER_SEC_CHN_INT];
    return( LL_STATUS_SUCCESS );
}

/*******************************************************************************
    @fn          LL_InitPediodicAdv

    @brief       The capability of periodic advertiser is decided by application.
                Application should allocate the memory for extend adv and init
                LL ext advertiser



    input parameters

    @param       extAdvInfo   - pointer to memory block for extended adv info
    @param       extAdvNumber - size of the memory block for extended adv info
    @param       advSetMaxLen   - maximum adv data set length

    output parameters

    @param       None.

    @return      None.
*/
llStatus_t LL_InitPeriodicAdv(extAdvInfo_t* extAdvInfo,
                              periodicAdvInfo_t* periodicAdvInfo,
                              uint8            periodicAdvSetNumber,
                              uint16           advSetMaxLen)
{
    int i;
    g_pExtendedAdvInfo  = extAdvInfo;
    g_extAdvNumber      = periodicAdvSetNumber;
    g_pPeriodicAdvInfo  = periodicAdvInfo;
    g_perioAdvNumber    = periodicAdvSetNumber;
    g_advSetMaximumLen  = advSetMaxLen;

    for (i = 0; i < g_extAdvNumber; i ++)
    {
        g_pExtendedAdvInfo[i].advHandle                       = LL_INVALID_ADV_SET_HANDLE;
        g_pExtendedAdvInfo[i].parameter.isOwnRandomAddressSet = FALSE;
        g_pExtendedAdvInfo[i].parameter.advertisingSID        = LL_INVALID_ADV_SET_HANDLE;
        g_pExtendedAdvInfo[i].adv_event_counter = 0;
        g_pExtendedAdvInfo[i].data.fragmentPreference = 0xFF;
        g_pExtendedAdvInfo[i].data.advertisingDataLength = 0;        // no data
        g_pExtendedAdvInfo[i].data.dataComplete = FALSE;
        g_pExtendedAdvInfo[i].data.advertisingData = NULL;           // should not be filled
        g_pExtendedAdvInfo[i].isPeriodic = FALSE;
    }

    for (i = 0; i < g_perioAdvNumber; i ++)
    {
        g_pPeriodicAdvInfo[i].advHandle         = LL_INVALID_ADV_SET_HANDLE;
        g_pPeriodicAdvInfo[i].active            = FALSE;
        g_pPeriodicAdvInfo[i].periodic_adv_event_counter = 0;
        g_pPeriodicAdvInfo[i].currentAdvOffset  = 0;
        g_pPeriodicAdvInfo[i].data.advertisingDataLength = 0;        // no data
        g_pPeriodicAdvInfo[i].data.dataComplete = FALSE;
    }

    // =============== init adv scheduler
    g_pAdvSchInfo_periodic = (llPeriodicAdvScheduleInfo_t*)osal_mem_alloc(sizeof(llPeriodicAdvScheduleInfo_t) * periodicAdvSetNumber);

    if (g_pAdvSchInfo_periodic == NULL)
        return (LL_STATUS_ERROR_OUT_OF_HEAP);

    for (i = 0; i < g_perioAdvNumber; i ++)
    {
        g_pAdvSchInfo_periodic[i].adv_handler = LL_INVALID_ADV_SET_HANDLE;
        g_pAdvSchInfo_periodic[i].pAdvInfo    = NULL;
    }

    g_schExtAdvNum_periodic  = 0;
    g_currentExtAdv_periodic = 0xFF;     // invalid
    // temp set, change to global_config
    g_advSlotPeriodic = pGlobal_config[LL_PRD_ADV_RSC_PERIOD];          // 1 second
    g_advPerSlotTick  =  pGlobal_config[LL_PRD_ADV_RSC_SLOT_DURATION];           // 10ms
    // the IFS time between 2 continuous AUX PDU in the same periodic/extended adv event
    g_interAuxPduDuration = pGlobal_config[LL_EXT_ADV_INTER_SEC_CHN_INT];
    llTaskState = LL_TASK_INVALID;
    return( LL_STATUS_SUCCESS );
}


// ======= extended adv/periodic adv PDU construction functions
//#pragma O0

//#define  LL_PERIOD_ADV_EXT_PART_DURATION       20000
// ADV_EXT_IND PDU construction function
void llSetupAdvExtIndPDU0(extAdvInfo_t*  pAdvInfo, periodicAdvInfo_t* pPrdAdv)
{
    uint8 advMode, extHeaderFlag, length, extHdrLength;
    uint8 offset = 0;

    // set AdvMode
    if (pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_CONN_BITMASK)
        advMode = LL_EXT_ADV_MODE_CONN;
    else if (pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_SCAN_BITMASK)
        advMode = LL_EXT_ADV_MODE_SC;
    else
        advMode = LL_EXT_ADV_MODE_NOCONN_NOSC;

    length = 0;
    // adv PDU header
    g_tx_ext_adv_buf.txheader = 0;
    // PDU type, 4 bits
    SET_BITS(g_tx_ext_adv_buf.txheader, ADV_EXT_TYPE, PDU_TYPE_SHIFT, PDU_TYPE_MASK);
    // RFU, ChSel, TxAdd, RxAdd
    SET_BITS(g_tx_ext_adv_buf.txheader, pAdvInfo->parameter.ownAddrType, TX_ADD_SHIFT, TX_ADD_MASK);
    // === step 1. decide the extended header fields
    extHdrLength = 0;
    // extended header
    extHeaderFlag = 0;
    extHdrLength ++;       // flag

    if (pAdvInfo->isPeriodic == FALSE && (pAdvInfo->data.dataComplete == TRUE && pAdvInfo->data.advertisingDataLength == 0))    // no aux PDU case
    {
        extHeaderFlag |= LE_EXT_HDR_ADVA_PRESENT_BITMASK;
        extHdrLength += 6;

        if (pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_DIRECT_BITMASK)
        {
            extHeaderFlag |= LE_EXT_HDR_TARGETA_PRESENT_BITMASK;
            extHdrLength += 6;
        }
    }
    else         // with auxilary PDU case
    {
        extHeaderFlag |= LE_EXT_HDR_ADI_PRESENT_BITMASK | LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK;
        extHdrLength += 5;
    }

    if (pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_TX_POWER_BITMASK)
    {
        extHeaderFlag |= LE_EXT_HDR_TX_PWR_PRESENT_BITMASK;
        extHdrLength ++;
    }

    length = 1 + extHdrLength;    // 1: extended header len(6bits) + advMode(2bit)
    // Length
    SET_BITS(g_tx_ext_adv_buf.txheader, length, LENGTH_SHIFT, LENGTH_MASK);
    // === step 2. fill extended header
    offset = 0;
    // Extended header length + AdvMode(1 octet)
    g_tx_ext_adv_buf.data[offset] = ((advMode & 0x3) << 6) | (extHdrLength & 0x3F);
    offset ++;
    g_tx_ext_adv_buf.data[offset] = extHeaderFlag;
    offset ++;

    // AdvA (6 octets)
    if (extHeaderFlag & LE_EXT_HDR_ADVA_PRESENT_BITMASK)
    {
        if (pAdvInfo->parameter.ownAddrType == LL_DEV_ADDR_TYPE_RANDOM && pAdvInfo->parameter.isOwnRandomAddressSet == TRUE)
            memcpy(&g_tx_ext_adv_buf.data[offset], pAdvInfo->parameter.ownRandomAddress, LL_DEVICE_ADDR_LEN);
        else    // public address
            memcpy(&g_tx_ext_adv_buf.data[offset], ownPublicAddr, LL_DEVICE_ADDR_LEN);

        offset += LL_DEVICE_ADDR_LEN;
    }

    // TargetA(6 octets)
    if (extHeaderFlag & LE_EXT_HDR_TARGETA_PRESENT_BITMASK)
    {
        // TODO: peer addr type process check
        memcpy(&g_tx_ext_adv_buf.data[offset], pAdvInfo->parameter.peerAddress, LL_DEVICE_ADDR_LEN);
        offset += LL_DEVICE_ADDR_LEN;
    }

    // CTEInfo(1 octets), always not present
//    if (extHeaderFlag & LE_EXT_HDR_CTE_INFO_PRESENT_BITMASK)
//    {
//        offset += 1;
//    }

    // AdvDataInfo(ADI)(2 octets)
    if (extHeaderFlag & LE_EXT_HDR_ADI_PRESENT_BITMASK)
    {
        uint16 adi;
        adi = ((pAdvInfo->parameter.advertisingSID & 0x0F) << 12) | (pAdvInfo->data.DIDInfo & 0x0FFF);
        memcpy(&g_tx_ext_adv_buf.data[offset], (uint8*)&adi, 2);
        offset += 2;
    }

    // AuxPtr(3 octets)
    if (extHeaderFlag & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK)
    {
        uint8   chn_idx, ca, offset_unit, aux_phy;
        uint16  aux_offset;
        uint32  temp = 0;
        chn_idx = 3 + (pAdvInfo->advHandle & 0x0F);        // temp set
        ca      = 0;                                       // 50-500ppm
        aux_phy = pAdvInfo->parameter.secondaryAdvPHY - 1;     // HCI & LL using different enum

        // for extenede adv case, the offset is calculated by auxPduRemainder
        if (pAdvInfo->isPeriodic == FALSE)
        {
            if (g_pAdvSchInfo[g_currentExtAdv].auxPduRemainder >= 245700)
                offset_unit = 1;                                   // 300us, for aux offset >= 245700us
            else
                offset_unit = 0;                                   // 30us, for aux offset < 245700us

            // calculate elapse time since last timer trigger
            //aux_offset = (g_pAdvSchInfo[g_currentExtAdv].auxPduRemainder - elapse_time) / ((offset_unit == 1) ? 300 : 30);
            aux_offset = g_pAdvSchInfo[g_currentExtAdv].auxPduRemainder / ((offset_unit == 1) ? 300 : 30);
        }
        // for periodic adv case, the offset is fixed 1500us + 5000 * adv channel left
        else
        {
            uint8_t temp, number;
            temp = 1 << (pPrdAdv->currentChn - LL_ADV_CHAN_FIRST);     // current bit mask
            temp = ~(temp | (temp - 1));
            temp = pAdvInfo->parameter.priAdvChnMap & temp;            // channel in the chan map to be broadcast
            number = (temp & 0x0001) +
                     ((temp & 0x0002) >> 1) +
                     ((temp & 0x0004) >> 2);
            // the interval between chan 37<->38, 38<->39 is 5000us, primary adv -> aux adv chn is 1500us
            offset_unit = 0;    // 30us, for aux offset < 245700us
            aux_offset = (number * pGlobal_config[LL_EXT_ADV_INTER_PRI_CHN_INT] + pGlobal_config[LL_EXT_ADV_PRI_2_SEC_CHN_INT]) / 30;
        }

//      LOG("#%d#%d# ", g_pAdvSchInfo[g_currentExtAdv].auxPduRemainder - elapse_time, aux_offset);
        temp |= (chn_idx & LL_AUX_PTR_CHN_IDX_MASK) << LL_AUX_PTR_CHN_IDX_SHIFT;
        temp |= (ca & LL_AUX_PTR_CA_MASK) << LL_AUX_PTR_CA_SHIFT;
        temp |= (offset_unit & LL_AUX_PTR_OFFSET_UNIT_MASK) << LL_AUX_PTR_OFFSET_UNIT_SHIFT;
        temp |= (aux_offset & LL_AUX_PTR_AUX_OFFSET_MASK) << LL_AUX_PTR_AUX_OFFSET_SHIFT;
        temp |= (aux_phy & LL_AUX_PTR_AUX_PHY_MASK) << LL_AUX_PTR_AUX_PHY_SHIFT;
        temp &= 0x00FFFFFF;
        memcpy(&g_tx_ext_adv_buf.data[offset], (uint8*)&temp, 3);
        pAdvInfo->auxChn = chn_idx;        // save secondary channel index
        pAdvInfo->currentAdvOffset = 0;    // reset offset in adv data set
        offset += 3;
    }

    // SyncInfo(18 octets)
//    if (extHeaderFlag & LE_EXT_HDR_SYNC_INFO_PRESENT_BITMASK)
//    {
//        // TODO
//        offset += 18;
//    }

    // TxPower(1 octets)
    if (extHeaderFlag & LE_EXT_HDR_TX_PWR_PRESENT_BITMASK)    // Tx power is optional, could we only filled it in AUX_ADV_IND?
    {
        int16  radia_pwr;
        radia_pwr = pAdvInfo->tx_power * 10 + g_rfTxPathCompensation;

        if (radia_pwr > 1270)   radia_pwr = 1270;

        if (radia_pwr < -1270)  radia_pwr = -1270;

        g_tx_ext_adv_buf.data[offset] = (uint8)(radia_pwr / 10);
        offset += 1;
    }

    // ACAD(varies)
    // init adv data offset
    pAdvInfo->currentAdvOffset = 0;
}

void llSetupAuxAdvIndPDU0(extAdvInfo_t*  pAdvInfo, periodicAdvInfo_t* pPrdAdv)
{
    uint8 advMode, extHeaderFlag, length, extHdrLength, advDataLen;
    uint8 offset = 0;
//  uint32 T2, elapse_time;

    // set AdvMode
    if (pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_CONN_BITMASK)
        advMode = LL_EXT_ADV_MODE_CONN;
    else if (pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_SCAN_BITMASK)
        advMode = LL_EXT_ADV_MODE_SC;
    else
        advMode = LL_EXT_ADV_MODE_NOCONN_NOSC;

    length = 0;
    // adv PDU header
    g_tx_ext_adv_buf.txheader = 0;
    // PDU type, 4 bits
    SET_BITS(g_tx_ext_adv_buf.txheader, ADV_EXT_TYPE, PDU_TYPE_SHIFT, PDU_TYPE_MASK);
    // RFU, ChSel, TxAdd, RxAdd
    SET_BITS(g_tx_ext_adv_buf.txheader, pAdvInfo->parameter.ownAddrType, TX_ADD_SHIFT, TX_ADD_MASK);

    if (pGlobal_config[LL_SWITCH] & CONN_CSA2_ALLOW)
        SET_BITS(g_tx_adv_buf.txheader, 1, CHSEL_SHIFT, CHSEL_MASK);

    extHdrLength = 0;
    // == step 1. decide what fields should be present in extended header
    // extended header
    extHeaderFlag = 0;
    extHdrLength ++;
    extHeaderFlag |= LE_EXT_HDR_ADI_PRESENT_BITMASK;
    extHdrLength += 2;

    if (pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_DIRECT_BITMASK)
    {
        extHeaderFlag |= LE_EXT_HDR_TARGETA_PRESENT_BITMASK;
        extHdrLength += 6;
    }

//  if (advMode != LL_EXT_ADV_MODE_NOCONN_NOSC)           // This field is C4 for LL_EXT_ADV_MODE_NOCONN_NOSC, we will not send AdvA in EXT_ADV_IND, so it is mandatory here
//  {
    extHeaderFlag |= LE_EXT_HDR_ADVA_PRESENT_BITMASK;
    extHdrLength += 6;
//  }

    if (pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_TX_POWER_BITMASK)
    {
        extHeaderFlag |= LE_EXT_HDR_TX_PWR_PRESENT_BITMASK;
        extHdrLength ++;
    }

    if (pAdvInfo->isPeriodic == TRUE)
    {
        extHeaderFlag |= LE_EXT_HDR_SYNC_INFO_PRESENT_BITMASK;
        extHdrLength += 18;
    }

    if (pAdvInfo->data.dataComplete == TRUE)
    {
        if (advMode == LL_EXT_ADV_MODE_NOCONN_NOSC
                && (pAdvInfo->isPeriodic == FALSE)
                && (pAdvInfo->data.advertisingDataLength > 255 - 1 - extHdrLength))
        {
            extHeaderFlag |= LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK;
            extHdrLength += 3;
            // maximum payload length = 255: header len (= 1), ext header len(extHdrLength), adv data(advDataLen)
            advDataLen = 255 - 1 - extHdrLength;       // adv data length. TODO: check spec
        }
        else
            advDataLen = pAdvInfo->data.advertisingDataLength;  // put all adv data in field "Adv Data"
    }
    else         // update 04-13, consider update adv data case
        advDataLen = 0;

    length = 1 + extHdrLength + advDataLen;    // 1: extended header len(6bits) + advMode(2bit)
    // Length
    SET_BITS(g_tx_ext_adv_buf.txheader, length, LENGTH_SHIFT, LENGTH_MASK);
    // === step 2.  fill AUX_ADV_IND PDU
    // Extended header length + AdvMode(1 octet)
    g_tx_ext_adv_buf.data[offset] = ((advMode & 0x3) << 6) | (extHdrLength & 0x3F);
    offset ++;
    g_tx_ext_adv_buf.data[offset] = extHeaderFlag;
    offset ++;

    // AdvA (6 octets)
    if (extHeaderFlag & LE_EXT_HDR_ADVA_PRESENT_BITMASK)
    {
        if (pAdvInfo->parameter.ownAddrType == LL_DEV_ADDR_TYPE_RANDOM && pAdvInfo->parameter.isOwnRandomAddressSet == TRUE)
            memcpy(&g_tx_ext_adv_buf.data[offset], pAdvInfo->parameter.ownRandomAddress, LL_DEVICE_ADDR_LEN);
        else    // public address
            memcpy(&g_tx_ext_adv_buf.data[offset], ownPublicAddr, LL_DEVICE_ADDR_LEN);

        offset += LL_DEVICE_ADDR_LEN;
    }

    // TargetA(6 octets)
    if (extHeaderFlag & LE_EXT_HDR_TARGETA_PRESENT_BITMASK)
    {
        memcpy(&g_tx_ext_adv_buf.data[offset], pAdvInfo->parameter.peerAddress, LL_DEVICE_ADDR_LEN);
        offset += LL_DEVICE_ADDR_LEN;
    }

    // CTEInfo(1 octets), not present for AUX_ADV_IND
    if (extHeaderFlag & LE_EXT_HDR_CTE_INFO_PRESENT_BITMASK)
    {
        // offset += 1;
    }

    // AdvDataInfo(ADI)(2 octets)
    if (extHeaderFlag & LE_EXT_HDR_ADI_PRESENT_BITMASK)
    {
        uint16 adi;
        adi = ((pAdvInfo->parameter.advertisingSID & 0x0F) << 12) | (pAdvInfo->data.DIDInfo & 0x0FFF);
        memcpy(&g_tx_ext_adv_buf.data[offset], (uint8*)&adi, 2);
        offset += 2;
    }

    // AuxPtr(3 octets)
    if (extHeaderFlag & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK)
    {
        uint8   chn_idx, ca, offset_unit, aux_phy;
        uint16  aux_offset;
        uint32  temp = 0;
//        if (pAdvInfo->isPeriodic == FALSE)   // no periodic adv case
//        {
        chn_idx = llGetNextAuxAdvChn(pAdvInfo->currentChn);
        ca      = 0;        // 50-500ppm
        offset_unit = 0;    // 30us, for aux offset < 245700us
        aux_phy = pAdvInfo->parameter.secondaryAdvPHY - 1;             // HCI & LL using different enum
        aux_offset = g_interAuxPduDuration / 30;
        pAdvInfo->currentChn = chn_idx;
//        }
//      else     // AUX_PTR field is not required for periodic adv AUX_ADV_IND
//      {
//      }
        temp |= (chn_idx & LL_AUX_PTR_CHN_IDX_MASK) << LL_AUX_PTR_CHN_IDX_SHIFT;
        temp |= (ca & LL_AUX_PTR_CA_MASK) << LL_AUX_PTR_CA_SHIFT;
        temp |= (offset_unit & LL_AUX_PTR_OFFSET_UNIT_MASK) << LL_AUX_PTR_OFFSET_UNIT_SHIFT;
        temp |= (aux_offset & LL_AUX_PTR_AUX_OFFSET_MASK) << LL_AUX_PTR_AUX_OFFSET_SHIFT;
        temp |= (aux_phy & LL_AUX_PTR_AUX_PHY_MASK) << LL_AUX_PTR_AUX_PHY_SHIFT;
        temp &= 0x00FFFFFF;
        memcpy(&g_tx_ext_adv_buf.data[offset], (uint8*)&temp, 3);
        offset += 3;
    }
    else if (pAdvInfo->isPeriodic == FALSE)   // only applicable to extended adv case
    {
        // no more aux PDU, update next channel number
        int i = 0;

        while ((i < 3) && !(pAdvInfo->parameter.priAdvChnMap & (1 << i))) i ++;

        pAdvInfo->currentChn = LL_ADV_CHAN_FIRST + i;
    }
    else    // periodic adv case
    {
        llPrdAdvDecideNextChn(pAdvInfo, pPrdAdv);
    }

    // SyncInfo(18 octets)
    if (extHeaderFlag & LE_EXT_HDR_SYNC_INFO_PRESENT_BITMASK)
    {
        // TODO
        llSetupSyncInfo(pAdvInfo, pPrdAdv);
        memcpy(&g_tx_ext_adv_buf.data[offset], (uint8*)&syncInfo, 18);
        offset += 18;
    }

    // TxPower(1 octets)
    if (extHeaderFlag & LE_EXT_HDR_TX_PWR_PRESENT_BITMASK)    // Tx power is optional, could we only filled it in AUX_ADV_IND?
    {
        int16  radia_pwr;
        radia_pwr = pAdvInfo->tx_power * 10 + g_rfTxPathCompensation;

        if (radia_pwr > 1270)   radia_pwr = 1270;

        if (radia_pwr < -1270)  radia_pwr = -1270;

        g_tx_ext_adv_buf.data[offset] = (uint8)(radia_pwr / 10);
        offset += 1;
    }

    // ACAD(varies), not present

    // copy adv data
    if (pAdvInfo->isPeriodic == FALSE)
    {
        memcpy(&g_tx_ext_adv_buf.data[offset], (uint8*)&pAdvInfo->data.advertisingData[pAdvInfo->currentAdvOffset], advDataLen);
        pAdvInfo->currentAdvOffset += advDataLen;
    }
}
//#pragma O0
void llSetupAuxChainIndPDU0(extAdvInfo_t*  pAdvInfo, periodicAdvInfo_t* pPrdAdv)
{
    uint8 advMode, extHeaderFlag, length, extHdrLength, advDataLen;
    uint8 offset = 0;
    // set AdvMode
    advMode = 0;
    length = 0;
    // adv PDU header
    g_tx_ext_adv_buf.txheader = 0;
    // PDU type, 4 bits
    SET_BITS(g_tx_ext_adv_buf.txheader, ADV_EXT_TYPE, PDU_TYPE_SHIFT, PDU_TYPE_MASK);
    // RFU, ChSel, TxAdd, RxAdd
    SET_BITS(g_tx_ext_adv_buf.txheader, pAdvInfo->parameter.ownAddrType, TX_ADD_SHIFT, TX_ADD_MASK);
    extHdrLength = 0;
    // extended header
    extHeaderFlag = 0;
    extHdrLength ++;

    // 2020-02-10 add for Connectionless CTE Info
    // CTEInfo field is C5: Optional
    if( pPrdAdv->PrdCTEInfo.enable == LL_CTE_ENABLE )
    {
        if( pPrdAdv->PrdCTEInfo.CTE_Count > pPrdAdv->PrdCTEInfo.CTE_Count_Idx )
        {
            extHeaderFlag |= LE_EXT_HDR_CTE_INFO_PRESENT_BITMASK;
            extHdrLength ++;
        }
    }

    // ADI field is C3, it is present in our implementation
    if (pAdvInfo->isPeriodic == FALSE)
    {
        extHeaderFlag |= LE_EXT_HDR_ADI_PRESENT_BITMASK;
        extHdrLength += 2;
    }

    // comment out because for periodic adv, tx pwr field is in AUX_SYNC_IND. for extended adv, txPwr field in AUX_ADV_IND
//  if (((pAdvInfo->isPeriodic == FALSE) && (pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_TX_POWER_BITMASK))
//    || ((pAdvInfo->isPeriodic == TRUE) && (pPrdAdv->adv_event_properties & LE_ADV_PROP_TX_POWER_BITMASK)))
//  {
//      extHeaderFlag |= LE_EXT_HDR_TX_PWR_PRESENT_BITMASK;
//      extHdrLength ++;
//  }

    // if Adv Data could not be sent completely in this PDU, need AuxPtr
    if (pAdvInfo->isPeriodic == FALSE)
    {
        if (pAdvInfo->data.dataComplete == TRUE)
        {
            if (pAdvInfo->data.advertisingDataLength - pAdvInfo->currentAdvOffset > 255 - 1 - extHdrLength)
            {
                extHeaderFlag |= LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK;
                extHdrLength += 3;
                // maximum payload length = 255, header len = 1, ADI len = 2, AUX_PTR len = 2
                advDataLen = 255 - 1 - extHdrLength;;
            }
            else
                advDataLen = pAdvInfo->data.advertisingDataLength - pAdvInfo->currentAdvOffset;  // put all remain adv data in field "Adv Data"
        }
        else      // update 04-13, adv data may be reconfigured during advertising, include no data in such case
            advDataLen = 0;
    }
    else
    {
        if (pPrdAdv->data.dataComplete == TRUE)
        {
            if (pPrdAdv->data.advertisingDataLength - pPrdAdv->currentAdvOffset > 255 - 1 - extHdrLength)
            {
                extHeaderFlag |= LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK;
                extHdrLength += 3;
                // maximum payload length = 255, header len = 1, ADI len = 2, AUX_PTR len = 2
                advDataLen = 255 - 1 - extHdrLength;;
            }
            else
                advDataLen = pPrdAdv->data.advertisingDataLength - pPrdAdv->currentAdvOffset;  // put all remain adv data in field "Adv Data"
        }
        else      // update 04-13, adv data may be reconfigured during advertising, include no data in such case
            advDataLen = 0;
    }

    length = 1 + extHdrLength + advDataLen;   // 1: extended header len(6bits) + advMode(2bit)
    // Length
    SET_BITS(g_tx_ext_adv_buf.txheader, length, LENGTH_SHIFT, LENGTH_MASK);
    // fill extended header
    offset = 0;
    // Extended header length + AdvMode(1 octet)
    g_tx_ext_adv_buf.data[offset] = ((advMode & 0x3) << 6) | (extHdrLength & 0x3F);
    offset ++;
    g_tx_ext_adv_buf.data[offset] = extHeaderFlag;
    offset ++;

    // CTEInfo(1 octets), not present for AUX_ADV_IND
    if (extHeaderFlag & LE_EXT_HDR_CTE_INFO_PRESENT_BITMASK)
    {
        //LOG("\n SyC \n");
        g_tx_ext_adv_buf.data[offset] = (   ( pPrdAdv->PrdCTEInfo.CTE_Type << 6 ) | \
                                            ( pPrdAdv->PrdCTEInfo.CTE_Length));
        pPrdAdv->PrdCTEInfo.CTE_Count_Idx ++;
        offset += 1;
    }

    // AdvDataInfo(ADI)(2 octets)
    if (extHeaderFlag & LE_EXT_HDR_ADI_PRESENT_BITMASK)
    {
        uint16 adi;
        adi = ((pAdvInfo->parameter.advertisingSID & 0x0F) << 12) | (pAdvInfo->data.DIDInfo & 0x0FFF);
        memcpy(&g_tx_ext_adv_buf.data[offset], (uint8*)&adi, 2);
        offset += 2;
    }

    // AuxPtr(3 octets)
    if (extHeaderFlag & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK)
    {
        uint8   chn_idx, ca, offset_unit, aux_phy;
        uint16  aux_offset;
        uint32  temp = 0;
        chn_idx = llGetNextAuxAdvChn(pAdvInfo->currentChn);
        ca      = 0;        // 50-500ppm
        offset_unit = 0;    // 30us, for aux offset < 245700us
        aux_phy = pAdvInfo->parameter.secondaryAdvPHY - 1;     // HCI & LL using different enum
        aux_offset = g_interAuxPduDuration / 30;
        temp |= (chn_idx & LL_AUX_PTR_CHN_IDX_MASK) << LL_AUX_PTR_CHN_IDX_SHIFT;
        temp |= (ca & LL_AUX_PTR_CA_MASK) << LL_AUX_PTR_CA_SHIFT;
        temp |= (offset_unit & LL_AUX_PTR_OFFSET_UNIT_MASK) << LL_AUX_PTR_OFFSET_UNIT_SHIFT;
        temp |= (aux_offset & LL_AUX_PTR_AUX_OFFSET_MASK) << LL_AUX_PTR_AUX_OFFSET_SHIFT;
        temp |= (aux_phy & LL_AUX_PTR_AUX_PHY_MASK) << LL_AUX_PTR_AUX_PHY_SHIFT;
        temp &= 0x00FFFFFF;
        memcpy(&g_tx_ext_adv_buf.data[offset], (uint8*)&temp, 3);
        pAdvInfo->currentChn = chn_idx;        // save secondary channel index
        pPrdAdv->currentChn = chn_idx;
        offset += 3;
    }
    else
    {
        // no more aux PDU, update next channel number
        if (pAdvInfo->isPeriodic == FALSE)
        {
            int i = 0;

            while ((i < 3) && !(pAdvInfo->parameter.priAdvChnMap & (1 << i))) i ++;

            pAdvInfo->currentChn = LL_ADV_CHAN_FIRST + i;
        }
        else    // periodic adv case
        {
            llPrdAdvDecideNextChn(pAdvInfo, pPrdAdv);
        }
    }

//    // TxPower(1 octets)
//    if (extHeaderFlag & LE_EXT_HDR_TX_PWR_PRESENT_BITMASK)
//    {
//        // TODO
//        offset += 1;
//    }

    // ACAD(varies), not present

    // copy adv data
    if (pAdvInfo == FALSE)
    {
        memcpy(&g_tx_ext_adv_buf.data[offset], (uint8*)&pAdvInfo->data.advertisingData[pAdvInfo->currentAdvOffset], advDataLen);
        pAdvInfo->currentAdvOffset += advDataLen;
    }
    else    // periodic adv case
    {
        memcpy(&g_tx_ext_adv_buf.data[offset], (uint8*)&pPrdAdv->data.advertisingData[pPrdAdv->currentAdvOffset], advDataLen);
        pPrdAdv->currentAdvOffset += advDataLen;

        // if finish broad current periodic adv event, revert the read pointer of adv data
        if (pPrdAdv->currentAdvOffset == pPrdAdv->data.advertisingDataLength)
        {
            // 2020-02-10 add logic for CTE info
            if( pPrdAdv->PrdCTEInfo.enable == LL_CTE_ENABLE )
            {
                if( pPrdAdv->PrdCTEInfo.CTE_Count_Idx >= pPrdAdv->PrdCTEInfo.CTE_Count )
                {
                    // already send all CTE info, then reset pPrdAdv->currentAdvOffset
                    pPrdAdv->currentAdvOffset = 0;
                    pPrdAdv->PrdCTEInfo.CTE_Count_Idx = 0;
                    // length set to zero means stop CTE
                    ll_hw_set_cte_txSupp( CTE_SUPP_LEN_SET | 0x0 );
                }
            }
            else
            {
                // 2020-02-21 bug fix: If CTE is not enabled, then
                // subsequent packets will not be sent
                pPrdAdv->currentAdvOffset = 0;
            }
        }
    }
}


void llSetupAuxSyncIndPDU0(extAdvInfo_t*  pAdvInfo, periodicAdvInfo_t* pPrdAdv)
{
    uint8 advMode, extHeaderFlag, length, extHdrLength, advDataLen;
    uint8 offset = 0;
    // set AdvMode
    advMode = 0;
    length = 0;
    // adv PDU header
    g_tx_ext_adv_buf.txheader = 0;
    // PDU type, 4 bits
    SET_BITS(g_tx_ext_adv_buf.txheader, ADV_EXT_TYPE, PDU_TYPE_SHIFT, PDU_TYPE_MASK);
    // RFU, ChSel, TxAdd, RxAdd
    SET_BITS(g_tx_ext_adv_buf.txheader, 0, TX_ADD_SHIFT, TX_ADD_MASK);    // it seems TxAdd is ignored in spec
    extHdrLength = 0;
    // extended header
    extHeaderFlag = 0;          // for AUX_SYNC_IND PDU: CTE info, AuxPtr, Tx power, ACAD, Adv Data are optional, other fields are absent
    extHdrLength ++;

    //extHeaderFlag |= LE_EXT_HDR_CTE_INFO_PRESENT_BITMASK;
    // 2020-02-10 add for CTE transmit
    // if periodic advertising CTE Enable
    if( pPrdAdv->PrdCTEInfo.enable == LL_CTE_ENABLE )
    {
        if( pPrdAdv->PrdCTEInfo.CTE_Count > 0 )
        {
            extHeaderFlag |= LE_EXT_HDR_CTE_INFO_PRESENT_BITMASK;
            extHdrLength ++;
        }
    }

    if (pPrdAdv->adv_event_properties & LE_ADV_PROP_TX_POWER_BITMASK)
    {
        extHeaderFlag |= LE_EXT_HDR_TX_PWR_PRESENT_BITMASK;
        extHdrLength ++;
    }

    // if Adv Data could not be sent completely in this PDU, need AuxPtr
    if (pPrdAdv->data.dataComplete == TRUE)
    {
        if (pPrdAdv->data.advertisingDataLength - pPrdAdv->currentAdvOffset > 255 - 1 - extHdrLength)
        {
            extHeaderFlag |= LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK;
            extHdrLength += 3;
            // maximum payload length = 255, header len = 1, ADI len = 2, AUX_PTR len = 2
            advDataLen = 255 - 1 - extHdrLength;
        }
        else
            advDataLen = pPrdAdv->data.advertisingDataLength - pPrdAdv->currentAdvOffset;  // put all remain adv data in field "Adv Data"
    }
    else              // update 04-13, adv data may be reconfigured during advertising, include no data in such case
        advDataLen = 0;

    length = 1 + extHdrLength + advDataLen;   // 1: extended header len(6bits) + advMode(2bit)
    // Length
    SET_BITS(g_tx_ext_adv_buf.txheader, length, LENGTH_SHIFT, LENGTH_MASK);
    // fill extended header
    offset = 0;
    // Extended header length + AdvMode(1 octet)
    g_tx_ext_adv_buf.data[offset] = ((advMode & 0x3) << 6) | (extHdrLength & 0x3F);
    offset ++;
    g_tx_ext_adv_buf.data[offset] = extHeaderFlag;
    offset ++;

    // CTEInfo(1 octets), not present for AUX_ADV_IND
    if (extHeaderFlag & LE_EXT_HDR_CTE_INFO_PRESENT_BITMASK)
    {
        g_tx_ext_adv_buf.data[offset] = (   ( pPrdAdv->PrdCTEInfo.CTE_Type << 6 ) | \
                                            ( pPrdAdv->PrdCTEInfo.CTE_Length));
        pPrdAdv->PrdCTEInfo.CTE_Count_Idx ++;
        ll_hw_set_cte_txSupp( CTE_SUPP_LEN_SET | pPrdAdv->PrdCTEInfo.CTE_Length );
        offset += 1;
//      LOG("\n Sy \n");
    }

    // AuxPtr(3 octets)
    if (extHeaderFlag & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK)
    {
        uint8   chn_idx, ca, offset_unit, aux_phy;
        uint16  aux_offset;
        uint32  temp = 0;
        chn_idx = llGetNextAuxAdvChn(pPrdAdv->currentChn);
        // fixed interval between periodic PDUs now
        ca          = 0;    // 50-500ppm
        offset_unit = 0;    // 30us, for aux offset < 245700us
        aux_offset  = g_interAuxPduDuration / 30;
        aux_phy = pAdvInfo->parameter.secondaryAdvPHY - 1;     // HCI & LL using different enum
        temp |= (chn_idx & LL_AUX_PTR_CHN_IDX_MASK) << LL_AUX_PTR_CHN_IDX_SHIFT;
        temp |= (ca & LL_AUX_PTR_CA_MASK) << LL_AUX_PTR_CA_SHIFT;
        temp |= (offset_unit & LL_AUX_PTR_OFFSET_UNIT_MASK) << LL_AUX_PTR_OFFSET_UNIT_SHIFT;
        temp |= (aux_offset & LL_AUX_PTR_AUX_OFFSET_MASK) << LL_AUX_PTR_AUX_OFFSET_SHIFT;
        temp |= (aux_phy & LL_AUX_PTR_AUX_PHY_MASK) << LL_AUX_PTR_AUX_PHY_SHIFT;
        temp &= 0x00FFFFFF;
        memcpy(&g_tx_ext_adv_buf.data[offset], (uint8*)&temp, 3);
        pPrdAdv->currentChn = chn_idx;        // save secondary channel index
        offset += 3;
    }
    else
    {
        // no more aux PDU, update next channel number
        if (pAdvInfo->isPeriodic == FALSE)
        {
            int i = 0;

            while ((i < 3) && !(pAdvInfo->parameter.priAdvChnMap & (1 << i))) i ++;

            pAdvInfo->currentChn = LL_ADV_CHAN_FIRST + i;
        }
        else    // periodic adv case
        {
            llPrdAdvDecideNextChn(pAdvInfo, pPrdAdv);
        }
    }

    // TxPower(1 octets)
    if (extHeaderFlag & LE_EXT_HDR_TX_PWR_PRESENT_BITMASK)    // Tx power is optional, could we only filled it in AUX_ADV_IND?
    {
        int16  radia_pwr;
        radia_pwr = pAdvInfo->tx_power * 10 + g_rfTxPathCompensation;

        if (radia_pwr > 1270)   radia_pwr = 1270;

        if (radia_pwr < -1270)  radia_pwr = -1270;

        g_tx_ext_adv_buf.data[offset] = (uint8)(radia_pwr / 10);
        offset += 1;
    }

    // ACAD(varies), not present
    // copy adv data
    memcpy(&g_tx_ext_adv_buf.data[offset], (uint8*)&pPrdAdv->data.advertisingData[pPrdAdv->currentAdvOffset], advDataLen);
    pPrdAdv->currentAdvOffset += advDataLen;

    // if finish broad current periodic adv event, revert the read pointer of adv data
    if (pPrdAdv->currentAdvOffset == pPrdAdv->data.advertisingDataLength)
    {
        // 2020-02-10 add logic for CTE info
        if( pPrdAdv->PrdCTEInfo.enable == LL_CTE_ENABLE )
        {
            if( pPrdAdv->PrdCTEInfo.CTE_Count_Idx >= pPrdAdv->PrdCTEInfo.CTE_Count )
            {
                // already send all CTE info, then reset pPrdAdv->currentAdvOffset
                pPrdAdv->currentAdvOffset = 0;
                // length set to zero means stop CTE
                ll_hw_set_cte_txSupp( CTE_SUPP_LEN_SET | 0x0 );
            }
        }
        else
        {
            // 2020-02-21 bug fix: If CTE is not enabled, then
            // subsequent packets will not be sent
            pPrdAdv->currentAdvOffset = 0;
        }
    }
}

void llSetupAuxScanRspPDU0(extAdvInfo_t*  pAdvInfo)
{
    uint8 advMode, extHeaderFlag, length, extHdrLength, advDataLen;
    uint8 offset = 0;
    // set AdvMode
    advMode = 0;
    length = 0;
    // adv PDU header
    g_tx_ext_adv_buf.txheader = 0;
    // PDU type, 4 bits
    SET_BITS(g_tx_ext_adv_buf.txheader, ADV_EXT_TYPE, PDU_TYPE_SHIFT, PDU_TYPE_MASK);
    // RFU, ChSel, TxAdd, RxAdd
    SET_BITS(g_tx_ext_adv_buf.txheader, 0, TX_ADD_SHIFT, TX_ADD_MASK);    // it seems TxAdd is ignored in spec
    extHdrLength = 0;
    // extended header
    extHeaderFlag = 0;          // for AUX_SYNC_IND PDU: CTE info, AuxPtr, Tx power, ACAD, Adv Data are optional, other fields are absent
    extHdrLength ++;
    extHeaderFlag |= LE_EXT_HDR_ADVA_PRESENT_BITMASK;
    extHdrLength += 6;
    advDataLen = pAdvInfo->scanRspMaxLength;
    length = 1 + extHdrLength + advDataLen;   // 1: extended header len(6bits) + advMode(2bit)
    // Length
    SET_BITS(g_tx_ext_adv_buf.txheader, length, LENGTH_SHIFT, LENGTH_MASK);
    // fill extended header
    offset = 0;
    // Extended header length + AdvMode(1 octet)
    g_tx_ext_adv_buf.data[offset] = ((advMode & 0x3) << 6) | (extHdrLength & 0x3F);
    offset ++;
    g_tx_ext_adv_buf.data[offset] = extHeaderFlag;
    offset ++;

    // AdvA (6 octets)
    if (extHeaderFlag & LE_EXT_HDR_ADVA_PRESENT_BITMASK)
    {
        if (pAdvInfo->parameter.ownAddrType == LL_DEV_ADDR_TYPE_RANDOM && pAdvInfo->parameter.isOwnRandomAddressSet == TRUE)
            memcpy(&g_tx_ext_adv_buf.data[offset], pAdvInfo->parameter.ownRandomAddress, LL_DEVICE_ADDR_LEN);
        else    // public address
            memcpy(&g_tx_ext_adv_buf.data[offset], ownPublicAddr, LL_DEVICE_ADDR_LEN);

        offset += LL_DEVICE_ADDR_LEN;
    }

    // copy adv data
    memcpy(&g_tx_ext_adv_buf.data[offset], (uint8*)&pAdvInfo->scanRspData[0], pAdvInfo->scanRspMaxLength);
}

//#pragma O2
void llSetupAuxConnectReqPDU0(void)
{
    uint8 offset;
    llConnState_t* connPtr;
    connPtr = &conn_param[extInitInfo.connId];

    if ((extInitInfo.current_scan_PHY == PKT_FMT_BLE1M)
            || (extInitInfo.current_scan_PHY == PKT_FMT_BLR125K))
    {
        connPtr->curParam.connInterval = extInitInfo.conn_interval_max[extInitInfo.current_index];
        connPtr->curParam.slaveLatency = extInitInfo.conn_latency[extInitInfo.current_index];
        connPtr->curParam.connTimeout = extInitInfo.supervision_timeout[extInitInfo.current_index];
    }
    else                 // 2Mbps case
    {
        connPtr->curParam.connInterval = extInitInfo.conn_interval_max_2Mbps;
        connPtr->curParam.slaveLatency = extInitInfo.conn_latency_2Mbps;
        connPtr->curParam.connTimeout = extInitInfo.supervision_timeout_2Mbps;
    }

    offset = 22;
    // Interval, Byte 22 ~ 23
    memcpy((uint8*)&g_tx_adv_buf.data[offset], (uint8*)&connPtr->curParam.connInterval, 2);
    offset += 2;
    // Latency, Byte 24 ~ 25
    memcpy((uint8*)&g_tx_adv_buf.data[offset], (uint8*)&connPtr->curParam.slaveLatency, 2);
    offset += 2;
    // Timeout, Byte 26 ~ 27
    memcpy((uint8*)&g_tx_adv_buf.data[offset], (uint8*)&connPtr->curParam.connTimeout, 2);
}

void llSetupAuxConnectRspPDU0(extAdvInfo_t*  pAdvInfo)
{
    uint8 advMode, extHeaderFlag, length, extHdrLength;
    uint8 offset = 0;
    length = 14;
    // adv PDU header
    g_tx_adv_buf.txheader = 0;
    // PDU type, 4 bits
    SET_BITS(g_tx_adv_buf.txheader, ADV_AUX_CONN_RSP, PDU_TYPE_SHIFT, PDU_TYPE_MASK);
    // RFU, ChSel, TxAdd, RxAdd
    SET_BITS(g_tx_adv_buf.txheader, pAdvInfo->parameter.ownAddrType, TX_ADD_SHIFT, TX_ADD_MASK);
    // Length
    SET_BITS(g_tx_adv_buf.txheader, length, LENGTH_SHIFT, LENGTH_MASK);
    offset = 0;
    extHdrLength = 13;    // ext header flag(1byte) + advA(6 octets) + targetA(6octets)
    // set AdvMode
    advMode = LL_EXT_ADV_MODE_AUX_CONN_RSP;
    g_tx_adv_buf.data[offset] = ((advMode & 0x3) << 6) | (extHdrLength & 0x3F);
    offset ++;
    // extended header
    extHeaderFlag = LE_EXT_HDR_ADVA_PRESENT_BITMASK | LE_EXT_HDR_TARGETA_PRESENT_BITMASK;
    g_tx_adv_buf.data[offset] = extHeaderFlag;
    offset ++;

    if (pAdvInfo->parameter.ownAddrType == LL_DEV_ADDR_TYPE_RANDOM && pAdvInfo->parameter.isOwnRandomAddressSet == TRUE)
        memcpy(&g_tx_adv_buf.data[offset], pAdvInfo->parameter.ownRandomAddress, LL_DEVICE_ADDR_LEN);
    else    // public address
        memcpy(&g_tx_adv_buf.data[offset], ownPublicAddr, LL_DEVICE_ADDR_LEN);

    offset += LL_DEVICE_ADDR_LEN;
    // TargetA(6 octets)
    osal_memcpy(&g_tx_adv_buf.data[offset], g_rx_adv_buf.data, 6);
    offset += LL_DEVICE_ADDR_LEN;
}


// for periodic adv, when finish extended adv part broadcast, or finish periodic adv part broadcase
// controller should decide next broadcast channel and PDU.
// extended adv part: ADV_EXT_IND + AUX_ADV_IND
// periodic adv part: AUX_SYNC_IND + AUX_CHAIN_IND(optional, for long adv data)
void llPrdAdvDecideNextChn(extAdvInfo_t*  pAdvInfo, periodicAdvInfo_t* pPrdAdv)
{
    llPeriodicAdvScheduleInfo_t* p_scheduler = NULL;
    p_scheduler = &g_pAdvSchInfo_periodic[g_currentExtAdv_periodic];

    if (p_scheduler->nextEventRemainder == LL_INVALID_TIME)
    {
        pPrdAdv->currentChn = llGetNextDataChanCSA2(pPrdAdv->periodic_adv_event_counter,\
                                                    (( ( pPrdAdv->AA & 0xFFFF0000 )>> 16 ) ^ ( pPrdAdv->AA  & 0x0000FFFF)),\
                                                    pPrdAdv->chn_map,\
                                                    pPrdAdv->chanMapTable,\
                                                    pPrdAdv->numUsedChans);
        pPrdAdv->pa_current_chn = pPrdAdv->currentChn;
        return;
    }

    if (p_scheduler->auxPduRemainder > p_scheduler->nextEventRemainder + pGlobal_config[LL_EXT_ADV_TASK_DURATION])
    {
        int i = 0;

        while ((i < 3) && !(pAdvInfo->parameter.priAdvChnMap & (1 << i))) i ++;

        pPrdAdv->currentChn = LL_ADV_CHAN_FIRST + i;
    }
    else if (p_scheduler->auxPduRemainder > p_scheduler->nextEventRemainder)     // no enough margin for ext adv
    {
        // skip 1 primary adv event
        p_scheduler->nextEventRemainder += pAdvInfo->primary_advertising_interval;
        pAdvInfo->adv_event_counter ++;
        pAdvInfo->adv_event_duration += pAdvInfo->primary_advertising_interval;
        // to simplify the process, here not check ext adv duration & counter, TO be add if required
        pPrdAdv->currentChn = llGetNextDataChanCSA2(pPrdAdv->periodic_adv_event_counter,\
                                                    (( ( pPrdAdv->AA & 0xFFFF0000 )>> 16 ) ^ ( pPrdAdv->AA  & 0x0000FFFF)),\
                                                    pPrdAdv->chn_map,\
                                                    pPrdAdv->chanMapTable,\
                                                    pPrdAdv->numUsedChans);
        pPrdAdv->pa_current_chn = pPrdAdv->currentChn;
    }
    else
    {
        pPrdAdv->currentChn = llGetNextDataChanCSA2(pPrdAdv->periodic_adv_event_counter,\
                                                    (( ( pPrdAdv->AA & 0xFFFF0000 )>> 16 ) ^ ( pPrdAdv->AA  & 0x0000FFFF)),\
                                                    pPrdAdv->chn_map,\
                                                    pPrdAdv->chanMapTable,\
                                                    pPrdAdv->numUsedChans);
        pPrdAdv->pa_current_chn = pPrdAdv->currentChn;
    }
}



void llSetupSyncInfo(extAdvInfo_t*  pAdvInfo, periodicAdvInfo_t* pPrdAdv)
{
	(void) pPrdAdv;
	(void) pAdvInfo;
    uint32      T2, elapse_time, remainder;
    llPeriodicAdvScheduleInfo_t* p_scheduler = NULL;
    p_scheduler = &g_pAdvSchInfo_periodic[g_currentExtAdv_periodic];
    memcpy(syncInfo.chn_map, pPrdAdv->chn_map, 4);
    syncInfo.chn_map4.chn_map = pPrdAdv->chn_map[4];
    syncInfo.chn_map4.sca     = pPrdAdv->sca;
    syncInfo.AA[0]            = pPrdAdv->AA & 0xff;
    syncInfo.AA[1]            = (pPrdAdv->AA >> 8) & 0xff;
    syncInfo.AA[2]            = (pPrdAdv->AA >> 16) & 0xff;
    syncInfo.AA[3]            = (pPrdAdv->AA >> 24) & 0xff;
    syncInfo.crcInit[0]       = pPrdAdv->crcInit & 0xff;
    syncInfo.crcInit[1]       = (pPrdAdv->crcInit >> 8) & 0xff;
    syncInfo.crcInit[2]       = (pPrdAdv->crcInit >> 16) & 0xff;
    syncInfo.offset.rfu       = 0;
    syncInfo.interval         =  pPrdAdv->adv_interval_max;// adv_interval;
    syncInfo.event_counter    = pPrdAdv->periodic_adv_event_counter;

    if (p_scheduler->auxPduRemainder >= 245700)
        syncInfo.offset.offsetUnit = 1;                                   // 300us, for aux offset >= 245700us
    else
        syncInfo.offset.offsetUnit = 0;                                   // 30us, for aux offset < 245700us

    syncInfo.offset.offsetAdj = 0;
    // calculate elapse time since last timer trigger
    T2 = read_current_fine_time();
    elapse_time = LL_TIME_DELTA(g_timerExpiryTick, T2);

    if (p_scheduler->auxPduRemainder > 2457600 + elapse_time)      // if > 2.4576 second
    {
        remainder = p_scheduler->auxPduRemainder - 2457600;
        syncInfo.offset.offsetAdj = 1;
    }
    else
        remainder = p_scheduler->auxPduRemainder;

    syncInfo.offset.syncPacketOffset = (remainder - elapse_time) / ((syncInfo.offset.offsetUnit == 1) ? 300 : 30);
}

void LL_EXT_Init_IQ_pBuff(uint16* ibuf,uint16* qbuf)
{
    if( ( ibuf != NULL ) && ( qbuf != NULL) )
    {
        g_pLLcteISample = ibuf;
        g_pLLcteQSample = qbuf;
    }
}


