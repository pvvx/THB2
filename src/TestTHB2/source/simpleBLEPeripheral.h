/**************************************************************************************************
  Filename:       simpleBLEperipheral.h
  Revised:         
  Revision:        

  Description:    This file contains the Simple BLE Peripheral sample application
                  definitions and prototypes.

 
**************************************************************************************************/

#ifndef SIMPLEBLEPERIPHERAL_H
#define SIMPLEBLEPERIPHERAL_H

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
// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL		24 // 12 -> 15 ms 
// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL		24 // 30 ms
// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY				29
// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT				400 // 4s

#define DEF_ADV_INERVAL  					8000 // = 5 sec, actual time = advInt * 625us
#define DEF_ADV_INERVAL_MS 				((DEF_ADV_INERVAL*625)/1000) // 5000 ms
#define DEF_CON_ADV_INERVAL 			2500 // 1.5625 sec
#define DEF_CON_ADV_INERVAL_MS 		((DEF_CON_ADV_INERVAL*625)/1000) // 1562 ms
// How often to perform periodic event
#define SBP_PERIODIC_EVT_PERIOD		5000

#define DEVINFO_SYSTEM_ID_LEN			8
#define DEVINFO_SYSTEM_ID					0
 
#define DEFAULT_DISCOVERABLE_MODE					GAP_ADTYPE_FLAGS_GENERAL

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST			TRUE
// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL			2

// Simple BLE Peripheral Task Events
#define SBP_START_DEVICE_EVT					0x0001
#define SBP_RESET_ADV_EVT							0x0002
#define	SBP_DEALDATA									0x0004
#define TIMER_BATT_EVT                0x0008  //for battery detect
#define BATT_VALUE_EVT                0x0010  //event for battery voltage value update
#define ADV_BROADCAST_EVT							0x0020

/*********************************************************************
 * MACROS
 */
#define MAC_DATA_LEN							6

#define GPIO_KEY									P7
#define GPIO_LED									P26

extern	uint8	simpleBLEPeripheral_TaskID;
/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the BLE Application
 */
extern void SimpleBLEPeripheral_Init( uint8 task_id );

/*
 * Task Event Processor for the BLE Application
 */
extern uint16 SimpleBLEPeripheral_ProcessEvent( uint8 task_id, uint16 events );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEBLEPERIPHERAL_H */
