
/**
    \file EM_timer.c

    This File contains source codes for the EtherMind
    Timer Library Implementation for FreeRTOS.
*/



/* ----------------------------------------------- Header File Inclusion */
#include "EXT_cbtimer.h"
//#include "EM_timer_internal.h"   //todo
//#include "EXT_cbtimer.h"
#include "OSAL_Clock.h"
#include "gpio.h"

#include "OSAL_Tasks.h"

/* ----------------------------------------------- Global Definitions */
/* Timer Elements */
EXT_CBTIMER_ENTITY ext_cbtimer_entity[EXT_CBTIMER_MAX_ENTITIES];
EXT_CBTIMER_ENTITY* ext_cbtimer_q_start = NULL;
EXT_CBTIMER_ENTITY* ext_cbtimer_q_end   = NULL;

#if 0
    /* Timer Library Mutex */
    EM_thread_mutex_type timer_mutex;
#endif /* 0 */

#if 1
/* Timer */
#if defined ( CBTIMER_NUM_TASKS )
    #include "cbTimer.h"
#endif


/* ----------------------------------------------- Static Global Variables */
/*********************************************************************
    CONSTANTS
*/
// Number of callback timers supported per task (limited by the number of OSAL event timers)
#define EXT_NUM_CBTIMERS_PER_TASK          16

// Total number of callback timers
#define EXT_NUM_CBTIMERS                   ( CBTIMER_NUM_TASKS * EXT_NUM_CBTIMERS_PER_TASK )

#define ext_timer_malloc            EM_alloc_mem
#define ext_timer_free              EM_free_mem


typedef struct
{
    extpfnCbTimer_t pfnCbTimer; // callback function to be called when timer expires
    uint8* pData;            // data to be passed in to callback function
} ext_cbTimer_t;

ext_cbTimer_t ext_cbTimers[EXT_NUM_CBTIMERS];
uint16 extbaseTaskID = TASK_NO_TASK;

#define EXT_EVENT_ID( timerId )            ( 0x0001 << ( ( timerId ) % EXT_NUM_CBTIMERS_PER_TASK ) )

// Find out task id using timer id
#define EXT_TASK_ID( timerId )             ( ( ( timerId ) / EXT_NUM_CBTIMERS_PER_TASK ) + extbaseTaskID )

// Find out bank task id using task id
#define EXT_BANK_TASK_ID( taskId )         ( ( ( taskId ) - extbaseTaskID ) * EXT_NUM_CBTIMERS_PER_TASK )

/*********************************************************************
    LOCAL FUNCTIONS
*/


/*********************************************************************
    API FUNCTIONS
*/
EM_RESULT ext_timer_init_entity (EXT_CBTIMER_ENTITY* timer);

EM_RESULT ext_timer_search_entity_timer_id
(
    EXT_CBTIMER_ENTITY** timer,
    UINT8          handle
);
EM_RESULT ext_timer_del_entity
(
    EXT_CBTIMER_ENTITY* timer,
    UCHAR free
);
EM_RESULT ext_timer_search_entity ( EXT_CBTIMER_ENTITY* timer );

EM_RESULT ext_timer_add_entity ( EXT_CBTIMER_ENTITY* timer );

#ifdef EXT_CBTIMER_SUPPORT_REMAINING_TIME

    /*
    This function returns elapsed time in microsecond since system power on.
    */
    UINT64 ext_cbtimer_get_ms_timestamp(void);
#endif

/*********************************************************************
    @fn          CbTimerInit

    @brief       Callback Timer task initialization function. This function
                can be called more than once (CBTIMER_NUM_TASKS times).

    @param       taskId - Message Timer task ID.

    @return      void
*/
void CbTimerInit( uint8 taskId )
{
    if ( extbaseTaskID == TASK_NO_TASK )
    {
        // Only initialize the base task id
        extbaseTaskID = taskId;
        // Initialize all timer structures
        osal_memset( ext_cbTimers, 0, sizeof( ext_cbTimers ) );
    }
}

/*********************************************************************
    @fn          CbTimerProcessEvent

    @brief       Callback Timer task event processing function.

    @param       taskId - task ID.
    @param       events - events.

    @return      events not processed
*/
uint16 CbTimerProcessEvent( uint8 taskId, uint16 events )
{
    if ( events )
    {
        uint8 i;
        uint16 event;

        // Process event timers
        for ( i = 0; i < EXT_NUM_CBTIMERS_PER_TASK; i++ )
        {
            if ( ( events >> i ) & 0x0001 )
            {
                ext_cbTimer_t* pTimer = &ext_cbTimers[EXT_BANK_TASK_ID( taskId )+i];
                // Found the first event
                event =  0x0001 << i;

                // Timer expired, call the registered callback function
                if(pTimer->pData != NULL)
                {
                    pTimer->pfnCbTimer( pTimer->pData );
                }

                // Mark entry as free
                pTimer->pfnCbTimer = NULL;
                // Null out data pointer
                pTimer->pData = NULL;
                // We only process one event at a time
                break;
            }
        }

        // return unprocessed events
        return ( events ^ event );
    }

    // If reach here, the events are unknown
    // Discard or make more handlers
    return 0;
}

/*********************************************************************
    @fn      CbTimerStart

    @brief   This function is called to start a callback timer to expire
            in n mSecs. When the timer expires, the registered callback
            function will be called.

    @param   pfnCbTimer - callback function to be called when timer expires
    @param   pData - data to be passed in to callback function
    @param   timeout - in milliseconds.
    @param   pTimerId - will point to new timer Id (if not null)

    @return  Success, or Failure.
*/
Status_t CbTimerStart( extpfnCbTimer_t pfnCbTimer, uint8* pData,
                       uint16 timeout, uint8* pTimerId )
{
    uint8 i;

    // Validate input parameters
    if ( pfnCbTimer == NULL )
    {
        return ( INVALIDPARAMETER );
    }

    // Look for an unused timer first
    for ( i = 0; i < EXT_NUM_CBTIMERS; i++ )
    {
        if ( ext_cbTimers[i].pfnCbTimer == NULL )
        {
            // Start the OSAL event timer first
            if ( osal_start_timerEx( EXT_TASK_ID( i ), EXT_EVENT_ID( i ), timeout ) == SUCCESS )
            {
                // Set up the callback timer
                ext_cbTimers[i].pfnCbTimer = pfnCbTimer;
                ext_cbTimers[i].pData = pData;

                if ( pTimerId != NULL )
                {
                    // Caller is intreseted in the timer id
                    *pTimerId = i;
                }

                return ( SUCCESS );
            }
        }
    }

    // No timer available
    return ( NO_TIMER_AVAIL );
}

/*********************************************************************
    @fn      CbTimerUpdate

    @brief   This function is called to update a message timer that has
            already been started. If SUCCESS, the function will update
            the timer's timeout value. If INVALIDPARAMETER, the timer
            either doesn't exit.

    @param   timerId - identifier of the timer that is to be updated
    @param   timeout - new timeout in milliseconds.

    @return  SUCCESS or INVALIDPARAMETER if timer not found
*/
Status_t CbTimerUpdate( uint8 timerId, uint16 timeout )
{
    // Look for the existing timer
    if ( timerId < EXT_NUM_CBTIMERS )
    {
        if ( ext_cbTimers[timerId].pfnCbTimer != NULL )
        {
            // Make sure the corresponding OSAL event timer is still running
            if ( osal_get_timeoutEx( EXT_TASK_ID( timerId ), EXT_EVENT_ID( timerId ) ) != 0 )
            {
                // Timer exists; update it
                osal_start_timerEx( EXT_TASK_ID( timerId ), EXT_EVENT_ID( timerId ), timeout );
                return (  SUCCESS );
            }
        }
    }

    // Timer not found
    return ( INVALIDPARAMETER );
}


/*********************************************************************
    @fn      CbTimerStop

    @brief   This function is called to stop a timer that has already been
            started. If SUCCESS, the function will cancel the timer. If
            INVALIDPARAMETER, the timer doesn't exit.

    @param   timerId - identifier of the timer that is to be stopped

    @return  SUCCESS or INVALIDPARAMETER if timer not found
*/
Status_t CbTimerStop( uint8 timerId )
{
    // Look for the existing timer
    if ( timerId < EXT_NUM_CBTIMERS )
    {
        if ( ext_cbTimers[timerId].pfnCbTimer != NULL )
        {
            // Timer exists; stop the OSAL event timer first
            osal_stop_timerEx( EXT_TASK_ID( timerId ), EXT_EVENT_ID( timerId ) );
            // Mark entry as free
            ext_cbTimers[timerId].pfnCbTimer = NULL;
            // Null out data pointer
            ext_cbTimers[timerId].pData = NULL;
            return ( SUCCESS );
        }
    }

    // Timer not found
    return ( INVALIDPARAMETER );
}



#endif

#undef EM_RESTART_TIMER

/* ----------------------------------------------- Functions */

void EXT_cbtimer_init (void)
{
    UINT16 index;
//    EM_TIMER_TRC(
//    "Initializing EtherMind Timer Library Module ...\n");
    #if 0
    /* Initialize Timer Mutex */
    EM_thread_mutex_init(&timer_mutex, NULL);
    #endif /* 0 */

    /* Initialize Timer Elements */
    for (index = 0; index < EXT_CBTIMER_MAX_ENTITIES; index ++)
    {
        ext_timer_init_entity(&ext_cbtimer_entity[index]);
        ext_cbtimer_entity[index].timer_id = EXT_PHYOS_INVALID_TIMER_ID;
        ext_cbtimer_entity[index].handle = index;
    }

    return;
}


void ext_cbtimer_em_init ( void )
{
    EXT_CBTIMER_ENTITY* timer;
    UINT16 index;

    /* Lock Timer */
//    timer_lock();

//    EM_TIMER_TRC(
//    "Stack ON Initialization for Timer Library ...\n");

    /* Initialize Timer Entities */
    for (index = 0; index < EXT_CBTIMER_MAX_ENTITIES; index ++)
    {
        timer = &ext_cbtimer_entity[index];
        timer->timer_id = EXT_PHYOS_INVALID_TIMER_ID;
    }

    /* Initialize Timer Q */
    ext_cbtimer_q_start = ext_cbtimer_q_end = NULL;
//    timer_unlock();
    return;
}

EM_RESULT EXT_cbtimer_start_timer
(
    EXT_cbtimer_handle* handle,
    UINT32 timeout,
    void (* callback) (void*, UINT16),
    void* data,
    UINT16 data_length
)
{
    UCHAR* data_ptr = NULL;
    EM_RESULT retval;
    EXT_CBTIMER_ENTITY current_timer;
    // HZF
    osalTimeUpdate();

    if (NULL == handle)
    {
//        EM_TIMER_ERR(
//        "NULL Argument Unacceptable for Timer Handles.\n");
        return EXT_CBTIMER_HANDLE_IS_NULL;
    }

//    EM_TIMER_TRC(
//    "Preparing to Add New Timer Entity. Timeout = %d, Data Size = %d.\n",
//    timeout, data_length);

    /* Timer Library expects to have a valid callback */
    if (NULL == callback)
    {
//        EM_TIMER_ERR(
//        "FAILED to Add New Timer Element. NULL Callback.\n");
        return EXT_CBTIMER_CALLBACK_IS_NULL;
    }

    if (0 != data_length)
    {
        if (data_length > EXT_CBTIMER_STATIC_DATA_SIZE)
        {
            data_ptr = (UCHAR*) ext_timer_malloc (data_length);

            if (NULL == data_ptr)
            {
//                EM_TIMER_ERR(
//                "FAILED to allocate Memory for Timer Handler Argument.\n");
                return EXT_CBTIMER_MEMORY_ALLOCATION_FAILED;
            }

            current_timer.allocated_data = data_ptr;
        }
        else
        {
            data_ptr = current_timer.static_data;
        }

        /* Copy the Data to be passed to the Timer Handler */
        EM_mem_copy(data_ptr, data, data_length);
    }

    /* Store Timer Data Length, Callback & Timeout */
    current_timer.callback = callback;
    current_timer.data_length = data_length;
    current_timer.timeout = timeout;
    /* Lock Timer */
//    timer_lock();
    /* Insert this Timer Entity into the List */
    retval = ext_timer_add_entity(&current_timer);

    if (EM_SUCCESS != retval)
    {
//        EM_TIMER_ERR(
//        "FAILED to Add New Timer to Timer Queue. Error Code = 0x%04X\n",
//        retval);
        if (current_timer.data_length > EXT_CBTIMER_STATIC_DATA_SIZE)
        {
            ext_timer_free (current_timer.allocated_data);
        }

//        timer_unlock();
        return retval;
    }

    /* Store the Handle */
    *handle = current_timer.handle;
//    printf(
//    "Successfully Added EXT New Timer to Timer Queue. Handle = 0x%02X\n",
//    *handle);
//    timer_unlock();
    return EM_SUCCESS;
}



void ext_cbtimer_em_shutdown ( void )
{
    UINT16 index;
    EXT_CBTIMER_ENTITY* timer;
    /* Lock Timer */
//    timer_lock();
    /* Initialize Timer Q */
    ext_cbtimer_q_start = ext_cbtimer_q_end = NULL;

    /* Initialize Timer Entities */
    for (index = 0; index < EXT_CBTIMER_MAX_ENTITIES; index++)
    {
        timer = &ext_cbtimer_entity[index];

        if (EXT_CBTIMER_ENTITY_IN_USE == timer->in_use)
        {
            /* Stop Timer */
            CbTimerStop(timer->timer_id);

            if (timer->data_length > EXT_CBTIMER_STATIC_DATA_SIZE)
            {
                ext_timer_free(timer->allocated_data);
            }

            ext_timer_init_entity(timer);
            timer->timer_id = EXT_PHYOS_INVALID_TIMER_ID;
        }
    }

//    EM_TIMER_TRC(
//    "Stack OFF on Timer Library Module ...\n");
//    timer_unlock();
    return;
}

//Status_t osal_CbTimerStart32( pfnCbTimer_t pfnCbTimer, uint8 *pData,
//                           uint32 timeout, uint8 *pTimerId )
//{
//  uint8 i;
//
//  // Validate input parameters
//  if ( pfnCbTimer == NULL )
//  {
//    return ( INVALIDPARAMETER );
//  }

//  // Look for an unused timer first
//  for ( i = 0; i < NUM_CBTIMERS32; i++ )
//  {
//    if ( cbTimers[i].pfnCbTimer == NULL )
//    {
//      // Start the OSAL event timer first
//      if ( osal_start_timerEx( TASK_ID32( i ), EVENT_ID32( i ), timeout ) == SUCCESS )
//      {
//        // Set up the callback timer
//        cbTimers[i].pfnCbTimer = pfnCbTimer;
//        cbTimers[i].pData = pData;

//        if ( pTimerId != NULL )
//        {
//          // Caller is intreseted in the timer id
//          *pTimerId = i;
//        }

//        return ( SUCCESS );
//      }
//    }
//  }

//  // No timer available
//  return ( NO_TIMER_AVAIL );
//}


/* Callback registered with timer module */
void ext_cbtimer_timeout_handler (UINT8* handle)
{
    EM_RESULT retval;
    EXT_CBTIMER_ENTITY* timer;
//    EM_TIMER_TRC (
//    "In Timer handler (Timer Handle: 0x%02X)\n", *handle);
    /* Lock Timer */
//    timer_lock();
    /* Get the appropriate timer entity */
    retval = ext_timer_search_entity_timer_id (&timer, *handle);

    if (EM_SUCCESS != retval)
    {
//        EM_TIMER_ERR(
//        "*** UNKNOWN Spurious Timeout Callback?!?!\n");
        /* Unlock Timer */
//        timer_unlock ();
        return;
    }

    /* Unlock Timer */
//    timer_unlock ();

    if (timer->data_length > EXT_CBTIMER_STATIC_DATA_SIZE)
    {
        /* Call the registered timeout handler */
        timer->callback (timer->allocated_data, timer->data_length);
    }
    else
    {
        /* Use Static Data */
        timer->callback (timer->static_data, timer->data_length);
    }

    /* Lock Timer */
//    timer_lock ();
    #if 0
    /* Stop Timer */
    xTimerStop  (timer->timer_id, 0);
    #endif /* 0 */
    /* Free the Timer */
    retval = ext_timer_del_entity (timer, 1);

    if (EM_SUCCESS != retval)
    {
//        EM_TIMER_ERR(
//        "FAILED to find Timer Element. Handle = 0x%02X. Error Code = 0x%04X\n",
//        handle, retval);
        printf(
            "FAILED to find Timer Element. Handle = 0x%02X. Error Code = 0x%04X\n",
            *handle, retval);
    }

    /* Unlock Timer */
//    timer_unlock ();
    return;
}


EM_RESULT EXT_cbtimer_stop_timer
(
    EXT_cbtimer_handle handle
)
{
    EXT_CBTIMER_ENTITY* timer;
    EM_RESULT retval;
    UINT8  timer_id;
    UINT32 new_timeout;
    Status_t status;

    if (EXT_CBTIMER_MAX_ENTITIES <= handle)
    {
//        EM_TIMER_ERR(
//        "NULL Argument Unacceptable for Timer Handles.\r\n");
        /* TODO: Use appropriate error value */
        return EXT_CBTIMER_HANDLE_IS_NULL;
    }

    osalTimeUpdate();
    retval = EM_FAILURE;
    /* Lock Timer */
//    timer_lock();
    timer = &ext_cbtimer_entity[handle];
    /* Store the timer id before deleting entity */
    timer_id = timer->timer_id;
    new_timeout = 0x10;

    if(ext_timer_search_entity(timer) == EM_SUCCESS)
    {
        if(CbTimerUpdate(timer_id,new_timeout) == EM_SUCCESS
                ||(osalFindTimer(EXT_TASK_ID( timer->timer_id ),EXT_EVENT_ID( timer->timer_id )) == NULL))
        {
            retval = ext_timer_del_entity(timer, 0x01);

            if (EM_SUCCESS != retval)
            {
                //            EM_TIMER_ERR(
                //            "FAILED to find Timer Element. Handle = 0x%02X. Error Code = 0x%04X\n",
                //            handle, retval);
                printf(
                    "FAILED to find Timer Element. Handle = 0x%02X. Error Code = 0x%04X\n",
                    handle, retval);
            }
            else
            {
                //            EM_TIMER_TRC(
                //            "Successfully Deleted Timer Element for Handle 0x%02X.\n",
                //            handle);
                /* Stop Timer */
                status = CbTimerStop(timer_id);
                osal_clear_event(EXT_TASK_ID( timer->timer_id ),EXT_EVENT_ID( timer->timer_id ));

                if (SUCCESS != status)
                {
                    retval = EM_FAILURE;
                }

                //            EM_TIMER_TRC("*** Stopped Timer [ID: %04X]\n",
                //            timer_id);
            }
        }
    }

    /* Unlock Timer */
//    timer_unlock();
    return retval;
}


UINT32 EXT_cbtimer_get_remain_timer
(
    EXT_cbtimer_handle handle
)
{
    EXT_CBTIMER_ENTITY* timer;
    UINT32 remain_timeout;

    if (EXT_CBTIMER_MAX_ENTITIES <= handle)
    {
//        EM_TIMER_ERR(
//        "NULL Argument Unacceptable for Timer Handles.\r\n");
        /* TODO: Use appropriate error value */
        return EXT_CBTIMER_HANDLE_IS_NULL;
    }

    /* Lock Timer */
//    timer_lock();
    timer = &ext_cbtimer_entity[handle];
    remain_timeout = osal_get_timeoutEx(EXT_TASK_ID( timer->timer_id ),EXT_EVENT_ID( timer->timer_id ));
    /* Unlock Timer */
//    timer_unlock();
    return remain_timeout;
}



EM_RESULT EXT_cbtimer_restart_timer
(
    EXT_cbtimer_handle handle,
    UINT32 new_timeout
)
{
    EXT_CBTIMER_ENTITY* timer;
    EM_RESULT retval;

    if (EXT_CBTIMER_MAX_ENTITIES <= handle)
    {
//        EM_TIMER_ERR(
//        "NULL Argument Unacceptable for Timer Handles.\n");
        /* TODO: Use appropriate error value */
        return EXT_CBTIMER_HANDLE_IS_NULL;
    }

//    timer_lock();
    timer = &ext_cbtimer_entity[handle];
    retval = ext_timer_search_entity(timer);

    if (EM_SUCCESS != retval)
    {
//        EM_TIMER_ERR(
//        "FAILED to Find Timer ELement for Handle 0x%02X. Error Code = 0x%04X\n",
//        handle, retval);
    }
    else
    {
        if(CbTimerUpdate(timer->timer_id,new_timeout) == EM_SUCCESS)
        {
            return ( SUCCESS );
        }
    }

    // No timer available
    return ( NO_TIMER_AVAIL );
}

EM_RESULT EXT_cbtimer_is_active_timer
(
    EXT_cbtimer_handle handle
)
{
    EXT_CBTIMER_ENTITY* timer;
    EM_RESULT retval;

    if (EXT_CBTIMER_MAX_ENTITIES <= handle)
    {
//        EM_TIMER_ERR(
//        "NULL Argument Unacceptable for Timer Handles.\n");
        /* TODO: Use appropriate error value */
        return EXT_CBTIMER_HANDLE_IS_NULL;
    }

    /* Lock Timer */
//    timer_lock();
    timer = &ext_cbtimer_entity[handle];
    retval = ext_timer_search_entity(timer);

    if (EM_SUCCESS != retval)
    {
//        EM_TIMER_ERR(
//        "FAILED to Find the Timer Entity for Handle 0x%02X. Error Code = 0x%04X\n",
//        handle, retval);
    }

//    timer_unlock();
    return retval;
}


EM_RESULT ext_timer_search_entity ( EXT_CBTIMER_ENTITY* timer )
{
    EXT_CBTIMER_ENTITY* current_timer;

    /* Is Queue Empty */
    if (NULL == ext_cbtimer_q_start)
    {
        return EXT_CBTIMER_QUEUE_EMPTY;
    }

    /* Handle the first Element */
    if (timer == ext_cbtimer_q_start)
    {
        return EM_SUCCESS;
    }

    current_timer = ext_cbtimer_q_start->next;

    while (NULL != current_timer)
    {
        if (timer == current_timer)
        {
            return EM_SUCCESS;
        }

        current_timer = current_timer->next;
    }

    return EXT_CBTIMER_ENTITY_SEARCH_FAILED;
}

/* Get the timer based on obtained timer id */
EM_RESULT ext_timer_search_entity_timer_id
(
    EXT_CBTIMER_ENTITY** timer,
    UINT8          handle
)
{
    EXT_CBTIMER_ENTITY* current_timer;

    /* Is Queue Empty */
    if (NULL == ext_cbtimer_q_start)
    {
        return EXT_CBTIMER_QUEUE_EMPTY;
    }

    /* Handle the first Element */
    if (handle == ext_cbtimer_q_start->handle)
    {
        /* Note the timer entity */
        *timer = ext_cbtimer_q_start;
        return EM_SUCCESS;
    }

    current_timer = ext_cbtimer_q_start->next;

    while (NULL != current_timer)
    {
        if (handle == current_timer->handle)
        {
            /* Note the timer entity */
            *timer = current_timer;
            return EM_SUCCESS;
        }

        current_timer = current_timer->next;
    }

    return EXT_CBTIMER_ENTITY_SEARCH_FAILED;
}

EM_RESULT ext_timer_add_entity ( EXT_CBTIMER_ENTITY* timer )
{
    UINT16 index;
    Status_t   ret;
    EXT_CBTIMER_ENTITY* new_timer;
    new_timer = NULL;

    for (index = 0; index < EXT_CBTIMER_MAX_ENTITIES; index++)
    {
        new_timer = &ext_cbtimer_entity[index];

        if (EXT_CBTIMER_ENTITY_FREE == new_timer->in_use)
        {
            new_timer->in_use = EXT_CBTIMER_ENTITY_IN_USE;
            break;
        }
        else
        {
            new_timer = NULL;
        }
    }

    if (NULL == new_timer)
    {
        printf(
            "FAILED to Allocate EXT New Timer Entity. Timer List FULL !\n");
        #ifdef EM_STATUS
        /* Timer List Full: Update EtherMind Status Flag */
        EM_status_set_bit (STATUS_BIT_TIMER_ENTITY_FULL, STATUS_BIT_SET);
        #endif /* EM_STATUS */
        return EXT_CBTIMER_QUEUE_FULL;
    }

    new_timer->next = NULL;
    new_timer->timeout = timer->timeout;
    new_timer->callback = timer->callback;
    new_timer->data_length = timer->data_length;

    if (new_timer->data_length > EXT_CBTIMER_STATIC_DATA_SIZE)
    {
        new_timer->allocated_data = timer->allocated_data;
    }
    else
    {
        EM_mem_copy
        (
            new_timer->static_data,
            timer->static_data,
            new_timer->data_length
        );
    }

    /* Start the timer */
    #ifdef EXT_CBTIMER_SUPPORT_REMAINING_TIME
    new_timer->start_timestamp = ext_cbtimer_get_ms_timestamp();
    #endif /* EM_TIMER_SUPPORT_REMAINING_TIME */
    /* Start timer. Set Timeout. This will also start the timer. */
    ret = CbTimerStart
          (
              ext_cbtimer_timeout_handler,
              &new_timer->handle,
              ((EXT_CBTIMEOUT_MILLISEC & new_timer->timeout) ?
               (new_timer->timeout & (UINT32)~(EXT_CBTIMEOUT_MILLISEC)):
               (new_timer->timeout * 1000)),
              &new_timer->timer_id
          );

    if (SUCCESS != ret)
    {
//        EM_TIMER_ERR("*** FAILED to Start timer\n");
//      printf("*** FAILED to Start timer, ret = %d\r\n", ret);
        return EXT_CBTIMER_FAILED_SET_TIME_EVENT;
    }

//    EM_TIMER_TRC("Successfully started Timer [ID: %02X]. Handle: 0x%02X\n",
//    new_timer->timer_id, timer->handle);
    timer->handle = new_timer->handle;
    timer->timer_id = new_timer->timer_id;

    /* If the Timer Q Empty */
    if (NULL == ext_cbtimer_q_start)
    {
        ext_cbtimer_q_start = ext_cbtimer_q_end = new_timer;
        return EM_SUCCESS;
    }

    ext_cbtimer_q_end->next = new_timer;
    ext_cbtimer_q_end = new_timer;
    return EM_SUCCESS;
}


EM_RESULT ext_timer_del_entity
(
    EXT_CBTIMER_ENTITY* timer,
    UCHAR free
)
{
    EXT_CBTIMER_ENTITY* current_timer, *previous_timer;

    /* Either None or One Element */
    if (ext_cbtimer_q_start == ext_cbtimer_q_end)
    {
        if (NULL == ext_cbtimer_q_start)
        {
            /* Queue is Empty */
            return EXT_CBTIMER_QUEUE_EMPTY;
        }
        else
        {
            if (timer == ext_cbtimer_q_start)
            {
                /* Queue has One Element */
                ext_cbtimer_q_start = ext_cbtimer_q_end = NULL;
            }
            else
            {
                /* Match NOT found in the Only element in Timer Queue */
                return EXT_CBTIMER_ENTITY_SEARCH_FAILED;
            }
        }
    }
    else
    {
        /* Queue has more than One Element */
        if (timer == ext_cbtimer_q_start)
        {
            /* Match in the First Element */
            ext_cbtimer_q_start = ext_cbtimer_q_start->next;
        }
        else
        {
            previous_timer = ext_cbtimer_q_start;
            current_timer = ext_cbtimer_q_start->next;

            while (NULL != current_timer)
            {
                if (timer == current_timer)
                {
                    previous_timer->next = current_timer->next;

                    if (current_timer == ext_cbtimer_q_end)
                    {
                        ext_cbtimer_q_end = previous_timer;
                    }

                    break;
                }

                previous_timer = current_timer;
                current_timer  = current_timer->next;
            }

            if (NULL == current_timer)
            {
                return EXT_CBTIMER_ENTITY_SEARCH_FAILED;
            }
        }
    }

    /* Free Allocated Data */
    if ((0x01 == free) &&
            (timer->data_length > EXT_CBTIMER_STATIC_DATA_SIZE))
    {
        ext_timer_free (timer->allocated_data);
    }

    ext_timer_init_entity(timer);
    return EM_SUCCESS;
}


EM_RESULT ext_timer_init_entity (EXT_CBTIMER_ENTITY* timer)
{
    timer->in_use = EXT_CBTIMER_ENTITY_FREE;
    timer->timeout = 0;
    timer->callback = NULL;
    timer->allocated_data = NULL;
    timer->data_length = 0;
    timer->next = NULL;
    #ifdef EM_TIMER_SUPPORT_REMAINING_TIME
    timer->start_timestamp = 0;
    #endif /* EM_TIMER_SUPPORT_REMAINING_TIME */
    return EM_SUCCESS;
}


#ifdef EXT_CBTIMER_SUPPORT_REMAINING_TIME

/*
    This function returns elapsed time in microsecond since system power on.
*/
UINT64 ext_cbtimer_get_ms_timestamp(void)
{
    return osal_GetSystemClock();
}

EM_RESULT EXT_cbtimer_get_remaining_time
(
    EXT_cbtimer_handle   handle,
    UINT32*           remaining_time_ms
)
{
    EXT_CBTIMER_ENTITY* timer;
    EM_RESULT     retval;
    UINT64        current_timestamp;
    UINT32        time_ms;

    if (NULL == handle)
    {
//        EM_TIMER_ERR(
//        "NULL Argument Unacceptable for Timer Handles.\n");
        return EXT_CBTIMER_HANDLE_IS_NULL;
    }

    /* Lock Timer */
//    timer_lock();
    timer = (EXT_CBTIMER_ENTITY*)handle;
    retval = ext_timer_search_entity(timer);

    if (EM_SUCCESS != retval)
    {
//        EM_TIMER_ERR(
//        "FAILED to Find Timer ELement for Handle %p. Error Code = 0x%04X\n",
//        (void *)handle, retval);
    }
    else
    {
        time_ms = ((EXT_CBTIMEOUT_MILLISEC & timer->timeout) ?
                   (timer->timeout & (UINT32)~(EXT_CBTIMEOUT_MILLISEC)) :
                   (timer->timeout * 1000));
        /* Get Current Time */
        /* Remaining Time = (Timeout in ms) - (Current Time - Timer Start Time) */
        current_timestamp = ext_cbtimer_get_ms_timestamp();

        if ((current_timestamp < timer->start_timestamp) ||
                (time_ms < (current_timestamp - timer->start_timestamp))
           )
        {
//            EM_TIMER_ERR(
//            "FAILED to Find Remaining Time.TO:%d < (CurT:%lld - StartT:%lld\n",
//            time_ms, current_timestamp, timer->start_timestamp);
        }
        else
        {
            time_ms -= (UINT32)(current_timestamp - timer->start_timestamp);
            *remaining_time_ms = time_ms;
//            EM_TIMER_TRC(
//            "[EM_TIMER] Remaining Time (ms): %d\n", time_ms);
            retval = EM_SUCCESS;
        }
    }

//    timer_unlock();
    return retval;
}
#endif /* EM_TIMER_SUPPORT_REMAINING_TIME */

EM_RESULT EXT_cbtimer_list_timer ( void )
{
    #ifdef EM_TIMERL_DEBUG
    UINT16 index, free;
    EXT_CBTIMER_ENTITY* timer;
    timer_lock();
    EM_TIMERL_TRC("\n");
    EM_TIMERL_TRC("========================================= \n");
    EM_TIMERL_TRC("Start Q = %p\n", ext_cbtimer_q_start);
    timer = ext_cbtimer_q_start;

    while(NULL != timer)
    {
        EM_TIMERL_TRC("    Handle = 0x%02X",
                      timer->handle);
        timer = timer->next;
    }

    EM_TIMERL_TRC("End   Q = %p\n", ext_cbtimer_q_end);
    free = 0;

    for (index = 0; index < EXT_CBTIMER_MAX_ENTITIES; index ++)
    {
        if (EXT_CBTIMER_ENTITY_FREE == ext_cbtimer_entity[index].in_use)
        {
            free ++;
        }
    }

    EM_TIMERL_TRC("Max Q Entity = %d, Free = %d\n",
                  EXT_CBTIMER_MAX_ENTITIES, free);
    EM_TIMERL_TRC("========================================= \n");
    EM_TIMERL_TRC("\n");
    timer_unlock();
    #endif /* EM_TIMERL_DEBUG */
    return EM_SUCCESS;
}

