/*************************************************************************************************
    Filename:       sm_pairing.c
    Revised:
    Revision:

    Description:    This file contains the SM Pairing Manager.

	SDK_LICENSE

**************************************************************************************************/

#include "bcomdef.h"
#include "l2cap.h"
#include "gap_internal.h"
#include "linkdb.h"
#include "sm.h"
#include "sm_internal.h"
#include "smp.h"

/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/

// Defines just for the IOCapMatrix table below
#define JUST_WORKS         SM_PAIRING_TYPE_JUST_WORKS
#define INIT_INP           SM_PAIRING_TYPE_PASSKEY_INITIATOR_INPUTS
#define RESP_INP           SM_PAIRING_TYPE_PASSKEY_RESPONDER_INPUTS
#define BOTH_INP           SM_PAIRING_TYPE_PASSKEY_BOTH_INPUTS

/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    GLOBAL VARIABLES
*/

// Pairing parameters, NULL while not pairing
#if MAX_NUM_LL_CONN==1
smPairingParams_t* pPairingParams[MAX_NUM_LL_CONN]= {NULL};
#elif MAX_NUM_LL_CONN==5
smPairingParams_t* pPairingParams[MAX_NUM_LL_CONN] = {NULL,NULL,NULL,NULL,NULL};
#else
smPairingParams_t* pPairingParams[MAX_NUM_LL_CONN] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
#endif

// Callback function pointers for Responder
smResponderCBs_t* pfnResponderCBs = NULL;

/*********************************************************************
    EXTERNAL VARIABLES
*/

/*********************************************************************
    EXTERNAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/

// Callback function pointers for Initiator
static const smInitiatorCBs_t* pfnInitiatorCBs = NULL;

// Determine the Pairing passkey requirement (from the spec):
//       IOCapMatrix[Responder's IOCapability][Initiator's IOCapability];
static CONST uint8 IOCapMatrix[5][5] =
{
    /*        Initiator:    DisplayOnly,  DisplayYesNo, KeyboardOnly, NoInputNoOutput,  KeyboardDisplay */
    /* Responder:      */
    /* DisplayOnly     */ { JUST_WORKS,   JUST_WORKS,   INIT_INP,     JUST_WORKS,       INIT_INP },
    /* DisplayYesNo    */ { JUST_WORKS,   JUST_WORKS,   INIT_INP,     JUST_WORKS,       INIT_INP },
    /* KeyboardOnly    */ { RESP_INP,     RESP_INP,     BOTH_INP,     JUST_WORKS,       RESP_INP },
    /* NoInputNoOutput */ { JUST_WORKS,   JUST_WORKS,   JUST_WORKS,   JUST_WORKS,       JUST_WORKS },
    /* KeyboardDisplay */ { RESP_INP,     RESP_INP,     INIT_INP,     JUST_WORKS,       RESP_INP }
};

/*********************************************************************
    LOCAL FUNCTIONS
*/
static uint8 smpProcessIncoming( uint16 connHandle, uint8 cmdID, smpMsgs_t* pParsedMsg );

static bStatus_t smDetermineIOCaps( uint16 connectionHandle,uint8 initiatorIO, uint8 responderIO );
static void smFreePairingParams( uint16 connectionHandle );
static void smSetPairingReqRsp( uint16 connectionHandle,smpPairingReq_t* pReq );


/*********************************************************************
    API FUNCTIONS
*/


/*********************************************************************
    FUNCTIONS - MASTER API - Only use these in a master device
*/


/*********************************************************************
    Start the pairing process with a slave device.

    Public function defined in sm.h.
*/
bStatus_t SM_StartPairing( uint8 initiator,
                           uint8 taskID,
                           uint16 connectionHandle,
                           smLinkSecurityReq_t* pSecReqs )
{
    bStatus_t ret = SUCCESS;    // Return value

    // Only one pairing at a time
    if ( pPairingParams[connectionHandle] )
    {
        return ( bleAlreadyInRequestedMode );
    }

    // Check parameters
    if ( (pSecReqs == NULL) || (pSecReqs->maxEncKeySize < GAP_GetParamValue( TGAP_SM_MIN_KEY_LEN ))
            || (pSecReqs->maxEncKeySize > GAP_GetParamValue( TGAP_SM_MAX_KEY_LEN )) )
    {
        return ( INVALIDPARAMETER );
    }

    pPairingParams[connectionHandle] = ( smPairingParams_t*)osal_mem_alloc( (uint16)sizeof ( smPairingParams_t ) );

    if ( pPairingParams[connectionHandle] == NULL )
    {
        return ( bleMemAllocError );
    }

    VOID osal_memset( pPairingParams[connectionHandle], 0, sizeof( smPairingParams_t ) );
    // Save parameters
    pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_INITIALIZE;
    pPairingParams[connectionHandle]->initiator = initiator;
    pPairingParams[connectionHandle]->taskID = taskID;
    pPairingParams[connectionHandle]->timerID = INVALID_TASK_ID;
    pPairingParams[connectionHandle]->connectionHandle = connectionHandle;
    pPairingParams[connectionHandle]->pSecReqs = pSecReqs;
    pPairingParams[connectionHandle]->pPairDev = NULL;
    pPairingParams[connectionHandle]->type = SM_PAIRING_TYPE_INIT;

    if ( initiator )
    {
        // Start the pairing process
        ret = smGeneratePairingReqRsp(connectionHandle);
    }

    pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_PAIRING_REQ_SENT;

    if ( ret != SUCCESS )
    {
        // Free the mem - not successful.
        smFreePairingParams( connectionHandle);
    }

    return ( ret );
}

/*********************************************************************
    Update the passkey for the pairing process.

    Public function defined in sm.h.
*/
bStatus_t SM_PasskeyUpdate( uint8* pPasskey, uint16 connectionHandle )
{
    bStatus_t ret = SUCCESS;    // return value
    uint8 sendConfirm = FALSE;  // flag to send pairing confirm message

    // Are we pairing?
    if ( pPairingParams[connectionHandle] == NULL )
    {
        return ( bleIncorrectMode );
    }

    // Are we expecting the passkey?
    if ( (pPairingParams[connectionHandle]->type != SM_PAIRING_TYPE_PASSKEY_INITIATOR_INPUTS)
            && (pPairingParams[connectionHandle]->type != SM_PAIRING_TYPE_PASSKEY_RESPONDER_INPUTS)
            && (pPairingParams[connectionHandle]->type != SM_PAIRING_TYPE_PASSKEY_BOTH_INPUTS) )
    {
        return ( bleIncorrectMode );
    }

    // Is this the correct connection?
    if ( pPairingParams[connectionHandle]->connectionHandle != connectionHandle )
    {
        return ( INVALIDPARAMETER );
    }

    // Copy the passkey (tk)
    VOID osal_memcpy( pPairingParams[connectionHandle]->tk, pPasskey, KEYLEN );
    // Generate Rand (MRand or SRand)
    smGenerateRandBuf( pPairingParams[connectionHandle]->myComp.rand, SMP_RANDOM_LEN );
    // Generate Confirm (MConfirm or SConfirm)
    sm_c1(connectionHandle, pPairingParams[connectionHandle]->tk,
          pPairingParams[connectionHandle]->myComp.rand,
          pPairingParams[connectionHandle]->myComp.confirm );

    // Set the correct state
    if ( (pPairingParams[connectionHandle]->initiator)
            && (pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_PASSKEY) )
    {
        // Send Confirm
        sendConfirm = TRUE;
        // Next, wait for Responder Confirm
        pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_CONFIRM;
    }
    else if ( (pPairingParams[connectionHandle]->initiator == 0)
              && (pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_CONFIRM_PASSKEY) )
    {
        pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_CONFIRM;
    }
    else if ( (pPairingParams[connectionHandle]->initiator == 0)
              && (pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_PASSKEY) )
    {
        // We have both confirm and passkey, send our own confirm
        sendConfirm = TRUE;
        // Next, wait for Pairing Random
        pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_RANDOM;
    }

    if ( sendConfirm )
    {
        #if defined ( TESTMODES )

        if ( GAP_GetParamValue( TGAP_SM_TESTCODE ) == SM_TESTMODE_SEND_BAD_CONFIRM )
        {
            VOID osal_memset( pPairingParams[connectionHandle]->myComp.confirm, 0, KEYLEN );
        }

        #endif // TESTMODE
        // Send the Pairing Confirm message
        ret = smGenerateConfirm( connectionHandle);
    }

    return ( ret );
}

/*********************************************************************
    PUBLIC INTERNAL SM FUNCTIONS
*/

/*********************************************************************
    @fn      smRegisterInitiator

    @brief   Register Initiator's processing function with SM task.

    @param   pfnCBs - pointer to Initiator's processing function

    @return  none
*/
void smRegisterInitiator( smInitiatorCBs_t* pfnCBs )
{
    pfnInitiatorCBs = pfnCBs;
}

/*********************************************************************
    @fn      smRegisterResponder

    @brief   Register Responder's processing function with SM task.

    @param   pfnCBs - pointer to Responder's processing function

    @return  none
*/
void smRegisterResponder( smResponderCBs_t* pfnCBs )
{
    pfnResponderCBs = pfnCBs;
}

/*********************************************************************
    @fn          smLinkCheck

    @brief       Callback for linkDB to indicate link changes.

    @param       connectionHandle - link connection handle
    @param       changeType - link connection handle
                                ex. LINKDB_STATUS_UPDATE_REMOVED

    @return      none
*/
void smLinkCheck( uint16 connectionHandle, uint8 changeType )
{
    if ( (pPairingParams[connectionHandle])
            && (pPairingParams[connectionHandle]->connectionHandle == connectionHandle)
            && (changeType == LINKDB_STATUS_UPDATE_REMOVED) )
    {
        // Connection is down, remove the pairing information
        smFreePairingParams( connectionHandle);
    }
}

/*********************************************************************
    @fn          smTimedOut

    @brief       Something didn't repond quickly enough.

    @param       none

    @return      none
*/
void smTimedOut( uint16 connectionHandle )
{
    smEndPairing( connectionHandle,bleTimeout );
}

/*********************************************************************
    @fn          smProcessDataMsg

    @brief       Process incoming L2CAP messages.

    @param       pMsg - pointer to message.

    @return      none
*/
void smProcessDataMsg( l2capDataEvent_t* pMsg )
{
    smpMsgs_t parsedMsg;        // Place to parse the message
    bStatus_t stat = SUCCESS;   // Return value
    smpPairingFailed_t failed;  // Pairing Failed message
    uint8 cmdID;                // Message command ID
    #if defined ( TESTMODES )

    if ( GAP_GetParamValue( TGAP_SM_TESTCODE ) == SM_TESTMODE_NO_RESPONSE )
    {
        // just ignore the messages
        return;
    }

    #endif // TESTMODE
    failed.reason = SUCCESS;  // Default to success
    // Parse the incoming message
    cmdID = *(pMsg->pkt.pPayload);

    switch ( cmdID )
    {
    case SMP_PAIRING_REQ:
    case SMP_PAIRING_RSP:
        stat = smpParsePairingReq( pMsg->pkt.pPayload, &(parsedMsg.pairingReq) );

        if ( stat == bleIncorrectMode )
        {
            failed.reason = SMP_PAIRING_FAILED_ENC_KEY_SIZE;
        }

        break;

    case SMP_PAIRING_CONFIRM:
        stat = smpParsePairingConfirm( pMsg->pkt.pPayload, &(parsedMsg.pairingConfirm) );
        break;

    case SMP_PAIRING_RANDOM:
        stat = smpParsePairingRandom( pMsg->pkt.pPayload, &(parsedMsg.pairingRandom) );
        break;

    case SMP_PAIRING_FAILED:
        stat = smpParsePairingFailed( pMsg->pkt.pPayload, &(parsedMsg.pairingFailed) );
        break;

    case SMP_ENCRYPTION_INFORMATION:
        stat = smpParseEncInfo( pMsg->pkt.pPayload, &(parsedMsg.encInfo) );
        break;

    case SMP_MASTER_IDENTIFICATION:
        stat = smpParseMasterID( pMsg->pkt.pPayload, &(parsedMsg.masterID) );
        break;

    case SMP_IDENTITY_INFORMATION:
        stat = smpParseIdentityInfo( pMsg->pkt.pPayload, &(parsedMsg.idInfo) );
        break;

    case SMP_IDENTITY_ADDR_INFORMATION:
        stat = smpParseIdentityAddrInfo( pMsg->pkt.pPayload, &(parsedMsg.idAddrInfo) );
        break;

    case SMP_SIGNING_INFORMATION:
        stat = smpParseSigningInfo( pMsg->pkt.pPayload, &(parsedMsg.signingInfo) );
        break;

    case SMP_SECURITY_REQUEST:
        stat = smpParseSecurityReq( pMsg->pkt.pPayload, &(parsedMsg.secReq) );
        break;

    default:
        stat = bleIncorrectMode;
        failed.reason = SMP_PAIRING_FAILED_CMD_NOT_SUPPORTED;
        break;
    }

    // Process the incoming message
    if ( stat == SUCCESS )
    {
        failed.reason = smpProcessIncoming( pMsg->connHandle, cmdID, &parsedMsg );
    }

    // Check for fail indication
    if ( failed.reason != SUCCESS )
    {
        // Send Pairing Failed message and close the pairing process
        smSendFailAndEnd( pMsg->connHandle, &failed );
    }
    else
    {
        // Reset the timeout
        smStartRspTimer(pMsg->connHandle);
    }
}

/*********************************************************************
    @fn          smSendFailAndEnd

    @brief       Send the pairing failed message and end existing pairing.

    @param       connHandle - link ID
    @param       pFailedMsg - Pairing Failed message

    @return      SUCCESS if pairing failed sent
                otherwise failure.
*/
bStatus_t smSendFailAndEnd( uint16 connHandle, smpPairingFailed_t* pFailedMsg )
{
    bStatus_t stat;   // return value
    // Send Pairing Failed message
    stat = smSendPairingFailed( connHandle, pFailedMsg );
    // Clean up after error, end pairing process
    smEndPairing( connHandle,pFailedMsg->reason );
    return ( stat );
}

/*********************************************************************
    @fn          smProcessEncryptChange

    @brief       Process the HCI BLE Encryption Change Event.

    @param       connectionHandle - link ID
    @param       reason - reason for change

    @return      TRUE - We are always expecting this message
*/
uint8 smProcessEncryptChange( uint16 connectionHandle, uint8 reason )
{
    uint8 sendBondEnd = FALSE;  // Assume not bonding

    // Check for the correct state and connection
    if ( (pPairingParams[connectionHandle])
            && (pPairingParams[connectionHandle]->connectionHandle == connectionHandle) )
    {
        if ( pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_STK )
        {
            if ( pPairingParams[connectionHandle]->initiator )
            {
                smLinkSecurityReq_t* pSecReq = pPairingParams[connectionHandle]->pSecReqs;
                smpPairingReq_t* pPairReq = pPairingParams[connectionHandle]->pPairDev;

                // Wait for key distribute messages from Responder
                if ( (pSecReq->keyDist.sEncKey) && (pPairReq->keyDist.sEncKey) )
                {
                    pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_SLAVE_ENCRYPTION_INFO;
                }
                else if ( (pSecReq->keyDist.sIdKey) && (pPairReq->keyDist.sIdKey) )
                {
                    pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_SLAVE_IDENTITY_INFO;
                }
                else if ( (pSecReq->keyDist.sSign) && (pPairReq->keyDist.sSign) )
                {
                    pPairingParams[connectionHandle]->state = SM_PAIRING_STATE_WAIT_SLAVE_SIGNING_INFO;
                }
                else
                {
                    // Send the next key distribution message to the responder
                    if ( pfnInitiatorCBs && pfnInitiatorCBs->pfnSendNextKeyInfo )
                    {
                        pfnInitiatorCBs->pfnSendNextKeyInfo(connectionHandle);
                    }
                }
            }
            else
            {
                // Send the next key distribution message
                if ( pfnResponderCBs && pfnResponderCBs->pfnSendNextKeyInfo )
                {
                    pfnResponderCBs->pfnSendNextKeyInfo(connectionHandle);
                }
            }

            // If we are done, end the pairing process
            if ( pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_DONE )
            {
                // Exit pairing
                smEndPairing( connectionHandle,SUCCESS );
            }
        }
        else if ( pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_WAIT_ENCRYPT )
        {
            // We are waiting for the encrypt complete to complete pairing
            if ( reason != SUCCESS )
            {
                // Generic bond error
                reason = bleGAPBondRejected;
            }

            smEndPairing( connectionHandle,reason );
        }
        else
        {
            // wasn't expecting
            sendBondEnd = TRUE;
        }
    }
    else
    {
        // loaded key
        sendBondEnd = TRUE;
    }

    if ( sendBondEnd == TRUE )
    {
        if ( reason == SUCCESS )
        {
            linkDBItem_t* pLink; // connection information
            pLink = linkDB_Find( connectionHandle );

            if ( pLink )
            {
                // Change the connections state to include Encrypted
                pLink->stateFlags |= LINK_ENCRYPTED;
            }
        }

        // Send the bond complete event to the app/profile
        gapSendBondCompleteEvent( reason, connectionHandle );
    }

    return ( TRUE );  // We are always expecting this message
}

/*********************************************************************
    @fn          smNextPairingState

    @brief       Do the next Pairing key distribution state.

    @param       none

    @return      none
*/
void smNextPairingState(uint16 connectionHandle )
{
    if ( pPairingParams[connectionHandle] )
    {
        if ( pPairingParams[connectionHandle]->initiator == FALSE )
        {
            // Send next responder key distribution message
            if ( pfnResponderCBs && pfnResponderCBs->pfnSendNextKeyInfo )
            {
                pfnResponderCBs->pfnSendNextKeyInfo(connectionHandle);
            }
        }
        else
        {
            // Send next initiator key distribution message
            if ( pfnInitiatorCBs && pfnInitiatorCBs->pfnSendNextKeyInfo )
            {
                pfnInitiatorCBs->pfnSendNextKeyInfo(connectionHandle);
            }
        }

        if ( pPairingParams[connectionHandle]->state == SM_PAIRING_STATE_DONE )
        {
            smEndPairing(connectionHandle,SUCCESS );
        }
    }
}

/*********************************************************************
    @fn          sm_c1

    @brief       SM Confirm value generation function c1

    NOTE:       This function can only be called during a connection
                and during the pairing process.

    @param       pK - key is 128 bits, LSByte first
    @param       pR - 128 bit, LSByte first
    @param       pC1 - pointer to 128 bit results space

    @return      status
*/
bStatus_t sm_c1(uint16 connectionHandle,uint8* pK, uint8* pR, uint8* pC1 )
{
    bStatus_t stat;                   // return value
    uint8 pres[SMP_PAIRING_RSP_LEN];  // Pairing Response message (raw)
    uint8 preq[SMP_PAIRING_REQ_LEN];  // Pairing Request message (raw)
    uint8 iat;                        // initiator address type
    uint8 rat;                        // responder address type
    uint8* pIA;                       // initiator address
    uint8* pRA;                       // responder address
    smpPairingReq_t pairingStruct;    // Pairing Reqeust struct
    smpPairingReq_t* pPairingReq;      // Pairing Reqeust
    smpPairingReq_t* pPairingRsp;      // Pairing Response
    linkDBItem_t* pLinkItem;           // Connection information

    // Make sure we are in the correct mode
    if ( pPairingParams[connectionHandle] == NULL )
    {
        return ( bleIncorrectMode );
    }

    // Find the connection
    pLinkItem = linkDB_Find( pPairingParams[connectionHandle]->connectionHandle );

    if ( pLinkItem == NULL )
    {
        return ( bleNotConnected );
    }

    // Setup pairing request/response parameters
    smSetPairingReqRsp( connectionHandle,&pairingStruct );

    if ( pPairingParams[connectionHandle]->initiator )
    {
        // Setup all of the variables for an initiator
        pPairingReq = &pairingStruct;
        pPairingRsp = pPairingParams[connectionHandle]->pPairDev;
        iat = gapGetDevAddressMode();
        pIA = gapGetDevAddress( FALSE );
        rat = pLinkItem->addrType;
        pRA = pLinkItem->addr;
    }
    else
    {
        // Setup all of the variables for a responder
        pPairingReq = pPairingParams[connectionHandle]->pPairDev;
        pPairingRsp = &pairingStruct;
        rat = gapGetDevAddressMode();
        pRA = gapGetDevAddress( FALSE );
        iat = pLinkItem->addrType;
        pIA = pLinkItem->addr;
    }

    // build the raw data for the Pairing Request and Pairing Response messages
    VOID smpBuildPairingRsp( pPairingRsp, pres );
    VOID smpBuildPairingReq( pPairingReq, preq );
    // Calculate the cl
    stat =  sm_c1new( pK, pR, pres, preq, iat, pIA, rat, pRA, pC1 );
    return ( stat );
}

/*********************************************************************
    @fn          smpProcessIncoming

    @brief       Process incoming parsed SM message.

    @param       connHandle - connection Handle
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
static uint8 smpProcessIncoming( uint16 connHandle, uint8 cmdID, smpMsgs_t* pParsedMsg )
{
    linkDBItem_t* pLinkItem; // connection information
    uint8 reason = SMP_PAIRING_FAILED_CMD_NOT_SUPPORTED; // return value
    // find the connection
    pLinkItem = linkDB_Find( connHandle );

    if ( pLinkItem == NULL )
    {
        return ( SMP_PAIRING_FAILED_UNSPECIFIED );
    }

    if( pLinkItem->role == LL_LINK_CONNECT_COMPLETE_MASTER )
    {
        if ( (gapProfileRole & GAP_PROFILE_CENTRAL) == GAP_PROFILE_CENTRAL )
        {
            //printf("gapProfileRole & GAP_PROFILE_CENTRAL \n");
            // Process Initiator pairing messages
            if ( pfnInitiatorCBs && pfnInitiatorCBs->pfnProcessMsg )
            {
                reason = pfnInitiatorCBs->pfnProcessMsg( pLinkItem, cmdID, pParsedMsg );
            }
        }
    }
    else if( pLinkItem->role == LL_LINK_CONNECT_COMPLETE_SLAVE )
    {
        //printf("gapProfileRole & GAP_PROFILE_PERIPHERAL \n");
        // Process Responder pairing messages
        if ( pfnResponderCBs && pfnResponderCBs->pfnProcessMsg )
        {
            reason = pfnResponderCBs->pfnProcessMsg( pLinkItem, cmdID, pParsedMsg );
        }
    }
    else
    {
        return ( SMP_PAIRING_FAILED_NOT_SUPPORTED );
    }

    return ( reason );
}

/*********************************************************************
    @fn          smProcessPairingReq

    @brief       Process Pairing Request.

    @param       pLinkItem - pointer to link item
    @param       pParsedMsg - pointer to request

    @return      void
*/
void smProcessPairingReq( linkDBItem_t* pLinkItem, gapPairingReq_t* pPairReq )
{
    if ( pfnResponderCBs && pfnResponderCBs->pfnProcessMsg )
    {
        uint8 reason;
        smpPairingReq_t pairingReq;
        pairingReq.ioCapability = pPairReq->ioCap;
        pairingReq.oobDataFlag = pPairReq->oobDataFlag;
        smUint8ToAuthReq( &(pairingReq.authReq), pPairReq->authReq );
        pairingReq.maxEncKeySize = pPairReq->maxEncKeySize;
        pairingReq.keyDist = pPairReq->keyDist;
        reason = pfnResponderCBs->pfnProcessMsg( pLinkItem, SMP_PAIRING_REQ,
                                                 (smpMsgs_t*)&pairingReq );

        if ( reason != SUCCESS )
        {
            smpPairingFailed_t failedMsg;
            failedMsg.reason = reason;
            VOID smSendFailAndEnd( pLinkItem->connectionHandle, &failedMsg );
        }
    }
}

/*********************************************************************
    @fn          smStartEncryption

    @brief       Perform Encrypt through HCI

    @param       connHandle - Connection Handle
    @param       pLTK - pointer to 16 byte lkt
    @param       div - div or ediv
    @param       pRandNum - pointer to 8 byte random number
    @param       keyLen - length of LTK (bytes)

    @return      SUCCESS
                INVALIDPARAMETER
                other from HCI/LL
*/
bStatus_t smStartEncryption( uint16 connHandle, uint8* pLTK, uint16 div,
                             uint8* pRandNum, uint8 keyLen )
{
    if ( pfnInitiatorCBs && pfnInitiatorCBs->pfnStartEncryption )
    {
        return ( pfnInitiatorCBs->pfnStartEncryption( connHandle, pLTK, div, pRandNum, keyLen ) );
    }

    return ( INVALIDPARAMETER );
}

/*********************************************************************
    @fn          smGeneratePairingReqRsp

    @brief       Generate a pairing req or response

    @param       none

    @return      SUCCESS, FAILURE
*/
bStatus_t smGeneratePairingReqRsp( uint16 connectionHandle )
{
    if ( pPairingParams[connectionHandle] )
    {
        smpPairingReq_t pairingReq; // Structure to build message
        // Setup pairing request/response parameters
        smSetPairingReqRsp( connectionHandle,&pairingReq );

        if ( pPairingParams[connectionHandle]->initiator )
        {
            // Send Request
            return ( smSendPairingReq( pPairingParams[connectionHandle]->connectionHandle, (smpMsgs_t*)(&pairingReq) ) );
        }
        else
        {
            // Send Response
            return ( smSendPairingRsp( pPairingParams[connectionHandle]->connectionHandle, (smpMsgs_t*)(&pairingReq) ) );
        }
    }
    else
    {
        return ( FAILURE );
    }
}

/*********************************************************************
    @fn          smGenerateConfirm

    @brief       Generate a Pairing Confirm

    @param       none

    @return      SUCCESS
*/
bStatus_t smGenerateConfirm( uint16 connectionHandle )
{
    smpPairingConfirm_t confirmMsg; // Parameters to build message
    // Copy the confirm
    VOID osal_memcpy( confirmMsg.confirmValue,
                      pPairingParams[connectionHandle]->myComp.confirm, SMP_CONFIRM_LEN );
    // Send confirm message
    return ( smSendPairingConfirm( pPairingParams[connectionHandle]->connectionHandle, &confirmMsg ) );
}

/*********************************************************************
    @fn          smGenerateRandMsg

    @brief       Generate a Pairing Random

    @param       none

    @return      SUCCESS
*/
bStatus_t smGenerateRandMsg( uint16 connectionHandle )
{
    smpPairingRandom_t randMsg;  // Parameters to build message
    // Build rand
    VOID osal_memcpy( randMsg.randomValue,
                      pPairingParams[connectionHandle]->myComp.rand, SMP_RANDOM_LEN );
    // Send Random message
    return ( smSendPairingRandom( pPairingParams[connectionHandle]->connectionHandle, &randMsg ) );
}

/*********************************************************************
    @fn          smSavePairInfo

    @brief       Save the Pairing Req or Rsp information

    @param       pPair - info to save

    @return      SUCCESS
                bleMemAllocError
                bleInvalidRange - auth reqs don't match
*/
bStatus_t smSavePairInfo( uint16 connectionHandle, smpPairingReq_t* pPair )
{
    bStatus_t ret = SUCCESS;
    // Allocate the pairing information
    pPairingParams[connectionHandle]->pPairDev = (smpPairingReq_t*)osal_mem_alloc( (uint16)sizeof( smpPairingReq_t ) );

    if ( pPairingParams[connectionHandle]->pPairDev )
    {
        smLinkSecurityReq_t* pSecReq = pPairingParams[connectionHandle]->pSecReqs;
        // Copy the pairing information into the pairingParam
        VOID osal_memcpy( pPairingParams[connectionHandle]->pPairDev, pPair, (unsigned int)sizeof ( smpPairingReq_t ) );

        if ( pPairingParams[connectionHandle]->initiator == FALSE  )
        {
            keyDist_t* pSecKey = &(pSecReq->keyDist);
            keyDist_t* pPairKey = &(pPairingParams[connectionHandle]->pPairDev->keyDist);
            // Responder: merge our key distribution plans with the initiator's
            pSecKey->sEncKey = ( (pSecKey->sEncKey == pPairKey->sEncKey) &&
                                 (pSecKey->sEncKey == TRUE) ) ? TRUE : FALSE;
            pSecKey->sIdKey =  ( (pSecKey->sIdKey == pPairKey->sIdKey) &&
                                 (pSecKey->sIdKey == TRUE) ) ? TRUE : FALSE;
            pSecKey->sSign =   ( (pSecKey->sSign == pPairKey->sSign) &&
                                 (pSecKey->sSign == TRUE) ) ? TRUE : FALSE;
            pSecKey->mEncKey = ( (pSecKey->mEncKey == pPairKey->mEncKey) &&
                                 (pSecKey->mEncKey == TRUE) ) ? TRUE : FALSE;
            pSecKey->mIdKey =  ( (pSecKey->mIdKey == pPairKey->mIdKey) &&
                                 (pSecKey->mIdKey == TRUE) ) ? TRUE : FALSE;
            pSecKey->mSign =   ( (pSecKey->mSign == pPairKey->mSign) &&
                                 (pSecKey->mSign == TRUE) ) ? TRUE : FALSE;
        }

        // Determine the pairing type
        if ( (pPair->oobDataFlag) && (pSecReq->oobAvailable) )
        {
            pPairingParams[connectionHandle]->type = SM_PAIRING_TYPE_OOB;
        }
        else if ( ((pPair->authReq.mitm) == 0) && ((pSecReq->authReq & SMP_AUTHREQ_MITM) == 0) )
        {
            // if either initiator and responder have no MITM
            pPairingParams[connectionHandle]->type = SM_PAIRING_TYPE_JUST_WORKS;
        }
        else if ( (pPair->authReq.mitm) || (pSecReq->authReq & SMP_AUTHREQ_MITM) )
        {
            uint8 initiatorIO;
            uint8 responderIO;

            if ( pPairingParams[connectionHandle]->initiator )
            {
                initiatorIO = pSecReq->ioCaps;
                responderIO = pPair->ioCapability;
            }
            else
            {
                responderIO = pSecReq->ioCaps;
                initiatorIO = pPair->ioCapability;
            }

            // Update the pairingParam type field
            ret = smDetermineIOCaps( connectionHandle,initiatorIO, responderIO );
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    else
    {
        ret = bleMemAllocError;
    }

    return ( ret );
}

/*********************************************************************
    @fn          smDetermineIOCaps

    @brief       Save the Pairing Req or Rsp information

    @param       initiatorIO - initiator's IO Capabilities
    @param       responderIO - responder's IO Capabilities

    NOTE: Also updated is pPairingParams[connectionHandle]->type

    @return      SUCCESS
                bleInvalidRange - IO capability out of range
*/
static bStatus_t smDetermineIOCaps( uint16 connectionHandle, uint8 initiatorIO, uint8 responderIO )
{
    if ( (initiatorIO > SMP_IO_CAP_KEYBOARD_DISPLAY) || (responderIO > SMP_IO_CAP_KEYBOARD_DISPLAY) )
    {
        return ( bleInvalidRange );
    }

    // From the Spec matrix determine the pairing type
    pPairingParams[connectionHandle]->type = IOCapMatrix[responderIO][initiatorIO];
    return ( SUCCESS );
}

/*********************************************************************
    @fn          smPairingSendEncInfo

    @brief       Send SM Encryption Information message

    @param       connHandle - connection handle
    @param       pLTK - pointer to LTK

    @return      SUCCESS
                INVALIDPARAMETER
                other from HCI/LL
*/
void smPairingSendEncInfo( uint16 connHandle, uint8* pLTK )
{
    // The smpEncInfo_t only has one field in it.
    VOID smSendEncInfo( connHandle, (smpEncInfo_t*)pLTK );
}

/*********************************************************************
    @fn          smPairingSendMasterID

    @brief       Send SM Master Identification message

    @param       connHandle - connection handle
    @param       ediv - enhanced div
    @param       pRand - pointer to 8 byte random number string

    @return      SUCCESS
                INVALIDPARAMETER
                other from HCI/LL
*/
void smPairingSendMasterID( uint16 connHandle, uint16 ediv, uint8* pRand )
{
    smpMasterID_t masterMsg;  // Message structure
    // Fill parameters
    masterMsg.ediv = ediv;
    VOID osal_memcpy( masterMsg.rand, pRand, B_RANDOM_NUM_SIZE );
    // Send Master ID message
    VOID smSendMasterID( connHandle, &masterMsg );
}

/*********************************************************************
    @fn          smPairingSendIdentityInfo

    @brief       Send SM Identity Information message

    @param       connHandle - connection handle
    @param       pIRK - pointer to IRK

    @return      SUCCESS
                INVALIDPARAMETER
                other from HCI/LL
*/
void smPairingSendIdentityInfo( uint16 connHandle, uint8* pIRK )
{
    VOID smSendIdentityInfo( connHandle, (smpIdentityInfo_t*)pIRK );
}

/*********************************************************************
    @fn          smPairingSendIdentityAddrInfo

    @brief       Send SM Identity Addr Information message

    @param       connHandle - connection handle
    @param       addrType - address type
    @param       pMACAddr - pointer to BD_ADDR

    @return      SUCCESS
                INVALIDPARAMETER
                other from HCI/LL
*/
void smPairingSendIdentityAddrInfo( uint16 connHandle, uint8 addrType, uint8* pMACAddr )
{
    smpIdentityAddrInfo_t identityAddrMsg;  // Message structure
    // Fill in parameters
    identityAddrMsg.addrType = addrType;
    VOID osal_memcpy( identityAddrMsg.bdAddr, pMACAddr, B_ADDR_LEN );
    // Send message
    VOID smSendIdentityAddrInfo( connHandle, &identityAddrMsg );
}

/*********************************************************************
    @fn          smPairingSendSingingInfo

    @brief       Send SM Signing Information message

    @param       connHandle - connection handle
    @param       pSRK - pointer to SRK

    @return      SUCCESS
                INVALIDPARAMETER
                other from HCI/LL
*/
void smPairingSendSingingInfo( uint16 connHandle, uint8* pSRK )
{
    VOID smSendSigningInfo( connHandle, (smpSigningInfo_t*)pSRK );
}

/*********************************************************************
    @fn          smFreePairingParams

    @brief       Free memory used in the Pairing Parameters

    @param       none

    @return      none
*/
static void smFreePairingParams( uint16 connectionHandle )
{
    #if !defined ( HOLD_PAIRING_PARAMETERS )
    // Clear the SM Timeout
    smStopRspTimer( connectionHandle);

    if ( pPairingParams[connectionHandle] )
    {
        // secReqs is not allocated in SM it's a borrowed struct from GAP.
        // So, no dealloc here.
        if ( pPairingParams[connectionHandle]->pPairDev )
        {
            osal_mem_free( pPairingParams[connectionHandle]->pPairDev );
        }

        if ( pPairingParams[connectionHandle]->pEncParams )
        {
            osal_mem_free( pPairingParams[connectionHandle]->pEncParams );
        }

        if ( pPairingParams[connectionHandle]->pDevEncParams )
        {
            osal_mem_free( pPairingParams[connectionHandle]->pDevEncParams );
        }

        if ( pPairingParams[connectionHandle]->pIdInfo )
        {
            osal_mem_free( pPairingParams[connectionHandle]->pIdInfo );
        }

        if ( pPairingParams[connectionHandle]->pSigningInfo )
        {
            osal_mem_free( pPairingParams[connectionHandle]->pSigningInfo );
        }

        osal_mem_free( pPairingParams[connectionHandle] );
        pPairingParams[connectionHandle] = NULL;
    }

    #endif
}

/*********************************************************************
    @fn          smEndPairing

    @brief       Pairing mode has ended.  Yeah. Notify the GAP and free
                up the memory used.

    @param       status - how was the pairing completed

    @return      none
*/
void smEndPairing( uint16 connectionHandle,uint8 status )
{
    if ( pPairingParams[connectionHandle] )
    {
        gapPairingCompleteCB( status, pPairingParams[connectionHandle]->initiator,
                              pPairingParams[connectionHandle]->connectionHandle,
                              pPairingParams[connectionHandle]->authState,
                              pPairingParams[connectionHandle]->pEncParams,
                              pPairingParams[connectionHandle]->pDevEncParams,
                              pPairingParams[connectionHandle]->pIdInfo,
                              pPairingParams[connectionHandle]->pSigningInfo );
        // free up the pPairingParams[connectionHandle]
        smFreePairingParams( connectionHandle);
    }
}

/*********************************************************************
    @fn          smDetermineKeySize

    @brief       Determine the maximum encryption key size.

    @param       none

    @return      the negotiated key size
*/
uint8 smDetermineKeySize( uint16 connectionHandle )
{
    uint8 keySize = KEYLEN;

    if ( pPairingParams[connectionHandle] )
    {
        if ( pPairingParams[connectionHandle]->pPairDev && pPairingParams[connectionHandle]->pSecReqs )
        {
            if ( pPairingParams[connectionHandle]->pPairDev->maxEncKeySize < pPairingParams[connectionHandle]->pSecReqs->maxEncKeySize )
            {
                keySize = pPairingParams[connectionHandle]->pPairDev->maxEncKeySize;
            }
            else
            {
                keySize = pPairingParams[connectionHandle]->pSecReqs->maxEncKeySize;
            }
        }
    }

    return ( keySize );
}

/*********************************************************************
    @fn          smSetPairingReqRsp

    @brief       Setup pairing request/response parameters.

    @param       pReq - Request/Response to be set

    @return      none
*/
static void smSetPairingReqRsp( uint16 connectionHandle,smpPairingReq_t* pReq )
{
    if ( pPairingParams[connectionHandle] && pPairingParams[connectionHandle]->pSecReqs )
    {
        smLinkSecurityReq_t* pSecReq = pPairingParams[connectionHandle]->pSecReqs;
        pReq->ioCapability = pSecReq->ioCaps;
        pReq->oobDataFlag = pSecReq->oobAvailable;
        smUint8ToAuthReq( &(pReq->authReq), pSecReq->authReq );
        pReq->maxEncKeySize = pSecReq->maxEncKeySize;
        pReq->keyDist = pSecReq->keyDist;
    }
    else
    {
        VOID osal_memset( pReq, 0, sizeof( smpPairingReq_t ) );
    }
}

/*********************************************************************
*********************************************************************/
