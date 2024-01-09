/*************************************************************************************************
    Filename:       sm_rsppairing.c
    Revised:
    Revision:

    Description:    This file contains the SM Responder Pairing Manager.

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
uint8 smState_CBTimer[15]= {0};


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
static uint8 smpResponderProcessIncoming( linkDBItem_t* pLinkItem, uint8 cmdID, smpMsgs_t* pParsedMsg );
static uint8 smpResponderProcessPairingConfirm( uint16 connectionHandle,smpPairingConfirm_t* pParsedMsg );
static uint8 smpResponderProcessPairingRandom( uint16 connectionHandle,smpPairingRandom_t* pParsedMsg );
static uint8 smpResponderProcessEncryptionInformation( uint16 connectionHandle,smpEncInfo_t* pParsedMsg );
static uint8 smpResponderProcessMasterID( uint16 connectionHandle,smpMasterID_t* pParsedMsg );
static uint8 smpResponderProcessIdentityInfo( uint16 connectionHandle,smpIdentityInfo_t* pParsedMsg );
static uint8 smpResponderProcessIdentityAddrInfo( uint16 connectionHandle,smpIdentityAddrInfo_t* pParsedMsg );
static uint8 smpResponderProcessSigningInfo( uint16 connectionHandle,smpSigningInfo_t* pParsedMsg );

static void smResponderSendNextKeyInfo( uint16 connectionHandle );
static uint8 smResponderProcessLTKReq( uint16 connectionHandle, uint8* pRandom, uint16 encDiv );

/*********************************************************************
    RESPONDER CALLBACKS
*/

// SM Responder Callbacks
static smResponderCBs_t smResponderCBs =
{
    smpResponderProcessIncoming, // Process SMP Message Callback
    smResponderSendNextKeyInfo,  // Send Next Key Message Callback
    smResponderProcessLTKReq     // HCI BLE LTK Request Callback
};

/*********************************************************************
    API FUNCTIONS
*/

/*********************************************************************
    FUNCTIONS - SLAVE API - Only use these in a slave device
*/

/*********************************************************************
    Initialize SM Responder on a slave device.

    Public function defined in sm.h.
*/
bStatus_t SM_ResponderInit( void )
{
    if ( gapProfileRole & GAP_PROFILE_PERIPHERAL )
    {
        // Set up Responder's processing function
        smRegisterResponder( &smResponderCBs );
    }
    else
    {
        smRegisterResponder( NULL );
    }

    return ( SUCCESS );
}

/*********************************************************************
    @fn          smResponderProcessLTKReq

    @brief       Process the HCI BLE LTK Request Event on Responder.

    @param       connectionHandle - link ID
    @param       pRandom - 8 byte random number
    @param       encDiv - encryption diversifier

    @return      TRUE - We are always expecting this message
*/
static uint8 smResponderProcessLTKReq( uint16 connectionHandle, uint8* pRandom, uint16 encDiv )
{
    uint16 connHandle = 0;      // Found connection handle
    uint8  cmdLtk[KEYLEN];      // Place to hold the LTK or STK
    bStatus_t stat = FAILURE;   // Assume failure until verified
    // Clear the LTK
    VOID osal_memset( cmdLtk, 0, KEYLEN );

    // Check the parameters
    if ( (pPairingParams[connectionHandle])
            && (pPairingParams[connectionHandle]->connectionHandle == connectionHandle)
            && (pPairingParams[connectionHandle]->initiator == FALSE) )
    {
        // Do we want the STK?
        if ( pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_STK )
        {
            uint8 ltk[KEYLEN];  // A place to build the LTK

            // Received Responder, generate STK
            if ( sm_s1( pPairingParams[connectionHandle]->tk, pPairingParams[connectionHandle]->myComp.rand,
                        pPairingParams[connectionHandle]->devComp.rand, ltk ) == SUCCESS )
            {
                // Copy the STK to the LTK space
                VOID osal_memcpy( cmdLtk, ltk, smDetermineKeySize( connectionHandle) );
                connHandle = connectionHandle;
                stat = SUCCESS;
            }
        }
        // How about the LTK?
        else if ( (pPairingParams[connectionHandle]->pEncParams)
                  && (pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_ENCRYPT) )
        {
            // Check DIV and Rand first
            if ( (encDiv == pPairingParams[connectionHandle]->pEncParams->div)
                    && (osal_memcmp( pRandom, pPairingParams[connectionHandle]->pEncParams->rand, B_RANDOM_NUM_SIZE ) == TRUE) )
            {
                // Must be the LTK encryption
                VOID osal_memcpy( cmdLtk, pPairingParams[connectionHandle]->pEncParams->ltk, smDetermineKeySize(connectionHandle) );
                connHandle = connectionHandle;
                stat = SUCCESS;
            }
            else
            {
                // Rejected, end bonding
                smEndPairing( connectionHandle, bleGAPBondRejected );
            }
        }
        else
        {
            // Not expecting but, if it's available - load the key
            if ( (pPairingParams[connectionHandle]->pEncParams)
                    && (encDiv == pPairingParams[connectionHandle]->pEncParams->div)
                    && (osal_memcmp( pRandom, pPairingParams[connectionHandle]->pEncParams->rand, B_RANDOM_NUM_SIZE ) == TRUE) )
            {
                VOID osal_memcpy( cmdLtk, pPairingParams[connectionHandle]->pEncParams->ltk, pPairingParams[connectionHandle]->pEncParams->keySize );
                connHandle = connectionHandle;
                stat = SUCCESS;
            }
        }
    }

    // Not directly handled, but make best attempt to load the key
    if ( stat != SUCCESS )
    {
        linkDBItem_t* pLinkItem;  // connection information
        // Find the connection
        pLinkItem = linkDB_Find( connectionHandle );

        if ( pLinkItem && (pLinkItem->pEncParams)
                && (encDiv == pLinkItem->pEncParams->div)
                && (osal_memcmp( pRandom, pLinkItem->pEncParams->rand, B_RANDOM_NUM_SIZE ) == TRUE) )
        {
            // Use the key in the connection information
            VOID osal_memcpy( cmdLtk, pLinkItem->pEncParams->ltk, pLinkItem->pEncParams->keySize );
            connHandle = connectionHandle;
            stat = SUCCESS;
        }
    }

    if ( stat == SUCCESS )
    {
        HCI_LE_LtkReqReplyCmd( connHandle, cmdLtk );
    }
    else
    {
        HCI_LE_LtkReqNegReplyCmd( connectionHandle );
    }

    return ( TRUE );  // We are always expecting this message
}

/*********************************************************************
    @fn          smpResponderProcessIncoming

    @brief       Process incoming parsed SM Responder message.

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
static uint8 smpResponderProcessIncoming( linkDBItem_t* pLinkItem, uint8 cmdID, smpMsgs_t* pParsedMsg )
{
    uint8 reason = SUCCESS;   // return value

    // check for pairing mode
    if ( pPairingParams[pLinkItem->connectionHandle] == NULL )
    {
        if ( cmdID == SMP_PAIRING_REQ )
        {
            smpPairingReq_t* pairingReq = (smpPairingReq_t*)pParsedMsg;
            // Notify the app
            gapSendPairingReqEvent( SUCCESS, pLinkItem->connectionHandle,
                                    pairingReq->ioCapability,
                                    pairingReq->oobDataFlag,
                                    smAuthReqToUint8( &(pairingReq->authReq) ),
                                    pairingReq->maxEncKeySize,
                                    pairingReq->keyDist );
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

    if ( pPairingParams[pLinkItem->connectionHandle]->initiator == TRUE )
    {
        return ( SMP_PAIRING_FAILED_CMD_NOT_SUPPORTED );
    }

    // Process the pairing messages
    switch ( cmdID )
    {
    case SMP_PAIRING_REQ:
        reason = smpResponderProcessPairingReq( pLinkItem->connectionHandle,(smpPairingReq_t*)pParsedMsg );
        break;

    case SMP_PAIRING_CONFIRM:
        reason = smpResponderProcessPairingConfirm( pLinkItem->connectionHandle,(smpPairingConfirm_t*)pParsedMsg );
        break;

    case SMP_PAIRING_RANDOM:
        reason = smpResponderProcessPairingRandom( pLinkItem->connectionHandle,(smpPairingRandom_t*)pParsedMsg );
        break;

    case SMP_ENCRYPTION_INFORMATION:
        reason = smpResponderProcessEncryptionInformation( pLinkItem->connectionHandle,(smpEncInfo_t*)pParsedMsg );
        break;

    case SMP_MASTER_IDENTIFICATION:
        reason = smpResponderProcessMasterID( pLinkItem->connectionHandle,(smpMasterID_t*)pParsedMsg );
        break;

    case SMP_IDENTITY_INFORMATION:
        reason = smpResponderProcessIdentityInfo( pLinkItem->connectionHandle,(smpIdentityInfo_t*)pParsedMsg );
        break;

    case SMP_IDENTITY_ADDR_INFORMATION:
        reason = smpResponderProcessIdentityAddrInfo( pLinkItem->connectionHandle,(smpIdentityAddrInfo_t*)pParsedMsg );
        break;

    case SMP_SIGNING_INFORMATION:
        reason = smpResponderProcessSigningInfo( pLinkItem->connectionHandle,(smpSigningInfo_t*)pParsedMsg );
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
    @fn          smpResponderProcessPairingReq

    @brief       Process incoming parsed Pairing Request.

    @param       pParsedMsg - pointer to parsed message

    @return      SUCCESS
                SMP_PAIRING_FAILED_CMD_NOT_SUPPORTED
                SMP_PAIRING_FAILED_UNSPECIFIED
                SMP_PAIRING_FAILED_AUTH_REQ
*/
uint8 smpResponderProcessPairingReq( uint16 connectionHandle,smpPairingReq_t* pParsedMsg )
{
    uint8 reason = SUCCESS; // return value
    bStatus_t stat; // status field
    // Save the pairing request information into pPairingParams[connectionHandle]
    stat = smSavePairInfo( connectionHandle,pParsedMsg );

    if ( stat == SUCCESS )
    {
        // Send the response
        smGeneratePairingReqRsp(connectionHandle);

        // Check if both sides have bonding
        if ( (pPairingParams[connectionHandle]->pSecReqs->authReq & SM_AUTH_STATE_BONDING)
                && (pPairingParams[connectionHandle]->pPairDev->authReq.bonding == SM_AUTH_REQ_BONDING) )
        {
            pPairingParams[connectionHandle]->authState |= SM_AUTH_STATE_BONDING;
        }

        // Do we need a passkey?
        if ( (pPairingParams[connectionHandle]->type == SM_PAIRING_TYPE_PASSKEY_INITIATOR_INPUTS)
                || (pPairingParams[connectionHandle]->type == SM_PAIRING_TYPE_PASSKEY_RESPONDER_INPUTS)
                || (pPairingParams[connectionHandle]->type == SM_PAIRING_TYPE_PASSKEY_BOTH_INPUTS) )
        {
            uint8 type;

            // Determine the passkey input/output requirements
            if ( pPairingParams[connectionHandle]->type == SM_PAIRING_TYPE_PASSKEY_INITIATOR_INPUTS )
            {
                type = SM_PASSKEY_TYPE_DISPLAY;
            }
            else
            {
                type = SM_PASSKEY_TYPE_INPUT;
            }

            // Ask the app/profile for passkey
            gapPasskeyNeededCB( pPairingParams[connectionHandle]->connectionHandle, type );
            pPairingParams[connectionHandle]->authState |= SM_AUTH_STATE_AUTHENTICATED;
            pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_CONFIRM_PASSKEY;
        }
        else if ( (pPairingParams[connectionHandle]->type == SM_PAIRING_TYPE_JUST_WORKS)
                  || (pPairingParams[connectionHandle]->type == SM_PAIRING_TYPE_OOB) )
        {
            // Get ready for the Confirm
            pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_CONFIRM;

            // Setup TK
            if ( pPairingParams[connectionHandle]->type == SM_PAIRING_TYPE_JUST_WORKS )
            {
                // Just Wait
                VOID osal_memset( pPairingParams[connectionHandle]->tk, 0, KEYLEN );
            }
            else
            {
                // OOB
                VOID osal_memcpy( pPairingParams[connectionHandle]->tk, pPairingParams[connectionHandle]->pSecReqs->oob, KEYLEN );
                pPairingParams[connectionHandle]->authState |= SM_AUTH_STATE_AUTHENTICATED;
            }

            // Generate Rand (SRand)
            smGenerateRandBuf( pPairingParams[connectionHandle]->myComp.rand, SMP_RANDOM_LEN );
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
    @fn          smpResponderProcessPairingConfirm

    @brief       Process incoming parsed Pairing Confirm.

    @param       pParsedMsg - pointer to parsed message

    @return      SUCCESS
                SMP_PAIRING_FAILED_UNSPECIFIED
*/
static uint8 smpResponderProcessPairingConfirm( uint16 connectionHandle,smpPairingConfirm_t* pParsedMsg )
{
    uint8 reason = SUCCESS;  // return value
    VOID osal_memcpy( pPairingParams[connectionHandle]->devComp.confirm, pParsedMsg->confirmValue, KEYLEN );

    // Received Initiator Confirm
    if ( pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_CONFIRM_PASSKEY )
    {
        pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_PASSKEY;
    }
    else
    {
        // Generate Confirm (MConfirm)
        sm_c1(connectionHandle, pPairingParams[connectionHandle]->tk,
              pPairingParams[connectionHandle]->myComp.rand,
              pPairingParams[connectionHandle]->myComp.confirm );
        #if defined ( TESTMODES )

        if ( GAP_GetParamValue( TGAP_SM_TESTCODE ) == SM_TESTMODE_SEND_BAD_CONFIRM )
        {
            VOID osal_memset( pPairingParams[connectionHandle]->myComp.confirm, 0, KEYLEN );
        }

        #endif // TESTMODE

        // Send our own confirm
        if ( smGenerateConfirm( connectionHandle) != SUCCESS )
        {
            reason = SMP_PAIRING_FAILED_UNSPECIFIED;
        }

        pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_RANDOM;
    }

    return ( reason );
}

/*********************************************************************
    @fn          smpResponderProcessPairingRandom

    @brief       Process incoming parsed Pairing Random.

    @param       parsedMsg - pointer to parsed message

    @return      SUCCESS
                SMP_PAIRING_FAILED_CONFIRM_VALUE
                SMP_PAIRING_FAILED_UNSPECIFIED
*/
static uint8 smpResponderProcessPairingRandom( uint16 connectionHandle,smpPairingRandom_t* pParsedMsg )
{
    uint8 reason = SUCCESS;   // return value
    uint8 confirm[KEYLEN];    // working area to calculate a confirm value
    VOID osal_memcpy( pPairingParams[connectionHandle]->devComp.rand, pParsedMsg->randomValue, SMP_RANDOM_LEN );
    // Check device's Confirm value
    sm_c1( connectionHandle,pPairingParams[connectionHandle]->tk,
           pPairingParams[connectionHandle]->devComp.rand,
           confirm );

    // Make sure that the calculated confirm matches the confirm from the other device
    if ( osal_memcmp( confirm, pPairingParams[connectionHandle]->devComp.confirm, KEYLEN ) == TRUE )
    {
        // Received Initiator's Random, so send our own Random
        if ( smGenerateRandMsg( connectionHandle) != SUCCESS )
        {
            reason = SMP_PAIRING_FAILED_UNSPECIFIED;
        }
        else
        {
            pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_STK;
        }
    }
    else
    {
        reason = SMP_PAIRING_FAILED_CONFIRM_VALUE;
    }

    return ( reason );
}

/*********************************************************************
    @fn          smpResponderProcessEncryptionInformation

    @brief       Process incoming parsed Encryption Information.

    @param       pParsedMsg - pointer to parsed message

    @return      SUCCESS
                SMP_PAIRING_FAILED_UNSPECIFIED
*/
static uint8 smpResponderProcessEncryptionInformation( uint16 connectionHandle,smpEncInfo_t* pParsedMsg )
{
    if ( pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_MASTER_ENCRYPTION_INFO )
    {
        // Save off the connected device's encryption information (LTK and key size)
        if ( pPairingParams[connectionHandle]->pDevEncParams == NULL )
        {
            pPairingParams[connectionHandle]->pDevEncParams = (smSecurityInfo_t*)osal_mem_alloc( (uint16)sizeof (smSecurityInfo_t ) );
        }

        if ( pPairingParams[connectionHandle]->pDevEncParams )
        {
            VOID osal_memcpy( pPairingParams[connectionHandle]->pDevEncParams->ltk, pParsedMsg->ltk, KEYLEN );
            pPairingParams[connectionHandle]->pDevEncParams->keySize = smDetermineKeySize(connectionHandle);
            pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_MASTER_MASTER_INFO;
            return ( SUCCESS );
        }
    }

    return ( SMP_PAIRING_FAILED_CMD_NOT_SUPPORTED );
}

/*********************************************************************
    @fn          smpResponderProcessMasterID

    @brief       Process incoming parsed Master Identification.

    @param       parsedMsg - pointer to parsed message

    @return      SUCCESS
                SMP_PAIRING_FAILED_UNSPECIFIED
*/
static uint8 smpResponderProcessMasterID( uint16 connectionHandle,smpMasterID_t* pParsedMsg )
{
    if ( (pPairingParams[connectionHandle]->pDevEncParams != NULL)
            && (pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_MASTER_MASTER_INFO) )
    {
        // Save off the rest of the connected device's encryption information
        pPairingParams[connectionHandle]->pDevEncParams->div = pParsedMsg->ediv;
        VOID osal_memcpy( pPairingParams[connectionHandle]->pDevEncParams->rand, pParsedMsg->rand, B_RANDOM_NUM_SIZE );

        // Setup the next state
        if ( (pPairingParams[connectionHandle]->pSecReqs->keyDist.mIdKey) && (pPairingParams[connectionHandle]->pPairDev->keyDist.mIdKey) )
        {
            pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_MASTER_IDENTITY_INFO;
        }
        else if ( (pPairingParams[connectionHandle]->pSecReqs->keyDist.mSign) && (pPairingParams[connectionHandle]->pPairDev->keyDist.mSign) )
        {
            pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_MASTER_SIGNING_INFO;
        }
        else
        {
            // Exit pairing
            smEndPairing( connectionHandle,SUCCESS );
        }

        return ( SUCCESS );
    }
    else
    {
        return (  SMP_PAIRING_FAILED_CMD_NOT_SUPPORTED );
    }
}

/*********************************************************************
    @fn          smpResponderProcessIdentityInfo

    @brief       Process incoming parsed Identity Information.

    @param       parsedMsg - pointer to parsed message

    @return      SUCCESS
                SMP_PAIRING_FAILED_UNSPECIFIED
*/
static uint8 smpResponderProcessIdentityInfo( uint16 connectionHandle,smpIdentityInfo_t* pParsedMsg )
{
    if ( pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_MASTER_IDENTITY_INFO )
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
        pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_MASTER_IDENTITY_ADDR_INFO;
        return ( SUCCESS );
    }
    else
    {
        return ( SMP_PAIRING_FAILED_CMD_NOT_SUPPORTED );
    }
}

/*********************************************************************
    @fn          smpResponderProcessIdentityAddrInfo

    @brief       Process incoming parsed Identity Address Information.

    @param       pParsedMsg - pointer to parsed message

    @return      SUCCESS
                SMP_PAIRING_FAILED_UNSPECIFIED
*/
static uint8 smpResponderProcessIdentityAddrInfo( uint16 connectionHandle,smpIdentityAddrInfo_t* pParsedMsg )
{
    if ( pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_MASTER_IDENTITY_ADDR_INFO )
    {
        VOID osal_memcpy( pPairingParams[connectionHandle]->pIdInfo->bd_addr, pParsedMsg->bdAddr, B_ADDR_LEN );

        if ( (pPairingParams[connectionHandle]->pSecReqs->keyDist.mSign) && (pPairingParams[connectionHandle]->pPairDev->keyDist.mSign) )
        {
            pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_MASTER_SIGNING_INFO;
        }
        else
        {
            // All done
            smEndPairing( connectionHandle,SUCCESS );
        }

        return ( SUCCESS );
    }
    else
    {
        return ( SMP_PAIRING_FAILED_CMD_NOT_SUPPORTED );
    }
}

/*********************************************************************
    @fn          smpResponderProcessSigningInfo

    @brief       Process incoming parsed Signing Information.

    @param       parsedMsg - pointer to parsed message

    @return      SUCCESS
                SMP_PAIRING_FAILED_UNSPECIFIED
*/
static uint8 smpResponderProcessSigningInfo( uint16 connectionHandle,smpSigningInfo_t* pParsedMsg )
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

        // All done
        smEndPairing( connectionHandle,SUCCESS );
        return ( SUCCESS );
    }
    else
    {
        return ( SMP_PAIRING_FAILED_CMD_NOT_SUPPORTED );
    }
}

/*********************************************************************
    @fn          smResponderSendNextKeyInfo

    @brief       Responder role: sends next key message, and sets state
                for next event.

    @param       none

    @return      none
*/
static void smResponderSendNextKeyInfo( uint16 connectionHandle )
{
    if ( pPairingParams[connectionHandle]->initiator == FALSE )
    {
        smLinkSecurityReq_t* pSecReq = pPairingParams[connectionHandle]->pSecReqs;
        smpPairingReq_t* pPairReq = pPairingParams[connectionHandle]->pPairDev;
        uint8 state = pPairingParams[connectionHandle]->state;

        // Determine key to send
        if ( state == SM_PAIRING_STATE_WAIT_STK )
        {
            if ( (pSecReq->keyDist.sEncKey) && (pPairReq->keyDist.sEncKey) )
            {
                state = SM_PAIRING_STATE_WAIT_SLAVE_ENCRYPTION_INFO;
            }
            else if ( (pSecReq->keyDist.sIdKey) && (pPairReq->keyDist.sIdKey) )
            {
                state = SM_PAIRING_STATE_WAIT_SLAVE_IDENTITY_INFO;
            }
            else if ( (pSecReq->keyDist.sSign) && (pPairReq->keyDist.sSign) )
            {
                state = SM_PAIRING_STATE_WAIT_SLAVE_SIGNING_INFO;
            }
        }

        // Send the correct message
        switch ( state )
        {
        case SM_PAIRING_STATE_WAIT_SLAVE_ENCRYPTION_INFO:

            // STK is setup, so send Encryption Information.
            if ( pPairingParams[connectionHandle]->pEncParams == NULL )
            {
                pPairingParams[connectionHandle]->pEncParams = (smSecurityInfo_t*)osal_mem_alloc( (uint16)sizeof (smSecurityInfo_t ) );
            }

            if ( pPairingParams[connectionHandle]->pEncParams )
            {
                smSecurityInfo_t* pEnc = pPairingParams[connectionHandle]->pEncParams;
                // For now, temp random the LTK, EDIV and RAND
                VOID osal_memset( pEnc->ltk, 0, KEYLEN );
                smGenerateRandBuf( pEnc->ltk, smDetermineKeySize(connectionHandle) );
                pEnc->div = osal_rand();
                smGenerateRandBuf( pEnc->rand, B_RANDOM_NUM_SIZE );
                pEnc->keySize = smDetermineKeySize(connectionHandle);
                // Send the Encryption Info
                smPairingSendEncInfo( pPairingParams[connectionHandle]->connectionHandle, pEnc->ltk );
            }

            break;

        case SM_PAIRING_STATE_WAIT_SLAVE_MASTER_INFO:
            if ( pPairingParams[connectionHandle]->pEncParams )
            {
                smPairingSendMasterID( pPairingParams[connectionHandle]->connectionHandle,
                                       pPairingParams[connectionHandle]->pEncParams->div,
                                       pPairingParams[connectionHandle]->pEncParams->rand );
            }

            break;

        case SM_PAIRING_STATE_WAIT_SLAVE_IDENTITY_INFO:
            smPairingSendIdentityInfo( pPairingParams[connectionHandle]->connectionHandle, gapGetIRK() );
            break;

        case SM_PAIRING_STATE_WAIT_SLAVE_IDENTITY_ADDR_INFO:
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

        case SM_PAIRING_STATE_WAIT_SLAVE_SIGNING_INFO:
            smPairingSendSingingInfo( pPairingParams[connectionHandle]->connectionHandle, gapGetSRK() );
            break;

        default:
            break;
        }

        // Determine the state
        if ( state == SM_PAIRING_STATE_WAIT_SLAVE_ENCRYPTION_INFO )
        {
            pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_SLAVE_MASTER_INFO;
        }
        else if ( state == SM_PAIRING_STATE_WAIT_SLAVE_MASTER_INFO )
        {
            if ( (pSecReq->keyDist.sIdKey) && (pPairReq->keyDist.sIdKey) )
            {
                pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_SLAVE_IDENTITY_INFO;
            }
            else if ( (pSecReq->keyDist.sSign) && (pPairReq->keyDist.sSign) )
            {
                pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_SLAVE_SIGNING_INFO;
            }
            else if ( (pSecReq->keyDist.mEncKey) && (pPairReq->keyDist.mEncKey) )
            {
                pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_MASTER_ENCRYPTION_INFO;
            }
            else if ( (pSecReq->keyDist.mIdKey) && (pPairReq->keyDist.mIdKey) )
            {
                pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_MASTER_IDENTITY_INFO;
            }
            else if ( (pSecReq->keyDist.mSign) && (pPairReq->keyDist.mSign) )
            {
                pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_MASTER_SIGNING_INFO;
            }
            else
            {
                pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_DONE;
            }
        }
        else if ( state == SM_PAIRING_STATE_WAIT_SLAVE_IDENTITY_INFO )
        {
            pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_SLAVE_IDENTITY_ADDR_INFO;
        }
        else if ( state == SM_PAIRING_STATE_WAIT_SLAVE_IDENTITY_ADDR_INFO )
        {
            if ( (pSecReq->keyDist.sSign) && (pPairReq->keyDist.sSign)  )
            {
                pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_SLAVE_SIGNING_INFO;
            }
            else if ( (pSecReq->keyDist.mEncKey) && (pPairReq->keyDist.mEncKey) )
            {
                pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_MASTER_ENCRYPTION_INFO;
            }
            else if ( (pSecReq->keyDist.mIdKey) && (pPairReq->keyDist.mIdKey) )
            {
                pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_MASTER_IDENTITY_INFO;
            }
            else if ( (pSecReq->keyDist.mSign) && (pPairReq->keyDist.mSign) )
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
            if ( (pSecReq->keyDist.mEncKey) && (pPairReq->keyDist.mEncKey) )
            {
                pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_MASTER_ENCRYPTION_INFO;
            }
            else if ( (pSecReq->keyDist.mIdKey) && (pPairReq->keyDist.mIdKey) )
            {
                pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_MASTER_IDENTITY_INFO;
            }
            else if ( (pSecReq->keyDist.mSign) && (pPairReq->keyDist.mSign) )
            {
                pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_MASTER_SIGNING_INFO;
            }
            else
            {
                pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_DONE;
            }
        }

        if ( (pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_SLAVE_ENCRYPTION_INFO)
                || (pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_SLAVE_MASTER_INFO)
                || (pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_SLAVE_IDENTITY_INFO)
                || (pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_SLAVE_IDENTITY_ADDR_INFO)
                || (pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_SLAVE_SIGNING_INFO) )
        {
            linkDBItem_t* pLinkItem;
            uint32 timeout;
            pLinkItem = linkDB_Find( pPairingParams[connectionHandle]->connectionHandle );

            if ( pLinkItem != NULL )
            {
                // Make the timeout 1.5 * connInterval (connInterval = 1.25 ms)
                timeout = pLinkItem->connInterval;
                timeout += pLinkItem->connInterval / 2;
            }
            else
            {
                timeout = SM_PAIRING_STATE_WAIT;
            }

            // Set up the next send
//            VOID osal_start_timerEx( smTaskID, SM_PAIRING_STATE_EVT, timeout );
            smState_CBTimer[connectionHandle] = connectionHandle;
            osal_CbTimerStart( smState_timerCB, &smState_CBTimer[connectionHandle],
                               timeout, &pPairingParams[connectionHandle]->stateID );
        }
    }
}



/*********************************************************************
*********************************************************************/
