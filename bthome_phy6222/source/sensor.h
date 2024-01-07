
#ifndef _SENSORS_H_
#define _SENSORS_H_

#include <stdint.h>


// Timing
#define SENSOR_POWER_TIMEOUT_ms  	5
#define SENSOR_RESET_TIMEOUT_ms		5
#define SENSOR_MEASURING_TIMEOUT_ms	7

//  I2C addres
#define CHT8310_I2C_ADDR0	0x40
#define CHT8310_I2C_ADDR1	0x44
#define CHT8310_I2C_ADDR2	0x48
#define CHT8310_I2C_ADDR3	0x4C

//  Registers
#define CHT8310_REG_TMP		0x00
#define CHT8310_REG_HMD		0x01
#define CHT8310_REG_STA		0x02
#define CHT8310_REG_CFG		0x03
#define CHT8310_REG_CRT		0x04
#define CHT8310_REG_TLL		0x05
#define CHT8310_REG_TLM		0x06
#define CHT8310_REG_HLL		0x07
#define CHT8310_REG_HLM		0x08
#define CHT8310_REG_OST		0x0f
#define CHT8310_REG_RST		0xfc
#define CHT8310_REG_ID		0xfe

//  Status register mask
#define CHT8310_STA_BUSY    0x8000
#define CHT8310_STA_THI     0x4000
#define CHT8310_STA_TLO     0x2000
#define CHT8310_STA_HHI     0x1000
#define CHT8310_STA_HLO     0x0800

//  Config register mask
#define CHT8310_CFG_MASK      0x8000
#define CHT8310_CFG_SD        0x4000
#define CHT8310_CFG_ALTH      0x2000
#define CHT8310_CFG_EM        0x1000
#define CHT8310_CFG_EHT       0x0100
#define CHT8310_CFG_TME       0x0080
#define CHT8310_CFG_POL       0x0020
#define CHT8310_CFG_ALT       0x0018
#define CHT8310_CFG_CONSEC_FQ 0x0006
#define CHT8310_CFG_ATM       0x0001


typedef struct _measured_data_t {
	uint16_t 	count;
	int16_t		temp; // x 0.01 C
	int16_t		humi; // x 0.01 %
	uint16_t	battery_mv; // mV
	uint8_t	  battery; // 0..100 % 
} measured_data_t;

extern measured_data_t measured_data;

void init_sensor(void);
int read_sensor(void);

//void init_i2c(void);
//void deinit_i2c(void);


#endif // _SENSORS_H_
