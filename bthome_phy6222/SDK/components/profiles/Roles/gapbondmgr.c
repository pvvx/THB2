/*************
 SDK_LICENSE
***************/
/**************************************************************************************************
    Filename:       gapbondmgr.c
    Revised:
    Revision:

    Description:    GAP peripheral profile manages bonded connections



**************************************************************************************************/

#if ( HOST_CONFIG & ( CENTRAL_CFG | PERIPHERAL_CFG ) )

/*********************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "OSAL.h"
#include "osal_snv.h"
#include "gap.h"
#include "linkdb.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "hci.h"
#include "gattservapp.h"
#include "gapgattserver.h"
#include "gapbondmgr.h"

// temp set
// #define osal_snv_write( a, b, c )      (1)
// #define osal_snv_read( a, b, c )       (1)
// #define osal_snv_compact( a )          (1)
#define BOND_LOG(...)
//#define BOND_LOG   LOG

/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/
// Task event types
//#define GAP_BOND_SYNC_CC_EVT                            0x0001 // Sync char config
//#define GAP_BOND_SAVE_REC_EVT                           0x0002 // Save bond record in NV

// Once NV usage reaches this percentage threshold, NV compaction gets triggered.
#define NV_COMPACT_THRESHOLD                            80

// Bonded State Flags
#define GAP_BONDED_STATE_AUTHENTICATED                  0x0001
#define GAP_BONDED_STATE_SERVICE_CHANGED                0x0002

/**
    GAP Bond Manager NV layout

    The NV definitions:
       BLE_NVID_GAP_BOND_START - starting NV ID
       GAP_BONDINGS_MAX - Maximum number of bonding allowed (10 is max for number of NV IDs allocated in bcomdef.h).

    A single bonding entry consists of 6 components (NV items):
       Bond Record - defined as gapBondRec_t and uses GAP_BOND_REC_ID_OFFSET for an NV ID
       local LTK Info - defined as gapBondLTK_t and uses GAP_BOND_LOCAL_LTK_OFFSET for an NV ID
       device LTK Info - defined as gapBondLTK_t and uses GAP_BOND_DEV_LTK_OFFSET for an NV ID
       device IRK - defined as "uint8 devIRK[KEYLEN]" and uses GAP_BOND_DEV_IRK_OFFSET for an NV ID
       device CSRK - defined as "uint8 devCSRK[KEYLEN]" and uses GAP_BOND_DEV_CSRK_OFFSET for an NV ID
       device Sign Counter - defined as a uint32 and uses GAP_BOND_DEV_SIGN_COUNTER_OFFSET for an NV ID

    When the device is initialized for the first time, all (GAP_BONDINGS_MAX) NV items are created and
    initialized to all 0xFF's. A bonding record of all 0xFF's indicates that the bonding record is empty
    and free to use.

    The calculation for each bonding records NV IDs:
      mainRecordNvID = ((bondIdx * GAP_BOND_REC_IDS) + BLE_NVID_GAP_BOND_START)
      localLTKNvID = (((bondIdx * GAP_BOND_REC_IDS) + GAP_BOND_LOCAL_LTK_OFFSET) + BLE_NVID_GAP_BOND_START)

*/
#define GAP_BOND_REC_ID_OFFSET              0 //!< NV ID for the main bonding record
#define GAP_BOND_LOCAL_LTK_OFFSET           1 //!< NV ID for the bonding record's local LTK information
#define GAP_BOND_DEV_LTK_OFFSET             2 //!< NV ID for the bonding records' device LTK information
#define GAP_BOND_DEV_IRK_OFFSET             3 //!< NV ID for the bonding records' device IRK
#define GAP_BOND_DEV_CSRK_OFFSET            4 //!< NV ID for the bonding records' device CSRK
#define GAP_BOND_DEV_SIGN_COUNTER_OFFSET    5 //!< NV ID for the bonding records' device Sign Counter

#define GAP_BOND_REC_IDS                    6

// Macros to calculate the index/offset in to NV space
#define calcNvID(Idx, offset)               (((((Idx) * GAP_BOND_REC_IDS) + (offset))) + BLE_NVID_GAP_BOND_START)
#define mainRecordNvID(bondIdx)             (calcNvID((bondIdx), GAP_BOND_REC_ID_OFFSET))
#define localLTKNvID(bondIdx)               (calcNvID((bondIdx), GAP_BOND_LOCAL_LTK_OFFSET))
#define devLTKNvID(bondIdx)                 (calcNvID((bondIdx), GAP_BOND_DEV_LTK_OFFSET))
#define devIRKNvID(bondIdx)                 (calcNvID((bondIdx), GAP_BOND_DEV_IRK_OFFSET))
#define devCSRKNvID(bondIdx)                (calcNvID((bondIdx), GAP_BOND_DEV_CSRK_OFFSET))
#define devSignCounterNvID(bondIdx)         (calcNvID((bondIdx), GAP_BOND_DEV_SIGN_COUNTER_OFFSET))

// Macros to calculate the GATT index/offset in to NV space
#define gattCfgNvID(Idx)                    ((Idx) + BLE_NVID_GATT_CFG_START)

// Key Size Limits
#define MIN_ENC_KEYSIZE                     7  //!< Minimum number of bytes for the encryption key
#define MAX_ENC_KEYSIZE                     16 //!< Maximum number of bytes for the encryption key

/*********************************************************************
    TYPEDEFS
*/

// Structure of NV data for the connected device's encryption information
typedef struct
{
    uint8   LTK[KEYLEN];              // Long Term Key (LTK)
    uint16  div;  //lint -e754        // LTK eDiv
    uint8   rand[B_RANDOM_NUM_SIZE];  // LTK random number
    uint8   keySize;                  // LTK key size
} gapBondLTK_t;

// Structure of NV data for the connected device's address information
typedef struct
{
    uint8   publicAddr[B_ADDR_LEN];     // Master's address
    uint8   reconnectAddr[B_ADDR_LEN];  // Privacy Reconnection Address
    uint16  stateFlags;                 // State flags: SM_AUTH_STATE_AUTHENTICATED & SM_AUTH_STATE_BONDING
} gapBondRec_t;

// Structure of NV data for the connected device's characteristic configuration
typedef struct
{
    uint16 attrHandle;  // attribute handle
    uint8  value;       // attribute value for this device
} gapBondCharCfg_t;

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
static uint8 gapBondMgr_TaskID = INVALID_TASK_ID;   // Task ID for internal task/event processing

// GAPBonding Parameters
uint8 gapBond_PairingMode[MAX_NUM_LL_CONN] = {GAPBOND_PAIRING_MODE_WAIT_FOR_REQ};
static uint16 gapBond_InitiateWait = 1000;  // Default to 1 second
static uint8 gapBond_MITM = FALSE;
static uint8 gapBond_IOCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
static uint8 gapBond_OOBDataFlag = FALSE;
static uint8 gapBond_OOBData[KEYLEN] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static uint8 gapBond_Bonding = FALSE;
// 2020-04-22 support secure connection pairing
static uint8 gapBond_SC = FALSE;
static uint8 gapBond_keyPress = FALSE;
static uint8 gapBond_CT2 = FALSE;
static uint8 gapBond_AutoFail = FALSE;
static uint8 gapBond_AutoFailReason = SMP_PAIRING_FAILED_NOT_SUPPORTED;
static uint8 gapBond_KeyDistList =
    (
        GAPBOND_KEYDIST_SENCKEY     // sEncKey enabled, to send the encryption key
        | GAPBOND_KEYDIST_SIDKEY   // sIdKey enabled, to send the IRK, and BD_ADDR
        | GAPBOND_KEYDIST_SSIGN    // sSign enabled, to send the CSRK
        | GAPBOND_KEYDIST_MENCKEY  // mEncKey enabled, to get the master's encryption key
        | GAPBOND_KEYDIST_MIDKEY   // mIdKey enabled, to get the master's IRK and BD_ADDR
        | GAPBOND_KEYDIST_MSIGN    // mSign enabled, to get the master's CSRK
    );
static uint32 gapBond_Passcode = 0;
static uint8  gapBond_KeySize = MAX_ENC_KEYSIZE;
#if (GAP_BOND_MGR_INDEX_REPLACE)
    static uint8 bondReplaceCnt=0;
#endif
#if ( HOST_CONFIG & CENTRAL_CFG )
    static uint8  gapBond_BondFailOption = GAPBOND_FAIL_TERMINATE_LINK;
#endif

static const gapBondCBs_t* pGapBondCB = NULL;

// Local RAM shadowed bond records
static gapBondRec_t bonds[GAP_BONDINGS_MAX] = {0};

static uint8 autoSyncWhiteList = FALSE;

static uint8 eraseAllBonds = FALSE;

static uint8 bondsToDelete[GAP_BONDINGS_MAX] = {FALSE};

// Globals used for saving bond record and CCC values in NV
static uint8 bondIdx = GAP_BONDINGS_MAX;
static gapAuthCompleteEvent_t* pAuthEvt[MAX_NUM_LL_CONN] = {NULL};
//static gapAuthCompleteEvent_t* pAuthBond[GAP_BONDINGS_MAX] = {NULL};


/*********************************************************************
    LOCAL FUNCTIONS
*/
static uint8 gapBondMgrChangeState( uint8 idx, uint16 state, uint8 set );
static uint8 gapBondMgrUpdateCharCfg( uint8 idx, uint16 attrHandle, uint16 value );
static gapBondCharCfg_t* gapBondMgrFindCharCfgItem( uint16 attrHandle,
                                                    gapBondCharCfg_t* charCfgTbl );
static void gapBondMgrInvertCharCfgItem( gapBondCharCfg_t* charCfgTbl );
static uint8 gapBondMgrAddBond( gapBondRec_t* pBondRec, gapAuthCompleteEvent_t* pPkt );
static uint8 gapBondMgrGetStateFlags( uint8 idx );
static bStatus_t gapBondMgrGetPublicAddr( uint8 idx, uint8* pAddr );
static uint8 gapBondMgrFindReconnectAddr( uint8* pReconnectAddr );
static uint8 gapBondMgrFindAddr( uint8* pDevAddr );
static uint8 gapBondMgrResolvePrivateAddr( uint8* pAddr );
static void gapBondMgrReadBonds( void );
static uint8 gapBondMgrFindEmpty( void );
static uint8 gapBondMgrBondTotal( void );
static bStatus_t gapBondMgrEraseAllBondings( void );
static bStatus_t gapBondMgrEraseBonding( uint8 idx );
static uint8 gapBondMgr_ProcessOSALMsg( osal_event_hdr_t* pMsg );
static void gapBondMgrSendServiceChange( linkDBItem_t* pLinkItem );
static void gapBondMgr_ProcessGATTMsg( gattMsgEvent_t* pMsg );
static void gapBondMgr_ProcessGATTServMsg( gattEventHdr_t* pMsg );
static void gapBondSetupPrivFlag( void );
static void gapBondMgrBondReq( uint16 connHandle, uint8 idx, uint8 stateFlags,
                               uint8 role, uint8 startEncryption );
static void gapBondMgrAuthenticate( uint16 connHandle, uint8 addrType,
                                    gapPairingReq_t* pPairReq );
static void gapBondMgr_SyncWhiteList( void );
static uint8 gapBondMgr_SyncCharCfg( uint16 connHandle );
static void gapBondFreeAuthEvt( uint16 connHandle );

static void gapBondRecvEvt(uint16 connHandle,gapBondRec_t* pBondRec, gapAuthCompleteEvent_t* pPkt );

#if ( HOST_CONFIG & PERIPHERAL_CFG )
    static void gapBondMgrSlaveSecurityReq( uint16 connHandle );
#endif

/*********************************************************************
    NETWORK LAYER CALLBACKS
*/

/*********************************************************************
    PUBLIC FUNCTIONS
*/

/*********************************************************************
    @brief   Set a GAP Bond Manager parameter.

    Public function defined in gapbondmgr.h.
*/
bStatus_t GAPBondMgr_SetParameter( uint16 param, uint8 len, void* pValue )
{
    bStatus_t ret = SUCCESS;  // return value

    switch ( param )
    {
    case GAPBOND_PAIRING_MODE:
        if ( (len == sizeof ( uint8 )) && (*((uint8*)pValue) <= GAPBOND_PAIRING_MODE_INITIATE) )
        {
//            gapBond_PairingMode = *((uint8*)pValue);
            osal_memset(gapBond_PairingMode,*((uint8*)pValue),sizeof(gapBond_PairingMode));
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPBOND_INITIATE_WAIT:
        if ( len == sizeof ( uint16 ) )
        {
            gapBond_InitiateWait = *((uint16*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPBOND_MITM_PROTECTION:
        if ( (len == sizeof ( uint8 )) && (*((uint8*)pValue) <= TRUE) )
        {
            gapBond_MITM = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPBOND_IO_CAPABILITIES:
        if ( (len == sizeof ( uint8 )) && (*((uint8*)pValue) <= GAPBOND_IO_CAP_KEYBOARD_DISPLAY) )
        {
            gapBond_IOCap = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPBOND_OOB_ENABLED:
        if ( (len == sizeof ( uint8 )) && (*((uint8*)pValue) <= TRUE) )
        {
            gapBond_OOBDataFlag = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPBOND_OOB_DATA:
        if ( len == KEYLEN )
        {
            VOID osal_memcpy( gapBond_OOBData, pValue, KEYLEN ) ;
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPBOND_BONDING_ENABLED:
        if ( (len == sizeof ( uint8 )) && (*((uint8*)pValue) <= TRUE) )
        {
            gapBond_Bonding = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPBOND_KEY_DIST_LIST:
        if ( len == sizeof ( uint8 ) )
        {
            gapBond_KeyDistList = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPBOND_DEFAULT_PASSCODE:
        if ( (len == sizeof ( uint32 ))
                && (*((uint32*)pValue) <= GAP_PASSCODE_MAX) )
        {
            gapBond_Passcode = *((uint32*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPBOND_ERASE_ALLBONDS:
        if ( len == 0 )
        {
            // Make sure there's no active connection
            if ( GAP_NumActiveConnections() == 0 )
            {
                // Erase all bonding records
                VOID gapBondMgrEraseAllBondings();
                // See if NV needs a compaction
                VOID osal_snv_compact( NV_COMPACT_THRESHOLD );
                // Make sure Bond RAM Shadow is up-to-date
                gapBondMgrReadBonds();
            }
            else
            {
                eraseAllBonds = TRUE;
            }
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPBOND_ERASE_SINGLEBOND:
        if ( len == (1 + B_ADDR_LEN) )
        {
            uint8 idx;
            uint8 devAddr[B_ADDR_LEN];
            // Reverse bytes
            VOID osal_revmemcpy( devAddr, (uint8*)pValue+1, B_ADDR_LEN );
            // Resolve address and find index
            idx = GAPBondMgr_ResolveAddr( *((uint8*)pValue), devAddr, NULL );

            if ( idx < GAP_BONDINGS_MAX )
            {
                // Make sure there's no active connection
                if ( GAP_NumActiveConnections() == 0 )
                {
                    // Erase bond
                    gapBondMgrEraseBonding( idx );
                    // See if NV needs a compaction
                    VOID osal_snv_compact( NV_COMPACT_THRESHOLD );
                    // Make sure Bond RAM Shadow is up-to-date
                    gapBondMgrReadBonds();
                }
                else
                {
                    // Mark entry to be deleted when disconnected
                    bondsToDelete[idx] = TRUE;
                }
            }
            else
            {
                ret = INVALIDPARAMETER;
            }
        }
        else
        {
            // Parameter is not the correct length
            ret = bleInvalidRange;
        }

        break;

    case GAPBOND_AUTO_FAIL_PAIRING:
        if ( (len == sizeof ( uint8 )) && (*((uint8*)pValue) <= TRUE) )
        {
            gapBond_AutoFail = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPBOND_AUTO_FAIL_REASON:
        if ( (len == sizeof ( uint8 )) && (*((uint8*)pValue) <= SMP_PAIRING_FAILED_REPEATED_ATTEMPTS) )
        {
            gapBond_AutoFailReason = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPBOND_KEYSIZE:
        if ( (len == sizeof ( uint8 ))
                && ((*((uint8*)pValue) >= MIN_ENC_KEYSIZE) && (*((uint8*)pValue) <= MAX_ENC_KEYSIZE)) )
        {
            gapBond_KeySize = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;

    case GAPBOND_AUTO_SYNC_WL:
        if ( len == sizeof( uint8 ) )
        {
            uint8 oldVal = autoSyncWhiteList;
            autoSyncWhiteList = *((uint8*)pValue);

            // only call if parameter changes from FALSE to TRUE
            if ( ( oldVal == FALSE ) && ( autoSyncWhiteList == TRUE ) )
            {
                // make sure bond is updated from NV
                gapBondMgrReadBonds();
            }
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;
#if ( HOST_CONFIG & CENTRAL_CFG )

    case GAPBOND_BOND_FAIL_ACTION:
        if ( (len == sizeof ( uint8 )) && (*((uint8*)pValue) <= GAPBOND_FAIL_TERMINATE_ERASE_BONDS) )
        {
            gapBond_BondFailOption = *((uint8*)pValue);
        }
        else
        {
            ret = bleInvalidRange;
        }

        break;
#endif

    default:

        // The param value isn't part of this profile, try the GAP.
        if ( (param < TGAP_PARAMID_MAX) && (len == sizeof ( uint16 )) )
        {
            ret = GAP_SetParamValue( param, *((uint16*)pValue) );
        }
        else
        {
            ret = INVALIDPARAMETER;
        }

        break;
    }

    return ( ret );
}

/*********************************************************************
    @brief   Get a GAP Bond Manager parameter.

    Public function defined in gapbondmgr.h.
*/
bStatus_t GAPBondMgr_GetParameter( uint16 param, void* pValue )
{
    bStatus_t ret = SUCCESS;  // return value

    switch ( param )
    {
    case GAPBOND_PAIRING_MODE:
//        *((uint8*)pValue) = gapBond_PairingMode;
        break;

    case GAPBOND_INITIATE_WAIT:
        *((uint16*)pValue) = gapBond_InitiateWait;
        break;

    case GAPBOND_MITM_PROTECTION:
        *((uint8*)pValue) = gapBond_MITM;
        break;

    case GAPBOND_IO_CAPABILITIES:
        *((uint8*)pValue) = gapBond_IOCap;
        break;

    case GAPBOND_OOB_ENABLED:
        *((uint8*)pValue) = gapBond_OOBDataFlag;
        break;

    case GAPBOND_OOB_DATA:
        VOID osal_memcpy( pValue, gapBond_OOBData, KEYLEN ) ;
        break;

    case GAPBOND_BONDING_ENABLED:
        *((uint8*)pValue) = gapBond_Bonding;
        break;

    case GAPBOND_KEY_DIST_LIST:
        *((uint8*)pValue) = gapBond_KeyDistList;
        break;

    case GAPBOND_DEFAULT_PASSCODE:
        *((uint32*)pValue) = gapBond_Passcode;
        break;

    case GAPBOND_AUTO_FAIL_PAIRING:
        *((uint8*)pValue) = gapBond_AutoFail;
        break;

    case GAPBOND_AUTO_FAIL_REASON:
        *((uint8*)pValue) = gapBond_AutoFailReason;
        break;

    case GAPBOND_KEYSIZE:
        *((uint8*)pValue) = gapBond_KeySize;
        break;

    case GAPBOND_AUTO_SYNC_WL:
        *((uint8*)pValue) = autoSyncWhiteList;
        break;

    case GAPBOND_BOND_COUNT:
        *((uint8*)pValue) = gapBondMgrBondTotal();
        break;

    default:

        // The param value isn't part of this profile, try the GAP.
        if ( param < TGAP_PARAMID_MAX )
        {
            *((uint16*)pValue) = GAP_GetParamValue( param );
        }
        else
        {
            ret = INVALIDPARAMETER;
        }

        break;
    }

    return ( ret );
}

/*********************************************************************
    @brief   Notify the Bond Manager that a connection has been made.

    Public function defined in gapbondmgr.h.
*/
bStatus_t GAPBondMgr_LinkEst( uint8 addrType, uint8* pDevAddr, uint16 connHandle, uint8 role )
{
    uint8 idx;                          // NV Index
    uint8 publicAddr[B_ADDR_LEN]        // Place to put the public address
        = {0, 0, 0, 0, 0, 0};
    idx = GAPBondMgr_ResolveAddr( addrType, pDevAddr, publicAddr );

    if ( idx < GAP_BONDINGS_MAX )
    {
        uint8 stateFlags = gapBondMgrGetStateFlags( idx );
        smSigningInfo_t signingInfo;
        gapBondCharCfg_t charCfg[GAP_CHAR_CFG_MAX]; // Space to read a char cfg record from NV
        // On peripheral, load the key information for the bonding
        // On central and initiaiting security, load key to initiate encyption
//        gapBondMgrBondReq( connHandle, idx, stateFlags, role,
//                           ((gapBond_PairingMode[connHandle] == GAPBOND_PAIRING_MODE_INITIATE ) ? TRUE : FALSE) );
        // bugfix : for multi-role , because GAP_Bond gapProfileRole & central to initiate bond request
        // if slave , shall not send bond request
        gapBondMgrBondReq( connHandle, idx, stateFlags, role,
                           ((role == GAP_PROFILE_CENTRAL ) ? TRUE : FALSE) );
        // Load the Signing Key
        VOID osal_memset( &signingInfo, 0, sizeof ( smSigningInfo_t ) );

        if ( osal_snv_read( devCSRKNvID(idx), KEYLEN, signingInfo.srk ) == SUCCESS )
        {
            if ( osal_isbufset( signingInfo.srk, 0xFF, KEYLEN ) == FALSE )
            {
                // Load the signing information for this connection
                VOID osal_snv_read( devSignCounterNvID(idx), sizeof ( uint32 ), &(signingInfo.signCounter) );
                VOID GAP_Signable( connHandle,
                                   ((stateFlags & GAP_BONDED_STATE_AUTHENTICATED) ? TRUE : FALSE),
                                   &signingInfo );
            }
        }

        // Load the characteristic configuration
        if ( osal_snv_read( gattCfgNvID(idx), sizeof ( charCfg ), charCfg ) == SUCCESS )
        {
            gapBondMgrInvertCharCfgItem( charCfg );

            for ( uint8 i = 0; i < GAP_CHAR_CFG_MAX; i++ )
            {
                gapBondCharCfg_t* pItem = &(charCfg[i]);

                // Apply the characteristic configuration for this connection
                if ( pItem->attrHandle != GATT_INVALID_HANDLE )
                {
                    VOID GATTServApp_UpdateCharCfg( connHandle, pItem->attrHandle,
                                                    (uint16)(pItem->value) );
                }
            }
        }

        // Has there been a service change?
        if ( stateFlags & GAP_BONDED_STATE_SERVICE_CHANGED )
        {
            VOID GATTServApp_SendServiceChangedInd( connHandle, gapBondMgr_TaskID );
        }
    }

#if ( HOST_CONFIG & CENTRAL_CFG )
    else if ( role == GAP_PROFILE_CENTRAL &&
              gapBond_PairingMode[connHandle] == GAPBOND_PAIRING_MODE_INITIATE )
    {
        // If Central and initiating and not bonded, then initiate pairing
        gapBondMgrAuthenticate( connHandle, addrType, NULL );

        // Call app state callback
        if ( pGapBondCB && pGapBondCB->pairStateCB )
        {
            pGapBondCB->pairStateCB( connHandle, GAPBOND_PAIRING_STATE_STARTED, SUCCESS );
        }
    }

#endif
#if ( HOST_CONFIG & PERIPHERAL_CFG )

    // If Peripheral and initiating, send a slave security request to
    // initiate either pairing or encryption
    if ( role == GAP_PROFILE_PERIPHERAL &&
            gapBond_PairingMode[connHandle] == GAPBOND_PAIRING_MODE_INITIATE )
    {
        gapBondMgrSlaveSecurityReq( connHandle );
    }

#endif
    return ( SUCCESS );
}

/*********************************************************************
    @brief   Resolve an address from bonding information.

    Public function defined in gapbondmgr.h.
*/
uint8 GAPBondMgr_ResolveAddr( uint8 addrType, uint8* pDevAddr, uint8* pResolvedAddr )
{
    uint8 idx = GAP_BONDINGS_MAX;

    switch ( addrType )
    {
    case ADDRTYPE_PUBLIC:
    case ADDRTYPE_STATIC:
        idx = gapBondMgrFindAddr( pDevAddr );

        if ( (idx < GAP_BONDINGS_MAX) && (pResolvedAddr) )
        {
            VOID osal_memcpy( pResolvedAddr, pDevAddr, B_ADDR_LEN );
        }

        break;

    case ADDRTYPE_PRIVATE_NONRESOLVE:
        // This could be a reconnection address
        idx = gapBondMgrFindReconnectAddr( pDevAddr );

        if ( (idx < GAP_BONDINGS_MAX) && (pResolvedAddr) )
        {
            VOID gapBondMgrGetPublicAddr( idx, pResolvedAddr );
        }

        break;

    case ADDRTYPE_PRIVATE_RESOLVE:
        // Master's don't use Private Resolvable addresses but just in case
        idx = gapBondMgrResolvePrivateAddr( pDevAddr );

        if ( (idx < GAP_BONDINGS_MAX) && (pResolvedAddr) )
        {
            VOID gapBondMgrGetPublicAddr( idx, pResolvedAddr );
        }

        break;

    default:
        break;
    }

    return ( idx );
}

/*********************************************************************
    @brief   Set/clear the service change indication in a bond record.

    Public function defined in gapbondmgr.h.
*/
bStatus_t GAPBondMgr_ServiceChangeInd( uint16 connectionHandle, uint8 setParam )
{
    bStatus_t ret = bleNoResources; // return value

    if ( connectionHandle == 0xFFFF )
    {
        uint8 idx;  // loop counter

        // Run through the bond database and update the Service Change indication
        for ( idx = 0; idx < GAP_BONDINGS_MAX; idx++ )
        {
            if ( gapBondMgrChangeState( idx, GAP_BONDED_STATE_SERVICE_CHANGED, setParam ) )
            {
                ret = SUCCESS;
            }
        }

        // If the service change indication is TRUE, tell the connected devices
        if ( setParam )
        {
            // Run connected database
            linkDB_PerformFunc( gapBondMgrSendServiceChange );
        }
    }
    else
    {
        // Find connection information
        linkDBItem_t* pLinkItem = linkDB_Find( connectionHandle );

        if ( pLinkItem )
        {
            uint8 idx; // loop counter
            idx = GAPBondMgr_ResolveAddr( pLinkItem->addrType, pLinkItem->addr, NULL );

            if ( idx < GAP_BONDINGS_MAX )
            {
                // Bond found, update it.
                VOID gapBondMgrChangeState( idx, GAP_BONDED_STATE_SERVICE_CHANGED, setParam );
                ret = SUCCESS;
            }

            // If the service change indication is TRUE, tell the connected device
            if ( setParam )
            {
                gapBondMgrSendServiceChange( pLinkItem );
            }
        }
        else
        {
            ret = bleNotConnected;
        }
    }

    return ( ret );
}

/*********************************************************************
    @brief   Update the Characteristic Configuration in a bond record.

    Public function defined in gapbondmgr.h.
*/
bStatus_t GAPBondMgr_UpdateCharCfg( uint16 connectionHandle, uint16 attrHandle, uint16 value )
{
    bStatus_t ret = bleNoResources; // return value

    if ( connectionHandle == INVALID_CONNHANDLE )
    {
        uint8 idx;  // loop counter

        // Run through the bond database and update the Characteristic Configuration
        for ( idx = 0; idx < GAP_BONDINGS_MAX; idx++ )
        {
            if ( gapBondMgrUpdateCharCfg( idx, attrHandle, value ) )
            {
                ret = SUCCESS;
            }
        }
    }
    else
    {
        // Find connection information
        linkDBItem_t* pLinkItem = linkDB_Find( connectionHandle );

        if ( pLinkItem )
        {
            uint8 idx = GAPBondMgr_ResolveAddr( pLinkItem->addrType, pLinkItem->addr, NULL );

            if ( idx < GAP_BONDINGS_MAX )
            {
                // Bond found, update it.
                VOID gapBondMgrUpdateCharCfg( idx, attrHandle, value );
                ret = SUCCESS;
            }
        }
        else
        {
            ret = bleNotConnected;
        }
    }

    return ( ret );
}

/*********************************************************************
    @brief   Register callback functions with the bond manager.

    Public function defined in gapbondmgr.h.
*/
void GAPBondMgr_Register( gapBondCBs_t* pCB )
{
    pGapBondCB = pCB;

    if(gapBondMgr_TaskID != INVALID_TASK_ID)
    {
        // Take over the processing of Authentication messages
        VOID GAP_SetParamValue( TGAP_AUTH_TASK_ID, gapBondMgr_TaskID );
        // Register with GATT Server App for event messages
        GATTServApp_RegisterForMsg( gapBondMgr_TaskID );
    }
}

/*********************************************************************
    @brief   Respond to a passcode request.

    Public function defined in gapbondmgr.h.
*/
bStatus_t GAPBondMgr_PasscodeRsp( uint16 connectionHandle, uint8 status, uint32 passcode )
{
    bStatus_t ret = SUCCESS;

    if ( status == SUCCESS )
    {
        // Truncate the passcode
        passcode = passcode % (GAP_PASSCODE_MAX + 1);
        ret = GAP_PasscodeUpdate( passcode, connectionHandle );

        if ( ret != SUCCESS )
        {
            VOID GAP_TerminateAuth( connectionHandle, SMP_PAIRING_FAILED_PASSKEY_ENTRY_FAILED );
        }
    }
    else
    {
        VOID GAP_TerminateAuth( connectionHandle, status );
    }

    return ret;
}

/*********************************************************************
    @brief   This is a bypass mechanism to allow the bond manager to process
                GAP messages.

    Public function defined in gapbondmgr.h.
*/
uint8 GAPBondMgr_ProcessGAPMsg( gapEventHdr_t* pMsg )
{
    switch ( pMsg->opcode )
    {
    case GAP_PASSKEY_NEEDED_EVENT:
    {
        gapPasskeyNeededEvent_t* pPkt = (gapPasskeyNeededEvent_t*)pMsg;

        if ( pGapBondCB && pGapBondCB->passcodeCB )
        {
            // Ask app for a passcode
            pGapBondCB->passcodeCB( pPkt->deviceAddr, pPkt->connectionHandle, pPkt->uiInputs, pPkt->uiOutputs );
        }
        else
        {
            // No app support, use the default passcode
            if ( GAP_PasscodeUpdate( gapBond_Passcode, pPkt->connectionHandle ) != SUCCESS )
            {
                VOID GAP_TerminateAuth( pPkt->connectionHandle, SMP_PAIRING_FAILED_PASSKEY_ENTRY_FAILED );
            }
        }
    }
    break;

    case GAP_AUTHENTICATION_COMPLETE_EVENT:
    {
        gapAuthCompleteEvent_t* pPkt = (gapAuthCompleteEvent_t*)pMsg;

        // Should we save bonding information (one save at a time)
        if ( (pPkt->hdr.status == SUCCESS)             &&
                (pPkt->authState & SM_AUTH_STATE_BONDING) &&
                (pAuthEvt[pPkt->connectionHandle] == NULL) )
        {
            gapBondRec_t bondRec;
            VOID osal_memset( &bondRec, 0, sizeof ( gapBondRec_t ) ) ;

            // Do we have a public address in the data?
            if ( pPkt->pIdentityInfo )
            {
                VOID osal_memcpy( bondRec.publicAddr, pPkt->pIdentityInfo->bd_addr, B_ADDR_LEN );
            }
            else
            {
                linkDBItem_t* pLinkItem = linkDB_Find( pPkt->connectionHandle );

                if ( pLinkItem )
                {
                    VOID osal_memcpy( bondRec.publicAddr, pLinkItem->addr, B_ADDR_LEN );
                }
                else
                {
                    // We don't have an address, so ignore the message.
                    break;
                }
            }

            // Save off of the authentication state
            bondRec.stateFlags |= (pPkt->authState & SM_AUTH_STATE_AUTHENTICATED) ? GAP_BONDED_STATE_AUTHENTICATED : 0;

            if ( !gapBondMgrAddBond( &bondRec, pPkt ) )
            {
                // Notify our task to save bonding information in NV
                gapBondRecvEvt( pPkt->connectionHandle,&bondRec, pPkt );
//            osal_set_event( gapBondMgr_TaskID, GAP_BOND_SAVE_REC_EVT );
//              AT_LOG("osal_set_event( gapBondMgr_TaskID, GAP_BOND_SAVE_REC_EVT )\n");
                // We're not done with this message; it will be freed later
                return ( FALSE );
            }
        }

        // Call app state callback in the fail case. Success is handled after GAP_BOND_SAVE_REC_EVT.
        if ( pGapBondCB && pGapBondCB->pairStateCB )
        {
            pGapBondCB->pairStateCB( pPkt->connectionHandle, GAPBOND_PAIRING_STATE_COMPLETE, pPkt->hdr.status );
        }
    }
    break;

    case GAP_BOND_COMPLETE_EVENT:
        // This message is received when the bonding is complete.  If hdr.status is SUCCESS
        // then call app state callback.  If hdr.status is NOT SUCCESS, the connection will be
        // dropped at the LL because of a MIC failure, so again nothing to do.
    {
        gapBondCompleteEvent_t* pPkt = (gapBondCompleteEvent_t*)pMsg;
#if ( HOST_CONFIG & CENTRAL_CFG )

        if ( pPkt->hdr.status == LL_ENC_KEY_REQ_REJECTED )
        {
            // LTK not found on peripheral device (Pin or Key Missing). See which
            // option was configured for unsuccessful bonding.
            linkDBItem_t* pLinkItem = linkDB_Find( pPkt->connectionHandle );

            if ( pLinkItem )
            {
                switch ( gapBond_BondFailOption )
                {
                case GAPBOND_FAIL_INITIATE_PAIRING:
                    // Initiate pairing
                    gapBondMgrAuthenticate( pPkt->connectionHandle, pLinkItem->addrType, NULL );
                    break;

                case GAPBOND_FAIL_TERMINATE_LINK:
                    // Drop connection
                    GAP_TerminateLinkReq( pLinkItem->taskID, pPkt->connectionHandle, HCI_DISCONNECT_AUTH_FAILURE );
                    break;

                case GAPBOND_FAIL_TERMINATE_ERASE_BONDS:
                    // Set up bond manager to erase all existing bonds after connection terminates
                    VOID GAPBondMgr_SetParameter( GAPBOND_ERASE_ALLBONDS, 0, NULL );
                    // Drop connection
                    GAP_TerminateLinkReq( pLinkItem->taskID, pPkt->connectionHandle, HCI_DISCONNECT_AUTH_FAILURE );
                    break;

                case GAPBOND_FAIL_NO_ACTION:

                // fall through
                default:
                    // do nothing
                    break;
                }
            }
        }

#endif

        if ( pGapBondCB && pGapBondCB->pairStateCB )
        {
            pGapBondCB->pairStateCB( pPkt->connectionHandle, GAPBOND_PAIRING_STATE_BONDED, pMsg->hdr.status );
        }
    }
    break;

    case GAP_SIGNATURE_UPDATED_EVENT:
    {
        uint8 idx;
        gapSignUpdateEvent_t* pPkt = (gapSignUpdateEvent_t*)pMsg;
        idx = GAPBondMgr_ResolveAddr( pPkt->addrType, pPkt->devAddr, NULL );

        if ( idx < GAP_BONDINGS_MAX )
        {
            // Save the sign counter
            VOID osal_snv_write( devSignCounterNvID(idx), sizeof ( uint32 ), &(pPkt->signCounter) );
        }
    }
    break;
#if ( HOST_CONFIG & PERIPHERAL_CFG )

    case GAP_PAIRING_REQ_EVENT:
    {
        gapPairingReqEvent_t* pPkt = (gapPairingReqEvent_t*)pMsg;

        if ( gapBond_AutoFail != FALSE )
        {
            // Auto Fail TEST MODE (DON'T USE THIS) - Sends pre-setup reason
            VOID GAP_TerminateAuth( pPkt->connectionHandle, gapBond_AutoFailReason );
        }
        else if ( gapBond_PairingMode[pPkt->connectionHandle] == GAPBOND_PAIRING_MODE_NO_PAIRING )
        {
            // No Pairing - Send error
            VOID GAP_TerminateAuth( pPkt->connectionHandle, SMP_PAIRING_FAILED_NOT_SUPPORTED );
        }
        else
        {
            linkDBItem_t* pLinkItem = linkDB_Find( pPkt->connectionHandle );

            // Requesting bonding?
            if ( pPkt->pairReq.authReq & SM_AUTH_STATE_BONDING )
            {
                if ( pLinkItem )
                {
                    if ( (pLinkItem->addrType != ADDRTYPE_PUBLIC) && (pPkt->pairReq.keyDist.mIdKey == FALSE) )
                    {
                        uint8 publicAddr[B_ADDR_LEN];

                        // Check if we already have the public address in NV
                        if ( GAPBondMgr_ResolveAddr(pLinkItem->addrType, pLinkItem->addr, publicAddr ) == FALSE )
                        {
                            // Can't bond to a non-public address if we don't know the public address
                            VOID GAP_TerminateAuth( pPkt->connectionHandle, SMP_PAIRING_FAILED_AUTH_REQ );
                            break;
                        }
                    }
                }
                else
                {
                    // Can't find the connection, ignore the message
                    break;
                }
            }

            // Send pairing response
            gapBondMgrAuthenticate( pPkt->connectionHandle, pLinkItem->addrType, &(pPkt->pairReq) );

            // Call app state callback
            if ( pGapBondCB && pGapBondCB->pairStateCB )
            {
                pGapBondCB->pairStateCB( pPkt->connectionHandle, GAPBOND_PAIRING_STATE_STARTED, SUCCESS );
            }
        }
    }
    break;
#endif
#if ( HOST_CONFIG & CENTRAL_CFG )

    case GAP_SLAVE_REQUESTED_SECURITY_EVENT:
    {
        uint16 connHandle = ((gapSlaveSecurityReqEvent_t*)pMsg)->connectionHandle;
        uint8 idx;
        uint8 publicAddr[B_ADDR_LEN] = {0, 0, 0, 0, 0, 0};
        linkDBItem_t* pLink = linkDB_Find( connHandle );

        // If link found and not already initiating security
        if (pLink != NULL && gapBond_PairingMode[connHandle] != GAPBOND_PAIRING_MODE_INITIATE)
        {
            // If already bonded initiate encryption
            idx = GAPBondMgr_ResolveAddr( pLink->addrType, pLink->addr, publicAddr );

            if ( idx < GAP_BONDINGS_MAX )
            {
                gapBondMgrBondReq( connHandle, idx, gapBondMgrGetStateFlags( idx ),
                                   GAP_PROFILE_CENTRAL, TRUE );
            }
            else if ( gapBond_PairingMode[connHandle] == GAPBOND_PAIRING_MODE_NO_PAIRING )
            {
                VOID GAP_TerminateAuth( connHandle, SMP_PAIRING_FAILED_NOT_SUPPORTED );
            }
            else if (gapBond_PairingMode[connHandle] == GAPBOND_PAIRING_MODE_WAIT_FOR_REQ)
            {
                gapBondMgrAuthenticate( connHandle, pLink->addrType, NULL );
            }
        }
    }
    break;
#endif

    case GAP_LINK_TERMINATED_EVENT:
    {
//        gapTerminateLinkEvent_t* pPkt = (gapTerminateLinkEvent_t*)pMsg;
//        gapAuthCompleteEvent_t* pAuthEvtTmp = NULL;
        if ( GAP_NumActiveConnections() == 0 )
        {
            // See if we're asked to erase all bonding records
            if ( eraseAllBonds == TRUE )
            {
                VOID gapBondMgrEraseAllBondings();
                eraseAllBonds = FALSE;
                // Reset bonds to delete table
                osal_memset( bondsToDelete, FALSE, sizeof( bondsToDelete ) );
            }
            // See if we're asked to erase all bonding records
            // Reset bonds to delete table
            else
            {
                // See if we're asked to erase any single bonding records
                for (uint8 idx = 0; idx < GAP_BONDINGS_MAX; idx++)
                {
                    if ( bondsToDelete[idx] == TRUE )
                    {
                        VOID gapBondMgrEraseBonding( idx );
                        bondsToDelete[idx] = FALSE;
                    }
                }
            }

            // See if NV needs a compaction
            VOID osal_snv_compact( NV_COMPACT_THRESHOLD );
            // Make sure Bond RAM Shadow is up-to-date
            // Make sure Bond RAM Shadow is up-to-date
            gapBondMgrReadBonds();
        }
    }
    break;

    default:
        break;
    }

    return ( TRUE );
}

/*********************************************************************
    LOCAL FUNCTION PROTOTYPES
*/

/*********************************************************************
    @fn      gapBondMgrChangeState

    @brief   Change a state flag in the stateFlags field of the bond record.

    @param   idx - Bond NV index
    @param   state - state flage to set or clear
    @param   set - TRUE to set the flag, FALSE to clear the flag

    @return  TRUE if NV Record exists, FALSE if NV Record is empty
*/
static uint8 gapBondMgrChangeState( uint8 idx, uint16 state, uint8 set )
{
    gapBondRec_t bondRec;   // Space to read a Bond record from NV

    // Look for public address that is used (not all 0xFF's)
    if ( (osal_snv_read( mainRecordNvID(idx), sizeof ( gapBondRec_t ), &bondRec ) == SUCCESS)
            && (osal_isbufset( bondRec.publicAddr, 0xFF, B_ADDR_LEN ) == FALSE) )
    {
        // Update the state of the bonded device.
        uint8 stateFlags = bondRec.stateFlags;

        if ( set )
        {
            stateFlags |= state;
        }
        else
        {
            stateFlags &= ~(state);
        }

        if ( stateFlags != bondRec.stateFlags )
        {
            bondRec.stateFlags = stateFlags;
            VOID osal_snv_write( mainRecordNvID(idx), sizeof ( gapBondRec_t ), &bondRec );
        }

        return ( TRUE );
    }

    return ( FALSE );
}

/*********************************************************************
    @fn      gapBondMgrUpdateCharCfg

    @brief   Update the Characteristic Configuration of the bond record.

    @param   idx - Bond NV index
    @param   attrHandle - attribute handle (0 means all handles)
    @param   value - characteristic configuration value

    @return  TRUE if NV Record exists, FALSE if NV Record is empty
*/
static uint8 gapBondMgrUpdateCharCfg( uint8 idx, uint16 attrHandle, uint16 value )
{
    gapBondRec_t bondRec;   // Space to read a Bond record from NV

    // Look for public address that is used (not all 0xFF's)
    if ( ( osal_snv_read( mainRecordNvID(idx), sizeof ( gapBondRec_t ), &bondRec ) == SUCCESS )
            && ( osal_isbufset( bondRec.publicAddr, 0xFF, B_ADDR_LEN ) == FALSE ) )
    {
        gapBondCharCfg_t charCfg[GAP_CHAR_CFG_MAX]; // Space to read a char cfg record from NV

        if ( osal_snv_read( gattCfgNvID(idx), sizeof ( charCfg ), charCfg ) == SUCCESS )
        {
            uint8 update = FALSE;
            gapBondMgrInvertCharCfgItem( charCfg );

            if ( attrHandle == GATT_INVALID_HANDLE )
            {
                if ( osal_isbufset( (uint8*)charCfg, 0x00, sizeof ( charCfg ) ) == FALSE )
                {
                    // Clear all characteristic configuration for this device
                    VOID osal_memset( (void*)charCfg, 0x00, sizeof ( charCfg ) );
                    update = TRUE;
                }
            }
            else
            {
                gapBondCharCfg_t* pItem = gapBondMgrFindCharCfgItem( attrHandle, charCfg );

                if ( pItem == NULL )
                {
                    // Must be a new item; ignore if the value is no operation (default)
                    if ( ( value == GATT_CFG_NO_OPERATION ) ||
                            ( ( pItem = gapBondMgrFindCharCfgItem( GATT_INVALID_HANDLE, charCfg ) ) == NULL ) )
                    {
                        return ( FALSE ); // No empty entry found
                    }

                    pItem->attrHandle = attrHandle;
                }

                if ( pItem->value != value )
                {
                    // Update characteristic configuration
                    pItem->value = (uint8)value;

                    if ( value == GATT_CFG_NO_OPERATION )
                    {
                        // Erease the item
                        pItem->attrHandle = GATT_INVALID_HANDLE;
                    }

                    update = TRUE;
                }
            }

            // Update the characteristic configuration of the bonded device.
            if ( update )
            {
                gapBondMgrInvertCharCfgItem( charCfg );
                VOID osal_snv_write( gattCfgNvID(idx), sizeof( charCfg ), charCfg );
            }
        }

        return ( TRUE );
    }

    return ( FALSE );
}

/*********************************************************************
    @fn      gapBondMgrFindCharCfgItem

    @brief   Find the Characteristic Configuration for a given attribute.
            Uses the attribute handle to search the charactersitic
            configuration table of a bonded device.

    @param   attrHandle - attribute handle.
    @param   charCfgTbl - characteristic configuration table.

    @return  pointer to the found item. NULL, otherwise.
*/
static gapBondCharCfg_t* gapBondMgrFindCharCfgItem( uint16 attrHandle,
                                                    gapBondCharCfg_t* charCfgTbl )
{
    for ( uint8 i = 0; i < GAP_CHAR_CFG_MAX; i++ )
    {
        if ( charCfgTbl[i].attrHandle == attrHandle )
        {
            return ( &(charCfgTbl[i]) );
        }
    }

    return ( (gapBondCharCfg_t*)NULL );
}

/*********************************************************************
    @fn      gapBondMgrFindCharCfgItem

    @brief   Invert the Characteristic Configuration for a given client.

    @param   charCfgTbl - characteristic configuration table.

    @return  none.
*/
static void gapBondMgrInvertCharCfgItem( gapBondCharCfg_t* charCfgTbl )
{
    for ( uint8 i = 0; i < GAP_CHAR_CFG_MAX; i++ )
    {
        charCfgTbl[i].attrHandle = ~(charCfgTbl[i].attrHandle);
        charCfgTbl[i].value = ~(charCfgTbl[i].value);
    }
}

/*********************************************************************
    @fn      gapBondMgrAddBond

    @brief   Save a bond from a GAP Auth Complete Event

    @param   pBondRec - basic bond record
    @param   pLocalLTK - LTK used by this device during pairing
    @param   pDevLTK - LTK used by the connected device during pairing
    @param   pIRK - IRK used by the connected device during pairing
    @param   pSRK - SRK used by the connected device during pairing
    @param   signCounter - Sign counter used by the connected device during pairing

    @return  TRUE, if done processing bond record. FALSE, otherwise.
*/
static uint8 gapBondMgrAddBond( gapBondRec_t* pBondRec, gapAuthCompleteEvent_t* pPkt )
{
    // See if this is a new bond record
    if ( pAuthEvt[pPkt->connectionHandle] == NULL )
    {
        // Make sure we have bonding info
        if ( ( pBondRec == NULL ) || ( pPkt == NULL ) )
        {
            return ( TRUE );
        }

        // First see if we already have an existing bond for this device
        bondIdx = gapBondMgrFindAddr( pBondRec->publicAddr );

        if ( bondIdx >= GAP_BONDINGS_MAX )
        {
            bondIdx = gapBondMgrFindEmpty();
        }

#if(GAP_BOND_MGR_INDEX_REPLACE)

        /*replace bondIdx*/
        if(bondIdx==GAP_BONDINGS_MAX)
        {
            bondIdx = (bondReplaceCnt++)%GAP_BONDINGS_MAX;
        }

#endif
    }

    if ( bondIdx < GAP_BONDINGS_MAX )
    {
        // See if this is a new bond record
        if ( pAuthEvt[pPkt->connectionHandle] == NULL )
        {
            gapBondCharCfg_t charCfg[GAP_CHAR_CFG_MAX];
            // Save the main information
            osal_snv_write( mainRecordNvID(bondIdx), sizeof ( gapBondRec_t ), pBondRec );
            // Write out FF's over the charactersitic configuration entry, to overwrite
            // any previous bond data that may have been stored
            VOID osal_memset( charCfg, 0xFF, sizeof ( charCfg ) );
            VOID osal_snv_write( gattCfgNvID(bondIdx), sizeof ( charCfg ), charCfg );
            // Update Bond RAM Shadow just with the newly added bond entry
            VOID osal_memcpy( &(bonds[bondIdx]), pBondRec, sizeof ( gapBondRec_t ) );
            // Keep the OSAL message to store the security keys later - will be freed then
            pAuthEvt[pPkt->connectionHandle] = pPkt;
        }
        else
        {
            // If available, save the LTK information
            if ( pAuthEvt[pPkt->connectionHandle]->pSecurityInfo )
            {
                VOID osal_snv_write( localLTKNvID(bondIdx), sizeof ( gapBondLTK_t ), pAuthEvt[pPkt->connectionHandle]->pSecurityInfo );
                pAuthEvt[pPkt->connectionHandle]->pSecurityInfo = NULL;
            }

            // If availabe, save the connected device's LTK information
            if ( pAuthEvt[pPkt->connectionHandle]->pDevSecInfo )
            {
                VOID osal_snv_write( devLTKNvID(bondIdx), sizeof ( gapBondLTK_t ), pAuthEvt[pPkt->connectionHandle]->pDevSecInfo );
                pAuthEvt[pPkt->connectionHandle]->pDevSecInfo = NULL;
            }

            // If available, save the connected device's IRK
            if ( pAuthEvt[pPkt->connectionHandle]->pIdentityInfo )
            {
                VOID osal_snv_write( devIRKNvID(bondIdx), KEYLEN, pAuthEvt[pPkt->connectionHandle]->pIdentityInfo->irk );
                pAuthEvt[pPkt->connectionHandle]->pIdentityInfo = NULL;
            }

            // If available, save the connected device's Signature information
            if ( pAuthEvt[pPkt->connectionHandle]->pSigningInfo )
            {
                VOID osal_snv_write( devCSRKNvID(bondIdx), KEYLEN, pAuthEvt[pPkt->connectionHandle]->pSigningInfo->srk );
                VOID osal_snv_write( devSignCounterNvID(bondIdx), sizeof ( uint32 ), &(pAuthEvt[pPkt->connectionHandle]->pSigningInfo->signCounter) );
                pAuthEvt[pPkt->connectionHandle]->pSigningInfo = NULL;
            }

//      else
            {
                if ( autoSyncWhiteList )
                {
                    gapBondMgr_SyncWhiteList();
                }

                // Update the GAP Privacy Flag Properties
                gapBondSetupPrivFlag();
                return ( TRUE );
            }
        }

        // We have more info to store
        return ( FALSE );
    }

    return ( TRUE );
}

/*********************************************************************
    @fn      gapBondMgrGetStateFlags

    @brief   Gets the state flags field of a bond record in NV

    @param   idx

    @return  stateFlags field
*/
static uint8 gapBondMgrGetStateFlags( uint8 idx )
{
    gapBondRec_t bondRec;

    if ( osal_snv_read( mainRecordNvID(idx), sizeof ( gapBondRec_t ), &bondRec ) == SUCCESS )
    {
        return ( bondRec.stateFlags );
    }

    return ( 0 );
}

/*********************************************************************
    @fn      gapBondMgrGetPublicAddr

    @brief   Copy the public Address from a bonding record

    @param   idx - Bond record index
    @param   pAddr - a place to put the public address from NV

    @return  SUCCESS if successful.
            Otherwise failure.
*/
static bStatus_t gapBondMgrGetPublicAddr( uint8 idx, uint8* pAddr )
{
    bStatus_t stat;         // return value
    gapBondRec_t bondRec;   // Work space for main bond record

    // Check parameters
    if ( (idx >= GAP_BONDINGS_MAX) || (pAddr == NULL) )
    {
        return ( INVALIDPARAMETER );
    }

    stat = osal_snv_read( mainRecordNvID(idx), sizeof ( gapBondRec_t ), &bondRec );

    if ( stat == SUCCESS )
    {
        VOID osal_memcpy( pAddr, bondRec.publicAddr, B_ADDR_LEN );
    }

    return ( stat );
}

/*********************************************************************
    @fn      gapBondMgrFindReconnectAddr

    @brief   Look through the bonding entries to find a
            reconnection address.

    @param   pReconnectAddr - device address to look for

    @return  index to found bonding (0 - (GAP_BONDINGS_MAX-1),
            GAP_BONDINGS_MAX if no empty entries
*/
static uint8 gapBondMgrFindReconnectAddr( uint8* pReconnectAddr )
{
    // Item doesn't exist, so create all the items
    for ( uint8 idx = 0; idx < GAP_BONDINGS_MAX; idx++ )
    {
        // compare reconnection address
        if ( osal_memcmp( bonds[idx].reconnectAddr, pReconnectAddr, B_ADDR_LEN ) )
        {
            return ( idx ); // Found it
        }
    }

    return ( GAP_BONDINGS_MAX );
}

/*********************************************************************
    @fn      gapBondMgrFindAddr

    @brief   Look through the bonding entries to find an address.

    @param   pDevAddr - device address to look for

    @return  index to empty bonding (0 - (GAP_BONDINGS_MAX-1),
            GAP_BONDINGS_MAX if no empty entries
*/
static uint8 gapBondMgrFindAddr( uint8* pDevAddr )
{
    // Item doesn't exist, so create all the items
    for ( uint8 idx = 0; idx < GAP_BONDINGS_MAX; idx++ )
    {
        // Read in NV Main Bond Record and compare public address
        if ( osal_memcmp( bonds[idx].publicAddr, pDevAddr, B_ADDR_LEN ) )
        {
            return ( idx ); // Found it
        }
    }

    return ( GAP_BONDINGS_MAX );
}

/*********************************************************************
    @fn      gapBondMgrResolvePrivateAddr

    @brief   Look through the NV bonding entries to resolve a private
            address.

    @param   pDevAddr - device address to look for

    @return  index to found bonding (0 - (GAP_BONDINGS_MAX-1),
            GAP_BONDINGS_MAX if no entry found
*/
static uint8 gapBondMgrResolvePrivateAddr( uint8* pDevAddr )
{
    for ( uint8 idx = 0; idx < GAP_BONDINGS_MAX; idx++ )
    {
        uint8 IRK[KEYLEN];

        // Read in NV IRK Record and compare resovable address
        if ( osal_snv_read( devIRKNvID(idx), KEYLEN, IRK ) == SUCCESS )
        {
            if ( ( osal_isbufset( IRK, 0xFF, KEYLEN ) == FALSE ) &&
                    ( GAP_ResolvePrivateAddr( IRK, pDevAddr ) == SUCCESS ) )
            {
                return ( idx ); // Found it
            }
        }
    }

    return ( GAP_BONDINGS_MAX );
}

/*********************************************************************
    @fn      gapBondMgrReadBonds

    @brief   Read through NV and store them in RAM.

    @param   none

    @return  none
*/
static void gapBondMgrReadBonds( void )
{
    for ( uint8 idx = 0; idx < GAP_BONDINGS_MAX; idx++ )
    {
        // See if the entry exists in NV
        if ( osal_snv_read( mainRecordNvID(idx), sizeof( gapBondRec_t ), &(bonds[idx]) ) != SUCCESS )
        {
            // Can't read the entry, assume that it doesn't exist
            VOID osal_memset( bonds[idx].publicAddr, 0xFF, B_ADDR_LEN );
            VOID osal_memset( bonds[idx].reconnectAddr, 0xFF, B_ADDR_LEN );
            bonds[idx].stateFlags = 0;
        }
    }

    if ( autoSyncWhiteList )
    {
        gapBondMgr_SyncWhiteList();
    }

    // Update the GAP Privacy Flag Properties
    gapBondSetupPrivFlag();
}

/*********************************************************************
    @fn      gapBondMgrFindEmpty

    @brief   Look through the bonding NV entries to find an empty.

    @param   none

    @return  index to empty bonding (0 - (GAP_BONDINGS_MAX-1),
            GAP_BONDINGS_MAX if no empty entries
*/
static uint8 gapBondMgrFindEmpty( void )
{
    // Item doesn't exist, so create all the items
    for ( uint8 idx = 0; idx < GAP_BONDINGS_MAX; idx++ )
    {
        // Look for public address of all 0xFF's
        if ( osal_isbufset( bonds[idx].publicAddr, 0xFF, B_ADDR_LEN ) )
        {
            return ( idx ); // Found one
        }
    }

    return ( GAP_BONDINGS_MAX );
}

/*********************************************************************
    @fn      gapBondMgrBondTotal

    @brief   Look through the bonding NV entries calculate the number
            entries.

    @param   none

    @return  total number of bonds found
*/
static uint8 gapBondMgrBondTotal( void )
{
    uint8 numBonds = 0;

    // Item doesn't exist, so create all the items
    for ( uint8 idx = 0; idx < GAP_BONDINGS_MAX; idx++ )
    {
        // Look for public address that are not 0xFF's
        if ( osal_isbufset( bonds[idx].publicAddr, 0xFF, B_ADDR_LEN ) == FALSE )
        {
            numBonds++; // Found one
        }
    }

    return ( numBonds );
}

/*********************************************************************
    @fn      gapBondMgrEraseAllBondings

    @brief   Write all 0xFF's to all of the bonding entries

    @param   none

    @return  SUCCESS if successful.
            Otherwise, NV_OPER_FAILED for failure.
*/
static bStatus_t gapBondMgrEraseAllBondings( void )
{
    bStatus_t stat = SUCCESS;  // return value

    // Item doesn't exist, so create all the items
    for ( uint8 idx = 0; (idx < GAP_BONDINGS_MAX) && (stat == SUCCESS); idx++ )
    {
        // Erasing will write/create a bonding entry
        stat = gapBondMgrEraseBonding( idx );
    }

    return ( stat );
}

/*********************************************************************
    @fn      gapBondMgrEraseBonding

    @brief   Write all 0xFF's to the complete bonding record

    @param   idx - bonding index

    @return  SUCCESS if successful.
            Otherwise, NV_OPER_FAILED for failure.
*/
static bStatus_t gapBondMgrEraseBonding( uint8 idx )
{
    bStatus_t ret;
    gapBondRec_t bondRec;

    if ( idx == bondIdx )
    {
        // Stop ongoing bond store process to prevent any invalid data be written.
//    osal_clear_event( gapBondMgr_TaskID, GAP_BOND_SYNC_CC_EVT );
//    osal_clear_event( gapBondMgr_TaskID, GAP_BOND_SAVE_REC_EVT );
//        gapBondFreeAuthEvt();
    }

    // First see if bonding record exists in NV, then write all 0xFF's to it
    if ( ( osal_snv_read( mainRecordNvID(idx), sizeof ( gapBondRec_t ), &bondRec ) == SUCCESS )
            && (osal_isbufset( bondRec.publicAddr, 0xFF, B_ADDR_LEN ) == FALSE) )
    {
        gapBondLTK_t ltk;
        gapBondCharCfg_t charCfg[GAP_CHAR_CFG_MAX];
        VOID osal_memset( &bondRec, 0xFF, sizeof ( gapBondRec_t ) );
        VOID osal_memset( &ltk, 0xFF, sizeof ( gapBondLTK_t ) );
        VOID osal_memset( charCfg, 0xFF, sizeof ( charCfg ) );
        // Write out FF's over the entire bond entry.
        ret = osal_snv_write( mainRecordNvID(idx), sizeof ( gapBondRec_t ), &bondRec );
        ret |= osal_snv_write( localLTKNvID(idx), sizeof ( gapBondLTK_t ), &ltk );
        ret |= osal_snv_write( devLTKNvID(idx), sizeof ( gapBondLTK_t ), &ltk );
        ret |= osal_snv_write( devIRKNvID(idx), KEYLEN, ltk.LTK );
        ret |= osal_snv_write( devCSRKNvID(idx), KEYLEN, ltk.LTK );
        ret |= osal_snv_write( devSignCounterNvID(idx), sizeof ( uint32 ), ltk.LTK );
        // Write out FF's over the charactersitic configuration entry.
        ret |= osal_snv_write( gattCfgNvID(idx), sizeof ( charCfg ), charCfg );
    }
    else
    {
        ret = SUCCESS;
    }

    return ( ret );
}

/*********************************************************************
    @brief   Task Initialization function.

    Internal function defined in gapbondmgr.h.
*/
void GAPBondMgr_Init( uint8 task_id )
{
    gapBondMgr_TaskID = task_id;  // Save task ID
    // Setup Bond RAM Shadow
    gapBondMgrReadBonds();
}

/*********************************************************************
    @brief   Task Event Processor function.

    Internal function defined in gapbondmgr.h.
*/
uint16 GAPBondMgr_ProcessEvent( uint8 task_id, uint16 events )
{
    VOID task_id; // OSAL required parameter that isn't used in this function

    if ( events & SYS_EVENT_MSG )
    {
        uint8* pMsg;

        if ( (pMsg = osal_msg_receive( gapBondMgr_TaskID )) != NULL )
        {
            if ( gapBondMgr_ProcessOSALMsg( (osal_event_hdr_t*)pMsg ) )
            {
                // Release the OSAL message
                VOID osal_msg_deallocate( pMsg );
            }

            // return unprocessed events
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

//  if ( events & GAP_BOND_SAVE_REC_EVT )
//  {
//
//    // Save bonding record in NV
//    if ( gapBondMgrAddBond( NULL, NULL ) )
//    {
//      // Notify our task to update NV with CCC values stored in GATT database
//      osal_set_event( gapBondMgr_TaskID, GAP_BOND_SYNC_CC_EVT );
//
//      return (events ^ GAP_BOND_SAVE_REC_EVT);
//    }
//
//    return ( GAP_BOND_SAVE_REC_EVT );
//  }
//
//  if ( events & GAP_BOND_SYNC_CC_EVT )
//  {
//    // Update NV to have same CCC values as GATT database
//    // Note: pAuthEvt is a global variable used for deferring the storage
//    if ( gapBondMgr_SyncCharCfg( pAuthEvt->connectionHandle ) )
//    {
//      if ( pGapBondCB && pGapBondCB->pairStateCB )
//      {
//        // Assume SUCCESS since we got this far.
//        pGapBondCB->pairStateCB( pAuthEvt->connectionHandle, GAPBOND_PAIRING_STATE_COMPLETE, SUCCESS );
//      }
//
//      // We're done storing bond record and CCC values in NV
//      gapBondFreeAuthEvt();
//
//      return (events ^ GAP_BOND_SYNC_CC_EVT);
//    }
//
//    return ( GAP_BOND_SYNC_CC_EVT );
//  }
    // Discard unknown events
    return 0;
}

/*********************************************************************
    @fn      gapBondMgr_ProcessOSALMsg

    @brief   Process an incoming task message.

    @param   pMsg - message to process

    @return  TRUE if safe to deallocate incoming message, FALSE otherwise.
*/
static uint8 gapBondMgr_ProcessOSALMsg( osal_event_hdr_t* pMsg )
{
    uint8 safeToDealloc = TRUE;

    switch ( pMsg->event )
    {
    case GAP_MSG_EVENT:
        safeToDealloc = GAPBondMgr_ProcessGAPMsg( (gapEventHdr_t*)pMsg );
        break;

    case GATT_MSG_EVENT:
        gapBondMgr_ProcessGATTMsg( (gattMsgEvent_t*)pMsg );
        break;

    case GATT_SERV_MSG_EVENT:
        gapBondMgr_ProcessGATTServMsg( (gattEventHdr_t*)pMsg );
        break;

    default:
        break;
    }

    return ( safeToDealloc );
}

/*********************************************************************
    @fn      GAPBondMgr_CheckNVLen

    @brief   This function will check the length of an NV Item.

    @param   id - NV ID.
    @param   len - lengths in bytes of item.

    @return  SUCCESS or FAILURE
*/
uint8 GAPBondMgr_CheckNVLen( uint8 id, uint8 len )
{
    uint8 stat = FAILURE;

    // Convert to index
    switch ( (id - BLE_NVID_GAP_BOND_START) % GAP_BOND_REC_IDS )
    {
    case GAP_BOND_REC_ID_OFFSET:
        if ( len == sizeof ( gapBondRec_t ) )
        {
            stat = SUCCESS;
        }

        break;

    case GAP_BOND_LOCAL_LTK_OFFSET:
    case GAP_BOND_DEV_LTK_OFFSET:
        if ( len == sizeof ( gapBondLTK_t ) )
        {
            stat = SUCCESS;
        }

        break;

    case GAP_BOND_DEV_IRK_OFFSET:
    case GAP_BOND_DEV_CSRK_OFFSET:
        if ( len == KEYLEN )
        {
            stat = SUCCESS;
        }

        break;

    case GAP_BOND_DEV_SIGN_COUNTER_OFFSET:
        if ( len == sizeof ( uint32 ) )
        {
            stat = SUCCESS;
        }

        break;

    default:
        break;
    }

    return ( stat );
}

/*********************************************************************
    @fn          gapBondMgr_ProcessGATTMsg

    @brief       Process an incoming GATT message.

    @param       pMsg - pointer to received message

    @return      none
*/
static void gapBondMgr_ProcessGATTMsg( gattMsgEvent_t* pMsg )
{
    // Process the GATT message
    switch ( pMsg->method )
    {
    case ATT_HANDLE_VALUE_CFM:
        // Clear Service Changed flag for this client
        VOID GAPBondMgr_ServiceChangeInd( pMsg->connHandle, 0x00 );
        break;

    default:
        // Unknown message
        break;
    }
}

/*********************************************************************
    @fn          gapBondMgr_ProcessGATTServMsg

    @brief       Process an incoming GATT Server App message.

    @param       pMsg - pointer to received message

    @return      none
*/
static void gapBondMgr_ProcessGATTServMsg( gattEventHdr_t* pMsg )
{
    // Process the GATT Server App message
    switch ( pMsg->method )
    {
    case GATT_CLIENT_CHAR_CFG_UPDATED_EVENT:
    {
        gattClientCharCfgUpdatedEvent_t* pEvent = (gattClientCharCfgUpdatedEvent_t*)pMsg;
        VOID GAPBondMgr_UpdateCharCfg( pEvent->connHandle, pEvent->attrHandle, pEvent->value );
    }
    break;

    default:
        // Unknown message
        break;
    }
}

/*********************************************************************
    @fn      gapBondMgrSendServiceChange

    @brief   Tell the GATT that a service change is needed.

    @param   pLinkItem - pointer to connection information

    @return  none
*/
static void gapBondMgrSendServiceChange( linkDBItem_t* pLinkItem )
{
    VOID GATTServApp_SendServiceChangedInd( pLinkItem->connectionHandle,
                                            gapBondMgr_TaskID );
}

/*********************************************************************
    @fn      gapBondSetupPrivFlag

    @brief   Setup the GAP Privacy Flag properties.

    @param   none

    @return  none
*/
static void gapBondSetupPrivFlag( void )
{
    uint8 privFlagProp;

    if ( gapBondMgrBondTotal() > 1 )
    {
        privFlagProp = GATT_PROP_READ;
    }
    else
    {
        privFlagProp = GATT_PROP_READ | GATT_PROP_WRITE;
    }

    // Setup the
    VOID GGS_SetParameter( GGS_PERI_PRIVACY_FLAG_PROPS, sizeof ( uint8 ), &privFlagProp );
}

/*********************************************************************
    @fn      gapBondMgrAuthenticate

    @brief   Initiate authentication

    @param   connHandle - connection handle
    @param   addrType - peer address type
    @param   pPairReq - Enter these parameters if the Pairing Request was already received.
            NULL, if waiting for Pairing Request or if initiating.

    @return  none
*/
static void gapBondMgrAuthenticate( uint16 connHandle, uint8 addrType,
                                    gapPairingReq_t* pPairReq )
{
    gapAuthParams_t params;
    VOID osal_memset( &params, 0, sizeof ( gapAuthParams_t ) );
    // Setup the pairing parameters
    params.connectionHandle = connHandle;
    params.secReqs.ioCaps = gapBond_IOCap;
    params.secReqs.oobAvailable = gapBond_OOBDataFlag;
    params.secReqs.maxEncKeySize = gapBond_KeySize;
    params.secReqs.keyDist.sEncKey = (gapBond_KeyDistList & GAPBOND_KEYDIST_SENCKEY) ? TRUE : FALSE;
    params.secReqs.keyDist.sIdKey = (gapBond_KeyDistList & GAPBOND_KEYDIST_SIDKEY) ? TRUE : FALSE;
    params.secReqs.keyDist.mEncKey = (gapBond_KeyDistList & GAPBOND_KEYDIST_MENCKEY) ? TRUE : FALSE;
    params.secReqs.keyDist.mIdKey = (gapBond_KeyDistList & GAPBOND_KEYDIST_MIDKEY) ? TRUE : FALSE;
    params.secReqs.keyDist.mSign = (gapBond_KeyDistList & GAPBOND_KEYDIST_MSIGN) ? TRUE : FALSE;
    params.secReqs.keyDist.sSign = (gapBond_KeyDistList & GAPBOND_KEYDIST_SSIGN) ? TRUE : FALSE;

    // Is bond manager setup for OOB data?
    if ( gapBond_OOBDataFlag )
    {
        VOID osal_memcpy( params.secReqs.oob, gapBond_OOBData, KEYLEN );
    }

    if ( gapBond_Bonding && addrType != ADDRTYPE_PUBLIC )
    {
        // Force a slave ID key
        params.secReqs.keyDist.sIdKey = TRUE;
    }

    params.secReqs.authReq |= (gapBond_Bonding) ? SM_AUTH_STATE_BONDING : 0;
    params.secReqs.authReq |= (gapBond_MITM) ? SM_AUTH_STATE_AUTHENTICATED : 0;
    params.secReqs.authReq |= (gapBond_SC) ? SM_AUTH_STATE_SC : 0;
    params.secReqs.authReq |= (gapBond_keyPress) ? SM_AUTH_STATE_KEYPRESS : 0;
    params.secReqs.authReq |= (gapBond_CT2) ? SM_AUTH_STATE_CT2 : 0;
    GAP_Authenticate( &params, pPairReq );
}

#if ( HOST_CONFIG & PERIPHERAL_CFG )
/*********************************************************************
    @fn      gapBondMgrSlaveSecurityReq

    authReq |= (gapBond_Bonding) ? SM_AUTH_STATE_BONDING : 0;
    authReq |= (gapBond_MITM) ? SM_AUTH_STATE_AUTHENTICATED : 0;
    authReq |= (gapBond_SC) ? SM_AUTH_STATE_SC : 0;
    authReq |= (gapBond_keyPress) ? SM_AUTH_STATE_KEYPRESS : 0;
    authReq |= (gapBond_CT2) ? SM_AUTH_STATE_CT2 : 0;

    @param   connHandle - connection handle

    @return  none
*/
static void gapBondMgrSlaveSecurityReq( uint16 connHandle )
{
    uint8 authReq = 0;
    authReq |= (gapBond_Bonding) ? SM_AUTH_STATE_BONDING : 0;
    authReq |= (gapBond_MITM) ? SM_AUTH_STATE_AUTHENTICATED : 0;
    authReq |= (gapBond_SC) ? SM_AUTH_STATE_SC : 0;
    authReq |= (gapBond_keyPress) ? SM_AUTH_STATE_KEYPRESS : 0;
    authReq |= (gapBond_CT2) ? SM_AUTH_STATE_CT2 : 0;
    GAP_SendSlaveSecurityRequest( connHandle, authReq );
}
#endif

/*********************************************************************
    @fn      gapBondMgrBondReq

    @brief   Initiate a GAP bond request

    @param   connHandle - connection handle
    @param   idx - NV index of bond entry
    @param   stateFlags - bond state flags
    @param   role - master or slave role
    @param   startEncryption - whether or not to start encryption

    @return  none
*/
static void gapBondMgrBondReq( uint16 connHandle, uint8 idx, uint8 stateFlags,
                               uint8 role, uint8 startEncryption )
{
    smSecurityInfo_t ltk;
    osalSnvId_t      nvId;

    if ( role == GAP_PROFILE_CENTRAL )
    {
        nvId = devLTKNvID( idx );
    }
    else
    {
        nvId = localLTKNvID( idx );
    }

    // Initialize the NV structures
    VOID osal_memset( &ltk, 0, sizeof ( smSecurityInfo_t ) );

    if ( osal_snv_read( nvId, sizeof ( smSecurityInfo_t ), &ltk ) == SUCCESS )
    {
        if ( (ltk.keySize >= MIN_ENC_KEYSIZE) && (ltk.keySize <= MAX_ENC_KEYSIZE) )
        {
            //bStatus_t ret =
            		GAP_Bond( connHandle,
                                      ((stateFlags & GAP_BONDED_STATE_AUTHENTICATED) ? TRUE : FALSE),
                                      &ltk, startEncryption );
        }
    }
}

/*********************************************************************
    @fn      gapBondMgr_SyncWhiteList

    @brief   syncronize the White List with the bonds

    @param   none

    @return  none
*/
static void gapBondMgr_SyncWhiteList( void )
{
    //erase the White List
    VOID HCI_LE_ClearWhiteListCmd();

    // Write bond addresses into the White List
    for( uint8 i = 0; i < GAP_BONDINGS_MAX; i++)
    {
        // Make sure empty addresses are not added to the White List
        if ( osal_isbufset( bonds[i].publicAddr, 0xFF, B_ADDR_LEN ) == FALSE )
        {
            VOID HCI_LE_AddWhiteListCmd( HCI_PUBLIC_DEVICE_ADDRESS, bonds[i].publicAddr );
        }
    }
}

/*********************************************************************
    @fn          gapBondMgr_SyncCharCfg

    @brief       Update the Bond Manager to have the same configurations as
                the GATT database.

    @param       connHandle - the current connection handle to find client configurations for

    @return      TRUE if sync done. FALSE, otherwise.
*/
static uint8 gapBondMgr_SyncCharCfg( uint16 connHandle )
{
    static gattAttribute_t* pAttr = NULL;
    static uint16 service;

    // Only attributes with attribute handles between and including the Starting
    // Handle parameter and the Ending Handle parameter that match the requested
    // attribute type and the attribute value will be returned.

    // All attribute types are effectively compared as 128-bit UUIDs,
    // even if a 16-bit UUID is provided in this request or defined
    // for an attribute.
    if ( pAttr == NULL )
    {
        pAttr = GATT_FindHandleUUID( GATT_MIN_HANDLE, GATT_MAX_HANDLE,
                                     clientCharCfgUUID, ATT_BT_UUID_SIZE, &service );
    }

    while ( pAttr != NULL )
    {
        uint16 len;
        uint8 attrVal[ATT_BT_UUID_SIZE];

        // It is not possible to use this request on an attribute that has a value
        // that is longer than 2.
        if ( GATTServApp_ReadAttr( connHandle, pAttr, service, attrVal,
                                   &len, 0, ATT_BT_UUID_SIZE ) == SUCCESS )
        {
            // It is not possible to use this request on an attribute that has a value
            // that is longer than 2.
            uint16 value = BUILD_UINT16(attrVal[0], attrVal[1]);

            if ( value != GATT_CFG_NO_OPERATION )
            {
                // NV must be updated to meet configuration of the database
                VOID GAPBondMgr_UpdateCharCfg( connHandle, pAttr->handle, value );
            }
        }

        // Try to find the next attribute
        pAttr = GATT_FindNextAttr( pAttr, GATT_MAX_HANDLE, service, NULL );
    }

    return ( pAttr == NULL );
}

/*********************************************************************
    @fn          gapBondFreeAuthEvt

    @brief       Free GAP Authentication Complete event.

    @param       none

    @return      none
*/
static void gapBondFreeAuthEvt( uint16 connHandle )
{
    if ( pAuthEvt[connHandle] != NULL )
    {
        // Release the OSAL message
        VOID osal_msg_deallocate( (uint8*)pAuthEvt[connHandle] );
        pAuthEvt[connHandle] = NULL;
    }

    bondIdx = GAP_BONDINGS_MAX;
}


/*****************************************************************************************
    fn:  gapBondRecvEvt

    date:2020-04-26

    brief:

    input parameters:connHandle connection handler


    output parameters:


    return       void

 *****************************************************************************************/
static void gapBondRecvEvt(uint16 connHandle, gapBondRec_t* pBondRec, gapAuthCompleteEvent_t* pPkt )
{
    if ( gapBondMgrAddBond( pBondRec, pPkt ) )
    {
        // Update NV to have same CCC values as GATT database
        // Note: pAuthEvt is a global variable used for deferring the storage
        if ( gapBondMgr_SyncCharCfg( pAuthEvt[connHandle]->connectionHandle ) )
        {
            if ( pGapBondCB && pGapBondCB->pairStateCB )
            {
                // Assume SUCCESS since we got this far.
                pGapBondCB->pairStateCB( pAuthEvt[connHandle]->connectionHandle, GAPBOND_PAIRING_STATE_COMPLETE, SUCCESS );
            }

            // We're done storing bond record and CCC values in NV
            gapBondFreeAuthEvt(connHandle);
        }
    }
}


#endif // ( CENTRAL_CFG | PERIPHERAL_CFG )

/*********************************************************************
*********************************************************************/
