/*
 * bthome_beacon.c
 *
 *  Created on: 17.10.23
 *      Author: pvvx
 */
#include "rom_sym_def.h"
#include "types.h"
#include "bcomdef.h"
#include "gapbondmgr.h"
#include "sensor.h"
#include "bthome_beacon.h"

//adv_buf_t adv_buf;

void bthome_data_beacon(padv_bthome_ns1_t p) {
	//	padv_bthome_ns1_t p = (padv_bthome_ns1_t)&adv_buf.data;
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
	p->data.b_id = BtHomeID_battery;
	p->data.battery_level = measured_data.battery;
	p->data.t_id = BtHomeID_temperature;
	p->data.temperature = measured_data.temp; // x0.01 C
	p->data.h_id = BtHomeID_humidity;
	p->data.humidity = measured_data.humi; // x0.01 %
	p->data.v_id = BtHomeID_voltage;
	p->data.battery_mv = measured_data.battery_mv; // x mV
	p->head.size = sizeof(adv_bthome_ns1_t) - sizeof(p->head.size) - sizeof(p->flag);
}




