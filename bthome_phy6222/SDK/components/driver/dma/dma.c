/*************
 dma.c
 SDK_LICENSE
***************/
		
#include <string.h>

#include "clock.h"
#include "pwrmgr.h"
#include "error.h"
#include "log.h"
#include "jump_function.h"

#include "dma.h"
#include "spi.h"
#include "uart.h"
#include "i2c.h"

typedef struct
{
    bool init_flg;
    DMA_CH_Ctx_t dma_ch_ctx[DMA_CH_NUM];
} dma_ctx_t;


dma_ctx_t s_dma_ctx =
{
    .init_flg = FALSE,
};

static DMA_CONN_e get_src_conn(uint32_t addr)
{
    if(addr == (uint32_t)&(AP_SPI0->DataReg))
        return DMA_CONN_SPI0_Rx;

    if(addr == (uint32_t)&(AP_SPI1->DataReg))
        return DMA_CONN_SPI1_Rx;

    if(addr == (uint32_t)&(AP_I2C0->IC_DATA_CMD))
        return DMA_CONN_I2C0_Rx;

    if(addr == (uint32_t)&(AP_I2C1->IC_DATA_CMD))
        return DMA_CONN_I2C1_Rx;

    if(addr == (uint32_t)&(AP_UART0->RBR))
        return DMA_CONN_UART0_Rx;

    if(addr == (uint32_t)&(AP_UART1->RBR))
        return DMA_CONN_UART1_Rx;

    return DMA_CONN_MEM;
}

static DMA_CONN_e get_dst_conn(uint32_t addr)
{
    if(addr == (uint32_t)&(AP_SPI0->DataReg))
        return DMA_CONN_SPI0_Tx;

    if(addr == (uint32_t)&(AP_SPI1->DataReg))
        return DMA_CONN_SPI1_Tx;

    if(addr == (uint32_t)&(AP_I2C0->IC_DATA_CMD))
        return DMA_CONN_I2C0_Tx;

    if(addr == (uint32_t)&(AP_I2C1->IC_DATA_CMD))
        return DMA_CONN_I2C1_Tx;

    if(addr == (uint32_t)&(AP_UART0->THR))
        return DMA_CONN_UART0_Tx;

    if(addr == (uint32_t)&(AP_UART1->THR))
        return DMA_CONN_UART1_Tx;

    return DMA_CONN_MEM;
}

static void dma_wakeup_handler(void)
{
    hal_clk_gate_enable(MOD_DMA);
    NVIC_SetPriority((IRQn_Type)DMAC_IRQn, IRQ_PRIO_HAL);
    NVIC_EnableIRQ((IRQn_Type)DMAC_IRQn);
    JUMP_FUNCTION(DMAC_IRQ_HANDLER)      =   (uint32_t)&hal_DMA_IRQHandler;
    AP_DMA_MISC->DmaCfgReg = DMA_DMAC_E;
}


int hal_dma_init_channel(HAL_DMA_t cfg)
{
    DMA_CH_Ctx_t* pctx;
    DMA_CH_t ch;

    if(!s_dma_ctx.init_flg)
        return PPlus_ERR_NOT_REGISTED;

    ch = cfg.dma_channel;

    if(ch >= DMA_CH_NUM)
        return PPlus_ERR_INVALID_PARAM;

    pctx = &s_dma_ctx.dma_ch_ctx[ch];

    if(pctx ->init_ch)
        return PPlus_ERR_INVALID_STATE;

    pctx->evt_handler = cfg.evt_handler;
    pctx->init_ch = true;
    return PPlus_SUCCESS;
}


int hal_dma_config_channel(DMA_CH_t ch, DMA_CH_CFG_t* cfg)
{
    DMA_CH_Ctx_t* pctx;
    DMA_CONN_e src_conn,dst_conn;
    uint32_t cctrl = 0;
    uint32_t transf_type = DMA_TRANSFERTYPE_M2M;
    uint32_t transf_per = 0;
    uint32_t spif_protect = AP_SPIF->wr_protection;
    uint32_t cache_bypass = AP_PCR->CACHE_BYPASS;

    if(!s_dma_ctx.init_flg)
        return PPlus_ERR_NOT_REGISTED;

    if(ch >= DMA_CH_NUM)
    {
        return PPlus_ERR_INVALID_PARAM;
    }

    pctx = &s_dma_ctx.dma_ch_ctx[ch];

    if(!pctx->init_ch)
        return PPlus_ERR_INVALID_STATE;

    if ((AP_DMA_MISC->ChEnReg & (DMA_DMACEnbldChns_Ch(ch))) || \
            (pctx->xmit_busy))
    {
        // This channel is enabled, return ERROR, need to release this channel first
        return PPlus_ERR_BUSY;
    }

    // Reset the Interrupt status
    AP_DMA_INT->ClearTfr = DMA_DMACIntTfrClr_Ch(ch);
    // UnMask interrupt
    AP_DMA_INT->MaskTfr = DMA_DMACCxIntMask_E(ch);
    src_conn = get_src_conn(cfg->src_addr);
    dst_conn = get_dst_conn(cfg->dst_addr);

    /* Assign Linker List Item value */
    if(src_conn && dst_conn)
    {
        transf_type = DMA_TRANSFERTYPE_P2P;
        transf_per = DMA_DMACCxConfig_SrcPeripheral(src_conn-1)| \
                     DMA_DMACCxConfig_DestPeripheral(dst_conn-1);
    }
    else if(src_conn)
    {
        transf_type = DMA_TRANSFERTYPE_P2M;
        transf_per = DMA_DMACCxConfig_SrcPeripheral(src_conn-1);
    }
    else if(dst_conn)
    {
        transf_type = DMA_TRANSFERTYPE_M2P;
        transf_per = DMA_DMACCxConfig_DestPeripheral(dst_conn-1);
    }

    if((cfg->dst_addr > 0x11000000) && (cfg->dst_addr <= 0x11080000)) // 512 k
    {
        pctx->xmit_flash = DMA_DST_XIMT_IS_FLASH;

        if(spif_protect)
        {
            AP_SPIF->wr_protection = 0;
        }

        if(cache_bypass == 0)
        {
            AP_PCR->CACHE_BYPASS = 1;
        }
    }
    else
    {
        pctx->xmit_flash = DMA_DST_XIMT_NOT_FLASH;
    }

    AP_DMA_CH_CFG(ch)->SAR = cfg->src_addr;
    AP_DMA_CH_CFG(ch)->DAR = cfg->dst_addr;
    AP_DMA_CH_CFG(ch)->LLP = 0;

    if(DMA_GET_MAX_TRANSPORT_SIZE(ch) < cfg->transf_size)
    {
        return PPlus_ERR_INVALID_PARAM;
    }

    AP_DMA_CH_CFG(ch)->CTL_H = DMA_DMACCxControl_TransferSize(cfg->transf_size);
    subWriteReg(&(AP_DMA_CH_CFG(ch)->CFG_H),15,7,transf_per);
    AP_DMA_CH_CFG(ch)->CFG = 0;
    cctrl = DMA_DMACCxConfig_TransferType(transf_type)| \
            DMA_DMACCxControl_SMSize(cfg->src_msize)| \
            DMA_DMACCxControl_DMSize(cfg->dst_msize)| \
            DMA_DMACCxControl_SWidth(cfg->src_tr_width)| \
            DMA_DMACCxControl_DWidth(cfg->dst_tr_width)| \
            DMA_DMACCxControl_SInc(cfg->sinc)| \
            DMA_DMACCxControl_DInc(cfg->dinc)| \
            DMA_DMAC_INT_E;
    AP_DMA_CH_CFG(ch)->CTL = cctrl;

    if(cfg->enable_int)
    {
        AP_DMA_INT->MaskTfr = DMA_DMACCxConfig_E(ch) | BIT(ch);
        pctx->interrupt = true;
    }
    else
    {
        AP_DMA_INT->ClearTfr = DMA_DMACIntTfrClr_Ch(ch);
        AP_DMA_INT->MaskTfr = DMA_DMACCxIntMask_E(ch);
        pctx->interrupt = false;
    }

    return PPlus_SUCCESS;
}

int hal_dma_start_channel(DMA_CH_t ch)
{
    DMA_CH_Ctx_t* pctx;

    if(!s_dma_ctx.init_flg)
        return PPlus_ERR_NOT_REGISTED;

    pctx = &s_dma_ctx.dma_ch_ctx[ch];
    AP_DMA_MISC->ChEnReg = DMA_DMACCxConfig_E(ch) | BIT(ch);
    pctx->xmit_busy = TRUE;
    hal_pwrmgr_lock(MOD_DMA);
    return PPlus_SUCCESS;
}

int hal_dma_stop_channel(DMA_CH_t ch)
{
    uint32_t spif_protect = AP_SPIF->wr_protection;
    uint32_t cache_bypass = AP_PCR->CACHE_BYPASS;
    DMA_CH_Ctx_t* pctx;

    if(!s_dma_ctx.init_flg)
        return PPlus_ERR_NOT_REGISTED;

    if(ch >= DMA_CH_NUM)
    {
        return PPlus_ERR_INVALID_PARAM;
    }

    pctx = &s_dma_ctx.dma_ch_ctx[ch];

    if(pctx->xmit_flash == DMA_DST_XIMT_IS_FLASH)
    {
        if(spif_protect)
        {
            AP_SPIF->wr_protection = 2;
        }

        if(cache_bypass == 0)
        {
            AP_PCR->CACHE_BYPASS = 0;
            AP_CACHE->CTRL0 = 0x01;
        }
    }

    // Reset the Interrupt status
    AP_DMA_INT->ClearTfr = DMA_DMACIntTfrClr_Ch(ch);
    // UnMask interrupt
//    AP_DMA_INT->MaskTfr = DMA_DMACCxIntMask_E(ch);
    AP_DMA_MISC->ChEnReg = DMA_DMACCxConfig_E(ch);
    pctx->xmit_busy = FALSE;
    hal_pwrmgr_unlock(MOD_DMA);
    return PPlus_SUCCESS;
}

int hal_dma_status_control(DMA_CH_t ch)
{
    DMA_CH_Ctx_t* pctx;

    if(!s_dma_ctx.init_flg)
        return PPlus_ERR_NOT_REGISTED;

    if(ch >= DMA_CH_NUM)
    {
        return PPlus_ERR_INVALID_PARAM;
    }

    pctx = &s_dma_ctx.dma_ch_ctx[ch];

    if(pctx->interrupt == false)
        hal_dma_wait_channel_complete(ch);

    return PPlus_SUCCESS;
}

int hal_dma_wait_channel_complete(DMA_CH_t ch)
{
    uint32_t Temp = 0;

    if(!s_dma_ctx.init_flg)
        return PPlus_ERR_NOT_REGISTED;

    while(1)
    {
        Temp ++;

        if(AP_DMA_INT->RawTfr)
        {
            break;
        }
    }

    hal_dma_stop_channel(ch);
    // LOG("wait count is %d\n",Temp);
    return PPlus_SUCCESS;
}

int hal_dma_init(void)
{
    uint8_t ret;
    hal_clk_gate_enable(MOD_DMA);
    hal_clk_reset(MOD_DMA);
    NVIC_SetPriority((IRQn_Type)DMAC_IRQn, IRQ_PRIO_HAL);
    NVIC_EnableIRQ((IRQn_Type)DMAC_IRQn);
    JUMP_FUNCTION(DMAC_IRQ_HANDLER)      =   (uint32_t)&hal_DMA_IRQHandler;
    ret = hal_pwrmgr_register(MOD_DMA,NULL, dma_wakeup_handler);

    if(ret == PPlus_SUCCESS)
    {
        s_dma_ctx.init_flg = TRUE;
        memset(&(s_dma_ctx.dma_ch_ctx[0]), 0, sizeof(DMA_CH_Ctx_t)*DMA_CH_NUM);
        //dmac controller enable
        AP_DMA_MISC->DmaCfgReg = DMA_DMAC_E;
    }

    return ret;
}

int hal_dma_deinit(void)
{
    //dmac controller disable
    AP_DMA_MISC->DmaCfgReg = DMA_DMAC_D;
    s_dma_ctx.init_flg = FALSE;
    memset(&(s_dma_ctx.dma_ch_ctx[0]), 0, sizeof(DMA_CH_Ctx_t)*DMA_CH_NUM);
    hal_pwrmgr_unregister(MOD_DMA);
    hal_clk_gate_disable(MOD_DMA);
    return PPlus_SUCCESS;
}

void __attribute__((used)) hal_DMA_IRQHandler(void)
{
    DMA_CH_t ch;

    for(ch = DMA_CH_0; ch < DMA_CH_NUM; ch++)
    {
        if(AP_DMA_INT->StatusTfr & BIT(ch))
        {
            hal_dma_stop_channel(ch);

            if(s_dma_ctx.dma_ch_ctx[ch].evt_handler != NULL)
            {
                s_dma_ctx.dma_ch_ctx[ch].evt_handler(ch);
            }
        }
    }
}




