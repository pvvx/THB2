/**************************************************************************************************
 SDK_LICENSE
**************************************************************************************************/


#ifndef __BLE_ANCS_HD_
#define __BLE_ANCS_HD_
#include "stdint.h"

#define ANCS_ATTR_DATA_MAX              32
#define ANCS_NB_OF_CATEGORY_ID          12
#define ANCS_NB_OF_NOTIF_ATTR           8
#define ANCS_NB_OF_APP_ATTR             1
#define ANCS_NB_OF_EVT_ID               3
#define ANCS_APP_ATTR_TX_SIZE           (32+16)

// ANCS: 7905F431-B5CE-4E99-A40F-4B1E122D00D0
#define ANCSAPP_ANCS_SVC_UUID 0xD0, 0x00, 0x2D, 0x12, 0x1E, 0x4B, 0x0F, 0xA4, 0x99, 0x4E, 0xCE, 0xB5, 0x31, 0xF4, 0x05, 0x79
// Notification Source: UUID 9FBF120D-6301-42D9-8C58-25E699A21DBD (notifiable)
#define ANCSAPP_NOTIF_SRC_CHAR_UUID                   0x1DBD
// Control point: UUID 69D1D8F3-45E1-49A8-9821-9BBDFDAAD9D9 (writable with response)
#define ANCSAPP_CTRL_PT_CHAR_UUID                     0xD9D9
// Data Source: UUID 22EAC6E9-24D6-4BB5-BE44-B36ACE7C7BFB (notifiable)
#define ANCSAPP_DATA_SRC_CHAR_UUID                    0x7BFB

#define CHAR_DESC_HDL_UUID128_LEN                     21 // (5 + 16) bytes = 21 bytes.
#define NUMBER_OF_ANCS_CHARS                          3
#define ANCS_NOTIF_UID_LENGTH                         4


typedef enum
{
    BLE_ANCS_CATEGORY_ID_OTHER,                /**< The iOS notification belongs to the "other" category.  */
    BLE_ANCS_CATEGORY_ID_INCOMING_CALL,        /**< The iOS notification belongs to the "Incoming Call" category. */
    BLE_ANCS_CATEGORY_ID_MISSED_CALL,          /**< The iOS notification belongs to the "Missed Call" category. */
    BLE_ANCS_CATEGORY_ID_VOICE_MAIL,           /**< The iOS notification belongs to the "Voice Mail" category. */
    BLE_ANCS_CATEGORY_ID_SOCIAL,               /**< The iOS notification belongs to the "Social" category. */
    BLE_ANCS_CATEGORY_ID_SCHEDULE,             /**< The iOS notification belongs to the "Schedule" category. */
    BLE_ANCS_CATEGORY_ID_EMAIL,                /**< The iOS notification belongs to the "E-mail" category. */
    BLE_ANCS_CATEGORY_ID_NEWS,                 /**< The iOS notification belongs to the "News" category. */
    BLE_ANCS_CATEGORY_ID_HEALTH_AND_FITNESS,   /**< The iOS notification belongs to the "Health and Fitness" category. */
    BLE_ANCS_CATEGORY_ID_BUSINESS_AND_FINANCE, /**< The iOS notification belongs to the "Buisness and Finance" category. */
    BLE_ANCS_CATEGORY_ID_LOCATION,             /**< The iOS notification belongs to the "Location" category. */
    BLE_ANCS_CATEGORY_ID_ENTERTAINMENT         /**< The iOS notification belongs to the "Entertainment" category. */
} ancs_notif_category_id;

/**@brief Event IDs for iOS notifications. */
typedef enum
{
    BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED,     /**< The iOS notification was added. */
    BLE_ANCS_EVENT_ID_NOTIFICATION_MODIFIED,  /**< The iOS notification was modified. */
    BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED    /**< The iOS notification was removed. */
} ancs_notif_evt_id;

#define EVENT_FLAG_silent          (1<<0)  //!< If this flag is set, the notification has a low priority.
#define EVENT_FLAG_important       (1<<1)  //!< If this flag is set, the notification has a high priority.
#define EVENT_FLAG_pre_existing    (1<<2)  //!< If this flag is set, the notification is pre-existing.
#define EVENT_FLAG_positive_action (1<<3)  //!< If this flag is set, the notification has a positive action that can be taken.
#define EVENT_FLAG_negative_action (1<<4)  //!< If this flag is set, the notification has a negative action that can be taken.

#define BLE_ANCS_APP_ATTR_ID_DISPLAY_NAME 0

typedef enum
{
    BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER = 0,     /**< Identifies that the attribute data is of an "App Identifier" type. */
    BLE_ANCS_NOTIF_ATTR_ID_TITLE,                  /**< Identifies that the attribute data is a "Title". */
    BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE,               /**< Identifies that the attribute data is a "Subtitle". */
    BLE_ANCS_NOTIF_ATTR_ID_MESSAGE,                /**< Identifies that the attribute data is a "Message". */
    BLE_ANCS_NOTIF_ATTR_ID_MESSAGE_SIZE,           /**< Identifies that the attribute data is a "Message Size". */
    BLE_ANCS_NOTIF_ATTR_ID_DATE,                   /**< Identifies that the attribute data is a "Date". */
    BLE_ANCS_NOTIF_ATTR_ID_POSITIVE_ACTION_LABEL,  /**< The notification has a "Positive action" that can be executed associated with it. */
    BLE_ANCS_NOTIF_ATTR_ID_NEGATIVE_ACTION_LABEL,  /**< The notification has a "Negative action" that can be executed associated with it. */
} ancs_notif_attr_id;



typedef enum
{
    BLE_ANCS_EVT_DISCOVERY_COMPLETE,
    BLE_ANCS_EVT_DISCOVERY_FAILED,
    BLE_ANCS_EVT_NOTIF,
    BLE_ANCS_EVT_NOTIF_ATTRIBUTE,
    BLE_ANCS_EVT_APP_ATTRIBUTE,
    BLE_ANCS_EVT_NP_ERROR,
} ancs_evt_type_t;


typedef struct
{
    bool          en;   //enable flag
    uint32_t      attr_id;
    uint16_t      attr_len;
    uint8_t*       p_attr_data;
} ancs_attr_list_t;


enum
{
    ANCS_NOTIF_SCR_HDL_START,         // ANCS Notification Source characteristic start handle.
    ANCS_NOTIF_SCR_HDL_END,           // ANCS Notification Source characteristic end handle.
    ANCS_NOTIF_SCR_HDL_CCCD,          // ANCS Notification Source CCCD handle.

    ANCS_CTRL_POINT_HDL_START,        // ANCS Control Point characteristic start handle.
    ANCS_CTRL_POINT_HDL_END,          // ANCS Control Point characteristic end handle.

    ANCS_DATA_SRC_HDL_START,          // ANCS Data Source characteristic start handle.
    ANCS_DATA_SRC_HDL_END,            // ANCS Data Source characteristic end handle.
    ANCS_DATA_SRC_HDL_CCCD,           // ANCS Data Source CCCD handle.
};

// Cache array length.
#define HDL_CACHE_LEN            8
enum
{
    ANCS_UNINIT,                      // uninitial idle state.
    ANCS_DISC_SERVICE,                // Discover the ANCS service by UUID.
    ANCS_STORE_SERVICE_HANDLES,       // Store the ANCS service handles in the cache.
    ANCS_DISC_CHARS,                  // Discover the three characteristics: Notification Source, Control Point, and Data Source.
    ANCS_STORE_CHARS_HANDLES,         // Store the handles of each characteristic in the handle cache.
    ANCS_DISC_NS_DESCS,               // Discover the descriptors of the Notification Source (Trying to locate the CCCD).
    ANCS_STORE_NS_DESCS_HANDLES,      // Store the descriptor's handles in the handle cache (ANCS_NOTIF_SCR_HDL_CCCD).
    ANCS_DISC_DS_DESCS,               // Discover the descriptors of the Data Source (Trying to locate the CCCD).
    ANCS_STORE_DS_DESCS_HANDLES,      // Store the descriptor's handles in the handle cache (ANCS_DATA_SRC_HDL_CCCD).
    ANCS_ENABLE_NS_CCCD,
    ANCS_ENABLE_DS_CCCD,
    ANCS_WAIT_CCCD_READY,
    ANCS_DISC_FINISH,                 // Final state signifying the end of the discovery process.

    ANCS_DISC_FAILED = 0xFF           // A failure state reached only if an error occurs.
};

enum
{
    ANCS_STATE_IDLE = 0,
    ANCS_STATE_DISCOVERY,
    ANCS_STATE_READY,
};

typedef struct
{
    uint16_t conn_hdl;
    uint8_t  expect_type_value_num;
    uint16_t service_hdl[2];
    uint8_t  chars_disc_num;
    uint16_t chars_hdl[HDL_CACHE_LEN];
} ancs_service_t;
typedef enum
{
    COMMAND_ID,    /**< Parsing the command ID. */
    NOTIF_UID,     /**< Parsing the notification UID. */
    APP_ID,        /**< Parsing app ID. */
    ATTR_ID,       /**< Parsing attribute ID. */
    ATTR_LEN1,     /**< Parsing the LSB of the attribute length. */
    ATTR_LEN2,     /**< Parsing the MSB of the attribute length. */
    ATTR_DATA,     /**< Parsing the attribute data. */
    ATTR_SKIP,     /**< Parsing is skipped for the rest (or entire) of an attribute. */
    DONE,          /**< Parsing for one attribute is done. */
} ancs_parse_state_t;

typedef enum
{
    ANCS_COMMAND_ID_GET_NOTIF_ATTRIBUTES,      /**< Requests attributes to be sent from the NP to the NC for a given notification. */
    ANCS_COMMAND_ID_GET_APP_ATTRIBUTES,        /**< Requests attributes to be sent from the NP to the NC for a given iOS app. */
    ANCS_COMMAND_ID_GET_PERFORM_NOTIF_ACTION,  /**< Requests an action to be performed on a given notification, for example, dismiss an alarm. */
} ancs_cmd_id_val_t;


typedef struct
{
    ancs_attr_list_t*       p_attr_list;              //!< The current list of attributes being parsed. This field will point to either @ref ble_ancs_c_t::ancs_notif_attr_list or @ref  ble_ancs_c_t::ancs_app_attr_list.
    uint32_t                nb_of_attr;               //!< Number of possible attributes. When parsing begins, it is set to either @ref ANCS_NB_OF_NOTIF_ATTR or @ref ANCS_NB_OF_APP_ATTR.
    uint32_t                expected_number_of_attrs; //!< The number of attributes expected upon receiving attributes. Keeps track of when to stop reading incoming attributes.
    ancs_parse_state_t      parse_state;              //!< ANCS notification attribute parsing state.
    ancs_cmd_id_val_t       command_id;               //!< Variable to keep track of what command type we are currently parsing ( @ref BLE_ANCS_COMMAND_ID_GET_NOTIF_ATTRIBUTES or @ref BLE_ANCS_COMMAND_ID_GET_APP_ATTRIBUTES.
    uint8_t*                  p_data_dest;              //!< Attribute that the parsed data will be copied into.
    uint16_t                current_attr_index;       //!< Variable to keep track of how much (for a given attribute) we are done parsing.
    uint32_t                current_app_id_index;     //!< Variable to keep track of how much (for a given app identifier) we are done parsing.
} ble_ancs_parse_fsm_t;

typedef struct
{
    uint32_t      notif_uid;
    uint8_t       app_id[ANCS_ATTR_DATA_MAX];
    uint16_t      attr_len;     //!< Length of the received attribute data.
    uint32_t      attr_id;      //!< Classification of the attribute type, for example, title or date.
    uint8_t*       p_attr_data;  //!< Pointer to where the memory is allocated for storing incoming attributes.
} ancs_attr_evt_t;

typedef struct
{
    uint8_t type;
    uint8_t len;
    void*   msg;
} ancs_evt_t;

typedef void (*ancs_evt_hdl_t) (ancs_evt_t* p_evt);

typedef struct
{
    uint8_t               app_task_ID;
    uint8_t               disc_state;
    uint8_t               app_state;
    ancs_attr_list_t      notif_attr_list[ANCS_NB_OF_NOTIF_ATTR];
    ancs_attr_list_t      app_attr_list[ANCS_NB_OF_APP_ATTR];
    ble_ancs_parse_fsm_t  parse_info;
    ancs_service_t        ancs_service;
    ancs_attr_evt_t       attr_evt_msg;
    ancs_evt_t            attr_rsp_evt;
    uint8_t               app_attr_tx_buf[ANCS_APP_ATTR_TX_SIZE];
    ancs_evt_hdl_t        callback;
} ancs_ctx_t;




typedef struct
{
    uint8_t eventID;
    // Store the ANCS notification's eventFlag
    uint8_t eventFlag;
    // Store the ANCS notification's categoryID
    uint8_t categoryID;
    uint8_t categoryCount;

    uint8_t notifUID[4];
} ancs_notify_evt_t;


bStatus_t ble_ancs_attr_add(const ancs_notif_attr_id id, uint8_t* p_data, const uint16_t len);
bStatus_t ble_ancs_get_notif_attrs(const uint8_t* pNotificationUID);
bStatus_t ble_ancs_get_app_attrs(const uint8_t* p_app_id, uint8_t app_id_len);
bStatus_t ble_ancs_start_descovery(uint16_t conn_handle);
bStatus_t ble_ancs_handle_gatt_event(gattMsgEvent_t* pMsg);
bStatus_t ble_ancs_init(ancs_evt_hdl_t evt_hdl, uint8_t task_ID);


#endif //__BLE_ANCS_HD_


