/*************
 hidkbdservice.c
 SDK_LICENSE
***************/

/*********************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "OSAL.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gatt_profile_uuid.h"
#include "linkdb.h"
#include "gattservapp.h"
#include "hidkbdservice.h"
#include "peripheral.h"
#include "hiddev.h"
#include "battservice.h"

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
// HID service
CONST uint8 hidServUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(HID_SERV_UUID), HI_UINT16(HID_SERV_UUID)
};

// HID Boot Keyboard Input Report characteristic
CONST uint8 hidBootKeyInputUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(BOOT_KEY_INPUT_UUID), HI_UINT16(BOOT_KEY_INPUT_UUID)
};

// HID Boot Mouse Input Report characteristic
CONST uint8 hidBootMouseInputUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(BOOT_MOUSE_INPUT_UUID), HI_UINT16(BOOT_MOUSE_INPUT_UUID)
};

// HID Boot Keyboard Output Report characteristic
CONST uint8 hidBootKeyOutputUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(BOOT_KEY_OUTPUT_UUID), HI_UINT16(BOOT_KEY_OUTPUT_UUID)
};

// HID Information characteristic
CONST uint8 hidInfoUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(HID_INFORMATION_UUID), HI_UINT16(HID_INFORMATION_UUID)
};

// HID Report Map characteristic
CONST uint8 hidReportMapUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(REPORT_MAP_UUID), HI_UINT16(REPORT_MAP_UUID)
};

// HID Control Point characteristic
CONST uint8 hidControlPointUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(HID_CTRL_PT_UUID), HI_UINT16(HID_CTRL_PT_UUID)
};

// HID Report characteristic
CONST uint8 hidReportUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(REPORT_UUID), HI_UINT16(REPORT_UUID)
};

// HID Protocol Mode characteristic
CONST uint8 hidProtocolModeUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(PROTOCOL_MODE_UUID), HI_UINT16(PROTOCOL_MODE_UUID)
};

/*********************************************************************
    EXTERNAL VARIABLES
*/

/*********************************************************************
    EXTERNAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/

// HID Information characteristic value
static CONST uint8 hidInfo[HID_INFORMATION_LEN] =
{
    LO_UINT16(0x0111), HI_UINT16(0x0111),             // bcdHID (USB HID version)
    0x00,                                             // bCountryCode
    HID_KBD_FLAGS                                     // Flags
};


static CONST uint8 hidReportMap[] =
{
    #if 0
    0x05, 0x01,  // Usage Page (Generic Desktop)
    0x09, 0x02,  // Usage (Mouse)
    0xA1, 0x01,  // Collection (Application)
    0x85, HID_RPT_ID_MOUSE_IN,  // Report Id (1) 0X01
    0x09, 0x01,  //   Usage (Pointer)
    0xA1, 0x00,  //   Collection (Physical)
    0x05, 0x09,  //     Usage Page (Buttons)
    0x19, 0x01,  //     Usage Minimum (01) - Button 1
    0x29, 0x03,  //     Usage Maximum (03) - Button 3
    0x15, 0x00,  //     Logical Minimum (0)
    0x25, 0x01,  //     Logical Maximum (1)
    0x75, 0x01,  //     Report Size (1)
    0x95, 0x03,  //     Report Count (3)
    0x81, 0x02,  //     Input (Data, Variable, Absolute) - Button states
    0x75, 0x05,  //     Report Size (5)
    0x95, 0x01,  //     Report Count (1)
    0x81, 0x01,  //     Input (Constant) - Padding or Reserved bits
    0x05, 0x01,  //     Usage Page (Generic Desktop)
    0x09, 0x30,  //     Usage (X)
    0x09, 0x31,  //     Usage (Y)
    0x09, 0x38,  //     Usage (Wheel)
    0x15, 0x81,  //     Logical Minimum (-127)
    0x25, 0x7F,  //     Logical Maximum (127)
    0x75, 0x08,  //     Report Size (8)
    0x95, 0x03,  //     Report Count (3)
    0x81, 0x06,  //     Input (Data, Variable, Relative) - X & Y coordinate
    0xC0,        //   End Collection
    0xC0,        // End Collection

    0x05, 0x01,  // Usage Pg (Generic Desktop)
    0x09, 0x06,  // Usage (Keyboard)
    0xA1, 0x01,  // Collection: (Application)
    0x85, HID_RPT_ID_KEY_IN,  // Report Id (2)
    //
    0x05, 0x07,  //   Usage Pg (Key Codes)
    0x19, 0xE0,  //   Usage Min (224)
    0x29, 0xE7,  //   Usage Max (231)
    0x15, 0x00,  //   Log Min (0)
    0x25, 0x01,  //   Log Max (1)
    //
    //   Modifier byte
    0x75, 0x01,  //   Report Size (1)
    0x95, 0x08,  //   Report Count (8)
    0x81, 0x02,  //   Input: (Data, Variable, Absolute)
    //
    //   Reserved byte
    0x95, 0x01,  //   Report Count (1)
    0x75, 0x08,  //   Report Size (8)
    0x81, 0x01,  //   Input: (Constant)
    //
    //   LED report
    0x95, 0x05,  //   Report Count (5)
    0x75, 0x01,  //   Report Size (1)
    0x05, 0x08,  //   Usage Pg (LEDs)
    0x19, 0x01,  //   Usage Min (1)
    0x29, 0x05,  //   Usage Max (5)
    0x91, 0x02,  //   Output: (Data, Variable, Absolute)
    //
    //   LED report padding
    0x95, 0x01,  //   Report Count (1)
    0x75, 0x03,  //   Report Size (3)
    0x91, 0x01,  //   Output: (Constant)
    //
    //   Key arrays (6 bytes)
    0x95, 0x06,  //   Report Count (6)
    0x75, 0x08,  //   Report Size (8)
    0x15, 0x00,  //   Log Min (0)
    0x25, 0x65,  //   Log Max (101)
    0x05, 0x07,  //   Usage Pg (Key Codes)
    0x19, 0x00,  //   Usage Min (0)
    0x29, 0x65,  //   Usage Max (101)
    0x81, 0x00,  //   Input: (Data, Array)
    //
    0xC0,        // End Collection
    #endif

    #if EN_CONSUMER_MODE
    #if 0
    0x05, 0x0C, // Usage Pg (Consumer Devices)
    0x09, 0x01,   // Usage (Consumer Control)
    0xA1, 0x01,   // Collection (Application)
    0x85, HID_RPT_ID_CC_IN,   // Report Id (3)
    0x09, 0x02,   //   Usage (Numeric Key Pad)
    0xA1, 0x02,   //   Collection (Logical)
    0x05, 0x09,   //     Usage Pg (Button)
    0x19, 0x01,   //     Usage Min (Button 1)
    0x29, 0x0A,   //     Usage Max (Button 10)
    0x15, 0x01,   //     Logical Min (1)
    0x25, 0x0A,   //     Logical Max (10)
    0x75, 0x04,   //     Report Size (4)
    0x95, 0x01,   //     Report Count (1)
    0x81, 0x00,   //     Input (Data, Ary, Abs)
    0xC0,         //   End Collection

    0x05, 0x0C,   //   Usage Pg (Consumer Devices)
    0x09, 0x86,   //   Usage (Channel)
    0x15, 0xFF,   //   Logical Min (-1)
    0x25, 0x01,   //   Logical Max (1)
    0x75, 0x02,   //   Report Size (2)
    0x95, 0x01,   //   Report Count (1)
    0x81, 0x46,   //   Input (Data, Var, Rel, Null)
    0x09, 0xE9,   //   Usage (Volume Up)
    0x09, 0xEA,   //   Usage (Volume Down)
    0x15, 0x00,   //   Logical Min (0)
    0x75, 0x01,   //   Report Size (1)
    0x95, 0x02,   //   Report Count (2)
    0x81, 0x02,   //   Input (Data, Var, Abs)
    0x09, 0xE2,   //   Usage (Mute)
    0x09, 0x30,   //   Usage (Power)
    0x09, 0x83,   //   Usage (Recall Last)
    0x09, 0x81,   //   Usage (Assign Selection)
    0x09, 0xB0,   //   Usage (Play)
    0x09, 0xB1,   //   Usage (Pause)
    0x09, 0xB2,   //   Usage (Record)
    0x09, 0xB3,   //   Usage (Fast Forward)
    0x09, 0xB4,   //   Usage (Rewind)
    0x09, 0xB5,   //   Usage (Scan Next)
    0x09, 0xB6,   //   Usage (Scan Prev)
    0x09, 0xB7,   //   Usage (Stop)
    0x15, 0x01,   //   Logical Min (1)
    0x25, 0x0C,   //   Logical Max (12)
    0x75, 0x04,   //   Report Size (4)
    0x95, 0x01,   //   Report Count (1)
    0x81, 0x00,   //   Input (Data, Ary, Abs)
    0x09, 0x80,   //   Usage (Selection)
    0xA1, 0x02,   //   Collection (Logical)
    0x05, 0x09,   //     Usage Pg (Button)
    0x19, 0x01,   //     Usage Min (Button 1)
    0x29, 0x03,   //     Usage Max (Button 3)
    0x15, 0x01,   //     Logical Min (1)
    0x25, 0x03,   //     Logical Max (3)
    0x75, 0x02,   //     Report Size (2)
    0x81, 0x00,   //     Input (Data, Ary, Abs)
    0xC0,         //   End Collection
    0x81, 0x03,   //   Input (Const, Var, Abs)
		0xC0                /*  End Collection                              */
    #else
		0x05, 0x01,         /*  Usage Page (Desktop),                   */
		0x09, 0x06,         /*  Usage (Keyboard),                       */
		0xA1, 0x01,         /*  Collection (Application),               */
		0x85, 0x02,         /*      Report ID (1),                      */
		0x05, 0x07,         /*      Usage Page (Keyboard),              */
		0x19, 0xE0,         /*      Usage Minimum (KB Leftcontrol),     */
		0x29, 0xE7,         /*      Usage Maximum (KB Right GUI),       */
		0x15, 0x00,         /*      Logical Minimum (0),                */
		0x25, 0x01,         /*      Logical Maximum (1),                */
		0x75, 0x01,         /*      Report Size (1),                    */
		0x95, 0x08,         /*      Report Count (8),                   */
		0x81, 0x02,         /*      Input (Variable),                   */
		0x95, 0x01,         /*      Report Count (1),                   */
		0x75, 0x08,         /*      Report Size (8),                    */
		0x81, 0x01,         /*      Input (Constant),                   */
		0x95, 0x05,         /*      Report Count (5),                   */
		0x75, 0x01,         /*      Report Size (1),                    */
		0x05, 0x08,         /*      Usage Page (LED),                   */
		0x19, 0x01,         /*      Usage Minimum (01h),                */
		0x29, 0x05,         /*      Usage Maximum (05h),                */
		0x91, 0x02,         /*      Output (Variable),                  */
		0x95, 0x01,         /*      Report Count (1),                   */
		0x75, 0x03,         /*      Report Size (3),                    */
		0x91, 0x01,         /*      Output (Constant),                  */
		0x95, 0x06,         /*      Report Count (6),                   */
		0x75, 0x08,         /*      Report Size (8),                    */
		0x15, 0x00,         /*      Logical Minimum (0),                */
		0x25, 0x65,         /*      Logical Maximum (101),              */
		0x05, 0x07,         /*      Usage Page (Keyboard),              */
		0x19, 0x00,         /*      Usage Minimum (None),               */
		0x29, 0x65,         /*      Usage Maximum (KB Application),     */
		0x81, 0x00,         /*      Input,                              */
		0xC0,               /*  End Collection,                         */

		0x05, 0x0C,         /*  Usage Page (Consumer),                  */
		0x09, 0x01,         /*  Usage (Consumer Control),               */
		0xA1, 0x01,         /*  Collection (Application),               */
		0x85, 0x03,         /*      Report ID (2),                      */
		0x75, 0x10,         /*      Report Size (16),                   */
		0x95, 0x01,         /*      Report Count (1),                   */
		0x15, 0x01,         /*      Logical Minimum (1),                */
		0x26, 0x8C, 0x02,   /*      Logical Maximum (652),              */
		0x19, 0x01,         /*      Usage Minimum (Consumer Control),   */
		0x2A, 0x8C, 0x02,   /*      Usage Maximum (AC Send),            */
		0x81, 0x60,         /*      Input (No Preferred, Null State),   */
		0xC0,               /*  End Collection,                         */
		
		#endif

    #endif

};




// HID report map length
uint16 hidReportMapLen = sizeof(hidReportMap);

// HID report mapping table
hidRptMap_t  hidRptMap[HID_NUM_REPORTS];

/*********************************************************************
    Profile Attributes - variables
*/

// HID Service attribute
static CONST gattAttrType_t hidService = { ATT_BT_UUID_SIZE, hidServUUID };

// Include attribute (Battery service)
static uint16 include = GATT_INVALID_HANDLE;

// HID Information characteristic
static uint8 hidInfoProps = GATT_PROP_READ;

// HID Report Map characteristic
static uint8 hidReportMapProps = GATT_PROP_READ;

// HID External Report Reference Descriptor
static uint8 hidExtReportRefDesc[ATT_BT_UUID_SIZE] =
{ LO_UINT16(BATT_LEVEL_UUID), HI_UINT16(BATT_LEVEL_UUID) };

// HID Control Point characteristic
static uint8 hidControlPointProps = GATT_PROP_WRITE_NO_RSP;
static uint8 hidControlPoint;

// HID Protocol Mode characteristic
static uint8 hidProtocolModeProps = GATT_PROP_READ | GATT_PROP_WRITE_NO_RSP;
uint8 hidProtocolMode = HID_PROTOCOL_MODE_REPORT;

// HID Report characteristic, key input
static uint8 hidReportKeyInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8 hidReportKeyIn;
static gattCharCfg_t hidReportKeyInClientCharCfg[GATT_MAX_NUM_CONN];

// HID Report Reference characteristic descriptor, key input
static uint8 hidReportRefKeyIn[HID_REPORT_REF_LEN] =
{ HID_RPT_ID_KEY_IN, HID_REPORT_TYPE_INPUT };

// HID Report characteristic, LED output
static uint8 hidReportLedOutProps = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;
static uint8 hidReportLedOut;

// HID Report Reference characteristic descriptor, LED output
static uint8 hidReportRefLedOut[HID_REPORT_REF_LEN] =
{ HID_RPT_ID_LED_OUT, HID_REPORT_TYPE_OUTPUT };

// HID Boot Keyboard Input Report
static uint8 hidReportBootKeyInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8 hidReportBootKeyIn;
static gattCharCfg_t hidReportBootKeyInClientCharCfg[GATT_MAX_NUM_CONN];

// HID Boot Keyboard Output Report
static uint8 hidReportBootKeyOutProps = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;
static uint8 hidReportBootKeyOut;

// HID Boot Mouse Input Report
static uint8 hidReportBootMouseInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8 hidReportBootMouseIn;
static gattCharCfg_t hidReportBootMouseInClientCharCfg[GATT_MAX_NUM_CONN];

// Feature Report
static uint8 hidReportFeatureProps = GATT_PROP_READ | GATT_PROP_WRITE;
static uint8 hidReportFeature;

// HID Report Reference characteristic descriptor, Feature
static uint8 hidReportRefFeature[HID_REPORT_REF_LEN] =
{ HID_RPT_ID_FEATURE, HID_REPORT_TYPE_FEATURE };

#if EN_MOUSE_REPORT

// HID Report Reference characteristic descriptor, mouse input
static uint8 hidReportRefMouseIn[HID_REPORT_REF_LEN] =
{ HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT };

static uint8 hidReportMouseInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8 hidReportMouseIn;
static gattCharCfg_t hidReportMouseInClientCharCfg[GATT_MAX_NUM_CONN];

#endif
#if EN_CONSUMER_MODE
// HID Report Reference characteristic descriptor, consumer control input
static uint8 hidReportRefCCIn[HID_REPORT_REF_LEN] =
{ HID_RPT_ID_CC_IN, HID_REPORT_TYPE_INPUT };

// HID Report characteristic, consumer control input
static uint8 hidReportCCInProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8 hidReportCCIn;
static gattCharCfg_t hidReportCCInClientCharCfg[GATT_MAX_NUM_CONN];

#endif



/*********************************************************************
    Profile Attributes - Table
*/

static gattAttribute_t hidAttrTbl[] =
{
    // HID Service
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
        GATT_PERMIT_READ,                         /* permissions */
        0,                                        /* handle */
        (uint8*)& hidService                      /* pValue */
    },

    // Included service (battery)
    {
        { ATT_BT_UUID_SIZE, includeUUID },
        GATT_PERMIT_READ,
        0,
        (uint8*)& include
    },
    #if 1

    // HID Information characteristic declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &hidInfoProps
    },

    // HID Information characteristic
    {
        { ATT_BT_UUID_SIZE, hidInfoUUID },
        GATT_PERMIT_ENCRYPT_READ,
        0,
        (uint8*) hidInfo
    },
    #endif

    // HID Control Point characteristic declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &hidControlPointProps
    },

    // HID Control Point characteristic
    {
        { ATT_BT_UUID_SIZE, hidControlPointUUID },
        GATT_PERMIT_ENCRYPT_WRITE,
        0,
        &hidControlPoint
    },

    // HID Protocol Mode characteristic declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &hidProtocolModeProps
    },

    // HID Protocol Mode characteristic
    {
        { ATT_BT_UUID_SIZE, hidProtocolModeUUID },
        GATT_PERMIT_ENCRYPT_READ | GATT_PERMIT_ENCRYPT_WRITE,
        0,
        &hidProtocolMode
    },


    // HID Report Map characteristic declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &hidReportMapProps
    },

    // HID Report Map characteristic
    {
        { ATT_BT_UUID_SIZE, hidReportMapUUID },
        GATT_PERMIT_ENCRYPT_READ,
        0,
        (uint8*) hidReportMap
    },

    // HID External Report Reference Descriptor
    {
        { ATT_BT_UUID_SIZE, extReportRefUUID },
        GATT_PERMIT_READ,
        0,
        hidExtReportRefDesc
    },
    #if EN_MOUSE_REPORT
// HID Report characteristic, mouse input declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &hidReportMouseInProps
    },

    // HID Report characteristic, mouse input
    {
        { ATT_BT_UUID_SIZE, hidReportUUID },
        GATT_PERMIT_ENCRYPT_READ,
        0,
        &hidReportMouseIn
    },

    // HID Report characteristic client characteristic configuration
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE,
        0,
        (uint8*)& hidReportMouseInClientCharCfg
    },

    // HID Report Reference characteristic descriptor, mouse input
    {
        { ATT_BT_UUID_SIZE, reportRefUUID },
        GATT_PERMIT_READ,
        0,
        hidReportRefMouseIn
    },


    #endif


    // HID Report characteristic, key input declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &hidReportKeyInProps
    },

    // HID Report characteristic, key input
    {
        { ATT_BT_UUID_SIZE, hidReportUUID },
        GATT_PERMIT_ENCRYPT_READ,
        0,
        &hidReportKeyIn
    },

    // HID Report characteristic client characteristic configuration
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE,
        0,
        (uint8*)& hidReportKeyInClientCharCfg
    },

    // HID Report Reference characteristic descriptor, key input
    {
        { ATT_BT_UUID_SIZE, reportRefUUID },
        GATT_PERMIT_READ,
        0,
        hidReportRefKeyIn
    },

    // HID Report characteristic, LED output declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &hidReportLedOutProps
    },

    // HID Report characteristic, LED output
    {
        { ATT_BT_UUID_SIZE, hidReportUUID },
        GATT_PERMIT_ENCRYPT_READ | GATT_PERMIT_ENCRYPT_WRITE,
        0,
        &hidReportLedOut
    },

    // HID Report Reference characteristic descriptor, LED output
    {
        { ATT_BT_UUID_SIZE, reportRefUUID },
        GATT_PERMIT_READ,
        0,
        hidReportRefLedOut
    },

    // HID Boot Keyboard Input Report declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &hidReportBootKeyInProps
    },

    // HID Boot Keyboard Input Report
    {
        { ATT_BT_UUID_SIZE, hidBootKeyInputUUID },
        GATT_PERMIT_ENCRYPT_READ,
        0,
        &hidReportBootKeyIn
    },

    // HID Boot Keyboard Input Report characteristic client characteristic configuration
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE,
        0,
        (uint8*)& hidReportBootKeyInClientCharCfg
    },

    // HID Boot Keyboard Output Report declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &hidReportBootKeyOutProps
    },

    // HID Boot Keyboard Output Report
    {
        { ATT_BT_UUID_SIZE, hidBootKeyOutputUUID },
        GATT_PERMIT_ENCRYPT_READ | GATT_PERMIT_ENCRYPT_WRITE,
        0,
        &hidReportBootKeyOut
    },

    #if EN_CONSUMER_MODE
    // HID Report characteristic declaration, consumer control
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &hidReportCCInProps
    },

    // HID Report characteristic, consumer control
    {
        { ATT_BT_UUID_SIZE, hidReportUUID },
        GATT_PERMIT_ENCRYPT_READ,
        0,
        &hidReportCCIn
    },

    // HID Report characteristic client characteristic configuration, consumer control
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE,
        0,
        (uint8*)& hidReportCCInClientCharCfg
    },

    // HID Report Reference characteristic descriptor, consumer control
    {
        { ATT_BT_UUID_SIZE, reportRefUUID },
        GATT_PERMIT_READ,
        0,
        hidReportRefCCIn
    },

    #endif

    // HID Boot Mouse Input Report declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &hidReportBootMouseInProps
    },

    // HID Boot Mouse Input Report
    {
        { ATT_BT_UUID_SIZE, hidBootMouseInputUUID },
        GATT_PERMIT_ENCRYPT_READ,
        0,
        &hidReportBootMouseIn
    },

    // HID Boot Mouse Input Report characteristic client characteristic configuration
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_ENCRYPT_WRITE,
        0,
        (uint8*)& hidReportBootMouseInClientCharCfg
    },

    // Feature Report declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &hidReportFeatureProps
    },

    // Feature Report
    {
        { ATT_BT_UUID_SIZE,  hidReportUUID},
        GATT_PERMIT_ENCRYPT_READ | GATT_PERMIT_ENCRYPT_WRITE,
        0,
        &hidReportFeature
    },

    // HID Report Reference characteristic descriptor, feature
    {
        { ATT_BT_UUID_SIZE, reportRefUUID },
        GATT_PERMIT_READ,
        0,
        hidReportRefFeature
    },


    #if 0

    // HID Information characteristic declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &hidInfoProps
    },

    // HID Information characteristic
    {
        { ATT_BT_UUID_SIZE, hidInfoUUID },
        GATT_PERMIT_ENCRYPT_READ,
        0,
        (uint8*) hidInfo
    },
    #endif
};

/*********************************************************************
    LOCAL FUNCTIONS
*/

/*********************************************************************
    PROFILE CALLBACKS
*/

// Service Callbacks
CONST gattServiceCBs_t hidKbdCBs =
{
    HidDev_ReadAttrCB,  // Read callback function pointer
    HidDev_WriteAttrCB, // Write callback function pointer
    NULL                // Authorization callback function pointer
};

/*********************************************************************
    PUBLIC FUNCTIONS
*/

/*********************************************************************
    @fn      HidKbd_AddService

    @brief   Initializes the HID Service by registering
            GATT attributes with the GATT server.

    @return  Success or Failure
*/
bStatus_t HidKbd_AddService( void )
{
    uint8 status = SUCCESS;
    // Initialize Client Characteristic Configuration attributes
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, hidReportKeyInClientCharCfg );
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, hidReportBootKeyInClientCharCfg );
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, hidReportBootMouseInClientCharCfg );
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( hidAttrTbl, GATT_NUM_ATTRS( hidAttrTbl ), &hidKbdCBs );
    // Set up included service
    Batt_GetParameter( BATT_PARAM_SERVICE_HANDLE,
                       &GATT_INCLUDED_HANDLE( hidAttrTbl, HID_INCLUDED_SERVICE_IDX ) );
    // Construct map of reports to characteristic handles
    // Each report is uniquely identified via its ID and type
    // Key input report
    hidRptMap[0].id = hidReportRefKeyIn[0];
    hidRptMap[0].type = hidReportRefKeyIn[1];
    hidRptMap[0].handle = hidAttrTbl[HID_REPORT_KEY_IN_IDX].handle;
    hidRptMap[0].cccdHandle = hidAttrTbl[HID_REPORT_KEY_IN_CCCD_IDX].handle;
    hidRptMap[0].mode = HID_PROTOCOL_MODE_REPORT;
    // LED output report
    hidRptMap[1].id = hidReportRefLedOut[0];
    hidRptMap[1].type = hidReportRefLedOut[1];
    hidRptMap[1].handle = hidAttrTbl[HID_REPORT_LED_OUT_IDX].handle;
    hidRptMap[1].cccdHandle = 0;
    hidRptMap[1].mode = HID_PROTOCOL_MODE_REPORT;
    // Boot keyboard input report
    // Use same ID and type as key input report
    hidRptMap[2].id = hidReportRefKeyIn[0];
    hidRptMap[2].type = hidReportRefKeyIn[1];
    hidRptMap[2].handle = hidAttrTbl[HID_BOOT_KEY_IN_IDX].handle;
    hidRptMap[2].cccdHandle = hidAttrTbl[HID_BOOT_KEY_IN_CCCD_IDX].handle;
    hidRptMap[2].mode = HID_PROTOCOL_MODE_BOOT;
    // Boot keyboard output report
    // Use same ID and type as LED output report
    hidRptMap[3].id = hidReportRefLedOut[0];
    hidRptMap[3].type = hidReportRefLedOut[1];
    hidRptMap[3].handle = hidAttrTbl[HID_BOOT_KEY_OUT_IDX].handle;
    hidRptMap[3].cccdHandle = 0;
    hidRptMap[3].mode = HID_PROTOCOL_MODE_BOOT;
    // Boot mouse input report
    hidRptMap[4].id = HID_RPT_ID_MOUSE_IN;
    hidRptMap[4].type = HID_REPORT_TYPE_INPUT;
    hidRptMap[4].handle = hidAttrTbl[HID_BOOT_MOUSE_IN_IDX].handle;
    hidRptMap[4].cccdHandle = hidAttrTbl[HID_BOOT_MOUSE_IN_CCCD_IDX].handle;
    hidRptMap[4].mode = HID_PROTOCOL_MODE_BOOT;
    // Feature report
    hidRptMap[5].id = hidReportRefFeature[0];
    hidRptMap[5].type = hidReportRefFeature[1];
    hidRptMap[5].handle = hidAttrTbl[HID_FEATURE_IDX].handle;
    hidRptMap[5].cccdHandle = 0;
    hidRptMap[5].mode = HID_PROTOCOL_MODE_REPORT;
    // Battery level input report
    Batt_GetParameter( BATT_PARAM_BATT_LEVEL_IN_REPORT, &(hidRptMap[6]) );
    #if EN_MOUSE_REPORT
    // Mouse input report
    hidRptMap[7].id = hidReportRefMouseIn[0];
    hidRptMap[7].type = hidReportRefMouseIn[1];
    hidRptMap[7].handle = hidAttrTbl[HID_REPORT_MOUSE_IN_IDX].handle;
    hidRptMap[7].cccdHandle = hidAttrTbl[HID_REPORT_MOUSE_IN_CCCD_IDX].handle;
    hidRptMap[7].mode = HID_PROTOCOL_MODE_REPORT;
    #endif
    #if EN_CONSUMER_MODE
    // Consumer Control input report
    hidRptMap[8].id = hidReportRefCCIn[0];
    hidRptMap[8].type = hidReportRefCCIn[1];
    hidRptMap[8].handle = hidAttrTbl[HID_REPORT_CC_IN_IDX].handle;
    hidRptMap[8].cccdHandle = hidAttrTbl[HID_REPORT_CC_IN_CCCD_IDX].handle;
    hidRptMap[8].mode = HID_PROTOCOL_MODE_REPORT;
    #endif
    // Setup report ID map
    HidDev_RegisterReports( HID_NUM_REPORTS, hidRptMap );
    return ( status );
}

/*********************************************************************
    @fn      HidKbd_SetParameter

    @brief   Set a HID Kbd parameter.

    @param   id - HID report ID.
    @param   type - HID report type.
    @param   uuid - attribute uuid.
    @param   len - length of data to right.
    @param   pValue - pointer to data to write.  This is dependent on
            the input parameters and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  GATT status code.
*/
uint8 HidKbd_SetParameter( uint8 id, uint8 type, uint16 uuid, uint16 len, void* pValue )
{
    bStatus_t ret = SUCCESS;

    switch ( uuid )
    {
    case REPORT_UUID:
        if ( type ==  HID_REPORT_TYPE_OUTPUT )
        {
            if ( len == 1 )
            {
                hidReportLedOut = *((uint8*)pValue);
            }
            else
            {
                ret = ATT_ERR_INVALID_VALUE_SIZE;
            }
        }
        else if ( type == HID_REPORT_TYPE_FEATURE )
        {
            if ( len == 1 )
            {
                hidReportFeature = *((uint8*)pValue);
            }
            else
            {
                ret = ATT_ERR_INVALID_VALUE_SIZE;
            }
        }
        else
        {
            ret = ATT_ERR_ATTR_NOT_FOUND;
        }

        break;

    case BOOT_KEY_OUTPUT_UUID:
        if ( len == 1 )
        {
            hidReportBootKeyOut = *((uint8*)pValue);
        }
        else
        {
            ret = ATT_ERR_INVALID_VALUE_SIZE;
        }

        break;

    default:
        // ignore the request
        break;
    }

    return ( ret );
}

/*********************************************************************
    @fn      HidKbd_GetParameter

    @brief   Get a HID Kbd parameter.

    @param   id - HID report ID.
    @param   type - HID report type.
    @param   uuid - attribute uuid.
    @param   pLen - length of data to be read
    @param   pValue - pointer to data to get.  This is dependent on
            the input parameters and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  GATT status code.
*/
uint8 HidKbd_GetParameter( uint8 id, uint8 type, uint16 uuid, uint16* pLen, void* pValue )
{
    switch ( uuid )
    {
    case REPORT_UUID:
        if ( type ==  HID_REPORT_TYPE_OUTPUT )
        {
            *((uint8*)pValue) = hidReportLedOut;
            *pLen = 1;
        }
        else if ( type == HID_REPORT_TYPE_FEATURE )
        {
            *((uint8*)pValue) = hidReportFeature;
            *pLen = 1;
        }
        else
        {
            *pLen = 0;
        }

        break;

    case BOOT_KEY_OUTPUT_UUID:
        *((uint8*)pValue) = hidReportBootKeyOut;
        *pLen = 1;
        break;

    default:
        *pLen = 0;
        break;
    }

    return ( SUCCESS );
}

/*********************************************************************
*********************************************************************/
