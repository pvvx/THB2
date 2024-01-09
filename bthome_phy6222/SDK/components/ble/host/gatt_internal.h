/*************
 gatt_internal.h
 SDK_LICENSE
***************/

#ifndef GATT_INTERNAL_H
#define GATT_INTERNAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
    INCLUDES
*/
#include "osal_cbtimer.h"

#include "att.h"
#include "gatt.h"

/*********************************************************************
    MACROS
*/
#define TIMER_VALID( id )                ( ( (id) != INVALID_TIMER_ID ) && \
                                           ( (id) != TIMEOUT_TIMER_ID ) )

#define TIMER_STATUS( id )               ( (id) == TIMEOUT_TIMER_ID ? bleTimeout : \
                                           (id) == INVALID_TIMER_ID ? SUCCESS : blePending )

/*********************************************************************
    CONSTANTS
*/

/*********************************************************************
    TYPEDEFS
*/
// Srtucture for Attribute Version Information attribute
typedef struct
{
    uint8 attVersion;        // Attribute Protocol Version
    uint8 gattVersion;       // Generic Attribute Profile Version
    uint16 manufacturerName; // Manufacturer Name
} gattVersionInfo_t;

// Function prototype to parse an attribute protocol request message
typedef bStatus_t (*gattParseReq_t)( uint8 sig, uint8 cmd, uint8* pParams, uint16 len, attMsg_t* pMsg );

// Function prototype to parse an attribute protocol response message
typedef bStatus_t (*gattParseRsp_t)( uint8* pParams, uint16 len, attMsg_t* pMsg );

// Function prototype to process an attribute protocol message
typedef bStatus_t (*gattProcessMsg_t)( uint16 connHandle,  attPacket_t* pPkt );

// Function prototype to process an attribute protocol request message
typedef bStatus_t (*gattProcessReq_t)( uint16 connHandle,  attMsg_t* pMsg );

/*********************************************************************
    VARIABLES
*/
extern uint8 gattTaskID;

/*********************************************************************
    FUNCTIONS
*/
extern void gattRegisterServer( gattProcessMsg_t pfnProcessMsg );

extern void gattRegisterClient( gattProcessMsg_t pfnProcessMsg );

extern bStatus_t gattNotifyEvent( uint8 taskId, uint16 connHandle, uint8 status,
                                  uint8 method, gattMsg_t* pMsg );

extern void gattStartTimer( pfnCbTimer_t pfnCbTimer, uint8* pData,
                            uint16 timeout, uint8* pTimerId );

extern void gattStopTimer( uint8* pTimerId );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* GATT_INTERNAL_H */
