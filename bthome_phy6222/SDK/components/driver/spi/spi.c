/*******************************************************************************
    @file   spi.c
    @brief  Contains all functions support for spi driver
    @version  0.0
    @date   18. Oct. 2017
    @author qing.han

 SDK_LICENSE

*******************************************************************************/
#include "rom_sym_def.h"
#include "bus_dev.h"
#include "spi.h"
#include "gpio.h"
#include "error.h"
#include <string.h>
#include "pwrmgr.h"
#include "clock.h"
#include "log.h"
#include "jump_function.h"

#if DMAC_USE
    #include "dma.h"
#endif

typedef struct _spi_Context
{
    spi_Cfg_t   cfg;
    hal_spi_t*  spi_info;
    bool        is_slave_mode;
    spi_xmit_t  transmit;
} spi_Ctx_t;

static spi_Ctx_t m_spiCtx[2];

#define SPI_HDL_VALIDATE(hdl)   {if((hdl == NULL) || (hdl->spi_index > 1))\
            return PPlus_ERR_INVALID_PARAM;\
        if((hdl != m_spiCtx[0].spi_info) && (hdl != m_spiCtx[1].spi_info))\
            return PPlus_ERR_NOT_REGISTED;}


////////////////// SPI  /////////////////////////////////////////
static void hal_spi_write_fifo(AP_SSI_TypeDef* Ssix,uint8_t len,uint8_t* tx_rx_ptr)
{
    uint8_t i=0;
    SPI_INDEX_e spi_index = (Ssix == AP_SPI0) ? SPI0 : SPI1;
    HAL_ENTER_CRITICAL_SECTION();

    while(i<len)
    {
        if (m_spiCtx[spi_index].cfg.spi_dfsmod <= SPI_1BYTE)
        {
            Ssix->DataReg = *(tx_rx_ptr+i);
        }
        else
        {
            Ssix->DataReg = *((uint16_t*)tx_rx_ptr+i);
        }

        i++;
    }

    HAL_EXIT_CRITICAL_SECTION();
}


void spi_int_enable(hal_spi_t* spi_ptr, uint32_t mask)
{
    AP_SSI_TypeDef* Ssix = NULL;
    Ssix = (spi_ptr->spi_index == SPI0) ? AP_SPI0 : AP_SPI1;
    Ssix->IMR = mask & 0x11;

    if(Ssix == AP_SPI0)
    {
        JUMP_FUNCTION(SPI0_IRQ_HANDLER)                  =   (uint32_t)&hal_SPI0_IRQHandler;
    }
    else
    {
        JUMP_FUNCTION(SPI1_IRQ_HANDLER)                  =   (uint32_t)&hal_SPI1_IRQHandler;
    }

    NVIC_EnableIRQ((IRQn_Type)(SPI0_IRQn + spi_ptr->spi_index));
    NVIC_SetPriority((IRQn_Type)(SPI0_IRQn + spi_ptr->spi_index), IRQ_PRIO_HAL);
}

static void spi_int_disable(hal_spi_t* spi_ptr)
{
    AP_SSI_TypeDef* Ssix = NULL;
    Ssix = (spi_ptr->spi_index == SPI0) ? AP_SPI0 : AP_SPI1;
    NVIC_DisableIRQ((IRQn_Type)(SPI0_IRQn + spi_ptr->spi_index));
    Ssix->IMR = 0x00;
}

static void spi_int_handle(uint8_t id, spi_Ctx_t* pctx, AP_SSI_TypeDef* Ssix)
{
    volatile uint8_t spi_irs_status;
    spi_evt_t evt;
    uint8_t i, cnt;
    spi_xmit_t* trans_ptr;
    trans_ptr = &pctx->transmit;
    spi_irs_status = Ssix->ISR;

    if(spi_irs_status & TRANSMIT_FIFO_EMPTY)
    {
        cnt = 8 - Ssix->TXFLR;

        for(i = 0; i< cnt; i++)
        {
            if(trans_ptr->tx_buf)
            {
                Ssix->DataReg = trans_ptr->tx_buf[trans_ptr->tx_offset ++];
            }
            else
            {
                trans_ptr->tx_offset++;
                Ssix->DataReg = 0;
            }

            if(trans_ptr->tx_offset == trans_ptr->xmit_len)
            {
                Ssix->IMR = 0x10;
                m_spiCtx[pctx->spi_info->spi_index].transmit.busy = FALSE;
                break;
            }
        }
    }

    if(spi_irs_status & RECEIVE_FIFO_FULL)
    {
        cnt = Ssix->RXFTLR;

        for(i = 0; i< cnt; i++)
        {
            trans_ptr->rx_buf[trans_ptr->rx_offset++] = Ssix->DataReg;
        }

        if(trans_ptr->rx_offset == trans_ptr->xmit_len)
        {
            if(pctx->cfg.force_cs == true)
                hal_gpio_fmux(pctx->cfg.ssn_pin, Bit_ENABLE);

            trans_ptr->busy = false;
            trans_ptr->rx_buf = NULL;
            trans_ptr->rx_offset = 0;
            evt.id = id;
            evt.evt = SPI_RX_COMPLETED;
            hal_pwrmgr_unlock((MODULE_e)(MOD_SPI0 + id));
            pctx->cfg.evt_handler(&evt);
            evt.evt = SPI_TX_COMPLETED;
            pctx->cfg.evt_handler(&evt);
        }
    }
}

static void spis_int_handle(uint8_t id, spi_Ctx_t* pctx, AP_SSI_TypeDef* Ssix)
{
    volatile uint8_t spi_irs_status;
    spi_xmit_t* trans_ptr;
    spi_evt_t evt;
    uint16_t i, cnt;
    trans_ptr = &(pctx->transmit);
    spi_irs_status = Ssix->ISR;

    if(spi_irs_status & TRANSMIT_FIFO_EMPTY)
    {
        cnt = 8 - Ssix->TXFLR;

        for(i = 0; i< cnt; i++)
        {
            if(trans_ptr->tx_offset == trans_ptr->xmit_len)
            {
                Ssix->IMR = 0x10;
                break;
            }

            if(trans_ptr->tx_buf)
            {
                Ssix->DataReg = trans_ptr->tx_buf[trans_ptr->tx_offset ++];
            }
            else
            {
                trans_ptr->tx_offset ++;
                Ssix->DataReg = 0;
            }
        }
    }

    if(spi_irs_status & RECEIVE_FIFO_FULL)
    {
        volatile uint32_t garbage;
        cnt = Ssix->RXFLR;

        if(trans_ptr->rx_buf)
        {
            for(i = 0; i< cnt; i++)
            {
                if(trans_ptr->xmit_len > trans_ptr->rx_offset)
                    trans_ptr->rx_buf[trans_ptr->rx_offset++] = Ssix->DataReg;
                else
                    garbage = Ssix->DataReg;
            }
        }
        else
        {
            uint8_t rxbuf[16];

            if(trans_ptr->busy)
                trans_ptr->rx_offset += cnt;

            for(i = 0; i< cnt; i++)
            {
                *(rxbuf+i) = Ssix->DataReg;
            }

            evt.id = id;
            evt.evt = SPI_RX_DATA_S;
            evt.data = rxbuf;
            evt.len = cnt;
            pctx->cfg.evt_handler(&evt);
        }

        if(trans_ptr->busy && trans_ptr->rx_offset >= trans_ptr->xmit_len)
        {
            memset(trans_ptr, 0, sizeof(spi_xmit_t));
            evt.id = id;
            evt.evt = SPI_RX_COMPLETED;
            evt.data = NULL;
            evt.len = cnt;
            pctx->cfg.evt_handler(&evt);
            evt.evt = SPI_TX_COMPLETED;
            pctx->cfg.evt_handler(&evt);
        }
    }
}

/**************************************************************************************
    @fn          hal_SPI0_IRQHandler

    @brief       This function process for spi0 interrupt,when use int please consummate its callbackfunction

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
void __attribute__((used)) hal_SPI0_IRQHandler(void)
{
    spi_Ctx_t* pctx = &m_spiCtx[0];

    if(pctx->spi_info == NULL)
        return;

    if(pctx->is_slave_mode)
        spis_int_handle(0, pctx, AP_SPI0);
    else
        spi_int_handle(0, pctx, AP_SPI0);
}

/**************************************************************************************
    @fn          hal_SPI1_IRQHandler

    @brief       This function process for spi1 interrupt,when use int please consummate its callbackfunction

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
void __attribute__((used)) hal_SPI1_IRQHandler(void)
{
    spi_Ctx_t* pctx = &m_spiCtx[1];

    if(pctx->spi_info == NULL)
        return;

    if(pctx->is_slave_mode)
        spis_int_handle(1, pctx, AP_SPI1);
    else
        spi_int_handle(1, pctx, AP_SPI1);
}

/**************************************************************************************
    @fn          hal_spi_pin_init

    @brief       This function process for spi pin initial(4 lines);You can use two spi,spi0 and spi1,should programe by USE_AP_SPIX

    input parameters

    @param       GPIO_Pin_e sck_pin: define sclk pin
                GPIO_Pin_e ssn_pin: define ssn pin
                GPIO_Pin_e tx_pin: define transmit pin;when use as master,it's mosi pin;corresponding,use as slave,it's miso
                GPIO_Pin_e rx_pin: define receive pin;when use as master,it's miso pin;corresponding,use as slave,it's mosi

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
static void hal_spi_pin_init(hal_spi_t* spi_ptr,GPIO_Pin_e sck_pin,GPIO_Pin_e ssn_pin,GPIO_Pin_e tx_pin,GPIO_Pin_e rx_pin)
{
    if(spi_ptr->spi_index == SPI0)
    {
        hal_gpio_fmux_set(sck_pin, FMUX_SPI_0_SCK);
        hal_gpio_fmux_set(ssn_pin, FMUX_SPI_0_SSN);
        hal_gpio_fmux_set(tx_pin, FMUX_SPI_0_TX);
        hal_gpio_fmux_set(rx_pin, FMUX_SPI_0_RX);
    }
    else if(spi_ptr->spi_index == SPI1)
    {
        hal_gpio_fmux_set(sck_pin, FMUX_SPI_1_SCK);
        hal_gpio_fmux_set(ssn_pin, FMUX_SPI_1_SSN);
        hal_gpio_fmux_set(tx_pin, FMUX_SPI_1_TX);
        hal_gpio_fmux_set(rx_pin, FMUX_SPI_1_RX);
    }
}

static void hal_spi_pin_deinit(GPIO_Pin_e sck_pin,GPIO_Pin_e ssn_pin,GPIO_Pin_e tx_pin,GPIO_Pin_e rx_pin)
{
    hal_gpio_fmux(sck_pin, Bit_DISABLE);
    hal_gpio_fmux(ssn_pin, Bit_DISABLE);
    hal_gpio_fmux(tx_pin, Bit_DISABLE);
    hal_gpio_fmux(rx_pin, Bit_DISABLE);
}

/**************************************************************************************
    @fn          hal_spi_master_init

    @brief       This function process for spi master initial

    input parameters

    @param       uint32_t baud: baudrate select
                SPI_SCMOD_e scmod: Serial Clock Polarity and Phase select;  SPI_MODE0,        //SCPOL=0,SCPH=0(default)
                                                                            SPI_MODE1,        //SCPOL=0,SCPH=1
                                                                            SPI_MODE2,        //SCPOL=1,SCPH=0
                                                                            SPI_MODE3,        //SCPOL=1,SCPH=1
                SPI_TMOD_e tmod: Transfer Mode                              SPI_TRXD,        //Transmit & Receive(default)
                                                                            SPI_TXD,         //Transmit Only
                                                                            SPI_RXD,         //Receive Only
                                                                            SPI_EEPROM,      //EEPROM Read

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
static void hal_spi_master_init(hal_spi_t* spi_ptr,uint32_t baud,SPI_SCMOD_e scmod,SPI_TMOD_e tmod)
{
    uint8_t shift = 0;
    AP_SSI_TypeDef* Ssix = NULL;
    AP_COM_TypeDef* apcom = AP_COM;
    uint16_t baud_temp;
    int pclk = clk_get_pclk();

    if(spi_ptr->spi_index == SPI1)
    {
        shift = 1;
    }

    Ssix = (spi_ptr->spi_index == SPI0) ? AP_SPI0 : AP_SPI1;
    Ssix->SSIEN = 0;  //DISABLE_SPI;
    apcom->PERI_MASTER_SELECT |= (BIT(shift)|BIT(shift+4));
    Ssix->CR0= ((Ssix->CR0) & 0xfffffc3f)|(scmod<<6)|(tmod<<8);
    baud_temp = (pclk + (baud>>1)) / baud;

    if(baud_temp<2)
    {
        baud_temp = 2;
    }
    else if(baud_temp>65534)
    {
        baud_temp =65534;
    }

    Ssix->BAUDR= baud_temp;   // set clock(round)
    Ssix->TXFTLR=4;    // set fifo threshold to triggle interrupt
    Ssix->RXFTLR=1;
    Ssix->IMR = 0x00;
    Ssix->SER=1;      //enable slave device
    Ssix->SSIEN = 1;  //ENABLE_SPI;
}

/**************************************************************************************
    @fn          hal_spi_slave_init

    @brief       This function process for spi slave initial

    input parameters

    @param       uint32_t baud: baudrate select
                SPI_SCMOD_e scmod: Serial Clock Polarity and Phase select;  SPI_MODE0,        //SCPOL=0,SCPH=0(default)
                                                                            SPI_MODE1,        //SCPOL=0,SCPH=1
                                                                            SPI_MODE2,        //SCPOL=1,SCPH=0
                                                                            SPI_MODE3,        //SCPOL=1,SCPH=1
                SPI_TMOD_e tmod: Transfer Mode                              SPI_TRXD,        //Transmit & Receive(default)
                                                                            SPI_TXD,         //Transmit Only
                                                                            SPI_RXD,         //Receive Only
                                                                            SPI_EEPROM,      //EEPROM Read

    output parameters

    @param       None.

    @return      None.
 **************************************************************************************/
/*static*/ void hal_spi_slave_init(hal_spi_t* spi_ptr,uint32_t baud,SPI_SCMOD_e scmod,SPI_TMOD_e tmod)
{
    uint8_t shift = 0;
    AP_SSI_TypeDef* Ssix = NULL;
    AP_COM_TypeDef* apcom = AP_COM;
    uint16_t baud_temp;
    int pclk = clk_get_pclk();

    if(spi_ptr->spi_index == SPI1)
    {
        shift = 1;
    }

    Ssix = (spi_ptr->spi_index == SPI0) ? AP_SPI0 : AP_SPI1;
    Ssix->SSIEN = 0;  //DISABLE_SPI;
    apcom->PERI_MASTER_SELECT &= ~(BIT(shift));
    Ssix->CR0= ((Ssix->CR0) & 0xfffffc3f)|(scmod<<6)|(tmod<<8)|0x400;
    baud_temp = (pclk + (baud>>1)) / baud;

    if(baud_temp<2)
    {
        baud_temp = 2;
    }
    else if(baud_temp>65534)
    {
        baud_temp =65534;
    }

    Ssix->BAUDR= baud_temp;   // set clock(round)
    Ssix->TXFTLR=4;    // set fifo threshold to triggle interrupt
    Ssix->RXFTLR=1;    //threshold is 1
    Ssix->IMR=0x11;    //enable tx and rx
    //  Ssix->SER=1;      //enable slave device
    Ssix->SSIEN = 1;  //ENABLE_SPI;
}
#if DMAC_USE
static void config_dma_channel4spitx(hal_spi_t*  spi_ptr,uint8_t* tx_buf,uint16_t tx_len)
{
    DMA_CH_CFG_t cfgc;
    AP_SSI_TypeDef* Ssix = NULL;
    Ssix = (spi_ptr->spi_index == SPI0) ? AP_SPI0 : AP_SPI1;
    Ssix->DMACR &= 0x01;
    spi_Ctx_t* pctx;
    pctx = &m_spiCtx[spi_ptr->spi_index];
    cfgc.sinc = DMA_INC_INC;

    if(pctx->cfg.spi_dfsmod <= SPI_1BYTE)
    {
        hal_spi_dfs_set(spi_ptr,SPI_1BYTE);
        cfgc.transf_size = tx_len;
        cfgc.src_tr_width = DMA_WIDTH_BYTE;
        cfgc.dst_tr_width = DMA_WIDTH_BYTE;
        cfgc.src_addr = (uint32_t)tx_buf;
    }
    else
    {
        uint16_t* size16_tx_buf = (uint16_t*)tx_buf;
        hal_spi_dfs_set(spi_ptr,SPI_2BYTE);
        cfgc.transf_size = tx_len/2;
        cfgc.src_tr_width = DMA_WIDTH_HALFWORD;
        cfgc.dst_tr_width = DMA_WIDTH_HALFWORD;
        cfgc.src_addr = (uint32_t)size16_tx_buf;
    }

    cfgc.src_msize = DMA_BSIZE_1;
    cfgc.dinc = DMA_INC_NCHG;
    cfgc.dst_msize = DMA_BSIZE_1;
    cfgc.dst_addr = (uint32_t)&(Ssix->DataReg);
    cfgc.enable_int = false;
    hal_dma_config_channel(DMA_CH_0,&cfgc);
    hal_dma_start_channel(DMA_CH_0);
    Ssix->DMACR |= 0x02;
    Ssix->DMATDLR = 0;
}

static void config_dma_channel4spirx(hal_spi_t*  spi_ptr,uint8_t* rx_buf,uint16_t rx_len)
{
    DMA_CH_CFG_t cfgc;
    AP_SSI_TypeDef* Ssix = NULL;
    Ssix = (spi_ptr->spi_index == SPI0) ? AP_SPI0 : AP_SPI1;
    Ssix->DMACR &= 0x02;
    cfgc.transf_size = rx_len;
    cfgc.sinc = DMA_INC_NCHG;
    cfgc.src_tr_width = DMA_WIDTH_BYTE;
    cfgc.src_msize = DMA_BSIZE_1;
    cfgc.src_addr = (uint32_t)&(Ssix->DataReg);
    cfgc.dinc = DMA_INC_INC;
    cfgc.dst_tr_width = DMA_WIDTH_BYTE;
    cfgc.dst_msize = DMA_BSIZE_1;
    cfgc.dst_addr = (uint32_t)rx_buf;
    cfgc.enable_int = false;
    hal_dma_config_channel(DMA_CH_0,&cfgc);
    hal_dma_start_channel(DMA_CH_0);
    Ssix->DMACR |= 0x01;
    Ssix->DMARDLR = 0;
}
#endif

static int hal_spi_xmit_polling
(
    hal_spi_t*  spi_ptr,
    uint8_t*    tx_buf,
    uint8_t*    rx_buf,
    uint16_t    tx_len,
    uint16_t    rx_len
)
{
    uint32_t rx_size = rx_len, tx_size = tx_len;
    uint32_t tmp_len,i,tmp_tx_buf_len = 0;
    AP_SSI_TypeDef* Ssix = NULL;
    #if DMAC_USE
    spi_Ctx_t* pctx;
    pctx = &m_spiCtx[spi_ptr->spi_index];
    #endif
    Ssix = (spi_ptr->spi_index == SPI0) ? AP_SPI0 : AP_SPI1;
    SPI_INIT_TOUT(to);
    #if DMAC_USE

    if(rx_len && pctx->cfg.dma_rx_enable)
    {
        config_dma_channel4spirx(spi_ptr,rx_buf,rx_len);
    }
    else if(tx_len && pctx->cfg.dma_tx_enable)
    {
        config_dma_channel4spitx(spi_ptr,tx_buf,tx_len);
    }

    #endif

    while(1)
    {
        if(Ssix->SR & TX_FIFO_NOT_FULL && tx_size
            #if DMAC_USE
                && !(pctx->cfg.dma_tx_enable)
            #endif
          )
//#if DMAC_USE
//        if(Ssix->SR & TX_FIFO_NOT_FULL && tx_size && !(pctx->cfg.dma_tx_enable))
//#else
//        if(Ssix->SR & TX_FIFO_NOT_FULL && tx_size)
//#endif
        {
            tmp_len = 8-Ssix->TXFLR;

            if(tmp_len > tx_size)
                tmp_len = tx_size;

            if(tx_buf)
            {
                //support divider 2
                if (m_spiCtx[spi_ptr->spi_index].cfg.spi_dfsmod <= SPI_1BYTE)
                {
                    switch (tmp_len)
                    {
                    case 1:
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len);
                        tmp_tx_buf_len += 1;
                        break;

                    case 2:
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+1);
                        tmp_tx_buf_len += 2;
                        break;

                    case 3:
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+1);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+2);
                        tmp_tx_buf_len += 3;
                        break;

                    case 4:
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+1);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+2);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+3);
                        tmp_tx_buf_len += 4;
                        break;

                    case 5:
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+1);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+2);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+3);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+4);
                        tmp_tx_buf_len += 5;
                        break;

                    case 6:
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+1);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+2);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+3);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+4);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+5);
                        tmp_tx_buf_len += 6;
                        break;

                    case 7:
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+1);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+2);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+3);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+4);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+5);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+6);
                        tmp_tx_buf_len += 7;
                        break;

                    case 8:
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+1);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+2);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+3);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+4);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+5);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+6);
                        Ssix->DataReg = *(tx_buf+tmp_tx_buf_len+7);
                        tmp_tx_buf_len += 8;
                        break;

                    default:
                        break;
                    }
                }
                else
                {
                    switch (tmp_len)
                    {
                    case 1:
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len);
                        tmp_tx_buf_len += 1;
                        break;

                    case 2:
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+1);
                        tmp_tx_buf_len += 2;
                        break;

                    case 3:
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+1);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+2);
                        tmp_tx_buf_len += 3;
                        break;

                    case 4:
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+1);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+2);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+3);
                        tmp_tx_buf_len += 4;
                        break;

                    case 5:
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+1);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+2);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+3);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+4);
                        tmp_tx_buf_len += 5;
                        break;

                    case 6:
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+1);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+2);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+3);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+4);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+5);
                        tmp_tx_buf_len += 6;
                        break;

                    case 7:
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+1);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+2);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+3);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+4);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+5);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+6);
                        tmp_tx_buf_len += 7;
                        break;

                    case 8:
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+1);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+2);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+3);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+4);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+5);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+6);
                        Ssix->DataReg = *((uint16_t*)tx_buf+tmp_tx_buf_len+7);
                        tmp_tx_buf_len += 8;
                        break;

                    default:
                        break;
                    }
                }
            }
            else
            {
                for(i = 0; i< tmp_len; i++)
                {
                    Ssix->DataReg = 0;
                }
            }

            tx_size -= tmp_len;
        }

        #if DMAC_USE

        if(((rx_len == 0) && ((tx_size == 0)||(tx_size && (pctx->cfg.dma_tx_enable)))) ||
                (rx_len && (tx_size == 0) && (pctx->cfg.dma_rx_enable)))
            break;
        else if(rx_len && !(pctx->cfg.dma_rx_enable))
        #else
        if((rx_len == 0) && ((tx_size == 0)))
            break;
        else if(rx_len)
        #endif
        {
            if(Ssix->RXFLR)
            {
                tmp_len = Ssix->RXFLR;

                for(i = 0; i< tmp_len; i++)
                {
                    *rx_buf++= Ssix->DataReg;
                }

                rx_size -= tmp_len;
            }

            if(rx_size == 0)
                break;
        }

        SPI_CHECK_TOUT(to, SPI_OP_TIMEOUT, "hal_spi_xmit_polling TO\n");
    }

    #if DMAC_USE

    if((pctx->cfg.dma_rx_enable) || (pctx->cfg.dma_tx_enable))
        hal_dma_status_control(DMA_CH_0);

    #endif

    while(Ssix->SR & SPI_BUSY)
    {
        SPI_CHECK_TOUT(to, SPI_OP_TIMEOUT, "hal_spi_xmit_polling TO\n");
    }

    return PPlus_SUCCESS;
}

static void spi0_sleep_handler(void)
{
    if(m_spiCtx[0].spi_info != NULL)
        hal_spi_bus_deinit(m_spiCtx[0].spi_info);
}

static void spi1_sleep_handler(void)
{
    if(m_spiCtx[1].spi_info != NULL)
        hal_spi_bus_deinit(m_spiCtx[1].spi_info);
}

static void spi0_wakeup_handler(void)
{
    NVIC_SetPriority((IRQn_Type)SPI0_IRQn, IRQ_PRIO_HAL);
}

static void spi1_wakeup_handler(void)
{
    NVIC_SetPriority((IRQn_Type)SPI1_IRQn, IRQ_PRIO_HAL);
}

void hal_spi_tmod_set(hal_spi_t* spi_ptr,SPI_TMOD_e mod)
{
    AP_SSI_TypeDef* Ssix = NULL;
    Ssix = (spi_ptr->spi_index == SPI0) ? AP_SPI0 : AP_SPI1;
    Ssix->SSIEN = 0;
    subWriteReg(&Ssix->CR0,9,8,mod);
    Ssix->SSIEN = 1;
}

void hal_spi_dfs_set(hal_spi_t* spi_ptr,SPI_DFS_e mod)
{
    AP_SSI_TypeDef* Ssix = NULL;
    spi_Ctx_t* pctx;
    Ssix = (spi_ptr->spi_index == SPI0) ? AP_SPI0 : AP_SPI1;
    pctx = &m_spiCtx[spi_ptr->spi_index];
    Ssix->SSIEN = 0;
    subWriteReg(&Ssix->CR0,3,0,mod);
    Ssix->SSIEN = 1;
    pctx->cfg.spi_dfsmod = mod;
}



static void hal_spi_ndf_set(hal_spi_t* spi_ptr,uint16_t len)
{
    AP_SSI_TypeDef* Ssix = NULL;

    if(len == 0)
        return;

    Ssix = (spi_ptr->spi_index == SPI0) ? AP_SPI0 : AP_SPI1;
    Ssix->SSIEN = 0;
    Ssix->CR1 = len-1;
    Ssix->SSIEN = 1;
}

int hal_spi_transmit
(
    hal_spi_t*  spi_ptr,
    SPI_TMOD_e  mod,
    uint8_t*    tx_buf,
    uint8_t*    rx_buf,
    uint16_t    tx_len,
    uint16_t    rx_len
)
{
    int ret;
    spi_Ctx_t* pctx;
    AP_SSI_TypeDef* Ssix = NULL;
    spi_xmit_t* trans_ptr;
    SPI_HDL_VALIDATE(spi_ptr);
    pctx = &m_spiCtx[spi_ptr->spi_index];
    trans_ptr = &(pctx->transmit);

    if(((tx_len == 0)&&(rx_len == 0))||(mod > SPI_EEPROM)||(tx_buf == NULL))
        return PPlus_ERR_INVALID_PARAM;

    if(pctx->transmit.busy == true)
        return PPlus_ERR_BUSY;

    #if DMAC_USE

    if((pctx->cfg.dma_rx_enable && (rx_len==0)) ||
            (pctx->cfg.dma_tx_enable && (tx_len==0)))
    {
        return PPlus_ERR_INVALID_PARAM;
    }

    #endif
    Ssix = (spi_ptr->spi_index == SPI0) ? AP_SPI0 : AP_SPI1;
    hal_spi_tmod_set(spi_ptr,mod);

    if(mod > SPI_TXD)  //spi receive only or eeprom read,should set read data len(ndf)
    {
        hal_spi_ndf_set(spi_ptr,rx_len);
    }

    if(pctx->cfg.force_cs == true /*&& pctx->is_slave_mode == FALSE*/)
    {
        hal_gpio_fmux(pctx->cfg.ssn_pin,Bit_DISABLE);
        hal_gpio_write(pctx->cfg.ssn_pin,0);
    }

    if(pctx->cfg.int_mode == false)
    {
        ret = hal_spi_xmit_polling(spi_ptr,tx_buf, rx_buf, tx_len,rx_len);

        if(pctx->cfg.force_cs == true  && pctx->is_slave_mode == FALSE)
            hal_gpio_fmux(pctx->cfg.ssn_pin,Bit_ENABLE);

        if(ret)
            return PPlus_ERR_TIMEOUT;
    }
    else
    {
        spi_int_disable(spi_ptr);

        if(trans_ptr->buf_len < tx_len)
            return PPlus_ERR_NO_MEM;

        if(tx_buf)
        {
            if(!trans_ptr->tx_buf)
                return PPlus_ERR_NO_MEM;
        }

        trans_ptr->tx_offset = 0;
        memcpy(trans_ptr->tx_buf,tx_buf,tx_len);

        if(Ssix->SR & TX_FIFO_NOT_FULL)
        {
            uint16_t _tx_len;
            uint8_t dummy[8];

            if(rx_buf)
            {
                trans_ptr->rx_buf = rx_buf;
            }

            hal_pwrmgr_lock((MODULE_e)(MOD_SPI0 + spi_ptr->spi_index));
            _tx_len = (tx_len >= 8)?8:tx_len;

            if(trans_ptr->tx_buf)
            {
                trans_ptr->tx_offset += _tx_len;
                hal_spi_write_fifo(Ssix,_tx_len,tx_buf);
            }
            else
            {
                hal_spi_write_fifo(Ssix,_tx_len,dummy);
            }

            trans_ptr->xmit_len = tx_len;
        }

        pctx->transmit.busy = true;
        spi_int_enable(spi_ptr, 0x11);
    }

    return PPlus_SUCCESS;
}

int hal_spi_set_tx_buffer(hal_spi_t* spi_ptr,uint8_t* tx_buf,uint16_t len)
{
    SPI_HDL_VALIDATE(spi_ptr);

    if((tx_buf == NULL) || (len == 0))
        return PPlus_ERR_INVALID_PARAM;

    m_spiCtx[spi_ptr->spi_index].transmit.tx_buf = tx_buf;//used when tx int
    m_spiCtx[spi_ptr->spi_index].transmit.buf_len = len;
    return PPlus_SUCCESS;
}

int hal_spi_set_int_mode(hal_spi_t* spi_ptr,bool en)
{
    SPI_HDL_VALIDATE(spi_ptr);
    m_spiCtx[spi_ptr->spi_index].cfg.int_mode = en;

    if(en)
    {
        m_spiCtx[spi_ptr->spi_index].cfg.int_mode = true;
        spi_int_enable(spi_ptr, 0x10);
    }
    else
    {
        m_spiCtx[spi_ptr->spi_index].cfg.int_mode = false;
        spi_int_disable(spi_ptr);
    }

    return PPlus_SUCCESS;
}

int hal_spi_set_force_cs(hal_spi_t* spi_ptr,bool en)
{
    SPI_HDL_VALIDATE(spi_ptr);
    m_spiCtx[spi_ptr->spi_index].cfg.force_cs = en;
    return PPlus_SUCCESS;
}

bool hal_spi_get_transmit_bus_state(hal_spi_t* spi_ptr)
{
    return m_spiCtx[spi_ptr->spi_index].transmit.busy;
}


int hal_spi_TxComplete(hal_spi_t* spi_ptr)
{
    AP_SSI_TypeDef* Ssix = NULL;
    SPI_HDL_VALIDATE(spi_ptr);
    Ssix = (spi_ptr->spi_index == SPI0) ? AP_SPI0 : AP_SPI1;
    SPI_INIT_TOUT(to);

    while(Ssix->SR & SPI_BUSY)
    {
        SPI_CHECK_TOUT(to, SPI_OP_TIMEOUT, "hal_spi_TxComplete TO\n");
    }

    return PPlus_SUCCESS;
}

int hal_spi_send_byte(hal_spi_t* spi_ptr,uint8_t data)
{
    AP_SSI_TypeDef* Ssix = NULL;
    SPI_HDL_VALIDATE(spi_ptr);
    Ssix = (spi_ptr->spi_index == SPI0) ? AP_SPI0 : AP_SPI1;

    if(Ssix->SR & TX_FIFO_NOT_FULL)
    {
        Ssix->DataReg = data & 0xff;
        SPI_INIT_TOUT(to);

        while(Ssix->SR & SPI_BUSY)
        {
            SPI_CHECK_TOUT(to, SPI_OP_TIMEOUT,"hal_spi_send_byte TO\n");
        }
    }

    return PPlus_SUCCESS;
}


int hal_spi_bus_init(hal_spi_t* spi_ptr,spi_Cfg_t cfg)
{
    spi_Ctx_t* pctx = NULL;

    if((spi_ptr == NULL) || (spi_ptr->spi_index > 1))
        return PPlus_ERR_INVALID_PARAM;

    pctx = &m_spiCtx[spi_ptr->spi_index];

    if(pctx->spi_info != NULL)
        return PPlus_ERR_BUSY;

    hal_clk_gate_enable((MODULE_e)(MOD_SPI0 + spi_ptr->spi_index));
    hal_spi_pin_init(spi_ptr,cfg.sclk_pin,cfg.ssn_pin,cfg.MOSI,cfg.MISO);
    hal_spi_master_init(spi_ptr,cfg.baudrate, cfg.spi_scmod, cfg.spi_tmod);
    hal_spi_dfs_set(spi_ptr,cfg.spi_dfsmod);
    pctx->cfg = cfg;
    pctx->transmit.busy = false;
    pctx->spi_info = spi_ptr;

    if(cfg.int_mode)
        spi_int_enable(spi_ptr, 0x10);
    else
        spi_int_disable(spi_ptr);

    pctx->is_slave_mode = false;
    return PPlus_SUCCESS;
}

int hal_spis_clear_rx(hal_spi_t* spi_ptr)
{
    AP_SSI_TypeDef* Ssix = NULL;
    volatile uint8_t rx;
    SPI_HDL_VALIDATE(spi_ptr);
    Ssix = (spi_ptr->spi_index == SPI0) ? AP_SPI0 : AP_SPI1;

    while(Ssix->RXFLR)
    {
        rx = Ssix->DataReg;
    }

    return (int)rx;
}

uint32_t hal_spis_rx_len(hal_spi_t* spi_ptr)
{
    AP_SSI_TypeDef* Ssix = NULL;
    Ssix = (spi_ptr->spi_index == SPI0) ? AP_SPI0 : AP_SPI1;
    return Ssix->RXFLR;
}

int hal_spis_read_rxn(hal_spi_t* spi_ptr, uint8_t* pbuf, uint16_t len)
{
    AP_SSI_TypeDef* Ssix = NULL;
    Ssix = (spi_ptr->spi_index == SPI0) ? AP_SPI0 : AP_SPI1;

    while(len)
    {
        *pbuf = Ssix->DataReg;
        pbuf ++;
        len --;
    }

    return PPlus_SUCCESS;
}

int hal_spis_bus_init(hal_spi_t* spi_ptr,spi_Cfg_t cfg)
{
    spi_Ctx_t* pctx = NULL;

    if((spi_ptr == NULL) || (spi_ptr->spi_index > 1))
        return PPlus_ERR_INVALID_PARAM;

    pctx = &m_spiCtx[spi_ptr->spi_index];

    if(pctx->spi_info != NULL)
        return PPlus_ERR_BUSY;

    hal_clk_gate_enable((MODULE_e)(MOD_SPI0 + spi_ptr->spi_index));
    hal_spi_pin_init(spi_ptr,cfg.sclk_pin,cfg.ssn_pin,cfg.MOSI,cfg.MISO);
    hal_spi_slave_init(spi_ptr,cfg.baudrate, cfg.spi_scmod, cfg.spi_tmod);
    pctx->cfg = cfg;
    memset(&(pctx->transmit), 0, sizeof(spi_xmit_t));
    pctx->spi_info = spi_ptr;
    pctx->is_slave_mode = true;

    if(cfg.int_mode)
        spi_int_enable(spi_ptr, 0x10);
    else
        spi_int_disable(spi_ptr);

    return PPlus_SUCCESS;
}

/**************************************************************************************
    @fn          hal_spi_deinit

    @brief       This function will deinit the spi you select.

    input parameters

    @param         hal_spi_t* spi_ptr: spi module handle.


    output parameters

    @param       None.

    @return
                PPlus_SUCCESS
                PPlus_ERR_INVALID_PARAM
 **************************************************************************************/
int hal_spi_bus_deinit(hal_spi_t* spi_ptr)
{
    SPI_HDL_VALIDATE(spi_ptr);
    hal_clk_gate_disable((MODULE_e)(MOD_SPI0 + spi_ptr->spi_index));
    //spi_int_disable(spi_ptr);
    hal_spi_pin_deinit(m_spiCtx[spi_ptr->spi_index].cfg.sclk_pin,m_spiCtx[spi_ptr->spi_index].cfg.ssn_pin,m_spiCtx[spi_ptr->spi_index].cfg.MOSI,m_spiCtx[spi_ptr->spi_index].cfg.MISO);
    memset(&m_spiCtx,0,2*sizeof(spi_Ctx_t));
    return PPlus_SUCCESS;
}



/**************************************************************************************
    @fn          hal_spi_init

    @brief       it is used to init spi module.

    input parameters
    @param       None

    output parameters
    @param       None.

    @return      None.
 **************************************************************************************/
int hal_spi_init(SPI_INDEX_e channel)
{
    int ret = 0;

    if(channel == SPI0)
    {
        ret = hal_pwrmgr_register(MOD_SPI0,spi0_sleep_handler, spi0_wakeup_handler);

        if(ret == PPlus_SUCCESS)
            memset(&m_spiCtx[0],0,sizeof(spi_Ctx_t));

        return ret;
    }
    else if(channel == SPI1)
    {
        ret = hal_pwrmgr_register(MOD_SPI1,spi1_sleep_handler, spi1_wakeup_handler);

        if(ret == PPlus_SUCCESS)
            memset(&m_spiCtx[1],0,sizeof(spi_Ctx_t));

        return ret;
    }

    return PPlus_ERR_INVALID_PARAM;
}

#if DMAC_USE
int hal_spi_dma_set(hal_spi_t* spi_ptr,bool ten,bool ren)
{
    spi_Ctx_t* pctx = NULL;

    if((spi_ptr == NULL) || (spi_ptr->spi_index > 1))
        return PPlus_ERR_INVALID_PARAM;

    pctx = &m_spiCtx[spi_ptr->spi_index];
    pctx->cfg.dma_rx_enable = ren;
    pctx->cfg.dma_tx_enable = ten;
    return PPlus_SUCCESS;
}
#endif


