
/*************************************************************************************************
    Filename:       gap_configmgr.c
    Revised:
    Revision:

    Description:    This file contains the GAP Configuration Manager.

	SDK_LICENSE

**************************************************************************************************/
#include "bcomdef.h"
#include "gap.h"
#include "gap_internal.h"
#include "linkdb.h"
#include "sm.h"
#include "sm_internal.h"

/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/

// Default Timer values
#define TGAP_GEN_DISC_ADV_MIN_DEFAULT         0       // mSec (0 = no timeout)
#define TGAP_LIM_ADV_TIMEOUT_DEFAULT          180     // 180 seconds
#define TGAP_GEN_DISC_SCAN_DEFAULT            10240   // mSec
#define TGAP_LIM_DISC_SCAN_DEFAULT            10240   // mSec
#define TGAP_CONN_EST_ADV_TIMEOUT_DEFAULT     10240   // mSec
#define TGAP_CONN_PARAM_TIMEOUT_DEFAULT       30000   // mSec

// GAP Constants defaults
#if defined ( GAP_STANDARDS )
    // Defined as defaults in spec
    #define TGAP_LIM_DISC_ADV_INT_MIN_DEFAULT     2048    // 1280 mSec (n * 0.625 mSec)
    #define TGAP_LIM_DISC_ADV_INT_MAX_DEFAULT     2048    // 1280 mSec (n * 0.625 mSec)
    #define TGAP_GEN_DISC_ADV_INT_MIN_DEFAULT     2048    // 1280 mSec (n * 0.625 mSec)
    #define TGAP_GEN_DISC_ADV_INT_MAX_DEFAULT     2048    // 1280 mSec (n * 0.625 mSec)
    #define TGAP_CONN_ADV_INT_MIN_DEFAULT         2048    // 1280 mSec (n * 0.625 mSec)
    #define TGAP_CONN_ADV_INT_MAX_DEFAULT         2048    // 1280 mSec (n * 0.625 mSec)
    #define TGAP_CONN_SCAN_INT_DEFAULT            2048    // 1280 mSec (n * 0.625 mSec)
    #define TGAP_CONN_SCAN_WIND_DEFAULT           18      // 11.25 mSec (n * 0.625 mSec)
    #define TGAP_CONN_HIGH_SCAN_INT_DEFAULT       16      // 10 mSec (n * 0.625 mSec)
    #define TGAP_CONN_HIGH_SCAN_WIND_DEFAULT      16      // 10 mSec (n * 0.625 mSec)
    #define TGAP_GEN_DISC_SCAN_INT_DEFAULT        16      // 10 mSec (n * 0.625 mSec)
    #define TGAP_GEN_DISC_SCAN_WIND_DEFAULT       16      // 10 mSec (n * 0.625 mSec)
    #define TGAP_LIM_DISC_SCAN_INT_DEFAULT        16      // 10 mSec (n * 0.625 mSec)
    #define TGAP_LIM_DISC_SCAN_WIND_DEFAULT       16      // 10 mSec (n * 0.625 mSec)
    #define TGAP_CONN_EST_ADV_DEFAULT             80      // 50 mSec (n * 0.625 mSec)
    #define TGAP_CONN_EST_INT_MIN_DEFAULT         400     // 500 mSec (n * 1.25 mSec)
    #define TGAP_CONN_EST_INT_MAX_DEFAULT         400     // 500 mSec (n * 1.25 mSec)
    #define TGAP_CONN_EST_SCAN_INT_DEFAULT        16      // 10 mSec (n * 0.625 mSec)
    #define TGAP_CONN_EST_SCAN_WIND_DEFAULT       16      // 10 mSec (n * 0.625 mSec)
    #define TGAP_CONN_EST_SUPERV_TIMEOUT_DEFAULT  2000    // 20 sec (n * 10 mSec)
    #define TGAP_CONN_EST_LATENCY_DEFAULT         0       // (in number of connection events)
    #define TGAP_CONN_EST_MIN_CE_LEN_DEFAULT      0       // (n * 0.625 mSec)
    #define TGAP_CONN_EST_MAX_CE_LEN_DEFAULT      0       // (n * 0.625 mSec)
    #define TGAP_PRIVATE_ADDR_INT_DEFAULT         15      // 15 minutes
    #define TGAP_CONN_PAUSE_CENTRAL_DEFAULT       1       // 1 seconds
    #define TGAP_CONN_PAUSE_PERIPHERAL_DEFAULT    5       // 5 seconds
#else
    // Actually works
    #define TGAP_LIM_DISC_ADV_INT_MIN_DEFAULT     160     // 100 mSec (n * 0.625 mSec)
    #define TGAP_LIM_DISC_ADV_INT_MAX_DEFAULT     160     // 100 mSec (n * 0.625 mSec)
    #define TGAP_GEN_DISC_ADV_INT_MIN_DEFAULT     160     // 100 mSec (n * 0.625 mSec)
    #define TGAP_GEN_DISC_ADV_INT_MAX_DEFAULT     160     // 100 mSec (n * 0.625 mSec)
    #define TGAP_CONN_ADV_INT_MIN_DEFAULT         2048    // 1280 mSec (n * 0.625 mSec)
    #define TGAP_CONN_ADV_INT_MAX_DEFAULT         2048    // 1280 mSec (n * 0.625 mSec)
    #define TGAP_CONN_SCAN_INT_DEFAULT            480     // 300 mSec (n * 0.625 mSec)
    #define TGAP_CONN_SCAN_WIND_DEFAULT           240     // 150 mSec (n * 0.625 mSec)
    #define TGAP_CONN_HIGH_SCAN_INT_DEFAULT       16      // 10 mSec (n * 0.625 mSec)
    #define TGAP_CONN_HIGH_SCAN_WIND_DEFAULT      16      // 10 mSec (n * 0.625 mSec)
    #define TGAP_GEN_DISC_SCAN_INT_DEFAULT        16      // 10 mSec (n * 0.625 mSec)
    #define TGAP_GEN_DISC_SCAN_WIND_DEFAULT       16      // 10 mSec (n * 0.625 mSec)
    #define TGAP_LIM_DISC_SCAN_INT_DEFAULT        16      // 10 mSec (n * 0.625 mSec)
    #define TGAP_LIM_DISC_SCAN_WIND_DEFAULT       16      // 10 mSec (n * 0.625 mSec)
    #define TGAP_CONN_EST_ADV_DEFAULT             80      // 50 mSec (n * 0.625 mSec)
    #define TGAP_CONN_EST_INT_MIN_DEFAULT         80      // 100 mSec (n * 1.25 mSec)
    #define TGAP_CONN_EST_INT_MAX_DEFAULT         80      // 100 mSec (n * 1.25 mSec)
    #define TGAP_CONN_EST_SCAN_INT_DEFAULT        16      // 10 mSec (n * 0.625 mSec)
    #define TGAP_CONN_EST_SCAN_WIND_DEFAULT       16      // 10 mSec (n * 0.625 mSec)
    #define TGAP_CONN_EST_SUPERV_TIMEOUT_DEFAULT  2000    // 20 sec (n * 10 mSec)
    #define TGAP_CONN_EST_LATENCY_DEFAULT         0       // (in number of connection events)
    #define TGAP_CONN_EST_MIN_CE_LEN_DEFAULT      0       // (n * 0.625 mSec)
    #define TGAP_CONN_EST_MAX_CE_LEN_DEFAULT      0       // (n * 0.625 mSec)
    #define TGAP_PRIVATE_ADDR_INT_DEFAULT         15      // 15 minutes
    #define TGAP_CONN_PAUSE_CENTRAL_DEFAULT       1       // 1 seconds
    #define TGAP_CONN_PAUSE_PERIPHERAL_DEFAULT    5       // 5 seconds
#endif

// Proprietary default values
#define TGAP_SM_TIMEOUT_DEFAULT               30000   // 30 seconds (milliseconds)
#define TGAP_SM_MIN_KEY_LEN_DEFAULT           7       // 7 Bytes minimum
#define TGAP_SM_MAX_KEY_LEN_DEFAULT           16      // 16 Bytes maximum
#define TGAP_FILTER_ADV_REPORTS_DEFAULT       TRUE    // Filter duplicate advertising reports
#define TGAP_SCAN_RSP_RSSI_MIN_DEFAULT        -127    // Minimum RSSI required for scan responses
#define TGAP_REJECT_CONN_PARAMS_DEFAULT       FALSE   // Accept Connection Parameter Update Request

#if defined ( TESTMODES )
    #define TGAP_GAP_TESTCODE_DEFAULT           GAP_TESTMODE_OFF
    #define TGAP_SM_TESTCODE_DEFAULT            SM_TESTMODE_OFF
#endif

#define TGAP_AUTH_TASK_ID_DEFAULT             0      // Default task ID, 0 if no default

/*********************************************************************
    TYPEDEFS
*/

// Used when reading variables from the HCI during initialization.
typedef enum
{
    GAP_INITSTATE_INVALID,
    GAP_INITSTATE_BD_ADDR,
    GAP_INITSTATE_BUFFERSIZE,
    GAP_INITSTATE_READY
} gapLLParamsStates_t;

typedef struct
{
    gapLLParamsStates_t state;              // Holds the state during initialization
    uint8*  pIRK;                            // Pointer to the device's IRK, this space is actually allocated in the controlling application
    uint8*  pSRK;                            // Pointer to the device's SRK, this space is actually allocated in the controlling application
    uint32* pSignCounter;                    // Pointer to the device's sign counter, this space is actually allocated in the controlling application
    uint8  BD_ADDR[B_ADDR_LEN];             // The device's public address, this value is read from HCI
    uint16 HC_LE_Data_Packet_Lenth;         // Maximum HCI packet length
    uint8  HC_Total_Num_LE_Data_Packets;    // Total number of data packets that can be sent, read from HCI
} gapConfigLLParams_t;

/*********************************************************************
    GLOBAL VARIABLES - These are available as part of the internal API
                  (gap_internal.h), not part of the public API (gap.h)
*/

uint8 gapAppTaskID = INVALID_TASK_ID; // default task ID to send events
uint8 gapProfileRole = NULL;          // device GAP Profile Role(s)


uint8 gapDeviceAddrMode = ADDRTYPE_PUBLIC;   // Current device's address mode

// Address change timeout, should only be non-0 if using ADDRTYPE_PRIVATE_RESOLVE
// Also, this timeout is in minutes.
uint16 gapPrivateAddrChangeTimeout = 0;

// Flag to control auto private resolvable address changes.
uint8 gapAutoAdvPrivateAddrChange = FALSE;

/*********************************************************************
    EXTERNAL VARIABLES
*/

/*********************************************************************
    EXTERNAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/

// Device's parameters, established during GAP_DeviceInit().
static gapConfigLLParams_t gapParams;

// GAP settable parameters, these can be accessed through GAP_SetParamValue() and
// GAP_GetParamValue() public APIs.
uint16 gapParameters[] =
{
    // Default Timer values
    TGAP_GEN_DISC_ADV_MIN_DEFAULT,
    TGAP_LIM_ADV_TIMEOUT_DEFAULT,
    TGAP_GEN_DISC_SCAN_DEFAULT,
    TGAP_LIM_DISC_SCAN_DEFAULT,
    TGAP_CONN_EST_ADV_TIMEOUT_DEFAULT,
    TGAP_CONN_PARAM_TIMEOUT_DEFAULT,

    // GAP Constants defaults
    TGAP_LIM_DISC_ADV_INT_MIN_DEFAULT,
    TGAP_LIM_DISC_ADV_INT_MAX_DEFAULT,
    TGAP_GEN_DISC_ADV_INT_MIN_DEFAULT,
    TGAP_GEN_DISC_ADV_INT_MAX_DEFAULT,
    TGAP_CONN_ADV_INT_MIN_DEFAULT,
    TGAP_CONN_ADV_INT_MAX_DEFAULT,
    TGAP_CONN_SCAN_INT_DEFAULT,
    TGAP_CONN_SCAN_WIND_DEFAULT,
    TGAP_CONN_HIGH_SCAN_INT_DEFAULT,
    TGAP_CONN_HIGH_SCAN_WIND_DEFAULT,
    TGAP_GEN_DISC_SCAN_INT_DEFAULT,
    TGAP_GEN_DISC_SCAN_WIND_DEFAULT,
    TGAP_LIM_DISC_SCAN_INT_DEFAULT,
    TGAP_LIM_DISC_SCAN_WIND_DEFAULT,
    TGAP_CONN_EST_ADV_DEFAULT,
    TGAP_CONN_EST_INT_MIN_DEFAULT,
    TGAP_CONN_EST_INT_MAX_DEFAULT,
    TGAP_CONN_EST_SCAN_INT_DEFAULT,
    TGAP_CONN_EST_SCAN_WIND_DEFAULT,
    TGAP_CONN_EST_SUPERV_TIMEOUT_DEFAULT,
    TGAP_CONN_EST_LATENCY_DEFAULT,
    TGAP_CONN_EST_MIN_CE_LEN_DEFAULT,
    TGAP_CONN_EST_MAX_CE_LEN_DEFAULT,
    TGAP_PRIVATE_ADDR_INT_DEFAULT,
    TGAP_CONN_PAUSE_CENTRAL_DEFAULT,
    TGAP_CONN_PAUSE_PERIPHERAL_DEFAULT,

    // Proprietary default values
    TGAP_SM_TIMEOUT_DEFAULT,
    TGAP_SM_MIN_KEY_LEN_DEFAULT,
    TGAP_SM_MAX_KEY_LEN_DEFAULT,
    TGAP_FILTER_ADV_REPORTS_DEFAULT,
    (uint16)TGAP_SCAN_RSP_RSSI_MIN_DEFAULT,
    TGAP_REJECT_CONN_PARAMS_DEFAULT,
#if defined ( TESTMODES )
    TGAP_GAP_TESTCODE_DEFAULT,
    TGAP_SM_TESTCODE_DEFAULT,
#endif
    TGAP_AUTH_TASK_ID_DEFAULT
};

// Storage for the current Random address (not Public).
static uint8 gapCurrentRandomAddr[B_ADDR_LEN] = {0,0,0,0,0,0};

/*********************************************************************
    LOCAL FUNCTION PROTOTYPES
*/
static void gapSendDeviceInitDoneEvent( uint8 status );
static void gapSendRandomAddrChangeEvent( uint8 status, uint8* pNewAddr );
static void gapSendSignUpdateEvent( uint8 taskID, uint8 addrType,
                                    uint8* pDevAddr, uint32 signCounter );

/*********************************************************************
    API FUNCTIONS
*/

/*********************************************************************
    Set a GAP Parameter value.  Use this function to change the default
    GAP parameter values.

    Public function defined in gap.h.
*/
bStatus_t GAP_SetParamValue( gapParamIDs_t paramID, uint16 paramValue )
{
    // Check for invalid parameters
    if ( (paramID < TGAP_PARAMID_MAX) && (paramValue != 0xFFFF) )
    {
#if defined ( TESTMODES )

        if ( (paramID == TGAP_SM_TESTCODE) && (paramValue == SM_TESTMODE_SEND_CONFIRM) )
        {
            smpPairingConfirm_t confirmMsg; // Parameters to build message
            // Clear the confirm
            VOID osal_memset( confirmMsg.confirmValue,  0, SMP_CONFIRM_LEN );
            // Send confirm message
            return ( smSendPairingConfirm( 0, &confirmMsg ) );
        }

#endif
        // Good so far
        gapParameters[paramID] = paramValue;
        return ( SUCCESS );
    }
    else
    {
        return ( INVALIDPARAMETER );
    }
}

/*********************************************************************
    @brief       Get a GAP Parameter value.

    Public function defined in gap.h.
*/
uint16 GAP_GetParamValue( gapParamIDs_t paramID )
{
    // Check for invalid parameter
    if ( paramID < TGAP_PARAMID_MAX )
    {
        return ( gapParameters[paramID] );
    }
    else
    {
        return ( 0xFFFF );
    }
}

/*********************************************************************
    GAP INTERNAL FUNCTIONS
*/

/*********************************************************************
    @fn          GAP_ParamsInit

    @brief       Setup the device configuration parameters.

    @param       taskID - Default task ID to send events.
    @param       profileRole - GAP Profile Roles

    @return      SUCCESS or bleIncorrectMode
*/
bStatus_t GAP_ParamsInit( uint8 taskID, uint8 profileRole )
{
    bStatus_t stat;   // Return status
    // Initialize the GAP Parameters
    VOID osal_memset( &gapParams, 0, (int)(sizeof ( gapConfigLLParams_t )) );
    gapAppTaskID = taskID;
    gapProfileRole = profileRole;
    // Start gathering LL (HCI) parameters
    stat = HCI_ReadBDADDRCmd();

    if ( stat == SUCCESS )
    {
        // Set state to wait for the HCI event with the address
        gapParams.state = GAP_INITSTATE_BD_ADDR;
    }
    else
    {
        // Enter an error state and setup the return for an error
        gapParams.state = GAP_INITSTATE_INVALID;
        stat = bleIncorrectMode;
    }

    return ( stat );
}

/*********************************************************************
    @fn          GAP_SecParamsInit

    @brief       Setup the device security configuration parameters.

    @param       pIRK - pointer to Identity Root Key, NULLKEY (all zeroes) if the app
                  wants the GAP to generate the key.
    @param       pSRK - pointer to Sign Resolving Key, NULLKEY if the app
                  wants the GAP to generate the key.
    @param       pSignCounter - 32 bit value used in the SM Signing
                  algorithm that shall be initialized to zero and incremented
                  with every new signing. This variable must also be maintained
                  by the application.

    @return      none
*/
void GAP_SecParamsInit( uint8* pIRK, uint8* pSRK, uint32* pSignCounter )
{
    gapParams.pSignCounter = pSignCounter;
    // Copy the key pointers
    gapParams.pSRK = pSRK;
    gapParams.pIRK = pIRK;

    // Check if the buffer is all 0's, which means the the app expects
    // that the GAP generate a random SRK
    if ( osal_isbufset( pSRK, 0, KEYLEN ) == TRUE )
    {
        // Generate a random number SRK
        smGenerateRandBuf( gapParams.pSRK, KEYLEN );
    }

    // Check if the buffer is all 0's, which means the the app expects
    // that the GAP generate a random IRK
    if ( osal_isbufset( pIRK, 0, KEYLEN ) == TRUE )
    {
        // Generate a random number IRK
        smGenerateRandBuf( gapParams.pIRK, KEYLEN );
    }
}

/*********************************************************************
    Setup the device's address type.  If ADDRTYPE_PRIVATE_RESOLVE
    is selected, the address will change periodically.

    Public function defined in gap.h.
*/
bStatus_t GAP_ConfigDeviceAddr( uint8 addrType, uint8* pStaticAddr )
{
    uint8 currentAddrMode = gapDeviceAddrMode; // Save current address mode, before changing it
    bStatus_t stat = SUCCESS;   // Assume a successful return
    uint8 newAddr[B_ADDR_LEN];

    // Check for valid address type.
    if ( (addrType > ADDRTYPE_PRIVATE_RESOLVE) )
    {
        return ( INVALIDPARAMETER );
    }

    // Make sure that we have been initialized first.
    if ( gapParams.state != GAP_INITSTATE_READY )
    {
        return ( bleNotReady );
    }

    // Only allow an address mode change if we aren't already doing something.
    if ( GAP_NumActiveConnections() || gapIsAdvertising() || gapIsScanning() )
    {
        return ( bleIncorrectMode );
    }

    if ( (addrType == ADDRTYPE_STATIC) || (addrType == ADDRTYPE_PRIVATE_NONRESOLVE) )
    {
        // Is the passed in pointer valid?
        if ( pStaticAddr )
        {
            // Make sure the not all zeros
            VOID osal_memset( newAddr, 0, B_ADDR_LEN );

            if ( osal_memcmp( pStaticAddr, newAddr, B_ADDR_LEN ) )
            {
                return ( INVALIDPARAMETER );
            }

            // Make sure the not all ones
            VOID osal_memset( newAddr, 0xFF, B_ADDR_LEN );

            if ( osal_memcmp( pStaticAddr, newAddr, B_ADDR_LEN ) )
            {
                return ( INVALIDPARAMETER );
            }

            // Use the passed in address
            VOID osal_memcpy( newAddr, pStaticAddr, B_ADDR_LEN );
        }
        else
        {
            // Not passed in - generate a new random address
            smGenerateRandBuf( newAddr, B_ADDR_LEN );
        }
    }
    else if ( addrType == ADDRTYPE_PRIVATE_RESOLVE )
    {
        // Change from Public to Random
        stat = SM_CalcRandomAddr( gapParams.pIRK, newAddr );
    }

    // Save off the new address type
    gapDeviceAddrMode = addrType;
    // Default the Resolvable Private Address Change timer
    VOID osal_stop_timerEx( gapTaskID, GAP_CHANGE_RESOLVABLE_PRIVATE_ADDR_EVT );
    gapPrivateAddrChangeTimeout = 0;

    if ( (stat == SUCCESS) && (addrType != ADDRTYPE_PUBLIC) )
    {
        // Send the new "Random" address to the LL
        stat = gapProcessNewAddr( newAddr );

        if ( (stat == SUCCESS) && (addrType == ADDRTYPE_PRIVATE_RESOLVE) )
        {
            // Private resolvable addresses are periodically changed,
            // so setup a timer for the next change
            gapPrivateAddrChangeTimeout = GAP_GetParamValue( TGAP_PRIVATE_ADDR_INT );

            // Start a timer
            if ( gapPrivateAddrChangeTimeout )
            {
                VOID osal_start_reload_timer( gapTaskID, GAP_CHANGE_RESOLVABLE_PRIVATE_ADDR_EVT,
                                              (uint32)GAP_PRIVATE_ADDR_CHANGE_RESOLUTION );
            }
        }
    }

    if ( stat != SUCCESS )
    {
        // something went wrong, restore address type
        gapDeviceAddrMode = currentAddrMode;
    }

    return ( stat );
}

/*********************************************************************
    @fn          gapReadBD_ADDRStatus

    @brief       Process the HCI Command Complete Event for the
                call to HCI_ReadBDADDRCmd().

    @param       status - SUCESS or error
    @param       bdAddr - pointer to the received device address

    @return      TRUE if expected, FALSE if not.
*/
uint8 gapReadBD_ADDRStatus( uint8 status, uint8* pBdAddr )
{
    uint8 expected = FALSE; // Assume we aren't expecting this HCI message

    if ( (status == SUCCESS) && (pBdAddr != NULL) )
    {
        VOID osal_memcpy( gapParams.BD_ADDR, pBdAddr, B_ADDR_LEN );
    }

    // Are we expecting this message?
    if ( gapParams.state == GAP_INITSTATE_BD_ADDR )
    {
        expected = TRUE;  // Yes, we are expecting this HCI message

        if ( status == SUCCESS )
        {
            // Get the LL HCI Buffer size
            gapParams.state = GAP_INITSTATE_BUFFERSIZE;
            VOID HCI_LE_ReadBufSizeCmd();
        }
        else
        {
            // Error state
            gapParams.state = GAP_INITSTATE_INVALID;
            gapSendDeviceInitDoneEvent( status );
        }
    }

    return ( expected );
}

/*********************************************************************
    @fn          gapReadBufSizeCmdStatus

    @brief       Process the HCI Command Complete Event for the
                call to HCI_BLEReadBufSizeCmd().

    @param       cmdStat - command status parameters

    @return      TRUE if expected, FALSE if not.
*/
uint8 gapReadBufSizeCmdStatus( hciRetParam_LeReadBufSize_t* pCmdStat )
{
    uint8 expected = FALSE; // Assume we aren't expecting this HCI message

    if ( pCmdStat )
    {
        if ( pCmdStat->status == SUCCESS )
        {
            gapParams.HC_LE_Data_Packet_Lenth = pCmdStat->dataPktLen;
            gapParams.HC_Total_Num_LE_Data_Packets = pCmdStat->numDataPkts;
        }

        if ( gapParams.state == GAP_INITSTATE_BUFFERSIZE )
        {
            expected = TRUE;

            if ( pCmdStat->status == SUCCESS )
            {
                gapParams.state = GAP_INITSTATE_READY;
            }
            else
            {
                gapParams.state = GAP_INITSTATE_INVALID;
            }

            gapSendDeviceInitDoneEvent( pCmdStat->status );
        }
    }

    return ( expected );
}

/*********************************************************************
    @fn          gapProcessNewAddr

    @brief       Process the SM New Random Addr Event message.

    @param       newAddr - Pointer to new address

    @return      SUCCESS
*/
bStatus_t gapProcessNewAddr( uint8* pNewAddr )
{
    if ( gapDeviceAddrMode != ADDRTYPE_PUBLIC )
    {
        VOID osal_memcpy( gapCurrentRandomAddr, pNewAddr, B_ADDR_LEN );
        VOID gapAddAddrAdj( gapDeviceAddrMode, gapCurrentRandomAddr );
        return ( HCI_LE_SetRandomAddressCmd( gapCurrentRandomAddr ) );
    }
    else
    {
        return ( SUCCESS );
    }
}

/*********************************************************************
    @fn          gapAddAddrAdj

    @brief       Add the top two bits based on the address type.

    @param       status - operation status - SUCCESS if successful.

    @return      the LL address type ADDRTYPE_PUBLIC or ADDRTYPE_RANDOM
*/
uint8 gapAddAddrAdj( uint8 addrType, uint8* pAddr )
{
    // Assume public address
    uint8 llAddrType = ADDRTYPE_PUBLIC;

    if ( (pAddr) && (addrType != ADDRTYPE_PUBLIC) )
    {
        // Update the Random Address type
        if ( addrType == ADDRTYPE_STATIC )
        {
            // Set the header bits of the Static Address
            pAddr[B_ADDR_LEN-1] |= STATIC_ADDR_HDR;
        }
        else
        {
            // Clear the header bits for Random addresses
            pAddr[B_ADDR_LEN-1] &= ~(RANDOM_ADDR_HDR);

            if ( addrType == ADDRTYPE_PRIVATE_RESOLVE )
            {
                // Set the address header bits
                pAddr[B_ADDR_LEN-1] |= PRIVATE_RESOLVE_ADDR_HDR;
            }
        }

        // If not public, then it must be a random address (LL)
        llAddrType = ADDRTYPE_RANDOM;
    }

    return ( llAddrType );
}

/*********************************************************************
    @fn          gapDetermineAddrType

    @brief       Determine the address type from the address.

    @param       addrType - LL address type
    @param       addr - 6 byte address

    @return      returns the type of Random address
                ADDRTYPE_STATIC, ADDRTYPE_PRIVATE_NONRESOLVE
                or ADDRTYPE_PRIVATE_RESOLVE
*/
uint8 gapDetermineAddrType( uint8 addrType, uint8* pAddr )    // TODO
{
    // Don't need to convert if no address or type is already public
    if ( (pAddr) && (addrType == 0x01)/*(addrType != ADDRTYPE_PUBLIC)*/ )     // HZF: HCI will report public or Random(static or private) for BLE4.0.
    {
        // BLE4.2 add 2 types: RPA_public(0x02) + RPA_static(0x03)
        // Get just the random address header bits
        uint8 addrTypeMask = (uint8)(pAddr[B_ADDR_LEN-1] & RANDOM_ADDR_HDR);

        // Get the address type from the address header
        if ( addrTypeMask == STATIC_ADDR_HDR )
        {
            addrType = ADDRTYPE_STATIC;
        }
        else if ( addrTypeMask == PRIVATE_RESOLVE_ADDR_HDR )
        {
            addrType = ADDRTYPE_PRIVATE_RESOLVE;
        }
        else
        {
            // It must be this, it's the only one left
            addrType = ADDRTYPE_PRIVATE_NONRESOLVE;
        }
    }

    return ( addrType );
}

/*********************************************************************
    @fn          gapProcessRandomAddrComplete

    @brief       Process the HCI Random Address Complete Event message.

    @param       status - operation status - SUCCESS if successful.

    @return      none
*/
void gapProcessRandomAddrComplete( uint8 status )
{
    // Inform the application
    gapSendRandomAddrChangeEvent( status, gapCurrentRandomAddr );

    if ( gapAutoAdvPrivateAddrChange == TRUE )
    {
        if ( pfnPeripheralCBs && pfnPeripheralCBs->pfnSetAdvParams )
        {
            // Force an advertisement parameter update.
            VOID pfnPeripheralCBs->pfnSetAdvParams();
        }

        // Start up the timer again
        VOID osal_start_reload_timer( gapTaskID, GAP_CHANGE_RESOLVABLE_PRIVATE_ADDR_EVT,
                                      (uint32)GAP_PRIVATE_ADDR_CHANGE_RESOLUTION );
    }
}

/*********************************************************************
    @fn          gapGetSRK

    @brief       Get pointer to the SRK.

    @param       none

    @return      pointer to the device's Signature Resolving Key
*/
uint8* gapGetSRK( void )
{
    return ( gapParams.pSRK );
}

/*********************************************************************
    @fn          gapGetSignCounter

    @brief       Get the signature counter.

    @param       none

    @return      the device's Signature counter
*/
uint32 gapGetSignCounter( void )
{
    return ( *gapParams.pSignCounter );
}

/*********************************************************************
    @fn          gapIncSignCounter

    @brief       Increment the signature counter.

    @param       none

    @return      none
*/
void gapIncSignCounter( void )
{
    // Increment the signCounter
    (*(gapParams.pSignCounter))++;

    // Tell the app
    if ( gapAppTaskID != INVALID_TASK_ID )
    {
        VOID osal_set_event( gapAppTaskID, GAP_EVENT_SIGN_COUNTER_CHANGED );
    }
}

/*********************************************************************
    @fn          gapUpdateConnSignCounter

    @brief       Update a link's sign counter.

    @param       none

    @return      none
*/
void gapUpdateConnSignCounter( uint16 connHandle, uint32 newSignCounter )
{
    linkDBItem_t* pConnItem;
    pConnItem = linkDB_Find( connHandle );

    if ( pConnItem )
    {
        uint8 taskID;
        pConnItem->sec.signCounter = newSignCounter;
        taskID = (uint8)GAP_GetParamValue( TGAP_AUTH_TASK_ID );

        if ( taskID == 0 )
        {
            taskID = pConnItem->taskID;
        }

        // Tell the app
        gapSendSignUpdateEvent( taskID, pConnItem->addrType,
                                pConnItem->addr, pConnItem->sec.signCounter );
    }
}

/*********************************************************************
    @fn          gapGetDevAddressMode

    @brief       Get the address mode of this device

    @param       none

    @return      address mode
*/
uint8 gapGetDevAddressMode( void )
{
    return ( gapDeviceAddrMode );
}

/*********************************************************************
    @fn          gapGetDevAddress

    @brief       Get the address of this device

    @param       real - TRUE to always return the BD_ADDR
                     FALSE - could be random if using random

    @return      pointer to device address.
*/
uint8* gapGetDevAddress( uint8 real )
{
    uint8* pAddr = gapParams.BD_ADDR;

    if ( real == FALSE )
    {
        if ( gapDeviceAddrMode != ADDRTYPE_PUBLIC )
        {
            pAddr = gapCurrentRandomAddr;
        }
    }

    return ( pAddr );
}

/*********************************************************************
    @fn          gapGetIRK

    @brief       Get the device's IRK

    @param       none

    @return      pointer to IRK
*/
uint8* gapGetIRK( void )
{
    return ( gapParams.pIRK );
}

/*********************************************************************
    FILE LOCAL FUNCTIONS
*/

/*********************************************************************
    @fn          gapSendDeviceInitDoneEvent

    @brief       Send the GAP_DEVICE_INIT_DONE_EVENT.

    @param       status - Status of initialization

    @return      none
*/
static void gapSendDeviceInitDoneEvent( uint8 status )
{
    if ( gapAppTaskID != INVALID_TASK_ID )
    {
        gapDeviceInitDoneEvent_t* pRsp;
        pRsp = (gapDeviceInitDoneEvent_t*)osal_msg_allocate( (uint16)(sizeof ( gapDeviceInitDoneEvent_t )) );

        if ( pRsp )
        {
            pRsp->hdr.event = GAP_MSG_EVENT;
            pRsp->hdr.status = status;
            pRsp->opcode = GAP_DEVICE_INIT_DONE_EVENT;
            VOID osal_memcpy( pRsp->devAddr, gapParams.BD_ADDR, B_ADDR_LEN );
            pRsp->dataPktLen = gapParams.HC_LE_Data_Packet_Lenth;
            pRsp->numDataPkts = gapParams.HC_Total_Num_LE_Data_Packets;
            VOID osal_msg_send( gapAppTaskID, (uint8*)pRsp );
        }
    }
}

/*********************************************************************
    @fn          gapSendRandomAddrChangeEvent

    @brief       Send the GAP_RANDOM_ADDR_CHANGED_EVENT.

    @param       status - Status of initialization
    @param       pNewAddr - new address

    @return      none
*/
static void gapSendRandomAddrChangeEvent( uint8 status, uint8* pNewAddr )
{
    if ( gapAppTaskID != INVALID_TASK_ID )
    {
        gapRandomAddrEvent_t* pRsp;
        pRsp = (gapRandomAddrEvent_t*)osal_msg_allocate( (uint16)(sizeof ( gapRandomAddrEvent_t )) );

        if ( pRsp )
        {
            pRsp->hdr.event = GAP_MSG_EVENT;
            pRsp->hdr.status = status;
            pRsp->opcode = GAP_RANDOM_ADDR_CHANGED_EVENT;
            pRsp->addrType = gapDeviceAddrMode;
            VOID osal_memcpy( pRsp->newRandomAddr, pNewAddr, B_ADDR_LEN );
            VOID osal_msg_send( gapAppTaskID, (uint8*)pRsp );
        }
    }
}

/*********************************************************************
    @fn          gapSendSignUpdateEvent

    @brief       Send the GAP_SIGNATURE_UPDATED_EVENT.

    @param       taskID - where to send this message
    @param       addrType - device address type
    @param       pDevAddr - device address of affected device
    @param       signCounter - new counter

    @return      none
*/
static void gapSendSignUpdateEvent( uint8 taskID, uint8 addrType, uint8* pDevAddr, uint32 signCounter )
{
    gapSignUpdateEvent_t* pRsp;
    pRsp = (gapSignUpdateEvent_t*)osal_msg_allocate( (uint16)(sizeof ( gapSignUpdateEvent_t )) );

    if ( pRsp )
    {
        pRsp->hdr.event = GAP_MSG_EVENT;
        pRsp->hdr.status = SUCCESS;
        pRsp->opcode = GAP_SIGNATURE_UPDATED_EVENT;
        pRsp->addrType = addrType;
        VOID osal_memcpy( pRsp->devAddr, pDevAddr, B_ADDR_LEN );
        pRsp->signCounter = signCounter;
        VOID osal_msg_send( taskID, (uint8*)pRsp );
    }
}

/*********************************************************************
    @fn          gapSendSlaveSecurityReqEvent

    @brief       Send the GAP_SLAVE_REQUESTED_SECURITY_EVENT.

    @param       taskID - where to send this message
    @param       connHandle - connection Handle
    @param       pDevAddr - device address of affected device
    @param       authReq - authentication request information

    @return      none
*/
void gapSendSlaveSecurityReqEvent( uint8 taskID, uint16 connHandle, uint8* pDevAddr, uint8 authReq )
{
    gapSlaveSecurityReqEvent_t* pRsp;
    pRsp = (gapSlaveSecurityReqEvent_t*)osal_msg_allocate( (uint16)(sizeof ( gapSlaveSecurityReqEvent_t )) );

    if ( pRsp )
    {
        pRsp->hdr.event = GAP_MSG_EVENT;
        pRsp->hdr.status = SUCCESS;
        pRsp->opcode = GAP_SLAVE_REQUESTED_SECURITY_EVENT;
        pRsp->connectionHandle = connHandle;
        VOID osal_memcpy( pRsp->deviceAddr, pDevAddr, B_ADDR_LEN );
        pRsp->authReq = authReq;
        VOID osal_msg_send( taskID, (uint8*)pRsp );
    }
}


/*********************************************************************
*********************************************************************/
