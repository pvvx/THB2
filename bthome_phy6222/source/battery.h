/*
 battery.h 
 Author: pvvx
*/

#ifndef _BATTERY_HEAD_FILE
#define _BATTERY_HEAD_FILE

#define BATT_TIMER_MEASURE_INTERVAL		3*60*1000 	// 3 minute interval

#define VBAT_ADC_P11	2
#define VBAT_ADC_P23	3
#define VBAT_ADC_P24	4
#define VBAT_ADC_P14	5
#define VBAT_ADC_P15	6
#define VBAT_ADC_P20	7

void batt_start_measure(void);
void check_battery(void);

#endif


