/*************************************************************************************************
    Filename:       sm_intpairing.c
    Revised:
    Revision:

    Description:    This file contains the SM Initiator Pairing Manager.

	SDK_LICENSE

**************************************************************************************************/


/*******************************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "l2cap.h"
#include "gap_internal.h"
#include "linkdb.h"
#include "sm.h"
#include "sm_internal.h"
#include "smp.h"
#include "osal_cbtimer.h"

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
extern uint8 smState_CBTimer[];

/*********************************************************************
    EXTERNAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/

/*********************************************************************
    LOCAL FUNCTIONS
*/
static uint8 smpInitiatorProcessIncoming( linkDBItem_t* pLinkItem, uint8 cmdID, smpMsgs_t* pParsedMsg );
static uint8 smpInitiatorProcessPairingRsp( uint16 connectionHandle,smpPairingRsp_t* parsedMsg );
static uint8 smpInitiatorProcessPairingConfirm( uint16 connectionHandle,smpPairingConfirm_t* pParsedMsg );
static uint8 smpInitiatorProcessPairingRandom( uint16 connectionHandle,smpPairingRandom_t* pParsedMsg );
static uint8 smpInitiatorProcessEncryptionInformation( uint16 connectionHandle,smpEncInfo_t* pParsedMsg );
static uint8 smpInitiatorProcessMasterID( uint16 connectionHandle,smpMasterID_t* pParsedMsg );
static uint8 smpInitiatorProcessIdentityInfo( uint16 connectionHandle,smpIdentityInfo_t* pParsedMsg );
static uint8 smpInitiatorProcessIdentityAddrInfo( uint16 connectionHandle,smpIdentityAddrInfo_t* pParsedMsg );
static uint8 smpInitiatorProcessSigningInfo( uint16 connectionHandle,smpSigningInfo_t* pParsedMsg );

static void setupInitiatorKeys( uint16 connectionHandle );
static void smInitiatorSendNextKeyInfo( uint16 connectionHandle );

/*********************************************************************
    INITIATOR CALLBACKS
*/

// SM Initiator Callbacks
static smInitiatorCBs_t smInitiatorCBs =
{
    smpInitiatorProcessIncoming, // Process SMP Message Callback
    smInitiatorSendNextKeyInfo,  // Send Next Key Message Callback
    SM_StartEncryption           // Start Encrypt Callback
};

/*********************************************************************
    API FUNCTIONS
*/


/*********************************************************************
    FUNCTIONS - MASTER API - Only use these in a master device
*/

/*********************************************************************
    Initialize SM Initiator on a master device.

    Public function defined in sm.h.
*/
bStatus_t SM_InitiatorInit( void )
{
    if ( gapProfileRole & GAP_PROFILE_CENTRAL )
    {
        // Set up Initiator's processing function
        smRegisterInitiator( &smInitiatorCBs );
    }
    else
    {
        smRegisterInitiator( NULL );
    }

    return ( SUCCESS );
}

/*********************************************************************
    @fn          smEncLTK

    @brief       Start LKT Encryption on an Initiator.

    @param       none

    @return      none
*/
void smEncLTK( uint16 connectionHandle )
{
    // Make sure we are in the right state
    if ( (pPairingParams[connectionHandle]) && (pPairingParams[connectionHandle]->initiator)
            && (pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_ENCRYPT)
            && (pPairingParams[connectionHandle]->pDevEncParams) )
    {
        if ( SM_StartEncryption( pPairingParams[connectionHandle]->connectionHandle,
                                 pPairingParams[connectionHandle]->pDevEncParams->ltk,
                                 pPairingParams[connectionHandle]->pDevEncParams->div,
                                 pPairingParams[connectionHandle]->pDevEncParams->rand,
                                 pPairingParams[connectionHandle]->pDevEncParams->keySize ) != SUCCESS )
        {
            // Start encryption failed
            smEndPairing( connectionHandle,SMP_PAIRING_FAILED_UNSPECIFIED );
        }
    }
}

/*********************************************************************
    @fn          SM_StartEncryption

    @brief       Send Start Encrypt through HCI

    @param       connHandle - Connection Handle
    @param       pLTK - pointer to 16 byte lkt
    @param       div - div or ediv
    @param       pRandNum - pointer to 8 byte random number
    @param       keyLen - length of LTK (bytes)

    @return      SUCCESS
                INVALIDPARAMETER
                other from HCI/LL
*/
bStatus_t SM_StartEncryption( uint16 connHandle, uint8* pLTK, uint16 div, uint8* pRandNum, uint8 keyLen )
{
    uint8 eDiv[2];                    // LTK div
    uint8 key[KEYLEN];                // LTK
    uint8 random[B_RANDOM_NUM_SIZE];  // LTK Random

    // check the parameters
    if ( pLTK == NULL )
    {
        return ( INVALIDPARAMETER );
    }

    // Setup encryption parameters
    eDiv[0] = LO_UINT16( div );
    eDiv[1] = HI_UINT16( div );
    VOID osal_memset( key, 0, KEYLEN );
    VOID osal_memcpy( key, pLTK, keyLen );

    // A null randNum means to build a random number of all zero's
    if ( pRandNum )
    {
        VOID osal_memcpy( random, pRandNum, B_RANDOM_NUM_SIZE );
    }
    else
    {
        VOID osal_memset( random, 0, B_RANDOM_NUM_SIZE );
    }

    return ( HCI_LE_StartEncyptCmd( connHandle, random, eDiv, key  ) );
}

/*********************************************************************
    @fn          smpInitiatorProcessIncoming

    @brief       Process incoming parsed SM Initiator message.

    @param       pLinkItem - connection information
    @param       cmdID - command ID
    @param       pParsedMsg - pointer to parsed message

    @return      SUCCESS
                SMP_PAIRING_FAILED_PASSKEY_ENTRY_FAILED
                SMP_PAIRING_FAILED_OOB_NOT_AVAIL
                SMP_PAIRING_FAILED_AUTH_REQ
                SMP_PAIRING_FAILED_CONFIRM_VALUE
                SMP_PAIRING_FAILED_NOT_SUPPORTED
                SMP_PAIRING_FAILED_ENC_KEY_SIZE
                SMP_PAIRING_FAILED_CMD_NOT_SUPPORTED
                SMP_PAIRING_FAILED_UNSPECIFIED
                SMP_PAIRING_FAILED_REPEATED_ATTEMPTS
*/
static uint8 smpInitiatorProcessIncoming( linkDBItem_t* pLinkItem, uint8 cmdID, smpMsgs_t* pParsedMsg )
{
    uint8 reason = SUCCESS;   // return value

    // check for pairing mode
    if ( pPairingParams[pLinkItem->connectionHandle] == NULL )
    {
        if ( cmdID == SMP_SECURITY_REQUEST )
        {
            // Notify app/profile
            gapSendSlaveSecurityReqEvent( pLinkItem->taskID, pLinkItem->connectionHandle,
                                          pLinkItem->addr, smAuthReqToUint8( &(pParsedMsg->secReq.authReq) ) );
            return ( SUCCESS );
        }
        else
        {
            // Ignore the message, don't respond
            return ( SUCCESS );
        }
    }

    // We can only handle one pairing at a time
    if ( pPairingParams[pLinkItem->connectionHandle]->connectionHandle != pLinkItem->connectionHandle )
    {
        return ( SMP_PAIRING_FAILED_UNSPECIFIED );
    }

    if ( pPairingParams[pLinkItem->connectionHandle]->initiator == FALSE )
    {
        return ( SMP_PAIRING_FAILED_CMD_NOT_SUPPORTED );
    }

    // Process the pairing messages
    switch ( cmdID )
    {
    case SMP_PAIRING_RSP:
        reason = smpInitiatorProcessPairingRsp( pLinkItem->connectionHandle,(smpPairingRsp_t*)pParsedMsg );
        break;

    case SMP_PAIRING_CONFIRM:
        reason = smpInitiatorProcessPairingConfirm( pLinkItem->connectionHandle,(smpPairingConfirm_t*)pParsedMsg );
        break;

    case SMP_PAIRING_RANDOM:
        reason = smpInitiatorProcessPairingRandom( pLinkItem->connectionHandle,(smpPairingRandom_t*)pParsedMsg );
        break;

    case SMP_ENCRYPTION_INFORMATION:
        reason = smpInitiatorProcessEncryptionInformation( pLinkItem->connectionHandle,(smpEncInfo_t*)pParsedMsg );
        break;

    case SMP_MASTER_IDENTIFICATION:
        reason = smpInitiatorProcessMasterID( pLinkItem->connectionHandle,(smpMasterID_t*)pParsedMsg );
        break;

    case SMP_IDENTITY_INFORMATION:
        reason = smpInitiatorProcessIdentityInfo( pLinkItem->connectionHandle,(smpIdentityInfo_t*)pParsedMsg );
        break;

    case SMP_IDENTITY_ADDR_INFORMATION:
        reason = smpInitiatorProcessIdentityAddrInfo( pLinkItem->connectionHandle,(smpIdentityAddrInfo_t*)pParsedMsg );
        break;

    case SMP_SIGNING_INFORMATION:
        reason = smpInitiatorProcessSigningInfo( pLinkItem->connectionHandle,(smpSigningInfo_t*)pParsedMsg );
        break;

    case SMP_SECURITY_REQUEST:
        if ( pPairingParams[pLinkItem->connectionHandle] )
        {
            // We are currently pairing. Ignore the message, don't respond
            return ( SUCCESS );
        }

        break;

    case SMP_PAIRING_FAILED:
        smEndPairing( pLinkItem->connectionHandle,pParsedMsg->pairingFailed.reason );
        break;

    default:
        reason = SMP_PAIRING_FAILED_CMD_NOT_SUPPORTED;
        break;
    }

    return ( reason );
}

/*********************************************************************
    @fn          smpInitiatorProcessPairingRsp

    @brief       Process incoming parsed Pairing Response.

    @param       pParsedMsg - pointer to parsed message

    @return      SUCCESS
                SMP_PAIRING_FAILED_CMD_NOT_SUPPORTED
                SMP_PAIRING_FAILED_UNSPECIFIED
                SMP_PAIRING_FAILED_AUTH_REQ
*/
static uint8 smpInitiatorProcessPairingRsp( uint16 connectionHandle,smpPairingRsp_t* pParsedMsg )
{
    uint8 reason = SUCCESS; // return value
    bStatus_t stat; // status field
    // Save the response information into pPairingParams[connectionHandle]
    stat = smSavePairInfo( connectionHandle,pParsedMsg );

    if ( stat == SUCCESS )
    {
        // Check for bonding
        if ( (pPairingParams[connectionHandle]->pSecReqs->authReq & SM_AUTH_STATE_BONDING)
                && (pPairingParams[connectionHandle]->pPairDev->authReq.bonding == SM_AUTH_REQ_BONDING) )
        {
            pPairingParams[connectionHandle]->authState |= SM_AUTH_STATE_BONDING;
        }

        if ( (pPairingParams[connectionHandle]->type == SM_PAIRING_TYPE_PASSKEY_INITIATOR_INPUTS)
                || (pPairingParams[connectionHandle]->type == SM_PAIRING_TYPE_PASSKEY_RESPONDER_INPUTS)
                || (pPairingParams[connectionHandle]->type == SM_PAIRING_TYPE_PASSKEY_BOTH_INPUTS) )
        {
            uint8 type;

            // Determine the passkey input/output user requirements
            if ( (pPairingParams[connectionHandle]->type == SM_PAIRING_TYPE_PASSKEY_BOTH_INPUTS)
                    || (pPairingParams[connectionHandle]->type == SM_PAIRING_TYPE_PASSKEY_INITIATOR_INPUTS) )
            {
                type = SM_PASSKEY_TYPE_INPUT;
            }
            else
            {
                type = SM_PASSKEY_TYPE_DISPLAY;
            }

            // Ask the app for passkey
            gapPasskeyNeededCB( pPairingParams[connectionHandle]->connectionHandle, type );
            pPairingParams[connectionHandle]->authState |= SM_AUTH_STATE_AUTHENTICATED;
            pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_PASSKEY;
        }
        else if ( (pPairingParams[connectionHandle]->type == SM_PAIRING_TYPE_JUST_WORKS)
                  || (pPairingParams[connectionHandle]->type == SM_PAIRING_TYPE_OOB) )
        {
            // Get ready for the Confirm
            pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_CONFIRM;
            // Initialize TK space
            VOID osal_memset( pPairingParams[connectionHandle]->tk, 0, KEYLEN );

            // Setup TK
            if ( pPairingParams[connectionHandle]->type != SM_PAIRING_TYPE_JUST_WORKS )
            {
                // OOB
                VOID osal_memcpy( pPairingParams[connectionHandle]->tk, pPairingParams[connectionHandle]->pSecReqs->oob, KEYLEN );
                pPairingParams[connectionHandle]->authState |= SM_AUTH_STATE_AUTHENTICATED;
            }

            // Generate Rand (MRand)
            smGenerateRandBuf( pPairingParams[connectionHandle]->myComp.rand, SMP_RANDOM_LEN );
            // Generate Confirm (MConfirm)
            sm_c1( connectionHandle,pPairingParams[connectionHandle]->tk,
                   pPairingParams[connectionHandle]->myComp.rand,
                   pPairingParams[connectionHandle]->myComp.confirm );
            #if defined ( TESTMODES )

            if ( GAP_GetParamValue( TGAP_SM_TESTCODE ) == SM_TESTMODE_SEND_BAD_CONFIRM )
            {
                VOID osal_memset( pPairingParams[connectionHandle]->myComp.confirm, 0, KEYLEN );
            }

            #endif // TESTMODE

            if ( smGenerateConfirm( connectionHandle) != SUCCESS )
            {
                reason = SMP_PAIRING_FAILED_UNSPECIFIED;
            }
        }
    }
    else if ( stat == bleInvalidRange )
    {
        reason = SMP_PAIRING_FAILED_AUTH_REQ;
    }
    else
    {
        reason = SMP_PAIRING_FAILED_UNSPECIFIED;
    }

    return ( reason );
}

/*********************************************************************
    @fn          smpInitiatorProcessPairingConfirm

    @brief       Process incoming parsed Pairing Confirm.

    @param       pParsedMsg - pointer to parsed message

    @return      SUCCESS
                SMP_PAIRING_FAILED_UNSPECIFIED
*/
static uint8 smpInitiatorProcessPairingConfirm( uint16 connectionHandle,smpPairingConfirm_t* pParsedMsg )
{
    uint8 reason = SUCCESS;  // return value
    VOID osal_memcpy( pPairingParams[connectionHandle]->devComp.confirm, pParsedMsg->confirmValue, KEYLEN );

    // Received Responder Confirm, send Rand message
    if ( smGenerateRandMsg(connectionHandle) != SUCCESS )
    {
        reason = SMP_PAIRING_FAILED_UNSPECIFIED;
    }

    pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_RANDOM;
    return ( reason );
}

/*********************************************************************
    @fn          smpInitiatorProcessPairingRandom

    @brief       Process incoming parsed Pairing Random.

    @param       parsedMsg - pointer to parsed message

    @return      SUCCESS
                SMP_PAIRING_FAILED_CONFIRM_VALUE
                SMP_PAIRING_FAILED_UNSPECIFIED
*/
static uint8 smpInitiatorProcessPairingRandom( uint16 connectionHandle,smpPairingRandom_t* pParsedMsg )
{
    uint8 reason = SUCCESS;   // return value
    uint8 confirm[KEYLEN];    // working area to calculate a confirm value
    VOID osal_memcpy( pPairingParams[connectionHandle]->devComp.rand, pParsedMsg->randomValue, SMP_RANDOM_LEN );
    // Check device's Confirm value
    sm_c1(connectionHandle, pPairingParams[connectionHandle]->tk,
          pPairingParams[connectionHandle]->devComp.rand,
          confirm );

    // Make sure that the calculated confirm matches the confirm from the other device
    if ( osal_memcmp( confirm, pPairingParams[connectionHandle]->devComp.confirm, KEYLEN ) == TRUE )
    {
        uint8 stk[KEYLEN];  // a place to generate the STK

        // Received Responder, generate STK
        if ( sm_s1( pPairingParams[connectionHandle]->tk, pPairingParams[connectionHandle]->devComp.rand,
                    pPairingParams[connectionHandle]->myComp.rand, stk ) == SUCCESS )
        {
            // Start Encrypt with STK
            if ( SM_StartEncryption( pPairingParams[connectionHandle]->connectionHandle,
                                     stk, 0, 0, smDetermineKeySize( connectionHandle) ) != SUCCESS )
            {
                reason = SMP_PAIRING_FAILED_UNSPECIFIED;
            }
        }
        else
        {
            reason = SMP_PAIRING_FAILED_UNSPECIFIED;
        }

        // Wait for STK to finish
        pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_STK;
    }
    else
    {
        reason = SMP_PAIRING_FAILED_CONFIRM_VALUE;
    }

    return ( reason );
}

/*********************************************************************
    @fn          smpInitiatorProcessEncryptionInformation

    @brief       Process incoming parsed Encryption Information.

    @param       pParsedMsg - pointer to parsed message

    @return      SUCCESS
                SMP_PAIRING_FAILED_UNSPECIFIED
*/
static uint8 smpInitiatorProcessEncryptionInformation( uint16 connectionHandle,smpEncInfo_t* pParsedMsg )
{
    if ( pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_SLAVE_ENCRYPTION_INFO )
    {
        // Save off the connected device's encryption information (LTK and key size)
        if ( pPairingParams[connectionHandle]->pDevEncParams == NULL )
        {
            pPairingParams[connectionHandle]->pDevEncParams = (smSecurityInfo_t*)osal_mem_alloc( (uint16)sizeof (smSecurityInfo_t ) );
        }

        if ( pPairingParams[connectionHandle]->pDevEncParams )
        {
            VOID osal_memcpy( pPairingParams[connectionHandle]->pDevEncParams->ltk, pParsedMsg->ltk, KEYLEN );
            pPairingParams[connectionHandle]->pDevEncParams->keySize = smDetermineKeySize( connectionHandle);
            pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_SLAVE_MASTER_INFO;
            return ( SUCCESS );
        }
    }

    return ( SMP_PAIRING_FAILED_CMD_NOT_SUPPORTED );
}

/*********************************************************************
    @fn          smpInitiatorProcessMasterID

    @brief       Process incoming parsed Master Identification.

    @param       parsedMsg - pointer to parsed message

    @return      SUCCESS
                SMP_PAIRING_FAILED_UNSPECIFIED
*/
static uint8 smpInitiatorProcessMasterID( uint16 connectionHandle,smpMasterID_t* pParsedMsg )
{
    if ( (pPairingParams[connectionHandle]->pDevEncParams != NULL)
            && (pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_SLAVE_MASTER_INFO) )
    {
        // Save off the rest of the connected device's encryption information
        pPairingParams[connectionHandle]->pDevEncParams->div = pParsedMsg->ediv;
        VOID osal_memcpy( pPairingParams[connectionHandle]->pDevEncParams->rand, pParsedMsg->rand, B_RANDOM_NUM_SIZE );

        // Setup the next state
        if ( (pPairingParams[connectionHandle]->pSecReqs->keyDist.sIdKey) && (pPairingParams[connectionHandle]->pPairDev->keyDist.sIdKey) )
        {
            pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_SLAVE_IDENTITY_INFO;
        }
        else if ( (pPairingParams[connectionHandle]->pSecReqs->keyDist.sSign) && (pPairingParams[connectionHandle]->pPairDev->keyDist.sSign) )
        {
            pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_SLAVE_SIGNING_INFO;
        }
        else
        {
            setupInitiatorKeys(connectionHandle);
        }

        return ( SUCCESS );
    }
    else
    {
        return (  SMP_PAIRING_FAILED_CMD_NOT_SUPPORTED );
    }
}

/*********************************************************************
    @fn          smpInitiatorProcessIdentityInfo

    @brief       Process incoming parsed Identity Information.

    @param       parsedMsg - pointer to parsed message

    @return      SUCCESS
                SMP_PAIRING_FAILED_UNSPECIFIED
*/
static uint8 smpInitiatorProcessIdentityInfo( uint16 connectionHandle,smpIdentityInfo_t* pParsedMsg )
{
    if ( pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_SLAVE_IDENTITY_INFO )
    {
        // Save off the info
        if ( pPairingParams[connectionHandle]->pIdInfo == NULL )
        {
            pPairingParams[connectionHandle]->pIdInfo = (smIdentityInfo_t*)osal_mem_alloc( (uint16)sizeof (smIdentityInfo_t ) );
        }

        if ( pPairingParams[connectionHandle]->pIdInfo )
        {
            VOID osal_memcpy( pPairingParams[connectionHandle]->pIdInfo->irk, pParsedMsg->irk, KEYLEN );
        }

        // Determine next state
        pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_SLAVE_IDENTITY_ADDR_INFO;
        return ( SUCCESS );
    }
    else
    {
        return ( SMP_PAIRING_FAILED_CMD_NOT_SUPPORTED );
    }
}

/*********************************************************************
    @fn          smpInitiatorProcessIdentityAddrInfo

    @brief       Process incoming parsed Identity Address Information.

    @param       pParsedMsg - pointer to parsed message

    @return      SUCCESS
                SMP_PAIRING_FAILED_UNSPECIFIED
*/
static uint8 smpInitiatorProcessIdentityAddrInfo( uint16 connectionHandle,smpIdentityAddrInfo_t* pParsedMsg )
{
    if ( pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_SLAVE_IDENTITY_ADDR_INFO )
    {
        VOID osal_memcpy( pPairingParams[connectionHandle]->pIdInfo->bd_addr, pParsedMsg->bdAddr, B_ADDR_LEN );

        // Determine the next state
        if ( (pPairingParams[connectionHandle]->pSecReqs->keyDist.sSign) && (pPairingParams[connectionHandle]->pPairDev->keyDist.sSign) )
        {
            pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_SLAVE_SIGNING_INFO;
        }
        else
        {
            // Start sending initiator (master) keys
            setupInitiatorKeys(connectionHandle);
        }

        return ( SUCCESS );
    }
    else
    {
        return ( SMP_PAIRING_FAILED_CMD_NOT_SUPPORTED );
    }
}

/*********************************************************************
    @fn          smpInitiatorProcessSigningInfo

    @brief       Process incoming parsed Signing Information.

    @param       parsedMsg - pointer to parsed message

    @return      SUCCESS
                SMP_PAIRING_FAILED_UNSPECIFIED
*/
static uint8 smpInitiatorProcessSigningInfo( uint16 connectionHandle,smpSigningInfo_t* pParsedMsg )
{
    if ( pPairingParams[connectionHandle] )
    {
        if ( pPairingParams[connectionHandle]->pSigningInfo == NULL )
        {
            pPairingParams[connectionHandle]->pSigningInfo = (smSigningInfo_t*)osal_mem_alloc( (uint16)sizeof( smSigningInfo_t ) );

            if ( pPairingParams[connectionHandle]->pSigningInfo == NULL )
            {
                // Only error available for memory error, this will end the pairing process
                return ( SMP_PAIRING_FAILED_UNSPECIFIED );
            }
        }

        // Copy signature information
        if ( pPairingParams[connectionHandle]->pSigningInfo )
        {
            VOID osal_memcpy( pPairingParams[connectionHandle]->pSigningInfo->srk, pParsedMsg->signature, KEYLEN );
            pPairingParams[connectionHandle]->pSigningInfo->signCounter = GAP_INIT_SIGN_COUNTER;
        }

        // Send the initiator key messages
        setupInitiatorKeys(connectionHandle);
        return ( SUCCESS );
    }
    else
    {
        return ( SMP_PAIRING_FAILED_CMD_NOT_SUPPORTED );
    }
}

/*********************************************************************
    @fn          setupInitiatorKeys

    @brief       Setup Initiator Key distribution

    @param       none

    @return      none
*/
static void setupInitiatorKeys( uint16 connectionHandle )
{
    if ( ((pPairingParams[connectionHandle]->pSecReqs->keyDist.mEncKey) && (pPairingParams[connectionHandle]->pPairDev->keyDist.mEncKey))
            || ((pPairingParams[connectionHandle]->pSecReqs->keyDist.mIdKey) && (pPairingParams[connectionHandle]->pPairDev->keyDist.mIdKey))
            || ((pPairingParams[connectionHandle]->pSecReqs->keyDist.mSign) && (pPairingParams[connectionHandle]->pPairDev->keyDist.mSign)) )
    {
        // Setup to send initiator key messages
        pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_STK;
        smInitiatorSendNextKeyInfo(connectionHandle);
    }
    else
    {
        // No keys to send
        pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_DONE;
    }

    if ( pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_DONE )
    {
        smEndPairing( connectionHandle,SUCCESS );
    }
}

/*********************************************************************
    @fn          smInitiatorSendNextKeyInfo

    @brief       Initiator role: sends next key message, and sets state
                for next event.

    @param       none

    @return      none
*/
static void smInitiatorSendNextKeyInfo( uint16 connectionHandle )
{
    if ( pPairingParams[connectionHandle]->initiator == TRUE )
    {
        smLinkSecurityReq_t* pSecReq = pPairingParams[connectionHandle]->pSecReqs;
        smpPairingReq_t* pPairReq = pPairingParams[connectionHandle]->pPairDev;
        uint8 state = pPairingParams[connectionHandle]->state;

        // Determine key to send
        if ( state == SM_PAIRING_STATE_WAIT_STK )
        {
            if ( (pPairReq->keyDist.mEncKey) && (pSecReq->keyDist.mEncKey) )
            {
                state = SM_PAIRING_STATE_WAIT_MASTER_ENCRYPTION_INFO;
            }
            else if ( (pPairReq->keyDist.mIdKey) && (pSecReq->keyDist.mIdKey) )
            {
                state = SM_PAIRING_STATE_WAIT_MASTER_IDENTITY_INFO;
            }
            else if ( (pPairReq->keyDist.mSign) && (pSecReq->keyDist.mSign) )
            {
                state = SM_PAIRING_STATE_WAIT_MASTER_SIGNING_INFO;
            }
        }

        // Send the correct message
        switch ( state )
        {
        case SM_PAIRING_STATE_WAIT_MASTER_ENCRYPTION_INFO:

            // send Encryption Information.
            if ( pPairingParams[connectionHandle]->pEncParams == NULL )
            {
                pPairingParams[connectionHandle]->pEncParams = (smSecurityInfo_t*)osal_mem_alloc( (uint16)sizeof (smSecurityInfo_t ) );

                if ( pPairingParams[connectionHandle]->pEncParams )
                {
                    VOID osal_memset( pPairingParams[connectionHandle]->pEncParams, 0, sizeof (smSecurityInfo_t ) );
                }
            }

            if ( pPairingParams[connectionHandle]->pEncParams )
            {
                smSecurityInfo_t* pEnc = pPairingParams[connectionHandle]->pEncParams;

                // Default the key size to the key size of this encryption session.
                if ( pEnc->keySize == 0 )
                {
                    pEnc->keySize = smDetermineKeySize( connectionHandle);
                }

                // For now, temp random the LTK, EDIV and RAND
                VOID osal_memset( pEnc->ltk, 0, KEYLEN );
                smGenerateRandBuf( pEnc->ltk, pPairingParams[connectionHandle]->pEncParams->keySize );
                pEnc->div = osal_rand();
                smGenerateRandBuf( pEnc->rand, B_RANDOM_NUM_SIZE );
                // Send the Encryption Info
                smPairingSendEncInfo( pPairingParams[connectionHandle]->connectionHandle, pEnc->ltk );
            }

            break;

        case SM_PAIRING_STATE_WAIT_MASTER_MASTER_INFO:
            if ( pPairingParams[connectionHandle]->pEncParams )
            {
                smPairingSendMasterID( pPairingParams[connectionHandle]->connectionHandle,
                                       pPairingParams[connectionHandle]->pEncParams->div,
                                       pPairingParams[connectionHandle]->pEncParams->rand );
            }

            break;

        case SM_PAIRING_STATE_WAIT_MASTER_IDENTITY_INFO:
            smPairingSendIdentityInfo( pPairingParams[connectionHandle]->connectionHandle, gapGetIRK() );
            break;

        case SM_PAIRING_STATE_WAIT_MASTER_IDENTITY_ADDR_INFO:
        {
            uint8 getRealAddr = TRUE;
            uint8 addrType = gapGetDevAddressMode();

            if ( addrType == ADDRTYPE_STATIC )
            {
                getRealAddr = FALSE;
            }
            else
            {
                addrType = ADDRTYPE_PUBLIC;
            }

            smPairingSendIdentityAddrInfo( pPairingParams[connectionHandle]->connectionHandle,
                                           addrType, gapGetDevAddress( getRealAddr ) );
        }
        break;

        case SM_PAIRING_STATE_WAIT_MASTER_SIGNING_INFO:
            smPairingSendSingingInfo( pPairingParams[connectionHandle]->connectionHandle, gapGetSRK() );
            break;

        default:
            break;
        }

        // Determine the next state
        if ( state == SM_PAIRING_STATE_WAIT_MASTER_ENCRYPTION_INFO )
        {
            pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_MASTER_MASTER_INFO;
        }
        else if ( state == SM_PAIRING_STATE_WAIT_MASTER_MASTER_INFO )
        {
            if ( (pPairReq->keyDist.mIdKey) && (pSecReq->keyDist.mIdKey) )
            {
                pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_MASTER_IDENTITY_INFO;
            }
            else if ((pPairReq->keyDist.mSign) && (pSecReq->keyDist.mSign) )
            {
                pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_MASTER_SIGNING_INFO;
            }
            else
            {
                pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_DONE;
            }
        }
        else if ( state == SM_PAIRING_STATE_WAIT_MASTER_IDENTITY_INFO )
        {
            pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_MASTER_IDENTITY_ADDR_INFO;
        }
        else if ( state == SM_PAIRING_STATE_WAIT_MASTER_IDENTITY_ADDR_INFO )
        {
            if ( (pPairReq->keyDist.mSign) && (pSecReq->keyDist.mSign) )
            {
                pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_MASTER_SIGNING_INFO;
            }
            else
            {
                pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_DONE;
            }
        }
        else
        {
            pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_DONE;
        }

        if ( (pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_MASTER_ENCRYPTION_INFO)
                || (pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_MASTER_MASTER_INFO)
                || (pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_MASTER_IDENTITY_INFO)
                || (pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_MASTER_IDENTITY_ADDR_INFO)
                || (pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_MASTER_SIGNING_INFO) )
        {
            linkDBItem_t* linkItem;
            uint32 timeout;
            linkItem = linkDB_Find( pPairingParams[connectionHandle]->connectionHandle );

            if ( linkItem != NULL )
            {
                // Make the timeout 1.5 * connInterval (connInterval = 1.25 ms)
                timeout = linkItem->connInterval;
                timeout += linkItem->connInterval / 2;
            }
            else
            {
                timeout = SM_PAIRING_STATE_WAIT;
            }

            // Set up the next send
            smState_CBTimer[connectionHandle] = connectionHandle;
            osal_CbTimerStart( smState_timerCB, &smState_CBTimer[connectionHandle],timeout, &pPairingParams[connectionHandle]->stateID );
        }
    }
}



/*********************************************************************
*********************************************************************/
