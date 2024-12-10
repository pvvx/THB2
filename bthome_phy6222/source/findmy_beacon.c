/*
 * findmy_beacon.c
 *
 *  Created on: 8 дек. 2024 г.
 *      Author: pvvx
 */
#include "rom_sym_def.h"
#include "types.h"
#include "config.h"

#if (DEV_SERVICES & SERVICE_FINDMY)
#include "findmy_beacon.h"
#include "sensors.h"

uint8_t findmy_key[SIZE_FINDMY_KEY];
uint8_t findmy_key_new[SIZE_FINDMY_KEY];

typedef struct __attribute__((packed)) _adv_bthome_noencrypt_t {
	uint8_t		head[6];
	uint8_t		state;
/* State:
 Bits 0—1: Reserved.
 Bit  2: Maintained
 Bits 3—4: Reserved
 Bits 5: 0b1
 Bits 6—7: Battery state:
  0 = Full
  1 = Medium
  2 = Low
  3 = Critically low
 */
	uint8_t		key[22];
	uint8_t		key0_bit67;
	uint8_t 	end;
} adv_findmy_t, * padv_findmy_t;

static uint8 findmy_head[] = {
	0x1e, /* Length (30) */
	0xff, /* Manufacturer Specific Data (type 0xff) */
	0x4c, 0x00, /* Company ID (Apple) */
	0x12, 0x19 /* Offline Finding type and length */
};


uint8_t findmy_beacon(void * padbuf) {
	padv_findmy_t p = (padv_findmy_t)padbuf;
	memcpy(p->head, findmy_head, sizeof(findmy_head));
	memcpy(p->key, &findmy_key[6], sizeof(p->key));
	p->key0_bit67 = findmy_key[0] >> 6;
	if(measured_data.battery > 80)
		p->state = 0 << 6; // Full
	else if(measured_data.battery > 60)
		p->state = 1 << 6; // Medium
	else if(measured_data.battery > 25)
		p->state = 2 << 6; // Low
	else
		p->state = 3 << 6; // Critically low
	p->end = 0;
	return sizeof(adv_findmy_t);
}

#endif	// USE_FINDMY
