/**************************************************************************************************

    Wuxi CMOSTEK Microelectronics Co., Limited confidential and proprietary.
    All rights reserved.

    IMPORTANT: All rights of this software belong to Wuxi CMOSTEK Microelectronics Co.,
    Limited ("CMOSTEK"). Your use of this Software is limited to those
    specific rights granted under  the terms of the business contract, the
    confidential agreement, the non-disclosure agreement and any other forms
    of agreements as a customer or a partner of CMOSTEK. You may not use this
    Software unless you agree to abide by the terms of these agreements.
    You acknowledge that the Software may not be modified, copied,
    distributed or disclosed unless embedded on a CMOSTEK Bluetooth Low Energy
    (BLE) integrated circuit, either as a product or is integrated into your
    products.  Other than for the aforementioned purposes, you may not use,
    reproduce, copy, prepare derivative works of, modify, distribute, perform,
    display or sell this Software and/or its documentation for any purposes.

    YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
    PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
    INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
    NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
    CMOSTEK OR ITS SUBSIDIARIES BE LIABLE OR OBLIGATED UNDER CONTRACT,
    NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
    LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
    INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
    OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
    OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
    (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

**************************************************************************************************/

/**************************************************************************************************
    Filename:       osal_cbtimer.h
    Revised:
    Revision:

    Description:    This file contains the Callback Timer definitions.


 **************************************************************************************************/

#ifndef CBTIMER_H
#define CBTIMER_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
    INCLUDES
*/

/*********************************************************************
    CONSTANTS
*/
// Invalid timer id
#define INVALID_TIMER_ID                           0xFF

// Timed out timer
#define TIMEOUT_TIMER_ID                           0xFE

/*********************************************************************
    VARIABLES
*/

/*********************************************************************
    MACROS
*/
#define  CBTIMER_NUM_TASKS  1                 // set by HZF, align to  project setting
#if ( CBTIMER_NUM_TASKS == 0 )
#error Callback Timer module shouldn't be included (no callback timer is needed)!
#elif ( CBTIMER_NUM_TASKS == 1 )
#define CBTIMER_PROCESS_EVENT( a )          ( a )
#elif ( CBTIMER_NUM_TASKS == 2 )
#define CBTIMER_PROCESS_EVENT( a )          ( a ), ( a )
#else
#error Maximum of 2 callback timer tasks are supported! Modify it here.
#endif

/*********************************************************************
    TYPEDEFS
*/

// Callback Timer function prototype. Callback function will be called
// when the associated timer expires.
//
// pData - pointer to data registered with timer
//
typedef void (*extpfnCbTimer_t)( uint8* pData );

/*********************************************************************
    VARIABLES
*/

/*********************************************************************
    FUNCTIONS
*/

/*
    Callback Timer task initialization function.
*/
void CbTimerInit( uint8 taskId );

/*
    Callback Timer task event processing function.
*/
uint16 CbTimerProcessEvent( uint8 taskId, uint16 events );

/*
    Function to start a timer to expire in n mSecs.
*/
Status_t CbTimerStart( extpfnCbTimer_t pfnCbTimer, uint8* pData,
                       uint16 timeout, uint8* pTimerId );

/*
    Function to update a timer that has already been started.
*/
Status_t CbTimerUpdate( uint8 timerId, uint16 timeout );

/*
    Function to stop a timer that has already been started.
*/
Status_t CbTimerStop( uint8 timerId );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* CBTIMER_H */
