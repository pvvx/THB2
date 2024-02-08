/*
 * bthome_beacon.c
 *
 *  Created on: 17.10.23
 *      Author: pvvx
 */
#include "rom_sym_def.h"
#include "types.h"
#include "config.h"
#include "bcomdef.h"
#include "gapbondmgr.h"
#include "sensors.h"
#include "bthome_beacon.h"

uint8_t bthome_data_beacon(void * padbuf) {
	padv_bthome_ns1_t p = (padv_bthome_ns1_t)padbuf;
	p->flag[0] = 0x02; // size
	p->flag[1] = GAP_ADTYPE_FLAGS; // type
	/*	Flags:
	 	bit0: LE Limited Discoverable Mode
		bit1: LE General Discoverable Mode
		bit2: BR/EDR Not Supported
		bit3: Simultaneous LE and BR/EDR to Same Device Capable (Controller)
		bit4: Simultaneous LE and BR/EDR to Same Device Capable (Host)
		bit5..7: Reserved
	*/
	p->flag[2] = GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED | GAP_ADTYPE_FLAGS_GENERAL; // Flags
	p->head.type = GAP_ADTYPE_SERVICE_DATA; // 16-bit UUID
	p->head.UUID = ADV_BTHOME_UUID16;
	p->info = BtHomeID_Info;
	p->p_id = BtHomeID_PacketId;
	p->pid = (uint8)measured_data.count;
#if (DEV_SERVICES & SERVICE_RDS)
	if(adv_wrk.adv_event) {
		padv_bthome_evns1_t pe = (padv_bthome_evns1_t)p;
		pe->data.o_id = BtHomeID_opened;
		pe->data.opened = measured_data.flg.pin_input;
		pe->data.c_id = BtHomeID_count32;
		pe->data.counter = adv_wrk.rds_count;
		pe->head.size = sizeof(adv_bthome_evns1_t) - sizeof(pe->head.size) - sizeof(pe->flag);
		return sizeof(adv_bthome_ns1_t);
	} else
#endif
#if (DEV_SERVICES & SERVICE_THS)
	{
		p->data.b_id = BtHomeID_battery;
		p->data.battery_level = measured_data.battery;
		p->data.t_id = BtHomeID_temperature;
		p->data.temperature = measured_data.temp; // x0.01 C
		p->data.h_id = BtHomeID_humidity;
		p->data.humidity = measured_data.humi; // x0.01 %
		p->data.v_id = BtHomeID_voltage;
		p->data.battery_mv = measured_data.battery_mv; // x mV
		p->head.size = sizeof(adv_bthome_ns1_t) - sizeof(p->head.size) - sizeof(p->flag);
		return sizeof(adv_bthome_ns1_t);
	}
#else
	{
		padv_bthome_ns2_t pe = (padv_bthome_ns2_t)p;
		pe->data.b_id = BtHomeID_battery;
		pe->data.battery_level = measured_data.battery;
		pe->data.v_id = BtHomeID_voltage;
		pe->data.battery_mv = measured_data.battery_mv; // x mV
		pe->head.size = sizeof(adv_bthome_ns2_t) - sizeof(pe->head.size) - sizeof(pe->flag);
		return sizeof(adv_bthome_ns2_t);
	}
#endif
}



