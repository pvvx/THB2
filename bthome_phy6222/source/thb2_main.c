/*******************************************************************************
  Filename:		  simpleBLEPeripheral.c
  Revised:
  Revision:

  Description:  This file contains the Simple BLE Peripheral sample application


*******************************************************************************/
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
#include "sensors.h"
#include "battery.h"
#include "sbp_profile.h"
#include "ble_ota.h"
#include "lcd_th05.h"
#include "logger.h"
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
static void peripheralStateReadRssiCB( int8 rssi );

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
	// TODO: pGlobal_config[MAC_ADDRESS_LOC]
}

static void set_serial_number(void)
{
	hal_get_flash_info();
	uint8_t *p = str_bin2hex(devInfoSerialNumber, (uint8_t *)&phy_flash.IdentificationID, 3);
	*p++ = '-';
#if (DEV_SERVICES & SERVICE_THS)
	p = str_bin2hex(p, (uint8_t *)&thsensor_cfg.mid, 4);
#else
	p = str_bin2hex(p, (uint8_t *)FLASH_BASE_ADDR, 4);
#endif
	*p++ = '-';
#if (DEV_SERVICES & SERVICE_LCD)
	p = str_bin2hex(p, (uint8_t *)&lcd_i2c_addr, 1);
#else
	*p++ = '0';
	*p = '0';
#endif
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
#if defined (__GNUC__)
	gapParameters[TGAP_LIM_DISC_ADV_INT_MIN] = advInt;
	gapParameters[TGAP_LIM_DISC_ADV_INT_MAX] = advInt + 10;
	gapParameters[TGAP_GEN_DISC_ADV_INT_MIN] = advInt;
	gapParameters[TGAP_GEN_DISC_ADV_INT_MAX] = advInt + 10;
#else
	GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, advInt );
	GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, advInt  + 10);
	GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, advInt );
	GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, advInt  + 10);
#endif
}

//extern void start_measure(void);

static void adv_measure(void) {
	if(gapRole_AdvEnabled) {
		uint32_t tmp = get_utc_time_sec();
		if(tmp - adv_wrk.measure_batt_tik >= cfg.batt_interval) {
			adv_wrk.measure_batt_tik = tmp;
			batt_start_measure();
#if ((DEV_SERVICES & SERVICE_THS) == 0)
			adv_wrk.adv_batt = 1;
		} else {
			if(adv_wrk.adv_batt) {
			adv_wrk.adv_batt = 0;
#if (DEV_SERVICES & SERVICE_SCREEN)
				show_lcd(1);
#endif
#if (DEV_SERVICES & SERVICE_HISTORY)
				if (cfg.averaging_measurements != 0)
					write_memo();
#endif

			LL_SetAdvData(bthome_data_beacon((void *) gapRole_AdvertData), gapRole_AdvertData);
#if (DEV_SERVICES & SERVICE_SCREEN)
			} else {
				show_lcd(0);
#endif
			}
#endif
		}
#if (DEV_SERVICES & SERVICE_THS)
		if(adv_wrk.adv_count == (uint8_t)(cfg.measure_interval - 1)) {
			start_measure();
#if (DEV_SERVICES & SERVICE_SCREEN)
			show_lcd(0);
#endif
		} else {
			if(adv_wrk.adv_count >= cfg.measure_interval) {
			adv_wrk.adv_count = 0;
			read_sensor();
#if (DEV_SERVICES & SERVICE_SCREEN)
				show_lcd(1);
#endif
#if (DEV_SERVICES & SERVICE_HISTORY)
				if (cfg.averaging_measurements != 0)
					write_memo();
#endif
			LL_SetAdvData(bthome_data_beacon((void *) gapRole_AdvertData), gapRole_AdvertData);
#if (DEV_SERVICES & SERVICE_SCREEN)
			} else {
				show_lcd(0);
#endif
			}
		}

#endif	// (DEV_SERVICES & SERVICE_THS)
		if(adv_wrk.adv_con_count) {
			if(--adv_wrk.adv_con_count == 0) {
				set_new_adv_interval(cfg.advertising_interval * 100);
			}
		}
		adv_wrk.adv_count++;
	}
}

#if (DEV_SERVICES & SERVICE_KEY)
/*********************************************************************
 * LED and Key
 */
static void posedge_int_wakeup_cb(GPIO_Pin_e pin, IO_Wakeup_Pol_e type)
{
	(void) pin;
	if(type == POSEDGE)
	{
		LOG("int or wakeup(pos):gpio:%d type:%d\n", pin, type);
#ifdef GPIO_LED
		hal_gpio_write(GPIO_LED, LED_OFF);
#endif
		if(gapRole_AdvEnabled) {
			adv_wrk.adv_con_count = 60000/DEF_CON_ADV_INERVAL_MS; // 60 sec
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
#ifdef GPIO_LED
		hal_gpio_write(GPIO_LED, LED_ON);
#endif
	}
	else
	{
		LOG("error\n");
	}
}
#endif // (DEV_SERVICES & SERVICE_KEY)

static void init_app_gpio(void)
{
#if (DEV_SERVICES & SERVICE_KEY)
	hal_gpioin_register(GPIO_KEY, posedge_int_wakeup_cb, negedge_int_wakeup_cb);
#endif
#ifdef GPIO_LED
	hal_gpio_write(GPIO_LED, LED_ON);
	hal_gpioretention_register(GPIO_LED);//enable this pin retention
#endif
#ifdef GPIO_LPWR // питание LCD драйвера
	hal_gpio_write(GPIO_LPWR, 1);
	hal_gpioretention_register(GPIO_LPWR);//enable this pin retention
#endif
#ifdef GPIO_SPWR  // питание сенсора CHT8305_VDD
	hal_gpio_write(GPIO_SPWR, 1);
	hal_gpioretention_register(GPIO_SPWR);//enable this pin retention
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

	init_app_gpio();

#if (DEV_SERVICES & SERVICE_SCREEN)
	init_lcd();
#endif
#if (DEV_SERVICES & SERVICE_THS)
	init_sensor();
#endif
	set_serial_number();

	// Setup the GAP
#if defined (__GNUC__)
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
#if (OTA_TYPE == OTA_TYPE_BOOT)
	if (read_reg(BOOT_MODE_SELECT_REG) == 0x55) {
		write_reg(BOOT_MODE_SELECT_REG, 0);
		adv_wrk.adv_con_count = 60000/DEF_OTA_ADV_INERVAL_MS; // 60 sec
		set_new_adv_interval(DEF_CON_ADV_INERVAL); // actual time = advInt * 625us
	} else
#endif
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
#if (DEV_SERVICES & SERVICE_THS)
	TH_AddService();
#endif
	SimpleProfile_AddService( GATT_ALL_SERVICES );		//	Simple GATT Profile

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
	//llInitFeatureSet2MPHY(TRUE);
	llInitFeatureSetDLE(TRUE);
#else
	llInitFeatureSet2MPHY(FALSE);
	llInitFeatureSetDLE(FALSE);
#endif
//    HCI_LE_SetDefaultPhyMode(0,0xff,0x01,0x01);

#ifdef MTU_SIZE
#if MTU_SIZE > ATT_MAX_MTU_SIZE
#error "MTU_SIZE > 517"
#endif
	ATT_SetMTUSizeMax(MTU_SIZE);
#else
	ATT_SetMTUSizeMax(ATT_MTU_SIZE_MIN);
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
	if ( events & ADV_BROADCAST_EVT) {
		adv_measure();
		LOG("advN%u\n", adv_wrk.adv_count);
		// return unprocessed events
		return (events ^ ADV_BROADCAST_EVT);
	}

	if ( events & SYS_EVENT_MSG ) {
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
	if ( events & SBP_RESET_ADV_EVT ) {
		LOG("SBP_RESET_ADV_EVT\n");
		adv_wrk.adv_count = 0;
		// set_new_adv_interval(DEF_ADV_INERVAL); // actual time = advInt * 625us
		gatrole_advert_enable(TRUE);
		return ( events ^ SBP_RESET_ADV_EVT );
	}
	if( events & TIMER_BATT_EVT) {
		LOG("TIMER_EVT\n");
#if (DEV_SERVICES & SERVICE_THS)
		uint32_t tmp = get_utc_time_sec();
		if(tmp - adv_wrk.measure_batt_tik >= cfg.batt_interval) {
			adv_wrk.measure_batt_tik = tmp;
			batt_start_measure();
		}
		read_sensor();
		start_measure();
#if (DEV_SERVICES & SERVICE_SCREEN)
		show_lcd(1);
#endif
#if (DEV_SERVICES & SERVICE_HISTORY)
		if (cfg.averaging_measurements != 0)
			write_memo();
#endif
		// TH Notify
		TH_NotifyLevel();
#else
		get_utc_time_sec();
		batt_start_measure();
#endif // (DEV_SERVICES & SERVICE_THS)
		// return unprocessed events
		return ( events ^ TIMER_BATT_EVT);
	}
	if( events & BATT_VALUE_EVT) {
		LOG("Vbat: %d mV, %d %%\n", measured_data.battery_mv, measured_data.battery);
		// Batt Notify
		if(!gapRole_AdvEnabled) {
			BattNotifyLevel();
#if ((DEV_SERVICES & SERVICE_THS)==0)
#if (DEV_SERVICES & SERVICE_SCREEN)
			show_measure();
#endif
#endif
		}
		// return unprocessed events
		return ( events ^ BATT_VALUE_EVT);
	}
	if ( events & SBP_START_DEVICE_EVT ) {
		// Start the Device
		VOID GAPRole_StartDevice( &simpleBLEPeripheral_PeripheralCBs );
#if (DEF_GAPBOND_MGR_ENABLE==1)
		// Start Bond Manager, 2017-11-15
		VOID GAPBondMgr_Register( &simpleBLEPeripheral_BondMgrCBs );
#endif
		HCI_LE_ReadResolvingListSizeCmd();
#ifdef GPIO_LED
		hal_gpio_write(GPIO_LED, LED_OFF);
#endif
		//adv_wrk.adv_count = 0;
#if (DEV_SERVICES & SERVICE_SCREEN)
		lcd_show_version();
#endif
		// return unprocessed events
		return ( events ^ SBP_START_DEVICE_EVT );
	}
#if (DEV_SERVICES & SERVICE_HISTORY)
	if(events & WRK_NOTIFY_EVT) {
		LOG("Wrk notify events\n");
		wrk_notify();
		return(events ^ WRK_NOTIFY_EVT);
	}
#endif
	if(events & SBP_CMDDATA) {
		LOG("CMD data events\n");
		new_cmd_data();
		// return unprocessed events
		return(events ^ SBP_CMDDATA);
	}
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
#if (DEV_SERVICES & SERVICE_THS)
			osal_start_reload_timer(simpleBLEPeripheral_TaskID, TIMER_BATT_EVT, adv_wrk.measure_interval_ms); // 10000 ms
#else
			osal_start_reload_timer(simpleBLEPeripheral_TaskID, TIMER_BATT_EVT, cfg.batt_interval*1000);
#endif
			HCI_PPLUS_ConnEventDoneNoticeCmd(simpleBLEPeripheral_TaskID, 0);
			LOG("Gaprole_Connected\n");
#if (DEV_SERVICES & SERVICE_SCREEN)
			show_ble_symbol(1);
			update_lcd();
#endif
		break;

		case GAPROLE_CONNECTED_ADV:

		break;

		case GAPROLE_WAITING:
			LOG("Gaprole_Disconnection\n");
			osal_stop_timerEx(simpleBLEPeripheral_TaskID, TIMER_BATT_EVT);
			bthome_data_beacon((void *) gapRole_AdvertData);
			gapRole_SlaveLatency = periConnParameters.latency = cfg.connect_latency;
			adv_wrk.adv_count = 0;
			adv_wrk.adv_con_count = 1;
#if (DEV_SERVICES & SERVICE_SCREEN)
			show_ble_symbol(0);
			update_lcd();
#endif
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

//	VOID gapProfileState;
}
