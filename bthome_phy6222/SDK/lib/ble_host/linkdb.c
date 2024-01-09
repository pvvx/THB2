/*************************************************************************************************
    Filename:       linkdb.c
    Revised:
    Revision:

    Description:    This file contains the Link Database.

    These functions are not intended to be used outside of the host base.

	SDK_LICENSE

**************************************************************************************************/


/*******************************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"

/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/

// Total number of tasks/apps that can be notified of a link database change.
#define LINKDB_MAX_CBS          MAX_NUM_LL_CONN + 4 // 4 for the Stack and 10 for the Application

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

//#define MAX_NUM_LL_CONN 1

// This is the link database, 1 record for each connection
static linkDBItem_t linkDB[MAX_NUM_LL_CONN];

// Table of callbacks to make when a connection changes state
static pfnLinkDBCB_t linkCBs[LINKDB_MAX_CBS];

/*********************************************************************
    LOCAL FUNCTIONS
*/

static void reportStatusChange( uint16 connectionHandle, uint8 changeType );

/*********************************************************************
    FUNCTIONS
*/

/*********************************************************************
    @fn          linkDB_Init

    @brief       Initialize the Link Database.

    @param       none

    @return      none
*/
void linkDB_Init( void )
{
    uint8 x; // loop counter

    // Initialize the table
    for ( x = 0; x < MAX_NUM_LL_CONN; x++ )
    {
        // Mark the record as unused.
        linkDB[x].connectionHandle = INVALID_CONNHANDLE;
        linkDB[x].stateFlags = LINK_NOT_CONNECTED;
        linkDB[x].pEncParams = NULL;
    }

    // Initialize the status callback registration table
    for ( x = 0; x < LINKDB_MAX_CBS; x++ )
    {
        // No callbacks
        linkCBs[x] = (pfnLinkDBCB_t)NULL;
    }
}

/*********************************************************************
    @fn          linkDB_Register

    @brief       Register with this function to receive a callback when
                status changes on a connection.  If the stateflag == 0,
                then the connection has been disconnected.

    @param       pFunc - function pointer to callback function

    @return      SUCCESS if successful
                bleMemAllocError if not table space available

*/
uint8 linkDB_Register( pfnLinkDBCB_t pFunc )
{
    // Find an empty slot
    for ( uint8 x = 0; x < LINKDB_MAX_CBS; x++ )
    {
        if ( linkCBs[x] == NULL )
        {
            linkCBs[x] = pFunc;
            return ( SUCCESS );
        }
    }

    return ( bleMemAllocError );
}

/*********************************************************************
    @fn          linkDB_Add

    @brief       Adds a record to the link database.

    @param       taskID - Application task ID
    @param       connectionHandle - new record connection handle
    @param       newState - starting connection state
    @param       addrType - new address type
    @param       pAddr - new address
    @param       connInterval - connection's communications interval (n * 1.23 ms)

    @return      SUCCESS if successful
                bleIncorrectMode - hasn't been initialized.
                bleNoResources - table full
                bleAlreadyInRequestedMode - already exist connectionHandle

*/
uint8 linkDB_Add( uint8 taskID, uint16 connectionHandle, uint8  stateFlags,uint8 role,
                  uint8 addrType, uint8* pAddr, uint16 connInterval )
{
    // Check for existing record
    linkDBItem_t* pItem = linkDB_Find( connectionHandle );

    if ( pItem )
    {
        // Item already exists - connectionHandle was found
        return ( bleAlreadyInRequestedMode );
    }
    else
    {
        pItem = linkDB_Find( INVALID_CONNHANDLE );

        if ( pItem )
        {
            // Copy link info
            pItem->addrType = addrType;
            VOID osal_memcpy( pItem->addr, pAddr, B_ADDR_LEN );
            pItem->connectionHandle = connectionHandle;
            pItem->stateFlags = stateFlags;
            pItem->role = role;
            pItem->taskID = taskID;
            pItem->pEncParams = NULL;
            pItem->connInterval = connInterval;
            reportStatusChange( connectionHandle, LINKDB_STATUS_UPDATE_NEW );
            return ( SUCCESS );
        }
        else
        {
            // Table is full
            return ( bleNoResources );
        }
    }
}

/*********************************************************************
    @fn          linkDB_Remove

    @brief       Removes a record from the link database.

    @param       connectionHandle - new record connection handle

    @return      SUCCESS if successful
                INVALIDPARAMETER - connectionHandle not found.

*/
uint8 linkDB_Remove( uint16 connectionHandle )
{
    // Get record
    linkDBItem_t* pItem = linkDB_Find( connectionHandle );

    if ( pItem )
    {
        reportStatusChange( pItem->connectionHandle, LINKDB_STATUS_UPDATE_REMOVED );

        // Free memory for LTK
        if ( pItem->pEncParams )
        {
            osal_mem_free( pItem->pEncParams );
        }

        // Clear the entire entry
        VOID osal_memset( pItem, 0, (int)(sizeof( linkDBItem_t )) );
        // Mark the record as unused.
        pItem->connectionHandle = INVALID_CONNHANDLE;
        pItem->stateFlags = LINK_NOT_CONNECTED;
        return ( SUCCESS );
    }
    else
    {
        // Record not found
        return ( INVALIDPARAMETER );
    }
}


/*********************************************************************
    @fn          linkDB_Update

    @brief       This function is used to update the stateFlags of
                a link record.

    @param       connectionHandle - maximum number of connections.
    @param       newState - new state flag.  This value is OR'd in
                           to this field.

    @return      SUCCESS if successful
                bleNoResources - connectionHandle not found.

*/
uint8 linkDB_Update( uint16 connectionHandle, uint8 newState )
{
    linkDBItem_t* pItem = linkDB_Find( connectionHandle );

    // Find the right connection
    if ( pItem )
    {
        pItem->stateFlags |= newState;
        reportStatusChange( pItem->connectionHandle,
                            LINKDB_STATUS_UPDATE_STATEFLAGS );
        return ( SUCCESS );
    }
    else
    {
        return ( bleNoResources );
    }
}

/*********************************************************************
    @fn          linkDB_Find

    @brief       Find the link.  Uses the connection handle to search
                the link database.

    @param       connectionHandle - controller link connection handle.

    @return      a pointer to the found link item, NULL if not found
*/
linkDBItem_t* linkDB_Find( uint16 connectionHandle )
{
    // Find link record
    for ( uint8 x = 0; x < MAX_NUM_LL_CONN; x++ )
    {
        if ( linkDB[x].connectionHandle == connectionHandle )
        {
            // Found
            return ( &linkDB[x] );
        }
    }

    // Not Found!!
    return ( (linkDBItem_t*)NULL );
}

/*********************************************************************
    @fn          linkDB_FindFirst

    @brief       Find the first link that matches the taskID.

    @param       taskID - taskID of app

    @return      a pointer to the found link item, NULL if not found
*/
linkDBItem_t* linkDB_FindFirst( uint8 taskID )
{
    // Find link record
    for ( uint8 x = 0; x < MAX_NUM_LL_CONN; x++ )
    {
        if ( (linkDB[x].connectionHandle != INVALID_CONNHANDLE)
                && (linkDB[x].taskID == taskID) )
        {
            // Found
            return ( &linkDB[x] );
        }
    }

    // Not Found!!
    return ( (linkDBItem_t*)NULL );
}

/*********************************************************************
    @fn          linkDB_State

    @brief       Check to see if a physical link is in a specific state.

    @param       connectionHandle - controller link connection handle.
    @param       state - state to look for.

    @return      TRUE if the link is found and state is set in
                state flags. FALSE, otherwise.
*/
uint8 linkDB_State( uint16 connectionHandle, uint8 state )
{
    linkDBItem_t* pLink;

    // Check to see if this is loopback connection
    if ( connectionHandle == LOOPBACK_CONNHANDLE )
    {
        return ( TRUE );
    }

    // Check to see if the physical link is up
    pLink = linkDB_Find( connectionHandle );

    if ( ( pLink != NULL ) && ( pLink->stateFlags & state ) )
    {
        return ( TRUE );
    }

    return ( FALSE );
}

/*********************************************************************
    @fn          linkDB_NumActive

    @brief       Counts the number of active connections (not .

    @param       connectionHandle - controller link connection handle.

    @return      number of active connections
*/
uint8 linkDB_NumActive( void )
{
    uint8 count = 0;

    // Find link record
    for ( uint8 x = 0; x < MAX_NUM_LL_CONN; x++ )
    {
        if ( linkDB[x].stateFlags )
        {
            count++;
        }
    }

    return ( count );
}

/*********************************************************************
    @fn          linkDB_Authen

    @brief       Check to see if the physical link is encrypted
                and authenticated.

    @param       connectionHandle - item's connection handle
    @param       keySize - minimum key size
    @param       mitmRequired - whether MITM protection is required

    @return      SUCCESS if link is authenticated.
                bleNotConnected - connection handle is invalid
                LINKDB_ERR_INSUFFICIENT_AUTHEN - link is not encrypted
                LINBDB_ERR_INSUFFICIENT_KEYSIZE - key size encrypted is not large enough
                LINKDB_ERR_INSUFFICIENT_ENCRYPTION - link is encrypted, but not authenticated
*/
uint8 linkDB_Authen( uint16 connectionHandle, uint8 keySize, uint8 mitmRequired )
{
    linkDBItem_t* pItem = linkDB_Find( connectionHandle );

    // Check that a connection exists
    if ( pItem == NULL )
    {
        // Check for loopback connection
        if ( connectionHandle == LOOPBACK_CONNHANDLE )
        {
            return ( SUCCESS );
        }
        else
        {
            return ( bleNotConnected );
        }
    }

    // If an LTK is not available, the service request shall be rejected with the
    // error code "Insufficient Authentication".

    // Note: When the link is not encrypted, the error code "Insufficient
    // Authentication" does not indicate that MITM protection is required.
    if ( (pItem->pEncParams == NULL) && ((pItem->stateFlags & LINK_ENCRYPTED) == 0) )
    {
        return ( LINKDB_ERR_INSUFFICIENT_AUTHEN );
    }

    // If an authenticated pairing is required but only an unauthenticated pairing
    // has occurred and the link is currently encrypted, the service request shall
    // be rejected with the error code "Insufficient Authentication."

    // Note: When unauthenticated pairing has occurred and the link is currently
    // encrypted, the error code "Insufficient Authentication" indicates that MITM
    // protection is required.
    if ( (pItem->pEncParams != NULL ) && (pItem->stateFlags & LINK_ENCRYPTED) &&
            ((pItem->stateFlags & LINK_AUTHENTICATED) == 0) && mitmRequired )
    {
        return ( LINKDB_ERR_INSUFFICIENT_AUTHEN );
    }

    // If an LTK is available and encryption is required (LE security mode 1) but
    // encryption is not enabled, the service request shall be rejected with the
    // error code “Insufficient Encryption?.
    if ( (pItem->stateFlags & LINK_ENCRYPTED) == 0 )
    {
        return ( LINKDB_ERR_INSUFFICIENT_ENCRYPTION );
    }

    // If the encryption is enabled with insufficient key size then the service
    // request shall be rejected with the error code “Insufficient Encryption Key
    // Size.?
    if ( pItem->pEncParams->keySize < keySize )
    {
        return ( LINBDB_ERR_INSUFFICIENT_KEYSIZE );
    }

    return ( SUCCESS );
}

/*********************************************************************
    @fn          linkDB_PerformFunc

    @brief       This function will call "cb" for every entry in the link database.

    @param       cb - function pointer to the function to call

    @return      none
*/
void linkDB_PerformFunc( pfnPerformFuncCB_t cb )
{
    if ( cb )
    {
        for ( uint8 x = 0; x < MAX_NUM_LL_CONN; x++ )
        {
            if ( linkDB[x].connectionHandle != INVALID_CONNHANDLE )
            {
                cb( &linkDB[x] );
            }
        }
    }
}

/*********************************************************************
    @fn          reportStatusChange

    @brief       Call all callback functions with the state update.

    @param       connectionHandle - item's connection handle
    @param       changeType - Change Type:
                  LINKDB_STATUS_UPDATE_NEW - added to the database
                  LINKDB_STATUS_UPDATE_REMOVED - deleted from the database
                  LINKDB_STATUS_UPDATE_STATEFLAGS - stateFlag item has changed

    @return      void
*/
static void reportStatusChange( uint16 connectionHandle, uint8 changeType )
{
    for ( uint8 x = 0; x < LINKDB_MAX_CBS; x++ )
    {
        if ( linkCBs[x] )
        {
            linkCBs[x]( connectionHandle, changeType );
        }
    }
}


/****************************************************************************
****************************************************************************/

