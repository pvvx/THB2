/*******************************************************************************
    @file     i2c.h
    @brief    Contains all functions support for i2c driver
    @version  0.0
    @date     25. Oct. 2017
    @author   qing.han

    SDK_LICENSE

*******************************************************************************/
#ifndef __I2C__H__
#define __I2C__H__


#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
    @ Module           :  pre-compile
    @ Description    :  NULL
*******************************************************************************/
#ifdef _I2C_CMD_
#define I2C_Ext
#else
#define I2C_Ext extern
#endif

/*******************************************************************************
    @ Module               :  Includes
    @ Description    :  None
*******************************************************************************/
#include "types.h"
#include "bus_dev.h"
#include "gpio.h"
#include "clock.h"

#define I2C_USE_TIMEOUT 1
#if(I2C_USE_TIMEOUT == 1)
#define I2C_INIT_TOUT(to) int to = hal_systick()
#define I2C_CHECK_TOUT(to, timeout, loginfo) {if(hal_ms_intv(to) > timeout){LOG(loginfo);return PPlus_ERR_TIMEOUT;}}
#else
#define I2C_INIT_TOUT(to)
#define I2C_CHECK_TOUT(to, timeout, loginfo)
#endif


/*******************************************************************************
    @ Module               :  Macro Define
    @ Description    :  None
*******************************************************************************/
typedef enum
{
    I2C_0                 =0,    //define master mode,1:master mode,0:salve mode
    I2C_1                 =1    //define master mode,1:master mode,0:salve mode
} i2c_dev_t;

/*******************************************************************************
    @ Module               :  Macro Define
    @ Description    :  I2C_SLAVE_ADDR_DEF ---- as slave mode
                     I2C_MASTER_ADDR_DEF --- as master mode addressing device
*******************************************************************************/
#define  I2C_SLAVE_ADDR_DEF           0x10
#define  I2C_MASTER_ADDR_DEF          I2C_SLAVE_ADDR_DEF

/*******************************************************************************
    @ Module           :  I2C Buffer Length
    @ Description    :  relate to RX FIFO & TX FIFO
*******************************************************************************/
#define I2C_FIFO_DEPTH          8
#define I2C_RX_TL_CNT               I2C_FIFO_DEPTH
#define I2C_TX_TL_CNT               I2C_FIFO_DEPTH

/*******************************************************************************
    @ Module               :  Macro Define
    @ Description    :  Which IIC To be used
*******************************************************************************/
#define  USE_AP_I2CX          I2C0    //define use i2c0 or i2c1,0:i2c0,1:i2c1

/*******************************************************************************
    @ Module               :  Macro Define
    @ Description    :  None
*******************************************************************************/
#define  I2C0_IRQ_ENABLE()           *(volatile unsigned int *) 0xe000e100 |= BIT(12)
#define  I2C0_IRQ_DISABLE()          *(volatile unsigned int *) 0xe000e100 &= ~BIT(12)
#define  I2C1_IRQ_ENABLE()           *(volatile unsigned int *) 0xe000e100 |= BIT(13)
#define  I2C1_IRQ_DISABLE()          *(volatile unsigned int *) 0xe000e100 &= ~BIT(13)
#define  I2C_READ_CMD(pi2cdev)              (pi2cdev->IC_DATA_CMD = 0x100)   //Read
#define  I2C_RX_FIFO_FULL(pi2cdev)          ((pi2cdev->IC_STATUS & 0x10)==0x10)
#define  I2C_RX_FIFO_NOT_EMPTY(pi2cdev)     ((pi2cdev->IC_STATUS & 0x08)==0x08)
#define  I2C_TX_FIFO_EMPTY(pi2cdev)         ((pi2cdev->IC_STATUS & 0x04)==0x04)
#define  I2C_TX_FIFO_NOT_FULL(pi2cdev)      ((pi2cdev->IC_STATUS & 0x02)==0x02)
#define  I2C_WAIT_RD_REQ(pi2cdev)           !((pi2cdev->IC_RAW_INTR_STAT & 0x20)==0x20)
#define  I2C_RD_REQ(pi2cdev)                ((pi2cdev->IC_RAW_INTR_STAT & 0x20)==0x20)
#define  I2C_NUMBER_DATA_RX_FIFO(pi2cdev)   (pi2cdev->IC_RXFLR)
#define  I2C_NUMBER_DATA_TX_FIFO(pi2cdev)   (pi2cdev->IC_TXFLR)
#define  I2C_CLR_RD_REQ(pi2cdev)            (pi2cdev->IC_CLR_RD_REG)
#define  I2C_CLR_TX_ABRT(pi2cdev)           (pi2cdev->IC_CLR_TX_ABRT)
#define  I2C_ENABLE(pi2cdev)                (pi2cdev->IC_ENABLE=1)
#define  I2C_DISABLE(pi2cdev)               (pi2cdev->IC_ENABLE=0)

/*******************************************************************************
    @ Module           :  I2C Interrupt Mask Register
    @ Description    :  Interrupt MASK bit
*******************************************************************************/
#define I2C_MASK_RX_UNDER           0x0001
#define I2C_MASK_RX_OVER            0x0002
#define I2C_MASK_RX_FULL            0x0004
#define I2C_MASK_TX_OVER            0x0008
#define I2C_MASK_TX_EMPTY           0x0010
#define I2C_MASK_RD_REQ             0x0020
#define I2C_MASK_TX_ABRT            0x0040
#define I2C_MASK_RX_DONE            0x0080
#define I2C_MASK_ACTIVITY           0x0100
#define I2C_MASK_STOP_DET           0x0200
#define I2C_MASK_START_DET      0x0400
#define I2C_MASK_GEN_CALL           0x0800

/*******************************************************************************
    @ Module           :  I2C Status Register
    @ Description    :  Status Register BIT(Indicate transfer and FIFO Status)
*******************************************************************************/
#define I2C_STATUS_ACTIVITY             0x0001
#define I2C_STATUS_TFNF                     0x0002
#define I2C_STATUS_TFE                      0x0004
#define I2C_STATUS_RFNE                     0x0008
#define I2C_STATUS_RFF                      0x0010
#define I2C_STATUS_MST_ACTIVITY     0x0020
#define I2C_STATUS_SLV_ACTIVITY     0x0040

#define I2C0_IRQ I2C0_IRQn
#define I2C1_IRQ I2C1_IRQn
/*******************************************************************************
    @ Module               :  IIC Speed Mode
    @ Description    :  None
*******************************************************************************/
typedef enum
{
    SPEED_STANDARD = 1,        //standard mode
    SPEED_FAST,                //fast mode
//   SPEED_HIGH                 //high mode
} I2C_SPEED_e;

/*******************************************************************************
    @ Module               :  IIC Clock
    @ Description    :  None
*******************************************************************************/
typedef enum
{
    I2C_CLOCK_100K  = 0x00,
    I2C_CLOCK_400K,
} I2C_CLOCK_e;

typedef struct _I2C_Evt_t
{
    uint16_t   type;
    uint8_t*  data;
    uint8_t   len;
} I2C_Evt_t;

typedef enum
{
    I2C_TX_STATE_UNINIT = 0,
    I2C_TX_STATE_IDLE,
    I2C_TX_STATE_TX,
    I2C_TX_STATE_ERR
} I2C_STATE;

/*******************************************************************************
    @ Module               :  Function declaration
    @ Description    :  None
*******************************************************************************/
int hal_i2c_send(void* pi2c, uint8_t* str,uint8_t len);
void* hal_i2c_init(i2c_dev_t dev, I2C_CLOCK_e i2c_clock_rate);
int hal_i2c_deinit(void* pi2c);
int hal_i2c_pin_init(i2c_dev_t dev, gpio_pin_e pin_sda, gpio_pin_e pin_clk);
int hal_i2c_addr_update(void* pi2c, uint8_t addr);
int hal_i2c_wait_tx_completed(void* pi2c);
int hal_i2c_tx_start(void* pi2c);
int hal_i2c_read(void* pi2c,uint8_t slave_addr,uint8_t reg,uint8_t* data,uint8_t size);

#ifdef __cplusplus
}
#endif


#endif
