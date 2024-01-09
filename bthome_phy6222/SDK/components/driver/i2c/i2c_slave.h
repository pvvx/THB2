/*******************************************************************************
    @file     i2c_slave.h
    @brief    i2c slave Configuration,API...
    @version  1.0

 SDK_LICENSE

*******************************************************************************/

#ifndef __I2C_SLAVE_H__
#define __I2C_SLAVE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
    @ Module           :  Includes
    @ Description    :  NULL
*******************************************************************************/
#include "types.h"
#include "i2c_common.h"

/*******************************************************************************
    @ Module               :   I2C Slave Paramter structure
    @ Description    :   None
*******************************************************************************/
typedef struct
{
    // General
    uint8_t                     id;                                         // set
    I2C_WorkMode            workmode;                               // init some Para ,according to workmode
    I2C_ADDRESS_e       AddressMode;
    uint8_t                     RX_FIFO_Len;                        // RX , TX FIFO SET
    uint8_t                     Tx_FIFO_Len;
    gpio_pin_e              SDA_PIN;                                // I2C Pin
    gpio_pin_e              SCL_PIN;
    uint32_t                    IRQ_Source;
    uint8_t                     Slave_Address;                  // SAR,when as Slave
    I2C_Hdl_t               evt_handler;
} I2C_Slave_Parameter;


/*******************************************************************************
    @ Module           :  Function Statement
    @ Description    :  NULL
*******************************************************************************/
uint8_t Hal_I2C_Slave_Init(I2C_Slave_Parameter* para,uint8_t* handle);
uint8_t Hal_I2c_Slave_Open(uint8_t handle);
void Hal_I2c_Slave_Close(uint8_t handle);
uint8_t Hal_I2C_Slave_Deinit(uint8_t* handle);
uint8_t Hal_Check_I2C_Slave_Closed(uint8_t handle);
void Hal_I2C_Slave_ReadRX_FIFO(uint8_t handle,uint8_t* p,uint8_t len);
void Hal_I2C_Slave_CLR_IRQs(uint8_t handle,uint32_t irqs);
void Hal_I2C_Slave_WriteTX_FIFO(uint8_t handle,uint8_t* p,uint8_t len);

#ifdef __cplusplus
}
#endif

#endif
