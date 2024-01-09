/*************
 ota_protocol.h
 SDK_LICENSE
***************/

#ifndef _OTA_PROTOCOL_H
#define _OTA_PROTOCOL_H

#define MAX_OTA_PARAM_SIZE 256
//#define OTA_BLOCK_SIZE    1024

#define OTA_DATA_BURST_SIZE (((uint32_t)s_ota_ctx.mtu_a) * ((uint32_t)s_ota_burst_size))

enum
{

    OTA_CMD_START_OTA = 1,
    OTA_CMD_PARTITION_INFO,
    OTA_CMD_BLOCK_INFO,
    OTA_CMD_REBOOT,
    OTA_CMD_ERASE,
    OTA_CMD_SEC_CONFIRM,
    OTA_CMD_RND_CHANGE,
    OTA_CMD_VERIFY_KEY,
    //OTA_CMD_VERIFY_INFO,


    OTA_RSP_START_OTA = 0x81,       //81
    OTA_RSP_PARAM,              //82
    OTA_RSP_OTA_COMPLETE,       //83
    OTA_RSP_PARTITION_INFO,     //84
    OTA_RSP_PARTITION_COMPLETE, //85
    OTA_RSP_BLOCK_INFO,         //86
    OTA_RSP_BLOCK_BURST,        //87
    OTA_RSP_BLOCK_COMPLETE,     //88
    OTA_RSP_ERASE,              //89
    OTA_RSP_REBOOT,             //8a
    OTA_RSP_SEC_CONFIRM,                //8b
    OTA_RSP_RND_CHANGE,                 //8c
    OTA_RSP_VERIFY_KEY,             //8d
    //OTA_RSP_VERIFY_INFO,              //8e
    OTA_RSP_ERROR = 0xff,

};

typedef struct
{
    uint8_t cmd;

    union
    {

        uint8_t random[16];

        uint8_t ver_key[16];
        uint8_t confirm[16];
        struct
        {
            uint8_t sector_num;
            uint8_t burst_size;
        } start;
        struct
        {
            uint8_t index;
            uint8_t flash_addr[4];
            uint8_t run_addr[4];
            uint8_t size[4];
            uint8_t checksum[4];
            //uint8_t mic[4];
        } part;
//              struct
//        {
//            uint8_t index;
//            uint8_t flash_addr[4];
//            uint8_t run_addr[4];
//            uint8_t size[4];
//            uint8_t checksum[4];
//        } part_sec;
        struct
        {
            uint8_t size[2]; //max 1024
            uint8_t index;
        } block;

        struct
        {
            uint8_t flash_addr[4];
            uint8_t size[4];
        } erase;

        uint8_t reboot_flag;

    } p; //parameter
} ota_cmd_t;

extern const char* OTA_CRYPTO_IV;
extern bool aes_ccm_phyplus_dec(const unsigned char* iv, unsigned char* din, int dLen, unsigned char* micIn, unsigned char* dout);

void otaProtocol_mtu(uint16_t mtu);
void otaProtocol_TimerEvt(void);
bool otaProtocol_address_plus(void);
void otaProtocol_BootMode(void);
void otaProtocol_RunApp(void);
int otaProtocol_init(uint8_t task_id, uint16_t tm_evt);

#endif
