
/**************************************************************************************************
    Filename:       ppsp_serv.h
    Revised:
    Revision:

    Description:    This file contains the Simple GATT profile definitions and
                  prototypes.

 **************************************************************************************************/

#ifndef PPSP_SERV_H
#define PPSP_SERV_H

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
#define PPSP_SERV_CFGS_CHAR_FFD4_INDX       0   // RW uint8 - Profile Characteristic 1 value
#define PPSP_SERV_CFGS_CHAR_FFD5_INDX       1   // RW uint8 - Profile Characteristic 2 value
#define PPSP_SERV_CFGS_CHAR_FFD6_INDX       2   // RW uint8 - Profile Characteristic 3 value
#define PPSP_SERV_CFGS_CHAR_FFD7_INDX       3   // RW uint8 - Profile Characteristic 4 value
#define PPSP_SERV_CFGS_CHAR_FFD8_INDX       4   // RW uint8 - Profile Characteristic 5 value

// Simple Profile Service UUID
#define PPSP_SERV_CFGS_SERV_FEB3_UUID       0xFEB3

// Key Pressed UUID
#define PPSP_SERV_CFGS_CHAR_FED4_UUID       0xFED4
#define PPSP_SERV_CFGS_CHAR_FED5_UUID       0xFED5
#define PPSP_SERV_CFGS_CHAR_FED6_UUID       0xFED6
#define PPSP_SERV_CFGS_CHAR_FED7_UUID       0xFED7
#define PPSP_SERV_CFGS_CHAR_FED8_UUID       0xFED8

// Simple Keys Profile Services bit fields
#define PPSP_SERV_CFGS_SERV_FEB3_MASK       0x00000001

// Data Length of Characteristic 5 in bytes
#define PPSP_SERV_CFGS_CHAR_FED4_DLEN       255//256
#define PPSP_SERV_CFGS_CHAR_FED5_DLEN       255//256
#define PPSP_SERV_CFGS_CHAR_FED6_DLEN       255//256
#define PPSP_SERV_CFGS_CHAR_FED7_DLEN       255//256
#define PPSP_SERV_CFGS_CHAR_FED8_DLEN       255//256

/*********************************************************************
    TYPEDEFS
*/
#include "bcomdef.h"

/*********************************************************************
    MACROS
*/

/*********************************************************************
    Profile Callbacks
*/

// Callback when a characteristic value has changed
typedef void (*ppsp_serv_hdlr_char_upda_t)( uint8 para, uint16 coun );

typedef struct
{
    ppsp_serv_hdlr_char_upda_t char_upda;  // Called when characteristic value changes
} ppsp_serv_appl_CBs_t;



/*********************************************************************
    API FUNCTIONS
*/


/*
    SimpleProfile_AddService- Initializes the Simple GATT Profile service by registering
            GATT attributes with the GATT server.

    @param   services - services to add. This is a bit map and can
                       contain more than one service.
*/

extern bStatus_t ppsp_serv_add_serv(uint32 serv);

/*
    SimpleProfile_RegisterAppCBs - Registers the application callback function.
                      Only call this function once.

      appCallbacks - pointer to application callbacks.
*/
extern bStatus_t ppsp_serv_reg_appl( ppsp_serv_appl_CBs_t* appl_hdlr );

/*
    SimpleProfile_SetParameter - Set a Simple GATT Profile parameter.

      param - Profile parameter ID
      len - length of data to right
      value - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).
*/
extern bStatus_t ppsp_serv_set_para( uint8 para, uint16 leng, void* valu );

/*
    SimpleProfile_GetParameter - Get a Simple GATT Profile parameter.

      param - Profile parameter ID
      value - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).
*/
extern bStatus_t ppsp_serv_get_para( uint8 para, void* valu, uint16 leng );

// extern bStatus_t simpleProfile_Notify( uint8 param, uint8 len, void *value );
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEGATTPROFILE_H */
