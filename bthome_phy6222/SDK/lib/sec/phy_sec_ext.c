/***************************
  phy_sec_ext.c
 
 SDK_LICENSE

***************************/

#include "flash.h"
#include "log.h"
#include "error.h"
#include "OSAL.h"
#include "mcu.h"
#include "aes.h"
#include "version.h"

#define CHIP_ID_FLASH_ADDRESS           0x11000800
#define CHIP_MADDR_FLASH_ADDRESS        (CHIP_ID_FLASH_ADDRESS+CHIP_ID_LENGTH*4)
#define CHIP_VERSION_L                  SDK_VER_CHIP
#define CHIP_VERSION_H                  (0x0BBB)

#define CHIP_BOOT_AREA_ADDR             0x3000
#define CHIP_BOOT_PART_SEC(n)           ((*(volatile uint32_t*)(SPIF_BASE_ADDR + CHIP_BOOT_AREA_ADDR+0x104+(n)*0x10))&0x80000000)
#define CHIP_SECURE_AREA_ADDR           (0x2900)
#define CHIP_SECURE_AUTH_WORD           (*(volatile uint32_t*)(SPIF_BASE_ADDR + CHIP_SECURE_AREA_ADDR ))
#define CHIP_SECURE_USE_EFUSE           (*(volatile uint32_t*)(SPIF_BASE_ADDR + CHIP_SECURE_AREA_ADDR + 0x4))
#define CHIP_SECURE_KEY_L               (*(volatile uint32_t*)(SPIF_BASE_ADDR + CHIP_SECURE_AREA_ADDR + 0x8))
#define CHIP_SECURE_KEY_H               (*(volatile uint32_t*)(SPIF_BASE_ADDR + CHIP_SECURE_AREA_ADDR + 0xc))
#define pCHIP_SECURE_PLAINTEXT          ((volatile uint8_t*)(SPIF_BASE_ADDR + CHIP_SECURE_AREA_ADDR + 0x10))
#define pCHIP_SECURE_MIC                ((volatile uint8_t*)(SPIF_BASE_ADDR + CHIP_SECURE_AREA_ADDR + 0x20))

#if 0
    chipId_t g_chipId;
    chipMAddr_t  g_chipMAddr;
#endif

const char* s_company_id = "PHY+62XXPLUS0504";

uint8_t MID, JID8, JID0;
enum
{
    OTA_SEC_KEY_NULL = 0,
    OTA_SEC_KEY_CCM = 1,
    OTA_SEC_KEY_FAIL = 0xff,
};

typedef enum
{
    EFUSE_BLOCK_0 = 0,
    EFUSE_BLOCK_1 = 1,
    EFUSE_BLOCK_2 = 2,
    EFUSE_BLOCK_3 = 3,

} EFUSE_block_t;
//extern int efuse_lock(EFUSE_block_t block);
extern int efuse_read(EFUSE_block_t block,uint32_t* buf);
extern int efuse_write(EFUSE_block_t block,uint32_t* buf,uint32_t us);

extern uint8 osal_memcmp( const void GENERIC* src1, const void GENERIC* src2, unsigned int len );
uint32_t g_ota_sec_key[4]= {0,0,0,0};
uint8_t g_ota_sec_key_valid = OTA_SEC_KEY_NULL;

extern void LL_ENC_AES128_Encrypt0( uint8* key,uint8* plaintext,uint8* ciphertext );
#define LL_ENC_AES128_Encrypt_X             LL_ENC_AES128_Encrypt0

const uint32_t ROM_Base4K[4]= {0x47704800,0x00017D00,0xB082B5F7,0x27009C02};
uint8_t PHY_Varify_Platform()
{
    uint8_t ret = TRUE;

    for(uint8 i=0; i<4; i++)
    {
        if( ROM_Base4K[i] != read_reg( 0x1000 + i*4 ) )
        {
            ret = FALSE;
            break;
        }
    }

    return ret;
}

extern int flash_load_parition(unsigned char* pflash, int size, unsigned char* micIn,unsigned char* run_addr);

bool _efuse_chip_version_check(void)
{
    uint32_t buf[2];
    //uint8_t key[16];
    efuse_read(EFUSE_BLOCK_1,buf);
    buf[0]= CHIP_VERSION_L;
    buf[1]= CHIP_VERSION_H;

    if(buf[0]==CHIP_VERSION_L && buf[1]==CHIP_VERSION_H)
    {
        if(finidv()==TRUE)
        {
            //ota_sec_efuse_lock();
            return 1;
        }

        return 1;
    }
    else
    {
        return 0;
    }
}

void efuse_init(void)
{
    write_reg(0x4000f054,0x0);
    write_reg(0x4000f140,0x0);
    write_reg(0x4000f144,0x0);
}

void _rom_sec_boot_init(void)
{
    efuse_init();

    if(_efuse_chip_version_check())
    {
        typedef void (*my_function)(void);
        my_function pFunc = (my_function)(0xa2e1);
        //ble_main();
        pFunc();
        return;
    }
    else
    {
        while(1);
    }
}

//int ota_sec_efuse_lock(void)
//{
//  efuse_lock(EFUSE_BLOCK_1);
//
//  return PPlus_SUCCESS;
//}

#if (0)
/*******************************************************************************
    @fn          pplus_LoadMACFromChipMAddr

    @brief       Used to load MAC Address from chip Maddr

    input parameters

    @param       None.

    output parameters

    @param       None.

    @return      CHIP_ID_STATUS_e.
*/
CHIP_ID_STATUS_e pplus_LoadMACFromChipMAddr(void)
{
    volatile uint8_t* p_ownPublicAddr = (volatile uint8_t*)0x1fff1231;

    if(g_chipMAddr.chipMAddrStatus==CHIP_ID_VALID)
    {
        for(uint8_t i =0; i<CHIP_MADDR_LEN; i++)
            *(p_ownPublicAddr++) = g_chipMAddr.mAddr[i];
    }

    return g_chipMAddr.chipMAddrStatus;
}






static CHIP_ID_STATUS_e chip_id_one_bit_hot_convter(uint8_t* b,uint32_t w)
{
    uint16 dh = w>>16;
    uint16 dl = w&0xffff;
    uint16 h1,h0,l1,l0;
    h0=l0=0xff;
    h1=l1=0;

    for(int i=0; i<16; i++)
    {
        l1+=((dl&(1<<i))>>i);

        if(l0==0xff && l1==1)
        {
            l0=i;
        }

        h1+=((dh&(1<<i))>>i);

        if(h0==0xff && h1==1)
        {
            h0=i;
        }
    }

    if(l1==1 && h1==1)
    {
        *b=((h0<<4)+l0);
        return CHIP_ID_VALID;
    }
    else if(l1==16 && h1==16)
    {
        return CHIP_ID_EMPTY;
    }
    else
    {
        return CHIP_ID_INVALID;
    }
}

CHIP_ID_STATUS_e read_chip_id(void)
{
    CHIP_ID_STATUS_e ret = CHIP_ID_UNCHECK;
    uint8_t b;

    for(int i=0; i<CHIP_ID_LENGTH; i++)
    {
        ret = chip_id_one_bit_hot_convter(&b, read_reg(CHIP_ID_FLASH_ADDRESS+(i<<2)));

        if(ret==CHIP_ID_VALID)
        {
            *(&g_chipId.pid[0]+i)=b;
        }
        else
        {
            if(i>0 && ret==CHIP_ID_EMPTY)
            {
                ret =CHIP_ID_INVALID;
            }

            return ret;
        }
    }

    return ret;
}

void check_chip_id(void)
{
    //============================================================
    //chip id check
    for(int i=0; i<CHIP_ID_LENGTH; i++)
    {
        *(&g_chipId.pid[0]+i)=0xff;
    }

    g_chipId.chipIdStatus=read_chip_id();
}

CHIP_ID_STATUS_e read_chip_mAddr(void)
{
    CHIP_ID_STATUS_e ret = CHIP_ID_UNCHECK;
    uint8_t b;

    for(int i=0; i<CHIP_MADDR_LEN; i++)
    {
        ret = chip_id_one_bit_hot_convter(&b, read_reg(CHIP_MADDR_FLASH_ADDRESS+(i<<2)));

        if(ret==CHIP_ID_VALID)
        {
            g_chipMAddr.mAddr[CHIP_MADDR_LEN-1-i]=b;
        }
        else
        {
            if(i>0 && ret==CHIP_ID_EMPTY)
            {
                ret =CHIP_ID_INVALID;
            }

            return ret;
        }
    }

    return ret;
}

void check_chip_mAddr(void)
{
    //============================================================
    //chip id check
    for(int i=0; i<CHIP_MADDR_LEN; i++)
    {
        g_chipMAddr.mAddr[i]=0xff;
    }

    g_chipMAddr.chipMAddrStatus=read_chip_mAddr();
}
/*******************************************************************************
    @fn          finidv

    @brief       This function is used to find aes crypto iv.

    input parameters

    @param       none.

    output parameters

    @param       v - Pointer to the output iv.

    @return      Boolean to indicate whether use crypto:
                TRUE:  Success
                FALSE: Not Done
*/
bool finidv(unsigned char* iv)
{
    int i = 0;
    unsigned int tmp = 0;
    unsigned short a, b, c;
    unsigned char* v = iv;

    if(v == NULL)
        v = (unsigned char*)0x200127e0;

    a = (unsigned short)(read_reg(0x11002300)&0xffff);
    b = c = 0;

    if(a== 0xffff)
        return FALSE;

    if(a == 1)//case chip id
    {
        if(read_chip_id() != CHIP_ID_VALID)
            return FALSE;

        osal_memcpy(v, &(g_chipId.tid[4]), 10);
        v[10] = g_chipId.sid[2];
        v[11] = g_chipId.sid[3];
        v[12] = g_chipId.sid[7];
    }
    else
    {
        for (i = 0; i<13; i++)
        {
            b = (((a >> 14) ^ (a >> 13)) & 1);
            a = ((a << 1) | b) & 0x7fff;
            c = c + 1 + (((a >> 8) ^ a) & 0x3f);
            tmp = read_reg(0x11002308 + (c&0xfffc));
            v[i] = (unsigned char)(tmp >> ((c&3)*8));
        }
    }

    return TRUE;
}
#endif
/****************************************************************************
    Function Name  : finidv
    Description    :
    Output         :
                 : *iv,  13 byte
    Return         : bool, if has crypto,return true
****************************************************************************/
bool finidv()
{
    int ret,i;
    uint32_t temp_sec_key[4];
    uint32_t cyr_out[4];    //phase1: validate phy_sec_mic, phase2: generate p_phy_sec_key
    uint32_t efuse[2] = {0, 0};

    if(g_ota_sec_key_valid == OTA_SEC_KEY_CCM)
    {
        return TRUE;
    }

    ret = efuse_read(EFUSE_BLOCK_1, efuse);

    if(ret!=0)
    {
        return FALSE;
    }

    temp_sec_key[0] = efuse[0];
    temp_sec_key[1] = efuse[1];
    temp_sec_key[2] = CHIP_SECURE_KEY_L;
    temp_sec_key[3] = CHIP_SECURE_KEY_H;
    //validate phy_sec_mic
    LL_ENC_AES128_Encrypt_X((uint8_t* )temp_sec_key, (uint8_t* ) pCHIP_SECURE_PLAINTEXT, (uint8_t* ) cyr_out);

    if(osal_memcmp((uint8_t*) cyr_out, (const void*)pCHIP_SECURE_MIC, 128/8) == 0)
    {
        //g_phy_sec_key_valid = PHY_SEC_KEY_FAIL;
        g_ota_sec_key_valid = OTA_SEC_KEY_FAIL;
        return FALSE;
    }

    //generate g_phy_sec_key
    g_ota_sec_key_valid = OTA_SEC_KEY_CCM;
    LL_ENC_AES128_Encrypt_X((uint8_t* )temp_sec_key, (uint8_t* ) pCHIP_SECURE_MIC, (uint8_t* ) g_ota_sec_key);
    efuse[0] = 0;
    efuse[1] = 0;

    for(i=0; i<4; i++)
    {
        temp_sec_key[i] = 0;
    }

    return TRUE;
}
bool verify_mic(unsigned char* buf, int size)
{
    //unsigned char key[16];
    bool is_encrypt = FALSE;
    is_encrypt = finidv();

    if(is_encrypt == FALSE)
    {
        return TRUE;
    }

    flash_load_parition(buf,size-4, buf+ size-4, NULL);
    return TRUE;
    //return aes_ccm_phyplus_dec(iv, buf,size-4,buf+ size-4,NULL);
}



#if 0
void LOG_CHIP_ID(void)
{
    LOG("\n");

    if(g_chipId.chipIdStatus==CHIP_ID_EMPTY)
    {
        LOG("[CHIP_ID EMPTY]\n");
    }
    else if(g_chipId.chipIdStatus==CHIP_ID_INVALID)
    {
        LOG("[CHIP_ID INVALID]\n");
    }
    else if(g_chipId.chipIdStatus==CHIP_ID_VALID)
    {
        LOG("[CHIP_ID VALID]\n");
        volatile uint8_t chipIdStr[CHIP_ID_LENGTH+1];

        for(int i=0; i<CHIP_ID_LENGTH; i++)
        {
            LOG("%x",*(&g_chipId.pid[0]+i));
            chipIdStr[i]=*(&g_chipId.pid[0]+i);
        }

        chipIdStr[CHIP_ID_LENGTH]='\0';
        LOG("\n");
        LOG("%s",chipIdStr);
    }
    else
    {
        LOG("[CHIP_ID UNCHECKED]\n");
    }
}

void LOG_CHIP_MADDR(void)
{
    LOG("\n");

    if(g_chipMAddr.chipMAddrStatus==CHIP_ID_EMPTY)
    {
        LOG("[CHIP_MADDR EMPTY]\n");
    }
    else if(g_chipMAddr.chipMAddrStatus==CHIP_ID_INVALID)
    {
        LOG("[CHIP_MADDR INVALID]\n");
    }
    else if(g_chipMAddr.chipMAddrStatus==CHIP_ID_VALID)
    {
        LOG("[CHIP_MADDR VALID]\n");

        for(int i=0; i<CHIP_MADDR_LEN; i++)
        {
            LOG("%02x",g_chipMAddr.mAddr[i]);
        }

        LOG("\n");
    }
    else
    {
        LOG("[CHIP_MADDR UNCHECKED]\n");
    }
}
#endif

