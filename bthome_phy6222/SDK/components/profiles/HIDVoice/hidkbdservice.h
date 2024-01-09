/*************
 hidkbdservice.h
 SDK_LICENSE
***************/
#ifndef HIDKBDSERVICE_H
#define HIDKBDSERVICE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
    INCLUDES
*/

/*********************************************************************
    CONSTANTS
*/

#define FOLLOW_TI_MAP   1


#if FOLLOW_TI_MAP
// Number of HID reports defined in the service
#define HID_NUM_REPORTS                  5//7

// HID Report IDs for the service
#define HID_RPT_ID_LED_OUT               0  // LED output report ID
#define HID_RPT_ID_KEY_IN                1  // Keyboard input report ID
#define HID_RPT_ID_CC_IN                 2  // Consumer Control input report ID
#define HID_RPT_ID_VOICE_START_IN        10  // Voice Start input report ID
#define HID_RPT_ID_VOICE_DATA_IN         11  // Voice Data input report ID

//voice defines
#define BLE_VOICE_CMD_STOP                    0x00
#define BLE_VOICE_CMD_START                   0x04
#define HID_VOICE_IN_START_LEN                5


#define HID_RPT_ID_MOUSE_IN 0xFF




enum
{
    HID_SERVICE_IDX,                // HID Service
    HID_INCLUDED_SERVICE_IDX,       // Included Service (battery)
    HID_INFO_DECL_IDX,              // HID Information characteristic declaration
    HID_INFO_IDX,                   // HID Information characteristic
    HID_CONTROL_POINT_DECL_IDX,     // HID Control Point characteristic declaration
    HID_CONTROL_POINT_IDX,          // HID Control Point characteristic
    HID_PROTOCOL_MODE_DECL_IDX,     // HID Protocol Mode characteristic declaration
    HID_PROTOCOL_MODE_IDX,          // HID Protocol Mode characteristic
    HID_REPORT_MAP_DECL_IDX,        // HID Report Map characteristic declaration
    HID_REPORT_MAP_IDX,             // HID Report Map characteristic
    HID_EXT_REPORT_REF_DESC_IDX,    // HID External Report Reference Descriptor
    HID_REPORT_KEY_IN_DECL_IDX,     // HID Report characteristic declaration, key input
    HID_REPORT_KEY_IN_IDX,          // HID Report characteristic, key input
    HID_REPORT_KEY_IN_CCCD_IDX,     // HID Report characteristic client characteristic configuration, key input
    HID_REPORT_REF_KEY_IN_IDX,      // HID Report Reference characteristic descriptor, key input
    HID_REPORT_LED_OUT_DECL_IDX,    // HID Report characteristic, LED output declaration
    HID_REPORT_LED_OUT_IDX,         // HID Report characteristic, LED output
    HID_REPORT_REF_LED_OUT_IDX,     // HID Report Reference characteristic descriptor, LED output
    HID_BOOT_KEY_IN_DECL_IDX,       // HID Boot Keyboard Input Report declaration
    HID_BOOT_KEY_IN_IDX,            // HID Boot Keyboard Input Report
    HID_BOOT_KEY_IN_CCCD_IDX,       // HID Boot Keyboard Input Report characteristic client characteristic configuration
    HID_BOOT_KEY_OUT_DECL_IDX,      // HID Boot Keyboard Output Report declaration
    HID_BOOT_KEY_OUT_IDX,           // HID Boot Keyboard Output Report
    HID_REPORT_CC_IN_DECL_IDX,      // HID Report characteristic declaration, consumer control
    HID_REPORT_CC_IN_IDX,           // HID Report characteristic, consumer control
    HID_REPORT_CC_IN_CCCD_IDX,      // HID Report characteristic client characteristic configuration, consumer control
    HID_REPORT_REF_CC_IN_IDX,       // HID Report Reference characteristic descriptor, consumer control
    HID_VOICE_START_IN_DECL_IDX,    // HID Voice Start Input Report declaration
    HID_VOICE_START_IN_IDX,         // HID Voice Start Input Report
    HID_VOICE_START_IN_CCCD_IDX,    // HID Voice Start Input Report characteristic client characteristic configuration
    HID_REPORT_REF_VOICE_START_IDX, // HID Report Reference characteristic descriptor, Voice Start
    HID_VOICE_DATA_IN_DECL_IDX,     // HID Voice Start Input Report declaration
    HID_VOICE_DATA_IN_IDX,          // HID Voice Start Input Report
    HID_VOICE_DATA_IN_CCCD_IDX,     // HID Voice Start Input Report characteristic client characteristic configuration
    HID_REPORT_REF_VOICE_DATA_IDX,  // HID Report Reference characteristic descriptor, Voice Start
};

#else


// HID Report IDs for the service


#define HID_RPT_ID_MOUSE_IN      1  // Mouse input report ID
#define HID_RPT_ID_KEY_IN        2  // Keyboard input report ID
#define HID_RPT_ID_CC_IN         3



#define HID_RPT_ID_LED_OUT       0  // LED output report ID
#define HID_RPT_ID_FEATURE       0  // Feature report ID


#define EN_VOICE_MODE  1

#define EN_CONSUMER_MODE  1

#define EN_MOUSE_REPORT 1


#if EN_VOICE_MODE

#define HID_RPT_ID_VOICE_START_IN        10  // Voice Start input report ID
#define HID_RPT_ID_VOICE_DATA_IN         11  // Voice Data input report ID

// Number of HID reports defined in the service
#define HID_NUM_REPORTS          11

//voice defines
#define BLE_VOICE_CMD_STOP                    0x00
#define BLE_VOICE_CMD_START                   0x04




#else
// Number of HID reports defined in the service
#define HID_NUM_REPORTS          7


#endif

// Attribute index enumeration-- these indexes match array elements above
enum
{
    HID_SERVICE_IDX,                // HID Service
    HID_INCLUDED_SERVICE_IDX,       // Included Service
    HID_INFO_DECL_IDX,              // HID Information characteristic declaration
    HID_INFO_IDX,                   // HID Information characteristic
    HID_CONTROL_POINT_DECL_IDX,     // HID Control Point characteristic declaration
    HID_CONTROL_POINT_IDX,          // HID Control Point characteristic
    HID_PROTOCOL_MODE_DECL_IDX,     // HID Protocol Mode characteristic declaration
    HID_PROTOCOL_MODE_IDX,          // HID Protocol Mode characteristic
    HID_REPORT_MAP_DECL_IDX,        // HID Report Map characteristic declaration
    HID_REPORT_MAP_IDX,             // HID Report Map characteristic
    HID_EXT_REPORT_REF_DESC_IDX,    // HID External Report Reference Descriptor

    #if EN_MOUSE_REPORT
    HID_REPORT_MOUSE_IN_DECL_IDX,   // HID Report characteristic, mouse input declaration
    HID_REPORT_MOUSE_IN_IDX,         // HID Report characteristic, mouse input
    HID_REPORT_MOUSE_IN_CCCD_IDX,    // HID Report characteristic client characteristic configuration
    HID_REPORT_REF_MOUSE_IN_IDX,     // HID Report Reference characteristic descriptor, mouse input

    #endif

    HID_REPORT_KEY_IN_DECL_IDX,     // HID Report characteristic, key input declaration
    HID_REPORT_KEY_IN_IDX,          // HID Report characteristic, key input
    HID_REPORT_KEY_IN_CCCD_IDX,     // HID Report characteristic client characteristic configuration
    HID_REPORT_REF_KEY_IN_IDX,      // HID Report Reference characteristic descriptor, key input
    HID_REPORT_LED_OUT_DECL_IDX,    // HID Report characteristic, LED output declaration
    HID_REPORT_LED_OUT_IDX,         // HID Report characteristic, LED output
    HID_REPORT_REF_LED_OUT_IDX,     // HID Report Reference characteristic descriptor, LED output
    HID_BOOT_KEY_IN_DECL_IDX,       // HID Boot Keyboard Input Report declaration
    HID_BOOT_KEY_IN_IDX,            // HID Boot Keyboard Input Report
    HID_BOOT_KEY_IN_CCCD_IDX,       // HID Boot Keyboard Input Report characteristic client characteristic configuration
    HID_BOOT_KEY_OUT_DECL_IDX,      // HID Boot Keyboard Output Report declaration
    HID_BOOT_KEY_OUT_IDX,           // HID Boot Keyboard Output Report

    #if EN_CONSUMER_MODE
    HID_REPORT_CC_IN_DECL_IDX,      // HID Report characteristic declaration, consumer control
    HID_REPORT_CC_IN_IDX,           // HID Report characteristic, consumer control
    HID_REPORT_CC_IN_CCCD_IDX,      // HID Report characteristic client characteristic configuration, consumer control
    HID_REPORT_REF_CC_IN_IDX,       // HID Report Reference characteristic descriptor, consumer control
    #endif
    HID_BOOT_MOUSE_IN_DECL_IDX,     // HID Boot Mouse Input Report declaration
    HID_BOOT_MOUSE_IN_IDX,          // HID Boot Mouse Input Report
    HID_BOOT_MOUSE_IN_CCCD_IDX,     // HID Boot Mouse Input Report characteristic client characteristic configuration
    HID_FEATURE_DECL_IDX,           // Feature Report declaration
    HID_FEATURE_IDX,                // Feature Report
    HID_REPORT_REF_FEATURE_IDX      // HID Report Reference characteristic descriptor, feature
    #if EN_VOICE_MODE
    ,
    HID_VOICE_START_IN_DECL_IDX,    // HID Voice Start Input Report declaration
    HID_VOICE_START_IN_IDX,         // HID Voice Start Input Report
    HID_VOICE_START_IN_CCCD_IDX,    // HID Voice Start Input Report characteristic client characteristic configuration
    HID_REPORT_REF_VOICE_START_IDX, // HID Report Reference characteristic descriptor, Voice Start
    HID_VOICE_DATA_IN_DECL_IDX,     // HID Voice Start Input Report declaration
    HID_VOICE_DATA_IN_IDX,          // HID Voice Start Input Report
    HID_VOICE_DATA_IN_CCCD_IDX,     // HID Voice Start Input Report characteristic client characteristic configuration
    HID_REPORT_REF_VOICE_DATA_IDX,  // HID Report Reference characteristic descriptor, Voice Start

    #endif
};

#endif



// HID feature flags
#define HID_KBD_FLAGS             HID_FLAGS_REMOTE_WAKE

/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    MACROS
*/

/*********************************************************************
    Profile Callbacks
*/


/*********************************************************************
    API FUNCTIONS
*/

/*********************************************************************
    @fn      HidKbd_AddService

    @brief   Initializes the HID service for keyboard by registering
            GATT attributes with the GATT server.

    @param   none

    @return  Success or Failure
*/
extern bStatus_t HidKbd_AddService(void);

/*********************************************************************
    @fn      HidKbd_SetParameter

    @brief   Set a HID Kbd parameter.

    @param   id     - HID report ID.
    @param   type   - HID report type.
    @param   uuid   - attribute uuid.
    @param   len    - length of data to right.
    @param   pValue - pointer to data to write.  This is dependent on
            the input parameters and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  GATT status code.
*/
extern uint8 HidKbd_SetParameter( uint8 id, uint8 type, uint16 uuid, uint16 len, void* pValue );

/*********************************************************************
    @fn      HidKbd_GetParameter

    @brief   Get a HID Kbd parameter.

    @param   id     - HID report ID.
    @param   type   - HID report type.
    @param   uuid   - attribute uuid.
    @param   pLen   - length of data to be read.
    @param   pValue - pointer to data to get.  This is dependent on
            the input parameters and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  GATT status code.
*/
extern uint8 HidKbd_GetParameter( uint8 id, uint8 type, uint16 uuid, uint16* pLen, void* pValue );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* HIDKBDSERVICE_H */
