/******************************************************************************
 * FileName: flash_eep.h
 * Description: FLASH
 * Alternate SDK 
 * Author: PV`
 * (c) PV` 2015
*******************************************************************************/
#ifndef __FLASH_EEP_H_
#define __FLASH_EEP_H_

#ifdef __cplusplus
extern "C" {
#endif

// EEPROM IDs
#define EEP_ID_MAC (0xACAD) // EEP ID MAC
#define EEP_ID_CFG (0x0CFC) // EEP ID config data
#define EEP_ID_CFS (0x0CF5) // EEP ID sensor coefficients
#define EEP_ID_TRG (0x1DF5) // EEP ID TH trigger config
//#define EEP_ID_PCD (0xC0DE) // EEP ID pincode
//#define EEP_ID_CMF (0x0FCC) // EEP ID comfort data
#define EEP_ID_DVN (0xDEAE) // EEP ID device name
#define EEP_ID_TIM (0x0ADA) // EEP ID time adjust
#define EEP_ID_KEY (0xBC0D) // EEP ID bindkey
#define EEP_ID_VER (0x5555) // EEP ID blk: unsigned int = minimum supported version
//-----------------------------------------------------------------------------
#ifndef FLASH_BASE_ADDR
#define FLASH_BASE_ADDR			0x11000000
#endif
#ifndef FLASH_SIZE
#define FLASH_SIZE				(512*1024)
#endif
#ifndef FLASH_SECTOR_SIZE
#define FLASH_SECTOR_SIZE		4096
#endif
#define FMEMORY_SCFG_BANK_SIZE	FLASH_SECTOR_SIZE // размер сектора, 4096 bytes
#define FMEMORY_SCFG_BANKS 		4 // кол-во секторов для работы - min 2
#define FMEMORY_SCFG_BASE_ADDR	(FLASH_SIZE - (FMEMORY_SCFG_BANKS*FMEMORY_SCFG_BANK_SIZE)) // 0x7C000
//-----------------------------------------------------------------------------
enum eFMEMORY_ERRORS {
	FMEM_NOT_FOUND = -1,	//  -1 - не найден
	FMEM_FLASH_ERR = -2,	//  -2 - flash rd/wr/erase error
	FMEM_ERROR = 	-3,		//  -3 - error
	FMEM_OVR_ERR = 	-4,		//  -4 - переполнение FMEMORY_SCFG_BANK_SIZE
	FMEM_MEM_ERR = 	-5		//  -5 - heap alloc error
};
//-----------------------------------------------------------------------------

#define MAX_FOBJ_SIZE 64 // максимальный размер сохраняемых объeктов (32,64,..512)

signed short flash_read_cfg(void *ptr, unsigned short id, unsigned short maxsize); // возврат: размер объекта последнего сохранения, -1 - не найден, -2 - error
bool flash_write_cfg(void *ptr, unsigned short id, unsigned short size);
bool flash_supported_eep_ver(unsigned int min_ver, unsigned int new_ver);
//-----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif


#endif /* __FLASH_EEP_H_ */
