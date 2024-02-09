/**************************************************************************************************
	Filename:		sbpProfile.h
	Revised:
	Revision:
	Description:	This file contains the Simple GATT profile definitions and
				  prototypes.
 **************************************************************************************************/

#ifndef _SBPPROFILE_H_
#define _SBPPROFILE_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */

// Profile Parameters
#define SIMPLEPROFILE_CHAR1						0  // RW uint8 - Profile Characteristic 1 value 
#define SIMPLEPROFILE_CHAR2						1  // RW uint8 - Profile Characteristic 2 value

// Simple Profile Service UUID
#define SIMPLEPROFILE_SERV_UUID					0xFFF0
	
// OTA UUID
#define SIMPLEPROFILE_CHAR1_UUID				0xFFF3
// CMD UUID
#define SIMPLEPROFILE_CHAR2_UUID				0xFFF4
  
// Simple X Profile Services bit fields
#define SIMPLEPROFILE_SERVICE					0x00000001

/*********************************************************************
 * TYPEDEFS
 */
  
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * Profile Callbacks
 */

// Callback when a characteristic value has changed
typedef void (*simpleProfileChange_t)( uint8_t paramID );

typedef struct
{
	simpleProfileChange_t		 pfnSimpleProfileChange;  // Called when characteristic value changes
} simpleProfileCBs_t;

	

/*********************************************************************
 * API FUNCTIONS 
 */


/*
 * SimpleProfile_AddService- Initializes the Simple GATT Profile service by registering
 *			GATT attributes with the GATT server.
 *
 * @param	services - services to add. This is a bit map and can
 *					   contain more than one service.
 */

extern bStatus_t SimpleProfile_AddService( uint32_t services );

//extern bStatus_t simpleProfile_Notify( uint8_t param, uint8_t len, void *value );

void new_cmd_data(void);
void new_ota_data(void);
void wrk_notify(void);
void measure_notify(void);
uint16_t make_measure_msg(uint8_t *pbuf);

#ifdef __cplusplus
}
#endif

#endif /* _SBPPROFILE_H_ */
