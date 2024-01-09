/*************************************************************************************************
    Filename:       smp.c
    Revised:
    Revision:

    Description:    This file contains the Security Manager Protocol.

	SDK_LICENSE

**************************************************************************************************/

#include "bcomdef.h"
#include "OSAL.h"
#include "osal_bufmgr.h"
#include "hci.h"
#include "l2cap.h"
#include "gap.h"
#include "sm.h"
#include "sm_internal.h"
#include "smp.h"

/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/

// Pairing Request & Response - Key Distribution Field - bit mask
//   These are used to break up byte in to keyDist_t
#define SMP_KEYDIST_ENCKEY                       0x01
#define SMP_KEYDIST_IDKEY                        0x02
#define SMP_KEYDIST_SIGN                         0x04

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
    FUNCTIONS
*/

/*********************************************************************
    @fn          smpBuildPairingReq

    @brief       Build an SM Pairing Request

    @param       pPairingReq - pointer to structure.
    @param       pBuf - data buffer to build into

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or pairingReq is NULL
*/
bStatus_t smpBuildPairingReq( smpPairingReq_t* pPairingReq, uint8* pBuf )
{
    return ( smpBuildPairingReqRsp( SMP_PAIRING_REQ, pPairingReq, pBuf ) );
}

/*********************************************************************
    @fn          smpBuildPairingRsp

    @brief       Build an SM Pairing Response

    @param       pPairingRsp - pointer to structure.
    @param       pBuf - data buffer to build into

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or pairingRsp is NULL
*/
bStatus_t smpBuildPairingRsp( smpPairingReq_t* pPairingRsp, uint8* pBuf )
{
    return ( smpBuildPairingReqRsp( SMP_PAIRING_RSP, pPairingRsp, pBuf ) );
}

/*********************************************************************
    @fn          smpBuildPairingReqRsp

    @brief       Build an SM Pairing Request or Response

    @param       code - either SMP_PAIRING_REQ or SMP_PAIRING_RSP
    @param       pPairingReq - pointer to structure.
    @param       pBuf - data buffer to build into

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or pairingReq is NULL
*/
bStatus_t smpBuildPairingReqRsp( uint8 opCode, smpPairingReq_t* pPairingReq, uint8* pBuf )
{
    // Check pointers
    if ( (pBuf == NULL) || (pPairingReq == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    *pBuf++ = opCode;
    *pBuf++ = pPairingReq->ioCapability;
    *pBuf++ = pPairingReq->oobDataFlag;
    *pBuf++ = smAuthReqToUint8( &(pPairingReq->authReq) );
    *pBuf++ = pPairingReq->maxEncKeySize;
    *pBuf = (pPairingReq->keyDist.mEncKey) ? (SMP_KEYDIST_ENCKEY) : 0;
    *pBuf |= (pPairingReq->keyDist.mIdKey) ? (SMP_KEYDIST_IDKEY) : 0;
    *pBuf |= (pPairingReq->keyDist.mSign) ? (SMP_KEYDIST_SIGN) : 0;
    *pBuf |= (pPairingReq->keyDist.mReserved) << 3;              // HZF: fixed bug 2018-11-27, recover reserved bits(5bits in BLE4.0)
    pBuf++;
    *pBuf = (pPairingReq->keyDist.sEncKey) ? (SMP_KEYDIST_ENCKEY) : 0;
    *pBuf |= (pPairingReq->keyDist.sIdKey) ? (SMP_KEYDIST_IDKEY) : 0;
    *pBuf |= (pPairingReq->keyDist.sSign) ? (SMP_KEYDIST_SIGN) : 0;
    *pBuf |= (pPairingReq->keyDist.sReserved) << 3;              // HZF: fixed bug 2018-11-27, recover reserved bits(5bits in BLE4.0)
    return ( SUCCESS );
}

/*********************************************************************
    @fn          smpParsePairingReq

    @brief       Parse an SM Pairing Request

    @param       pBuf - data buffer to parse
    @param       pPairingReq - pointer to structure.

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or pairingReq is NULL
                bleInvalidRange if a generic field is out of range
                bleIncorrectMode if enc key is out of range
*/
bStatus_t smpParsePairingReq( uint8* pBuf, smpPairingReq_t* pPairingReq )
{
    uint8 tmp;

    // Check pointers
    if ( (pBuf == NULL) || (pPairingReq == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    pBuf++; // Skip code
    pPairingReq->ioCapability = *pBuf++;
    pPairingReq->oobDataFlag = *pBuf++;
    smUint8ToAuthReq( &(pPairingReq->authReq), *pBuf++ );
    pPairingReq->maxEncKeySize = *pBuf++;
    tmp = *pBuf++;
    pPairingReq->keyDist.mEncKey = (tmp & SMP_KEYDIST_ENCKEY) ? TRUE : FALSE;
    pPairingReq->keyDist.mIdKey = (tmp & SMP_KEYDIST_IDKEY) ? TRUE : FALSE;
    pPairingReq->keyDist.mSign = (tmp & SMP_KEYDIST_SIGN) ? TRUE : FALSE;
    pPairingReq->keyDist.mReserved = (tmp & 0xF8)>>3;     // HZF: fixed bug 2018-11-27, saved reserved bits(5bits in BLE4.0)
    tmp = *pBuf;
    pPairingReq->keyDist.sEncKey = (tmp & SMP_KEYDIST_ENCKEY) ? TRUE : FALSE;
    pPairingReq->keyDist.sIdKey = (tmp & SMP_KEYDIST_IDKEY) ? TRUE : FALSE;
    pPairingReq->keyDist.sSign = (tmp & SMP_KEYDIST_SIGN) ? TRUE : FALSE;
    pPairingReq->keyDist.sReserved = (tmp & 0xF8)>>3;     // HZF: fixed bug 2018-11-27, saved reserved bits(5bits in BLE4.0)

    // Check the encryption key size
    if ( (pPairingReq->maxEncKeySize < GAP_GetParamValue( TGAP_SM_MIN_KEY_LEN ))
            || (pPairingReq->maxEncKeySize > GAP_GetParamValue( TGAP_SM_MAX_KEY_LEN )) )
    {
        return ( bleIncorrectMode );
    }

    // Check for range of fields
    if ( (pPairingReq->ioCapability > SMP_IO_CAP_KEYBOARD_DISPLAY)
            || (pPairingReq->oobDataFlag > SMP_OOB_AUTH_DATA_REMOTE_DEVICE_PRESENT) )
    {
        return ( bleInvalidRange );
    }
    else
    {
        return ( SUCCESS );
    }
}

/*********************************************************************
    @fn          smpBuildPairingConfirm

    @brief       Build an SM Pairing Confirm

    @param       pPairingConfirm - pointer to structure.
    @param       pBuf - data buffer to build into

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or pairingConfirm is NULL
*/
bStatus_t smpBuildPairingConfirm( smpPairingConfirm_t* pPairingConfirm,
                                  uint8* pBuf )
{
    // Check pointers
    if ( (pBuf == NULL) || (pPairingConfirm == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    *pBuf++ = SMP_PAIRING_CONFIRM;
    VOID osal_memcpy( pBuf, pPairingConfirm->confirmValue, SMP_CONFIRM_LEN );
    return ( SUCCESS );
}

/*********************************************************************
    @fn          smpParsePairingConfirm

    @brief       Parse an SM Pairing Confirm

    @param       pBuf - data buffer to build into
    @param       pPairingConfirm - pointer to structure.

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or pairingConfirm is NULL
*/
bStatus_t smpParsePairingConfirm( uint8* pBuf,
                                  smpPairingConfirm_t* pPairingConfirm )
{
    // Check pointers
    if ( (pBuf == NULL) || (pPairingConfirm == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    pBuf++; // Skip code
    VOID osal_memcpy( pPairingConfirm->confirmValue, pBuf, SMP_CONFIRM_LEN );
    return ( SUCCESS );
}

/*********************************************************************
    @fn          smpBuildPairingRandom

    @brief       Build an SM Pairing Random

    @param       pPairingRandom - pointer to structure.
    @param       pBuf - data buffer to build into

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or pairingRandom is NULL
*/
bStatus_t smpBuildPairingRandom( smpPairingRandom_t* pPairingRandom,
                                 uint8* pBuf )
{
    // Check pointers
    if ( (pBuf == NULL) || (pPairingRandom == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    *pBuf++ = SMP_PAIRING_RANDOM;
    VOID osal_memcpy( pBuf, pPairingRandom->randomValue, SMP_RANDOM_LEN );
    return ( SUCCESS );
}

/*********************************************************************
    @fn          smpParsePairingRandom

    @brief       Parse an SM Pairing Random

    @param       pBuf - data buffer to build into
    @param       pPairingRandom - pointer to structure.

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or pairingRandom is NULL
*/
bStatus_t smpParsePairingRandom( uint8* pBuf,
                                 smpPairingRandom_t* pPairingRandom )
{
    // Check pointers
    if ( (pBuf == NULL) || (pPairingRandom == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    pBuf++; // Skip code
    VOID osal_memcpy( pPairingRandom->randomValue, pBuf, SMP_RANDOM_LEN );
    return ( SUCCESS );
}

/*********************************************************************
    @fn          smpBuildPairingFailed

    @brief       Build an SM Pairing Failed

    @param       pPairingFailed - pointer to structure.
    @param       pBuf - data buffer to build into

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or pairingFailed is NULL
*/
bStatus_t smpBuildPairingFailed( smpPairingFailed_t* pPairingFailed,
                                 uint8* pBuf )
{
    // Check pointers
    if ( (pBuf == NULL) || (pPairingFailed == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    *pBuf++ = SMP_PAIRING_FAILED;
    *pBuf = pPairingFailed->reason;
    return ( SUCCESS );
}

/*********************************************************************
    @fn          smpParsePairingFailed

    @brief       Parse an SM Pairing Failed

    @param       pBuf - data buffer to build into
    @param       pPairingFailed - pointer to structure.

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or pairingFailed is NULL
                bleInvalidRange if the reason field is out of range
*/
bStatus_t smpParsePairingFailed( uint8* pBuf,
                                 smpPairingFailed_t* pPairingFailed )
{
    // Check pointers
    if ( (pBuf == NULL) || (pPairingFailed == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    pBuf++; // Skip code
    pPairingFailed->reason = *pBuf;

    // Reason range check
    if ( pPairingFailed->reason > SMP_PAIRING_FAILED_REPEATED_ATTEMPTS
            || (pPairingFailed->reason == 0) )
    {
        return ( bleInvalidRange );
    }
    else
    {
        return ( SUCCESS );
    }
}

/*********************************************************************
    @fn          smpBuildEncInfo

    @brief       Build an SM Encryption Information

    @param       pEncInfo - pointer to structure.
    @param       pBuf - data buffer to build into

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or struct ptr is NULL
*/
bStatus_t smpBuildEncInfo( smpEncInfo_t* pEncInfo, uint8* pBuf )
{
    // Check pointers
    if ( (pBuf == NULL) || (pEncInfo == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    *pBuf++ = SMP_ENCRYPTION_INFORMATION;
    VOID osal_memcpy( pBuf, pEncInfo->ltk, KEYLEN );
    return ( SUCCESS );
}

/*********************************************************************
    @fn          smpParseEncInfo

    @brief       Parse an SM Encryption Information

    @param       pBuf - data buffer to build into
    @param       pEncInfo - pointer to structure.

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or struct ptr is NULL
*/
bStatus_t smpParseEncInfo( uint8* pBuf, smpEncInfo_t* pEncInfo )
{
    // Check pointers
    if ( (pBuf == NULL) || (pEncInfo == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    pBuf++; // Skip code
    VOID osal_memcpy( pEncInfo->ltk, pBuf, KEYLEN );
    return ( SUCCESS );
}

/*********************************************************************
    @fn          smpBuildMasterID

    @brief       Build an SM Master Identification

    @param       pMasterID - pointer to structure.
    @param       pBuf - data buffer to build into

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or struct ptr is NULL
*/
bStatus_t smpBuildMasterID( smpMasterID_t* pMasterID, uint8* pBuf )
{
    // Check pointers
    if ( (pBuf == NULL) || (pMasterID == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    *pBuf++ = SMP_MASTER_IDENTIFICATION;
    *pBuf++ = LO_UINT16( pMasterID->ediv );
    *pBuf++ = HI_UINT16( pMasterID->ediv );
    VOID osal_memcpy( pBuf, pMasterID->rand, B_RANDOM_NUM_SIZE );
    return ( SUCCESS );
}

/*********************************************************************
    @fn          smpParseMasterID

    @brief       Parse an SM Master Identification

    @param       pBuf - data buffer to build into
    @param       pMasterID - pointer to structure.

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or struct ptr is NULL
*/
bStatus_t smpParseMasterID( uint8* pBuf, smpMasterID_t* pMasterID )
{
    // Check pointers
    if ( (pBuf == NULL) || (pMasterID == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    pBuf++; // Skip code
    pMasterID->ediv = BUILD_UINT16( pBuf[0], pBuf[1] );
    VOID osal_memcpy( pMasterID->rand, &pBuf[2], B_RANDOM_NUM_SIZE );
    return ( SUCCESS );
}

/*********************************************************************
    @fn          smpBuildIdentityInfo

    @brief       Build an SM Identity Information

    @param       pIdInfo - pointer to structure.
    @param       pBuf - data buffer to build into

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or struct ptr is NULL
*/
bStatus_t smpBuildIdentityInfo( smpIdentityInfo_t* pIdInfo, uint8* pBuf )
{
    // Check pointers
    if ( (pBuf == NULL) || (pIdInfo == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    *pBuf++ = SMP_IDENTITY_INFORMATION;
    VOID osal_memcpy( pBuf, pIdInfo->irk, KEYLEN );
    return ( SUCCESS );
}

/*********************************************************************
    @fn          smpBuildIdentityAddrInfo

    @brief       Build an SM Identity Address Information

    @param       pIdInfo - pointer to structure.
    @param       pBuf - data buffer to build into

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or struct ptr is NULL
*/
bStatus_t smpBuildIdentityAddrInfo( smpIdentityAddrInfo_t* pIdInfo, uint8* pBuf )
{
    // Check pointers
    if ( (pBuf == NULL) || (pIdInfo == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    *pBuf++ = SMP_IDENTITY_ADDR_INFORMATION;
    *pBuf++ = pIdInfo->addrType;
    VOID osal_memcpy( pBuf, pIdInfo->bdAddr, B_ADDR_LEN );
    return ( SUCCESS );
}

/*********************************************************************
    @fn          smpParseIdentityInfo

    @brief       Parse an SM Identity Information

    @param       pBuf - data buffer to build into
    @param       pIdInfo - pointer to structure.

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or struct ptr is NULL
*/
bStatus_t smpParseIdentityInfo( uint8* pBuf, smpIdentityInfo_t* pIdInfo )
{
    // Check pointers
    if ( (pBuf == NULL) || (pIdInfo == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    pBuf++; // Skip code
    VOID osal_memcpy( pIdInfo->irk, pBuf, KEYLEN );
    return ( SUCCESS );
}

/*********************************************************************
    @fn          smpParseIdentityAddrInfo

    @brief       Parse an SM Identity Address Information

    @param       pBuf - data buffer to build into
    @param       pIdInfo - pointer to structure.

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or struct ptr is NULL
*/
bStatus_t smpParseIdentityAddrInfo( uint8* pBuf, smpIdentityAddrInfo_t* pIdInfo )
{
    // Check pointers
    if ( (pBuf == NULL) || (pIdInfo == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    pBuf++; // Skip code
    pIdInfo->addrType = *pBuf++;
    VOID osal_memcpy( pIdInfo->bdAddr, pBuf, B_ADDR_LEN );
    return ( SUCCESS );
}

/*********************************************************************
    @fn          smpBuildSigningInfo

    @brief       Build an SM Signing Information

    @param       pSigningInfo - pointer to structure.
    @param       pBuf - data buffer to build into

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or struct ptr is NULL
*/
bStatus_t smpBuildSigningInfo( smpSigningInfo_t* pSigningInfo, uint8* pBuf )
{
    // Check pointers
    if ( (pBuf == NULL) || (pSigningInfo == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    *pBuf++ = SMP_SIGNING_INFORMATION;
    VOID osal_memcpy( pBuf, pSigningInfo->signature, KEYLEN );
    return ( SUCCESS );
}

/*********************************************************************
    @fn          smpParseSigningInfo

    @brief       Parse an SM Signing Information

    @param       pBuf - data buffer to build into
    @param       pSigningInfo - pointer to structure.

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or struct ptr is NULL
*/
bStatus_t smpParseSigningInfo( uint8* pBuf, smpSigningInfo_t* pSigningInfo )
{
    // Check pointers
    if ( (pBuf == NULL) || (pSigningInfo == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    pBuf++; // Skip code
    VOID osal_memcpy( pSigningInfo->signature, pBuf, KEYLEN );
    return ( SUCCESS );
}

/*********************************************************************
    @fn          smpBuildSecurityReq

    @brief       Build an SM Slave Security Request

    @param       pSecReq - pointer to structure.
    @param       pBuf - data buffer to build into

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or struct ptr is NULL
*/
bStatus_t smpBuildSecurityReq( smpSecurityReq_t* pSecReq, uint8* pBuf )
{
    // Check pointers
    if ( (pBuf == NULL) || (pSecReq == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    *pBuf++ = SMP_SECURITY_REQUEST;
    *pBuf = smAuthReqToUint8( &(pSecReq->authReq) );
    return ( SUCCESS );
}

/*********************************************************************
    @fn          smpParseSecurityReq

    @brief       Parse an SM Slave Security Request

    @param       pBuf - data buffer to build into
    @param       pSecReq - pointer to structure.

    @return      SUCCESS if parsed
                INVALIDPARAMETER if buf or struct ptr is NULL
*/
bStatus_t smpParseSecurityReq( uint8* pBuf, smpSecurityReq_t* pSecReq )
{
    // Check pointers
    if ( (pBuf == NULL) || (pSecReq == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    pBuf++; // Skip code
    smUint8ToAuthReq( &(pSecReq->authReq), *pBuf );
    return ( SUCCESS );
}

/*********************************************************************
    @fn          smSendSMMsg

    @brief       Allocate send buffer, build the SM message and send
                the message to L2CAP.

    @param       connHandle - Connection to send message
    @param       bufLen - Length of buffer needed
    @param       pMsg - message structure
    @param       buildFn - function pointer to build function

    @return      status
*/
bStatus_t smSendSMMsg( uint16 connHandle, uint8 bufLen, smpMsgs_t* pMsg, pfnSMBuildCmd_t buildFn )
{
    bStatus_t stat;
    l2capPacket_t sendData;
    // Allocate a buffer
    sendData.CID = L2CAP_CID_SMP;
    sendData.len = bufLen;
    sendData.pPayload = (uint8*)L2CAP_bm_alloc( sendData.len );

    if ( sendData.pPayload )
    {
        stat = buildFn( pMsg, sendData.pPayload );

        if ( stat == SUCCESS )
        {
            stat = L2CAP_SendData( connHandle, &sendData );
        }

        if ( stat != SUCCESS )
        {
            osal_bm_free( sendData.pPayload );
        }
    }
    else
    {
        stat = bleMemAllocError;
    }

    // TODO : if stat is not success , start sm tsp timer is there any problem ?
    // Start the SM Timeout
    smStartRspTimer(connHandle);
    return ( stat );
}



