/*
  main.c
*/

#include "bus_dev.h"
#include "config.h"
#include "gpio.h"
#include "clock.h"
#include "global_config.h"
#include "timer.h"
#include "jump_function.h"
#include "pwrmgr.h"
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "rf_phy_driver.h"
#include "flash.h"
#include "flash_eep.h"
#include "version.h"
#include "watchdog.h"
#include "adc.h"
#include "ble_ota.h"

#define DEFAULT_UART_BAUD	115200

/*********************************************************************
 LOCAL FUNCTION PROTOTYPES
 */

/*********************************************************************
 EXTERNAL FUNCTIONS
 */

extern void init_config(void);
extern void app_main(void);
extern void hal_rom_boot_init(void);
/*********************************************************************
 CONNECTION CONTEXT RELATE DEFINITION
 */

#define	  BLE_MAX_ALLOW_CONNECTION				1
#define	  BLE_MAX_ALLOW_PKT_PER_EVENT_TX		3
#define	  BLE_MAX_ALLOW_PKT_PER_EVENT_RX		3
#define	  BLE_PKT_VERSION						BLE_PKT_VERSION_5_1	 // BLE_PKT_VERSION_4_0

/*	BLE_MAX_ALLOW_PER_CONNECTION
 {
 ...
 struct ll_pkt_desc *tx_conn_desc[MAX_LL_BUF_LEN];	   // new Tx data buffer
 struct ll_pkt_desc *rx_conn_desc[MAX_LL_BUF_LEN];

 struct ll_pkt_desc *tx_not_ack_pkt;
 struct ll_pkt_desc *tx_ntrm_pkts[MAX_LL_BUF_LEN];
 ...
 }
 tx_conn_desc[] + tx_ntrm_pkts[]	--> BLE_MAX_ALLOW_PKT_PER_EVENT_TX * BLE_PKT_BUF_SIZE*2
 rx_conn_desc[]				--> BLE_MAX_ALLOW_PKT_PER_EVENT_RX * BLE_PKT_BUF_SIZE
 tx_not_ack_pkt				--> 1*BLE_PKT_BUF_SIZE

 */

#define	  BLE_PKT_BUF_SIZE					(((BLE_PKT_VERSION == BLE_PKT_VERSION_5_1) ? 1 : 0) *  BLE_PKT51_LEN \
											 + ((BLE_PKT_VERSION == BLE_PKT_VERSION_4_0) ? 1 : 0) * BLE_PKT40_LEN \
											 + (sizeof(struct ll_pkt_desc) - 2))

#define	  BLE_MAX_ALLOW_PER_CONNECTION			( (BLE_MAX_ALLOW_PKT_PER_EVENT_TX * BLE_PKT_BUF_SIZE*2) \
												  +(BLE_MAX_ALLOW_PKT_PER_EVENT_RX * BLE_PKT_BUF_SIZE)	 \
												  + BLE_PKT_BUF_SIZE )

#define	  BLE_CONN_BUF_SIZE					(BLE_MAX_ALLOW_CONNECTION * BLE_MAX_ALLOW_PER_CONNECTION)

ALIGN4_U8 g_pConnectionBuffer[BLE_CONN_BUF_SIZE];
llConnState_t pConnContext[BLE_MAX_ALLOW_CONNECTION];

/*********************************************************************
 CTE IQ SAMPLE BUF config
 */
//#define BLE_SUPPORT_CTE_IQ_SAMPLE TRUE
#ifdef BLE_SUPPORT_CTE_IQ_SAMPLE
	uint16 g_llCteSampleI[LL_CTE_MAX_SUPP_LEN * LL_CTE_SUPP_LEN_UNIT];
	uint16 g_llCteSampleQ[LL_CTE_MAX_SUPP_LEN * LL_CTE_SUPP_LEN_UNIT];
#endif

/*********************************************************************
 OSAL LARGE HEAP CONFIG
 */
#define		LARGE_HEAP_SIZE	 (4*1024)
ALIGN4_U8 g_largeHeap[LARGE_HEAP_SIZE];

#if 0 // SDK_VER_RELEASE_ID >= 0x030103 ?
#define		LL_LINKBUF_CFG_NUM	  0
#define		LL_PKT_BUFSIZE					  280
#define		LL_LINK_HEAP_SIZE	 ( ( BLE_MAX_ALLOW_CONNECTION * 3 + LL_LINKBUF_CFG_NUM ) * LL_PKT_BUFSIZE )//basic Space + configurable Space
ALIGN4_U8 g_llLinkHeap[LL_LINK_HEAP_SIZE];
#endif

/*********************************************************************
 GLOBAL VARIABLES
 */
volatile uint8 g_clk32K_config;
volatile sysclk_t g_spif_clk_config;

/*********************************************************************
 EXTERNAL VARIABLES
 */
//extern uint32_t __initial_sp;

static void hal_low_power_io_init(void) {
//========= disable all gpio pullup/down to preserve juice
const ioinit_cfg_t ioInit[] = {
#if(SDK_VER_CHIP == __DEF_CHIP_QFN32__)
#if DEVICE == DEVICE_THB2
		{ GPIO_P00, GPIO_PULL_DOWN },
		{ GPIO_P01, GPIO_PULL_DOWN },
#ifdef GPIO_TRG
		{ GPIO_P02, GPIO_FLOATING }, // TX2 - GPIO_TRG
#else
		{ GPIO_P02, GPIO_PULL_UP }, // TX2
#endif
		{ GPIO_P03, GPIO_PULL_DOWN },
		{ GPIO_P07, GPIO_PULL_UP }, // KEY
		{ GPIO_P09, GPIO_PULL_UP }, // TX
		{ GPIO_P10, GPIO_PULL_UP }, // RX - GPIO_INP
		{ GPIO_P11, GPIO_FLOATING }, // ADC_VBAT
		{ GPIO_P14, GPIO_PULL_DOWN },
		{ GPIO_P15, GPIO_PULL_DOWN },
		{ GPIO_P16, GPIO_PULL_DOWN },
		{ GPIO_P17, GPIO_PULL_DOWN },
		{ GPIO_P18, GPIO_FLOATING }, // I2C_SDA
		{ GPIO_P20, GPIO_FLOATING }, // I2C_SCL
		{ GPIO_P23, GPIO_PULL_DOWN },
		{ GPIO_P24, GPIO_PULL_DOWN },
		{ GPIO_P25, GPIO_PULL_DOWN },
#ifdef GPIO_LED
		{ GPIO_P26, GPIO_FLOATING }, // LED - GPIO_LED
#else
		{ GPIO_P26, GPIO_FLOATING }, // LED - GPIO_LED
#endif
//		{GPIO_P27, GPIO_FLOATING },
		{ GPIO_P31, GPIO_PULL_DOWN },
		{ GPIO_P32, GPIO_PULL_DOWN },
		{ GPIO_P33, GPIO_PULL_DOWN },
		{ GPIO_P34, GPIO_PULL_DOWN }
#elif (DEVICE == DEVICE_BTH01)
 		{ GPIO_P00, GPIO_PULL_UP_S },	// GPIO_SPWR Sensor Vdd
		{ GPIO_P01, GPIO_PULL_DOWN },
		{ GPIO_P02, GPIO_PULL_DOWN },
		{ GPIO_P03, GPIO_PULL_DOWN },
		{ GPIO_P07, GPIO_PULL_DOWN },
		{ GPIO_P09, GPIO_PULL_UP }, // TX1
		{ GPIO_P10, GPIO_PULL_UP }, // RX1
		{ GPIO_P11, GPIO_PULL_UP }, // ADC Vbat
		{ GPIO_P14, GPIO_PULL_UP }, // KEY
#ifdef GPIO_LED
		{ GPIO_P15, GPIO_FLOATING }, // LED - GPIO_LED
#else
		{ GPIO_P15, GPIO_PULL_DOWN },
#endif
		{ GPIO_P16, GPIO_PULL_DOWN },
		{ GPIO_P17, GPIO_PULL_DOWN },
		{ GPIO_P18, GPIO_PULL_UP }, // RX2 - GPIO_INP
#ifdef GPIO_TRG
		{ GPIO_P20, GPIO_FLOATING }, // TX2 - GPIO_TRG
#else
		{ GPIO_P20, GPIO_PULL_UP }, // TX2 - GPIO_TRG
#endif
		{ GPIO_P23, GPIO_PULL_DOWN },
		{ GPIO_P24, GPIO_PULL_DOWN },
		{ GPIO_P25, GPIO_PULL_DOWN }, // P25
		{ GPIO_P26, GPIO_PULL_DOWN },
//		{GPIO_P27, GPIO_FLOATING },
		{ GPIO_P31, GPIO_PULL_DOWN },
		{ GPIO_P32, GPIO_PULL_DOWN },
		{ GPIO_P33, GPIO_FLOATING }, // I2C_SDA
		{ GPIO_P34, GPIO_FLOATING } // // I2C_SCL
#elif (DEVICE == DEVICE_TH05)
		{ GPIO_P00, GPIO_PULL_UP_S }, // GPIO_SPWR Sensor Vdd
		{ GPIO_P01, GPIO_PULL_DOWN },
		{ GPIO_P02, GPIO_PULL_UP }, // GPIO_LPWR
		{ GPIO_P03, GPIO_PULL_DOWN },
		{ GPIO_P07, GPIO_PULL_DOWN },
		{ GPIO_P09, GPIO_PULL_UP }, // TX1
		{ GPIO_P10, GPIO_PULL_UP }, // RX1
		{ GPIO_P11, GPIO_PULL_UP }, // ADC Vbat
		{ GPIO_P14, GPIO_PULL_UP }, // KEY - GPIO_KEY
		{ GPIO_P15, GPIO_PULL_DOWN },
		{ GPIO_P16, GPIO_PULL_DOWN },
		{ GPIO_P17, GPIO_PULL_DOWN },
		{ GPIO_P18, GPIO_PULL_UP }, // RX2  - GPIO_INP
#ifdef GPIO_TRG
		{ GPIO_P20, GPIO_FLOATING }, // TX2 - GPIO_TRG
#else
		{ GPIO_P20, GPIO_PULL_UP }, // TX2 - GPIO_TRG
#endif

		{ GPIO_P23, GPIO_PULL_DOWN },
		{ GPIO_P24, GPIO_PULL_DOWN },
		{ GPIO_P25, GPIO_PULL_DOWN }, // P25
		{ GPIO_P26, GPIO_PULL_DOWN },
//		{GPIO_P27, GPIO_FLOATING },
		{ GPIO_P31, GPIO_PULL_DOWN },
		{ GPIO_P32, GPIO_PULL_DOWN },
		{ GPIO_P33, GPIO_FLOATING }, // I2C_SDA
		{ GPIO_P34, GPIO_FLOATING }  // I2C_SCL

#elif (DEVICE == DEVICE_THB1) || (DEVICE == DEVICE_THB3)
		{ GPIO_P00, GPIO_PULL_DOWN },
		{ GPIO_P01, GPIO_PULL_UP }, // KEY - GPIO_KEY
		{ GPIO_P02, GPIO_PULL_DOWN },
		{ GPIO_P03, GPIO_PULL_DOWN },
		{ GPIO_P07, GPIO_PULL_DOWN },
#ifdef GPIO_TRG
		{ GPIO_P09, GPIO_FLOATING }, // TX - GPIO_TRG
#else
		{ GPIO_P09, GPIO_PULL_UP }, // TX
#endif
		{ GPIO_P10, GPIO_PULL_UP }, // RX - GPIO_INP
		{ GPIO_P11, GPIO_FLOATING }, // ADC Vbat с делителем! Не используется - Выкусить резистор R3!
		{ GPIO_P14, GPIO_PULL_UP }, // назначен как ADC_PIN, т.к. вывод P11 подключен к делителю
		{ GPIO_P15, GPIO_PULL_DOWN },
		{ GPIO_P16, GPIO_PULL_DOWN },
		{ GPIO_P17, GPIO_PULL_DOWN },
		{ GPIO_P18, GPIO_FLOATING }, // I2C_SDA CHT8310
		{ GPIO_P20, GPIO_FLOATING }, // I2C_SCL CHT8310
		{ GPIO_P23, GPIO_PULL_DOWN },
		{ GPIO_P24, GPIO_PULL_DOWN },
		{ GPIO_P25, GPIO_PULL_DOWN },
		{ GPIO_P26, GPIO_PULL_DOWN },
//		{ GPIO_P27, GPIO_PULL_DOWN },
		{ GPIO_P31, GPIO_PULL_DOWN },
		{ GPIO_P32, GPIO_PULL_DOWN },
		{ GPIO_P33, GPIO_PULL_UP }, // I2C_SDA CNV1972
		{ GPIO_P34, GPIO_PULL_UP }  // I2C_SCL CNV1972
#elif (DEVICE == DEVICE_TH05D)
#ifdef GPIO_LED
		{ GPIO_P00, GPIO_FLOATING }, // LED
#else
		{ GPIO_P00, GPIO_PULL_DOWN }, // LED - не припаян R1 Q10 ...
#endif
		{ GPIO_P01, GPIO_PULL_DOWN },
		{ GPIO_P02, GPIO_PULL_UP }, // KEY - GPIO_KEY
		{ GPIO_P03, GPIO_PULL_DOWN },
		{ GPIO_P07, GPIO_PULL_UP }, // CHT8305 Alert
#ifdef GPIO_TRG
		{ GPIO_P09, GPIO_FLOATING }, // TX - GPIO_TRG
#else
		{ GPIO_P09, GPIO_PULL_UP }, // TX
#endif
		{ GPIO_P10, GPIO_PULL_UP }, // RX - GPIO_INP
		{ GPIO_P11, GPIO_FLOATING }, // BL55072 SDA
		{ GPIO_P14, GPIO_FLOATING }, // BL55072 SCL
		{ GPIO_P15, GPIO_PULL_UP }, // назначен как ADC_PIN, т.к. вывод P20 подключен к делителю
		{ GPIO_P16, GPIO_PULL_DOWN },
		{ GPIO_P17, GPIO_PULL_DOWN },
		{ GPIO_P18, GPIO_FLOATING }, // I2C_SDA CHT8310
		{ GPIO_P20, GPIO_FLOATING }, // ADC Vbat с делителем! Не используется - Выкусить резистор R18!
		{ GPIO_P23, GPIO_PULL_DOWN },
		{ GPIO_P24, GPIO_PULL_DOWN },
		{ GPIO_P25, GPIO_PULL_DOWN },
		{ GPIO_P26, GPIO_PULL_DOWN },
//		{ GPIO_P27, GPIO_PULL_DOWN },
		{ GPIO_P31, GPIO_FLOATING }, // CHT8305 SDA
		{ GPIO_P32, GPIO_FLOATING }, // CHT8305 SCL
		{ GPIO_P33, GPIO_PULL_DOWN },
		{ GPIO_P34, GPIO_PULL_DOWN }
#elif (DEVICE == DEVICE_TH05F)
		{ GPIO_P00, GPIO_PULL_UP_S }, // GPIO_SPWR Sensor Vdd
		{ GPIO_P01, GPIO_PULL_DOWN },
		{ GPIO_P02, GPIO_FLOATING }, // GPIO_LPWR питание LCD драйвера
		{ GPIO_P03, GPIO_PULL_DOWN },
		{ GPIO_P07, GPIO_PULL_DOWN },
		{ GPIO_P09, GPIO_PULL_UP }, // TX
		{ GPIO_P10, GPIO_PULL_UP }, // RX - GPIO_INP
		{ GPIO_P11, GPIO_PULL_UP }, // назначен как ADC_PIN
		{ GPIO_P14, GPIO_PULL_UP }, // KEY - GPIO_KEY
#ifdef GPIO_LED
		{ GPIO_P15, GPIO_FLOATING }, // LED (R7/D1/GND)
#else
		{ GPIO_P15, GPIO_PULL_UP }, // LED (R7/D1/GND)
#endif
		{ GPIO_P16, GPIO_PULL_DOWN },
		{ GPIO_P17, GPIO_PULL_DOWN },
		{ GPIO_P18, GPIO_PULL_UP }, // RX2
#ifdef GPIO_TRG
		{ GPIO_P20, GPIO_FLOATING }, // TX2 - GPIO_TRG
#else
		{ GPIO_P20, GPIO_PULL_UP }, // TX2 - GPIO_TRG
#endif
		{ GPIO_P23, GPIO_PULL_DOWN },
		{ GPIO_P24, GPIO_PULL_DOWN },
		{ GPIO_P25, GPIO_PULL_DOWN }, // mark "P25"
		{ GPIO_P26, GPIO_FLOATING }, // LCD I2C SDA
//		{ GPIO_P27, GPIO_PULL_DOWN },
		{ GPIO_P31, GPIO_FLOATING }, // LCD I2C SCL
		{ GPIO_P32, GPIO_PULL_DOWN },
		{ GPIO_P33, GPIO_FLOATING }, // CHT8305 SDA
		{ GPIO_P34, GPIO_FLOATING } // CHT8305 SCL
#else
#error "DEVICE Not released!"
#endif
#else
		{GPIO_P02, GPIO_FLOATING },
		{GPIO_P03, GPIO_FLOATING },
		{GPIO_P07, GPIO_FLOATING },
		{GPIO_P09, GPIO_FLOATING },
		{GPIO_P10, GPIO_FLOATING },
		{GPIO_P11, GPIO_FLOATING },
		{GPIO_P14, GPIO_FLOATING },
		{GPIO_P15, GPIO_FLOATING },
		{GPIO_P18, GPIO_FLOATING },
		{GPIO_P20, GPIO_FLOATING },
		{GPIO_P34, GPIO_FLOATING },

#endif
	};

	for (uint8_t i = 0; i < sizeof(ioInit) / sizeof(ioinit_cfg_t); i++) {
		hal_gpio_pull_set(ioInit[i].pin, ioInit[i].type);
	}
#ifdef GPIO_SPWR
	hal_gpio_write(GPIO_SPWR, 1);
#endif
#ifdef GPIO_LED
	hal_gpio_write(GPIO_LED, LED_ON);
#endif
	DCDC_CONFIG_SETTING(0x0a);
	DCDC_REF_CLK_SETTING(1);
	DIG_LDO_CURRENT_SETTING(1);
#if defined ( __GNUC__ )
#if 0 // test
	hal_pwrmgr_RAM_retention(RET_SRAM0 | RET_SRAM1 | RET_SRAM2);
#else
	extern uint32 g_irqstack_top;
	// Check IRQ STACK (1KB) location

/*
	if ((uint32_t) &g_irqstack_top > 0x1fffc000) {
		hal_pwrmgr_RAM_retention(RET_SRAM0 | RET_SRAM1 | RET_SRAM2);
	} else
*/
	if ((uint32_t) &g_irqstack_top > 0x1fff8000) {
		hal_pwrmgr_RAM_retention(RET_SRAM0 | RET_SRAM1);
    } else {
		hal_pwrmgr_RAM_retention(RET_SRAM0); // RET_SRAM0|RET_SRAM1|RET_SRAM2
	}
#endif
#else
#if DEBUG_INFO || SDK_VER_RELEASE_ID != 0x03010102
		hal_pwrmgr_RAM_retention(RET_SRAM0 | RET_SRAM1); // RET_SRAM0|RET_SRAM1|RET_SRAM2
#else
		hal_pwrmgr_RAM_retention(RET_SRAM0 | RET_SRAM1); // RET_SRAM0|RET_SRAM1|RET_SRAM2
#endif
#endif
	hal_pwrmgr_RAM_retention_set();
	subWriteReg(0x4000f014,26,26, 1); // hal_pwrmgr_LowCurrentLdo_enable();
	//hal_pwrmgr_LowCurrentLdo_disable();
}

static void ble_mem_init_config(void) {
#if 0	// SDK_VER_RELEASE_ID >= 0x030103 ?
	//ll linkmem setup
	  extern void ll_osalmem_init(osalMemHdr_t* hdr, uint32 size);
	ll_osalmem_init((osalMemHdr_t*)g_llLinkHeap, LL_LINK_HEAP_SIZE);
#endif
	osal_mem_set_heap((osalMemHdr_t*) g_largeHeap, LARGE_HEAP_SIZE);
	LL_InitConnectContext(pConnContext, g_pConnectionBuffer,
			BLE_MAX_ALLOW_CONNECTION,
			BLE_MAX_ALLOW_PKT_PER_EVENT_TX,
			BLE_MAX_ALLOW_PKT_PER_EVENT_RX,
			BLE_PKT_VERSION);
#if ( MAX_CONNECTION_SLAVE_NUM > 0 )
	static ALIGN4_U8	g_llDevList[BLE_CONN_LL_DEV_LIST_SIZE];
	ll_multi_conn_llDevList_Init(g_llDevList);
#endif

#if MAX_NUM_LL_CONN > 1
	Host_InitContext(	MAX_NUM_LL_CONN,
			glinkDB,glinkCBs,
			smPairingParam,
			gMTU_Size,
			gAuthenLink,
			l2capReassembleBuf,l2capSegmentBuf,
			gattClientInfo,
			gattServerInfo);
#endif
#ifdef	BLE_SUPPORT_CTE_IQ_SAMPLE
	LL_EXT_Init_IQ_pBuff(g_llCteSampleI,g_llCteSampleQ);
#endif
}

static void hal_rfphy_init(void) {
	//Watchdog_Init(NULL);
	//============config the txPower
	g_rfPhyTxPower = RF_PHY_TX_POWER_0DBM;
	//============config BLE_PHY TYPE
	g_rfPhyPktFmt = PKT_FMT_BLE1M;
	//============config RF Frequency Offset
	g_rfPhyFreqOffSet = RF_PHY_FREQ_FOFF_00KHZ; //	hal_rfPhyFreqOff_Set();
	//============config xtal 16M cap
	XTAL16M_CAP_SETTING(0x09); 					//	hal_xtal16m_cap_Set();
	XTAL16M_CURRENT_SETTING(0x01);

	hal_rc32k_clk_tracking_init();
    { /* замена hal_rom_boot_init() */
		extern void efuse_init(void);
		efuse_init();
        typedef void (*my_function)(void);
        my_function pFunc = (my_function)(0xa2e1);
        //ble_main();
        pFunc();
    }
	NVIC_SetPriority((IRQn_Type) BB_IRQn, IRQ_PRIO_REALTIME);
	NVIC_SetPriority((IRQn_Type) TIM1_IRQn, IRQ_PRIO_HIGH);		//ll_EVT
	NVIC_SetPriority((IRQn_Type) TIM2_IRQn, IRQ_PRIO_HIGH);		//OSAL_TICK
	NVIC_SetPriority((IRQn_Type) TIM4_IRQn, IRQ_PRIO_HIGH);		//LL_EXA_ADV
	//ble memory init and config
	ble_mem_init_config();
}

static void hal_init(void) {
	hal_low_power_io_init();
	clk_init(g_system_clk); //system init
	hal_rtc_clock_config((CLK32K_e) g_clk32K_config);
	hal_pwrmgr_init();
	// g_system_clk, SYS_CLK_DLL_64M, SYS_CLK_RC_32M / XFRD_FCMD_READ_QUAD, XFRD_FCMD_READ_DUAL
	hal_spif_cache_init(SYS_CLK_DLL_64M, XFRD_FCMD_READ_DUAL);
	hal_gpio_init();
	LOG_INIT();
	hal_adc_init();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(void) {
	g_system_clk = SYS_CLK_XTAL_16M; // SYS_CLK_XTAL_16M, SYS_CLK_DBL_32M, SYS_CLK_DLL_64M
	g_clk32K_config = CLK_32K_RCOSC; // CLK_32K_XTAL, CLK_32K_RCOSC

#if 0 // defined ( __GNUC__ ) // -> *.ld
	extern const uint32_t *const jump_table_base[];
	memcpy((void*) 0x1fff0000, (void*) jump_table_base, 1024);
#endif
	wrk.boot_flg = (uint8_t)read_reg(OTA_MODE_SELECT_REG);
#if defined(OTA_TYPE) && OTA_TYPE == OTA_TYPE_BOOT
#if (DEV_SERVICES & SERVICE_KEY)
	hal_gpio_pin_init(GPIO_KEY, GPIO_INPUT);
	if (hal_gpio_read(GPIO_KEY) == 0
    	|| wrk.boot_flg == BOOT_FLG_OTA
    	|| wrk.boot_flg == BOOT_FLG_FW0) {
#else
        if (wrk.boot_flg == BOOT_FLG_OTA
        	|| wrk.boot_flg == BOOT_FLG_FW0) {
#endif
    	write_reg(OTA_MODE_SELECT_REG,0);
    } else { // boot FW OTA
    	spif_config(SYS_CLK_DLL_64M, 1, XFRD_FCMD_READ_DUAL, 0, 0);
    	AP_PCR->CACHE_BYPASS = 1; // just bypass cache
    	startup_app();
	}
#endif // OTA_TYPE == OTA_TYPE_BOOT

#if CFG_SLEEP_MODE == PWR_MODE_SLEEP
	watchdog_config(WDG_32S);
#endif

//	spif_config(SYS_CLK_DLL_64M, 1, XFRD_FCMD_READ_DUAL, 0, 0);

#if (FLASH_PROTECT_FEATURE == 1)
#if SDK_VER_RELEASE_ID == 0x03010102
	hal_flash_lock();
#else
	hal_flash_enable_lock(MAIN_INIT);
#endif
#endif
	drv_irq_init();
	init_config();
#if ( HOST_CONFIG & OBSERVER_CFG )
	extern void ll_patch_advscan(void);
//	ll_patch_advscan();
#else
	extern void ll_patch_slave(void);
	ll_patch_slave();
//	extern void ll_patch_master(void);
//	ll_patch_master();
#endif

	hal_rfphy_init();
	hal_init();

	restore_utc_time_sec();
#if 0 //def STACK_MAX_SRAM
        extern uint32 g_stack;
    __set_MSP((uint32_t)(&g_stack));
#endif
	load_eep_config();

	LOG("SDK Version ID %08x \n",SDK_VER_RELEASE_ID);
	LOG("rfClk %d rcClk %d sysClk %d tpCap[%02x %02x]\n",g_rfPhyClkSel,g_clk32K_config,g_system_clk,g_rfPhyTpCal0,g_rfPhyTpCal1);
	LOG("sizeof(struct ll_pkt_desc) = %d, buf size = %d\n", sizeof(struct ll_pkt_desc), BLE_CONN_BUF_SIZE);
	LOG("sizeof(g_pConnectionBuffer) = %d, sizeof(pConnContext) = %d, sizeof(largeHeap)=%d \n",
		sizeof(g_pConnectionBuffer), sizeof(pConnContext),sizeof(g_largeHeap)); LOG("[REST CAUSE] %d\n ",g_system_reset_cause);
	app_main(); // No Return from here

	return 0;
}


