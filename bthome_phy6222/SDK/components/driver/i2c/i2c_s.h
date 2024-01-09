/*************
 i2c_s.h
 SDK_LICENSE
***************/

#ifndef _IIC_SLAVE_H
#define _IIC_SLAVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gpio.h"

#define I2CS_RX_MAX_SIZE  64
#define I2CS_TX_MAX_SIZE  64

#define IIC0_SCL  FMUX_IIC0_SCL
#define IIC0_SDA  FMUX_IIC0_SDA

#define IIC1_SCL  FMUX_IIC1_SCL
#define IIC1_SDA  FMUX_IIC1_SDA

//i2cs working state
enum
{
    I2CSST_IDLE = 0,
    I2CSST_XMITING
};

typedef enum
{
    I2CS_0 = 1,
    I2CS_1,
} i2cs_channel_t;

typedef enum
{
    I2CS_MODE_REG_8BIT = 1,
    I2CS_MODE_REG_16BIT,
    I2CS_MODE_RAW
} i2cs_mode_t;

enum
{
    I2CS_EVT_REG_REQ_READ = 1, //register mode read, master read request
    I2CS_EVT_REG_REQ_READ_CMPL, //register mode read, read completed
    I2CS_EVT_REG_RECV,          //register mode write, recieved data

    I2CS_ET_RAW_RECV,   //master write data to slave
    I2CS_ET_RAW_TX_CMPL,  //master read data from slave completed
};//event type


typedef struct
{
    uint8_t   type;
    uint8_t   reg_u8;
    uint16_t  reg_u16;
    uint8_t*  dat;
} i2cs_evt_t;

typedef void (*i2cs_hdl_t)(i2cs_evt_t* pev);
void __attribute__((weak)) hal_I2C0_IRQHandler(void);
void __attribute__((weak)) hal_I2C1_IRQHandler(void);
int i2cs_init(
    i2cs_channel_t  ch_id,
    i2cs_mode_t     mode,
    uint8_t         saddr,  //slave address
    gpio_pin_e      cs,  //if need not cs, choose GPIO_DUMMY_PIN
    gpio_pin_e      sda,
    gpio_pin_e      scl,
    i2cs_hdl_t      evt_handler);


#ifdef __cplusplus
}
#endif


#endif

