/**************************************************************************************************
  Filename:		  simpleBLEPeripheral.c
  Revised:
  Revision:

  Description:	  This file contains the Simple BLE Peripheral sample application


**************************************************************************************************/
/*********************************************************************
 * INCLUDES
 */
#include "bcomdef.h"
#include "config.h"
#include "rf_phy_driver.h"
#include "global_config.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "gatt.h"
#include "hci.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "ota_app_service.h"
#include "thb2_peripheral.h"
#include "gapbondmgr.h"
#include "pwrmgr.h"
#include "gpio.h"
#include "bleperipheral.h"
#include "ll.h"
#include "ll_hw_drv.h"
#include "ll_def.h"
#include "hci_tl.h"
#include "flash.h"
//#include "fs.h"
#include "flash_eep.h"
#include "battservice.h"
#include "thservice.h"
#include "thb2_peripheral.h"
#include "bthome_beacon.h"
#include "sensor.h"
#include "battery.h"
#include "sbp_profile.h"
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#define INVALID_CONNHANDLE						0xFFFF
// Default passcode
#define DEFAULT_PASSCODE						0 //19655
// Length of bd addr as a string
#define B_ADDR_STR_LEN							15
#define RESOLVING_LIST_ENTRY_NUM				10
// Offset of advertData&scanRspData
#define	RSP_OFFSET_MAC							4

/*********************************************************************
 * build define
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
perStatsByChan_t g_perStatsByChanTest;

/*********************************************************************
 * EXTERNAL VARIABLES
 */
//volatile uint8_t g_current_advType = LL_ADV_CONNECTABLE_UNDIRECTED_EVT;


/*********************************************************************
 * EXTERNAL FUNCTIONS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */

adv_work_t adv_wrk;

uint8	simpleBLEPeripheral_TaskID;	  // Task ID for internal task/event processing

static	gaprole_States_t	gapProfileState	=	GAPROLE_INIT;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void simpleBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void peripheralStateNotificationCB( gaprole_States_t newState );
//static void simpleProfileChangeCB( uint8 paramID );
static void peripheralStateReadRssiCB( int8 rssi  );

const char* hex_ascii = { "0123456789ABCDEF" };
uint8_t * str_bin2hex(uint8_t *d, uint8_t *s, int len) {
	while(len--) {
		*d++ = hex_ascii[(*s >> 4) & 0xf];
		*d++ = hex_ascii[(*s++ >> 0) & 0xf];
	}
	return d;
}

// GAP - SCAN RSP data (max size = 31 bytes)
static void set_def_name(uint8_t * mac)
{
	uint8 * p = gapRole_ScanRspData;
	gapRole_ScanRspDataLen = sizeof(DEF_MODEL_NUMBER_STR) + 8;
	*p++ = sizeof(DEF_MODEL_NUMBER_STR) + 7;
	*p++ = GAP_ADTYPE_LOCAL_NAME_COMPLETE;
	osal_memcpy(p, devInfoModelNumber, sizeof(DEF_MODEL_NUMBER_STR)-1);
	p += sizeof(DEF_MODEL_NUMBER_STR) - 1;
	*p++ = '-';
	p = str_bin2hex(p, mac+2, 1);
	p = str_bin2hex(p, mac+1, 1);
	str_bin2hex(p, mac, 1);
}

static void set_mac(void)
{
	extern uint8 ownPublicAddr[LL_DEVICE_ADDR_LEN];
	if (read_chip_mAddr(ownPublicAddr) != CHIP_ID_VALID) {
		if(flash_read_cfg(ownPublicAddr, EEP_ID_MAC, LL_DEVICE_ADDR_LEN) != LL_DEVICE_ADDR_LEN) {
			LL_Rand(ownPublicAddr,3);
			// Tuya mac[0:3]
			ownPublicAddr[3] = 0x8d;
			ownPublicAddr[4] = 0x1f;
			ownPublicAddr[5] = 0x38;
			flash_write_cfg(ownPublicAddr, EEP_ID_MAC, LL_DEVICE_ADDR_LEN);
		}
	}
	set_def_name(ownPublicAddr);
}

static void set_serial_number(void)
{
	hal_get_flash_info();
	uint8_t *p = str_bin2hex(devInfoSerialNumber, (uint8_t *)&phy_flash.IdentificationID, 3);
	*p++ = '-';
	p = str_bin2hex(p, (uint8_t *)&thsensor_cfg.mid, 4);
	*p++ = '-';
	*p++ = '0';
	*p = '0';
}

extern gapPeriConnectParams_t periConnParameters;
extern uint16 gapParameters[];
static void set_adv_interval(uint16 advInt);

// Set new advertising interval
static void set_new_adv_interval(uint16 advInt)
{
	set_adv_interval(advInt);
	GAP_EndDiscoverable( gapRole_TaskID );
	gapRole_state = GAPROLE_WAITING_AFTER_TIMEOUT;
	// Turn advertising back on.
	osal_set_event( gapRole_TaskID, START_ADVERTISING_EVT );
}
// Set advertising interval
static void set_adv_interval(uint16 advInt)
{
#ifdef __GCC
	gapParameters[TGAP_LIM_DISC_ADV_INT_MIN] = advInt;
	gapParameters[TGAP_LIM_DISC_ADV_INT_MAX] = advInt;
	gapParameters[TGAP_GEN_DISC_ADV_INT_MIN] = advInt;
	gapParameters[TGAP_GEN_DISC_ADV_INT_MAX] = advInt;
#else
	GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, advInt );
	GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, advInt );
	GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, advInt );
	GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, advInt );
#endif
}

static void adv_measure(void) {
	if(gapRole_AdvEnabled) {
		if(++adv_wrk.adv_count >= cfg.measure_interval) {
			adv_wrk.adv_count = 0;
			read_sensor();
			if(++adv_wrk.adv_batt_count >= cfg.batt_interval) { // = 60
				adv_wrk.adv_batt_count = 0;
				batt_start_measure();
			}
			bthome_data_beacon((padv_bthome_ns1_t) gapRole_AdvertData);
			LL_SetAdvData(sizeof(adv_bthome_ns1_t), gapRole_AdvertData);
		}
		if(adv_wrk.adv_con_count) {
			if(--adv_wrk.adv_con_count == 0) {
				set_new_adv_interval(cfg.advertising_interval * 100);
			}
		}
	}
}

/*********************************************************************
 * LED and Key
 */
static void posedge_int_wakeup_cb(GPIO_Pin_e pin, IO_Wakeup_Pol_e type)
{
	(void) pin;
	if(type == POSEDGE)
	{
		adv_wrk.adv_con_count = 30000/DEF_CON_ADV_INERVAL_MS; // 60 sec
		LOG("int or wakeup(pos):gpio:%d type:%d\n", pin, type);
		hal_gpio_write(GPIO_LED, LED_OFF);
		if(gapRole_AdvEnabled) {
			set_new_adv_interval(DEF_CON_ADV_INERVAL); // actual time = advInt * 625us
		}
	}
	else
	{
		LOG("error\n");
	}
}

static void negedge_int_wakeup_cb(GPIO_Pin_e pin, IO_Wakeup_Pol_e type)
{
	(void) pin;
	if(type == NEGEDGE)
	{
		LOG("int or wakeup(neg):gpio:%d type:%d\n", pin, type);
		hal_gpio_write(GPIO_LED, LED_ON);
	}
	else
	{
		LOG("error\n");
	}
}

void init_led_key(void)
{
	hal_gpioin_register(GPIO_KEY, posedge_int_wakeup_cb, negedge_int_wakeup_cb);
	hal_gpioretention_register(GPIO_LED);//enable this pin retention
	hal_gpio_write(GPIO_LED, LED_ON);
#if DEVICE == DEVICE_BTH01
	hal_gpioretention_register(GPIO_SPWR);//enable this pin retention
	hal_gpio_write(GPIO_SPWR, 1);
#endif
}

/*********************************************************************
 * GAPROLE ADVERTISING
 */
void gatrole_advert_enable(bool enable) {
	uint8 oldAdvEnabled = gapRole_AdvEnabled;
	gapRole_AdvEnabled = enable;

	if ( (oldAdvEnabled) && (gapRole_AdvEnabled == FALSE) )
	{
		// Turn off Advertising
		if ( ( gapRole_state == GAPROLE_ADVERTISING )
				|| ( gapRole_state == GAPROLE_CONNECTED_ADV )
				|| ( gapRole_state == GAPROLE_WAITING_AFTER_TIMEOUT ) )
		{
			GAP_EndDiscoverable( gapRole_TaskID );
		}
	}
	else if ( (oldAdvEnabled == FALSE) && (gapRole_AdvEnabled) )
	{
		// Turn on Advertising
		if ( (gapRole_state == GAPROLE_STARTED)
				|| (gapRole_state == GAPROLE_WAITING)
				|| (gapRole_state == GAPROLE_CONNECTED)
				|| (gapRole_state == GAPROLE_WAITING_AFTER_TIMEOUT) )
		{
			osal_set_event( gapRole_TaskID, START_ADVERTISING_EVT );
		}
	}
}

/*********************************************************************
 * PROFILE CALLBACKS
 */
// GAP Role Callbacks
static gapRolesCBs_t simpleBLEPeripheral_PeripheralCBs =
{
	peripheralStateNotificationCB,	// Profile State Change Callbacks
	peripheralStateReadRssiCB		// When a valid RSSI is read from controller (not used by application)
};
#if (DEF_GAPBOND_MGR_ENABLE==1)
// GAP Bond Manager Callbacks, add 2017-11-15
static gapBondCBs_t simpleBLEPeripheral_BondMgrCBs =
{
	NULL,					  // Passcode callback (not used by application)
	NULL					  // Pairing / Bonding state Callback (not used by application)
};
#endif
/*********************************************************************
 * PUBLIC FUNCTIONS
 */
/*********************************************************************
 * @fn		SimpleBLEPeripheral_Init
 *
 * @brief	Initialization function for the Simple BLE Peripheral App Task.
 *			This is called during initialization and should contain
 *			any application specific initialization (ie. hardware
 *			initialization/setup, table initialization, power up
 *			notificaiton ... ).
 *
 * @param	task_id - the ID assigned by OSAL.	This ID should be
 *					  used to send messages and set timers.
 *
 * @return	none
 */
void SimpleBLEPeripheral_Init( uint8 task_id )
{
	simpleBLEPeripheral_TaskID = task_id;

	init_led_key();

	init_sensor();

	set_serial_number();

	// Setup the GAP
#ifdef __GCC
	gapParameters[TGAP_CONN_PAUSE_PERIPHERAL] = DEFAULT_CONN_PAUSE_PERIPHERAL;
#else
	GAP_SetParamValue( TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL );
#endif

	// Setup the GAP Peripheral Role Profile
	{

		set_mac();
		// gapRole_AdvEventType = LL_ADV_CONNECTABLE_UNDIRECTED_EVT; // already set default
		// gapRole_AdvDirectAddr[B_ADDR_LEN] = {1,2,3,4,5,6}; // already set default
		// gapRole_AdvChanMap = GAP_ADVCHAN_37 | GAP_ADVCHAN_38 | GAP_ADVCHAN_39; // already set default
		// Set the GAP Role Parameters
		// device starts advertising upon initialization
		gatrole_advert_enable(FALSE);
		// gapRole_AdvertOffTime = 0; // already set default
		GAP_UpdateAdvertisingData( gapRole_TaskID, TRUE, gapRole_AdvertDataLen, gapRole_AdvertData );
		GAP_UpdateAdvertisingData( gapRole_TaskID, FALSE, gapRole_ScanRspDataLen, gapRole_ScanRspData );
		gapRole_ParamUpdateEnable = DEFAULT_ENABLE_UPDATE_REQUEST;

		/*  already set default -> config.h
		// extern gapPeriConnectParams_t periConnParameters;
		gapRole_MinConnInterval = periConnParameters.intervalMin;
		gapRole_MaxConnInterval = periConnParameters.intervalMax;
		gapRole_SlaveLatency = periConnParameters.latency;
		gapRole_TimeoutMultiplier = periConnParameters.timeout;
		*/
	}

	// Set the GAP Characteristics
	GGS_SetParameter( GGS_DEVICE_NAME_ATT, gapRole_ScanRspData[0] - 1, (void *)&gapRole_ScanRspData[2] ); // GAP_DEVICE_NAME_LEN, attDeviceName );

	// Set advertising interval
	set_adv_interval(DEF_ADV_INERVAL); // actual time = advInt * 625us

	HCI_PPLUS_AdvEventDoneNoticeCmd(simpleBLEPeripheral_TaskID, ADV_BROADCAST_EVT);
#if (DEF_GAPBOND_MGR_ENABLE==1)
	// Setup the GAP Bond Manager, add 2017-11-15
	{
		uint32 passkey = DEFAULT_PASSCODE;
		uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
		uint8 mitm = TRUE;
		uint8 ioCap = GAPBOND_IO_CAP_NO_INPUT_NO_OUTPUT;
		uint8 bonding = TRUE;
		GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
		GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
		GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
		GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
		GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
	}
#endif
	// Initialize GATT attributes
	GGS_AddService( GATT_ALL_SERVICES );				//	GAP
	GATTServApp_AddService( GATT_ALL_SERVICES );		//	GATT attributes
	DevInfo_AddService();								//	Device Information Service
	Batt_AddService();
	TH_AddService();
	//Batt_Register(NULL);
	SimpleProfile_AddService( GATT_ALL_SERVICES );		//	Simple GATT Profile

	//uint8	OTA_Passward_AscII[8]	=	{'1','2','3','4','5','6','7','8'};
	//ota_app_AddService_UseKey(8, OTA_Passward_AscII);
	// ota_app_AddService();

#if (1)
#if 0 // CODED PHY not work?
	deviceFeatureSet.featureSet[1] |= (uint8)(
			LL_FEATURE_2M_PHY
			| LL_FEATURE_CODED_PHY
			| LL_FEATURE_CSA2);
   // CSA2 feature setting
	pGlobal_config[LL_SWITCH] |= CONN_CSA2_ALLOW;
	llInitFeatureSetCodedPHY(TRUE);
#endif
//	llInitFeatureSet2MPHY(TRUE);
	llInitFeatureSetDLE(TRUE);
#else
	llInitFeatureSet2MPHY(FALSE);
	llInitFeatureSetDLE(FALSE);
#endif
//    HCI_LE_SetDefaultPhyMode(0,0xff,0x01,0x01);

#ifdef MTU_SIZE
	ATT_SetMTUSizeMax(MTU_SIZE);
#else
	ATT_SetMTUSizeMax(23);
#endif
	// Setup a delayed profile startup
	osal_set_event( simpleBLEPeripheral_TaskID, SBP_START_DEVICE_EVT );
	// for receive HCI complete message
	GAP_RegisterForHCIMsgs(simpleBLEPeripheral_TaskID);
	LL_PLUS_PerStats_Init(&g_perStatsByChanTest);
	batt_start_measure();

	LOG("=====SimpleBLEPeripheral_Init Done=======\n");
}

/*********************************************************************
 * @fn		BLEPeripheral_ProcessEvent
 *
 * @brief	Simple BLE Peripheral Application Task event processor.	 This function
 *			is called to process all events for the task.  Events
 *			include timers, messages and any other user defined events.
 *
 * @param	task_id	 - The OSAL assigned task ID.
 * @param	events - events to process.	 This is a bit map and can
 *					 contain more than one event.
 *
 * @return	events not processed
 */
uint16 BLEPeripheral_ProcessEvent( uint8 task_id, uint16 events )
{
	VOID task_id; // OSAL required parameter that isn't used in this function
	if ( events & ADV_BROADCAST_EVT)
	{
		adv_measure();
		LOG("advN%u\n", adv_wrk.adv_count);
		// return unprocessed events
		return (events ^ ADV_BROADCAST_EVT);
	}

	if ( events & SYS_EVENT_MSG )
	{
		uint8 *pMsg;

		if ( (pMsg = osal_msg_receive( simpleBLEPeripheral_TaskID )) != NULL )
		{
			simpleBLEPeripheral_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );
			// Release the OSAL message
			VOID osal_msg_deallocate( pMsg );
		}

		// return unprocessed events
		return (events ^ SYS_EVENT_MSG);
	}

	// enable adv (from gaprole start)
	if ( events & SBP_RESET_ADV_EVT )
	{
		LOG("SBP_RESET_ADV_EVT\n");
		adv_wrk.adv_count = 0;
		// set_new_adv_interval(DEF_ADV_INERVAL); // actual time = advInt * 625us
		gatrole_advert_enable(TRUE);
		return ( events ^ SBP_RESET_ADV_EVT );
	}
	if( events & TIMER_BATT_EVT)
	{
		LOG("TIMER_EVT\n");
		read_sensor();
		// TH Notify
		TH_NotifyLevel();
		if(++adv_wrk.adv_batt_count >= cfg.batt_interval) { // 60 sec
			adv_wrk.adv_batt_count = 0;
			batt_start_measure();
		}
		// return unprocessed events
		return ( events ^ TIMER_BATT_EVT);
	}
	if( events & BATT_VALUE_EVT)
	{
		LOG("Vbat: %d mV, %d %%\n", measured_data.battery_mv, measured_data.battery);
		// Batt Notify
		if(!gapRole_AdvEnabled) {
			BattNotifyLevel();
		}
		// return unprocessed events
		return ( events ^ BATT_VALUE_EVT);
	}
	if ( events & SBP_START_DEVICE_EVT )
	{
		// Start the Device
		VOID GAPRole_StartDevice( &simpleBLEPeripheral_PeripheralCBs );
#if (DEF_GAPBOND_MGR_ENABLE==1)
		// Start Bond Manager, 2017-11-15
		VOID GAPBondMgr_Register( &simpleBLEPeripheral_BondMgrCBs );
#endif
		HCI_LE_ReadResolvingListSizeCmd();
		hal_gpio_write(GPIO_LED, LED_OFF);
		// return unprocessed events
		return ( events ^ SBP_START_DEVICE_EVT );
	}
	if(events & SBP_CMDDATA)
	{
		LOG("CMD data events\n");
		new_cmd_data();
		// return unprocessed events
		return(events ^ SBP_CMDDATA);
	}
#if OTA_TYPE
	if(events & SBP_OTADATA)
	{
		LOG("OTA data events\n");
		new_ota_data();
		// return unprocessed events
		return(events ^ SBP_OTADATA);
	}
#endif
	// Discard unknown events
	return 0;
}

/*********************************************************************
 * @fn		simpleBLEPeripheral_ProcessOSALMsg
 *
 * @brief	Process an incoming task message.
 *
 * @param	pMsg - message to process
 *
 * @return	none
 */
static void simpleBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
#if DEBUG_INFO
	hciEvt_CmdComplete_t *pHciMsg;
#endif
	switch ( pMsg->event ){
		case HCI_GAP_EVENT_EVENT:{
			switch( pMsg->status ){

				case HCI_COMMAND_COMPLETE_EVENT_CODE:
#if DEBUG_INFO
					pHciMsg = (hciEvt_CmdComplete_t *)pMsg;
					LOG("==> HCI_COMMAND_COMPLETE_EVENT_CODE: %x\n", pHciMsg->cmdOpcode);
					//safeToDealloc = gapProcessHCICmdCompleteEvt( (hciEvt_CmdComplete_t *)pMsg );
#endif
				break;

				default:
					//safeToDealloc = FALSE;  // Send to app
				break;
			}
		}
	}
}
/*********************************************************************
 * @fn		peripheralStateReadRssiCB
 *
 * @brief	Notification from the profile of a state change.
 *
 * @param	newState - new state
 *
 * @return	none
 */
static void peripheralStateReadRssiCB( int8	 rssi )
{
	(void)rssi;
}

/*********************************************************************
 * @fn		peripheralStateNotificationCB
 *
 * @brief	Notification from the profile of a state change.
 *
 * @param	newState - new state
 *
 * @return	none
 */
 static void peripheralStateNotificationCB( gaprole_States_t newState )
{
	switch ( newState )
	{
		case GAPROLE_STARTED:
		{
			LOG("Gaprole_start\n");
			osal_set_event(simpleBLEPeripheral_TaskID, SBP_RESET_ADV_EVT);
		}
		break;

		case GAPROLE_ADVERTISING:
		{
			LOG("Gaprole_adversting\n");
			osal_stop_timerEx(simpleBLEPeripheral_TaskID, TIMER_BATT_EVT);
			adv_wrk.adv_count = 0;
		}
		break;

		case GAPROLE_CONNECTED:
			adv_wrk.adv_count = 0;
			adv_wrk.adv_con_count = 0;
			osal_start_reload_timer(simpleBLEPeripheral_TaskID, TIMER_BATT_EVT, adv_wrk.measure_interval_ms); // 10000 ms
			HCI_PPLUS_ConnEventDoneNoticeCmd(simpleBLEPeripheral_TaskID, NULL);
			LOG("Gaprole_Connected\n");
		break;

		case GAPROLE_CONNECTED_ADV:

		break;

		case GAPROLE_WAITING:
			LOG("Gaprole_Disconnection\n");
			adv_wrk.adv_con_count = 1;
			osal_stop_timerEx(simpleBLEPeripheral_TaskID, TIMER_BATT_EVT);
			adv_wrk.adv_count = 0;
		break;

		case GAPROLE_WAITING_AFTER_TIMEOUT:
			LOG("Gaprole_waitting_after_timerout\n");
		break;

		case GAPROLE_ERROR:
			LOG("Gaprole error!\n");
		break;

		default:
		break;
	}
	gapProfileState = newState;
	LOG("[GAP ROLE %d]\n",newState);

	VOID gapProfileState;
}

