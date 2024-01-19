/******************************************************************************
 * @file     ble_ota.c
 *
 ******************************************************************************/
#include "bcomdef.h"
#include "config.h"
#include "OSAL.h"
#include "sbp_profile.h"
#include "ble_ota.h"

struct {
	u8  err_flag;
	u8  version;
	u8  start_flag;
	u8  reboot_flag;
	u32 program_offset;
	u16 pkt_index;
	u16 pkt_total;
	u32 fw_value;
	u32 crc32;
	u32 erase_offset;
} ota;

#define blt_ota_timeout_us	30000;  // default 30 second

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

u32 get_crc32_16bytes(unsigned int crc_init, unsigned char *data) {
	// split 16 bytes OTA data into 32 half bytes to caculate CRC.
	u8 ota_dat[32];
	for (int i = 0; i < 16; i++) {
		ota_dat[i * 2] = data[i] & 0x0f;
		ota_dat[i * 2 + 1] = data[i] >> 4;
	}
	return crc32_half_cal(crc_init, ota_dat,
			(unsigned int*) crc32_half_tbl,	32);
}


void ota_reload_imer(void) {

}

int otaWrite(unsigned char imsg, unsigned char isize) {
	u32 tmp;
	u16 crc;
	u16 ota_adr = imsg[0] | (imsg[1] << 8);
	u8 flash_check[16];
	u8 err_flg = OTA_SUCCESS;
	if(isize > 2) {
		//	ota_reload_imer();
		if (ota_adr >= CMD_OTA_START) {
			if (ota_adr == CMD_OTA_START) {
				ota.erase_offset = FADD_APP_SEC;
				ota.fw_value = START_UP_FLAG;
				ota.start_flag = 0;
				ota.err_flag = OTA_SUCCESS;
			} else if (ota_adr == CMD_OTA_SET) {
				if(ota.start_flag) {
					err_flg = OTA_NO_START;
				} else if(isize >= 2 + 10) {
					ota.program_offset = ((imsg[2] & 0xf0)
							| (imsg[3] << 8)
							| (imsg[4] << 16)
							| (imsg[5] << 24));
					ota.fw_value = (imsg[6]
							| (imsg[7] << 8)
							| (imsg[8] << 16)
							| (imsg[9] << 24));
					ota.pkt_total = (imsg[10]
							| (imsg[11] << 8));
					if(ota.program_offset >= FADD_APP_SEC
						&& ota.program_offset + (ota.pkt_total << 4) <= FLASH_MAX_SIZE) {
						ota.pkt_index = -1;
						ota.start_flag = 1;   //set flag
					} else
						err_flg = OTA_ERR_PARAM; // invalid offset
				} else
					err_flg = OTA_PKT_SIZE_ERR; // size error
			} else if (ota_adr == CMD_OTA_END) {
				//terminateConnection(0x13);
				//timer(reboot)
			} else
				err_flg = OTA_UNKNOWN_CMD; // unknown commad
		} else if(ota.start_flag) {
			if (ota.pkt_index + 1 == ota_adr) {   // correct OTA data index
				if(isize < 2+16+2) {
					crc = (imsg[19] << 8) | imsg[18];
					if (crc == crc16(imsg, 18)) {
						if (ota_adr == 0) {
							ota.crc32 = 0xFFFFFFFF;  // crc init set to 0xFFFFFFFF
							tmp = ((imsg[2])
								| (imsg[3] << 8)
								| (imsg[4] << 16)
								| (imsg[5] << 24));
							if (tmp != ota.fw_value) // id != ?
								err_flg = OTA_FW_CHECK_ERR;
						} else if (ota_adr == ota.pkt_total - 1) {
							tmp = ((imsg[2])
									| (imsg[3] << 8)
									| (imsg[4] << 16)
									| (imsg[5] << 24));
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
							ota.crc32 = get_crc32_16bytes(ota.crc32, imsg + 2);
						if (ota_adr < ota.pkt_total) {
							if (ota_adr == 0) {
								imsg[2] = 0xff;
								imsg[3] = 0xff;
								imsg[4] = 0xff;
								imsg[5] = 0xff;
							}
							hal_flash_write(ota.program_offset + (ota_adr << 4),
									imsg + 2, 16);
							hal_flash_read(ota.program_offset + (ota_adr << 4),
									flash_check,  16);
							if (!memcmp(flash_check, imsg + 2, 16)) {  // OK
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
	if (err_flg) {
		ota.err_flag = err_flg;
		blt_ota_finished_flag_set(err_flg);
	}
	return 0;
}
