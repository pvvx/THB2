/*
 ******************************************************************************
    # Hist:
            Date;           Author;         Description
            30 Oct. 2017;   Chen, George;   file creation, transported from prtos
 ******************************************************************************
*/


/*
 ******************************************************************************
    Includes
 ******************************************************************************
*/

// #include <string.h>
#include "OSAL.h"
#include "list_slst.h"
// #include "logs_logs.h"



/*
 ******************************************************************************
    defines
 ******************************************************************************
*/
// #define LIST_SLST_DEFS_LOGS_TAG     "SLST"
#ifdef  LIST_SLST_DEFS_LOGS_TAG
/* ERROR */
#define log_err(fmt, ...)                                                \
    logs_logs_err(LIST_SLST_DEFS_LOGS_TAG, fmt, ##__VA_ARGS__)

/* WARNING */
#define log_war(fmt, ...)                                                \
    logs_logs_war(LIST_SLST_DEFS_LOGS_TAG, fmt, ##__VA_ARGS__)

/* INFORMATION */
#define log_inf(fmt, ...)                                                \
    logs_logs_inf(LIST_SLST_DEFS_LOGS_TAG, fmt, ##__VA_ARGS__)

/* VERB */
#define log_ver(fmt, ...)                                                \
    logs_logs_ver(LIST_SLST_DEFS_LOGS_TAG, fmt, ##__VA_ARGS__)

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

#endif /* LIST_SLST_DEFS_LOGS_TAG */


/*
 ******************************************************************************
    Private Functions
 ******************************************************************************
*/


/*
    # name:
    # desc:
    # para:
    # rslt:
*/
static signed int
list_snod_ini_lin(list_snod_t* snod)
{
    signed int rslt = 0;

    if ( 0 == snod )    /* validation check */
    {
        //
        rslt = 0;
    }
    else                /* initialize to 0  */
    {
        snod->next = 0;
        rslt = 1;
    }

    return ( rslt );
}

/*
    # name:
    # desc:
    # para:
    # rslt:
*/
static signed int
list_snod_ini_cir(list_snod_t* snod)
{
    signed int rslt = 0;

    if ( 0 == snod )    /* validation check */
    {
        //
        rslt = 0;
    }
    else                /* initial to self  */
    {
        snod->next = snod;
        rslt = 1;
    }

    return ( rslt );
}

/*
    # name:
    # desc:
    # para:
    # rslt:
*/
static signed int
list_snod_ins_aft(list_snod_t* nod0, list_snod_t* nod1)
{
    signed int rslt = 0;

    /* validation checking */
    if ( 0 == nod0 || 0 == nod1 )
    {
        rslt = 0;
        goto ERR_INV;
    }

    /* insert list node */
    nod1->next = nod0->next;
    nod0->next = nod1;
    /* result success */
    rslt = 1;
    goto ERR_SUC;
ERR_INV:
ERR_SUC:
    return ( rslt );
}

/*
    # name:
    # desc:
    # para:
    # rslt:
*/
static signed int
list_snod_rmv_aft(list_snod_t* snod)
{
    signed int rslt = 0;

    /* validation checking */
    if ( 0 == snod )
    {
        rslt = 0;
        goto ERR_INV;
    }

    if ( 0 != snod->next )
    {
        snod->next = snod->next->next;
    }

    /* result success */
    rslt = 1;
    goto ERR_SUC;
ERR_INV:
ERR_SUC:
    return ( rslt );
}

/*
 ******************************************************************************
    Public Functions
 ******************************************************************************
*/
signed int
list_snod_ini(list_snod_t* snod, list_snod_e type)
{
    signed int rslt = 0;

    if ( list_slst_enum_snod_lin == type )
    {
        rslt = list_snod_ini_lin(snod);
    }
    else if ( list_slst_enum_snod_cir == type )
    {
        rslt = list_snod_ini_cir(snod);
    }

    return ( rslt );
}

/*
 ******************************************************************************
    Public Functions : slst
 ******************************************************************************
*/
/*
    # name:
    # desc:
    # para:
    # rslt:
*/
signed int
list_slst_ini(list_slst_t* slst)
{
    signed int rslt = 0;

    /* validation checking */
    if ( 0 == slst )
    {
        rslt = 0;
        goto ERR_INV;
    }

    slst->head = 0; // [set by sys] head node
    slst->tail = 0; // [set by sys] tail node
    slst->iter = 0; // [set by sys] iter node
    slst->coun = 0; // [set by sys] list size of entity
    /* result success */
    rslt = 1;
    goto ERR_SUC;
ERR_INV:
ERR_SUC:
    return ( rslt );
}

/*
    # name:
    # desc: insert at tail
    # para:
    # rslt:
*/
signed int
list_slst_add(list_slst_t* slst, list_snod_t* snod)
{
    signed int      rslt = 0;
    list_snod_t*    curr = 0;

    if ( 0 == snod )
    {
        rslt = 0;
        goto ERR_INV;
    }

    list_slst_emu(slst);

    while ( 1 )
    {
        curr = list_slst_nxt(slst);

        if ( 0 == curr )        // not found
        {
            if ( 0 >= slst->coun )              // empty list?,
            {
                slst->head = snod;              // then, become the first node
                slst->tail = snod;
            }
            else                                // otherwise, insert @ tail
            {
                // ignore result check, always true at this case
                list_snod_ins_aft(slst->tail, snod);
                slst->tail = snod;              // update tail
            }

            slst->coun ++;                      // update list capability
            rslt = 1;
            goto ERR_SUC;       // SUCCESS
        }
        else if ( snod == curr )      // found
        {
            rslt = 0;
            goto ERR_DOE;       // DOEXIST
        }
    }

ERR_DOE:
ERR_INV:
ERR_SUC:
    return ( rslt );
}

/* TODO: MARK HERE */
/*
    # name:
    # desc: remove at head
    # para:
    # rslt:
*/
signed int
list_slst_rmv(list_slst_t* slst, list_snod_t* snod)
{
    signed int      rslt = 0;
    list_snod_t*    prev = 0;
    list_snod_t*    curr = 0;

    /* validation checking */
    if ( 0 == snod )
    {
        rslt = 0;
        goto ERR_INV;
    }

    list_slst_emu(slst);

    while ( 1 )
    {
        curr = list_slst_nxt(slst);

        if ( 0 == curr )        // not found
        {
            rslt = 0;
            goto ERR_NOF;
        }
        else if ( snod == curr )      // found
        {
            slst->coun --;      // [set by sys] list size of entity

            if (    0 >= slst->coun )
            {
                slst->tail = slst->head = 0;
            }
            else if ( snod == slst->head )
            {
                slst->head = snod->next;
            }
            else if ( snod == slst->tail )
            {
                slst->tail = prev;
            }

            // remove given snod
            if ( 0 != prev )
            {
                list_snod_rmv_aft(prev);
            }

            rslt = 1;
            goto ERR_SUC;
        }

        prev = curr;
    }

ERR_NOF:
ERR_INV:
ERR_SUC:
    return ( rslt );
}

/*
    # name:
    # desc:
    # para:
    # rslt:
*/
signed int
list_slst_has(list_slst_t* slst, const list_snod_t* snod)
{
    signed int      rslt = 0;
    list_snod_t*    curr = 0;

    /* validation checking */
    if ( 0 == snod )
    {
        rslt = 0;
        goto ERR_INV;
    }

    list_slst_emu(slst);

    while ( 1 )
    {
        curr = list_slst_nxt(slst);

        if (    0 == curr )     // not found
        {
            rslt = 0;
            break;
        }
        else if ( snod == curr )      // found
        {
            rslt = 1;
            break;
        }
    }

ERR_INV:
    return ( rslt );
}

/*
    # name:
    # desc:
    # para:
    # rslt:
*/
signed int
list_slst_emu(list_slst_t* slst)
{
    return ( 0 != (slst->iter = slst->head) ? 1 : 0 );
}

/*
    # name:
    # desc:
    # para:
    # rslt:
*/
list_snod_t*
list_slst_nxt(list_slst_t* slst)
{
    list_snod_t* rslt = 0;

    if ( 0 != slst )
    {
        rslt = slst->iter;
    }

    if ( 0 != rslt )
    {
        slst->iter = rslt->next;
    }

    if ( slst->iter == slst->head )
    {
        rslt = 0;
    }

    return ( rslt );
}

/*
 ******************************************************************************
    Public Functions : sque
 ******************************************************************************
*/
/*
    # name:
    # desc:
    # para:
    # rslt:
*/
signed int
list_sque_ini(list_sque_t* sque)
{
    signed int rslt = 0;

    if ( 0 == sque )
    {
        rslt = 0;
        goto ERR_INV;
    }

    // ignore validation check, always result true in this case
    list_slst_ini(&sque->list);
    /*
        initialize post & pend ptrs to 0.
        sque->list now is a empty list.
        util_sque_fsh should called after the sque->list is filled
        before using the sque.
    */
    sque->post = 0; // [set by sys] head node to pop out/out box
    sque->pend = 0; // [set by sys] tail node to push in/in  box
    sque->coun = 0; // [set by sys] counts of records
    /* result success */
    rslt = 1;
    goto ERR_SUC;
ERR_INV:
ERR_SUC:
    return ( rslt );
}

/*
    # name:
    # desc:
    # para:
    # rslt:
*/
signed int
list_sque_fsh(list_sque_t* sque)
{
    signed int rslt = 0;

    /* validation checking */
    if ( 0 == sque )
    {
        rslt = 0;
        goto ERR_INV;
    }

    sque->post = sque->list.head;
    sque->pend = sque->list.head;
    sque->coun = 0;
    /* result success */
    rslt = 1;
    goto ERR_SUC;
ERR_INV:
ERR_SUC:
    return ( rslt );
}

/*
    # name:
    # desc:
    # para:
    # rslt:
*/
signed int
list_sque_pop(list_sque_t* sque, void* data, unsigned int size)
{
    signed int rslt = 0;

    /* validation checking */
    /* NOTE: dst and scpy validation should be checked by user or in scpy */
    if ( 0 == sque )
    {
        rslt = 0;
        goto ERR_INV;
    }

    /* NOTE: do nothing on empty queue */
    if ( 0 >= sque->coun )
    {
        rslt = 0;
        goto ERR_INV;
    }

    /* NOTE: push & pops only works on circular queue */
    if ( sque->list.head != sque->list.tail->next )
    {
        rslt = 0;
        goto ERR_INV;
    }

    osal_memcpy(data, sque->post+1, size);
    /* maintainence */
    sque->post = sque->post->next;
    sque->coun --;
    /* result success */
    rslt = 1;
    goto ERR_SUC;
ERR_INV:
ERR_SUC:
    return ( rslt );
}

/*
    # name:
    # desc:
    # para:
    # rslt:
*/
signed int
list_sque_psh(list_sque_t* sque, void* data, unsigned int size)
{
    signed int rslt = 0;

    /* validation checking */
    /* NOTE: src and scpy validation should be checked by user or in scpy */
    if ( 0 == sque )
    {
        rslt = 0;
        goto ERR_INV;
    }

    /* NOTE: do nothing on full list */
    if ( sque->coun >= sque->list.coun )
    {
        rslt = 0;
        goto ERR_INV;
    }

    /* NOTE: push & pops only works on circular queue */
    if ( sque->list.head != sque->list.tail->next )
    {
        rslt = 0;
        goto ERR_INV;
    }

    osal_memcpy(sque->pend+1, data, size);
    sque->pend = sque->pend->next;
    sque->coun ++;
    /* result success */
    rslt = 1;
    goto ERR_SUC;
ERR_INV:
ERR_SUC:
    return ( rslt );
}

