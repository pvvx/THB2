/*******************************************************************************
    @file     ali_genie_profile.c
    @brief    Contains all functions support for ali genie profile
    @version  1.0
    @date     28. Feb. 2019
    @author   Zhongqi Yang

 SDK_LICENSE

*******************************************************************************/
#include "sha256.h"
#include "flash.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ali_genie_profile.h"

#define ALI_GENIE_PID_LEN               4
#define ALI_GENIE_SEC_LEN               16


#define VENDOR_PRODUCT_ID_ADDR          0x4030
#define VENDOR_PRODUCT_SECRERT_ADDR     0x4010
#define VENDOR_PRODUCT_MAC_ADDR         0x4000




unsigned char ali_genie_pid[4]  = {0x00};
unsigned char ali_genie_mac[6] = {0x00};
unsigned char ali_genie_macStr[12];

unsigned char ali_genie_sec[16] = {0x00};
unsigned char ali_genie_auth[16]= {0x00};

void hex2Str( unsigned char* pIn,unsigned char* pOut,int len,int reverse)
{
    int       i;
    unsigned char hex[] = "0123456789abcdef";

    // Start from end of addr
    if(reverse)
    {
        pIn += len;

        for ( i = len; i > 0; i-- )
        {
            *pOut++ = hex[*--pIn >> 4];
            *pOut++ = hex[*pIn & 0x0F];
        }
    }
    else
    {
        for ( i = 0; i <len; i++ )
        {
            *pOut++ = hex[*pIn >> 4];
            *pOut++ = hex[*pIn++ & 0x0F];
        }
    }
}



int gen_aligenie_auth_val(void)
{
    int i, buflen, ret = 0;
    //unsigned char buf[]="000293e2,abcdf0f1f2f3,53daed805bc534a4a93c825ed20a7063";
    unsigned char buf[54];
//    unsigned char pidStr[8];
//    unsigned char macAddrStr[12];
//    unsigned char secStr[32];
    unsigned char sha256sum[32];
    mbedtls_sha256_context ctx;

    //load ProductID
    for(i=0; i<4; i++)
        hal_flash_read(VENDOR_PRODUCT_ID_ADDR+i,ali_genie_pid+i,1);

//        ali_genie_pid[i]=(uint8_t)ReadFlash(VENDOR_PRODUCT_ID_ADDR+i);
    hex2Str(ali_genie_pid,buf,4,0);
    printf("\n ===  PID  === \n");

    for(i=0; i<4; i++)
        printf("%02X ",ali_genie_pid[i]);

    printf("\n");
    buf[8]=0x2c;
    //load MAC
    uint32 address = VENDOR_PRODUCT_MAC_ADDR;
    hal_flash_read(address ++,&ali_genie_mac[3],1);
    hal_flash_read(address ++,&ali_genie_mac[2],1);
    hal_flash_read(address ++,&ali_genie_mac[1],1);
    hal_flash_read(address ++,&ali_genie_mac[0],1);
    hal_flash_read(address ++,&ali_genie_mac[5],1);
    hal_flash_read(address ++,&ali_genie_mac[4],1);
    printf("\n ===  MAC  === \n");

    for(i=0; i<6; i++)
        printf("%02X ",ali_genie_mac[5-i]);

    printf("\n");
    //hex2Str(ali_genie_mac,buf+9,6,1);
    hex2Str(ali_genie_mac,ali_genie_macStr,6,1);

    for(i=0; i<12; i++)
        buf[9+i]=ali_genie_macStr[i];

    buf[21]=0x2c;

    //load secert
    for(i=0; i<16; i++)
        hal_flash_read(VENDOR_PRODUCT_SECRERT_ADDR+i,ali_genie_sec+i,1);

//        ali_genie_sec[i]=(uint8_t)ReadFlash(VENDOR_PRODUCT_SECRERT_ADDR+i);
    hex2Str(ali_genie_sec,buf+22,16,0);
    mbedtls_sha256_init( &ctx );

    if( ( ret = mbedtls_sha256_starts_ret( &ctx, 0 ) ) != 0 )
        goto fail;

    buflen = 54;

//    printf("\n input %d ",buflen);
//    for(i=0;i<buflen;i++)
//        printf("%02x ",buf[i]);
//
//    printf("\n");

    if( (ret = mbedtls_sha256_update_ret( &ctx, buf, buflen )) != 0 )
        goto fail;

    if( ( ret = mbedtls_sha256_finish_ret( &ctx, sha256sum ) ) != 0 )
        goto fail;

//    for(i=0;i<32;i++)
//        printf("%02x ",sha256sum[i]);
    goto exit;
fail:
    printf( "failed\n" );
exit:
    //mbedtls_sha256_free( &ctx );

    for(i=0; i<16; i++)
    {
        ali_genie_auth[i]=sha256sum[i];
    }

    return( ret );
}

