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
    Filename:       gap_devmgr.c
    Revised:
    Revision:

    Description:    This file contains the GAP Device Manager.


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
