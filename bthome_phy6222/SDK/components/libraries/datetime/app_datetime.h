/*************
 app_datetime.h
 SDK_LICENSE
***************/

#ifndef APP_DATETIME_H
#define APP_DATETIME_H


#include "types.h"



#define DATETIME_SYNC_INTERVAL      30*1*1000 //interval is 30s

#define DTM2TM(tm, dtm)     {   (tm)->tm_mon = (dtm)->month-1;\
        (tm)->tm_year = (dtm)->year-1900;\
        (tm)->tm_mday = (dtm)->day;\
        (tm)->tm_hour = (dtm)->hour;\
        (tm)->tm_min = (dtm)->minutes;\
        (tm)->tm_sec = (dtm)->seconds;}


#define TM2DTM(dtm, tm)     {   (dtm)->month = (tm)->tm_mon+1;\
        (dtm)->year = (tm)->tm_year+1900;\
        (dtm)->day = (tm)->tm_mday;\
        (dtm)->hour = (tm)->tm_hour;\
        (dtm)->minutes = (tm)->tm_min;\
        (dtm)->seconds = (tm)->tm_sec;}


#define BCDTM2DTM(dtm, bcdtm)   {   (dtm)->year = bcdtm[0] + 2000;\
        (dtm)->month = bcdtm[1];\
        (dtm)->day = bcdtm[2];\
        (dtm)->hour = bcdtm[3];\
        (dtm)->minutes = bcdtm[4];\
        (dtm)->seconds = bcdtm[5];}

#define DTM2BCDTM(bcdtm, dtm)   {   bcdtm[0] = (dtm)->year - 2000;\
        bcdtm[1] = (dtm)->month;\
        bcdtm[2] = (dtm)->day;\
        bcdtm[3] = (dtm)->hour;\
        bcdtm[4] = (dtm)->minutes;\
        bcdtm[5] = (dtm)->seconds;}

#define DTM2U32(u32val, dtm)    {   uint32 tmp = (dtm)->year;   u32val = (tmp << 16);\
        tmp = (dtm)->month; u32val |= (tmp << 8);\
        tmp = (dtm)->day; u32val |= tmp;}

#define U322DTM(dtm,u32val) {   (dtm)->year = (uint16)((u32val)>>16);\
        (dtm)->month = (uint8)(((u32val) >> 8) & 0xff);\
        (dtm)->day = (uint8)((u32val) &0xff);\
        (dtm)->hour = 0; (dtm)->minutes  =0; (dtm)->seconds = 0;}

typedef struct
{
    uint64_t tm_base;
    uint32_t snapshot;
    uint32_t reserved;  //reserved for 4 byte align
} datetime_cfg_t;




typedef struct
{
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} datetime_t;


typedef enum
{
    DTM_EV_ALRRM_0 = 1,
    DTM_EV_ALRRM_1,
    DTM_EV_ALRRM_2,
    DTM_EV_ALRRM_3,
    DTM_EV_ALRRM_4,
} app_dtm_evt_t;

//typedef void (*app_dtm_hdl_t)(app_dtm_evt_t ev);


void app_datetime_sync_handler(void );
int app_datetime_diff(const datetime_t* pdtm_base, const datetime_t* pdtm_cmp);
int app_datetime_set(datetime_t dtm);
int app_datetime(datetime_t* pdtm);
void app_datetime_init(void);//app_dtm_hdl_t evt_hdl);

#endif

