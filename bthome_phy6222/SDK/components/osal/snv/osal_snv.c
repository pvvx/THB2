/*************
 osal_snv.c
 SDK_LICENSE
***************/

#include <stdint.h>
#include "OSAL.h"
#include "flash.h"
#include "error.h"
#include "osal_snv.h"
#include "log.h"

#ifndef USE_FS
    #define USE_FS 1
#endif

#if (USE_FS == 0)

#define NVM_BASE_ADDR           0x1103C000  //16K bytes

uint8 osal_snv_init( void )
{
    return PPlus_ERR_FATAL;
}

uint8 osal_snv_read( osalSnvId_t id, osalSnvLen_t len, void* pBuf)
{
    (void)(id);
    (void)(len);
    (void)(pBuf);
    return PPlus_ERR_FATAL;
}

uint8 osal_snv_write( osalSnvId_t id, osalSnvLen_t len, void* pBuf)
{
    (void)(id);
    (void)(len);
    (void)(pBuf);
    return PPlus_ERR_FATAL;
}

uint8 osal_snv_compact( uint8 threshold )
{
	(void) threshold;
    return SUCCESS;
}

#else

#include "fs.h"

uint8 osal_snv_init( void )
{
    if(!hal_fs_initialized())
        return NV_OPER_FAILED;

    return SUCCESS;
}

uint8 osal_snv_read( osalSnvId_t id, osalSnvLen_t len, void* pBuf)
{
    int ret;
    // LOG("osal_snv_read:%x\n",id);
    ret = hal_fs_item_read((uint16_t)id,(uint8_t*) pBuf, (uint16_t)len,NULL);

    if(ret != PPlus_SUCCESS)
    {
        // LOG("rd_ret:%d\n",ret);
        return NV_OPER_FAILED;
    }

    // LOG_DUMP_BYTE(pBuf, len);
    return SUCCESS;
}

uint8 osal_snv_write( osalSnvId_t id, osalSnvLen_t len, void* pBuf)
{
    int ret = PPlus_SUCCESS;
    // LOG("osal_snv_write:%x,%d\n",id,len);
    // LOG_DUMP_BYTE(pBuf, len);

    if(hal_fs_get_free_size() < len+32)
    {
        if(hal_fs_get_garbage_size(NULL) > len+32)
        {
            hal_fs_garbage_collect();
        }
        else
        {
            return NV_OPER_FAILED;
        }
    }

    ret = hal_fs_item_write((uint16_t) id, (uint8_t*) pBuf, (uint16_t) len);

    if(ret !=0)
    {
        // LOG("wr_ret:%d\n",ret);
        return NV_OPER_FAILED;
    }

    //LOG("Success\n");
    return SUCCESS;
}

uint8 osal_snv_compact( uint8 threshold )
{
    return 0;
}

#endif

