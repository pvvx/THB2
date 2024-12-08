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

uint8_t findmy_key[22];

typedef struct __attribute__((packed)) _adv_bthome_noencrypt_t {
	uint8_t		head[7];
	uint8_t		key[22];
	uint8_t 	end[2];
} adv_findmy_t, * padv_findmy_t;

static uint8 findmy_head[] = {
	0x1e, /* Length (30) */
	0xff, /* Manufacturer Specific Data (type 0xff) */
	0x4c, 0x00, /* Company ID (Apple) */
	0x12, 0x19, /* Offline Finding type and length */
	0x00 /* State */
};



uint8_t findmy_beacon(void * padbuf) {
	padv_findmy_t p = (padv_findmy_t)padbuf;
	memcpy(p->head, findmy_head, sizeof(findmy_head));
	memcpy(p->key, findmy_key, sizeof(findmy_key));
	p->end[0] = 0;
	p->end[1] = 0;
	return sizeof(adv_findmy_t);
}

#endif	// USE_FINDMY
