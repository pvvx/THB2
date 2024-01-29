/*
 * dev_i2c.h
 *
 *  Created on: 25 янв. 2024 г.
 *      Author: pvvx
 */

#ifndef _DEV_I2C_H_
#define _DEV_I2C_H_

typedef enum {
	I2C_100KHZ,
	I2C_400KHZ
}i2c_speed_e;

void init_i2c(i2c_speed_e speed400khz);
void deinit_i2c(void);
int send_i2c_byte(uint8_t addr, uint8_t data);
int send_i2c_wreg(uint8 addr, uint8 reg, uint16 data);
int send_i2c_buf(uint8 addr, uint8 * pdata, int len);
int read_i2c_bytes(uint8 addr, uint8 reg, uint8 * data, uint8 size);
int read_i2c_nabuf(uint8 addr, uint8 * data, uint8 size);


#endif /* _DEV_I2C_H_ */
