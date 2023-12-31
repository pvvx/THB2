/*
 ******************************************************************************
    History: Date;           Author;         Description
            07 Dec. 2015;   Chen, George;   file creation
 ******************************************************************************
*/


/*
 ******************************************************************************
    explanation
 ******************************************************************************
    Multiple Task Pending On Single Msg Queue.

         +============+             +===========+     +===========+
         |   msgq     |             |   pdat    |     |   pdat    |
         +============+             +===========+     +===========+
         |     ~      |             |     ~     |     |     ~     |
         |  pdat_head |-+    NULL/-+| pdat_list |/-~-/| pdat_list |->NULL
         |     ~      | +---------/+|     ~     |     |     ~     |
         +------------+             +-----------+     +-----------+

*/

/*
 ******************************************************************************
    Includes
 ******************************************************************************
*/
#include "core_queu.h"
// #include "core_imgr.h"


/*
 ******************************************************************************
    Definition
 ******************************************************************************
*/
// #define CORE_QUEU_CFGS_LOGS_TAG           "QUEU"
#ifdef  CORE_QUEU_CFGS_LOGS_TAG
/* ERROR */
#define log_err(fmt, ...)                                                \
    logs_logs_err(CORE_QUEU_CFGS_LOGS_TAG, fmt, ##__VA_ARGS__)

/* WARNING */
#define log_war(fmt, ...)                                                \
    logs_logs_war(CORE_QUEU_CFGS_LOGS_TAG, fmt, ##__VA_ARGS__)

/* INFORMATION */
#define log_inf(fmt, ...)                                                \
    logs_logs_inf(CORE_QUEU_CFGS_LOGS_TAG, fmt, ##__VA_ARGS__)

/* VERB */
#define log_ver(fmt, ...)                                                \
    logs_logs_ver(CORE_QUEU_CFGS_LOGS_TAG, fmt, ##__VA_ARGS__)

/* Function entry */
#define log_ent(fmt, ...)                                                \
    logs_logs_ent(fmt, ##__VA_ARGS__)

/* function exit */
#define log_exi(fmt, ...)                                                \
    logs_logs_exi(fmt, ##__VA_ARGS__)

#else
/* ERROR */
#define log_err(fmt, ...)

/* WARNING */
#define log_war(fmt, ...)

/* INFORMATION */
#define log_inf(fmt, ...)

/* VERB */
#define log_ver(fmt, ...)

/* Function entry */
#define log_ent(fmt, ...)

/* function exit */
#define log_exi(fmt, ...)

#endif /* CORE_QUEU_CFGS_LOGS_TAG */


/*
 ******************************************************************************
    Definition
 ******************************************************************************
*/


/*
 ******************************************************************************
    public function implementations
 ******************************************************************************
*/
/*
    # Name:  core_msgq_ini
    # Desc:  initialize a msg queue
    # Para:  msgq: msg queue to be initialised.
            name: name of new task, in c string.
    # rslt:  0 success; others failure.
*/
int32
core_sque_ini(core_sque_t* sque)
{
    int32 rslt = FALSE;
    uint32 size = 0;
    uint32 coun = 0;
    void* blck = NULL;
    log_ent("PARA>>msgq:0x%08x", sque);

    /* message pool */
    // sque->sque_pool; // [set by usr]
    // sque->pool_size; // [set by usr]

    /* message size */
    // sque->size_qdat; // [set by usr]

    if ( NULL == sque->sque_pool )
    {
        goto ERR_POOL;
    }

    /* ini queue */
    if ( TRUE != list_sque_ini(&sque->sque_dats) )
    {
        goto ERR_SQUE;
    }

    /* insert queue node */
    size = sizeof ( core_qdat_t ) + sque->size_qdat;
    coun = (sque->size_pool / size);
    blck = ((uint8*) sque->sque_pool) + coun * size;

    while ( coun -- )
    {
        blck = ((uint8*) blck) - size;

        // if ( TRUE != list_snod_ini_cir((list_snod_t*) blck) ) {
        if ( TRUE != list_snod_ini((list_snod_t*) blck, list_slst_enum_snod_cir) )
        {
            goto ERR_SQUE;
        }

        if ( TRUE != list_slst_add(&sque->sque_dats.list, (list_snod_t*) blck) )
        {
            goto ERR_SQUE;
        }
    }

    /* get ready to use */
    if ( TRUE != list_sque_fsh(&sque->sque_dats) )
    {
        goto ERR_SQUE;
    }

    rslt = TRUE;
    goto ERR_SUCC;
ERR_SQUE:
ERR_POOL:
ERR_SUCC:
    log_exi("RSLT>>rslt: %d", rslt);
    return ( rslt );
}

core_sque_t*
core_sque_new(uint32 size, uint32 coun)
{
    uint32     plsz = (size + sizeof ( core_qdat_t )) * coun;  // pool size of all msg blk + msg dat
    core_sque_t*    sque = NULL;
    log_ent("PARA>>size:%d, cunt:%d", size, coun);

    // allocate memory for task control block & task stack, shares one mem control block
    if ( NULL == (sque = (core_sque_t*) osal_mem_alloc(sizeof ( core_sque_t ) + plsz)) )
    {
        goto ERR_MMGR;
    }

    // initialize sque control block
    sque->sque_pool = ((uint8*) sque + sizeof ( core_sque_t ));
    sque->size_pool = plsz;
    sque->size_qdat = size;

    // sque->sque_msgs
    if ( TRUE != core_sque_ini(sque) )
    {
        goto ERR_SQUE;
    }

    goto ERR_SUCC;
ERR_SQUE:
    osal_mem_free(sque);
    sque = NULL;
ERR_MMGR:
ERR_SUCC:
    log_exi("RSLT>>sque:0x%08x", sque);
    return ( sque );
}

int32
core_sque_del(core_sque_t* sque)
{
    osal_mem_free(sque);
    return ( TRUE );
}

int32
core_sque_pop(core_sque_t* sque, void* data)
{
    int32     rslt = FALSE;
    log_ent("PARA>>sque:0x%08x, data:0x%08x", sque, data);

    if ( NULL == sque )
    {
        goto ERR_SQUE;
    }

    if ( TRUE != list_sque_pop(&sque->sque_dats, data, sque->size_qdat) )
    {
        goto ERR_SQUE;
    }

    rslt = TRUE;
    goto ERR_SUCC;
ERR_SQUE:
ERR_SUCC:
    log_exi("RSLT>>rslt: %d", rslt);
    return ( rslt );
}

int32
core_sque_psh(core_sque_t* sque, const void* data)
{
    int32     rslt = FALSE;
    log_ent("PARA>>sque:0x%08x, data:0x%08x", sque, data);

    if ( NULL == sque )
    {
        goto ERR_SQUE;
    }

    if ( TRUE != list_sque_psh(&sque->sque_dats, (void*)data, sque->size_qdat) )
    {
        goto ERR_SQUE;
    }

    rslt = TRUE;
    goto ERR_SUCC;
ERR_SQUE:
ERR_SUCC:
    log_exi("RSLT>>rslt: %d", rslt);
    return ( rslt );
}


/*
    # Name:  core_msgq_ini
    # Desc:  initialize a msg queue
    # Para:  msgq: msg queue to be initialised.
            name: name of new task, in c string.
    # rslt:  0 success; others failure.
*/
// int32
// core_msgq_ini(core_msgq_t* msgq)
// {
//     int32 rslt = FALSE;

//     log_ent("PARA>>msgq:0x%08x", msgq);

//     /* pending task */
//     msgq->pend_task = NULL;

//     /* init' queue */
//     if ( TRUE != core_sque_ini(&msgq->msgq_msgs) ) {
//         goto ERRN_SQUE;
//     }

//     rslt = TRUE;
//     goto   ERRN_SUCC;

//     ERRN_SQUE:
//     ERRN_SUCC:
//     log_exi("RSLT>>rslt: %d", rslt);
//     return ( rslt );
// }

// core_msgq_t*
// core_msgq_new(uint32 size, uint32 coun)
// {
//     uint32     plsz = (size + sizeof ( core_qdat_t )) * coun;    // pool size of all msg blk + msg dat
//     core_msgq_t*    msgq = NULL;

//     log_ent("PARA>>size:%d, cunt:%d", size, coun);

//     // allocate memory for task control block & task stack, shares one mem control block
//     if ( NULL == (msgq = (core_msgq_t*) core_mmgr_new(sizeof ( core_msgq_t ) + plsz)) ) {
//         goto ERR_MMGR;
//     }

//     // initialize msgq control block
//     msgq->msgq_msgs.sque_pool = ((uint8*) msgq + sizeof ( core_msgq_t ));
//     msgq->msgq_msgs.size_pool = plsz;
//     msgq->msgq_msgs.size_qdat = size;
//     //msgq->sque_msgs
//     if ( TRUE != core_msgq_ini(msgq) ) {
//         goto ERR_MSGQ;
//     }

//     goto ERR_SUCC;

//     ERR_MSGQ:
//     core_mmgr_del(msgq);
//     msgq = NULL;
//     ERR_MMGR:
//     ERR_SUCC:
//     log_exi("RSLT>>msgq:0x%08x", msgq);
//     return ( msgq );
// }

// int32
// core_msgq_del(core_msgq_t* msgq)
// {
//     return ( core_mmgr_del(msgq) );
// }

// int32
// core_msgq_pop(core_msgq_t* msgq, void* data)
// {
//     return ( core_sque_pop(&msgq->msgq_msgs, data) );
// }

// int32
// core_msgq_psh(core_msgq_t* msgq, const void* data)
// {
//     return ( core_sque_psh(&msgq->msgq_msgs, data) );
// }