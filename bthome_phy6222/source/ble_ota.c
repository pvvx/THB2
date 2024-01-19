/******************************************************************************
 * @file     ble_ota.c
 *
 ******************************************************************************/
#include "bcomdef.h"
#include "config.h"
#if OTA_TYPE
#include "OSAL.h"
#include "flash.h"
#include "ble_ota.h"
#include "sbp_profile.h"
#include "thb2_peripheral.h"

/*******************************************************************************
* CONSTANTS */
#define ota_timeout_us	30000;  // default 30 second
/*******************************************************************************
* Prototypes */
typedef struct _app_info_t {
	uint32_t flag;
	uint32_t seg_count;
	uint32_t start_addr;
	uint32_t app_size;
} app_info_t;

typedef struct _app_info_seg_t {
	uint32_t faddr;
	uint32_t size;
	uint32_t saddr;
	uint32_t chk;
} app_info_seg_t;

/*******************************************************************************
* LOCAL VARIABLES */
ota_par_t ota = {
		.err_flag = OTA_SUCCESS,
		.version = 1,
		.start_flag = 0,
		.reboot_flag = 0,
		.program_offset = FADDR_APP_SEC,
		.pkt_index = -1,
		.fw_value = START_UP_FLAG,
};
/*********************************************************************
* EXTERNAL VARIABLES */


/*********************************************************************
* LOCAL FUNCTION */

unsigned short crc16(unsigned char *pD, int len) {
	static unsigned short poly[2] = { 0, 0xa001 };          //0x8005 <==> 0xa001
	unsigned short crc = 0xffff;
	int i, j;

	for (j = len; j > 0; j--) {
		unsigned char ds = *pD++;
		for (i = 0; i < 8; i++) {
			crc = (crc >> 1) ^ poly[(crc ^ ds) & 1];
			ds = ds >> 1;
		}
	}
	return crc;
}

static const unsigned int crc32_half_tbl[16] = {
	0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
	0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
	0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
	0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

unsigned int crc32_half_cal(unsigned int crc, unsigned char *input,
		unsigned int *table, int len) {
	unsigned char *pch = input;
	for (int i = 0; i < len; i++) {
		crc = (crc >> 4) ^ table[(crc ^ *pch) & 0x0f];
		pch++;
	}
	return crc;
}

uint32_t get_crc32_16bytes(unsigned int crc_init, unsigned char *data) {
	// split 16 bytes OTA data into 32 half bytes to calculate CRC.
	uint8_t ota_dat[32];
	for (int i = 0; i < 16; i++) {
		ota_dat[i * 2] = data[i] & 0x0f;
		ota_dat[i * 2 + 1] = data[i] >> 4;
	}
	return crc32_half_cal(crc_init, ota_dat,
			(unsigned int*) crc32_half_tbl,	32);
}


int ota_parser(unsigned char *pout, unsigned char *pmsg, unsigned int msg_size) {
	uint32_t tmp;
	uint16_t crc;
	uint16_t ota_adr = pmsg[0] | (pmsg[1] << 8);
	uint8_t flash_check[16];
	int err_flg = OTA_SUCCESS;
	if(msg_size >= 2) {
		if (ota_adr >= CMD_OTA_START) {
			if (ota_adr == CMD_OTA_START) {
				if(msg_size == 2 + 4) {
					ota.erase_addr = (pmsg[2]
							| (pmsg[3] << 8)
							| (pmsg[4] << 16)
							| (pmsg[5] << 24));
				} else
					ota.erase_addr = FADDR_APP_SEC-1;
				ota.fw_value = START_UP_FLAG;
				ota.start_flag = 0;
				ota.err_flag = OTA_SUCCESS;
			} else if (ota_adr == CMD_OTA_SET) {
				if(ota.start_flag) {
					err_flg = OTA_NO_START;
				} else if(msg_size >= 2 + 10) {
					ota.program_offset = ((pmsg[2] & 0xf0)
							| (pmsg[3] << 8)
							| (pmsg[4] << 16)
							| (pmsg[5] << 24));
					ota.fw_value = (pmsg[6]
							| (pmsg[7] << 8)
							| (pmsg[8] << 16)
							| (pmsg[9] << 24));
					ota.pkt_total = (pmsg[10]
							| (pmsg[11] << 8));
					if(ota.program_offset >= FADDR_APP_SEC
						&& ota.program_offset + (ota.pkt_total << 4) <= FLASH_MAX_SIZE) {
						ota.pkt_index = -1;
						ota.start_flag = 1;   //set flag
					} else
						err_flg = OTA_ERR_PARAM; // invalid offset
				} else
					err_flg = OTA_PKT_SIZE_ERR; // size error
			} else if (ota_adr == CMD_OTA_END) {
				//go to reboot or start app
				GAPRole_TerminateConnection();
				hal_system_soft_reset();
			} else
				err_flg = OTA_UNKNOWN_CMD; // unknown commad
		} else if(ota.err_flag) {
			// stop - old error
		} else if(ota.start_flag) {
			if (ota.pkt_index + 1 == ota_adr) {   // correct OTA data index
				if(msg_size < 2+16+2) {
					crc = (pmsg[19] << 8) | pmsg[18];
					if (crc == crc16(pmsg, 18)) {
						if (ota_adr == 0) {
							ota.crc32 = 0xFFFFFFFF;  // crc init set to 0xFFFFFFFF
							tmp = ((pmsg[2])
								| (pmsg[3] << 8)
								| (pmsg[4] << 16)
								| (pmsg[5] << 24));
							if (tmp != ota.fw_value) // id != ?
								err_flg = OTA_FW_CHECK_ERR;
						} else if (ota_adr == ota.pkt_total - 1) {
							tmp = ((pmsg[2])
									| (pmsg[3] << 8)
									| (pmsg[4] << 16)
									| (pmsg[5] << 24));
							if (ota.crc32 != tmp) // crc32 != ?
								err_flg = OTA_FW_CRC32_ERR;
							else {
								hal_flash_write(ota.program_offset,
										(uint8_t*) &ota.fw_value, 4);
								hal_flash_read(ota.program_offset,
										(uint8_t*) &tmp,  4);
								if (ota.fw_value == tmp) {  // OK
									err_flg = OTA_END;
								} else
									err_flg = OTA_WRITE_FLASH_ERR; // flash write err
							}
						}
						if (((ota_adr == 0) || ota_adr < ota.pkt_total - 1))
							ota.crc32 = get_crc32_16bytes(ota.crc32, pmsg + 2);
						if (ota_adr < ota.pkt_total) {
							tmp = (ota.program_offset + (ota_adr << 4))
									& (~(FLASH_SECTOR_SIZE-1));
							if (tmp > ota.erase_addr) {
								ota.erase_addr = tmp;
								hal_flash_erase_sector(tmp);
							}
							if (ota_adr == 0) {
								pmsg[2] = 0xff;
								pmsg[3] = 0xff;
								pmsg[4] = 0xff;
								pmsg[5] = 0xff;
							}
							hal_flash_write(ota.program_offset + (ota_adr << 4),
									pmsg + 2, 16);
							hal_flash_read(ota.program_offset + (ota_adr << 4),
									flash_check,  16);
							if (!osal_memcmp(flash_check, pmsg + 2, 16)) {  // OK
								ota.pkt_index = ota_adr;
							} else
								err_flg = OTA_WRITE_FLASH_ERR; // flash write err
						} else
							err_flg = OTA_OVERFLOW;
					} else
						err_flg = OTA_PKT_CRC_ERR; // crc err
				} else
					err_flg = OTA_PKT_SIZE_ERR; // size error
			} else if (ota_adr <= ota.pkt_index) {
				// maybe repeated OTA data, we neglect it, do not consider it ERR
			} else
				err_flg = OTA_PACKET_LOSS; // adr index err, missing at least one OTA data
		} else
			err_flg = OTA_NO_PARAM;
	} else
		err_flg = OTA_PKT_SIZE_ERR; // size error
	if (err_flg != OTA_SUCCESS) {
		ota.err_flag = err_flg;
		//send/Notify?
		osal_memcpy(pout, &ota, 20);
		return 20;
	}
	return 0;
}

__ATTR_SECTION_XIP__
static uint32_t start_app(void) {
	app_info_t info_app;
	app_info_seg_t info_seg;
	uint32_t info_seg_faddr = FADDR_APP_SEC;

	spif_read(info_seg_faddr, (uint8_t*)&info_app, sizeof(info_app));
	if(info_app.flag == START_UP_FLAG) {
/* Move OTA not released!
		if(info_app.app_size != 0xFFFFFFFF) {
			info_app.app_size += FLASH_SECTOR_SIZE-1;
			info_app.app_size &= ~(FLASH_SECTOR_SIZE-1);
			info_app.app_size += FADDR_START_ADDR;
			spif_read(info_app.app_size, (uint8_t*)&info_app.flag, sizeof(info_app.flag));
			if(info_app.flag == START_UP_FLAG)
				move_ota_app(info_app.app_size);
		}
*/
		if(info_app.seg_count <= 16) {
			while(info_app.seg_count) {
				info_seg_faddr +=  sizeof(info_app);
				spif_read(info_seg_faddr, (uint8_t*)&info_seg, sizeof(info_seg));
				if(info_app.start_addr == 0xffffffff) // если не назначен
					info_app.start_addr = info_seg.saddr; // берется первый сегмент
				info_seg.faddr += FADDR_START_ADDR;
				info_seg.size &= 0x000fffff;
				if (info_seg.saddr != info_seg.faddr // не XIP
						&& info_seg.size < (128*1024)) { // < 128k
					osal_memcpy((void *)info_seg.saddr, (void *)info_seg.faddr, info_seg.size);
				}
				info_app.seg_count--;
			}
		}
		if(info_app.start_addr == 0xffffffff) {
			info_app.start_addr = 0;
		}
	} else
		info_app.start_addr = 0;
	return info_app.start_addr;
}

#if   defined ( __CC_ARM )
__asm void __attribute__((section("ota_app_loader_area"))) jump2app(uint32_t entry)
{
    LDR R0, = __APP_RUN_ADDR__
              LDR R1, [R0, #4]
              BX R1
              ALIGN
}
#elif defined ( __GNUC__ )
__ATTR_SECTION_XIP__
void jump2app(uint32_t entry)
{
    __ASM volatile("ldr r0, %0\n\t"
                   "ldr r1, [r0, #4]\n\t"
                   "bx r1"
                   :"+m"(entry)
                  );
}
#endif

__ATTR_SECTION_XIP__
void startup_ota(void) {
    uint32_t start_addr;

    HAL_ENTER_CRITICAL_SECTION();

    start_addr = start_app();
    if(start_addr) {
        //AP_PCR->CACHE_BYPASS = 1; // bypass cache
        jump2app(start_addr);
    }
}

#endif // OTA_TYPE

