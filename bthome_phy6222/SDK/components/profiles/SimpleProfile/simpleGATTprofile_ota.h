/**************************************************************************************************
    Filename:       simpleGATTprofile.h
    Revised:
    Revision:

    Description:    This file contains the Simple GATT profile definitions and
                  prototypes.

 SDK_LICENSE

 **************************************************************************************************/

#ifndef SIMPLEGATTPROFILE_OTA_H
#define SIMPLEGATTPROFILE_OTA_H

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

// Profile Parameters
#define SIMPLEPROFILE_CHAR1                   0  // RW uint8 - Profile Characteristic 1 value 
#define SIMPLEPROFILE_CHAR2                   1  // RW uint8 - Profile Characteristic 2 value
#define SIMPLEPROFILE_CHAR3                   2  // RW uint8 - Profile Characteristic 3 value
#define SIMPLEPROFILE_CHAR4                   3  // RW uint8 - Profile Characteristic 4 value
#define SIMPLEPROFILE_CHAR5                   4  // RW uint8 - Profile Characteristic 4 value
#define SIMPLEPROFILE_CHAR6                   5  // RW uint8 - Profile Characteristic 4 value

// Simple Profile Service UUID
#define SIMPLEPROFILE_SERV_UUID               0xFFF0

// Key Pressed UUID
#define SIMPLEPROFILE_CHAR1_UUID            0xFFF1
#define SIMPLEPROFILE_CHAR2_UUID            0xFFF2
#define SIMPLEPROFILE_CHAR3_UUID            0xFFF3
#define SIMPLEPROFILE_CHAR4_UUID            0xFFF4
#define SIMPLEPROFILE_CHAR5_UUID            0xFFF5
#define SIMPLEPROFILE_CHAR6_UUID            0xFFF6

// Simple Keys Profile Services bit fields
#define SIMPLEPROFILE_SERVICE               0x00000001

// Length of Characteristic 5 in bytes
//#define SIMPLEPROFILE_CHAR5_LEN           5
#define IBEACON_UUID_LEN                  16

/*********************************************************************
    TYPEDEFS
*/


/*********************************************************************
    MACROS
*/

/*********************************************************************
    Profile Callbacks
*/

// Callback when a characteristic value has changed
typedef void (*simpleProfileChange_t)( uint8 paramID );

typedef struct
{
    simpleProfileChange_t        pfnSimpleProfileChange;  // Called when characteristic value changes
} simpleProfileCBs_t;



/*********************************************************************
    API FUNCTIONS
*/


/*
    SimpleProfile_AddService- Initializes the Simple GATT Profile service by registering
            GATT attributes with the GATT server.

    @param   services - services to add. This is a bit map and can
                       contain more than one service.
*/

extern bStatus_t SimpleProfile_AddService( uint32 services );

/*
    SimpleProfile_RegisterAppCBs - Registers the application callback function.
                      Only call this function once.

      appCallbacks - pointer to application callbacks.
*/
extern bStatus_t SimpleProfile_RegisterAppCBs( simpleProfileCBs_t* appCallbacks );

/*
    SimpleProfile_SetParameter - Set a Simple GATT Profile parameter.

      param - Profile parameter ID
      len - length of data to right
      value - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).
*/
extern bStatus_t SimpleProfile_SetParameter( uint8 param, uint8 len, void* value );

/*
    SimpleProfile_GetParameter - Get a Simple GATT Profile parameter.

      param - Profile parameter ID
      value - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).
*/
extern bStatus_t SimpleProfile_GetParameter( uint8 param, void* value );

extern bStatus_t simpleProfile_Notify( uint8 param, uint8 len, void* value );
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEGATTPROFILE_H */
