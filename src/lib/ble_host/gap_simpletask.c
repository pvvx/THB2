/**************************************************************************************************

    Phyplus Microelectronics Limited confidential and proprietary.
    All rights reserved.

    IMPORTANT: All rights of this software belong to Phyplus Microelectronics
    Limited ("Phyplus"). Your use of this Software is limited to those
    specific rights granted under  the terms of the business contract, the
    confidential agreement, the non-disclosure agreement and any other forms
    of agreements as a customer or a partner of Phyplus. You may not use this
    Software unless you agree to abide by the terms of these agreements.
    You acknowledge that the Software may not be modified, copied,
    distributed or disclosed unless embedded on a Phyplus Bluetooth Low Energy
    (BLE) integrated circuit, either as a product or is integrated into your
    products.  Other than for the aforementioned purposes, you may not use,
    reproduce, copy, prepare derivative works of, modify, distribute, perform,
    display or sell this Software and/or its documentation for any purposes.

    YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
    PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
    INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
    NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
    PHYPLUS OR ITS SUBSIDIARIES BE LIABLE OR OBLIGATED UNDER CONTRACT,
    NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
    LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
    INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
    OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
    OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
    (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

**************************************************************************************************/

/*************************************************************************************************
    Filename:       gap_simpletask.c
    Revised:
    Revision:

    Description:    This file contains the GAP Simple Task.


**************************************************************************************************/

#if !( HOST_CONFIG & ( CENTRAL_CFG | PERIPHERAL_CFG ) )

/*******************************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "OSAL.h"
#include "hci_tl.h"

#include "gap.h"
#include "gap_internal.h"

/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/

/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    GLOBAL VARIABLES
*/
uint8 gapTaskID;                           // The GAP's Task ID
uint8 gapUnwantedTaskID = INVALID_TASK_ID; // The task ID of an app/profile that
// wants unexpected HCI messages.

// Callback function pointers for Peripheral
gapPeripheralCBs_t* pfnPeripheralCBs = NULL;

/*********************************************************************
    EXTERNAL VARIABLES
*/

/*********************************************************************
    EXTERNAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/

// Callback function pointers for Central
static gapCentralCBs_t* pfnCentralCBs = NULL;

/*********************************************************************
    LOCAL FUNCTIONS
*/
static uint8 gapProcessOSALMsg( osal_event_hdr_t* pMsg );
static uint8 gapProcessBLEEvents( osal_event_hdr_t* pMsg );
static uint8 gapProcessHCICmdCompleteEvt( hciEvt_CmdComplete_t* pMsg );

/*********************************************************************
    API FUNCTIONS
*/

/*********************************************************************
    Register your task ID to receive extra (unwanted) HCI status and
    complete events.

    Public function defined in gap.h.
*/
void GAP_RegisterForHCIMsgs( uint8 taskID )
{
    gapUnwantedTaskID = taskID;
}

/*********************************************************************
    @fn          GAP_Init

    @brief       GAP Task initialization function.

    @param       taskID - GAP task ID.

    @return      void
*/
void GAP_Init( uint8 task_id )
{
    // Save our own Task ID
    gapTaskID = task_id;
    // Register with HCI to receive events
    HCI_GAPTaskRegister( gapTaskID );
}

/*********************************************************************
    @fn          GAP_ProcessEvent

    @brief       GAP Task event processing function.

    @param       taskID - GAP task ID
    @param       events - GAP events.

    @return      events not processed
*/
uint16 GAP_ProcessEvent( uint8 task_id, uint16 events )
{
    VOID task_id; // OSAL required parameter that isn't used in this function

    if ( events & SYS_EVENT_MSG )
    {
        uint8* pMsg;

        if ( (pMsg = osal_msg_receive( gapTaskID )) != NULL )
        {
            if ( !gapProcessOSALMsg( (osal_event_hdr_t*)pMsg ) )
            {
                // Send it to the registered application.
                if ( gapUnwantedTaskID != INVALID_TASK_ID )
                {
                    // This message wasn't processed in the GAP, send it
                    // to an app/profile that wants it.
                    if ( osal_msg_send( gapUnwantedTaskID, pMsg ) == SUCCESS )
                    {
                        // return unprocessed events and don't dealloc
                        // the message because it was sent elsewhere
                        return (events ^ SYS_EVENT_MSG);
                    }
                }
            }

            // Release the OSAL message
            VOID osal_msg_deallocate( pMsg );
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if ( events & GAP_OSAL_TIMER_SCAN_DURATION_EVT )
    {
        // Device Discovery time expired, stop the scan
        if ( pfnCentralCBs && pfnCentralCBs->pfnProcessScanningEvt )
        {
            pfnCentralCBs->pfnProcessScanningEvt( NULL );
        }

        return ( events ^ GAP_OSAL_TIMER_SCAN_DURATION_EVT );
    }

    if ( events & GAP_END_ADVERTISING_EVT )
    {
        // Advertising time expired, stop advertising
        if ( pfnPeripheralCBs && pfnPeripheralCBs->pfnProcessAdvertisingEvt )
        {
            pfnPeripheralCBs->pfnProcessAdvertisingEvt( TRUE );
        }

        return ( events ^ GAP_END_ADVERTISING_EVT );
    }

    // If reach here, the events are unknown
    // Discard or make more handlers
    return 0;
}

/*********************************************************************
    @fn          GAP_NumActiveConnections

    @brief       Returns the number of active connections.

    @param       none

    @return      number of active connections
*/
uint8 GAP_NumActiveConnections( void )
{
    return ( 0 );
}

/*********************************************************************
    @fn      gapProcessOSALMsg

    @brief   Process an incoming task message.

    @param   pMsg - message to process

    @return  TRUE if processed and safe to deallocate, FALSE if passed
            off to another task.
*/
static uint8 gapProcessOSALMsg( osal_event_hdr_t* pMsg )
{
    uint8 safeToDealloc = TRUE;

    switch ( pMsg->event )
    {
    case HCI_GAP_EVENT_EVENT:
    {
        switch( pMsg->status )
        {
        case HCI_COMMAND_COMPLETE_EVENT_CODE:
            safeToDealloc = gapProcessHCICmdCompleteEvt( (hciEvt_CmdComplete_t*)pMsg );
            break;

        case HCI_LE_EVENT_CODE:
            // BLE Events
            safeToDealloc = gapProcessBLEEvents( pMsg );
            break;

        default:
            safeToDealloc = FALSE;  // Send to app
            break;
        }
    }
    break;

    default:
        safeToDealloc = FALSE;  // Send to app
        break;
    }

    return ( safeToDealloc );
}

/*********************************************************************
    @fn      gapProcessBLEEvents

    @brief   Process an incoming OSAL HCI BLE specific events.

    @param   pMsg - message to process

    @return  TRUE if processed and safe to deallocate, FALSE if passed
            off to another task.
*/
static uint8 gapProcessBLEEvents( osal_event_hdr_t* pMsg )
{
    uint8 safeToDealloc = TRUE;

    switch ( ((hciEvt_BLEAdvPktReport_t*)(pMsg))->BLEEventCode )
    {
    case HCI_BLE_ADV_REPORT_EVENT:
        if ( pfnCentralCBs && pfnCentralCBs->pfnProcessScanningEvt )
        {
            pfnCentralCBs->pfnProcessScanningEvt( (hciEvt_BLEAdvPktReport_t*)pMsg );
        }

        break;

    default:
        safeToDealloc = FALSE;
        break;
    }

    return ( safeToDealloc );
}

/*********************************************************************
    @fn      gapProcessHCICmdCompleteEvt

    @brief   Process an incoming OSAL HCI Command Complete Event.

    @param   pMsg - message to process

    @return  TRUE if processed and safe to deallocate, FALSE if passed
            off to another task.
*/
static uint8 gapProcessHCICmdCompleteEvt( hciEvt_CmdComplete_t* pMsg )
{
    uint8 safeToDealloc = TRUE;

    switch ( pMsg->cmdOpcode )
    {
    case HCI_LE_SET_RANDOM_ADDR:
        gapProcessRandomAddrComplete( *(pMsg->pReturnParam) );
        break;

    case HCI_LE_READ_BUFFER_SIZE:
        safeToDealloc = gapReadBufSizeCmdStatus(
                            (hciRetParam_LeReadBufSize_t*)pMsg->pReturnParam );
        break;

    case HCI_READ_BDADDR:
        safeToDealloc = gapReadBD_ADDRStatus( pMsg->pReturnParam[0],
                                              &(pMsg->pReturnParam[1]) );
        break;

    case HCI_LE_SET_SCAN_ENABLE:
        if ( *(pMsg->pReturnParam) == SUCCESS )
        {
            break;
        }

    // fallthrough
    case HCI_LE_SET_SCAN_PARAM:
        if ( pfnCentralCBs && pfnCentralCBs->pfnProcessHCICmdEvt )
        {
            safeToDealloc = pfnCentralCBs->pfnProcessHCICmdEvt( pMsg->cmdOpcode, pMsg );
        }

        break;

    case HCI_LE_CREATE_CONNECTION_CANCEL:
        break;

    case HCI_LE_SET_ADV_PARAM:
    case HCI_LE_SET_ADV_ENABLE:
    case HCI_LE_SET_SCAN_RSP_DATA:
    case HCI_LE_SET_ADV_DATA:
        if ( pfnPeripheralCBs && pfnPeripheralCBs->pfnProcessHCICmdCompleteEvt )
        {
            safeToDealloc = pfnPeripheralCBs->pfnProcessHCICmdCompleteEvt( pMsg );
        }

        break;

    default:
        safeToDealloc = FALSE;  // send this message to the app
        break;
    }

    return ( safeToDealloc );
}

/*********************************************************************
    @fn      gapRegisterCentral

    @brief   Register Central's processing function with GAP task.

    @param   pfnCBs - pointer to Central's processing function

    @return  none
*/
void gapRegisterCentral( gapCentralCBs_t* pfnCBs )
{
    pfnCentralCBs = pfnCBs;
}

/*********************************************************************
    @fn      gapRegisterPeripheral

    @brief   Register Peripheral's processing function with GAP task.

    @param   pfnCBs - pointer to Peripheral's processing function

    @return  none
*/
void gapRegisterPeripheral( gapPeripheralCBs_t* pfnCBs )
{
    pfnPeripheralCBs = pfnCBs;
}

#endif // !( CENTRAL_CFG | PERIPHERAL_CFG )

/****************************************************************************
****************************************************************************/

