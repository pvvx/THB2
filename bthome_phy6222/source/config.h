/*
 * config.h
 *
 *  Created on: 11 янв. 2024 г.
 *      Author: pvvx
 */

#ifndef SOURCE_CONFIG_H_
#define SOURCE_CONFIG_H_


#define APP_VERSION	0x04	// BCD

#define DEVICE_BTH2		1
#define DEVICE_BTH01	2
#define DEVICE_TH05		3

#ifndef DEVICE
#define DEVICE DEVICE_BTH2
#endif

#define DEF_SOFTWARE_REVISION	{'V', '0'+ (APP_VERSION >> 4), '.' , '0'+ (APP_VERSION & 0x0F), 0}

#if DEVICE == DEVICE_BTH2
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

#else
#error "DEVICE Not released!"
#endif


// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL		24 // 12 -> 15 ms
// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL		24 // 30 ms
// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY			29
// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT			400 // 4s


// fs ids
#define FS_ID_MAC 	0xACAD

#endif /* SOURCE_CONFIG_H_ */
