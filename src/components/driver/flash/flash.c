/**************************************************************************************************
*******
**************************************************************************************************/

/*******************************************************************************
    @file     flash.c
    @brief    Contains all functions support for flash driver
    @version  0.0
    @date     27. Nov. 2017
    @author   qing.han



*******************************************************************************/
#include "rom_sym_def.h"
#include <string.h>
#include "types.h"
#include "flash.h"
#include "log.h"
#include "pwrmgr.h"
#include "error.h"

#define SPIF_WAIT_IDLE_CYC                          (32)

#define SPIF_STATUS_WAIT_IDLE(n)                    \
    do                                              \
    {                                               \
        while((AP_SPIF->fcmd &0x02)==0x02);         \
        {                                           \
            volatile int delay_cycle = n;           \
            while (delay_cycle--){;}                \
        }                                           \
        while ((AP_SPIF->config & 0x80000000) == 0);\
    } while (0);


#define HAL_CACHE_ENTER_BYPASS_SECTION()  do{ \
        HAL_ENTER_CRITICAL_SECTION();\
        AP_CACHE->CTRL0 = 0x02; \
        AP_PCR->CACHE_RST = 0x02;\
        AP_PCR->CACHE_BYPASS = 1;    \
        HAL_EXIT_CRITICAL_SECTION();\
    }while(0);


#define HAL_CACHE_EXIT_BYPASS_SECTION()  do{ \
        HAL_ENTER_CRITICAL_SECTION();\
        AP_CACHE->CTRL0 = 0x00;\
        AP_PCR->CACHE_RST = 0x03;\
        AP_PCR->CACHE_BYPASS = 0;\
        HAL_EXIT_CRITICAL_SECTION();\
    }while(0);

#define spif_wait_nobusy(flg, tout_ns, return_val)   {if(_spif_wait_nobusy_x(flg, tout_ns)){if(return_val){ return return_val;}}}

static xflash_Ctx_t s_xflashCtx = {.spif_ref_clk=SYS_CLK_DLL_64M,.rd_instr=XFRD_FCMD_READ_DUAL};

chipMAddr_t  g_chipMAddr;

__ATTR_SECTION_SRAM__  static inline uint32_t spif_lock()
{
    HAL_ENTER_CRITICAL_SECTION();
    uint32_t vic_iser = NVIC->ISER[0];
    //mask all irq
    NVIC->ICER[0] = 0xFFFFFFFF;
    //enable ll irq and tim1 irq
    NVIC->ISER[0] = 0x100010;
    HAL_EXIT_CRITICAL_SECTION();
    return vic_iser;
}

__ATTR_SECTION_SRAM__  static inline void spif_unlock(uint32_t vic_iser)
{
    HAL_ENTER_CRITICAL_SECTION();
    NVIC->ISER[0] = vic_iser;
    HAL_EXIT_CRITICAL_SECTION();
}

static void hal_cache_tag_flush(void)
{
    HAL_ENTER_CRITICAL_SECTION();
    uint32_t cb = AP_PCR->CACHE_BYPASS;
    volatile int dly = 8;

    if(cb==0)
    {
        AP_PCR->CACHE_BYPASS = 1;
    }

    AP_CACHE->CTRL0 = 0x02;

    while (dly--) {;};

    AP_CACHE->CTRL0 = 0x03;

    dly = 8;

    while (dly--) {;};

    AP_CACHE->CTRL0 = 0x00;

    if(cb==0)
    {
        AP_PCR->CACHE_BYPASS = 0;
    }

    HAL_EXIT_CRITICAL_SECTION();
}


static uint8_t _spif_read_status_reg_x(void)
{
    uint8_t status;
    spif_cmd(FCMD_RDST, 0, 2, 0, 0, 0);
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_rddata(&status, 1);
    return status;
}

static int _spif_wait_nobusy_x(uint8_t flg, uint32_t tout_ns)
{
    uint8_t status;
    volatile int tout = (int )(tout_ns);

    for(; tout ; tout --)
    {
        status = _spif_read_status_reg_x();

        if((status & flg) == 0)
            return PPlus_SUCCESS;

        //insert polling interval
        //5*32us
        WaitRTCCount(5);
    }

    return PPlus_ERR_BUSY;
}

static void hal_cache_init(void)
{
    volatile int dly=100;
    //clock gate
    hal_clk_gate_enable(MOD_HCLK_CACHE);
    hal_clk_gate_enable(MOD_PCLK_CACHE);
    //cache rst ahp
    AP_PCR->CACHE_RST=0x02;

    while(dly--) {};

    AP_PCR->CACHE_RST=0x03;

    hal_cache_tag_flush();

    //cache enable
    AP_PCR->CACHE_BYPASS = 0;
}

FLASH_CHIP_INFO phy_flash =
{
    .init_flag = FALSE,
    .IdentificationID = 0x00,
    .Capacity = 0x80000,
};


int hal_get_flash_info(void)
{
    uint32_t cs;
    uint8_t data[17];

    if(phy_flash.init_flag == TRUE)
    {
        return PPlus_SUCCESS;
    }

    cs = spif_lock();
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    spif_cmd(FCMD_RDID, 0, 3, 0, 0, 0);
    spif_rddata(data, 3);
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    spif_unlock(cs);
    phy_flash.IdentificationID = (data[2]<<16) | (data[1]<<8) | data[0];

    if((data[2] >= 0x11) && (data[2] <= 0x16))//most use:256K~2M.reserved:128K,4M
    {
        phy_flash.Capacity = (1ul << data[2]);
        *(volatile int*)0x1fff0898 = phy_flash.Capacity;
    }
    else
    {
        phy_flash.Capacity = 512*1024;
        *(volatile int*)0x1fff0898 = phy_flash.Capacity;
    }

    phy_flash.init_flag = TRUE;
    return PPlus_SUCCESS;
}

#if(FLASH_PROTECT_FEATURE == 1)
int hal_flash_lock(void)
{
    uint32_t cs = spif_lock();
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    AP_SPIF->fcmd = 0x6000001;
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    AP_SPIF->fcmd_wrdata[0] = 0x7c;
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    AP_SPIF->fcmd = 0x1008001;
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    spif_unlock(cs);
    return PPlus_SUCCESS;
}

int hal_flash_unlock(void)
{
    uint32_t cs = spif_lock();
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    AP_SPIF->fcmd = 0x6000001;
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    AP_SPIF->fcmd_wrdata[0] = 0x00;
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    AP_SPIF->fcmd = 0x1008001;
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    spif_unlock(cs);
    return PPlus_SUCCESS;
}

uint8_t hal_flash_get_lock_state(void)
{
    uint32_t cs = spif_lock();
    uint8_t status;
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    status = _spif_read_status_reg_x();
    status = (status & 0x7c)>>2;
    spif_unlock(cs);
    return status;
}
#endif

static void hw_spif_cache_config(void)
{
    spif_config(s_xflashCtx.spif_ref_clk,/*div*/1,s_xflashCtx.rd_instr,0,0);
    AP_SPIF->wr_completion_ctrl=0xff010005;//set longest polling interval
    NVIC_DisableIRQ(SPIF_IRQn);
    NVIC_SetPriority((IRQn_Type)SPIF_IRQn, IRQ_PRIO_HAL);
    hal_cache_init();
    hal_get_flash_info();
}

int hal_spif_cache_init(xflash_Ctx_t cfg)
{
    memset(&(s_xflashCtx), 0, sizeof(s_xflashCtx));
    memcpy(&(s_xflashCtx), &cfg, sizeof(s_xflashCtx));
    hw_spif_cache_config();
    hal_pwrmgr_register(MOD_SPIF, NULL,  hw_spif_cache_config);
    return PPlus_SUCCESS;
}


int hal_flash_read(uint32_t addr, uint8_t* data, uint32_t size)
{
    uint32_t cs = spif_lock();
    volatile uint8_t* u8_spif_addr = (volatile uint8_t*)((addr & 0x7ffff) | FLASH_BASE_ADDR);
    uint32_t cb = AP_PCR->CACHE_BYPASS;
    uint32_t remap;

    if(phy_flash.Capacity > 0x80000)
    {
        remap = addr & 0xf80000;

        if (remap)
        {
            AP_SPIF->remap = remap;
            AP_SPIF->config |= 0x10000;
        }
    }

    //read flash addr direct access
    //bypass cache
    if(cb == 0)
    {
        HAL_CACHE_ENTER_BYPASS_SECTION();
    }

    for(int i=0; i<size; i++)
        data[i]=u8_spif_addr[i];

    //bypass cache
    if(cb == 0)
    {
        HAL_CACHE_EXIT_BYPASS_SECTION();
    }

    if(phy_flash.Capacity > 0x80000)
    {
        if (remap)
        {
            AP_SPIF->remap = 0;
            AP_SPIF->config &= ~0x10000ul;
        }
    }

    spif_unlock(cs);
    return PPlus_SUCCESS;
}

int hal_flash_write(uint32_t addr, uint8_t* data, uint32_t size)
{
    uint8_t retval;
    #if(FLASH_PROTECT_FEATURE == 1)
    hal_flash_unlock();
    #endif
    uint32_t cs = spif_lock();
    HAL_CACHE_ENTER_BYPASS_SECTION();
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    retval = spif_write(addr,data,size);
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    HAL_CACHE_EXIT_BYPASS_SECTION();
    spif_unlock(cs);
    #if(FLASH_PROTECT_FEATURE == 1)
    hal_flash_lock();
    #endif
    return retval;
}

int hal_flash_write_by_dma(uint32_t addr, uint8_t* data, uint32_t size)
{
    uint8_t retval;
    #if(FLASH_PROTECT_FEATURE == 1)
    hal_flash_unlock();
    #endif
    uint32_t cs = spif_lock();
    HAL_CACHE_ENTER_BYPASS_SECTION();
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    retval = spif_write_dma(addr,data,size);
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    HAL_CACHE_EXIT_BYPASS_SECTION();
    spif_unlock(cs);
    #if(FLASH_PROTECT_FEATURE == 1)
    hal_flash_lock();
    #endif
    return retval;
}

int hal_flash_erase_sector(unsigned int addr)
{
    uint8_t retval;
    #if(FLASH_PROTECT_FEATURE == 1)
    hal_flash_unlock();
    #endif
    uint32_t cs = spif_lock();
    uint32_t cb = AP_PCR->CACHE_BYPASS;
    HAL_CACHE_ENTER_BYPASS_SECTION();
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    retval = spif_erase_sector(addr);
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WELWIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    HAL_CACHE_EXIT_BYPASS_SECTION();

    if(cb == 0)
    {
        hal_cache_tag_flush();
    }

    spif_unlock(cs);
    #if(FLASH_PROTECT_FEATURE == 1)
    hal_flash_lock();
    #endif
    return retval;
}

int hal_flash_erase_block64(unsigned int addr)
{
    uint8_t retval;
    #if(FLASH_PROTECT_FEATURE == 1)
    hal_flash_unlock();
    #endif
    uint32_t cs = spif_lock();
    uint32_t cb = AP_PCR->CACHE_BYPASS;
    HAL_CACHE_ENTER_BYPASS_SECTION();
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    retval = spif_erase_block64(addr);
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WELWIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    HAL_CACHE_EXIT_BYPASS_SECTION();

    if(cb == 0)
    {
        hal_cache_tag_flush();
    }

    spif_unlock(cs);
    #if(FLASH_PROTECT_FEATURE == 1)
    hal_flash_lock();
    #endif
    return retval;
}

int hal_flash_erase_all(void)
{
    uint8_t retval;
    #if(FLASH_PROTECT_FEATURE == 1)
    hal_flash_unlock();
    #endif
    uint32_t cs = spif_lock();
    uint32_t cb = AP_PCR->CACHE_BYPASS;
    HAL_CACHE_ENTER_BYPASS_SECTION();
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    retval = spif_erase_all();
    SPIF_STATUS_WAIT_IDLE(SPIF_WAIT_IDLE_CYC);
    spif_wait_nobusy(SFLG_WELWIP, SPIF_TIMEOUT, PPlus_ERR_BUSY);
    HAL_CACHE_EXIT_BYPASS_SECTION();

    if(cb == 0)
    {
        hal_cache_tag_flush();
    }

    spif_unlock(cs);
    #if(FLASH_PROTECT_FEATURE == 1)
    hal_flash_lock();
    #endif
    return retval;
}

int flash_write_word(unsigned int offset, uint32_t  value)
{
    uint32_t temp = value;
    offset &= 0x00ffffff;
    return (hal_flash_write (offset, (uint8_t*) &temp, 4));
}


CHIP_ID_STATUS_e read_chip_mAddr(void)
{
    CHIP_ID_STATUS_e ret = CHIP_ID_UNCHECK;
    uint8_t b;
    for(int i=0;i<CHIP_MADDR_LEN;i++)
    {
        ret = chip_id_one_bit_hot_convter(&b, read_reg(CHIP_MADDR_FLASH_ADDRESS+(i<<2)));

        if(ret==CHIP_ID_VALID)
        {
            g_chipMAddr.mAddr[CHIP_MADDR_LEN-1-i]=b;
        }
        else
        {
            if(i>0 && ret==CHIP_ID_EMPTY)
            {
                ret =CHIP_ID_INVALID;
            }

            return ret;
        }
    }

    return ret;
}

void check_chip_mAddr(void)
{
    //chip id check
    for(int i=0;i<CHIP_MADDR_LEN;i++)
    {
       g_chipMAddr.mAddr[i]=0xff;
    }
    g_chipMAddr.chipMAddrStatus=read_chip_mAddr();
}

void LOG_CHIP_MADDR(void)
{
    
    LOG("\n");
    if(g_chipMAddr.chipMAddrStatus==CHIP_ID_EMPTY)
    {
        LOG("[CHIP_MADDR EMPTY]\n");
    }
    else if(g_chipMAddr.chipMAddrStatus==CHIP_ID_INVALID)
    {
        LOG("[CHIP_MADDR INVALID]\n");
    }
    else if(g_chipMAddr.chipMAddrStatus==CHIP_ID_VALID)
    {

        LOG("[CHIP_MADDR VALID]\n");
        for(int i=0;i<CHIP_MADDR_LEN;i++)
        {
            LOG("%02x",g_chipMAddr.mAddr[i]);
        }
        LOG("\n");

    }
    else
    {
        LOG("[CHIP_MADDR UNCHECKED]\n");
    }
}


