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

typedef struct _dev_i2c_t {
	AP_I2C_TypeDef * pi2cdev;
	uint8_t scl;	// gpio_pin_e
	uint8_t sda;	// gpio_pin_e
	uint8_t	speed;	// i2c_speed_e
	uint8_t i2c_num;
} dev_i2c_t, * pdev_i2c_t;

void init_i2c(pdev_i2c_t pi2c_dev);
void deinit_i2c(pdev_i2c_t pi2c_dev);
int send_i2c_byte(pdev_i2c_t pi2c_dev, uint8_t addr, uint8_t data);
int send_i2c_wreg(pdev_i2c_t pi2c_dev, uint8 addr, uint8 reg, uint16 data);
int send_i2c_buf(pdev_i2c_t pi2c_dev, uint8 addr, uint8 * pdata, int len);
int read_i2c_bytes(pdev_i2c_t pi2c_dev, uint8 addr, uint8 reg, uint8 * data, uint8 size);
int read_i2c_nabuf(pdev_i2c_t pi2c_dev, uint8 addr, uint8 * data, uint8 size);


#endif /* _DEV_I2C_H_ */
