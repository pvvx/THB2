/*************
 hci_task.h
 SDK_LICENSE
***************/
#ifndef HCI_TASK_H
#define HCI_TASK_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
    INCLUDES
*/
#include "OSAL.h"
#include "hci.h"
#include "uart.h"
#include "hci_host.h"

#include "hal.h"     // added by ZJP


/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/


/* UART port */
#define HCI_UART_PORT                  HAL_UART_PORT_0
#define HCI_UART_BR                    HAL_UART_BR_38400
#define HCI_UART_FC                    TRUE
#define HCI_UART_FC_THRESHOLD          48
#define HCI_UART_RX_BUF_SIZE           128
#define HCI_UART_TX_BUF_SIZE           128
#define HCI_UART_IDLE_TIMEOUT          6
#define HCI_UART_INT_ENABLE            TRUE

/* HCI Event List */
#define HCI_EVENT_SEND_DATA            0x01
#define HCI_EVENT_SEND_CMD             0x02
#define HCI_HOST_PARSE_EVT             0x04
#define HCI_HOST_INCOMING_EVT          0x08
#define HCI_HOST_INCOMING_DATA         0x10


/* Define the osal queue size for data and cmd */
#define HCI_HOST_MAX_DATAQUEUE_SIZE    20
#define HCI_HOST_MAX_CMDQUEUE_SIZE     20

/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    GLOBAL VARIABLES
*/
osal_msg_q_t HCI_HostDataQueue;

uint8 hciHostNumQueuedData;           /* Number of data packets queued */
const uint8 hciHostMaxNumDataQueue;   /* Max number of data packets queued */

/*********************************************************************
    FUNCTIONS - API
*/
extern Status_t HCI_AddDataQueue( void* buf );
extern Status_t HCI_AddCmdQueue( void* buf );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* HCI_TASK_H */






