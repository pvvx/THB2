/**************************************************************************************************
    Filename:       multiRoleProfile.h
    Revised:
    Revision:

    Description:    This file contains the Simple GATT profile definitions and
                  prototypes.

 SDK_LICENSE

 **************************************************************************************************/

#ifndef MULTIROLE_PROFILE_H
#define MULTIROLE_PROFILE_H

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

/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    MACROS
*/
#define MULTIPROFILE_CHAR1             0x02
#define MULTIPROFILE_CHAR2             0x03

/*********************************************************************
    Profile Callbacks
*/

// Callback when a characteristic value has changed
typedef void (*multiProfileChange_t)( uint16 connHandle,uint16 paramID, uint16 len);

typedef struct
{
    multiProfileChange_t        pfnMultiProfileChange;  // Called when characteristic value changes
} multiProfileCBs_t;



/*********************************************************************
    API FUNCTIONS
*/


/*
    SimpleProfile_AddService- Initializes the Simple GATT Profile service by registering
            GATT attributes with the GATT server.

    @param   services - services to add. This is a bit map and can
                       contain more than one service.
*/

extern bStatus_t MultiProfile_AddService( uint32 services );

/*
    SimpleProfile_RegisterAppCBs - Registers the application callback function.
                      Only call this function once.

      appCallbacks - pointer to application callbacks.
*/
extern bStatus_t MultiProfile_RegisterAppCBs( multiProfileCBs_t* appCallbacks );

/*
    SimpleProfile_SetParameter - Set a Simple GATT Profile parameter.

      param - Profile parameter ID
      len - length of data to right
      value - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).
*/
extern bStatus_t MultiProfile_SetParameter( uint8 param, uint8 len, void* value );

/*
    SimpleProfile_GetParameter - Get a Simple GATT Profile parameter.

      param - Profile parameter ID
      value - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).
*/
extern bStatus_t MultiProfile_GetParameter(uint16 connHandle, uint8 param, void* value );

extern bStatus_t MultiProfile_Notify(uint16 connHandle, uint8 param, uint16 len, void* value );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEGATTPROFILE_H */
