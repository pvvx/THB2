/***********
 rflib.h
 SDK_LICENSE
*************/

#ifndef __RF_LIB_H
#define __RF_LIB_H
#include "hci.h"
#include "hci_tl.h"
#include "ll.h"
#include "ll_def.h"

void rflib_vesion(uint8_t* major, uint8_t* minor, uint8_t* revision, char* test_build);
void check_PerStatsProcess(void);
int8  LL_PLUS_GetCurrentRSSI(void);

hciStatus_t HCI_LE_SetHostChanClassificationCmd(uint8* chanMap);

void LL_PLUS_GetCurrentPduDle(uint8_t connId, ll_pdu_length_ctrl_t* ppdu);

#endif

