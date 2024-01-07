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
    Filename:       att_client.c
    Revised:
    Revision:

    Description:    This file contains the Attribute Protocol Client.


**************************************************************************************************/


/*********************************************************************
    INCLUDES
*/
#include "bcomdef.h"

#include "att_internal.h"

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

/*********************************************************************
    EXTERNAL VARIABLES
*/

/*********************************************************************
    EXTERNAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/

/*********************************************************************
    LOCAL FUNCTIONS
*/

/*********************************************************************
    API FUNCTIONS
*/

/*********************************************************************
    @fn      ATT_ExchangeMTUReq

    @brief   Send Exchange MTU Request.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_ExchangeMTUReq( uint16 connHandle, attExchangeMTUReq_t* pReq )
{
    // Validate Type field
    if ( pReq->clientRxMTU >= ATT_MTU_SIZE_MIN &&
            pReq->clientRxMTU <= ATT_MTU_SIZE)                // add 2020-0319
    {
        return ( attSendMsg( connHandle, ATT_BuildExchangeMTUReq, ATT_EXCHANGE_MTU_REQ, (uint8*)pReq ) );
    }

    return ( INVALIDPARAMETER );
}

/*********************************************************************
    @fn      ATT_FindInfoReq

    @brief   Send Find Information Request.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_FindInfoReq( uint16 connHandle, attFindInfoReq_t* pReq )
{
    return ( attSendMsg( connHandle, ATT_BuildFindInfoReq, ATT_FIND_INFO_REQ, (uint8*)pReq ) );
}

/*********************************************************************
    @fn      ATT_FindByTypeValueReq

    @brief   Send Find By Type Value Request.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_FindByTypeValueReq( uint16 connHandle, attFindByTypeValueReq_t* pReq )
{
    // Validate the UUID field
    if ( pReq->type.len == ATT_BT_UUID_SIZE )
    {
        return ( attSendMsg( connHandle, ATT_BuildFindByTypeValueReq,
                             ATT_FIND_BY_TYPE_VALUE_REQ, (uint8*)pReq ) );
    }

    return ( INVALIDPARAMETER );
}

/*********************************************************************
    @fn      ATT_ReadByTypeReq

    @brief   Send Read By Type Request.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_ReadByTypeReq( uint16 connHandle, attReadByTypeReq_t* pReq )
{
    // Validate Type field
    if ( ( pReq->type.len == ATT_UUID_SIZE ) || ( pReq->type.len == ATT_BT_UUID_SIZE ) )
    {
        return ( attSendMsg( connHandle, ATT_BuildReadByTypeReq, ATT_READ_BY_TYPE_REQ, (uint8*)pReq ) );
    }

    return ( INVALIDPARAMETER );
}

/*********************************************************************
    @fn      ATT_ReadReq

    @brief   Send Read Request.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_ReadReq( uint16 connHandle, attReadReq_t* pReq )
{
    return ( attSendMsg( connHandle, ATT_BuildReadReq, ATT_READ_REQ, (uint8*)pReq ) );
}

/*********************************************************************
    @fn      ATT_ReadBlobReq

    @brief   Send Read Blob Request.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_ReadBlobReq( uint16 connHandle, attReadBlobReq_t* pReq )
{
    return ( attSendMsg( connHandle, ATT_BuildReadBlobReq, ATT_READ_BLOB_REQ, (uint8*)pReq ) );
}

/*********************************************************************
    @fn      ATT_ReadMultiReq

    @brief   Send Read Multiple Request.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_ReadMultiReq( uint16 connHandle, attReadMultiReq_t* pReq )
{
    // Validate number of attributes
    if ( pReq->numHandles >= ATT_MIN_NUM_HANDLES )
    {
        return ( attSendMsg( connHandle, ATT_BuildReadMultiReq, ATT_READ_MULTI_REQ, (uint8*)pReq ) );
    }

    return ( INVALIDPARAMETER );
}

/*********************************************************************
    @fn      ATT_ReadByGrpTypeReq

    @brief   Send Read By Group Type Request.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_ReadByGrpTypeReq( uint16 connHandle, attReadByGrpTypeReq_t* pReq )
{
    // Validate Type field
    if ( ( pReq->type.len == ATT_UUID_SIZE ) || ( pReq->type.len == ATT_BT_UUID_SIZE ) )
    {
        return ( attSendMsg( connHandle, ATT_BuildReadByTypeReq, ATT_READ_BY_GRP_TYPE_REQ, (uint8*)pReq ) );
    }

    return ( INVALIDPARAMETER );
}

/*********************************************************************
    @fn      ATT_WriteReq

    @brief   Send Write Request.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
            bleLinkEncrypted: Connection is already encrypted.
*/
bStatus_t ATT_WriteReq( uint16 connHandle, attWriteReq_t* pReq )
{
    uint8 opcode = ATT_WRITE_REQ;

    if ( pReq->sig == TRUE )
    {
        // Authentication Signature requested to be included
        if ( pReq->len > ( gAttMtuSize[connHandle] - 15 ) )
        {
            return ( INVALIDPARAMETER );
        }

        // Set the Authentication Signature Flag
        opcode |= ATT_AUTHEN_SIG_FLAG_BIT;
    }

    if ( pReq->cmd == TRUE )
    {
        // Set the Command Flag
        opcode |= ATT_CMD_FLAG_BIT;
    }

    return ( attSendMsg( connHandle, ATT_BuildWriteReq, opcode, (uint8*)pReq ) );
}

/*********************************************************************
    @fn      ATT_PrepareWriteReq

    @brief   Send Prepare Write Request.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_PrepareWriteReq( uint16 connHandle, attPrepareWriteReq_t* pReq )
{
    return ( attSendMsg( connHandle, ATT_BuildPrepareWriteReq, ATT_PREPARE_WRITE_REQ, (uint8*)pReq ) );
}

/*********************************************************************
    @fn      ATT_ExecuteWriteReq

    @brief   Send Execute Write Request.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_ExecuteWriteReq( uint16 connHandle, attExecuteWriteReq_t* pReq )
{
    return ( attSendMsg( connHandle, ATT_BuildExecuteWriteReq, ATT_EXECUTE_WRITE_REQ, (uint8*)pReq ) );
}

/*********************************************************************
    @fn      ATT_HandleValueCfm

    @brief   Send Handle Value Confirmation.

    @param   connHandle - connection to use

    @return  SUCCESS: Confirmation was sent successfully.
            INVALIDPARAMETER: Invalid confirmation field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_HandleValueCfm( uint16 connHandle )
{
    return ( attSendMsg( connHandle, NULL, ATT_HANDLE_VALUE_CFM, NULL ) );
}


/****************************************************************************
****************************************************************************/
