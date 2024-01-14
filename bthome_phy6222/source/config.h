/*
 * config.h
 *
 *  Created on: 11 янв. 2024 г.
 *      Author: pvvx
 */

#ifndef SOURCE_CONFIG_H_
#define SOURCE_CONFIG_H_

#ifndef APP_VERSION
#define APP_VERSION	0x05	// BCD
#endif

#define DEVICE_THB2		1
#define DEVICE_BTH01	2
#define DEVICE_TH05		3

#ifndef DEVICE
#define DEVICE DEVICE_THB2
#endif

#define DEF_SOFTWARE_REVISION	{'V', '0'+ (APP_VERSION >> 4), '.' , '0'+ (APP_VERSION & 0x0F), 0}

#if DEVICE == DEVICE_THB2
/* Model: THB2 */
#define ADC_PIN_USE_OUT		0
#define ADC_PIN 	GPIO_P11
#define ADC_CHL 	ADC_CH1N_P11

#define I2C_SDA 	GPIO_P18
#define I2C_SCL 	GPIO_P20
#define GPIO_KEY	GPIO_P07
#define GPIO_LED	GPIO_P26
#define LED_ON		0
#define LED_OFF		1

#define DEF_MODEL_NUMBER_STR		"THB2"
#define DEF_HARDWARE_REVISION		"0001"
#define DEF_MANUFACTURE_NAME_STR	"Tuya"

#elif DEVICE == DEVICE_BTH01
/* Model: BTH01 */

#define ADC_PIN_USE_OUT		1	// hal_gpio_write(ADC_PIN, 1);
#define ADC_PIN 	GPIO_P11
#define ADC_CHL 	ADC_CH1N_P11

#define I2C_SDA 	GPIO_P33 // CHT8305_SDA
#define I2C_SCL 	GPIO_P34 // CHT8305_SCL
#define GPIO_SPWR	GPIO_P00 // питание сенсора CHT8305_VDD
#define GPIO_KEY	GPIO_P14
#define GPIO_LED	GPIO_P15
#define LED_ON		1
#define LED_OFF		0

#define DEF_MODEL_NUMBER_STR		"BTH01"
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
#define DEFAULT_DESIRED_SLAVE_LATENCY			29 // (29+1)*30 = 900 ms
// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT			400 // 4s

#endif /* SOURCE_CONFIG_H_ */
