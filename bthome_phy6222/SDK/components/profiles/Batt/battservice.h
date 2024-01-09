/**************************************************************************************************
 SDK_LICENSE
**************************************************************************************************/


#ifndef BATTSERVICE_H
#define BATTSERVICE_H

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

// Battery Service Get/Set Parameters
#define BATT_PARAM_LEVEL                0
#define BATT_PARAM_CRITICAL_LEVEL       1
#define BATT_PARAM_SERVICE_HANDLE       2
#define BATT_PARAM_BATT_LEVEL_IN_REPORT 3

// Callback events
#define BATT_LEVEL_NOTI_ENABLED         1
#define BATT_LEVEL_NOTI_DISABLED        2

// HID Report IDs for the service
#define HID_RPT_ID_BATT_LEVEL_IN        4  // Battery Level input report ID

#ifdef HID_VOICE_SPEC
#define GATT_DESC_LENGTH_UUID            0x3111 // Used with Unit percent
#endif


/*********************************************************************
    TYPEDEFS
*/

// Battery Service callback function
typedef void (*battServiceCB_t)(uint8 event);

// Battery measure HW setup function
typedef void (*battServiceSetupCB_t)(void);

// Battery measure percentage calculation function
typedef uint8 (*battServiceCalcCB_t)(uint16 adcVal);

// Battery measure HW teardown function
typedef void (*battServiceTeardownCB_t)(void);

/*********************************************************************
    MACROS
*/

/*********************************************************************
    Profile Callbacks
*/


/*********************************************************************
    API FUNCTIONS
*/

/*********************************************************************
    @fn      Batt_AddService

    @brief   Initializes the Battery service by registering
            GATT attributes with the GATT server.

    @return  Success or Failure
*/
extern bStatus_t Batt_AddService( void );

/*********************************************************************
    @fn      Batt_Register

    @brief   Register a callback function with the Battery Service.

    @param   pfnServiceCB - Callback function.

    @return  None.
*/
extern void Batt_Register( battServiceCB_t pfnServiceCB );

/*********************************************************************
    @fn      Batt_SetParameter

    @brief   Set a Battery Service parameter.

    @param   param - Profile parameter ID
    @param   len - length of data to right
    @param   value - pointer to data to write.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  bStatus_t
*/
extern bStatus_t Batt_SetParameter( uint8 param, uint8 len, void* value );

/*********************************************************************
    @fn      Batt_GetParameter

    @brief   Get a Battery parameter.

    @param   param - Profile parameter ID
    @param   value - pointer to data to get.  This is dependent on
            the parameter ID and WILL be cast to the appropriate
            data type (example: data type of uint16 will be cast to
            uint16 pointer).

    @return  bStatus_t
*/
extern bStatus_t Batt_GetParameter( uint8 param, void* value );

/*********************************************************************
    @fn          Batt_MeasLevel

    @brief       Measure the battery level and update the battery
                level value in the service characteristics.  If
                the battery level-state characteristic is configured
                for notification and the battery level has changed
                since the last measurement, then a notification
                will be sent.

    @return      Success or Failure
*/
extern bStatus_t Batt_MeasLevel( void );

/*********************************************************************
    @fn      Batt_Setup

    @brief   Set up which ADC source is to be used. Defaults to VDD/3.

    @param   adc_ch - ADC Channel, e.g. HAL_ADC_CHN_AIN6
    @param   minVal - max battery level
    @param   maxVal - min battery level
    @param   sCB - HW setup callback
    @param   tCB - HW tear down callback
    @param   cCB - percentage calculation callback

    @return  none.
*/
extern void Batt_Setup( uint8 adc_ch, uint16 minVal, uint16 maxVal,
                        battServiceSetupCB_t sCB, battServiceTeardownCB_t tCB,
                        battServiceCalcCB_t cCB );

/*********************************************************************
    @fn          Batt_HandleConnStatusCB

    @brief       Battery Service link status change handler function.

    @param       connHandle - connection handle
    @param       changeType - type of change

    @return      none
*/
void Batt_HandleConnStatusCB( uint16 connHandle, uint8 changeType );

/*********************************************************************
    @fn      battNotifyLevelState

    @brief   Send a notification of the battery level state
            characteristic if a connection is established.

    @return  None.
*/
void BattNotifyLevel( void );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* BATTSERVICE_H */
