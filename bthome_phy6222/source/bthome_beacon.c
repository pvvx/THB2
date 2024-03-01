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

#if (DEV_SERVICES & SERVICE_BINDKEY)

#include "thb2_main.h"
#include "ccm.h"
#include "flash_eep.h"
#include "ll.h"

/* Encrypted bthome nonce */
typedef struct __attribute__((packed)) _bthome_beacon_nonce_t{
    uint8_t  mac[6];
    uint16_t uuid16;	// = 0xfcd2
    uint8_t  info;		// = 0x41
	uint32_t cnt32;
} bthome_beacon_nonce_t, * pbthome_beacon_nonce_t;

bthome_beacon_nonce_t bthome_nonce;
uint8_t bindkey[16];

void bthome_beacon_init(void) {
	// SwapMacAddress(bthome_nonce.mac, ownPublicAddr);
	bthome_nonce.mac[0] = ownPublicAddr[5];
	bthome_nonce.mac[1] = ownPublicAddr[4];
	bthome_nonce.mac[2] = ownPublicAddr[3];
	bthome_nonce.mac[3] = ownPublicAddr[2];
	bthome_nonce.mac[4] = ownPublicAddr[1];
	bthome_nonce.mac[5] = ownPublicAddr[0];
	bthome_nonce.uuid16 = ADV_BTHOME_UUID16;
	bthome_nonce.info = BtHomeID_Info_Encrypt;
	if (flash_read_cfg(bindkey, EEP_ID_KEY, sizeof(bindkey))
			!= sizeof(bindkey)) {
		LL_Rand(bindkey, sizeof(bindkey));
		flash_write_cfg(bindkey, EEP_ID_KEY, sizeof(bindkey));
	}
}

uint8_t adv_encrypt(uint8_t * p, uint8_t data_size) {
	uint8_t *pmic = &p[data_size];
	bthome_nonce.cnt32 = measured_data.count;
	*pmic++ = (uint8_t)measured_data.count;
	*pmic++ = (uint8_t)(measured_data.count>>8);
	*pmic++ = (uint8_t)(measured_data.count>>16);
	*pmic++ = (uint8_t)(measured_data.count>>24);
	ccm_auth_crypt(CCM_ENCRYPT, (const unsigned char *)&bindkey,
					   (uint8_t*)&bthome_nonce, sizeof(bthome_nonce),
					   (const unsigned char *)p, data_size,
					   p,
					   pmic, 4);
	return data_size + 4 + 4; // + mic + count
}
#endif // (DEV_SERVICES & SERVICE_BINDKEY)

#if (DEV_SERVICES & SERVICE_THS)

uint8_t adv_set_data(void * pd) {
	padv_bthome_data1_t p = (padv_bthome_data1_t)pd;
	p->b_id = BtHomeID_battery;
	p->battery_level = measured_data.battery;
	p->t_id = BtHomeID_temperature;
	p->temperature = measured_data.temp; // x0.01 C
	p->h_id = BtHomeID_humidity;
	p->humidity = measured_data.humi; // x0.01 %
	p->v_id = BtHomeID_voltage;
	p->battery_mv = measured_data.battery_mv; // x mV
	return sizeof(adv_bthome_data1_t);
}

#else

uint8_t adv_set_data(void * pd) {
	padv_bthome_data2_t p = (padv_bthome_data2_t)pd;
	p->b_id = BtHomeID_battery;
	p->battery_level = measured_data.battery;
	p->v_id = BtHomeID_voltage;
	p->battery_mv = measured_data.battery_mv; // x mV
	return sizeof(adv_bthome_data2_t);
}

#endif

#if (DEV_SERVICES & SERVICE_RDS)
uint8_t adv_set_event(void * ped) {
	padv_bthome_event1_t p = (padv_bthome_event1_t)ped;
	p->o_id = BtHomeID_opened;
	p->opened = measured_data.flg.pin_input;
	p->c_id = BtHomeID_count32;
	p->counter = adv_wrk.rds_count;
	return sizeof(adv_bthome_event1_t);
}
#endif

uint8_t bthome_data_beacon(void * padbuf) {
	padv_bthome_noencrypt_t p = (padv_bthome_noencrypt_t)padbuf;
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
#if (DEV_SERVICES & SERVICE_BINDKEY)
	if (cfg.flg & FLG_ADV_CRYPT) {
		padv_bthome_encrypt_t pe = (padv_bthome_encrypt_t)p;
		pe->info = BtHomeID_Info_Encrypt;
#if (DEV_SERVICES & SERVICE_RDS)
		if(adv_wrk.adv_event) {
			p->head.size = adv_encrypt(pe->data, adv_set_event(pe->data)) + sizeof(pe->head) - sizeof(pe->head.size) + sizeof(pe->info);
		} else
#endif
		{
			p->head.size = adv_encrypt(pe->data, adv_set_data(pe->data)) + sizeof(pe->head) - sizeof(pe->head.size) + sizeof(pe->info);
		}
	} else
#endif	// (DEV_SERVICES & SERVICE_BINDKEY)
	{
		p->info = BtHomeID_Info;
		p->p_id = BtHomeID_PacketId;
		p->pid = (uint8)measured_data.count;
	#if (DEV_SERVICES & SERVICE_RDS)
		if(adv_wrk.adv_event) {
			p->head.size = adv_set_event(p->data) + sizeof(p->head) - sizeof(p->head.size) + sizeof(p->info) + sizeof(p->p_id) + sizeof(p->pid);
		} else
	#endif
		{
			p->head.size = adv_set_data(p->data) + sizeof(p->head) - sizeof(p->head.size) + sizeof(p->info) + sizeof(p->p_id) + sizeof(p->pid);
		}

	}
	return p->head.size + sizeof(p->flag) + 1;
}

