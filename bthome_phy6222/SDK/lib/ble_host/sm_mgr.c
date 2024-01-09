/*************************************************************************************************
    Filename:       sm_mgr.c
    Revised:
    Revision:

    Description:    This file contains the SM Manager.

	SDK_LICENSE

**************************************************************************************************/

#include "bcomdef.h"
#include "OSAL.h"
#include "hci.h"
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

// Generate Key States
#define GENERATE_KEY_INIT         0 // Initial state of key generation
#define GENERATE_KEY_RAND1        1 // Performing a Rand on the key

#define LEN_64BIT                 8 // Number of bytes in a 64 bit number
#define LEN_32BIT                 4 // Number of bytes in a 32 bit number
#define LEN_24BIT                 3 // Number of bytes in a 24 bit number

#define PRAND_SIZE                LEN_24BIT // PRAND size in the Private Resolvable Address calculation

// Defines used in the sm_c1new() function
#define PADDINGLEN                4 // Padding length (bytes)
#define P1LEN                     (SMP_PAIRING_RSP_LEN + SMP_PAIRING_REQ_LEN + 1 + 1)
#define P2LEN                     (PADDINGLEN + B_ADDR_LEN + B_ADDR_LEN )

/*********************************************************************
    TYPEDEFS
*/

// Structure used to generate a Key
typedef struct
{
    uint8 state;          // Key Generation State
    uint8 taskID;         // Task ID of task that wants the key
    uint8 key[KEYLEN];    // Place to put the key
} smGenKey_t;

// Encrypt function structure
typedef struct
{
    uint8 key[KEYLEN];              // Key to encrypt with
    uint8 plainTextData[KEYLEN];    // Plain Text data to encrypt
    uint8 result[KEYLEN];           // Result of encrypt (key + Plain Text data)
} sm_Encrypt_t;

/*********************************************************************
    GLOBAL VARIABLES
*/
uint8 smTO_CBTimer[15]= {0};

/*********************************************************************
    EXTERNAL VARIABLES
*/

/*********************************************************************
    EXTERNAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/

// Variables used in key generation
static smGenKey_t* pSmGenKey = (smGenKey_t*)NULL;

// Const table used in key calculations
static CONST uint8 const_Rb[16] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87
};

/*********************************************************************
    LOCAL FUNCTIONS
*/
static void smSendGenKeyEvent( uint8 status );
static bStatus_t smEncrypt( sm_Encrypt_t* pParam );
static bStatus_t smEncryptLocal( uint8* pKey, uint8* pPlaintextData, uint8* pEncryptedData );

static void sm_xor( uint8* p1, uint8* p2 );
static bStatus_t sm_CMAC( uint8* pK, uint8* pM, uint8 mLen, uint8* pMac );
static bStatus_t generate_subkey( uint8* pKey, uint8* pK1, uint8* pK2 );
static void xor_128( uint8* pA, CONST uint8* pB, uint8* pOutcome );
static void padding ( uint8* pLastb, uint8* pPad, uint8 length );
static void leftshift_onebit( uint8* pInp, uint8* pOutp );

// The next section is used to validate the Key Calculation Algorithm
// used during the pairing process.
#if defined ( TEST_CMAC )
// Test function to test key calculation algorithm - Spec test
void Testsm_CMAC( void );

// Tables used to test key calculation algorithm - Spec Test
unsigned char M[64] =
{
    0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
    0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
    0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
    0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
    0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11,
    0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
    0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17,
    0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10
};
unsigned char key[16] =
{
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};

// Test function from the AES-CMAC Algorithm Network Working Group Spec (NIST) - June 2006
void Testsm_CMAC( void )
{
    static unsigned char  T[16];
    //  printf("--------------------------------------------------\n");
    //  printf("K              "); print128(key); printf("\n");
    //  osal_memset( L, 0, 16 );
    //  osal_memset( K1, 0, 16 );
    //  osal_memset( K2, 0, 16 );
    osal_memset( T, 0, 16 );
    //  osal_memset( TT, 0, 16 );
    //  printf("\nSubkey Generation\n");
    //  AES_128(key,const_Zero,L);
    //  VOID smEncryptLocal( key, const_Zero, L );
    //  printf("AES_128(key,0) "); print128(L); printf("\n");
    //  generate_subkey(key,K1,K2);
    //  printf("K1             "); print128(K1); printf("\n");
    //  printf("K2             "); print128(K2); printf("\n");
    //  printf("\nExample 1: len = 0\n");
    //  printf("M              "); printf("<empty string>\n");
    //  AES_CMAC(key,M,0,T);
    sm_CMAC( key, M, 0, T );
    //  printf("AES_CMAC       "); print128(T); printf("\n");
    //  printf("\nExample 2: len = 16\n");
    //  printf("M              "); print_hex("                ",M,16);
    //  AES_CMAC(key,M,16,T);
    sm_CMAC( key, M, 16, T );
    //  printf("AES_CMAC       "); print128(T); printf("\n");
    //  printf("\nExample 3: len = 40\n");
    //  printf("M              "); print_hex("               ",M,40);
    //  AES_CMAC(key,M,40,T);
    sm_CMAC( key, M, 40, T );
    //  printf("AES_CMAC       "); print128(T); printf("\n");
    //  printf("\nExample 4: len = 64\n");
    //  printf("M              "); print_hex("               ",M,64);
    //  AES_CMAC(key,M,64,T);
    sm_CMAC( key, M, 64, T );
    return;
}
#endif

/*********************************************************************
    Generate a key with a random value.

    Public function defined in sm.h.
*/
bStatus_t SM_NewRandKey( uint8 taskID )
{
    // Already doing something?
    if ( smInProcess() )
    {
        return ( bleNotReady );
    }

    // Ask for a random number
    if ( HCI_LE_RandCmd() == SUCCESS )
    {
        // Keep the state of the random generation
        pSmGenKey = (smGenKey_t*)osal_mem_alloc( sizeof( smGenKey_t ) );

        if ( pSmGenKey )
        {
            pSmGenKey->state = GENERATE_KEY_INIT;
            pSmGenKey->taskID = taskID;
            return ( SUCCESS );
        }
        else
        {
            return ( bleMemAllocError );
        }
    }
    else
    {
        return ( FAILURE );
    }
}

/*********************************************************************
    Calculate a new Private Resolvable address.

    Public function defined in sm.h.
*/
bStatus_t SM_CalcRandomAddr( uint8* pIRK, uint8* pNewAddr )
{
    bStatus_t status;         // return value
    uint8 PRand[PRAND_SIZE];  // Place to hold the PRAND

    // parameter validation
    if ( (pIRK == NULL) || (pNewAddr == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    // Generate Random number
    smGenerateRandBuf( PRand, PRAND_SIZE );
    // Clear the Random Address header bits and force the address type
    PRand[PRAND_SIZE-1] &= ~(RANDOM_ADDR_HDR);
    PRand[PRAND_SIZE-1] |= PRIVATE_RESOLVE_ADDR_HDR;
    // Create the new address
    status = sm_ah( pIRK, PRand, pNewAddr );

    if ( status == SUCCESS )
    {
        // attach the PRAND to the new address
        VOID osal_memcpy( &(pNewAddr[PRAND_SIZE]), PRand, PRAND_SIZE );
    }

    return ( status );
}

/*********************************************************************
    Resolve a Private Resolvable Address.

    Public function defined in sm.h.
*/
bStatus_t SM_ResolveRandomAddrs( uint8* pIRK, uint8* pAddr )
{
    bStatus_t stat;          // return value
    uint8 rand[PRAND_SIZE];  // place for PRAND
    uint8 hash[PRAND_SIZE];  // place for hash (calc PRAND)

    // Parameter check
    if ( (pIRK == NULL) || (pAddr == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    // Get PRAND out of address
    VOID osal_memcpy( rand, &(pAddr[PRAND_SIZE]), PRAND_SIZE );
    // Clear the Random Address header bits and force the address type
    rand[PRAND_SIZE-1] &= ~(RANDOM_ADDR_HDR);
    rand[PRAND_SIZE-1] |= PRIVATE_RESOLVE_ADDR_HDR;
    // Calculate Hash from PRAND
    stat = sm_ah( pIRK, rand, hash );

    if ( stat != SUCCESS )
    {
        // Something went wrong with hash function
        return ( stat );
    }

    // Compare hash to address portion of address
    if ( osal_memcmp( hash, pAddr, LEN_24BIT ) == TRUE )
    {
        // Matched
        return ( SUCCESS );
    }
    else
    {
        // not Matched
        return ( FAILURE );
    }
}

/*********************************************************************
    Encrypt the plain text data with the key.

    Public function defined in sm.h.
*/
bStatus_t SM_Encrypt( uint8* pKey, uint8* pPlainText, uint8* pResult )
{
    bStatus_t stat;       // return value
    sm_Encrypt_t param;   // place to perform work

    // Check Parameters
    if ( (pKey == NULL) || (pPlainText == NULL) || (pResult == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    // Copy MSByte first
    VOID osal_revmemcpy( param.key, pKey, KEYLEN );
    VOID osal_revmemcpy( param.plainTextData, pPlainText, KEYLEN );
    // Perform Encrypt
    stat = smEncrypt( &param );
    // Reverse results
    VOID osal_revmemcpy( pResult, param.result, KEYLEN );
    return ( stat );
}

/*********************************************************************
    Generate an outgoing Authentication Signature.

    Public function defined in sm.h.
*/
bStatus_t SM_GenerateAuthenSig( uint8* pData, uint8 len, uint8* pAuthenSig )
{
    bStatus_t stat;         // return value
    uint8* pM;               // "M" variable in CMAC algorithm
    uint32 signCounter;     // Sign Counter
    uint8 mLen;             // Length of "M" needed
    uint8 mac[LEN_64BIT];   // CMAC result
    uint8 revKey[KEYLEN];   // A place to hold the reverse SRK

    // Check parameters
    if ( (pData == NULL) || (pAuthenSig == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    // Calculate the space needed for "M" and allocate "M"
    mLen = len + LEN_32BIT;
    pM = osal_mem_alloc( mLen );

    if ( pM )
    {
        // Get this device's sign counter
        signCounter = gapGetSignCounter();
        // Pass all variables to CMAC in MSByte order
        VOID osal_revmemcpy( &pM[LEN_32BIT], pData, len );
        pM[3] = BREAK_UINT32( signCounter, 0 );
        pM[2] = BREAK_UINT32( signCounter, 1 );
        pM[1] = BREAK_UINT32( signCounter, 2 );
        pM[0] = BREAK_UINT32( signCounter, 3 );
        VOID osal_revmemcpy( revKey, gapGetSRK(), KEYLEN );
        // Perform CMAC algorithm
        stat = sm_CMAC( revKey, pM, mLen, mac );

        if ( stat == SUCCESS )
        {
            // Build message authentication
            pAuthenSig[0] = BREAK_UINT32( signCounter, 0 );
            pAuthenSig[1] = BREAK_UINT32( signCounter, 1 );
            pAuthenSig[2] = BREAK_UINT32( signCounter, 2 );
            pAuthenSig[3] = BREAK_UINT32( signCounter, 3 );
            VOID osal_revmemcpy( &pAuthenSig[LEN_32BIT], mac, LEN_64BIT );
            // Increment this device's signature counter
            gapIncSignCounter();
        }

        osal_mem_free( pM ); // Not needed anymore
    }
    else
    {
        stat = bleMemAllocError;
    }

    return ( stat );
}

/*********************************************************************
    Verify an Authentication Signature.

    Public function defined in sm.h.
*/
bStatus_t SM_VerifyAuthenSig( uint16 connHandle, uint8 authentication,
                              uint8* pData, uint16 len, uint8* pAuthenSig )
{
    bStatus_t stat;           // return value
    linkDBItem_t* pConnItem;  // pointer to connection information
    uint32 signCounter;       // sign counter in pAuthenSig field
    uint8* pM;                // "M" variable
    uint8 mLen;               // Length of "M"
    uint8 mac[LEN_64BIT];     // CMAC
    uint8 revmac[LEN_64BIT];  // A place to reverse CMAC
    uint8 revKey[KEYLEN];     // A place to reverse the SRK

    // Check parameters
    if ( (pData == NULL) || (pAuthenSig == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    // Find the connection information
    pConnItem = linkDB_Find( connHandle );

    if ( pConnItem == NULL )
    {
        return ( bleNotConnected );
    }

    // Check sign counter and make sure it's more than the last sign counter received.
    signCounter = BUILD_UINT32( pAuthenSig[0], pAuthenSig[1], pAuthenSig[2], pAuthenSig[3] );

    if ( (signCounter <= pConnItem->sec.signCounter)
            && (pConnItem->sec.signCounter != GAP_INIT_SIGN_COUNTER) )
    {
        return ( INVALIDPARAMETER );
    }

    // Check authentication level
    if ( (authentication) && ((pConnItem->stateFlags & LINK_AUTHENTICATED) == 0) )
    {
        // Signing information wasn't authenticated
        return ( FAILURE );
    }

    // Initialize CMAC
    VOID osal_memset( mac, 0, LEN_64BIT );
    // Adjust the M length for the sign counter and allocate space for "M"
    mLen = len + LEN_32BIT;
    pM = osal_mem_alloc( mLen );

    if ( pM == NULL )
    {
        return ( bleMemAllocError );
    }

    // Pass all variables to CMAC in MSByte order
    VOID osal_revmemcpy( &pM[LEN_32BIT], pData, len );
    VOID osal_revmemcpy( pM, pAuthenSig, LEN_32BIT );
    VOID osal_revmemcpy( revKey, pConnItem->sec.srk, KEYLEN );
    // perform CMAC algorithm
    stat = sm_CMAC( revKey, pM, mLen, mac );

    if ( stat == SUCCESS )
    {
        // Compare the calculated CMAC against the CMAC in the signature
        VOID osal_revmemcpy( revmac, mac, LEN_64BIT );

        if ( osal_memcmp( revmac, &pAuthenSig[LEN_32BIT], LEN_64BIT ) != TRUE )
        {
            stat = FAILURE;
        }
        else
        {
            // Update the connection's signCounter
            gapUpdateConnSignCounter( connHandle, signCounter );
        }
    }

    osal_mem_free( pM ); // Don't need "M" anymore
    return ( stat );
}

/*********************************************************************
    @fn          smProcessRandComplete

    @brief       Process the HCI Random Complete Event.

    @param       status - status of the command.
    @param       rand - pointer to the 8 bytes random string

    @return      TRUE if we asked for this message
                FALSE if we aren't expecting this messsage
*/
uint8 smProcessRandComplete( uint8 status, uint8* rand )
{
    if ( pSmGenKey )
    {
        uint8 stateDone = FALSE;  // Process finish flag

        if ( status == SUCCESS )
        {
            // Are we generating a key?
            if ( pSmGenKey->state == GENERATE_KEY_INIT )
            {
                // Save off the random bytes
                VOID osal_memcpy( pSmGenKey->key, rand, LEN_64BIT );

                // Ask for more random bytes
                if ( HCI_LE_RandCmd() == SUCCESS )
                {
                    // Mark the state
                    pSmGenKey->state = GENERATE_KEY_RAND1;
                }
                else
                {
                    // Error
                    stateDone = TRUE;
                    status = FAILURE;
                }
            }
            else
            {
                // This is the second set of random bytes, we are done
                VOID osal_memcpy( &(pSmGenKey->key[LEN_64BIT]), rand, LEN_64BIT );
                stateDone = TRUE;
            }
        }
        else
        {
            // Error
            stateDone = TRUE;
        }

        if ( stateDone )
        {
            // Send the key
            smSendGenKeyEvent( status );
        }

        return ( TRUE );
    }
    else
    {
        // Don't know why we received this message
        return ( FALSE );
    }
}

/*********************************************************************
    @fn          smStartRspTimer

    @brief       Start the SM Response Timer.

    @param       none

    @return      none
*/

void smStartRspTimer( uint16 connectionHandle )
{
    // Get the timeout value
    uint32 timeout = GAP_GetParamValue( TGAP_SM_TIMEOUT );

    if( pPairingParams[connectionHandle] )
    {
        if ( timeout )
        {
            if( ( pPairingParams[connectionHandle]->timerID != INVALID_TASK_ID) && (osal_CbTimerUpdate(pPairingParams[connectionHandle]->timerID,timeout ) == SUCCESS ))
            {
                LOG("%s,update connectionHandle %d timerID %d\n",__func__,connectionHandle,pPairingParams[connectionHandle]->timerID);
            }
            else
            {
                smTO_CBTimer[connectionHandle] = connectionHandle;
                osal_CbTimerStart( smTo_timerCB, &smTO_CBTimer[connectionHandle],
                                   timeout, &pPairingParams[connectionHandle]->timerID );
                LOG("%s,start connectionHandle %d timerid %d\n",__func__,connectionHandle,pPairingParams[connectionHandle]->timerID );
            }
        }
    }
    else
    {
        LOG("pPairingParams[%d] NULL \n",connectionHandle);
    }
}

/*********************************************************************
    @fn          smStopRspTimer

    @brief       Stop the SM Response Timer.

    @param       none

    @return      none
*/
void smStopRspTimer( uint16 connectionHandle )
{
//    VOID osal_stop_timerEx( smTaskID, SM_TIMEOUT_EVT );
    if( pPairingParams[connectionHandle] )
    {
        LOG("smStopRspTimer connectionHandle %d timerID %d\n",connectionHandle,pPairingParams[connectionHandle]->timerID);
        osal_CbTimerStop( pPairingParams[connectionHandle]->timerID );
        pPairingParams[connectionHandle]->timerID = INVALID_TASK_ID;
    }
    else
    {
        LOG("pPairingParams[%d] NULL \n",connectionHandle);
    }
}

/*********************************************************************
    @fn          smInProcess

    @brief       Checks to see if the SM is already processing something.

    @param       none

    @return      TRUE if already processing, FALSE if not
*/
uint8 smInProcess( void )
{
    if ( pSmGenKey  )
    {
        return ( TRUE );
    }
    else
    {
        return ( FALSE );
    }
}

/*********************************************************************
    @fn          sm_d1

    @brief       SM diversifying function d1

    @param       pK - key is 128 bits, LSByte first
    @param       d - 16 bit
    @param       pD1 - pointer to 128 bit results space

    @return      status
*/
bStatus_t sm_d1( uint8* pK, uint16 d, uint8* pD1 )
{
    bStatus_t stat;       // return value
    sm_Encrypt_t param;   // place to work for the encrypt function
    // Clear the encrypt parameters
    VOID osal_memset( &param, 0, sizeof ( sm_Encrypt_t ) );
    // Copy the encrypt variables
    VOID osal_revmemcpy( param.key, pK, KEYLEN );
    param.plainTextData[KEYLEN - 2] = HI_UINT16( d );
    param.plainTextData[KEYLEN - 1] = LO_UINT16( d );
    stat = smEncrypt( &param );
    // Copy the results
    VOID osal_revmemcpy( pD1, param.result, KEYLEN );
    return ( stat );
}

/*********************************************************************
    @fn          sm_ah

    @brief       Random address hash function.

    @param       pK - key is 128 bits, LSByte first
    @param       pR - 24 bit, LSByte first
    @param       pAh - pointer to 24 bit results space

    @return      status
*/
bStatus_t sm_ah( uint8* pK, uint8* pR, uint8* pAh )
{
    bStatus_t stat;       // return value
    sm_Encrypt_t param;   // place to work for the encrypt function
    // Clear the encrypt parameters
    VOID osal_memset( &param, 0, sizeof ( sm_Encrypt_t ) );
    // Copy the encrypt variables
    VOID osal_revmemcpy( param.key, pK, KEYLEN );
    VOID osal_revmemcpy( &(param.plainTextData[KEYLEN - LEN_24BIT]),
                         pR, LEN_24BIT );
    stat = smEncrypt( &param );
    // Copy the results
    VOID osal_revmemcpy( pAh, &(param.result[KEYLEN - LEN_24BIT]), LEN_24BIT );
    return ( stat );
}

/*********************************************************************
    @fn          sm_dm

    @brief       SM DIV Maxk generation function dm

    @param       pK - key is 128 bits, LSByte first
    @param       pR - 64 bit, LSByte first
    @param       pDm - pointer to 16 bit result

    @return      status
*/
bStatus_t sm_dm( uint8* pK, uint8* pR, uint16* pDm )
{
    bStatus_t stat;       // return value
    sm_Encrypt_t param;   // place to work for the encrypt function
    // Clear the encrypt parameters
    VOID osal_memset( &param, 0, sizeof ( sm_Encrypt_t ) );
    // Copy the encrypt variables
    VOID osal_revmemcpy( param.key, pK, KEYLEN );
    VOID osal_revmemcpy( &(param.plainTextData[KEYLEN - LEN_64BIT]), pR, LEN_64BIT );
    stat = smEncrypt( &param );
    // Copy the results
    *pDm = BUILD_UINT16( param.result[KEYLEN - 1], param.result[KEYLEN - 2] );
    return ( stat );
}

/*********************************************************************
    @fn          sm_c1new

    @brief       SM Confirm value generation function c1.  This function
                implements the following formula from the spec:

                  p1 = pres || preq || rat?|| iat?
                  p2 = padding || ia || ra
                  e(k, e(k, r XOR p1) XOR p2)

    @param       pK - key is 128 bits, LSByte first
    @param       pR - 128 bit, LSByte first
    @param       pRes - 65 bit, LSByte first - Pairing Response Command
    @param       pReq - 65 bit, LSByte first - Pairing Request Command
    @param       iat - 1 bit - Initiator Address Type
    @param       pIA - 48 bit, LSByte first - Initiator Address
    @param       rat - 1 bit - Responder Address Type
    @param       pRA - 48 bit, LSByte first - Responder Address
    @param       pC1 - pointer to 128 bit results space

    @return      status
*/
bStatus_t sm_c1new( uint8* pK, uint8* pR, uint8* pRes, uint8* pReq,
                    uint8 iat, uint8* pIA, uint8 rat, uint8* pRA, uint8* pC1 )
{
    bStatus_t stat;         // return value
    uint8 p1[P1LEN];        // major formula variable
    uint8 p2[P2LEN];        // major formula variable
    sm_Encrypt_t c1param;   // Place to work for the encrypt
    // First, clear the structure
    VOID osal_memset( &c1param, 0, sizeof ( sm_Encrypt_t ) );
    // Copy the key into the correct format
    VOID osal_revmemcpy( c1param.key, pK, KEYLEN );
    // Make p1
    VOID osal_memset( p1, 0, P1LEN );
    VOID osal_revmemcpy( p1, pRes, SMP_PAIRING_RSP_LEN );
    VOID osal_revmemcpy( &p1[SMP_PAIRING_RSP_LEN], pReq, SMP_PAIRING_REQ_LEN );
    p1[SMP_PAIRING_RSP_LEN + SMP_PAIRING_REQ_LEN] = (rat > ADDRTYPE_PUBLIC) ? 1 : 0;
    p1[SMP_PAIRING_RSP_LEN + SMP_PAIRING_REQ_LEN + 1] = (iat > ADDRTYPE_PUBLIC) ? 1 : 0;
    // Make p2
    VOID osal_memset( p2, 0, P2LEN );
    VOID osal_revmemcpy( &(p2[PADDINGLEN]), pIA, B_ADDR_LEN );
    VOID osal_revmemcpy( &(p2[PADDINGLEN + B_ADDR_LEN]), pRA, B_ADDR_LEN );
    // copy r into the correct format
    VOID osal_revmemcpy( c1param.plainTextData, pR, SMP_RANDOM_LEN );
    // r XOR p1
    sm_xor( c1param.plainTextData, p1 );
    // e(k, r XOR p1)
    stat = smEncrypt( &c1param );
    VOID osal_memcpy( c1param.plainTextData, c1param.result, KEYLEN );
    // e(k, r XOR p1) XOR p2
    sm_xor( c1param.plainTextData, p2 );
    // e(k, e(k, r XOR p1) XOR p2)
    stat |= smEncrypt( &c1param );
    // copy the result
    VOID osal_revmemcpy( pC1, c1param.result, KEYLEN );
    return ( stat );
}


/*********************************************************************
    @fn          sm_xor

    @brief       xor 2 fix length arrays (16 bytes).
                This function destroys p1.

    @param       p1 - array #1
    @param       p2 - array #2

    @return      none
*/
static void sm_xor( uint8* p1, uint8* p2 )
{
    uint8 x;  // loop counter

    for ( x = 0; x < KEYLEN; x++, p1++, p2++ )
    {
        // XOR byte by byte
        *p1 = *p1 ^ *p2;
    }
}

/*********************************************************************
    @fn          sm_s1

    @brief       SM key generation function s1

    @param       pK - key is 128 bits, LSByte first
    @param       pR1 - 64 bit, LSByte first
    @param       pR2 - 64 bit, LSByte first
    @param       pS1 - pointer to 128 bit results space

    @return      status
*/
bStatus_t sm_s1( uint8* pK, uint8* pR1, uint8* pR2, uint8* pS1 )
{
    bStatus_t stat;       // return value
    sm_Encrypt_t param;   // place to work for the encrypt function
    // Fill in the parameters
    VOID osal_revmemcpy( param.key, pK, KEYLEN );
    VOID osal_revmemcpy( param.plainTextData, pR1, LEN_64BIT );
    VOID osal_revmemcpy( &(param.plainTextData[LEN_64BIT]), pR2, LEN_64BIT );
    stat = smEncrypt( &param );
    // Copy the results
    VOID osal_revmemcpy( pS1, param.result, KEYLEN );
    return ( stat );
}

/*********************************************************************
    @fn          smGenerateRandBuf

    @brief       Generate a buffer with random numbers.

    @param       pRandNum - pointer to buffer
    @param       len - length of buffer.

    @return      none
*/
void smGenerateRandBuf( uint8* pRandNum, uint8 len )
{
    uint8 x = 0;  // loop counter
    uint16 rand;  // Random value

    while ( x < len  )
    {
        // Get a random value
        rand = osal_rand();
        pRandNum[x] = HI_UINT16( rand );
        x++;

        // Can we use the other half?
        if ( x < len )
        {
            pRandNum[x] = LO_UINT16( rand );
            x++;
        }
    }
}

/*********************************************************************
    @fn          smAuthReqToUint8

    @brief       Conversion function to convert authReq_t to uint8

    @param       pAuthReq - pointer to

    @return      uint8 conversion
*/
uint8 smAuthReqToUint8( authReq_t* pAuthReq )
{
    uint8 tmp;
    tmp = pAuthReq->bonding;
    tmp |= (pAuthReq->mitm << 2);
    tmp |= (pAuthReq->reserved << 3);
    return ( tmp );
}

/*********************************************************************
    @fn          smUint8ToAuthReq

    @brief       Conversion function to convert uint8 to authReq_t

    @param       pAuthReq - pointer to structure
    @param       authReqUint8 - byte version of authReq

    @return      none
*/
void smUint8ToAuthReq( authReq_t* pAuthReq, uint8 authReqUint8 )
{
    pAuthReq->bonding = (authReqUint8 & 0x03);
    pAuthReq->mitm = (authReqUint8 & 0x04) ? TRUE : FALSE;
    pAuthReq->reserved = (authReqUint8 >> 3) & 0x1F;
}

/*********************************************************************
    @fn          smSendGenKeyEvent

    @brief       Send the SM new random key Event message and free
                pSmGenKey.

    @param       status - status of the command.

    @return      none
*/
static void smSendGenKeyEvent( uint8 status )
{
    if ( pSmGenKey )
    {
        smNewRandKeyEvent_t* pRsp;
        pRsp = (smNewRandKeyEvent_t*)osal_msg_allocate( sizeof ( smNewRandKeyEvent_t ) );

        if ( pRsp )
        {
            pRsp->hdr.event = SM_NEW_RAND_KEY_EVENT;
            pRsp->hdr.status = status;
            VOID osal_memcpy( pRsp->newKey, pSmGenKey->key, KEYLEN );
            VOID osal_msg_send( pSmGenKey->taskID, (uint8*)pRsp );
        }

        osal_mem_free( pSmGenKey );
        pSmGenKey = NULL;
    }
}

/*********************************************************************
    @fn          smEncrypt

    @brief       Seperates the sm_Encrypt_t structure and calls
                smEncryptLocal()

    @param       pParam - pointer to the parameter structure

    @return      Status_t
*/
static bStatus_t smEncrypt( sm_Encrypt_t* pParam )
{
    return ( smEncryptLocal( pParam->key, pParam->plainTextData, pParam->result ) );
}

/*********************************************************************
    @fn          smEncryptLocal

    @brief       Perform the HCI Encrypt function.

    @param       pKey - Encrypt Key
    @param       pPlaintextData - plain text to encrypt
    @param       pEncryptedData - pointer to result space

    @return      Status_t
*/
static bStatus_t smEncryptLocal( uint8* pKey, uint8* pPlaintextData, uint8* pEncryptedData )
{
    // This function is kind of a cheat because it isn't going through
    // HCI.  If the host is to be used on different (from LL) processor
    // it will have to provide a local copy of LL_Encrypt, or it will
    // have to be written to use state machine events.
    return ( LL_Encrypt( pKey, pPlaintextData, pEncryptedData ) );
}

/*********************************************************************
    @fn          sm_CMAC

    @brief       An implementation of the CMAC algorithm.

    NOTE:  All of the input and output buffers are MSByte first.
          The calling function must take care of the byte order, before
          and after calling this function

    @param       pK - key
    @param       pM - data
    @param       mLen - data length
    @param       pMac - pointer to the 64 bit results

    @return      bStatus_t
*/
static bStatus_t sm_CMAC( uint8* pK, uint8* pM, uint8 mLen, uint8* pMac )
{
    int16 n, i, flag;
    uint8* pK1;
    uint8* pK2;
    bStatus_t stat;
    // Allocate RAM needed
    pK1 = osal_mem_alloc( KEYLEN );
    pK2 = osal_mem_alloc( KEYLEN );

    // Make sure they allocated
    if ( (pK1 != NULL) && (pK2 != NULL) )
    {
        stat = generate_subkey( pK, pK1, pK2 );

        if ( stat == SUCCESS )
        {
            uint8*  pX;
            uint8*  pM_last;
            pX = osal_mem_alloc( KEYLEN );
            pM_last = osal_mem_alloc( KEYLEN );

            if ( (pX != NULL) && (pM_last != NULL) )
            {
                uint8  mIdx;
                n = (mLen + 15)/16;  /* n is number of rounds */

                if ( n == 0 )
                {
                    n = 1;
                    flag = 0;
                }
                else
                {
                    if ( (mLen%16) == 0 ) /* last block is a complete block */
                    {
                        flag = 1;
                    }
                    else /* last block is not complete block */
                    {
                        flag = 0;
                    }
                }

                mIdx = (uint8)(16*(n-1));

                if ( flag ) /* last block is complete block */
                {
                    xor_128( &pM[mIdx], pK1, pM_last );
                }
                else
                {
                    uint8 padded[KEYLEN];
                    padding( &pM[mIdx], padded, mLen%16 );
                    xor_128( padded, pK2, pM_last );
                }

                VOID osal_memset( pX, 0, KEYLEN );
                {
                    uint8 Y[KEYLEN];

                    for ( i = 0; (i < (n-1)) && (stat == SUCCESS); i++ )
                    {
                        mIdx = (uint8)(16*i);
                        xor_128( pX, &pM[mIdx], Y ); /* Y := Mi (+) X  */
                        stat = smEncryptLocal( pK, Y, pX );
                    }

                    if ( stat == SUCCESS )
                    {
                        xor_128( pX, pM_last, Y );
                        stat = smEncryptLocal( pK, Y, pX );
                        // T = MSB[Tlen]( Cn ); T = mac, Tlen = LEN_64BIT, Cn = X
                        VOID osal_memcpy( pMac, pX, LEN_64BIT );
                    }
                }
            }
            else
            {
                stat = bleMemAllocError;
            }

            if ( pX )
            {
                osal_mem_free( pX );
            }

            if ( pM_last )
            {
                osal_mem_free( pM_last );
            }
        }
    }
    else
    {
        stat = bleMemAllocError;
    }

    if ( pK1 )
    {
        osal_mem_free( pK1 );
    }

    if ( pK2 )
    {
        osal_mem_free( pK2 );
    }

    return ( stat );
}

/*********************************************************************
    @fn          generate_subkey

    @brief       Generate CMAC Subkeys.  Only call from sm_CMAC.

    @param       pKey - key
    @param       pK1 - sub key 1
    @param       pK2 - sub key 2

    @return      bStatus_t
*/
static bStatus_t generate_subkey( uint8* pKey, uint8* pK1, uint8* pK2 )
{
    uint8* pL;
    uint8* pTmp;
    uint8 stat;
    pL = osal_mem_alloc( KEYLEN );
    pTmp = osal_mem_alloc( KEYLEN );

    if ( (pL != NULL) && (pTmp != NULL) )
    {
        VOID osal_memset( pTmp, 0, KEYLEN );
        stat = smEncryptLocal( pKey, pTmp, pL );

        if ( stat == SUCCESS )
        {
            if ( (pL[0] & 0x80) == 0 ) /* If MSB(L) = 0, then K1 = L << 1 */
            {
                leftshift_onebit( pL, pK1 );
            }
            else /* Else pK1 = ( pL << 1 ) (+) Rb */
            {
                leftshift_onebit( pL, pTmp );
                xor_128( pTmp, const_Rb, pK1 );
            }

            if ( (pK1[0] & 0x80) == 0 )
            {
                leftshift_onebit( pK1, pK2 );
            }
            else
            {
                leftshift_onebit( pK1, pTmp );
                xor_128( pTmp, const_Rb, pK2 );
            }
        }
    }
    else
    {
        stat = bleMemAllocError;
    }

    if ( pL )
    {
        osal_mem_free( pL );
    }

    if ( pTmp )
    {
        osal_mem_free( pTmp );
    }

    return ( stat );
}

/*********************************************************************
    @fn          xor_128

    @brief       XOR 128 bits - outcome = a XOR B
                This function doesn't XOR in place.

    @param       pA - var 1
    @param       pB - var 2
    @param       pOutcome - outcome

    @return      None
*/
static void xor_128( uint8* pA, CONST uint8* pB, uint8* pOutcome )
{
    for ( uint8 i = 0; i < KEYLEN; i++ )
    {
        pOutcome[i] = pA[i] ^ pB[i];
    }
}

/*********************************************************************
    @fn          padding

    @brief       add padding to "pad", first copy lastb, then on the last
                byte of the "lastb" set to 0x80, the rest of the
                buffer set to 0.

    @param       pLastb - previous buffer (last block)
    @param       pPad - new buffer
    @param       length - length non-padding

    @return      None
*/
static void padding ( uint8* pLastb, uint8* pPad, uint8 length )
{
    /* original last block */
    for ( uint8 j = 0; j < KEYLEN; j++ )
    {
        if ( j < length )
        {
            pPad[j] = pLastb[j];
        }
        else if ( j == length )
        {
            pPad[j] = 0x80;    // mark the last non-pad byte
        }
        else
        {
            pPad[j] = 0x00;
        }
    }
}

/*********************************************************************
    @fn          leftshift_onebit

    @brief       Left shift 128 bits

    @param       pInp - input buffer
    @param       pOutp - output buffer

    @return      None
*/
static void leftshift_onebit( uint8* pInp, uint8* pOutp )
{
    uint8 overflow = 0;

    for ( int8 i = 15; i >= 0; i-- )
    {
        pOutp[i] = pInp[i] << 1;
        pOutp[i] |= overflow;
        overflow = (pInp[i] & 0x80) ? 1 : 0;
    }
}
