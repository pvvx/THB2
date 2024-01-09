/*******************************************************************************
    @file     sdk_version.h
    @brief
    @author

 SDK_LICENSE

*******************************************************************************/
#ifndef __SDK_VER_H__
#define __SDK_VER_H__

#define __DEF_CHIP_QFN32__                  (0x0001)
#define __DEF_CHIP_TSOP16__                  (0x0002)
#define SDK_VER_MAJOR                      3
#define SDK_VER_MINOR                      1
#define SDK_VER_REVISION                   1
#define SDK_SUB_CODE                       2
#define SDK_VER_RELEASE_ID                 ((SDK_VER_MAJOR<<24)|(SDK_VER_MINOR<<16)|(SDK_VER_REVISION<<8)|(SDK_SUB_CODE<<0))
#define SDK_VER_CHIP                      __DEF_CHIP_QFN32__ // __DEF_CHIP_TSOP16__
//#define SDK_VER_TEST_BUILD ""
#endif

