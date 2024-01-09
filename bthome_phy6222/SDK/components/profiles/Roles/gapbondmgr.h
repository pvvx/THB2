/*************
 SDK_LICENSE
***************/
/**
    @headerfile:    gapbondmgr.h
    $Date:
    $Revision:

    @mainpage  BLE GAP Bond Manager

    This GAP profile manages bonded connections between devices.<BR><BR>


*/

#ifndef GAPBONDMGR_H
#define GAPBONDMGR_H

#ifdef __cplusplus
extern "C"
{
#endif

/*  -------------------------------------------------------------------
    INCLUDES
*/
#include "gap.h"

/*  -------------------------------------------------------------------
    CONSTANTS
*/

#if !defined ( GAP_BONDINGS_MAX )
#define GAP_BONDINGS_MAX    10    //!< Maximum number of bonds that can be saved in NV.
#endif

#if !defined ( GAP_BOND_MGR_INDEX_REPLACE )
#define GAP_BOND_MGR_INDEX_REPLACE  TRUE    //!< Maximum number of bonds that can be saved in NV.
#endif

#if !defined ( GAP_CHAR_CFG_MAX )
#define GAP_CHAR_CFG_MAX    4    //!< Maximum number of characteristic configuration that can be saved in NV.
#endif
/** @defgroup GAPBOND_CONSTANTS_NAME GAP Bond Manager Constants
    @{
*/

/** @} End GAPBOND_CONSTANTS_NAME */

/** @defgroup GAPBOND_PROFILE_PARAMETERS GAP Bond Manager Parameters
    @{
*/
#define GAPBOND_PAIRING_MODE       0x400  //!< Pairing Mode: @ref  GAPBOND_PAIRING_MODE_DEFINES. Read/Write. Size is uint8. Default is GAPBOND_PAIRING_MODE_WAIT_FOR_REQ.
#define GAPBOND_INITIATE_WAIT      0x401  //!< Pairing Mode Initiate wait timeout.  This is the time it will wait for a Pairing Request before sending the Slave Initiate Request. Read/Write. Size is uint16. Default is 1000(in milliseconds).
#define GAPBOND_MITM_PROTECTION    0x402  //!< Man-In-The-Middle (MITM) basically turns on Passkey protection in the pairing algorithm. Read/Write. Size is uint8. Default is 0(disabled).
#define GAPBOND_IO_CAPABILITIES    0x403  //!< I/O capabilities.  Read/Write. Size is uint8. Default is GAPBOND_IO_CAP_DISPLAY_ONLY @ref GAPBOND_IO_CAP_DEFINES.
#define GAPBOND_OOB_ENABLED        0x404  //!< OOB data available for pairing algorithm. Read/Write. Size is uint8. Default is 0(disabled).
#define GAPBOND_OOB_DATA           0x405  //!< OOB Data. Read/Write. size uint8[16]. Default is all 0's.
#define GAPBOND_BONDING_ENABLED    0x406  //!< Request Bonding during the pairing process if enabled.  Read/Write. Size is uint8. Default is 0(disabled).
#define GAPBOND_KEY_DIST_LIST      0x407  //!< The key distribution list for bonding.  size is uint8.  @ref GAPBOND_KEY_DIST_DEFINES. Default is sEncKey, sIdKey, mIdKey, mSign enabled.
#define GAPBOND_DEFAULT_PASSCODE   0x408  //!< The default passcode for MITM protection. size is uint32. Range is 0 - 999,999. Default is 0.
#define GAPBOND_ERASE_ALLBONDS     0x409  //!< Erase all of the bonded devices. Write Only. No Size.
#define GAPBOND_AUTO_FAIL_PAIRING  0x40A  //!< TEST MODE (DO NOT USE) to automatically send a Pairing Fail when a Pairing Request is received. Read/Write. size is uint8. Default is 0 (disabled).
#define GAPBOND_AUTO_FAIL_REASON   0x40B  //!< TEST MODE (DO NOT USE) Pairing Fail reason when auto failing. Read/Write. size is uint8. Default is 0x05 (SMP_PAIRING_FAILED_NOT_SUPPORTED).
#define GAPBOND_KEYSIZE            0x40C  //!< Key Size used in pairing. Read/Write. size is uint8. Default is 16.
#define GAPBOND_AUTO_SYNC_WL       0x40D  //!< Clears the White List adds to it each unique address stored by bonds in NV. Read/Write. Size is uint8. Default is FALSE.
#define GAPBOND_BOND_COUNT         0x40E  //!< Gets the total number of bonds stored in NV. Read Only. Size is uint8. Default is 0 (no bonds).
#define GAPBOND_BOND_FAIL_ACTION   0x40F  //!< Possible actions Central may take upon an unsuccessful bonding. Write Only. Size is uint8. Default is 0x02 (Terminate link upon unsuccessful bonding).
#define GAPBOND_ERASE_SINGLEBOND   0x410  //!< Erase a single bonded device. Write only. Must provide address type followed by device address.

/** @} End GAPBOND_PROFILE_PARAMETERS */

/** @defgroup GAPBOND_PAIRING_MODE_DEFINES GAP Bond Manager Pairing Modes
    @{
*/
#define GAPBOND_PAIRING_MODE_NO_PAIRING          0x00  //!< Pairing is not allowed
#define GAPBOND_PAIRING_MODE_WAIT_FOR_REQ        0x01  //!< Wait for a pairing request or slave security request
#define GAPBOND_PAIRING_MODE_INITIATE            0x02  //!< Don't wait, initiate a pairing request or slave security request
/** @} End GAPBOND_PAIRING_MODE_DEFINES */

/** @defgroup GAPBOND_IO_CAP_DEFINES GAP Bond Manager I/O Capabilities
    @{
*/
#define GAPBOND_IO_CAP_DISPLAY_ONLY              0x00  //!< Display Only Device
#define GAPBOND_IO_CAP_DISPLAY_YES_NO            0x01  //!< Display and Yes and No Capable
#define GAPBOND_IO_CAP_KEYBOARD_ONLY             0x02  //!< Keyboard Only
#define GAPBOND_IO_CAP_NO_INPUT_NO_OUTPUT        0x03  //!< No Display or Input Device
#define GAPBOND_IO_CAP_KEYBOARD_DISPLAY          0x04  //!< Both Keyboard and Display Capable
/** @} End GAPBOND_IO_CAP_DEFINES */

/** @defgroup GAPBOND_KEY_DIST_DEFINES GAP Bond Manager Key Distribution
    @{
*/
#define GAPBOND_KEYDIST_SENCKEY                  0x01  //!< Slave Encryption Key
#define GAPBOND_KEYDIST_SIDKEY                   0x02  //!< Slave IRK and ID information
#define GAPBOND_KEYDIST_SSIGN                    0x04  //!< Slave CSRK
#define GAPBOND_KEYDIST_MENCKEY                  0x08  //!< Master Encrypton Key
#define GAPBOND_KEYDIST_MIDKEY                   0x10  //!< Master IRK and ID information
#define GAPBOND_KEYDIST_MSIGN                    0x20  //!< Master CSRK
/** @} End GAPBOND_IO_CAP_DEFINES */


/** @defgroup GAPBOND_PAIRING_STATE_DEFINES GAP Bond Manager Pairing States
    @{
*/
#define GAPBOND_PAIRING_STATE_STARTED             0x00  //!< Pairing started
#define GAPBOND_PAIRING_STATE_COMPLETE            0x01  //!< Pairing complete
#define GAPBOND_PAIRING_STATE_BONDED              0x02  //!< Devices bonded
/** @} End GAPBOND_PAIRING_STATE_DEFINES */

/** @defgroup SMP_PAIRING_FAILED_DEFINES Pairing failure status values
    @{
*/
#define SMP_PAIRING_FAILED_PASSKEY_ENTRY_FAILED   0x01 //!< The user input of the passkey failed, for example, the user cancelled the operation.
#define SMP_PAIRING_FAILED_OOB_NOT_AVAIL          0x02 //!< The OOB data is not available
#define SMP_PAIRING_FAILED_AUTH_REQ               0x03 //!< The pairing procedure can't be performed as authentication requirements can't be met due to IO capabilities of one or both devices
#define SMP_PAIRING_FAILED_CONFIRM_VALUE          0x04 //!< The confirm value doesn't match the calculated compare value
#define SMP_PAIRING_FAILED_NOT_SUPPORTED          0x05 //!< Pairing isn't supported by the device
#define SMP_PAIRING_FAILED_ENC_KEY_SIZE           0x06 //!< The resultant encryption key size is insufficient for the security requirements of this device.
#define SMP_PAIRING_FAILED_CMD_NOT_SUPPORTED      0x07 //!< The SMP command received is not supported on this device.
#define SMP_PAIRING_FAILED_UNSPECIFIED            0x08 //!< Pairing failed due to an unspecified reason
#define SMP_PAIRING_FAILED_REPEATED_ATTEMPTS      0x09 //!< Pairing or authenication procedure is disallowed because too little time has elapsed since the last pairing request or security request.
/** @} End SMP_PAIRING_FAILED_DEFINES */

/** @defgroup GAPBOND_BONDING_FAILURE_DEFINES Bonding Failure Actions
    @{
*/
#define GAPBOND_FAIL_NO_ACTION                         0x00 //!< Take no action upon unsuccessful bonding
#define GAPBOND_FAIL_INITIATE_PAIRING                  0x01 //!< Initiate pairing upon unsuccessful bonding
#define GAPBOND_FAIL_TERMINATE_LINK                    0x02 //!< Terminate link upon unsuccessful bonding
#define GAPBOND_FAIL_TERMINATE_ERASE_BONDS             0x03 //!< Terminate link and erase all existing bonds on device upon unsuccessful bonding
/** @} End GAPBOND_BONDING_FAILURE_DEFINES */

/*  -------------------------------------------------------------------
    TYPEDEFS
*/


/**
    Passcode Callback Function
*/
typedef void (*pfnPasscodeCB_t)
(
    uint8*  deviceAddr,                   //!< address of device to pair with, and could be either public or random.
    uint16 connectionHandle,              //!< Connection handle
    uint8  uiInputs,                      //!< Pairing User Interface Inputs - Ask user to input passcode
    uint8  uiOutputs                      //!< Pairing User Interface Outputs - Display passcode
);

/**
    Pairing State Callback Function
*/
typedef void (*pfnPairStateCB_t)
(
    uint16 connectionHandle,              //!< Connection handle
    uint8  state,                         //!< Pairing state @ref GAPBOND_PAIRING_STATE_DEFINES
    uint8  status                         //!< Pairing status
);

/**
    Callback Registration Structure
*/
typedef struct
{
    pfnPasscodeCB_t     passcodeCB;       //!< Passcode callback
    pfnPairStateCB_t    pairStateCB;      //!< Pairing state callback
} gapBondCBs_t;

/*  -------------------------------------------------------------------
    MACROS
*/

/*  -------------------------------------------------------------------
    API FUNCTIONS
*/

/**
    @defgroup GAPROLES_BONDMGR_API GAP Bond Manager API Functions

    @{
*/

/**
    @brief       Set a GAP Bond Manager parameter.

    NOTE: You can call this function with a GAP Parameter ID and it will set the
          GAP Parameter.  GAP Parameters are defined in (gap.h).  Also,
          the "len" field must be set to the size of a "uint16" and the
          "pValue" field must point to a "uint16".

    @param       param - Profile parameter ID: @ref GAPBOND_PROFILE_PARAMETERS
    @param       len - length of data to write
    @param       pValue - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return      SUCCESS or INVALIDPARAMETER (invalid paramID)
*/
extern bStatus_t GAPBondMgr_SetParameter( uint16 param, uint8 len, void* pValue );

/**
    @brief       Get a GAP Bond Manager parameter.

    NOTE: You can call this function with a GAP Parameter ID and it will get a
          GAP Parameter.  GAP Parameters are defined in (gap.h).  Also, the
          "pValue" field must point to a "uint16".

    @param       param - Profile parameter ID: @ref GAPBOND_PROFILE_PARAMETERS
    @param       pValue - pointer to location to get the value.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return      SUCCESS or INVALIDPARAMETER (invalid paramID)
*/
extern bStatus_t GAPBondMgr_GetParameter( uint16 param, void* pValue );

/**
    @brief       Notify the Bond Manager that a connection has been made.

     NOTE:      The GAP Peripheral/Central Role profile will
                call this function, if they are included in the project.

    @param       addrType - device's address type. Reference GAP_ADDR_TYPE_DEFINES in gap.h
    @param       pDevAddr - device's address
    @param       connHandle - connection handle
    @param       role - master or slave role.  Reference GAP_PROFILE_ROLE_DEFINES in gap.h

    @return      SUCCESS, otherwise failure
*/
extern bStatus_t GAPBondMgr_LinkEst( uint8 addrType, uint8* pDevAddr, uint16 connHandle, uint8 role );

/**
    @brief       Resolve an address from bonding information.

    @param       addrType - device's address type. Reference GAP_ADDR_TYPE_DEFINES in gap.h
    @param       pDevAddr - device's address
    @param       pResolvedAddr - pointer to buffer to put the resolved address

    @return      bonding index (0 - (GAP_BONDINGS_MAX-1) if found,
                GAP_BONDINGS_MAX if not found
*/
extern uint8 GAPBondMgr_ResolveAddr( uint8 addrType, uint8* pDevAddr, uint8* pResolvedAddr );

/**
    @brief       Set/clear the service change indication in a bond record.

    @param       connectionHandle - connection handle of the connected device or 0xFFFF
                                   if all devices in database.
    @param       setParam - TRUE to set the service change indication, FALSE to clear it.

    @return      SUCCESS - bond record found and changed,<BR>
                bleNoResources - bond record not found (for 0xFFFF connectionHandle),<BR>
                bleNotConnected - connection not found - connectionHandle is invalid (for non-0xFFFF connectionHandle).
*/
extern bStatus_t GAPBondMgr_ServiceChangeInd( uint16 connectionHandle, uint8 setParam );

/**
    @brief       Update the Characteristic Configuration in a bond record.

    @param       connectionHandle - connection handle of the connected device or 0xFFFF
                                   if all devices in database.
    @param       attrHandle - attribute handle.
    @param       value - characteristic configuration value.

    @return      SUCCESS - bond record found and changed,<BR>
                bleNoResources - bond record not found (for 0xFFFF connectionHandle),<BR>
                bleNotConnected - connection not found - connectionHandle is invalid (for non-0xFFFF connectionHandle).
*/
extern bStatus_t GAPBondMgr_UpdateCharCfg( uint16 connectionHandle, uint16 attrHandle, uint16 value );

/**
    @brief       Register callback functions with the bond manager.

     NOTE:      There is no need to register a passcode callback function
                if the passcode will be handled with the GAPBOND_DEFAULT_PASSCODE parameter.

    @param       pCB - pointer to callback function structure.

    @return      none
*/
extern void GAPBondMgr_Register( gapBondCBs_t* pCB );

/**
    @brief       Respond to a passcode request.

    @param       connectionHandle - connection handle of the connected device or 0xFFFF
                                   if all devices in database.
    @param       status - SUCCESS if passcode is available, otherwise see @ref SMP_PAIRING_FAILED_DEFINES.
    @param       passcode - integer value containing the passcode.

    @return      SUCCESS - bond record found and changed,<BR>
                bleIncorrectMode - Link not found.
*/
extern bStatus_t GAPBondMgr_PasscodeRsp( uint16 connectionHandle, uint8 status, uint32 passcode );

/**
    @brief       This is a bypass mechanism to allow the bond manager to process
                GAP messages.

     NOTE:      This is an advanced feature and shouldn't be called unless
                the normal GAP Bond Manager task ID registration is overridden.

    @param       pMsg - GAP event message

    @return      TRUE if safe to deallocate incoming GAP message, FALSE otherwise.
*/
extern uint8 GAPBondMgr_ProcessGAPMsg( gapEventHdr_t* pMsg );

/**
    @brief       This function will check the length of a Bond Manager NV Item.

    @param       id - NV ID.
    @param       len - lengths in bytes of item.

    @return      SUCCESS or FAILURE
*/
extern uint8 GAPBondMgr_CheckNVLen( uint8 id, uint8 len );

/**
    @} End GAPROLES_BONDMGR_API
*/



/*  -------------------------------------------------------------------
    TASK FUNCTIONS - Don't call these. These are system functions.
*/

/**
    @internal

    @brief       Initialization function for the GAP Bond Manager Task.
            This is called during initialization and should contain
            any application specific initialization (ie. hardware
            initialization/setup, table initialization, power up
            notificaiton ... ).

    @param       the ID assigned by OSAL.  This ID should be
                      used to send messages and set timers.

    @return      void
*/
extern void GAPBondMgr_Init( uint8 task_id );

/**
    @internal

    @brief       GAP Bond Manager Task event processor.
            This function is called to process all events for the task.
            Events include timers, messages and any other user defined
            events.

    @param   task_id  - The OSAL assigned task ID.
    @param   events - events to process.  This is a bit map and can
                     contain more than one event.

    @return      events not processed
*/
extern uint16 GAPBondMgr_ProcessEvent( uint8 task_id, uint16 events );

/*  -------------------------------------------------------------------
    -------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* GAPBONDMGR_H */
