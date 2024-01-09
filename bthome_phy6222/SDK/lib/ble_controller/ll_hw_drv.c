/**********************************************************************
    @file     ll_hw_drv.c
    @brief    Contains all functions support for PHYPLUS LL HW
    @version  1.0
    @date     26. Mar. 2017
    @author   Zhongqi Yang

 SDK_LICENSE

***********************************************************************/

#include "ll_hw_drv.h"
#include "rf_phy_driver.h"

extern uint32 hclk_per_us;
uint8_t whiten_seed[40];

// ==================  A2 metal change add
extern volatile uint8_t g_same_rf_channel_flag;
extern llGlobalStatistics_t g_pmCounters;
extern uint32 llWaitingIrq;
/**************************************************************************************
    @fn          ll_hw_set_stx

    @brief       This function process for HW LL single tx mode setting.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_set_stx(void)
{
    *(volatile uint32_t*)(LL_HW_BASE+ 0x04) = 0x000100;     //[15:8]txNum=1 [23:16]rxNum=0,mode=stx
    *(volatile uint32_t*)(LL_HW_BASE+ 0x38) = 0x7;          //bypass sn nesn md
}



/**************************************************************************************
    @fn          ll_hw_set_srx

    @brief       This function process for HW LL single rx mode setting.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_set_srx(void)
{
    *(volatile uint32_t*)(LL_HW_BASE+ 0x04) = 0x010001;     //[15:8]txNum=0 [23:16]rxNum=1,mode=srx
    *(volatile uint32_t*)(LL_HW_BASE+ 0x38) = 0x7;          //bypass sn nesn md
}




/**************************************************************************************
    @fn          ll_hw_set_trx

    @brief       This function process for HW LL signgle trx mode setting.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_set_trx(void)
{
    *(volatile uint32_t*)(LL_HW_BASE+ 0x04) = 0x010102;     //[15:8]txNum=1 [23:16]rxNum=1,mode=trx
    *(volatile uint32_t*)(LL_HW_BASE+ 0x38) = 0x7;          //bypass sn nesn md
}



/**************************************************************************************
    @fn          ll_hw_set_rtx

    @brief       This function process for HW LL single rtx mode setting.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_set_rtx(void)
{
    *(volatile uint32_t*)(LL_HW_BASE+ 0x04) = 0x010103;     //[15:8]txNum=1 [23:16]rxNum=1,mode=rtx
    *(volatile uint32_t*)(LL_HW_BASE+ 0x38) = 0x7;          //bypass sn nesn md
}



/**************************************************************************************
    @fn          ll_hw_set_trlp

    @brief       This function process for HW LL TX-RX Loop mode setting.

    input parameters

    @param       snNesn      :set for the sn nesn ini value
                txPktNum    :tx packet number, will impact the md bit
                rxPktNum    :rx packet number, will trigger mode done
                mdRx        :md_rx initial value and soft value(when crc error occored)

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_set_trlp(uint8_t snNesn,uint8_t txPktNum,uint8_t rxPktNum,uint8_t mdRx)
{
    uint8_t mdIni = txPktNum>1 ? 1 : 0;
    *(volatile uint32_t*)(LL_HW_BASE+ 0x04) = 0x4 | (txPktNum<<8) | (rxPktNum<<16);
    *(volatile uint32_t*)(LL_HW_BASE+ 0x38) = 0x00;             //use MdSnNesn Ini

    if(mdRx==0)
    {
        *(volatile uint32_t*)(LL_HW_BASE+ 0x3c) = LL_HW_MD_RX_SET0 | (mdIni<<2) | (0x03 & snNesn);
    }
    else
    {
        *(volatile uint32_t*)(LL_HW_BASE+ 0x3c) = LL_HW_MD_RX_SET1 | (mdIni<<2) | (0x03 & snNesn);
    }

    *(volatile uint32_t*)(LL_HW_BASE+ 0x5c) = 0x00;             //clr rd_cnt_last and ini
}


/**************************************************************************************
    @fn          ll_hw_set_rtlp

    @brief       This function process for HW LL RX-TX Loop mode setting.

    input parameters

    @param       snNesn      :set for the sn nesn ini value
                txPktNum    :tx packet number, will impact the md bit
                rxPktNum    :rx packet number, will trigger mode done
                rdCntIni    :tx 2nd pkt's addr while rx the 1st pkt is an ACK
                mdRx        :md_rx initial value and soft value(when crc error occored)

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_set_rtlp(uint8_t snNesn,uint8_t txPktNum,uint8_t rxPktNum,uint8_t mdRx,uint32_t rdCntIni)
{
    uint8_t mdIni = txPktNum>1 ? 1 : 0;
    *(volatile uint32_t*)(LL_HW_BASE+ 0x04) = 0x5 | (txPktNum<<8) | (rxPktNum<<16);
    *(volatile uint32_t*)(LL_HW_BASE+ 0x38) = 0x00;                             //use MdSnNesn Ini

    if(mdRx==0)
    {
        *(volatile uint32_t*)(LL_HW_BASE+ 0x3c) = LL_HW_MD_RX_SET0 | (mdIni<<2) | (0x03 & snNesn);
    }
    else
    {
        *(volatile uint32_t*)(LL_HW_BASE+ 0x3c) = LL_HW_MD_RX_SET1 | (mdIni<<2) | (0x03 & snNesn);
    }

    *(volatile uint32_t*)(LL_HW_BASE+ 0x5c) = 0x7ff&rdCntIni;                   //set rd_cnt_last
}

/**************************************************************************************
    @fn          ll_hw_set_rtlp_1st

    @brief       This function process for HW LL RX-TX Loop first time mode setting.

    input parameters

    @param       snNesn      :set for the sn nesn ini value
                txPktNum    :tx packet number, will impact the md bit
                rxPktNum    :rx packet number, will trigger mode done
                rdCntIni    : fix as 0. for the 1st rtloop
                mdRx        :md_rx initial value and soft value(when crc error occored)

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_set_rtlp_1st(uint8_t snNesn,uint8_t txPktNum,uint8_t rxPktNum,uint8_t mdRx)
{
    uint8_t mdIni = txPktNum>1 ? 1 : 0;
    *(volatile uint32_t*)(LL_HW_BASE+ 0x04) = 0x5 | (txPktNum<<8) | (rxPktNum<<16);
    *(volatile uint32_t*)(LL_HW_BASE+ 0x38) = 0x00;                             //use MdSnNesn Ini

    if(mdRx==0)
    {
        *(volatile uint32_t*)(LL_HW_BASE+ 0x3c) = LL_HW_MD_RX_SET0 | (mdIni<<2) | (0x03 & snNesn);
    }
    else
    {
        *(volatile uint32_t*)(LL_HW_BASE+ 0x3c) = LL_HW_MD_RX_SET1 | (mdIni<<2) | (0x03 & snNesn);
    }

    *(volatile uint32_t*)(LL_HW_BASE+ 0x5c) = 0;                                //set rd_cnt_last
}

/**************************************************************************************
    @fn          ll_hw_config

    @brief       This function process for LL HW config setting.

    input parameters

    @param       llMode      : current ll_hw mode
                snNesn      : set for the sn nesn ini value
                txPktNum    : tx packet number, will impact the md bit
                rxPktNum    : rx packet number, will trigger mode done
                rdCntIni    : fix as 0. for the 1st rtloop
                mdRx        : md_rx initial value and soft value(when crc error occored)

    output parameters

    @param       None.

    @return      None.
*/
void     ll_hw_config(uint8_t llMode,uint8_t snNesn,uint8_t txNum,uint8_t rxNum,uint8_t mdRx,uint32_t rdCntIni)
{
    if(      llMode==LL_HW_STX )
    {
        ll_hw_set_stx();
    }
    else if(llMode==LL_HW_SRX )
    {
        ll_hw_set_srx();
    }
    else if(llMode==LL_HW_TRX )
    {
        ll_hw_set_trx();
    }
    else if(llMode==LL_HW_RTX )
    {
        ll_hw_set_rtx();
    }
    else if(llMode==LL_HW_TRLP)
    {
        ll_hw_set_trlp(     snNesn,
                            txNum,
                            rxNum,
                            mdRx);
    }
    else if(llMode==LL_HW_TRLP_EMPT)
    {
        ll_hw_set_trlp(     snNesn,
                            txNum+1,/*add one empty pkt*/
                            rxNum,
                            mdRx);
    }
    else if(llMode==LL_HW_RTLP)
    {
        ll_hw_set_rtlp(     snNesn,
                            txNum,
                            rxNum,
                            mdRx,
                            rdCntIni);
    }
    else if(llMode==LL_HW_RTLP_EMPT)
    {
        ll_hw_set_rtlp(     snNesn,
                            txNum+1,/*add one empty pkt*/
                            rxNum,
                            mdRx,
                            /*one empty pkt*/1);
    }
    else if(llMode==LL_HW_RTLP_1ST)
    {
        ll_hw_set_rtlp_1st( snNesn,
                            txNum,
                            rxNum,
                            mdRx);
    }
}

/**************************************************************************************
    @fn          ll_hw_go

    @brief       This function process for HW LL trigger.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_go0(void)
{
    //20190115 ZQ recorded ll re-trigger
    if(llWaitingIrq==TRUE)
    {
        g_pmCounters.ll_trigger_err++;
    }

    *(volatile uint32_t*)(LL_HW_BASE+ 0x14) = LL_HW_IRQ_MASK;   //clr  irq status
    *(volatile uint32_t*)(LL_HW_BASE+ 0x0c) = 0x0001;           //mask irq :only use mode done
    *(volatile uint32_t*)(LL_HW_BASE+ 0x00) = 0x0001;           //trig
    //2018-05-23 ZQ
    //fix negative rfPhyFreqOff bug, when in scan_rsq case, ll_hw_go will be  excuted before set_channel()
    //so do not change the tx_rx_foff
    //next metal change could modified the set_channel() to deal with the tx_rx_foff
    uint8_t rfChnIdx = PHY_REG_RD(0x400300b4)&0xff;

    if(!g_same_rf_channel_flag)
    {
        if(g_rfPhyFreqOffSet>=0)
            PHY_REG_WT(0x400300b4, (g_rfPhyFreqOffSet<<16)+(g_rfPhyFreqOffSet<<8)+rfChnIdx);
        else
            PHY_REG_WT(0x400300b4, ((255+g_rfPhyFreqOffSet)<<16)+((255+g_rfPhyFreqOffSet)<<8)+(rfChnIdx-1) );
    }

    //2018-02-09 ZQ
    //considering the ll_trigger timing, Trigger first, then set the tp_cal cap

    if(rfChnIdx<2)
    {
        rfChnIdx=2;
    }
    else if(rfChnIdx>80)
    {
        rfChnIdx=80;
    }

//    if(g_rfPhyPktFmt==PKT_FMT_BLE2M)
//        subWriteReg(0x40030094,7,0,g_rfPhyTpCalCapArry_2Mbps[(rfChnIdx-2)>>1]);
//    else
//        subWriteReg(0x40030094,7,0,g_rfPhyTpCalCapArry[(rfChnIdx-2)>>1]);

    if(g_rfPhyPktFmt==PKT_FMT_BLE2M)
        subWriteReg(0x40030094,7,0,RF_PHY_TPCAL_CALC(g_rfPhyTpCal0_2Mbps,g_rfPhyTpCal1_2Mbps,(rfChnIdx-2)>>1));
    else
        subWriteReg(0x40030094,7,0,RF_PHY_TPCAL_CALC(g_rfPhyTpCal0,g_rfPhyTpCal1,(rfChnIdx-2)>>1));
}
/**************************************************************************************
    @fn          ll_hw_trigger

    @brief       This function process for HW LL trigger.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_trigger0(void)
{
    *(volatile uint32_t*)(LL_HW_BASE+ 0x00) = 0x0001;           //trig
}
/**************************************************************************************
    @fn          ll_hw_clr_irq

    @brief       This function process for HW LL irq clean.

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_clr_irq(void)
{
    *(volatile uint32_t*)(LL_HW_BASE+ 0x14) = LL_HW_IRQ_MASK;
}

/**************************************************************************************
    @fn          ll_hw_set_empty_head

    @brief       This function process for HW LL set header of empty tx packet.

    input parameters

    @param       txHeader: tx empty packet header 2Byte.

    output parameters

    @param       None.

    @return      None.
*/
void     ll_hw_set_empty_head(uint16_t txHeader)
{
    *(volatile uint32_t*)(LL_HW_BASE+ 0x2c) = (txHeader & 0xffff)<<16;
}

/**************************************************************************************
    @fn          ll_hw_set_irq

    @brief       This function process for HW LL irq mask selection.

    input parameters

    @param       mask: irq mask.

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_set_irq(uint32_t mask)
{
    *(volatile uint32_t*)(LL_HW_BASE+ 0x0c)=mask;
}

/**************************************************************************************
    @fn          ll_hw_set_rx_timeout_1st

    @brief       This function process for HW LL setting 1st rx Time Out in rtloop.

    input parameters

    @param       rxTimeOut: (us).

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_set_rx_timeout_1st(uint32_t rxTimeOut)
{
    *(volatile uint32_t*)(LL_HW_BASE+ 0x24) = rxTimeOut & 0xffff;
}

/**************************************************************************************
    @fn          ll_hw_set_rx_timeout

    @brief       This function process for HW LL setting rx Time Out in rtloop.

    input parameters

    @param       rxTimeOut (us).

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_set_rx_timeout(uint32_t rxTimeOut)
{
    *(volatile uint32_t*)(LL_HW_BASE+ 0x28) = rxTimeOut & 0xffff;
}


/**************************************************************************************
    @fn          ll_hw_set_tx_rx_release

    @brief       This function process for HW LL setting tx rx en release.

    input parameters

    @param       txTime (us), rxTime(us)

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_set_tx_rx_release(uint16_t txTime,uint16_t rxTime)
{
    uint32_t tcycle=txTime * hclk_per_us;
    uint32_t rcycle=rxTime * hclk_per_us;
    *(volatile uint32_t*)(LL_HW_BASE+ 0x20) = (tcycle<<16) | rcycle;
}

/**************************************************************************************
    @fn          ll_hw_set_rx_tx_interval

    @brief       This function process for HW LL setting RX to TX interval.

    input parameters

    @param       intvTime(us)

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_set_rx_tx_interval(uint32_t intvTime)
{
    uint32_t cycle=intvTime * hclk_per_us;
    *(volatile uint32_t*)(LL_HW_BASE+ 0x1c) = cycle;
}

/**************************************************************************************
    @fn          ll_hw_set_tx_rx_interval

    @brief       This function process for HW LL setting TX to RX interval.

    input parameters

    @param       intvTime(us)

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_set_tx_rx_interval(uint32_t intvTime)
{
    uint32_t cycle= intvTime * hclk_per_us;
    *(volatile uint32_t*)(LL_HW_BASE+ 0x18) = cycle;
}

/**************************************************************************************
    @fn          ll_hw_set_trx_settle

    @brief       This function process for HW LL setting TRX settle time.

    input parameters

    @param       time(us)

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_set_trx_settle(uint8_t tmBb,uint8_t tmAfe,uint8_t tmPll)
{
    *(volatile uint32_t*)(BB_HW_BASE+ 0xbc) = (tmBb<<16) | (tmAfe<<8) | tmPll;
}

/**************************************************************************************
    @fn          ll_hw_set_loop_timeout

    @brief       This function process for HW LL setting loop time out in trlp and rtlp.

    input parameters

    @param       loopTimeOut(us)

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_set_loop_timeout(uint32_t loopTimeOut)
{
    *(volatile uint32_t*) (LL_HW_BASE + 0x60) = loopTimeOut* hclk_per_us ;
}

/**************************************************************************************
    @fn          ll_hw_set_loop_nack_num

    @brief       This function process for HW LL setting loop nAck Number in trlp and rtlp.

    input parameters

    @param       nAckNum

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_set_loop_nack_num(uint8_t nAckNum)
{
    *(volatile uint32_t*) (LL_HW_BASE + 0x64) = nAckNum ;
}

/**************************************************************************************
    @fn          ll_hw_set_timing0

    @brief       This function process for HW LL tx-rx swith timing related setting.
                In order to keep the T_IFS=150us, the setting will be different for different
                pktFmt
    input parameters

    @param       pktFmt      : 0:ZB, 1:BLE1M, 2:BLE2M, 3:BLE500K, 4:BLE125K

    output parameters

    @param       None.

    @return      None.
*/
void     ll_hw_set_timing0(uint8 pktFmt)
{
    if(      pktFmt==1)
    {
        ll_hw_set_tx_rx_release (10,     1);
        ll_hw_set_rx_tx_interval(       60);        //T_IFS=150us for BLE 1M
        ll_hw_set_tx_rx_interval(       66);        //T_IFS=150us for BLE 1M
        ll_hw_set_trx_settle    (57, 8, 52);        //TxBB,RxAFE,PLL
    }
    else if(pktFmt==2)
    {
        ll_hw_set_tx_rx_release (10,     1);
        ll_hw_set_rx_tx_interval(       73);        //T_IFS=150us for BLE 2M
        ll_hw_set_tx_rx_interval(       72);        //T_IFS=150us for BLE 2M
        ll_hw_set_trx_settle    (59, 8, 52);        //TxBB,RxAFE,PLL
    }
    else if(pktFmt==3)
    {
        ll_hw_set_tx_rx_release (10,     1);
        ll_hw_set_rx_tx_interval(       13);        //T_IFS=150us for BLE 500K
        ll_hw_set_tx_rx_interval(       74);        //T_IFS=150us for BLE 500K
        ll_hw_set_trx_settle    (57, 8, 52);        //TxBB,RxAFE,PLL
    }
    else if(pktFmt==4)
    {
        ll_hw_set_tx_rx_release (10,     1);
        ll_hw_set_rx_tx_interval(        3);        //T_IFS=150us for BLE 125K
        ll_hw_set_tx_rx_interval(       64);        //T_IFS=150us for BLE 125K
        ll_hw_set_trx_settle    (57, 8, 52);        //TxBB,RxAFE,PLL
    }
    else
    {
        ll_hw_set_tx_rx_release (10,     1);
        ll_hw_set_rx_tx_interval(       62);        //T_IFS=150us for ZB
        ll_hw_set_tx_rx_interval(       72);        //T_IFS=150us for ZB
        ll_hw_set_trx_settle    (57, 8, 52);        //TxBB,RxAFE,PLL
    }
}

/**************************************************************************************
    @fn          ll_hw_set_tfifo_space

    @brief       This function process for HW LL FIFO Space  Config


    input parameters

    @param       space : tx fifo depth. RX+TX FIFO Space = 1024 Word.

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_set_tfifo_space(uint16 space)
{
    *(volatile uint32_t*)(LL_HW_BASE+ 0x74) =((0x400 - (space&0x3ff))<<16) | (space&0x3ff) ;
}


/**************************************************************************************
    @fn          ll_hw_rst_rfifo

    @brief       This function process for HW LL rx fifo reset
                Only reset the wr/rd ptr to 0

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_rst_rfifo(void)
{
    int rdPtr, wrPtr, rdDepth;
    ll_hw_get_rfifo_info(&rdPtr, &wrPtr, &rdDepth);

    if( rdPtr > 0 && (rdPtr != wrPtr ) )
    {
        g_pmCounters.ll_rfifo_read_err++;
    }

    *(volatile uint32_t*)(LL_HW_BASE+ 0x58) = 0x0100;
    g_pmCounters.ll_rfifo_rst_cnt++;
}
/**************************************************************************************
    @fn          ll_hw_rst_tfifo

    @brief       This function process for HW LL tx fifo reset,
                Only reset the wr/rd ptr to 0

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_rst_tfifo(void)
{
    *(volatile uint32_t*)(LL_HW_BASE+ 0x58) = 0x0001;
}


/**************************************************************************************
    @fn          ll_hw_ign_rfifo

    @brief       This function process for HW LL rx fifo control,ignored pkt will not be
                writen into rfifo and the rxPkt Num will not be counted

    input parameters

    @param       ignCtrl [2]ignSSN : ignore same sn pkt
                        [1]ignCrc : ignore crc err pkt
                        [0]ignEmpt: ignore empty pkt

    output parameters

    @param       None.

    @return      None.
*/
void ll_hw_ign_rfifo(uint8_t ignCtrl)
{
    *(volatile uint32_t*)(LL_HW_BASE+ 0x68) = ignCtrl;
}

/**************************************************************************************
    @fn          ll_hw_get_tfifo_info

    @brief       This function process for HW LL getting tx fifo information

    input parameters

    @param

    output parameters

    @param       rdPtr  : read pointer.
                wrPtr  : write pointer
                wrDepth: fifo depth can be writen

    @return      None.
*/
void ll_hw_get_tfifo_info(int* rdPtr,int* wrPtr,int* wrDepth)
{
    uint32_t tmp = *(volatile uint32_t*)(LL_HW_BASE + 0x50);
    *rdPtr    =  0x07ff & tmp;
    *wrPtr    =  0x07ff & (tmp>>16);
    *wrDepth  =  (*wrPtr) - (*rdPtr) ;
}


/**************************************************************************************
    @fn          ll_hw_get_rfifo_info

    @brief       This function process for HW LL getting rx fifo information

    input parameters

    @param

    output parameters

    @param       rdPtr: read pointer.
                wrPtr: write pointer
                depth: fifo depth can be readed

    @return      None.
*/
void ll_hw_get_rfifo_info(int* rdPtr,int* wrPtr,int* rdDepth)
{
    uint32_t tmp = *(volatile uint32_t*)(LL_HW_BASE + 0x54);
    *rdPtr    =  0x07ff & tmp;
    *wrPtr    =  0x07ff & (tmp>>16);
    *rdDepth  =  (*wrPtr) - (*rdPtr) ;
}

/**************************************************************************************
    @fn          ll_hw_get_rxPkt_stats

    @brief       This function process for getting rxPkt statistic, crc ok crc error and
                total pkt crc err number in trlp or rtlp

    input parameters

    @param       None.

    output parameters

    @param       crcErrNum
                rxTotalNum
                rxPktNum.

    @return      None.
*/
void  ll_hw_get_rxPkt_stats(uint8_t* crcErrNum,uint8_t* rxTotalNum,uint8_t* rxPktNum)
{
    uint32_t tmp= *(uint32_t*) (LL_HW_BASE + 0x30) ;
    *crcErrNum  = (0xff0000 & tmp)>>16;
    *rxTotalNum = (0x00ff00 & tmp)>> 8;
    *rxPktNum   = (0x0000ff & tmp);
}

/**************************************************************************************
    @fn          ll_hw_read_rfifo

    @brief       This function process for HW LL rx fifo pop out each calling will pop one
                rx paket as output

    input parameters

    @param

    output parameters

    @param       rxPkt   : buf for poped rx pkt,only the header+pdu, w/o crc
                pktLen  : length of rxPkt=pdulen+2
                pktFoot0: foot0 of the rx pkt
                pktF00t1: foot1 of the rx pkt

    @return      wlen: return rfifo poped cnt, 0: no pkt poped.
*/
uint8_t  ll_hw_read_rfifo(uint8_t* rxPkt, uint16_t* pktLen, uint32_t* pktFoot0, uint32_t* pktFoot1)
{
    int       rdPtr, wrPtr, rdDepth, blen, wlen;
    uint32_t* p_rxPkt = (uint32_t*)rxPkt;
    ll_hw_get_rfifo_info(&rdPtr, &wrPtr, &rdDepth);

    if(rdDepth > 0)
    {
        *p_rxPkt++ = *(volatile uint32_t*)(LL_HW_RFIFO);
        uint8_t sp =BLE_HEAD_WITH_CTE(rxPkt[0]);
        blen    = rxPkt[1]+sp;                      //get the byte length for header
        wlen    = 1+ ( (blen+2+3-1) >>2 );      //+2 for Header, +3 for crc

        //compared the wlen and HW_WTR
        //20190115 ZQ
        if( (wlen+2) >rdDepth)
        {
            g_pmCounters.ll_rfifo_read_err++;
            rxPkt[0]  = 0;
            *pktFoot0 = 0;
            *pktFoot1 = 0;
            *pktLen   = 0;
            return 0;
        }

        while(p_rxPkt < (uint32_t*)rxPkt + wlen)
        {
            *p_rxPkt++ = *(volatile uint32_t*)(LL_HW_RFIFO);
        }

        *pktFoot0   = *(volatile uint32_t*)(LL_HW_RFIFO);
        *pktFoot1   = *(volatile uint32_t*)(LL_HW_RFIFO);
        *pktLen     = blen + 2;
        return wlen;
    }
    else
    {
        rxPkt[0]  = 0;
        *pktFoot0 = 0;
        *pktFoot1 = 0;
        *pktLen   = 0;
        return 0;
    }
}

/**************************************************************************************
    @fn          ll_hw_read_rfifo_zb

    @brief       This function process for HW LL rx fifo pop out Zigbee, each calling will pop one
                rx paket as output

    input parameters

    @param

    output parameters

    @param       rxPkt   : buf for poped rx pkt,only the header+pdu, w/o crc
                pktLen  : length of rxPkt=pdulen+1
                pktFoot0: foot0 of the rx pkt
                pktF00t1: foot1 of the rx pkt

    @return      wlen: return rfifo poped cnt, 0: no pkt poped.
*/
uint8_t  ll_hw_read_rfifo_zb(uint8_t* rxPkt, uint16_t* pktLen, uint32_t* pktFoot0, uint32_t* pktFoot1)
{
    int rdPtr,wrPtr,rdDepth,blen,wlen;
    uint32_t* p_rxPkt=(uint32_t*)rxPkt;
    ll_hw_get_rfifo_info(&rdPtr,&wrPtr,&rdDepth);

    if(rdDepth>0)
    {
        *p_rxPkt++ = *(volatile uint32_t*)(LL_HW_RFIFO);
        blen    = rxPkt[0];                     //get the byte length for header
        wlen    = 1+ ( (blen) >>2 );            //blen included the 2byte crc

        while(p_rxPkt<(uint32_t*)rxPkt+wlen)
        {
            *p_rxPkt++ = *(volatile uint32_t*)(LL_HW_RFIFO);
        }

        *pktFoot0   = *(volatile uint32_t*)(LL_HW_RFIFO);
        *pktFoot1   = *(volatile uint32_t*)(LL_HW_RFIFO);
        *pktLen     = blen+1;
        return wlen;
    }
    else
    {
        rxPkt[0]  = 0;
        *pktFoot0 = 0;
        *pktFoot1 = 0;
        *pktLen   = 0;
        return 0;
    }
}

/**************************************************************************************
    @fn          ll_hw_read_rfifo_pplus

    @brief       This function process for HW LL rx fifo pop out each calling will pop one
                rx paket as output

    input parameters

    @param

    output parameters

    @param       rxPkt   : buf for poped rx pkt,only the header+pdu, w/o crc
                pktLen  : length of rxPkt=pdulen+2
                pktFoot0: foot0 of the rx pkt
                pktF00t1: foot1 of the rx pkt

    @return      wlen: return rfifo poped cnt, 0: no pkt poped.
*/
uint8_t  ll_hw_read_rfifo_pplus(uint8_t* rxPkt, uint16_t* pktLen, uint32_t* pktFoot0, uint32_t* pktFoot1)
{
    int rdPtr,wrPtr,rdDepth,blen,wlen;
    uint32_t* p_rxPkt=(uint32_t*)rxPkt;
    ll_hw_get_rfifo_info(&rdPtr,&wrPtr,&rdDepth);

    if(rdDepth>0)
    {
        *p_rxPkt++ = *(volatile uint32_t*)(LL_HW_RFIFO);
        blen    = (0xff00 & (*(volatile uint32_t*)(BB_HW_BASE+0x40)))>>8;  //get the byte length from reg
        wlen    = 1+ ( (blen+2-1) >>2 );        //+2 for pplus pktfmt_len register define

        while(p_rxPkt<(uint32_t*)rxPkt+wlen)
        {
            *p_rxPkt++ = *(volatile uint32_t*)(LL_HW_RFIFO);
        }

        *pktFoot0   = *(volatile uint32_t*)(LL_HW_RFIFO);
        *pktFoot1   = *(volatile uint32_t*)(LL_HW_RFIFO);
        *pktLen     = blen+2;
        return wlen;
    }
    else
    {
        rxPkt[0]  = 0;
        *pktFoot0 = 0;
        *pktFoot1 = 0;
        *pktLen   = 0;
        return 0;
    }
}

/**************************************************************************************
    @fn          ll_hw_write_tfifo

    @brief       This function process for HW LL write tfifo once one pkt, heder+pdu

    input parameters

    @param       *txPkt : tx packet pointer
                 pktLen: packet length--> header+pdu, can not be 0

    output parameters

    @param       None.

    @return      wlen: tfifo push cnt, 0: no pkt write.
*/
uint8_t  ll_hw_write_tfifo(uint8_t* txPkt, uint16_t pktLen)
{
    int rdPtr,wrPtr,wrDepth,wlen;
    uint32_t* p_txPkt=(uint32_t*)txPkt;
    ll_hw_get_tfifo_info(&rdPtr,&wrPtr,&wrDepth);
    uint16_t tfifoSpace  = 0x07ff & (*(uint32_t*) (LL_HW_BASE + 0x74));

    if((pktLen>0) && (wrDepth+(pktLen>>2)<tfifoSpace-LL_HW_FIFO_MARGIN))    // make sure write the longest pkt will not overflow
    {
        // make sure pktLen > 0
        wlen = 1+((pktLen-1)>>2);                       // calc the write tfifo count

        //--------------------------------------------------------------
        //write tfifo wlen-1 firstly
        while(p_txPkt<((uint32_t*)txPkt+wlen-1))
        {
            *(volatile uint32_t*)(LL_HW_TFIFO) = *p_txPkt++;
        }

        //--------------------------------------------------------------
        //calc the residue txPkt length
        //write tfifo last time
        int rduLen = pktLen&0x03;//pktLen%4

        if(         rduLen==3)
        {
            *(volatile uint32_t*)(LL_HW_TFIFO) = *(uint16_t*)(txPkt+pktLen-3) | (txPkt[pktLen-1]<<16) ;
        }
        else if(   rduLen==2)
        {
            *(volatile uint32_t*)(LL_HW_TFIFO) = *(uint16_t*)(txPkt+pktLen-2);
        }
        else if(   rduLen==1)
        {
            *(volatile uint32_t*)(LL_HW_TFIFO) = *(txPkt+pktLen-1);
        }
        else
        {
            *(volatile uint32_t*)(LL_HW_TFIFO) = *p_txPkt;
        }

        return wlen;
    }
    else
    {
        return 0;
    }
}
/**************************************************************************************
    @fn          ll_hw_set_crc_fmt

    @brief       This function set crc format

    input parameters

    @param       txCrc,rxCrc.

    output parameters

    @param       None.

    @return      txAckNum.
*/
void     ll_hw_set_crc_fmt(uint8_t txCrc,uint8_t rxCrc)
{
    *(volatile uint32_t*) (BB_HW_BASE+0x124) =(txCrc<<8) | rxCrc;
}

/**************************************************************************************
    @fn          ll_hw_set_pplus_pktfmt

    @brief       This function set pplus pktfmt and pkt len

    input parameters

    @param       pktlen.

    output parameters

    @param       None.

    @return      txAckNum.
*/
void     ll_hw_set_pplus_pktfmt(uint8_t plen)
{
    uint32_t tmp  = 0xffff00e0 & (*(uint32_t*) (BB_HW_BASE + 0x40));
    *(volatile uint32_t*)(BB_HW_BASE+ 0x40) =tmp| ((plen-2)<<8) | 0x1f;
    ll_hw_set_crc_fmt(LL_HW_CRC_NULL,LL_HW_CRC_NULL);
}

/**************************************************************************************
    @fn          ll_hw_set_ant_switch_mode

    @brief       This function set ant switch mode for AOA/AOD

    input parameters

    @param       mode: 0 : ant swith manual mode,
                      1 : ant switch auto mode(ctrl by PDU Header)

    output parameters

    @param       None.

    @return      None.
*/
void     ll_hw_set_ant_switch_mode(uint8_t mode)
{
    uint32_t tmp  = 0x0000ffff & (*(uint32_t*) (BB_HW_BASE + 0x110));
    *(volatile uint32_t*)(BB_HW_BASE+ 0x110) = tmp |(mode<<16);
}

/**************************************************************************************
    @fn          ll_hw_set_ant_switch_timing

    @brief       This function set ant switch for AOA/AOD

    input parameters

    @param       antWin: window for each ant arrary
    @param       antDly: dly for rx ant swith only

    output parameters

    @param       None.

    @return      None.
*/
void     ll_hw_set_ant_switch_timing(uint8_t antWin,uint8_t antDly)
{
    uint32_t tmp  = 0xffff0000 & (*(uint32_t*) (BB_HW_BASE + 0x110));
    *(volatile uint32_t*)(BB_HW_BASE+ 0x110) = tmp |( antWin<<8) | antDly;
}

/**************************************************************************************
    @fn          ll_hw_set_ant_pattern

    @brief       This function set ant pattern for AOA/AOD

    input parameters

    @param       an0: ant pattern 0 -7.
    @param       ant1: ant pattern 8-15

    output parameters

    @param       None.

    @return      None.
*/
void     ll_hw_set_ant_pattern(uint32_t ant1, uint32_t ant0)
{
    *(volatile uint32_t*)(BB_HW_BASE+ 0x114) =ant0;
    *(volatile uint32_t*)(BB_HW_BASE+ 0x118) =ant1;
}
/**************************************************************************************
    @fn          ll_hw_set_cte_rxSupp

    @brief       This function set cte rx supplement config

    input parameters

    @param       rxSupp.[7] check cp, [6] suppLen=CTEInfo, [5:0] suppLen

    output parameters

    @param       None.

    @return      None.
*/
void     ll_hw_set_cte_rxSupp(uint8_t rxSupp)
{
    uint32_t tmp  = 0xffff00ff & (*(uint32_t*) (BB_HW_BASE + 0x0c));
    *(volatile uint32_t*)(BB_HW_BASE+ 0x0c) = tmp |( rxSupp<<8);
}
/**************************************************************************************
    @fn          ll_hw_set_cte_txSupp

    @brief       This function set cte tx supplement config

    input parameters

    @param       txSupp..[7] check cp, [6] suppLen=CTEInfo, [5:0] suppLen

    output parameters

    @param       None.

    @return      None.
*/
void     ll_hw_set_cte_txSupp(uint8_t txSupp)
{
    uint32_t tmp  = 0x00ffffff & (*(uint32_t*) (BB_HW_BASE + 0x40));
    *(volatile uint32_t*)(BB_HW_BASE+ 0x40) = tmp |( txSupp<<24);
}
/**************************************************************************************
    @fn          ll_hw_get_iq_RawSample

    @brief       This function get rx iq sample for AOA/AOD

    input parameters

    @param       None

    output parameters

    @param       p_i_sample: ptr of i sample [9:0] 2's complement
    @param       p_q_sample: ptr of q sample [9:0] 2's complement

    @return      iq_cnt:  iq sample count
*/
uint8_t    ll_hw_get_iq_RawSample(uint16_t* p_iSample, uint16_t* p_qSample)
{
    uint8_t iqCnt = 0xff &( (*(uint32_t*) (BB_HW_BASE + 0xFC))>>20);
    uint32_t tmp=0;
    uint8_t i= 0;

    for (i=0; i<iqCnt; i++)
    {
        tmp =  *(uint32_t*) (LL_HW_BASE + 0x200);
        *(p_iSample+i)  = (uint16_t)(  tmp      & 0x3ff);
        *(p_qSample+i)  = (uint16_t)((tmp>>10) & 0x3ff);
    }

    return iqCnt;
}
/**************************************************************************************
    @fn          ll_hw_get_snNesn

    @brief       This function process for getting sn nesn after trlp or rtlp mode done

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      local_sn | local_nesn.
*/
uint8_t  ll_hw_get_snNesn(void)
{
    return  ( (uint8_t) (0x03 & (*(uint32_t*) (LL_HW_BASE + 0x44) ) ) ) ;
}

/**************************************************************************************
    @fn          ll_hw_get_txAck

    @brief       This function process for getting txPkt Ack number after trlp or rtlp

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      txAckNum.
*/
uint8_t  ll_hw_get_txAck(void)
{
    return  ( (uint8_t)(0xff & (*(uint32_t*) (LL_HW_BASE + 0x34) ) ) );
}

/**************************************************************************************
    @fn          ll_hw_get_nAck

    @brief       This function process for getting NACK number after trlp or rtlp

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      txAckNum.
*/
uint8_t  ll_hw_get_nAck(void)
{
    return  ( (uint8_t)(0x0f & ( (*(uint32_t*) (LL_HW_BASE + 0x34) )>>8) ) );
}

/**************************************************************************************
    @fn          ll_hw_get_rxPkt_num

    @brief       This function process for getting rxPkt number after trlp or rtlp

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      rxPktNum.
*/
uint8_t  ll_hw_get_rxPkt_num(void)
{
    return  ( (uint8_t)(0xff & (*(uint32_t*) (LL_HW_BASE + 0x30) ) ) );
}

/**************************************************************************************
    @fn          ll_hw_get_rxPkt_Total_num

    @brief       This function process for getting rxPkt Total number after trlp or rtlp

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      rxPktTotalNum.
*/
uint8_t  ll_hw_get_rxPkt_Total_num(void)
{
    return  ( (uint8_t)(0xff & ( (*(uint32_t*) (LL_HW_BASE + 0x30) )>>8) ) );
}

/**************************************************************************************
    @fn          ll_hw_get_rxPkt_CrcErr_num

    @brief       This function process for getting rxPkt CrcErr  number after trlp or rtlp

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      rxPktTotalNum.
*/
uint8_t  ll_hw_get_rxPkt_CrcErr_num(void)
{
    return  ( (uint8_t)(0xff & ( (*(uint32_t*) (LL_HW_BASE + 0x30) )>>16) ) );
}


/**************************************************************************************
    @fn          ll_hw_get_rxPkt_CrcOk_num

    @brief       This function process for getting rxPkt CrcOk  number after trlp or rtlp

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      rxPktTotalNum.
*/
uint8_t  ll_hw_get_rxPkt_CrcOk_num(void)
{
    uint32_t tmp= *(uint32_t*) (LL_HW_BASE + 0x30) ;
    return ( ( (0x00ff00 & tmp)>> 8) - ((0xff0000 & tmp)>>16) ) ;
}


/**************************************************************************************
    @fn          ll_hw_get_anchor

    @brief       This function process for getting anchor point count after rtlp

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      anchorPoint[31:0],hclk cycle between ll_trig and bb_sync
*/
uint32_t  ll_hw_get_anchor(void)
{
    return (*(uint32_t*) (LL_HW_BASE + 0x6c));
}

/**************************************************************************************
    @fn          ll_hw_get_irq_status

    @brief       This function process for getting HW LL irq status

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      irq_status.
*/
uint32_t ll_hw_get_irq_status(void)
{
    return (*(volatile uint32_t*) (LL_HW_BASE+ 0x10) & LL_HW_IRQ_MASK);
}

/**************************************************************************************
    @fn          ll_hw_get_fsm_status

    @brief       This function process for getting HW LL FSM status

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      fsm status.
*/
uint8_t  ll_hw_get_fsm_status(void)
{
    return ( (uint8_t)(*(volatile uint32_t*) (LL_HW_BASE + 0x08) & 0x0f ) );
}


/**************************************************************************************
    @fn          ll_hw_get_last_ack

    @brief       This function process for getting HW LL last pkt ack status

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      lastAck= 1: ack, 0:nack.
*/
uint8_t  ll_hw_get_last_ack(void)
{
    uint16_t snNesnStatus;
    uint8_t nesnRx,snLocal,lastAck;
    snNesnStatus= (uint16_t)(*(volatile uint32_t*) (LL_HW_BASE + 0x44) & 0xffff ) ;
    nesnRx      = 0x01 &(snNesnStatus>>8);
    snLocal     = 0x01 &(snNesnStatus>>1);
    lastAck     = (nesnRx==snLocal);
    return lastAck;
}

/**************************************************************************************
    @fn          ll_hw_get_loop_cycle

    @brief       This function process for getting HW LL loop cycle from ll_trigger

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      fsm status.
*/
uint32_t  ll_hw_get_loop_cycle(void)
{
    return (*(volatile uint32_t*) (LL_HW_BASE + 0x70) );
}


/**************************************************************************************
    @fn          ll_hw_update_rtlp_mode

    @brief       This function process for rtlp update next connect event llMode
                This fucnction MUST BE called while ll mode_done irq happened!!!

    input parameters

    @param

    output parameters

    @param       None.

    @return      rtlpMode
*/
uint8_t  ll_hw_update_rtlp_mode(uint8_t llMode)
{
    uint8_t     txAckNum;
    uint8_t     llTxNum;
    uint16_t    rtlpMode;
    uint8_t     rtlpNoTx;
    llTxNum         =   ((*(volatile uint32_t*)(LL_HW_BASE+ 0x04))>>8) & 0x00ff; //txNum include fist empty pkt
    txAckNum        =   ll_hw_get_txAck();                                       //txAck include fist empty pkt
    rtlpNoTx        =   (LIRQ_TD & ll_hw_get_irq_status())==0;                  //mode_done with tx_done

    //---------------------------------------------------------------------------------
    //when last tx pkt is empty pkt, change the next mode as RTLP_1ST
    //OR RX TIME OUT occoured when recevie the 1st pkt(never tx any pkt)
    //
    if((llTxNum==0) ||( (txAckNum>0) && (txAckNum==llTxNum)         /*tx tail empty pkt*/)
            ||( (llMode==LL_HW_RTLP_EMPT) && (txAckNum==0) )/*tx first empty pkt*/ )
    {
        rtlpMode    =   LL_HW_RTLP_EMPT;
    }
    else
    {
        rtlpMode    =   LL_HW_RTLP;
    }

    rtlpMode    =   (rtlpNoTx==1) ? llMode : rtlpMode;
    return rtlpMode;
}

/**************************************************************************************
    @fn          ll_hw_update_trlp_mode

    @brief       This function process for trlp update next connect event llMode
                This fucnction MUST BE called while ll mode_done irq happened!!!

    input parameters

    @param

    output parameters

    @param       None.

    @return      trlpMode
*/
uint8_t  ll_hw_update_trlp_mode(uint8_t llMode)
{
    uint8_t     txAckNum;
    uint8_t     llTxNum;
    uint8_t     trlpMode;
    uint16_t    lastAck;
    uint8_t     rxTimeOut;
    llTxNum         =   ((*(volatile uint32_t*)(LL_HW_BASE+ 0x04))>>8) & 0x00ff; //txNum include fist empty pkt
    txAckNum        =   ll_hw_get_txAck();                                       //txAck include fist empty pkt
    lastAck         =   ll_hw_get_last_ack();
    rxTimeOut       =   (LIRQ_RTO & ll_hw_get_irq_status())>>2;                  //mode_done with rx_time_out

    //---------------------------------------------------------------------------------
    //when last tx pkt is empty pkt, change the next mode
    //depends on the last rx pkt ack
    if((llTxNum==0) ||( (txAckNum>0) && (txAckNum==llTxNum) )       /*tx tail empty pkt*/
            ||( (llMode==LL_HW_TRLP_EMPT) && (txAckNum==0) )/*tx first empty pkt*/      )
    {
        if(rxTimeOut==1)
        {
            trlpMode    =   LL_HW_TRLP_EMPT;
        }
        else
        {
            trlpMode    =   (lastAck==1) ? LL_HW_TRLP : LL_HW_TRLP_EMPT;
        }
    }
    else
    {
        trlpMode    =   LL_HW_TRLP;
    }

    return trlpMode;
}


/**************************************************************************************
    @fn          ll_hw_update

    @brief       This function process for update next connect event llMode
                This fucnction MUST BE called while ll mode_done irq happened!!!

    input parameters

    @param       lMode : current ll_hw_mode
                txAck : ptr of tx Ack number, when ****_EMPT, txAckNum will excluded the first EMPTY pkt
                rxRec : ptr of rx pkt number
                snNesn: ptr of snNesn local

    output parameters

    @param       None.

    @return      llMode
*/
uint8_t  ll_hw_update(uint8_t llMode,uint8_t* txAck,uint8_t* rxRec,uint8_t* snNesn)
{
    uint8_t txAckNum,rxPktNum,snNesnLocal;
    uint8_t nextLlMode;
    txAckNum    =   ll_hw_get_txAck();
    rxPktNum    =   ll_hw_get_rxPkt_num();
    snNesnLocal =   ll_hw_get_snNesn();

    //----------------------------------------------------------------------------------------
    //remove the txAck of first tx empty pkt
    if((llMode == LL_HW_TRLP_EMPT) || (llMode == LL_HW_RTLP_EMPT))
    {
        txAckNum = (txAckNum==0) ? 0 : txAckNum-1;
    }

    //----------------------------------------------------------------------------------------
    //when RTLP last trans empty packet, the txAck number will equal tx_num
    //for next turn, we should change to RTLP_EMPT and write back the empty pkt as the fisrt
    //pkt in tfifo.
    //when TRLP last trans empty packet, we should change the ll_mode
    //depends on the last rx pkt status
    if(         (llMode==LL_HW_RTLP_EMPT)  || (  llMode==LL_HW_RTLP)
                ||  (  llMode==LL_HW_RTLP_1ST))
    {
        nextLlMode = ll_hw_update_rtlp_mode(llMode);
    }
    else if(   (llMode==LL_HW_TRLP_EMPT) || (  llMode==LL_HW_TRLP))
    {
        nextLlMode = ll_hw_update_trlp_mode(llMode);
    }
    else
        nextLlMode = llMode;      // should not be here, set default to supress warning

    *txAck = txAckNum;
    *rxRec = rxPktNum;
    *snNesn= snNesnLocal;
    return nextLlMode;
}

/**************************************************************************************
    @fn          byte_to_bit

    @brief       This function process for byte to bin converter

    input parameters

    @param       byte.

    output parameters

    @param       bitOut ptr 8bit

    @return      None.
*/
void  byte_to_bit(uint8_t byteIn,uint8_t* bitOut)
{
    int i=0;

    for(i=0; i<8; i++)
    {
        *(bitOut+i) = (byteIn>>i) & 0x01;
    }
}

/**************************************************************************************
    @fn          bit_to_byte

    @brief       This function process for byte to bin converter

    input parameters

    @param       bit input ptr.

    output parameters

    @param       bitOut ptr

    @return      None.
*/
void  bit_to_byte(uint8_t* bitIn,uint8_t* byteOut)
{
    int i=0;
    uint8_t tmp=0;

    for(i=0; i<8; i++)
    {
        tmp = tmp | (bitIn[i]<<i);
    }

    *byteOut = tmp;
}

/**************************************************************************************
    @fn          zigbee_crc16_gen

    @brief       This function process for getting zigbee crc

    input parameters

    @param       None.

    output parameters

    @param       crcCode 2Byte.

    @return      None.
*/
void  zigbee_crc16_gen(uint8_t* dataIn,int length,uint8_t* seed,uint8_t* crcCode)
{
    uint8_t bitOut[8]= {0};
    uint8_t i=0;
    uint8_t j=0;
    uint8_t feedback = 0;
    uint8_t reg[16]= {0};

    for(i=0; i<16; i++)
    {
        reg[i] = seed[i];
    }

    for(i=0; i<length; i++)
    {
        byte_to_bit(dataIn[i],bitOut);

        for(j=0; j<8; j++)
        {
            feedback = reg[0]^bitOut[j];
            reg[0]   = reg[1];
            reg[1]   = reg[2];
            reg[2]   = reg[3];
            reg[3]   = reg[4]^feedback;
            reg[4]   = reg[5];
            reg[5]   = reg[6];
            reg[6]   = reg[7];
            reg[7]   = reg[8];
            reg[8]   = reg[9];
            reg[9]   = reg[10];
            reg[10]  = reg[11]^feedback;
            reg[11]  = reg[12];
            reg[12]  = reg[13];
            reg[13]  = reg[14];
            reg[14]  = reg[15];
            reg[15]  = feedback;
        }
    }

    bit_to_byte(reg,crcCode);
    bit_to_byte(reg+8,crcCode+1);
}


void  ble_crc24_gen(uint8_t* dataIn,int length,uint32_t seed,uint8_t* crcCode)
{
    uint8_t bitOut[8]= {0};
    uint16_t i=0;
    uint16_t j=0;
    uint8_t feedback = 0;
    uint8_t reg[24]= {0};

    for(i=0; i<24; i++)
    {
        reg[i] = (seed>>i)&0x01;
    }

    for(i=0; i<length; i++)
    {
        byte_to_bit(dataIn[i],bitOut);

        for(j=0; j<8; j++)
        {
            feedback = reg[23] ^ bitOut[j];
            reg[23] = reg[22];
            reg[22] = reg[21];
            reg[21] = reg[20];
            reg[20] = reg[19];
            reg[19] = reg[18];
            reg[18] = reg[17];
            reg[17] = reg[16];
            reg[16] = reg[15];
            reg[15] = reg[14];
            reg[14] = reg[13];
            reg[13] = reg[12];
            reg[12] = reg[11];
            reg[11] = reg[10];
            reg[10] = reg[9] ^ feedback;
            reg[9]  = reg[8] ^ feedback;
            reg[8]  = reg[7];
            reg[7]  = reg[6];
            reg[6]  = reg[5] ^ feedback;
            reg[5]  = reg[4] ;
            reg[4]  = reg[3] ^ feedback;
            reg[3]  = reg[2] ^ feedback;
            reg[2]  = reg[1];
            reg[1]  = reg[0] ^ feedback;
            reg[0]  = feedback;
        }
    }

    bit_to_byte(reg,crcCode);
    bit_to_byte(reg+8,crcCode+1);
    bit_to_byte(reg+16,crcCode+2);
}


// ========================================== copy from rf.c, write by Zeng jiaping
void set_channel(uint32_t  channel)
{
    uint32_t temp;
    uint32_t f = 0;
    temp= *(volatile uint32_t*)(BB_HW_BASE + 0xb4) & 0xffffff00;

    if(channel==37) f=0;
    else if(channel==38)  f=12;
    else if(channel==39)  f=39;
    else if(channel<=10)  f=channel +1;
    else if(channel<=36)  f=channel +2;

    temp=temp | ((f*2+2) & 0xff);
    *(volatile int*)(BB_HW_BASE + 0xb4) = temp;
}

void set_access_address( uint32_t  access)
{
    *(volatile uint32_t*)(BB_HW_BASE + 0x4c) = access;
}

void set_crc_seed(uint32_t  seed)
{
    uint32_t temp;
    temp= *(volatile uint32_t*)(BB_HW_BASE + 0x48) & 0xff000000;
    temp = temp | (seed & 0xffffff);
    *(volatile int*)(BB_HW_BASE + 0x48) = temp;
}

void calculate_whiten_seed(void)
{
    int      channel;
    uint32_t  seed;
    channel = 0;

    while(channel<40)
    {
        seed =0;
        seed = (channel & 0x1)*64;
        seed = seed | (channel & 0x2)*16;
        seed = seed | (channel & 0x4)*4;
        seed = seed | (channel & 0x8);
        seed = seed | (channel & 0x10)/4;
        seed = seed | (channel & 0x20)/16;
        seed = seed | (channel & 0x40)/64;
        seed = seed | 0x1;
        whiten_seed [channel++] = seed;
    }
}

void set_whiten_seed(uint32_t channel)
{
    uint32_t  temp;
    temp= (*(volatile uint32_t*)(BB_HW_BASE + 0x48)) & 0x00ffffff;
    temp=  whiten_seed[channel] <<24 | temp;
    *(volatile int*)(BB_HW_BASE + 0x48)  =  temp;
}



void set_max_length(uint32_t length)
{
    uint32_t  temp;
    temp = (*(volatile uint32_t*)(BB_HW_BASE + 0xc)) & 0xffffff00;
    temp |= length & 0xff;
    *(volatile uint32_t*)(BB_HW_BASE + 0xc) = temp;
}



