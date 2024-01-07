/**************************************************************************************************
*******
**************************************************************************************************/

/**************************************************************************************************
    Filename:       gpio_demo.h
    Revised:        $Date $
    Revision:       $Revision $


**************************************************************************************************/

#ifndef __HAL_PERIPHERAL_H__
#define __HAL_PERIPHERAL_H__

#include "types.h"
#include "key.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define START_DEVICE_EVT                    0x0001
#define KEY_DEMO_UART_RX_EVT                0x0002
#define LIGHT_PRCESS_EVT                    0x0004



/*********************************************************************
    FUNCTIONS
*/
uint16 HalPeripheral_ProcessEvent( uint8 task_id, uint16 events );
void HalPeripheral_Init(uint8 task_id);
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* HEARTRATE_H */
