/*******************************************************************************
    Filename:       patch_ext_adv.c, updated based on ll.c, ll_hwItf.c
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
#include "ll_debug.h"
#include "ll_enc.h"
#include "hci_event.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "osal_bufmgr.h"
#include "bus_dev.h"
#include "jump_function.h"
#include "global_config.h"
#include "rf_phy_driver.h"
#include "time.h"
#include "rom_sym_def.h"

// DEFINES
// #define OWN_PUBLIC_ADDR_POS      0x11004000
#define LL_HW_MODE_STX           0x00
#define LL_HW_MODE_SRX           0x01
#define LL_HW_MODE_TRX           0x02
#define LL_HW_MODE_RTX           0x03
#define LL_HW_MODE_TRLP          0x04
#define LL_HW_MODE_RTLP          0x05

// according to BQB test case HCI/GEV/BV-02-C & HCI/GEV/BV-03-C,
// mixed legacy advertisement and extended advertisement is disallowed
// mixed legacy scan and extended scan is disallowed
#define LL_MODE_INVALID       0xFF
#define LL_MODE_LEGACY        0x00
#define LL_MODE_EXTENDED      0x01
extern uint8  g_llScanMode;
extern uint8  g_llAdvMode;

#define LL_CALC_NEXT_SCAN_CHN(chan)     { chan ++; \
        chan = (chan > LL_SCAN_ADV_CHAN_39) ? LL_SCAN_ADV_CHAN_37 : chan;}

extern uint32 g_timer4_irq_pending_time;
extern uint8  llSecondaryState;

extern uint8 isTimer1Running(void);
extern uint8 isTimer4Running(void);
extern int   clear_timer_int(AP_TIM_TypeDef* TIMx);
extern void  clear_timer(AP_TIM_TypeDef* TIMx);

//------------------------------------------------------------------------------------
//extern rom function
//
extern uint8   ll_processExtAdvIRQ(uint32_t  irq_status);
extern uint8   ll_processPrdAdvIRQ(uint32_t  irq_status);
extern uint8   ll_processExtScanIRQ(uint32_t irq_status);
extern uint8   ll_processExtInitIRQ(uint32_t irq_status);
extern uint8   ll_processPrdScanIRQ(uint32_t irq_status);
extern uint8   ll_processBasicIRQ(uint32_t   irq_status);

// global configuration in SRAM, it could be change by application
// ================== VARIABLES  ==================================
extern uint32    global_config[];

// patch.c
extern uint32 ISR_entry_time;

extern uint32  read_ll_adv_remainder_time(void);
extern uint8_t ll_hw_read_rfifo1(uint8_t* rxPkt, uint16_t* pktLen, uint32_t* pktFoot0, uint32_t* pktFoot1);
// ll.c
extern uint32   g_interAuxPduDuration;

// FUNCTIONS
extern uint32 llWaitingIrq;
extern uint8  ll_hw_get_tr_mode(void);
extern int    ll_hw_get_rfifo_depth(void);

// ll_hwItf.c
/*******************************************************************************
    CONSTANTS
*/
// Master Sleep Clock Accurracy, in PPM
// Note: Worst case range value is assumed.
extern const uint16 SCA[] ;		//= {500, 250, 150, 100, 75, 50, 30, 20};
extern uint8 ownPublicAddr[];   // index 0..5 is LSO..MSB
extern uint8 ownRandomAddr[];
//
extern volatile uint8_t g_same_rf_channel_flag;


//  ====== RPA
extern uint8    isPeerRpaStore;
extern uint8    currentPeerRpa[LL_DEVICE_ADDR_LEN];
extern uint8    storeRpaListIndex;
extern uint8    g_currentLocalRpa[LL_DEVICE_ADDR_LEN];
extern uint8    g_currentPeerRpa[LL_DEVICE_ADDR_LEN];
extern uint8    g_currentPeerAddrType;
extern uint8    g_currentLocalAddrType;

void ll_hw_tx2rx_timing_config(uint8 pkt);
void ll_hw_trx_settle_config(uint8 pkt);


// =======  A2 multi-connection ========================
extern struct buf_tx_desc g_tx_adv_buf;
extern struct buf_tx_desc g_tx_ext_adv_buf;
extern struct buf_tx_desc tx_scanRsp_desc;

extern struct buf_rx_desc g_rx_adv_buf;

extern uint32 g_new_master_delta;

extern syncInfo_t         syncInfo;
extern scannerSyncInfo_t  scanSyncInfo;

// periodic advertiser device list
periodicAdvertiserListInfo_t g_llPeriodicAdvlist[LL_PRD_ADV_ENTRY_NUM];
uint8                        g_llPrdAdvDeviceNum; // current periodic advertiser device number


// RF path compensation, to be move to rf_phy_driver.c ?
extern int16  g_rfTxPathCompensation;
extern int16  g_rfRxPathCompensation;

//   EXTERNAL FUNCTIONS
void llWaitUs(uint32_t wtTime);

void llPrdAdvDecideNextChn(extAdvInfo_t* pAdvInfo, periodicAdvInfo_t* pPrdAdv);
void llSetupSyncInfo(extAdvInfo_t* pAdvInfo, periodicAdvInfo_t* pPrdAdv);


// for scan
uint32_t llScanT1;
uint32_t llScanTime = 0;
uint32_t llCurrentScanChn;


// New
uint8  scanningScanInd;
uint8  scanningauxchain;     
uint8  scanningpeer_addrtype;
uint8  scanningpeer_addr[LL_DEVICE_ADDR_LEN];    

uint8  activeScanAdi;              
uint16 extscanrsp_offset;          
uint32 llScanDuration;
uint32 g_auxconnreq_ISR_entry_time;

uint8_t tempAdvA[LL_DEVICE_ADDR_LEN];
uint8_t tempTargetA[LL_DEVICE_ADDR_LEN];


///================  for master =======================================
/*******************************************************************************
    @fn          move_to_master_function

    @brief       LL processing function for transit from init state to master state


    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None,

*/
void move_to_master_function1(void)

{
    llConnState_t * connPtr;
    int i;

//  hal_gpio_write(GPIO_P15, 1);
    if (g_llScanMode == LL_MODE_EXTENDED)
        connPtr = &conn_param[extInitInfo.connId];
    else
        connPtr = &conn_param[initInfo.connId];

//    LOG("move_to_master_function\r\n");
    // set connection parameters
    LL_set_default_conn_params(connPtr);
    for (i = 0; i < MAX_LL_BUF_LEN; i++)
        connPtr->ll_buf.tx_conn_desc[i] = connPtr->ll_buf.tx_not_ack_pkt;

    for (i = 0; i < MAX_LL_BUF_LEN; i++)
        connPtr->ll_buf.rx_conn_desc[i] = connPtr->ll_buf.tx_not_ack_pkt;

    // clear the connection buffer
    reset_conn_buf(connPtr->connId);
    for (i = 0; i < MAX_LL_BUF_LEN; i++)
        connPtr->ll_buf.tx_conn_desc[i] = 0;

    for (i = 0; i < MAX_LL_BUF_LEN; i++)
        connPtr->ll_buf.rx_conn_desc[i] = 0;
    
    initInfo.scanMode = LL_SCAN_STOP;
    // extInitInfo.scanMode = LL_SCAN_STOP;
    // adjust time units to 625us
    connPtr->curParam.winSize      <<= 1; // in 1.25ms units so convert to 625us
    connPtr->curParam.winOffset    <<= 1; // in 1.25ms units so convert to 625us
    connPtr->curParam.connInterval <<= 1; // in 1.25ms units so convert to 625us
    connPtr->curParam.connTimeout  <<= 4; // in 10ms units so convert to 625us
    // convert the LSTO from time to an expiration connection event count
    llConvertLstoToEvent( connPtr, &connPtr->curParam );
    // set the expiration connection event count to a specified limited number
    // Note: This is required in case the Master never sends that first packet.
    connPtr->expirationEvent = LL_LINK_SETUP_TIMEOUT; // 6 connection intervals (i.e. 0..5)
    // convert the Control Procedure timeout into connection event count
    llConvertCtrlProcTimeoutToEvent( connPtr );
    
    // calculate channel for data
//  llProcessChanMap(&conn_param[connId], conn_param[connId].chanMap );
    // need?
//  connPtr->slaveLatency = connPtr->curParam.slaveLatency ;
//  connPtr->slaveLatencyValue = connPtr->curParam.slaveLatency;    
    
    connPtr->currentEvent = 0;
    connPtr->nextEvent    = 0;
    connPtr->active       = 1;
    connPtr->sn_nesn      = 0;          // 1st rtlp, init sn/nesn as 0
    connPtr->llMode       = LL_HW_TRLP; // set as RTLP_1ST for the 1st connection event
    connPtr->currentChan  = 0;
    
    if (connPtr->channel_selection == LL_CHN_SEL_ALGORITHM_1)
        connPtr->currentChan = llGetNextDataChan(connPtr, 1);
    
    else
    {
        // channel selection algorithm 2
        connPtr->currentChan = llGetNextDataChanCSA2(0,
                                                     ( (connPtr->accessAddr & 0xFFFF0000) >> 16 ) ^ ( connPtr->accessAddr  & 0x0000FFFF ),
                                                     connPtr->chanMap,
                                                     connPtr->chanMapTable,
                                                     connPtr->numUsedChans);
    }
    
    llState = LL_STATE_CONN_MASTER;
    llSecondaryState = LL_SEC_STATE_IDLE; // add for multi-connection
    // wait to the next event start
    uint32  calibrate = pGlobal_config[LL_MOVE_TO_MASTER_DELAY];
    uint32  schedule_time;
    
    // get current allocate ID delta time to ID 0
    if (g_ll_conn_ctx.currentConn != LL_INVALID_CONNECTION_ID)
    {
        // delta timing to previous connection slot
        schedule_time = g_new_master_delta;// * 625;
        ll_addTask(connPtr->connId, g_new_master_delta); // shcedule new connection
        g_ll_conn_ctx.scheduleInfo[connPtr->connId].task_duration = 2700;   // master task duration
        
        // current link id may be updated in ll_addTask, update the ll state
        if (g_ll_conn_ctx.scheduleInfo[g_ll_conn_ctx.currentConn].linkRole == LL_ROLE_MASTER)
            llState = LL_STATE_CONN_MASTER;
        else if (g_ll_conn_ctx.scheduleInfo[g_ll_conn_ctx.currentConn].linkRole == LL_ROLE_SLAVE)
            llState = LL_STATE_CONN_SLAVE;
    }
    else //  1st connection
    {
        if (g_llScanMode == LL_MODE_EXTENDED)
        {
            if ( (connPtr->llRfPhyPktFmt == PKT_FMT_BLR500K)
              || (connPtr->llRfPhyPktFmt == PKT_FMT_BLR125K) )
                schedule_time = 3750 + connPtr->curParam.winOffset * 625 + 2900 + calibrate;// 2900(us): CONN_REQ duration
            
            else
                schedule_time = 2500 + connPtr->curParam.winOffset * 625 + 352 + calibrate;// 352(us): CONN_REQ duration
        }
        else
            schedule_time = 1250 + connPtr->curParam.winOffset * 625 + 352 + calibrate;// 352(us): CONN_REQ duration
        
        ll_addTask(connPtr->connId, schedule_time);
        g_ll_conn_ctx.scheduleInfo[connPtr->connId].task_duration = 2700;   // master task duration

        // current link id may be updated in ll_addTask, update the ll state
        if (g_ll_conn_ctx.scheduleInfo[g_ll_conn_ctx.currentConn].linkRole == LL_ROLE_MASTER)
            llState = LL_STATE_CONN_MASTER;
        else if (g_ll_conn_ctx.scheduleInfo[g_ll_conn_ctx.currentConn].linkRole == LL_ROLE_SLAVE)
            llState = LL_STATE_CONN_SLAVE;
    }
    
    g_ll_conn_ctx.scheduleInfo[connPtr->connId].linkRole = LL_ROLE_MASTER;
    
    if (g_llScanMode != LL_MODE_EXTENDED)
        (void)osal_set_event(LL_TaskID, LL_EVT_MASTER_CONN_CREATED);

    g_pmCounters.ll_conn_succ_cnt ++;
//  hal_gpio_write(GPIO_P15, 0);
//    LOG("M2M ");
}


/*******************************************************************************
    @fn          move_to_slave_function0

    @brief       This function is used to process CONN_REQ and move the llState to slave


    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None,

*/
void move_to_slave_function3() {
    llConnState_t * connPtr; // r0 MAPDST
    uint8 *         pBuf; // r0 MAPDST
    uint8_t         tempByte;
    uint8_t         chnSel;
    uint32_t        calibra_time, T2;
    
    connPtr = (llConnState_t * ) llAllocConnId();
    if ( connPtr == NULL )
    {
        return;
    }

    adv_param.connId = connPtr->connId;
    chnSel   = (g_rx_adv_buf.rxheader & CHSEL_MASK) >> CHSEL_SHIFT;

    if (pGlobal_config[LL_SWITCH] & CONN_CSA2_ALLOW)
        connPtr->channel_selection = chnSel;
    else
        connPtr->channel_selection = LL_CHN_SEL_ALGORITHM_1;
    
    pBuf = g_rx_adv_buf.data;
    // reset connection parameters
    LL_set_default_conn_params(connPtr);
    // clear the connection buffer
    reset_conn_buf(connPtr->connId);
    
    // switch off the adv, will be switch on after link terminate by GAP
    if (llTaskState == LL_TASK_EXTENDED_ADV)
    {
        // TODO: ext advertiser process
    }
    else
        adv_param.advMode = LL_ADV_MODE_OFF;

    pBuf += 12;      // skip initA and AdvA
    pBuf = llMemCopySrc( (uint8*)&connPtr->accessAddr,             pBuf, 4 );
    pBuf = llMemCopySrc( (uint8*)&connPtr->initCRC,                pBuf, 3 );
    pBuf = llMemCopySrc( (uint8*)&connPtr->curParam.winSize,       pBuf, 1 );
    pBuf = llMemCopySrc( (uint8*)&connPtr->curParam.winOffset,     pBuf, 2 );
    pBuf = llMemCopySrc( (uint8*)&connPtr->curParam.connInterval,  pBuf, 2 );
    pBuf = llMemCopySrc( (uint8*)&connPtr->curParam.slaveLatency,  pBuf, 2 );
    pBuf = llMemCopySrc( (uint8*)&connPtr->curParam.connTimeout,   pBuf, 2 );
    // TI style: convert to 625us tick
    connPtr->curParam.winSize      <<= 1;
    connPtr->curParam.winOffset    <<= 1;
    connPtr->curParam.connInterval <<= 1;
    connPtr->curParam.connTimeout  <<= 4;
    llConvertLstoToEvent( connPtr, &(connPtr->curParam) );     // 16MHz CLK, need 56.5us
    // bug fixed 2018-4-4, calculate control procedure timeout value when connection setup
    // convert the Control Procedure timeout into connection event count
   
    llConvertCtrlProcTimeoutToEvent(connPtr);
    
    if (((connPtr->curParam.connTimeout <= ((connPtr->curParam.slaveLatency ) * connPtr->curParam.connInterval << 1)))
            || (connPtr->curParam.connInterval == 0) )
    {
        // schedule LL Event to notify the Host a connection was formed with
        // a bad parameter
        // Note: This event doesn't take parameters, so it is assumed there that
        //       the reason code was due to an unacceptable connection interval.
        (void)osal_set_event( LL_TaskID, LL_EVT_SLAVE_CONN_CREATED_BAD_PARAM );
        return;
    }
    
    pBuf = llMemCopySrc( (uint8*)connPtr->chanMap,   pBuf, 5 );
    pBuf = llMemCopySrc( &tempByte,   pBuf, 1 );
    connPtr->hop  = tempByte & 0x1F;
    connPtr->sleepClkAccuracy  = (tempByte >> 5) & 0x07;
    // calculate channel for data
    llProcessChanMap(connPtr, connPtr->chanMap );   // 16MHz clk, cost 116us!
    connPtr->slaveLatency = 0;        //correct 05-09, no latency before connect
    connPtr->slaveLatencyValue = connPtr->curParam.slaveLatency;
    connPtr->accuTimerDrift = 0;
    llAdjSlaveLatencyValue(connPtr);
    
    connPtr->llRfPhyPktFmt = g_rfPhyPktFmt;   
    
    // combine slave SCA with master's SCA and calculate timer drift factor
    //connPtr->scaFactor = llCalcScaFactor( connPtr->sleepClkAccuracy );
    //connPtr->currentChan = llGetNextDataChan(1);
    llState = LL_STATE_CONN_SLAVE;
    ll_debug_output(DEBUG_LL_STATE_CONN_SLAVE);
    connPtr->active = TRUE;
    connPtr->sn_nesn = 0;                  // 1st rtlp, init sn/nesn as 0
    connPtr->llMode = LL_HW_RTLP_1ST;     // set as RTLP_1ST for the 1st connection event
    // calculate the 1st channel
    connPtr->currentChan = 0;
//  hal_gpio_write(GPIO_P15, 1);
//  BM_SET(reg_gpio_ioe_porta, BIT(GPIO_P15));
//  BM_SET(reg_gpio_swporta_dr, BIT(GPIO_P15));
    if (connPtr->channel_selection == LL_CHN_SEL_ALGORITHM_1)
        connPtr->currentChan = llGetNextDataChan(connPtr, 1);
    else
    {
        // channel selection algorithm 2
        connPtr->currentChan = llGetNextDataChanCSA2(0,
                                                     (( connPtr->accessAddr & 0xFFFF0000 )>> 16 ) ^ ( connPtr->accessAddr  & 0x0000FFFF),
                                                     connPtr->chanMap,
                                                     connPtr->chanMapTable,
                                                     connPtr->numUsedChans);
    }
//  hal_gpio_write(GPIO_P15, 0);
//  BM_SET(reg_gpio_ioe_porta, BIT(GPIO_P15));
//  BM_CLR(reg_gpio_swporta_dr, BIT(GPIO_P15));
    // calculate timer drift
    llCalcTimerDrift(connPtr->curParam.winOffset + 2,        // 1250us + win offset, in 625us tick
                     connPtr->slaveLatency,
                     connPtr->sleepClkAccuracy,
                     (uint32*)&(connPtr->timerDrift));
    T2 = read_current_fine_time();
    // calculate the SW delay from ISR to here
    calibra_time = (T2 > ISR_entry_time) ? (T2 - ISR_entry_time) : (BASE_TIME_UNITS - ISR_entry_time + T2);
    // TODO: need consider the case: 0ms window offset, sometimes timer offset will < 0,
//    timer1 = 1250 + conn_param[connId].curParam.winOffset * 625 - calibra_time - conn_param[connId].timerDrift;
//    if (timer1 < 0)
//        while(1);
//
//    ll_schedule_next_event(timer1);
//#ifdef MULTI_ROLE
    uint32_t  temp = 1250;
        
    if (g_llAdvMode == LL_MODE_EXTENDED)
    {
        extAdvInfo_t * pAdvInfo = g_pAdvSchInfo[g_currentExtAdv].pAdvInfo;
        if (pAdvInfo)
        {
            if (!ll_isLegacyAdv(pAdvInfo))
            {
                temp = 2500;
                uint8_t pktFmt = connPtr->llRfPhyPktFmt;
                if ( (pktFmt == PKT_FMT_BLR500K) || (pktFmt == PKT_FMT_BLR125K) )
                    temp = 3750;
                    
			    calibra_time = (T2 > g_auxconnreq_ISR_entry_time)
			                 ? (T2 - g_auxconnreq_ISR_entry_time)
			                 : (BASE_TIME_UNITS - g_auxconnreq_ISR_entry_time + T2);
            }
        }
    }
    // other delay: conn req tail->ISR: 32us, timing advance: 50us, HW engine startup: 60us
    // start slave event SW process time: 50us
    // soft parameter: pGlobal_config[CONN_REQ_TO_SLAVE_DELAY]
    calibra_time += pGlobal_config[CONN_REQ_TO_SLAVE_DELAY];     //(32 + 50 + 60 + 50 + pGlobal_config[CONN_REQ_TO_SLAVE_DELAY]);

    
    if (g_ll_conn_ctx.numLLConns == 1)     // 1st connection, time1 is for adv event
        clear_timer(AP_TIM1);                // stop the timer between different adv channel
        
    temp = temp + connPtr->curParam.winOffset * 625 - calibra_time - connPtr->timerDrift;

    ll_addTask(connPtr->connId, temp);

    g_ll_conn_ctx.scheduleInfo[connPtr->connId].task_duration = 3000;

    // current link id may be updated in ll_addTask, update the ll state
    if (g_ll_conn_ctx.scheduleInfo[g_ll_conn_ctx.currentConn].linkRole == LL_ROLE_MASTER)
        llState = LL_STATE_CONN_MASTER;
    else if (g_ll_conn_ctx.scheduleInfo[g_ll_conn_ctx.currentConn].linkRole == LL_ROLE_SLAVE)
        llState = LL_STATE_CONN_SLAVE;

    llSecondaryState = LL_SEC_STATE_IDLE;
//  LOG("M2S:%d\n", temp);
//#else
//    ll_schedule_next_event(1250 + connPtr->curParam.winOffset * 625 - calibra_time - connPtr->timerDrift);
//    g_ll_conn_ctx.currentConn = connPtr->connId;
//#endif
    g_ll_conn_ctx.scheduleInfo[connPtr->connId].linkRole = LL_ROLE_SLAVE;
    (void)osal_set_event( LL_TaskID, LL_EVT_SLAVE_CONN_CREATED);
    g_pmCounters.ll_conn_succ_cnt ++;     // move to anchor point catch ?
//    hal_gpio_write(GPIO_P15, 0);
   
}


void ll_adv_scheduler0(void)

{
    uint32  T2, T3, delta;
    uint32  minAuxPduTime, minPriPduTime;
    uint8   minIndexAux, minIndexPri;
    uint8   done = FALSE;
    int     i;

    llAdvScheduleInfo_t* p_scheduler;
    extAdvInfo_t*  pAdvInfo;

    if (g_currentExtAdv == LL_INVALID_ADV_SET_HANDLE) {
        return;
    }

    // calculate elapse time since last timer trigger
    T2 = read_current_fine_time();
    delta = LL_TIME_DELTA(g_timerExpiryTick, T2);
    p_scheduler = &g_pAdvSchInfo[g_currentExtAdv];
    pAdvInfo = p_scheduler->pAdvInfo;
    
    // if current adv is not active, delete it from task list
    if (pAdvInfo->active == FALSE) {
        ll_delete_adv_task(g_currentExtAdv);
        return;
    }

    // the advertisement scheduler will schedule next task according to current adv's next advertise channel
    // case 1: not finish primary ADV channel or AUX_XXX_IND chain, continue advertise current adv task
    if (pAdvInfo->currentChn > LL_ADV_CHAN_FIRST &&   // next channel in primary adv channel
            pAdvInfo->active == TRUE                 &&   // add 04-01, legacy adv may stop when receive conn_req
            !ll_isFirstAdvChn(pAdvInfo->parameter.priAdvChnMap, pAdvInfo->currentChn))       // not the 1st primary ADV channel
    {
        ll_ext_adv_schedule_next_event(pGlobal_config[LL_EXT_ADV_INTER_PRI_CHN_INT] - delta);
        done = TRUE;
    }
    else if (pAdvInfo->currentChn < LL_ADV_CHAN_FIRST    // broadcast in aux adv chn
             &&  pAdvInfo->currentAdvOffset > 0)              // offset in adv data set greater than 0 means next PDU is AUX_CHAIN_IND PDU
    {
        // g_interAuxPduDuration is the time between the edge of AUX_ADV_IND & AUX_CHAIN_IND,
        // delta consider the time between timer expiry time and new timer trigger point,
        // the time from TO to ll_hw_trigger is not consider, this time denote as alpha below.
        // If alpha for AUX_ADV_IND and AUX_CHAIN_IND is the same, then no compensation is required
        // but if the alpha for AUX_ADV_IND and AUX_CHAIN_IND is different, using pGlobal_config[LL_EXT_ADV_INTER_SEC_COMP]
        // to compensate it
        if (extscanrsp_offset != 100) {
            delta = (g_interAuxPduDuration - delta);
        } else {
            delta = (g_interAuxPduDuration - delta) - 125;
        }
        ll_ext_adv_schedule_next_event(delta);
        done = TRUE;
    }

    // update scheduler task list
    ll_updateExtAdvRemainderTime(delta);

    if (done)               // next task schedule done
        return;

    // case 2: finish broadcast in primary adv channel/aux adv channel. Check ext adv scheduler to get the earliest task
    // search the nearest expiry task and PDU type
    minAuxPduTime = g_pAdvSchInfo[g_currentExtAdv].auxPduRemainder;
    minPriPduTime = g_pAdvSchInfo[g_currentExtAdv].nextEventRemainder;
    minIndexAux = g_currentExtAdv;
    minIndexPri = g_currentExtAdv;

	for (i = 0; i < g_extAdvNumber; i++) {
		if (g_pAdvSchInfo[i].adv_handler != LL_INVALID_ADV_SET_HANDLE)
		{
		    if (g_pAdvSchInfo[i].auxPduRemainder < minAuxPduTime) {
		        minIndexAux = i;
		        minAuxPduTime = g_pAdvSchInfo[i].auxPduRemainder;
		    }
		    if (g_pAdvSchInfo[i].nextEventRemainder < minPriPduTime) {
                minIndexPri = i;
                minPriPduTime = g_pAdvSchInfo[i].nextEventRemainder;
		    }
		}
	}
    T3 = read_current_fine_time();
    delta = LL_TIME_DELTA(T2, T3);
    // compare the minimum AUX channel remainder time & minimum Primary channel PDU remainder time,
    // AUX channel PDU could add some pre-emphesis here
    uint32  auxPduEmphesis = pGlobal_config[LL_EXT_ADV_INTER_PRI_CHN_INT] * 3;                           // add 03-31

    if ( (minAuxPduTime != LL_INVALID_TIME) &&
         (minAuxPduTime < (minPriPduTime + auxPduEmphesis)) ) // next schedule task is aux PDU
    {
        ll_ext_adv_schedule_next_event(minAuxPduTime - delta);
        g_currentExtAdv   = minIndexAux;
	}
	else // next schedule task is pri PDU
	{        
        ll_ext_adv_schedule_next_event(minPriPduTime - delta);
        g_currentExtAdv   = minIndexPri;
    }

    // update scheduler task list
    ll_updateExtAdvRemainderTime(delta);
}


void ll_add_adv_task0(extAdvInfo_t * pExtAdv)

{
    int i, spare;
    llAdvScheduleInfo_t* p_scheduler = NULL, *p_current_scheduler;
    uint32  remainder;
    uint8   chanNumber, temp;

    uint8 isLegacy = ll_isLegacyAdv(pExtAdv);
    
    temp = pExtAdv->parameter.priAdvChnMap;
    chanNumber = (temp & 0x01) + ((temp & 0x02) >> 1) + ((temp & 0x04) >> 2);
    // init new Ext adv event context
    pExtAdv->currentChn = ((temp & ((~temp) + 1)) >> 1) + 37;       // calculate 1st adv channel
    pExtAdv->adv_event_counter  = 0;
    pExtAdv->currentAdvOffset   = 0;
    pExtAdv->adv_event_duration = 0;

    // ======== case 1: the 1st adv task
    if (g_currentExtAdv == LL_INVALID_ADV_SET_HANDLE)     //    if (!isTimer4Running())
    {
        g_schExtAdvNum  = 1;
        g_currentExtAdv = 0;          // scheduler index = 0
        p_current_scheduler = &g_pAdvSchInfo[g_currentExtAdv];
        
        if (isLegacy)            // Legacy Adv PDU has no aux PDU
        {
            g_pAdvSchInfo[g_currentExtAdv].auxPduRemainder = LL_INVALID_TIME;
		}
		else            
		{
			if ( pExtAdv->data.advertisingDataLength ||
                (pExtAdv->parameter.advEventProperties & LE_ADV_PROP_SCAN_BITMASK) ||
                (pExtAdv->parameter.advEventProperties & LE_ADV_PROP_CONN_BITMASK) )
	            ll_allocAuxAdvTimeSlot(g_currentExtAdv); // g_currentExtAdv == 0
	        else
	            g_pAdvSchInfo[g_currentExtAdv].auxPduRemainder = LL_INVALID_TIME;
		}
        
//      LOG("[%d]  ", g_pAdvSchInfo[i].auxPduRemainder);

        if (llWaitingIrq == TRUE)
        {
            g_currentAdvTimer = pGlobal_config[LL_CONN_TASK_DURATION];
        }
        else
        {
        	if ( isTimer1Running() &&
                 ((remainder = read_LL_remainder_time()) < pGlobal_config[LL_EXT_ADV_TASK_DURATION]) )     // timer1 for connection or legacy adv
            {
                g_currentAdvTimer = pGlobal_config[LL_CONN_TASK_DURATION] + remainder;
            }
            else
            {
		        if (chanNumber > 1)
		            g_currentAdvTimer = pGlobal_config[LL_EXT_ADV_INTER_PRI_CHN_INT];
		        else
		            g_currentAdvTimer = (p_current_scheduler->auxPduRemainder < p_current_scheduler->pAdvInfo->primary_advertising_interval) ?
		                                p_current_scheduler->auxPduRemainder : p_current_scheduler->pAdvInfo->primary_advertising_interval;
                
                g_timerExpiryTick = read_current_fine_time();
	            // invoke set up ext adv function
                llSetupExtAdvEvent(pExtAdv);
            }
        }
        
        //ll_ext_adv_schedule_next_event(g_currentAdvTimer);
        //g_timerExpiryTick = read_current_fine_time();  // fake timer expiry tick
        p_current_scheduler->pAdvInfo = pExtAdv;
        p_current_scheduler->adv_handler = pExtAdv->advHandle;
        p_current_scheduler->nextEventRemainder = p_current_scheduler->pAdvInfo->primary_advertising_interval;  // add some random delay between 0-10ms?
        pExtAdv->active = TRUE;
        return;
    }

    // ===== case 2: there are ongoing adv
    p_current_scheduler = &g_pAdvSchInfo[g_currentExtAdv];

    // get the 1st spare scheduler slot
    for (i = 0; i < g_extAdvNumber; i++)    // bug fixed 04-07
    {
        if (g_pAdvSchInfo[i].adv_handler == LL_INVALID_ADV_SET_HANDLE)
        {
            p_scheduler = &g_pAdvSchInfo[i];
            spare = i;
            break;
        }
    }

    // no empty scheduler slot, return
    if (p_scheduler == NULL)
        return;

    g_schExtAdvNum ++;
    pExtAdv->active = TRUE;

    // arrange the timing of AUX_XXX_IND, it is independent to EXT_ADV_IND
    // add 03-31
    if (!isLegacy && pExtAdv->data.advertisingDataLength)
        ll_allocAuxAdvTimeSlot(spare);
    else
        g_pAdvSchInfo[spare].auxPduRemainder = LL_INVALID_TIME;

    if (isTimer4Running())
    {
        read_ll_adv_remainder_time();
        read_current_fine_time();
    }
    
    if (isTimer1Running())
    {
        read_LL_remainder_time();
    }
    
    // case 2.2: no enough time, start new adv after current one

    // add new adv to adv scheduler list, not change current adv task
    p_scheduler->nextEventRemainder = p_current_scheduler[g_currentExtAdv].nextEventRemainder
    								+ (g_advSlotPeriodic >> 2)
    								* (spare > g_currentExtAdv ? 
    								   spare - g_currentExtAdv :
    								   g_extAdvNumber + spare - g_currentExtAdv);
    p_scheduler->adv_handler = pExtAdv->advHandle;
    p_scheduler->pAdvInfo    = pExtAdv;
    return;
}

void ll_delete_adv_task0(uint8 index)

{
    uint32  T1, T2, delta, remainder, elapse_time;
    uint32  minAuxPduTime, minPriPduTime;
    uint8   minIndexAux, minIndexPri;
    int i;
//  LOG("=== adv task % deleted \r\n", index);
    g_pAdvSchInfo[index].adv_handler        = LL_INVALID_ADV_SET_HANDLE;
    g_pAdvSchInfo[index].pAdvInfo           = NULL;
    g_pAdvSchInfo[index].auxPduRemainder    = LL_INVALID_TIME;
    g_pAdvSchInfo[index].nextEventRemainder = LL_INVALID_TIME;
    g_schExtAdvNum --;

    // only 1 task case, clear scheduler info and stop timer
    if (g_schExtAdvNum == 0) {
        g_currentExtAdv = LL_INVALID_ADV_SET_HANDLE;
        g_schExtAdvNum = 0;
        clear_timer(AP_TIM4);
        return;
    }

    // current awaiting adv is disable, and there are more than 1 task
    if ( (index == g_currentExtAdv) &&
    	  isTimer4Running())
   	{
        remainder = read_ll_adv_remainder_time();
        T1 = read_current_fine_time();
        elapse_time = g_currentAdvTimer - remainder;
        
        minAuxPduTime = g_pAdvSchInfo[g_currentExtAdv].auxPduRemainder;      // LL_INVALID_TIME
        minPriPduTime = g_pAdvSchInfo[g_currentExtAdv].nextEventRemainder;
        minIndexAux = g_currentExtAdv;
        minIndexPri = g_currentExtAdv;

        for (i = 0; i < g_extAdvNumber; i++)
        {
	        if (g_pAdvSchInfo[i].adv_handler != LL_INVALID_ADV_SET_HANDLE)
	        {
                if (g_pAdvSchInfo[i].auxPduRemainder < minAuxPduTime)
                {
                    minIndexAux = i;
                    minAuxPduTime = g_pAdvSchInfo[i].auxPduRemainder;
                }
                
	            if (g_pAdvSchInfo[i].nextEventRemainder < minPriPduTime)
	            {
	                minIndexPri = i;
	                minPriPduTime = g_pAdvSchInfo[i].nextEventRemainder;
	            }
            }
        }

        // start new timer
        T2  = read_current_fine_time();
        delta = LL_TIME_DELTA(T1, T2);

        if (minAuxPduTime < minPriPduTime)   // next schedule task is aux PDU
        {
            ll_ext_adv_schedule_next_event(minAuxPduTime - elapse_time - delta);
            g_currentExtAdv = minIndexAux;
        }
        else   // next schedule task is pri PDU
        {
            ll_ext_adv_schedule_next_event(minPriPduTime - elapse_time - delta);
            g_currentExtAdv = minIndexPri;
        }

        // update the scheduler list
        ll_updateExtAdvRemainderTime(elapse_time + delta);
    }
}

uint8 llSetupExtAdvEvent0(extAdvInfo_t * pAdvInfo)

{
    uint8 ch_idx, pktFmt, auxPduIndFlag = FALSE;
    int i;
    uint32 T2, T1, delta, temp;

    ch_idx = pAdvInfo->currentChn;
    T1 = read_current_fine_time();
    //LOG("%d ", pAdvInfo->currentChn);
    
    if (ch_idx >= LL_ADV_CHAN_FIRST && ch_idx <= LL_ADV_CHAN_LAST )   // advertise at primary channel case
    {
        llSetupAdvExtIndPDU(pAdvInfo, NULL);
        // decide next adv channel
        i = ch_idx - LL_ADV_CHAN_FIRST + 1;
        
        while ((i < 3) && !(pAdvInfo->parameter.priAdvChnMap & (1 << i))) i ++;       // search channel map for next adv channel number
        
        if (i == 3)   // finish primary adv channel broadcast
        {
            if ((g_pAdvSchInfo[g_currentExtAdv].auxPduRemainder != LL_INVALID_TIME) &&           
            	(g_pAdvSchInfo[g_currentExtAdv].auxPduRemainder > g_pAdvSchInfo[g_currentExtAdv].nextEventRemainder))
                pAdvInfo->currentChn = ll_getFirstAdvChn(pAdvInfo->parameter.priAdvChnMap);
            else
                pAdvInfo->currentChn = pAdvInfo->auxChn;
        }
        else
            pAdvInfo->currentChn = LL_ADV_CHAN_FIRST + i;

        // config primary PHY
        pktFmt = pAdvInfo->parameter.primaryAdvPHY;
    }
    else  // advertise at primary channel case
    {
        // Note: if the Ext adv has no aux pdu, llSetupExtAdv should not be invoked
        
        if (((extscanrsp_offset == 0) || 
        	(pAdvInfo->scanRspMaxLength <= extscanrsp_offset)) &&
            (pAdvInfo->currentAdvOffset == 0)) // 1st AUX PDU. AUX_ADV_IND should include advData
        {
            llSetupAuxAdvIndPDU(pAdvInfo, NULL);
            auxPduIndFlag = TRUE;
        }
        else
            llSetupAuxChainIndPDU(pAdvInfo, NULL);

        // config secondary PHY
        pktFmt = pAdvInfo->parameter.secondaryAdvPHY;
    }
    
    if (pAdvInfo->parameter.secondaryAdvPHY == 3)           // coded PHY
        pktFmt = PKT_FMT_BLR125K;          //
        
    HAL_ENTER_CRITICAL_SECTION();

    // if there is ongoing LL HW task, skip this task
    if (llWaitingIrq == TRUE)
    {
//      g_pmCounters.ll_tbd_cnt1++;
        HAL_EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    //============== configure and trigger LL HW engine, LL HW work in Single Tx mode  ==================
    g_rfPhyPktFmt = pktFmt;
    rf_phy_change_cfg0(pktFmt);
    ll_hw_tx2rx_timing_config(pktFmt);
    set_crc_seed(ADV_CRC_INIT_VALUE);     // crc seed for adv is same for all channels
    set_access_address(ADV_SYNCH_WORD);   // access address
    set_channel(ch_idx);             // channel
    set_whiten_seed(ch_idx);         // whiten seed
    set_max_length(50);            // rx PDU max length
    // reset Rx/Tx FIFO
    ll_hw_rst_rfifo();
    ll_hw_rst_tfifo();

	// for AUX_ADV_IND, connectable/scannable case, should configure TRX
    if ((auxPduIndFlag == TRUE)                      &&                     // AUX_ADV_IND
            ((pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_CONN_BITMASK) ||
             (pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_SCAN_BITMASK)))
    {

        ll_hw_trx_settle_config(g_rfPhyPktFmt);
        ll_hw_set_trx();
        if ((g_rfPhyPktFmt == PKT_FMT_BLE1M) || (g_rfPhyPktFmt == PKT_FMT_BLE2M))
	        ll_hw_set_rx_timeout(500);
        else
	        ll_hw_set_rx_timeout(3000);
    }
    else
    {
        ll_hw_set_stx();
    }
    
    ll_hw_ign_rfifo(LL_HW_IGN_EMP | LL_HW_IGN_CRC);       //set the rfifo ign control
    //write Tx FIFO
    ll_hw_write_tfifo((uint8*)&(g_tx_ext_adv_buf.txheader), ((g_tx_ext_adv_buf.txheader & 0xff00) >> 8) + 2);
    T2 = read_current_fine_time();
    delta = LL_TIME_DELTA(T1, T2);
    temp = ( pGlobal_config[LL_EXT_ADV_PROCESS_TARGET] > delta) ? (pGlobal_config[LL_EXT_ADV_PROCESS_TARGET] - delta) : 0;
    llWaitUs(temp);             // insert delay to make process time equal PROCESS_TARGET
    ll_hw_go();

    llWaitingIrq = TRUE;
    llTaskState = LL_TASK_EXTENDED_ADV;

    HAL_EXIT_CRITICAL_SECTION();
    return TRUE;
}


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

    // adv PDU header
    g_tx_ext_adv_buf.txheader = 0;
	// PDU type, 4 bits
	SET_BITS(g_tx_ext_adv_buf.txheader, ADV_EXT_TYPE, PDU_TYPE_SHIFT, PDU_TYPE_MASK);
	// RFU, ChSel, TxAdd, RxAdd
	if ((pAdvInfo->parameter.ownAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC) ||
		(pAdvInfo->parameter.ownAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM))
	{
	
		if ((adv_param.ownAddr[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR)
            SET_BITS(g_tx_adv_buf.txheader, LL_DEV_ADDR_TYPE_RANDOM, TX_ADD_SHIFT, TX_ADD_MASK);
	}
	else // This will cutoff ownAddrType high bit
        SET_BITS(g_tx_adv_buf.txheader, pAdvInfo->parameter.ownAddrType, TX_ADD_SHIFT, TX_ADD_MASK);

    if (pGlobal_config[LL_SWITCH] & CONN_CSA2_ALLOW)
        SET_BITS(g_tx_adv_buf.txheader, 1, CHSEL_SHIFT, CHSEL_MASK);

    extHdrLength = 0;
    // == step 1. decide what fields should be present in extended header
    // extended header
    extHeaderFlag = 0;
    extHdrLength ++;

	if ( (pAdvInfo->isPeriodic == FALSE) &&
	     (pAdvInfo->data.dataComplete == TRUE) &&
	     (pAdvInfo->data.advertisingDataLength == 0) &&
	     (advMode == LL_EXT_ADV_MODE_NOCONN_NOSC) )
    {
    	extHeaderFlag |= LE_EXT_HDR_ADVA_PRESENT_BITMASK;
	    extHdrLength += 6;
		if (pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_DIRECT_BITMASK)
		{
		    extHeaderFlag |= LE_EXT_HDR_TARGETA_PRESENT_BITMASK;
		    extHdrLength += 6;
		}
    	
    }
    else
    {
   	    extHeaderFlag |= LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK;
	    extHdrLength += 2;

        extHeaderFlag |= LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK;
        extHdrLength += 3;
	}
	
    if (pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_TX_POWER_BITMASK)
    {
        extHeaderFlag |= LE_EXT_HDR_TX_PWR_PRESENT_BITMASK;
        extHdrLength ++;
    }

	
    length = 1 + extHdrLength;    // 1: extended header len(6bits) + advMode(2bit)
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
        if (g_currentLocalAddrType == LL_DEV_ADDR_TYPE_RANDOM && pAdvInfo->parameter.isOwnRandomAddressSet == TRUE)
            memcpy(&g_tx_ext_adv_buf.data[offset], pAdvInfo->parameter.ownRandomAddress, LL_DEVICE_ADDR_LEN);
        else    // public address
            memcpy(&g_tx_ext_adv_buf.data[offset], adv_param.ownAddr, LL_DEVICE_ADDR_LEN);

        offset += LL_DEVICE_ADDR_LEN;
    }

    // TargetA(6 octets)
    if (extHeaderFlag & LE_EXT_HDR_TARGETA_PRESENT_BITMASK)
    {
    	if (g_currentTimerTask)
	        memcpy(&g_tx_ext_adv_buf.data[offset], pAdvInfo->parameter.peerAddress, LL_DEVICE_ADDR_LEN);
    	else
	        memcpy(&g_tx_ext_adv_buf.data[offset], g_currentPeerRpa, LL_DEVICE_ADDR_LEN);
    	   
        offset += LL_DEVICE_ADDR_LEN;
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
            // the interval between chan 37<->38, 38<->39 is 5000us, primary adv->aux adv chn is 1500us
            offset_unit = 0;    // 30us, for aux offset < 245700us
            aux_offset = (number * pGlobal_config[LL_EXT_ADV_INTER_PRI_CHN_INT] + g_interAuxPduDuration) / 30;
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

    if (extHeaderFlag & LE_EXT_HDR_TX_PWR_PRESENT_BITMASK)
    {
        short radio_pwr = pAdvInfo->tx_power * 10 + g_rfTxPathCompensation;
        if (radio_pwr < -1270)
        	radio_pwr = -1270;
        else if (radio_pwr > 1270)
            radio_pwr = 1270;

        g_tx_ext_adv_buf.data[offset] = radio_pwr / 10;;
    }
    pAdvInfo->currentAdvOffset = 0;

}


void LL_slave_conn_event1(void)

{
    uint16_t ll_rdCntIni;
    uint32_t      tx_num, rx_num;
    llConnState_t* connPtr;

    g_ll_conn_ctx.timerExpiryTick = read_current_fine_time();                     // A2 multiconnection
//  hal_gpio_write(GPIO_P14, 1);
    connPtr = &conn_param[g_ll_conn_ctx.currentConn];
    // time critical process, disable interrupt
    HAL_ENTER_CRITICAL_SECTION();
    tx_num = pGlobal_config[LL_TX_PKTS_PER_CONN_EVT];
    rx_num = pGlobal_config[LL_RX_PKTS_PER_CONN_EVT];

    if (tx_num > g_maxPktPerEventTx || tx_num == 0)    tx_num = g_maxPktPerEventTx;

    if (rx_num > g_maxPktPerEventRx || rx_num == 0)    rx_num = g_maxPktPerEventRx;

    connPtr->pmCounter.ll_conn_event_cnt ++;

    //ZQ 20191209
    //restore the currentChan for disable slavelatency
    //ZQ20200207 should use nextChan
    connPtr->lastCurrentChan = connPtr->nextChan;
    // counter for one connection event
    llResetRfCounters();
    //support rf phy change
    rf_phy_change_cfg(connPtr->llRfPhyPktFmt);
    ll_hw_tx2rx_timing_config(connPtr->llRfPhyPktFmt);
    
    ll_hw_rst_rfifo();
    ll_hw_rst_tfifo();
    
    // channel physical configuration
    set_crc_seed(connPtr->initCRC);            // crc seed for data PDU is from CONNECT_REQ
    set_access_address(connPtr->accessAddr);     // access address
    set_channel(connPtr->currentChan);        // set channel
    set_whiten_seed(connPtr->currentChan);     // set whiten seed
    // A2-multiconn
    ll_hw_set_rx_timeout(88);
    if ( (connPtr->llRfPhyPktFmt == PKT_FMT_BLR125K) ||
         (connPtr->llRfPhyPktFmt == PKT_FMT_BLR500K) )
	    ll_hw_set_rx_timeout(350);
    else
	    ll_hw_set_rx_timeout(88);

    set_max_length(0xff);

    // win size for 1st packet
    if (connPtr->firstPacket)     // not received the 1st packet, CRC error or correct
    {
        //ll_hw_set_rx_timeout_1st(conn_param[connId].curParam.winSize * 625 + conn_param[connId].timerDrift * 2 );
        //20180412 enlarge the connectInd or connect_update timing tolerence
        uint32_t first_window_timout=pGlobal_config[LL_SMART_WINDOW_FIRST_WINDOW] + connPtr->curParam.winSize * 625 + connPtr->timerDrift * 2 ;
        //The transmitWindowOffset shall be a multiple of 1.25 ms in the range of 0 ms
        //to connInterval. The transmitWindowSize shall be a multiple of 1.25 ms in the
        //range of 1.25 ms to the lesser of 10 ms and (connInterval - 1.25 ms).
        //ZQ 20200208
        uint32_t winSizeLimt = MIN(10000, (connPtr->curParam.connInterval * 625 - 1250) );
        
        if (winSizeLimt < first_window_timout)
            first_window_timout = winSizeLimt;
        
        ll_hw_set_rx_timeout_1st(first_window_timout);        
    }
    else                  // A1 ROM metal change , 2018 - 1 - 3
    {
        if (connPtr->rx_timeout)    // timeout case
            ll_hw_set_rx_timeout_1st(pGlobal_config[LL_HW_RTLP_1ST_TIMEOUT] + pGlobal_config[SLAVE_CONN_DELAY_BEFORE_SYNC] * 2 + (connPtr->timerDrift + connPtr->accuTimerDrift) * 2 );
        else
            ll_hw_set_rx_timeout_1st(pGlobal_config[LL_HW_RTLP_1ST_TIMEOUT] + pGlobal_config[SLAVE_CONN_DELAY] * 2 + connPtr->timerDrift * 2 );
    }
    
    // configure loop timeout
    // considering the dle case
    uint32_t temp = connPtr->curParam.connInterval * 625 - connPtr->llPduLen.local.MaxRxTime- pGlobal_config[LL_HW_RTLP_TO_GAP];       // 500us: margin for timer1 IRQ
    ll_hw_set_loop_timeout(temp > pGlobal_config[LL_HW_RTLP_LOOP_TIMEOUT] ?
                           pGlobal_config[LL_HW_RTLP_LOOP_TIMEOUT] : temp);         // 2018-6-20, global config for the parameter
    
    // now we support 4 RT in one RTLP, if PDU size is 256Byte, need (256*8 + 150) * 8 = 17684us,
    // not consider Rx packet size
    ll_hw_trx_settle_config(connPtr->llRfPhyPktFmt);
    ll_hw_set_loop_nack_num(4);
    ll_hw_ign_rfifo(LL_HW_IGN_ALL);
    tx_num = ll_generateTxBuffer(tx_num, &ll_rdCntIni);
    // TODO: consider Rx flow control here
//    if (LL_RX_FLOW_CONTROL_ENABLED == rxFifoFlowCtrl)
//    {
//        // configure LL HW to keep NESN
//    }
    ll_hw_config( LL_HW_RTLP,  //connPtr->llMode,
                  connPtr->sn_nesn,   // sn,nesn init
                  tx_num,                    // ll_txNum
                  rx_num,                    // ll_rxNum
                  1,                         // ll_mdRx
                  ll_rdCntIni);              // rdCntIni
    
    uint8 temp_rf_fmt = g_rfPhyPktFmt;
    g_rfPhyPktFmt = connPtr->llRfPhyPktFmt;
    ll_hw_go();
    llWaitingIrq = TRUE;
    g_rfPhyPktFmt = temp_rf_fmt;
    HAL_EXIT_CRITICAL_SECTION();
// hal_gpio_write(GPIO_P14, 0);
//    LOG("%d-%d ", g_ll_conn_ctx.numLLConns, g_ll_conn_ctx.currentConn);
//    LOG("%d ", g_ll_conn_ctx.currentConn);
    ll_debug_output(DEBUG_LL_HW_SET_RTLP);
}


void llSetupAuxAdvIndPDU1(extAdvInfo_t * pAdvInfo, periodicAdvInfo_t * pPrdAdv)

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
    // 
    SET_BITS(g_tx_ext_adv_buf.txheader, pAdvInfo->parameter.ownAddrType, TX_ADD_SHIFT, TX_ADD_MASK);

    if (pGlobal_config[LL_SWITCH] & CONN_CSA2_ALLOW)
        SET_BITS(g_tx_adv_buf.txheader, 1, CHSEL_SHIFT, CHSEL_MASK);

    extHdrLength = 0;
    // == step 1. decide what fields should be present in extended header
    // extended header
    extHeaderFlag = 0;
    extHdrLength ++;

    // CTEInfo(1 octets), not present for AUX_ADV_IND

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
                && (pAdvInfo->data.advertisingDataLength > (255 - 1 - extHdrLength)))
        {
            extHeaderFlag |= LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK;
            extHdrLength += 3;
            // maximum payload length = 255: header len (= 1), ext header len(extHdrLength), adv data(advDataLen)
            advDataLen = (255 - 1) - extHdrLength;       // adv data length. TODO: check spec
        }
        else
        {
			// put all adv data in field "Adv Data"
            advDataLen = (255 - 1) - extHdrLength;
            if (advDataLen > pAdvInfo->data.advertisingDataLength)
               advDataLen = pAdvInfo->data.advertisingDataLength;
        }    
    }
    else         // update 04-13, consider update adv data case
        advDataLen = 0;
    
    length = 1 + extHdrLength + advDataLen;    // 1: extended header len(6bits) + advMode(2bit)
    // Length
    SET_BITS(g_tx_ext_adv_buf.txheader, length, LENGTH_SHIFT, LENGTH_MASK);
    
    SET_BITS(g_tx_ext_adv_buf.txheader, pAdvInfo->parameter.ownAddrType, TX_ADD_SHIFT, TX_ADD_MASK);
    
    // === step 2.  fill AUX_ADV_IND PDU
    // Extended header length + AdvMode(1 octet)
    g_tx_ext_adv_buf.data[offset] = ((advMode & 0x3) << 6) | (extHdrLength & 0x3F);
    offset ++;
    g_tx_ext_adv_buf.data[offset] = extHeaderFlag;
    offset ++;
    
    // AdvA (6 octets)
    if (extHeaderFlag & LE_EXT_HDR_ADVA_PRESENT_BITMASK)
    {
        if ( (pAdvInfo->parameter.ownAddrType == LL_DEV_ADDR_TYPE_RANDOM) && 
        	 (pAdvInfo->parameter.isOwnRandomAddressSet == TRUE) )
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
        int16  radio_pwr;
        radio_pwr = pAdvInfo->tx_power * 10 + g_rfTxPathCompensation;

        if (radio_pwr > 1270)   radio_pwr = 1270;

        else if (radio_pwr < -1270)  radio_pwr = -1270;

        g_tx_ext_adv_buf.data[offset] = (uint8)(radio_pwr / 10);
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


void llSetupAuxChainIndPDU1(extAdvInfo_t * pAdvInfo, periodicAdvInfo_t * pPrdAdv)

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
        if ( extscanrsp_offset && 
        	(extscanrsp_offset < pAdvInfo->scanRspMaxLength) )
			advDataLen = pAdvInfo->scanRspMaxLength - extscanrsp_offset;
		else
	        if (pAdvInfo->data.dataComplete == TRUE)
                advDataLen = pAdvInfo->data.advertisingDataLength - pAdvInfo->currentAdvOffset;  // put all remain adv data in field "Adv Data"
		else            
			// update 04-13, adv data may be reconfigured during advertising, include no data in such case
        	advDataLen = 0;
    }
    else
    {
        if (pAdvInfo->data.dataComplete == TRUE)
            advDataLen = pAdvInfo->data.advertisingDataLength - pAdvInfo->currentAdvOffset;  // put all remain adv data in field "Adv Data"
		else            
			// update 04-13, adv data may be reconfigured during advertising, include no data in such case
        	advDataLen = 0;
	}
	// TODO Check this condition more precise
	if ( advDataLen > (255 - 1 - extHdrLength) )
	{
		extHeaderFlag |= LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK;
        extHdrLength += 3;
        // maximum payload length = 255, header len = 1, ADI len = 2, AUX_PTR len = 2
        advDataLen = 255 - 1 - extHdrLength;;
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
    
    // ACAD(varies), not present

    // copy adv data
    if (pAdvInfo->isPeriodic == 0) {

        if (!extscanrsp_offset || (extscanrsp_offset > pAdvInfo->scanRspMaxLength))
        {
            memcpy(&g_tx_ext_adv_buf.data[offset], pAdvInfo->data.advertisingData + pAdvInfo->currentAdvOffset, advDataLen);
            pAdvInfo->currentAdvOffset = pAdvInfo->currentAdvOffset + advDataLen;
        }
        else
        {
            memcpy(&g_tx_ext_adv_buf.data[offset], pAdvInfo->scanRspData + extscanrsp_offset, advDataLen);
            if (pAdvInfo->scanRspMaxLength == (extscanrsp_offset + advDataLen))
                extscanrsp_offset = 0;
            else            
	            extscanrsp_offset += advDataLen;
        }
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

void llSetupAuxScanRspPDU1(extAdvInfo_t * pAdvInfo)

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
	if ((pAdvInfo->parameter.ownAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC) ||
		(pAdvInfo->parameter.ownAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM))
	{
		if ((adv_param.ownAddr[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR)
            SET_BITS(g_tx_adv_buf.txheader, LL_DEV_ADDR_TYPE_RANDOM, TX_ADD_SHIFT, TX_ADD_MASK);
	}
	else // This will cutoff ownAddrType high bit
        SET_BITS(g_tx_adv_buf.txheader, pAdvInfo->parameter.ownAddrType, TX_ADD_SHIFT, TX_ADD_MASK);

    extHdrLength = 0;
    // extended header
    extHeaderFlag = 0;          // for AUX_SYNC_IND PDU: CTE info, AuxPtr, Tx power, ACAD, Adv Data are optional, other fields are absent
    extHdrLength ++;

    extHeaderFlag |= LE_EXT_HDR_ADVA_PRESENT_BITMASK;
    extHdrLength += 6;
    
    extHeaderFlag |= LE_EXT_HDR_ADI_PRESENT_BITMASK;
    extHdrLength += 2;
    
    advDataLen = pAdvInfo->scanRspMaxLength - extscanrsp_offset;
    if (advDataLen > 100)
    {
        extHeaderFlag |= LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK;
        extHdrLength += 3;
        advDataLen = 100;
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
    
    // AdvA (6 octets)
    if (extHeaderFlag & LE_EXT_HDR_ADVA_PRESENT_BITMASK)
    {
        if ( (g_currentLocalAddrType == LL_DEV_ADDR_TYPE_RANDOM) &&
        	 (pAdvInfo->parameter.isOwnRandomAddressSet == TRUE) )
            memcpy(&g_tx_ext_adv_buf.data[offset], pAdvInfo->parameter.ownRandomAddress, LL_DEVICE_ADDR_LEN);
        else    // public address
            memcpy(&g_tx_ext_adv_buf.data[offset], adv_param.ownAddr, LL_DEVICE_ADDR_LEN);

        offset += LL_DEVICE_ADDR_LEN;
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
        offset += 3;
    }
    
    // copy adv data
    memcpy(&g_tx_ext_adv_buf.data[offset], pAdvInfo->scanRspData + extscanrsp_offset, advDataLen);
    
    if (pAdvInfo->scanRspMaxLength == (extscanrsp_offset + advDataLen))
        extscanrsp_offset = 0;
    else
	    extscanrsp_offset += advDataLen;
}

void ll_hw_trx_settle_bb(uint8_t phyPktFmt, uint8_t delay)

{
    if (phyPktFmt == PKT_FMT_BLE1M)
        ll_hw_set_trx_settle(delay, pGlobal_config[LL_HW_AFE_DELAY], pGlobal_config[LL_HW_PLL_DELAY]);
	else if (phyPktFmt == PKT_FMT_BLE1M)
        ll_hw_set_trx_settle(delay, pGlobal_config[LL_HW_AFE_DELAY_2MPHY], pGlobal_config[LL_HW_PLL_DELAY_2MPHY]);
	else if (phyPktFmt == PKT_FMT_BLR500K)
        ll_hw_set_trx_settle(delay, pGlobal_config[LL_HW_AFE_DELAY_500KPHY], pGlobal_config[LL_HW_PLL_DELAY_500KPHY]);
	else // (phyPktFmt == PKT_FMT_BLR500K)
        ll_hw_set_trx_settle(delay, pGlobal_config[LL_HW_AFE_DELAY_125KPHY], pGlobal_config[LL_HW_PLL_DELAY_125KPHY]);

}

int8 ll_processExtAdvIRQ1(uint32_t irq_status)

{
    uint8         mode;
    extAdvInfo_t*  pAdvInfo;
    uint32_t      T2, delay;
    pAdvInfo = g_pAdvSchInfo[g_currentExtAdv].pAdvInfo;

    if (pAdvInfo == NULL)
        return FALSE;

    HAL_ENTER_CRITICAL_SECTION();
    mode = ll_hw_get_tr_mode();
    
    if (ll_isLegacyAdv(pAdvInfo))
    {
        // process legacy Adv
        uint8_t  packet_len, pdu_type, txAdd;
        uint8_t*  peerAddr, *ownAddr;
        uint8_t  bWlRlCheckOk = TRUE;
        uint16_t pktLen;
        uint32_t pktFoot0, pktFoot1;
        int      calibra_time;                 // this parameter will be provided by global_config
        ll_debug_output(DEBUG_LL_HW_TRX);
        // read packet
        packet_len = ll_hw_read_rfifo1((uint8_t*)(&(g_rx_adv_buf.rxheader)),
                                       &pktLen,
                                       &pktFoot0,
                                       &pktFoot1);

        if (ll_hw_get_rfifo_depth() > 0)
        {
            g_pmCounters.ll_rfifo_read_err++;
            packet_len = 0;
            pktLen     = 0;
        }

        // check receive pdu type
        pdu_type = g_rx_adv_buf.rxheader & PDU_TYPE_MASK;
        txAdd    = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random

        if ( (pAdvInfo->parameter.ownAddrType == LL_DEV_ADDR_TYPE_RANDOM)
          && (pAdvInfo->parameter.isOwnRandomAddressSet == TRUE) )
            ownAddr = pAdvInfo->parameter.ownRandomAddress;
        else
            ownAddr = ownPublicAddr;

        if ( (packet_len > 0)                       // any better checking rule for rx anything?
          && (pdu_type == ADV_SCAN_REQ)
          && ((pAdvInfo->parameter.advEventProperties == LL_EXT_ADV_PROP_ADV_IND)
            ||(pAdvInfo->parameter.advEventProperties == LL_EXT_ADV_PROP_ADV_SCAN_IND)) )
        {
            // 1. scan req
            g_pmCounters.ll_recv_scan_req_cnt ++;

            // check AdvA
            if ((g_rx_adv_buf.data[6]  != ownAddr[0])
             || (g_rx_adv_buf.data[7]  != ownAddr[1])
             || (g_rx_adv_buf.data[8]  != ownAddr[2])
             || (g_rx_adv_buf.data[9]  != ownAddr[3])
             || (g_rx_adv_buf.data[10] != ownAddr[4])
             || (g_rx_adv_buf.data[11] != ownAddr[5]))
            {
            }
            else
            {
                uint8_t  rpaListIndex;
                peerAddr = &g_rx_adv_buf.data[0];      // ScanA

                // Resolving list checking
                if ( (g_llRlEnable == TRUE)
                  && (txAdd == LL_DEV_ADDR_TYPE_RANDOM)
                  && ((g_rx_adv_buf.data[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR) )
                {
                    rpaListIndex = ll_getRPAListEntry(&g_rx_adv_buf.data[0]);

                    if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                    {
                        peerAddr = &g_llResolvinglist[rpaListIndex].peerAddr[0];
                    }
                    else
                        bWlRlCheckOk = FALSE;
                }

                // check white list
                if ( (pGlobal_config[LL_SWITCH] & LL_WHITELIST_ALLOW)
                  && ((adv_param.wlPolicy  == LL_ADV_WL_POLICY_WL_SCAN_REQ)
                   || (adv_param.wlPolicy  == LL_ADV_WL_POLICY_WL_ALL_REQ))
                   && (bWlRlCheckOk == TRUE) )
                {
                    // check white list
                    bWlRlCheckOk = ll_isAddrInWhiteList(txAdd, peerAddr);
                }
                
                if (bWlRlCheckOk == FALSE)   // if not in white list, do nothing
                {
                    g_pmCounters.ll_filter_scan_req_cnt ++;
                }
                else
                {
                    g_pmCounters.ll_rx_peer_cnt++;
                    uint8 retScanRspFilter = 1;

                    if(LL_PLUS_ScanRequestFilterCBack)
                    {
                        retScanRspFilter = 1; //LL_PLUS_ScanRequestFilterCBack();
                    }
                    if(retScanRspFilter)
                    {
                        // send scan rsp
                        ll_hw_set_stx();             // set LL HW as single Tx mode
                        g_same_rf_channel_flag = TRUE;
                        // calculate the delay
                        T2 = read_current_fine_time();
                        delay = (T2 > ISR_entry_time) ? (T2 - ISR_entry_time) : (BASE_TIME_UNITS - ISR_entry_time + T2);
                        calibra_time = pGlobal_config[SCAN_RSP_DELAY];            // consider rx_done to ISR time, SW delay after read_current_fine_time(), func read_current_fine_time() delay ...
                        delay = 118 - delay - calibra_time;                       // IFS = 150us, Tx tail->Rx done time: about 32us
                        ll_hw_set_trx_settle(delay,                               // set BB delay, about 80us in 16MHz HCLK
                                             pGlobal_config[LL_HW_AFE_DELAY],
                                             pGlobal_config[LL_HW_PLL_DELAY]);        //RxAFE,PLL
                        ll_hw_go();
                        llWaitingIrq = TRUE;
                        g_same_rf_channel_flag = FALSE;
                        // reset Rx/Tx FIFO
                        ll_hw_rst_rfifo();
                        ll_hw_rst_tfifo();
                        //write Tx FIFO
                        ll_hw_write_tfifo((uint8*)&(tx_scanRsp_desc.txheader),
                                          ((tx_scanRsp_desc.txheader & 0xff00) >> 8) + 2);   // payload length + header length(2)
                        ll_debug_output(DEBUG_LL_HW_SET_STX);
                        g_pmCounters.ll_send_scan_rsp_cnt ++;
	                }
                }
            }
        }
        else if (pdu_type == ADV_CONN_REQ
                 && (pAdvInfo->parameter.advEventProperties == LL_EXT_ADV_PROP_ADV_IND
                     || pAdvInfo->parameter.advEventProperties == LL_EXT_ADV_PROP_ADV_LDC_ADV
                     || pAdvInfo->parameter.advEventProperties == LL_EXT_ADV_PROP_ADV_HDC_ADV))
        {
            // 2. connect req
            g_pmCounters.ll_recv_conn_req_cnt ++;

            // check AdvA
            if ((g_rx_adv_buf.data[6]  != ownAddr[0])
             || (g_rx_adv_buf.data[7]  != ownAddr[1])
             || (g_rx_adv_buf.data[8]  != ownAddr[2])
             || (g_rx_adv_buf.data[9]  != ownAddr[3])
             || (g_rx_adv_buf.data[10] != ownAddr[4])
             || (g_rx_adv_buf.data[11] != ownAddr[5]))
            {
                // nothing to do
            }
            else
            {
                uint8_t  rpaListIndex;
                peerAddr = &g_rx_adv_buf.data[0];        // initA

                // Resolving list checking
                if ( (g_llRlEnable == TRUE)
                  && (txAdd == LL_DEV_ADDR_TYPE_RANDOM)
                  && ((g_rx_adv_buf.data[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR) )
                {
                    rpaListIndex = ll_getRPAListEntry(&g_rx_adv_buf.data[0]);

                    if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                    {
                        peerAddr = &g_llResolvinglist[rpaListIndex].peerAddr[0];
                    }
                    else
                        bWlRlCheckOk = FALSE;
                }
                
                // check white list
                if ( (pGlobal_config[LL_SWITCH] & LL_WHITELIST_ALLOW)
                  && (llState == LL_STATE_ADV_UNDIRECTED)
                  && ((adv_param.wlPolicy == LL_ADV_WL_POLICY_WL_CONNECT_REQ)
                   || (adv_param.wlPolicy  == LL_ADV_WL_POLICY_WL_ALL_REQ))
                  && (bWlRlCheckOk == TRUE) )
                {
                    // check white list
                    bWlRlCheckOk = ll_isAddrInWhiteList(txAdd, peerAddr);
                }
                
                // fixed bug 2018-09-25, LL/CON/ADV/BV-04-C, for direct adv, initA should equal peer Addr
                if (llState == LL_STATE_ADV_DIRECTED)
                {
                    if ( (txAdd != peerInfo.peerAddrType)
			          || (peerAddr[0]  != peerInfo.peerAddr[0])
			          || (peerAddr[1]  != peerInfo.peerAddr[1])
			          || (peerAddr[2]  != peerInfo.peerAddr[2])
			          || (peerAddr[3]  != peerInfo.peerAddr[3])
			          || (peerAddr[4]  != peerInfo.peerAddr[4])
			          || (peerAddr[5]  != peerInfo.peerAddr[5]) )
                    {
                        // not match, check next
                        bWlRlCheckOk = FALSE;
                    }
                }    

                if (bWlRlCheckOk == FALSE)   // if not in white list, do nothing
                {
                    g_pmCounters.ll_filter_conn_req_cnt ++;
                }
                else
                {
                    // increment statistics counter
                    g_pmCounters.ll_rx_peer_cnt++;
                    // bug fixed 2018-01-23, peerAddrType should read TxAdd
                    peerInfo.peerAddrType = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random
                    osal_memcpy( peerInfo.peerAddr, g_rx_adv_buf.data, LL_DEVICE_ADDR_LEN);
                    move_to_slave_function();    // move to slave role for connection state
                    // add 04-01, set adv inactive, and it will be remove from the scheduler when invoke ll_adv_scheduler()
                    pAdvInfo->active = FALSE;
                    LL_AdvSetTerminatedCback(LL_STATUS_SUCCESS,
                                             pAdvInfo->advHandle,
                                             adv_param.connId,
                                             pAdvInfo->adv_event_counter);
                }
            }
        }
    }
    else if (mode == LL_HW_MODE_TRX  &&
             (irq_status & LIRQ_COK))
    {
        // TRX mode, receives AUX_SCAN_REQ or AUX_CONNECT_REQ
        uint8_t  packet_len, pdu_type, txAdd;
        uint8_t* peerAddr;
        uint8_t  bWlRlCheckOk = TRUE;
        uint16_t pktLen;
        uint32_t pktFoot0, pktFoot1;
        int      calibra_time;                 // this parameter will be provided by global_config
        uint8*   ownAddr;

        ll_debug_output(DEBUG_LL_HW_TRX);
        // read packet
        packet_len = ll_hw_read_rfifo1((uint8_t*)(&(g_rx_adv_buf.rxheader)),
                                       &pktLen,
                                       &pktFoot0,
                                       &pktFoot1);

        if(ll_hw_get_rfifo_depth() > 0)
        {
            g_pmCounters.ll_rfifo_read_err++;
            packet_len = 0;
            pktLen     = 0;
        }

        // check receive pdu type
        pdu_type = g_rx_adv_buf.rxheader & PDU_TYPE_MASK;
        txAdd    = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random

        if ( (pAdvInfo->parameter.ownAddrType == LL_DEV_ADDR_TYPE_RANDOM)
         &&  (pAdvInfo->parameter.isOwnRandomAddressSet == TRUE) )
            ownAddr = pAdvInfo->parameter.ownRandomAddress;
        else if (g_currentLocalAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM)
            ownAddr = g_currentLocalRpa;
        else
            ownAddr = ownPublicAddr;

        if ( (packet_len > 0)
          && (pdu_type == ADV_AUX_SCAN_REQ)
          && ((pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_CONN_BITMASK)
           || (pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_SCAN_BITMASK)) )
        {
            // 1. scan req
            g_pmCounters.ll_recv_scan_req_cnt ++;

            // check AdvA
            if ( (g_rx_adv_buf.data[6]  != ownAddr[0])
              || (g_rx_adv_buf.data[7]  != ownAddr[1])
              || (g_rx_adv_buf.data[8]  != ownAddr[2])
              || (g_rx_adv_buf.data[9]  != ownAddr[3])
              || (g_rx_adv_buf.data[10] != ownAddr[4])
              || (g_rx_adv_buf.data[11] != ownAddr[5]) )
            {
            }
            else
            {
                uint8_t  rpaListIndex;
                peerAddr = &g_rx_adv_buf.data[0];      // ScanA

                // Resolving list checking
                if ( (g_llRlEnable == TRUE)
                 &&  (txAdd == LL_DEV_ADDR_TYPE_RANDOM)
                 &&  ((g_rx_adv_buf.data[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR) )
                {
                    rpaListIndex = ll_getRPAListEntry(&g_rx_adv_buf.data[0]);

                    if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                    {
                        peerAddr = &g_llResolvinglist[rpaListIndex].peerAddr[0];
                    }
                    else
                        bWlRlCheckOk = FALSE;
                }

                // check white list
                if ( (pGlobal_config[LL_SWITCH] & LL_WHITELIST_ALLOW)
                  && ((adv_param.wlPolicy  == LL_ADV_WL_POLICY_WL_SCAN_REQ)
                   || (adv_param.wlPolicy  == LL_ADV_WL_POLICY_WL_ALL_REQ))
                  && (bWlRlCheckOk == TRUE) )
                {
                    // check white list
                    bWlRlCheckOk = ll_isAddrInWhiteList(txAdd, peerAddr);
                }

                if (bWlRlCheckOk == FALSE)   // if not in white list, do nothing
                {
                    g_pmCounters.ll_filter_scan_req_cnt ++;
                }
                else
                {
                    g_pmCounters.ll_rx_peer_cnt++;
                    llSetupAuxScanRspPDU(pAdvInfo);
                    // send scan rsp
                    ll_hw_set_stx();             // set LL HW as single Tx mode
                    g_same_rf_channel_flag = TRUE;
                    // calculate the delay
                    T2 = read_current_fine_time();
                    delay = (T2 > ISR_entry_time) ? (T2 - ISR_entry_time) : (BASE_TIME_UNITS - ISR_entry_time + T2);
                    
                    if (g_rfPhyPktFmt == PKT_FMT_BLE1M)
                    {
                    	calibra_time = pGlobal_config[EXT_ADV_AUXSCANRSP_DELAY_1MPHY];
                    	delay = 118 - (delay + calibra_time); // IFS = 150us, Tx tail->Rx done time: about 32us
                    }
                    else if (g_rfPhyPktFmt == PKT_FMT_BLE2M)
                    {
                    	calibra_time = pGlobal_config[EXT_ADV_AUXSCANRSP_DELAY_2MPHY];
   	                    delay = 132 - (delay + calibra_time);
                    }
                    else
                    { // PKT_FMT_BLR125K
                    	calibra_time = pGlobal_config[EXT_ADV_AUXSCANRSP_DELAY_125KPHY];
                    	delay = 118 - (delay + calibra_time);
                    }
		            ll_hw_trx_settle_bb(g_rfPhyPktFmt, delay);
                    ll_hw_go();
                    llWaitingIrq = TRUE;
                    g_same_rf_channel_flag = FALSE;
                    // reset Rx/Tx FIFO
                    ll_hw_rst_rfifo();
                    ll_hw_rst_tfifo();
                    //write Tx FIFO
                    // =================== TODO: change the buffer to ext adv set
                    ll_hw_write_tfifo((uint8*)&(g_tx_ext_adv_buf.txheader),
                                      ((g_tx_ext_adv_buf.txheader & 0xff00) >> 8) + 2);   // payload length + header length(2)
                    ll_debug_output(DEBUG_LL_HW_SET_STX);
                    g_pmCounters.ll_send_scan_rsp_cnt ++;
                }
            }
        }
        else if ( (pdu_type == ADV_CONN_REQ)
               && (pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_CONN_BITMASK) )
        {
            // 2. connect req
	        // ??? g_auxconnreq_ISR_entry_time = ISR_entry_time;
            g_pmCounters.ll_recv_conn_req_cnt ++;

            // check AdvA
            if (g_rx_adv_buf.data[6]  != ownAddr[0]
                    || g_rx_adv_buf.data[7]  != ownAddr[1]
                    || g_rx_adv_buf.data[8]  != ownAddr[2]
                    || g_rx_adv_buf.data[9]  != ownAddr[3]
                    || g_rx_adv_buf.data[10] != ownAddr[4]
                    || g_rx_adv_buf.data[11] != ownAddr[5])
            {
                // nothing to do
            }
            else
            {
                uint8_t  rpaListIndex;
                peerAddr = &g_rx_adv_buf.data[0];      // ScanA

                // Resolving list checking
                if ( (g_llRlEnable == TRUE)
                 &&  (txAdd == LL_DEV_ADDR_TYPE_RANDOM)
                 &&  ((g_rx_adv_buf.data[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR) )
                {
                    rpaListIndex = ll_getRPAListEntry(&g_rx_adv_buf.data[0]);

                    if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                    {
                        peerAddr = &g_llResolvinglist[rpaListIndex].peerAddr[0];
                    }
                    else
                        bWlRlCheckOk = FALSE;
                }

                // check white list
                if ( (pGlobal_config[LL_SWITCH] & LL_WHITELIST_ALLOW)
                  && ((adv_param.wlPolicy  == LL_ADV_WL_POLICY_WL_SCAN_REQ)
                   || (adv_param.wlPolicy  == LL_ADV_WL_POLICY_WL_ALL_REQ))
                  && (bWlRlCheckOk == TRUE) )
                {
                    // check white list
                    bWlRlCheckOk = ll_isAddrInWhiteList(txAdd, peerAddr);
                }

                if (bWlRlCheckOk == FALSE)   // if not in white list, do nothing
                {
                    g_pmCounters.ll_filter_conn_req_cnt ++;
                }
                else
                {
                    // increment statistics counter
                    g_pmCounters.ll_rx_peer_cnt++;
//==============
                    llSetupAuxConnectRspPDU(pAdvInfo);
			        if (llWaitingIrq == FALSE)
			        {
		                // send scan rsp
		                ll_hw_set_stx();             // set LL HW as single Tx mode
		                g_same_rf_channel_flag = TRUE;
		                // calculate the delay
		                T2 = read_current_fine_time();
		                delay = (T2 > ISR_entry_time) ? (T2 - ISR_entry_time) : (BASE_TIME_UNITS - ISR_entry_time + T2);

		                if (g_rfPhyPktFmt == PKT_FMT_BLE1M)
		                {
		                	calibra_time = pGlobal_config[EXT_ADV_AUXCONNRSP_DELAY_1MPHY];
		                	delay = 118 - (delay + calibra_time); // IFS = 150us, Tx tail->Rx done time: about 32us
		                }
		                else if (g_rfPhyPktFmt == PKT_FMT_BLE2M)
		                {
		                	calibra_time = pGlobal_config[EXT_ADV_AUXCONNRSP_DELAY_2MPHY];
	   	                    delay = 132 - (delay + calibra_time);
		                }
		                else
		                { // PKT_FMT_BLR125K
		                	calibra_time = pGlobal_config[EXT_ADV_AUXCONNRSP_DELAY_125KPHY];
		                	delay = 118 - (delay + calibra_time);
		                }
						ll_hw_trx_settle_bb(g_rfPhyPktFmt, delay);
						ll_hw_go();
						llWaitingIrq = TRUE;
		                g_same_rf_channel_flag = FALSE;
		                // reset Rx/Tx FIFO
		                ll_hw_rst_rfifo();
		                ll_hw_rst_tfifo();
		                //write Tx FIFO
		                ll_hw_write_tfifo((uint8*)&(g_tx_adv_buf.txheader),
		                                  ((g_tx_adv_buf.txheader & 0xff00) >> 8) + 2);   // payload length + header length(2)
		                ll_debug_output(DEBUG_LL_HW_SET_STX);
		                g_pmCounters.ll_send_conn_rsp_cnt ++;
//==============
		                // bug fixed 2018-01-23, peerAddrType should read TxAdd
		                peerInfo.peerAddrType = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random
		                osal_memcpy( peerInfo.peerAddr, g_rx_adv_buf.data, LL_DEVICE_ADDR_LEN);
		                move_to_slave_function3();    // move to slave role for connection state
		                // add 04-01, set adv inactive, and it will be remove from the scheduler when invoke ll_adv_scheduler()
		                pAdvInfo->active = FALSE;
		                LL_AdvSetTerminatedCback(LL_STATUS_SUCCESS,
		                                         pAdvInfo->advHandle,
		                                         adv_param.connId,
		                                         pAdvInfo->adv_event_counter);
		            }
	            }
    	    }
        }
    }
    else if (mode == LL_HW_MODE_STX )
    {
    }
		                
//  // update scheduler list
//  ll_adv_scheduler();

    if (!llWaitingIrq)
    {
        // update scheduler list  // update 04-01, consider sending aux_scan_rsp/aux_conn_rsp case, will invoke scheduler after STX IRQ
        ll_adv_scheduler();
        ll_hw_clr_irq();
        llTaskState = LL_TASK_OTHERS;
    }

    HAL_EXIT_CRITICAL_SECTION();
    return TRUE;
}

// HCI Event mask - octet index
#define HCI_EVT_INDEX_DISCONN_COMPLETE              0
#define HCI_EVT_INDEX_ENCRYPTION_CHANGE             0
#define HCI_EVT_INDEX_READ_REMOTE_VER               1
#define HCI_EVT_INDEX_HW_ERROR                      1
#define HCI_EVT_INDEX_FLUSH_OCCURRED                2
#define HCI_EVT_INDEX_BUF_OVERFLOW                  3
#define HCI_EVT_INDEX_KEY_REFRESH_COMPLETE          5
#define HCI_EVT_INDEX_LE                            7

// HCI Event mask - octet mask
#define HCI_EVT_MASK_DISCONN_COMPLETE               0x10
#define HCI_EVT_MASK_ENCRYPTION_CHANGE              0x80
#define HCI_EVT_MASK_READ_REMOTE_VER                0x08
#define HCI_EVT_MASK_HW_ERROR                       0x80
#define HCI_EVT_MASK_FLUSH_OCCURRED                 0x01
#define HCI_EVT_MASK_BUF_OVERFLOW                   0x02
#define HCI_EVT_MASK_KEY_REFRESH_COMPLETE           0x80
#define HCI_EVT_MASK_LE                             0x20


// SDK 3.1.3

#define LL_EXT_ADV_RPT_SCAN_DATA  0x00000010
#define LL_EXT_ADV_RPT_MORE_FLAG  0x00000020
#define LL_EXT_ADV_RPT_NO_DATA    0x00000040

#define LE_EVT_MASK_EXT_ADV_REPORT 0x001000

#define HCI_BLE_SCAN_TIMEOUT_EVENT                     0x11



typedef struct
{
    uint16  advEvt;
    uint8   advAddrType;
    uint8   advAddr[B_ADDR_LEN];
    uint8   primaryPHY;
    uint8   secondaryPHY;
    uint8   advertisingSID;
    uint8   txPower;
    int8    rssi;
    uint16  periodicAdvertisingInterval;
    uint8   directAddrType;
    uint8   directAddr[B_ADDR_LEN];
    uint16  dataLen;
    uint8   rptData[0];
} gapExtScan_AdvRpt_t;

typedef struct
{
    uint8  hciPktType;
    uint8  hciEvtCode;
    uint8  dataLength;
    uint8  evtCode;
    uint8  numRpts;
    gapExtScan_AdvRpt_t   advrpt;
} hciGapExtScan_AdvInfo_t;

// LE Advertising Set Terminated event
typedef struct
{
    uint8             status;
    uint8             adv_handle;
    uint16            connHandle;                      // connection handle
    uint8             Num_Completed_Extended_Advertising_Events;
} gapAdvSetTerminated_t;

typedef struct
{
    uint8  hciPktType;
    uint8  hciEvtCode;
    uint8  dataLength;
    uint8  evtCode;
    gapAdvSetTerminated_t advtrm;
} hciGapAdvSetTerminated_t;



void LL_ExtAdvReportCback0
    (uint8 advEvt, uint8 advAddrType, uint8 * advAddr, uint8 primaryPHY, uint8 secondaryPHY,
        uint8 advertisingSID, uint8 txPower, int8 rssi, uint8 periodicAdvertisingInterval,
        uint8 directAddrType, uint8 * directAddr, uint8 dataLen, uint8 * rptData)

{
    if (hciGapTaskID)
    {
        hciEvt_BLEExtAdvPktReport_t* pkt;
        hciEvt_ExtAdvRptInfo_t*      rptInfo;
        uint8 x;
//    if (dataLen > B_MAX_ADV_LEN)      // guard the memory access, 2018-10-15
//    {
//        return;
//    }
        if ( advAddrType == LL_EXT_ADV_RPT_NO_DATA )
        {
            pkt = (hciEvt_BLEExtAdvPktReport_t* )osal_msg_allocate(
                    sizeof ( hciEvt_BLEExtAdvPktReport_t ) + sizeof ( hciEvt_ExtAdvRptInfo_t ) );
            if ( pkt )
            {
                    
                pkt->hdr.event    = HCI_GAP_EVENT_EVENT;
                pkt->hdr.status   = HCI_LE_EVENT_CODE;
                pkt->BLEEventCode = HCI_BLE_EXT_ADV_REPORT_EVENT;
                pkt->numReports   = 1;  // assume one report each event now
                pkt->rptInfo      = rptInfo = (hciEvt_ExtAdvRptInfo_t *)(pkt + 1);
                
                for (x = 0; x < pkt->numReports; x++, rptInfo++)
                {
                    /* Fill in the device info */
                    rptInfo->eventType = advEvt;
                    rptInfo->addrType = advAddrType;
                    
                    (void)osal_memcpy(rptInfo->addr, advAddr, LL_DEVICE_ADDR_LEN);
                    rptInfo->primaryPHY = primaryPHY;
                    
                    if (secondaryPHY == PKT_FMT_BLR125K)
                        rptInfo->secondaryPHY = 0x03;           //  convert 4 -> 3
                    else
                        rptInfo->secondaryPHY = secondaryPHY;
        
                    rptInfo->advertisingSID = advertisingSID;
                    rptInfo->txPower = txPower;
                    rptInfo->rssi   = rssi;
                    rptInfo->periodicAdvertisingInterval = periodicAdvertisingInterval;
                    rptInfo->directAddrType = directAddrType;
                    rptInfo->dataLen = dataLen;
                }
                (void)osal_msg_send( hciGapTaskID, (uint8*)pkt );
            }
        }
        else
        {
            uint8 len;
            for (int i = 0; i < dataLen; i += len) {
                len = dataLen - i;
                if (len > 229) len = 229;

                //pkt = (hciEvt_BLEExtAdvPktReport_t * ) osal_msg_allocate(262);
                pkt = (hciEvt_BLEExtAdvPktReport_t*)osal_msg_allocate(
                        sizeof ( hciEvt_BLEExtAdvPktReport_t ) + sizeof ( hciEvt_ExtAdvRptInfo_t ) );
                
                if ( pkt )
                {
                    rptInfo = (hciEvt_ExtAdvRptInfo_t *)(pkt + 1);
                    
                    pkt->hdr.event    = HCI_GAP_EVENT_EVENT;
                    pkt->hdr.status   = HCI_LE_EVENT_CODE;
                    pkt->BLEEventCode = HCI_BLE_EXT_ADV_REPORT_EVENT;
                    pkt->numReports   = 1;  // assume one report each event now
                    pkt->rptInfo      = rptInfo = (hciEvt_ExtAdvRptInfo_t *)(pkt + 1);
                
                    for (x = 0; x < pkt->numReports; x++, rptInfo++)
                    {
                        /* Fill in the device info */
                        if ( (dataLen - i) > 229 )
                           rptInfo->eventType = advEvt | LL_EXT_ADV_RPT_MORE_FLAG;
                        else
                            rptInfo->eventType = advEvt;

                        rptInfo->addrType = advAddrType;
                        (void)osal_memcpy(rptInfo->addr, advAddr, LL_DEVICE_ADDR_LEN);
                        rptInfo->primaryPHY = primaryPHY;
                        
                        if (secondaryPHY == PKT_FMT_BLR125K)
                            rptInfo->secondaryPHY = 0x03;           //  convert 4 -> 3
                        else
                            rptInfo->secondaryPHY = secondaryPHY;

                        rptInfo->advertisingSID = advertisingSID;
                        rptInfo->txPower = txPower;
                        rptInfo->rssi   = rssi;
                        rptInfo->periodicAdvertisingInterval = periodicAdvertisingInterval;
                        rptInfo->directAddrType = directAddrType;
        
                        if (advEvt & LE_ADV_PROP_DIRECT_BITMASK)
                        {
                            (void)osal_memcpy( rptInfo->directAddr, directAddr, B_ADDR_LEN );
                        }
        
                        rptInfo->dataLen = len;
                        (void)osal_memcpy( rptInfo->rptData, rptData, len );
                        // rptInfo->rssi = rssi;
                        

                        rptInfo->advertisingSID = advertisingSID;
                    }
                    
                    (void)osal_msg_send( hciGapTaskID, (uint8*)pkt );
                }
            }
        }
    }
    else
    {    
        hciPacket_t* pkt;
        hciGapExtScan_AdvInfo_t *rptInfo;
        uint8 totalLength;
        uint8 dataLength;

        if ( ((pHciEvtMask[HCI_EVT_INDEX_LE] & HCI_EVT_MASK_LE) == 0) 
          || ((bleEvtMask & LE_EVT_MASK_EXT_ADV_REPORT) == 0) ) // LE_EVT_MASK_SCAN_TIMEOUT
            // the event mask is not set for this event
            return;

        // data length 26 + dataLen
        dataLength = sizeof(gapExtScan_AdvRpt_t) + dataLen;

        // OSAL message header + HCI event header + data (8 + 3 + dataLength)
        totalLength = sizeof (hciPacket_t) + HCI_EVENT_MIN_LENGTH + dataLength;
        pkt = (hciPacket_t*)osal_msg_allocate(totalLength);

        if (pkt)
        {
            // message type, length
            pkt->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
            pkt->hdr.status = 0xFF;
            // create message
            rptInfo = (hciGapExtScan_AdvInfo_t *)(pkt + 1);
            pkt->pData = (uint8 *)rptInfo; 

            rptInfo->hciPktType = HCI_EVENT_PACKET;       
            rptInfo->hciEvtCode = HCI_LE_EVENT_CODE;
        
            rptInfo->dataLength = sizeof(gapExtScan_AdvRpt_t) + dataLen;
            rptInfo->evtCode = HCI_BLE_EXT_ADV_REPORT_EVENT;
            rptInfo->numRpts = 1;
            rptInfo->advrpt.advEvt  = advEvt;
            rptInfo->advrpt.advAddrType = advAddrType;
            osal_memcpy(rptInfo->advrpt.advAddr, advAddr, 6);
            rptInfo->advrpt.primaryPHY = primaryPHY;
            rptInfo->advrpt.secondaryPHY = secondaryPHY;
            rptInfo->advrpt.advertisingSID = advertisingSID;
            rptInfo->advrpt.txPower = txPower;
            rptInfo->advrpt.rssi = rssi;
            rptInfo->advrpt.periodicAdvertisingInterval = periodicAdvertisingInterval;
            rptInfo->advrpt.directAddrType = directAddrType;
            if (advEvt & LE_ADV_PROP_DIRECT_BITMASK)
                (void)osal_memcpy (rptInfo->advrpt.directAddr, directAddr, B_ADDR_LEN); // address
            else
                (void)osal_memset(rptInfo->advrpt.directAddr, 0, B_ADDR_LEN);
        
            rptInfo->advrpt.dataLen = dataLen;
            osal_memcpy(rptInfo->advrpt.rptData, rptData, dataLen);
            osal_msg_send( hciTaskID, (uint8 * )pkt);
        }
    }
}

void LL_AdvSetTerminatedCback(uint8          status,
                              uint8   adv_handle,
                              uint16  connHandle,
                              uint8   Num_Completed_Extended_Advertising_Events)
{
    // check if this is for the Host
    if ( hciGapTaskID != 0 )
    {
        hciEvt_AdvSetTerminated_t* pkt;
        pkt = (hciEvt_AdvSetTerminated_t*)osal_msg_allocate(sizeof(hciEvt_AdvSetTerminated_t ));

        if ( pkt )
        {
            pkt->hdr.event    = HCI_GAP_EVENT_EVENT;
            pkt->hdr.status   = HCI_LE_EVENT_CODE;
            pkt->BLEEventCode = HCI_LE_ADVERTISING_SET_TERMINATED;
            pkt->status       = status;
            pkt->adv_handle   = adv_handle;
            pkt->connHandle   = connHandle;
            pkt->Num_Completed_Extended_Advertising_Events = Num_Completed_Extended_Advertising_Events;
            (void)osal_msg_send( hciGapTaskID, (uint8*)pkt );
        }
    }
    else
    {
        hciPacket_t* msg;
        hciGapAdvSetTerminated_t *pkt;
        uint8 totalLength;
        uint8 dataLength;
        // data length
        dataLength = sizeof(hciGapAdvSetTerminated_t); // HCI_ADV_SET_TERM_EVENT_LEN;
        // OSAL message header + HCI event header + data
        totalLength = sizeof (hciPacket_t) + HCI_EVENT_MIN_LENGTH + dataLength;
        msg = (hciPacket_t*)osal_msg_allocate(totalLength);

        if (msg)
        {
            // message type, length
            msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
            msg->hdr.status = 0xFF;
            // create message
            pkt = (hciGapAdvSetTerminated_t *)(msg + 1);
            msg->pData = (uint8 *)pkt; 

            pkt->hciPktType = HCI_EVENT_PACKET;       
            pkt->hciEvtCode = HCI_LE_EVENT_CODE;
        
            pkt->dataLength = sizeof(gapAdvSetTerminated_t);
            pkt->evtCode = HCI_LE_ADVERTISING_SET_TERMINATED;
            // populate event
            pkt->advtrm.status     = status;
            pkt->advtrm.adv_handle = adv_handle;
            pkt->advtrm.connHandle = connHandle; // connection handle
            pkt->advtrm.Num_Completed_Extended_Advertising_Events = Num_Completed_Extended_Advertising_Events;
            // send the message
            (void)osal_msg_send( hciTaskID, (uint8*)msg );
        }
    }
}


void LL_ScanTimeoutCback(void)

{
    if (hciGapTaskID == 0) {
        hciPacket_t * msg;
        uint8         totalLength;
        
        // check if LE Meta-Events are enabled and this event is enabled
        if ( ((pHciEvtMask[HCI_EVT_INDEX_LE] & HCI_EVT_MASK_LE) == 0) 
          || ((bleEvtMask & 0x10000) == 0) ) // LE_EXT_EVT_MASK_SCAN_TIMEOUT
            // the event mask is not set for this event
            return;

        // OSAL message header + HCI event header + parameters
        totalLength = sizeof (hciPacket_t) +
                      HCI_EVENT_MIN_LENGTH +
                      1; // // HCI_SCAN_TIMEOUT_EVENT_LEN
        msg = (hciPacket_t*)osal_msg_allocate(totalLength);
    
        if (msg)
        {
            // create message header
            msg->hdr.event  = HCI_CTRL_TO_HOST_EVENT;
            msg->hdr.status = 0xFF;
            // create event header
            msg->pData    = (uint8 *)(msg + 1);
            msg->pData[0] = HCI_EVENT_PACKET;
            msg->pData[1] = HCI_LE_EVENT_CODE;
            msg->pData[2] = 1; // HCI_SCAN_TIMEOUT_EVENT_LEN
            // Event Code
            msg->pData[3] = HCI_BLE_SCAN_TIMEOUT_EVENT; //  0x11
            // send message
            (void)osal_msg_send( hciTaskID, (uint8 *)msg );
        }
    } else {
        hciEvt_BLEEvent_Hdr_t * msg;
        msg = (hciEvt_BLEEvent_Hdr_t * ) osal_msg_allocate(sizeof(hciEvt_BLEEvent_Hdr_t));
        if (msg)
        {
            msg->hdr.event = HCI_GAP_EVENT_EVENT;
            msg->hdr.status = HCI_LE_EVENT_CODE;
            msg->BLEEventCode = HCI_BLE_SCAN_TIMEOUT_EVENT; // 0x11
            // send message
            (void)osal_msg_send( hciGapTaskID, (uint8 *)msg );
        }
    }
}

uint8 ll_processExtScanIRQ1(uint32_t irq_status)

{
    uint8         ll_mode, adv_mode, ext_hdr_len;
//  extScanInfo_t  *pScanInfo;
    uint32_t      T2, delay;
//  pScanInfo = &extScanInfo;

    HAL_ENTER_CRITICAL_SECTION();
    ll_mode = ll_hw_get_tr_mode();

    if (ll_mode == LL_HW_MODE_SRX) // passive scan
    {
        uint8_t  packet_len;
        uint8_t  bWlRlCheckOk = TRUE;
        uint8_t* peerAddr = NULL;
        uint8_t  bSendingScanReq = FALSE;
        uint8_t  bMatchAdv = FALSE;
        
        ll_debug_output(DEBUG_LL_HW_SRX);
        if (llTaskState != LL_TASK_EXTENDED_SCAN) goto LAB_00014b2c;

        memset(&ext_adv_hdr, 0, sizeof(ext_adv_hdr));
        // check status
        if (!(irq_status & LIRQ_RD) || !(irq_status & LIRQ_COK)) {
            if (scanningauxchain)
            {
                LL_ExtAdvReportCback0(LL_EXT_ADV_RPT_NO_DATA,
                	                  scanningpeer_addrtype,
                	                  scanningpeer_addr,
                	                  1,
                	                  0,
                	                  255,
                	                  127,
                	                  127,
                	                  0,
                	                  0,
                	                  0,
                	                  0,
                	                  0);
                scanningauxchain = 0;
            }
            packet_len = 8;
            goto LAB_0001459e;
        }
		// rx done
		uint8_t  pdu_type;
		uint16_t pktLen;
		uint32_t pktFoot0, pktFoot1;
		// read packet
        
        packet_len = ll_hw_read_rfifo1((uint8_t *)&g_rx_adv_buf.rxheader, &pktLen, &pktFoot0, &pktFoot1);
        // check receive pdu type
        pdu_type = g_rx_adv_buf.rxheader & 0xf;
        
        if(ll_hw_get_rfifo_depth() > 0)
        {
            g_pmCounters.ll_rfifo_read_err++;
            packet_len = 0;
            pktLen = 0;
        }
        if ( (packet_len != 0) 
          && (pdu_type == ADV_EXT_TYPE) ) 
        {	
      		// adv PDU header, bit 6: TxAdd, 0 - public, 1 - random
            uint8_t txAdd = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;
            uint8   payload_len = (g_rx_adv_buf.rxheader & 0xFF00) >> LENGTH_SHIFT;
            uint8   rpaListIndex = LL_RESOLVINGLIST_ENTRY_NUM;

            adv_mode    = (g_rx_adv_buf.data[0] & 0xc0) >> 6;
            ext_hdr_len =  g_rx_adv_buf.data[0] & 0x3f;
            ll_parseExtHeader(&g_rx_adv_buf.data[1], ext_hdr_len);
            peerAddr = &ext_adv_hdr.advA[0];

            if (ext_adv_hdr.header & LE_EXT_HDR_ADVA_PRESENT_BITMASK) {
                memcpy( tempAdvA, ext_adv_hdr.advA, LL_DEVICE_ADDR_LEN);
                peerAddr = ext_adv_hdr.advA;
            }
                
            // Resolving list checking
            if ( (g_llRlEnable == TRUE)
              && (adv_mode == LL_EXT_ADV_MODE_SC)
              && !(ext_adv_hdr.header & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK)
              && (ext_adv_hdr.header & LE_EXT_HDR_ADVA_PRESENT_BITMASK) )

            {
		        // if ScanA is resolvable private address
		        if ((ext_adv_hdr.advA[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR)
		        {
                    bWlRlCheckOk = FALSE;
                    if ( (isPeerRpaStore == TRUE)
                      && (currentPeerRpa[0] == ext_adv_hdr.advA[0])
                      && (currentPeerRpa[1] == ext_adv_hdr.advA[1])
                      && (currentPeerRpa[2] == ext_adv_hdr.advA[2])
                      && (currentPeerRpa[3] == ext_adv_hdr.advA[3])
                      && (currentPeerRpa[4] == ext_adv_hdr.advA[4])
                      && (currentPeerRpa[5] == ext_adv_hdr.advA[5]) )
					{
                    	bMatchAdv = TRUE;
                        rpaListIndex = storeRpaListIndex;
                    }
                    else
                    {
                        rpaListIndex = ll_getRPAListEntry(ext_adv_hdr.advA);
					}
                    if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                    {
                        peerAddr = &g_llResolvinglist[rpaListIndex].peerAddr[0];
                        txAdd = g_llResolvinglist[rpaListIndex].peerAddrType;
                        bWlRlCheckOk = TRUE;
                    }                   
                }
                else // ScanA is device Identity, if the device ID in the RPA list, check whether RPA should be used
                {
                    bWlRlCheckOk = TRUE;
                
                    for (int i = 0; i < LL_RESOLVINGLIST_ENTRY_NUM; i++)
                    {
                        if ( (ext_adv_hdr.advA[0] == g_llResolvinglist[i].peerAddr[0])
                          && (ext_adv_hdr.advA[1] == g_llResolvinglist[i].peerAddr[1])
                          && (ext_adv_hdr.advA[2] == g_llResolvinglist[i].peerAddr[2])
                          && (ext_adv_hdr.advA[3] == g_llResolvinglist[i].peerAddr[3])
                          && (ext_adv_hdr.advA[4] == g_llResolvinglist[i].peerAddr[4])
                          && (ext_adv_hdr.advA[5] == g_llResolvinglist[i].peerAddr[5]) )
                        {
                            if ( (g_llResolvinglist[i].privacyMode == NETWORK_PRIVACY_MODE)
                              && !ll_isIrkAllZero(g_llResolvinglist[i].peerIrk) )
                                bWlRlCheckOk = FALSE;
                            else
                                rpaListIndex = i;

                            break;
                        }
                    }
                    
                }
            }
            // check white list
            if ( (pGlobal_config[LL_SWITCH] & LL_WHITELIST_ALLOW)
              && (extScanInfo.wlPolicy  == LL_SCAN_WL_POLICY_USE_WHITE_LIST)
              && (bWlRlCheckOk == TRUE) )
            {
                // check white list
                bWlRlCheckOk = ll_isAddrInWhiteList(txAdd, peerAddr);
            }
            if (ext_adv_hdr.header & LE_EXT_HDR_ADVA_PRESENT_BITMASK)
            {
                scanningpeer_addrtype = txAdd;
                memcpy(scanningpeer_addr, peerAddr, LL_DEVICE_ADDR_LEN);
            }
            if ( (scanSyncInfo.valid == TRUE)
              && (ext_adv_hdr.header & LE_EXT_HDR_ADVA_PRESENT_BITMASK)
              && (bWlRlCheckOk == TRUE) )
            {
                if (!(scanSyncInfo.options & 1))
                {
                    if ( (scanSyncInfo.advertiser_Address_Type != txAdd)
                      || (memcmp(peerAddr, scanSyncInfo.advertiser_Address, LL_DEVICE_ADDR_LEN) != 0) )
                    {
                        bWlRlCheckOk = FALSE;
                    }
                }
                else
                {
                    bWlRlCheckOk = FALSE;
                    if (g_llPrdAdvDeviceNum != 0)
                    {
                        for (int i = 0; i < LL_PRD_ADV_ENTRY_NUM; i++)
                        {
                            if ( (g_llPeriodicAdvlist[i].addrType == txAdd)
                              && (ext_adv_hdr.advA[0] == g_llPeriodicAdvlist[i].addr[0])
                              && (ext_adv_hdr.advA[1] == g_llPeriodicAdvlist[i].addr[1])
                              && (ext_adv_hdr.advA[2] == g_llPeriodicAdvlist[i].addr[2])
                              && (ext_adv_hdr.advA[3] == g_llPeriodicAdvlist[i].addr[3])
                              && (ext_adv_hdr.advA[4] == g_llPeriodicAdvlist[i].addr[4])
                              && (ext_adv_hdr.advA[5] == g_llPeriodicAdvlist[i].addr[5])
                              && (((ext_adv_hdr.adi & 0xF000) >> 12) == g_llPeriodicAdvlist[i].sid) )
                            {
                                bWlRlCheckOk = TRUE;
                                break;
                            }
                        }
                    }
                }
            }
            if ( (ext_adv_hdr.header & LE_EXT_HDR_TARGETA_PRESENT_BITMASK)
              && (bWlRlCheckOk == TRUE) )
            {
                if (bMatchAdv == FALSE)
                {
			        if ((ext_adv_hdr.advA[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR)
			        {
                        // ========================== active scan path   ===================
                        // fill scanA, using RPA or device ID address
                        if ( (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                          && !ll_isIrkAllZero(g_llResolvinglist[rpaListIndex].localIrk) )
                        {
                            // for resolving private address case, calculate the scanA with Local IRK
                            if (!ll_ResolveRandomAddrs(g_llResolvinglist[rpaListIndex].localIrk, ext_adv_hdr.targetA))
                            	goto LAB_00014056;
                        }
                    }
                    else
                    {
                        uint8_t *ownAddr;
                        
                        if ( (extScanInfo.ownAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC) 
                          || (extScanInfo.ownAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM) )
                        {
		                    if ( (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
		                      && !ll_isIrkAllZero(g_llResolvinglist[rpaListIndex].localIrk) )
                                bWlRlCheckOk = FALSE;
                        
                            if (extScanInfo.ownAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM)
                                ownAddr = ownRandomAddr;
                            else
                            	ownAddr = ownPublicAddr;
                        }
                        else
                        {
                            if (extScanInfo.ownAddrType == LL_DEV_ADDR_TYPE_RANDOM)
                                ownAddr = ownRandomAddr;
                            else
                            	ownAddr = ownPublicAddr;
                        }
                        if ( (ext_adv_hdr.targetA[0] == ownAddr[0])
                          && (ext_adv_hdr.targetA[1] == ownAddr[1])
                          && (ext_adv_hdr.targetA[2] == ownAddr[2])
                          && (ext_adv_hdr.targetA[3] == ownAddr[3])
                          && (ext_adv_hdr.targetA[4] == ownAddr[4])
                          && (ext_adv_hdr.targetA[5] == ownAddr[5]) )
                        {
				            if (bWlRlCheckOk == FALSE)
				            {
				                ext_adv_hdr.header = 0;
				                goto LAB_0001459e;
				            }
                        }  
                    }
                    ext_adv_hdr.header = 0;
                    goto LAB_0001459e;
                }
            }
            else
            {
                if (bWlRlCheckOk == FALSE)
                {
                    ext_adv_hdr.header = 0;
                    goto LAB_0001459e;
                }
            }
LAB_00014056:
            uint16 extHeaderFlag;
            uint8  advEventType = adv_mode;
            uint8  primaryPHY;
            uint8  secondaryPHY;
            uint8  txPower = 127;
            uint8  sid;
            int8   rssi;
            
            if ( (extScanInfo.scanType[extScanInfo.current_index] == LL_SCAN_ACTIVE)
              && (adv_mode == ADV_NONCONN_IND)
              && !(ext_adv_hdr.header & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK) )
            {
				g_tx_adv_buf.txheader = 0xC03;
                txPower = 127;
                
                if ( (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                  && !ll_isIrkAllZero(g_llResolvinglist[rpaListIndex].localIrk) )
                {
					osal_memcpy(g_tx_adv_buf.data, ext_adv_hdr.targetA, LL_DEVICE_ADDR_LEN);
                    g_tx_adv_buf.txheader |= (((g_rx_adv_buf.rxheader & TX_ADD_MASK) << 1)
                                             | (LL_DEV_ADDR_TYPE_RANDOM << TX_ADD_SHIFT & TX_ADD_MASK));
				}
				else
				{
					memcpy(g_tx_adv_buf.data, extScanInfo.ownAddr, LL_DEVICE_ADDR_LEN);
                    g_tx_adv_buf.txheader |= (((g_rx_adv_buf.rxheader & TX_ADD_MASK) << 1)
                                             | (extScanInfo.ownAddrType << TX_ADD_SHIFT & TX_ADD_MASK));
				}
				
                g_same_rf_channel_flag = TRUE;
                ll_hw_set_tx_rx_interval(10);
                ll_hw_set_rx_timeout(500);
                set_max_length(0xff); // add 2020-03-10
                
                T2 = read_current_fine_time();
                delay = (T2 > ISR_entry_time) ? (T2 - ISR_entry_time) : (BASE_TIME_UNITS - ISR_entry_time + T2);
                // delay = 118 - delay - pGlobal_config[LL_ADV_TO_SCAN_REQ_DELAY];
                
                if ( (delay > (140 - pGlobal_config[LL_ADV_TO_SCAN_REQ_DELAY] - pGlobal_config[LL_HW_PLL_DELAY]))
                  && (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM) )
                {
                    isPeerRpaStore = TRUE;
                    storeRpaListIndex = rpaListIndex;
                    osal_memcpy(currentPeerRpa, ext_adv_hdr.advA, LL_DEVICE_ADDR_LEN);
                    g_same_rf_channel_flag = FALSE;
                }
                else
                {
                    if (g_rfPhyPktFmt == PKT_FMT_BLE1M)
                        delay = 118 - delay - pGlobal_config[EXT_ADV_AUXSCANREQ_DELAY_1MPHY];
                    else if (g_rfPhyPktFmt == PKT_FMT_BLE2M)
                        delay = 133 - delay - pGlobal_config[EXT_ADV_AUXSCANREQ_DELAY_2MPHY];
                    else if (g_rfPhyPktFmt == PKT_FMT_BLR125K)
                        delay = 118 - delay - pGlobal_config[EXT_ADV_AUXSCANREQ_DELAY_125KPHY];

                    ll_hw_set_trx();
                    ll_hw_trx_settle_bb(g_rfPhyPktFmt, delay);
                    ll_hw_go();
                    g_pmCounters.ll_send_scan_req_cnt ++;
                    llWaitingIrq = TRUE;
                    // reset Rx/Tx FIFO
                    ll_hw_rst_rfifo();
                    ll_hw_rst_tfifo();
                    ll_hw_ign_rfifo(LL_HW_IGN_CRC | LL_HW_IGN_EMP);
//                  //20181012 ZQ: change the txheader according to the adtype
//                  g_tx_adv_buf.txheader |=(((g_rx_adv_buf.rxheader & TX_ADD_MASK) << 1)
//                                        | (extScanInfo.ownAddrType<< TX_ADD_SHIFT & TX_ADD_MASK));
                    // AdvA, for SCAN REQ, it should identical to the ADV_IND/ADV_SCAN_IND
                    g_tx_adv_buf.data[6]  = ext_adv_hdr.advA[0];
                    g_tx_adv_buf.data[7]  = ext_adv_hdr.advA[1];
                    g_tx_adv_buf.data[8]  = ext_adv_hdr.advA[2];
                    g_tx_adv_buf.data[9]  = ext_adv_hdr.advA[3];
                    g_tx_adv_buf.data[10] = ext_adv_hdr.advA[4];
                    g_tx_adv_buf.data[11] = ext_adv_hdr.advA[5];
                    //write Tx FIFO
                    ll_hw_write_tfifo( (uint8*)&g_tx_adv_buf.txheader,
                                       ((g_tx_adv_buf.txheader & 0xff00) >> 8) + 2 ); // payload length + header length(2)
                    bSendingScanReq = TRUE;
                    g_same_rf_channel_flag = FALSE;
                    isPeerRpaStore = FALSE;
                }
            }
            extHeaderFlag = ext_adv_hdr.header;
            
            if (extHeaderFlag & LE_EXT_HDR_ADI_PRESENT_BITMASK)
                sid = (ext_adv_hdr.adi & 0xF000) >> 12;
            else
                sid = 0xFF;
                
            activeScanAdi = sid;

            if (extHeaderFlag & LE_EXT_HDR_TX_PWR_PRESENT_BITMASK)
                txPower = ext_adv_hdr.txPower;

            // activeScanAdi = sid;
            rssi = -(pktFoot1 >> 24);
            if (extHeaderFlag & LE_EXT_HDR_SYNC_INFO_PRESENT_BITMASK)
            {
                if ( peerAddr
                  || ((peerAddr[0] == 0) 
                   && (peerAddr[1] == 0)
                   && (peerAddr[2] == 0)
                   && (peerAddr[3] == 0)
                   && (peerAddr[4] == 0)
                   && (peerAddr[5] == 0)) )
                    peerAddr = tempAdvA;

                primaryPHY = extScanInfo.scanPHYs[extScanInfo.current_index];
                if (primaryPHY == PKT_FMT_BLR125K)
                    primaryPHY = 0x03;           //  convert 4->3

                secondaryPHY = extScanInfo.current_scan_PHY;
                if (secondaryPHY == PKT_FMT_BLR125K)
                    secondaryPHY = 0x03;           //  convert 4->3

                LL_ExtAdvReportCback0(0x00,
                                     txAdd,
                                     peerAddr,
                                     primaryPHY,
                                     secondaryPHY,
                                     sid,
                                     txPower,
                                     rssi,
                                     syncInfo.interval,
                                     0,
                                     NULL,
                                     payload_len - ext_hdr_len - 1,
                                     &g_rx_adv_buf.data[ext_hdr_len + 1]);
LAB_00014504:
                    if (!bSendingScanReq) goto LAB_00014b2c;
                    
            }
            else
            {
                if (!bSendingScanReq) goto LAB_00014b2c;
                
                if (!(extHeaderFlag & LE_EXT_HDR_TARGETA_PRESENT_BITMASK))
                {
                    osal_memcpy(tempTargetA, ext_adv_hdr.targetA, LL_DEVICE_ADDR_LEN);

                    if (extHeaderFlag & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK)
                        advEventType = advEventType | LL_EXT_ADV_RPT_SCAN_DATA;

                    primaryPHY = extScanInfo.scanPHYs[extScanInfo.current_index];
                    if (primaryPHY == PKT_FMT_BLR125K)
                        primaryPHY = 0x03;           //  convert 4->3

                    secondaryPHY = extScanInfo.current_scan_PHY;
                    if (extScanInfo.current_scan_PHY == PKT_FMT_BLR125K)
                        secondaryPHY = 0x03;           //  convert 4->3

                    LL_ExtAdvReportCback0(advEventType,
                                          scanningpeer_addrtype,
                                          scanningpeer_addr,
                                          primaryPHY,
                                          secondaryPHY,
                                          sid,
                                          txPower,
                                          rssi,
                                          0,
                                          0,
                                          NULL,
                                          payload_len - ext_hdr_len - 1,
                                          &g_rx_adv_buf.data[ext_hdr_len + 1]);
                    g_pmCounters.ll_recv_adv_pkt_cnt = g_pmCounters.ll_recv_adv_pkt_cnt + 1;
                }
            }
        }
        else if ( (pdu_type == ADV_IND)
               || (pdu_type == ADV_DIRECT_IND)
               || (pdu_type == ADV_NONCONN_IND)
               || (pdu_type == ADV_SCAN_IND) )
        {
            bWlRlCheckOk = TRUE;                
            uint8_t txAdd = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;
            if ( (extScanInfo.ownAddrType == LL_DEV_ADDR_TYPE_RANDOM) 
              || (extScanInfo.ownAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM) )
                peerAddr = ownRandomAddr;
            else
                peerAddr = ownPublicAddr;
                
            // check white list
            if ( (pGlobal_config[LL_SWITCH] & LL_WHITELIST_ALLOW)
              && (extScanInfo.wlPolicy == LL_SCAN_WL_POLICY_USE_WHITE_LIST) )
                bWlRlCheckOk = ll_isAddrInWhiteList(txAdd, g_rx_adv_buf.data); 
                
            if (pdu_type == ADV_DIRECT_IND)
            {
                if ( (g_rx_adv_buf.data[6]  != peerAddr[0])
                  || (g_rx_adv_buf.data[7]  != peerAddr[1])
                  || (g_rx_adv_buf.data[8]  != peerAddr[2])
                  || (g_rx_adv_buf.data[9]  != peerAddr[3])
                  || (g_rx_adv_buf.data[10] != peerAddr[4])
                  || (g_rx_adv_buf.data[11] != peerAddr[5]) )
                    bWlRlCheckOk = FALSE;
            }
            if (bWlRlCheckOk == TRUE)
            {
                if ( (extScanInfo.scanType[0] == LL_SCAN_ACTIVE)
                  && ((pdu_type == ADV_IND) || (pdu_type == ADV_SCAN_IND)) )
                {
                    g_tx_adv_buf.txheader = 0xC03;
                    g_same_rf_channel_flag = TRUE;
                    ll_hw_set_tx_rx_interval(10);
                    ll_hw_set_rx_timeout(300);
                    set_max_length(0xff);
                    T2 = read_current_fine_time();                           
                    delay = (T2 > ISR_entry_time) ? (T2 - ISR_entry_time) : (BASE_TIME_UNITS - ISR_entry_time + T2);
                    delay = 118 - delay - pGlobal_config[LL_ADV_TO_SCAN_REQ_DELAY];
                    
                    ll_hw_set_trx();
                    ll_hw_trx_settle_bb(g_rfPhyPktFmt, delay);
                    ll_hw_go();
                    g_pmCounters.ll_send_scan_req_cnt ++;
                    llWaitingIrq = TRUE;
                    // reset Rx/Tx FIFO
                    ll_hw_rst_rfifo();
                    ll_hw_rst_tfifo();
                    ll_hw_ign_rfifo(LL_HW_IGN_CRC | LL_HW_IGN_EMP);
                    g_tx_adv_buf.txheader |= ((g_rx_adv_buf.rxheader & TX_ADD_MASK) << 1)
                                          |  ((extScanInfo.ownAddrType << TX_ADD_SHIFT) & TX_ADD_MASK);
                    memcpy(g_tx_adv_buf.data, extScanInfo.ownAddr, LL_DEVICE_ADDR_LEN);
                    g_tx_adv_buf.data[6]  = g_rx_adv_buf.data[0];
                    g_tx_adv_buf.data[7]  = g_rx_adv_buf.data[1];
                    g_tx_adv_buf.data[8]  = g_rx_adv_buf.data[2];
                    g_tx_adv_buf.data[9]  = g_rx_adv_buf.data[3];
                    g_tx_adv_buf.data[10] = g_rx_adv_buf.data[4];
                    g_tx_adv_buf.data[11] = g_rx_adv_buf.data[5];
                    //write Tx FIFO
                    ll_hw_write_tfifo( (uint8*)&g_tx_adv_buf.txheader,
                                       (g_tx_adv_buf.txheader >> 8) + 2); // payload length + header length(2)
                    bSendingScanReq = TRUE;
                    g_same_rf_channel_flag = FALSE;
                    
                    if (pdu_type == ADV_SCAN_IND)
                        scanningScanInd = TRUE;
                }

                uint8 advEventType;
                if (pdu_type == ADV_IND)
                    advEventType = LL_EXT_ADV_RPT_MORE_FLAG | LE_ADV_PROP_SCAN_BITMASK | LE_ADV_PROP_CONN_BITMASK;

                else if (pdu_type == ADV_DIRECT_IND)
                    advEventType = LL_EXT_ADV_RPT_MORE_FLAG | LE_ADV_PROP_DIRECT_BITMASK | LE_ADV_PROP_CONN_BITMASK;

                else if (pdu_type == ADV_NONCONN_IND)
                    advEventType = LL_EXT_ADV_RPT_MORE_FLAG;

                else if (pdu_type == ADV_SCAN_IND)
                     advEventType = LL_EXT_ADV_RPT_MORE_FLAG  | LE_ADV_PROP_SCAN_BITMASK;

                else
                    advEventType = 0x00;
                    
                if (pktLen > 7)
                {
                    int8 rssi = -(pktFoot1 >> 24);
                    LL_ExtAdvReportCback0(advEventType,
                                          txAdd,
                                          g_rx_adv_buf.data,
                                          1,
                                          0,
                                          255,
                                          127,
                                          rssi,
                                          0,
                                          0,
                                          NULL,
                                          pktLen - 8,
                                          &g_rx_adv_buf.data[6]);
                    g_pmCounters.ll_recv_adv_pkt_cnt ++;
                }
                goto LAB_00014504;
            }
        }
LAB_0001459e:
        uint16 extHeaderFlag = ext_adv_hdr.header;
        if ( (extHeaderFlag & LE_EXT_HDR_SYNC_INFO_PRESENT_BITMASK)
          && (scanSyncInfo.valid == TRUE) )
        {
            uint16    sync_handler;
            uint32    schedule_time;
            uint32    accessAddress;
            llPeriodicScannerInfo_t*  pPrdScanInfo;
        
            uint32 usLen;
             
            if (extScanInfo.current_scan_PHY == PKT_FMT_BLE1M)
                usLen = packet_len << 5;
            else if (extScanInfo.current_scan_PHY == PKT_FMT_BLE2M)
                usLen = packet_len << 4;
            else
                usLen = packet_len << 8;

            sync_handler = llAllocateSyncHandle();
            pPrdScanInfo = g_llPeriodAdvSyncInfo + sync_handler;
            pPrdScanInfo->syncHandler  = sync_handler;
            pPrdScanInfo->eventCounter = syncInfo.event_counter;
            pPrdScanInfo->advInterval  = syncInfo.interval * 1250;
            memcpy(&pPrdScanInfo->chnMap[0], &syncInfo.chn_map[0], 4);
            pPrdScanInfo->chnMap[4]    = syncInfo.chn_map4.chn_map;
            pPrdScanInfo->sca          = syncInfo.chn_map4.sca;
            memcpy(&pPrdScanInfo->accessAddress[0], &syncInfo.AA[0], 4);
            memcpy(&pPrdScanInfo->crcInit[0], &syncInfo.crcInit[0], 3);  // TO check bit order
            accessAddress = (pPrdScanInfo->accessAddress[3] << 24)
                          | (pPrdScanInfo->accessAddress[2] << 16)
                          | (pPrdScanInfo->accessAddress[1] << 8)
                          |  pPrdScanInfo->accessAddress[0];
            pPrdScanInfo->advPhy = extScanInfo.current_scan_PHY;
            pPrdScanInfo->syncTimeout = scanSyncInfo.sync_Timeout * 1250;
            pPrdScanInfo->syncCteType = scanSyncInfo.sync_CTE_Type;
            pPrdScanInfo->skip        = scanSyncInfo.skip;
            // calculate next event timer, need consider the start point of AUX_ADV_IND
            pPrdScanInfo->nextEventRemainder = ((syncInfo.offset.offsetUnit == 1) ? 300 : 30) * syncInfo.offset.syncPacketOffset - usLen;
            pPrdScanInfo->syncEstOk = FALSE;
            pPrdScanInfo->event1stFlag = TRUE;        // receiving the 1st PDU of a periodic event
            // TODO
            pPrdScanInfo->channelIdentifier = ( (accessAddress & 0xFFFF0000) >> 16 ) ^ (accessAddress & 0x0000FFFF);
            schedule_time = pPrdScanInfo->nextEventRemainder; // - 2500;  // 1000: timing advance
            if (schedule_time <= 2200)
                schedule_time = 200;
            else
                schedule_time -= 2000;
             
            for (int i = 0; i < LL_NUM_BYTES_FOR_CHAN_MAP; i++)
            {
                // for each channel given by a bit in each of the bytes
                // Note: When i is on the last byte, only 5 bits need to be checked, but
                //       it is easier here to check all 8 with the assumption that the rest
                //       of the reserved bits are zero.
                for (uint8 j = 0; j < 8; j++)
                {
                    // check if the channel is used; only interested in used channels
                    if ( (pPrdScanInfo->chnMap[i] >> j) & 1 )
                    {
                        // sequence used channels in ascending order
                        pPrdScanInfo->chanMapTable[pPrdScanInfo->numUsedChans] = (i * 8U) + j;
                        // count it
                        pPrdScanInfo->numUsedChans++;
                    }
                }
            }

            pPrdScanInfo->currentEventChannel = llGetNextDataChanCSA2(pPrdScanInfo->eventCounter,
                                                                      pPrdScanInfo->channelIdentifier,
                                                                      pPrdScanInfo->chnMap,
                                                                      pPrdScanInfo->chanMapTable,
                                                                      pPrdScanInfo->numUsedChans);
            g_llPeriodAdvSyncInfo[sync_handler].current_channel = pPrdScanInfo->currentEventChannel;
            // start timer
            ll_prd_scan_schedule_next_event(schedule_time);
//                          LOG("<%d>", schedule_time);
            goto LAB_00014b2c;
        }
        llScanDuration += ((ISR_entry_time > llScanT1) ? (ISR_entry_time - llScanT1) : (BASE_TIME_UNITS - llScanT1 + ISR_entry_time));

        if ( (extScanInfo.duration == 0)
          || (extScanInfo.period != 0)
          || (llScanDuration < extScanInfo.duration * 10000) )
        {
            if ( (extHeaderFlag & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK)
              && (extScanInfo.current_chn < LL_SCAN_ADV_CHAN_37) )
                scanningauxchain = TRUE;
            else
            {
                scanningauxchain = FALSE;
                if ( !(extHeaderFlag & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK) )
                {
                    if (extScanInfo.current_chn < LL_SCAN_ADV_CHAN_37)
                    {
                        // case 2: scanning aux channel, no more aux PDU, continue to scan in primary channel
                        extScanInfo.current_chn = LL_SCAN_ADV_CHAN_37;
                        
                        if (extScanInfo.numOfScanPHY > 1)
                            extScanInfo.current_index = (extScanInfo.current_index + 1) & 0x01;
    
                        extScanInfo.current_scan_PHY = extScanInfo.scanPHYs[extScanInfo.current_index];
                        
                        scanningauxchain = FALSE;
                        llSetupExtScan(extScanInfo.current_chn);
                    }
                    else
                    {
                        scanningauxchain = FALSE;
                        llScanTime += ((ISR_entry_time > llScanT1) ? (ISR_entry_time - llScanT1) : (BASE_TIME_UNITS - llScanT1 + ISR_entry_time));

                        if (llScanTime >= extScanInfo.scanWindow[extScanInfo.current_index] * 625)
                        {
                            if ( (extScanInfo.numOfScanPHY > 1)
                              && (extScanInfo.current_chn == LL_SCAN_ADV_CHAN_39) )
                            {
                                extScanInfo.current_index = (extScanInfo.current_index + 1) & 0x01;
                                extScanInfo.current_scan_PHY = extScanInfo.scanPHYs[extScanInfo.current_index];
                            }

                            LL_CALC_NEXT_SCAN_CHN(extScanInfo.current_chn);

                            // schedule next scan event
                            if (extScanInfo.scanWindow[extScanInfo.current_index] == extScanInfo.scanInterval[extScanInfo.current_index])      // scanWindow == scanInterval, trigger immediately
                            {
                                scanningauxchain = FALSE;
                                llSetupExtScan(extScanInfo.current_chn);
                            }
                            else
                            {
                                ll_ext_scan_schedule_next_event((extScanInfo.scanInterval[extScanInfo.current_index]
                                                                - extScanInfo.scanWindow[extScanInfo.current_index]) * 625);

                                llScanDuration += (extScanInfo.scanInterval[extScanInfo.current_index]
                                                  - extScanInfo.scanWindow[extScanInfo.current_index]) * 625;
                            }
                        }
                        else
                        {
                            llSetupExtScan(extScanInfo.current_chn);
                        }
                    }
                    // reset scan total time
                    llScanTime = 0;
                    goto LAB_00014b2c;                    
                }
            }
            uint32 usLen;
            uint32 wait_time;
            
            if (extScanInfo.current_scan_PHY == PKT_FMT_BLE1M)
                usLen = packet_len << 5;
            else if (extScanInfo.current_scan_PHY == PKT_FMT_BLE2M)
                usLen = packet_len << 4;
            else
                usLen = packet_len << 8;

            extScanInfo.current_chn      = ext_adv_hdr.auxPtr.chn_idx;
            extScanInfo.current_scan_PHY = ext_adv_hdr.auxPtr.aux_phy;
            wait_time = ext_adv_hdr.auxPtr.aux_offset * ((ext_adv_hdr.auxPtr.offset_unit == 1) ? 300 : 30) - usLen;

            if (wait_time <= 1000)
                wait_time = 10;
            else
                wait_time -= 990;     // scan advance
            // reset scan total time
            llScanTime = 0;
            ll_ext_scan_schedule_next_event(wait_time);

            goto LAB_00014b2c;
        }
    }
    else if ( (ll_mode == LL_HW_MODE_TRX) && (llTaskState == LL_TASK_EXTENDED_SCAN) )
    {
        uint8_t * peerAddr = ext_adv_hdr.advA;
        memset(&ext_adv_hdr, 0, sizeof(ext_adv_hdr));

        // check status
        if ( (irq_status & LIRQ_RD) && (irq_status & LIRQ_COK) )
        {
            // rx done
            uint8_t  packet_len, pdu_type;
            uint16_t pktLen;
            uint32_t pktFoot0, pktFoot1;
            // read packet
            packet_len = ll_hw_read_rfifo((uint8_t*)(&(g_rx_adv_buf.rxheader)),
                                          &pktLen,
                                          &pktFoot0,
                                          &pktFoot1);
            // check receive pdu type
            pdu_type = g_rx_adv_buf.rxheader & PDU_TYPE_MASK;
            
            if (ll_hw_get_rfifo_depth() > 0)
            {
                g_pmCounters.ll_rfifo_read_err = g_pmCounters.ll_rfifo_read_err + 1;
                packet_len = 0;
                pktLen = 0;
            }
            else if (pdu_type == ADV_SCAN_RSP)
            {
                // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random
                uint8   txAdd    = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;
                uint8   rpaListIndex;
                int8    rssi;
                uint8   bReportCback = TRUE;

                peerAddr = g_rx_adv_buf.data;

                if ( (g_rx_adv_buf.data[0] == g_tx_adv_buf.data[6])
                  && (g_rx_adv_buf.data[1] == g_tx_adv_buf.data[7])
                  && (g_rx_adv_buf.data[2] == g_tx_adv_buf.data[8])
                  && (g_rx_adv_buf.data[3] == g_tx_adv_buf.data[9])
                  && (g_rx_adv_buf.data[4] == g_tx_adv_buf.data[10])
                  && (g_rx_adv_buf.data[5] == g_tx_adv_buf.data[11]) )
                {
                    if ( (g_llRlEnable == TRUE)
                      && (txAdd == LL_DEV_ADDR_TYPE_RANDOM)
                      && ((g_rx_adv_buf.data[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR) )
                    {
                        rpaListIndex = ll_getRPAListEntry(&g_rx_adv_buf.data[0]);
                        if (LL_RESOLVINGLIST_ENTRY_NUM <= rpaListIndex) goto LAB_00014a2c;
                        if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                        {
                        
                            peerAddr = g_llResolvinglist[rpaListIndex].peerAddr;
                            txAdd    = g_llResolvinglist[rpaListIndex].peerAddrType + LL_DEV_ADDR_TYPE_RPA_PUBLIC;
                        }
                        else
                            bReportCback = FALSE;                            
                    }
                    if (bReportCback)
                    {
                        uint8 advEventType;
                        
                        if (scanningScanInd == TRUE)
                            advEventType = LL_EXT_ADV_RPT_MORE_FLAG | LE_ADV_PROP_HI_DC_CONN_BITMASK | LE_ADV_PROP_SCAN_BITMASK;
                        else
                            advEventType = LL_EXT_ADV_RPT_MORE_FLAG | LE_ADV_PROP_HI_DC_CONN_BITMASK | LE_ADV_PROP_SCAN_BITMASK | LE_ADV_PROP_CONN_BITMASK;

                        rssi = -(pktFoot1 >> 24);
    
                        LL_ExtAdvReportCback0(advEventType,
                                              txAdd,
                                              peerAddr,
                                              1,
                                              0,
                                              255,
                                              127,
                                              rssi,
                                              0,
                                              0,
                                              NULL,
                                              packet_len - 8,
                                              &g_rx_adv_buf.data[6]);
                        g_pmCounters.ll_recv_scan_rsp_cnt ++;
                    }
                }
            }
            else if (pdu_type == ADV_EXT_TYPE)
            {
          		// adv PDU header, bit 6: TxAdd, 0 - public, 1 - random
                uint8_t txAdd = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;
                uint8   payload_len = (g_rx_adv_buf.rxheader & 0xFF00) >> LENGTH_SHIFT;
            
                uint8  sid;
                uint8  txPower = 127;
                int8   rssi;
                // uint8  advEventType = adv_mode;
                uint8  primaryPHY;
                uint8  secondaryPHY;
                uint8  rpaListIndex;

                ext_hdr_len = g_rx_adv_buf.data[0] & 0x3f;
                ll_parseExtHeader(&g_rx_adv_buf.data[1], ext_hdr_len);
                    
                rpaListIndex = ll_getRPAListEntry(&ext_adv_hdr.advA[0]);
                if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                {                    
                    peerAddr = g_llResolvinglist[rpaListIndex].peerAddr;
                    txAdd    = g_llResolvinglist[rpaListIndex].peerAddrType;
                }
                uint16 extHeaderFlag = ext_adv_hdr.header;
                
                if (extHeaderFlag & LE_EXT_HDR_ADI_PRESENT_BITMASK)
                    sid = (ext_adv_hdr.adi & 0xF000) >> 12;
                else
                    sid = activeScanAdi;

                if (extHeaderFlag & LE_EXT_HDR_TX_PWR_PRESENT_BITMASK)
                    txPower = ext_adv_hdr.txPower;

                uint8 advEventType;
                
                if (extHeaderFlag & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK)
                    advEventType = LL_EXT_ADV_RPT_MORE_FLAG | LE_ADV_PROP_HI_DC_CONN_BITMASK | LE_ADV_PROP_SCAN_BITMASK;
                else
                    advEventType = LE_ADV_PROP_HI_DC_CONN_BITMASK | LE_ADV_PROP_SCAN_BITMASK;

                primaryPHY = extScanInfo.scanPHYs[extScanInfo.current_index];
                if (primaryPHY == PKT_FMT_BLR125K)
                    primaryPHY = 0x03;           //  convert 4->3

                secondaryPHY = extScanInfo.current_scan_PHY;
                if (extScanInfo.current_scan_PHY == PKT_FMT_BLR125K)
                    secondaryPHY = 0x03;           //  convert 4->3

                rssi = -(pktFoot1 >> 24);

                LL_ExtAdvReportCback0(advEventType,
                                      txAdd,
                                      peerAddr,
                                      primaryPHY,
                                      secondaryPHY,
                                      sid,
                                      txPower,
                                      rssi,
                                      0,
                                      0,
                                      NULL,
                                      payload_len - ext_hdr_len - 1,
                                      &g_rx_adv_buf.data[ext_hdr_len + 1]);
                
                g_pmCounters.ll_recv_adv_pkt_cnt ++;
            }
LAB_00014a2c:
            llScanDuration += ((ISR_entry_time > llScanT1) ? (ISR_entry_time - llScanT1) : (BASE_TIME_UNITS - llScanT1 + ISR_entry_time));
    
            if ( (extScanInfo.duration == 0)
              || (extScanInfo.period != 0)
              || (llScanDuration < extScanInfo.duration * 10000) )
            {
    
                if ( (ext_adv_hdr.header & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK)
                  && (extScanInfo.current_chn < LL_SCAN_ADV_CHAN_37) )
                    scanningauxchain = TRUE;
    
                else
                {
                    scanningauxchain = FALSE;
                    if ( !(ext_adv_hdr.header & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK) )
                    {
                        if (extScanInfo.current_chn < LL_SCAN_ADV_CHAN_37)
                        {
                            extScanInfo.current_chn = LL_SCAN_ADV_CHAN_37;
                            if (extScanInfo.numOfScanPHY > 1)
                                extScanInfo.current_index = (extScanInfo.current_index + 1) & 1;
    
                            extScanInfo.current_scan_PHY = extScanInfo.scanPHYs[extScanInfo.current_index];
    
                            scanningauxchain = FALSE;
                            llSetupExtScan(extScanInfo.current_chn);
                        }
                        else
                        {
                            scanningauxchain = FALSE;
                            llScanTime += ((ISR_entry_time > llScanT1) ? (ISR_entry_time - llScanT1) : (BASE_TIME_UNITS - llScanT1 + ISR_entry_time));
    
                            if (llScanTime >= extScanInfo.scanWindow[extScanInfo.current_index] * 625)
                            {
                                if ( (extScanInfo.numOfScanPHY > 1)
                                  && (extScanInfo.current_chn == LL_SCAN_ADV_CHAN_39) )
                                {
                                    extScanInfo.current_index = (extScanInfo.current_index + 1) & 0x01;
                                    extScanInfo.current_scan_PHY = extScanInfo.scanPHYs[extScanInfo.current_index];
                                }
    
                                LL_CALC_NEXT_SCAN_CHN(extScanInfo.current_chn);
    
                                // schedule next scan event
                                if (extScanInfo.scanWindow[extScanInfo.current_index] == extScanInfo.scanInterval[extScanInfo.current_index])      // scanWindow == scanInterval, trigger immediately
                                {
                                    scanningauxchain = FALSE;
                                    llSetupExtScan(extScanInfo.current_chn);
                                }
                                else
                                {
                                    ll_ext_scan_schedule_next_event((extScanInfo.scanInterval[extScanInfo.current_index]
                                                                    - extScanInfo.scanWindow[extScanInfo.current_index]) * 625);
    
                                    llScanDuration += (extScanInfo.scanInterval[extScanInfo.current_index]
                                                      - extScanInfo.scanWindow[extScanInfo.current_index]) * 625;
                                }
                            }
                            else
                            {
                                llSetupExtScan(extScanInfo.current_chn);
                            }
                        }
                        // reset scan total time
                        llScanTime = 0;
                        goto LAB_00014b2c;
                    }
                }
                uint32 usLen;
                uint32 wait_time;
                
                if (extScanInfo.current_scan_PHY == PKT_FMT_BLE1M)
                    usLen = packet_len << 5;
                else if (extScanInfo.current_scan_PHY == PKT_FMT_BLE2M)
                    usLen = packet_len << 4;
                else // PKT_FMT_BLR125K 
                    usLen = packet_len << 8;
    
                extScanInfo.current_chn = ext_adv_hdr.auxPtr.chn_idx;
                extScanInfo.current_scan_PHY = ext_adv_hdr.auxPtr.aux_phy;
                wait_time = ext_adv_hdr.auxPtr.aux_offset * ((ext_adv_hdr.auxPtr.offset_unit == 1) ? 300 : 30) - usLen;
                
                if (wait_time <= 1000)
                    wait_time = 10;
                else
                    wait_time -= 990;     // scan advance
                // reset scan total time
                llScanTime = 0;
                ll_ext_scan_schedule_next_event(wait_time);
                memcpy(tempAdvA, peerAddr, LL_DEVICE_ADDR_LEN);
                goto LAB_00014b2c;
            }
        }
    }
    llTaskState = -1;
    LL_ScanTimeoutCback();
    
LAB_00014b2c:
    if (llWaitingIrq == FALSE)
    {
        ll_hw_clr_irq();
        llTaskState = LL_TASK_OTHERS;
    }
    
    HAL_EXIT_CRITICAL_SECTION();
    return TRUE;
}


uint8 ll_processExtInitIRQ1(uint32_t irq_status)
{
        
    uint8           ll_mode;
    llConnState_t * connPtr;
    uint32_t        T2, delay;
    
    HAL_ENTER_CRITICAL_SECTION();
    ll_mode = ll_hw_get_tr_mode();
//  hal_gpio_write(GPIO_P14, 1);
//  hal_gpio_write(GPIO_P14, 0);

    if (ll_mode == LL_HW_MODE_SRX)
    {
        uint8 packet_len;
        uint8 adv_mode = 0xFF;
        uint8 bWlRlCheckOk = TRUE;
        uint8 bConnecting = FALSE;
        connPtr = &conn_param[extInitInfo.connId]; // connId is allocated when create conn
        memset( &ext_adv_hdr, 0, sizeof(ext_adv_hdr) );

        // check status
        if ((irq_status & LIRQ_RD) && (irq_status & LIRQ_COK))
        {
            // rx done
            uint8_t  pdu_type;
            uint16_t pktLen;
            uint32_t pktFoot0, pktFoot1;
            
            if ( (g_rfPhyPktFmt == PKT_FMT_BLR125K)
              && (extInitInfo.current_chn < LL_SCAN_ADV_CHAN_37) )
            {
                g_same_rf_channel_flag = TRUE;
                ll_hw_set_trx();
                ll_hw_go();
                llWaitingIrq = TRUE;
                ll_hw_set_trx_settle(43, pGlobal_config[LL_HW_AFE_DELAY_125KPHY], pGlobal_config[LL_HW_PLL_DELAY_125KPHY]);
            }
            // read packet
            packet_len = ll_hw_read_rfifo((uint8_t*)(&(g_rx_adv_buf.rxheader)),
                                          &pktLen,
                                          &pktFoot0,
                                          &pktFoot1);
            // check receive pdu type
            pdu_type = g_rx_adv_buf.rxheader & 0x0f;
            
            if(ll_hw_get_rfifo_depth() > 0)
            {
                g_pmCounters.ll_rfifo_read_err++;
                packet_len = 0;
                pktLen = 0;
            }

            if (packet_len != 0)
            {
				// adv PDU header, bit 6: TxAdd, 0 - public, 1 - random
                uint8_t txAdd     = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;
                // uint8 payload_len = (g_rx_adv_buf.rxheader & 0xFF00) >> LENGTH_SHIFT;
                uint8 ext_hdr_len =  g_rx_adv_buf.data[0] & 0x3F;
                
                adv_mode          = (g_rx_adv_buf.data[0] & 0xC0) >> 6;
                
                if (pdu_type == ADV_EXT_TYPE)
                {
                    ll_parseExtHeader(&g_rx_adv_buf.data[1], ext_hdr_len);

                    if ( (osal_memcmp(ext_adv_hdr.advA, peerInfo.peerAddr, LL_DEVICE_ADDR_LEN) != 0) 
                      && (adv_mode == LL_EXT_ADV_MODE_CONN)
                      && ((ext_adv_hdr.header & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK) == 0) )
                    {
                        uint8 bWaitingIrq = llWaitingIrq;                   
                        if (llWaitingIrq == FALSE)
                        {
                            g_same_rf_channel_flag = TRUE;
                            ll_hw_set_trx();
                            ll_hw_go();
                            llWaitingIrq = TRUE;
                        }
                        conn_param[extInitInfo.connId].channel_selection = LL_CHN_SEL_ALGORITHM_2;
                        // ===============  Cnstruct AUX_CONN_REQ PDU
                        llSetupAuxConnectReqPDU();
                        rf_phy_change_cfg0(g_rfPhyPktFmt);
                        ll_hw_tx2rx_timing_config(g_rfPhyPktFmt);
                        // send conn req
                        if (bWaitingIrq == FALSE)
                        {
                            T2 = read_current_fine_time();
                            delay = (T2 > ISR_entry_time) ? (T2 - ISR_entry_time) : (BASE_TIME_UNITS - ISR_entry_time + T2);
        
                            if (g_rfPhyPktFmt == PKT_FMT_BLE1M)
                                delay = 118 + 17 - delay - pGlobal_config[EXT_ADV_AUXCONNREQ_DELAY_1MPHY];
                            else if (g_rfPhyPktFmt == PKT_FMT_BLE2M)
                                delay = 133 + 17 - delay - pGlobal_config[EXT_ADV_AUXCONNREQ_DELAY_2MPHY];
                            //=== PKT_FMT_BLR125K ????
                        
                            ll_hw_set_trx_settle(delay, 0, pGlobal_config[LL_HW_PLL_DELAY]);
                        }
                        ll_hw_rst_rfifo();
                        ll_hw_rst_tfifo();
                        ll_hw_ign_rfifo(LL_HW_IGN_CRC | LL_HW_IGN_EMP);
                        
                        //  hal_gpio_write(GPIO_P15, 1);
                        //  hal_gpio_write(GPIO_P15, 0);
                        // AdvA, offset 6
                        memcpy( &g_tx_adv_buf.data[6], ext_adv_hdr.advA, LL_DEVICE_ADDR_LEN);
                        //write Tx FIFO
                        ll_hw_write_tfifo( (uint8 *)& g_tx_adv_buf.txheader, (g_tx_adv_buf.txheader >> 8) + 2);
                        ll_hw_set_rx_timeout(700);
                        set_max_length(0xff);
                        connPtr->llRfPhyPktFmt = g_rfPhyPktFmt;
                        connPtr->curParam.winOffset = pGlobal_config[LL_CONN_REQ_WIN_OFFSET];
                        move_to_master_function1();
                        g_same_rf_channel_flag = FALSE;
                        bConnecting = TRUE;
                    }
                }
                else if ( (pdu_type == ADV_IND)
                       || (pdu_type == ADV_DIRECT_IND) )
                {
                    uint8   rpaListIndex = LL_RESOLVINGLIST_ENTRY_NUM;
                    uint8 * peerAddr = &g_rx_adv_buf.data[0];
                    uint8   bCurrentPeer = FALSE;

                    g_currentPeerAddrType = txAdd;
// ================ RPA for extended init, need investigate more
                    // Resolving list checking
                    if ( (txAdd == LL_DEV_ADDR_TYPE_RANDOM)
                      && ((peerAddr[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR) )
                    {
                        bWlRlCheckOk = FALSE;
                        if (g_llRlEnable == TRUE)
                        {
                            if ( (isPeerRpaStore == TRUE)
                              && (peerAddr[0] == currentPeerRpa[0])
                              && (peerAddr[1] == currentPeerRpa[1])
                              && (peerAddr[2] == currentPeerRpa[2])
                              && (peerAddr[3] == currentPeerRpa[3])
                              && (peerAddr[4] == currentPeerRpa[4])
                              && (peerAddr[5] == currentPeerRpa[5]) )
                            {
                                rpaListIndex = storeRpaListIndex;
                                peerAddr = g_llResolvinglist[rpaListIndex].peerAddr;
                                g_currentPeerAddrType = g_llResolvinglist[rpaListIndex].peerAddrType + '\x02';
                                bWlRlCheckOk = TRUE;
                                bCurrentPeer = TRUE;
                            } else
                            {
                                rpaListIndex = ll_getRPAListEntry(peerAddr);
                                if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                                {
                                    peerAddr = g_llResolvinglist[rpaListIndex].peerAddr;
                                    g_currentPeerAddrType = g_llResolvinglist[rpaListIndex].peerAddrType + '\x02';
                                    bWlRlCheckOk = TRUE;
                                }
                            }
                        }
                    }
                    else
                    {
                        for (int i = 0; i < LL_RESOLVINGLIST_ENTRY_NUM; i++)
                        {
                            if ( (peerAddr[0] == g_llResolvinglist[i].peerAddr[0])
                              && (peerAddr[1] == g_llResolvinglist[i].peerAddr[1])
                              && (peerAddr[2] == g_llResolvinglist[i].peerAddr[2])
                              && (peerAddr[3] == g_llResolvinglist[i].peerAddr[3])
                              && (peerAddr[4] == g_llResolvinglist[i].peerAddr[4])
                              && (peerAddr[5] == g_llResolvinglist[i].peerAddr[5]) )
                            {
                                if ( (g_llResolvinglist[i].privacyMode == NETWORK_PRIVACY_MODE)
                                  && !ll_isIrkAllZero(g_llResolvinglist[i].peerIrk) )
                                    bWlRlCheckOk = FALSE;

                                else
                                    rpaListIndex = i;
                            }
                        }
                    }
// ================ RPA for extended init, end
                    if ( (pdu_type == ADV_DIRECT_IND)
                      && (bWlRlCheckOk == TRUE)
                      && (bCurrentPeer == FALSE) )
                    {
                        if ( (g_rx_adv_buf.data[11] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR )
                        {
                            if ( (initInfo.ownAddrType != LL_DEV_ADDR_TYPE_RPA_PUBLIC)
                              && (initInfo.ownAddrType != LL_DEV_ADDR_TYPE_RPA_RANDOM) )
                                bWlRlCheckOk = FALSE;

                            if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                            {
                                if ( ll_isIrkAllZero(g_llResolvinglist[rpaListIndex].localIrk)
                                  || ll_ResolveRandomAddrs(g_llResolvinglist[rpaListIndex].localIrk, &g_rx_adv_buf.data[6]) )
                                    bWlRlCheckOk = FALSE;
                            }
                        }
                        else
                        {
                            uint8_t *ownAddr;
                            
                            if ( ((initInfo.ownAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC)
                               || (initInfo.ownAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM))
                              && (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                              && !ll_isIrkAllZero(g_llResolvinglist[rpaListIndex].localIrk) )
                                bWlRlCheckOk = FALSE;
                            
                            if ( ((g_rx_adv_buf.rxheader & RX_ADD_MASK) >> RX_ADD_SHIFT) == LL_DEV_ADDR_TYPE_RANDOM)
                                ownAddr = ownRandomAddr;
                            else
                                ownAddr = ownPublicAddr;

                            // check AdvA
                            if ((g_rx_adv_buf.data[6]  != ownAddr[0])
                             || (g_rx_adv_buf.data[7]  != ownAddr[1])
                             || (g_rx_adv_buf.data[8]  != ownAddr[2])
                             || (g_rx_adv_buf.data[9]  != ownAddr[3])
                             || (g_rx_adv_buf.data[10] != ownAddr[4])
                             || (g_rx_adv_buf.data[11] != ownAddr[5]))
                            {
                                bWlRlCheckOk = FALSE;
                            }
                        }
                    }
                    // initiator, 2 types of filter process: 1. connect to peer address set by host   2. connect to  address in whitelist only
                    // 1. connect to peer address set by host
                    if ( (initInfo.wlPolicy == LL_INIT_WL_POLICY_USE_PEER_ADDR)
                      && (bWlRlCheckOk == TRUE) )
                    {
                        //??? if ( (txAdd          != peerInfo.peerAddrType) ||
                        if ( (peerAddr[0]  != peerInfo.peerAddr[0])
                          || (peerAddr[1]  != peerInfo.peerAddr[1])
                          || (peerAddr[2]  != peerInfo.peerAddr[2])
                          || (peerAddr[3]  != peerInfo.peerAddr[3])
                          || (peerAddr[4]  != peerInfo.peerAddr[4])
                          || (peerAddr[5]  != peerInfo.peerAddr[5]) )
                            // not match, not init connect
                            bWlRlCheckOk = FALSE;
                    }
                    // 2. connect to  address in whitelist only
                    else if ( (initInfo.wlPolicy == LL_INIT_WL_POLICY_USE_WHITE_LIST)
                           && (bWlRlCheckOk == TRUE) )
                    {
                        // if advA in whitelist list, connect
                        // check white list
                        bWlRlCheckOk = ll_isAddrInWhiteList(txAdd, peerAddr);
                    }
                    
                    if (bWlRlCheckOk == TRUE)
                    {
                        g_same_rf_channel_flag = TRUE;
                        llSetupAuxConnectReqPDU();
                        if ( (pGlobal_config[LL_SWITCH] & CONN_CSA2_ALLOW)
                          && (g_rx_adv_buf.rxheader & CHSEL_MASK) )
                        {
                            conn_param[initInfo.connId].channel_selection = LL_CHN_SEL_ALGORITHM_2;
                            g_tx_adv_buf.txheader = g_tx_adv_buf.txheader | CHSEL_MASK;
                        }
                        else
                        {
                            conn_param[initInfo.connId].channel_selection = LL_CHN_SEL_ALGORITHM_1;
                            g_tx_adv_buf.txheader = g_tx_adv_buf.txheader & ~CHSEL_MASK;
                        }
                        
                        if ( (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                            && (ll_isIrkAllZero(g_llResolvinglist[rpaListIndex].localIrk) == FALSE)
                            && ((initInfo.ownAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC
                             || (initInfo.ownAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM))) )
                        {
                            
                            ll_CalcRandomAddr(g_llResolvinglist[rpaListIndex].localIrk, &g_tx_adv_buf.data[0]);
                            SET_BITS(g_tx_adv_buf.txheader, LL_DEV_ADDR_TYPE_RANDOM, TX_ADD_SHIFT, TX_ADD_MASK);
                            g_currentLocalAddrType = LL_DEV_ADDR_TYPE_RPA_RANDOM;
                        }
                        else
                        {
                            if ( (initInfo.ownAddrType == LL_DEV_ADDR_TYPE_PUBLIC)
                              || (initInfo.ownAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC) )
                            {
                                osal_memcpy(&g_tx_adv_buf.data[0], ownPublicAddr, LL_DEVICE_ADDR_LEN);
                                SET_BITS(g_tx_adv_buf.txheader, LL_DEV_ADDR_TYPE_PUBLIC, TX_ADD_SHIFT, TX_ADD_MASK);
                                g_currentLocalAddrType = LL_DEV_ADDR_TYPE_PUBLIC;
                            }
                            else
                            {
                                osal_memcpy(&g_tx_adv_buf.data[0], ownRandomAddr, LL_DEVICE_ADDR_LEN);
                                SET_BITS(g_tx_adv_buf.txheader, LL_DEV_ADDR_TYPE_RANDOM, TX_ADD_SHIFT, TX_ADD_MASK);
                                g_currentLocalAddrType = LL_DEV_ADDR_TYPE_RANDOM;
                            }
                        }
                        T2 = read_current_fine_time();
                        delay = (T2 > ISR_entry_time) ? (T2 - ISR_entry_time) : (BASE_TIME_UNITS - ISR_entry_time + T2);
                        
                        if (delay > (118 - pGlobal_config[LL_ADV_TO_CONN_REQ_DELAY] - pGlobal_config[LL_HW_PLL_DELAY]) ) 
                        {
                            isPeerRpaStore = TRUE;
                            storeRpaListIndex = rpaListIndex;
                            osal_memcpy(currentPeerRpa, &g_rx_adv_buf.data[0], LL_DEVICE_ADDR_LEN);
                            g_same_rf_channel_flag = FALSE;
                        }
                        else
                        {
                            delay = 118 - delay - pGlobal_config[LL_ADV_TO_CONN_REQ_DELAY];
                            ll_hw_set_trx_settle(delay, pGlobal_config[LL_HW_AFE_DELAY], pGlobal_config[LL_HW_PLL_DELAY]);
                            ll_hw_rst_rfifo();
                            ll_hw_rst_tfifo();
                            ll_hw_set_stx();
                            ll_hw_go();
                            llWaitingIrq = TRUE;
                            osal_memcpy( &g_tx_adv_buf.data[6], &g_rx_adv_buf.data[0], LL_DEVICE_ADDR_LEN);
                            ll_hw_write_tfifo((uint8 *)&g_tx_adv_buf.txheader, 
                                              ((g_tx_adv_buf.txheader & 0xFF00)>> 8) + 2);
                            if ( (g_currentPeerAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC)
                              || (g_currentPeerAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM) )
                                osal_memcpy(g_currentPeerRpa, &g_rx_adv_buf.data[0], LL_DEVICE_ADDR_LEN);

                            if (g_currentLocalAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM)
                                osal_memcpy(g_currentLocalRpa, &g_tx_adv_buf.data[0], LL_DEVICE_ADDR_LEN);

                            move_to_master_function1();
                            isPeerRpaStore = FALSE;
                            g_same_rf_channel_flag = FALSE;
                            
                            extInitInfo.scanMode = LL_SCAN_STOP;
                            osal_set_event(LL_TaskID, LL_EVT_MASTER_CONN_CREATED);
                        }
                    }
                }
            }
        }
        // scan again if not start connect
        if (bConnecting == FALSE) // if not waiting for scan rsp, schedule next scan
        {
            if (extInitInfo.scanMode == LL_SCAN_STOP)
            {
                // scan has been stopped
                llState = LL_STATE_IDLE;
                //  release the associated allocated connection
                llReleaseConnId(connPtr);
                g_ll_conn_ctx.numLLMasterConns --;
                (void)osal_set_event(LL_TaskID, LL_EVT_MASTER_CONN_CANCELLED);
            }
            else        
            {
                // not sending SCAN REQ
                if (extInitInfo.current_chn < LL_SCAN_ADV_CHAN_37)
                {
                    extInitInfo.current_chn = LL_SCAN_ADV_CHAN_37;
                    if (extInitInfo.numOfScanPHY > 1)
                        extInitInfo.current_index = (extInitInfo.current_index + 1) & 1;
    
                    extInitInfo.current_scan_PHY = extInitInfo.initPHYs[extInitInfo.current_index];

                    llSetupExtInit();
                    // reset scan total time
                    llScanTime = 0;
                }
                else if ( (adv_mode == ADV_DIRECT_IND)
                       && (ext_adv_hdr.header & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK) )
                {
                    uint32 usLen;
                    uint32 wait_time; 
                    
                    if (extInitInfo.current_scan_PHY == PKT_FMT_BLE1M)
                        usLen = packet_len << 5;
                    else if (extInitInfo.current_scan_PHY == PKT_FMT_BLE2M)
                        usLen = packet_len << 4;
                    else
                        usLen = packet_len << 8;

                    extInitInfo.current_chn = ext_adv_hdr.auxPtr.chn_idx;
                    extInitInfo.current_scan_PHY = ext_adv_hdr.auxPtr.aux_phy;
                    wait_time = ext_adv_hdr.auxPtr.aux_offset * ((ext_adv_hdr.auxPtr.offset_unit == 1) ? 300 : 30);
                    wait_time -= usLen;
                    if (wait_time <= 1000)
                        wait_time = 10;
                    else
                        wait_time -= 990; // scan advance
                    // reset scan total time
                    llScanTime = 0;
                    ll_ext_init_schedule_next_event(wait_time);
                }
                else
                {
                    // Update scan time
                    llScanTime += ((ISR_entry_time > llScanT1) ? (ISR_entry_time - llScanT1) : (BASE_TIME_UNITS - llScanT1 + ISR_entry_time));
                    
                    if (llScanTime >= extInitInfo.scanWindow[extInitInfo.current_index] * 625)
                    {
                    
                        if ( (extInitInfo.numOfScanPHY > 1)
                          && (extInitInfo.current_chn == LL_SCAN_ADV_CHAN_39) )
                        {
                            extInitInfo.current_index    = (extInitInfo.current_index + 1) & 0x01;
                            extInitInfo.current_scan_PHY = extInitInfo.initPHYs[extInitInfo.current_index];
                        }
                        
                        LL_CALC_NEXT_SCAN_CHN(extInitInfo.current_chn)
        
                        // schedule next scan event
                        if (extInitInfo.scanWindow[extInitInfo.current_index] ==
                            extInitInfo.scanInterval[extInitInfo.current_index])
                            llSetupExtInit();
    
                        else
                            ll_ext_init_schedule_next_event(
                                ( extInitInfo.scanInterval[extInitInfo.current_index]
                                - extInitInfo.scanWindow[extInitInfo.current_index] ) * 625);
    
                        // reset scan total time
                        llScanTime = 0;
                    }
                    else
                    {
                        extInitInfo.current_scan_PHY = extInitInfo.initPHYs[extInitInfo.current_index];
                        llSetupExtInit();
                    }
                }
            }
        }
    }
    else if (ll_mode == LL_HW_MODE_TRX)
    {
//=============
        uint8_t  packet_len, pdu_type;//, txAdd;
        uint16_t pktLen;
        uint32_t pktFoot0, pktFoot1;
        uint8    cancelConnect = FALSE;

        ll_debug_output(DEBUG_LL_HW_TRX);
        
        if ((irq_status & LIRQ_RD) && (irq_status & LIRQ_COK))
        {
            // read packet
            packet_len = ll_hw_read_rfifo1((uint8_t*)(&(g_rx_adv_buf.rxheader)),
                                           &pktLen,
                                           &pktFoot0,
                                           &pktFoot1);

            if(ll_hw_get_rfifo_depth() > 0)
            {
                g_pmCounters.ll_rfifo_read_err++;
                packet_len = 0;
                pktLen = 0;
            }

            // check receive pdu type
            pdu_type = g_rx_adv_buf.rxheader & PDU_TYPE_MASK;
//          txAdd    = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random

            if ( (packet_len > 0) // any better checking rule for rx anything?
              && (pdu_type == ADV_AUX_CONN_RSP) )
            {
//               g_pmCounters.ll_recv_scan_req_cnt ++;
                // check AdvA
                if (g_rx_adv_buf.data[8]  != ext_adv_hdr.advA[0]
                        || g_rx_adv_buf.data[9]  != ext_adv_hdr.advA[1]
                        || g_rx_adv_buf.data[10] != ext_adv_hdr.advA[2]
                        || g_rx_adv_buf.data[11] != ext_adv_hdr.advA[3]
                        || g_rx_adv_buf.data[12] != ext_adv_hdr.advA[4]
                        || g_rx_adv_buf.data[13] != ext_adv_hdr.advA[5])
                {
                    // receive err response, cancel the connection
                    cancelConnect = TRUE;
                }
            }
        }
        else
            cancelConnect = TRUE;

        if (cancelConnect == TRUE)
        {
            connPtr = &conn_param[extInitInfo.connId]; // connId is allocated when create conn

            // receive error connect rsp or timeout, cancel connection            
            if (extInitInfo.scanMode == LL_SCAN_STOP)
            {
                llState = 0;
                ll_deleteTask(extInitInfo.connId);
                g_ll_conn_ctx.currentConn = LL_INVALID_CONNECTION_ID;
                llReleaseConnId(connPtr);
                g_ll_conn_ctx.numLLMasterConns --;
                osal_set_event(LL_TaskID, LL_EVT_MASTER_CONN_CANCELLED);
            }
            else
            {
                llState = 0;
                ll_deleteTask(extInitInfo.connId);
                g_ll_conn_ctx.currentConn = LL_INVALID_CONNECTION_ID;
                extInitInfo.current_chn = LL_SCAN_ADV_CHAN_37;
                if (extInitInfo.numOfScanPHY > 1)
                    extInitInfo.current_index = (extInitInfo.current_index + 1) & 0x01;

                extInitInfo.current_scan_PHY = extInitInfo.initPHYs[extInitInfo.current_index];
                llSetupExtInit();
                llScanTime = 0;
            }
        }
        else
        {
            extInitInfo.scanMode = LL_SCAN_STOP;
            osal_set_event(LL_TaskID, LL_EVT_MASTER_CONN_CREATED);
        }
//===========
    }
    if (llWaitingIrq == FALSE) {
        ll_hw_clr_irq();
        llTaskState = LL_TASK_OTHERS;
    }

    HAL_EXIT_CRITICAL_SECTION();
    return TRUE;
}

   
void LL_IRQHandler2(void)

{
    char ret;
    uint irq_status;

    ISR_entry_time = read_current_fine_time();
    ll_debug_output(DEBUG_ISR_ENTRY);
    irq_status = ll_hw_get_irq_status();

    if ((irq_status & LIRQ_MD) == 0) { // only process IRQ of MODE DONE
        ll_hw_clr_irq(); // clear irq status
        return;
    }
    llWaitingIrq = FALSE;
    if (llTaskState == LL_TASK_EXTENDED_ADV) {
        ret = ll_processExtAdvIRQ1(irq_status);
    } else if (llTaskState == LL_TASK_EXTENDED_SCAN) {
        ret = ll_processExtScanIRQ(irq_status);
    } else if (llTaskState == LL_TASK_EXTENDED_INIT) {
        ret = ll_processExtInitIRQ(irq_status);
    } else if (llTaskState == LL_TASK_PERIODIC_ADV) {
        ret = ll_processPrdAdvIRQ(irq_status);
    } else if (llTaskState == LL_TASK_PERIODIC_SCAN) {
        ret = ll_processPrdScanIRQ(irq_status);
    } else {
        ll_processBasicIRQ(irq_status);
        ret = FALSE;
    }
    if (ret != TRUE) {
        // ================ Post ISR process: secondary pending state process
        // conn-adv case 2: other ISR, there is pending secondary advertise event, make it happen
        if (llSecondaryState == LL_SEC_STATE_ADV_PENDING)
        {
            if (llSecAdvAllow()) // for multi-connection case, it is possible still no enough time for adv
            {
                llSetupSecAdvEvt();
                llSecondaryState = LL_SEC_STATE_ADV;
            }
        }
        // there is pending scan event, make it happen, note that it may stay pending if there is no enough idle time
        else if (llSecondaryState == LL_SEC_STATE_SCAN_PENDING)
        {
            // trigger scan
            llSetupSecScan(scanInfo.nextScanChan);
        }
        // there is pending init event, make it happen, note that it may stay pending if there is no enough idle time
        else if (llSecondaryState == LL_SEC_STATE_INIT_PENDING)
        {
            // trigger init
            llSetupSecInit(initInfo.nextScanChan);
        }
        ll_debug_output(DEBUG_ISR_EXIT);
    }
}


void TIM4_IRQHandler(void)

{
    HAL_ENTER_CRITICAL_SECTION();
    
    if (AP_TIM4->status & 0x1) {
        g_timer4_irq_pending_time = AP_TIM4->CurrentCount - AP_TIM4->LoadCount;
        clear_timer_int(AP_TIM4);
        clear_timer(AP_TIM4);
        if (g_currentTimerTask == LL_TASK_EXTENDED_ADV) {
            LL_extAdvTimerExpProcess();
        } else if (g_currentTimerTask == LL_TASK_PERIODIC_ADV) {
            LL_prdAdvTimerExpProcess();
        } else if (g_currentTimerTask == LL_TASK_EXTENDED_SCAN) {
            LL_extScanTimerExpProcess();
        } else if (g_currentTimerTask == LL_TASK_EXTENDED_INIT) {
            llSetupExtInit();
        } else if (g_currentTimerTask == LL_TASK_PERIODIC_SCAN) {
            LL_prdScanTimerExpProcess();
        }
    }

    HAL_EXIT_CRITICAL_SECTION();
}

void LL_IRQHandler3(void)

{
    char ret;
    uint irq_status;

    ISR_entry_time = read_current_fine_time();
    ll_debug_output(DEBUG_ISR_ENTRY);
    irq_status = ll_hw_get_irq_status();
    
    if ((irq_status & LIRQ_MD) == 0) {
        ll_hw_clr_irq();
        return;
    }
    llWaitingIrq = FALSE;
    if (llTaskState == LL_TASK_EXTENDED_ADV) {
        ret = ll_processExtAdvIRQ(irq_status);
    } else if (llTaskState == LL_TASK_EXTENDED_SCAN) {
        ret = ll_processExtScanIRQ1(irq_status);
    } else if (llTaskState == LL_TASK_EXTENDED_INIT) {
        ret = ll_processExtInitIRQ1(irq_status);
    } else if (llTaskState == LL_TASK_PERIODIC_ADV) {
        ret = ll_processPrdAdvIRQ(irq_status);
    } else if (llTaskState == LL_TASK_PERIODIC_SCAN) {
        ret = ll_processPrdScanIRQ(irq_status);
    } else {
        ll_processBasicIRQ(irq_status);
        ret = FALSE;
    }
    if (ret != TRUE) {
        if (llSecondaryState == LL_SEC_STATE_ADV_PENDING)
        {
            if (llSecAdvAllow())
            {
                llSetupSecAdvEvt();
                llSecondaryState = LL_SEC_STATE_ADV;
            }
        }
        else if (llSecondaryState == LL_SEC_STATE_SCAN_PENDING)
        {
            llSetupSecScan(scanInfo.nextScanChan);
        } else if (llSecondaryState == LL_SEC_STATE_INIT_PENDING)
        {
            llSetupSecInit(initInfo.nextScanChan);
        }
        ll_debug_output(DEBUG_ISR_EXIT);
    }
}


__ATTR_SECTION_XIP__
void init_extadv_config(void)

{
    pGlobal_config = global_config;
    // BB_IRQ_HANDLER
    JUMP_FUNCTION(V4_IRQ_HANDLER) = (uint32_t) LL_IRQHandler2;
    JUMP_FUNCTION(TIM4_IRQ_HANDLER) = (uint32_t) TIM4_IRQHandler;
    JUMP_FUNCTION(LL_ADV_SCHEDULER) = (uint32_t) ll_adv_scheduler0;
    JUMP_FUNCTION(LL_ADV_ADD_TASK) = (uint32_t) ll_add_adv_task0;
    JUMP_FUNCTION(LL_ADV_DEL_TASK) = (uint32_t) ll_delete_adv_task0;
    JUMP_FUNCTION(LL_SETUP_EXT_ADV_EVENT) = (uint32_t) llSetupExtAdvEvent0;
    JUMP_FUNCTION(LL_SETUP_ADV_EXT_IND_PDU) = (uint32_t) llSetupAdvExtIndPDU0;
    JUMP_FUNCTION(LL_SLAVE_CONN_EVENT) = (uint32_t) LL_slave_conn_event1;
    JUMP_FUNCTION(LL_SETUP_AUX_ADV_IND_PDU) = (uint32_t) llSetupAuxAdvIndPDU1;
    JUMP_FUNCTION(LL_SETUP_AUX_CHAIN_IND_PDU) = (uint32_t) llSetupAuxChainIndPDU1;
    JUMP_FUNCTION(LL_SETUP_AUX_SCAN_RSP_PDU) = (uint32_t) llSetupAuxScanRspPDU1;
    pGlobal_config[LL_SWITCH] = pGlobal_config[LL_SWITCH] | 0x80;
    pGlobal_config[LL_EXT_ADV_TASK_DURATION] = 17000;
    pGlobal_config[LL_PRD_ADV_TASK_DURATION] = 20000;
    pGlobal_config[LL_EXT_ADV_INTER_PRI_CHN_INT] = 1500;
    pGlobal_config[LL_CONN_TASK_DURATION] = 5000;
    pGlobal_config[LL_EXT_ADV_INTER_SEC_CHN_INT] = 2500;
    pGlobal_config[LL_EXT_ADV_INTER_SEC_CHN_INT_2MPHY] = 1400;
    pGlobal_config[LL_EXT_ADV_PRI_2_SEC_CHN_INT] = 1500;
    pGlobal_config[LL_EXT_ADV_RSC_PERIOD] = 400000;
    pGlobal_config[LL_EXT_ADV_RSC_SLOT_DURATION] = 10000;
    pGlobal_config[LL_PRD_ADV_RSC_PERIOD] = 1000000;
    pGlobal_config[LL_PRD_ADV_RSC_SLOT_DURATION] = 10000;
    pGlobal_config[LL_EXT_ADV_PROCESS_TARGET] = 150;
    pGlobal_config[LL_PRD_ADV_PROCESS_TARGET] = 150;
    if (g_system_clk == SYS_CLK_DLL_48M) {
        pGlobal_config[EXT_ADV_AUXSCANRSP_DELAY_1MPHY] = 15;
        pGlobal_config[EXT_ADV_AUXCONNRSP_DELAY_1MPHY] = 15;
        pGlobal_config[EXT_ADV_AUXSCANRSP_DELAY_2MPHY] = 15;
        pGlobal_config[EXT_ADV_AUXCONNRSP_DELAY_2MPHY] = 15;
        pGlobal_config[EXT_ADV_AUXSCANRSP_DELAY_125KPHY] = 63;
        pGlobal_config[EXT_ADV_AUXCONNRSP_DELAY_125KPHY] = 63;
    }
}
