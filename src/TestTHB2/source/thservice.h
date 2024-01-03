
#ifndef _THSERVICE_H_
#define _THSERVICE_H_

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
#define ENV_SENSING_SERV_UUID         0x181A 
#define TEMPERATYRE_UUID              0x2A6E  
#define HUMIDITY_UUID                 0x2A6F 


// Battery Service Get/Set Parameters
#define BATT_PARAM_LEVEL                0
#define BATT_PARAM_CRITICAL_LEVEL       1
#define BATT_PARAM_SERVICE_HANDLE       2
#define BATT_PARAM_BATT_LEVEL_IN_REPORT 3

// Callback events
#define TEMP_LEVEL_NOTI_ENABLED         1
#define TEMP_LEVEL_NOTI_DISABLED        2


/*********************************************************************
    TYPEDEFS
*/

// TH Service callback function
typedef void (*thServiceCB_t)(uint8 event);

// TH measure HW setup function
typedef void (*thServiceSetupCB_t)(void);

// TH measure percentage calculation function
typedef uint8 (*thServiceCalcCB_t)(uint16 adcVal);

// TH measure HW teardown function
typedef void (*thServiceTeardownCB_t)(void);

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
extern bStatus_t TH_AddService( void );

/*********************************************************************
    @fn      Batt_Register

    @brief   Register a callback function with the Battery Service.

    @param   pfnServiceCB - Callback function.

    @return  None.
*/
extern void TH_Register( thServiceCB_t pfnServiceCB );

/*********************************************************************
    @fn          Batt_HandleConnStatusCB

    @brief       Battery Service link status change handler function.

    @param       connHandle - connection handle
    @param       changeType - type of change

    @return      none
*/
void TH_HandleConnStatusCB( uint16 connHandle, uint8 changeType );

/*********************************************************************
    @fn      thNotifyLevelState

    @brief   Send a notification of the battery level state
            characteristic if a connection is established.

    @return  None.
*/
void TH_NotifyLevel(void);
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* _THSERVICE_H_ */
