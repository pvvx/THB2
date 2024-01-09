/*************
 att_internal.h
 SDK_LICENSE
***************/

#ifndef ATT_INTERNAL_H
#define ATT_INTERNAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
    INCLUDES
*/
#include "osal_cbtimer.h"

#include "l2cap.h"
#include "att.h"

/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/

/*********************************************************************
    TYPEDEFS
*/

// Function prototype to build an attribute protocol message
typedef uint16 (*attBuildMsg_t)( uint8* pBuf, uint8* pMsg );

/*********************************************************************
    VARIABLES
*/

/*********************************************************************
    FUNCTIONS
*/

extern uint16 attBuildExecuteWriteRsp( uint8* pBuf, uint8* pMsg );

extern uint16 attBuildHandleValueCfm( uint8* pBuf, uint8* pMsg );

extern bStatus_t attSendMsg( uint16 connHandle, attBuildMsg_t pfnBuildMsg,
                             uint8 opcode, uint8* pMsg );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ATT_INTERNAL_H */
