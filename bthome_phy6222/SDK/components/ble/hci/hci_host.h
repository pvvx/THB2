/*************
 hci_host.h
 SDK_LICENSE
***************/
#ifndef HCI_HOST_H
#define HCI_HOST_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
    INCLUDES
*/
#include "OSAL.h"
#include "osal_bufmgr.h"
#include "hci.h"
#include "hci_task.h"


/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/

/* HCI packet header length */
#define HCI_EVT_HEADER_LEN             3  /* packet type + evt code(1) + len(1) */
#define HCI_DATA_HEADER_LEN            5  /* packet type + connection  handle(2) + len(2) */

/* First 12 bits of the HCI data packet is connection handle */
#define HCI_CONNECTION_HANDLE_MASK                0x0FFF
#define HCI_PB_MASK                               0x03
/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    GLOBAL VARIABLES
*/


/*********************************************************************
    FUNCTIONS - API
*/


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* HCI_HOST_H */


