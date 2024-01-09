/*************
 dma.h
 SDK_LICENSE
***************/

/* Peripheral group ----------------------------------------------------------- */
/** @defgroup GPDMA GPDMA (General Purpose Direct Memory Access)
    @ingroup LPC177x_8xCMSIS_FwLib_Drivers
    @{
*/

#ifndef __DMA_H_
#define __DMA_H_

/* Includes ------------------------------------------------------------------- */
#include "bus_dev.h"


#ifdef __cplusplus
extern "C"
{
#endif

/* Public Macros -------------------------------------------------------------- */
/** @defgroup GPDMA_Public_Macros GPDMA Public Macros
    @{
*/

/** DMAC Connection number definitions */
typedef enum
{
    DMA_CONN_MEM=0,//               ((0))           /*memory*/
    DMA_CONN_SPI0_Tx=1,//           ((0UL))         /** SSP0 Tx */
    DMA_CONN_SPI0_Rx,//             ((1UL))         /** SSP0 Rx */
    DMA_CONN_SPI1_Tx,//             ((2UL))         /** SSP1 Tx */
    DMA_CONN_SPI1_Rx,//             ((3UL))         /** SSP1 Rx */

    DMA_CONN_I2C0_Tx=9,//           ((8UL))         /** IIC0 Tx */
    DMA_CONN_I2C0_Rx,//             ((9UL))         /** IIC0 Rx */
    DMA_CONN_I2C1_Tx,//             ((10UL))        /** IIC1 Tx */
    DMA_CONN_I2C1_Rx,//             ((11UL))        /** IIC1 Rx */

    DMA_CONN_UART0_Tx,//            ((10UL))        /** UART0 Tx */
    DMA_CONN_UART0_Rx,//            ((11UL))        /** UART0 Rx */
    DMA_CONN_UART1_Tx,//            ((12UL))        /** UART1 Tx */
    DMA_CONN_UART1_Rx,//            ((13UL))        /** UART1 Rx */
} DMA_CONN_e;


/** Burst size in Source and Destination definitions */
#define DMA_BSIZE_1     ((0UL)) /**< Burst size = 1 */
#define DMA_BSIZE_4     ((1UL)) /**< Burst size = 4 */
#define DMA_BSIZE_8     ((2UL)) /**< Burst size = 8 */
#define DMA_BSIZE_16    ((3UL)) /**< Burst size = 16 */
#define DMA_BSIZE_32    ((4UL)) /**< Burst size = 32 */
#define DMA_BSIZE_64    ((5UL)) /**< Burst size = 64 */
#define DMA_BSIZE_128   ((6UL)) /**< Burst size = 128 */
#define DMA_BSIZE_256   ((7UL)) /**< Burst size = 256 */

/** Width in Source transfer width and Destination transfer width definitions */
#define DMA_WIDTH_BYTE      ((0UL)) /**< Width = 1 byte */
#define DMA_WIDTH_HALFWORD  ((1UL)) /**< Width = 2 bytes */
#define DMA_WIDTH_WORD      ((2UL)) /**< Width = 4 bytes */
#define DMA_WIDTH_2WORD     ((3UL)) /**< Width = 8 bytes */
#define DMA_WIDTH_4WORD     ((4UL)) /**< Width = 16 bytes */
#define DMA_WIDTH_8WORD     ((5UL)) /**< Width = 32 bytes */




/** DMAC Address Increment definitions */
#define DMA_INC_INC         ((0UL)) /**< Increment */
#define DMA_INC_DEC         ((1UL)) /**< Decrement */
#define DMA_INC_NCHG        ((2UL)) /**< No change */


/**
    @}
*/


/* Private Macros ------------------------------------------------------------- */
/** @defgroup GPDMA_Private_Macros GPDMA Private Macros
    @{
*/

/* --------------------- BIT DEFINITIONS -------------------------------------- */
/*********************************************************************//**
    Macro defines for DMA Interrupt Status register
 **********************************************************************/
#define DMA_DMACIntStat_Ch(n)           (((1UL<<n)&0xFF))
#define DMA_DMACIntStat_BITMASK         ((0xFF))

/*********************************************************************//**
    Macro defines for DMA Interrupt Terminal Count Request Status register
 **********************************************************************/
#define DMA_DMACIntTCStat_Ch(n)         (((1UL<<n)&0xFF))
#define DMA_DMACIntTCStat_BITMASK       ((0xFF))

/*********************************************************************//**
    Macro defines for DMA Interrupt Terminal Count Request Clear register
 **********************************************************************/
#define DMA_DMACIntTCClear_Ch(n)        (((1UL<<n)&0xFF))
#define DMA_DMACIntTCClear_BITMASK  ((0xFF))

/*********************************************************************//**
    Macro defines for DMA Interrupt Error Status register
 **********************************************************************/
#define DMA_DMACIntErrStat_Ch(n)        (((1UL<<n)&0xFF))
#define DMA_DMACIntErrStat_BITMASK  ((0xFF))

/*********************************************************************//**
    Macro defines for DMA Interrupt Tfr Clear register
 **********************************************************************/
#define DMA_DMACIntTfrClr_Ch(n)     (((1UL<<n)&0xFF))
#define DMA_DMACIntTfrClr_BITMASK       ((0xFF))

/*********************************************************************//**
    Macro defines for DMA Interrupt Block Clear register
 **********************************************************************/
#define DMA_DMACIntBlockClr_Ch(n)       (((1UL<<n)&0xFF))
#define DMA_DMACIntBlockClr_BITMASK     ((0xFF))

/*********************************************************************//**
    Macro defines for DMA Interrupt SrcTran Clear register
 **********************************************************************/
#define DMA_DMACIntSrcTranClr_Ch(n)     (((1UL<<n)&0xFF))
#define DMA_DMACIntSrcTranClr_BITMASK       ((0xFF))

/*********************************************************************//**
    Macro defines for DMA Interrupt DstTran Clear register
 **********************************************************************/
#define DMA_DMACIntDstTranClr_Ch(n)     (((1UL<<n)&0xFF))
#define DMA_DMACIntDstTranClr_BITMASK       ((0xFF))

/*********************************************************************//**
    Macro defines for DMA Interrupt Error Clear register
 **********************************************************************/
#define DMA_DMACIntErrClr_Ch(n)     (((1UL<<n)&0xFF))
#define DMA_DMACIntErrClr_BITMASK       ((0xFF))

/*********************************************************************//**
    Macro defines for DMA Raw Interrupt Terminal Count Status register
 **********************************************************************/
#define DMA_DMACRawIntTCStat_Ch(n)  (((1UL<<n)&0xFF))
#define DMA_DMACRawIntTCStat_BITMASK    ((0xFF))

/*********************************************************************//**
    Macro defines for DMA Raw Error Interrupt Status register
 **********************************************************************/
#define DMA_DMACRawIntErrStat_Ch(n) (((1UL<<n)&0xFF))
#define DMA_DMACRawIntErrStat_BITMASK   ((0xFF))

/*********************************************************************//**
    Macro defines for DMA Enabled Channel register
 **********************************************************************/
#define DMA_DMACEnbldChns_Ch(n)     (((1UL<<n)&0xFF))
#define DMA_DMACEnbldChns_BITMASK       ((0xFF))

/*********************************************************************//**
    Macro defines for DMA Software Burst Request register
 **********************************************************************/
#define DMA_DMACSoftBReq_Src(n)     (((1UL<<n)&0xFFFF))
#define DMA_DMACSoftBReq_BITMASK        ((0xFFFF))

/*********************************************************************//**
    Macro defines for DMA Software Single Request register
 **********************************************************************/
#define DMA_DMACSoftSReq_Src(n)         (((1UL<<n)&0xFFFF))
#define DMA_DMACSoftSReq_BITMASK        ((0xFFFF))

/*********************************************************************//**
    Macro defines for DMA Software Last Burst Request register
 **********************************************************************/
#define DMA_DMACSoftLBReq_Src(n)        (((1UL<<n)&0xFFFF))
#define DMA_DMACSoftLBReq_BITMASK       ((0xFFFF))

/*********************************************************************//**
    Macro defines for DMA Software Last Single Request register
 **********************************************************************/
#define DMA_DMACSoftLSReq_Src(n)        (((1UL<<n)&0xFFFF))
#define DMA_DMACSoftLSReq_BITMASK       ((0xFFFF))

/*********************************************************************//**
    Macro defines for DMA Configuration register
 **********************************************************************/
#define DMA_DMAC_E                      ((0x01))     /**< DMA Controller enable*/
#define DMA_DMAC_D                      ((0x00))     /**< DMA Controller disable*/
#define DMA_DMAC_INT_E                  ((0x01))     /**< DMA Controller Interrupt enable*/


/*********************************************************************//**
    Macro defines for DMA Synchronization register
 **********************************************************************/
#define DMA_DMACSync_Src(n)         (((1UL<<n)&0xFFFF))
#define DMA_DMACSync_BITMASK            ((0xFFFF))

/*********************************************************************//**
    Macro defines for DMA Request Select register
 **********************************************************************/
#define DMA_DMAReqSel_Input(n)      (((1UL<<(n-8))&0xFF))
#define DMA_DMAReqSel_BITMASK           ((0xFF))

/*********************************************************************//**
    Macro defines for DMA Channel Linked List Item registers
 **********************************************************************/
/** DMAC Channel Linked List Item registers bit mask*/
#define DMA_DMACCxLLI_BITMASK       ((0xFFFFFFFC))

/*********************************************************************//**
    Macro defines for DMA channel control registers
 **********************************************************************/
/** Transfer size*/
#define DMA_DMACCxControl_TransferSize(n) (n&0x7FF)
/** Source burst size*/
#define DMA_DMACCxControl_SMSize(n)     (((n&0x07)<<14))
/** Destination burst size*/
#define DMA_DMACCxControl_DMSize(n)     (((n&0x07)<<11))
/** Source transfer width*/
#define DMA_DMACCxControl_SWidth(n)     (((n&0x07)<<4))
/** Destination transfer width*/
#define DMA_DMACCxControl_DWidth(n)     (((n&0x07)<<1))
/** Source increment*/
#define DMA_DMACCxControl_SInc(n)       (((n&0x03)<<9))
/** Destination increment*/
#define DMA_DMACCxControl_DInc(n)       (((n&0x03)<<7))

/*********************************************************************//**
    Macro defines for DMA Channel Configuration registers
 **********************************************************************/
/** DMAC channel write bit enable*/
#define DMA_DMACCxConfig_E(n)                       ((0x100UL<<n))
/** DMAC channel int mask bit enable*/
#define DMA_DMACCxIntMask_E(n)                      ((0x100UL<<n))

/** Source peripheral*/
#define DMA_DMACCxConfig_SrcPeripheral(n)       (((n&0x1F)))
/** Destination peripheral*/
#define DMA_DMACCxConfig_DestPeripheral(n)      (((n&0x1F)<<4))
/** This value indicates the type of transfer*/
#define DMA_DMACCxConfig_TransferType(n)        (((n&0x7)<<20))
/** Interrupt error mask*/
#define DMA_DMACCxConfig_IE                     ((1UL<<14))
/** Terminal count interrupt mask*/
#define DMA_DMACCxConfig_ITC                    ((1UL<<15))
/** Lock*/
#define DMA_DMACCxConfig_L                      ((1UL<<16))
/** Active*/
#define DMA_DMACCxConfig_A                      ((1UL<<17))
/** Halt*/
#define DMA_DMACCxConfig_H                      ((1UL<<18))
/** DMAC Channel Configuration registers bit mask */
#define DMA_DMACCxConfig_BITMASK                ((0x7FFFF))

/**
    @}
*/
#define DMA_TRANSFERTYPE_M2M            ((0UL))
/** DMAC Transfer type definitions: Memory to peripheral - DMA control */
#define DMA_TRANSFERTYPE_M2P            ((1UL))
/** DMAC Transfer type definitions: Peripheral to memory - DMA control */
#define DMA_TRANSFERTYPE_P2M            ((2UL))
/** DMAC Source peripheral to destination peripheral - DMA control */
#define DMA_TRANSFERTYPE_P2P            ((3UL))

/** If dstnation address is flash, should set this bit- DMA control */
#define DMA_DST_XIMT_IS_FLASH           ((0UL))
/** DMAC Source peripheral to destination peripheral - DMA control */
#define DMA_DST_XIMT_NOT_FLASH          ((1UL))

#define DMA_GET_MAX_TRANSPORT_SIZE(ch)      ((ch == DMA_CH_0) ? 0x7ff : 0x1f)



/**
    @brief DMAC Interrupt clear status enumeration
*/
typedef enum
{
    DMA_STATCLR_INTTC,  /**< GPDMA Interrupt Terminal Count Request Clear */
    DMA_STATCLR_INTERR  /**< GPDMA Interrupt Error Clear */
} DMA_StateClear_Type;

/**
    @brief DMAC Channel configuration structure type definition
*/
typedef struct
{
    uint32_t    transf_size;    /**< Length/Size of transfer */

    uint8_t     sinc;
    uint8_t     src_tr_width;
    uint8_t     src_msize;
    uint32_t    src_addr;

    uint8_t     dinc;
    uint8_t     dst_tr_width;
    uint8_t     dst_msize;
    uint32_t    dst_addr;

    bool        enable_int;
} DMA_CH_CFG_t;

typedef enum
{
    DMA_CH_0 = 0,
    DMA_CH_1,
    DMA_CH_2,
    DMA_CH_3,
    DMA_CH_NUM,
} DMA_CH_t;

/**
    @brief DMAC Linker List Item structure type definition
*/
typedef struct
{
    uint32_t  src_addr; /**< Source Address */
    uint32_t  dst_addr; /**< Destination address */
    uint32_t  lli;      /**< Next LLI address, otherwise set to '0' */
    uint32_t  ctrl;     /**< GPDMA Control of this LLI */
} DMA_LLI_t;

/**
    @brief DMAC callback definition
*/
typedef void (*DMA_Hdl_t)(DMA_CH_t);

typedef struct
{
    DMA_CH_t    dma_channel;
    DMA_Hdl_t   evt_handler;
} HAL_DMA_t;

typedef struct
{
    uint8_t     init_ch;
    uint8_t     interrupt;
    uint8_t     xmit_busy;
    uint8_t     xmit_flash;
    DMA_Hdl_t   evt_handler;
} DMA_CH_Ctx_t;

int hal_dma_init_channel(HAL_DMA_t cfg);
int hal_dma_config_channel(DMA_CH_t ch, DMA_CH_CFG_t* cfg);
int hal_dma_start_channel(DMA_CH_t ch);
int hal_dma_stop_channel(DMA_CH_t ch);
int hal_dma_wait_channel_complete(DMA_CH_t ch);
int hal_dma_status_control(DMA_CH_t ch);
int hal_dma_init(void);
int hal_dma_deinit(void);
void __attribute__((used)) hal_DMA_IRQHandler(void);




#ifdef __cplusplus
}
#endif

#endif /* __LPC177X_8X_GPDMA_H_ */

/**
    @}
*/

/* --------------------------------- End Of File ------------------------------ */
