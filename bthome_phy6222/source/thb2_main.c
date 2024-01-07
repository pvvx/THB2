/**************************************************************************************************
*******
**************************************************************************************************/

/**************************************************************************************************
  Filename:       simpleBLEPeripheral.c
  Revised:        
  Revision:       

  Description:    This file contains the Simple BLE Peripheral sample application
                  

**************************************************************************************************/
/*********************************************************************
 * INCLUDES
 */
#include "bcomdef.h"
#include "rf_phy_driver.h"
#include "global_config.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "gatt.h"
#include "hci.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "sbp_profile_ota.h"
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
#include "fs.h"
#include "battservice.h"
#include "thservice.h"
#include "thb2_peripheral.h"
#include "bthome_beacon.h"
#include "sensor.h"
#include "battery.h"
/*********************************************************************
 * MACROS
 */
//#define LOG(...)  
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
volatile uint8_t g_current_advType = LL_ADV_CONNECTABLE_UNDIRECTED_EVT;


/*********************************************************************
 * EXTERNAL FUNCTIONS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
uint8	simpleBLEPeripheral_TaskID;   // Task ID for internal task/event processing

static	gaprole_States_t	gapProfileState	=	GAPROLE_INIT;


/** Advertisement payload */
static const uint8 advertData[] = 
{
    0x02,   // length of this data
    GAP_ADTYPE_FLAGS,
    GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED
};

// GAP GATT Attributes
static uint8 attDeviceName[] = "THB2-000000"; // GAP_DEVICE_NAME_LEN

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 scanRspData[GAP_DEVICE_NAME_LEN + 2];


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

static void set_mac(void)
{
		extern uint8 ownPublicAddr[LL_DEVICE_ADDR_LEN];
		uint8 * p = &attDeviceName[5];
#if 1 // =0 - test!	
		uint16 len;
		if(hal_fs_item_read(0xACAD, ownPublicAddr, LL_DEVICE_ADDR_LEN, &len) != PPlus_SUCCESS) {
			LL_Rand(ownPublicAddr,3);
			ownPublicAddr[3] = 0x8d;
			ownPublicAddr[4] = 0x1f;
			ownPublicAddr[5] = 0x38;
			hal_fs_item_write(0xACAD, ownPublicAddr, LL_DEVICE_ADDR_LEN);
		}
#else
			ownPublicAddr[0] = 0x56;
			ownPublicAddr[1] = 0x34;
			ownPublicAddr[2] = 0x12;
			ownPublicAddr[3] = 0x34;
			ownPublicAddr[4] = 0x12;
			ownPublicAddr[5] = 0x25;
#endif	

		p = str_bin2hex(p, &ownPublicAddr[2], 1); 
		p = str_bin2hex(p, &ownPublicAddr[1], 1); 
		str_bin2hex(p, &ownPublicAddr[0], 1);
		scanRspData[0] = sizeof(attDeviceName) + 1;
		scanRspData[1] = GAP_ADTYPE_LOCAL_NAME_COMPLETE;
		osal_memcpy(&scanRspData[2], attDeviceName, sizeof(attDeviceName));
}


uint8 adv_count;
uint8 adv_con_count;
/*
extern uint8  gapRole_AdvEnabled;
extern uint8  gapRole_AdvertData[B_MAX_ADV_LEN];
extern uint8  gapRole_AdvDirectAddr[B_ADDR_LEN];
extern uint8  gapRole_AdvEventType;
extern uint8  gapRole_AdvDirectType;
extern uint8  gapRole_AdvChanMap;
extern uint8  gapRole_AdvFilterPolicy;
extern uint8 gapRole_TaskID;
extern gaprole_States_t gapRole_state;
#ifndef START_ADVERTISING_EVT
#define START_ADVERTISING_EVT 1
#endif
*/

extern gapPeriConnectParams_t periConnParameters;

// Set new advertising interval
static void set_adv_interval(uint16 advInt)
{
		GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, advInt );
		GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, advInt );
		GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, advInt );
		GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, advInt );
	  GAP_EndDiscoverable( gapRole_TaskID );
	  gapRole_state = GAPROLE_WAITING_AFTER_TIMEOUT;
/*	
 	  LL_SetAdvParam(advInt, advInt, // actual time = advInt * 625us 
			LL_ADV_CONNECTABLE_UNDIRECTED_EVT,
      gapRole_AdvEventType,
		  gapRole_AdvDirectType,
      gapRole_AdvDirectAddr,
      gapRole_AdvChanMap,
      gapRole_AdvFilterPolicy ); */
    // Turn advertising back on.
    osal_set_event( gapRole_TaskID, START_ADVERTISING_EVT );	
}

static void adv_measure(void) {
	if(gapRole_AdvEnabled) {
		if(++adv_count & 1) {
			read_sensor();
			bthome_data_beacon((padv_bthome_ns1_t) gapRole_AdvertData);
			LL_SetAdvData(sizeof(adv_bthome_ns1_t), gapRole_AdvertData);
			if(adv_con_count) {
				if(--adv_con_count == 0) {
					set_adv_interval(DEF_ADV_INERVAL); 
				}
			}
		} 
		else if(adv_count >= BATT_TIMER_MEASURE_INTERVAL/DEF_ADV_INERVAL_MS) // = 60 
		{
				adv_count = 0;
				batt_start_measure();
		}
	}
}

static void posedge_int_wakeup_cb(GPIO_Pin_e pin,IO_Wakeup_Pol_e type)
{
	(void) pin;
	if(type == POSEDGE)
	{
		adv_con_count = 30000/DEF_CON_ADV_INERVAL_MS; // 60 sec
		LOG("int or wakeup(pos):gpio:%d type:%d\n",pin,type);
		hal_gpio_write(GPIO_LED,1);
		if(gapRole_AdvEnabled) {
			set_adv_interval(DEF_CON_ADV_INERVAL); // actual time = advInt * 625us
		}
	}
	else
	{
		LOG("error\n");
	}
}
static void negedge_int_wakeup_cb(GPIO_Pin_e pin,IO_Wakeup_Pol_e type)
{
	(void) pin;
	if(type == NEGEDGE)
	{
		LOG("int or wakeup(neg):gpio:%d type:%d\n",pin,type);
		hal_gpio_write(GPIO_LED,0);
	}
	else
	{
		LOG("error\n");
	}
}
static void init_led_key(void)
{
	//hal_gpio_pin_init(GPIO_KEY, GPIO_INPUT);
	hal_gpioin_register(GPIO_KEY, posedge_int_wakeup_cb, negedge_int_wakeup_cb); 
	hal_gpioretention_register(GPIO_LED);//enable this pin retention
	//hal_gpioretention_unregister(pin);//disable this pin retention
	hal_gpio_write(GPIO_LED, 1);
}

/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t simpleBLEPeripheral_PeripheralCBs =
{
    peripheralStateNotificationCB,  // Profile State Change Callbacks
    peripheralStateReadRssiCB       // When a valid RSSI is read from controller (not used by application)
};
#if (DEF_GAPBOND_MGR_ENABLE==1)
// GAP Bond Manager Callbacks, add 2017-11-15
static gapBondCBs_t simpleBLEPeripheral_BondMgrCBs =
{
	NULL,                     // Passcode callback (not used by application)
	NULL                      // Pairing / Bonding state Callback (not used by application)
};
#endif
// Simple GATT Profile Callbacks
//static 
#if 0
simpleProfileCBs_t simpleBLEPeripheral_SimpleProfileCBs =
{
    simpleProfileChangeCB    // Charactersitic value change callback
};
#endif
/*********************************************************************
 * PUBLIC FUNCTIONS
 */
extern gapPeriConnectParams_t periConnParameters;
/*********************************************************************
 * @fn      SimpleBLEPeripheral_Init
 *
 * @brief   Initialization function for the Simple BLE Peripheral App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SimpleBLEPeripheral_Init( uint8 task_id )
{
    simpleBLEPeripheral_TaskID = task_id;
		
	  init_led_key();
		
	  init_sensor();

    // Setup the GAP
    VOID GAP_SetParamValue( TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL );
  
    // Setup the GAP Peripheral Role Profile
    {
        // device starts advertising upon initialization
        uint8 initial_advertising_enable	=	FALSE;

        uint8 enable_update_request			=	DEFAULT_ENABLE_UPDATE_REQUEST;
        uint8 advChnMap		=	GAP_ADVCHAN_37 | GAP_ADVCHAN_38 | GAP_ADVCHAN_39; 
        
        // By setting this to zero, the device will go into the waiting state after
        // being discoverable for 30.72 second, and will not being advertising again
        // until the enabler is set back to TRUE
        uint16 gapRole_AdvertOffTime	=	0;
        
        uint8 peerPublicAddr[] = {
			0x01,
			0x02,
			0x03,
			0x04,
			0x05,
			0x06
		};
		set_mac();
				uint8 advType = LL_ADV_CONNECTABLE_UNDIRECTED_EVT;
        GAPRole_SetParameter( GAPROLE_ADV_EVENT_TYPE,	sizeof( uint8 ),		&advType );
				GAPRole_SetParameter( GAPROLE_ADV_DIRECT_ADDR,	sizeof(peerPublicAddr), peerPublicAddr);
        // set adv channel map
        GAPRole_SetParameter( GAPROLE_ADV_CHANNEL_MAP,	sizeof( uint8 ),		&advChnMap);        
        // Set the GAP Role Parameters
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED,	sizeof( uint8 ),		&initial_advertising_enable );
        GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME,	sizeof( uint16 ),		&gapRole_AdvertOffTime 	);
        GAPRole_SetParameter( GAPROLE_ADVERT_DATA,	sizeof(advertData), (void *)advertData); // 	advertData	
        GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, scanRspData[0] + 1, scanRspData );
        GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE,	sizeof( uint8  ),	&enable_update_request);
        GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL,	sizeof( uint16 ),	&periConnParameters.intervalMin);
        GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL,	sizeof( uint16 ),	&periConnParameters.intervalMax	);
        GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY,		sizeof( uint16 ),	&periConnParameters.latency	);
        GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER,	sizeof( uint16 ),	&periConnParameters.timeout	);
    }

	// Set the GAP Characteristics
	GGS_SetParameter( GGS_DEVICE_NAME_ATT, sizeof(attDeviceName), (void *)attDeviceName ); // GAP_DEVICE_NAME_LEN, attDeviceName );

    // Set advertising interval
    {
				uint16 advInt = DEF_ADV_INERVAL; // actual time = advInt * 625us
				GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, advInt );
				GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, advInt );
				GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, advInt );
				GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, advInt );			
    }
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
		GGS_AddService( GATT_ALL_SERVICES );            	// 	GAP
		GATTServApp_AddService( GATT_ALL_SERVICES );    	// 	GATT attributes
		DevInfo_AddService();                           	// 	Device Information Service
		//SimpleProfile_AddService( GATT_ALL_SERVICES );  	// 	Simple GATT Profile
		Batt_AddService();
		//Batt_Register(NULL);
		TH_AddService();

		//uint8	OTA_Passward_AscII[8]	=	{'1','2','3','4','5','6','7','8'};
		//ota_app_AddService_UseKey(8, OTA_Passward_AscII);
		// ota_app_AddService();

#if (1)
		llInitFeatureSet2MPHY(TRUE);
		llInitFeatureSetDLE(TRUE);
#else
		llInitFeatureSet2MPHY(FALSE);
		llInitFeatureSetDLE(FALSE);
#endif

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
 * @fn      SimpleBLEPeripheral_ProcessEvent
 *
 * @brief   Simple BLE Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16 SimpleBLEPeripheral_ProcessEvent( uint8 task_id, uint16 events )
{
    VOID task_id; // OSAL required parameter that isn't used in this function
		if ( events & ADV_BROADCAST_EVT)
		{
			  adv_measure(); 
			  LOG("advN%u\n", adv_count);
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

    // enable adv
    if ( events & SBP_RESET_ADV_EVT )
    {
			  LOG("SBP_RESET_ADV_EVT\n");
        adv_count = 0;
			
			  //adv_con_count = 1;
				// set_adv_interval(DEF_ADV_INERVAL); // actual time = advInt * 625us
			  uint8 initial_advertising_enable = TRUE;
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
        return ( events ^ SBP_RESET_ADV_EVT );
    }
    if( events & TIMER_BATT_EVT)
    {
				LOG("TIMER_EVT\n");
			  read_sensor();
				// TH Notify
				TH_NotifyLevel();
 			  if(++adv_count >= 6) { // 60 sec
					adv_count = 0;
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
				// return unprocessed events
        return ( events ^ SBP_START_DEVICE_EVT );
    }
		
	  if(events & SBP_DEALDATA)
		{
		  LOG("\ndeal app datas in events!!!\n");
			// return unprocessed events
		  return(events ^ SBP_DEALDATA);
	  }
    // Discard unknown events
    return 0;
}

/*********************************************************************
 * @fn      simpleBLEPeripheral_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
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
 * @fn      peripheralStateReadRssiCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateReadRssiCB( int8  rssi )
{
    (void)rssi;
}

/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
 static void peripheralStateNotificationCB( gaprole_States_t newState )
{
    switch ( newState )
    {
        case GAPROLE_STARTED:
        {
					// set_mac();
					LOG("Gaprole_start\n");
					osal_set_event(simpleBLEPeripheral_TaskID, SBP_RESET_ADV_EVT);                                     
        }
        break;
        
        case GAPROLE_ADVERTISING:
				{
					LOG("Gaprole_adversting\n");
					osal_stop_timerEx(simpleBLEPeripheral_TaskID, TIMER_BATT_EVT);
					adv_count = 0;
				  //bthome_data_beacon((padv_bthome_ns1_t) gapRole_AdvertData);
				  //LL_SetAdvData(sizeof(adv_bthome_ns1_t), gapRole_AdvertData);						
				}   
        break;
        
        case GAPROLE_CONNECTED:
					  adv_con_count = 0;
						osal_start_reload_timer(simpleBLEPeripheral_TaskID, TIMER_BATT_EVT, 2*DEF_ADV_INERVAL_MS);
            HCI_PPLUS_ConnEventDoneNoticeCmd(simpleBLEPeripheral_TaskID, NULL);
            LOG("Gaprole_Connected\n");
        break;
        
        case GAPROLE_CONNECTED_ADV:
					  
	      break;      
				
        case GAPROLE_WAITING:
        	LOG("Gaprole_Disconnection\n");
				  adv_con_count = 1;
				  osal_stop_timerEx(simpleBLEPeripheral_TaskID, TIMER_BATT_EVT);
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

#if 0
/*********************************************************************
 * @fn      simpleProfileChangeCB
 *
 * @brief   Callback from SimpleBLEProfile indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void simpleProfileChangeCB( uint8 paramID )
{
    
	switch( paramID )
	{
		case SIMPLEPROFILE_CHAR1:
 
		break;

		default:
			// not process other attribute change
		break;
	}
}
#endif

/*********************************************************************
*********************************************************************/
