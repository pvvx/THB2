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
    Filename:       att_server.c
    Revised:
    Revision:

    Description:    This file contains the Attribute Protocol Server.


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
    @fn      ATT_ErrorRsp

    @brief   Send Error Response.

    @param   connHandle - connection to use
    @param   pRsp - pointer to error response to be sent

    @return  SUCCESS: Response was sent successfully.
            INVALIDPARAMETER: Invalid response field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_ErrorRsp( uint16 connHandle, attErrorRsp_t* pRsp )
{
    return ( attSendMsg( connHandle, ATT_BuildErrorRsp, ATT_ERROR_RSP, (uint8*)pRsp ) );
}

/*********************************************************************
    @fn      ATT_ExchangeMTURsp

    @brief   Send Exchange MTU Response.

    @param   connHandle - connection to use
    @param   pRsp - pointer to request to be sent

    @return  SUCCESS: Response was sent successfully.
            INVALIDPARAMETER: Invalid response field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_ExchangeMTURsp( uint16 connHandle, attExchangeMTURsp_t* pRsp )
{
    // Validate Type field
    if ( pRsp->serverRxMTU >= ATT_MTU_SIZE_MIN )
    {
        bStatus_t ret = attSendMsg( connHandle, ATT_BuildExchangeMTURsp, ATT_EXCHANGE_MTU_RSP, (uint8*)pRsp ) ;
        #if 1

        if(ret==SUCCESS)
        {
//        ATT_MTU_SIZE_UPDATE(MIN(pRsp->serverRxMTU,g_attMtuClientServer.clientMTU));
            ATT_UpdateMtuSize(connHandle, MIN(pRsp->serverRxMTU,g_attMtuClientServer.clientMTU));
        }

        #endif
        return (ret );
    }

    return ( INVALIDPARAMETER );
}

/*********************************************************************
    @fn      ATT_FindInfoRsp

    @brief   Send Find Information Response.

    @param   connHandle - connection to use
    @param   pRsp - pointer to response to be sent

    @return  SUCCESS: Response was sent successfully.
            INVALIDPARAMETER: Invalid response field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_FindInfoRsp( uint16 connHandle, attFindInfoRsp_t* pRsp )
{
    // Validate Type field
    if ( ( ( pRsp->format == ATT_HANDLE_BT_UUID_TYPE ) ||
            ( pRsp->format == ATT_HANDLE_UUID_TYPE ) )  &&
            ( pRsp->numInfo > 0 ) )
    {
        return ( attSendMsg( connHandle, ATT_BuildFindInfoRsp, ATT_FIND_INFO_RSP, (uint8*)pRsp ) );
    }

    return ( INVALIDPARAMETER );
}

/*********************************************************************
    @fn      ATT_FindByTypeValueRsp

    @brief   Send Find By Tyep Value Response.

    @param   connHandle - connection to use
    @param   pRsp - pointer to response to be sent

    @return  SUCCESS: Response was sent successfully.
            INVALIDPARAMETER: Invalid response field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_FindByTypeValueRsp( uint16 connHandle, attFindByTypeValueRsp_t* pRsp )
{
    // Validate number of handle range field
    if ( pRsp->numInfo > 0 )
    {
        return ( attSendMsg( connHandle, ATT_BuildFindByTypeValueRsp,
                             ATT_FIND_BY_TYPE_VALUE_RSP, (uint8*)pRsp ) );
    }

    return ( INVALIDPARAMETER );
}

/*********************************************************************
    @fn      ATT_ReadByTypeRsp

    @brief   Send Read By Type Respond.

    @param   connHandle - connection to use
    @param   pRsp - pointer to response to be sent

    @return  SUCCESS: Response was sent successfully.
            INVALIDPARAMETER: Invalid response field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_ReadByTypeRsp( uint16 connHandle, attReadByTypeRsp_t* pRsp )
{
    if ( pRsp->numPairs > 0 )
    {
        return ( attSendMsg( connHandle, ATT_BuildReadByTypeRsp, ATT_READ_BY_TYPE_RSP, (uint8*)pRsp ) );
    }

    return ( INVALIDPARAMETER );
}

/*********************************************************************
    @fn      ATT_ReadRsp

    @brief   Send Read Response.

    @param   connHandle - connection to use
    @param   pRsp - pointer to response to be sent

    @return  SUCCESS: Response was sent successfully.
            INVALIDPARAMETER: Invalid response field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_ReadRsp( uint16 connHandle, attReadRsp_t* pRsp )
{
    return ( attSendMsg( connHandle, ATT_BuildReadRsp, ATT_READ_RSP, (uint8*)pRsp ) );
}

/*********************************************************************
    @fn      ATT_ReadBlobRsp

    @brief   Send Read Blob Response.

    @param   connHandle - connection to use
    @param   pRsp - pointer to response to be sent

    @return  SUCCESS: Response was sent successfully.
            INVALIDPARAMETER: Invalid response field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_ReadBlobRsp( uint16 connHandle, attReadBlobRsp_t* pRsp )
{
    return ( attSendMsg( connHandle, ATT_BuildReadBlobRsp, ATT_READ_BLOB_RSP, (uint8*)pRsp ) );
}

/*********************************************************************
    @fn      ATT_ReadMultiRsp

    @brief   Send Read Multiple Response.

    @param   connHandle - connection to use
    @param   pRsp - pointer to response to be sent

    @return  SUCCESS: Response was sent successfully.
            INVALIDPARAMETER: Invalid response field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_ReadMultiRsp( uint16 connHandle, attReadMultiRsp_t* pRsp )
{
    return ( attSendMsg( connHandle, ATT_BuildReadMultiRsp, ATT_READ_MULTI_RSP, (uint8*)pRsp ) );
}

/*********************************************************************
    @fn      ATT_ReadByGrpTypeRsp

    @brief   Send Read By Group Type Respond.

    @param   connHandle - connection to use
    @param   pRsp - pointer to response to be sent

    @return  SUCCESS: Response was sent successfully.
            INVALIDPARAMETER: Invalid response field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_ReadByGrpTypeRsp( uint16 connHandle, attReadByGrpTypeRsp_t* pRsp )
{
    if ( pRsp->numGrps > 0 )
    {
        return ( attSendMsg( connHandle, ATT_BuildReadByGrpTypeRsp, ATT_READ_BY_GRP_TYPE_RSP, (uint8*)pRsp ) );
    }

    return ( INVALIDPARAMETER );
}

/*********************************************************************
    @fn      ATT_WriteRsp

    @brief   Send Write Response.

    @param   connHandle - connection to use

    @return  SUCCESS: Response was sent successfully.
            INVALIDPARAMETER: Invalid response field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_WriteRsp( uint16 connHandle )
{
    return ( attSendMsg( connHandle, NULL, ATT_WRITE_RSP, NULL ) );
}

/*********************************************************************
    @fn      ATT_PrepareWriteRsp

    @brief   Send Prepare Write Response.

    @param   connHandle - connection to use
    @param   pRsp - pointer to response to be sent

    @return  SUCCESS: Response was sent successfully.
            INVALIDPARAMETER: Invalid response field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_PrepareWriteRsp( uint16 connHandle, attPrepareWriteRsp_t* pRsp )
{
    return ( attSendMsg( connHandle, ATT_BuildPrepareWriteRsp, ATT_PREPARE_WRITE_RSP, (uint8*)pRsp ) );
}

/*********************************************************************
    @fn      ATT_ExecuteWriteRsp

    @brief   Send Execute Write Response.

    @param   connHandle - connection to use

    @return  SUCCESS: Response was sent successfully.
            INVALIDPARAMETER: Invalid response field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_ExecuteWriteRsp( uint16 connHandle )
{
    return ( attSendMsg( connHandle, NULL, ATT_EXECUTE_WRITE_RSP, NULL ) );
}

/*********************************************************************
    @fn      ATT_HandleValueNoti

    @brief   Send Handle Value Notification.

    @param   connHandle - connection to use
    @param   pNoti - pointer to notification to be sent

    @return  SUCCESS: Notification was sent successfully.
            INVALIDPARAMETER: Invalid notification field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_HandleValueNoti( uint16 connHandle, attHandleValueNoti_t* pNoti )
{
    return ( attSendMsg( connHandle, ATT_BuildHandleValueInd, ATT_HANDLE_VALUE_NOTI, (uint8*)pNoti ) );
}

/*********************************************************************
    @fn      ATT_HandleValueInd

    @brief   Send Handle Value Indication.

    @param   connHandle - connection to use
    @param   pInd - pointer to indication to be sent

    @return  SUCCESS: Indication was sent successfully.
            INVALIDPARAMETER: Invalid indication field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t ATT_HandleValueInd( uint16 connHandle, attHandleValueInd_t* pInd )
{
    return ( attSendMsg( connHandle, ATT_BuildHandleValueInd, ATT_HANDLE_VALUE_IND, (uint8*)pInd ) );
}


/****************************************************************************
****************************************************************************/
