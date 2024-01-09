/*************
 fs_test.h
 SDK_LICENSE
***************/

#ifndef __FS_TEST_H__
#define __FS_TEST_H__

//be care:when assign the fs size,please refer to Flash_Distribution
#define FS_OFFSET_ADDRESS         0x1103c000 //for 256KB Flash
//#define FS_OFFSET_ADDRESS         0x11034000 //for 512KB Flash
#define FS_SECTOR_NUM             2



#define FS_EXAMPLE       0x01
#define FS_XIP_TEST      0x02
//#define FS_MODULE_TEST   0x04
//#define FS_TIMING_TEST   0x08

#define FS_TEST_TYPE     FS_EXAMPLE

#if (FS_TEST_TYPE == FS_EXAMPLE)
    void fs_example(void);
#elif (FS_TEST_TYPE == FS_XIP_TEST)
    void fs_xip_test(void);
#elif (FS_TEST_TYPE == FS_MODULE_TEST)
    void ftcase_simple_write_test(void);
    void ftcase_write_del_test(void);
    void ftcase_write_del_and_ble_enable_test(void);
#elif (FS_TEST_TYPE == FS_TIMING_TEST)
    void fs_timing_test(void);
#else
    #error please check your config parameter
#endif


#endif
