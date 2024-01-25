/*
 * logger.h
 *
 *  Created on: 29.01.2021
 *      Author: pvvx
 */

#ifndef _LOGGER_H_
#define _LOGGER_H_
#include <string.h>
#include "config.h"
#if (DEV_SERVICES & SERVICE_HISTORY)
#include "flash.h"
#include "flash_eep.h"
#include "ble_ota.h"

#define MEMO_SEC_ID		0x55AAC0DE // sector head
#define FLASH_ADDR_START_MEMO	((FADDR_APP_SEC - FADDR_START_ADDR) + 0x20000 ) // + 128 kbytes
#define FLASH_ADDR_END_MEMO		FMEMORY_SCFG_BASE_ADDR

typedef struct _memo_blk_t {
	uint32_t time;  // time (UTC)
	int16_t temp;	// x0.01 C
	uint16_t humi;  // x0.01 %
	uint16_t vbat;  // mV
}memo_blk_t, * pmemo_blk_t;
#define _memo_blk_size 10

typedef struct _memo_inf_t {
	uint32_t faddr;
	uint32_t cnt_cur_sec;
}memo_inf_t;

typedef struct _memo_rd_t {
	memo_inf_t saved;
	uint32_t cnt;
	uint32_t cur;
}memo_rd_t;

typedef struct _memo_head_t {
	uint32_t id;  // = 0x55AAC0DE (MEMO_SEC_ID)
	uint16_t flg;  // = 0xffff - new sector, = 0 close sector
}memo_head_t;
#define _memo_head_size (4+2)


extern memo_rd_t rd_memo;
extern memo_inf_t memo;

void memo_init(void);
void clear_memo(void);
unsigned get_memo(uint32_t bnum, pmemo_blk_t p);
void write_memo(void);
int send_memo_blk(uint8_t * send_buf);

#endif // (DEV_SERVICES & SERVICE_HISTORY)
#endif /* _LOGGER_H_ */
