/**************************************************************************************************
    Filename:       simplekeys.h
    Revised:
    Revision:

    Description:    This file contains the Simple Keys Profile header file.

 SDK_LICENSE

**************************************************************************************************/

#ifndef SIMPLEKEYS_H
#define SIMPLEKEYS_H

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
#define SK_KEY_ATTR                   0  // RW uint8 - Profile Attribute value

// SK Service UUID
#define SK_SERV_UUID                  0xFFE0

// Key Pressed UUID
#define SK_KEYPRESSED_UUID            0xFFE1

// Key Values
#define SK_KEY_LEFT                   0x01
#define SK_KEY_RIGHT                  0x02

// Simple Keys Profile Services bit fields
#define SK_SERVICE                    0x00000001

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

/*
    SK_AddService- Initializes the Simple Key service by registering
            GATT attributes with the GATT server.

    @param   services - services to add. This is a bit map and can
                       contain more than one service.
*/

extern bStatus_t SK_AddService( uint32 services );

/*
    SK_SetParameter - Set a Simple Key Profile parameter.

      param - Profile parameter ID
      len - length of data to right
      pValue - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).
*/
extern bStatus_t SK_SetParameter( uint8 param, uint8 len, void* pValue );

/*
    SK_GetParameter - Get a Simple Key Profile parameter.

      param - Profile parameter ID
      pValue - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).
*/
extern bStatus_t SK_GetParameter( uint8 param, void* pValue );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEKEYS_H */
