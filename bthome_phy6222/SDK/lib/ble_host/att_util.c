/*************************************************************************************************
    Filename:       att_util.c
    Revised:
    Revision:

    Description:    This file contains the the utility functions used by
                  the Attribute Protocol Client and Server.

	SDK_LICENSE

**************************************************************************************************/


/*********************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "osal_bufmgr.h"
#include "linkdb.h"
#include "sm.h"

#include "att_internal.h"

/*********************************************************************
    MACROS
*/
// Attribute opcode fields
#define attMethod( opcode )                ( (opcode) & ATT_METHOD_BITS )
#define attCmdFlag( opcode )               ( (opcode) & ATT_CMD_FLAG_BIT )
#define attAuthenSigFlag( opcode )         ( (opcode) & ATT_AUTHEN_SIG_FLAG_BIT )

/*********************************************************************
    CONSTANTS
*/
// Length of Exchange MTU Request: client receive MTU size (2)
#define EXCHANGE_MTU_REQ_SIZE              2

// Length of Exchange MTU Response: server receive MTU size (2)
#define EXCHANGE_MTU_RSP_SIZE              2

// Length of Error Response: Command opcode in error (1) + Attribute handle in error (2) + Status code (1)
#define ERROR_RSP_SIZE                     4

// Length of Find Information Request's fixed fields: First handle number (2) + Last handle number (2)
#define FIND_INFO_REQ_FIXED_SIZE           4

// Length of Find Information Response's fixed field: Format (1)
#define FIND_INFO_RSP_FIXED_SIZE           1

// Length of Find By Type Value Request's fixed fields: First handle number (2) + Last handle number (2)
#define FIND_BY_TYPE_VALUE_REQ_FIXED_SIZE  4

// Length of Read By Type Response's fixed fields: Length (1) + Attribute handle number (2)
#define READ_BY_TYPE_RSP_FIXED_SIZE        3

// Length of Read Request: Attribute Handle (2)
#define READ_REQ_SIZE                      2

// Length of Write Request's fixed field: Attribute Handle (2)
#define WRITE_REQ_FIXED_SIZE               2

// Length of Read Blob Request: Attribute Handle (2) + Value Offset (2)
#define READ_BLOB_REQ_SIZE                 4

// Length of Prepare Write Response's fixed size: Attribute Handle (2) + Value Offset (2)
#define PREPARE_WRITE_RSP_FIXED_SIZE       4

// Length of Execute Write Request: Flags (1)
#define EXECUTE_WRITE_REQ_SIZE             1

// Length of Handle Value Indication's fixed size: Attribute Handle (2)
#define HANDLE_VALUE_IND_FIXED_SIZE        2

// Length of Authentication Signature field
#define AUTHEN_SIG_LEN                     12

/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    GLOBAL VARIABLES
*/

// Bluetooth base UUID for Attribute Protocol: 00000000-0000-1000-8000-00805F9B34FB
CONST uint8 btBaseUUID[ATT_UUID_SIZE] =
{
    0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//add for MTU size exchange
uint16 g_ATT_MTU_SIZE_MAX   = ATT_MTU_SIZE;

uint16 g_ATT_MAX_NUM_HANDLES_INFO   =( ( ATT_MTU_SIZE_MIN - 1 ) / 4 );
uint16 g_ATT_MAX_NUM_HANDLES        =( ( ATT_MTU_SIZE_MIN - 1 ) / 2 );
attMTU_t g_attMtuClientServer;

// multi-role MTU size variables
uint16  gAttMtuSize[MAX_NUM_LL_CONN];

/*********************************************************************
    EXTERNAL VARIABLES
*/

/*********************************************************************
    EXTERNAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/
#if defined ( TESTMODES )
    static uint16 paramValue = 0;
#endif

/*********************************************************************
    LOCAL FUNCTIONS
*/

/*********************************************************************
    API FUNCTIONS
*/

/*********************************************************************
    @fn      ATT_ParsePacket

    @brief   Parse an attribute protocol message received over the ATT
            fixed channel.

    @param   pL2capMsg �C pointer to received L2CAP message
    @param   pPkt �C pointer to parsed packet

    @return  SUCCESS or FAILURE
*/
uint8 ATT_ParsePacket( l2capDataEvent_t* pL2capMsg, attPacket_t* pPkt )
{
    uint16 len = pL2capMsg->pkt.len;

    // PDU should contain at least the Opcode
    if ( len > 0 )
    {
        // Attribute Opcode
        uint8 opcode = pL2capMsg->pkt.pPayload[0];

        // First check for authenticated data
        if ( attAuthenSigFlag( opcode ) != 0 )
        {
            uint8 authenSig; // Whether or not to authenticate incoming signature

            // PDU should contain at least the Opcode and Authentication Signature
            if ( len <= AUTHEN_SIG_LEN )
            {
                return ( FAILURE );
            }

            #if defined ( TESTMODES )

            if ( paramValue == ATT_TESTMODE_UNAUTHEN_SIG )
            {
                authenSig = FALSE; // Don't authenticate incoming signature
            }
            else
            #endif
                authenSig = TRUE; // Authenticate incoming signature

            len -= AUTHEN_SIG_LEN; // Subtract length of Authentication Signature

            // Verify the Athentication Signature
            if ( SM_VerifyAuthenSig( pL2capMsg->connHandle, authenSig, pL2capMsg->pkt.pPayload,
                                     len, &(pL2capMsg->pkt.pPayload[len]) ) == SUCCESS )
            {
                pPkt->sig = ATT_SIG_VALID;
            }
            else
            {
                pPkt->sig = ATT_SIG_INVALID;
            }
        }
        else
        {
            // No Athentication Signature is included with this PDU
            pPkt->sig = ATT_SIG_NOT_INCLUDED;
        }

        // Command Flag
        pPkt->cmd = attCmdFlag( opcode ) == 0 ? FALSE : TRUE;
        // Method
        pPkt->method = attMethod( opcode );
        // Length of the attribute PDU paramters
        pPkt->len = len - 1;

        if ( pPkt->len > 0 )
        {
            // Attribute PDU parameters
            pPkt->pParams = &(pL2capMsg->pkt.pPayload[1]);
        }
        else
        {
            // No attribute PDU parameter
            pPkt->pParams = NULL;
        }

        return ( SUCCESS );
    }

    return ( FAILURE );
}

/*********************************************************************
    @fn      ATT_BuildErrorRsp

    @brief   Build Error Response.

    @param   pBuf - pointer to buffer to be build
    @param   pMsg - pointer to message structure

    @return  length of the command data
*/
uint16 ATT_BuildErrorRsp( uint8* pBuf, uint8* pMsg )
{
    attErrorRsp_t* pRsp = (attErrorRsp_t*)pMsg;
    // Command opcode in error
    *pBuf++ = pRsp->reqOpcode;
    // Attribute handle in error
    *pBuf++ = LO_UINT16( pRsp->handle );
    *pBuf++ = HI_UINT16( pRsp->handle );
    // Error code
    *pBuf = pRsp->errCode;
    return ( ERROR_RSP_SIZE );
}

/*********************************************************************
    @fn      ATT_ParseErrorRsp

    @brief   Parse Error Response message.

    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseErrorRsp( uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    if ( len == ERROR_RSP_SIZE )
    {
        attErrorRsp_t* pRsp = &pMsg->errorRsp;
        // Command opcode in error
        pRsp->reqOpcode = pParams[0];
        // Attribute handle in error
        pRsp->handle = BUILD_UINT16( pParams[1], pParams[2] );
        // Error code
        pRsp->errCode = pParams[3];
        return ( SUCCESS );
    }

    return ( ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_BuildExchangeMTUReq

    @brief   Build Exchange MTU Request.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the request data
*/
uint16 ATT_BuildExchangeMTUReq( uint8* pBuf, uint8* pMsg )
{
    attExchangeMTUReq_t* pReq = (attExchangeMTUReq_t*)pMsg;
    // Client receive MTU size
    *pBuf++ = LO_UINT16( pReq->clientRxMTU );
    *pBuf   = HI_UINT16( pReq->clientRxMTU );
    return ( READ_REQ_SIZE );
}

/*********************************************************************
    @fn      ATT_ParseExchangeMTUReq

    @brief   Parse Exchange MTU message.

    @param   sig - authentication signature status
    @param   cmd - command flag
    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseExchangeMTUReq( uint8 sig, uint8 cmd, uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    VOID sig; // Not applicable to this message
    VOID cmd; // Not applicable to this message

    if ( len == EXCHANGE_MTU_REQ_SIZE )
    {
        attExchangeMTUReq_t* pReq = &pMsg->exchangeMTUReq;
        // Client receive MTU size
        pReq->clientRxMTU = BUILD_UINT16( pParams[0], pParams[1] );
        return ( SUCCESS);
    }

    return ( ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_BuildExchangeMTURsp

    @brief   Build Exchange MTU Response.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the command data
*/
uint16 ATT_BuildExchangeMTURsp( uint8* pBuf, uint8* pMsg )
{
    attExchangeMTURsp_t* pRsp = (attExchangeMTURsp_t*)pMsg;
    // Server receive MTU size
    *pBuf++ = LO_UINT16( pRsp->serverRxMTU );
    *pBuf   = HI_UINT16( pRsp->serverRxMTU );
    return ( EXCHANGE_MTU_RSP_SIZE );
}

/*********************************************************************
    @fn      ATT_ParseExchangeMTURsp

    @brief   Parse Exchange MTU Response message.

    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseExchangeMTURsp( uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    if ( len == EXCHANGE_MTU_RSP_SIZE )
    {
        attExchangeMTURsp_t* pRsp = &pMsg->exchangeMTURsp;
        // Server receive MTU size
        pRsp->serverRxMTU = BUILD_UINT16( pParams[0], pParams[1] );
        return ( SUCCESS);
    }

    return ( ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_BuildFindInfoReq

    @brief   Build Find Information Request.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the command data
*/
uint16 ATT_BuildFindInfoReq( uint8* pBuf, uint8* pMsg )
{
    attFindInfoReq_t* pReq = (attFindInfoReq_t*)pMsg;
    // First requested handle number
    *pBuf++ = LO_UINT16( pReq->startHandle );
    *pBuf++ = HI_UINT16( pReq->startHandle );
    // Last requested handle number
    *pBuf++ = LO_UINT16( pReq->endHandle );
    *pBuf   = HI_UINT16( pReq->endHandle );
    return ( FIND_INFO_REQ_FIXED_SIZE );
}

/*********************************************************************
    @fn      ATT_ParseFindInfoReq

    @brief   Parse FInd Information Request message.

    @param   sig - authentication signature status
    @param   cmd - command flag
    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseFindInfoReq( uint8 sig, uint8 cmd, uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    VOID sig; // Not applicable to this message
    VOID cmd; // Not applicable to this message

    // Requested UUID
    if ( len == FIND_INFO_REQ_FIXED_SIZE )
    {
        attFindInfoReq_t* pReq = &pMsg->findInfoReq;
        // First requested handle number
        pReq->startHandle = BUILD_UINT16( pParams[0], pParams[1] );
        // Last requested handle number
        pReq->endHandle = BUILD_UINT16( pParams[2], pParams[3] );
        return ( SUCCESS );
    }

    return ( ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_BuildFindInfoRsp

    @brief   Build Find Information Response.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the command data
*/
uint16 ATT_BuildFindInfoRsp( uint8* pBuf, uint8* pMsg )
{
    uint16 len = FIND_INFO_RSP_FIXED_SIZE;
    attFindInfoRsp_t* pRsp = (attFindInfoRsp_t*)pMsg;
    // Format
    *pBuf++ = pRsp->format;

    // Information data (handle-UUID pairs)
    for ( uint8 i = 0; i < pRsp->numInfo; i++ )
    {
        if ( pRsp->format == ATT_HANDLE_BT_UUID_TYPE )
        {
            // Handle
            *pBuf++ = LO_UINT16( pRsp->info.btPair[i].handle );
            *pBuf++ = HI_UINT16( pRsp->info.btPair[i].handle );
            len += 2;
            // 2-octet Bluetooth UUID
            VOID osal_memcpy( pBuf, pRsp->info.btPair[i].uuid, ATT_BT_UUID_SIZE );
            pBuf += ATT_BT_UUID_SIZE;
            len  += ATT_BT_UUID_SIZE;
        }
        else // ATT_HANDLE_UUID_TYPE
        {
            // Handle
            *pBuf++ = LO_UINT16( pRsp->info.pair[i].handle );
            *pBuf++ = HI_UINT16( pRsp->info.pair[i].handle );
            len += 2;
            // 16-octet UUID
            VOID osal_memcpy( pBuf, pRsp->info.pair[i].uuid, ATT_UUID_SIZE );
            pBuf += ATT_UUID_SIZE;
            len  += ATT_UUID_SIZE;
        }
    } // for

    return ( len );
}

/*********************************************************************
    @fn      ATT_ParseFindInfoRsp

    @brief   Parse Find Information Response message.

    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseFindInfoRsp( uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    uint8 stat = ATT_ERR_INVALID_PDU;

    if ( len >= FIND_INFO_RSP_FIXED_SIZE )
    {
        uint8 numInfo = 0;
        uint8 dataLen = len - FIND_INFO_RSP_FIXED_SIZE; // Length of information data field

        // Find out the number of handle-UUID pairs
        if ( ( pParams[0] == ATT_HANDLE_BT_UUID_TYPE ) && ( dataLen % ( 2 + ATT_BT_UUID_SIZE ) == 0 ) )
        {
            numInfo = dataLen / ( 2 + ATT_BT_UUID_SIZE );

            // Validate the number of Handle and 2-octet UUID pairs
            if ( ( numInfo > 0 ) && ( numInfo <= ATT_MAX_NUM_HANDLE_BT_UUID ) )
            {
                stat = SUCCESS;
            }
        }
        else if ( ( pParams[0] == ATT_HANDLE_UUID_TYPE ) && ( dataLen % ( 2 + ATT_UUID_SIZE ) == 0 ) )
        {
            numInfo = dataLen / ( 2 + ATT_UUID_SIZE );

            // Validate the number of handle and 16-octet UUID pairs
            if ( ( numInfo > 0 ) && ( numInfo <= ATT_MAX_NUM_HANDLE_UUID ) )
            {
                stat = SUCCESS;
            }
        }

        // See if we need to parse the rest of the PDU
        if ( stat == SUCCESS )
        {
            attFindInfoRsp_t* pRsp = &pMsg->findInfoRsp;
            // Format
            pRsp->format = *pParams++;
            // Number of handle-UUID pairs
            pRsp->numInfo = numInfo;

            // Parse information data (handle-UUID pairs)
            for ( uint8 i = 0; i < numInfo; i++ )
            {
                if ( pRsp->format == ATT_HANDLE_BT_UUID_TYPE )
                {
                    // Handle
                    pRsp->info.btPair[i].handle = BUILD_UINT16( pParams[0], pParams[1] );
                    pParams += 2;
                    // 2-octet Bluetooth UUID
                    VOID osal_memcpy( pRsp->info.btPair[i].uuid, pParams, ATT_BT_UUID_SIZE );
                    pParams += ATT_BT_UUID_SIZE;
                }
                else // ATT_HANDLE_UUID_TYPE
                {
                    // Handle
                    pRsp->info.pair[i].handle = BUILD_UINT16( pParams[0], pParams[1] );
                    pParams += 2;
                    // 16-octet Bluetooth UUID
                    VOID osal_memcpy( pRsp->info.pair[i].uuid, pParams, ATT_UUID_SIZE );
                    pParams += ATT_UUID_SIZE;
                }
            } // for
        }
    }

    return ( stat );
}

/*********************************************************************
    @fn      ATT_BuildFindByTypeValueReq

    @brief   Build Find By Type Value Request.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the command data
*/
uint16 ATT_BuildFindByTypeValueReq( uint8* pBuf, uint8* pMsg )
{
    attFindByTypeValueReq_t* pReq = (attFindByTypeValueReq_t*)pMsg;
    // First requested handle number
    *pBuf++ = LO_UINT16( pReq->startHandle );
    *pBuf++ = HI_UINT16( pReq->startHandle );
    // Last requested handle number
    *pBuf++ = LO_UINT16( pReq->endHandle );
    *pBuf++ = HI_UINT16( pReq->endHandle );
    // Requested 2 octet UUID
    VOID osal_memcpy( pBuf, pReq->type.uuid, pReq->type.len );
    pBuf += pReq->type.len;
    // Attribute value
    VOID osal_memcpy( pBuf, pReq->value, pReq->len );
    return ( FIND_BY_TYPE_VALUE_REQ_FIXED_SIZE + pReq->type.len + pReq->len );
}

/*********************************************************************
    @fn      ATT_ParseFindByTypeValueReq

    @brief   Parse Find By Type Value Request message.

    @param   sig - authentication signature status
    @param   cmd - command flag
    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseFindByTypeValueReq( uint8 sig, uint8 cmd, uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    VOID sig; // Not applicable to this message
    VOID cmd; // Not applicable to this message

    if ( len >= FIND_BY_TYPE_VALUE_REQ_FIXED_SIZE + ATT_BT_UUID_SIZE )
    {
        attFindByTypeValueReq_t* pReq = &pMsg->findByTypeValueReq;
        // First requested handle number
        pReq->startHandle = BUILD_UINT16( pParams[0], pParams[1] );
        // Last requested handle number
        pReq->endHandle = BUILD_UINT16( pParams[2], pParams[3] );
        // 2 octet UUID
        pReq->type.len = ATT_BT_UUID_SIZE;
        // Requested 2 octet UUID
        VOID osal_memcpy( pReq->type.uuid, &pParams[4], pReq->type.len );
        // length of attribute value
        pReq->len = len - ( FIND_BY_TYPE_VALUE_REQ_FIXED_SIZE + ATT_BT_UUID_SIZE );

        // Requested attribute value
        if ( pReq->len <= ATT_MTU_SIZE - 7 )
        {
            VOID osal_memcpy( pReq->value, &pParams[6], pReq->len );
            return ( SUCCESS);
        }
    }

    return ( ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_BuildFindByTypeValueRsp

    @brief   Build Find By Type Value Response.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the command data
*/
uint16 ATT_BuildFindByTypeValueRsp( uint8* pBuf, uint8* pMsg )
{
    attFindByTypeValueRsp_t* pRsp = (attFindByTypeValueRsp_t*)pMsg;

    for ( uint8 i = 0; i < pRsp->numInfo; i++ )
    {
        // Attribute handle
        *pBuf++ = LO_UINT16( pRsp->handlesInfo[i].handle );
        *pBuf++ = HI_UINT16( pRsp->handlesInfo[i].handle );
        // End handle
        *pBuf++ = LO_UINT16( pRsp->handlesInfo[i].grpEndHandle );
        *pBuf++ = HI_UINT16( pRsp->handlesInfo[i].grpEndHandle );
    } // for

    return ( pRsp->numInfo * 4 );
}

/*********************************************************************
    @fn      ATT_ParseFindByTypeValueRsp

    @brief   Parse Find By Type Value Response message.

    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseFindByTypeValueRsp( uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    // Make sure there's at least one handle range in the response
    if ( len % 4 == 0 )
    {
        uint8 numInfo = len / 4; // Number of handle ranges

        if ( ( numInfo > 0 ) && ( numInfo <= g_ATT_MAX_NUM_HANDLES_INFO ) )
        {
            attFindByTypeValueRsp_t* pRsp = &pMsg->findByTypeValueRsp;
            pRsp->numInfo = numInfo;

            for ( uint8 i = 0; i < numInfo; i++ )
            {
                // First requested handle number
                pRsp->handlesInfo[i].handle = BUILD_UINT16( pParams[0], pParams[1] );
                // Last requested handle number
                pRsp->handlesInfo[i].grpEndHandle = BUILD_UINT16( pParams[2], pParams[3] );
                // Next handle range
                pParams += 4;
            }

            return ( SUCCESS );
        }
    }

    return ( ATT_ERR_INVALID_PDU);
}

/*********************************************************************
    @fn      ATT_BuildReadByTypeReq

    @brief   Build Read By Type Request.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the command data
*/
uint16 ATT_BuildReadByTypeReq( uint8* pBuf, uint8* pMsg )
{
    uint16 len = READ_BY_TYPE_REQ_FIXED_SIZE;
    attReadByTypeReq_t* pReq = (attReadByTypeReq_t*)pMsg;
    // First requested handle number
    *pBuf++ = LO_UINT16( pReq->startHandle );
    *pBuf++ = HI_UINT16( pReq->startHandle );
    // Last requested handle number
    *pBuf++ = LO_UINT16( pReq->endHandle );
    *pBuf++ = HI_UINT16( pReq->endHandle );
    // Requested UUID (2 or 16 octect)
    VOID osal_memcpy( pBuf, pReq->type.uuid, pReq->type.len );
    len += pReq->type.len;
    return ( len );
}

/*********************************************************************
    @fn      ATT_ParseReadByTypeReq

    @brief   Parse Read By Type Request message.

    @param   sig - authentication signature status
    @param   cmd - command flag
    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseReadByTypeReq( uint8 sig, uint8 cmd, uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    attReadByTypeReq_t* pReq = &pMsg->readByTypeReq;
    VOID sig; // Not applicable to this message
    VOID cmd; // Not applicable to this message

    if ( len == READ_BY_TYPE_REQ_FIXED_SIZE + ATT_BT_UUID_SIZE )
    {
        // 2-octet UUID
        pReq->type.len = ATT_BT_UUID_SIZE;
    }
    else if ( len == READ_BY_TYPE_REQ_FIXED_SIZE + ATT_UUID_SIZE )
    {
        // 16-octet UUID
        pReq->type.len = ATT_UUID_SIZE;
    }
    else
    {
        return ( ATT_ERR_INVALID_PDU );
    }

    // First requested handle number
    pReq->startHandle = BUILD_UINT16( pParams[0], pParams[1] );
    // Last requested handle number
    pReq->endHandle = BUILD_UINT16( pParams[2], pParams[3] );
    // Requested UUID
    VOID osal_memcpy( pReq->type.uuid, &pParams[4], pReq->type.len );
    return ( SUCCESS);
}

/*********************************************************************
    @fn      ATT_BuildReadByTypeRsp

    @brief   Build Read By Type Response.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the command data
*/
uint16 ATT_BuildReadByTypeRsp( uint8* pBuf, uint8* pMsg )
{
    attReadByTypeRsp_t* pRsp = (attReadByTypeRsp_t*)pMsg;
    uint8 dataLen = pRsp->numPairs * pRsp->len;
    // Length of each attribute handle-value pair
    *pBuf++ = pRsp->len;
    // List of 1 or more attribute handle-value pairs
    VOID osal_memcpy( pBuf, pRsp->dataList, dataLen );
    return ( dataLen + 1 );
}

/*********************************************************************
    @fn      ATT_ParseReadByTypeRsp

    @brief   Parse Read By Type Response message.

    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseReadByTypeRsp( uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    attReadByTypeRsp_t* pRsp = &pMsg->readByTypeRsp;

    if ( len >= READ_BY_TYPE_RSP_FIXED_SIZE )
    {
        uint8 dataLen = len - 1;
        // Length of each attribute handle-value pair
        pRsp->len = pParams[0];

        if ( ( dataLen <= ATT_MTU_SIZE - 2 ) &&
                ( pRsp->len > 0 )             &&
                ( ( dataLen % pRsp->len ) == 0 ) )
        {
            // Total length of attribute handle-value pairs
            pRsp->numPairs = dataLen / pRsp->len;
            // List of 1 or more attribute handle-value pairs
            VOID osal_memcpy( pRsp->dataList, &pParams[1], dataLen );
            return ( SUCCESS);
        }
    }

    return ( ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_BuildReadReq

    @brief   Build Read Request.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the request data
*/
uint16 ATT_BuildReadReq( uint8* pBuf, uint8* pMsg )
{
    attReadReq_t* pReq = (attReadReq_t*)pMsg;
    // Attribute handle
    *pBuf++ = LO_UINT16( pReq->handle );
    *pBuf   = HI_UINT16( pReq->handle );
    return ( READ_REQ_SIZE );
}

/*********************************************************************
    @fn      ATT_ParseReadReq

    @brief   Parse Read Request message.

    @param   sig - authentication signature status
    @param   cmd - command flag
    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseReadReq( uint8 sig, uint8 cmd, uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    VOID sig; // Not applicable to this message
    VOID cmd; // Not applicable to this message

    if ( len == READ_REQ_SIZE )
    {
        attReadReq_t* pReq = &pMsg->readReq;
        // Attribute handle
        pReq->handle = BUILD_UINT16( pParams[0], pParams[1] );
        return ( SUCCESS);
    }

    return ( ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_BuildReadRsp

    @brief   Build Read Response.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the command data
*/
uint16 ATT_BuildReadRsp( uint8* pBuf, uint8* pMsg )
{
    attReadRsp_t* pRsp = (attReadRsp_t*)pMsg;
    // Attribute value
    VOID osal_memcpy( pBuf, pRsp->value, pRsp->len );
    return ( pRsp->len );
}

/*********************************************************************
    @fn      ATT_ParseReadRsp

    @brief   Parse Read Response message.

    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseReadRsp( uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    attReadRsp_t* pRsp = &pMsg->readRsp;

    // Attribute value
    if ( len <= ATT_MTU_SIZE - 1 )
    {
        pRsp->len = len;
        VOID osal_memcpy( pRsp->value, pParams, len );
        return ( SUCCESS);
    }

    return ( ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_ParseWriteReq

    @brief   Parse Write Command message.

    @param   sig - authentication signature status
    @param   cmd - command flag
    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseWriteReq( uint8 sig, uint8 cmd, uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    if ( len >= WRITE_REQ_FIXED_SIZE )
    {
        attWriteReq_t* pReq = &pMsg->writeReq;
        uint16 maxLen;
        // Attribute handle
        pReq->handle = BUILD_UINT16( pParams[0], pParams[1] );
        // Authentication signature
        pReq->sig = sig;
        // Command flag
        pReq->cmd = cmd;

        if ( sig == ATT_SIG_NOT_INCLUDED )
        {
            maxLen = ATT_MTU_SIZE - 3;
        }
        else
        {
            maxLen = ATT_MTU_SIZE - (AUTHEN_SIG_LEN + 3);
        }

        // Attribute value length
        pReq->len = len - WRITE_REQ_FIXED_SIZE;

        // Attribute value
        if ( pReq->len <= maxLen )
        {
            VOID osal_memcpy( pReq->value, &pParams[2], pReq->len );
            return ( SUCCESS);
        }
    }

    return ( ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_BuildWriteReq

    @brief   Build Write Request/Command.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the request data
*/
uint16 ATT_BuildWriteReq( uint8* pBuf, uint8* pMsg )
{
    attWriteReq_t* pReq = (attWriteReq_t*)pMsg;
    // Attribute handle
    *pBuf++ = LO_UINT16( pReq->handle );
    *pBuf++ = HI_UINT16( pReq->handle );
    // Attribute value
    VOID osal_memcpy( pBuf, pReq->value, pReq->len );
    return ( WRITE_REQ_FIXED_SIZE + pReq->len );
}

/*********************************************************************
    @fn      ATT_ParseWriteRsp

    @brief   Parse Write Response message.

    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseWriteRsp( uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    VOID pParams; // Not applicable to this message
    VOID pMsg; // Not applicable to this message
    return ( len == 0 ? SUCCESS : ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_BuildReadBlobReq

    @brief   Build Read Blob Request.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the request data
*/
uint16 ATT_BuildReadBlobReq( uint8* pBuf, uint8* pMsg )
{
    attReadBlobReq_t* pReq = (attReadBlobReq_t*)pMsg;
    // Attribute handle
    *pBuf++ = LO_UINT16( pReq->handle );
    *pBuf++ = HI_UINT16( pReq->handle );
    // Value Offset
    *pBuf++ = LO_UINT16( pReq->offset );
    *pBuf   = HI_UINT16( pReq->offset );
    return ( READ_BLOB_REQ_SIZE );
}

/*********************************************************************
    @fn      ATT_ParseReadBlobReq

    @brief   Parse Read Blob Request message.

    @param   sig - authentication signature status
    @param   cmd - command flag
    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseReadBlobReq( uint8 sig, uint8 cmd, uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    VOID sig; // Not applicable to this message
    VOID cmd; // Not applicable to this message

    if ( len == READ_BLOB_REQ_SIZE )
    {
        attReadBlobReq_t* pReq = &pMsg->readBlobReq;
        // Attribute handle
        pReq->handle = BUILD_UINT16( pParams[0], pParams[1] );
        // Value Offset
        pReq->offset = BUILD_UINT16( pParams[2], pParams[3] );
        return ( SUCCESS);
    }

    return ( ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_BuildReadBlobRsp

    @brief   Build Read Blob Response.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the command data
*/
uint16 ATT_BuildReadBlobRsp( uint8* pBuf, uint8* pMsg )
{
    attReadBlobRsp_t* pRsp = (attReadBlobRsp_t*)pMsg;
    // Part of attribute value
    VOID osal_memcpy( pBuf, pRsp->value, pRsp->len );
    return ( pRsp->len );
}

/*********************************************************************
    @fn      ATT_ParseReadBlobRsp

    @brief   Parse Read Blob Response message.

    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseReadBlobRsp( uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    attReadBlobRsp_t* pRsp = &pMsg->readBlobRsp;
    // Part Attribute value
    pRsp->len = len;

    if ( pRsp->len <= ATT_MTU_SIZE - 1 )
    {
        VOID osal_memcpy( pRsp->value, pParams, pRsp->len );
        return ( SUCCESS);
    }

    return ( ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_BuildReadMultiReq

    @brief   Build Read Multiple Request.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the request data
*/
uint16 ATT_BuildReadMultiReq( uint8* pBuf, uint8* pMsg )
{
    attReadMultiReq_t* pReq = (attReadMultiReq_t*)pMsg;

    for ( uint8 i = 0; i < pReq->numHandles; i++ )
    {
        // Attribute handle
        *pBuf++ = LO_UINT16( pReq->handle[i] );
        *pBuf++ = HI_UINT16( pReq->handle[i] );
    }

    return ( pReq->numHandles * 2 );
}

/*********************************************************************
    @fn      ATT_ParseReadMultiReq

    @brief   Parse Read Multiple Request message.

    @param   sig - authentication signature status
    @param   cmd - command flag
    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseReadMultiReq( uint8 sig, uint8 cmd, uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    VOID sig; // Not applicable to this message
    VOID cmd; // Not applicable to this message

    // Make sure the length of attribute handles is even
    if ( len % 2 == 0 )
    {
        uint8 numHandles = len / 2; // Number of attribute handles

        // Make sure there're at least two attribute handles in the request
        if ( ( numHandles >= ATT_MIN_NUM_HANDLES ) && ( numHandles <= g_ATT_MAX_NUM_HANDLES ) )
        {
            attReadMultiReq_t* pReq = &pMsg->readMultiReq;
            pReq->numHandles = numHandles;

            for ( uint8 i = 0; i < numHandles; i++ )
            {
                // Attribute handle
                pReq->handle[i] = BUILD_UINT16( pParams[0], pParams[1] );
                // Next handle
                pParams += 2;
            }

            return ( SUCCESS);
        }
    }

    return ( ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_BuildReadMultiRsp

    @brief   Build Read Multiple Response.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the command data
*/
uint16 ATT_BuildReadMultiRsp( uint8* pBuf, uint8* pMsg )
{
    attReadMultiRsp_t* pRsp = (attReadMultiRsp_t*)pMsg;
    // A set of two or more values
    VOID osal_memcpy( pBuf, pRsp->values, pRsp->len );
    return ( pRsp->len );
}

/*********************************************************************
    @fn      ATT_ParseReadMultiRsp

    @brief   Parse Read Multiple Response message.

    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseReadMultiRsp( uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    attReadMultiRsp_t* pRsp = &pMsg->readMultiRsp;
    pRsp->len = len;

    // A set of two or more values
    if ( pRsp->len <= ATT_MTU_SIZE - 1 )
    {
        VOID osal_memcpy( pRsp->values, &pParams[0], pRsp->len );
        return ( SUCCESS );
    }

    return ( ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_BuildReadByGrpTypeRsp

    @brief   Build Read By Group Type Response.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the command data
*/
uint16 ATT_BuildReadByGrpTypeRsp( uint8* pBuf, uint8* pMsg )
{
    attReadByGrpTypeRsp_t* pRsp = (attReadByGrpTypeRsp_t*)pMsg;
    uint8 dataLen = pRsp->numGrps * pRsp->len;
    // Length of each attribute handle, group end handle and value set
    *pBuf++ = pRsp->len;
    // List of 1 or more attribute handle, group end handle and value
    VOID osal_memcpy( pBuf, pRsp->dataList, dataLen );
    return ( dataLen + 1 );
}

/*********************************************************************
    @fn      ATT_ParseReadByGrpTypeRsp

    @brief   Parse Read By Group Type Response message.

    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseReadByGrpTypeRsp( uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    attReadByGrpTypeRsp_t* pRsp = &pMsg->readByGrpTypeRsp;

    if ( len >= READ_BY_TYPE_RSP_FIXED_SIZE )
    {
        uint8 dataLen = len - 1;
        // Length of each attribute handle, group end handle and value set
        pRsp->len = pParams[0];

        if ( ( dataLen <= ATT_MTU_SIZE - 2 ) &&
                ( pRsp->len > 0 )             &&
                ( ( dataLen % pRsp->len ) == 0 ) )
        {
            // Number of all attribute handle, group end handle and value sets found
            pRsp->numGrps = dataLen / pRsp->len;
            // List of 1 or more attribute handle, end group handle and value set
            VOID osal_memcpy( pRsp->dataList, &pParams[1], dataLen );
            return ( SUCCESS);
        }
    }

    return ( ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_BuildPrepareWriteReq

    @brief   Build Prepare Write Request.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the request data
*/
uint16 ATT_BuildPrepareWriteReq( uint8* pBuf, uint8* pMsg )
{
    attPrepareWriteReq_t* pReq = (attPrepareWriteReq_t*)pMsg;
    // Attribute handle
    *pBuf++ = LO_UINT16( pReq->handle );
    *pBuf++ = HI_UINT16( pReq->handle );
    // Value Offset
    *pBuf++ = LO_UINT16( pReq->offset );
    *pBuf++ = HI_UINT16( pReq->offset );
    // Part Attribute value
    VOID osal_memcpy( pBuf, pReq->value, pReq->len );
    return ( PREPARE_WRITE_REQ_FIXED_SIZE + pReq->len );
}

/*********************************************************************
    @fn      ATT_ParsePrepareWriteReq

    @brief   Parse Write Prepare Request message.

    @param   sig - authentication signature status
    @param   cmd - command flag
    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParsePrepareWriteReq( uint8 sig, uint8 cmd, uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    VOID sig; // Not applicable to this message
    VOID cmd; // Not applicable to this message

    if ( len >= PREPARE_WRITE_REQ_FIXED_SIZE )
    {
        attPrepareWriteReq_t* pReq = &pMsg->prepareWriteReq;
        // Attribute handle
        pReq->handle = BUILD_UINT16( pParams[0], pParams[1] );
        // Value Offset
        pReq->offset = BUILD_UINT16( pParams[2], pParams[3] );
        // Part Attribute value
        pReq->len = len - PREPARE_WRITE_REQ_FIXED_SIZE;

        if ( pReq->len <= ATT_MTU_SIZE - 5 )
        {
            VOID osal_memcpy( pReq->value, &pParams[4], pReq->len );
            return ( SUCCESS);
        }
    }

    return ( ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_BuildPrepareWriteRsp

    @brief   Build Prepare Write Response.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the command data
*/
uint16 ATT_BuildPrepareWriteRsp( uint8* pBuf, uint8* pMsg )
{
    attPrepareWriteRsp_t* pReq = (attPrepareWriteRsp_t*)pMsg;
    // Attribute handle
    *pBuf++ = LO_UINT16( pReq->handle );
    *pBuf++ = HI_UINT16( pReq->handle );
    // Value Offset
    *pBuf++ = LO_UINT16( pReq->offset );
    *pBuf++ = HI_UINT16( pReq->offset );
    // Part Attribute value
    VOID osal_memcpy( pBuf, pReq->value, pReq->len );
    return ( PREPARE_WRITE_RSP_FIXED_SIZE + pReq->len );
}

/*********************************************************************
    @fn      ATT_ParsePrepareWriteRsp

    @brief   Parse Prepare Write Response message.

    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParsePrepareWriteRsp( uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    if ( len >= PREPARE_WRITE_RSP_FIXED_SIZE )
    {
        attPrepareWriteRsp_t* pRsp = &pMsg->prepareWriteRsp;
        // Attribute handle
        pRsp->handle = BUILD_UINT16( pParams[0], pParams[1] );
        // Value Offset
        pRsp->offset = BUILD_UINT16( pParams[2], pParams[3] );
        // Part Attribute value
        pRsp->len = len - PREPARE_WRITE_RSP_FIXED_SIZE;

        if ( pRsp->len <= ATT_MTU_SIZE - 5 )
        {
            VOID osal_memcpy( pRsp->value, &pParams[4], pRsp->len );
            return ( SUCCESS);
        }
    }

    return ( ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_BuildExecuteWriteReq

    @brief   Build Execute Write Request.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the request data
*/
uint16 ATT_BuildExecuteWriteReq( uint8* pBuf, uint8* pMsg )
{
    attExecuteWriteReq_t* pReq = (attExecuteWriteReq_t*)pMsg;
    // Flags
    *pBuf = pReq->flags;
    return ( EXECUTE_WRITE_REQ_SIZE );
}

/*********************************************************************
    @fn      ATT_ParseExecuteWriteReq

    @brief   Parse Execute Write Request message.

    @param   sig - authentication signature status
    @param   cmd - command flag
    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseExecuteWriteReq( uint8 sig, uint8 cmd, uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    VOID sig; // Not applicable to this message
    VOID cmd; // Not applicable to this message

    if ( len == EXECUTE_WRITE_REQ_SIZE )
    {
        attExecuteWriteReq_t* pReq = &pMsg->executeWriteReq;
        // Attribute handle
        pReq->flags = pParams[0];
        return ( SUCCESS);
    }

    return ( ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_ParseExecuteWriteRsp

    @brief   Parse Execute Write Response message.

    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseExecuteWriteRsp( uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    VOID pParams; // Not applicable to this message
    VOID pMsg; // Not applicable to this message
    return ( len == 0 ? SUCCESS : ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_BuildHandleValueInd

    @brief   Build Handle Value Indication.

    @param   pBuf - pointer to buffer to be built
    @param   pMsg - pointer to message structure

    @return  length of the command data
*/
uint16 ATT_BuildHandleValueInd( uint8* pBuf, uint8* pMsg )
{
    attHandleValueInd_t* pReq = (attHandleValueInd_t*)pMsg;
    // Attribute handle
    *pBuf++ = LO_UINT16( pReq->handle );
    *pBuf++ = HI_UINT16( pReq->handle );
    // Attribute value
    VOID osal_memcpy( pBuf, pReq->value, pReq->len );
    return ( HANDLE_VALUE_IND_FIXED_SIZE + pReq->len );
}

/*********************************************************************
    @fn      ATT_ParseHandleValueInd

    @brief   Parse Handle Value Indication message.

    @param   sig - authentication signature status
    @param   cmd - command flag
    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseHandleValueInd( uint8 sig, uint8 cmd, uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    VOID sig; // Not applicable to this message
    VOID cmd; // Not applicable to this message

    if ( len >= HANDLE_VALUE_IND_FIXED_SIZE )
    {
        attHandleValueInd_t* pInd = &pMsg->handleValueInd;
        // Attribute handle
        pInd->handle = BUILD_UINT16( pParams[0], pParams[1] );
        // Attribute value length
        pInd->len = len - HANDLE_VALUE_IND_FIXED_SIZE;

        if ( pInd->len <= ATT_MTU_SIZE - 3 )
        {
            VOID osal_memcpy( pInd->value, &pParams[2], pInd->len );
            return ( SUCCESS);
        }
    }

    return ( ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      ATT_ParseHandleValueCfm

    @brief   Parse  Handle Value Confirmation message.

    @param   pParams - pointer to received parameters
    @param   len - length of parameters
    @param   pMsg - pointer to message structure to be built

    @return  SUCCESS or ATT_ERR_INVALID_PDU
*/
bStatus_t ATT_ParseHandleValueCfm( uint8* pParams, uint16 len, attMsg_t* pMsg )
{
    VOID pParams; // Not applicable to this message
    VOID pMsg; // Not applicable to this message
    return ( len == 0 ? SUCCESS : ATT_ERR_INVALID_PDU );
}

/*********************************************************************
    @fn      attSendMsg

    @brief   Send an attribute protocol message over the ATT fixed channel.

    @param   connHandle �C connection to use
    @param   pfnBuildMsg �C build function for message
    @param   opcode �C operation code
    @param   pMsg �C pointer to message to be sent

    @return  SUCCESS: Request was sent successfully.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
            bleLinkEncrypted: Connection is already encrypted.
*/
bStatus_t attSendMsg( uint16 connHandle, attBuildMsg_t pfnBuildMsg, uint8 opcode, uint8* pMsg )
{
    uint8* buf;
    uint8 status;
    // Allocate space for the message
    buf = (uint8*)L2CAP_bm_alloc( gAttMtuSize[connHandle] );

    if ( buf != NULL )
    {
        uint8* pBuf = buf;
        uint16 len = 1; // opcode
        // operation code
        *pBuf++ = opcode;

        // Build the command specific data
        if ( pfnBuildMsg != NULL )
        {
            len += (*pfnBuildMsg)( pBuf, pMsg );
        }

        // Do we need to include an Authentication Signature?
        if ( attAuthenSigFlag( opcode ) != 0 )
        {
            // Make sure the link is not encrypted
            if ( !linkDB_Encrypted( connHandle ) )
            {
                // Calcuate the Authentication Signature field, which will provide
                // an Authentication Signature for the following values in this
                // order: Opcode, Attribute Handle, and Attribute Value.
                // Append the Authentication Signature after the Attribute Value
                status = SM_GenerateAuthenSig( buf, len, &(buf[len]) );

                if ( status == SUCCESS )
                {
                    len += AUTHEN_SIG_LEN; // Add length of Authentication Signature
                }
            }
            else
            {
                // An Attribute PDU that includes an Authentication Signature should
                // not be sent on an encrypted link. Note: an encrypted link already
                // includes authentication data on every packet and therefore adding
                // more authentication data is not required.
                status = bleLinkEncrypted;
            }
        }
        else
        {
            status = SUCCESS;
        }

        // Send the message
        if ( status == SUCCESS )
        {
            l2capPacket_t pkt;
            // Create an L2CAP packet
            pkt.CID = L2CAP_CID_ATT;
            pkt.pPayload = buf;
            pkt.len = len;
            // Send the packet over the ATT fixed channel
            status = L2CAP_SendData( connHandle, &pkt );
        }

        if ( status != SUCCESS )
        {
            // free the buffer
            osal_bm_free( buf );
        }
    }
    else
    {
        status = bleMemAllocError;
    }

    return ( status );
}

/*********************************************************************
    @fn      ATT_CompareUUID

    @brief   Compare two UUIDs. The UUIDs are converted if necessary.
            Valid lengths are 2 or 16 octets.

    @param   pUUID1 - pointer to first UUID
    @param   len1 - length of first UUID
    @param   pUUID2 - pointer to second UUID
    @param   len2 - length of second UUID

    @return  TRUE if equal. FALSE, otherwise.
*/
uint8 ATT_CompareUUID( const uint8* pUUID1, uint16 len1,
                       const uint8* pUUID2, uint16 len2 )
{
    uint8 longUUID[ATT_UUID_SIZE];

    // Make sure both of them have the right length
    if ( ( len1 != ATT_BT_UUID_SIZE ) && ( len1 != ATT_UUID_SIZE ) )
    {
        return ( FALSE );
    }

    if ( ( len2 != ATT_BT_UUID_SIZE ) && ( len2 != ATT_UUID_SIZE ) )
    {
        return ( FALSE );
    }

    // See if both are the same length
    if ( len1 == len2 )
    {
        // Both are the same length; just compare them
        return ( osal_memcmp( pUUID1, pUUID2, len1 ) );
    }

    // One of them is 16 octets and other is not
    if ( len1 == ATT_UUID_SIZE )
    {
        // The first UUID is 16 octets; convert the second one to 16 octets
        VOID ATT_ConvertUUIDto128( pUUID2, longUUID );
        // Compare them now
        return ( osal_memcmp( pUUID1, longUUID, ATT_UUID_SIZE ) );
    }

    // The second UUID is 16 octets; convert the first one to 16 octets
    VOID ATT_ConvertUUIDto128( pUUID1, longUUID );
    // Compare them now
    return ( osal_memcmp( pUUID2, longUUID, ATT_UUID_SIZE ) );
}

/*********************************************************************
    @fn      ATT_ConvertUUIDto128

    @brief   Convert a 16-bit UUID to 128-bit UUID. Simply, the 2 byte
            Attribute UUID replaces the x's in the following:
            0000xxxx-0000-1000-8000-00805F9B34FB

    @param   pUUID16 - pointer to 16-bit UUID
    @param   pUUID128 - pointer to 128-bit UUID to be built

    @return  TRUE if converted. FALSE, otherwise.
*/
uint8 ATT_ConvertUUIDto128( const uint8* pUUID16, uint8* pUUID128 )
{
    if ( ( pUUID16 != NULL ) && ( pUUID128 != NULL ) )
    {
        // Start off with the Bluetooth base UUID
        VOID osal_memcpy( pUUID128, btBaseUUID, ATT_UUID_SIZE );
        // Copy the 2 byte UUID over
        pUUID128[12] = pUUID16[0];
        pUUID128[13] = pUUID16[1];
        return ( TRUE );
    }

    return ( FALSE );
}

/*********************************************************************
    @fn      ATT_ConvertUUIDto16

    @brief   Convert a 128-bit UUID to 16-bit UUID. Simply, the 2 byte
            Attribute UUID represents the x's in the following:
            0000xxxx-0000-1000-8000-00805F9B34FB

    @param   pUUID128 - pointer to 128-bit UUID
    @param   pUUID16 - pointer to 16-bit UUID to be built

    @return  TRUE if converted. FALSE, otherwise.
*/
uint8 ATT_ConvertUUIDto16( const uint8* pUUID128, uint8* pUUID16 )
{
    if ( ( pUUID16 != NULL ) && ( pUUID128 != NULL ) )
    {
        uint8 uuid[ATT_UUID_SIZE];
        // First make sure the 128-bit UUID is a valid Bluetooth UUID
        VOID osal_memcpy( uuid, pUUID128, ATT_UUID_SIZE );
        uuid[12] = 0x00;
        uuid[13] = 0x00;

        // Compare it to the Bluetooth base UUID
        if ( osal_memcmp( uuid, btBaseUUID, ATT_UUID_SIZE ) == TRUE )
        {
            // Extract the 2 byte UUID
            pUUID16[0] = pUUID128[12];
            pUUID16[1] = pUUID128[13];
            return ( TRUE );
        }
    }

    return ( FALSE );
}

/*********************************************************************
    @fn      ATT_SetParamValue

    @brief   Set a ATT Parameter value.  Use this function to change
            the default ATT parameter values.

    @param   value - new param value

    @return  void
*/
void ATT_SetParamValue( uint16 value )
{
    #if defined ( TESTMODES )
    paramValue = value;
    #else
    VOID value;
    #endif
}

/*********************************************************************
    @fn      ATT_GetParamValue

    @brief   Get a ATT Parameter value.

    @param   none

    @return  ATT Parameter value
*/
uint16 ATT_GetParamValue( void )
{
    #if defined ( TESTMODES )
    return ( paramValue );
    #else
    return ( 0 );
    #endif
}

void ATT_SetMTUSizeMax(uint16 mtuSize)
{
    g_ATT_MTU_SIZE_MAX           = mtuSize > ATT_MTU_SIZE ? ATT_MTU_SIZE : mtuSize;
    g_ATT_MAX_NUM_HANDLES_INFO   =   ( ( g_ATT_MTU_SIZE_MAX - 1 ) / 4 );
    g_ATT_MAX_NUM_HANDLES        =   ( ( g_ATT_MTU_SIZE_MAX - 1 ) / 2 );
    return ;
}

void ATT_UpdateMtuSize(uint16 connHandle, uint16 mtuSize)
{
    if (mtuSize > g_ATT_MTU_SIZE_MAX)
        return;

    gAttMtuSize[connHandle] = mtuSize;
    LOG("[ATT_MTU] %d \n", gAttMtuSize[connHandle]);
}

uint16 ATT_GetCurrentMTUSize(uint16 connHandle)
{
    return gAttMtuSize[connHandle];
}

void ATT_InitMtuSize(void)
{
    for (int i = 0; i < MAX_NUM_LL_CONN; i ++)
    {
        gAttMtuSize[i] = ATT_MTU_SIZE_MIN;
    }
}

#if 0
uint16 ATT_GetCurrentMTUSize(void)
{
    return g_ATT_MTU_SIZE;
}

void ATT_MTU_SIZE_UPDATE(uint8 mtuSize)
{
    g_ATT_MTU_SIZE                  =   mtuSize;
    LOG("[ATT_MTU] %d \n",g_ATT_MTU_SIZE);
//    g_ATT_MAX_NUM_HANDLE_BT_UUID    =   ( ( g_ATT_MTU_SIZE - 2 ) / ( 2 + ATT_BT_UUID_SIZE ) );
//    g_ATT_MAX_NUM_HANDLE_UUID       =   ( ( g_ATT_MTU_SIZE - 2 ) / ( 2 + ATT_UUID_SIZE ) );
}
#endif
/****************************************************************************
****************************************************************************/
