/*************
 dma.c
 SDK_LICENSE
***************/

#include <stdlib.h>
#include "OSAL.h"
#include <string.h>
#include <time.h>
#include "app_err.h"
#include "app_wrist.h"
#include "app_datetime.h"
#include "types.h"
#include "clock.h"
#include "log.h"

#ifndef USE_SYS_TICK
    #define USE_SYS_TICK  FASLE  //default use RTC, if system use RC32K as RTC clock source, system tick is usually recommended
#endif

#if(USE_SYS_TICK)
    #define RTC_CNT_RANGE 0x100000000
    #define TM_RATE (1000000/625)
#else
    #define RTC_CNT_RANGE 0x1000000
    #define TM_RATE (32768)
#endif
#define DT_INTV_SYNC 60    //sync interval every 60s



//static app_dtm_hdl_t s_evt_hdl = NULL;

static datetime_cfg_t s_stm_cfg = {0};

static const char* const month_str[12] =
{
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
};


static void app_datetime_adjust_baseline(struct tm* datetm);

static void print_hex (uint8_t* data, uint16 len)
{
    uint16 i;

    for (i = 0; i < len - 1; i++)
    {
        LOG("%x",data[i]);
        LOG(" ");
    }

    LOG("%x",data[i]);
}

static void dt_timer_start(uint32_t intval_ms)
{
    osal_start_timerEx(AppWrist_TaskID, TIMER_DT_EVT, intval_ms);
}

void timer_cnt_get(uint32_t* tick)
{
    #if(USE_SYS_TICK)
    *tick = hal_systick();         // read current RTC counter
    #else
    *tick = rtc_get_counter();         // read current RTC counter
    #endif
}


static void get_default_tm(struct tm* datetm)
{
    int i;
    datetime_t dtm;
    char dt[20];
    char* pstr = NULL;
    LOG(__DATE__);
    LOG(__TIME__);
    strcpy(dt, __DATE__);
    pstr = strtok(dt, " ");

    for(i = 0; i< 12; i++)
    {
        if(strcmp(dt,month_str[i]) == 0)
        {
            dtm.month = i+1;
            break;
        }
    }

    pstr = strtok(NULL, " ");
    dtm.day = atoi(pstr);
    pstr = strtok(NULL, " ");
    dtm.year = atoi(pstr);
    strcpy(dt, __TIME__);
    pstr = strtok(dt, ":");
    dtm.hour = atoi(pstr);
    pstr = strtok(NULL, ":");
    dtm.minutes = atoi(pstr);
    pstr = strtok(NULL, ":");
    dtm.seconds = atoi(pstr);
    DTM2TM(datetm, &dtm);
}


static void check_default_datetime(void)
{
    struct tm datetm;
    get_default_tm(&datetm);
    LOG("check_default_datetime\nTime is %d-%d-%d, %d:%d:%d",
        datetm.tm_year+1900,
        datetm.tm_mon+1,
        datetm.tm_mday,
        datetm.tm_hour,
        datetm.tm_min,
        datetm.tm_sec);
    app_datetime_adjust_baseline(&datetm);
    return;
}

static void datetime_print(void)
{
    datetime_t dtm;
    app_datetime(&dtm);
    LOG("Time is %d-%d-%d, %d:%d:%d\n", dtm.year, dtm.month, dtm.day, dtm.hour, dtm.minutes, dtm.seconds);
}

void app_datetime_sync_handler(void )
{
    uint32_t tick= 0;
    timer_cnt_get(&tick);

    if(tick < s_stm_cfg.snapshot)
        s_stm_cfg.tm_base += RTC_CNT_RANGE;

    s_stm_cfg.snapshot = tick;
    dt_timer_start(DATETIME_SYNC_INTERVAL);
    datetime_print();
}



static void app_datetime_adjust_baseline(struct tm* datetm)
{
    uint64_t tmstamp;
    uint32_t ticks;
    timer_cnt_get(&ticks);
    tmstamp = (uint64_t)mktime(datetm);
    LOG("%x",(uint32_t)tmstamp);
    tmstamp = tmstamp*TM_RATE;
    s_stm_cfg.tm_base = tmstamp - ticks;
    s_stm_cfg.snapshot = ticks;
    LOG("app_datetime_adjust_baseline:\n");
    print_hex((uint8_t*)&s_stm_cfg, sizeof(s_stm_cfg));
    LOG("\n");
}


int app_datetime_set(datetime_t dtm)
{
    struct tm tm;
    memset(&tm,0, sizeof(tm));
    DTM2TM(&tm, &dtm);
    LOG("\napp_datetime_set:\n");
    LOG("Time is %d-%d-%d, %d:%d:%d\n", dtm.year, dtm.month, dtm.day, dtm.hour, dtm.minutes, dtm.seconds);
    app_datetime_adjust_baseline(&tm);
    print_hex((uint8_t*)&s_stm_cfg, sizeof(s_stm_cfg));
    LOG("\n");
    return APP_SUCCESS;
}




int app_datetime_diff(const datetime_t* pdtm_base, const datetime_t* pdtm_cmp)
{
    time_t time_base, time_cmp;
    struct tm tm_base, tm_cmp;
    DTM2TM(&tm_base, pdtm_base);
    DTM2TM(&tm_cmp, pdtm_cmp);
    time_base = mktime(&tm_base);
    time_cmp = mktime(&tm_cmp);
    return (int)difftime(time_base, time_cmp);
}


time_t app_datetime_time_t(void)
{
    time_t tm = (time_t)(s_stm_cfg.tm_base / TM_RATE);
    return tm;
}

int app_datetime(datetime_t* pdtm)
{
    struct tm* ptm = NULL;
    uint32_t ticks;
    time_t tm = (time_t)(s_stm_cfg.tm_base / TM_RATE);
    timer_cnt_get(&ticks);

    if(s_stm_cfg.snapshot > ticks)
        tm += RTC_CNT_RANGE/TM_RATE;

    tm += ticks/TM_RATE;
    ptm = localtime(&tm);
    TM2DTM(pdtm, ptm)
    return APP_SUCCESS;
}

void app_datetime_init(void)//app_dtm_hdl_t evt_hdl)
{
    //s_evt_hdl = evt_hdl;
    check_default_datetime();
    dt_timer_start(DATETIME_SYNC_INTERVAL);
}

