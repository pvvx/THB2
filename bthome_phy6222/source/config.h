/*
 * config.h
 *
 *  Created on: 11 янв. 2024 г.
 *      Author: pvvx
 */

#ifndef SOURCE_CONFIG_H_
#define SOURCE_CONFIG_H_

#include <string.h>
#include "types.h"

#ifndef APP_VERSION
#define APP_VERSION	0x08	// BCD
#endif

/*
#define BOARD_LYWSD03MMC_B14		0 // number used for BLE firmware
#define BOARD_MHO_C401				1
#define BOARD_CGG1					2
#define BOARD_LYWSD03MMC_B19		3 // number used for BLE firmware
#define BOARD_LYWSD03MMC_DEVBIS		3 // ver https://github.com/devbis/z03mmc
#define BOARD_LYWSD03MMC_B16		4 // number used for BLE firmware
#define BOARD_WATERMETER			4 // ver https://github.com/slacky1965/watermeter_zed
#define BOARD_LYWSD03MMC_B17		5 // number used for BLE firmware
#define BOARD_CGDK2					6
#define BOARD_CGG1N					7 // 2022
#define BOARD_MHO_C401N				8 // 2022
#define BOARD_MJWSD05MMC			9
#define BOARD_LYWSD03MMC_B15		10 // number used for BLE firmware
#define BOARD_MHO_C122				11
#define BOARD_TNK					16 // Water tank controller (not yet published at the moment)
#define BOARD_TS0201_TZ3000			17
#define BOARD_TS0202_TZ3000			18 // ?
*/
#define DEVICE_THB2		19
#define DEVICE_BTH01	20
#define DEVICE_TH05		21

#ifndef DEVICE
#define DEVICE		DEVICE_THB2
#info "Device not selected, assuming THB2"
#endif

// supported services by the device (bits)
#define SERVICE_OTA			0x00000001
#define SERVICE_OTA_EXT		0x00000002
#define SERVICE_PINCODE 	0x00000004	// пока нет
#define SERVICE_BINDKEY 	0x00000008	// пока нет
#define SERVICE_HISTORY 	0x00000010
#define SERVICE_SCREEN		0x00000020
#define SERVICE_LE_LR		0x00000040	// пока нет
#define SERVICE_THS			0x00000080
#define SERVICE_RDS			0x00000100	// пока нет
#define SERVICE_KEY			0x00000200
#define SERVICE_OUTS		0x00000400	// пока нет
#define SERVICE_INS			0x00000800	// пока нет
#define SERVICE_TIME_ADJUST 0x00001000	// пока нет
#define SERVICE_HARD_CLOCK	0x00002000	// пока нет

#define OTA_TYPE_NONE	0	// нет OTA
#define OTA_TYPE_BOOT	(SERVICE_OTA | SERVICE_OTA_EXT)		// вариант для прошивки boot + OTA
#define OTA_TYPE_APP	SERVICE_OTA_EXT	// переключение из APP на OTA + boot прошивку, пока не реализовано

#ifndef OTA_TYPE
#define OTA_TYPE	OTA_TYPE_BOOT
#info "OTA type not selected, assuming OTA boot"
#endif

#define DEF_SOFTWARE_REVISION	{'V', '0'+ (APP_VERSION >> 4), '.' , '0'+ (APP_VERSION & 0x0F), 0}

#if DEVICE == DEVICE_THB2
/* Model: THB2 */
#if (OTA_TYPE == OTA_TYPE_BOOT)
#define DEV_SERVICES ( \
          OTA_TYPE \
		| SERVICE_THS \
		| SERVICE_KEY \
)
#else
#define DEV_SERVICES ( \
          OTA_TYPE \
		| SERVICE_THS \
		| SERVICE_KEY \
		| SERVICE_HISTORY \
)
#endif

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
#if (OTA_TYPE == OTA_TYPE_BOOT)
#define DEV_SERVICES (OTA_TYPE \
		| SERVICE_THS \
		| SERVICE_KEY \
)
#else
#define DEV_SERVICES (OTA_TYPE \
		| SERVICE_THS \
		| SERVICE_KEY \
		| SERVICE_HISTORY \
)
#endif

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

#elif DEVICE == DEVICE_TH05
/* Model: TH05 */
#if (OTA_TYPE == OTA_TYPE_BOOT)
#define DEV_SERVICES (OTA_TYPE \
		| SERVICE_SCREEN \
		| SERVICE_THS \
		| SERVICE_KEY \
)
#else
#define DEV_SERVICES (OTA_TYPE \
		| SERVICE_SCREEN \
		| SERVICE_THS \
		| SERVICE_KEY \
		| SERVICE_HISTORY \
)
#endif

#define ADC_PIN_USE_OUT		1	// hal_gpio_write(ADC_PIN, 1);
#define ADC_PIN 	GPIO_P11
#define ADC_CHL 	ADC_CH1N_P11

#define USE_TH_SENSOR	1
#define USE_RS_SENSOR	0
#define USE_SECREEN		1

#define I2C_SDA 	GPIO_P33 // AHT20_SDA
#define I2C_SCL 	GPIO_P34 // AHT20_SCL
#define GPIO_SPWR	GPIO_P00 // питание сенсора CHT8305_VDD
#define GPIO_KEY	GPIO_P14

#define GPIO_LPWR	GPIO_P02 // питание LCD драйвера
//#define GPIO_LED	GPIO_P20
//#define LED_ON		1
//#define LED_OFF		0

#define DEF_MODEL_NUMBER_STR		"TH05"
#define DEF_HARDWARE_REVISION		"0001"
#define DEF_MANUFACTURE_NAME_STR	"Tuya"

#else
#error "DEVICE Not released!"
#endif

#define BOOT_MODE_SELECT_REG 0x4000f034 // == 0x55 -> OTA

// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL		24 // 12 -> 15 ms
// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL		24 // 30 ms
// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY			0 // (29+1)*30 = 900 ms
// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT			400 // 4s

typedef struct _cfg_t {
	uint32_t flg;

	uint8_t rf_tx_power; // 0..0x3F
	uint8_t advertising_interval; // multiply by 62.5 for value in ms (1..160,  62.5 ms .. 10 sec)
	uint8_t connect_latency; // +1 x 0.03 sec ( = connection interval), Tmin = 1*30 = 30 ms, Tmax = 256 * 30 = 7680 ms
	uint8_t reserved1;

	uint8_t measure_interval; // measure TH sensor count * advertising_interval
	uint8_t batt_interval; // measure battery * seconds
	uint8_t averaging_measurements; // * measure_interval, 0 - off, 1..255 * measure_interval
	uint8_t reserved2;

} cfg_t;

extern cfg_t cfg;
extern const cfg_t def_cfg;

typedef struct _adv_work_t {
	uint32_t	measure_interval_ms;
	uint32_t	measure_batt_tik;
	uint8_t		adv_count;
	uint8_t 	adv_con_count;
	uint8_t		adv_batt;
} adv_work_t;

extern adv_work_t adv_wrk;

// uint32_t rtc_get_counter(void); // tik 32768
#if 1
#define clock_time_rtc() rtc_get_counter()
#else
inline uint32 clock_time_rtc(void) { return AP_AON->RTCCNT; } // (*(volatile unsigned int*)0x4000f028); }// & 0xffffff; // max 512 sec
#endif
// uint32_t get_delta_time_rtc(uint32_t start_time_rtc);

typedef struct _clock_time_t {
	uint32_t utc_time_sec; // utc, sec 01 01 1970
	uint32_t utc_time_add; // add
	uint32_t utc_time_tik; // old rtc tik, in 32768 Hz
	uint32_t utc_set_time_sec; // время установки utc_time_sec
#if (DEV_SERVICES & SERVICE_TIME_ADJUST)
	int32_t delta_time; // коррекция времени rtc
#endif
} clock_time_t;
extern clock_time_t clkt;

uint32_t get_utc_time_sec(void);
void restore_utc_time_sec(void);

void test_config(void);
void load_eep_config(void);

#endif /* SOURCE_CONFIG_H_ */
