/**************************************************************************************************
	Filename:		OSAL_SimpleBLEPeripheral.c
	Revised:
	Revision:
	Description:	This file contains function that allows user setup tasks
**************************************************************************************************/

/**************************************************************************************************
											  INCLUDES
 **************************************************************************************************/
#include "config.h"
//#if (APP_CFG == 0)
#include "OSAL.h"
#include "OSAL_Tasks.h"

/* LL */
#include "ll.h"

/* HCI */
#include "hci_tl.h"

#if defined ( OSAL_CBTIMER_NUM_TASKS )
	#include "osal_cbtimer.h"
#endif

/* L2CAP */
#include "l2cap.h"

/* gap */
#include "gap.h"
#include "gapgattserver.h"
#include "gapbondmgr.h"

/* GATT */
#include "gatt.h"

#include "gattservapp.h"

/* Profiles */
#include "thb2_peripheral.h"

/* Application */
#include "thb2_main.h"
#include "halperipheral.h"

/*********************************************************************
	GLOBAL VARIABLES
*/

/*
extern l2capSegmentBuff_t   l2capSegmentPkt[MAX_NUM_LL_CONN];
uint8_t notify_enable_TaskID;
extern uint16 gapRole_ConnectionHandle;
// add event free buf
uint16 _HCI_ProcessEvent( uint8 task_id, uint16 events ) {
	uint16 ret = HCI_ProcessEvent(task_id, events);
	if(ret == 0) {
		if(notify_enable_TaskID) {
			//&& l2capSegmentPkt[gapRole_ConnectionHandle].fragment
			//&& l2capSegmentPkt[gapRole_ConnectionHandle].depth < 3) {
			osal_set_event(notify_enable_TaskID, WRK_NOTIFY_EVT);
		}
	}
	return ret;
}
*/

// The order in this table must be identical to the task initialization calls below in osalInitTask.
const pTaskEventHandlerFn tasksArr[] =
{
	LL_ProcessEvent,												// task 0
	HCI_ProcessEvent,												// task 1
#if defined ( OSAL_CBTIMER_NUM_TASKS )
	OSAL_CBTIMER_PROCESS_EVENT( osal_CbTimerProcessEvent ),			// task 3
#endif
	L2CAP_ProcessEvent,												// task 2
	SM_ProcessEvent,												// task 3
	GAP_ProcessEvent,												// task 4
	GATT_ProcessEvent,												// task 5
	GAPRole_ProcessEvent,											// task 6
#if (DEF_GAPBOND_MGR_ENABLE==1)
	GAPBondMgr_ProcessEvent,										// task , add 2017-11-15
#endif
	GATTServApp_ProcessEvent,										// task 7
	BLEPeripheral_ProcessEvent,										// task 8

};

const uint8 tasksCnt = sizeof( tasksArr ) / sizeof( tasksArr[0] );
uint16* tasksEvents;

/*********************************************************************
	FUNCTIONS
 *********************************************************************/
/*********************************************************************
	@fn		 osalInitTasks

	@brief	 This function invokes the initialization function for each task.

	@param	 void

	@return	 none
*/
void osalInitTasks( void )
{
	uint8 taskID = 0;
	tasksEvents = (uint16*)osal_mem_alloc( sizeof( uint16 ) * tasksCnt);
	memset( tasksEvents, 0, (sizeof( uint16 ) * tasksCnt));
	/* LL Task */
	LL_Init( taskID++ );
	/* HCI Task */
	HCI_Init( taskID++ );
#if defined ( OSAL_CBTIMER_NUM_TASKS )
	/* Callback Timer Tasks */
	osal_CbTimerInit( taskID );
	taskID += OSAL_CBTIMER_NUM_TASKS;
#endif
	/* L2CAP Task */
	L2CAP_Init( taskID++ );
	/* SM Task */
	SM_Init( taskID++ );
	/* GAP Task */
	GAP_Init( taskID++ );
	/* GATT Task */
	GATT_Init( taskID++ );
	/* Profiles */
	GAPRole_Init( taskID++ );
#if(DEF_GAPBOND_MGR_ENABLE==1)
	GAPBondMgr_Init( taskID++ );		  // 2017-11-15
#endif
	GATTServApp_Init( taskID++ );
	/* Application */
	SimpleBLEPeripheral_Init( taskID++ );
}
//#endif // (APP_CFG == 0)

