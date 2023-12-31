/*
 ******************************************************************************
    File: util_slst.h

    # Hist:
            Date;           Author;         Description
            30 Oct. 2017;   Chen, George;   file creation, transported from prtos
 ******************************************************************************
*/
#ifndef __LIST_SLST_H__
#define __LIST_SLST_H__

/*
 ******************************************************************************
    Includes
 ******************************************************************************
*/
//#include "../comn/comn_cfgs.h"


#ifdef __cplusplus
extern "C" {
#endif

/* #######################  Single Linked List Types  ####################### */
/*
    USAGE
    refer to util_dlst.h
*/
typedef enum list_slst_enum_snod_def
{
    list_slst_enum_snod_lin = 0,
    list_slst_enum_snod_cir,
} list_snod_e;

typedef struct list_slst_stru_snod_def
{
    struct list_slst_stru_snod_def* next;   // next node
} list_snod_t;

/* slist types definitions */
typedef struct list_slst_stru_slst_def
{
    list_snod_t*    head;   // [set by sys] head node
    list_snod_t*    tail;   // [set by sys] tail node
    list_snod_t*    iter;   // [set by sys] node iterator
    unsigned int    coun;   // [set by sys] counts of entity
} list_slst_t;

typedef struct list_slst_stru_sque_def
{
    list_slst_t     list;
    list_snod_t*    post;   // [set by sys] head node to pop out
    list_snod_t*    pend;   // [set by sys] tail node to push in
    unsigned int    coun;   // [set by sys] counts of records
} list_sque_t;

/* ########################## function prototypes  ########################## */
/*
    # Name:  util_slst_ini
    # Desc:  initialize single linked list
    # Para:  node: list being initialized
    # return:node
*/
signed int
list_snod_ini(list_snod_t* snod, list_snod_e type);

// signed int
// list_snod_ini_lin(list_snod_t* snod);

// signed int
// list_snod_ini_cir(list_snod_t* snod);

// /*
//  * # Name:  util_slst_ins_aft
//  * # Desc:  insert node after a list, link member pointers each other
//  * # Para:  slist1: list to which new list being put after
//  *          slist2: list which being put after a list
//  * # return:slist1
//  */
// signed int
// list_snod_ins_aft(list_snod_t* nod0, list_snod_t* nod1);

// /*
//  * # Name:  util_slst_rmv_aft
//  * # Desc:  remove a node after a list.
//  * # Para:  slist: list after which a node being removed.
//  * # return:slist
//  */
// signed int
// list_snod_rmv_aft(list_snod_t* snod);

/*
    # Name:  list_slst_ini
    # Desc:  initialize a single linked list.
    # Para:  slst: .
    # rslt:  0 failure otherwise success.
*/
signed int
list_slst_ini(list_slst_t* slst);

/*
    # Name:  list_slst_add
    # Desc:  add node to list tail.
    # Para:  slst: .
            snod:
    # rslt:  0 failure otherwise success.
*/
signed int
list_slst_add(list_slst_t* slst, list_snod_t* snod);

/*
    # Name:  list_slst_rmv
    # Desc:  search & remove node in list.
    # Para:  slst: .
            node:
    # rslt:  0 failure otherwise success.
*/
signed int
list_slst_rmv(list_slst_t* slst, list_snod_t* snod);

/*
    # Name:  list_slst_has
    # Desc:  check if the list contains the node.
    # Para:  list: list .
            node: node .
    # return:TRUE sucess, otherwise FALSE.
*/
signed int
list_slst_has(list_slst_t* slst, const list_snod_t* snod);

/*
    # Name:  list_dlst_emu
    # Desc:  iterates each node.
    # Para:  list:
    # return:num of nodes
*/
signed int
list_slst_emu(list_slst_t* slst);

/*
    # Name:  list_dlst_nxt
    # Desc:  get iterated node.
    # Para:  list:
    # return:num of nodes
*/
list_snod_t*
list_slst_nxt(list_slst_t* slst);

/*
    # Name:  list_sque_ini
    # Desc:  initialize single linked list queue.
    # Para:  list:
    # return:num of nodes
*/
signed int
list_sque_ini(list_sque_t* sque);

/*
    # Name:  util_slst_fsh
    # Desc:  reset queue.
    # Para:  list: list should be operated on.
    # return:TRUE sucess, otherwise FALSE.
*/
signed int
list_sque_fsh(list_sque_t* sque);

signed int
list_sque_pop(list_sque_t* sque, void* data, unsigned int size);

signed int
list_sque_psh(list_sque_t* sque, void* data, unsigned int size);

#ifdef __cplusplus
}
#endif

#endif  // __UTIL_SLST_H__
