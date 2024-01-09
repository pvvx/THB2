/*******************************************************************************
    @file     spi.h
    @brief    Contains all functions support for spi driver
    @version  0.0
    @date     18. Oct. 2017
    @author   qing.han

 SDK_LICENSE

*******************************************************************************/
#ifndef _SPI_H_
#define _SPI_H_

#include "types.h"
#include "gpio.h"
#include "clock.h"
#include "bus_dev.h"

#define  SPI_MASTER_MODE      1    //define master mode,1:master mode,0:salve mode

#define  SPI_USE_TIMEOUT 1
#define  SPI_OP_TIMEOUT  1000        //100ms for an Byte operation
#if(SPI_USE_TIMEOUT == 1)
    #define SPI_INIT_TOUT(to)               int to = hal_systick()
    #define SPI_CHECK_TOUT(to, timeout,loginfo)   {if(hal_ms_intv(to) > timeout){LOG(loginfo);return PPlus_ERR_TIMEOUT;}}
#else
    #define SPI_INIT_TOUT(to)
    #define SPI_CHECK_TOUT(to, timeout,loginfo)
#endif

#define  ENABLE_SPI           Ssix->SSIEN = 1
#define  DISABLE_SPI          Ssix->SSIEN = 0
#define  NUMBER_DATA_RX_FIFO  Ssix->RXFLR
#define  NUMBER_DATA_TX_FIFO  Ssix->TXFLR

#define  SPI_BUSY             0x1
#define  TX_FIFO_NOT_FULL     0x2
#define  TX_FIFO_EMPTY        0x4
#define  RX_FIFO_NOT_EMPTY    0x8

typedef enum
{
    SPI_MODE0=0,      //SCPOL=0,SCPH=0
    SPI_MODE1,        //SCPOL=0,SCPH=1
    SPI_MODE2,        //SCPOL=1,SCPH=0
    SPI_MODE3,        //SCPOL=1,SCPH=1
} SPI_SCMOD_e;

typedef enum
{


    SPI_4BIT = 0x03,
    SPI_5BIT = 0x04,
    SPI_6BIT = 0x05,
    SPI_7BIT = 0x06,
    SPI_8BIT = 0x07,SPI_1BYTE = 0x07, // 1byte
    SPI_9BIT = 0x08,
    SPI_10BIT = 0x09,
    SPI_11BIT = 0x0a,
    SPI_12BIT = 0x0b,
    SPI_13BIT = 0x0c,
    SPI_14BIT = 0x0d,
    SPI_15BIT = 0x0e,
    SPI_16BIT = 0x0f,SPI_2BYTE = 0x0f, // 2byte
} SPI_DFS_e;

typedef enum
{
    SPI_TRXD=0,        //Transmit & Receive
    SPI_TXD,         //Transmit Only
    SPI_RXD,         //Receive Only
    SPI_EEPROM,      //EEPROM Read
} SPI_TMOD_e;

typedef enum
{
    SPI0=0,          //use spi 0
    SPI1,          //use spi 1
} SPI_INDEX_e;


typedef enum
{
    SPI_TX_COMPLETED = 1,
    SPI_RX_COMPLETED,
    SPI_TX_REQ_S,   //slave tx
    SPI_RX_DATA_S,  //slave rx
} SPI_EVT_e;

typedef struct _spi_evt_t
{
    uint8_t     id;
    SPI_EVT_e   evt;
    uint8_t*    data;
    uint8_t     len;
} spi_evt_t;

typedef void (*spi_hdl_t)(spi_evt_t* pevt);

typedef struct _spi_Cfg_t
{
    GPIO_Pin_e      sclk_pin;
    GPIO_Pin_e      ssn_pin;
    GPIO_Pin_e      MOSI;
    GPIO_Pin_e      MISO;
    uint32_t        baudrate;
    SPI_TMOD_e      spi_tmod;
    SPI_SCMOD_e     spi_scmod;
    SPI_DFS_e       spi_dfsmod;
    #if DMAC_USE
    bool            dma_tx_enable;
    bool            dma_rx_enable;
    #endif
    bool            int_mode;
    bool            force_cs;
    spi_hdl_t       evt_handler;
} spi_Cfg_t;

typedef enum
{
    TRANSMIT_FIFO_EMPTY = 0x01,
    TRANSMIT_FIFO_OVERFLOW = 0x02,
    RECEIVE_FIFO_UNDERFLOW = 0x04,
    RECEIVE_FIFO_OVERFLOW = 0x08,
    RECEIVE_FIFO_FULL = 0x10,
} SPI_INT_STATUS_e;

typedef struct _hal_spi_t
{
    SPI_INDEX_e spi_index;
} hal_spi_t;

typedef struct
{
    bool        busy;
    uint16_t    xmit_len;
    uint16_t    buf_len;
    uint8_t*   tx_buf;
    uint8_t*   rx_buf;
    uint16_t    tx_offset;
    uint16_t    rx_offset;
} spi_xmit_t;

void __attribute__((weak)) hal_SPI0_IRQHandler(void);

void __attribute__((weak)) hal_SPI1_IRQHandler(void);

void hal_spi_tmod_set(hal_spi_t* spi_ptr,SPI_TMOD_e mod);
void hal_spi_dfs_set(hal_spi_t* spi_ptr,SPI_DFS_e mod);

int hal_spis_clear_rx(hal_spi_t* spi_ptr);
uint32_t hal_spis_rx_len(hal_spi_t* spi_ptr);
int hal_spis_read_rxn(hal_spi_t* spi_ptr, uint8_t* pbuf, uint16_t len);
int hal_spi_bus_init(hal_spi_t* spi_ptr,spi_Cfg_t cfg);
int hal_spis_bus_init(hal_spi_t* spi_ptr,spi_Cfg_t cfg);

int hal_spi_bus_deinit(hal_spi_t* spi_ptr);

int hal_spi_init(SPI_INDEX_e channel);

int hal_spi_transmit
(
    hal_spi_t*  spi_ptr,
    SPI_TMOD_e  mod,
    uint8_t*    tx_buf,
    uint8_t*    rx_buf,
    uint16_t    tx_len,
    uint16_t    rx_len
);

int hal_spi_set_tx_buffer(hal_spi_t* spi_ptr,uint8_t* tx_buf,uint16_t len);
int hal_spi_set_int_mode(hal_spi_t* spi_ptr,bool en);
int hal_spi_set_force_cs(hal_spi_t* spi_ptr,bool en);

bool hal_spi_get_transmit_bus_state(hal_spi_t* spi_ptr);
int hal_spi_TxComplete(hal_spi_t* spi_ptr);
int hal_spi_send_byte(hal_spi_t* spi_ptr,uint8_t data);

#if DMAC_USE
    int hal_spi_dma_set(hal_spi_t* spi_ptr,bool ten,bool ren);
#endif


#endif
