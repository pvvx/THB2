#include "ll.h"
#include "rf_phy_driver.h"
#include "global_config.h"
#include "jump_function.h"
#include "pwrmgr.h"
#include "uart.h"
#include "ll_sleep.h"
#include "ll_debug.h"
#include "ll.h"
#include "bus_dev.h"
#include "ll_hw_drv.h"
#include "gpio.h"
#include "ll_enc.h"
#include "OSAL_Clock.h"
#include "osal_bufmgr.h"
#include "OSAL_Memory.h"
#include "log.h"
#include "hci.h"
#include "hci_tl.h"
#include "version.h"
#include "flash.h"
#include "gatt.h"
#include "att.h"
#include "error.h"
#include "clock.h"
#include "rf_phy_driver.h"

extern uint32 g_timer4_irq_pending_time;


void LL_IRQHandler2(void)

{
    char ret;
    uint irq_status;
    int iVar2;

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
    } else(llTaskState == LL_TASK_PERIODIC_SCAN) {
        ret = ll_processPrdScanIRQ();
    } else {
        ll_processBasicIRQ(irq_status);
        ret = FALSE;
    }
    if (ret != TRUE) {

        // ================ Post ISR process: secondary pending state process
        // conn-adv case 2: other ISR, there is pending secondary advertise event, make it happen
        if (llSecondaryState == LL_SEC_STATE_ADV_PENDING) {
            if (llSecAdvAllow()) // for multi-connection case, it is possible still no enough time for adv
            {
                llSetupSecAdvEvt();
                llSecondaryState = LL_SEC_STATE_ADV;
            }
        }
        // there is pending scan event, make it happen, note that it may stay pending if there is no enough idle time
        else if (llSecondaryState == LL_SEC_STATE_SCAN_PENDING) {
            // trigger scan
            llSetupSecScan(scanInfo.nextScanChan);
        }
        // there is pending init event, make it happen, note that it may stay pending if there is no enough idle time
        else if (llSecondaryState == LL_SEC_STATE_INIT_PENDING) {
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


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void ll_adv_scheduler0(void)

{
    uint32  T2, T3, delta;
    uint32  minAuxPduTime, minPriPduTime;
    uint8   minIndexAux, minIndexPri;
    uint8   done = FALSE;
    int     i;
    uint32  minAuxPduTime, minPriPduTime;
    uint8   minIndexAux, minIndexPri;

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
    
    if ((minAuxPduTime != LL_INVALID_TIME) ||
     && (minAuxPduTime < minPriPduTime + auxPduEmphesis))   // next schedule task is aux PDU
     {
        ll_ext_adv_schedule_next_event(minAuxPduTime - delta);
        g_currentExtAdv   = minIndexAux;
	} else   // next schedule task is pri PDU
	{        
        ll_ext_adv_schedule_next_event(minPriPduTime - delta);
        g_currentExtAdv   = minIndexPri;
    }

    // update scheduler task list
    ll_updateExtAdvRemainderTime(delta);
}


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void ll_add_adv_task0(undefined * pExtAdv)

{
    undefined * puVar1;
    int isLegacy;
    uint uVar2;
    uint i;
    uint uVar4;
    undefined * puVar5;

    isLegacy = ll_isLegacyAdv();
    temp = pExtAdv->parameter.priAdvChnMap;
    chanNumber = (temp & 0x01) + ((temp & 0x02) >> 1) + ((temp & 0x04) >> 2);
    // init new Ext adv event context
    pExtAdv->currentChn = ((temp & ((~temp) + 1)) >> 1) + 37;       // calculate 1st adv channel
    pExtAdv->adv_event_counter = 0;
    pExtAdv->currentAdvOffset = 0;
    pExtAdv->adv_event_duration = 0;
    puVar1 = _g_pAdvSchInfo;

    // ======== case 1: the 1st adv task
    if (g_currentExtAdv == LL_INVALID_ADV_SET_HANDLE)     //    if (!isTimer4Running())
    {
        g_schExtAdvNum = 1;
        g_currentExtAdv = 0;          // scheduler index = 0
        p_current_scheduler = &g_pAdvSchInfo[g_currentExtAdv];
        
        if (isLegacy)            // Legacy Adv PDU has no aux PDU
        && 
        {
        
        if (!isLegacy && 
            ( pExtAdv->data.advertisingDataLength
           || (pExtAdv->advEventProperties & LE_ADV_PROP_SCAN_BITMASK)
           || (pExtAdv->advEventProperties & LE_ADV_PROP_CONN_BITMASK) ) ) {
            ll_allocAuxAdvTimeSlot(g_currentExtAdv);
        } else {
            g_pAdvSchInfo[g_currentExtAdv].auxPduRemainder = LL_INVALID_TIME;
        }
        
//      LOG("[%d]  ", g_pAdvSchInfo[i].auxPduRemainder);

        if (llWaitingIrq)
        {
            g_currentAdvTimer = pGlobal_config[LL_CONN_TASK_DURATION];
        }
        else if (isTimer1Running()
	        && ((remainder = read_LL_remainder_time()) <= pGlobal_config[LL_EXT_ADV_TASK_DURATION]))     // timer1 for connection or legacy adv
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
    if (g_currentExtAdv < spare) {
        i = spare - g_currentExtAdv;
    } else {
        i = g_extAdvNumber + spare - g_currentExtAdv;
    }
    // add new adv to adv scheduler list, not change current adv task
    p_scheduler->nextEventRemainder = p_current_scheduler[g_currentExtAdv].nextEventRemainder + (g_advSlotPeriodic >> 2) * i;
    p_scheduler->adv_handler = pExtAdv->advHandle;
    p_scheduler->pAdvInfo    = pExtAdv;
    return;
}

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void ll_delete_adv_task0(uint8 index)

{
    uint32  T1, T2, delta, remainder, elapse_time;
    uint32  minAuxPduTime, minPriPduTime;
    uint8   minIndexAux, minIndexPri;
    int i;
//  LOG("=== adv task % deleted \r\n", index);

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

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
    if (index == g_currentExtAdv) &&
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
        g_pAdvSchInfo = LL_TIME_DELTA(T1, T2);

        if (minAuxPduTime < minPriPduTime)   // next schedule task is aux PDU
        {
            ll_ext_adv_schedule_next_event(minAuxPduTime - elapse_time - g_pAdvSchInfo);
            g_currentExtAdv = minIndexAux;
        }
        else   // next schedule task is pri PDU
        {
            ll_ext_adv_schedule_next_event(minPriPduTime - elapse_time - g_pAdvSchInfo);
            g_currentExtAdv = minIndexPri;
        }

        // update the scheduler list
        ll_updateExtAdvRemainderTime(elapse_time + g_pAdvSchInfo);
    }
}

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint8 llSetupExtAdvEvent0(void * pAdvInfo)

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
    if (llWaitingIrq)
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
            tout = 500;
        else
            tout = 3000;

        ll_hw_set_rx_timeout(tout);
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
    ownAddrType = pAdvInfo->parameter.ownAddrType;
	if ((ownAddrType == LL_DEV_ADDR_TYPE_RPA_PUBLIC) ||
		(ownAddrType == LL_DEV_ADDR_TYPE_RPA_RANDOM))
	{
		if ((adv_param.ownAddr[5] >> 6) == 1)
		{
			ownAddrType = LL_DEV_ADDR_TYPE_RANDOM;
		}
		else
			ownAddrType &= LL_DEV_ADDR_TYPE_RANDOM;
	}		
	// PDU type, 4 bits
	SET_BITS(g_tx_ext_adv_buf.txheader, ADV_EXT_TYPE, PDU_TYPE_SHIFT, PDU_TYPE_MASK);
	// RFU, ChSel, TxAdd, RxAdd
	SET_BITS(g_tx_ext_adv_buf.txheader, ownAddrType, TX_ADD_SHIFT, TX_ADD_MASK | RX_ADD_MASK);

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
    	extHeaderFlag |= LE_EXT_HDR_ADVA_PRESENT_BITMASK
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
            // the interval between chan 37<->38, 38<->39 is 5000us, primary adv -> aux adv chn is 1500us
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

    if (extHeaderFlagLE_EXT_HDR_TX_PWR_PRESENT_BITMASK)
    {
        short tx_power_10 = pAdvInfo->tx_power * 10 + g_rfTxPathCompensation;
        if (tx_power_10 < -1270)
        	tx_power_10 = -1270;
        else if (tx_power_10 > 1270)
            tx_power_10 = 1270;

        tx_power = tx_power_10 / 10;
        g_tx_ext_adv_buf.data[offset] = tx_power;
    }
    pAdvInfo -> currentAdvOffset = 0;

}


/* WARNING: Unable to use type for symbol temp_rf_fmtg_rfPhyPktFmt */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void LL_slave_conn_event1(void)

{
    uint16_t ll_rdCntIni;
    uint32_t      tx_num, rx_num;
    llConnState_t* connPtr;


    llConnState_t* connPtr;
??    g_ll_conn_ctx.timerExpiryTick = read_current_fine_time();                     // A2 multiconnection
??    g_ll_conn_ctx.scheduleInfo[0].remainder = read_current_fine_time();                     // A2 multiconnection
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
    else {
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
    
    temp_rf_fmt = g_rfPhyPktFmt;
    g_rfPhyPktFmt = connPtr->llRfPhyPktFmt;
    ll_hw_go();
    llWaitingIrq = 1;
    g_rfPhyPktFmt = temp_rf_fmt;
    HAL_EXIT_CRITICAL_SECTION();
// hal_gpio_write(GPIO_P14, 0);
//    LOG("%d-%d ", g_ll_conn_ctx.numLLConns, g_ll_conn_ctx.currentConn);
//    LOG("%d ", g_ll_conn_ctx.currentConn);
    ll_debug_output(DEBUG_LL_HW_SET_RTLP);
}

    
void LL_IRQHandler3(void)

{
    char ret;
    uint irq_status;
    int iVar1;

    ISR_entry_time = read_current_fine_time();
    ll_debug_output(DEBUG_ISR_ENTRY);
    irq_status = ll_hw_get_irq_status();
    
    if ((irq_status & LIRQ_MD) == 0) {
        ll_hw_clr_irq();
        return;
    }
    llWaitingIrq = 0;
    if (llTaskState == LL_TASK_EXTENDED_ADV) {
        ret = ll_processExtAdvIRQ();
    } else if (llTaskState == LL_TASK_EXTENDED_SCAN) {
        ret = ll_processExtScanIRQ1();
    } else if (llTaskState == LL_TASK_EXTENDED_INIT) {
        ret = ll_processExtInitIRQ1();
    } else if (llTaskState == LL_TASK_PERIODIC_ADV) {
        ret = ll_processPrdAdvIRQ(irq_status);
    } else if (llTaskState == LL_TASK_PERIODIC_SCAN) {
        ret = ll_processPrdScanIRQ();
    } else {
        ll_processBasicIRQ(irq_status);
        ret = FALSE;
    }
    if (ret != TRUE) {
        if (llSecondaryState == LL_SEC_STATE_ADV_PENDING) {
            if (llSecAdvAllow()) {
                llSetupSecAdvEvt();
                llSecondaryState = LL_SEC_STATE_ADV;
            }
        }
        else if (llSecondaryState == LL_SEC_STATE_SCAN_PENDING) {
            llSetupSecScan(scanInfo.nextScanChan);
        } else if (llSecondaryState == LL_SEC_STATE_INIT_PENDING) {
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
        pGlobal_config[EXT_ADV_AUXSCANRSP_DELAY_1MPHY] = 0xf;
        pGlobal_config[EXT_ADV_AUXCONNRSP_DELAY_1MPHY] = 0xf;
        pGlobal_config[EXT_ADV_AUXSCANRSP_DELAY_2MPHY] = 0xf;
        pGlobal_config[EXT_ADV_AUXCONNRSP_DELAY_2MPHY] = 0xf;
        pGlobal_config[EXT_ADV_AUXSCANRSP_DELAY_125KPHY] = 0x3f;
        pGlobal_config[EXT_ADV_AUXCONNRSP_DELAY_125KPHY] = 0x3f;
    }
}
