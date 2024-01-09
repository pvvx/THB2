/*************************************************************************************************
    Filename:       gap_devmgr.c
    Revised:
    Revision:

    Description:    This file contains the GAP Device Manager.

	SDK_LICENSE

**************************************************************************************************/
#include "bcomdef.h"
#include "gap.h"
#include "gap_internal.h"



/*******************************************************************************
    INCLUDES
*/


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

// Device Discovery Parameters for Central
gapDevDiscReq_t* pGapDiscReq = NULL;

// Discoverable State Parameters for Peripheral
gapAdvertState_t* pGapAdvertState = NULL;

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
    PUBLIC FUNCTIONS
*/

/*********************************************************************
    Resolves a private address against an IRK.

    Public function defined in gap.h.
*/
bStatus_t GAP_ResolvePrivateAddr( uint8* pIRK, uint8* pAddr )
{
    // SM will resolve the address
    return ( SM_ResolveRandomAddrs( pIRK, pAddr ) );
}

/*********************************************************************
    PUBLIC INTERNAL GAP FUNCTIONS
*/

/*********************************************************************
    @fn          gapIsAdvertising

    @brief       Check if we are currently advertising.

    @param       none

    @return      TRUE if currently advertising, FALSE if not
*/
uint8 gapIsAdvertising( void )
{
    if ( pGapAdvertState )
    {
        return ( TRUE );
    }
    else
    {
        return ( FALSE );
    }
}

/*********************************************************************
    @fn          gapIsScanning

    @brief       Check if we are currently scanning.

    @param       none

    @return      TRUE if currently scanning, FALSE if not
*/
uint8 gapIsScanning( void )
{
    if ( pGapDiscReq )
    {
        return ( TRUE );
    }
    else
    {
        return ( FALSE );
    }
}

/*********************************************************************
    @fn          gapValidADType

    @brief       Is a Advertisement Data Type valid.

    @param       adType - Advertisement data type

    @return      TRUE is valid, FALSE is not valid
*/
uint8 gapValidADType( uint8 adType )
{
    if ( ((adType > 0) && (adType <= GAP_ADTYPE_SERVICE_DATA))
            || (adType == GAP_ADTYPE_MANUFACTURER_SPECIFIC) )
    {
        return ( TRUE );
    }
    else
    {
        return ( FALSE );
    }
}

/*********************************************************************
    @fn          gapFindADType

    @brief       Find Advertisement Data Type field in advertising data
                field.

    @param       adType - Advertisment Data type
    @param       pAdLen - pointer to variable, this value is updated
                   with found token's length
    @param       dataLen - length of the dataField
    @param       pDataField - pointer to the data field (advertisement data)

    @return      found token pointer or NULL if not found
*/
uint8* gapFindADType( uint8 adType, uint8* pAdLen,
                      uint8 dataLen, uint8* pDataField )
{
    uint8 x = 0; // dataField index
    *pAdLen = 0; // Initialize the field

    // Look through the dataField for tokens
    while ( x < dataLen )
    {
        // Get the token length
        uint8 tokenLen = pDataField[x++];

        // Check valid token lengths
        if ( (tokenLen > 0) && ((x + tokenLen) <= dataLen) )
        {
            // Get the token type - ADType
            uint8 tokenType = pDataField[x++];

            // Check the token's validity
            if ( gapValidADType( tokenType ) == FALSE )
            {
                // Advertisement packet is invalid, get out
                return ( (uint8*)NULL );
            }

            // Save off the token data length
            *pAdLen = (uint8)(tokenLen - 1);

            if ( tokenType == adType )
            {
                // Found
                return ( &pDataField[x] );
            }
            else
            {
                // Skip the token data
                x += (*pAdLen);
            }
        }
        else
        {
            break; // Force end no more tokens
        }
    }

    // Not found
    return ( (uint8*)NULL );
}


/****************************************************************************
****************************************************************************/
