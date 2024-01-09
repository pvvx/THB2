/*************
 i2c_io.h
 SDK_LICENSE
***************/

#ifndef __IIC_H
#define __IIC_H

#ifdef __cplusplus
extern "C" {
#endif


#include "sys.h"
#include "delay.h"
#include "stdbool.h"



void I2C_GPIO_Config(void);
void I2C_delay(void);
void I2C_delay_100us(u32 nCount);
bool I2C_Start(void);
void I2C_Stop(void);
void I2C_Ack(void);
void I2C_NoAck(void);
bool I2C_WaitAck(void);
void I2C_SendByte(u8 SendByte);
u8   I2C_ReceiveByte(void);
bool I2C_WriteByte(u8 SendByte, u16 WriteAddress, u8 DeviceAddress);
bool I2C_ReadByte(u8* pBuffer,u8 length,u16 ReadAddress,u8 DeviceAddress);

#ifdef __cplusplus
}
#endif


#endif
















