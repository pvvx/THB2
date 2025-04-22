/*
 * config.h
 *
 *  Created on: 11/01/2024
 *      Author: pvvx
 */

#ifndef SOURCE_CONFIG_H_
#define SOURCE_CONFIG_H_

#include <string.h>
#include "types.h"
// #include "bus_dev.h"

#ifndef APP_VERSION
#define APP_VERSION	0x21	// BCD
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
#define BOARD_TS0202_TZ3000			18
#define BOARD_TH03Z   				22 // ZigBee TH03Z
#define BOARD_ZTH01   				27 // ZigBee ZTH01
#define BOARD_ZTH02   				28 // ZigBee ZTH02
#define BOARD_PLM1 					29 // Tuya BLE Plant monitor ECF-SGS01-A rev1.3 (BT3L Tuya module)  (not yet published at the moment)
#define BOARD_ZTH03   				30 // ZigBee TH03 TS0201(_TZ3000_gohcgzj7, _TZ3000_bguser20) LCD
#define BOARD_LKTMZL02				31 // ZigBee LKTMZL02 LCD
#define BOARD_ZTH05					33 // Tuya ZigBee TS0601_TZE204
#define BOARD_CB3S					35  // development is not completed!Tuya ZigBee "Smart Button" TS0041_TZ3000_fa9mlvja
#define BOARD_HS09					36  // development is not completed! TS0201_TZ3000_1twfmkcc: Tuya ZigBee "Smart Humidity Sensor"
#define BOARD_ZYZTH02				37 // Tuya ZY-ZTH02 Zigbee, 2 x AAA, SHT30/CHT832x
#define BOARD_ZYZTH01				38  // Tuya ZY-ZTH02Pro Zigbee LCD, 2 x AAA, SHT30/CHT832x
#define BOARD_ZG_227Z				39  // Zigbee ZG-227Z
#define BOARD_TS0202_PIR1 			40 // Tuya TS0202_TZ3000_6ygjfyll PIR
#define BOARD_TS0202_PIR2 			41 // Tuya TS0202_TZ3040_bb6xaihh PIR
*/
#define DEVICE_THB2		19
#define DEVICE_BTH01	20
#define DEVICE_TH05		21	// TH05_V1.4..1.6
#define DEVICE_THB1		23
#define DEVICE_TH05D	24  // TH05_V1.3
#define DEVICE_TH05F	25  // TH05Y_V1.2
#define DEVICE_THB3		26
#define DEVICE_KEY2		32
#define DEVICE_TH04 	34
#define DEVICE_HDP16 42 // MiniTag-F2-YY-v2, Model: HD-P16, MiLi MiTag F2, SoC: ST17F65

#ifndef DEVICE
#define DEVICE		DEVICE_TH05F
#endif

// supported services by the device (bits)
#define SERVICE_OTA			0x00000001	// есть функция OTA
//#define SERVICE_OTA_EXT		0x00000002	// пока нет // есть расширенная функция OTA
//#define SERVICE_PINCODE 	0x00000004	// пока нет // есть установка pin-code
#define SERVICE_BINDKEY 	0x00000008	// есть шифрование
#define SERVICE_HISTORY 	0x00000010	// есть запись истории
#define SERVICE_SCREEN		0x00000020	// есть экран
//#define SERVICE_LE_LR		0x00000040	// пока нет // Есть поддержка рекламы в LE Long Range
#define SERVICE_THS			0x00000080	// есть датчик температуры и влажности
#define SERVICE_RDS			0x00000100	// есть обслуживние геркона/счета импульсов
#define SERVICE_KEY			0x00000200	// есть кнопка
//#define SERVICE_OUTS		0x00000400	// пока нет // есть обслуживние выходных пинов
//#define SERVICE_INS			0x00000800	// пока нет // есть обслуживние входных пинов
//#define SERVICE_TIME_ADJUST 0x00001000	// пока нет // есть функция коррекции счета времени
//#define SERVICE_HARD_CLOCK	0x00002000	// пока нет // есть реальные часы RTC
#define SERVICE_TH_TRG		0x00004000	// триггер по температуре и влажности
#define SERVICE_LED			0x00008000	// use led
//#define SERVICE_MI_KEYS	0x00010000	// use mi keys (не используется)
//#define SERVICE_PRESSURE	0x00020000	// pressure sensor (пока нет реализации)
//#define SERVICE_18B20		0x00040000	// use sensor(s) MY18B20 (пока нет реализации)
//#define SERVICE_IUS		0x00080000	// use I and U sensor (INA226) (пока нет реализации)
//#define SERVICE_PLM		0x00100000	// use PWM-RH and NTC (пока нет реализации)
#define SERVICE_BUTTON		0x00200000	// брелок-кнопка
#define SERVICE_FINDMY		0x00400000	// FindMy
//#define SERVICE_SCANTIM		0x00800000	// Scan Time (develop, test only!)
//#define SERVICE_ZIGBEE		0x01000000	// BZ-version
//#define SERVICE_PIR			0x02000000	// use PIR sensor
#define SERVICE_EXTENDED	0x80000000  //

#define OTA_TYPE_NONE	0	// нет OTA, только переключение из APP на boot прошивку
#define OTA_TYPE_BOOT	SERVICE_OTA		// вариант для прошивки boot + OTA
#define OTA_TYPE_APP	OTA_TYPE_NONE

#ifndef OTA_TYPE
#define OTA_TYPE	OTA_TYPE_APP
#endif

#if OTA_TYPE == OTA_TYPE_BOOT
#define DEF_SOFTWARE_REVISION	{'B', '0'+ (APP_VERSION >> 4), '.' , '0'+ (APP_VERSION & 0x0F), 0}
#else
#define DEF_SOFTWARE_REVISION	{'V', '0'+ (APP_VERSION >> 4), '.' , '0'+ (APP_VERSION & 0x0F), 0}
#endif

#if DEVICE == DEVICE_THB2
/* Model: THB2 */
#if OTA_TYPE == OTA_TYPE_BOOT
#define DEV_SERVICES (OTA_TYPE \
		| SERVICE_THS \
		| SERVICE_KEY \
)
#else
#define DEV_SERVICES (OTA_TYPE \
		| SERVICE_THS \
		| SERVICE_KEY \
		| SERVICE_HISTORY \
		| SERVICE_TH_TRG \
		| SERVICE_RDS \
		| SERVICE_BINDKEY \
)
#endif

#define PERIOD_BEACON_EVENT 120

#define ADC_PIN_USE_OUT		0 //  есть резистор 0 к +Vbat
#define ADC_PIN				GPIO_P11
#define ADC_VBAT_CHL		VBAT_ADC_P11

#define I2C_SDA 	GPIO_P18
#define I2C_SCL 	GPIO_P20
#define GPIO_KEY	GPIO_P07
#define KEY_PRESSED	0
#define GPIO_LED	GPIO_P26
#define LED_ON		0
#define LED_OFF		1

#define GPIO_TRG	GPIO_P09 // TX
#define GPIO_INP	GPIO_P10 // RX

#define DEF_MODEL_NUMBER_STR		"THB2"
#define DEF_HARDWARE_REVISION		"0013"
#define DEF_MANUFACTURE_NAME_STR	"Tuya"

#elif DEVICE == DEVICE_BTH01
/* Model: BTH01 */
#if OTA_TYPE == OTA_TYPE_BOOT
#define DEV_SERVICES (OTA_TYPE \
		| SERVICE_THS \
		| SERVICE_KEY \
)
#else
#define DEV_SERVICES (OTA_TYPE \
		| SERVICE_THS \
		| SERVICE_KEY \
		| SERVICE_HISTORY \
		| SERVICE_TH_TRG \
		| SERVICE_RDS \
		| SERVICE_BINDKEY \
)
#endif

#define ADC_PIN_USE_OUT		1	// hal_gpio_write(ADC_PIN, 1);
#define ADC_PIN				GPIO_P11
#define ADC_VBAT_CHL		VBAT_ADC_P11

#define I2C_SDA 	GPIO_P33 // SDA
#define I2C_SCL 	GPIO_P34 // SCL
#define GPIO_SPWR	GPIO_P00 // питание сенсора
#define GPIO_KEY	GPIO_P14
#define KEY_PRESSED	0
#define GPIO_LED	GPIO_P15
#define LED_ON		1
#define LED_OFF		0

#define GPIO_TRG	GPIO_P20 // mark TX2
#define GPIO_INP	GPIO_P18 // mark RX2

#define DEF_MODEL_NUMBER_STR		"BTH01"
#define DEF_HARDWARE_REVISION		"0014"
#define DEF_MANUFACTURE_NAME_STR	"Tuya"

#elif DEVICE == DEVICE_TH05
/* Model: TH05 v1.3*/
#if OTA_TYPE == OTA_TYPE_BOOT
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
		| SERVICE_TH_TRG \
		| SERVICE_RDS \
		| SERVICE_BINDKEY \
)
#endif

#define ADC_PIN_USE_OUT		1	// hal_gpio_write(ADC_PIN, 1);
#define ADC_PIN				GPIO_P11
#define ADC_VBAT_CHL		VBAT_ADC_P11

#define USE_TH_SENSOR	1
#define USE_SECREEN		1

#define I2C_SDA 	GPIO_P33 // SDA
#define I2C_SCL 	GPIO_P34 // SCL

#define I2C_LCD_SDA GPIO_P33 // SDA
#define I2C_LCD_SCL GPIO_P34 // SCL

#define GPIO_SPWR	GPIO_P00 // питание сенсора
#define GPIO_KEY	GPIO_P14
#define KEY_PRESSED	0
#define GPIO_LPWR	GPIO_P02 // питание LCD драйвера

#define GPIO_TRG	GPIO_P20 // mark TX2
#define GPIO_INP	GPIO_P18 // mark RX2

//#define GPIO_LED	GPIO_P20
//#define LED_ON		1
//#define LED_OFF		0

#define DEF_MODEL_NUMBER_STR		"TH05"
#define DEF_HARDWARE_REVISION		"0015"
#define DEF_MANUFACTURE_NAME_STR	"Tuya"

#elif (DEVICE == DEVICE_THB1) || (DEVICE == DEVICE_THB3)
/* Model: THB1 */
#if OTA_TYPE == OTA_TYPE_BOOT
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
		| SERVICE_TH_TRG \
		| SERVICE_RDS \
		| SERVICE_BINDKEY \
)
#endif

#define ADC_PIN_USE_OUT		1	// нет подключения к +Vbat
#define ADC_PIN				GPIO_P14
#define ADC_VBAT_CHL		VBAT_ADC_P14

#define USE_TH_SENSOR	1
#define USE_SECREEN		1

#define I2C_SDA 	GPIO_P18 // SDA
#define I2C_SCL 	GPIO_P20 // SCL

#define I2C_LCD_SDA GPIO_P34 // SDA
#define I2C_LCD_SCL GPIO_P33 // SCL

#define GPIO_KEY	GPIO_P01
#define KEY_PRESSED	0

#define GPIO_TRG	GPIO_P09 // mark TX
#define GPIO_INP	GPIO_P10 // mark RX

#if DEVICE == DEVICE_THB1
#define DEF_MODEL_NUMBER_STR		"THB1"
#define DEF_HARDWARE_REVISION		"0017"
#else
#define DEF_MODEL_NUMBER_STR		"THB3"
#define DEF_HARDWARE_REVISION		"001A"
#endif
#define DEF_MANUFACTURE_NAME_STR	"Tuya"

#elif DEVICE == DEVICE_TH05D
/* Model: TH05 v1.3 */
#if OTA_TYPE == OTA_TYPE_BOOT
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
		| SERVICE_TH_TRG \
		| SERVICE_RDS \
		| SERVICE_BINDKEY \
)
#endif

//#define GPIO_LED	GPIO_P00 // не припаян
//#define LED_ON		1
//#define LED_OFF		0

#define ADC_PIN_USE_OUT		1	// нет подключения к +Vbat
#define ADC_PIN				GPIO_P15
#define ADC_VBAT_CHL		VBAT_ADC_P15

#define USE_TH_SENSOR	1
#define USE_SECREEN		1

#define I2C_SDA 	GPIO_P31 // SDA
#define I2C_SCL 	GPIO_P32 // SCL

#define I2C_LCD_SDA GPIO_P11 // SDA
#define I2C_LCD_SCL GPIO_P14 // SCL

#define GPIO_KEY	GPIO_P02
#define KEY_PRESSED	0

#define GPIO_TRG	GPIO_P09 // mark TX
#define GPIO_INP	GPIO_P10 // mark RX

#define DEF_MODEL_NUMBER_STR		"TH05D"
#define DEF_HARDWARE_REVISION		"0018"
#define DEF_MANUFACTURE_NAME_STR	"Tuya"

#elif DEVICE == DEVICE_TH05F
/* Model: TH05Y_V1.2/1.2 */
#if OTA_TYPE == OTA_TYPE_BOOT
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
		| SERVICE_TH_TRG \
		| SERVICE_RDS \
		| SERVICE_BINDKEY \
)
#endif

//#define GPIO_LED	GPIO_P15 // не припаян
//#define LED_ON		1
//#define LED_OFF		0

#define ADC_PIN_USE_OUT		1	// нет подключения к +Vbat
#define ADC_PIN				GPIO_P11
#define ADC_VBAT_CHL		VBAT_ADC_P11

#define USE_TH_SENSOR	1
#define USE_SECREEN		1

#define I2C_SDA 	GPIO_P33 // SDA
#define I2C_SCL 	GPIO_P34 // SCL

#define I2C_LCD_SDA GPIO_P26 // SDA
#define I2C_LCD_SCL GPIO_P31 // SCL

#define GPIO_SPWR	GPIO_P00 // питание сенсора
#define GPIO_KEY	GPIO_P14
#define KEY_PRESSED	0
#define GPIO_LPWR	GPIO_P02 // питание LCD драйвера

#define GPIO_TRG	GPIO_P20 // mark TX2
#define GPIO_INP	GPIO_P18 // mark RX2

#define DEF_MODEL_NUMBER_STR		"TH05F"
#define DEF_HARDWARE_REVISION		"0019"
#define DEF_MANUFACTURE_NAME_STR	"Tuya"

#elif DEVICE == DEVICE_KEY2
/* Model: iSearch ver2 ST17H66 (TSSOP16) */
#define SDK_VER_CHIP  __DEF_CHIP_TSOP16__

#if OTA_TYPE == OTA_TYPE_BOOT
#define DEV_SERVICES (OTA_TYPE \
		| SERVICE_BUTTON \
		| SERVICE_FINDMY \
		| SERVICE_BINDKEY \
)
#else
#define DEV_SERVICES (OTA_TYPE \
		| SERVICE_BUTTON \
		| SERVICE_FINDMY \
		| SERVICE_BINDKEY \
)
#endif

#define ADC_PIN_USE_OUT		1	// hal_gpio_write(ADC_PIN, 1);
#define ADC_PIN				GPIO_P11
#define ADC_VBAT_CHL		VBAT_ADC_P11

#define GPIO_KEY	GPIO_P15
#define KEY_PRESSED	1
#define GPIO_LED	GPIO_P03
#define LED_ON		1
#define LED_OFF		0

#define GPIO_BUZZER	   GPIO_P09
#define PWM_CHL_BUZZER PWM_CH0
#define BUZZER_ON	1
#define BUZZER_OFF	0

//#define GPIO_INP	GPIO_P15

#define DEF_MODEL_NUMBER_STR		"KEY"
#define DEF_HARDWARE_REVISION		"0020"
#define DEF_MANUFACTURE_NAME_STR	"DIY"

#elif DEVICE == DEVICE_HDP16

#if OTA_TYPE == OTA_TYPE_BOOT
#define DEV_SERVICES (OTA_TYPE \
		| SERVICE_BUTTON \
		| SERVICE_FINDMY \
		| SERVICE_BINDKEY \
)
#else
#define DEV_SERVICES (OTA_TYPE \
		| SERVICE_BUTTON \
		| SERVICE_FINDMY \
		| SERVICE_BINDKEY \
)
#endif

#define ADC_PIN_USE_OUT		1	// hal_gpio_write(ADC_PIN, 1);
#define ADC_PIN				GPIO_P11
#define ADC_VBAT_CHL		VBAT_ADC_P11

#define GPIO_KEY	GPIO_P14
#define KEY_PRESSED	0
//#define GPIO_LED	GPIO_P03
//#define LED_ON	1
//#define LED_OFF	0

#define GPIO_BUZZER	GPIO_P34
#define PWM_CHL_BUZZER	PWM_CH0
#define BUZZER_ON	1
#define BUZZER_OFF	0

//#define GPIO_INP     GPIO_P15

#define DEF_MODEL_NUMBER_STR           "HDP16"
#define DEF_HARDWARE_REVISION          "0020"
#define DEF_MANUFACTURE_NAME_STR       "MiLi"


#elif DEVICE == DEVICE_TH04

#if OTA_TYPE == OTA_TYPE_BOOT
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
		| SERVICE_TH_TRG \
		| SERVICE_RDS \
		| SERVICE_BINDKEY \
)
#endif

// TODO: I still need to figure this out
#define ADC_PIN_USE_OUT		1
#define ADC_PIN				GPIO_P11
#define ADC_VBAT_CHL		VBAT_ADC_P11

#define GPIO_KEY	GPIO_P24
#define KEY_PRESSED	0

#define I2C_SDA 	GPIO_P07 // SDA
#define I2C_SCL 	GPIO_P03 // SCL

#define GPIO_TRG	GPIO_P09 // mark TX
#define GPIO_INP	GPIO_P10 // mark RX

#define DEF_MODEL_NUMBER_STR		"TH04"
#define DEF_HARDWARE_REVISION		"0022" 
#define DEF_MANUFACTURE_NAME_STR	"Tuya" 

#else
#error "DEVICE Not released!"
#endif

#if ((DEV_SERVICES & SERVICE_THS) == 0) && (DEV_SERVICES & SERVICE_TH_TRG)
#error "Not SERVICE_TH_TRG!"
#endif


// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL		24 // 12 -> 15 ms
// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL		24 // 30 ms
// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY			29 // (29+1)*30 = 900 ms, max MAX_SLAVE_LATENCY 500
// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT			400 // 4s

// For FLG_DISPLAY_SLEEP
#define DISPLAY_SLEEP_DELAY						60 	// Seconds after which display is switched off
#define DELAY_BEFORE_UNIT_CHANGE				3 	// Long key press: Seconds after which temp unit is changed
#define DELAY_BEFORE_RESET						10	// Long key press: Seconds after which reset is triggered

typedef struct _cfg_t {
	uint32_t flg;

	uint8_t rf_tx_power; // 0..0x3F
	uint8_t advertising_interval; // multiply by 62.5 for value in ms (1..160,  62.5 ms .. 10 sec)
	uint8_t connect_latency; // +1 x 0.03 sec ( = connection interval), Tmin = 1*30 = 30 ms, Tmax = 256 * 30 = 7680 ms
	uint8_t adv_event_cnt;

	uint8_t measure_interval; // measure TH sensor count * advertising_interval
	uint8_t batt_interval; // measure battery * seconds
	uint8_t averaging_measurements; // * measure_interval, 0 - off, 1..255 * measure_interval
	uint8_t reserved2;
}cfg_t;
extern cfg_t cfg;
extern const cfg_t def_cfg;

#define FLG_MEAS_NOTIFY		0x00000001	// Enable sending notifications for measurements
#define FLG_SHOW_TIME		0x00000002	// Enable clock display on LCD
#define FLG_SHOW_SMILEY		0x00000004	// Enable smiley on display
#define FLG_SHOW_TRG		0x00000008	// Smiley shows TRG status
#define FLG_DISPLAY_OFF		0x00000010	// Turn off display
#define FLG_ADV_CRYPT		0x00000020	// Encrypted BLE advertising (bindkey)
#define FLG_SHOW_TF			0x00000040	// Show temperature in F.
#define FLG_FINDMY			0x00000080	// FindMy
#define FLG_DISPLAY_SLEEP	0x00000100  // Switch off display after 60 seconds, Re-enable after key press.

typedef struct _adv_work_t {
	uint32_t	measure_interval_ms;
	uint32_t	measure_batt_tik;
#if (DEV_SERVICES & (SERVICE_RDS | SERVICE_BUTTON))
	uint32_t	rds_timer_tik;	// rds & button time tik (in 32768 Hz)
	uint32_t	rds_count;		// rds & button count
#endif
	uint8_t		meas_count;		// счет до нового измерения в кол-ве вызовов adv_measure()
	uint8_t 	adv_reload_count; // кол-во передач рекламы до перехода к новому типу и установкам рекламы
	uint8_t		new_battery; 	// flag: new battery
	uint8_t		adv_event; 		// flag: rds event
} adv_work_t;
extern adv_work_t adv_wrk;

#define OTA_MODE_SELECT_REG 0x4000f034
//#define OTA_MODE_SELECT_REG (AP_AON->RTCCC2) // [0x4000f034] == 0x55 -> OTA
#define BOOT_FLG_OTA	0x55 // перезагрузка в FW Boot для OTA (ожидание соединения 80 сек)
#define BOOT_FLG_FW0	0x33 // перезагрузка в FW Boot

// States for work_parm_t.long_press_state
#define LONG_PRESS_NONE					0	// Default state, key not pressed
#define LONG_PRESS_BEFORE_UNIT_CHANGE 	1	// Key just pressed, advertising frequency will be temporarily increased when the key is released in this state
#define LONG_PRESS_UNIT_CHANGED			2	// Unit changed on display, Config is saved when key is released in this state.
#define LONG_PRESS_DO_RESET				3	// Display shows "rESEt" and the device is rebooted when releasing the key

typedef struct _work_parm_t {
#if (DEV_SERVICES & SERVICE_SCREEN)
	uint8_t lcd_clock: 1;	 // If FLG_SHOW_TIME is enabled: Toggle between clock and temp/humidity.
	uint8_t lcd_sleeping: 1; // If 1, display is currently in sleep mode (off), but can be re-enabled by pressing the key.
#endif
#if (DEV_SERVICES & SERVICE_KEY)
	uint8_t long_press_state; // see LONG_PRESS_...
#endif
//	uint8_t lcd_ext_chow; // показ TH/Clock отключен
	uint8_t reboot; // reboot on disconnect, записывается в [OTA_MODE_SELECT_REG]
	uint8_t boot_flg; // байт из [OTA_MODE_SELECT_REG]
} work_parm_t;
extern work_parm_t wrk;

// uint32_t rtc_get_counter(void); // tik 32768
#if 1
#define clock_time_rtc() rtc_get_counter()
#else
inline uint32 clock_time_rtc(void) {
	uint32_t tick;
	do
		tick = *(volatile uint32_t*) 0x4000f028;          // read current RTC counter (AP_AON->RTCCNT)
	while(tick != *(volatile uint32_t*) 0x4000f028;) // AP_AON->RTCCNT
	return tick; } // (*(volatile unsigned int*)0x4000f028); }// & 0xffffff; // max 512 sec
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
void save_config(void);

#endif /* SOURCE_CONFIG_H_ */
