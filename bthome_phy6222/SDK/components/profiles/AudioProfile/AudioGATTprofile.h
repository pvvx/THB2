/**************************************************************************************************
    Filename:       AudioGATTprofile.h
    Revised:
    Revision:

    Description:    This file contains the Simple GATT profile definitions and
                  prototypes.

 SDK_LICENSE

 **************************************************************************************************/

#ifndef AUDIOGATTPROFILE_H
#define AUDIOGATTPROFILE_H

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
#define AUDIOPROFILE_CHAR1                   0  // RW uint8 - Profile Characteristic 1 value 
#define AUDIOPROFILE_CHAR2                   1  // RW uint8 - Profile Characteristic 2 value



//product id
#define PRODUCT_ID                            0x0046

// AUDIO Profile Service UUID
#define AUDIOPROFILE_SERV_UUID               0xB000


// Key Pressed UUID
#define AUDIOPROFILE_CHAR1_UUID              0xB001
#define AUDIOPROFILE_CHAR2_UUID              0xB002


// AUDIO Keys Profile Services bit fields
#define AUDIOPROFILE_SERVICE               0x00000001

#define  AUDIOPROFILE_CHAR1_LEN            20
#define  AUDIOPROFILE_CHAR2_LEN             20
// Length of Characteristic 5 in bytes
#define AUDIOPROFILE_CHAR5_LEN           5


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
typedef void (*AudioProfileChange_t)( uint8 paramID );

typedef struct
{
    AudioProfileChange_t        pfnAudioProfileChange;  // Called when characteristic value changes
} AudioProfileCBs_t;



/*********************************************************************
    API FUNCTIONS
*/


/*
    AudioProfile_AddService- Initializes the Audio GATT Profile service by registering
            GATT attributes with the GATT server.

    @param   services - services to add. This is a bit map and can
                       contain more than one service.
*/

extern bStatus_t AudioProfile_AddService( uint32 services );

/*
    AudioProfile_RegisterAppCBs - Registers the application callback function.
                      Only call this function once.

      appCallbacks - pointer to application callbacks.
*/
extern bStatus_t AudioProfile_RegisterAppCBs( AudioProfileCBs_t* appCallbacks );

/*
    AudioProfile_SetParameter - Set a Audio GATT Profile parameter.

      param - Profile parameter ID
      len - length of data to right
      value - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).
*/
extern bStatus_t AudioProfile_SetParameter( uint8 param, uint8 len, void* value );

/*
    AudioProfile_GetParameter - Get a Audio GATT Profile parameter.

      param - Profile parameter ID
      value - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).
*/
extern bStatus_t AudioProfile_GetParameter( uint8 param, void* value );

extern bStatus_t AudioProfile_Notify( uint8 param, uint8 len, void* value );

extern bStatus_t AudioProfile_Read( uint16 connHandle, gattAttribute_t* pAttr,
                                    uint8* pValue, uint16* pLen, uint16 offset, uint8 maxLen );

extern bStatus_t AudioProfile_Write( uint16 connHandle, gattAttribute_t* pAttr,
                                     uint8* pValue, uint8 len, uint16 offset );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEGATTPROFILE_H */
