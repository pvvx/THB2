/**
 ****************************************************************************************
 *
 * @file ll_debug.h
 *
 * @brief This file defines the status used for debug
 *
 *
 * $Rev:  $
 *
 * SDK_LICENSE
 *
 ****************************************************************************************
 */


#ifndef _LL_DEBUG_H_
#define _LL_DEBUG_H_

#define    DEBUG_ENTER_SYSTEM_SLEEP                      0
#define    DEBUG_ENTER_MCU_SLEEP                         1
#define    DEBUG_WAKEUP                                  2

#define    DEBUG_ISR_EXIT                                3
#define    DEBUG_ISR_ENTRY                               4

#define    DEBUG_LL_HW_STX                               5
#define    DEBUG_LL_HW_SRX                               6

#define    DEBUG_LL_HW_TRX                               7
#define    DEBUG_LL_HW_RTX                               8
#define    DEBUG_LL_HW_TRLP                              9
#define    DEBUG_LL_HW_TRLP_EMPT                         10
#define    DEBUG_LL_HW_RTLP                              11
#define    DEBUG_LL_HW_RTLP_1ST                          12
#define    DEBUG_LL_HW_RTLP_EMPT                         13

#define    DEBUG_LL_HW_SET_STX                           14
#define    DEBUG_LL_HW_SET_SRX                           15

#define    DEBUG_LL_HW_SET_TRX                           16
#define    DEBUG_LL_HW_SET_RTX                           17
#define    DEBUG_LL_HW_SET_TRLP                          18
#define    DEBUG_LL_HW_SET_TRLP_EMPT                     19
#define    DEBUG_LL_HW_SET_RTLP                          20
#define    DEBUG_LL_HW_SET_RTLP_1ST                      21
#define    DEBUG_LL_HW_SET_RTLP_EMPT                     22

#define    DEBUG_LL_STATE_IDLE                           30                  
#define    DEBUG_LL_STATE_ADV_UNDIRECTED                 31 
#define    DEBUG_LL_STATE_ADV_DIRECTED                   32 
#define    DEBUG_LL_STATE_ADV_SCAN                       33 
#define    DEBUG_LL_STATE_ADV_NONCONN                    34 
#define    DEBUG_LL_STATE_SCAN                           35 
#define    DEBUG_LL_STATE_INIT                           36 
#define    DEBUG_LL_STATE_CONN_SLAVE                     37 
#define    DEBUG_LL_STATE_CONN_MASTER                    38 
#define    DEBUG_LL_STATE_DIRECT_TEST_MODE_TX            39 
#define    DEBUG_LL_STATE_DIRECT_TEST_MODE_RX            40 
#define    DEBUG_LL_STATE_MODEM_TEST_TX                  41 
#define    DEBUG_LL_STATE_MODEM_TEST_RX                  42 
#define    DEBUG_LL_STATE_MODEM_TEST_TX_FREQ_HOPPING     43 

#define    DEBUG_LL_SEND_ADV                             44

#define    DEBUG_LL_TIMER_EXPIRY_ENTRY                   50
#define    DEBUG_LL_TIMER_EXPIRY_EXIT                    51



#endif // _LL_DEBUG_H_
