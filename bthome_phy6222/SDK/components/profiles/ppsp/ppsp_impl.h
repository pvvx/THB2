#ifndef PPSP_IMPL_H
#define PPSP_IMPL_H

#ifdef __cplusplus
extern "C"
{
#endif


/*********************************************************************
    INCLUDES
*/
#include "comdef.h"

/*********************************************************************
    CONSTANTS
*/
#define PPSP_IMPL_CFGS_TIMR_TOUT                              (100)

/*
    callback of tick, on which used as watch guard and ticker for xfer
*/
void
ppsp_impl_appl_timr_hdlr(void);

uint8
ppsp_impl_ini(void);

uint32
ppsp_impl_get_stat(void);

uint8
ppsp_impl_get_pids(uint8* pids);

uint8
ppsp_impl_get_macs(uint8* macs);

uint8
ppsp_impl_get_scrt(uint8* scrt);

uint8
ppsp_impl_cal_keys(const uint8* rand, uint8 rsiz, const uint8* pids, uint8 psiz, const uint8* macs, uint8 msiz, const uint8* scrt, uint8 ssiz);

uint8
ppsp_impl_enc_text(uint8* text, uint8* cipr);

uint8
ppsp_impl_dec_cipr(uint8* text, uint8* cipr);

/*
    callback of connection stat changes
*/
void
ppsp_impl_ack_conn(uint8 flag);

// uint8
// ppsp_impl_set_msgs();

#ifdef __cplusplus
}
#endif

#endif /*  */
