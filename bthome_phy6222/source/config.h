/*
 * config.h
 *
 *  Created on: 11 янв. 2024 г.
 *      Author: pvvx
 */

#ifndef SOURCE_CONFIG_H_
#define SOURCE_CONFIG_H_

#define APP_VERSION	0x01	// BCD

#define DEF_SOFTWARE_REVISION	{'V', '0'+ (APP_VERSION >> 4), '.' , '0'+ (APP_VERSION & 0x0F), 0}

/* Model: THB2 */

#define ADC_PIN 	GPIO_P11
#define ADC_CHL 	ADC_CH1N_P11
#define I2C_SDA 	GPIO_P18
#define I2C_SCL 	GPIO_P20
#define GPIO_KEY	GPIO_P07
#define GPIO_LED	GPIO_P26

#define DEF_MODEL_NUMBER_STR		"THB2"
#define DEF_HARDWARE_REVISION		"0001"
#define DEF_MANUFACTURE_NAME_STR	"Tuya"



#define FS_ID_MAC 	0xACAD

#endif /* SOURCE_CONFIG_H_ */
