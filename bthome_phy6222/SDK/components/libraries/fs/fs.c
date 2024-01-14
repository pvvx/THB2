/*******************************************************************************
    @file     fs.c
    @brief    Contains all functions support for spi driver
    @version  0.0
    @date     18. Oct. 2017
    @author

 SDK_LICENSE

*******************************************************************************/
#include "rom_sym_def.h"
#include "OSAL.h"
#include "fs.h"
#include "flash.h"
#include "error.h"
#include "log.h"

//#define FS_DBBUG
#ifdef FS_DBBUG
    #define FS_LOG  LOG
#else
    #define FS_LOG(...)
#endif


static uint8_t  fs_sector_num;
static uint32_t fs_offset_address;

#define FS_ITEM_LEN_16BYTE 0
#define FS_ITEM_LEN_32BYTE 1
#define FS_ITEM_LEN_64BYTE 2

#ifndef FS_SETTING
    #define FS_SETTING FS_ITEM_LEN_16BYTE
#endif

#if (FS_SETTING == FS_ITEM_LEN_16BYTE)
    #define FS_ITEM_LEN                                                             16
#elif (FS_SETTING == FS_ITEM_LEN_32BYTE)
    #define FS_ITEM_LEN                                                             32
#elif (FS_SETTING == FS_ITEM_LEN_64BYTE)
    #define FS_ITEM_LEN                                                             64
#else
    #error please check your config parameter
#endif

/*
    fs struct:
    sector0
    sector_head
    file_head(4byte)+file_data
    ...
    file_head(4byte)+file_data
*/
//please do not modify the following parameters
#define FS_ITEM_HEAD_LEN                          4
#define FS_ITEM_DATA_LEN                          (FS_ITEM_LEN - FS_ITEM_HEAD_LEN)
#define FS_SECTOR_ITEM_NUM                        (4096/FS_ITEM_LEN - 1)
#define FS_SECTOR_NUM_BUFFER_SIZE                 (312/4)
#define FS_ABSOLUTE_ADDR(offset)                  (fs.cfg.sector_addr + offset)

typedef enum
{
    ITEM_DEL    = 0x00,//zone is deleted
    ITEM_UNUSED = 0x03,//zone is free
    ITEM_USED   = 0x02,//zone is used
    ITEM_RESERVED = 0x01//to be extend
} item_pro;

typedef enum
{
    ITEM_SF = 0x03,//single frame file
    ITEM_MF_F = 0x01,//multiple frame file,first frame
    ITEM_MF_C   = 0x02,//multiple frame file,continue frame
    ITEM_MF_E = 0x00//multiple frame file,end frame
} item_frame;

typedef enum
{
    FLASH_UNCHECK = 0,//before analysis fs
    FLASH_NEW = 1,//new fs,its are 0xFF
    FLASH_ORIGINAL_ORDER = 2,//fs has data,its order is the original
    FLASH_NEW_ORDER = 3,//fs has data,its order is not the original
    FLASH_CONTEXT_ERROR = 4,//fs has data,but data is broken
} FS_FLASH_TYPE;

/*
    file head struct:
    len(12bit)+frame(2bit)+pro(2bit)+id(16bit)
*/
typedef union
{
    struct
    {
        uint32_t id:16;//file id
        uint32_t pro:2;//file property
        uint32_t frame:2;//file frame
        uint32_t len:12;//file length
    } b;
    uint32_t reg;
} fs_item_t;

/*
    sector head struct:
    sector_addr(one word)+(ff+index+item_len+sector_num)(one word)+(0xffffffff)(one word)~(0xffffffff)(one word)
*/
typedef struct
{
    uint32_t sector_addr;//fs start address
    uint8_t  sector_num;//fs sector number
    uint8_t  item_len;//item length
    uint8_t  index;//sector index
    uint8_t  reserved[FS_ITEM_LEN-7];
} fs_cfg_t;

typedef struct
{
    fs_cfg_t    cfg;
    uint8_t current_sector;//free sector index
    uint8_t exchange_sector;//exchange sector,only use it when garbage collect
    uint16_t offset;//free position in free sector index
} fs_t;

static fs_t fs;
static bool fs_init_flag = false;

typedef enum
{
    SEARCH_FREE_ITEM = 0,
    SEARCH_APPOINTED_ITEM = 1,
    SEARCH_DELETED_ITEMS = 2,
} search_type;

extern uint32_t __psr(void);//check if in int process

static void fs_erase_ucds_all_sector(void)
{
    int i;

    for(i=0; i<fs_sector_num; i++)
        hal_flash_erase_sector(fs_offset_address + (i*4096));
}

static void fs_erase_ucds_one_sector(uint32_t addr_erase)
{
    hal_flash_erase_sector(fs_offset_address + addr_erase);
}

static int fs_spif_write(uint32_t addr,uint8_t* value,uint16_t len)
{
    if(__psr()&0x3f)
    {
        return PPlus_ERR_FS_IN_INT;
    }

    if((addr < fs_offset_address) || (addr >= (fs_offset_address+fs_sector_num*4096)) || ((addr&0x03)>0))
    {
        return PPlus_ERR_FS_PARAMETER;
    }

    return(hal_flash_write(addr,value,(uint32_t)len));
}

static uint32_t fs_spif_read(uint32_t addr,uint8_t* buf,uint32_t len)
{
    if ((addr < fs_offset_address) || (addr >= (fs_offset_address + fs_sector_num*4096)) || (buf == NULL) || (len == 0))
    {
        return PPlus_ERR_FS_PARAMETER;
    }

    return( hal_flash_read(addr,buf,len) );
}

static void check_addr(uint32_t* addr)
{
    if((*addr % 4096) == 0)
    {
        *addr += sizeof(fs_cfg_t);

        if(*addr >= 4096 *fs_sector_num)
            *addr -= 4096 *fs_sector_num;
    }
}

static int fs_search_items(search_type type,uint32_t* para1,uint32_t* para2)
{
    uint8_t m,n;
    uint16_t j,g_offset = 1;
    uint32_t sector_addr,ab_addr;
    fs_item_t i1;
    bool from_last_sector = false;

    for(m = 1; m < fs_sector_num; m++)
    {
        n = (m + fs.exchange_sector) % fs.cfg.sector_num;

        if(g_offset >= FS_SECTOR_ITEM_NUM)
        {
            g_offset -= FS_SECTOR_ITEM_NUM;

            if(g_offset >= FS_SECTOR_ITEM_NUM)
            {
                continue;
            }
        }

        if(SEARCH_FREE_ITEM == type)
            fs.current_sector = (m + fs.exchange_sector) % fs.cfg.sector_num;

        sector_addr = n * 4096;

        for(j = 1; j <= FS_SECTOR_ITEM_NUM; j++)
        {
            if(from_last_sector == true)
            {
                from_last_sector = false;
                j += g_offset;
            }
            else
            {
                if(g_offset > 1)
                {
                    if((j - 2 + g_offset) < FS_SECTOR_ITEM_NUM)
                    {
                        j = j - 1 + g_offset;
                    }
                    else
                    {
                        g_offset -= (FS_SECTOR_ITEM_NUM + 2 - j);
                        from_last_sector = true;
                        break;
                    }
                }
            }

            ab_addr = sector_addr + (j * FS_ITEM_LEN);
            fs_spif_read(FS_ABSOLUTE_ADDR(ab_addr),(uint8_t*)&i1,FS_ITEM_HEAD_LEN);

            switch(type)
            {
            case SEARCH_FREE_ITEM:
            {
                switch(i1.b.pro)
                {
                case ITEM_DEL:
                case ITEM_USED:
                {
                    if(i1.b.frame == ITEM_MF_F)
                        g_offset = (i1.b.len/FS_ITEM_DATA_LEN) + ((i1.b.len%FS_ITEM_DATA_LEN)?1:0);
                    else
                        g_offset = 1;
                }
                break;

                case ITEM_UNUSED:
                    *para1 = ab_addr%4096;
                    return PPlus_SUCCESS;

                default:
                    break;
                }
            }
            break;

            case SEARCH_APPOINTED_ITEM:
            {
                switch(i1.b.pro)
                {
                case ITEM_DEL:
                case ITEM_USED:
                    if((ITEM_USED == i1.b.pro) && (i1.b.id == (uint16)(*para1)))
                    {
                        *para2 = ab_addr;
                        return PPlus_SUCCESS;
                    }
                    else
                    {
                        if(i1.b.frame == ITEM_MF_F)
                            g_offset = (i1.b.len/FS_ITEM_DATA_LEN) + ((i1.b.len%FS_ITEM_DATA_LEN)?1:0);
                        else
                            g_offset = 1;
                    }

                    break;

                case ITEM_UNUSED:
                    return PPlus_ERR_FS_NOT_FIND_ID;

                default:
                    break;
                }
            }
            break;

            case SEARCH_DELETED_ITEMS:
            {
                switch(i1.b.pro)
                {
                case ITEM_DEL:
                {
                    if(i1.b.frame == ITEM_MF_F)
                    {
                        g_offset = (i1.b.len/FS_ITEM_DATA_LEN) + ((i1.b.len%FS_ITEM_DATA_LEN)?1:0);
                        *para1 += g_offset*FS_ITEM_DATA_LEN;
                    }
                    else
                    {
                        g_offset = 1;
                        *para1 += FS_ITEM_DATA_LEN;
                    }

                    *para2 += 1;
                }
                break;

                case ITEM_USED:
                {
                    if(i1.b.frame == ITEM_MF_F)
                    {
                        g_offset = (i1.b.len/FS_ITEM_DATA_LEN) + ((i1.b.len%FS_ITEM_DATA_LEN)?1:0);
                    }
                    else
                    {
                        g_offset = 1;
                    }
                }
                break;

                case ITEM_UNUSED:
                    return PPlus_SUCCESS;

                default:
                    break;
                }
            }
            break;

            default:
                return PPlus_ERR_INVALID_PARAM;
            }
        }
    }

    switch(type)
    {
    case SEARCH_FREE_ITEM:
        return PPlus_ERR_FS_FULL;

    case SEARCH_APPOINTED_ITEM:
        return PPlus_ERR_FS_NOT_FIND_ID;

    default:
        return PPlus_SUCCESS;
    }
}

static int fs_get_free_item(void)
{
    int ret;
    uint32_t posiztion = 0;

    if(fs_init_flag == false)
        return  PPlus_ERR_FS_UNINITIALIZED;

    ret = fs_search_items(SEARCH_FREE_ITEM,&posiztion,0);

    if(PPlus_SUCCESS == ret)
    {
        fs.offset = posiztion;
    }
    else if(PPlus_ERR_FS_FULL == ret)
    {
        fs.offset = 4096;
    }

    return ret;
}

static int fs_init(void)
{
    uint8_t i = 0,sector_order[FS_SECTOR_NUM_BUFFER_SIZE],ret = PPlus_ERR_FS_UNINITIALIZED;
    FS_FLASH_TYPE flash = FLASH_UNCHECK;
    fs_cfg_t flash_rd_cfg;
    fs.cfg.sector_addr = fs_offset_address;
    fs.cfg.sector_num = fs_sector_num;;
    fs.cfg.index = 0xff;
    fs.cfg.item_len = FS_ITEM_LEN;
    osal_memset((fs.cfg.reserved),0xff,(FS_ITEM_LEN-7)*sizeof(uint8_t));
    osal_memset((sector_order),0x00,FS_SECTOR_NUM_BUFFER_SIZE);
    FS_LOG("fs_init:\n");

    for(i = 0; i < fs.cfg.sector_num; i++)
    {
        fs_spif_read(FS_ABSOLUTE_ADDR(4096*i),(uint8_t*)(&flash_rd_cfg),sizeof(fs_cfg_t));
        FS_LOG("flash_rd_cfg.sector_addr:%x\n",flash_rd_cfg.sector_addr);
        FS_LOG("flash_rd_cfg.sector_num:%x\n",flash_rd_cfg.sector_num);
        FS_LOG("flash_rd_cfg.item_len:%x\n",flash_rd_cfg.item_len);
        FS_LOG("flash_rd_cfg.index:%x\n",flash_rd_cfg.index);

        if((flash_rd_cfg.sector_addr == fs.cfg.sector_addr) &&
                (flash_rd_cfg.sector_num == fs.cfg.sector_num) &&
                (flash_rd_cfg.item_len == fs.cfg.item_len))
        {
            if(flash_rd_cfg.index < (fs_sector_num - 1))
            {
                if(i == flash_rd_cfg.index)
                {
                    flash = FLASH_ORIGINAL_ORDER;
                    FS_LOG("FLASH_ORIGINAL_ORDER\n");
                }
                else
                {
                    flash = FLASH_NEW_ORDER;
                    FS_LOG("FLASH_NEW_ORDER\n");
                }

                sector_order[i] = flash_rd_cfg.index;
                fs.cfg.index = flash_rd_cfg.index;
            }
            else
            {
                flash = FLASH_CONTEXT_ERROR;
                FS_LOG("FLASH_CONTEXT_ERROR1\n");
                break;
            }
        }
#if 1
		else {
			hal_flash_erase_sector(FS_ABSOLUTE_ADDR(4096*i));
			osal_memset(&flash_rd_cfg, 0xff, sizeof(flash_rd_cfg));
			sector_order[i] = 0xff;
		}
#else
        else if((flash_rd_cfg.sector_addr == 0xffffffff) &&
                (flash_rd_cfg.sector_num == 0xff) &&
                (flash_rd_cfg.item_len == 0xff))
        {
            sector_order[i] = 0xff;
        }
        else
        {
            flash = FLASH_CONTEXT_ERROR;
            FS_LOG("FLASH_CONTEXT_ERROR2\n");
            break;
        }
#endif
    }

    if(flash == FLASH_CONTEXT_ERROR)
    {
        return PPlus_ERR_FS_CONTEXT;
    }

    if(fs.cfg.index == 0xff)
        flash = FLASH_NEW;

    if(flash == FLASH_NEW)
    {
        for(i = 0; i < (fs.cfg.sector_num - 1); i++)
        {
            fs.cfg.index = i;

            if(PPlus_SUCCESS != fs_spif_write((FS_ABSOLUTE_ADDR(4096*i)),(uint8_t*)(&(fs.cfg)),sizeof(fs_cfg_t)))
            {
                FS_LOG("PPlus_ERR_FS_WRITE_FAILED\n");
                return PPlus_ERR_FS_WRITE_FAILED;
            }
        }

        fs.current_sector = 0;
        fs.exchange_sector = fs.cfg.sector_num - 1;
        fs.offset = sizeof(fs_cfg_t);
        fs_init_flag = TRUE;
    }
    else
    {
        for(i = 0; i < fs.cfg.sector_num; i++)
        {
            if(sector_order[i] == 0)
                break;
        }

        if(i < fs.cfg.sector_num)
        {
            fs.exchange_sector = (i + fs.cfg.sector_num - 1) % fs.cfg.sector_num;
            fs_init_flag = TRUE;
            ret = fs_get_free_item();

            if((ret != PPlus_ERR_FS_FULL) && (ret != PPlus_SUCCESS))
            {
                FS_LOG("PPlus_ERR_FS_RESERVED_ERROR\n");
                return PPlus_ERR_FS_RESERVED_ERROR;
            }
        }
    }

    FS_LOG("PPlus_SUCCESS\n");
    return PPlus_SUCCESS;
}

uint32_t hal_fs_get_free_size(void)
{
    uint32_t size = 0;

    if(fs_init_flag == false)
    {
        //LOG("fs_init_flag = false,free\n");
        return 0;
    }

    if(fs.offset < 4096)
    {
        size = ((fs.exchange_sector + fs.cfg.sector_num - fs.current_sector - 1)%fs.cfg.sector_num)*(4096-sizeof(fs_cfg_t));
        size += (4096 - fs.offset);
        size = size*FS_ITEM_DATA_LEN/FS_ITEM_LEN;
    }

    return size;
}

int hal_fs_get_garbage_size(uint32_t* garbage_file_num)
{
    uint32_t garbage_size = 0,garbage_count = 0;
    int ret;

    if(fs_init_flag == false)
        return  PPlus_ERR_FS_UNINITIALIZED;

    ret = fs_search_items(SEARCH_DELETED_ITEMS,&garbage_size,&garbage_count);

    if(PPlus_SUCCESS == ret)
    {
        if(NULL != garbage_file_num)
            *garbage_file_num = garbage_count;

        return garbage_size;
    }

    return PPlus_ERR_FATAL;
}

int hal_fs_item_find_id(uint16_t id,uint32_t* id_addr)
{
    int ret;
    uint32_t file_id = 0;

    if(fs_init_flag == false)
        return  PPlus_ERR_FS_UNINITIALIZED;

    file_id = id & 0xffff;
    ret = fs_search_items(SEARCH_APPOINTED_ITEM,&file_id,id_addr);
    return ret;
}

int hal_fs_item_write(uint16_t id,uint8_t* buf,uint16_t len)
{
    uint8_t frame_len,wr_buf[FS_ITEM_LEN];
    uint16_t i,item_len;
    uint32_t addr;
    fs_item_t i1;

    if(__psr()&0x3f)
    {
        return PPlus_ERR_FS_IN_INT;
    }

    if(fs_init_flag == false)
    {
        //LOG("fs_init_flag = false,write\n");
        return PPlus_ERR_FS_UNINITIALIZED;
    }

    if((buf == NULL) || (len == 0)||(len > 4095))
        return PPlus_ERR_FS_PARAMETER;

    if(len > hal_fs_get_free_size()) {

    	if(hal_fs_get_garbage_size(NULL) > len+32)
            hal_fs_garbage_collect();
        else
        	return PPlus_ERR_FS_NOT_ENOUGH_SIZE;
    }
    //if(hal_fs_item_find_id(id,&addr) == PPlus_SUCCESS)
    //  return PPlus_ERR_FS_EXIST_SAME_ID;
    if(hal_fs_item_find_id(id,&addr) == PPlus_SUCCESS)
    {
        if(PPlus_SUCCESS != hal_fs_item_del(id))
            return PPlus_ERR_FATAL;
    }

    item_len = len;
    i1.b.len = len;
    i1.b.id = id;
    i1.b.pro = ITEM_USED;

    if(len <= FS_ITEM_DATA_LEN)
        i1.b.frame = ITEM_SF;

    i = 0;

    while(len > 0)
    {
        if(len > FS_ITEM_DATA_LEN)
        {
            if(item_len == len)
                i1.b.frame = ITEM_MF_F;
            else
                i1.b.frame = ITEM_MF_C;

            frame_len = FS_ITEM_DATA_LEN;
            len -= FS_ITEM_DATA_LEN;
        }
        else
        {
            if((i1.b.frame == ITEM_MF_C) || (i1.b.frame == ITEM_MF_F))
                i1.b.frame = ITEM_MF_E;

            frame_len = len;
            len = 0;
        }

        addr = FS_ABSOLUTE_ADDR((fs.current_sector * 4096) + fs.offset);
        osal_memcpy(wr_buf,(uint8_t*)(&i1.reg),FS_ITEM_HEAD_LEN);
        osal_memcpy((wr_buf + FS_ITEM_HEAD_LEN),(buf + i),frame_len);

        if(PPlus_SUCCESS != fs_spif_write(addr,wr_buf,(frame_len+FS_ITEM_HEAD_LEN)))
            return PPlus_ERR_FS_WRITE_FAILED;

        i += frame_len;
        fs.offset += FS_ITEM_LEN;

        if(fs.offset == 4096)
        {
            if(((fs.current_sector + 1) % fs.cfg.sector_num) != fs.exchange_sector)
            {
                fs.offset = sizeof(fs_cfg_t);
                fs.current_sector = (fs.current_sector + 1) % fs.cfg.sector_num;
            }
        }
    }

    return PPlus_SUCCESS;
}

int hal_fs_item_read(uint16_t id,uint8_t* buf,uint16_t buf_len,uint16_t* len)
{
    uint8_t rd_len;
    uint16_t i = 0,temp_len;
    uint32_t addr;
    fs_item_t i1;

    if(__psr()&0x3f)
    {
        return PPlus_ERR_FS_IN_INT;
    }

    if(fs_init_flag == false)
        return PPlus_ERR_FS_UNINITIALIZED;

    if((buf == NULL) || (buf_len == 0))
        return PPlus_ERR_FS_PARAMETER;

    if(hal_fs_item_find_id(id,&addr) == PPlus_SUCCESS)
    {
        fs_spif_read(FS_ABSOLUTE_ADDR(addr),(uint8_t*)&i1,FS_ITEM_HEAD_LEN);

        if(len != NULL)
        {
            *len = i1.b.len;
        }

        temp_len = i1.b.len;

        if(buf_len >= i1.b.len)
        {
            while(temp_len > 0)
            {
                rd_len = (temp_len >= FS_ITEM_DATA_LEN) ? FS_ITEM_DATA_LEN : temp_len;
                fs_spif_read(FS_ABSOLUTE_ADDR(addr + FS_ITEM_HEAD_LEN),(buf + i),rd_len);
                temp_len -= rd_len;
                addr += FS_ITEM_LEN;
                i += rd_len;
                check_addr(&addr);
            }

            return PPlus_SUCCESS;
        }

        return PPlus_ERR_FS_BUFFER_TOO_SMALL;
    }

    return PPlus_ERR_FS_NOT_FIND_ID;
}

int hal_fs_item_del(uint16_t id)
{
    uint16_t i = 0,count = 1;
    uint32_t addr = 0;
    fs_item_t i1;

    if(__psr()&0x3f)
    {
        return PPlus_ERR_FS_IN_INT;
    }

    if(fs_init_flag == FALSE)
        return PPlus_ERR_FS_UNINITIALIZED;

    if(hal_fs_item_find_id(id,&addr) == PPlus_SUCCESS)
    {
        fs_spif_read(FS_ABSOLUTE_ADDR(addr),(uint8_t*)&i1,FS_ITEM_HEAD_LEN);
        count = i1.b.len/FS_ITEM_DATA_LEN + ((i1.b.len % FS_ITEM_DATA_LEN)?1:0);

        for(i = 0; i < count; i++)
        {
            fs_spif_read(FS_ABSOLUTE_ADDR(addr),(uint8_t*)&i1,FS_ITEM_HEAD_LEN);
            i1.b.pro = ITEM_DEL;

            if(PPlus_SUCCESS != fs_spif_write(FS_ABSOLUTE_ADDR(addr),(uint8_t*)(&i1.reg),FS_ITEM_HEAD_LEN))
                return PPlus_ERR_FS_WRITE_FAILED;

            addr += FS_ITEM_LEN;
            check_addr(&addr);
        }

        return PPlus_SUCCESS;
    }
    else
    {
        return PPlus_ERR_FS_NOT_FIND_ID;
    }
}

int hal_fs_garbage_collect(void)
{
    uint8_t  i,j,buf[FS_ITEM_DATA_LEN];
    uint8_t from_sector_index = 0,to_sector_index = 0;
    uint32_t addr_rd=0,addr_wr=0,addr_erase;
    fs_item_t i1;

    if(__psr()&0x3f)
    {
        return PPlus_ERR_FS_IN_INT;
    }

    if(fs_init_flag == FALSE)
        return PPlus_ERR_FS_UNINITIALIZED;

    to_sector_index = fs.exchange_sector;
    from_sector_index = (fs.exchange_sector + 1) % fs.cfg.sector_num;
    addr_wr = 4096*to_sector_index;

    for(i = 0; i < (fs.cfg.sector_num - 1); i++)
    {
        addr_rd = 4096*((from_sector_index + i) % fs.cfg.sector_num);
        addr_erase = addr_rd;
        fs.cfg.index = i;

        if(PPlus_SUCCESS != fs_spif_write(FS_ABSOLUTE_ADDR((4096*((to_sector_index + i) % fs.cfg.sector_num))),(uint8_t*)(&(fs.cfg)),sizeof(fs_cfg_t)))
            return PPlus_ERR_FS_WRITE_FAILED;

        if(i == 0)
            addr_wr += sizeof(fs_cfg_t);

        addr_rd += sizeof(fs_cfg_t);

        for(j = 0; j < FS_SECTOR_ITEM_NUM; j++)
        {
            fs_spif_read(FS_ABSOLUTE_ADDR(addr_rd),(uint8_t*)&i1,FS_ITEM_HEAD_LEN);

            if(i1.b.pro == ITEM_USED)
            {
                fs_spif_read(FS_ABSOLUTE_ADDR(addr_rd + FS_ITEM_HEAD_LEN),buf,FS_ITEM_DATA_LEN);

                if(PPlus_SUCCESS != fs_spif_write(FS_ABSOLUTE_ADDR(addr_wr),(uint8_t*)(&i1.reg),FS_ITEM_HEAD_LEN))
                    return PPlus_ERR_FS_WRITE_FAILED;

                if(PPlus_SUCCESS != fs_spif_write(FS_ABSOLUTE_ADDR(addr_wr + FS_ITEM_HEAD_LEN),buf,FS_ITEM_DATA_LEN))
                    return PPlus_ERR_FS_WRITE_FAILED;

                addr_wr += FS_ITEM_LEN;
                check_addr(&addr_wr);
            }
            else if(i1.b.pro == ITEM_UNUSED)
            {
                break;
            }

            addr_rd += FS_ITEM_LEN;
        }

        fs_erase_ucds_one_sector(addr_erase);
    }

    return fs_init();
}

int hal_fs_format(uint32_t fs_start_address,uint8_t sector_num)
{
    if(__psr()&0x3f)
    {
        return PPlus_ERR_FS_IN_INT;
    }

    fs_init_flag = FALSE;

    if((fs_start_address % 0x1000) || (sector_num < 3))
    {
        return PPlus_ERR_INVALID_PARAM;
    }

    fs_sector_num = sector_num;
    fs_offset_address = fs_start_address;
    fs_erase_ucds_all_sector();
    return hal_fs_init(fs_start_address,sector_num);
}

int hal_fs_init(uint32_t fs_start_address,uint8_t sector_num)
{
    if(fs_init_flag == TRUE)
    {
        return PPlus_ERR_FS_UNINITIALIZED;
    }

    if((fs_start_address % 0x1000) || (sector_num < 2))
    {
        return PPlus_ERR_INVALID_PARAM;
    }

    fs_sector_num = sector_num;
    fs_offset_address = fs_start_address;
    return fs_init();
}


bool hal_fs_initialized(void)
{
    return fs_init_flag;
}
