/**************************************************************************************************
  Filename:      thb2_main.h
  Revised:
  Revision:

  Description:    This file contains the Simple BLE Peripheral sample application
                  definitions and prototypes.

**************************************************************************************************/

#ifndef _THB2_MAIN_H_
#define _THB2_MAIN_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */

#define DEF_ADV_INERVAL  			8000 // = 5 sec, actual time = advInt * 625us
#define DEF_ADV_INERVAL_MS 			((DEF_ADV_INERVAL*625)/1000) // 5000 ms
#define DEF_CON_ADV_INERVAL 		2500 // 1.5625 sec
#define DEF_CON_ADV_INERVAL_MS 		((DEF_CON_ADV_INERVAL*625)/1000) // 1562 ms
#define DEF_EVENT_ADV_INERVAL 		80 // 50 ms
#define DEF_EVENT_ADV_INERVAL_MS 	((DEF_EVENT_ADV_INERVAL*625)/1000) // 50 ms
#define DEF_OTA_ADV_INERVAL 		1600 // 1 sec
#define DEF_OTA_ADV_INERVAL_MS 		((DEF_OTA_ADV_INERVAL*625)/1000) // 1000 ms

// adv. event
#define RDS_STEP_TIMER_SEC		1800 // шаг передачи 30 минут
#define RDS_RETRY_DOUBLE_SEC	(RDS_STEP_TIMER_SEC-12) // дубль через 12 сек
#define RDS_RETRY_START_SEC		(RDS_STEP_TIMER_SEC-120) // старт 2 минуты
#define RDS_RETRY_ADV_COUNT		16

// How often to perform periodic event
#define SBP_PERIODIC_EVT_PERIOD		5000

#define DEVINFO_SYSTEM_ID_LEN		8
#define DEVINFO_SYSTEM_ID			0

#define DEFAULT_DISCOVERABLE_MODE	GAP_ADTYPE_FLAGS_GENERAL

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST			TRUE
// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL			2

// Simple BLE Peripheral Task Events
#define SBP_START_DEVICE_EVT		0x0001  // start
#define SBP_RESET_ADV_EVT			0x0002  // enable adv (from gaprole_start)
#define	SBP_CMDDATA					0x0004  // receive command data
#define TIMER_BATT_EVT				0x0008  // for battery detect
#define BATT_VALUE_EVT				0x0010  // Event for battery voltage value update
#define ADV_BROADCAST_EVT			0x0020  // Advent. Event Done Notice
#define	WRK_NOTIFY_EVT				0x0040  // work notify
#define	PIN_INPUT_EVT				0x0080  // pin input event

/*********************************************************************
 * MACROS
 */
// #define MAC_DATA_LEN				6

/*********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
*********************************************************************/

#define MAC_LEN		6
extern uint8 ownPublicAddr[MAC_LEN];
extern uint8_t	simpleBLEPeripheral_TaskID;	  // Task ID for internal task/event processing

void SimpleBLEPeripheral_Init( uint8_t task_id );

uint16_t BLEPeripheral_ProcessEvent( uint8_t task_id, uint16_t events );

void set_def_name(void);

void set_dev_name(void);

#ifdef __cplusplus
}

#endif

#endif /* _THB2_MAIN_H_ */
