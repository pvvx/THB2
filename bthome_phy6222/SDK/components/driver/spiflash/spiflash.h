/*************
 spiflash.h
 SDK_LICENSE
***************/

#ifndef __SPIFLASH_H__
#define __SPIFLASH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "spi.h"

/*gd25q16 cmd define*/
#define FLASH_WREN                      0x06
#define FLASH_WRDIS                     0x04
#define FLASH_CE                            0x60
//#define FLASH_CE                      0xC7

#define FLASH_DP                            0xB9
#define FLASH_RDI                           0xAB
#define FLASH_SE                            0x20
#define FLASH_BE_32KB                   0x52
#define FLASH_BE_64KB                   0xD8
#define FLASH_WRSR                      0x01
#define FLASH_RDID                      0x9F
#define FLASH_RDSR_LOW              0x05
#define FLASH_RDSR_HIGH             0x35
#define FLASH_PP                            0x02
#define FLASH_READ                      0x03

#define FLASH_PES                                               0x75
#define FLASH_PER                                               0x7A

extern uint32_t spiflash_space;

typedef enum
{
    FLASH_ERROR = 0,
    FLASH_IDLE = 1,
    FLASH_ERASING = 2,
    FALSH_DATA_WRITING = 3,
    FLASH_STATUS_WRITEING = 4,
    FLASH_READING = 5
} FLASH_STATUS_e;

extern hal_spi_t spiflash_spi;

/*gd25q16 api for hardware debug*/
uint32_t spiflash_read_identification(void);
uint16_t spiflash_read_status_register(uint8_t bitsSel);
bool spiflash_bus_busy(void);
void spiflash_program_erase_suspend(void);
void spiflash_program_erase_resume(void);
void spiflash_deep_powerdown(void);
void spiflash_release_from_powerdown(void);

void spiflash_write_enable(void);
void spiflash_write_disable(void);
void spiflash_chip_erase(void);
void spiflash_sector_erase(uint32_t addr);
void spiflash_block_erase_32KB(uint32_t addr);
void spiflash_block_erase_64KB(uint32_t addr);
void spiflash_write_status_register(uint8_t data);

void spiflash_write(uint32_t addr,uint8_t* tx_buf,uint16_t tx_len);
void spiflash_read(uint32_t addr,uint8_t* rx_buf,uint16_t rx_len);

int spiflash_init(void);

/*gd25q16 api for user develop*/
int vendorflash_init(void);
int vendorflash_read(uint32_t addr,uint8_t* data,uint16_t len);
int vendorflash_erase(uint32_t addr,uint32_t len);
int vendorflash_write(uint32_t addr,const uint8_t* data,uint16_t len);

#ifdef __cplusplus
}
#endif


#endif
