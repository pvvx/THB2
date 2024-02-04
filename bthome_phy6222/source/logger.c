/*
 * logger.c
 *
 *  Created on: 29.01.2021
 *      Author: pvvx
 */
#include "config.h"
#if (DEV_SERVICES & SERVICE_HISTORY)
#include "flash.h"
#include "flash_eep.h"
#include "logger.h"
#include "sensors.h"
#include "cmd_parser.h"

#define RAM

#define MEMO_SEC_COUNT		((FLASH_ADDR_END_MEMO - FLASH_ADDR_START_MEMO) / FLASH_SECTOR_SIZE) // 76 sectors
#define MEMO_SEC_RECS		((FLASH_SECTOR_SIZE-_memo_head_size)/_memo_blk_size) // -  sector: 409 records
//#define MEMO_REC_COUNT		(MEMO_SEC_RECS*(MEMO_SEC_COUNT-1))// max (76-1)*409 = 30675 records

#define _flash_read(a,b,c) memcpy((void *)c, (void *)(FLASH_BASE_ADDR + (unsigned int)a), b) // _flash_read(rdaddr, len, pbuf);
#define _flash_erase_sector(addr) hal_flash_erase_sector(FLASH_BASE_ADDR + addr)
#define _flash_write_dword(addr, wd) flash_write_word(FLASH_BASE_ADDR + addr,wd)
#define _flash_write(a,l,b) hal_flash_write(FLASH_BASE_ADDR + a,(unsigned char *)b, l)

typedef struct _summ_data_t {
	uint32_t	battery_mv; // mV
	int32_t		temp; // x 0.01 C
	uint32_t	humi; // x 0.01 %
	uint32_t 	count;
} summ_data_t;
RAM summ_data_t summ_data;

RAM memo_inf_t memo;
RAM memo_rd_t rd_memo;

static uint32_t test_next_memo_sec_addr(uint32_t faddr) {
	uint32_t mfaddr = faddr;
	if (mfaddr >= FLASH_ADDR_END_MEMO)
		mfaddr = FLASH_ADDR_START_MEMO;
	else if (mfaddr < FLASH_ADDR_START_MEMO)
		mfaddr = FLASH_ADDR_END_MEMO - FLASH_SECTOR_SIZE;
	return mfaddr;
}

static void memo_sec_init(uint32_t faddr) {
	uint32_t mfaddr = faddr;
	mfaddr &= ~(FLASH_SECTOR_SIZE-1);
	_flash_erase_sector(mfaddr);
	_flash_write_dword(mfaddr, MEMO_SEC_ID);
	memo.faddr = mfaddr + _memo_head_size;
	memo.cnt_cur_sec = 0;
}

static void memo_sec_close(uint32_t faddr) {
	uint32_t mfaddr = faddr;
	uint16_t flg = 0;
	mfaddr &= ~(FLASH_SECTOR_SIZE-1);
	_flash_write(mfaddr + _memo_head_size - 2, 2, &flg); // sizeof(flg), sizeof(flg), &flg);
	memo_sec_init(test_next_memo_sec_addr(mfaddr + FLASH_SECTOR_SIZE));
}

#if 0
void memo_init_count(void) {
	memo_head_t mhs;
	uint32_t cnt, i = 0;
	uint32_t faddr = memo.faddr & (~(FLASH_SECTOR_SIZE-1));
	cnt = memo.faddr - faddr - _memo_head_size; // смещение в секторе
	cnt /= _memo_blk_size;
	do {
		faddr = test_next_memo_sec_addr(faddr - FLASH_SECTOR_SIZE);
		_flash_read(faddr, &mhs, _memo_head_size);
		i++;
	} while (mhs.id == MEMO_SEC_ID && mhs.flg == 0 && i < MEMO_SEC_COUNT);
	cnt += i *MEMO_SEC_RECS;
	memo.count = cnt;
}
#endif


static void restore_time(uint32_t time) {
	if(time && clkt.utc_time_sec == 0) {
		clkt.utc_time_sec = time;
		clkt.utc_time_add = 1024;
		clkt.utc_time_tik = clock_time_rtc();
	}
}

void memo_init(void) {
	memo_head_t mhs;
	uint32_t tmp, fsec_end, time = 0;
	uint32_t faddr = FLASH_ADDR_START_MEMO;
	memo.cnt_cur_sec = 0;
	while (faddr < FLASH_ADDR_END_MEMO) {
		_flash_read(faddr, _memo_head_size, &mhs);
		if (mhs.id != MEMO_SEC_ID) {
			memo_sec_init(faddr);
			restore_time(time);
			return;
		} else if (mhs.flg == 0xffff) {
			fsec_end = faddr + FLASH_SECTOR_SIZE;
			faddr += _memo_head_size;
			while (faddr < fsec_end) {
				_flash_read(faddr, sizeof(tmp), &tmp);
				if (tmp == 0xffffffff) {
					memo.faddr = faddr;
					restore_time(time);
					return;
				}
				if(time < tmp)
					time = tmp;
				memo.cnt_cur_sec++;
				faddr += _memo_blk_size;
			}
			memo_sec_close(fsec_end - FLASH_SECTOR_SIZE);
			restore_time(time);
			return;
		}
		faddr += FLASH_SECTOR_SIZE;
	}
	memo_sec_init(FLASH_ADDR_START_MEMO);
	restore_time(time);
	return;
}

void clear_memo(void) {
	uint32_t tmp;
	uint32_t faddr = FLASH_ADDR_START_MEMO + FLASH_SECTOR_SIZE;
	memo.cnt_cur_sec = 0;
	while (faddr < FLASH_ADDR_END_MEMO) {
		_flash_read(faddr, 4, &tmp); // sizeof(tmp), &tmp);
		if (tmp == MEMO_SEC_ID)
			_flash_erase_sector(faddr);
		faddr += FLASH_SECTOR_SIZE;
	}
	memo_sec_init(FLASH_ADDR_START_MEMO);
	return;
}

unsigned get_memo(uint32_t bnum, pmemo_blk_t p) {
	memo_head_t mhs;
	uint32_t faddr;
	faddr = rd_memo.saved.faddr & (~(FLASH_SECTOR_SIZE-1));
	if (bnum > rd_memo.saved.cnt_cur_sec) {
		bnum -= rd_memo.saved.cnt_cur_sec;
		faddr -= FLASH_SECTOR_SIZE;
		if (faddr < FLASH_ADDR_START_MEMO)
			faddr = FLASH_ADDR_END_MEMO - FLASH_SECTOR_SIZE;
		while (bnum > MEMO_SEC_RECS) {
			bnum -= MEMO_SEC_RECS;
			faddr -= FLASH_SECTOR_SIZE;
			if (faddr < FLASH_ADDR_START_MEMO)
				faddr = FLASH_ADDR_END_MEMO - FLASH_SECTOR_SIZE;
		}
		bnum = MEMO_SEC_RECS - bnum;
		_flash_read(faddr, _memo_head_size, &mhs);
		if (mhs.id != MEMO_SEC_ID || mhs.flg != 0)
			return 0;
	} else {
		bnum = rd_memo.saved.cnt_cur_sec - bnum;
	}
	faddr += _memo_head_size; // смещение в секторе
	faddr += bnum * _memo_blk_size; // * size memo
	_flash_read(faddr, _memo_blk_size, p);
	return 1;
}

int send_memo_blk(uint8_t * send_buf) {
	int olen = 0;
	send_buf[0] = CMD_ID_LOGGER;
	if (++rd_memo.cur > rd_memo.cnt || (!get_memo(rd_memo.cur, (pmemo_blk_t)&send_buf[3]))) {
		send_buf[1] = 0;
		send_buf[2] = 0;
		olen = 3;
		rd_memo.cnt = 0;
	} else {
		send_buf[1] = rd_memo.cur;
		send_buf[2] = rd_memo.cur >> 8;
		olen = 3 + _memo_blk_size;
	}
	return olen;
}


// if (cfg.averaging_measurements != 0) write_memo();
void write_memo(void) {
	memo_blk_t mblk;
	if (cfg.averaging_measurements == 1) {
		mblk.temp = measured_data.temp;
		mblk.humi = measured_data.humi;
		mblk.vbat = measured_data.battery_mv;
	} else {
		summ_data.temp += measured_data.temp;
		summ_data.humi += measured_data.humi;
		summ_data.battery_mv += measured_data.battery_mv;
		summ_data.count++;
		if (cfg.averaging_measurements > summ_data.count)
			return;
		mblk.temp = (int16_t)(summ_data.temp/(int32_t)summ_data.count);
		mblk.humi = (uint16_t)(summ_data.humi/summ_data.count);
		mblk.vbat = (uint16_t)(summ_data.battery_mv/summ_data.count);
		memset(&summ_data, 0, sizeof(summ_data));
	}
	mblk.time = clkt.utc_time_sec;
	uint32_t faddr = memo.faddr;
	if (!faddr) {
		memo_init();
		faddr = memo.faddr;
	}
	_flash_write(faddr, _memo_blk_size, &mblk);
	faddr += _memo_blk_size;
	faddr &= (~(FLASH_SECTOR_SIZE-1));
	if (memo.cnt_cur_sec >= MEMO_SEC_RECS - 1 ||
		(memo.faddr & (~(FLASH_SECTOR_SIZE-1))) != faddr) {
		memo_sec_close(memo.faddr);
	} else {
		memo.cnt_cur_sec++;
		memo.faddr += _memo_blk_size;
	}
}

#endif // defined(DEV_SERVICES) && (DEV_SERVICES & SERVICE_HISTORY)
