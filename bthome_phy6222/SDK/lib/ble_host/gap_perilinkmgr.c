/**************************************************************************************************

    Phyplus Microelectronics Limited confidential and proprietary.
    All rights reserved.

    IMPORTANT: All rights of this software belong to Phyplus Microelectronics
    Limited ("Phyplus"). Your use of this Software is limited to those
    specific rights granted under  the terms of the business contract, the
    confidential agreement, the non-disclosure agreement and any other forms
    of agreements as a customer or a partner of Phyplus. You may not use this
    Software unless you agree to abide by the terms of these agreements.
    You acknowledge that the Software may not be modified, copied,
    distributed or disclosed unless embedded on a Phyplus Bluetooth Low Energy
    (BLE) integrated circuit, either as a product or is integrated into your
    products.  Other than for the aforementioned purposes, you may not use,
    reproduce, copy, prepare derivative works of, modify, distribute, perform,
    display or sell this Software and/or its documentation for any purposes.

    YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
    PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
    INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
    NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
    PHYPLUS OR ITS SUBSIDIARIES BE LIABLE OR OBLIGATED UNDER CONTRACT,
    NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
    LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
    INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
    OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
    OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
    (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

**************************************************************************************************/

/*************************************************************************************************
    Filename:       gap_perilinkmgr.c
    Revised:
    Revision:

    Description:    This file contains the GAP Peripheral Link Manager.


**************************************************************************************************/



/*******************************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "gap.h"
#include "gap_internal.h"
#include "linkdb.h"
#include "sm.h"
#include "sm_internal.h"
#include "smp.h"

/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/

/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    GLOBAL VARIABLES
*/

/*********************************************************************
    EXTERNAL VARIABLES
*/

/*********************************************************************
    EXTERNAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/

/*********************************************************************
    LOCAL FUNCTIONS
*/

/*********************************************************************
    API FUNCTIONS
*/


/*********************************************************************
    Generate a Slave Requested Security message to the master.

    Public function defined in gap.h.
*/
bStatus_t GAP_SendSlaveSecurityRequest( uint16 connectionHandle, uint8 authReq )
{
    linkDBItem_t* pItem;     // Link parameters pointer

    // Check for wrong role
    if ( (gapProfileRole & GAP_PROFILE_PERIPHERAL) == 0 )
    {
        return ( bleIncorrectMode );
    }

    // Check for connection
    pItem = linkDB_Find( connectionHandle );

    if ( pItem )
    {
        smpSecurityReq_t sReq;
        uint8 tmp;

        // Are we already setup in an pAuthLink[connectionHandle]?
        if ( pAuthLink[connectionHandle] )
        {
            // Use the pAuthLink[connectionHandle]'s requirements
            tmp = pAuthLink[connectionHandle]->secReqs.authReq;
        }
        else
        {
            // Use the passed in requirements
            tmp = authReq;
        }

        // Convert byte to struct
        smUint8ToAuthReq( &(sReq.authReq), tmp );
        // Tell SM to send the Slave Security Request
        return ( smSendSecurityReq( connectionHandle, &sReq ) );
    }
    else
    {
        return ( bleNotConnected );
    }
}



/****************************************************************************
****************************************************************************/
