/**************************************************************************************************
    Filename:       devinfoservice.h
    Revised:        $Date $
    Revision:       $Revision $

    Description:    This file contains the Device Information service definitions and
                  prototypes.

 SDK_LICENSE


**************************************************************************************************/

#ifndef DEVINFOSERVICE_H
#define DEVINFOSERVICE_H

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

// Device Information Service Parameters
#define DEVINFO_SYSTEM_ID                 0
#define DEVINFO_MODEL_NUMBER              1
#define DEVINFO_SERIAL_NUMBER             2
#define DEVINFO_FIRMWARE_REV              3
#define DEVINFO_HARDWARE_REV              4
#define DEVINFO_SOFTWARE_REV              5
#define DEVINFO_MANUFACTURER_NAME         6
#define DEVINFO_11073_CERT_DATA           7
#define DEVINFO_PNP_ID                    8

// IEEE 11073 authoritative body values
#define DEVINFO_11073_BODY_EMPTY          0
#define DEVINFO_11073_BODY_IEEE           1
#define DEVINFO_11073_BODY_CONTINUA       2
#define DEVINFO_11073_BODY_EXP            254

// System ID length
#define DEVINFO_SYSTEM_ID_LEN             8

// PnP ID length
#define DEVINFO_PNP_ID_LEN                7

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
    DevInfo_AddService- Initializes the Device Information service by registering
            GATT attributes with the GATT server.

*/

extern bStatus_t DevInfo_AddService( void );

/*********************************************************************
    @fn      DevInfo_SetParameter

    @brief   Set a Device Information parameter.

    @param   param - Profile parameter ID
    @param   len - length of data to right
    @param   value - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  bStatus_t
*/
bStatus_t DevInfo_SetParameter( uint8 param, uint8 len, void* value );

/*
    DevInfo_GetParameter - Get a Device Information parameter.

      param - Profile parameter ID
      value - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).
*/
extern bStatus_t DevInfo_GetParameter( uint8 param, void* value );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* DEVINFOSERVICE_H */
