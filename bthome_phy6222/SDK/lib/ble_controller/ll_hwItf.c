
// this file is temporary, most new chip code will be wrote in this file
// after stable, the functions should be move to other LL files
/*******************************************************************************
    Filename:       ll_hwItf.c
    Revised:
    Revision:

    Description:    Interface functions to LL HW.

 SDK_LICENSE

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ll_hw_drv.h"

#include "timer.h"
#include "ll_buf.h"
#include "ll_def.h"
#include "ll.h"
#include "ll_common.h"
#include "hci_event.h"
#include "osal_bufmgr.h"
#include "bus_dev.h"
#include "ll_enc.h"
#include "rf_phy_driver.h"
#include "jump_function.h"
#include "global_config.h"
#include "ll_debug.h"
#include "log.h"

// =============== compile flag, comment out if not required below feature
//#define  PRD_ADV_ENABLE
//#define  EXT_ADV_ENABLE
//#define  EXT_SCAN_ENABLE

// ==============

/*******************************************************************************
    MACROS
*/

// add by HZF, merge with ll_hw_drv.h later
#define LL_HW_MODE_STX           0x00
#define LL_HW_MODE_SRX           0x01
#define LL_HW_MODE_TRX           0x02
#define LL_HW_MODE_RTX           0x03
#define LL_HW_MODE_TRLP          0x04
#define LL_HW_MODE_RTLP          0x05

// =========== A1 ROM metal change add
#define MAX_HDC_DIRECT_ADV_TIME          1280000              // 1.28s in unit us

#define LL_CALC_NEXT_SCAN_CHN(chan)     { chan ++; \
        chan = (chan > LL_SCAN_ADV_CHAN_39) ? LL_SCAN_ADV_CHAN_37 : chan;}

/*******************************************************************************
    CONSTANTS
*/
// Master Sleep Clock Accurracy, in PPM
// Note: Worst case range value is assumed.
extern const uint16 SCA[] ;//= {500, 250, 150, 100, 75, 50, 30, 20};
extern uint8 ownPublicAddr[];     // index 0..5 is LSO..MSB
extern uint8 ownRandomAddr[];
/*******************************************************************************
    TYPEDEFS
*/

/*******************************************************************************
    LOCAL VARIABLES
*/

///*******************************************************************************
// * GLOBAL VARIABLES
// */

uint32 llWaitingIrq = FALSE;
uint32 ISR_entry_time = 0;
int slave_conn_event_recv_delay = 0;
// for HCI ext command: enable/disable notify for connection/adv event
uint8    g_adv_taskID = 0;
uint16   g_adv_taskEvent = 0;
uint8    g_conn_taskID = 0;
uint16   g_conn_taskEvent = 0;
uint8    g_dle_taskID = 0;
uint16   g_dle_taskEvent = 0;
uint8    g_phyChg_taskID = 0;
uint16   g_phyChg_taskEvent = 0;


// =========== A2 metal change add, to move to llConnState_t(per connection parameters)
uint32_t  g_smartWindowLater          = 0;
uint32_t  g_smartWindowSize           = 0;
uint32_t  g_smartWindowSizeNew        = 0;           // to omit
bool      g_smartWindowActive         = 0;
uint32_t  g_smartWindowActiveCnt      = 0;
uint32_t  g_smartWindowPreAnchPoint   = 0;
uint8_t   g_smartWindowRTOCnt         = 0;


volatile uint32  g_getPn23_cnt  = 0;
volatile uint32  g_getPn23_seed = 0x12345678;

// =======  A2 multi-connection ========================
struct buf_tx_desc g_tx_adv_buf;
struct buf_tx_desc g_tx_ext_adv_buf;
struct buf_tx_desc tx_scanRsp_desc;

struct buf_rx_desc g_rx_adv_buf;
uint32 g_new_master_delta;


// =======  A2 multi-connection end  =====================

// ==============  BBB change
uint32   g_interAuxPduDuration = 5000;              // aux PUD duration: PDU + IFS

//  ====== RPA
uint8    g_currentLocalRpa[LL_DEVICE_ADDR_LEN];
uint8    g_currentPeerRpa[LL_DEVICE_ADDR_LEN];
uint8    g_currentPeerAddrType;
uint8    g_currentLocalAddrType;

uint8    isPeerRpaStore = FALSE;
uint8    storeRpaListIndex;
uint8    currentPeerRpa[LL_DEVICE_ADDR_LEN];

extern uint8 isTimer1Running(void);
extern void clear_timer(AP_TIM_TypeDef* TIMx);
//20180523 by ZQ
//path for tx_rx_offset issue in scan_rsq
volatile uint8_t g_same_rf_channel_flag     = FALSE;            //path for tx_rx_offset config in scan_rsp

// for scan
uint32_t llScanT1;
uint32_t llScanTime = 0;
uint32_t llCurrentScanChn;

//===================== external
//extern ctrl_packet_buf   ctrlData;

extern uint32            osal_sys_tick;
extern uint32            hclk_per_us,   hclk_per_us_shift;
extern peerInfo_t        g_llWhitelist[];

// RX Flow Control
extern uint8             rxFifoFlowCtrl;

// ============== A1 ROM metal change add
extern uint32_t g_llHdcDirAdvTime;      // for HDC direct adv

// =====   A2 metal change add
extern uint8_t             llSecondaryState;            // secondary state of LL
perStatsByChan_t*          p_perStatsByChan = NULL;

extern llConns_t           g_ll_conn_ctx;

extern syncInfo_t   syncInfo;
extern scannerSyncInfo_t    scanSyncInfo;

uint8    llTaskState;

// RF path compensation, to be move to rf_phy_driver.c ?
int16  g_rfTxPathCompensation = 0;
int16  g_rfRxPathCompensation = 0;


/*******************************************************************************
    Functions
*/
uint32_t ll_hw_get_loop_time(void);

int ll_hw_get_rfifo_depth(void);
uint16_t ll_hw_get_tfifo_wrptr(void);

// A2 metal change add
uint32_t getPN23RandNumber(void);
void ll_adptive_smart_window(uint32_t irq_status,uint32_t anchor_point);
//////////////  For Master
void move_to_master_function(void);

void LL_master_conn_event(void);

void ll_hw_read_tfifo_trlp(void);

///////////// For avtive scan
static void llAdjBoffUpperLimitSuccess( void );

static void llAdjBoffUpperLimitFailure( void );

static void llGenerateNextBackoffCount( void );


LL_PLUS_AdvDataFilterCB_t LL_PLUS_AdvDataFilterCBack=NULL;
LL_PLUS_ScanRequestFilterCB_t LL_PLUS_ScanRequestFilterCBack=NULL;

void LL_PLUS_PerStats_Init(perStatsByChan_t* p_per);
void LL_PLUS_PerStatsReset(void);

void LL_PLUS_PerStasReadByChn(uint8 chnId,perStats_t* perStats);

void ll_hw_tx2rx_timing_config(uint8 pkt);
void ll_hw_trx_settle_config(uint8 pkt);
// externel
extern uint32_t get_timer_count(AP_TIM_TypeDef* TIMx);
extern uint8 ll_processMissMasterEvt(uint8 connId);
extern uint8 ll_processMissSlaveEvt(uint8 connId);

// local function
uint32  read_ll_adv_remainder_time(void);
uint8   ll_processExtAdvIRQ(uint32_t      irq_status);
uint8   ll_processPrdAdvIRQ(uint32_t      irq_status);
uint8   ll_processExtScanIRQ(uint32_t      irq_status);
uint8   ll_processExtInitIRQ(uint32_t      irq_status);
uint8   ll_processPrdScanIRQ(uint32_t      irq_status);
uint8   ll_processBasicIRQ(uint32_t      irq_status);

uint8 llSetupExtAdvLegacyEvent(extAdvInfo_t*  pAdvInfo);

// add by HZF, to move to ll_hw_drv.c
/**************************************************************************************
    @fn          ll_hw_get_tr_mode

    @brief       This function get the current LL HW engine TR mode.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      mode  -  STX(0), SRX(1), TRX(2), RTX(3), TRLP(4), RTLP(5).
*/
uint8 ll_hw_get_tr_mode(void)
{
    uint8 mode;
    mode = (*(volatile uint32_t*)(LL_HW_BASE+ 0x04)) & 0x0f;    //[3:0] RW  4'b0    Mode
    return mode;
}

/*******************************************************************************
    @fn          LL_IRQHandler

    @brief      Interrupt Request Handler for Link Layer

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None
*/
void LL_IRQHandler(void)
{
    uint32         irq_status;
    int8 ret;
    ISR_entry_time = read_current_fine_time();
    ll_debug_output(DEBUG_ISR_ENTRY);
    irq_status = ll_hw_get_irq_status();

    if (!(irq_status & LIRQ_MD))          // only process IRQ of MODE DONE
    {
        ll_hw_clr_irq();                  // clear irq status
        return;
    }

    llWaitingIrq = FALSE;

    if (llTaskState == LL_TASK_EXTENDED_ADV)
    {
        ret = ll_processExtAdvIRQ(irq_status);

        // TODO: consider whether need process secondary adv/scan here
        if (ret == TRUE)
            return;
    }
    else if (llTaskState == LL_TASK_EXTENDED_SCAN)
    {
        ret = ll_processExtScanIRQ(irq_status);

        // TODO: consider whether need process secondary adv/scan here
        if (ret == TRUE)
            return;
    }
    else if (llTaskState == LL_TASK_EXTENDED_INIT)
    {
        ret = ll_processExtInitIRQ(irq_status);

        // TODO: consider whether need process secondary adv/scan here
        if (ret == TRUE)
            return;
    }
    else if (llTaskState == LL_TASK_PERIODIC_ADV)
    {
        ret = ll_processPrdAdvIRQ(irq_status);

        // TODO: consider whether need process secondary adv/scan here
        if (ret == TRUE)
            return;
    }
    else if (llTaskState == LL_TASK_PERIODIC_SCAN)
    {
        ret = ll_processPrdScanIRQ(irq_status);

        // TODO: consider whether need process secondary adv/scan here
        if (ret == TRUE)
            return;
    }
    else
    {
        ret = ll_processBasicIRQ(irq_status);
//      if (ret == TRUE)
//          return;
    }

    // ================ Post ISR process: secondary pending state process
    // conn-adv case 2: other ISR, there is pending secondary advertise event, make it happen
    if (llSecondaryState == LL_SEC_STATE_ADV_PENDING)
    {
        if (llSecAdvAllow())    // for multi-connection case, it is possible still no enough time for adv
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
        // trigger scan
        llSetupSecInit(initInfo.nextScanChan);
    }

    ll_debug_output(DEBUG_ISR_EXIT);
}

/*
    this function is implemented per interval,  it include multi loops of M->S and S->M within one interval

*/
// to check whether we need it later
void LL_set_default_conn_params0(llConnState_t* connPtr)
{
    connPtr->sn_nesn = 0;
    connPtr->rx_timeout   = 0;
//  connPtr->connected = 0;
    connPtr->firstPacket  = 1;
    connPtr->txDataEnabled = TRUE;
    connPtr->rxDataEnabled = TRUE;      // bug fixed 218-04-08
    connPtr->lastTimeToNextEvt  = 0;
    connPtr->lastSlaveLatency   = 0;
    connPtr->encEnabled  = FALSE;
    connPtr->lastRssi = 0;          // A1 ROM metal change add
    connPtr->expirationEvent = LL_LINK_SETUP_TIMEOUT;
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
void move_to_slave_function0(void)
{
    llConnState_t* connPtr;
    uint8_t*       pBuf;
    uint8_t       tempByte;
    uint8_t       chnSel;
    uint32_t      calibra_time, T2;

//  hal_gpio_write(GPIO_P15, 1);

    if ( (connPtr = llAllocConnId()) == NULL )
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
//hal_gpio_write(GPIO_P15, 1);
//  BM_SET(reg_gpio_ioe_porta, BIT(GPIO_P15));
//    BM_SET(reg_gpio_swporta_dr, BIT(GPIO_P15));

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

//    hal_gpio_write(GPIO_P15, 0);
//  BM_SET(reg_gpio_ioe_porta, BIT(GPIO_P15));
//    BM_CLR(reg_gpio_swporta_dr, BIT(GPIO_P15));
    // calculate timer drift
    llCalcTimerDrift(connPtr->curParam.winOffset + 2,        // 1250us + win offset, in 625us tick
                     connPtr->slaveLatency,
                     connPtr->sleepClkAccuracy,
                     (uint32*)&(connPtr->timerDrift));
    T2 = read_current_fine_time();
    // calculate the SW delay from ISR to here
    calibra_time = (T2 > ISR_entry_time) ? (T2 - ISR_entry_time) : (BASE_TIME_UNITS - ISR_entry_time + T2);
    // other delay: conn req tail -> ISR: 32us, timing advance: 50us, HW engine startup: 60us
    // start slave event SW process time: 50us
    // soft parameter: pGlobal_config[CONN_REQ_TO_SLAVE_DELAY]
    calibra_time += pGlobal_config[CONN_REQ_TO_SLAVE_DELAY];     //(32 + 50 + 60 + 50 + pGlobal_config[CONN_REQ_TO_SLAVE_DELAY]);
    // TODO: need consider the case: 0ms window offset, sometimes timer offset will < 0,
//    timer1 = 1250 + conn_param[connId].curParam.winOffset * 625 - calibra_time - conn_param[connId].timerDrift;
//    if (timer1 < 0)
//        while(1);
//
//    ll_schedule_next_event(timer1);
//#ifdef MULTI_ROLE
    uint32_t  temp;

    if (g_ll_conn_ctx.numLLConns == 1)     // 1st connection, time1 is for adv event
        clear_timer(AP_TIM1);                // stop the timer between different adv channel

    temp = 1250 + connPtr->curParam.winOffset * 625 - calibra_time - connPtr->timerDrift;
    ll_addTask(connPtr->connId, temp);
//    ll_addTask(connPtr->connId, 1250 + connPtr->curParam.winOffset * 625 - calibra_time - connPtr->timerDrift);
    g_ll_conn_ctx.scheduleInfo[connPtr->connId].task_duration = 3000;     // slave task duration: 150 + 80 + 150 + 2120 + window

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


/*******************************************************************************
    @fn          LL_slave_conn_event

    @brief       This function process slave connection event


    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None,

*/
void LL_slave_conn_event0(void)            // TODO: update connection context select
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

    if(p_perStatsByChan!=NULL)
        p_perStatsByChan->connEvtCnt[connPtr->currentChan]++;

    //ZQ 20191209
    //restore the currentChan for disable slavelatency
    //ZQ20200207 should use nextChan
    connPtr->lastCurrentChan = connPtr->nextChan;
    // counter for one connection event
    llResetRfCounters();
    //support rf phy change
    rf_phy_change_cfg(connPtr->llRfPhyPktFmt);
    ll_hw_tx2rx_timing_config(connPtr->llRfPhyPktFmt);
    // reset Rx/Tx FIFO
    ll_hw_rst_rfifo();
    ll_hw_rst_tfifo();
    // channel physical configuration
    set_crc_seed(connPtr->initCRC );            // crc seed for data PDU is from CONNECT_REQ
    set_access_address(connPtr->accessAddr);     // access address
    set_channel(connPtr->currentChan );        // set channel
    set_whiten_seed(connPtr->currentChan);     // set whiten seed
    // A2-multiconn
    ll_hw_set_rx_timeout(88);
    set_max_length(0xff);                  // add 2020-03-10

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

        if(winSizeLimt<first_window_timout)
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
    // retransmit count limit
    ll_hw_set_loop_nack_num( 4 );
    //set the rfifo ign control
    ll_hw_ign_rfifo(LL_HW_IGN_ALL);
    // write packets to Tx FIFO
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
    // start LL HW engine
    ll_hw_go();
    llWaitingIrq = TRUE;
    g_rfPhyPktFmt = temp_rf_fmt;
    HAL_EXIT_CRITICAL_SECTION();
// hal_gpio_write(GPIO_P14, 0);
//    LOG("%d-%d ", g_ll_conn_ctx.numLLConns, g_ll_conn_ctx.currentConn);
//    LOG("%d ", g_ll_conn_ctx.currentConn);
    ll_debug_output(DEBUG_LL_HW_SET_RTLP);
}

/*******************************************************************************
    @fn          LL_evt_schedule0

    @brief       Link layer event process entry


    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None,

*/
void LL_evt_schedule0(void)
{
    if (llWaitingIrq == TRUE)
    {
        // normally should not be here
        //LOG("event collision detect\n");
        g_pmCounters.ll_evt_shc_err++;
    }

    switch(llState)
    {
    // adv envent entry
    case LL_STATE_ADV_UNDIRECTED:
    case LL_STATE_ADV_DIRECTED:
    case LL_STATE_ADV_NONCONN:
    case LL_STATE_ADV_SCAN:
        VOID llSetupAdv();
        break;

    // slave connect event entry
    case LL_STATE_CONN_SLAVE:       // TO update

        //  set_default_conn_data();
        if(g_ll_conn_ctx.currentConn != LL_INVALID_CONNECTION_ID && conn_param [g_ll_conn_ctx.currentConn].active)
        {
            LL_slave_conn_event();
        }
        else
        {
            // add by HZF on 2017-12-12, if connection is not active, release
            llReleaseConnId(&conn_param[g_ll_conn_ctx.currentConn]);
        }

        break;

    case LL_STATE_SCAN:
        llScanTime = 0;
        llSetupScan(scanInfo.nextScanChan);
        break;

    case LL_STATE_INIT:
        llScanTime = 0;
        llSetupScan(initInfo.nextScanChan);
        break;

    case LL_STATE_CONN_MASTER:
        if(g_ll_conn_ctx.currentConn != LL_INVALID_CONNECTION_ID && conn_param [g_ll_conn_ctx.currentConn].active)
        {
            LL_master_conn_event();
        }
        else
        {
            // add by ZQ 20181125 necessary???
            llReleaseConnId(&conn_param[g_ll_conn_ctx.currentConn]);
            g_ll_conn_ctx.numLLMasterConns --;
        }

        break;

    case LL_STATE_IDLE:
    default:
//            ll_schedule_next_event(20000);     // add by HZF, period trigger LL task when IDLE and other not process ll state
        // it may useful when some application change the ll state but not trigger ll loop
        break;
    }
}

/*******************************************************************************
    @fn          llSetupAdv0

    @brief       This routine is used to setup the Controller for Advertising
                based on the Advertising event type.

    input parameters

    @param       None.

    output== parameters

    @param       None.

    @return      LL_STATUS_SUCCESS
*/
llStatus_t llSetupAdv0( void )
{
    if ((llState == LL_STATE_ADV_DIRECTED && (adv_param.advEvtType != LL_ADV_CONNECTABLE_HDC_DIRECTED_EVT)  && (adv_param.advEvtType != LL_ADV_CONNECTABLE_LDC_DIRECTED_EVT))
            || (llState == LL_STATE_ADV_UNDIRECTED && (adv_param.advEvtType !=  LL_ADV_CONNECTABLE_UNDIRECTED_EVT) )
            || (llState == LL_STATE_ADV_NONCONN && (adv_param.advEvtType !=  LL_ADV_NONCONNECTABLE_UNDIRECTED_EVT) )
            || (llState == LL_STATE_ADV_SCAN && (adv_param.advEvtType != LL_ADV_SCANNABLE_UNDIRECTED_EVT) ))
    {
        // sanity check failure
    }

    g_rfPhyPktFmt = LE_1M_PHY;
    //support rf phy change
    rf_phy_change_cfg(g_rfPhyPktFmt);
    ll_hw_ign_rfifo(LL_HW_IGN_CRC|LL_HW_IGN_EMP);
    ll_hw_set_rx_timeout(88);
    // reset all FIFOs; all data is forfeit
    ll_hw_rst_rfifo();
    ll_hw_rst_tfifo();

    // setup and start the advertising task based on the Adv event
    switch( adv_param.advEvtType )
    {
    // can only get a CONNECT_REQ
    case LL_ADV_CONNECTABLE_HDC_DIRECTED_EVT:
    case LL_ADV_CONNECTABLE_LDC_DIRECTED_EVT:
        // setup a Directed Advertising Event
        llSetupDirectedAdvEvt();
        break;

    // can get a SCAN_REQ or a CONNECT_REQ
    case LL_ADV_CONNECTABLE_UNDIRECTED_EVT:
        // setup a Undirected Advertising Event
        llSetupUndirectedAdvEvt();
        break;

    // neither a SCAN_REQ nor a CONNECT_REQ can be received
    case LL_ADV_NONCONNECTABLE_UNDIRECTED_EVT:
        // setup a Undirected Advertising Event
        llSetupNonConnectableAdvEvt();
        break;

    // can only get a SCAN_REQ
    case LL_ADV_SCANNABLE_UNDIRECTED_EVT:
        // setup a Discoverable Undirected Advertising Event
        llSetupScannableAdvEvt();
        break;

    default:
        // Note: this should not ever happen as the params are checked by
        //       LL_SetAdvParam()
        return( LL_STATUS_ERROR_UNKNOWN_ADV_EVT_TYPE );
    }

    // notify upper layer if required
    if (g_adv_taskID != 0)
    {
        uint8_t firstAdvChan = (adv_param.advChanMap & LL_ADV_CHAN_37) != 0 ? 37 :
                               (adv_param.advChanMap & LL_ADV_CHAN_38) != 0 ? 38 : 39;

        if(adv_param.advNextChan == firstAdvChan)
        {
            osal_set_event(g_adv_taskID, g_adv_taskEvent);
        }
    }

    return( LL_STATUS_SUCCESS );
}
/*******************************************************************************
    @fn          llSetupUndirectedAdvEvt0

    @brief       This function process for Undirected Advertising.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
void  llSetupUndirectedAdvEvt0(void)
{
    uint32_t  ch_idx;
    int i;

    // next adv channel invalid, get 1st adv chn
    if (adv_param.advNextChan > LL_ADV_CHAN_LAST || adv_param.advNextChan < LL_ADV_CHAN_FIRST)
        adv_param.advNextChan =  llGetNextAdvChn(0);     // get 1st adv chn

    ch_idx = adv_param.advNextChan;
    adv_param.advNextChan = llGetNextAdvChn(adv_param.advNextChan);

    // schedule next adv time
    if (ch_idx >= adv_param.advNextChan)
    {
        // next adv event
        int random_delay = get_timer_count(AP_TIM3) & 0x3ff;     // random adv event delay to avoid collision in the air, 0 - 1023us
        i = (adv_param.advChanMap & 0x01)
            + ((adv_param.advChanMap & 0x02) >> 1)
            + ((adv_param.advChanMap & 0x04) >> 2)
            - 1;
        ll_schedule_next_event(adv_param.advInterval * 625 - (i * pGlobal_config[ADV_CHANNEL_INTERVAL]) + (getPN23RandNumber()>>11)+random_delay);
    }
    else
    {
        ll_schedule_next_event(pGlobal_config[ADV_CHANNEL_INTERVAL]);
    }

    // set proposed state
    llState = LL_STATE_ADV_UNDIRECTED;
    ll_debug_output(DEBUG_LL_STATE_ADV_UNDIRECTED);
    //============== configure and trigger LL HW engine, LL HW work in Tx - Rx mode  ==================
    set_crc_seed(ADV_CRC_INIT_VALUE);     // crc seed for adv is same for all channels
    set_access_address(ADV_SYNCH_WORD);   // access address
    set_channel(ch_idx);             // channel
    set_whiten_seed(ch_idx);         // whiten seed
    set_max_length(50);            // rx PDU max length, may receive SCAN_REQ/CONN_REQ
    ll_hw_set_trx_settle  (pGlobal_config[LL_HW_BB_DELAY_ADV],
                           pGlobal_config[LL_HW_AFE_DELAY_ADV],
                           pGlobal_config[LL_HW_PLL_DELAY_ADV]);        //TxBB,RxAFE,PLL
    // reset Rx/Tx FIFO
    ll_hw_rst_rfifo();
    ll_hw_rst_tfifo();
    ll_hw_set_trx();                      // set LL HW as Tx - Rx mode
    ll_hw_ign_rfifo(LL_HW_IGN_EMP | LL_HW_IGN_CRC);     //set the rfifo ign control
    //write Tx FIFO
    ll_hw_write_tfifo((uint8*)&(g_tx_adv_buf.txheader), ((g_tx_adv_buf.txheader & 0xff00) >> 8) + 2);
    ll_hw_go();
    llWaitingIrq = TRUE;
    g_pmCounters.ll_send_undirect_adv_cnt ++;
    ll_debug_output(DEBUG_LL_HW_SET_TRX);
}

/*******************************************************************************
    @fn          llSetupNonConnectableAdvEvt0

    @brief       This function process for Nonconnectable Advertising.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
void llSetupNonConnectableAdvEvt0( void )
{
    uint32_t  ch_idx;
    int i;

    // next adv channel invalid, get 1st adv chn
    if (adv_param.advNextChan > LL_ADV_CHAN_LAST || adv_param.advNextChan < LL_ADV_CHAN_FIRST)
        adv_param.advNextChan =  llGetNextAdvChn(0);     // get 1st adv chn

    ch_idx = adv_param.advNextChan;
    adv_param.advNextChan = llGetNextAdvChn(adv_param.advNextChan);

    // schedule next adv time
    if (ch_idx >= adv_param.advNextChan)
    {
        // next adv event
        int random_delay = get_timer_count(AP_TIM3) & 0x3ff;;     // random adv event delay to avoid collision in the air
        i = (adv_param.advChanMap & 0x01)
            + ((adv_param.advChanMap & 0x02) >> 1)
            + ((adv_param.advChanMap & 0x04) >> 2)
            - 1;
        ll_schedule_next_event(adv_param.advInterval * 625 - (i * pGlobal_config[NON_ADV_CHANNEL_INTERVAL]) + random_delay);
    }
    else
        ll_schedule_next_event(pGlobal_config[NON_ADV_CHANNEL_INTERVAL]);

    // set proposed state
    llState = LL_STATE_ADV_NONCONN;
    ll_debug_output(DEBUG_LL_STATE_ADV_NONCONN);
    //============== configure and trigger LL HW engine, LL HW work in Single Tx mode  ==================
    set_crc_seed(ADV_CRC_INIT_VALUE);     // crc seed for adv is same for all channels
    set_access_address(ADV_SYNCH_WORD);   // access address
    set_channel(ch_idx);             // channel
    set_whiten_seed(ch_idx);         // whiten seed
    set_max_length(0xff);            // rx PDU max length
    ll_hw_set_trx_settle(pGlobal_config[LL_HW_BB_DELAY_ADV],
                         pGlobal_config[LL_HW_AFE_DELAY_ADV],
                         pGlobal_config[LL_HW_PLL_DELAY_ADV]);        //TxBB,RxAFE,PLL
    // reset Rx/Tx FIFO
    ll_hw_rst_rfifo();
    ll_hw_rst_tfifo();
    ll_hw_set_stx();                      // set LL HW as Tx - Rx mode
    ll_hw_ign_rfifo(LL_HW_IGN_ALL);     //set the rfifo ign control
    //write Tx FIFO
    ll_hw_write_tfifo((uint8*)&(g_tx_adv_buf.txheader), ((g_tx_adv_buf.txheader & 0xff00) >> 8) + 2);
    ll_hw_go();
    llWaitingIrq = TRUE;
    g_pmCounters.ll_send_nonconn_adv_cnt ++;
    ll_debug_output(DEBUG_LL_HW_SET_STX);
}  /*  end of function  */

/*******************************************************************************
    @fn          llSetupScannableAdvEvt0

    @brief       This function process Discoverable Undirected
                Advertising.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
void llSetupScannableAdvEvt0( void )
{
    uint32_t  ch_idx;
    int i;

    // next adv channel invalid, get 1st adv chn
    if (adv_param.advNextChan > LL_ADV_CHAN_LAST || adv_param.advNextChan < LL_ADV_CHAN_FIRST)
        adv_param.advNextChan =  llGetNextAdvChn(0);     // get 1st adv chn

    ch_idx = adv_param.advNextChan;
    adv_param.advNextChan = llGetNextAdvChn(adv_param.advNextChan);

    // schedule next adv time
    if (ch_idx >= adv_param.advNextChan)
    {
        // next adv event
        int random_delay = get_timer_count(AP_TIM3) & 0x3ff;;     // random adv event delay to avoid collision in the air
        i = (adv_param.advChanMap & 0x01)
            + ((adv_param.advChanMap & 0x02) >> 1)
            + ((adv_param.advChanMap & 0x04) >> 2)
            - 1;
        ll_schedule_next_event(adv_param.advInterval * 625 - (i * pGlobal_config[ADV_CHANNEL_INTERVAL]) + random_delay);
    }
    else
        ll_schedule_next_event(pGlobal_config[ADV_CHANNEL_INTERVAL]);

    // set proposed state
    llState = LL_STATE_ADV_SCAN;
    ll_debug_output(DEBUG_LL_STATE_SCAN);
    //============== configure and trigger LL HW engine, LL HW work in Tx - Rx mode  ==================
    set_crc_seed(ADV_CRC_INIT_VALUE);     // crc seed for adv is same for all channels
    set_access_address(ADV_SYNCH_WORD);   // access address
    set_channel(ch_idx);             // channel
    set_whiten_seed(ch_idx);         // whiten seed
    set_max_length(50);            // rx PDU max length, may receive SCAN_REQ
    ll_hw_set_trx_settle(pGlobal_config[LL_HW_BB_DELAY_ADV],
                         pGlobal_config[LL_HW_AFE_DELAY_ADV],
                         pGlobal_config[LL_HW_PLL_DELAY_ADV]);        //TxBB,RxAFE,PLL
    // reset Rx/Tx FIFO
    ll_hw_rst_rfifo();
    ll_hw_rst_tfifo();
    ll_hw_set_trx();                      // set LL HW as Tx - Rx mode
    ll_hw_ign_rfifo(LL_HW_IGN_EMP | LL_HW_IGN_CRC);     //set the rfifo ign control
    //write Tx FIFO
    ll_hw_write_tfifo((uint8*)&(g_tx_adv_buf.txheader), ((g_tx_adv_buf.txheader & 0xff00) >> 8) + 2);
    ll_hw_go();
    llWaitingIrq = TRUE;
    g_pmCounters.ll_send_scan_adv_cnt ++;
    ll_debug_output(DEBUG_LL_HW_SET_TRX);
}


/*******************************************************************************
    @fn          llSetupDirectedAdvEvt0

    @brief       This function process for Directed Advertising.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
// for high duty cycle, should send pdu in all adv channels in 3.75ms
void llSetupDirectedAdvEvt0( void )
{
    uint32  ch_idx, interval;
    int i;

    if (adv_param.advEvtType == LL_ADV_CONNECTABLE_HDC_DIRECTED_EVT)
        interval = pGlobal_config[HDC_DIRECT_ADV_INTERVAL];    //   adv event < 3.75ms
    else
        interval = pGlobal_config[LDC_DIRECT_ADV_INTERVAL];    // should < 10ms according to spec

// 2020-5-7, g_tx_adv_buf should be filled in function LL_SetAdvParam0. To test
    #if 0
    // construct transmit PDU
    osal_memset(g_tx_adv_buf.data, 0, sizeof(g_tx_adv_buf.data));
    SET_BITS(g_tx_adv_buf.txheader, ADV_DIRECT_IND, PDU_TYPE_SHIFT, PDU_TYPE_MASK);
    SET_BITS(g_tx_adv_buf.txheader, adv_param.ownAddrType, TX_ADD_SHIFT, TX_ADD_MASK);
    SET_BITS(g_tx_adv_buf.txheader, peerInfo.peerAddrType, RX_ADD_SHIFT, RX_ADD_MASK);
    SET_BITS(g_tx_adv_buf.txheader, 12, LENGTH_SHIFT, LENGTH_MASK);
    osal_memcpy(g_tx_adv_buf.data,  adv_param.ownAddr, 6);
    osal_memcpy((uint8_t*) &(g_tx_adv_buf.data[6]), peerInfo.peerAddr, 6);
    #endif

    // next adv channel invalid, get 1st adv chn
    if (adv_param.advNextChan > LL_ADV_CHAN_LAST || adv_param.advNextChan < LL_ADV_CHAN_FIRST)
        adv_param.advNextChan =  llGetNextAdvChn(0);     // get 1st adv chn

    ch_idx = adv_param.advNextChan;
    adv_param.advNextChan = llGetNextAdvChn(adv_param.advNextChan);

    // schedule next adv time
    if (adv_param.advEvtType == LL_ADV_CONNECTABLE_HDC_DIRECTED_EVT)
    {
        g_llHdcDirAdvTime += interval + pGlobal_config[DIR_ADV_DELAY] ;

        if (g_llHdcDirAdvTime >= MAX_HDC_DIRECT_ADV_TIME)     // for HDC direct adv, should not adv more than 1.28s
        {
            llState = LL_STATE_IDLE;                       // back to idle
            adv_param.advMode = LL_ADV_MODE_OFF;
            (void)osal_set_event( LL_TaskID, LL_EVT_DIRECTED_ADV_FAILED );
            return;
        }

        ll_schedule_next_event(interval);
    }
    else if (ch_idx >= adv_param.advNextChan)
    {
        // next adv event
        i = (adv_param.advChanMap & 0x01)
            + ((adv_param.advChanMap & 0x02) >> 1)
            + ((adv_param.advChanMap & 0x04) >> 2)
            - 1;
        ll_schedule_next_event(adv_param.advInterval * 625 - i * interval);
    }
    else
        ll_schedule_next_event(interval);

    // set proposed state
    llState = LL_STATE_ADV_DIRECTED;
    ll_debug_output(DEBUG_LL_STATE_ADV_DIRECTED);
    //============== configure and trigger LL HW engine, LL HW work in Tx - Rx mode  ==================
    set_crc_seed(ADV_CRC_INIT_VALUE);     // crc seed for adv is same for all channels
    set_access_address(ADV_SYNCH_WORD);   // access address
    set_channel(ch_idx);             // channel
    set_whiten_seed(ch_idx);         // whiten seed
    set_max_length(0xff);            // rx PDU max length
    ll_hw_set_trx_settle(pGlobal_config[LL_HW_BB_DELAY_ADV],
                         pGlobal_config[LL_HW_AFE_DELAY_ADV],
                         pGlobal_config[LL_HW_PLL_DELAY_ADV]);      //TxBB,RxAFE,PLL
    // reset Rx/Tx FIFO
    ll_hw_rst_rfifo();
    ll_hw_rst_tfifo();
    ll_hw_set_trx();                      // set LL HW as Tx - Rx mode
    ll_hw_ign_rfifo(LL_HW_IGN_EMP | LL_HW_IGN_CRC);     //set the rfifo ign control
    //write Tx FIFO
    ll_hw_write_tfifo((uint8*)&(g_tx_adv_buf.txheader), ((g_tx_adv_buf.txheader & 0xff00) >> 8) + 2);
    ll_hw_go();
    llWaitingIrq = TRUE;

    if (adv_param.advEvtType == LL_ADV_CONNECTABLE_HDC_DIRECTED_EVT)
        g_pmCounters.ll_send_hdc_dir_adv_cnt ++;
    else
        g_pmCounters.ll_send_ldc_dir_adv_cnt ++;

    ll_debug_output(DEBUG_LL_HW_SET_TRX);
} /*  end of function */

/*******************************************************************************
    @fn          llCalcTimerDrift0

    @brief       This function is used to calculate the timer drift based on a
                given time interval and a Sleep Clock Accuracy (SCA) for the
                connection.

                Note: This routine assumes that the scaValue index is valid.



    input parameters

    @param       connInterval - The connection interval in 625us ticks.
    @param       slaveLatency - Number of skipped events.
    @param       sleepClkAccuracy - SCA of peer
    @param       timerDrift   - Pointer for storing the timer drift adjustment.

    output parameters

    @param       timerDrift   - The timer drift adjustment (coarse/fine ticks).

    @return      None.
*/
void llCalcTimerDrift0( uint32    connInterval,
                        uint16   slaveLatency,
                        uint8    sleepClkAccuracy,
                        uint32*   timerDrift )      // HZF, it seems we need't corse & fine time, change to uint32
{
    uint32 time;
    uint16 sca = 0;
    // adjust the connection interval by the slave latency
    time =  (uint32)connInterval * (uint32)(slaveLatency + 1);
    // include the Slave's SCA in timer drift correction
    sca = adv_param.scaValue;
    // convert master's SCA to PPM and combine with slave
    sca += SCA[sleepClkAccuracy ];
    time *= sca;
    time = time / 1600;      // time * 625 / 1000 000 => time / 1600
    *timerDrift = time;
    return;
}

/*******************************************************************************
    @fn          ll_generateTxBuffer0

    @brief       This function generate Tx data and find in Tx FIFO
                there are 4 kinds of data:
                   1. control data
                   2. last no-ack data
                   3. last no-transmit data
                   4. new data
                 in the new RTLP buffer, the data should be in the below sequence:
                     2 --> 1 --> 3 --> 4

    input parameters

    @param       txFifo_vacancy - allow max tx packet number.

    output parameters

    @param       None.

    @return      the pointer of 1st not transmit packet/new packet.

*/
uint16 ll_generateTxBuffer0(int txFifo_vacancy, uint16* pSave_ptr)
{
    int i, new_pkts_num, tx_num = 0;
    llConnState_t* connPtr;
    connPtr = &conn_param[g_ll_conn_ctx.currentConn];

    // 0. write empty packet
    if(connPtr->llMode == LL_HW_RTLP_EMPT
            || connPtr->llMode == LL_HW_TRLP_EMPT)     //  TRLP case, to be confirmed/test
    {
        LL_HW_WRT_EMPTY_PKT;
        connPtr->ll_buf.tx_not_ack_pkt->valid = 0;                    // empty mode, tx_not_ack buffer null or empty packet
        tx_num ++;
    }
    // 1. write last not-ACK packet
    else if (connPtr->ll_buf.tx_not_ack_pkt->valid != 0)            // TODO: if the valid field could omit, move the not-ACK flag to buf.
    {
        ll_hw_write_tfifo((uint8*)&(connPtr->ll_buf.tx_not_ack_pkt->header), ((connPtr->ll_buf.tx_not_ack_pkt->header & 0xff00) >> 8) + 2);
        //txFifo_vacancy --;
        tx_num ++;
        connPtr->ll_buf.tx_not_ack_pkt->valid = 0;
    }

    // 1st RTLP event, no porcess 0/1, it should be 0 because we have reset the TFIFO
    // other case, it is 1st not transmit packet/new packet
    *pSave_ptr = ll_hw_get_tfifo_wrptr();
    rfCounters.numTxCtrl = 0;    // add on 2017-11-15, set tx control packet number 0

    // 2. write control packet
    if ((connPtr->ll_buf.tx_not_ack_pkt->valid == 0 ||                 // no tx not_ack packet, add on 2017-11-15
            (connPtr->ll_buf.tx_not_ack_pkt->header & 0x3) != LL_DATA_PDU_HDR_LLID_CONTROL_PKT)    // last nack packet is not a control packet
            && connPtr->ctrlDataIsPending                                               // we only support 1 control procedure per connection
            && !connPtr->ctrlDataIsProcess
            && txFifo_vacancy > connPtr->ll_buf.ntrm_cnt)    // tricky here:  if the Tx FIFO is full and nothing is sent in last event, then it can't fill new packet(include ctrl pkt) in new event
    {
        // not in a control procedure, and there is control packet pending
        // fill ctrl packet
        ll_hw_write_tfifo((uint8*)&(connPtr->ctrlData .header), ((connPtr->ctrlData .header & 0xff00) >> 8) + 2);
        txFifo_vacancy --;
        tx_num ++;
        // put Ctrl packet in TFIFO, change the control procedure status
        connPtr->ctrlDataIsPending = 0;
        connPtr->ctrlDataIsProcess = 1;
        rfCounters.numTxCtrl = 1;     // add 2017-11-15, if put new ctrl packet in FIFO, add the counter
    }

    // 3. write last not transmit packets
    if (connPtr->ll_buf.ntrm_cnt > 0
            && txFifo_vacancy >= connPtr->ll_buf.ntrm_cnt)
    {
        for (i = 0; i < connPtr->ll_buf.ntrm_cnt ; i++)
        {
            ll_hw_write_tfifo((uint8*)&(connPtr->ll_buf.tx_ntrm_pkts[i]->header), ((connPtr->ll_buf.tx_ntrm_pkts[i]->header & 0xff00) >> 8) + 2);
        }

        txFifo_vacancy -= connPtr->ll_buf.ntrm_cnt;
        tx_num += connPtr->ll_buf.ntrm_cnt;
        connPtr->ll_buf.ntrm_cnt = 0;
    }

    if (connPtr->ll_buf.ntrm_cnt != 0)
    {
        // should not be here, new packets should not be sent if there is not-transmit packets
        return tx_num;
    }

    // 4. write  new data packets to FIFO
    new_pkts_num = getTxBufferSize(connPtr);

    if ((new_pkts_num > 0)
            && txFifo_vacancy > 0)
    {
        // fill the data packet to Tx FIFO
        for (i = 0; i < new_pkts_num && i < txFifo_vacancy; i++)
        {
            uint8_t idx = get_tx_read_ptr(connPtr);
            ll_hw_write_tfifo((uint8*)&(connPtr->ll_buf.tx_conn_desc[idx]->header), ((connPtr->ll_buf.tx_conn_desc[idx]->header & 0xff00) >> 8) + 2);
            update_tx_read_ptr(connPtr);
            tx_num++;
            // update PM counter, add A1 ROM metal change
            connPtr->pmCounter.ll_send_data_pkt_cnt ++;
        }
    }

    // 2020-02-13 periodic cte req & rsp
    if( ( connPtr->llConnCTE.enable ) && ( connPtr->llCTE_ReqFlag ))
    {
        if( connPtr->llConnCTE.CTE_Request_Intv > 0 )
        {
            if( connPtr->llConnCTE.CTE_Count_Idx < connPtr->llConnCTE.CTE_Request_Intv )
                connPtr->llConnCTE.CTE_Count_Idx++;
            else
            {
                connPtr->llConnCTE.CTE_Count_Idx = 0;
                llEnqueueCtrlPkt(connPtr, LL_CTRL_CTE_REQ );
            }
        }
    }

    return tx_num;
}


/*******************************************************************************
    @fn          ll_read_rxfifo0

    @brief       This function read HW Rx FIFO to internal buffer

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
void ll_read_rxfifo0(void)
{
    uint8_t packet_len, idx;
    uint16_t pktLen;
    uint32_t pktFoot0, pktFoot1;
    int depth;
    llConnState_t* connPtr;
    connPtr = &conn_param[g_ll_conn_ctx.currentConn];        // TODO
    depth = ll_hw_get_rfifo_depth();

    // read packet
    while (getRxBufferFree(connPtr) > 0      // SW rx buffer has vacancy
            && depth > 0)               // something to read from HW Rx FIFO
    {
        idx = get_rx_write_ptr(connPtr);
        packet_len = ll_hw_read_rfifo((uint8_t*)(&(connPtr->ll_buf.rx_conn_desc[idx]->header)),
                                      &pktLen,
                                      &pktFoot0,
                                      &pktFoot1);

        if (packet_len == 0)
        {
            break;
        }

        connPtr->ll_buf.rx_conn_desc[idx]->valid = 1;
        update_rx_write_ptr(connPtr);     // increment write pointer
        depth -= (packet_len + 2) ;
    }

    connPtr->lastRssi =   pktFoot1 >> 24;           // RSSI , -dBm

    // TODO: get other information from pktFoot0/pktFoot1

    if (depth > 0)
    {
        // warning: some data in Rx FIFO is not read, normal this should not happen
        connPtr->pmCounter.ll_recv_abnormal_cnt ++;
    }
}


/*******************************************************************************
    @fn          ll_hw_read_tfifo_rtlp0

    @brief       This function read not-ack packet and untransmit packets in RT Loop
                This function is also used in TR Loop mode, note that for TRLP,
                Tx_fifo_rd_addr_last == Tx_fifo_rd_addr, no not-ACK packet

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_read_tfifo_rtlp0(void)
{
    uint32_t  tmp, irq_status;
    uint32_t last_rd_addr, current_wr_addr;
    uint32_t current_rd_addr;
    int depth, len;
    llConnState_t* connPtr;
    connPtr = &conn_param[g_ll_conn_ctx.currentConn];
    // reset the not-ACK buf & not-transmit buf status
    connPtr->ll_buf.tx_not_ack_pkt->valid = 0;
    connPtr->ll_buf.ntrm_cnt = 0;
    // read registers for LL HW status
    last_rd_addr =  (*(volatile uint32_t*)(LL_HW_BASE + 0x4c)) & 0x7ff;
    current_wr_addr = 0x07ff & ((*(volatile uint32_t*)(LL_HW_BASE + 0x50)) >> 16);
    current_rd_addr = 0x07ff & ((*(volatile uint32_t*)(LL_HW_BASE + 0x50)));
    // revert HW read pointer
    tmp = *(volatile uint32_t*)(LL_HW_BASE + 0x5C);
    *(volatile uint32_t*)(LL_HW_BASE + 0x5C) = (tmp & 0x7ff0000) | last_rd_addr;    // set rd_cnt_ini as Tx_fifo_rd_addr_last
    // calculate depth in Tx FIFO, include not-ack + not-transmit packets, note that the depth in 4bytes unit
    depth = current_wr_addr - last_rd_addr;     // Tx_fifo_wr_addr - Tx_fifo_rd_addr_last

    //read not ack packet
    if (depth > 0)
    {
        if (current_rd_addr != last_rd_addr)
        {
            len = ll_hw_read_tfifo_packet((uint8*)&(connPtr->ll_buf.tx_not_ack_pkt->header));
            depth -= len;
            connPtr->ll_buf.tx_not_ack_pkt->valid = 1;                // set not ack packet buffer valid
        }
        else          // current_rd_addr == last_rd_addr
        {
            irq_status = ll_hw_get_irq_status();

            if (irq_status & LIRQ_CERR2)                 // double CRC error, HW will not send packet when 2nd CRC error
            {
                len = ll_hw_read_tfifo_packet((uint8*)&(connPtr->ll_buf.tx_not_ack_pkt->header));
                depth -= len;
                connPtr->ll_buf.tx_not_ack_pkt->valid = 1;                // set not ack packet buffer valid
            }
            else
            {
                // should not be here
            }
        }
    }

    // read not transmit packets
    while (depth > 0
            && connPtr->ll_buf.ntrm_cnt < g_maxPktPerEventTx)     // A1 ROM metal change: change to MAX_LL_BUF_LEN // 05-19: change size from (MAX_CONN_BUF - 1) to MAX_CONN_BUF. not ack packet is not part of tx num
    {
        len = ll_hw_read_tfifo_packet((uint8*)&(connPtr->ll_buf.tx_ntrm_pkts[connPtr->ll_buf.ntrm_cnt]->header));    // read a packet from Tx FIFO
        depth -= len;
        connPtr->ll_buf.ntrm_cnt ++;
    }
}

/*******************************************************************************
    @fn          ll_hw_read_tfifo_packet0

    @brief       This function read a packet form HW Tx FIFO

    input parameters

    @param       pkt    - packet buffer pointer

    output parameters

    @param       None.

    @return      length of pkt in 4bytes unit.
*/
int ll_hw_read_tfifo_packet0(uint8* pkt)
{
    int j, len;
    *((uint32_t*)&pkt[0]) = *(volatile uint32_t*)(LL_HW_TFIFO);
    uint8_t sp =BLE_HEAD_WITH_CTE(pkt[0]);
    len =   (pkt[1] + 2 + 3 +sp ) & 0x1fc;                 //+2 for Header, +3 to get ceil

    for(j = 4; j < len; j += 4)
    {
        *((uint32_t*)&pkt[j]) = *(volatile uint32_t*)(LL_HW_TFIFO);
    }

    return (len >> 2);
}


/*******************************************************************************
    @fn          ll_hw_process_RTO0

    @brief       This function will update the TFIFO last read pointer in receive time out case

    input parameters

    @param       ack_num

    output parameters

    @param       None.

    @return      None.
    @Note        if receive timeout in 1st connection event and there is data in TFIFO, there should be no
                Not-ACK packet. This function not consider this because we will not send data packet in 1st event
*/
void ll_hw_process_RTO0(uint32 ack_num)
{
    uint32_t  tmp;
    uint32_t last_rd_addr;
    uint32_t current_rd_addr;
    uint32 len, i;
    // read registers for LL HW status
    last_rd_addr = 0;
    //current_wr_addr = 0x07ff & ((*(volatile uint32_t *)(LL_HW_BASE + 0x50)) >> 16);
    //current_rd_addr = 0x07ff & ((*(volatile uint32_t *)(LL_HW_BASE + 0x50)));       //  equal last_rd_addr in RTO case

    //*(volatile uint32_t *)0x40030068 = current_rd_addr;

    // 1. get last rd addr
    for (i = 0; i < ack_num; i ++)
    {
        tmp = *(volatile uint32_t*)(LL_HW_BASE + 0x5C);
        *(volatile uint32_t*)(LL_HW_BASE + 0x5C) = (tmp & 0x7ff0000) | last_rd_addr;    // set rd_cnt_ini as Tx_fifo_rd_addr_last
        tmp = *(volatile uint32*)(LL_HW_TFIFO);
        len = (tmp >> 8) & 0xff;                            // length of the packet
        len = ((len + 2 + 3 ) >> 2) ;                   // +2 for Header, +3 to get ceil
        last_rd_addr += len ;
    }

    // 2. get current rd addr
    tmp = *(volatile uint32_t*)(LL_HW_BASE + 0x5C);
    *(volatile uint32_t*)(LL_HW_BASE + 0x5C) = (tmp & 0x7ff0000) | last_rd_addr;    // set rd_cnt_ini as Tx_fifo_rd_addr_last
    tmp = *(volatile uint32*)(LL_HW_TFIFO);
    len = (tmp >> 8) & 0xff;                            // length of the packet
    len = ((len + 2 + 3 ) >> 2) ;                   // +2 for Header, +3 to get ceil
    current_rd_addr = last_rd_addr + len;
    // set HW read pointer
    *(volatile uint32_t*)(LL_HW_BASE + 0x5C) =  (last_rd_addr << 16) | current_rd_addr;
}

/**************************************************************************************
    @fn          ll_hw_get_tfifo_wrptr

    @brief       This function process for HW LL getting tx fifo information

    input parameters

    @param

    output parameters

    @param       rdPtr  : read pointer.
                wrPtr  : write pointer
                wrDepth: fifo depth can be writen

    @return      None.
*/
uint16_t ll_hw_get_tfifo_wrptr(void)
{
    uint16_t wrPtr;
    wrPtr    =  0x07ff & ((*(volatile uint32_t*)(LL_HW_BASE + 0x50)) >>16);
    return wrPtr;
}

/**************************************************************************************
    @fn          ll_hw_get_loop_time

    @brief       This function get the loop timeout timer counter

    input parameters

    @param        None.

    output parameters

    @param        None.



    @return      loop timer counter.
*/
uint32_t ll_hw_get_loop_time(void)
{
    return (*(volatile uint32_t*)(LL_HW_BASE + 0x70));
}

/**************************************************************************************
    @fn          ll_hw_get_rfifo_depth

    @brief       This function get the HW LL rx fifo depth

    input parameters

    @param

    output parameters

    @param       None.



    @return      Rx FIFO depth(i.e. words available) in 4bytes unit
*/
int ll_hw_get_rfifo_depth(void)
{
    uint16_t    rdPtr, wrPtr;
    uint32_t tmp = *(volatile uint32_t*)(LL_HW_BASE + 0x54);
    rdPtr    =  0x07ff & tmp;
    wrPtr    =  0x07ff & (tmp>>16);
    return ( wrPtr - rdPtr);
}



/*******************************************************************************
    @fn          ll_adptive_smart_window

    @brief      will adptive modified following var to adjust the rx window in RTLP
                1.slave_conn_event_recv_delay
                2.pGlobal_config[LL_HW_RTLP_1ST_TIMEOUT

    input parameters
    @param       irq_status
    @param       anchor_point

    output parameters

    @param       None

    @return      None.
*/
void ll_adptive_smart_window0(uint32 irq_status,uint32 anchor_point)
{
    if(irq_status & LIRQ_TD )
    {
        //tracking anchpoint
        if((        irq_status & LIRQ_COK)
                &&  (0== conn_param[g_ll_conn_ctx.currentConn].firstPacket) )
        {
            //==========================================================
            // when the anchor point change litter, active smartwindow
            uint32_t anchPre_anchCurrent;
            anchPre_anchCurrent =   (g_smartWindowPreAnchPoint>anchor_point)
                                    ? g_smartWindowPreAnchPoint-anchor_point
                                    : anchor_point-g_smartWindowPreAnchPoint;

            if(anchPre_anchCurrent < pGlobal_config[LL_SMART_WINDOW_ACTIVE_RANGE])
            {
                g_smartWindowActiveCnt = (g_smartWindowActiveCnt > pGlobal_config[LL_SMART_WINDOW_ACTIVE_THD])
                                         ? g_smartWindowActiveCnt : g_smartWindowActiveCnt+1;
            }
        }
        else
        {
            g_smartWindowLater      =   0;
            g_smartWindowActiveCnt  =   0;
        }
    }
    else
    {
        g_smartWindowLater      =   0;
        g_smartWindowActiveCnt  =   0;
    }

    //record the pre anchor point
    if(irq_status& LIRQ_TD && irq_status & LIRQ_COK)
    {
        g_smartWindowPreAnchPoint = anchor_point;
    }

    if(g_smartWindowActiveCnt > pGlobal_config[LL_SMART_WINDOW_ACTIVE_THD])
    {
        int dlt_anchPoint = anchor_point - pGlobal_config[LL_SMART_WINDOW_TARGET];
        g_smartWindowLater = g_smartWindowLater + (dlt_anchPoint >> pGlobal_config[LL_SMART_WINDOW_COEF_ALPHA]);
        slave_conn_event_recv_delay -= g_smartWindowLater;
    }

    //-------------------------------------------------------------------------------
    //enlarge the RX WINDOW when rxtimeout
    if(irq_status & LIRQ_TD )
    {
        g_smartWindowRTOCnt = 0;
    }
    else
    {
        g_smartWindowRTOCnt = (g_smartWindowRTOCnt > 5) ? g_smartWindowRTOCnt : (g_smartWindowRTOCnt + 1);
    }
}

uint32_t getPN23RandNumber(void)
{
    g_getPn23_cnt++;
    uint32_t feedback = ((g_getPn23_seed & (1<<23))>>23) + ((g_getPn23_seed&(1<<18))>>18) & 0x01;
    g_getPn23_seed = ((0x007fffff & g_getPn23_seed)<<1) + feedback;
    return (g_getPn23_seed);
}

//////////////////////////
void ll_schedule_next_event(int time)
{
    set_timer(AP_TIM1, time);
}

void ll_ext_adv_schedule_next_event(int time)
{
    set_timer(AP_TIM4, time);
    g_currentAdvTimer = time;
    g_currentTimerTask = LL_TASK_EXTENDED_ADV;
}

void ll_prd_adv_schedule_next_event(int time)
{
    set_timer(AP_TIM4, time);
    g_currentAdvTimer = time;
    g_currentTimerTask = LL_TASK_PERIODIC_ADV;
}

void ll_ext_scan_schedule_next_event(int time)
{
    set_timer(AP_TIM4, time);
    g_currentTimerTask = LL_TASK_EXTENDED_SCAN;
}

void ll_prd_scan_schedule_next_event(int time)
{
    set_timer(AP_TIM4, time);
    g_currentTimerTask = LL_TASK_PERIODIC_SCAN;
}

void ll_ext_init_schedule_next_event(int time)
{
    set_timer(AP_TIM4, time);
    g_currentTimerTask = LL_TASK_EXTENDED_INIT;
}

/**************************************************************************************
    @fn          ll_debug_output

    @brief       update the debug state

    input parameters

    @param       state  -  the LL status, the SRAM project will interpret the status and dirve GPIO, UART, memory print, ...

    output parameters

    @param       None


    @return      None
*/
void ll_debug_output(uint32 state)
{
    if (pGlobal_config[LL_SWITCH] & LL_DEBUG_ALLOW)
    {
        debug_print(state);
    }
}


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
void move_to_master_function0(void)
{
    int  calibrate;
    llConnState_t* connPtr;
//  hal_gpio_write(GPIO_P15, 1);
    connPtr = &conn_param[initInfo.connId];
//    LOG("move_to_master_function\r\n");
    // set connection parameters
    LL_set_default_conn_params(connPtr);
    // clear the connection buffer
    reset_conn_buf(connPtr->connId);
    // configure rx timeout once
    ll_hw_set_rx_timeout(268);
    initInfo.scanMode = LL_SCAN_STOP;
    extInitInfo.scanMode = LL_SCAN_STOP;
    // adjust time units to 625us
    connPtr->curParam.winSize      <<= 1; // in 1.25ms units so convert to 625us
    connPtr->curParam.winOffset    <<= 1; // in 1.25ms units so convert to 625us
    connPtr->curParam.connInterval <<= 1; // in 1.25ms units so convert to 625us
    connPtr->curParam.connTimeout  <<= 4; // in 10ms units so convert to 625us
    // convert the LSTO from time to an expiration connection event count
    llConvertLstoToEvent( connPtr, &connPtr->curParam );
    // set the expiration connection event count to a specified limited number
    // Note: This is required in case the Master never sends that first packet.
    connPtr->expirationEvent = LL_LINK_SETUP_TIMEOUT;
    // convert the Control Procedure timeout into connection event count
    llConvertCtrlProcTimeoutToEvent( connPtr );
//  // calculate channel for data
//  llProcessChanMap(&conn_param[connId], conn_param[connId].chanMap );
    // need?
//  connPtr->slaveLatency = connPtr->curParam.slaveLatency ;
//  connPtr->slaveLatencyValue = connPtr->curParam.slaveLatency;
    connPtr->currentEvent               = 0;
    connPtr->nextEvent                  = 0;
    connPtr->active = 1;
    connPtr->sn_nesn = 0;                  // 1st rtlp, init sn/nesn as 0
    connPtr->llMode = LL_HW_TRLP;         // set as RTLP_1ST for the 1st connection event
    connPtr->currentChan = 0;

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

    llState = LL_STATE_CONN_MASTER;
    llSecondaryState = LL_SEC_STATE_IDLE;                 // add for multi-connection
    // wait to the next event start
    calibrate = pGlobal_config[LL_MOVE_TO_MASTER_DELAY];
    uint32  schedule_time;

//  margin = 2500;
    // get current allocate ID delta time to ID 0
    if (g_ll_conn_ctx.currentConn != LL_INVALID_CONNECTION_ID)
    {
        schedule_time = g_new_master_delta;// * 625;     // delta timing to previous connection slot
        ll_addTask(connPtr->connId, schedule_time);         // shcedule new connection
        g_ll_conn_ctx.scheduleInfo[connPtr->connId].task_duration = 2700;   // master task duration

        // current link id may be updated in ll_addTask, update the ll state
        if (g_ll_conn_ctx.scheduleInfo[g_ll_conn_ctx.currentConn].linkRole == LL_ROLE_MASTER)
            llState = LL_STATE_CONN_MASTER;
        else if (g_ll_conn_ctx.scheduleInfo[g_ll_conn_ctx.currentConn].linkRole == LL_ROLE_SLAVE)
            llState = LL_STATE_CONN_SLAVE;
    }
    else   //  1st connection
    {
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
    (void)osal_set_event( LL_TaskID, LL_EVT_MASTER_CONN_CREATED );
    g_pmCounters.ll_conn_succ_cnt ++;
//  hal_gpio_write(GPIO_P15, 0);
//    LOG("M2M ");
}

void llWaitUs(uint32_t wtTime);
/*******************************************************************************
    @fn          LL_master_conn_event

    @brief       This function process master connection event


    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None,

*/
void LL_master_conn_event0(void)
{
    uint16_t ll_rdCntIni;
    uint32_t  tx_num, rx_num;
    uint32_t  temp;
    uint32_t  T1, T2, delta;
    llConnState_t* connPtr;
    T1 = read_current_fine_time();

    if (llWaitingIrq)
        g_pmCounters.ll_tbd_cnt3++;

    connPtr = &conn_param[g_ll_conn_ctx.currentConn];
//  hal_gpio_write(GPIO_P18, 1);
    // time critical process, disable interrupt
    HAL_ENTER_CRITICAL_SECTION();
    g_ll_conn_ctx.timerExpiryTick = T1;                    // A2 multiconnection
//#ifdef  MULTI_ROLE
//#else
//  schedule_time = ll_get_next_timer(g_ll_conn_ctx.currentConn);
//
//    // next TRLP event start, 10 for timer ISR delay  --> to evaluate changing it to periodic timer? NO for multi-connection reason
////    ll_schedule_next_event(connPtr->curParam.connInterval * 625 - 20);    // 10us: rough delay from timer expire to here
//    ll_schedule_next_event(schedule_time - 10);
//#endif
//
    connPtr->pmCounter.ll_conn_event_cnt ++;

    if(p_perStatsByChan!=NULL)
        p_perStatsByChan->connEvtCnt[connPtr->currentChan]++;

    tx_num = pGlobal_config[LL_TX_PKTS_PER_CONN_EVT];
    rx_num = pGlobal_config[LL_RX_PKTS_PER_CONN_EVT];

    if (tx_num > g_maxPktPerEventTx || tx_num == 0)    tx_num = g_maxPktPerEventTx;

    if (rx_num > g_maxPktPerEventRx || rx_num == 0)    rx_num = g_maxPktPerEventRx;

    // counter for one connection event
    llResetRfCounters();
    //support rf phy change
    rf_phy_change_cfg(connPtr->llRfPhyPktFmt);
    ll_hw_tx2rx_timing_config(connPtr->llRfPhyPktFmt);
    // reset Rx/Tx FIFO
    ll_hw_rst_rfifo();
    ll_hw_rst_tfifo();
    // channel physical configuration
    set_crc_seed(connPtr->initCRC );            // crc seed for data PDU is from CONNECT_REQ
    set_access_address(connPtr->accessAddr);     // access address
    set_channel(connPtr->currentChan );        // set channel
    set_whiten_seed(connPtr->currentChan);     // set whiten seed
    // A2 multi-conn
    ll_hw_set_rx_timeout(88);
    ll_hw_set_rx_timeout_1st(88);
//  set_max_length(40);
    set_max_length(0xff);
    // configure loop timeout
    temp = connPtr->curParam.connInterval * 625 - connPtr->llPduLen.local.MaxRxTime - pGlobal_config[LL_HW_TRLP_TO_GAP];       // 500us: margin for timer1 IRQ
    temp = temp >> 3;                    // maximum 8 connections, HZF
    ll_hw_set_loop_timeout(temp > pGlobal_config[LL_HW_TRLP_LOOP_TIMEOUT] ?
                           pGlobal_config[LL_HW_TRLP_LOOP_TIMEOUT] : temp);         // 2018-6-20, global config for the parameter
    // retransmit count limit
    ll_hw_set_loop_nack_num( 4 );
    //set the rfifo ign control
    ll_hw_ign_rfifo(LL_HW_IGN_ALL);
    // write packets to Tx FIFO
    tx_num = ll_generateTxBuffer(tx_num, &ll_rdCntIni);
    // config TRLP
    ll_hw_config( LL_HW_TRLP,//conn_param[connId].llMode,  // LL HW logical mode
                  connPtr->sn_nesn,                        // sn,nesn init
                  tx_num,                                  // ll_txNum
                  rx_num,                                  // ll_rxNum
                  1,                                       // ll_mdRx
                  0);                                      // rdCntIni
    T2 = read_current_fine_time();
    delta = LL_TIME_DELTA(T1, T2);                        // software process delay
    delta += pGlobal_config[LL_MASTER_TIRQ_DELAY];         // compensate the timer IRQ pending -> timer ISR delay
    temp = (pGlobal_config[LL_MASTER_PROCESS_TARGET] > delta) ? (pGlobal_config[LL_MASTER_PROCESS_TARGET] - delta) : 0;
    llWaitUs(temp);             // insert delay to make process time equal PROCESS_TARGET
    ll_hw_trx_settle_config(connPtr->llRfPhyPktFmt);
    uint8 temp_rf_fmt = g_rfPhyPktFmt;
    g_rfPhyPktFmt = connPtr->llRfPhyPktFmt;
    // start LL HW engine
    ll_hw_go();
    g_rfPhyPktFmt = temp_rf_fmt;
    llWaitingIrq = TRUE;
//   hal_gpio_write(GPIO_P18, 0);
    HAL_EXIT_CRITICAL_SECTION();
//  LOG("%d-%d ", g_ll_conn_ctx.numLLConns, g_ll_conn_ctx.currentConn);
//  LOG("%d ", g_ll_conn_ctx.currentConn);
//  LOG(" %d> ", schedule_time);
    ll_debug_output(DEBUG_LL_HW_SET_TRLP);
}

// ======================== scan back off process function
/*******************************************************************************
    @fn          llAdjBoffUpperLimitSuccess

    @brief       This function is used to handle the calculation of the Scan
                backoff counter upper limit based on the number of successfully
                received Scan Responses to our Scan Requests. When two
                consecutive Scan Responses are successfully received, the upper
                limit of the Scan backoff counter is halfed (but no less than
                one).

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
static void llAdjBoffUpperLimitSuccess( void )
{
    // first, since this is a success, clear the number of consecutive failures
    scanInfo.numFailure = 0;

    // check if we received two successful in a row
    if ( ++scanInfo.numSuccess == 2 )
    {
        // yes, so half backoff upper limit
        scanInfo.scanBackoffUL >>= 1;

        // however, the minimum is 1
        if ( scanInfo.scanBackoffUL == 0 )
        {
            scanInfo.scanBackoffUL = 1;
        }

        // reset consecutive count
        scanInfo.numSuccess = 0;
    }

    return;
}


/*******************************************************************************
    @fn          llAdjBoffUpperLimitFailure

    @brief       This function is used to handle the calculation of the Scan
                backoff counter upper limit based on the number of failured to
                receive Scan Responses to our Scan Requests. When two
                consecutive Scan Responses fail to be received, the upper
                limit of the Scan backoff counter is doubled (but no more than
                256).

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
static void llAdjBoffUpperLimitFailure( void )
{
    // first, since this was a failure, clear the number of consecutive successes
    scanInfo.numSuccess = 0;

    // check if we received two failures in a row
    if ( ++scanInfo.numFailure == 2 )
    {
        // yes, so double backoff upper limit
        scanInfo.scanBackoffUL <<= 1;

        // maximum is 256
        if ( scanInfo.scanBackoffUL > 256 )
        {
            scanInfo.scanBackoffUL = 256;
        }

        // reset consecutive count
        scanInfo.numFailure = 0;
    }

    g_pmCounters.ll_tbd_cnt4++;
    return;
}


/*******************************************************************************
    @fn          llGenerateNextBackoffCount

    @brief       This function is used to find the next Scan backoff
                count which determines when the next Scan Request will be sent
                when the appropriate Adv packet is correctly received. The
                backoff count is just to limit collisions. The backoff count
                is randomly generated, but must be between 1 and the backoff
                count upper limit (max 256). The upper limit changes based on
                the number of consecutive successful or failed Scan Responses.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
static void llGenerateNextBackoffCount( void )
{
    // determine the new backoff count constrained by upper limit
    // Note: Backoff and Upper Limit can be 1..256.
    if ( scanInfo.scanBackoffUL == 1 )
    {
        scanInfo.currentBackoff = 1;
    }
    else // backoff count is a random number from 1..UL
    {
        scanInfo.currentBackoff = ((uint16)LL_ENC_GeneratePseudoRandNum() % scanInfo.scanBackoffUL) + 1;
    }

//    hal_uart_tx("scanBackoffUL = ");
//    hal_uart_send_int(scanInfo.scanBackoffUL);
//    hal_uart_tx(",currentBackoff = ");
//    hal_uart_send_int(scanInfo.currentBackoff);
//    hal_uart_tx("\r\n");
    return;
}

// ===============================

void llWaitUs(uint32_t wtTime)
{
    uint32_t T1, T2, deltTick;

    if (wtTime == 0)
        return;

    T1 = read_current_fine_time();

    while(1)
    {
        T2 = read_current_fine_time();
        deltTick = (T2 >= T1) ? (T2 - T1) : (BASE_TIME_UNITS - T1 + T2);

        if(deltTick > wtTime)
            break;
    }
}

uint8_t* LL_PLUS_GetAdvDataExtendData(void)
{
//  llConnState_t *connPtr;
//    connPtr = &conn_param[0];
    return((uint8_t*)&g_rx_adv_buf.data[0]);
}

void LL_PLUS_SetAdvDataFilterCB(LL_PLUS_AdvDataFilterCB_t AdvDataFilterCBack)
{
    LL_PLUS_AdvDataFilterCBack=AdvDataFilterCBack;
}

void LL_PLUS_SetScanRequestData(uint8 dLen, uint8* pData)
{
//  llConnState_t *connPtr;

//    connPtr = &conn_param[0];
    for(int i=0; i<dLen; i++)
        g_tx_adv_buf.data[12+i]=pData[i];

    g_tx_adv_buf.txheader=((dLen+12)<<8)|(g_tx_adv_buf.txheader&0x00ff);
}

void LL_PLUS_SetScanRequestFilterCB(LL_PLUS_ScanRequestFilterCB_t ScanRequestFilterCBack)
{
    LL_PLUS_ScanRequestFilterCBack=ScanRequestFilterCBack;
}

uint8 LL_PLUS_GetScanRequestExtendData(uint8* pData)
{
//  llConnState_t *connPtr;
//    connPtr = &conn_param[0];
    uint8 eLen=((g_rx_adv_buf.rxheader&0xff00)>>8)-12;

    for(uint8 i=0; i<eLen; i++)
        pData[i] = g_rx_adv_buf.data[12+i];

    return eLen;
}

void LL_PLUS_GetScanerAddr(uint8* pData)
{
//  llConnState_t *connPtr;

//    connPtr = &conn_param[0];
    for(uint8 i=0; i<6; i++)
        pData[i] = g_rx_adv_buf.data[0+i];

    return;
}

void LL_PLUS_SetScanRsqData(uint8 dLen,uint8* pData)
{
//  llConnState_t *connPtr;

//    connPtr = &conn_param[0];
    for(uint8 i=0; i<dLen; i++)
    {
        g_tx_adv_buf.data[6+i]=pData[i];
    }

    g_tx_adv_buf.txheader=((dLen+6)<<8)|(g_tx_adv_buf.txheader&0x00ff);
}

void LL_PLUS_SetScanRsqDataByIndex(uint8 dIdx,uint8 data)
{
//llConnState_t *connPtr;
//connPtr = &conn_param[0];
    tx_scanRsp_desc.data[6+dIdx]=data;
}
void LL_PLUS_PerStats_Init(perStatsByChan_t* p_per)
{
    p_perStatsByChan = p_per;
    LL_PLUS_PerStatsReset();
}

void LL_PLUS_PerStatsReset(void)
{
    if(p_perStatsByChan!=NULL)
        osal_memset(p_perStatsByChan, 0,sizeof(perStatsByChan_t));
}

void LL_PLUS_PerStasReadByChn(uint8 chnId,perStats_t* perStats)
{
    if( (p_perStatsByChan!=NULL) && (perStats!=NULL) )
    {
        perStats->rxNumPkts     =p_perStatsByChan->rxNumPkts[chnId];
        perStats->rxNumCrcErr   =p_perStatsByChan->rxNumCrcErr[chnId];
        perStats->txNumRetry    =p_perStatsByChan->txNumRetry[chnId];
        perStats->TxNumAck      =p_perStatsByChan->TxNumAck[chnId];
        perStats->rxToCnt       =p_perStatsByChan->rxToCnt[chnId];
        perStats->connEvtCnt    =p_perStatsByChan->connEvtCnt[chnId];
    }
}

void ll_hw_tx2rx_timing_config(uint8 pkt)
{
    if(pkt==PKT_FMT_BLE1M)
    {
        ll_hw_set_rx_tx_interval(    pGlobal_config[LL_HW_Rx_TO_TX_INTV]);      //T_IFS=150us for BLE 1M
        ll_hw_set_tx_rx_interval(    pGlobal_config[LL_HW_Tx_TO_RX_INTV]);      //T_IFS=150us for BLE 1M
    }
    else if(pkt==PKT_FMT_BLE2M)
    {
        ll_hw_set_rx_tx_interval(    pGlobal_config[LL_HW_Rx_TO_TX_INTV_2MPHY]);        //T_IFS=150us for BLE 1M
        ll_hw_set_tx_rx_interval(    pGlobal_config[LL_HW_Tx_TO_RX_INTV_2MPHY]);
    }
    else if(pkt==PKT_FMT_BLR500K)
    {
        ll_hw_set_rx_tx_interval(    pGlobal_config[LL_HW_Rx_TO_TX_INTV_500KPHY]);      //T_IFS=150us for BLE 1M
        ll_hw_set_tx_rx_interval(    pGlobal_config[LL_HW_Tx_TO_RX_INTV_500KPHY]);
    }
    else
    {
        ll_hw_set_rx_tx_interval(    pGlobal_config[LL_HW_Rx_TO_TX_INTV_125KPHY]);      //T_IFS=150us for BLE 1M
        ll_hw_set_tx_rx_interval(    pGlobal_config[LL_HW_Tx_TO_RX_INTV_125KPHY]);
    }
}

void ll_hw_trx_settle_config(uint8 pkt)
{
    if(pkt==PKT_FMT_BLE1M)
    {
        ll_hw_set_trx_settle(pGlobal_config[LL_HW_BB_DELAY],
                             pGlobal_config[LL_HW_AFE_DELAY],
                             pGlobal_config[LL_HW_PLL_DELAY]);      // TxBB, RxAFE, PLL
    }
    else if(pkt==PKT_FMT_BLE2M)
    {
        ll_hw_set_trx_settle(pGlobal_config[LL_HW_BB_DELAY_2MPHY],
                             pGlobal_config[LL_HW_AFE_DELAY_2MPHY],
                             pGlobal_config[LL_HW_PLL_DELAY_2MPHY]);        // TxBB, RxAFE, PLL
    }
    else if(pkt==PKT_FMT_BLR500K)
    {
        ll_hw_set_trx_settle(pGlobal_config[LL_HW_BB_DELAY_500KPHY],
                             pGlobal_config[LL_HW_AFE_DELAY_500KPHY],
                             pGlobal_config[LL_HW_PLL_DELAY_500KPHY]);      // TxBB, RxAFE, PLL
    }
    else
    {
        ll_hw_set_trx_settle(pGlobal_config[LL_HW_BB_DELAY_125KPHY],
                             pGlobal_config[LL_HW_AFE_DELAY_125KPHY],
                             pGlobal_config[LL_HW_PLL_DELAY_125KPHY]);      // TxBB, RxAFE, PLL
    }
}


// multi-connection
/*******************************************************************************
    @fn          llSetupSecAdvEvt0

    @brief       This function process for Nonconnectable Advertising.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      uint8 - TRUE(send success), FALSE(send fail).
*/
uint8 llSetupSecAdvEvt0( void )
{
    uint8 ret = FALSE;

    if (llState == LL_STATE_IDLE)
    {
        if (adv_param.advEvtType == LL_ADV_CONNECTABLE_UNDIRECTED_EVT)
            llState = LL_STATE_ADV_UNDIRECTED;
        else if (adv_param.advEvtType == LL_ADV_NONCONNECTABLE_UNDIRECTED_EVT)
            llState = LL_STATE_ADV_NONCONN;
        else if (adv_param.advEvtType == LL_ADV_SCANNABLE_UNDIRECTED_EVT)
            llState = LL_STATE_ADV_SCAN;

        llSetupAdv();
        llSecondaryState = LL_SEC_STATE_IDLE;
        return TRUE;
    }

    if (adv_param.advEvtType == LL_ADV_CONNECTABLE_UNDIRECTED_EVT)
        ret = llSetupSecConnectableAdvEvt();
    else if (adv_param.advEvtType == LL_ADV_NONCONNECTABLE_UNDIRECTED_EVT)
        ret = llSetupSecNonConnectableAdvEvt();
    else if (adv_param.advEvtType == LL_ADV_SCANNABLE_UNDIRECTED_EVT)
        ret = llSetupSecScannableAdvEvt();
    else
        return FALSE;          // other type adv should not here

    return ret;
}

/*******************************************************************************
    @fn          llSetupSecConnectableAdvEvt

    @brief       This function process for set up sec undirect Advertising.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      uint8 - TRUE(send success), FALSE(send fail).
*/
uint8 llSetupSecConnectableAdvEvt0( void )
{
    uint32_t  ch_idx;
    // Hold off interrupts.
    HAL_ENTER_CRITICAL_SECTION(  );

    if (llWaitingIrq)
    {
        g_pmCounters.ll_tbd_cnt1++;
        HAL_EXIT_CRITICAL_SECTION(  );
        return FALSE;
    }

    // next adv channel invalid, get 1st adv chn
    if (adv_param.advNextChan > LL_ADV_CHAN_LAST || adv_param.advNextChan < LL_ADV_CHAN_FIRST)
        adv_param.advNextChan =  llGetNextAdvChn(0);     // get 1st adv chn

    ch_idx = adv_param.advNextChan;
    adv_param.advNextChan = llGetNextAdvChn(adv_param.advNextChan);
    //============== configure and trigger LL HW engine, LL HW work in Single Tx mode  ==================
    set_crc_seed(ADV_CRC_INIT_VALUE);     // crc seed for adv is same for all channels
    set_access_address(ADV_SYNCH_WORD);   // access address
    set_channel(ch_idx);             // channel
    set_whiten_seed(ch_idx);         // whiten seed
    set_max_length(0xff);            // rx PDU max length
    ll_hw_set_trx_settle(pGlobal_config[LL_HW_BB_DELAY_ADV],
                         pGlobal_config[LL_HW_AFE_DELAY_ADV],
                         pGlobal_config[LL_HW_PLL_DELAY_ADV]);        //TxBB,RxAFE,PLL
    // reset Rx/Tx FIFO
    ll_hw_rst_rfifo();
    ll_hw_rst_tfifo();
    ll_hw_set_trx();                      // set LL HW as Tx - Rx mode
    ll_hw_ign_rfifo(LL_HW_IGN_EMP | LL_HW_IGN_CRC);     //set the rfifo ign control
    //write Tx FIFO
    ll_hw_write_tfifo((uint8*)&(g_tx_adv_buf.txheader), ((g_tx_adv_buf.txheader & 0xff00) >> 8) + 2);
    ll_hw_go();
    llWaitingIrq = TRUE;
    HAL_EXIT_CRITICAL_SECTION(  );
    g_pmCounters.ll_send_conn_adv_cnt ++;           // adv in conn state counter
    g_pmCounters.ll_send_undirect_adv_cnt ++;
    #ifdef DEBUG_LL
    LOG("c%d ", ch_idx);
    #endif
    return TRUE;
}  /*  end of function  */


/*******************************************************************************
    @fn          llSetupSecScannableAdvEvt0

    @brief       This function process for setup scannable Advertising.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      uint8 - TRUE(send success), FALSE(send fail).
*/
uint8 llSetupSecScannableAdvEvt0( void )
{
    uint32_t  ch_idx;
    // Hold off interrupts.
    HAL_ENTER_CRITICAL_SECTION(  );

    if (llWaitingIrq)
    {
        g_pmCounters.ll_tbd_cnt1++;           // to update the conuter name
        HAL_EXIT_CRITICAL_SECTION(  );
        return FALSE;
    }

    // next adv channel invalid, get 1st adv chn
    if (adv_param.advNextChan > LL_ADV_CHAN_LAST || adv_param.advNextChan < LL_ADV_CHAN_FIRST)
        adv_param.advNextChan =  llGetNextAdvChn(0);     // get 1st adv chn

    ch_idx = adv_param.advNextChan;
    adv_param.advNextChan = llGetNextAdvChn(adv_param.advNextChan);
    //============== configure and trigger LL HW engine, LL HW work in Single Tx mode  ==================
    set_crc_seed(ADV_CRC_INIT_VALUE);     // crc seed for adv is same for all channels
    set_access_address(ADV_SYNCH_WORD);   // access address
    set_channel(ch_idx);             // channel
    set_whiten_seed(ch_idx);         // whiten seed
    set_max_length(0xff);            // rx PDU max length
    ll_hw_set_trx_settle(pGlobal_config[LL_HW_BB_DELAY_ADV],
                         pGlobal_config[LL_HW_AFE_DELAY_ADV],
                         pGlobal_config[LL_HW_PLL_DELAY_ADV]);        //TxBB,RxAFE,PLL
    // reset Rx/Tx FIFO
    ll_hw_rst_rfifo();
    ll_hw_rst_tfifo();
    ll_hw_set_trx();                      // set LL HW as Tx - Rx mode
    ll_hw_ign_rfifo(LL_HW_IGN_EMP | LL_HW_IGN_CRC);     //set the rfifo ign control
    //write Tx FIFO
    ll_hw_write_tfifo((uint8*)&(g_tx_adv_buf.txheader), ((g_tx_adv_buf.txheader & 0xff00) >> 8) + 2);
    ll_hw_go();
    llWaitingIrq = TRUE;
    HAL_EXIT_CRITICAL_SECTION(  );
    g_pmCounters.ll_send_conn_adv_cnt ++;           // adv in conn state counter
    g_pmCounters.ll_send_scan_adv_cnt ++;
    #ifdef DEBUG_LL
    LOG("c%d ", ch_idx);
    #endif
    return TRUE;
}  /*  end of function  */


// add in A2
/*******************************************************************************
    @fn          llSetupSecNonConnectableAdvEvt

    @brief       This function process for Nonconnectable Advertising.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      uint8 - TRUE(send success), FALSE(send fail).
*/
uint8 llSetupSecNonConnectableAdvEvt0( void )
{
    uint32_t  ch_idx;
    // Hold off interrupts.
    HAL_ENTER_CRITICAL_SECTION( );

    if (llWaitingIrq)
    {
        g_pmCounters.ll_tbd_cnt1++;
        HAL_EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    // next adv channel invalid, get 1st adv chn
    if (adv_param.advNextChan > LL_ADV_CHAN_LAST || adv_param.advNextChan < LL_ADV_CHAN_FIRST)
        adv_param.advNextChan =  llGetNextAdvChn(0);     // get 1st adv chn

    ch_idx = adv_param.advNextChan;
    adv_param.advNextChan = llGetNextAdvChn(adv_param.advNextChan);
    //============== configure and trigger LL HW engine, LL HW work in Single Tx mode  ==================
    set_crc_seed(ADV_CRC_INIT_VALUE);     // crc seed for adv is same for all channels
    set_access_address(ADV_SYNCH_WORD);   // access address
    set_channel(ch_idx);             // channel
    set_whiten_seed(ch_idx);         // whiten seed
    set_max_length(0xff);            // rx PDU max length
    ll_hw_set_trx_settle(pGlobal_config[LL_HW_BB_DELAY_ADV],
                         pGlobal_config[LL_HW_AFE_DELAY_ADV],
                         pGlobal_config[LL_HW_PLL_DELAY_ADV]);        //TxBB,RxAFE,PLL
    // reset Rx/Tx FIFO
    ll_hw_rst_rfifo();
    ll_hw_rst_tfifo();
    ll_hw_set_stx();                      // set LL HW as Tx - Rx mode
    ll_hw_ign_rfifo(LL_HW_IGN_ALL);     //set the rfifo ign control
    //write Tx FIFO
    ll_hw_write_tfifo((uint8*)&(g_tx_adv_buf.txheader), ((g_tx_adv_buf.txheader & 0xff00) >> 8) + 2);
    ll_hw_go();
    llWaitingIrq = TRUE;
    HAL_EXIT_CRITICAL_SECTION();
    g_pmCounters.ll_send_conn_adv_cnt ++;             // adv in conn state counter
    g_pmCounters.ll_send_nonconn_adv_cnt ++;
    #ifdef DEBUG_LL
    LOG("nc%d ", ch_idx);
    #endif
    return TRUE;
}  /*  end of function  */

/*******************************************************************************
    @fn          llSecAdvAllow

    @brief       Decision the remain time to next LL conn interval is enough
                for no_conn adv, consider the time for no conn advertisement
                in channel 37/38/39. Plus some margin.



    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      TRUE:  Secondary advertisement allow
                FALSE: Secondary advertisement NOT allow
*/
uint8 llSecAdvAllow0(void)
{
    uint32 advTime, margin;
    uint32 remainTime;
    uint8 ret = FALSE;
    // Hold off interrupts.
    HAL_ENTER_CRITICAL_SECTION( );
    // read global config to get advTime and margin
    advTime = pGlobal_config[LL_NOCONN_ADV_EST_TIME];
    margin = pGlobal_config[LL_NOCONN_ADV_MARGIN];
    // remain time before trigger LL HW
    remainTime = read_LL_remainder_time();

    if ((remainTime > advTime + margin)
            && !llWaitingIrq)
        ret = TRUE;
    else
    {
        llSecondaryState = LL_SEC_STATE_ADV_PENDING;
        g_pmCounters.ll_conn_adv_pending_cnt ++;
    }

    HAL_EXIT_CRITICAL_SECTION();
    return ret;
}

/*******************************************************************************
    @fn          llCalcMaxScanTime

    @brief       Decide the maximum scan time, consider remain time and
                the margin


    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      allow maximum scan time in us

*/
uint32 llCalcMaxScanTime0(void)
{
    uint32 margin, scanTime;
    uint32 remainTime;
    margin = pGlobal_config[LL_SEC_SCAN_MARGIN];
    // Hold off interrupts.
    HAL_ENTER_CRITICAL_SECTION( );
    // remain time before trigger LL HW
    remainTime = read_LL_remainder_time();
    scanTime = 0;

    if (remainTime > margin + pGlobal_config[LL_MIN_SCAN_TIME]
            && !llWaitingIrq)
        scanTime = remainTime - margin;

    HAL_EXIT_CRITICAL_SECTION();
    return (scanTime);
}

/*******************************************************************************
    @fn          llSetupSecScan

    @brief       This function readies the device as a Scanner.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
void llSetupSecScan0( uint8 chan )
{
    uint32 scanTime;
    // Hold off interrupts.
    HAL_ENTER_CRITICAL_SECTION( );
    scanTime = scanInfo.scanWindow * 625;

//  if(llWaitingIrq)
//  {
//      LOG("==== error, mode: %d\n", scanInfo.scanMode);
//  }

    if (llState == LL_STATE_IDLE)
    {
        llState = LL_STATE_SCAN;
        llSecondaryState = LL_SEC_STATE_IDLE;
    }
    else
    {
        // calculate scan time
        scanTime = llCalcMaxScanTime();

        if (scanTime)       // trigger scan
        {
            llSecondaryState = LL_SEC_STATE_SCAN;
        }
        else                // no enough time to scan, pending
        {
            llSecondaryState = LL_SEC_STATE_SCAN_PENDING;
            g_pmCounters.ll_conn_scan_pending_cnt ++;
            HAL_EXIT_CRITICAL_SECTION(  );
            return;
        }
    }

    if (scanTime > scanInfo.scanWindow * 625)
        scanTime = scanInfo.scanWindow * 625;

    // reset all FIFOs; all data is forfeit
    ll_hw_rst_tfifo();
    ll_hw_rst_rfifo();
    set_crc_seed(ADV_CRC_INIT_VALUE); // crc seed for adv is same for all channels
    set_access_address(ADV_SYNCH_WORD);
    set_channel(chan);
    set_whiten_seed(chan);
    set_max_length(0xff);
    ll_hw_set_rx_timeout(scanTime);   // maximum scan time, note that actual scan time may exceed the limit if timer expiry when LL engine receiving a report
    ll_hw_set_srx();
    ll_hw_ign_rfifo(LL_HW_IGN_CRC|LL_HW_IGN_EMP);
    ll_hw_go();
    llScanT1 = read_current_fine_time();
    llWaitingIrq = TRUE;
    HAL_EXIT_CRITICAL_SECTION();
//    uint32 remainTime = read_LL_remainder_time();
//  LOG("<%d %d>", scanTime, remainTime);
    return;
}


/*******************************************************************************
    @fn          llSetupSecInit

    @brief       This function readies the device as a Scanner.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
void llSetupSecInit( uint8 chan )
{
    uint32 scanTime;
    // Hold off interrupts.
    HAL_ENTER_CRITICAL_SECTION(  );
    scanTime = scanInfo.scanWindow * 625;

    if (llState == LL_STATE_IDLE)
    {
        llState = LL_STATE_INIT;
        llSecondaryState = LL_SEC_STATE_IDLE;
    }
    else
    {
        // calculate scan time
        scanTime = llCalcMaxScanTime();

        if (scanTime)       // trigger scan
        {
            llSecondaryState = LL_SEC_STATE_INIT;
        }
        else                // no enough time to scan, pending
        {
            llSecondaryState = LL_SEC_STATE_INIT_PENDING;
//          g_pmCounters.ll_conn_scan_pending_cnt ++;
            HAL_EXIT_CRITICAL_SECTION(  );
            return;
        }
    }

    if (scanTime > initInfo.scanWindow * 625)
        scanTime = initInfo.scanWindow * 625;

    // reset all FIFOs; all data is forfeit
    ll_hw_rst_tfifo();
    ll_hw_rst_rfifo();
    set_crc_seed(ADV_CRC_INIT_VALUE); // crc seed for adv is same for all channels
    set_access_address(ADV_SYNCH_WORD);
    set_channel(chan);
    set_whiten_seed(chan);
    set_max_length(0xff);
    // A2 multi-conn
    ll_hw_set_rx_timeout(88);
//  ll_hw_set_rx_timeout_1st(88);
    ll_hw_set_rx_timeout(scanTime);   // maximum scan time, note that actual scan time may exceed the limit if timer expiry when LL engine receiving a report
    ll_hw_set_srx();
    ll_hw_ign_rfifo(LL_HW_IGN_CRC|LL_HW_IGN_EMP);
    ll_hw_go();
    llScanT1 = read_current_fine_time();
//hal_gpio_write(GPIO_P18, 1);
    llWaitingIrq = TRUE;
    HAL_EXIT_CRITICAL_SECTION(  );
    return;
}

uint8_t   ll_get_next_active_conn(uint8_t current_conn_id)
{
    int i;
    uint8_t next_conn_id = LL_INVALID_CONNECTION_ID;

    if (g_ll_conn_ctx.numLLConns != 0)
    {
        // get next active connection as current connection
        i = current_conn_id + 1;

        while (i < g_maxConnNum)
        {
            if (conn_param[i].active == TRUE)
                break;

            i ++;
        }

        if (i != g_maxConnNum)
            next_conn_id = i;
        else
        {
            for (i = 0; i <= current_conn_id; i ++)
            {
                if (conn_param[i].active == TRUE)
                    break;
            }

            if (i != current_conn_id + 1)
                next_conn_id = i;
        }
    }
    else
        return LL_INVALID_CONNECTION_ID;

    return next_conn_id;
}

uint32  ll_get_next_timer(uint8 current_conn_id)
{
    uint8  next;
    uint32 timer;
    next = ll_get_next_active_conn(current_conn_id);

    if (next == LL_INVALID_CONNECTION_ID)
        return 0;

    timer = (next > current_conn_id) ?
            ((next - current_conn_id) * g_ll_conn_ctx.per_slot_time * 625) :
            (((g_ll_conn_ctx.connInterval << 1) - (current_conn_id - next) * g_ll_conn_ctx.per_slot_time) * 625);
    return timer;
}
//#pragma O0


/*******************************************************************************
    @fn          ll_scheduler

    @brief       schedule next task, if current connection will be free, input
                parameter should be LL_INVALID_TIME. The function is invoked
                after old connection task end, it will not add new task but may
                delete exist task

    input parameters

    @param       time - schedule time for current connection

    output parameters

    @param       None.

    @return      None.
*/
void ll_scheduler0(uint32 time)
{
    uint32  T1, T2, delta, min, prio_adj;
    uint8   i, next, temp;
    T1 = read_current_fine_time();

    // timer1 is running, normally it should not occur
    if (isTimer1Running())
    {
//        LOG("=== ASSERT FAIL, timer1 running when invoke ll_scheduler ===\n");
        g_pmCounters.ll_evt_shc_err++;
        return;
    }

    // if timer1 is not running, calculate the time elapse since last timer expiry
    delta = g_ll_conn_ctx.current_timer + LL_TIME_DELTA(g_ll_conn_ctx.timerExpiryTick, T1) + pGlobal_config[TIMER_ISR_ENTRY_TIME];
    // update current context
    g_ll_conn_ctx.scheduleInfo[g_ll_conn_ctx.currentConn].remainder = time;      // if current conn terminal, the parameter "time" shall be LL_INVALID_TIME
    min = time;

    if (time == LL_INVALID_TIME)
    {
        ll_deleteTask(g_ll_conn_ctx.currentConn);
        g_ll_conn_ctx.currentConn = LL_INVALID_CONNECTION_ID;
    }

    next = g_ll_conn_ctx.currentConn;

    if (next != LL_INVALID_CONNECTION_ID)
    {
        // if we want master or slave connection has higher schedule priority, set LL_MASTER_PREEMPHASIS/LL_SLAVE_PREEMPHASIS
        if (g_ll_conn_ctx.scheduleInfo[next].linkRole == LL_ROLE_MASTER)
            min = (time > pGlobal_config[LL_MULTICONN_MASTER_PREEMP]) ? (time - pGlobal_config[LL_MULTICONN_MASTER_PREEMP]) : 0;

        if (g_ll_conn_ctx.scheduleInfo[next].linkRole == LL_ROLE_SLAVE)
            min = (time > pGlobal_config[LL_MULTICONN_SLAVE_PREEMP]) ? (time - pGlobal_config[LL_MULTICONN_SLAVE_PREEMP]) : 0;
    }

    // update schedule task list and get the earliest task
    for (i = 0; i < g_maxConnNum; i++)
    {
        if ((i != g_ll_conn_ctx.currentConn) && conn_param[i].active)
        {
            // task conflict process
            // if there is no enough time for new task, invoke relate slave/master conn event process function
//              if (g_ll_conn_ctx.scheduleInfo[i].remainder < delta + g_ll_conn_ctx.scheduleInfo[i].task_duration)
            if (g_ll_conn_ctx.scheduleInfo[i].remainder < delta + 40)     // 40 : margin for process delay, unit: us
            {
                // no enough time to process the event, regard the event as missed and update the conn context and timer
                uint8  ret = LL_PROC_LINK_KEEP;

                if (g_ll_conn_ctx.scheduleInfo[i].linkRole == LL_ROLE_MASTER)
                {
                    // temporary update g_ll_conn_ctx.currentConn to current connection ID because
                    // ll_processMissMasterEvt will invoke function using global variable g_ll_conn_ctx.currentConn
                    temp = g_ll_conn_ctx.currentConn;
                    g_ll_conn_ctx.currentConn = i;
                    ret = ll_processMissMasterEvt(i);
                    g_ll_conn_ctx.currentConn = temp;
                }
                else if (g_ll_conn_ctx.scheduleInfo[i].linkRole == LL_ROLE_SLAVE)
                {
                    // TODO: add by Zhangzhufei for Huapu, to check
                    // temporary update g_ll_conn_ctx.currentConn to current connection ID because
                    // ll_processMissSlaveEvt will invoke function using global variable g_ll_conn_ctx.currentConn
                    temp = g_ll_conn_ctx.currentConn;
                    g_ll_conn_ctx.currentConn = i;

                    if( delta > g_ll_conn_ctx.scheduleInfo[i].remainder )
                    {
                        llConnState_t* connPtr = &conn_param[i];
                        uint8 missCE = (( delta - g_ll_conn_ctx.scheduleInfo[i].remainder ) / connPtr->curParam.connInterval ) + 1;

                        for( uint8 misI = 0; misI < missCE ; misI++)
                            ll_processMissSlaveEvt(i);
                    }
                    else
                    {
                        ret = ll_processMissSlaveEvt(i);
                    }

                    g_ll_conn_ctx.currentConn = temp;
                }

                if (ret == LL_PROC_LINK_TERMINATE)    // the connection is terminated, update shcedule information.
                {
                    ll_deleteTask(i);
                    continue;                  // continue next link
                }

                // increase the task priority
                g_ll_conn_ctx.scheduleInfo[i].priority ++;

                if (g_ll_conn_ctx.scheduleInfo[i].priority == LL_SCH_PRIO_LAST)
                    g_ll_conn_ctx.scheduleInfo[i].priority = LL_SCH_PRIO_IMMED;
            }

            prio_adj =0;

            if (min != LL_INVALID_TIME)
            {
                // consider the task prioriy
                switch (g_ll_conn_ctx.scheduleInfo[i].priority)
                {
                case LL_SCH_PRIO_LOW:
                    prio_adj = 0;
                    break;

                case LL_SCH_PRIO_MED:
                    prio_adj = MAX(LL_TASK_MASTER_DURATION, LL_TASK_SLAVE_DURATION) + 2000;
                    break;

                case LL_SCH_PRIO_HIGH:
                    prio_adj = MAX(LL_TASK_MASTER_DURATION, LL_TASK_SLAVE_DURATION) + 10 + 2000;
                    break;

                case LL_SCH_PRIO_IMMED:
                    prio_adj = MAX(LL_TASK_MASTER_DURATION, LL_TASK_SLAVE_DURATION) + 20 + 2000;
                    break;

                default:
                    prio_adj = 0;
                    break;
                }

                if (g_ll_conn_ctx.scheduleInfo[i].linkRole == LL_ROLE_MASTER)
                    prio_adj += pGlobal_config[LL_MULTICONN_MASTER_PREEMP];

                if (g_ll_conn_ctx.scheduleInfo[i].linkRole == LL_ROLE_SLAVE)
                    prio_adj += pGlobal_config[LL_MULTICONN_SLAVE_PREEMP];
            }

            // update remainder time
            g_ll_conn_ctx.scheduleInfo[i].remainder -= delta;

            if (g_ll_conn_ctx.scheduleInfo[i].remainder < min + prio_adj)
            {
                next = i;
                min = (g_ll_conn_ctx.scheduleInfo[i].remainder > prio_adj) ? (g_ll_conn_ctx.scheduleInfo[i].remainder - prio_adj) : 0;
            }
        }
    }

    if (min == LL_INVALID_TIME)     // all task may be delete, not start timer
    {
//      LOG("=== all task delete ====\n");
        return;
    }

    T2 = read_current_fine_time();
    // calculate the time elapse since enter this function.
    delta = LL_TIME_DELTA(T1, T2);

    if (g_ll_conn_ctx.scheduleInfo[next].remainder < delta)          // TODO: should not go here, if this issue detected, root cause should be invest
    {
//      LOG("delta = %d\n", delta);
        set_timer(AP_TIM1,20);
        g_ll_conn_ctx.current_timer = 20;                        // add 2020-03-30
    }
    else
    {
        set_timer(AP_TIM1,g_ll_conn_ctx.scheduleInfo[next].remainder - delta);
        // update connection context & schedule info
        g_ll_conn_ctx.current_timer = g_ll_conn_ctx.scheduleInfo[next].remainder - delta;
    }

    g_ll_conn_ctx.currentConn = next;

    // set ll state according to current connection LL state
    if (g_ll_conn_ctx.scheduleInfo[g_ll_conn_ctx.currentConn].linkRole == LL_ROLE_SLAVE)
        llState = LL_STATE_CONN_SLAVE;
    else if (g_ll_conn_ctx.scheduleInfo[g_ll_conn_ctx.currentConn].linkRole == LL_ROLE_MASTER)
        llState = LL_STATE_CONN_MASTER;

    // the task is scheduled, set the priority as low
    g_ll_conn_ctx.scheduleInfo[g_ll_conn_ctx.currentConn].priority = LL_SCH_PRIO_LOW;

    // take into account the time between start timer1 and T1
    for (i = 0; i < g_maxConnNum; i++)
    {
        if (conn_param[i].active)
            g_ll_conn_ctx.scheduleInfo[i].remainder -= delta;
    }

//    LOG("<%x, %x> ", g_ll_conn_ctx.scheduleInfo[0].remainder, g_ll_conn_ctx.scheduleInfo[1].remainder);
//    LOG("<%x, %x, %x, %x> ", g_ll_conn_ctx.scheduleInfo[0].remainder, g_ll_conn_ctx.scheduleInfo[1].remainder,
//                             g_ll_conn_ctx.scheduleInfo[2].remainder, g_ll_conn_ctx.scheduleInfo[3].remainder);
}

//#define LL_ADD_TASK_MARGIN           3000
/*******************************************************************************
    @fn          ll_addTask

    @brief       when a connection is created, new task will be added to task list.
                There are 3 case:
                   1. the connection is the 1st connection
                   2. new task scheduler time earlier than current, re-schedule task
                   3. new task scheduler time later than current, keep current
                      task and add new task to shceduler list

    input parameters

    @param       connId  - connection ID
                time    - schedule time for new task

    output parameters

    @param       None.

    @return      None.
*/
void ll_addTask0(uint8 connId, uint32 time)
{
    uint32  T1, T2, delta, remainder, elapse_time;
    uint8   i;
    T1 = read_current_fine_time();
    // update current context
//    g_ll_conn_ctx.scheduleInfo[connId].task_period = time;
    g_ll_conn_ctx.scheduleInfo[connId].remainder = time;
    g_ll_conn_ctx.scheduleInfo[connId].priority = LL_SCH_PRIO_LOW;

    // case 1: the 1st task. Now only active link use timer1
    if (!isTimer1Running())
    {
        T2 = read_current_fine_time();
        delta = LL_TIME_DELTA(T1, T2);
        set_timer(AP_TIM1,time - delta);
        g_ll_conn_ctx.scheduleInfo[connId].remainder -= delta;
        g_ll_conn_ctx.currentConn = connId;
        g_ll_conn_ctx.current_timer = g_ll_conn_ctx.scheduleInfo[connId].remainder;
//          LOG("[case 1: %d, %d] ", g_ll_conn_ctx.currentConn, time - delta);
        return;
    }

    remainder = read_LL_remainder_time();

    // case 2: current awaiting task is earlier than new, not change the current task
    if (remainder + pGlobal_config[LL_CONN_TASK_DURATION] < time)              // new task has higher priority than old task, so add margin
    {
        T2 = read_current_fine_time();
        delta = LL_TIME_DELTA(T1, T2);
        // when current timer expiry, the timer in the task list will subtract the time elapse, compensate the new task to the same start point
        g_ll_conn_ctx.scheduleInfo[connId].remainder = (g_ll_conn_ctx.scheduleInfo[connId].remainder - delta) + (g_ll_conn_ctx.current_timer - remainder);
//          LOG("[case 2: %d, %d] ", g_ll_conn_ctx.currentConn, g_ll_conn_ctx.scheduleInfo[connId].remainder);
        return;
    }

    // case 3: current awaiting task is later than new, change the current task
//    elapse_time = ((CP_TIM1->LoadCount - CP_TIM1->CurrentCount) >> 2) + 5;      // 5: rough time from read old timer1 to kick new timer1
    elapse_time = g_ll_conn_ctx.current_timer - ((AP_TIM1->CurrentCount) >> 2) + 2;      // 2: rough time from read old timer1 to kick new timer1
    T2 = read_current_fine_time();
    // schedule the timer
    delta = LL_TIME_DELTA(T1, T2);
    set_timer(AP_TIM1,time - delta);
    g_ll_conn_ctx.current_timer = time - delta;

    // update task contexts
    for (i = 0; i < g_maxConnNum; i++)
    {
        if (i != connId && conn_param[i].active)
        {
            g_ll_conn_ctx.scheduleInfo[i].remainder -= elapse_time;
        }
    }

    g_ll_conn_ctx.scheduleInfo[connId].remainder -= delta;
    g_ll_conn_ctx.currentConn = connId;
//  LOG("[case 3: %d, %d] ", g_ll_conn_ctx.currentConn, time - delta);
    // update other values?
}


// delete task: when the link terminate, remove it from schdule task list
/*******************************************************************************
    @fn          ll_deleteTask

    @brief       delete task schedule info in the scheduler list.
                Note that connection parameters are reset in other functions.

    input parameters

    @param       connId  - connection ID

    output parameters

    @param       None.

    @return      None.
*/
void ll_deleteTask0(uint8 connId)
{
    g_ll_conn_ctx.scheduleInfo[connId].linkRole    = LL_ROLE_INVALID;
    g_ll_conn_ctx.scheduleInfo[connId].remainder   = LL_INVALID_TIME;
    g_ll_conn_ctx.scheduleInfo[connId].priority    = LL_SCH_PRIO_LOW;
    g_ll_conn_ctx.scheduleInfo[connId].task_duration = 0;
//  g_ll_conn_ctx.scheduleInfo[connId].task_period = 0;
//  g_ll_conn_ctx.scheduleInfo[connId].rsc_idx     = 0xFF
}

// temp set
#define LL_ADV_TIMING_COMPENSATE          5                 // rough compensation for the time not consider in calculation

#ifdef EXT_ADV_ENABLE
//#pragma O0
// adv scheduler - conn scheduler interaction functions
void ll_adv_scheduler0(void)
{
    uint32  T2, T3, delta;
    uint32  minAuxPduTime, minPriPduTime;
    uint8   minIndexAux, minIndexPri;
    uint8   done = FALSE;
    int i;
    llAdvScheduleInfo_t* p_scheduler = NULL;
    extAdvInfo_t*  pAdvInfo = NULL;
    // calculate elapse time since last timer trigger
    T2 = read_current_fine_time();
    delta = LL_TIME_DELTA(g_timerExpiryTick, T2);
    p_scheduler = &g_pAdvSchInfo[g_currentExtAdv];
    pAdvInfo = p_scheduler->pAdvInfo;

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
        ll_ext_adv_schedule_next_event(g_interAuxPduDuration - delta);
        done = TRUE;
    }

    // update scheduler task list
    ll_updateExtAdvRemainderTime(delta);

    if (done)               // next task schedule done
        return;

    // if current adv is not active, delete it from task list
    if (pAdvInfo->active == FALSE)
        ll_delete_adv_task(g_currentExtAdv);

    // case 2: finish broadcast in primary adv channel/aux adv channel. Check ext adv scheduler to get the earliest task
    // search the nearest expiry task and PDU type
    minAuxPduTime = g_pAdvSchInfo[g_currentExtAdv].auxPduRemainder;
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

    T3 = read_current_fine_time();
    delta = LL_TIME_DELTA(T2, T3);
    // compare the minimum AUX channel remainder time & minimum Primary channel PDU remainder time,
    // AUX channel PDU could add some pre-emphesis here
    uint32  auxPduEmphesis = pGlobal_config[LL_EXT_ADV_INTER_PRI_CHN_INT] * 3;                           // add 03-31

    if (minAuxPduTime != LL_INVALID_TIME
            && (minAuxPduTime < minPriPduTime + auxPduEmphesis))   // next schedule task is aux PDU
    {
        ll_ext_adv_schedule_next_event(minAuxPduTime - delta);
        g_currentExtAdv   = minIndexAux;
    }
    else   // next schedule task is pri PDU
    {
        ll_ext_adv_schedule_next_event(minPriPduTime - delta);
        g_currentExtAdv   = minIndexPri;
    }

    // update scheduler task list
    ll_updateExtAdvRemainderTime(delta);
}

void ll_add_adv_task0(extAdvInfo_t* pExtAdv)
{
    int i, spare;
    llAdvScheduleInfo_t* p_scheduler = NULL, *p_current_scheduler;
    uint32  T1 = 0, T2, delta, remainder, elapse_time = 0;
    uint8   chanNumber, temp;
    uint8   isLegacy = ll_isLegacyAdv(pExtAdv);
    temp = pExtAdv->parameter.priAdvChnMap;
    chanNumber = (temp & 0x01) + ((temp & 0x02) >> 1) + ((temp & 0x04) >> 2);
    // init new Ext adv event context
    pExtAdv->currentChn = ((temp & ((~temp) + 1)) >> 1) + 37;       // calculate 1st adv channel
    pExtAdv->adv_event_counter = 0;
    pExtAdv->currentAdvOffset = 0;
    pExtAdv->adv_event_duration = 0;

    // ======== case 1: the 1st adv task
    if (g_currentExtAdv == LL_INVALID_ADV_SET_HANDLE)     //    if (!isTimer4Running())
    {
        g_schExtAdvNum = 1;
        g_currentExtAdv = 0;          // scheduler index = 0
        p_current_scheduler = &g_pAdvSchInfo[g_currentExtAdv];

        // add 03-31
        if (isLegacy)            // Legacy Adv PDU has no aux PDU
        {
            g_pAdvSchInfo[g_currentExtAdv].auxPduRemainder = LL_INVALID_TIME;
        }
        else
            ll_allocAuxAdvTimeSlot(g_currentExtAdv);

//      LOG("[%d]  ", g_pAdvSchInfo[i].auxPduRemainder);

        if (llWaitingIrq)
        {
            g_currentAdvTimer = pGlobal_config[LL_CONN_TASK_DURATION];
        }
        else if (isTimer1Running()
                 && ((remainder = read_LL_remainder_time()) < pGlobal_config[LL_EXT_ADV_TASK_DURATION]))     // timer1 for connection or legacy adv
        {
            g_currentAdvTimer =  remainder + pGlobal_config[LL_CONN_TASK_DURATION];   // no enough time for adv case
        }
        else
        {
            if (chanNumber > 1)
                g_currentAdvTimer = pGlobal_config[LL_EXT_ADV_INTER_PRI_CHN_INT];
            else
                g_currentAdvTimer = (p_current_scheduler->auxPduRemainder < p_current_scheduler->pAdvInfo->primary_advertising_interval) ?
                                    p_current_scheduler->auxPduRemainder : p_current_scheduler->pAdvInfo->primary_advertising_interval;

            // invoke set up ext adv function
            llSetupExtAdvEvent(pExtAdv);
        }

        ll_ext_adv_schedule_next_event(g_currentAdvTimer);
        g_timerExpiryTick = read_current_fine_time();                 // fake timer expiry tick
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
    if (isLegacy)
    {
        g_pAdvSchInfo[spare].auxPduRemainder = LL_INVALID_TIME;
    }
    else
        ll_allocAuxAdvTimeSlot(spare);

    if (isTimer4Running())
    {
        remainder = read_ll_adv_remainder_time();
        T1   = read_current_fine_time();
        // calculate the elapse time since current adv timer trigger
        elapse_time = g_currentAdvTimer - remainder;
    }
    else
        remainder = 0;

    if (isTimer1Running())
    {
        uint32 temp = read_LL_remainder_time();
        remainder = (temp < remainder) ? temp : remainder;
    }

    if (llWaitingIrq)           // there is ongoing LL IRQ, not setup adv
        remainder = 0;

    // case 2.1: there is enough time for EXT_ADV_IND, setup new adv first
    if (remainder > pGlobal_config[LL_EXT_ADV_TASK_DURATION])
    {
        g_currentExtAdv = spare;

        if (chanNumber > 1)
            g_currentAdvTimer = pGlobal_config[LL_EXT_ADV_INTER_PRI_CHN_INT];

        ll_ext_adv_schedule_next_event(g_currentAdvTimer);
        // setup ext adv event
        llSetupExtAdvEvent(pExtAdv);
        T2 = read_current_fine_time();
        delta = LL_TIME_DELTA(T1, T2);
        p_current_scheduler->nextEventRemainder = p_current_scheduler->pAdvInfo->primary_advertising_interval;  // add some random delay between 0-10ms?

        // update the timer in the scheduler info list
        for (i = 0; i < g_extAdvNumber; i++)
        {
            if (g_pAdvSchInfo[i].adv_handler != LL_INVALID_ADV_SET_HANDLE &&
                    i != g_currentExtAdv)
            {
                if (g_pAdvSchInfo[i].nextEventRemainder < (elapse_time + delta))
                {
                    g_pAdvSchInfo[i].nextEventRemainder += g_pAdvSchInfo[i].pAdvInfo->primary_advertising_interval;
                    g_pAdvSchInfo[i].pAdvInfo->adv_event_duration += g_pAdvSchInfo[i].pAdvInfo->primary_advertising_interval;
                    g_pAdvSchInfo[i].pAdvInfo->adv_event_counter ++;
                }

                if (g_pAdvSchInfo[i].auxPduRemainder < (elapse_time + delta))           // normally this case should not occur
                    g_pAdvSchInfo[i].auxPduRemainder += g_advSlotPeriodic;

                g_pAdvSchInfo[i].nextEventRemainder -= (elapse_time + delta);

                if (g_pAdvSchInfo[i].auxPduRemainder != LL_INVALID_TIME)
                    g_pAdvSchInfo[i].auxPduRemainder    -= (elapse_time + delta);
            }
        }
    }
    // case 2.2: no enough time, start new adv after current one
    else
    {
        // add new adv to adv scheduler list, not change current adv task
        p_scheduler->nextEventRemainder = p_current_scheduler->nextEventRemainder
                                          + (spare > g_currentExtAdv ? (spare - g_currentExtAdv) : (g_extAdvNumber + spare - g_currentExtAdv)) * pGlobal_config[LL_EXT_ADV_TASK_DURATION];
    }

    p_scheduler->adv_handler = pExtAdv->advHandle;
    p_scheduler->pAdvInfo    = pExtAdv;
    g_schExtAdvNum ++;
}

// TODO: function split between ll_adv_scheduler() & ll_delete_adv_task
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
    if (g_schExtAdvNum == 0)
    {
        clear_timer(AP_TIM4);
        return;
    }

    // current awaiting adv is disable, and there are more than 1 task
    if (index == g_currentExtAdv)
    {
        remainder = read_ll_adv_remainder_time();
        T1  = read_current_fine_time();
        elapse_time = g_currentAdvTimer - remainder;
        // find the earliest task
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
//          g_currentAdvTimer = minAuxPduTime - elapse_time - delta;
            g_currentExtAdv   = minIndexAux;
        }
        else   // next schedule task is pri PDU
        {
            ll_ext_adv_schedule_next_event(minPriPduTime - elapse_time - delta);
//          g_currentAdvTimer = minPriPduTime - elapse_time - delta;
            g_currentExtAdv   = minIndexPri;
        }

        // update the scheduler list
        ll_updateExtAdvRemainderTime(elapse_time + delta);
    }
}
#else
void ll_adv_scheduler0(void)
{
}

void ll_add_adv_task0(extAdvInfo_t* pExtAdv)
{
	(void) pExtAdv;
}

void ll_delete_adv_task0(uint8 index)
{
	(void) index;
}

#endif

// get remainder adv time
uint32  read_ll_adv_remainder_time(void)
{
    uint32 currentCount;
    currentCount = AP_TIM4->CurrentCount;
    currentCount = currentCount >> 2;    // convert to us
    return currentCount;
}


/*******************************************************************************
    @fn          llSetupExtAdvEvent

    @brief       This function will setup ext adv event
                1. fill ext adv pdu(EXT_ADV_IND or AUX_XXX_IND)
                2. update timer info for next chn EXT_ADV_IND or AUX_XXX_IND

    input parameters

    @param       None.

    output== parameters

    @param       None.

    @return      LL_STATUS_SUCCESS
*/
uint8 llSetupExtAdvEvent0(extAdvInfo_t*  pAdvInfo)
{
    uint8 ch_idx, pktFmt, auxPduIndFlag = FALSE;
    int i;
    uint32 T2, T1, delta, temp;
//  hal_gpio_write(GPIO_P14, 1);
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
            if (g_pAdvSchInfo[g_currentExtAdv].auxPduRemainder > g_pAdvSchInfo[g_currentExtAdv].nextEventRemainder)
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
        if (pAdvInfo->currentAdvOffset == 0)    // 1st AUX PDU. AUX_ADV_IND should include advData
        {
            llSetupAuxAdvIndPDU(pAdvInfo, NULL);
            auxPduIndFlag = TRUE;
        }
        else
            llSetupAuxChainIndPDU(pAdvInfo, NULL);

        // config secondary PHY
        if (pAdvInfo->parameter.secondaryAdvPHY == 3)           // coded PHY
            pktFmt = PKT_FMT_BLR125K;          //
        else
            pktFmt = pAdvInfo->parameter.secondaryAdvPHY;
    }

    HAL_ENTER_CRITICAL_SECTION();

    // if there is ongoing LL HW task, skip this task
    if (llWaitingIrq)
    {
//      g_pmCounters.ll_tbd_cnt1++;
        HAL_EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    //============== configure and trigger LL HW engine, LL HW work in Single Tx mode  ==================
    rf_phy_change_cfg(pktFmt);
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
        ll_hw_set_trx_settle(pGlobal_config[LL_HW_BB_DELAY_ADV],
                             pGlobal_config[LL_HW_AFE_DELAY_ADV],
                             pGlobal_config[LL_HW_PLL_DELAY_ADV]);        //TxBB,RxAFE,PLL
        ll_hw_set_trx();                      // set LL HW as Tx - Rx mode
    }
    else
    {
        ll_hw_set_stx();                      // set LL HW as Tx - Rx mode
    }

    ll_hw_ign_rfifo(LL_HW_IGN_ALL);       //set the rfifo ign control
    //write Tx FIFO
    ll_hw_write_tfifo((uint8*)&(g_tx_ext_adv_buf.txheader), ((g_tx_ext_adv_buf.txheader & 0xff00) >> 8) + 2);
    T2 = read_current_fine_time();
    delta = LL_TIME_DELTA(T1, T2);
    temp = ( pGlobal_config[LL_EXT_ADV_PROCESS_TARGET] > delta) ? (pGlobal_config[LL_EXT_ADV_PROCESS_TARGET] - delta) : 0;
    llWaitUs(temp);             // insert delay to make process time equal PROCESS_TARGET
    ll_hw_go();
//  hal_gpio_write(GPIO_P14, 1);
//  hal_gpio_write(GPIO_P14, 0);
    llWaitingIrq = TRUE;
    llTaskState = LL_TASK_EXTENDED_ADV;
    HAL_EXIT_CRITICAL_SECTION();
    return TRUE;
}


// -------------------------------------
#ifdef PRD_ADV_ENABLE
void ll_adv_scheduler_periodic0(void)
{
    uint32  T2, T3, delta;
    uint32  minAuxPduTime, minPriPduTime;
    uint8   minIndexAux, minIndexPri;
    uint8   done = FALSE;
    int i;
    llPeriodicAdvScheduleInfo_t* p_scheduler = NULL;
    extAdvInfo_t*  pAdvInfo = NULL;
    periodicAdvInfo_t* pPrdAdv = NULL;
    // calculate elapse time since last timer trigger
    T2 = read_current_fine_time();
    delta = LL_TIME_DELTA(g_timerExpiryTick, T2);
    p_scheduler = &g_pAdvSchInfo_periodic[g_currentExtAdv_periodic];
    pPrdAdv = p_scheduler->pAdvInfo_prd;
    pAdvInfo = p_scheduler->pAdvInfo;

    // case 1: not finish primary ADV channel or AUX_XXX_IND chain, continue advertise current adv task
    if (pPrdAdv->currentChn > LL_ADV_CHAN_FIRST &&   // next channel in primary adv channel
            !ll_isFirstAdvChn(pAdvInfo->parameter.priAdvChnMap, pPrdAdv->currentChn))       // not the 1st primary ADV channel
    {
        ll_prd_adv_schedule_next_event(pGlobal_config[LL_EXT_ADV_INTER_PRI_CHN_INT] - delta);
//          LOG("<%d> ", pGlobal_config[LL_EXT_ADV_INTER_PRI_CHN_INT] - delta);
        done = TRUE;
    }
    else if (pPrdAdv->currentChn < LL_ADV_CHAN_FIRST && pAdvInfo->sendingAuxAdvInd == TRUE)    // extended adv part, sending aux PDU
    {
        ll_prd_adv_schedule_next_event(g_interAuxPduDuration - delta);
        done = TRUE;
    }
    else if (pPrdAdv->currentChn < LL_ADV_CHAN_FIRST    // period adv part
             &&  pPrdAdv->currentAdvOffset > 0)              // offset in adv data set greater than 0 means next PDU is AUX_CHAIN_IND PDU
    {
        ll_prd_adv_schedule_next_event(g_interAuxPduDuration - delta);
        done = TRUE;
    }

    // update scheduler task list
    for (i = 0; i < g_extAdvNumber; i++)
    {
        if (g_pAdvSchInfo_periodic[i].adv_handler != LL_INVALID_ADV_SET_HANDLE)
        {
            if (g_pAdvSchInfo_periodic[i].pAdvInfo->active == TRUE)
                g_pAdvSchInfo_periodic[i].nextEventRemainder -= delta;

            g_pAdvSchInfo_periodic[i].auxPduRemainder    -= delta;
        }
    }

    if (done)               // next task schedule done
        return;

    // case 2: finish broadcast in primary adv channel/aux adv channel. Check ext adv scheduler to get the earliest task
    // search the nearest expiry task and PDU type
    minAuxPduTime = g_pAdvSchInfo_periodic[g_currentExtAdv_periodic].auxPduRemainder;
    minPriPduTime = g_pAdvSchInfo_periodic[g_currentExtAdv_periodic].nextEventRemainder;
    minIndexAux = g_currentExtAdv_periodic;
    minIndexPri = g_currentExtAdv_periodic;

    for (i = 0; i < g_extAdvNumber; i++)
    {
        if (g_pAdvSchInfo_periodic[i].adv_handler != LL_INVALID_ADV_SET_HANDLE)
        {
            if (g_pAdvSchInfo_periodic[i].auxPduRemainder < minAuxPduTime)
            {
                minIndexAux = i;
                minAuxPduTime = g_pAdvSchInfo_periodic[i].auxPduRemainder;
            }

            if (g_pAdvSchInfo_periodic[i].pAdvInfo->active == TRUE && g_pAdvSchInfo_periodic[i].nextEventRemainder < minPriPduTime)
            {
                minIndexPri = i;
                minPriPduTime = g_pAdvSchInfo_periodic[i].nextEventRemainder;
            }
        }
    }

    T3 = read_current_fine_time();
    delta = LL_TIME_DELTA(T2, T3);

    if (minAuxPduTime < minPriPduTime)   // next schedule task is aux PDU
    {
        ll_prd_adv_schedule_next_event(minAuxPduTime - delta);
        g_currentExtAdv_periodic   = minIndexAux;
    }
    else   // next schedule task is pri PDU
    {
        ll_prd_adv_schedule_next_event(minPriPduTime - delta);
        g_currentExtAdv_periodic   = minIndexPri;
    }

    // update scheduler task list
    for (i = 0; i < g_extAdvNumber; i++)
    {
        if (g_pAdvSchInfo_periodic[i].adv_handler != LL_INVALID_ADV_SET_HANDLE)
        {
            if (g_pAdvSchInfo_periodic[i].pAdvInfo->active == TRUE)
                g_pAdvSchInfo_periodic[i].nextEventRemainder -= delta;

            g_pAdvSchInfo_periodic[i].auxPduRemainder    -= delta;
        }
    }
}



void ll_add_adv_task_periodic0(periodicAdvInfo_t* pPrdAdv, extAdvInfo_t* pExtAdv)
{
    int i, spare;
    llPeriodicAdvScheduleInfo_t* p_scheduler = NULL, *p_current_scheduler = NULL;
    uint32  T1 = 0, T2, delta, remainder, elapse_time = 0;
    uint8   chanNumber, temp;
    temp = pExtAdv->parameter.priAdvChnMap;
    chanNumber = (temp & 0x01) + ((temp & 0x02) >> 1) + ((temp & 0x04) >> 2);
    // init new Ext adv event context
    pPrdAdv->currentChn = ((temp & ((~temp) + 1)) >> 1) + 37;       // calculate 1st adv channel
    pPrdAdv->currentAdvOffset = 0;
    pPrdAdv->periodic_adv_event_counter = 0;
//  pPrdAdv->adv_interval = pPrdAdv->adv_interval_max;               // TODO: select property adv interval
    // parameters of primary adv channel
    pExtAdv->adv_event_duration = 0;
    pExtAdv->adv_event_counter  = 0;

    // case 1: the 1st task
    if (g_currentExtAdv_periodic == LL_INVALID_ADV_SET_HANDLE)
    {
        g_schExtAdvNum_periodic = 1;
        g_currentExtAdv_periodic = 0;          // scheduler index = 0
        p_current_scheduler = &g_pAdvSchInfo_periodic[g_currentExtAdv_periodic];
        // allocate auxilary PDU timeslot
        ll_allocAuxAdvTimeSlot_prd(g_currentExtAdv_periodic);
//      LOG("[%d]  ", p_current_scheduler->auxPduRemainder);
        p_current_scheduler->pAdvInfo     = pExtAdv;
        p_current_scheduler->pAdvInfo_prd = pPrdAdv;
        p_current_scheduler->adv_handler  = pExtAdv->advHandle;

        if (llWaitingIrq)
        {
            g_currentAdvTimer = pGlobal_config[LL_CONN_TASK_DURATION];
        }
        else if (isTimer1Running()
                 && ((remainder = read_LL_remainder_time()) < pGlobal_config[LL_PRD_ADV_TASK_DURATION]))     // timer1 for connection or legacy adv
        {
            g_currentAdvTimer =  remainder + pGlobal_config[LL_CONN_TASK_DURATION];   // no enough time for adv case
        }
        else
        {
            if (chanNumber > 1)
                g_currentAdvTimer = pGlobal_config[LL_EXT_ADV_INTER_PRI_CHN_INT];
            else    // only adv in 1 primary adv channel case
                g_currentAdvTimer = pGlobal_config[LL_EXT_ADV_PRI_2_SEC_CHN_INT];

//              g_currentAdvTimer = (p_current_scheduler->auxPduRemainder < p_current_scheduler->pAdvInfo->primary_advertising_interval) ?
//                                    p_current_scheduler->auxPduRemainder : p_current_scheduler->pAdvInfo->primary_advertising_interval;
            // invoke set up ext adv function
            llSetupPrdAdvEvent(pPrdAdv, pExtAdv);              /// TODO
        }

        ll_prd_adv_schedule_next_event(g_currentAdvTimer);
        g_timerExpiryTick = read_current_fine_time();                 // fake timer expiry tick
        p_current_scheduler->nextEventRemainder = p_current_scheduler->pAdvInfo->primary_advertising_interval;  // add some random delay between 0-10ms?
        pExtAdv->active = TRUE;
        return;
    }

    // case 2: there are ongoing adv, get the 1st spare scheduler slot
    p_current_scheduler = &g_pAdvSchInfo_periodic[g_currentExtAdv_periodic];

    for (i = 0; i < g_schExtAdvNum_periodic; i++)
    {
        if (g_pAdvSchInfo_periodic[i].adv_handler == LL_INVALID_ADV_SET_HANDLE)
        {
            p_scheduler = &g_pAdvSchInfo_periodic[i];
            spare = i;
            break;
        }
    }

    // no empty scheduler slot, return
    if (p_scheduler == NULL)
    {
        // to add error counter here
        return;
    }

    g_schExtAdvNum_periodic ++;
    pExtAdv->active = TRUE;
    // arrange the timing of AUX_XXX_IND, it is independent to EXT_ADV_IND
    ll_allocAuxAdvTimeSlot_prd(spare);

    if (isTimer4Running())
    {
        remainder = read_ll_adv_remainder_time();
        T1   = read_current_fine_time();
        // calculate the elapse time since current adv timer trigger
        elapse_time = g_currentAdvTimer - remainder;
    }
    else
        remainder = 0;

    if (isTimer1Running())
    {
        uint32 temp = read_LL_remainder_time();
        remainder = (temp < remainder) ? temp : remainder;
    }

    if (llWaitingIrq)           // there is ongoing LL IRQ, not setup adv
        remainder = 0;

    // case 2.1: there is enough time for EXT_ADV_IND, setup new adv first
    if (remainder > pGlobal_config[LL_EXT_ADV_TASK_DURATION])
    {
        g_currentExtAdv = spare;

        if (chanNumber > 1)
            g_currentAdvTimer = pGlobal_config[LL_EXT_ADV_INTER_PRI_CHN_INT];

        ll_prd_adv_schedule_next_event(g_currentAdvTimer);
        // setup ext adv event
        llSetupPrdAdvEvent(pPrdAdv, pExtAdv);
        T2 = read_current_fine_time();
        delta = LL_TIME_DELTA(T1, T2);
        p_current_scheduler->nextEventRemainder = p_current_scheduler->pAdvInfo->primary_advertising_interval;  // add some random delay between 0-10ms?

        // update the timer in the scheduler info list
        for (i = 0; i < g_extAdvNumber; i++)
        {
            if (g_pAdvSchInfo_periodic[i].adv_handler != LL_INVALID_ADV_SET_HANDLE &&
                    i != g_currentExtAdv_periodic)
            {
                if (g_pAdvSchInfo_periodic[i].pAdvInfo->active == TRUE)
                    g_pAdvSchInfo_periodic[i].nextEventRemainder -= (elapse_time + delta);

                g_pAdvSchInfo_periodic[i].auxPduRemainder    -= (elapse_time + delta);
            }
        }
    }
    // case 2.2: no enough time, start new adv after current one
    else
    {
        // add new adv to adv scheduler list, not change current adv task
        p_scheduler->nextEventRemainder = p_current_scheduler->nextEventRemainder
                                          + (spare > g_currentExtAdv_periodic ?
                                             (spare - g_currentExtAdv_periodic) :
                                             (g_extAdvNumber + spare - g_currentExtAdv_periodic)) * pGlobal_config[LL_PRD_ADV_TASK_DURATION];
    }

    p_current_scheduler->pAdvInfo = pExtAdv;
    p_current_scheduler->pAdvInfo_prd = pPrdAdv;
    p_current_scheduler->adv_handler = pExtAdv->advHandle;
    g_schExtAdvNum_periodic ++;
}


void ll_delete_adv_task_periodic0(uint8 index)
{
    uint32  T1, T2, delta, remainder, elapse_time;
    uint32  minAuxPduTime, minPriPduTime;
    uint8   minIndexAux, minIndexPri;
    int i;
//  LOG("=== adv task % deleted \r\n", index);
    g_pAdvSchInfo_periodic[index].adv_handler        = LL_INVALID_ADV_SET_HANDLE;
    g_pAdvSchInfo_periodic[index].pAdvInfo           = NULL;
    g_pAdvSchInfo_periodic[index].auxPduRemainder    = LL_INVALID_TIME;
    g_pAdvSchInfo_periodic[index].nextEventRemainder = LL_INVALID_TIME;
    g_schExtAdvNum_periodic --;

    // only 1 task case, clear scheduler info and stop timer
    if (g_schExtAdvNum_periodic == 0)
    {
        clear_timer(AP_TIM4);
        return;
    }

    // current awaiting adv is disable, and there are more than 1 task
    if (index == g_currentExtAdv_periodic)
    {
        remainder = read_ll_adv_remainder_time();
        T1  = read_current_fine_time();
        elapse_time = g_currentAdvTimer - remainder;
        // find the earliest task
        minAuxPduTime = g_pAdvSchInfo_periodic[g_currentExtAdv_periodic].auxPduRemainder;      // LL_INVALID_TIME
        minPriPduTime = g_pAdvSchInfo_periodic[g_currentExtAdv_periodic].nextEventRemainder;
        minIndexAux = g_currentExtAdv_periodic;
        minIndexPri = g_currentExtAdv_periodic;

        for (i = 0; i < g_extAdvNumber; i++)
        {
            if (g_pAdvSchInfo_periodic[i].adv_handler != LL_INVALID_ADV_SET_HANDLE)
            {
                if (g_pAdvSchInfo_periodic[i].auxPduRemainder < minAuxPduTime)
                {
                    minIndexAux = i;
                    minAuxPduTime = g_pAdvSchInfo_periodic[i].auxPduRemainder;
                }

                if (g_pAdvSchInfo_periodic[i].nextEventRemainder < minPriPduTime)
                {
                    minIndexPri = i;
                    minPriPduTime = g_pAdvSchInfo_periodic[i].nextEventRemainder;
                }
            }
        }

        // start new timer
        T2  = read_current_fine_time();
        delta = LL_TIME_DELTA(T1, T2);

        if (minAuxPduTime < minPriPduTime)   // next schedule task is aux PDU
        {
            ll_prd_adv_schedule_next_event(minAuxPduTime - elapse_time - delta);
//          g_currentAdvTimer = minAuxPduTime - elapse_time - delta;
            g_currentExtAdv_periodic   = minIndexAux;
        }
        else   // next schedule task is pri PDU
        {
            ll_prd_adv_schedule_next_event(minPriPduTime - elapse_time - delta);
//          g_currentAdvTimer = minPriPduTime - elapse_time - delta;
            g_currentExtAdv_periodic   = minIndexPri;
        }

        // update the scheduler list
        for (i = 0; i < g_extAdvNumber; i++)
        {
            if (g_pAdvSchInfo_periodic[i].adv_handler != LL_INVALID_ADV_SET_HANDLE)
            {
                g_pAdvSchInfo_periodic[i].auxPduRemainder -= (elapse_time + delta);

                if (g_pAdvSchInfo_periodic[i].pAdvInfo->active == TRUE)
                    g_pAdvSchInfo_periodic[i].nextEventRemainder -= (elapse_time + delta);
            }
        }
    }
}
#else
void ll_adv_scheduler_periodic0(void)
{
}

void ll_add_adv_task_periodic0(periodicAdvInfo_t* pPrdAdv, extAdvInfo_t* pExtAdv)
{
	(void) pPrdAdv;
	(void) pExtAdv;
}

void ll_delete_adv_task_periodic0(uint8 index)
{
	(void) index;
}

#endif

/*******************************************************************************
    @fn          llSetupPrdAdvEvent

    @brief       This function will setup ext adv event
                1. fill ext adv pdu(EXT_ADV_IND or AUX_XXX_IND)
                2. update timer info for next chn EXT_ADV_IND or AUX_XXX_IND

    input parameters

    @param       None.

    output== parameters

    @param       None.

    @return      LL_STATUS_SUCCESS
*/
uint8 llSetupPrdAdvEvent0(periodicAdvInfo_t* pPrdAdv, extAdvInfo_t* pExtAdv)
{
    uint8 ch_idx, pktFmt;
    int i;
    uint32  crcInit, accessAddress;
    uint32 T2, T1, delta, temp;
    ch_idx = pPrdAdv->currentChn;
    T1 = read_current_fine_time();
    //LOG("%d ", pPrdAdv->currentChn);
    crcInit = ADV_CRC_INIT_VALUE;
    accessAddress  = ADV_SYNCH_WORD;

    if (ch_idx >= LL_ADV_CHAN_FIRST && ch_idx <= LL_ADV_CHAN_LAST )   // primary channel case
    {
        llSetupAdvExtIndPDU(pExtAdv, pPrdAdv);
        // decide next adv channel
        i = ch_idx - LL_ADV_CHAN_FIRST + 1;

        while ((i < 3) && !(pExtAdv->parameter.priAdvChnMap & (1 << i))) i ++;       // search channel map for next adv channel number

        if (i == 3)   // finish primary adv channel broadcast
        {
            pPrdAdv->currentChn = pExtAdv->auxChn;
            pExtAdv->sendingAuxAdvInd = TRUE;
        }
        else
            pPrdAdv->currentChn = LL_ADV_CHAN_FIRST + i;

        // config primary PHY
        pktFmt = pExtAdv->parameter.primaryAdvPHY;
    }
    else            // aux channel case
    {
        // branch 1,  send AUX_ADV_IND
        if (pExtAdv->sendingAuxAdvInd == TRUE)
        {
            llSetupAuxAdvIndPDU(pExtAdv, pPrdAdv);
            pExtAdv->sendingAuxAdvInd = FALSE;
        }
        // branch 2, send AUX_SYNC_IND/AUX_CHAIN_IND
        else if (pPrdAdv->currentAdvOffset == 0)    // 1st AUX PDU. AUX_ADV_IND should include advData
        {
            // set up AUX_SYNC_IND
            llSetupAuxSyncIndPDU(pExtAdv, pPrdAdv);
            crcInit = pPrdAdv->crcInit;
            accessAddress  = pPrdAdv->AA;
            pPrdAdv->periodic_adv_event_counter ++;
        }
        else    // set up AUX_CHAIN_IND
        {
            crcInit = pPrdAdv->crcInit;
            accessAddress  = pPrdAdv->AA;
            llSetupAuxChainIndPDU(pExtAdv, pPrdAdv);
        }

        // config secondary PHY
        if (pExtAdv->parameter.secondaryAdvPHY == 3)           // coded PHY
            pktFmt = PKT_FMT_BLR125K;          //
        else
            pktFmt = pExtAdv->parameter.secondaryAdvPHY;
    }

    HAL_ENTER_CRITICAL_SECTION();

    // if there is ongoing LL HW task, skip this task
    if (llWaitingIrq)
    {
//      g_pmCounters.ll_tbd_cnt1++;
        HAL_EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    //============== configure and trigger LL HW engine, LL HW work in Single Tx mode  ==================
    rf_phy_change_cfg(pktFmt);
    ll_hw_tx2rx_timing_config(pktFmt);
    set_crc_seed(crcInit);     // crc seed for adv is same for all channels
    set_access_address(accessAddress);   // access address
    set_channel(ch_idx);             // channel
    set_whiten_seed(ch_idx);         // whiten seed
//    set_max_length(50);            // rx PDU max length
    // reset Rx/Tx FIFO
    ll_hw_rst_rfifo();
    ll_hw_rst_tfifo();
    ll_hw_set_stx();                      // set LL HW as Tx - Rx mode
    ll_hw_ign_rfifo(LL_HW_IGN_ALL);       //set the rfifo ign control
    //write Tx FIFO
    ll_hw_write_tfifo((uint8*)&(g_tx_ext_adv_buf.txheader), ((g_tx_ext_adv_buf.txheader & 0xff00) >> 8) + 2);
    T2 = read_current_fine_time();
    delta = LL_TIME_DELTA(T1, T2);
    temp = ( pGlobal_config[LL_PRD_ADV_PROCESS_TARGET] > delta) ? (pGlobal_config[LL_PRD_ADV_PROCESS_TARGET] - delta) : 0;
    llWaitUs(temp);             // insert delay to make process time equal PROCESS_TARGET
    ll_hw_go();
//  hal_gpio_write(GPIO_P14, 1);
//  hal_gpio_write(GPIO_P14, 0);
    llWaitingIrq = TRUE;
    llTaskState = LL_TASK_PERIODIC_ADV;
    HAL_EXIT_CRITICAL_SECTION();
    return TRUE;
}



// -------------------------------------
#ifdef EXT_ADV_ENABLE
uint8 ll_processExtAdvIRQ(uint32_t      irq_status)
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
        packet_len = ll_hw_read_rfifo((uint8_t*)(&(g_rx_adv_buf.rxheader)),
                                      &pktLen,
                                      &pktFoot0,
                                      &pktFoot1);

        if(ll_hw_get_rfifo_depth()>0)
        {
            g_pmCounters.ll_rfifo_read_err++;
            packet_len=0;
            pktLen=0;
        }

        // check receive pdu type
        pdu_type = g_rx_adv_buf.rxheader & PDU_TYPE_MASK;
        txAdd    = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random

        if (pAdvInfo->parameter.ownAddrType == LL_DEV_ADDR_TYPE_RANDOM && pAdvInfo->parameter.isOwnRandomAddressSet == TRUE)
            ownAddr = pAdvInfo->parameter.ownRandomAddress;
        else
            ownAddr = ownPublicAddr;

        if (packet_len > 0                       // any better checking rule for rx anything?
                && pdu_type == ADV_SCAN_REQ
                && (pAdvInfo->parameter.advEventProperties == LL_EXT_ADV_PROP_ADV_IND
                    || pAdvInfo->parameter.advEventProperties == LL_EXT_ADV_PROP_ADV_SCAN_IND))
        {
            // 1. scan req
            g_pmCounters.ll_recv_scan_req_cnt ++;

            // check AdvA
            if (g_rx_adv_buf.data[6]  != ownAddr[0]
                    || g_rx_adv_buf.data[7]  != ownAddr[1]
                    || g_rx_adv_buf.data[8]  != ownAddr[2]
                    || g_rx_adv_buf.data[9]  != ownAddr[3]
                    || g_rx_adv_buf.data[10] != ownAddr[4]
                    || g_rx_adv_buf.data[11] != ownAddr[5])
            {
            }
            else
            {
                uint8_t  rpaListIndex;
                peerAddr = &g_rx_adv_buf.data[0];      // ScanA

                // Resolving list checking
                if (g_llRlEnable == TRUE          &&
                        txAdd == LL_DEV_ADDR_TYPE_RANDOM &&
                        (g_rx_adv_buf.data[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR)
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
                if ((pGlobal_config[LL_SWITCH] & LL_WHITELIST_ALLOW)
                        && (adv_param.wlPolicy  == LL_ADV_WL_POLICY_WL_SCAN_REQ
                            || adv_param.wlPolicy  == LL_ADV_WL_POLICY_WL_ALL_REQ)
                        && (bWlRlCheckOk == TRUE))
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
                        retScanRspFilter = 1;//LL_PLUS_ScanRequestFilterCBack();
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
                        delay = 118 - delay - calibra_time;                       // IFS = 150us, Tx tail -> Rx done time: about 32us
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
                peerAddr = &g_rx_adv_buf.data[0];        // initA

                // Resolving list checking
                if (g_llRlEnable == TRUE             &&
                        txAdd == LL_DEV_ADDR_TYPE_RANDOM   &&
                        (g_rx_adv_buf.data[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR)
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
                if ((pGlobal_config[LL_SWITCH] & LL_WHITELIST_ALLOW)
                        && (llState == LL_STATE_ADV_UNDIRECTED)
                        && (adv_param.wlPolicy   == LL_ADV_WL_POLICY_WL_CONNECT_REQ
                            || adv_param.wlPolicy  == LL_ADV_WL_POLICY_WL_ALL_REQ)
                        && (bWlRlCheckOk == TRUE))
                {
                    // check white list
                    bWlRlCheckOk = ll_isAddrInWhiteList(txAdd, peerAddr);
                }

                // fixed bug 2018-09-25, LL/CON/ADV/BV-04-C, for direct adv, initA should equal peer Addr
                if (llState == LL_STATE_ADV_DIRECTED)
                {
                    if (txAdd         != peerInfo.peerAddrType
                            || peerAddr[0]  != peerInfo.peerAddr[0]
                            || peerAddr[1]  != peerInfo.peerAddr[1]
                            || peerAddr[2]  != peerInfo.peerAddr[2]
                            || peerAddr[3]  != peerInfo.peerAddr[3]
                            || peerAddr[4]  != peerInfo.peerAddr[4]
                            || peerAddr[5]  != peerInfo.peerAddr[5])
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
                    osal_memcpy( peerInfo.peerAddr, g_rx_adv_buf.data, 6);
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
        uint8_t*  peerAddr;
        uint8_t  bWlRlCheckOk = TRUE;
        uint16_t pktLen;
        uint32_t pktFoot0, pktFoot1;
        int      calibra_time;                 // this parameter will be provided by global_config
        uint8*    ownAddr;
        ll_debug_output(DEBUG_LL_HW_TRX);
        // read packet
        packet_len = ll_hw_read_rfifo((uint8_t*)(&(g_rx_adv_buf.rxheader)),
                                      &pktLen,
                                      &pktFoot0,
                                      &pktFoot1);

        if(ll_hw_get_rfifo_depth() > 0)
        {
            g_pmCounters.ll_rfifo_read_err++;
            packet_len=0;
            pktLen=0;
        }

        // check receive pdu type
        pdu_type = g_rx_adv_buf.rxheader & PDU_TYPE_MASK;
        txAdd    = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random

        if (pAdvInfo->parameter.ownAddrType == LL_DEV_ADDR_TYPE_RANDOM && pAdvInfo->parameter.isOwnRandomAddressSet == TRUE)
            ownAddr = pAdvInfo->parameter.ownRandomAddress;
        else
            ownAddr = ownPublicAddr;

        if (packet_len > 0
                && pdu_type == ADV_AUX_SCAN_REQ
                && (pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_CONN_BITMASK
                    || pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_SCAN_BITMASK))
        {
            // 1. scan req
//            g_pmCounters.ll_recv_scan_req_cnt ++;

            // check AdvA
            if (g_rx_adv_buf.data[6]  != ownAddr[0]
                    || g_rx_adv_buf.data[7]  != ownAddr[1]
                    || g_rx_adv_buf.data[8]  != ownAddr[2]
                    || g_rx_adv_buf.data[9]  != ownAddr[3]
                    || g_rx_adv_buf.data[10] != ownAddr[4]
                    || g_rx_adv_buf.data[11] != ownAddr[5])
            {
            }
            else
            {
                uint8_t  rpaListIndex;
                peerAddr = &g_rx_adv_buf.data[0];      // ScanA

                // Resolving list checking
                if (g_llRlEnable == TRUE          &&
                        txAdd == LL_DEV_ADDR_TYPE_RANDOM &&
                        (g_rx_adv_buf.data[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR)
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
                if ((pGlobal_config[LL_SWITCH] & LL_WHITELIST_ALLOW)
                        && (adv_param.wlPolicy  == LL_ADV_WL_POLICY_WL_SCAN_REQ
                            || adv_param.wlPolicy  == LL_ADV_WL_POLICY_WL_ALL_REQ)
                        && (bWlRlCheckOk == TRUE))
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
                    calibra_time = pGlobal_config[SCAN_RSP_DELAY];            // consider rx_done to ISR time, SW delay after read_current_fine_time(), func read_current_fine_time() delay ...
                    delay = 118 - delay - calibra_time;                       // IFS = 150us, Tx tail -> Rx done time: about 32us
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
                    // =================== TODO: change the buffer to ext adv set
                    ll_hw_write_tfifo((uint8*)&(g_tx_ext_adv_buf.txheader),
                                      ((g_tx_ext_adv_buf.txheader & 0xff00) >> 8) + 2);   // payload length + header length(2)
                    ll_debug_output(DEBUG_LL_HW_SET_STX);
                    g_pmCounters.ll_send_scan_rsp_cnt ++;
                }
            }
        }
        else if (pdu_type == ADV_CONN_REQ
                 && (pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_CONN_BITMASK))
        {
            // 2. connect req
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
                peerAddr = &g_rx_adv_buf.data[0];        // initA

                // Resolving list checking
                if (g_llRlEnable == TRUE             &&
                        txAdd == LL_DEV_ADDR_TYPE_RANDOM   &&
                        (g_rx_adv_buf.data[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR)
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
                if ((pGlobal_config[LL_SWITCH] & LL_WHITELIST_ALLOW)
                        && (llState == LL_STATE_ADV_UNDIRECTED)
                        && (adv_param.wlPolicy   == LL_ADV_WL_POLICY_WL_CONNECT_REQ
                            || adv_param.wlPolicy  == LL_ADV_WL_POLICY_WL_ALL_REQ)
                        && (bWlRlCheckOk == TRUE))
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
                    // send scan rsp
                    ll_hw_set_stx();             // set LL HW as single Tx mode
                    g_same_rf_channel_flag = TRUE;
                    // calculate the delay
                    T2 = read_current_fine_time();
                    delay = (T2 > ISR_entry_time) ? (T2 - ISR_entry_time) : (BASE_TIME_UNITS - ISR_entry_time + T2);
                    calibra_time = pGlobal_config[SCAN_RSP_DELAY];            // consider rx_done to ISR time, SW delay after read_current_fine_time(), func read_current_fine_time() delay ...
                    delay = 118 - delay - calibra_time;                       // IFS = 150us, Tx tail -> Rx done time: about 32us
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
                    ll_hw_write_tfifo((uint8*)&(g_tx_adv_buf.txheader),
                                      ((g_tx_adv_buf.txheader & 0xff00) >> 8) + 2);   // payload length + header length(2)
                    ll_debug_output(DEBUG_LL_HW_SET_STX);
                    g_pmCounters.ll_send_conn_rsp_cnt ++;
//==============
                    // bug fixed 2018-01-23, peerAddrType should read TxAdd
                    peerInfo.peerAddrType = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random
                    osal_memcpy( peerInfo.peerAddr, g_rx_adv_buf.data, 6);
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
#else
uint8 ll_processExtAdvIRQ(uint32_t      irq_status)
{
	(void) irq_status;
    return TRUE;
}
#endif

#ifdef PRD_ADV_ENABLE
uint8 ll_processPrdAdvIRQ(uint32_t      irq_status)
{
    uint8              mode;
//  extAdvInfo_t       *pAdvInfo;
//  periodicAdvInfo_t  *pAdvInfo_prd;
//  uint32_t           T2, delay;
//  pAdvInfo     = g_pAdvSchInfo_periodic[g_currentExtAdv_periodic].pAdvInfo;
//  pAdvInfo_prd = g_pAdvSchInfo_periodic[g_currentExtAdv_periodic].pAdvInfo_prd;
//  if (pAdvInfo == NULL || pAdvInfo_prd == NULL)
//      return FALSE;
    HAL_ENTER_CRITICAL_SECTION();
    mode = ll_hw_get_tr_mode();

    if (mode == LL_HW_MODE_STX )
    {
    }

    // update scheduler list
    ll_adv_scheduler_periodic();

    if (!llWaitingIrq)
    {
        ll_hw_clr_irq();
        llTaskState = LL_TASK_OTHERS;
    }

    HAL_EXIT_CRITICAL_SECTION();
    return TRUE;
}
#else
uint8 ll_processPrdAdvIRQ(uint32_t      irq_status)
{
	(void) irq_status;
    return TRUE;
}
#endif

#ifdef EXT_SCAN_ENABLE
//#pragma O0
uint8 ll_processExtScanIRQ(uint32_t      irq_status)
{
    uint8         ll_mode, adv_mode, ext_hdr_len;
//  extScanInfo_t  *pScanInfo;
    uint32_t      T2, delay;
//  pScanInfo = &extScanInfo;
    HAL_ENTER_CRITICAL_SECTION();
    ll_mode = ll_hw_get_tr_mode();

    if (ll_mode == LL_HW_MODE_SRX)      // passive scan
    {
        uint8_t  rpaListIndex = LL_RESOLVINGLIST_ENTRY_NUM;
        uint8_t  bWlRlCheckOk = TRUE;
        uint8_t*  peerAddr = &ext_adv_hdr.advA[0];
        uint8   peerAddrType;                  // peer address type
        ll_debug_output(DEBUG_LL_HW_SRX);

        // ============= scan case
        if (llTaskState == LL_TASK_EXTENDED_SCAN)
        {
            uint8   bSendingScanReq = FALSE;
            memset(&ext_adv_hdr, 0, sizeof(ext_adv_hdr));

            // check status
            if ((irq_status & LIRQ_RD) && (irq_status & LIRQ_COK))
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
                pdu_type = g_rx_adv_buf.rxheader & 0x0f;

                if(ll_hw_get_rfifo_depth() > 0)
                {
                    g_pmCounters.ll_rfifo_read_err++;
                    packet_len = 0;
                    pktLen = 0;
                }

                if (packet_len   != 0
                        && (pdu_type == ADV_EXT_TYPE))
                {
                    uint8_t txAdd = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random
                    uint8   payload_len = (g_rx_adv_buf.rxheader & 0xFF00) >> LENGTH_SHIFT;
                    uint8   adv_data_len;
                    peerAddrType = txAdd;
                    adv_mode    = (g_rx_adv_buf.data[0] & 0xc0) >> 6;
                    ext_hdr_len =  g_rx_adv_buf.data[0] & 0x3f;
                    ll_parseExtHeader(&g_rx_adv_buf.data[1], payload_len - 1);
                    peerAddr = &ext_adv_hdr.advA[0];

                    // Resolving list checking
                    if (g_llRlEnable == TRUE
                            && (adv_mode== LL_EXT_ADV_MODE_CONN || adv_mode == LL_EXT_ADV_MODE_SC )   // BQB test only required checking RL for scanable ADV
                            && (ext_adv_hdr.header & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK))
                    {
                        // if ScanA is resolvable private address
                        if ((ext_adv_hdr.advA[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR)
                        {
                            bWlRlCheckOk = FALSE;
                            rpaListIndex = ll_getRPAListEntry(&ext_adv_hdr.advA[0]);

                            if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                            {
                                peerAddr = &g_llResolvinglist[rpaListIndex].peerAddr[0];
                                bWlRlCheckOk = TRUE;
                            }
                        }
                        else   // ScanA is device Identity, if the device ID in the RPA list, check whether RPA should be used
                        {
                            bWlRlCheckOk = TRUE;

                            for (int i = 0; i < LL_RESOLVINGLIST_ENTRY_NUM; i++)
                            {
                                if (ext_adv_hdr.advA[0] == g_llResolvinglist[i].peerAddr[0]
                                        && ext_adv_hdr.advA[1] == g_llResolvinglist[i].peerAddr[1]
                                        && ext_adv_hdr.advA[2] == g_llResolvinglist[i].peerAddr[2]
                                        && ext_adv_hdr.advA[3] == g_llResolvinglist[i].peerAddr[3]
                                        && ext_adv_hdr.advA[4] == g_llResolvinglist[i].peerAddr[4]
                                        && ext_adv_hdr.advA[5] == g_llResolvinglist[i].peerAddr[5])
                                {
                                    if (g_llResolvinglist[i].privacyMode == NETWORK_PRIVACY_MODE &&
                                            !ll_isIrkAllZero(g_llResolvinglist[i].peerIrk))
                                        bWlRlCheckOk = FALSE;

                                    break;
                                }
                            }
                        }
                    }

                    // check white list
                    if ((pGlobal_config[LL_SWITCH] & LL_WHITELIST_ALLOW)
                            && (extScanInfo.wlPolicy  == LL_SCAN_WL_POLICY_USE_WHITE_LIST)
                            && (bWlRlCheckOk == TRUE))
                    {
                        // check white list
                        bWlRlCheckOk = ll_isAddrInWhiteList(txAdd, peerAddr);
                    }

                    // if valid, trigger osal event to report adv
                    if (bWlRlCheckOk == TRUE)
                    {
                        uint8  advEventType = 0;
                        int8   rssi;

//                        llCurrentScanChn = extScanInfo.nextScanChan;

                        // active scan scenario, send scan req
                        if (extScanInfo.scanType[extScanInfo.current_index] == LL_SCAN_ACTIVE
                                && adv_mode == LL_EXT_ADV_MODE_SC )      // only scannable adv accept AUX_SCAN_REQ
                        {
                            // ========================== active scan path   ===================
                            // fill scanA, using RPA or device ID address
                            if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM &&
                                    !ll_isIrkAllZero(g_llResolvinglist[rpaListIndex].localIrk))
                            {
                                // for resolving private address case, calculate the scanA with Local IRK
                                ll_CalcRandomAddr(g_llResolvinglist[rpaListIndex].localIrk, &g_tx_adv_buf.data[0]);
                                g_tx_adv_buf.txheader |=(((g_rx_adv_buf.rxheader & TX_ADD_MASK) << 1)
                                                         | (LL_DEV_ADDR_TYPE_RANDOM << TX_ADD_SHIFT & TX_ADD_MASK));
                            }
                            else
                            {
                                memcpy((uint8*)&g_tx_adv_buf.data[0], &extScanInfo.ownAddr[0], 6);
                                g_tx_adv_buf.txheader |=(((g_rx_adv_buf.rxheader & TX_ADD_MASK) << 1)
                                                         | (extScanInfo.ownAddrType << TX_ADD_SHIFT & TX_ADD_MASK));
                            }

                            g_tx_adv_buf.txheader = 0xC03;
                            g_same_rf_channel_flag = TRUE;
                            ll_hw_set_tx_rx_interval(10);
                            ll_hw_set_rx_timeout(158);
                            set_max_length(0xFF);               // add 2020-03-10
                            T2 = read_current_fine_time();
                            delay = (T2 > ISR_entry_time) ? (T2 - ISR_entry_time) : (BASE_TIME_UNITS - ISR_entry_time + T2);
                            delay = 118 - delay - pGlobal_config[LL_ADV_TO_SCAN_REQ_DELAY];
                            ll_hw_set_trx();             // set LL HW as single TRx mode
                            ll_hw_set_trx_settle(delay,                               // set BB delay, about 80us in 16MHz HCLK
                                                 pGlobal_config[LL_HW_AFE_DELAY],
                                                 pGlobal_config[LL_HW_PLL_DELAY]);        //RxAFE,PLL
                            ll_hw_go();
                            g_pmCounters.ll_send_scan_req_cnt++;
                            llWaitingIrq = TRUE;
                            // reset Rx/Tx FIFO
                            ll_hw_rst_rfifo();
                            ll_hw_rst_tfifo();
                            ll_hw_ign_rfifo(LL_HW_IGN_CRC | LL_HW_IGN_EMP);
//                            //20181012 ZQ: change the txheader according to the adtype
//                            g_tx_adv_buf.txheader |=(((g_rx_adv_buf.rxheader & TX_ADD_MASK) << 1)
//                                                      | (extScanInfo.ownAddrType<< TX_ADD_SHIFT & TX_ADD_MASK));
                            // AdvA, for SCAN REQ, it should identical to the ADV_IND/ADV_SCAN_IND
                            g_tx_adv_buf.data[6]  = peerAddr[0];
                            g_tx_adv_buf.data[7]  = peerAddr[1];
                            g_tx_adv_buf.data[8]  = peerAddr[2];
                            g_tx_adv_buf.data[9]  = peerAddr[3];
                            g_tx_adv_buf.data[10] = peerAddr[4];
                            g_tx_adv_buf.data[11] = peerAddr[5];
                            //write Tx FIFO
                            ll_hw_write_tfifo((uint8*)&(g_tx_adv_buf.txheader),
                                              ((g_tx_adv_buf.txheader & 0xff00) >> 8) + 2);   // payload length + header length(2)
                            bSendingScanReq = TRUE;
                            g_same_rf_channel_flag = FALSE;
                            // ========================== active scan path end  ===================
                        }

                        adv_data_len = payload_len - ext_hdr_len - 1;
                        rssi  =  -(pktFoot1 >> 24);

                        if (adv_data_len > 0)
                        {
                            // copy the adv_data
                            LL_ExtAdvReportCback( advEventType,                         // event type
                                                  peerAddrType,                                // Adv address type (TxAdd)
                                                  &peerAddr[0],                         // Adv address (AdvA)
                                                  extScanInfo.scanPHYs[extScanInfo.current_index],  // primary PHY
                                                  extScanInfo.current_scan_PHY,          // secondary PHY
                                                  (ext_adv_hdr.adi & 0xF000) >> 12,
                                                  ext_adv_hdr.txPower,
                                                  rssi,
                                                  0,                                     // periodicAdvertisingInterval, reserved
                                                  0,
                                                  NULL,                                 // NULL for undirect adv report
                                                  pktLen - 8,                           // length of rest of the payload, 2 - header, 6 - advA
                                                  &g_rx_adv_buf.data[6]);                // rest of payload
                            g_pmCounters.ll_recv_adv_pkt_cnt ++;
                        }
                        else if (ext_adv_hdr.header & LE_EXT_HDR_SYNC_INFO_PRESENT_BITMASK)
                        {
                            LL_ExtAdvReportCback( 0,                            // event type
                                                  peerAddrType,                                // Adv address type (TxAdd)
                                                  &peerAddr[0],                         // Adv address (AdvA)
                                                  extScanInfo.scanPHYs[extScanInfo.current_index],  // primary PHY
                                                  extScanInfo.current_scan_PHY,          // secondary PHY
                                                  (ext_adv_hdr.adi & 0xF000) >> 12,
                                                  ext_adv_hdr.txPower,
                                                  rssi,
                                                  syncInfo.interval,                                     // periodicAdvertisingInterval, TO update
                                                  0,
                                                  NULL,                                 // NULL for undirect adv report
                                                  0,                                    // for periodic syncInfo, no adv data
                                                  &g_rx_adv_buf.data[6]);                // rest of payload
                        }
                    }
                }
                else
                {
                    // invalid ADV PDU type
//                    llSetupScan();
                }
            }

            // if not waiting for scan rsp, schedule next scan
            if (!bSendingScanReq)
            {
                uint8   bStartPeriodScan = FALSE;

                // =========== scanning periodic adv case
                if ((ext_adv_hdr.header & LE_EXT_HDR_SYNC_INFO_PRESENT_BITMASK)
                        && (scanSyncInfo.valid == TRUE))
                {
                    // create sync with periodic advertiser
                    if (scanSyncInfo.options & LL_PERIODIC_ADV_CREATE_SYNC_USING_ADV_LIST_BITMASK)
                    {
                        // using device list case, TO be added in future release
                        bStartPeriodScan = TRUE;
                    }
                    else
                    {
                        if (((ext_adv_hdr.adi >> 12) & 0x0F)  == scanSyncInfo.advertising_SID
                                && peerAddrType == scanSyncInfo.advertiser_Address_Type
                                && memcmp(&peerAddr[0], &scanSyncInfo.advertiser_Address[0], LL_DEVICE_ADDR_LEN) == 0)
                        {
                            // find the periodic sync info, sync with the period adv
                            bStartPeriodScan = TRUE;
                        }
                    }

                    if (bStartPeriodScan == TRUE)
                    {
                        uint16    sync_handler;
                        uint32    schedule_time;
                        uint32    accessAddress;
                        llPeriodicScannerInfo_t*  pPrdScanInfo;
                        sync_handler = llAllocateSyncHandle();
                        pPrdScanInfo = &g_llPeriodAdvSyncInfo[sync_handler];     // only 1 period scanner
                        // get elapse time since the AUX_ADV_IND start point
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
                        pPrdScanInfo->nextEventRemainder = ((syncInfo.offset.offsetUnit == 1) ? 300 : 30) * syncInfo.offset.syncPacketOffset;
                        pPrdScanInfo->syncEstOk = FALSE;
                        pPrdScanInfo->event1stFlag = TRUE;        // receiving the 1st PDU of a periodic event
                        // TODO
                        pPrdScanInfo->channelIdentifier = (( accessAddress & 0xFFFF0000 )>> 16 ) ^ ( accessAddress & 0x0000FFFF);
                        schedule_time = g_llPeriodAdvSyncInfo[sync_handler].nextEventRemainder - 2500;  // 1000: timing advance

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
                    }
                }

                // ============== scanning extended adv case
                // case 1: receives auxPtr, update PHY/chn according to AuxPtr, start timer according to aux offset
                if (bStartPeriodScan == FALSE)
                {
                    if (ext_adv_hdr.header & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK)
                    {
                        uint32    wait_time;
                        extScanInfo.current_chn = ext_adv_hdr.auxPtr.chn_idx;
                        extScanInfo.current_scan_PHY = ext_adv_hdr.auxPtr.aux_phy;
                        wait_time = ext_adv_hdr.auxPtr.aux_offset * ((ext_adv_hdr.auxPtr.offset_unit == 1) ? 300 : 30);

                        if (wait_time < 2000)
                            wait_time = 10;
                        else
                            wait_time -= 1000;     // scan advance

                        llScanTime = 0;
                        ll_ext_scan_schedule_next_event(wait_time);
                    }
                    // case 2: scanning aux channel, no more aux PDU, continue to scan in primary channel
                    else if (extScanInfo.current_chn < LL_SCAN_ADV_CHAN_37)
                    {
                        extScanInfo.current_chn = LL_SCAN_ADV_CHAN_37;

                        if (extScanInfo.numOfScanPHY > 1)
                        {
                            extScanInfo.current_index = (extScanInfo.current_index + 1) & 0x01;
                        }

                        extScanInfo.current_scan_PHY = extScanInfo.scanPHYs[extScanInfo.current_index];
                        llSetupExtScan(extScanInfo.current_chn);
                        llScanTime = 0;
                    }
                    // case 3: update scan time, only when scan primary channel and not receive AuxPtr
                    else
                    {
                        // not sending SCAN REQ, update scan time
                        llScanTime += ((ISR_entry_time > llScanT1) ? (ISR_entry_time - llScanT1) : (BASE_TIME_UNITS - llScanT1 + ISR_entry_time));

                        if (llScanTime >= extScanInfo.scanWindow[extScanInfo.current_index] * 625)
                        {
                            if (extScanInfo.numOfScanPHY > 1 && extScanInfo.current_chn == LL_SCAN_ADV_CHAN_39)
                            {
                                extScanInfo.current_index = (extScanInfo.current_index + 1) & 0x01;
                                extScanInfo.current_scan_PHY = extScanInfo.scanPHYs[extScanInfo.current_index];
                            }

                            LL_CALC_NEXT_SCAN_CHN(extScanInfo.current_chn);

                            // schedule next scan event
                            if (extScanInfo.scanWindow[extScanInfo.current_index] == extScanInfo.scanInterval[extScanInfo.current_index])      // scanWindow == scanInterval, trigger immediately
                                llSetupExtScan(extScanInfo.current_chn);
                            else
                                ll_ext_scan_schedule_next_event((extScanInfo.scanInterval[extScanInfo.current_index]
                                                                 - extScanInfo.scanWindow[extScanInfo.current_index]) * 625);

                            // reset scan total time
                            llScanTime = 0;
                        }
                        else
                        {
                            llSetupExtScan(extScanInfo.current_chn);
                        }
                    }
                }
            }
        }
        // ===========  initiator case
        else if (llTaskState == LL_TASK_EXTENDED_INIT)
        {
        }
    }
    else if (ll_mode == LL_HW_MODE_TRX && llTaskState == LL_TASK_EXTENDED_SCAN)            // active scan
    {
        if ((irq_status & LIRQ_RD) && (irq_status & LIRQ_COK))
        {
            // rx done
        }
    }

    if (!llWaitingIrq)
    {
        ll_hw_clr_irq();
        llTaskState = LL_TASK_OTHERS;
    }

    HAL_EXIT_CRITICAL_SECTION();
    return TRUE;
}
//#pragma O2

uint8 ll_processExtInitIRQ(uint32_t      irq_status)
{
    //uint8          ll_mode, adv_mode, ext_hdr_len;
    uint8          ll_mode, adv_mode;
    llConnState_t*  connPtr;
    uint32_t      T2, delay;
    HAL_ENTER_CRITICAL_SECTION();
    ll_mode = ll_hw_get_tr_mode();
//  hal_gpio_write(GPIO_P14, 1);
//  hal_gpio_write(GPIO_P14, 0);

    if (ll_mode == LL_HW_MODE_SRX)
    {
        uint8_t  bWlRlCheckOk = TRUE;
        uint8 bConnecting = FALSE;
        connPtr = &conn_param[extInitInfo.connId];           // connId is allocated when create conn
        memset(&ext_adv_hdr, 0, sizeof(ext_adv_hdr));

        // check status
        if ((irq_status & LIRQ_RD) && (irq_status & LIRQ_COK))
        {
            // rx done
            uint8_t packet_len, pdu_type;
            uint16_t pktLen;
            uint32_t pktFoot0, pktFoot1;
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

            if (packet_len   != 0
                    && (pdu_type == ADV_EXT_TYPE))
            {
                // never used (addrType, txAdd)
                //uint8   addrType;                  // peer address type
                //uint8_t txAdd = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random
                uint8   payload_len = (g_rx_adv_buf.rxheader & 0xFF00) >> LENGTH_SHIFT;
                //addrType = txAdd;          // TO check  //never used
                adv_mode    = (g_rx_adv_buf.data[0] & 0xc0) >> 6;
                //ext_hdr_len =  g_rx_adv_buf.data[0] & 0x3f; //never used
                ll_parseExtHeader(&g_rx_adv_buf.data[1], payload_len - 1);
// ================ RPA for extended init, need investigate more
                // Resolving list checking
//                  if (g_llRlEnable == TRUE           &&
//                   txAdd == LL_DEV_ADDR_TYPE_RANDOM  &&
//                  (g_rx_adv_buf.data[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR)
//
//                  {
//                      rpaListIndex = ll_getRPAListEntry(&g_rx_adv_buf.data[0]);
//                      if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
//                      {
//                          peerAddr = &g_llResolvinglist[rpaListIndex].peerAddr[0];
//                      }
//                      else
//                          bWlRlCheckOk = FALSE;
//          }
// ================ RPA for extended init, end
                #if 0

                // initiator, 2 types of filter process: 1. connect to peer address set by host   2. connect to  address in whitelist only
                // 1. connect to peer address set by host
                if (initInfo.wlPolicy == LL_INIT_WL_POLICY_USE_PEER_ADDR
                        && bWlRlCheckOk == TRUE)
                {
                    if (txAdd          != peerInfo.peerAddrType
                            || peerAddr[0]  != peerInfo.peerAddr[0]
                            || peerAddr[1]  != peerInfo.peerAddr[1]
                            || peerAddr[2]  != peerInfo.peerAddr[2]
                            || peerAddr[3]  != peerInfo.peerAddr[3]
                            || peerAddr[4]  != peerInfo.peerAddr[4]
                            || peerAddr[5]  != peerInfo.peerAddr[5])
                    {
                        // not match, not init connect
                        bWlRlCheckOk = FALSE;
                    }
                }
                // 2. connect to  address in whitelist only
                else if (initInfo.wlPolicy == LL_INIT_WL_POLICY_USE_WHITE_LIST &&
                         bWlRlCheckOk == TRUE)
                {
                    // if advA in whitelist list, connect
                    // check white list
                    bWlRlCheckOk = ll_isAddrInWhiteList(txAdd, peerAddr);
                }

                #endif

                if (bWlRlCheckOk == TRUE
                        && (adv_mode== LL_EXT_ADV_MODE_CONN)             // connectable adv
                        && !(ext_adv_hdr.header & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK))    // AUX_ADV_IND
                {
                    g_same_rf_channel_flag = TRUE;
                    // ===============  TODO: construct AUX_CONN_REQ PDU
                    llSetupAuxConnectReqPDU();
                    ll_hw_set_tx_rx_interval(10);
                    ll_hw_set_rx_timeout(158);
                    set_max_length(50);
                    // send conn req
                    T2 = read_current_fine_time();
                    delay = (T2 > ISR_entry_time) ? (T2 - ISR_entry_time) : (BASE_TIME_UNITS - ISR_entry_time + T2);
                    delay = 118 - delay - pGlobal_config[LL_ADV_TO_CONN_REQ_DELAY];
                    ll_hw_set_trx_settle(delay,                               // set BB delay, about 80us in 16MHz HCLK
                                         pGlobal_config[LL_HW_AFE_DELAY],
                                         pGlobal_config[LL_HW_PLL_DELAY]);        //RxAFE,PLL
                    // send conn req
                    ll_hw_set_trx();             // set LL HW as single Tx mode
                    ll_hw_go();
                    // reset Rx/Tx FIFO
                    ll_hw_rst_rfifo();
                    ll_hw_rst_tfifo();
                    ll_hw_ign_rfifo(LL_HW_IGN_CRC | LL_HW_IGN_EMP);
                    llWaitingIrq = TRUE;
//  hal_gpio_write(GPIO_P15, 1);
//  hal_gpio_write(GPIO_P15, 0);
                    // AdvA, offset 6
                    memcpy((uint8*)&g_tx_adv_buf.data[6], &ext_adv_hdr.advA[0], 6);
                    //write Tx FIFO
                    ll_hw_write_tfifo((uint8*)&(g_tx_adv_buf.txheader),
                                      ((g_tx_adv_buf.txheader & 0xff00) >> 8) + 2);   // payload length + header length(2)
                    move_to_master_function();
                    bConnecting = TRUE;
                    g_same_rf_channel_flag = FALSE;
                }
            }
            else if (packet_len   != 0
                     && (pdu_type == ADV_DIRECT_IND))     // TODO: add process of direct ADV
            {
            }
        }

        // scan again if not start connect
        if (!bConnecting)           // if not waiting for scan rsp, schedule next scan
        {
            if (extInitInfo.scanMode == LL_SCAN_STOP)
            {
                // scan has been stopped
                llState = LL_STATE_IDLE;                                                 // for single connection case, set the LL state idle
                //  release the associated allocated connection
                llReleaseConnId(connPtr);                                                // new for multi-connection
                g_ll_conn_ctx.numLLMasterConns --;
                (void)osal_set_event( LL_TaskID, LL_EVT_MASTER_CONN_CANCELLED );         // inform high layer
            }
            else
            {
                // not sending SCAN REQ, update scan time
                llScanTime += ((ISR_entry_time > llScanT1) ? (ISR_entry_time - llScanT1) : (BASE_TIME_UNITS - llScanT1 + ISR_entry_time));

                if (ext_adv_hdr.header & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK)    // receive aux channel information
                {
                    uint32    wait_time;
                    extInitInfo.current_chn = ext_adv_hdr.auxPtr.chn_idx;
                    extInitInfo.current_scan_PHY = ext_adv_hdr.auxPtr.aux_phy;
                    wait_time = ext_adv_hdr.auxPtr.aux_offset * ((ext_adv_hdr.auxPtr.offset_unit == 1) ? 300 : 30);
                    wait_time -= (ext_adv_hdr.auxPtr.ca == 0) ? 500 : 499;     // temporary setting, consider clock accuracy
                    llScanTime = 0;
                    ll_ext_init_schedule_next_event(wait_time);
                }
                else if (llScanTime >= extInitInfo.scanWindow[extInitInfo.current_index] * 625)
                {
                    if (extInitInfo.numOfScanPHY > 1 && extInitInfo.current_chn == LL_SCAN_ADV_CHAN_39)
                    {
                        extInitInfo.current_index = (extInitInfo.current_index + 1) & 0x01;
                        extInitInfo.current_scan_PHY = extInitInfo.initPHYs[extInitInfo.current_index];
                    }

                    LL_CALC_NEXT_SCAN_CHN(extInitInfo.current_chn);

                    // schedule next scan event
                    if (extInitInfo.scanWindow[extInitInfo.current_index] == extInitInfo.scanInterval[extInitInfo.current_index])      // scanWindow == scanInterval, trigger immediately
                        llSetupExtInit();
                    else
                        ll_ext_init_schedule_next_event((extInitInfo.scanInterval[extInitInfo.current_index]
                                                         - extInitInfo.scanWindow[extInitInfo.current_index]) * 625);

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
    else if (ll_mode == LL_HW_MODE_TRX)            // init case, waiting for AUX_CONNECT_RSP
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
            packet_len = ll_hw_read_rfifo((uint8_t*)(&(g_rx_adv_buf.rxheader)),
                                          &pktLen,
                                          &pktFoot0,
                                          &pktFoot1);

            if(ll_hw_get_rfifo_depth() > 0)
            {
                g_pmCounters.ll_rfifo_read_err++;
                packet_len=0;
                pktLen=0;
            }

            // check receive pdu type
            pdu_type = g_rx_adv_buf.rxheader & PDU_TYPE_MASK;
//        txAdd    = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random

            if (packet_len > 0                       // any better checking rule for rx anything?
                    && pdu_type == ADV_AUX_CONN_RSP)
            {
//            g_pmCounters.ll_recv_scan_req_cnt ++;

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
            // TODO
            // receive error connect rsp or timeout, cancel connection
        }

//===========
    }

    if (!llWaitingIrq)
    {
        ll_hw_clr_irq();
        llTaskState = LL_TASK_OTHERS;
    }

    HAL_EXIT_CRITICAL_SECTION();
    return TRUE;
}

uint8 ll_processPrdScanIRQ(uint32_t      irq_status)
{
//    uint8         ll_mode, adv_mode, ext_hdr_len;
    uint8   ext_hdr_len;
    uint8       temp_event1stFlag;
    uint32_t      T2, wait_time = 0;
    int8     rssi;
    llPeriodicScannerInfo_t*  pPrdScanInfo;
    pPrdScanInfo = &g_llPeriodAdvSyncInfo[0];
    memset(&ext_adv_hdr, 0, sizeof(ext_adv_hdr));
    HAL_ENTER_CRITICAL_SECTION();
    // pPrdScanInfo->event1stFlag indicate we are searching AUX_SYNC_IND PDU
    temp_event1stFlag = pPrdScanInfo->event1stFlag;

    if (pPrdScanInfo->event1stFlag == TRUE)
    {
        pPrdScanInfo->nextEventRemainder = pPrdScanInfo->advInterval;
        pPrdScanInfo->eventCounter ++;         // increase PA event counter
        pPrdScanInfo->currentEventChannel = llGetNextDataChanCSA2(pPrdScanInfo->eventCounter,
                                                                  pPrdScanInfo->channelIdentifier,
                                                                  pPrdScanInfo->chnMap,
                                                                  pPrdScanInfo->chanMapTable,
                                                                  pPrdScanInfo->numUsedChans);
        pPrdScanInfo->event1stFlag = FALSE;
    }

    if ((irq_status & LIRQ_RD) && (irq_status & LIRQ_COK))
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
        pdu_type = g_rx_adv_buf.rxheader & 0x0f;

        if(ll_hw_get_rfifo_depth() > 0)
        {
            g_pmCounters.ll_rfifo_read_err++;
            packet_len = 0;
            pktLen = 0;
        }

        pPrdScanInfo->syncLostTime = 0;

        if (pPrdScanInfo->syncEstOk == FALSE)
        {
            pPrdScanInfo->syncEstOk = TRUE;
            LL_PrdAdvSyncEstablishedCback(0,                             // status
                                          pPrdScanInfo->syncHandler,     // syncHandle,
                                          scanSyncInfo.advertising_SID,  // advertisingSID,
                                          scanSyncInfo.advertiser_Address_Type, //   advertiserAddressType,
                                          scanSyncInfo.advertiser_Address, // uint8   *advertiserAddress,
                                          pPrdScanInfo->advPhy,//   advertiserPHY,
                                          pPrdScanInfo->advInterval,//  periodicAdvertisingInterval,
                                          pPrdScanInfo->sca//   advertiserClockAccuracy
                                         );
            scanSyncInfo.valid = FALSE;           // sync establised, exit sync pending status
        }

        if (packet_len   != 0
                && (pdu_type == ADV_EXT_TYPE))
        {
            //never used(txAdd)
            //uint8_t txAdd = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random
            uint8   payload_len = (g_rx_adv_buf.rxheader & 0xFF00) >> LENGTH_SHIFT;
            uint8   adv_data_len;
            uint8   dataStatus;
//            addrType = txAdd;          // TO check
            //adv_mode    = (g_rx_adv_buf.data[0] & 0xc0) >> 6;   //never used
            ext_hdr_len =  g_rx_adv_buf.data[0] & 0x3f;
            ll_parseExtHeader(&g_rx_adv_buf.data[1], payload_len - 1);
            pPrdScanInfo->IQSampleInfo.CTE_Type = ext_adv_hdr.cteInfo >> 6;
            pPrdScanInfo->IQSampleInfo.CTE_Length = ext_adv_hdr.cteInfo & 0X3F;
            adv_data_len = payload_len - ext_hdr_len - 1;
            rssi  =  -(pktFoot1 >> 24);

            if (adv_data_len > 0)
            {
                // copy the adv_data
                //LOG("len=%d\n", adv_data_len);
                if (ext_adv_hdr.header & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK)
                    dataStatus = 1;
                else
                    dataStatus = 0;

                LL_PrdAdvReportCback(0,        // sync handler
                                     0,      //txPower,
                                     rssi,
                                     pPrdScanInfo->IQSampleInfo.CTE_Type,       // to update , uint8 cteType,
                                     dataStatus,
                                     pktLen - 8,
                                     &g_rx_adv_buf.data[6]
                                    );
                g_pmCounters.ll_recv_adv_pkt_cnt ++;
            }

            // for the 1st PDU of periodic adv & we have sync, fine adjust the anchor point
            // loop_time: time elapse between LL trigger and now
            // anchor point: time elapse between LL trigger and bb sync
            // the time between bb_sync and now is what we should fine adjust
            if (temp_event1stFlag == TRUE)
            {
                uint32 loop_time, anchor_point;

                // read loop timeout counter, system clock may be 16MHz, 32MHz, 64MHz and 48MHz, 96MHz
                if (hclk_per_us_shift != 0)
                    loop_time = ll_hw_get_loop_cycle() >> hclk_per_us_shift;      // convert to us
                else
                    loop_time = ll_hw_get_loop_cycle() / hclk_per_us;             // convert to us

                // read anchor point
                if (hclk_per_us_shift != 0)
                    anchor_point = ll_hw_get_anchor() >> hclk_per_us_shift;      // convert to us
                else
                    anchor_point = ll_hw_get_anchor() / hclk_per_us;      // convert to us

                pPrdScanInfo->nextEventRemainder -= loop_time - anchor_point;
            }

            // 2020-01-21 add for connectionless IQ Sample Report
            if( g_llPeriodAdvSyncInfo[0].IQSampleInfo.enable )
            {
                if (ext_adv_hdr.header & LE_EXT_HDR_CTE_INFO_PRESENT_BITMASK)
                {
                    uint8 iqCnt = 0;

//                  uint16 iSample[LL_CTE_MAX_SUPP_LEN * LL_CTE_SUPP_LEN_UNIT];
//                  uint16 qSample[LL_CTE_MAX_SUPP_LEN * LL_CTE_SUPP_LEN_UNIT];
                    if( ( g_pLLcteISample != NULL ) && ( g_pLLcteQSample != NULL) )
                        iqCnt = ll_hw_get_iq_RawSample( g_pLLcteISample, g_pLLcteQSample );

                    if(iqCnt>0)
                    {
                        LL_ConnectionlessIQReportCback( 0,                  // SDK only support 1 periodic scanning
                                                        ext_adv_hdr.auxPtr.chn_idx,
                                                        rssi,
                                                        // before CTE Transmit and sampling , no Antenna change , default 0
                                                        0,
                                                        g_llPeriodAdvSyncInfo[0].IQSampleInfo.CTE_Type,
                                                        g_llPeriodAdvSyncInfo[0].IQSampleInfo.slot_Duration,
                                                        // Packet_Status=0, CRC success,cause only CRC Correctly that can run here
                                                        0,
                                                        g_llPeriodAdvSyncInfo[0].eventCounter,
                                                        iqCnt,
                                                        g_pLLcteISample,
                                                        g_pLLcteQSample);
                    }
                }
            }

            if (ext_adv_hdr.header & LE_EXT_HDR_AUX_PTR_PRESENT_BITMASK)
            {
                pPrdScanInfo->current_channel = ext_adv_hdr.auxPtr.chn_idx;
                // pPrdScanInfo->advPhy = ext_adv_hdr.auxPtr.aux_phy;    // periodic adv could not change PHY
                wait_time = ext_adv_hdr.auxPtr.aux_offset * ((ext_adv_hdr.auxPtr.offset_unit == 1) ? 300 : 30);

                // consider the time elapse since last timer expire
                if (temp_event1stFlag == FALSE)     // if this is not the 1st PDU of periodic adv, update nextEventRemainder
                {
                    uint32  elapse_time;
                    // calculate elapse time since last timer trigger
                    T2 = read_current_fine_time();
                    elapse_time = LL_TIME_DELTA(g_timerExpiryTick, T2);
                    pPrdScanInfo->nextEventRemainder -= elapse_time;
                }

                // the wait_time calculate from AUX_PTR starts from the frame start, need compensate the frame duration
                uint32 loop_time, anchor_point;

                // read loop timeout counter, system clock may be 16MHz, 32MHz, 64MHz and 48MHz, 96MHz
                if (hclk_per_us_shift != 0)
                    loop_time = ll_hw_get_loop_cycle() >> hclk_per_us_shift;      // convert to us
                else
                    loop_time = ll_hw_get_loop_cycle() / hclk_per_us;             // convert to us

                // read anchor point
                if (hclk_per_us_shift != 0)
                    anchor_point = ll_hw_get_anchor() >> hclk_per_us_shift;      // convert to us
                else
                    anchor_point = ll_hw_get_anchor() / hclk_per_us;      // convert to us

                wait_time -= loop_time - anchor_point;

//              LOG("<%d>", wait_time);

                if (wait_time < 2000)
                    wait_time = 10;
                else
                    wait_time -= 2000;     // scan advance
            }
            else
            {
                // finish receive current periodic adv event
                pPrdScanInfo->current_channel = pPrdScanInfo->currentEventChannel;
                pPrdScanInfo->event1stFlag = TRUE;

                if (temp_event1stFlag == FALSE)     // if this is not the 1st PDU of periodic adv, update nextEventRemainder
                {
                    uint32  elapse_time;
                    // calculate elapse time since last timer trigger
                    T2 = read_current_fine_time();
                    elapse_time = LL_TIME_DELTA(g_timerExpiryTick, T2);
                    pPrdScanInfo->nextEventRemainder -= elapse_time;
                }

                wait_time = pPrdScanInfo->nextEventRemainder;
                wait_time -= 2500;             // advance
            }
        }
    }
    else
    {
        // sync lost, should we consider CRC NOK case???
        pPrdScanInfo->syncLostTime += pPrdScanInfo->advInterval;
        pPrdScanInfo->current_channel = pPrdScanInfo->currentEventChannel;
        pPrdScanInfo->event1stFlag = TRUE;
//      if (temp_event1stFlag == FALSE)     // if this is not the 1st PDU of periodic adv, update nextEventRemainder
//      {
        uint32  elapse_time;
        // calculate elapse time since last timer trigger
        T2 = read_current_fine_time();
        elapse_time = LL_TIME_DELTA(g_timerExpiryTick, T2);
        pPrdScanInfo->nextEventRemainder -= elapse_time;
//      }
        wait_time = pPrdScanInfo->nextEventRemainder;
        wait_time -= 2500;             // advance

        if (pPrdScanInfo->syncEstOk == TRUE &&
                pPrdScanInfo->syncLostTime > pPrdScanInfo->syncTimeout)
        {
            // sync timeout, report HCI event HCI_LE_Periodic_Advertising_Sync_Lost
            LL_PrdAdvSyncLostCback(pPrdScanInfo->syncHandler);
            llDeleteSyncHandle(pPrdScanInfo->syncHandler);
            // TODO: should clear sync info and stop timer
        }
        else if (pPrdScanInfo->syncEstOk == FALSE &&
                 pPrdScanInfo->syncLostTime >= 6 * pPrdScanInfo->advInterval)
        {
            // failed to establish sync, report HCI event HCI_LE_Periodic_Advertising_Sync_Lost
            LL_PrdAdvSyncEstablishedCback(LL_STATUS_ERROR_SYNC_FAILED_TO_BE_ESTABLISHED,
                                          0,                                    //  syncHandle,
                                          scanSyncInfo.advertising_SID,         //  advertisingSID,
                                          scanSyncInfo.advertiser_Address_Type, //  advertiserAddressType,
                                          scanSyncInfo.advertiser_Address,      //  advertiserAddress,
                                          0,                                    //  advertiserPHY,
                                          0,                                    //  periodicAdvertisingInterval,
                                          0                                     //  advertiserClockAccuracy
                                         );
            scanSyncInfo.valid = FALSE;
            // TODO: should clear sync info and stop timer
        }
    }

    if (pPrdScanInfo->valid == FALSE)
    {
        // llDeleteSyncHandle(pPrdScanInfo->syncHandler);
        // pPrdScanInfo->syncHandler = 0xFFFF;
    }
    else    // in future release, below set timer function should change to scheduler
    {
        ll_prd_scan_schedule_next_event(wait_time);
        pPrdScanInfo->nextEventRemainder -= wait_time;
    }

//  LOG("<%d>", wait_time);

    if (!llWaitingIrq)
    {
        ll_hw_clr_irq();
        llTaskState = LL_TASK_OTHERS;
    }

    HAL_EXIT_CRITICAL_SECTION();
    return TRUE;
}

#else
uint8 ll_processExtScanIRQ(uint32_t      irq_status)
{
	(void) irq_status;
    return TRUE;
}

uint8 ll_processExtInitIRQ(uint32_t      irq_status)
{
	(void) irq_status;
    return TRUE;
}

uint8 ll_processPrdScanIRQ(uint32_t      irq_status)
{
	(void) irq_status;
    return TRUE;
}
#endif

#ifdef EXT_ADV_ENABLE
uint8 LL_extAdvTimerExpProcess(void)
{
    extAdvInfo_t*  pAdvInfo;
    uint8  current_chn;
    uint16 current_offset;
    // TODO: if the timer IRQ is pending, should advanced expiry tick
    g_timerExpiryTick = read_current_fine_time();
    pAdvInfo = g_pAdvSchInfo[g_currentExtAdv].pAdvInfo;
    current_chn = pAdvInfo->currentChn;
    current_offset = pAdvInfo->currentAdvOffset;
    // update scheduler task list
    ll_updateExtAdvRemainderTime(g_currentAdvTimer + LL_ADV_TIMING_COMPENSATE);

    // check timer1, if no enough margin, start timer
    if (llWaitingIrq ||
            (isTimer1Running() && read_LL_remainder_time() < pGlobal_config[LL_EXT_ADV_TASK_DURATION]))
    {
        ll_ext_adv_schedule_next_event(pGlobal_config[LL_EXT_ADV_TASK_DURATION]);
        return TRUE;
    }
    else
    {
        if (ll_isLegacyAdv(pAdvInfo))
            llSetupExtAdvLegacyEvent(pAdvInfo);       // send legacy ADV PDU
        else
            llSetupExtAdvEvent(pAdvInfo);       // send extended advertisement
    }

    // for 1st pri adv channel , update remainder time in scheduler
    if (ll_isFirstAdvChn(pAdvInfo->parameter.priAdvChnMap, current_chn))
    {
        g_pAdvSchInfo[g_currentExtAdv].nextEventRemainder += pAdvInfo->primary_advertising_interval;
        g_pAdvSchInfo[g_currentExtAdv].pAdvInfo->adv_event_duration += pAdvInfo->primary_advertising_interval;
        g_pAdvSchInfo[g_currentExtAdv].pAdvInfo->adv_event_counter ++;

        // event expiry decision, update 2020-04-07
        if (g_pAdvSchInfo[g_currentExtAdv].pAdvInfo->duration != 0 &&
                g_pAdvSchInfo[g_currentExtAdv].pAdvInfo->adv_event_duration > g_pAdvSchInfo[g_currentExtAdv].pAdvInfo->duration)
        {
            g_pAdvSchInfo[g_currentExtAdv].pAdvInfo->active = FALSE;          // mark as inactive
            LL_AdvSetTerminatedCback(LL_STATUS_ERROR_LIMIT_REACHED,
                                     g_pAdvSchInfo[g_currentExtAdv].pAdvInfo->advHandle,
                                     0,
                                     g_pAdvSchInfo[g_currentExtAdv].pAdvInfo->adv_event_counter);
        }
        else if (g_pAdvSchInfo[g_currentExtAdv].pAdvInfo->maxExtAdvEvents != 0
                 && g_pAdvSchInfo[g_currentExtAdv].pAdvInfo->adv_event_counter >= g_pAdvSchInfo[g_currentExtAdv].pAdvInfo->maxExtAdvEvents)
        {
            g_pAdvSchInfo[g_currentExtAdv].pAdvInfo->active = FALSE;          // mark as inactive
            LL_AdvSetTerminatedCback(LL_STATUS_ERROR_DIRECTED_ADV_TIMEOUT,
                                     g_pAdvSchInfo[g_currentExtAdv].pAdvInfo->advHandle,
                                     0,
                                     g_pAdvSchInfo[g_currentExtAdv].pAdvInfo->adv_event_counter);
        }
    }
    // for 1st aux adv channel, update remainder time in scheduler
    else if (current_chn < LL_ADV_CHAN_FIRST    // broadcast in aux adv chn
             &&  current_offset == 0)                // offset in adv data set greater than 0 means next PDU is AUX_CHAIN_IND PDU
    {
        ll_updateAuxAdvTimeSlot(g_currentExtAdv);        // update aux PDU
    }

    return TRUE;
}
#else
uint8 LL_extAdvTimerExpProcess(void)
{
    return TRUE;
}
#endif

#ifdef PRD_ADV_ENABLE
uint8 LL_prdAdvTimerExpProcess(void)
{
    int i;
    extAdvInfo_t*       pAdvInfo;
    periodicAdvInfo_t*  pPrdAdv;
    llPeriodicAdvScheduleInfo_t* p_scheduler = NULL;
    // TODO: if the timer IRQ is pending, should advanced expiry tick
    g_timerExpiryTick = read_current_fine_time();
    p_scheduler = &g_pAdvSchInfo_periodic[g_currentExtAdv_periodic];
    pAdvInfo    = p_scheduler->pAdvInfo;
    pPrdAdv     = p_scheduler->pAdvInfo_prd;

    // for 1st pri adv channel , update remainder time in scheduler
    if (ll_isFirstAdvChn(pAdvInfo->parameter.priAdvChnMap, pPrdAdv->currentChn))
    {
        p_scheduler->nextEventRemainder += pAdvInfo->primary_advertising_interval;
        pAdvInfo->adv_event_duration    += pAdvInfo->primary_advertising_interval;
        pAdvInfo->adv_event_counter ++;

        // event expiry decision, update 2020-04-07
        if ((pAdvInfo->duration != 0 && pAdvInfo->adv_event_duration > pAdvInfo->duration)
                || (pAdvInfo->maxExtAdvEvents != 0 && pAdvInfo->adv_event_counter >= pAdvInfo->maxExtAdvEvents))
        {
            pAdvInfo->active = FALSE;          // mark as inactive
            p_scheduler->nextEventRemainder = LL_INVALID_TIME;
        }
    }
    // for 1st aux adv channel, update remainder time in scheduler
    else if (pPrdAdv->currentChn < LL_ADV_CHAN_FIRST    // broadcast in aux adv chn
             &&  pAdvInfo->sendingAuxAdvInd == FALSE         // not sending extended aux PDU
             &&  pPrdAdv->currentAdvOffset == 0)             // offset in adv data set greater than 0 means next PDU is AUX_CHAIN_IND PDU
    {
//      ll_updateAuxAdvTimeSlot(g_currentExtAdv);        // update aux PDU
        p_scheduler->auxPduRemainder += pPrdAdv->adv_interval;
    }

    // update scheduler task list
    for (i = 0; i < g_extAdvNumber; i++)
    {
        if (g_pAdvSchInfo_periodic[i].adv_handler != LL_INVALID_ADV_SET_HANDLE)
        {
            if (g_pAdvSchInfo_periodic[i].pAdvInfo->active == TRUE)
                g_pAdvSchInfo_periodic[i].nextEventRemainder -= (g_currentAdvTimer + LL_ADV_TIMING_COMPENSATE);

            g_pAdvSchInfo_periodic[i].auxPduRemainder    -= (g_currentAdvTimer + LL_ADV_TIMING_COMPENSATE);
        }
    }

    // check timer1, if no enough margin, start timer
    if (llWaitingIrq ||
            (isTimer1Running() && read_LL_remainder_time() < pGlobal_config[LL_EXT_ADV_TASK_DURATION]))
    {
        ll_prd_adv_schedule_next_event(pGlobal_config[LL_EXT_ADV_TASK_DURATION]);
    }
    else
        llSetupPrdAdvEvent(pPrdAdv, pAdvInfo);       // send extended advertisement

    return TRUE;
}
#else
uint8 LL_prdAdvTimerExpProcess(void)
{
    return TRUE;
}
#endif

#ifdef EXT_SCAN_ENABLE
uint8 LL_prdScanTimerExpProcess(void)
{
    // TODO: if the timer IRQ is pending, should advanced expiry tick
    g_timerExpiryTick = read_current_fine_time();
    llSetupPrdScan();
    return TRUE;
}

void LL_extScanTimerExpProcess(void)
{
    llSetupExtScan(extScanInfo.current_chn);
}

void LL_extInitTimerExpProcess(void)
{
    llSetupExtInit();
}
#else
uint8 LL_prdScanTimerExpProcess(void)
{
    return TRUE;
}

void LL_extScanTimerExpProcess(void)
{
}

void LL_extInitTimerExpProcess(void)
{
}

#endif

extern uint8 isTimer4Running(void);
// allocate the aux PDU time slot
uint8 ll_allocAuxAdvTimeSlot(uint8 index)
{
    uint8 ret = TRUE;
    llAdvScheduleInfo_t* p_scheduler = NULL, *p_current_scheduler;
    uint32   pri_adv_duration;
    p_scheduler = &g_pAdvSchInfo[index];
    // consider the adv in primary channel(EXT_ADV_IND) + and IFS and process time(2000)
    pri_adv_duration = pGlobal_config[LL_EXT_ADV_INTER_PRI_CHN_INT] * 3 + 2000;

    // allocate time slot for AUX_XXX_IND PDU
    // case 1: the 1st task
    if (!isTimer4Running())
    {
        if (isTimer1Running())
            p_scheduler->auxPduRemainder = read_LL_remainder_time() + pri_adv_duration + pGlobal_config[LL_CONN_TASK_DURATION];
        else
            p_scheduler->auxPduRemainder = pri_adv_duration;

        return TRUE;
    }

    // case 2: there are ongoing adv, calculate aux pdu timing with the current adv as anchor
    p_current_scheduler = &g_pAdvSchInfo[g_currentExtAdv];
    p_scheduler->auxPduRemainder = p_current_scheduler->auxPduRemainder +
                                   (index > g_currentExtAdv ? ((index - g_currentExtAdv) * g_advPerSlotTick) : (g_advSlotPeriodic + (index - g_currentExtAdv) * g_advPerSlotTick));
    return ret;
}

// allocate the aux PDU time slot
uint8 ll_allocAuxAdvTimeSlot_prd(uint8 index)
{
    uint8 ret = TRUE;
    llPeriodicAdvScheduleInfo_t* p_scheduler = NULL, *p_current_scheduler;
    uint32   ext_adv_part_duration;
    p_scheduler = &g_pAdvSchInfo_periodic[index];
    // consider the adv in primary channel(EXT_ADV_IND) + AUX_ADV_IND + and IFS and process time(2000)
    ext_adv_part_duration = pGlobal_config[LL_EXT_ADV_INTER_PRI_CHN_INT] * 3
                            + pGlobal_config[LL_EXT_ADV_INTER_SEC_CHN_INT]
                            + pGlobal_config[LL_EXT_ADV_PRI_2_SEC_CHN_INT]
                            + 2000;              // rough compensation for IFS and process time

    // allocate time slot for AUX_XXX_IND PDU
    // case 1: the 1st task
    if (!isTimer4Running())
    {
        if (isTimer1Running())
            p_scheduler->auxPduRemainder = read_LL_remainder_time() + ext_adv_part_duration + pGlobal_config[LL_CONN_TASK_DURATION];
        else
            p_scheduler->auxPduRemainder = ext_adv_part_duration;

        return TRUE;
    }

    // case 2: there are ongoing adv, calculate aux pdu timing with the current adv as anchor
    p_current_scheduler = &g_pAdvSchInfo_periodic[g_currentExtAdv_periodic];
    p_scheduler->auxPduRemainder = p_current_scheduler->auxPduRemainder +
                                   (index > g_currentExtAdv_periodic ?
                                    ((index - g_currentExtAdv_periodic) * g_advPerSlotTick) :
                                    (g_advSlotPeriodic + (index - g_currentExtAdv_periodic) * g_advPerSlotTick));
    return ret;
}

// update next aux PDU time slot
void ll_updateAuxAdvTimeSlot(uint8 index)
{
    while (g_pAdvSchInfo[index].auxPduRemainder < g_pAdvSchInfo[index].nextEventRemainder)
        g_pAdvSchInfo[index].auxPduRemainder += g_advSlotPeriodic;
}

//#define INVALID_ADV_RSC_POOL_POS   0xFFFFFFFF
//// get remain time to Aux Adv resource pool position
//uint32 ll_getAuxAdvRscPoolPosition(void)
//{
//    int i;
//  uint32 pos, temp;
//
//  for (i = 0; i < g_extAdvNumber; i++)
//  {
//      if (g_pAdvSchInfo[i].adv_handler != LL_INVALID_ADV_SET_HANDLE)
//          break;
//  }
//
//  if (i == g_extAdvNumber)
//      return INVALID_ADV_RSC_POOL_POS;
//
//    temp = i * g_advPerSlotTick;
//  if (g_pAdvSchInfo[i].auxPduRemainder > temp)
//      pos = g_pAdvSchInfo[i].auxPduRemainder - temp;
//  else
//      return INVALID_ADV_RSC_POOL_POS;
//
//  return pos;
//}
//
//uint32 ll_getAuxAdvRscPooPeriod(void)
//{
//    return g_advPerSlotTick * g_extAdvNumber;
//}

void ll_updateExtAdvRemainderTime(uint32 time)
{
    int i;

    for (i = 0; i < g_extAdvNumber; i++)
    {
        if (g_pAdvSchInfo[i].adv_handler != LL_INVALID_ADV_SET_HANDLE)
        {
            if (g_pAdvSchInfo[i].nextEventRemainder < time)
            {
                g_pAdvSchInfo[i].nextEventRemainder += g_pAdvSchInfo[i].pAdvInfo->primary_advertising_interval;
                g_pAdvSchInfo[i].pAdvInfo->adv_event_duration += g_pAdvSchInfo[i].pAdvInfo->primary_advertising_interval;
                g_pAdvSchInfo[i].pAdvInfo->adv_event_counter ++;
            }

            if (g_pAdvSchInfo[i].auxPduRemainder < time)     // normally this case should not occur
                g_pAdvSchInfo[i].auxPduRemainder += g_advSlotPeriodic;

            if (g_pAdvSchInfo[i].auxPduRemainder != LL_INVALID_TIME)
                g_pAdvSchInfo[i].auxPduRemainder -= time;

            g_pAdvSchInfo[i].nextEventRemainder -= time;
        }
    }
}

/*******************************************************************************
    @fn          LL_processBasicIRQ

    @brief      Interrupt Request Handler for Link Layer

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None
*/
uint8 ll_processBasicIRQ(uint32_t      irq_status)
{
    uint8         mode;
    uint32_t      T2, delay;
    llConnState_t* connPtr;
    connPtr = &conn_param[0];        // To update
    HAL_ENTER_CRITICAL_SECTION();
    mode = ll_hw_get_tr_mode();

    // ===================   mode TRX process 1
    if (mode == LL_HW_MODE_TRX  &&
            (irq_status & LIRQ_COK) &&                       // bug correct 2018-10-15
            (llState == LL_STATE_ADV_UNDIRECTED ||
             llState == LL_STATE_ADV_SCAN       ||
             llState == LL_STATE_ADV_DIRECTED)
       )
    {
        uint8_t  packet_len, pdu_type, txAdd;
        uint8_t*  peerAddr;
        uint8_t  bWlRlCheckOk = TRUE;
        uint16_t pktLen;
        uint32_t pktFoot0, pktFoot1;
        int      calibra_time;                 // this parameter will be provided by global_config
        ll_debug_output(DEBUG_LL_HW_TRX);
        // read packet
        packet_len = ll_hw_read_rfifo((uint8_t*)(&(g_rx_adv_buf.rxheader)),
                                      &pktLen,
                                      &pktFoot0,
                                      &pktFoot1);

        if(ll_hw_get_rfifo_depth()>0)
        {
            g_pmCounters.ll_rfifo_read_err++;
            packet_len=0;
            pktLen=0;
        }

        // check receive pdu type
        pdu_type = g_rx_adv_buf.rxheader & PDU_TYPE_MASK;
        txAdd    = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random

        if (packet_len > 0                       // any better checking rule for rx anything?
                && pdu_type == ADV_SCAN_REQ
                && (llState == LL_STATE_ADV_UNDIRECTED
                    || llState == LL_STATE_ADV_SCAN))
        {
            // 1. scan req
            g_pmCounters.ll_recv_scan_req_cnt ++;

            // check AdvA
            if (g_rx_adv_buf.data[6]  != adv_param.ownAddr[0]
                    || g_rx_adv_buf.data[7]  != adv_param.ownAddr[1]
                    || g_rx_adv_buf.data[8]  != adv_param.ownAddr[2]
                    || g_rx_adv_buf.data[9]  != adv_param.ownAddr[3]
                    || g_rx_adv_buf.data[10] != adv_param.ownAddr[4]
                    || g_rx_adv_buf.data[11] != adv_param.ownAddr[5])
            {
            }
            else
            {
                uint8_t  rpaListIndex;
                peerAddr = &g_rx_adv_buf.data[0];      // ScanA

                // === Resolving list checking
                // if ScanA is resolvable private address
                if (txAdd == LL_DEV_ADDR_TYPE_RANDOM
                        && (g_rx_adv_buf.data[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR)
                {
                    bWlRlCheckOk = TRUE;

                    if (g_llRlEnable == TRUE)
                    {
                        bWlRlCheckOk = FALSE;
                        rpaListIndex = ll_getRPAListEntry(&g_rx_adv_buf.data[0]);

                        if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                        {
                            peerAddr = &g_llResolvinglist[rpaListIndex].peerAddr[0];
                            bWlRlCheckOk = TRUE;
                        }
                    }
                }
                else   // ScanA is device Identity, if the device ID in the RPA list, check whether RPA should be used
                {
                    bWlRlCheckOk = TRUE;

                    for (int i = 0; i < LL_RESOLVINGLIST_ENTRY_NUM; i++)
                    {
                        if (g_llResolvinglist[i].peerAddr[0] == g_rx_adv_buf.data[0]
                                && g_llResolvinglist[i].peerAddr[1] == g_rx_adv_buf.data[1]
                                && g_llResolvinglist[i].peerAddr[2] == g_rx_adv_buf.data[2]
                                && g_llResolvinglist[i].peerAddr[3] == g_rx_adv_buf.data[3]
                                && g_llResolvinglist[i].peerAddr[4] == g_rx_adv_buf.data[4]
                                && g_llResolvinglist[i].peerAddr[5] == g_rx_adv_buf.data[5]
                                && g_llResolvinglist[i].peerAddrType == txAdd)
                        {
                            if (g_llResolvinglist[i].privacyMode == NETWORK_PRIVACY_MODE &&
                                    !ll_isIrkAllZero(g_llResolvinglist[i].peerIrk))
                                bWlRlCheckOk = FALSE;
                            else
                                rpaListIndex = i;

                            break;
                        }
                    }
                }

                // === check white list
                if ((pGlobal_config[LL_SWITCH] & LL_WHITELIST_ALLOW)
                        && (adv_param.wlPolicy  == LL_ADV_WL_POLICY_WL_SCAN_REQ
                            || adv_param.wlPolicy  == LL_ADV_WL_POLICY_WL_ALL_REQ)
                        && (bWlRlCheckOk == TRUE))
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
                    uint8 retScanRspFilter=1;

                    if(LL_PLUS_ScanRequestFilterCBack)
                    {
                        retScanRspFilter = LL_PLUS_ScanRequestFilterCBack();
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
                        delay = 118 - delay - calibra_time;                       // IFS = 150us, Tx tail -> Rx done time: about 32us
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
                 && (llState == LL_STATE_ADV_UNDIRECTED
                     || llState == LL_STATE_ADV_DIRECTED))
        {
            // 2. connect req
            g_currentPeerAddrType = txAdd;
            g_pmCounters.ll_recv_conn_req_cnt ++;

            // check AdvA
            if (g_rx_adv_buf.data[6]  != adv_param.ownAddr[0]
                    || g_rx_adv_buf.data[7]  != adv_param.ownAddr[1]
                    || g_rx_adv_buf.data[8]  != adv_param.ownAddr[2]
                    || g_rx_adv_buf.data[9]  != adv_param.ownAddr[3]
                    || g_rx_adv_buf.data[10] != adv_param.ownAddr[4]
                    || g_rx_adv_buf.data[11] != adv_param.ownAddr[5])
            {
                // nothing to do
            }
            else
            {
                uint8_t  rpaListIndex = LL_RESOLVINGLIST_ENTRY_NUM;
                peerAddr = &g_rx_adv_buf.data[0];        // initA

                // ====== check Resolving list
                if (txAdd == LL_DEV_ADDR_TYPE_RANDOM   &&
                        (g_rx_adv_buf.data[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR)
                {
                    bWlRlCheckOk = TRUE;

                    if (g_llRlEnable == TRUE)
                    {
                        bWlRlCheckOk = FALSE;
                        rpaListIndex = ll_getRPAListEntry(&g_rx_adv_buf.data[0]);

                        if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                        {
                            // save resolved peer address
                            peerAddr = &g_llResolvinglist[rpaListIndex].peerAddr[0];
                            // if resolved address success, map the peer address type to 0x02 or 0x03
                            g_currentPeerAddrType = g_llResolvinglist[rpaListIndex].peerAddrType + 2;
                            osal_memcpy( &g_currentPeerRpa[0],  &g_rx_adv_buf.data[0], 6);   // save latest peer RPA
                            bWlRlCheckOk = TRUE;
                        }
                    }
                }
                else   // InitA is device Identity, check whether the device Addr in the RPA list, if it is
                {
                    // in the RPA list and network privacy mode is selected and non all-0 IRK, check failed
                    bWlRlCheckOk = TRUE;

                    for (int i = 0; i < LL_RESOLVINGLIST_ENTRY_NUM; i++)
                    {
                        if (g_llResolvinglist[i].peerAddr[0] == g_rx_adv_buf.data[0]
                                && g_llResolvinglist[i].peerAddr[1] == g_rx_adv_buf.data[1]
                                && g_llResolvinglist[i].peerAddr[2] == g_rx_adv_buf.data[2]
                                && g_llResolvinglist[i].peerAddr[3] == g_rx_adv_buf.data[3]
                                && g_llResolvinglist[i].peerAddr[4] == g_rx_adv_buf.data[4]
                                && g_llResolvinglist[i].peerAddr[5] == g_rx_adv_buf.data[5]
                                && g_llResolvinglist[i].peerAddrType == txAdd)
                        {
                            if (g_llResolvinglist[i].privacyMode == NETWORK_PRIVACY_MODE &&
                                    !ll_isIrkAllZero(g_llResolvinglist[i].peerIrk))
                                bWlRlCheckOk = FALSE;
                            else
                                rpaListIndex = i;

                            break;
                        }
                    }
                }

                // ====== check white list
                if ((pGlobal_config[LL_SWITCH] & LL_WHITELIST_ALLOW)
                        && (llState == LL_STATE_ADV_UNDIRECTED)
                        && (adv_param.wlPolicy   == LL_ADV_WL_POLICY_WL_CONNECT_REQ
                            || adv_param.wlPolicy  == LL_ADV_WL_POLICY_WL_ALL_REQ)
                        && (bWlRlCheckOk == TRUE))
                {
                    // check white list
                    bWlRlCheckOk = ll_isAddrInWhiteList(txAdd, peerAddr);
                }

                // fixed bug 2018-09-25, LL/CON/ADV/BV-04-C, for direct adv, initA should equal peer Addr
                if (llState == LL_STATE_ADV_DIRECTED)
                {
                    if (//txAdd         != peerInfo.peerAddrType    // for (extended) set adv param, peer addr type could only be 0x0 or 0x01
                        peerAddr[0]     != peerInfo.peerAddr[0]
                        || peerAddr[1]  != peerInfo.peerAddr[1]
                        || peerAddr[2]  != peerInfo.peerAddr[2]
                        || peerAddr[3]  != peerInfo.peerAddr[3]
                        || peerAddr[4]  != peerInfo.peerAddr[4]
                        || peerAddr[5]  != peerInfo.peerAddr[5])
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
                    peerInfo.peerAddrType = txAdd;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random
                    osal_memcpy(peerInfo.peerAddr, &peerAddr[0], 6);
                    move_to_slave_function();    // move to slave role for connection state
                }
            }
        }
        //test for fast adv
        else if(llState == LL_STATE_ADV_UNDIRECTED)
        {
            uint8_t firstAdvChan = (adv_param.advChanMap&LL_ADV_CHAN_37) !=0 ? 37 :
                                   (adv_param.advChanMap&LL_ADV_CHAN_38) !=0 ? 38 : 39;

            if(adv_param.advNextChan>firstAdvChan)
            {
                //llSetupUndirectedAdvEvt1();
                ll_schedule_next_event(50);         //20180623 modified by ZQ
                // reset the timer1 instead of llSetupUndirectedAdvEvt1
                // reduced the process time in LL_IRQ
                // llSetupUndirectedAdvEvt1 will cost about 120us
            }
        }
    }
    // =================== mode TRX process 2, for scanner, active scan case
    else if (mode == LL_HW_MODE_TRX  &&
             (llState == LL_STATE_SCAN))
    {
        // check whether receives SCAN RSP
        ll_debug_output(DEBUG_LL_HW_TRX);
        llScanTime += ((ISR_entry_time > llScanT1) ? (ISR_entry_time - llScanT1) : (BASE_TIME_UNITS - llScanT1 + ISR_entry_time));

        // check whether receives SCAN RSP
        if (irq_status & LIRQ_COK)                        // bug correct 2018-10-15
        {
            // rx done
            uint8_t packet_len, pdu_type;
            uint16_t pktLen;
            uint32_t pktFoot0, pktFoot1;
            // read packet
            packet_len = ll_hw_read_rfifo((uint8_t*)(&(g_rx_adv_buf.rxheader)),
                                          &pktLen,
                                          &pktFoot0,
                                          &pktFoot1);
            // check receive pdu type
            pdu_type = g_rx_adv_buf.rxheader & 0x0f;

            if(ll_hw_get_rfifo_depth()>0)
            {
                g_pmCounters.ll_rfifo_read_err++;
                packet_len=0;
                pktLen=0;
            }

            if (packet_len > 0 && pdu_type == ADV_SCAN_RSP)
            {
                // receives SCAN_RSP
                uint8  advEventType;
                uint8  rpaListIndex;
                uint8* peerAddr;
                uint8  addrType = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;
                uint8  dataLen  = pktLen - 8;
                int8   rssi     =  -(pktFoot1 >> 24);
                uint8  bCheckOk = TRUE;
                peerAddr = &g_rx_adv_buf.data[0];

                //===
                // AdvA of SCAN_RSP should also be checked here. Refer to 4.4.3.2 Active Scanning
                // After sending a scan request PDU the Link Layer listens for a scan response
                //PDU from that advertiser. If the scan response PDU was not received from that
                //advertiser, it is considered a failure; otherwise it is considered a success.

                // check AdvA in Scan Rsp is identical to Scan Req
                if (g_rx_adv_buf.data[0] != g_tx_adv_buf.data[6]  ||
                        g_rx_adv_buf.data[1] != g_tx_adv_buf.data[7]  ||
                        g_rx_adv_buf.data[2] != g_tx_adv_buf.data[8]  ||
                        g_rx_adv_buf.data[3] != g_tx_adv_buf.data[9]  ||
                        g_rx_adv_buf.data[4] != g_tx_adv_buf.data[10] ||
                        g_rx_adv_buf.data[5] != g_tx_adv_buf.data[11]
                   )
                    bCheckOk = FALSE;

                // RPA checking. Note that we do not check whether it is the same RPA index
                if (addrType == LL_DEV_ADDR_TYPE_RANDOM  &&
                        (g_rx_adv_buf.data[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR)
                {
                    if (g_llRlEnable == TRUE)
                    {
                        rpaListIndex = ll_getRPAListEntry(&g_rx_adv_buf.data[0]);

                        if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                        {
                            peerAddr = &g_llResolvinglist[rpaListIndex].peerAddr[0];
                            // refer to HCI LE Advertising Report Event, RPA address type should be
                            // 0x02: Public Identity Address (Corresponds to Resolved Private Address)
                            // 0x03: Random (static) Identity Address (Corresponds to Resolved Private Address)
                            addrType = g_llResolvinglist[rpaListIndex].peerAddrType + 2;
                            bCheckOk = TRUE;
                        }
                        else
                            bCheckOk = FALSE;
                    }
                }

                //===

                if (bCheckOk == TRUE)
                {
                    advEventType = LL_ADV_RPT_SCAN_RSP;
                    // below function cost 51us/66us(measure with GPIO)
                    LL_AdvReportCback( advEventType,                   // event type
                                       addrType,                             // Adv address type (TxAdd)
                                       peerAddr,                             // Adv address (AdvA)
                                       dataLen,                              // length of rest of the payload
                                       &g_rx_adv_buf.data[6],                // rest of payload
                                       rssi );                               // RSSI
                    g_pmCounters.ll_recv_scan_rsp_cnt ++;
                    llAdjBoffUpperLimitSuccess();
                }
            }
            else
                llAdjBoffUpperLimitFailure();
        }
        else
            llAdjBoffUpperLimitFailure();

        // update back off value according to new backoff upperLimit
        llGenerateNextBackoffCount();

        if (llScanTime >= scanInfo.scanWindow * 625)
        {
            // calculate next scan channel
            LL_CALC_NEXT_SCAN_CHN(scanInfo.nextScanChan);

            // schedule next scan event
            if (scanInfo.scanWindow == scanInfo.scanInterval)      // scanWindow == scanInterval, trigger immediately
                LL_evt_schedule();
            else
//                set_timer4((scanInfo.scanInterval - scanInfo.scanWindow) * 625);
                ll_schedule_next_event((scanInfo.scanInterval - scanInfo.scanWindow) * 625);

            // reset scan total time
            llScanTime = 0;
        }
        else
            llSetupScan(scanInfo.nextScanChan);
    }
    // =================== mode SRX process, for scan/init
    else if (mode == LL_HW_MODE_SRX
             && (llState == LL_STATE_SCAN || llState == LL_STATE_INIT))
    {
        ll_debug_output(DEBUG_LL_HW_SRX);
        uint8_t  rpaListIndex = LL_RESOLVINGLIST_ENTRY_NUM;
        uint8_t  bWlRlCheckOk = TRUE;
        uint8_t*  peerAddr;

        // ============= scan case
        if (llState == LL_STATE_SCAN)
        {
            uint8   bSendingScanReq = FALSE;

            // check status
            if ((irq_status & LIRQ_RD) && (irq_status & LIRQ_COK))       // bug correct 2018-10-15
            {
                // rx done
                uint8_t  packet_len, pdu_type;
                uint16_t pktLen;
                uint32_t pktFoot0, pktFoot1;
                // read packet
                // cost 21-26us(measure with GPIO), depneds on the length of ADV
                packet_len = ll_hw_read_rfifo((uint8_t*)(&(g_rx_adv_buf.rxheader)),
                                              &pktLen,
                                              &pktFoot0,
                                              &pktFoot1);
                // check receive pdu type
                pdu_type = g_rx_adv_buf.rxheader & 0x0f;

                if(ll_hw_get_rfifo_depth()>0)
                {
                    g_pmCounters.ll_rfifo_read_err++;
                    packet_len=0;
                    pktLen=0;
                }

                if (packet_len   != 0
                        && ((pdu_type == ADV_IND)
                            || (pdu_type  == ADV_NONCONN_IND)
                            || (pdu_type  == ADV_SCAN_IND)
                            || (pdu_type == ADV_DIRECT_IND)))
                {
                    uint8   addrType;                  // peer address type
                    uint8_t txAdd = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random
                    peerAddr = &g_rx_adv_buf.data[0];        // AdvA
                    addrType = txAdd;

                    // Resolving list checking
                    // case 1: receive ScanA using RPA
                    if (txAdd == LL_DEV_ADDR_TYPE_RANDOM  &&
                            (g_rx_adv_buf.data[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR)
                    {
                        bWlRlCheckOk = FALSE;

                        if (g_llRlEnable == TRUE)
                        {
                            rpaListIndex = ll_getRPAListEntry(&g_rx_adv_buf.data[0]);

                            if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                            {
                                peerAddr = &g_llResolvinglist[rpaListIndex].peerAddr[0];
                                // refer to HCI LE Advertising Report Event, RPA address type should be
                                // 0x02: Public Identity Address (Corresponds to Resolved Private Address)
                                // 0x03: Random (static) Identity Address (Corresponds to Resolved Private Address)
                                addrType = g_llResolvinglist[rpaListIndex].peerAddrType + 2;
                                bWlRlCheckOk = TRUE;
                            }
                        }
                    }
                    else     // case 2: receive ScanA using device ID, or scan device not using RPA
                    {
                        bWlRlCheckOk = TRUE;

                        for (int i = 0; i < LL_RESOLVINGLIST_ENTRY_NUM; i++)
                        {
                            if ( g_llResolvinglist[i].peerAddr[0] == g_rx_adv_buf.data[0]
                                    && g_llResolvinglist[i].peerAddr[1] == g_rx_adv_buf.data[1]
                                    && g_llResolvinglist[i].peerAddr[2] == g_rx_adv_buf.data[2]
                                    && g_llResolvinglist[i].peerAddr[3] == g_rx_adv_buf.data[3]
                                    && g_llResolvinglist[i].peerAddr[4] == g_rx_adv_buf.data[4]
                                    && g_llResolvinglist[i].peerAddr[5] == g_rx_adv_buf.data[5])
                            {
                                // the device ID in the RPA list
                                if (g_llResolvinglist[i].privacyMode == DEVICE_PRIVACY_MODE ||
                                        ll_isIrkAllZero(g_llResolvinglist[i].peerIrk))
                                    rpaListIndex = i;
                                else
                                    bWlRlCheckOk = FALSE;      // the device in the RPA list but not using RPA, reject it

                                break;
                            }
                        }
                    }

                    // check white list
                    if ((pGlobal_config[LL_SWITCH] & LL_WHITELIST_ALLOW)
                            && (scanInfo.wlPolicy  == LL_SCAN_WL_POLICY_USE_WHITE_LIST)
                            && (bWlRlCheckOk == TRUE))
                    {
                        // check white list
                        bWlRlCheckOk = ll_isAddrInWhiteList(txAdd, peerAddr);
                    }
                    else if(pdu_type == ADV_DIRECT_IND)    // direct adv only report addr & addr type match the whitelist
                        bWlRlCheckOk = FALSE;

                    // if valid, trigger osal event to report adv
                    if (bWlRlCheckOk == TRUE)
                    {
                        uint8  advEventType;
                        int8   rssi;
                        llCurrentScanChn = scanInfo.nextScanChan;

                        // active scan scenario, send scan req
                        if (scanInfo.scanType == LL_SCAN_ACTIVE
                                && (pdu_type== ADV_IND
                                    || pdu_type == ADV_SCAN_IND ))
                        {
                            // back off process
                            scanInfo.currentBackoff = (scanInfo.currentBackoff > 0) ? (scanInfo.currentBackoff - 1) : 0;

                            if (scanInfo.currentBackoff == 0)      // back off value = 0, send scan req
                            {
                                g_tx_adv_buf.txheader = 0xC03;
                                //ZQ 20181012: add AdvFilterCB
                                uint8_t retAdvFilter = 1;

                                if(LL_PLUS_AdvDataFilterCBack)
                                {
                                    //!!!CATION!!!
                                    //timing critical
                                    //txbuf will be changed
                                    retAdvFilter = LL_PLUS_AdvDataFilterCBack();
                                }

                                if(retAdvFilter)
                                {
                                    g_same_rf_channel_flag = TRUE;
                                    ll_hw_set_tx_rx_interval(10);
                                    ll_hw_set_rx_timeout(158);
                                    set_max_length(0xFF);                    // add 2020-03-10
                                    T2 = read_current_fine_time();
                                    delay = (T2 > ISR_entry_time) ? (T2 - ISR_entry_time) : (BASE_TIME_UNITS - ISR_entry_time + T2);
                                    delay = 118 - delay - pGlobal_config[LL_ADV_TO_SCAN_REQ_DELAY];
                                    ll_hw_set_trx();             // set LL HW as single TRx mode
                                    ll_hw_set_trx_settle(delay,                               // set BB delay, about 80us in 16MHz HCLK
                                                         pGlobal_config[LL_HW_AFE_DELAY],
                                                         pGlobal_config[LL_HW_PLL_DELAY]);        //RxAFE,PLL
                                    ll_hw_go();
                                    g_pmCounters.ll_send_scan_req_cnt++;
                                    llWaitingIrq = TRUE;
                                    // reset Rx/Tx FIFO
                                    ll_hw_rst_rfifo();
                                    ll_hw_rst_tfifo();
                                    ll_hw_ign_rfifo(LL_HW_IGN_CRC | LL_HW_IGN_EMP);
                                    // construct SCAN REQ packet
                                    //g_tx_adv_buf.txheader = 0xCC3;
                                    //20181012 ZQ: change the txheader according to the adtype
                                    g_tx_adv_buf.txheader |=(((g_rx_adv_buf.rxheader&0x40)<<1)
                                                             | (scanInfo.ownAddrType<< TX_ADD_SHIFT & TX_ADD_MASK));

                                    // fill scanA, using RPA or device ID address   // TODO: move below code before ll_hw_go?
                                    if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM &&
                                            !ll_isIrkAllZero(g_llResolvinglist[rpaListIndex].localIrk))
                                    {
                                        // for resolving private address case, calculate the scanA with Local IRK
                                        ll_CalcRandomAddr(g_llResolvinglist[rpaListIndex].localIrk, &g_tx_adv_buf.data[0]);
//                                      osal_memcpy( &g_currentLocalRpa[0],  &g_tx_adv_buf.data[0], 6);
                                    }
                                    else
                                    {
                                        //LL_ReadBDADDR(&g_tx_adv_buf.data[0]);
                                        memcpy((uint8*)&g_tx_adv_buf.data[0], &scanInfo.ownAddr[0], 6);
                                    }

                                    // AdvA, for SCAN REQ, it should identical to the ADV_IND/ADV_SCAN_IND
                                    g_tx_adv_buf.data[6]  = peerAddr[0];
                                    g_tx_adv_buf.data[7]  = peerAddr[1];
                                    g_tx_adv_buf.data[8]  = peerAddr[2];
                                    g_tx_adv_buf.data[9]  = peerAddr[3];
                                    g_tx_adv_buf.data[10] = peerAddr[4];
                                    g_tx_adv_buf.data[11] = peerAddr[5];
                                    //write Tx FIFO
                                    ll_hw_write_tfifo((uint8*)&(g_tx_adv_buf.txheader),
                                                      ((g_tx_adv_buf.txheader & 0xff00) >> 8) + 2);   // payload length + header length(2)
                                    bSendingScanReq = TRUE;
                                    g_same_rf_channel_flag = FALSE;
                                }
                            }
                        }

                        // convert pdu type to GAP enum
                        switch (pdu_type)
                        {
                        case ADV_IND:
                            advEventType = LL_ADV_RPT_ADV_IND;
                            break;

                        case ADV_SCAN_IND:
                            advEventType = LL_ADV_RPT_ADV_SCANNABLE_IND;
                            break;

                        case ADV_DIRECT_IND:
                            advEventType = LL_ADV_RPT_ADV_DIRECT_IND;
                            break;

                        case ADV_NONCONN_IND:
                            advEventType = LL_ADV_RPT_ADV_NONCONN_IND;
                            break;

                        case ADV_SCAN_RSP:
                            advEventType = LL_ADV_RPT_INVALID;
                            break;

                        default:
                            advEventType = LL_ADV_RPT_ADV_IND;
                            break;
                        }

                        rssi  =  -(pktFoot1 >> 24);
                        // below function cost 51us/66us(measure with GPIO)
                        LL_AdvReportCback( advEventType,                         // event type
                                           addrType,                                // Adv address type (TxAdd)
                                           &peerAddr[0],                         // Adv address (AdvA)
                                           pktLen - 8,                           // length of rest of the payload, 2 - header, 6 - advA
                                           &g_rx_adv_buf.data[6],                // rest of payload
                                           rssi );                               // RSSI
                        g_pmCounters.ll_recv_adv_pkt_cnt ++;
                    }
                }
                else
                {
                    // invalid ADV PDU type
//                    llSetupScan();
                }
            }

            // if not waiting for scan rsp, schedule next scan
            if (!bSendingScanReq)
            {
                // not sending SCAN REQ, update scan time
                llScanTime += ((ISR_entry_time > llScanT1) ? (ISR_entry_time - llScanT1) : (BASE_TIME_UNITS - llScanT1 + ISR_entry_time));

                if (llScanTime >= scanInfo.scanWindow * 625)
                {
                    // calculate next scan channel
                    LL_CALC_NEXT_SCAN_CHN(scanInfo.nextScanChan);

                    // schedule next scan event
                    if (scanInfo.scanWindow == scanInfo.scanInterval)      // scanWindow == scanInterval, trigger immediately
                        LL_evt_schedule();
                    else
//                        set_timer4((scanInfo.scanInterval - scanInfo.scanWindow) * 625);
                        ll_schedule_next_event((scanInfo.scanInterval - scanInfo.scanWindow) * 625);

                    // reset scan total time
                    llScanTime = 0;
                }
                else
                {
//                    AT_LOG("%03x %x %d %d %d %d\n",irq_status,*(volatile uint32_t *)(0x40031054),ll_hw_get_anchor(),
//                                                        g_rfifo_rst_cnt,(uint32_t)ISR_entry_time,read_current_fine_time());
                    llSetupScan(scanInfo.nextScanChan);
                }
            }
        }
        // ===========  initiator case
        else if (llState == LL_STATE_INIT)
        {
            uint8 bConnecting = FALSE;
            uint8 bMatchAdv = FALSE;     // RPA checking OK in previous adv event, and new adv event identical to the old one
            connPtr = &conn_param[initInfo.connId];           // connId is allocated when create conn

            // check status
            if ((irq_status & LIRQ_RD) && (irq_status & LIRQ_COK))       // bug correct 2018-10-15
            {
                // rx done
                uint8_t packet_len, pdu_type;
                uint16_t pktLen;
                uint32_t pktFoot0, pktFoot1;
                // read packet
                // cost 21-26us(measure with GPIO), depneds on the length of ADV
                packet_len = ll_hw_read_rfifo((uint8_t*)(&(g_rx_adv_buf.rxheader)),
                                              &pktLen,
                                              &pktFoot0,
                                              &pktFoot1);
                // check receive pdu type
                pdu_type = g_rx_adv_buf.rxheader & 0x0f;

                if(ll_hw_get_rfifo_depth() > 0)
                {
                    g_pmCounters.ll_rfifo_read_err++;
                    packet_len=0;
                    pktLen=0;
                }

                if (packet_len    != 0
                        && ((pdu_type == ADV_IND) || pdu_type == ADV_DIRECT_IND))
                {
                    uint8_t txAdd = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random
                    uint8_t chSel = (g_rx_adv_buf.rxheader & CHSEL_MASK) >> CHSEL_SHIFT;
                    rpaListIndex = LL_RESOLVINGLIST_ENTRY_NUM;
                    peerAddr = &g_rx_adv_buf.data[0];        // AdvA
                    g_currentPeerAddrType = txAdd;

                    // ================= Resolving list checking
                    // case 1: receive InitA using RPA
                    if (txAdd == LL_DEV_ADDR_TYPE_RANDOM  &&
                            ((g_rx_adv_buf.data[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR))
                    {
                        bWlRlCheckOk = FALSE;

                        if (g_llRlEnable == TRUE)
                        {
                            // if the RPA checking is done in previous scan, compare
                            if (isPeerRpaStore  == TRUE  &&
                                    currentPeerRpa[0] == g_rx_adv_buf.data[0]
                                    && currentPeerRpa[1] == g_rx_adv_buf.data[1]
                                    && currentPeerRpa[2] == g_rx_adv_buf.data[2]
                                    && currentPeerRpa[3] == g_rx_adv_buf.data[3]
                                    && currentPeerRpa[4] == g_rx_adv_buf.data[4]
                                    && currentPeerRpa[5] == g_rx_adv_buf.data[5])
                            {
                                rpaListIndex = storeRpaListIndex;
                                peerAddr = &g_llResolvinglist[rpaListIndex].peerAddr[0];
                                g_currentPeerAddrType = g_llResolvinglist[rpaListIndex].peerAddrType + 2;
                                bWlRlCheckOk = TRUE;
                                bMatchAdv = TRUE;
                            }
                            else   // resolve the address
                            {
                                rpaListIndex = ll_getRPAListEntry(&g_rx_adv_buf.data[0]);    // spend 30us(48MHz) when the 1st item match

                                if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                                {
                                    peerAddr = &g_llResolvinglist[rpaListIndex].peerAddr[0];
                                    g_currentPeerAddrType = g_llResolvinglist[rpaListIndex].peerAddrType + 2;
                                    bWlRlCheckOk = TRUE;
                                }
                            }
                        }
                    }
                    // case 2: receive InitA using device ID, or init device not using RPA
                    else
                    {
                        for (int i = 0; i < LL_RESOLVINGLIST_ENTRY_NUM; i++)
                        {
                            if ( g_llResolvinglist[i].peerAddr[0] == g_rx_adv_buf.data[0]
                                    && g_llResolvinglist[i].peerAddr[1] == g_rx_adv_buf.data[1]
                                    && g_llResolvinglist[i].peerAddr[2] == g_rx_adv_buf.data[2]
                                    && g_llResolvinglist[i].peerAddr[3] == g_rx_adv_buf.data[3]
                                    && g_llResolvinglist[i].peerAddr[4] == g_rx_adv_buf.data[4]
                                    && g_llResolvinglist[i].peerAddr[5] == g_rx_adv_buf.data[5])
                            {
                                // the device ID in the RPA list
                                if (g_llResolvinglist[i].privacyMode == NETWORK_PRIVACY_MODE &&
                                        !ll_isIrkAllZero(g_llResolvinglist[i].peerIrk))
                                    bWlRlCheckOk = FALSE;
                                else
                                    rpaListIndex = i;
                            }
                        }
                    }

                    // ====== for direct adv, also check initA == own addr
                    if (pdu_type == ADV_DIRECT_IND && bWlRlCheckOk == TRUE && bMatchAdv != TRUE)
                    {
                        // initA is resolvable address case
                        if ((g_rx_adv_buf.data[11] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR)
                        {
                            // should not use RPA case
                            if (initInfo.ownAddrType != LL_DEV_ADDR_TYPE_RPA_PUBLIC && initInfo.ownAddrType != LL_DEV_ADDR_TYPE_RPA_RANDOM)
                                bWlRlCheckOk = FALSE;

                            if (rpaListIndex >= LL_RESOLVINGLIST_ENTRY_NUM
                                    || (ll_isIrkAllZero(g_llResolvinglist[rpaListIndex].localIrk))    // all-0 local IRK
                                    || (ll_ResolveRandomAddrs(g_llResolvinglist[rpaListIndex].localIrk, &g_rx_adv_buf.data[6]) != SUCCESS))   // resolve failed
                                bWlRlCheckOk = FALSE;
                        }
                        else
                        {
                            uint8* localAddr;
                            uint8_t rxAdd = (g_rx_adv_buf.rxheader & RX_ADD_MASK) >> RX_ADD_SHIFT;

                            // should not use device ID case
                            if ((initInfo.ownAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC || initInfo.ownAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM )
                                    && (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM
                                        && !ll_isIrkAllZero(g_llResolvinglist[rpaListIndex].localIrk)))
                            {
                                bWlRlCheckOk = FALSE;
                            }

                            if (rxAdd == LL_DEV_ADDR_TYPE_RANDOM)
                                localAddr = ownRandomAddr;
                            else
                                localAddr = ownPublicAddr;

                            if (g_rx_adv_buf.data[6]  != localAddr[0]
                                    || g_rx_adv_buf.data[7]  != localAddr[1]
                                    || g_rx_adv_buf.data[8]  != localAddr[2]
                                    || g_rx_adv_buf.data[9]  != localAddr[3]
                                    || g_rx_adv_buf.data[10] != localAddr[4]
                                    || g_rx_adv_buf.data[11] != localAddr[5])
                            {
                                bWlRlCheckOk = FALSE;
                            }
                        }
                    }

                    // initiator, 2 types of filter process: 1. connect to peer address set by host   2. connect to  address in whitelist only
                    // 1. connect to peer address set by host
                    if (initInfo.wlPolicy == LL_INIT_WL_POLICY_USE_PEER_ADDR
                            && bWlRlCheckOk == TRUE)
                    {
                        if (peerAddr[0]  != peerInfo.peerAddr[0]
                                || peerAddr[1]  != peerInfo.peerAddr[1]
                                || peerAddr[2]  != peerInfo.peerAddr[2]
                                || peerAddr[3]  != peerInfo.peerAddr[3]
                                || peerAddr[4]  != peerInfo.peerAddr[4]
                                || peerAddr[5]  != peerInfo.peerAddr[5])
                        {
                            // not match, not init connect
                            bWlRlCheckOk = FALSE;
                        }
                    }
                    // 2. connect to  address in whitelist only
                    else if (initInfo.wlPolicy == LL_INIT_WL_POLICY_USE_WHITE_LIST &&
                             bWlRlCheckOk == TRUE)
                    {
                        // if advA in whitelist list, connect
                        // check white list
                        bWlRlCheckOk = ll_isAddrInWhiteList(txAdd, peerAddr);
                    }

                    if (bWlRlCheckOk == TRUE)
                    {
                        g_same_rf_channel_flag = TRUE;

                        // channel selection algorithm decision
                        if ((pGlobal_config[LL_SWITCH] & CONN_CSA2_ALLOW)
                                && chSel == LL_CHN_SEL_ALGORITHM_2)
                        {
                            conn_param[initInfo.connId].channel_selection = LL_CHN_SEL_ALGORITHM_2;
                            SET_BITS(g_tx_adv_buf.txheader, LL_CHN_SEL_ALGORITHM_2, CHSEL_SHIFT, CHSEL_MASK);
                        }
                        else
                        {
                            conn_param[initInfo.connId].channel_selection = LL_CHN_SEL_ALGORITHM_1;
                            SET_BITS(g_tx_adv_buf.txheader, LL_CHN_SEL_ALGORITHM_1, CHSEL_SHIFT, CHSEL_MASK);
                        }

                        // calculate initA if using RPA list, otherwise copy the address stored in initInfo
                        if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM &&
                                !ll_isIrkAllZero(g_llResolvinglist[rpaListIndex].localIrk) &&
                                (initInfo.ownAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC || initInfo.ownAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM))
                        {
                            // for resolving private address case, calculate the scanA with Local IRK
                            ll_CalcRandomAddr(g_llResolvinglist[rpaListIndex].localIrk, &g_tx_adv_buf.data[0]);
                            SET_BITS(g_tx_adv_buf.txheader, LL_DEV_ADDR_TYPE_RANDOM, TX_ADD_SHIFT, TX_ADD_MASK);
//                           osal_memcpy( &g_currentLocalRpa[0],  &g_tx_adv_buf.data[0], 6);
                            g_currentLocalAddrType = LL_DEV_ADDR_TYPE_RPA_RANDOM;        // not accute local type, for branch selection in enh conn complete event
                        }
                        else
                        {
                            if (initInfo.ownAddrType == LL_DEV_ADDR_TYPE_PUBLIC || initInfo.ownAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC)
                            {
                                memcpy((uint8*)&g_tx_adv_buf.data[0], &ownPublicAddr[0], 6);
                                SET_BITS(g_tx_adv_buf.txheader, LL_DEV_ADDR_TYPE_PUBLIC, TX_ADD_SHIFT, TX_ADD_MASK);
                            }
                            else
                            {
                                memcpy((uint8*)&g_tx_adv_buf.data[0], &ownRandomAddr[0], 6);
                                SET_BITS(g_tx_adv_buf.txheader, LL_DEV_ADDR_TYPE_RANDOM, TX_ADD_SHIFT, TX_ADD_MASK);
                            }

                            g_currentLocalAddrType = LL_DEV_ADDR_TYPE_RANDOM;             // not accute local type, for branch selection in enh conn complete event
                        }

                        // send conn req
                        T2 = read_current_fine_time();
                        delay = (T2 > ISR_entry_time) ? (T2 - ISR_entry_time) : (BASE_TIME_UNITS - ISR_entry_time + T2);

                        if (delay > 118 - pGlobal_config[LL_ADV_TO_CONN_REQ_DELAY] - pGlobal_config[LL_HW_PLL_DELAY])   // not enough time
                        {
                            // not enough time to send conn req, store the RPA
                            isPeerRpaStore = TRUE;
                            storeRpaListIndex = rpaListIndex;
                            osal_memcpy(&currentPeerRpa[0], &g_rx_adv_buf.data[0], 6);
//                          LOG("store %d\n", storeRpaListIndex);
                            g_same_rf_channel_flag = FALSE;
                            LOG("<%d>", delay);
                        }
                        else
                        {
                            delay = 118 - delay - pGlobal_config[LL_ADV_TO_CONN_REQ_DELAY];
                            ll_hw_set_trx_settle(delay,                               // set BB delay, about 80us in 16MHz HCLK
                                                 pGlobal_config[LL_HW_AFE_DELAY],
                                                 pGlobal_config[LL_HW_PLL_DELAY]);        //RxAFE,PLL
                            // reset Rx/Tx FIFO
                            ll_hw_rst_rfifo();
                            ll_hw_rst_tfifo();
                            // send conn req
                            ll_hw_set_stx();             // set LL HW as single Tx mode
                            ll_hw_go();
                            llWaitingIrq = TRUE;
                            // AdvA, offset 6
                            memcpy((uint8*)&g_tx_adv_buf.data[6], &g_rx_adv_buf.data[0], 6);
                            //write Tx FIFO
                            ll_hw_write_tfifo((uint8*)&(g_tx_adv_buf.txheader),
                                              ((g_tx_adv_buf.txheader & 0xff00) >> 8) + 2);   // payload length + header length(2)

                            if (g_currentPeerAddrType >= 0x02)
                                osal_memcpy(&g_currentPeerRpa[0], &g_rx_adv_buf.data[0], 6);

                            if (g_currentLocalAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM)
                                osal_memcpy( &g_currentLocalRpa[0],  &g_tx_adv_buf.data[0], 6);

                            move_to_master_function();
                            isPeerRpaStore = FALSE;
                            bConnecting = TRUE;
                            g_same_rf_channel_flag = FALSE;
                        }
                    }
                }
                else if (packet_len   != 0
                         && (pdu_type == ADV_DIRECT_IND))     // TODO: add process of direct ADV
                {
                }
            }

            // scan again if not start connect
            if (!bConnecting)           // if not waiting for scan rsp, schedule next scan
            {
                if (initInfo.scanMode == LL_SCAN_STOP)
                {
                    // scan has been stopped
                    llState = LL_STATE_IDLE;                                                 // for single connection case, set the LL state idle
                    //  release the associated allocated connection
                    llReleaseConnId(connPtr);                                                // new for multi-connection
                    g_ll_conn_ctx.numLLMasterConns --;
                    (void)osal_set_event( LL_TaskID, LL_EVT_MASTER_CONN_CANCELLED );         // inform high layer
                }
                else
                {
                    // not sending SCAN REQ, update scan time
                    llScanTime += ((ISR_entry_time > llScanT1) ? (ISR_entry_time - llScanT1) : (BASE_TIME_UNITS - llScanT1 + ISR_entry_time));

                    if (llScanTime >= initInfo.scanWindow * 625)
                    {
                        // calculate next scan channel
                        LL_CALC_NEXT_SCAN_CHN(initInfo.nextScanChan);

                        // schedule next scan event
                        if (initInfo.scanWindow == initInfo.scanInterval)      // scanWindow == scanInterval, trigger immediately
                            LL_evt_schedule();
                        else
//                            set_timer4((initInfo.scanInterval - initInfo.scanWindow) * 625);
                            ll_schedule_next_event((initInfo.scanInterval - initInfo.scanWindow) * 625);

                        // reset scan total time
                        llScanTime = 0;
                    }
                    else
                        llSetupScan(initInfo.nextScanChan);
                }
            }
        }
    }
    // =================== mode RTLP process
    else if (mode == LL_HW_MODE_RTLP
             && llState == LL_STATE_CONN_SLAVE)
    {
        // slave
        uint8_t  ack_num, tx_num;
        uint32_t anchor_point, loop_time;
        uint8 temp_sn_nesn;                        // 2018-2-28 add, for BQB
        connPtr = &conn_param[g_ll_conn_ctx.currentConn];
        ll_debug_output(DEBUG_LL_HW_RTLP);
        connPtr->rx_crcok = 0;
        connPtr->rx_timeout = 0;

        // read loop timeout counter, system clock may be 16MHz, 32MHz, 64MHz and 48MHz, 96MHz
        if (hclk_per_us_shift != 0)
            loop_time = ll_hw_get_loop_cycle() >> hclk_per_us_shift;      // convert to us
        else
            loop_time = ll_hw_get_loop_cycle() / hclk_per_us;             // convert to us

        // read anchor point
        if (irq_status & LIRQ_TD)   // sync OK, then anchor point is OK, whether received something or CRC error => something will be Tx and done
        {
            if (hclk_per_us_shift != 0)
                anchor_point = ll_hw_get_anchor() >> hclk_per_us_shift;      // convert to us
            else
                anchor_point = ll_hw_get_anchor() / hclk_per_us;      // convert to us

            if (connPtr->firstPacket)                  // anchor point catched, change state
            {
                connPtr->firstPacket = 0;
            }

            //20180715 by ZQ
            //get the rssi form rf_phy register
            rf_phy_get_pktFoot(&connPtr->lastRssi, &connPtr->foff, &connPtr->carrSens);
            // global_config[SLAVE_CONN_DELAY]: soft parameter could be set after release
            slave_conn_event_recv_delay = loop_time - anchor_point + pGlobal_config[SLAVE_CONN_DELAY];

            if (irq_status & LIRQ_COK)     // Rx CRC OK, empty of data packet
            {
                connPtr->rx_crcok = 1;
            }
        }
        else
        {
            // timeout case
            connPtr->rx_timeout = 1;
            connPtr->pmCounter.ll_conn_event_timeout_cnt ++;
            // pGlobal_config[SLAVE_CONN_DELAY_BEFORE_SYNC]: soft parameter could be set after release
            slave_conn_event_recv_delay = loop_time - 270 + pGlobal_config[SLAVE_CONN_DELAY_BEFORE_SYNC];   // refer to llSlaveEvt_TaskEndOk(), we use the same formula for timeout & sync case

            //20181014 ZQ: perStats
            if(p_perStatsByChan!=NULL)
                p_perStatsByChan->rxToCnt[connPtr->currentChan]++;
        }

        //====== 20180324 modified by ZQ
//        ll_adptive_smart_window(irq_status,anchor_point);      // comment-out by HZF
        ack_num = ll_hw_get_txAck();
        rfCounters.numTxDone = ack_num;
        // below 2 counters are update in  ll_hw_update()
        //rfCounters.numTxAck = ack_num;         // not align to TI, ACK for empty packet is not considered
        //rfCounters.numRxOk = ll_hw_get_rxPkt_num();
        // TODO: update more Rf counters
        rfCounters.numRxNotOk    = 0;
        rfCounters.numRxIgnored  = 0;
        rfCounters.numRxEmpty    = 0;
        rfCounters.numRxFifoFull = 0;

        // A1 ROM metal change add
        if ((irq_status & LIRQ_CERR2)
                || (irq_status & LIRQ_CERR ))
        {
            connPtr->pmCounter.ll_recv_crcerr_event_cnt ++;
            rfCounters.numRxNotOk = 1;                  // CRC error, add 2018-6-12
        }

        //rfCounters.numTxRetrans = 0;
        //rfCounters.numTx = 0;
        //rfCounters.numRxCtrl = 0;
        temp_sn_nesn = connPtr->sn_nesn;     // 2018-2-28, BQB
        // update the LL HW engine mode and save the sn, nesn
        connPtr->llMode = ll_hw_update(connPtr->llMode,         // Attention: this mode is not real HW mode
                                       &(rfCounters.numTxAck),
                                       &(rfCounters.numRxOk),
                                       &(connPtr->sn_nesn));

        // add 2018-2-28, for slave latency scenario in BQB test
        if (connPtr->firstPacket == 0      // anchor point catched
                && (temp_sn_nesn & 0x02) != (connPtr->sn_nesn & 0x02)  // local sn has changed
                && rfCounters.numTxAck == 0)
        {
            rfCounters.numTxAck = 1;
        }

        // ==== check HW Tx FIFO, read packets which not transmit or transit but not receive ACK
        tx_num = (*(volatile uint32_t*)(LL_HW_BASE + 0x04) >> 8) & 0xff;

        if ( connPtr->encEnabled )
            g_pmCounters.ll_tbd_cnt2 += ack_num;

        //20200128 ZQ: perStats
        if(p_perStatsByChan!=NULL)
        {
            p_perStatsByChan->TxNumAck[connPtr->currentChan]+=ack_num;
            p_perStatsByChan->txNumRetry[connPtr->currentChan]+=ll_hw_get_nAck();
            uint8_t crcErrNum,rxTotalNum,rxPktNum;
            ll_hw_get_rxPkt_stats(&crcErrNum,&rxTotalNum,&rxPktNum);
            p_perStatsByChan->rxNumPkts[connPtr->currentChan]+=rxTotalNum;
            p_perStatsByChan->rxNumCrcErr[connPtr->currentChan]+=crcErrNum;
        }

        if (irq_status & LIRQ_RTO
                && ack_num < tx_num)   // receive time out, there are bugs in LL HW process TFIFO pointer, recover the pointer by SW
        {
            ll_hw_process_RTO(ack_num);
        }

        ll_hw_read_tfifo_rtlp();
        // update the numTxCtrlAck counter, add on 2017-11-15
        rfCounters.numTxCtrlAck = 0;

        if (connPtr->ctrlDataIsProcess == 1              // control packet in this RTLP event TFIFO
                && ack_num > 0                        // get ACK in this event
                && (connPtr->ll_buf.tx_not_ack_pkt->valid == 0  // no not_ack packet
                    || (connPtr->ll_buf.tx_not_ack_pkt->header & 0x3) != LL_DATA_PDU_HDR_LLID_CONTROL_PKT))  // not_ack packet is not ctrl packet
        {
            rfCounters.numTxCtrlAck = 1;
            connPtr->ctrlDataIsProcess = 0;
        }

        // if receive some packets, read them
        if (rfCounters.numRxOk)
        {
            // read HW Rx FIFO to internal buffer
            ll_read_rxfifo();
        }

        // call llSlaveEvt_TaskEndOk
        llSlaveEvt_TaskEndOk();

        // connection event notify
        if (g_conn_taskID != 0)
            osal_set_event(g_conn_taskID, g_conn_taskEvent);
    }
    // =================== mode TRLP process
    else if (mode == LL_HW_MODE_TRLP
             && llState == LL_STATE_CONN_MASTER)
    {
        // master
        ll_debug_output(DEBUG_LL_HW_TRLP);
        uint8_t  ack_num,tx_num;
        connPtr = &conn_param[g_ll_conn_ctx.currentConn];
        connPtr->rx_crcok = 0;
        connPtr->rx_timeout = 0;

        // TODO: read anchor point, it seems LIRQ_RD always be set here, to confirm
        if (irq_status & LIRQ_RD)   // sync OK, then anchor point is OK, whether received something or CRC error => something will be Tx and done
        {
            //20180715 by ZQ
            //get the rssi form rf_phy register
            rf_phy_get_pktFoot(&connPtr->lastRssi, &connPtr->foff,&connPtr->carrSens);

            if (irq_status & LIRQ_COK)     // Rx CRC OK, empty or data packet
            {
                connPtr->rx_crcok = 1;
            }
            else if ((irq_status & LIRQ_CERR2)
                     || (irq_status & LIRQ_CERR))
            {
                g_pmCounters.ll_tbd_cnt4 = 0;
            }
            else
            {
                connPtr->pmCounter.ll_conn_event_timeout_cnt ++;
                connPtr->rx_timeout = 1;
            }
        }
        else
        {
            // timeout case
            connPtr->rx_timeout = 1;

            //20181014 ZQ: perStats
            if(p_perStatsByChan!=NULL)
                p_perStatsByChan->rxToCnt[connPtr->currentChan]++;
        }

        // CRC error counter
        if ((irq_status & LIRQ_CERR2)
                || (irq_status & LIRQ_CERR ))
        {
            connPtr->pmCounter.ll_recv_crcerr_event_cnt ++;
            rfCounters.numRxNotOk = 1;                 // CRC Error
        }

        // Tx done OK counter
        ack_num = ll_hw_get_txAck();
        rfCounters.numTxDone = ack_num;
        uint8_t curr_llMode = connPtr->llMode;
        // update the LL HW engine mode and save the sn, nesn
        connPtr->llMode = ll_hw_update(connPtr->llMode,         // Attention: this mode is not real HW mode
                                       &(rfCounters.numTxAck),
                                       &(rfCounters.numRxOk),
                                       &(connPtr->sn_nesn));

        //20200128 ZQ: perStats
        if(p_perStatsByChan!=NULL)
        {
            p_perStatsByChan->TxNumAck[connPtr->currentChan]+=ack_num;
            p_perStatsByChan->txNumRetry[connPtr->currentChan]+=ll_hw_get_nAck();
            uint8_t crcErrNum,rxTotalNum,rxPktNum;
            ll_hw_get_rxPkt_stats(&crcErrNum,&rxTotalNum,&rxPktNum);
            p_perStatsByChan->rxNumPkts[connPtr->currentChan]+=rxTotalNum;
            p_perStatsByChan->rxNumCrcErr[connPtr->currentChan]+=crcErrNum;
        }

        // ==== check HW Tx FIFO, read packets which not transmit or transit but not receive ACK
        tx_num = (*(volatile uint32_t*)(LL_HW_BASE + 0x04) >> 8) & 0xff;

        if (!(curr_llMode==LL_HW_TRLP_EMPT && tx_num==1)  //when only tx empty pkt in fifo
                && (irq_status & LIRQ_RTO)
                && (ack_num < tx_num) )   // receive time out, there are bugs in LL HW process TFIFO pointer, recover the pointer by SW
        {
            ll_hw_process_RTO(ack_num);
        }

        ll_hw_read_tfifo_rtlp();      // reused rtlp function
        // update the numTxCtrlAck counter, add on 2017-11-15
        rfCounters.numTxCtrlAck = 0;

        if (connPtr->ctrlDataIsProcess == 1              // control packet in this RTLP event TFIFO
                && ack_num > 0                        // get ACK in this event
                && (connPtr->ll_buf.tx_not_ack_pkt->valid == 0  // no not_ack packet
                    || (connPtr->ll_buf.tx_not_ack_pkt->header & 0x3) != LL_DATA_PDU_HDR_LLID_CONTROL_PKT))  // not_ack packet is not ctrl packet
        {
            rfCounters.numTxCtrlAck = 1;
            connPtr->ctrlDataIsProcess = 0;
        }

        // if receive some packets, read them
        if (rfCounters.numRxOk)
        {
            // read HW Rx FIFO to internal buffer
            ll_read_rxfifo();
        }

        // call master process task
        llMasterEvt_TaskEndOk();
//      hal_gpio_write(GPIO_P18, 0);

        //20181014 ZQ:
        // connection event notify
        if (g_conn_taskID != 0)
            osal_set_event(g_conn_taskID, g_conn_taskEvent);

//#ifndef MULTI_ROLE
//      // switch connection context
//      g_ll_conn_ctx.currentConn = ll_get_next_active_conn(g_ll_conn_ctx.currentConn);
//#endif
    }
    // =================== other mode(STX, RTX), no process(send SCAN RSP done) or no used
    // === A2 add for simultaneous connect event & adv event
    // conn-adv case 1: STX ISR, continue broadcast left sec adv channels
    else if ((llSecondaryState == LL_SEC_STATE_ADV || llSecondaryState == LL_SEC_STATE_IDLE_PENDING)
             && (mode == LL_HW_MODE_STX ))
    {
        // secondary adv state
        uint8 i;
        #ifdef DEBUG_LL
        LOG("Sec Adv\r\n");
        #endif
        i = 0;

        while (!(adv_param.advChanMap & (1 << i)))   i ++;    // get the 1st adv channel

        // adv_param.advNextChan stores the next adv channel, when adv the last adv channel, advNextChan should equal 1st adv channel
        if (adv_param.advNextChan != (LL_ADV_CHAN_FIRST + i))           // not finish adv the last channel, continue adv
        {
            llSetupSecAdvEvt();
        }
        else
        {
            if (llSecondaryState == LL_SEC_STATE_IDLE_PENDING)         // advertise last channel and transiting to IDLE
                llSecondaryState = LL_SEC_STATE_IDLE;
            else                                                       // otherwise, schedule next adv
                osal_start_timerEx(LL_TaskID, LL_EVT_SECONDARY_ADV, (adv_param.advInterval * 5) >> 3);   // * 625 / 1000
        }

        ll_debug_output(DEBUG_LL_HW_STX);
    }
    // multi-connection, support connectable/scannable adv
    else if ((llSecondaryState == LL_SEC_STATE_ADV || llSecondaryState == LL_SEC_STATE_IDLE_PENDING)
             && (mode == LL_HW_MODE_TRX )
             && (adv_param.advEvtType == LL_ADV_CONNECTABLE_UNDIRECTED_EVT || adv_param.advEvtType == LL_ADV_SCANNABLE_UNDIRECTED_EVT))
    {
        // secondary adv state, connectable adv or scannable adv
        uint8_t  packet_len, pdu_type, txAdd;
        uint16_t pktLen;
        uint32_t pktFoot0, pktFoot1;
        int      calibra_time;                 // this parameter will be provided by global_config
        //int      i;
//        ll_debug_output(DEBUG_LL_HW_TRX);
        // read packet
        packet_len = ll_hw_read_rfifo((uint8_t*)(&(g_rx_adv_buf.rxheader)),
                                      &pktLen,
                                      &pktFoot0,
                                      &pktFoot1);

        if(ll_hw_get_rfifo_depth() > 0)
        {
            g_pmCounters.ll_rfifo_read_err++;
            packet_len=0;
            pktLen=0;
        }

        // check receive pdu type
        pdu_type = g_rx_adv_buf.rxheader & PDU_TYPE_MASK;
        txAdd    = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random

        if (packet_len > 0                       // any better checking rule for rx anything?
                && (irq_status & LIRQ_COK)
                && pdu_type == ADV_SCAN_REQ)
//          && (llState == LL_STATE_ADV_UNDIRECTED
//              || llState == LL_STATE_ADV_SCAN))
        {
            // 1. scan req
            g_pmCounters.ll_recv_scan_req_cnt ++;

            // check AdvA
            if (g_rx_adv_buf.data[6]  != adv_param.ownAddr[0]
                    || g_rx_adv_buf.data[7]  != adv_param.ownAddr[1]
                    || g_rx_adv_buf.data[8]  != adv_param.ownAddr[2]
                    || g_rx_adv_buf.data[9]  != adv_param.ownAddr[3]
                    || g_rx_adv_buf.data[10] != adv_param.ownAddr[4]
                    || g_rx_adv_buf.data[11] != adv_param.ownAddr[5])
            {
            }
            else
            {
//===
                uint8_t  rpaListIndex, bWlRlCheckOk;
                uint8_t* peerAddr = &g_rx_adv_buf.data[0];      // ScanA

                // === Resolving list checking
                if (txAdd == LL_DEV_ADDR_TYPE_RANDOM
                        && (g_rx_adv_buf.data[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR)
                {
                    bWlRlCheckOk = TRUE;

                    // if ScanA is resolvable private address
                    if (g_llRlEnable == TRUE)
                    {
                        bWlRlCheckOk = FALSE;
                        rpaListIndex = ll_getRPAListEntry(&g_rx_adv_buf.data[0]);

                        if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                        {
                            peerAddr = &g_llResolvinglist[rpaListIndex].peerAddr[0];
                            bWlRlCheckOk = TRUE;
                        }
                    }
                }
                else   // ScanA is device Identity, if the device ID in the RPA list, check whether RPA should be used
                {
                    bWlRlCheckOk = TRUE;

                    for (int i = 0; i < LL_RESOLVINGLIST_ENTRY_NUM; i++)
                    {
                        if (g_llResolvinglist[i].peerAddr[0] == g_rx_adv_buf.data[0]
                                && g_llResolvinglist[i].peerAddr[1] == g_rx_adv_buf.data[1]
                                && g_llResolvinglist[i].peerAddr[2] == g_rx_adv_buf.data[2]
                                && g_llResolvinglist[i].peerAddr[3] == g_rx_adv_buf.data[3]
                                && g_llResolvinglist[i].peerAddr[4] == g_rx_adv_buf.data[4]
                                && g_llResolvinglist[i].peerAddr[5] == g_rx_adv_buf.data[5]
                                && g_llResolvinglist[i].peerAddrType == txAdd)
                        {
                            if (g_llResolvinglist[i].privacyMode == NETWORK_PRIVACY_MODE &&
                                    !ll_isIrkAllZero(g_llResolvinglist[i].peerIrk))
                                bWlRlCheckOk = FALSE;

                            break;
                        }
                    }
                }

                // === check white list
                if ((pGlobal_config[LL_SWITCH] & LL_WHITELIST_ALLOW)
                        && (adv_param.wlPolicy  == LL_ADV_WL_POLICY_WL_SCAN_REQ
                            || adv_param.wlPolicy  == LL_ADV_WL_POLICY_WL_ALL_REQ)
                        && (bWlRlCheckOk == TRUE))
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
                    uint8 retScanRspFilter=1;

                    if(LL_PLUS_ScanRequestFilterCBack)
                    {
                        retScanRspFilter = LL_PLUS_ScanRequestFilterCBack();
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
                        delay = 118 - delay - calibra_time;                       // IFS = 150us, Tx tail -> Rx done time: about 32us
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
                 && (irq_status & LIRQ_COK) )
//                && (llState == LL_STATE_ADV_UNDIRECTED
//                   || llState == LL_STATE_ADV_DIRECTED))
        {
            uint8_t*  peerAddr;
            uint8_t  bWlRlCheckOk = TRUE;
            // 2. connect req
            g_pmCounters.ll_recv_conn_req_cnt ++;

            // check AdvA
            if (g_rx_adv_buf.data[6]  != adv_param.ownAddr[0]
                    || g_rx_adv_buf.data[7]  != adv_param.ownAddr[1]
                    || g_rx_adv_buf.data[8]  != adv_param.ownAddr[2]
                    || g_rx_adv_buf.data[9]  != adv_param.ownAddr[3]
                    || g_rx_adv_buf.data[10] != adv_param.ownAddr[4]
                    || g_rx_adv_buf.data[11] != adv_param.ownAddr[5])
            {
                // nothing to do
            }
            else
            {
                uint8_t  rpaListIndex = LL_RESOLVINGLIST_ENTRY_NUM;
                peerAddr = &g_rx_adv_buf.data[0];        // initA

                // ====== check Resolving list
                if (txAdd == LL_DEV_ADDR_TYPE_RANDOM   &&
                        (g_rx_adv_buf.data[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR)
                {
                    bWlRlCheckOk = TRUE;

                    if (g_llRlEnable == TRUE)
                    {
                        bWlRlCheckOk = FALSE;
                        rpaListIndex = ll_getRPAListEntry(&g_rx_adv_buf.data[0]);

                        if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                        {
                            // save resolved peer address
                            peerAddr = &g_llResolvinglist[rpaListIndex].peerAddr[0];
                            // if resolved address success, map the peer address type to 0x02 or 0x03
                            g_currentPeerAddrType = g_llResolvinglist[rpaListIndex].peerAddrType + 2;
                            osal_memcpy( &g_currentPeerRpa[0],  &g_rx_adv_buf.data[0], 6);   // save latest peer RPA
                            bWlRlCheckOk = TRUE;
                        }
                    }
                }
                else   // InitA is device Identity, check whether the device Addr in the RPA list, if it is
                {
                    // in the RPA list and network privacy mode is selected and non all-0 IRK, check failed
                    bWlRlCheckOk = TRUE;

                    for (int i = 0; i < LL_RESOLVINGLIST_ENTRY_NUM; i++)
                    {
                        if (g_llResolvinglist[i].peerAddr[0] == g_rx_adv_buf.data[0]
                                && g_llResolvinglist[i].peerAddr[1] == g_rx_adv_buf.data[1]
                                && g_llResolvinglist[i].peerAddr[2] == g_rx_adv_buf.data[2]
                                && g_llResolvinglist[i].peerAddr[3] == g_rx_adv_buf.data[3]
                                && g_llResolvinglist[i].peerAddr[4] == g_rx_adv_buf.data[4]
                                && g_llResolvinglist[i].peerAddr[5] == g_rx_adv_buf.data[5]
                                && g_llResolvinglist[i].peerAddrType == txAdd)
                        {
                            if (g_llResolvinglist[i].privacyMode == NETWORK_PRIVACY_MODE &&
                                    !ll_isIrkAllZero(g_llResolvinglist[i].peerIrk))
                                bWlRlCheckOk = FALSE;

                            break;
                        }
                    }
                }

                // ====== check white list
                if ((pGlobal_config[LL_SWITCH] & LL_WHITELIST_ALLOW)
                        && (llState == LL_STATE_ADV_UNDIRECTED)
                        && (adv_param.wlPolicy   == LL_ADV_WL_POLICY_WL_CONNECT_REQ
                            || adv_param.wlPolicy  == LL_ADV_WL_POLICY_WL_ALL_REQ)
                        && (bWlRlCheckOk == TRUE))
                {
                    // check white list
                    bWlRlCheckOk = ll_isAddrInWhiteList(txAdd, peerAddr);
                }

                // fixed bug 2018-09-25, LL/CON/ADV/BV-04-C, for direct adv, initA should equal peer Addr
                if (llState == LL_STATE_ADV_DIRECTED)
                {
                    if (//txAdd         != peerInfo.peerAddrType    // for (extended) set adv param, peer addr type could only be 0x0 or 0x01
                        peerAddr[0]  != peerInfo.peerAddr[0]
                        || peerAddr[1]  != peerInfo.peerAddr[1]
                        || peerAddr[2]  != peerInfo.peerAddr[2]
                        || peerAddr[3]  != peerInfo.peerAddr[3]
                        || peerAddr[4]  != peerInfo.peerAddr[4]
                        || peerAddr[5]  != peerInfo.peerAddr[5])
                    {
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
                    peerInfo.peerAddrType = txAdd;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random
                    osal_memcpy(peerInfo.peerAddr, &peerAddr[0], 6);
                    move_to_slave_function();    // move to slave role for connection state
                }
            }
        }
        //test for fast adv
        else //if(llState == LL_STATE_ADV_UNDIRECTED)
        {
            // adv in next channel, or schedule next adv event
            uint8 i = 0;

            while (!(adv_param.advChanMap & (1 << i)))   i ++;    // get the 1st adv channel

            // adv_param.advNextChan stores the next adv channel, when adv the last adv channel, advNextChan should equal 1st adv channel
            if (adv_param.advNextChan != (LL_ADV_CHAN_FIRST + i))           // not finish adv the last channel, continue adv
            {
                llSetupSecAdvEvt();
            }
            else
            {
                if (llSecondaryState == LL_SEC_STATE_IDLE_PENDING)         // advertise last channel and transiting to IDLE
                    llSecondaryState = LL_SEC_STATE_IDLE;
                else                                                       // otherwise, schedule next adv
                    osal_start_timerEx(LL_TaskID, LL_EVT_SECONDARY_ADV, (adv_param.advInterval * 5) >> 3);   // * 625 / 1000
            }

//            uint8_t firstAdvChan = (adv_param.advChanMap&LL_ADV_CHAN_37) !=0 ? 37 :
//                                   (adv_param.advChanMap&LL_ADV_CHAN_38) !=0 ? 38 : 39;
//
//            if(adv_param.advNextChan>firstAdvChan)
//            {
//                //llSetupUndirectedAdvEvt1();
//                ll_schedule_next_event(50);         //20180623 modified by ZQ
//                                                    // reset the timer1 instead of llSetupUndirectedAdvEvt1
//                                                    // reduced the process time in LL_IRQ
//                                                    // llSetupUndirectedAdvEvt1 will cost about 120us
//            }
            //llWaitingIrq = TRUE;
        }
    }
    // TODO: add RPA list checking, identical to single role case
    else if (mode == LL_HW_MODE_SRX
             && llSecondaryState == LL_SEC_STATE_SCAN)
    {
        // check status
        if ((irq_status & LIRQ_RD) && (irq_status & LIRQ_COK))       // bug correct 2018-10-15
        {
            // rx done
            uint8_t packet_len, pdu_type;
            uint16_t pktLen;
            uint32_t pktFoot0, pktFoot1;
            // read packet
            // cost 21-26us(measure with GPIO), depneds on the length of ADV
            packet_len = ll_hw_read_rfifo((uint8_t*)(&(g_rx_adv_buf.rxheader)),
                                          &pktLen,
                                          &pktFoot0,
                                          &pktFoot1);
            // check receive pdu type
            pdu_type = g_rx_adv_buf.rxheader & 0x0f;

            if (packet_len   != 0
                    && ((pdu_type == ADV_IND)
                        || (pdu_type  == ADV_NONCONN_IND)
                        || (pdu_type  == ADV_SCAN_IND)))
            {
                int     i = 0;
                uint8_t txAdd = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random

                // check white list
                if ((pGlobal_config[LL_SWITCH] & LL_WHITELIST_ALLOW)
                        && (scanInfo.wlPolicy  == LL_SCAN_WL_POLICY_USE_WHITE_LIST))
                {
                    // check white list
                    for (i = 0; i < LL_WHITELIST_ENTRY_NUM; i++)
                    {
                        if (txAdd                 != g_llWhitelist[i].peerAddrType
                                || g_rx_adv_buf.data[0]  != g_llWhitelist[i].peerAddr[0]
                                || g_rx_adv_buf.data[1]  != g_llWhitelist[i].peerAddr[1]
                                || g_rx_adv_buf.data[2]  != g_llWhitelist[i].peerAddr[2]
                                || g_rx_adv_buf.data[3]  != g_llWhitelist[i].peerAddr[3]
                                || g_rx_adv_buf.data[4]  != g_llWhitelist[i].peerAddr[4]
                                || g_rx_adv_buf.data[5]  != g_llWhitelist[i].peerAddr[5])
                        {
                            // not match, check next
                            continue;
                        }
                        else
                            break;
                    }
                }

                // if valid, trigger osal event to report adv
                if (i < LL_WHITELIST_ENTRY_NUM)
                {
                    uint8  advEventType;
                    int8   rssi;
                    llCurrentScanChn = scanInfo.nextScanChan;

                    // no active scan scenario

                    // convert pdu type to GAP enum
                    switch (pdu_type)
                    {
                    case ADV_IND:
                        advEventType = LL_ADV_RPT_ADV_IND;
                        break;

                    case ADV_SCAN_IND:
                        advEventType = LL_ADV_RPT_ADV_SCANNABLE_IND;
                        break;

                    case ADV_DIRECT_IND:
                        advEventType = LL_ADV_RPT_ADV_DIRECT_IND;
                        break;

                    case ADV_NONCONN_IND:
                        advEventType = LL_ADV_RPT_ADV_NONCONN_IND;
                        break;

                    case ADV_SCAN_RSP:
                        advEventType = LL_ADV_RPT_INVALID;
                        break;

                    default:
                        advEventType = LL_ADV_RPT_ADV_IND;
                        break;
                    }

                    rssi  =  -(pktFoot1 >> 24);
                    // below function cost 51us/66us(measure with GPIO)
                    LL_AdvReportCback( advEventType,                         // event type
                                       txAdd,                                // Adv address type (TxAdd)
                                       &g_rx_adv_buf.data[0],       // Adv address (AdvA)
                                       pktLen - 8,                           // length of rest of the payload, 2 - header, 6 - advA
                                       &g_rx_adv_buf.data[6],       // rest of payload
                                       rssi );                               // RSSI
                    g_pmCounters.ll_recv_adv_pkt_cnt ++;
                }
            }
        }

        //  update scan time
        llScanTime += ((ISR_entry_time > llScanT1) ? (ISR_entry_time - llScanT1) : (BASE_TIME_UNITS - llScanT1 + ISR_entry_time));

        if (llScanTime >= scanInfo.scanWindow * 625)
        {
            // switch scan channel, set event instead of trigger immediately
            // calculate next scan channel
            LL_CALC_NEXT_SCAN_CHN(scanInfo.nextScanChan);

            // schedule next scan event
            if (scanInfo.scanWindow == scanInfo.scanInterval)      // scanWindow == scanInterval, trigger immediately
                osal_set_event(LL_TaskID, LL_EVT_SECONDARY_SCAN);
            else
                osal_start_timerEx(LL_TaskID, LL_EVT_SECONDARY_SCAN, ((scanInfo.scanInterval - scanInfo.scanWindow) * 5) >> 3 );

            // reset scan total time
            llScanTime = 0;
        }
        else if (llSecondaryState == LL_SEC_STATE_SCAN)
            llSetupSecScan(scanInfo.nextScanChan);
    }
    // ======= A2 multi-connection
    // TODO: add RPA list checking, identical to single role case
    else if (mode == LL_HW_MODE_SRX
             && llSecondaryState == LL_SEC_STATE_INIT)   // TODO: consider whether could merge to LL_STATE_INIT branch
    {
        uint8 bConnecting = FALSE;
//          hal_gpio_write(GPIO_P18, 0);
        connPtr = &conn_param[initInfo.connId];           // connId is allocated when create conn

        // check status
        if ((irq_status & LIRQ_RD) && (irq_status & LIRQ_COK))       // bug correct 2018-10-15
        {
            // rx done
            uint8_t packet_len, pdu_type;
            uint16_t pktLen;
            uint32_t pktFoot0, pktFoot1;
            // read packet
            // cost 21-26us(measure with GPIO), depneds on the length of ADV
            packet_len = ll_hw_read_rfifo((uint8_t*)(&(g_rx_adv_buf.rxheader)),
                                          &pktLen,
                                          &pktFoot0,
                                          &pktFoot1);
            // check receive pdu type
            pdu_type = g_rx_adv_buf.rxheader & 0x0f;

            if(ll_hw_get_rfifo_depth()>0)
            {
                g_pmCounters.ll_rfifo_read_err++;
                packet_len=0;
                pktLen=0;
            }

            if (packet_len   != 0
                    && ((pdu_type == ADV_IND)))
            {
                uint8_t txAdd = (g_rx_adv_buf.rxheader & TX_ADD_MASK) >> TX_ADD_SHIFT;    // adv PDU header, bit 6: TxAdd, 0 - public, 1 - random
                uint8_t chSel = (g_rx_adv_buf.rxheader & CHSEL_MASK) >> CHSEL_SHIFT;
                uint8_t bWlRlCheckOk = TRUE;
                uint8_t* peerAddr;
                uint8_t rpaListIndex = LL_RESOLVINGLIST_ENTRY_NUM;
//-====
                peerAddr = &g_rx_adv_buf.data[0];        // AdvA
                g_currentPeerAddrType = txAdd;

                // Resolving list checking
                // case 1: receive InitA using RPA
                if (txAdd == LL_DEV_ADDR_TYPE_RANDOM  &&
                        (g_rx_adv_buf.data[5] & RANDOM_ADDR_HDR) == PRIVATE_RESOLVE_ADDR_HDR)
                {
                    bWlRlCheckOk = FALSE;

                    if (g_llRlEnable == TRUE)
                    {
                        rpaListIndex = ll_getRPAListEntry(&g_rx_adv_buf.data[0]);

                        if (rpaListIndex < LL_RESOLVINGLIST_ENTRY_NUM)
                        {
                            peerAddr = &g_llResolvinglist[rpaListIndex].peerAddr[0];
                            g_currentPeerAddrType = g_llResolvinglist[rpaListIndex].peerAddrType + 2;
                            osal_memcpy(&g_currentPeerRpa[0], &g_rx_adv_buf.data[0], 6);
                            bWlRlCheckOk = TRUE;
                        }
                    }
                }
                else     // case 2: receive InitA using device ID, or init device not using RPA
                {
                    bWlRlCheckOk = TRUE;

                    for (int i = 0; i < LL_RESOLVINGLIST_ENTRY_NUM; i++)
                    {
                        if ( g_llResolvinglist[i].peerAddr[0] == g_rx_adv_buf.data[0]
                                && g_llResolvinglist[i].peerAddr[1] == g_rx_adv_buf.data[1]
                                && g_llResolvinglist[i].peerAddr[2] == g_rx_adv_buf.data[2]
                                && g_llResolvinglist[i].peerAddr[3] == g_rx_adv_buf.data[3]
                                && g_llResolvinglist[i].peerAddr[4] == g_rx_adv_buf.data[4]
                                && g_llResolvinglist[i].peerAddr[5] == g_rx_adv_buf.data[5])
                        {
                            // the device ID in the RPA list
                            if (g_llResolvinglist[i].privacyMode == DEVICE_PRIVACY_MODE ||
                                    ll_isIrkAllZero(g_llResolvinglist[i].peerIrk))
                                rpaListIndex = i;
                            else
                                bWlRlCheckOk = FALSE;      // the device in the RPA list but not using RPA, reject it

                            break;
                        }
                    }
                }

                // initiator, 2 types of filter process: 1. connect to peer address set by host   2. connect to  address in whitelist only
                // 1. connect to peer address set by host
                if (initInfo.wlPolicy == LL_INIT_WL_POLICY_USE_PEER_ADDR
                        && bWlRlCheckOk == TRUE)
                {
                    if (//txAdd          != peerInfo.peerAddrType
                        peerAddr[0]  != peerInfo.peerAddr[0]
                        || peerAddr[1]  != peerInfo.peerAddr[1]
                        || peerAddr[2]  != peerInfo.peerAddr[2]
                        || peerAddr[3]  != peerInfo.peerAddr[3]
                        || peerAddr[4]  != peerInfo.peerAddr[4]
                        || peerAddr[5]  != peerInfo.peerAddr[5])
                    {
                        // not match, not init connect
                        bWlRlCheckOk = FALSE;
                    }
                }
                // 2. connect to  address in whitelist only
                else if (initInfo.wlPolicy == LL_INIT_WL_POLICY_USE_WHITE_LIST &&
                         bWlRlCheckOk == TRUE)
                {
                    // if advA in whitelist list, connect
                    // check white list
                    bWlRlCheckOk = ll_isAddrInWhiteList(txAdd, peerAddr);
                }

                if (bWlRlCheckOk == TRUE)
                {
                    g_same_rf_channel_flag = TRUE;
                    // calculate connPtr->curParam.winOffset and set tx buffer
                    uint16  win_offset;
                    uint32  remainder;

                    // calculate windows offset in multiconnection case
                    if (g_ll_conn_ctx.currentConn != LL_INVALID_CONNECTION_ID)
                    {
//#ifdef MULTI_ROLE
                        // allocate time slot for new connection
                        // calculate delta to current connection
                        // calculate new win_offset
                        uint32 temp, temp1, temp2;
                        int   i;

                        for (i = 0; i < g_maxConnNum; i++ )
                        {
                            if (g_ll_conn_ctx.scheduleInfo[i].linkRole == LL_ROLE_MASTER && conn_param[i].active)
                                break;
                        }

                        if (i == g_maxConnNum)
                        {
                            // case 1:  no master connection, schedule new connection after the current slave connection
                            g_new_master_delta = 12 * 625;                       // delta time to the current slave event
                            remainder = read_LL_remainder_time();
                            g_new_master_delta += remainder;
                            remainder = g_new_master_delta - 352;             // time of CONN_REQ
                            remainder = (remainder + (remainder >> 1) + (remainder >> 3) + (remainder >> 7)) >> 10;     // rough estimate of (x / 625) = (1/1024 + 1/2048 + 1/8192)

                            // winoffset should less then conn interval
                            if (g_new_master_delta - 2 > (uint32)(conn_param[initInfo.connId].curParam.connInterval << 1))      // win_offset should less then conn interval
                                g_new_master_delta -= conn_param[initInfo.connId].curParam.connInterval << 1;

                            win_offset = (remainder - 2) >> 1;
                        }
                        else
                        {
                            // case 2:  master connection exist, select the 1st master connection as anchor master connection

                            // calculate the delta to the anchor master connection
                            if (initInfo.connId > i)
                                g_new_master_delta = (initInfo.connId - i) * g_ll_conn_ctx.per_slot_time;
                            else
                                g_new_master_delta = (conn_param[i].curParam.connInterval << 1) - (i - initInfo.connId) * g_ll_conn_ctx.per_slot_time;

                            // schedule the new connection after the anchor master connection
                            g_new_master_delta = g_new_master_delta * 625 + g_ll_conn_ctx.scheduleInfo[i].remainder;
                            // elapse time since last schedule
                            temp1 = g_ll_conn_ctx.current_timer - ((AP_TIM1->CurrentCount) >> 2) + 2;
                            g_new_master_delta -= temp1;

                            if (g_new_master_delta - 1250 > (conn_param[initInfo.connId].curParam.connInterval * 1250))     // win_offset should less then conn interval
                                g_new_master_delta -= conn_param[initInfo.connId].curParam.connInterval * 1250;

                            // calculate win_offset
                            temp = g_new_master_delta - 352;            // 352: CONN_REQ time
                            temp2 = (temp + (temp >> 1) + (temp >> 3) + (temp >> 7)) >> 10;           // rough estimate of (x / 625)
                            win_offset = (temp2 - 2) >> 1;
                            // calculate remainder time of anchor master connection
//                                temp1 = (CP_TIM1->LoadCount - CP_TIM1->CurrentCount) >> 2;     // get elapse time //read_LL_remainder_time();
//                                temp1 = g_ll_conn_ctx.current_timer - ((CP_TIM1->CurrentCount) >> 2) + 2;   // 2: rough time from read old timer1 to kick new timer1
//                              temp = (g_ll_conn_ctx.scheduleInfo[i].remainder - temp1 - 352);// / 625;
//                              temp2 = (temp + (temp >> 1) + (temp >> 3) + (temp >> 7)) >> 10;           // rough estimate of (x / 625)
//
//                                // remainder time of new connection = remainder time of anchor master connection + delta
//                              g_new_master_delta += temp2;
//
//                              // winoffset should less then conn interval
//                              if (g_new_master_delta - 2 > (conn_param[initInfo.connId].curParam.connInterval << 1))      // win_offset should less then conn interval
//                                  g_new_master_delta -= conn_param[initInfo.connId].curParam.connInterval << 1;
//
//                              win_offset = (g_new_master_delta - 2) >> 1;
//                              g_new_master_delta = win_offset * 1250 + 352;
                        }

//#else
//                          if (initInfo.connId > g_ll_conn_ctx.currentConn)
//                              g_new_master_delta = (initInfo.connId - g_ll_conn_ctx.currentConn) * g_ll_conn_ctx.per_slot_time;
//                          else
//                              g_new_master_delta = (conn_param[initInfo.connId].curParam.connInterval << 1) - (g_ll_conn_ctx.currentConn - initInfo.connId) * g_ll_conn_ctx.per_slot_time;
//
//                          //  there are 2 case for new connection timing : 1. before next current connection slot  2. after next current connection slot.
//                          // Note: we will send the 1st master packet at time (1.25ms + winoffset) after send CONN REQ msg,
//                          //       the time should align to allocate time slot, i.e.
//                          //        remain time of timer1 + delta tick  = 2 + winOffset + CONN REQ msg length(352us)
//                          remainder = (read_LL_remainder_time() - 352);//  / 625;
//                          remainder = (remainder + (remainder >> 1) + (remainder >> 3) + (remainder >> 7)) >> 10;     // rough estimate of (x / 625) = (1/1024 + 1/2048 + 1/8192)
//
//                          win_offset = (remainder + g_new_master_delta - 2) >> 1;
//                          if (win_offset > (conn_param[initInfo.connId].curParam.connInterval << 1))      // case 1
//                              win_offset -= (conn_param[initInfo.connId].curParam.connInterval << 1);
//
////                            g_new_master_delta = win_offset << 1;
//                          g_new_master_delta = win_offset * 1250 + 352;
//#endif
                        // WinOffset, Byte 20 ~ 21
                        memcpy((uint8*)&g_tx_adv_buf.data[20], (uint8*)&win_offset, 2);
                        conn_param[initInfo.connId].curParam.winOffset = win_offset;
                    }

                    // channel selection algorithm decision
                    if ((pGlobal_config[LL_SWITCH] & CONN_CSA2_ALLOW)
                            && chSel == LL_CHN_SEL_ALGORITHM_2)
                    {
                        conn_param[initInfo.connId].channel_selection = LL_CHN_SEL_ALGORITHM_2;
                        SET_BITS(g_tx_adv_buf.txheader, LL_CHN_SEL_ALGORITHM_2, CHSEL_SHIFT, CHSEL_MASK);
                    }
                    else
                        conn_param[initInfo.connId].channel_selection = LL_CHN_SEL_ALGORITHM_1;

                    // send conn req
                    T2 = read_current_fine_time();
                    delay = (T2 > ISR_entry_time) ? (T2 - ISR_entry_time) : (BASE_TIME_UNITS - ISR_entry_time + T2);
                    delay = 118 - delay - pGlobal_config[LL_ADV_TO_CONN_REQ_DELAY];
                    ll_hw_set_trx_settle(delay,                               // set BB delay, about 80us in 16MHz HCLK
                                         pGlobal_config[LL_HW_AFE_DELAY],
                                         pGlobal_config[LL_HW_PLL_DELAY]);        //RxAFE,PLL
                    // reset Rx/Tx FIFO
                    ll_hw_rst_rfifo();
                    ll_hw_rst_tfifo();
                    // send conn req
                    ll_hw_set_stx();             // set LL HW as single Tx mode
                    ll_hw_go();
                    llWaitingIrq = TRUE;
                    // AdvA, offset 6
                    memcpy((uint8*)&g_tx_adv_buf.data[6], &g_rx_adv_buf.data[0], 6);
                    //write Tx FIFO
                    ll_hw_write_tfifo((uint8*)&(g_tx_adv_buf.txheader),
                                      ((g_tx_adv_buf.txheader & 0xff00) >> 8) + 2);   // payload length + header length(2)
                    move_to_master_function();
                    //LOG("win_off = %d\n", win_offset);
                    //LOG("remainder = %d\n", remainder);
                    bConnecting = TRUE;
                    g_same_rf_channel_flag = FALSE;
                }
            }
            else if (packet_len   != 0
                     && (pdu_type == ADV_DIRECT_IND))     // TODO: add process of direct ADV
            {
            }
        }

        // scan again if not start connect
        if (!bConnecting)           // if not start connect, schedule next scan
        {
            if (initInfo.scanMode == LL_SCAN_STOP)
            {
                // scan has been stopped
                llSecondaryState = LL_SEC_STATE_IDLE;                    // bug fixed by Zhufei                             // set the LL state idle
                //  release the associated allocated connection
                llReleaseConnId(connPtr);                                                // new for multi-connection
                g_ll_conn_ctx.numLLMasterConns --;
                (void)osal_set_event( LL_TaskID, LL_EVT_MASTER_CONN_CANCELLED );         // inform high layer
            }
            else
            {
                // not sending SCAN REQ, update scan time
                llScanTime += ((ISR_entry_time > llScanT1) ? (ISR_entry_time - llScanT1) : (BASE_TIME_UNITS - llScanT1 + ISR_entry_time));

                if (llScanTime >= initInfo.scanWindow * 625)
                {
                    // calculate next scan channel
                    LL_CALC_NEXT_SCAN_CHN(initInfo.nextScanChan);

                    // schedule next scan event
                    if (initInfo.scanWindow == initInfo.scanInterval)      // scanWindow == scanInterval, trigger immediately
                        osal_set_event(LL_TaskID, LL_EVT_SECONDARY_INIT);
                    else
                        osal_start_timerEx(LL_TaskID, LL_EVT_SECONDARY_INIT, ((initInfo.scanInterval - initInfo.scanWindow) * 5) >> 3 );

                    // reset scan total time
                    llScanTime = 0;
                }
                else
                    llSetupSecInit(initInfo.nextScanChan);
            }
        }
    }

    // post ISR process
    if (!llWaitingIrq)                      // bug fixed 2018-05-04, only clear IRQ status when no config new one
        ll_hw_clr_irq();

    HAL_EXIT_CRITICAL_SECTION();
    return TRUE;
}

/*******************************************************************************
    @fn          llSetupExtAdvLegacyEvent

    @brief       This function will setup ext adv event with Legacy PDU
                1. fill ext adv pdu(EXT_ADV_IND or AUX_XXX_IND)
                2. update timer info for next chn EXT_ADV_IND or AUX_XXX_IND

    input parameters

    @param       None.

    output== parameters

    @param       None.

    @return      LL_STATUS_SUCCESS
*/
uint8 llSetupExtAdvLegacyEvent(extAdvInfo_t*  pAdvInfo)
{
    uint8 ch_idx, pktFmt;
    int i;
    uint8 pduType = ADV_IND;
    ch_idx = pAdvInfo->currentChn;
    LOG("<%d> ", pAdvInfo->currentChn);

    if (ch_idx < LL_ADV_CHAN_FIRST || ch_idx > LL_ADV_CHAN_LAST )
        return FALSE;

    // fill advertisement PDU
    g_tx_ext_adv_buf.txheader = 0;

    // AdvA
    if (pAdvInfo->parameter.ownAddrType == LL_DEV_ADDR_TYPE_RANDOM && pAdvInfo->parameter.isOwnRandomAddressSet == TRUE)
        memcpy(&g_tx_ext_adv_buf.data[0], pAdvInfo->parameter.ownRandomAddress, LL_DEVICE_ADDR_LEN);
    else    // public address
        memcpy(&g_tx_ext_adv_buf.data[0], ownPublicAddr, LL_DEVICE_ADDR_LEN);

    switch (pAdvInfo->parameter.advEventProperties)
    {
    case LL_EXT_ADV_PROP_ADV_IND:
        pduType = ADV_IND;
        // Length
        SET_BITS(g_tx_ext_adv_buf.txheader, (6 + pAdvInfo->data.advertisingDataLength), LENGTH_SHIFT, LENGTH_MASK);
        // AdvData
        osal_memcpy((uint8_t*)&(g_tx_ext_adv_buf.data[6]), &pAdvInfo->data.advertisingData[0], pAdvInfo->data.advertisingDataLength);
        break;

    case LL_EXT_ADV_PROP_ADV_SCAN_IND:
        pduType = ADV_SCAN_IND;
        // Length
        SET_BITS(g_tx_ext_adv_buf.txheader, (6 + pAdvInfo->data.advertisingDataLength), LENGTH_SHIFT, LENGTH_MASK);
        // AdvData
        osal_memcpy((uint8_t*)&(g_tx_ext_adv_buf.data[6]), pAdvInfo->data.advertisingData, pAdvInfo->data.advertisingDataLength);
        break;

    case LL_EXT_ADV_PROP_ADV_NOCONN_IND:
        pduType = ADV_NONCONN_IND;
        // Length
        SET_BITS(g_tx_ext_adv_buf.txheader, (6 + pAdvInfo->data.advertisingDataLength), LENGTH_SHIFT, LENGTH_MASK);
        // AdvData
        osal_memcpy((uint8_t*)&(g_tx_ext_adv_buf.data[6]), pAdvInfo->data.advertisingData, pAdvInfo->data.advertisingDataLength);
        break;

    case LL_EXT_ADV_PROP_ADV_LDC_ADV:
    case LL_EXT_ADV_PROP_ADV_HDC_ADV:
        pduType = ADV_DIRECT_IND;
        // Length
        SET_BITS(g_tx_ext_adv_buf.txheader, 12, LENGTH_SHIFT, LENGTH_MASK);
        SET_BITS(g_tx_ext_adv_buf.txheader, pAdvInfo->parameter.peerAddrType, RX_ADD_SHIFT, RX_ADD_MASK);
        // initA
        osal_memcpy((uint8_t*)&(g_tx_ext_adv_buf.data[6]), &pAdvInfo->parameter.peerAddress[0], 6);
        break;

    default:
        break;
    }

    // PDU type, 4 bits
    SET_BITS(g_tx_ext_adv_buf.txheader, pduType, PDU_TYPE_SHIFT, PDU_TYPE_MASK);
    // RFU, ChSel, TxAdd, RxAdd
    SET_BITS(g_tx_ext_adv_buf.txheader, pAdvInfo->parameter.ownAddrType, TX_ADD_SHIFT, TX_ADD_MASK);

    if ((pduType == ADV_IND
            || pduType == ADV_DIRECT_IND)
            && pGlobal_config[LL_SWITCH] & CONN_CSA2_ALLOW)
        SET_BITS(g_tx_ext_adv_buf.txheader, 1, CHSEL_SHIFT, CHSEL_MASK);

    osal_memcpy( g_tx_ext_adv_buf.data,  adv_param.ownAddr, 6);
    // decide next adv channel
    i = ch_idx - LL_ADV_CHAN_FIRST + 1;

    while ((i < 3) && !(pAdvInfo->parameter.priAdvChnMap & (1 << i))) i ++;       // search channel map for next adv channel number

    if (i == 3)   // finish primary adv channel broadcast
    {
        pAdvInfo->currentChn = ll_getFirstAdvChn(pAdvInfo->parameter.priAdvChnMap);
    }
    else
        pAdvInfo->currentChn = LL_ADV_CHAN_FIRST + i;

    // Legacy Adv always in 1M PHY
    pktFmt = LE_1M_PHY;
    HAL_ENTER_CRITICAL_SECTION();

    // if there is ongoing LL HW task, skip this task
    if (llWaitingIrq)
    {
        HAL_EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    //============== configure and trigger LL HW engine, LL HW work in Single Tx mode  ==================
    rf_phy_change_cfg(pktFmt);
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
    if ((pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_CONN_BITMASK) ||
            (pAdvInfo->parameter.advEventProperties & LE_ADV_PROP_SCAN_BITMASK))
    {
        ll_hw_set_trx_settle(pGlobal_config[LL_HW_BB_DELAY_ADV],
                             pGlobal_config[LL_HW_AFE_DELAY_ADV],
                             pGlobal_config[LL_HW_PLL_DELAY_ADV]);        //TxBB,RxAFE,PLL
        ll_hw_set_trx();                      // set LL HW as Tx - Rx mode
    }
    else
    {
        ll_hw_set_stx();                      // set LL HW as Tx - Rx mode
    }

    ll_hw_ign_rfifo(LL_HW_IGN_ALL);       //set the rfifo ign control
    //write Tx FIFO
    ll_hw_write_tfifo((uint8*)&(g_tx_ext_adv_buf.txheader), ((g_tx_ext_adv_buf.txheader & 0xff00) >> 8) + 2);
//    T2 = read_current_fine_time();
//    delta = LL_TIME_DELTA(T1, T2);
//
//    temp = ( pGlobal_config[LL_EXT_ADV_PROCESS_TARGET] > delta) ? (pGlobal_config[LL_EXT_ADV_PROCESS_TARGET] - delta) : 0;
//    llWaitUs(temp);             // insert delay to make process time equal PROCESS_TARGET
    ll_hw_go();
//  hal_gpio_write(GPIO_P14, 1);
    llWaitingIrq = TRUE;
    llTaskState = LL_TASK_EXTENDED_ADV;
    HAL_EXIT_CRITICAL_SECTION();
//    hal_gpio_write(GPIO_P14, 0);
    return TRUE;
}

//============



