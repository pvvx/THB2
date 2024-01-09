/**************************************************************************************************
 SDK_LICENSE
**************************************************************************************************/


#ifndef __ANCS_ATTR_H
#define __ANCS_ATTR_H

#include "ble_ancs.h"

#define GATTC_OPCODE_SIZE                1      /**< Size of the GATTC OPCODE. */
#define GATTC_ATTR_HANDLE_SIZE           4      /**< Size of the Attribute handle Size. */


#define ANCS_GATTC_WRITE_PAYLOAD_LEN_MAX (23 - GATTC_OPCODE_SIZE - GATTC_ATTR_HANDLE_SIZE)  /**< Maximum Length of the data we can send in one write. */

typedef enum
{
    APP_ATTR_COMMAND_ID, /**< Currently encoding the Command ID. */
    APP_ATTR_APP_ID,     /**< Currently encoding the App ID. */
    APP_ATTR_ATTR_ID,    /**< Currently encoding the Attribute ID. */
    APP_ATTR_DONE        /**< Encoding done. */
} encode_app_attr_t;

extern void ancs_parse_get_attrs_response(ancs_ctx_t*   p_ancs, const uint8_t* p_data_src, uint8_t  hvx_data_len);
extern bStatus_t app_attrs_get(ancs_ctx_t*   p_ancs, const uint8_t* p_app_id, uint8_t app_id_len);
extern bStatus_t notif_attrs_get(ancs_ctx_t*   p_ancs,const uint8_t* pNotificationUID);

#endif //__ANCS_ATTR_H

