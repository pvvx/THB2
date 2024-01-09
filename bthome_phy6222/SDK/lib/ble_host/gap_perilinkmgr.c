/*************************************************************************************************
    Filename:       gap_perilinkmgr.c
    Revised:
    Revision:

    Description:    This file contains the GAP Peripheral Link Manager.

	SDK_LICENSE

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
