
/**
    \file EM_timer.h

    This Header File contains the APIs and the ADTs exported by the
    EtherMind Timer Library for Windows (User-mode).
*/

/*
    Copyright (C) 2013. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_EM_TIMER_
#define _H_EM_TIMER_

/* --------------------------------------------------- Header File Inclusion */
#include "EM_os.h"
#include "osal_cbtimer.h"

/* Enable support to get remaining time to expire of a timer entity */
#define EM_TIMER_SUPPORT_REMAINING_TIME

/* --------------------------------------------------- Global Definitions */
/* Maximum number of timer entities */
#define EM_TIMER_MAX_ENTITIES                       15

/* Mask to indicate millisecond timeout */
#define EM_TIMEOUT_MILLISEC                         0x80000000

/* Timer Handles must be initialized to this value */
#define EM_TIMER_HANDLE_INIT_VAL                    0xFF

#define EM_TIMER_STATIC_DATA_SIZE                   32

/* Flag: Timer Entity State */
#define TIMER_ENTITY_FREE                           0x00
#define TIMER_ENTITY_IN_USE                         0x01
#define TIMER_ENTITY_IN_FREE                        0x02

/* Flag: Timer Entity Data to be freed or not */
#define TIMER_ENTITY_HOLD_ALLOC_DATA                0x00
#define TIMER_ENTITY_FREE_ALLOC_DATA                0x01

/* Timer module ID and Error codes */
#define EM_TIMER_ERR_ID                            0xC000

#define EM_TIMER_MUTEX_INIT_FAILED                 (0x0001 | EM_TIMER_ERR_ID)
#define EM_TIMER_COND_INIT_FAILED                  (0x0002 | EM_TIMER_ERR_ID)
#define EM_TIMER_MUTEX_LOCK_FAILED                 (0x0003 | EM_TIMER_ERR_ID)
#define EM_TIMER_MUTEX_UNLOCK_FAILED               (0x0004 | EM_TIMER_ERR_ID)
#define EM_TIMER_MEMORY_ALLOCATION_FAILED          (0x0005 | EM_TIMER_ERR_ID)

#define EM_TIMER_HANDLE_IS_NULL                    (0x0011 | EM_TIMER_ERR_ID)
#define EM_TIMER_CALLBACK_IS_NULL                  (0x0012 | EM_TIMER_ERR_ID)
#define EM_TIMER_QUEUE_EMPTY                       (0x0013 | EM_TIMER_ERR_ID)
#define EM_TIMER_QUEUE_FULL                        (0x0014 | EM_TIMER_ERR_ID)
#define EM_TIMER_ENTITY_SEARCH_FAILED              (0x0015 | EM_TIMER_ERR_ID)
#define EM_TIMER_NULL_PARAMETER_NOT_ALLOWED        (0x0016 | EM_TIMER_ERR_ID)
#define EM_TIMER_TIMEOUT_ZERO_NOT_ALLOWED          (0x0017 | EM_TIMER_ERR_ID)
#define EM_TIMER_FAILED_SET_TIME_EVENT             (0x0018 | EM_TIMER_ERR_ID)

/**
    Invalid Timer ID for PhyOS.
    NUM_CBTIMERS is not exposed from osal_cbtimer.c
*/
#define PHYOS_INVALID_TIMER_ID                     0xFF

/* ----------------------------------------------- Structures/Data Types */

/* Timer Entity */
typedef struct timer_entity_struct
{
    /* The Timer Handle - Index of the timer entity */
    UINT8 handle;

    /* Callback to call when Timer expires */
    void (* callback) (void*, UINT16);

    /**
        Timer Callback Parameter if
        data_length > EM_TIMER_STATIC_DATA_SIZE
    */
    UCHAR*  allocated_data;

    /* Next Element in the Timer Q */
    struct timer_entity_struct* next;

    /**
        Timer Callback Parameter if
        data_length <= EM_TIMER_STATIC_DATA_SIZE
    */
    UCHAR  static_data[EM_TIMER_STATIC_DATA_SIZE];

    /* Timeout Value asked by the User */
    UINT32 timeout;

    #ifdef EM_TIMER_SUPPORT_REMAINING_TIME
    /* Start Time Stamp - used to calculate remaining time */
    UINT64 start_timestamp;
    #endif /* EM_TIMER_SUPPORT_REMAINING_TIME */

    /* Length of the data */
    UINT16 data_length;

    /* Is this Entity Allocated ? */
    UCHAR in_use;

    /* PhyOS Timer ID */
    UINT8 timer_id;

} TIMER_ENTITY;

typedef UINT8  EM_timer_handle;



/* --------------------------------------------------- Function Declarations */

#ifdef __cplusplus
extern "C" {
#endif

void EM_timer_init ( void );
void timer_em_init ( void );
void timer_em_shutdown ( void );

EM_RESULT EM_start_timer
(
    EM_timer_handle* handle,
    UINT32 timeout,
    void (* callback) (void*, UINT16),
    void* args, UINT16 size_args
);

EM_RESULT EM_restart_timer
(
    EM_timer_handle handle,
    UINT32 new_timeout
);

EM_RESULT EM_stop_timer ( EM_timer_handle* handle );

UINT32 EM_get_remain_timer
(
    EM_timer_handle handle
);


EM_RESULT EM_timer_get_remaining_time
(
    EM_timer_handle   handle,
    UINT32*           remaining_time_ms
);

EM_RESULT EM_is_active_timer ( EM_timer_handle handle );

/* Debug Routine - Internal Use Only */
EM_RESULT EM_list_timer ( void );

#ifdef __cplusplus
};
#endif

#endif /* _H_EM_TIMER_ */

