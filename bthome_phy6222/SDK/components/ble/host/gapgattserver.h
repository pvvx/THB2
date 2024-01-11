/**************************************************************************************************
    Filename:       gapgattserver.h
    Revised:
    Revision:

    Description:    This file contains GAP GATT attribute definitions
                  and prototypes.

 SDK_LICENSE
**************************************************************************************************/

#ifndef GAPGATTSERVER_H
#define GAPGATTSERVER_H

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

#define GAP_DEVICE_NAME_LEN                     (20+1)

// Privacy Flag States
#define GAP_PRIVACY_DISABLED                    0x00
#define GAP_PRIVACY_ENABLED                     0x01

// GAP GATT Server Parameters
#define GGS_DEVICE_NAME_ATT                     0   // RW  uint8[GAP_DEVICE_NAME_LEN]
#define GGS_APPEARANCE_ATT                      1   // RW  uint16
#define GGS_PERI_PRIVACY_FLAG_ATT               2   // RW  uint8
#define GGS_RECONNCT_ADDR_ATT                   3   // RW  uint8[B_ADDR_LEN]
#define GGS_PERI_CONN_PARAM_ATT                 4   // RW  sizeof(gapPeriConnectParams_t)
#define GGS_PERI_PRIVACY_FLAG_PROPS             5   // RW  uint8
#define GGS_W_PERMIT_DEVICE_NAME_ATT            6   // W   uint8
#define GGS_W_PERMIT_APPEARANCE_ATT             7   // W   uint8
#define GGS_W_PERMIT_PRIVACY_FLAG_ATT           8   // W   uint8

// GAP Services bit fields
#define GAP_SERVICE                             0x00000001

// Attribute ID used with application's callback when attribute value is changed OTA
#define GGS_DEVICE_NAME_ID                      0
#define GGS_APPEARANCE_ID                       1

#if defined ( TESTMODES )
// GGS TestModes
#define GGS_TESTMODE_OFF                      0 // No Test mode
#define GGS_TESTMODE_W_PERMIT_DEVICE_NAME     1 // Make Device Name attribute writable
#define GGS_TESTMODE_W_PERMIT_APPEARANCE      2 // Make Appearance attribute writable
#define GGS_TESTMODE_W_PERMIT_PRIVACY_FLAG    3 // Make Peripheral Privacy Flag attribute writable with authentication
#endif  // TESTMODES

/*********************************************************************
    TYPEDEFS
*/
// Callback to notify when attribute value is changed over the air.
typedef void (*ggsAttrValueChange_t)( uint8 attrId );

// GAP GATT Server callback structure
typedef struct
{
    ggsAttrValueChange_t  pfnAttrValueChange;  // When attribute value is changed OTA
} ggsAppCBs_t;

/*********************************************************************
    MACROS
*/

/*********************************************************************
    Profile Callbacks
*/

/*********************************************************************
    API FUNCTIONS
*/

/**
    @brief   Set a GAP GATT Server parameter.

    @param   param - Profile parameter ID<BR>
    @param   len - length of data to right
    @param   value - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).<BR>

    @return  bStatus_t
*/
extern bStatus_t GGS_SetParameter( uint8 param, uint8 len, void* value );

/**
    @brief   Get a GAP GATT Server parameter.

    @param   param - Profile parameter ID<BR>
    @param   value - pointer to data to put.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).<BR>

    @return  bStatus_t
*/
extern bStatus_t GGS_GetParameter( uint8 param, void* value );

/**
    @brief   Add function for the GAP GATT Service.

    @param   services - services to add. This is a bit map and can
                       contain more than one service.

    @return  SUCCESS: Service added successfully.<BR>
            INVALIDPARAMETER: Invalid service field.<BR>
            FAILURE: Not enough attribute handles available.<BR>
            bleMemAllocError: Memory allocation error occurred.<BR>
*/
extern bStatus_t GGS_AddService( uint32 services );

/**
    @brief   Delete function for the GAP GATT Service.

    @param   services - services to delete. This is a bit map and can
                       contain more than one service.

    @return  SUCCESS: Service deleted successfully.<BR>
            FAILURE: Service not found.<BR>
*/
extern bStatus_t GGS_DelService( uint32 services );

/**
    @brief   Registers the application callback function.

            Note: Callback registration is needed only when the
                  Device Name is made writable. The application
                  will be notified when the Device Name is changed
                  over the air.

    @param   appCallbacks - pointer to application callbacks.

    @return  none
*/
extern void GGS_RegisterAppCBs( ggsAppCBs_t* appCallbacks );

/**
    @brief   Set a GGS Parameter value. Use this function to change
            the default GGS parameter values.

    @param   value - new GGS param value

    @return  void
*/
extern void GGS_SetParamValue( uint16 value );

/**
    @brief   Get a GGS Parameter value.

    @param   none

    @return  GGS Parameter value
*/
extern uint16 GGS_GetParamValue( void );

/*********************************************************************
    TASK FUNCTIONS - Don't call these. These are system functions.
*/

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* GAPGATTSERVER_H */
