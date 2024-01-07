/*
 ******************************************************************************
    History: Date;           Author;         Description
        07 Dec. 2015;   Chen, George;   file creation
 ******************************************************************************
*/

#ifndef __CORE_QUEU_H__
#define __CORE_QUEU_H__


/*
 ******************************************************************************
    Includes
 ******************************************************************************
*/
// #include "tbox_cfgs.h"
// #include "comn_cfgs.h"
// #include "core_task.h"
#include "OSAL.h"
#include "list_slst.h"

#ifdef __cplusplus
extern "C" {
#endif


/* ###########################  msg queue Types  ########################### */

/* msg types definitions */
typedef struct core_msgq_stru_qdat_def
{
    list_snod_t qdat_node;
} core_qdat_t;

/* msgq types definitions */
typedef struct core_msgq_stru_sque_def
{
    // message pool
    void*         sque_pool;    // [set by usr] data pool, ptr from malloc
    uint32         size_pool;  // [set by usr] pool size in bytes

    // message ctrlq
    uint32         size_qdat;   // [set by usr] a data size in bytes (excl. core_msg_t)
    list_sque_t         sque_dats;   // [set by sys] circle singled linked list
} core_sque_t;

/* msgq types definitions */
// typedef struct core_msgq_stru_msgq_def {
//     //
//     // void*         pend_task;

//     core_sque_t         msgq_msgs;
// } core_msgq_t, *core_msgq_h;
/*
 ******************************************************************************
    function prototypes
 ******************************************************************************
*/
int32
core_sque_ini(core_sque_t* sque);

core_sque_t*
core_sque_new(uint32 size, uint32 coun);

int32
core_sque_del(core_sque_t* sque);

int32
core_sque_pop(core_sque_t* sque, void* data);

int32
core_sque_psh(core_sque_t* sque, const void* data);


// int
// core_msgq_ini(core_msgq_t* msgq);

// core_msgq_t*
// core_msgq_new(uint32 size, uint32 coun);

// int
// core_msgq_del(core_msgq_t* msgq);

// int
// core_msgq_pop(core_msgq_t* msgq, void* data);

// int
// core_msgq_psh(core_msgq_t* msgq, const void* data);

#ifdef __cplusplus
}
#endif

#endif  // __CORE_CMN_H__
