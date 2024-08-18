/*
	sensor.h
	Author: pvvx
*/

#ifndef _SRC_SENSORS_H_
#define _SRC_SENSORS_H_

#include "config.h"

#if (DEV_SERVICES & SERVICE_THS)

#include "dev_i2c.h"

// Timing
#define SENSOR_POWER_TIMEOUT_ms		3
#define SENSOR_RESET_TIMEOUT_ms		3
#define SENSOR_MEASURING_TIMEOUT_ms	7

#define CHT83xx_I2C_ADDR	0x40
#define CHT83xx_REG_TMP		0x00
#define CHT83xx_REG_HMD		0x01
#define CHT83xx_REG_MID		0xfe
#define CHT83xx_REG_VID		0xff

#define CHT83xx_MID	0x5959


/* CHT8310 https://github.com/pvvx/pvvx.github.io/blob/master/THB2/CHT8310.Advanced.Datasheet_Ver1.0.20230407.pdf */
//	I2C addres

#define CHT8215_I2C_ADDR0	0x40
#define CHT8215_I2C_ADDR1	0x44
#define CHT8215_I2C_ADDR2	0x48
#define CHT8215_I2C_ADDR3	0x4C

//	Registers
#define CHT8215_REG_TMP		0x00
#define CHT8215_REG_HMD		0x01
#define CHT8215_REG_STA		0x02
#define CHT8215_REG_CFG		0x03
#define CHT8215_REG_CRT		0x04
#define CHT8215_REG_TLL		0x05
#define CHT8215_REG_TLM		0x06
#define CHT8215_REG_HLL		0x07
#define CHT8215_REG_HLM		0x08
#define CHT8215_REG_OST		0x0f
#define CHT8215_REG_RST		0xfc
#define CHT8215_REG_MID		0xfe
#define CHT8215_REG_VID		0xff

//	Status register mask
#define CHT8215_STA_BUSY	0x8000
#define CHT8215_STA_THI		0x4000
#define CHT8215_STA_TLO		0x2000
#define CHT8215_STA_HHI		0x1000
#define CHT8215_STA_HLO		0x0800

//	Config register mask
#define CHT8215_CFG_MASK		0x8000
#define CHT8215_CFG_SD			0x4000
#define CHT8215_CFG_ALTH		0x2000
#define CHT8215_CFG_EM			0x1000
#define CHT8215_CFG_EHT			0x0100
#define CHT8215_CFG_TME			0x0080
#define CHT8215_CFG_POL			0x0020
#define CHT8215_CFG_ALT			0x0018
#define CHT8215_CFG_CONSEC_FQ	0x0006
#define CHT8215_CFG_ATM			0x0001

#define CHT8315_MID	0x5959
#define CHT8215_VID	0x1582

/* CHT8305 https://github.com/pvvx/pvvx.github.io/blob/master/BTH01/CHT8305.pdf */

//  I2C addres
#define CHT8305_I2C_ADDR0		0x40
#define CHT8305_I2C_ADDR1		0x41
#define CHT8305_I2C_ADDR2		0x42
#define CHT8305_I2C_ADDR3		0x43
#define CHT8305_I2C_ADDR_MAX	0x43

//  Registers
#define CHT8305_REG_TMP		0x00
#define CHT8305_REG_HMD		0x01
#define CHT8305_REG_CFG		0x02
#define CHT8305_REG_ALR		0x03
#define CHT8305_REG_VLT		0x04
#define CHT8305_REG_MID		0xfe
#define CHT8305_REG_VID		0xff

//  Config register mask
#define CHT8305_CFG_SOFT_RESET          0x8000
#define CHT8305_CFG_CLOCK_STRETCH       0x4000
#define CHT8305_CFG_HEATER              0x2000
#define CHT8305_CFG_MODE                0x1000
#define CHT8305_CFG_VCCS                0x0800
#define CHT8305_CFG_TEMP_RES            0x0400
#define CHT8305_CFG_HUMI_RES            0x0300
#define CHT8305_CFG_ALERT_MODE          0x00C0
#define CHT8305_CFG_ALERT_PENDING       0x0020
#define CHT8305_CFG_ALERT_HUMI          0x0010
#define CHT8305_CFG_ALERT_TEMP          0x0008
#define CHT8305_CFG_VCC_ENABLE          0x0004
#define CHT8305_CFG_RESERVED	        0x0003

/*
struct __attribute__((packed)) _cht8305_config_t{
	uint16_t reserved 	: 2;
	uint16_t vccen		: 1;
	uint16_t talt		: 1;
	uint16_t halt 		: 1;
	uint16_t aps 		: 1;
	uint16_t altm		: 2;
	uint16_t h_res 		: 2;
	uint16_t t_res		: 1;
	uint16_t vccs		: 1;
	uint16_t mode		: 1;
	uint16_t heater		: 1;
	uint16_t clkstr		: 1;
	uint16_t srst		: 1;
} cht8305_config_t;
*/

#define CHT8305_MID	0x5959
#define CHT8305_VID	0x0583

/*---------------------------------------
 Датчик влажности AHT25
---------------------------------------*/
#define AHT2x_I2C_ADDR	0x38

#define AHT2x_CMD_INI	0x0E1  // Initialization Command
#define AHT2x_CMD_TMS	0x0AC  // Trigger Measurement Command
#define AHT2x_DATA_TMS	0x3300  // Trigger Measurement data
#define AHT2x_CMD_RST	0x0BA  // Soft Reset Command
#define AHT2x_DATA_LPWR	0x0800 // go into low power mode

enum {
	TH_SENSOR_NONE = 0,
	TH_SENSOR_SHTC3,   // 1
	TH_SENSOR_SHT4x,   // 2
	TH_SENSOR_SHT30,	// 3
	TH_SENSOR_CHT8305,	// 4
	TH_SENSOR_AHT2x,	// 5
	TH_SENSOR_CHT8215,	// 6
	TH_SENSOR_TYPE_MAX // 7
}; // TH_SENSOR_TYPES

typedef struct __attribute__((packed)) _measured_flg_t {
	uint8_t 	pin_input	:	1; // GPIO_INP input pin
	uint8_t 	trg_output	:	1; // GPIO_TRG pin output value
	uint8_t 	comfort		:	1; // Temperature or Humidity comfort
	uint8_t 	trg_on 		:	1; // Temperature or Humidity trigger on
	uint8_t 	temp_trg_on :	1; // Temperature trigger on
	uint8_t 	humi_trg_on :	1; // Humidity trigger on
} measured_flg_t;

typedef struct _measured_data_t {
	uint16_t	count;
	int16_t		temp; // x 0.01 C
	int16_t		humi; // x 0.01 %
	uint16_t	battery_mv; // mV
	uint8_t		battery; // 0..100 %
	measured_flg_t flg;
} measured_data_t;
#define send_len_measured_data 10
extern measured_data_t measured_data;

typedef struct _thsensor_coef_t {
	uint32_t temp_k;
	uint32_t humi_k;
	int16_t temp_z;
	int16_t humi_z;
} thsensor_coef_t;	// [12]

typedef struct _thsensor_def_cfg_t {
	thsensor_coef_t coef;
//	uint32_t measure_timeout;
	uint8_t sensor_type; // TH_SENSOR_TYPES
} thsensor_def_cfg_t;

typedef int (*psernsor_rd_t)(pdev_i2c_t pi2c_dev);
//typedef void (*psernsor_sm_t)(void);

typedef struct _thsensor_cfg_t {
	thsensor_coef_t coef;
	uint16_t mid;
	uint16_t vid;
	uint8_t i2c_addr;
	uint8_t sensor_type; // TH_SENSOR_TYPES
	psernsor_rd_t read_sensor;
//	psernsor_sm_t start_measure;
} thsensor_cfg_t;
extern thsensor_cfg_t thsensor_cfg;
#define thsensor_cfg_send_size 19

void init_sensor(void);
void start_measure(void);
int read_sensors(void);

#else // (DEV_SERVICES & SERVICE_THS)

typedef struct _measured_data_t {
	uint16_t	count;
//	int16_t		temp; // x 0.01 C
//	int16_t		humi; // x 0.01 %
	uint16_t	battery_mv; // mV
	uint8_t		battery; // 0..100 %
} measured_data_t;

extern measured_data_t measured_data;


#endif // (DEV_SERVICES & SERVICE_THS)

#endif // _SRC_SENSORS_H_
