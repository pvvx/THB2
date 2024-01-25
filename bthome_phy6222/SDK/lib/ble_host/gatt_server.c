/*************************************************************************************************
    Filename:       gatt_server.c
    Revised:
    Revision:

    Description:    This file contains the Generic Attribute Profile Server.

	SDK_LICENSE

**************************************************************************************************/


/*********************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "linkdb.h"

#include "gatt.h"
#include "gatt_uuid.h"
#include "gatt_internal.h"


/*********************************************************************
    MACROS
*/
// Parse and process request function pointers
#define gattParseReq( method )     ( serverReqTbl[((method)/2)-1].pfnParseReq )
#define gattProcessReq( method )   ( serverReqTbl[((method)/2)-1].pfnProcessReq )

/*********************************************************************
    CONSTANTS
*/

/*********************************************************************
    TYPEDEFS
*/
// Service record list item
typedef struct _attAttrList
{
    struct _attAttrList* next;  // pointer to next service record
    gattService_t service;      // service record
} gattServiceList_t;


// Structure to keep the Parse and Process function pointers for Requests
typedef struct
{
    gattParseReq_t pfnParseReq;     // function to parse an incoming request
    gattProcessReq_t pfnProcessReq; // function to process an incoming request
} gattHandleReq_t;

/*********************************************************************
    GLOBAL VARIABLES
*/


// Server Info table (one entry per each physical link)
gattServerInfo_t serverInfoTbl[GATT_MAX_NUM_CONN];

// Task to be notified of requests
uint8 reqTaskId = INVALID_TASK_ID;

/*********************************************************************
    EXTERNAL VARIABLES
*/

/*********************************************************************
    EXTERNAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/
// GATT Service List
static gattServiceList_t* pServiceList = NULL;

// Next available attribute handle
static uint16 nextHandle = GATT_MIN_HANDLE;

/*********************************************************************
    LOCAL FUNCTIONS
*/
static bStatus_t gattProcessExchangeMTUReq( uint16 connHandle, attMsg_t* pMsg );
static bStatus_t gattProcessFindInfoReq( uint16 connHandle, attMsg_t* pMsg );
static bStatus_t gattProcessFindByTypeValueReq( uint16 connHandle, attMsg_t* pMsg );
static bStatus_t gattProcessReadByTypeReq( uint16 connHandle, attMsg_t* pMsg );
static bStatus_t gattProcessReadReq( uint16 connHandle, attMsg_t* pMsg );
static bStatus_t gattProcessReadMultiReq( uint16 connHandle, attMsg_t* pMsg );
static bStatus_t gattProcessReadByGrpTypeReq( uint16 connHandle, attMsg_t* pMsg );
static bStatus_t gattProcessWriteReq( uint16 connHandle, attMsg_t* pMsg );
static bStatus_t gattProcessExecuteWriteReq( uint16 connHandle, attMsg_t* pMsg );

static void gattStoreServerInfo( gattServerInfo_t* pServer, uint8 taskId );
//static bStatus_t gattGetServerStatus( uint16 connHandle, gattServerInfo_t** p2pServer );
static gattServerInfo_t* gattFindServerInfo( uint16 connHandle );
static void gattResetServerInfo( gattServerInfo_t* pServer );
static void gattServerStartTimer( uint8* pData, uint16 timeout, uint8* pTimerId );
static uint16 gattServiceLastHandle( uint16 handle );

// Callback functions
static bStatus_t gattServerProcessMsgCB( uint16 connHandle, attPacket_t* pPkt );
static void gattServerHandleTimerCB( uint8* pData );
static void gattServerHandleConnStatusCB( uint16 connectionHandle, uint8 changeType );

/*********************************************************************
    Server Handle Request Table
*/
static CONST gattHandleReq_t serverReqTbl[] =
{
    { ATT_ParseExchangeMTUReq,     gattProcessExchangeMTUReq     }, /* ATT_EXCHANGE_MTU_REQ */
    { ATT_ParseFindInfoReq,        gattProcessFindInfoReq        }, /* ATT_FIND_INFO_REQ */
    { ATT_ParseFindByTypeValueReq, gattProcessFindByTypeValueReq }, /* ATT_FIND_BY_TYPE_VALUE_REQ */
    { ATT_ParseReadByTypeReq,      gattProcessReadByTypeReq      }, /* ATT_READ_BY_TYPE_REQ */
    { ATT_ParseReadReq,            gattProcessReadReq            }, /* ATT_READ_REQ */
    { ATT_ParseReadBlobReq,        gattProcessReadReq            }, /* ATT_READ_BLOB_REQ */
    { ATT_ParseReadMultiReq,       gattProcessReadMultiReq       }, /* ATT_READ_MULTI_REQ */
    { ATT_ParseReadByTypeReq,      gattProcessReadByGrpTypeReq   }, /* ATT_READ_BY_GRP_TYPE_REQ */
    { ATT_ParseWriteReq,           gattProcessWriteReq           }, /* ATT_WRITE_REQ */
    { NULL,                        NULL                          }, /* Undefined */
    { ATT_ParsePrepareWriteReq,    gattProcessWriteReq           }, /* ATT_PREPARE_WRITE_REQ */
    { ATT_ParseExecuteWriteReq,    gattProcessExecuteWriteReq    }  /* ATT_EXECUTE_WRITE_REQ */
};

/*********************************************************************
    API FUNCTIONS
*/

/*  -------------------------------------------------------------------
    GATT Server Public APIs
*/

/******************************************************************************
    @fn      GATT_InitServer

    @brief   Initialize the Generic Attribute Profile Server.

    @return  SUCCESS: Server initialized successfully.
*/
bStatus_t GATT_InitServer( void )
{
    // Mark all Server records as unused
    for ( uint8 i = 0; i < GATT_MAX_NUM_CONN; i++ )
    {
        gattServerInfo_t* pServer = &serverInfoTbl[i];

        // Initialize connection handle
        if ( i == 0 )
        {
            pServer->connHandle = LOOPBACK_CONNHANDLE;
        }
        else
        {
            pServer->connHandle = INVALID_CONNHANDLE;
        }

        // Initialize Handle Value Confirmation info
        pServer->taskId = INVALID_TASK_ID;
        pServer->timerId = INVALID_TIMER_ID;
    }

    // Set up the server's processing function
    gattRegisterServer( gattServerProcessMsgCB );
    // Register with Link DB to receive link status change callback
    linkDB_Register( gattServerHandleConnStatusCB );
    return ( SUCCESS );
}

/******************************************************************************
    @fn      GATT_RegisterService

    @brief   Register a service attribute list with the GATT Server. A service
            is composed of characteristics or references to other services.
            Each characteristic contains a value and may contain optional
            information about the value. There are two types of services:
            primary service and secondary service.

            A service definition begins with a service declaration and ends
            before the next service declaration or the maximum Attribute Handle.

            A characteristic definition begins with a characteristic declaration
            and ends before the next characteristic or service declaration or
            maximum Attribute Handle.

            The attribute server will only keep a pointer to the attribute
            list, so the calling application will have to maintain the code
            and RAM associated with this list.

    @param   pService - pointer to service attribute list to be registered

    @return  SUCCESS: Service registered successfully.
            INVALIDPARAMETER: Invalid service field.
            FAILURE: Not enough attribute handles available.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t GATT_RegisterService( gattService_t* pService )
{
    gattServiceList_t* pNewItem;

    // Make sure the service attribute list begins with a service declaration
    if ( ( pService->numAttrs == 0 ) || !gattServiceType( pService->attrs[0].type ) )
    {
        return ( INVALIDPARAMETER );
    }

    // Make sure we have enough attribute handles available for this service
    if ( ( nextHandle == 0 ) || ( pService->numAttrs > ( GATT_MAX_HANDLE - nextHandle ) + 1 ) )
    {
        return ( FAILURE );
    }

    // Allocate space for the new service item
    pNewItem = (gattServiceList_t*)osal_mem_alloc( sizeof( gattServiceList_t ) );

    if ( pNewItem == NULL )
    {
        // Not enough memory
        return ( bleMemAllocError );
    }

    // Assign attribute handles
    for ( uint16 i = 0; i < pService->numAttrs; i++ )
    {
        pService->attrs[i].handle = nextHandle++;
    }

    // Set up new service item
    pNewItem->next = NULL;
    osal_memcpy( &(pNewItem->service), pService, sizeof( gattService_t ) );

    // Find spot in list
    if ( pServiceList == NULL )
    {
        // First item in list
        pServiceList = pNewItem;
    }
    else
    {
        gattServiceList_t* pLoop = pServiceList;

        // Look for end of list
        while ( pLoop->next != NULL )
        {
            pLoop = pLoop->next;
        }

        // Put new item at end of list
        pLoop->next = pNewItem;
    }

    return ( SUCCESS );
}

/******************************************************************************
    @fn      GATT_DeregisterService

    @brief   Deregister a service attribute list with the GATT Server.

            NOTE: It's the caller's responsibility to free the service attribute
            list returned from this API.

    @param   handle - handle of service to be deregistered
    @param   pService - pointer to deregistered service (to be returned)

    @return  SUCCESS: Service deregistered successfully.
            FAILURE: Service not found.
*/
bStatus_t GATT_DeregisterService( uint16 handle, gattService_t* pService )
{
    gattServiceList_t* pLoop = pServiceList;
    gattServiceList_t* pPrev = NULL;

    // Look for service
    while ( pLoop != NULL )
    {
        if ( pLoop->service.attrs[0].handle == handle )
        {
            // Service found; unlink it
            if ( pPrev == NULL )
            {
                // First item in list
                pServiceList = pLoop->next;
            }
            else
            {
                pPrev->next = pLoop->next;
            }

            // Application will free the service attribute list
            if ( pService != NULL )
            {
                VOID osal_memcpy( pService, &(pLoop->service), sizeof( gattService_t ) );
            }

            // Free the service record
            osal_mem_free( pLoop );
            return ( SUCCESS );
        }

        pPrev = pLoop;
        pLoop = pLoop->next;
    }

    // Service not found
    return ( FAILURE );
}

/******************************************************************************
    @fn      GATT_RegisterForReq

    @brief   Register to receive incoming ATT Requests.

    @param   taskId ? task to forward requests to

    @return  void
*/
void GATT_RegisterForReq( uint8 taskId )
{
    reqTaskId = taskId;
}

/*********************************************************************
    @fn      GATT_VerifyReadPermissions

    @brief   Verify the permissions of an attribute for reading.

    @param   connHandle - connection to use
    @param   permissions - attribute permissions

    @return  SUCCESS: Attribute can be read
            ATT_ERR_READ_NOT_PERMITTED: Attribute cannot be read
            ATT_ERR_INSUFFICIENT_AUTHEN: Attribute requires authentication
            ATT_ERR_INSUFFICIENT_KEY_SIZE: Key Size used for encrypting is insufficient
            ATT_ERR_INSUFFICIENT_ENCRYPT: Attribute requires encryption
*/
bStatus_t GATT_VerifyReadPermissions( uint16 connHandle, uint8 permissions )
{
    // Make sure the requesting device has sufficient security
    if ( gattPermitAuthorRead( permissions ) || gattPermitAuthenRead( permissions ) )
    {
        // Authorization is handled by the application/profile but make sure the
        // requesting device is authenticated and the link is encrypted.
        return ( linkDB_Authen( connHandle, GATT_ENCRYPT_KEY_SIZE, TRUE ) );
    }

    if ( gattPermitEncryptRead( permissions ) )
    {
        // Read operation requires an encrypted link (unauthenticated)
        return ( linkDB_Authen( connHandle, GATT_ENCRYPT_KEY_SIZE, FALSE ) );
    }

    // Make sure the attribute has sufficient permissions to allow reading
    if ( !gattPermitRead( permissions ) )
    {
        return ( ATT_ERR_READ_NOT_PERMITTED );
    }

    return ( SUCCESS );
}

/*********************************************************************
    @fn      GATT_VerifyWritePermissions

    @brief   Verify the permissions of an attribute for writing.

    @param   connHandle - connection to use
    @param   permissions - attribute permissions
    @param   pReq - pointer to write request

    @return  SUCCESS: Attribute can be written
            ATT_ERR_READ_NOT_PERMITTED: Attribute cannot be written
            ATT_ERR_INSUFFICIENT_AUTHEN: Attribute requires authentication
            ATT_ERR_INSUFFICIENT_KEY_SIZE: Key Size used for encrypting is insufficient
            ATT_ERR_INSUFFICIENT_ENCRYPT: Attribute requires encryption
*/
bStatus_t GATT_VerifyWritePermissions( uint16 connHandle, uint8 permissions, attWriteReq_t* pReq )
{
    // Make sure the requesting device has sufficient security
    if ( gattPermitAuthorWrite( permissions ) )
    {
        // Authorization is handled by the application/profile but make sure the
        // requesting device is authenticated and the link is encrypted.
        return ( linkDB_Authen( connHandle, GATT_ENCRYPT_KEY_SIZE, TRUE ) );
    }

    if ( gattPermitAuthenWrite( permissions ) || gattPermitEncryptWrite( permissions ) )
    {
        uint8 status = linkDB_Authen( connHandle, GATT_ENCRYPT_KEY_SIZE, gattPermitAuthenWrite( permissions ) );

        // Write operation requires an encrypted link or an authenticated signed command
        if ( ( status != SUCCESS ) && ( ( pReq->cmd == FALSE ) || ( pReq->sig != ATT_SIG_VALID ) ) )
        {
            return ( status );
        }
    }
    // Make sure the attribute has sufficient permissions to allow writing
    else if ( !gattPermitWrite( permissions ) )
    {
        return ( ATT_ERR_WRITE_NOT_PERMITTED );
    }

    return ( SUCCESS );
}

/*********************************************************************
    @fn      GATT_ServiceChangedInd

    @brief   Send out a Service Changed Indication.

    @param   connHandle - connection to use
    @param   taskId - task to be notified of confirmation

    @return  SUCCESS: Indication was sent successfully.
            FAILURE: Service Changed attribute not found.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A confirmation is pending with this client.
*/
uint8 GATT_ServiceChangedInd( uint16 connHandle, uint8 taskId )
{
    gattAttribute_t* pAttr;
    // Find the Service Changed attribute record
    pAttr = GATT_FindHandleUUID( GATT_MIN_HANDLE, GATT_MAX_HANDLE,
                                 serviceChangedUUID, ATT_BT_UUID_SIZE, NULL );

    if ( pAttr != NULL )
    {
        attHandleValueInd_t ind;
        ind.handle = pAttr->handle;
        // Set the affected Attribute Handle range to 0x0001 to 0xFFFF to
        // indicate to the client to rediscover the entire set of Attribute
        // Handles on the server.
        ind.len = 4;
        ind.value[0] = LO_UINT16( GATT_MIN_HANDLE );
        ind.value[1] = HI_UINT16( GATT_MIN_HANDLE );
        ind.value[2] = LO_UINT16( GATT_MAX_HANDLE );
        ind.value[3] = HI_UINT16( GATT_MAX_HANDLE );
        return ( GATT_Indication( connHandle, &ind, FALSE, taskId ) );
    }

    return ( FAILURE );
}

/*********************************************************************
    @fn      GATT_FindHandleUUID

    @brief   Find the attribute record for a given handle and UUID.

    @param   startHandle - first handle to look for
    @param   endHandle - last handle to look for
    @param   pUUID - pointer to UUID to look for
    @param   len - length of UUID
    @param   pHandle - handle of owner of attribute (to be returned)

    @return  Pointer to attribute record. NULL, otherwise.
*/
gattAttribute_t* GATT_FindHandleUUID( uint16 startHandle, uint16 endHandle, const uint8* pUUID,
                                      uint16 len, uint16* pHandle )
{
    gattServiceList_t* pLoop = pServiceList;

    while ( pLoop != NULL )
    {
        for ( uint16 i = 0; i < pLoop->service.numAttrs; i++ )
        {
            gattAttribute_t* pAttr = &(pLoop->service.attrs[i]);

            // Check to see if this handle falls within the starting and ending handles
            if ( ( pAttr->handle >= startHandle ) && ( pAttr->handle <= endHandle ) )
            {
                // Compare UUIDs if one is provided
                if ( ( len == 0 ) ||
                        ( ATT_CompareUUID( pAttr->type.uuid, pAttr->type.len,
                                           pUUID, len ) ) )
                {
                    // Entry found
                    if ( pHandle != NULL )
                    {
                        // Handle of the service that the attribute belongs to
                        *pHandle = pLoop->service.attrs[0].handle;
                    }

                    return ( pAttr );
                }
            }
        }

        // Try next service
        pLoop = pLoop->next;
    }

    return ( (gattAttribute_t*)NULL );
}

/*********************************************************************
    @fn      GATT_FindHandle

    @brief   Find the attribute record for a given handle

    @param   handle - handle to look for
    @param   pHandle - handle of owner of attribute (to be returned)

    @return  Pointer to attribute record. NULL, otherwise.
*/
gattAttribute_t* GATT_FindHandle( uint16 handle, uint16* pHandle )
{
    gattServiceList_t* pLoop = pServiceList;

    while ( pLoop != NULL )
    {
        uint16 serviceHandle = pLoop->service.attrs[0].handle;

        // See if the handle falls within this service
        if ( ( handle >= serviceHandle ) && ( handle < serviceHandle + pLoop->service.numAttrs ) )
        {
            for ( uint16 i = 0; i < pLoop->service.numAttrs; i++ )
            {
                gattAttribute_t* pAttr = &(pLoop->service.attrs[i]);

                if ( pAttr->handle == handle )
                {
                    // Entry found
                    if ( pHandle != NULL )
                    {
                        // Handle of the service that the attribute belongs to
                        *pHandle = serviceHandle;
                    }

                    return ( pAttr );
                }
            }
        }

        // Try next service
        pLoop = pLoop->next;
    }

    return ( (gattAttribute_t*)NULL );
}

/*********************************************************************
    @fn      GATT_FindNextAttr

    @brief   Find the next attribute of the same type for a given attribute.

    @param   pAttr - pointer to attribute to find a next for
    @param   endHandle - last handle to look for
    @param   service - handle of owner service
    @param   pLastHandle - handle of last attribute (to be returned)

    @return  Pointer to next attribute record. NULL, otherwise.
*/
gattAttribute_t* GATT_FindNextAttr( gattAttribute_t* pAttr, uint16 endHandle,
                                    uint16 service, uint16* pLastHandle )
{
    uint16 lastHandle;
    gattAttribute_t* pNext = NULL;
    uint16 owner = GATT_INVALID_HANDLE;

    // Try to find the next attribute of the same type

    // All attribute types are effectively compared as 128-bit UUIDs,
    // even if a 16-bit UUID is provided in this request or defined
    // for an attribute.
    if ( pAttr->handle != GATT_MAX_HANDLE )
    {
        pNext = GATT_FindHandleUUID( pAttr->handle+1, endHandle, pAttr->type.uuid,
                                     pAttr->type.len, &owner );
    }

    // Try to find the handle of the last attribute
    if ( gattServiceType( pAttr->type ) )
    {
        // Get the handle of the last attribute withis this service
        lastHandle = gattServiceLastHandle( pAttr->handle );
    }
    else if ( gattCharacterType( pAttr->type ) )
    {
        // Check to see if this is the last characteristic within the service
        if ( ( pNext == NULL ) || ( owner != service ) )
        {
            lastHandle = gattServiceLastHandle( service );
        }
        else
        {
            lastHandle = pNext->handle - 1;
        }
    }
    else
    {
        // Not a grouping attribute -- return its handle
        lastHandle = pAttr->handle;
    }

    if ( pLastHandle != NULL )
    {
        *pLastHandle = lastHandle;
    }

    return ( pNext );
}

/*********************************************************************
    @fn      GATT_ServiceNumAttrs

    @brief   Get the number of attributes for a given service

    @param   handle - service handle to look for

    @return  Number of attributes. 0, otherwise.
*/
uint16 GATT_ServiceNumAttrs( uint16 handle )
{
    gattServiceList_t* pLoop = pServiceList;

    while ( pLoop != NULL )
    {
        if ( pLoop->service.attrs[0].handle == handle )
        {
            // Service found
            return ( pLoop->service.numAttrs );
        }

        // Try next service
        pLoop = pLoop->next;
    }

    return ( 0 );
}

/*  -------------------------------------------------------------------
    GATT Server Sub-Procedure APIs
*/

/*********************************************************************
    @fn      GATT_Indication

    @brief   This sub-procedure is used when a server is configured to
            indicate a characteristic value to a client and expects an
            attribute protocol layer acknowledgement that the indication
            was successfully received.

            The ATT Handle Value Indication is used in this sub-procedure.

            If the return status from this function is SUCCESS, the calling
            application task will receive an OSAL GATT_MSG_EVENT message.
            The type of the message will be ATT_HANDLE_VALUE_CFM.

            Note: This sub-procedure is complete when ATT_HANDLE_VALUE_CFM
                  (with SUCCESS or bleTimeout status) is received by the
                  calling application task.

    @param   connHandle - connection to use
    @param   pInd - pointer to indication to be sent
    @param   taskId - task to be notified of confirmation
    @param   authenticated - whether an authenticated link is required

    @return  SUCCESS: Indication was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            ATT_ERR_INSUFFICIENT_AUTHEN - link is not encrypted
            ATT_ERR_INSUFFICIENT_KEY_SIZE - key size encrypted is not large enough
            ATT_ERR_INSUFFICIENT_ENCRYPT - link is encrypted, but not authenticated
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A confirmation is pending with this client.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_Indication( uint16 connHandle, attHandleValueInd_t* pInd,
                           uint8 authenticated, uint8 taskId )
{
    gattServerInfo_t* pServer;
    uint8 status;
    // Make sure we're allowed to send a new indication
    status = gattGetServerStatus( connHandle, &pServer );

    if ( status == SUCCESS )
    {
        // Make sure the link is authenticated if requested
        if ( authenticated )
        {
            // Indication operation requires authentication
            status = linkDB_Authen( connHandle, GATT_ENCRYPT_KEY_SIZE, TRUE );
        }

        if ( status == SUCCESS )
        {
            status = ATT_HandleValueInd( connHandle, pInd );

            if ( status == SUCCESS )
            {
                // Store server info
                gattStoreServerInfo( pServer, taskId );
            }
        }
    }

    return ( status );
}

/*********************************************************************
    @fn      GATT_Notification

    @brief   This sub-procedure is used when a server is configured to
            notify a characteristic value to a client without expecting
            any attribute protocol layer acknowledgement that the
            notification was successfully received.

            The ATT Handle Value Notification is used in this sub-procedure.

            Note: A notification may be sent at any time and does not
            invoke a confirmation.

            No confirmation will be sent to the calling application task for
            this sub-procedure.

    @param   connHandle - connection to use
    @param   pNoti - pointer to notification to be sent
    @param   authenticated - whether an authenticated link is required

    @return  SUCCESS: Notification was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            ATT_ERR_INSUFFICIENT_AUTHEN - link is not encrypted
            ATT_ERR_INSUFFICIENT_KEY_SIZE - key size encrypted is not large enough
            ATT_ERR_INSUFFICIENT_ENCRYPT - link is encrypted, but not authenticated
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_Notification( uint16 connHandle, attHandleValueNoti_t* pNoti,
                             uint8 authenticated )
{
    gattServerInfo_t* pServer;
    uint8 status;
    // Make sure we're allowed to send a new notification
    status = gattGetServerStatus( connHandle, &pServer );

    if ( status != bleTimeout )
    {
        // Make sure the link is authenticated if requested
        if ( authenticated )
        {
            // Notification operation requires authentication
            status = linkDB_Authen( connHandle, GATT_ENCRYPT_KEY_SIZE, TRUE );

            if ( status != SUCCESS )
            {
                return ( status );
            }
        }

        status = ATT_HandleValueNoti( connHandle, pNoti );
    }

    return ( status );
}

/*  -------------------------------------------------------------------
    GATT Server Internal Functions
*/

/*********************************************************************
    @fn      gattServiceLastHandle

    @brief   Get the handle of the last attribute within a given service.

    @param   handle - service handle

    @return  Handle of last attribute for service. Handle, otherwise.
*/
static uint16 gattServiceLastHandle( uint16 handle )
{
    uint16 lastHandle;
    // Find out the handle of the last attribute withis this service
    lastHandle = GATT_ServiceNumAttrs( handle );

    if ( lastHandle != 0 )
    {
        lastHandle += (handle - 1);
    }
    else
    {
        lastHandle = handle;
    }

    return ( lastHandle );
}

/*********************************************************************
    @fn      gattStoreServerInfo

    @brief   Store server info.

    @param   connHandle ? connection to use
    @param   taskId ? task to be notified of response

    @return  void
*/
static void gattStoreServerInfo( gattServerInfo_t* pServer, uint8 taskId )
{
    if ( taskId != INVALID_TASK_ID )
    {
        // Start a timeout timer for the confirmation
        gattServerStartTimer( (uint8*)pServer, ATT_MSG_TIMEOUT, &pServer->timerId );
        // Store task id to forward the confirmation to
        pServer->taskId = taskId;
    }
}

/*********************************************************************
    @fn          gattProcessServerMsgCB

    @brief       GATT Server message processing function.

    @param       connHandle - connection packet was received on
    @param       pPkt - pointer to received packet

    @return      SUCCESS: Message processed successfully
                ATT_ERR_UNSUPPORTED_REQ: Unsupported request or command
                ATT_ERR_INVALID_PDU: Invalid PDU
                bleMemAllocError: Memory allocation error occurred
*/
static bStatus_t gattServerProcessMsgCB( uint16 connHandle, attPacket_t* pPkt )
{
    gattMsg_t msg;
    uint8 status = SUCCESS;

    // See if this is a confirmation to an indication
    if ( pPkt->method == ATT_HANDLE_VALUE_CFM )
    {
        gattServerInfo_t* pServer;
        // Make sure we have the info about the Server that sent the indication
        pServer = gattFindServerInfo( connHandle );

        if ( pServer != NULL )
        {
            status = gattNotifyEvent( pServer->taskId, connHandle, SUCCESS, pPkt->method, NULL );
            // Reset server info
            gattResetServerInfo( pServer );
        }

        // We're done here
        return ( status );
    }

    // Make sure the incoming request or command is supported
    if ( ( reqTaskId == INVALID_TASK_ID )           ||
            ( pPkt->method > ATT_EXECUTE_WRITE_REQ )   ||
            ( gattParseReq( pPkt->method ) == NULL )   ||
            ( gattProcessReq( pPkt->method ) == NULL ) ||
            ( ( ( pPkt->sig != ATT_SIG_NOT_INCLUDED )  ||
                ( pPkt->cmd == TRUE ) )                &&
              ( pPkt->method != ATT_WRITE_REQ ) ) )
    {
        // Unsupported request or command
        return ( ATT_ERR_UNSUPPORTED_REQ );
    }

    // Parse the incoming request or command
    status = gattParseReq( pPkt->method )( pPkt->sig, pPkt->cmd, pPkt->pParams,
                                           pPkt->len, (attMsg_t*)&msg );

    if ( status == SUCCESS )
    {
        // Try to process the request or command
        status = gattProcessReq( pPkt->method )( connHandle, (attMsg_t*)&msg );

        if ( status == SUCCESS )
        {
            // Foward the request up to the application for further processing
            if ( pPkt->method != ATT_FIND_INFO_REQ )
            {
                status = gattNotifyEvent( reqTaskId, connHandle, SUCCESS, pPkt->method, &msg );
            }
        }
        // Do not send an error response back for any command
        else if ( pPkt->cmd == FALSE )
        {
            attErrorRsp_t errorRsp;
            // Send an Error Response back
            errorRsp.reqOpcode = pPkt->method;
            errorRsp.errCode = status;

            // Set the handle
            if ( ( pPkt->method == ATT_FIND_INFO_REQ )          ||
                    ( pPkt->method == ATT_FIND_BY_TYPE_VALUE_REQ ) ||
                    ( pPkt->method == ATT_READ_BY_TYPE_REQ )       ||
                    ( pPkt->method == ATT_READ_BY_GRP_TYPE_REQ ) )
            {
                // Set handle to the starting handle
                errorRsp.handle = msg.findInfoReq.startHandle;
            }
            else
            {
                // All requests share the handle field
                errorRsp.handle = msg.readReq.handle;
            }

            // Send an Error Response back
            VOID ATT_ErrorRsp( connHandle, &errorRsp );
            // We're done with this request
            status = SUCCESS;
        }
    }

    return ( status );
}

/*********************************************************************
    @fn      gattProcessExchangeMTUReq

    @brief   Process Exchange MTU Request.

    @param   connHandle ? connection message was received on
    @param   pMsg - pointer to message structure

    @return  SUCCESS: Forward the request up to the application
*/
static bStatus_t gattProcessExchangeMTUReq( uint16 connHandle, attMsg_t* pMsg )
{
    VOID connHandle; // Not used here
    //VOID pMsg; // Not used here
    attExchangeMTUReq_t* pReq = &pMsg->exchangeMTUReq;
    g_attMtuClientServer.clientMTU=ATT_MTU_SIZE_MIN;

    if(pReq->clientRxMTU < ATT_MTU_SIZE_MIN)
    {
        return (ATT_ERR_INVALID_PDU);
    }

    g_attMtuClientServer.clientMTU=pReq->clientRxMTU;
    return ( SUCCESS );
}

/*********************************************************************
    @fn      gattProcessFindInfoReq

    @brief   Process Find Information Request.

    @param   connHandle ? connection message was received on
    @param   pMsg - pointer to message structure

    @return  SUCCESS: Forward the request up to the application
            ATT_ERR_INVALID_HANDLE: Invalid attribute handle
            ATT_ERR_ATTR_NOT_FOUND: Attribute not found
*/
static bStatus_t gattProcessFindInfoReq( uint16 connHandle, attMsg_t* pMsg )
{
    attFindInfoReq_t* pReq = &pMsg->findInfoReq;
    uint16 startHandle = pReq->startHandle;
    attFindInfoRsp_t rsp;
    uint8 done = FALSE;

    // If the starting handle greater than the ending handle or the starting
    // handle is 0x0000 then return the status code Invalid Handle
    if ( ( startHandle > pReq->endHandle ) || ( startHandle == 0 ) )
    {
        return ( ATT_ERR_INVALID_HANDLE );
    }

    rsp.numInfo = 0;

    while ( !done )
    {
        // All attribute types are effectively compared as 128 bit UUIDs, even if
        // a 16 bit UUID is provided in this request or defined for an attribute.
        gattAttribute_t* pAttr = GATT_FindHandleUUID( startHandle, pReq->endHandle,
                                                      NULL, 0, NULL );

        // If the size of the UUID Filter parameter is 0 octets, then all attribute
        // types will be returned. If the size of the UUID Filter parameter is 2 or
        // 16 octets, then only attribute types with this UUID Filter will be returned.
        if ( pAttr == NULL )
        {
            // No more attributes found
            break;
        }

        // Is this the first UUID found?
        if ( rsp.numInfo == 0 )
        {
            // Set the Format field using the first UUID's length
            if ( pAttr->type.len == ATT_BT_UUID_SIZE )
            {
                // A list of 1 or more handles with their 16 bit Bluetooth UUIDs
                rsp.format = ATT_HANDLE_BT_UUID_TYPE;
            }
            else
            {
                // A list of 1 or more handles with their 128 bit UUIDs
                rsp.format = ATT_HANDLE_UUID_TYPE;
            }
        }

        // Copy handle and UUID into the response
        if ( rsp.format == ATT_HANDLE_BT_UUID_TYPE )
        {
            // Handle with its 16 bit Bluetooth UUID
            if ( pAttr->type.len != ATT_BT_UUID_SIZE )
            {
                // It's not possible to include attributes with differing UUID sizes
                // into a single response
                break;
            }

            rsp.info.btPair[rsp.numInfo].handle = pAttr->handle;
            VOID osal_memcpy( rsp.info.btPair[rsp.numInfo].uuid, pAttr->type.uuid, ATT_BT_UUID_SIZE );

            if ( ( ++rsp.numInfo >= ATT_MAX_NUM_HANDLE_BT_UUID ) ||
                    ( pAttr->handle == GATT_MAX_HANDLE ) )
            {
                done = TRUE; // We're done successfully
            }
        }
        else // ATT_HANDLE_UUID_TYPE
        {
            // Handle with its 128 bit Bluetooth UUID
            rsp.info.pair[rsp.numInfo].handle = pAttr->handle;
            VOID osal_memcpy( rsp.info.pair[rsp.numInfo].uuid, pAttr->type.uuid, ATT_UUID_SIZE );

            if ( ( ++rsp.numInfo >= ATT_MAX_NUM_HANDLE_UUID ) ||
                    ( pAttr->handle == GATT_MAX_HANDLE ) )
            {
                done = TRUE; // We're done successfully
            }
        }

        // Update start handle and search again
        startHandle = pAttr->handle + 1;
    }

    // If no attribute is found then return the status code Attribute Not Found
    if ( rsp.numInfo == 0 )
    {
        return ( ATT_ERR_ATTR_NOT_FOUND );
    }

    // Send a Find Info Response back
    VOID ATT_FindInfoRsp( connHandle, &rsp );
    return ( SUCCESS );
}

/*********************************************************************
    @fn      gattProcessFindByTypeVaueReq

    @brief   Process Find By Type Value Request.

    @param   connHandle ? connection message was received on
    @param   pMsg - pointer to message structure

    @return  SUCCESS: Forward the request up to the application
            ATT_ERR_INVALID_HANDLE: Invalid attribute handle
            ATT_ERR_ATTR_NOT_FOUND: Attribute not found
*/
static bStatus_t gattProcessFindByTypeValueReq( uint16 connHandle, attMsg_t* pMsg )
{
    attFindByTypeValueReq_t* pReq = &pMsg->findByTypeValueReq;
    VOID connHandle; // Not used here

    // If the starting handle greater than the ending handle or the starting
    // handle is 0x0000 then return the status code Invalid Handle
    if ( ( pReq->startHandle > pReq->endHandle ) || ( pReq->startHandle == 0 ) )
    {
        return ( ATT_ERR_INVALID_HANDLE );
    }

    // Only attributes with attribute handles between and including the Starting
    // Handle parameter and the Ending Handle parameter that match the requested
    // attribute type and the attribute value will be returned.

    // All attribute types are effectively compared as 128 bit UUIDs, even if
    // a 16 bit UUID is provided in this request or defined for an attribute.
    if ( GATT_FindHandleUUID( pReq->startHandle, pReq->endHandle,
                              pReq->type.uuid, pReq->type.len, NULL ) == NULL )
    {
        // If no attribute with the given type exists within the handle range
        // then return the status code Attribute Not Found
        return ( ATT_ERR_ATTR_NOT_FOUND );
    }

    // Forward the request up to the application
    return ( SUCCESS );
}

/*********************************************************************
    @fn      gattProcessReadByTypeReq

    @brief   Process Read By Type Request.

    @param   connHandle ? connection message was received on
    @param   pMsg - pointer to message structure

    @return  SUCCESS: Forward the request up to the application
            ATT_ERR_INVALID_HANDLE: Invalid attribute handle
            ATT_ERR_ATTR_NOT_FOUND: Attribute not found
*/
static bStatus_t gattProcessReadByTypeReq( uint16 connHandle, attMsg_t* pMsg )
{
    attReadByTypeReq_t* pReq = &pMsg->readByTypeReq;
    VOID connHandle; // Not used here

    // If the starting handle greater than the ending handle or the starting
    // handle is 0x0000 then return the status code Invalid Handle
    if ( ( pReq->startHandle > pReq->endHandle ) || ( pReq->startHandle == 0 ) )
    {
        return ( ATT_ERR_INVALID_HANDLE );
    }

    // Only an attribute with attribute type that is the same as the Type
    // given will be returned. The attribute returned must be the attribute
    // with the lowest handle within the handle range.

    // All attribute types are effectively compared as 128 bit UUIDs, even if
    // a 16 bit UUID is provided in this request or defined for an attribute.
    if ( GATT_FindHandleUUID( pReq->startHandle, pReq->endHandle,
                              pReq->type.uuid, pReq->type.len, NULL ) == NULL )
    {
        // If no attribute with the given type exists within the handle range
        // then return the status code Attribute Not Found
        return ( ATT_ERR_ATTR_NOT_FOUND );
    }

    // Forward the request up to the application
    return ( SUCCESS );
}

/*********************************************************************
    @fn      gattProcessReadReq

    @brief   Process Read Request.

    @param   connHandle ? connection message was received on
    @param   pMsg - pointer to message structure

    @return  SUCCESS: Forward the request up to the application
            ATT_ERR_INVALID_HANDLE: Invalid attribute handle
            ATT_ERR_READ_NOT_PERMITTED: Attribute cannot be read
            ATT_ERR_INSUFFICIENT_AUTHEN: Attribute requires authentication
            ATT_ERR_INSUFFICIENT_KEY_SIZE: Key Size used for encrypting is insufficient
            ATT_ERR_INSUFFICIENT_ENCRYPT: Attribute requires encryption
*/
static bStatus_t gattProcessReadReq( uint16 connHandle, attMsg_t* pMsg )
{
    gattAttribute_t* pAttr;
    uint16 handle = pMsg->readReq.handle;
    // Make sure the handle is valid
    pAttr = GATT_FindHandle( handle, NULL );

    if ( pAttr == NULL )
    {
        return ( ATT_ERR_INVALID_HANDLE );
    }

    // Forward the request up to the application (if reading allowed)
    return ( GATT_VerifyReadPermissions( connHandle, pAttr->permissions ) );
}

/*********************************************************************
    @fn      gattProcessReadMultiReq

    @brief   Process Read Multiple Request.

    @param   connHandle ? connection message was received on
    @param   pMsg - pointer to message structure

    @return  SUCCESS: Forward the request up to the application
            ATT_ERR_INVALID_HANDLE: Invalid attribute handle
            ATT_ERR_READ_NOT_PERMITTED: Attribute cannot be read
            ATT_ERR_INSUFFICIENT_AUTHEN: Attribute requires authentication
            ATT_ERR_INSUFFICIENT_KEY_SIZE: Key Size used for encrypting is insufficient
            ATT_ERR_INSUFFICIENT_ENCRYPT: Attribute requires encryption
*/
static bStatus_t gattProcessReadMultiReq( uint16 connHandle, attMsg_t* pMsg )
{
    attReadMultiReq_t* pReq = &pMsg->readMultiReq;

    // Make sure all the handles are valid and all attributes have sufficient
    // permissions to allow reading.
    for ( uint8 i = 0; i < pReq->numHandles; i++ )
    {
        gattAttribute_t* pAttr;
        uint8 status;
        // Make sure the handle is valid
        pAttr = GATT_FindHandle( pReq->handle[i], NULL );

        if ( pAttr == NULL )
        {
            // The handle of the first attribute causing the error
            pReq->handle[0] = pReq->handle[i];
            return ( ATT_ERR_INVALID_HANDLE );
        }

        // Make sure the attribute has sufficient permissions to allow reading
        status = GATT_VerifyReadPermissions( connHandle, pAttr->permissions );

        if ( status != SUCCESS )
        {
            // The handle of the first attribute causing the error
            pReq->handle[0] = pReq->handle[i];
            return ( status );
        }
    }

    // Forward the request up to the application
    return ( SUCCESS );
}

/*********************************************************************
    @fn      gattProcessReadByGrpTypeReq

    @brief   Process Read By Group Type Request.

    @param   connHandle ? connection message was received on
    @param   pMsg - pointer to message structure

    @return  SUCCESS: Forward the request up to the application
            ATT_ERR_INVALID_HANDLE: Invalid attribute handle
            ATT_ERR_UNSUPPORTED_GRP_TYPE: Group attribute type not supported
            ATT_ERR_ATTR_NOT_FOUND: Attribute not found
*/
static bStatus_t gattProcessReadByGrpTypeReq( uint16 connHandle, attMsg_t* pMsg )
{
    attReadByGrpTypeReq_t* pReq = &pMsg->readByGrpTypeReq;
    VOID connHandle; // Not used here

    // If the starting handle greater than the ending handle or the starting
    // handle is 0x0000 then return the status code Invalid Handle
    if ( ( pReq->startHandle > pReq->endHandle ) || ( pReq->startHandle == 0 ) )
    {
        return ( ATT_ERR_INVALID_HANDLE );
    }

    // If the Attribute Group Type is not a supported grouping attribute
    // the return the status code Unsupported Group Type
    if ( !gattPrimaryServiceType( pReq->type ) )
    {
        return ( ATT_ERR_UNSUPPORTED_GRP_TYPE );
    }

    // Only the attributes with attribute handles between and including the
    // Starting Handle and the Ending Handle with the attribute type that
    // is the same as the Attribute Group Type given will be returned.

    // All attribute types are effectively compared as 128 bit UUIDs, even if
    // a 16 bit UUID is provided in this request or defined for an attribute.
    if ( GATT_FindHandleUUID( pReq->startHandle, pReq->endHandle,
                              pReq->type.uuid, pReq->type.len, NULL ) == NULL )
    {
        // If no attribute with the given type exists within the handle range
        // then return the status code Attribute Not Found
        return ( ATT_ERR_ATTR_NOT_FOUND );
    }

    // Forward the request up to the application
    return ( SUCCESS );
}

/*********************************************************************
    @fn      gattProcessWriteReq

    @brief   Process Write Request or Command.

    @param   connHandle ? connection message was received on
    @param   pMsg - pointer to message structure

    @return  SUCCESS: Forward the request up to the application
            ATT_ERR_READ_NOT_PERMITTED: Attribute cannot be written
            ATT_ERR_INSUFFICIENT_AUTHEN: Attribute requires authentication
            ATT_ERR_INSUFFICIENT_KEY_SIZE: Key Size used for encrypting is insufficient
            ATT_ERR_INSUFFICIENT_ENCRYPT: Attribute requires encryption
*/
static bStatus_t gattProcessWriteReq( uint16 connHandle, attMsg_t* pMsg )
{
    gattAttribute_t* pAttr;
    attWriteReq_t* pReq = &(pMsg->writeReq);
    // Make sure the handle is valid
    pAttr = GATT_FindHandle( pReq->handle, NULL );

    if ( pAttr == NULL )
    {
        return ( ATT_ERR_INVALID_HANDLE );
    }

    // Forward the request up to the application (if writting allowed)
    return ( GATT_VerifyWritePermissions( connHandle, pAttr->permissions, pReq ) );
}

/*********************************************************************
    @fn      gattProcessExecuteWriteReq

    @brief   Process Execute Write Request.

    @param   connHandle ? connection message was received on
    @param   pMsg - pointer to message structure

    @return  SUCCESS: Forward the request up to the application
*/
static bStatus_t gattProcessExecuteWriteReq( uint16 connHandle, attMsg_t* pMsg )
{
    VOID connHandle; // Not used here
    VOID pMsg; // Not used here
    // Forward the request up to the application
    return ( SUCCESS );
}

/*********************************************************************
    @fn          gattGetServerStatus

    @brief       Get the status for a given server.

    @param       connHandle - client connection to server
    @param       p2pServer - pointer to server info (to be returned)

    @return      SUCCESS: No confirmation pending
                INVALIDPARAMETER: Invalid connection handle
                blePending: Confirmation pending
                bleTimeout: Previous transaction timed out
*/
bStatus_t gattGetServerStatus( uint16 connHandle, gattServerInfo_t** p2pServer )
{
    gattServerInfo_t* pServer;
    pServer = gattFindServerInfo( connHandle );

    if ( pServer != NULL )
    {
        if ( p2pServer != NULL )
        {
            *p2pServer = pServer;
        }

        // Make sure there's no confirmation pending or timed out with this client
        return ( TIMER_STATUS( pServer->timerId ) );
    }

    // Connection handle not found
    return ( INVALIDPARAMETER );
}

/*********************************************************************
    @fn      attFindServerInfo

    @brief   Find the server info.  Uses the connection handle to search
            the server info table.

    @param   connHandle - connection handle.

    @return  a pointer to the found item. NULL, otherwise.
*/
static gattServerInfo_t* gattFindServerInfo( uint16 connHandle )
{
    uint8 i;

    for ( i = 0; i < GATT_MAX_NUM_CONN; i++ )
    {
        if ( serverInfoTbl[i].connHandle == connHandle )
        {
            // Entry found
            return ( &serverInfoTbl[i] );
        }
    }

    return ( (gattServerInfo_t*)NULL );
}

/*********************************************************************
    @fn      gattResetServerInfo

    @brief   Reset the server info.

    @param   pServer - pointer to server info.

    @return  void
*/
static void gattResetServerInfo( gattServerInfo_t* pServer )
{
    // Cancel the confirmation timer
    gattStopTimer( &pServer->timerId );
    // Reset confirmation info
    pServer->taskId = INVALID_TASK_ID;
}

/*********************************************************************
    @fn      attServertStartTimer

    @brief   Start a server timer to expire in n seconds.

    @param   pData - data to be passed in to callback function
    @param   timeout - in milliseconds.
    @param   pTimerId - will point to new timer Id (if not null)

    @return  void
*/
static void gattServerStartTimer( uint8* pData, uint16 timeout, uint8* pTimerId )
{
    gattStartTimer( gattServerHandleTimerCB, pData, timeout, pTimerId );
}

/*********************************************************************
    @fn      gattServerHandleTimerCB

    @brief   Handle a callback for a timer that has just expired.

    @param   pData - pointer to timer data

    @return  void
*/
static void gattServerHandleTimerCB( uint8* pData )
{
    gattServerInfo_t* pServer = (gattServerInfo_t*)pData;

    // Response timer has expired
    if ( ( pServer != NULL ) && TIMER_VALID( pServer->timerId ) )
    {
        // Notify the application about the timeout
        VOID gattNotifyEvent( pServer->taskId, pServer->connHandle, bleTimeout,
                              ATT_HANDLE_VALUE_CFM, NULL );
        // Timer has expired. If a transaction has not completed before it times
        // out, then this transaction shall be considered to have failed. No more
        // attribute protocol requests, commands, indications or notifications
        // shall be sent to the target device on this ATT Bearer.
        pServer->timerId = TIMEOUT_TIMER_ID;
        // Reset confirmation info
        pServer->taskId = INVALID_TASK_ID;
    }
}

/*********************************************************************
    @fn          gattServerHandleConnStatusCB

    @brief       GATT link status change handler function.

    @param       connHandle - connection handle
    @param       changeType - type of change

    @return      void
*/
static void gattServerHandleConnStatusCB( uint16 connHandle, uint8 changeType )
{
    gattServerInfo_t* pServer = NULL;

    // Check to see if this is loopback connection
    if ( connHandle == LOOPBACK_CONNHANDLE )
    {
        return;
    }

    if ( changeType == LINKDB_STATUS_UPDATE_NEW )
    {
        // A new connection has been made
        pServer = gattFindServerInfo( connHandle );

        if ( pServer == NULL )
        {
            // Entry not found; add it to the server table
            pServer = gattFindServerInfo( INVALID_CONNHANDLE );

            if ( pServer != NULL )
            {
                // Empty entry found
                pServer->connHandle = connHandle;
            }
        }

        // We're done here!
        return;
    }

    if ( changeType == LINKDB_STATUS_UPDATE_REMOVED )
    {
        pServer = gattFindServerInfo( connHandle );

        if ( pServer != NULL )
        {
            // Entry found; remove it from the server table
            pServer->connHandle = INVALID_CONNHANDLE;
        }
    }
    else if ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS )
    {
        // Check to see if the connection has dropped
        if ( !linkDB_Up( connHandle ) )
        {
            pServer = gattFindServerInfo( connHandle );
        }
    }

    // Connection has dropped; notify the application
    if ( pServer != NULL )
    {
        if ( pServer->timerId != INVALID_TIMER_ID )
        {
            if ( pServer->timerId != TIMEOUT_TIMER_ID )
            {
                // Notify the application about the link disconnect
                VOID gattNotifyEvent( pServer->taskId, connHandle, bleNotConnected,
                                      ATT_HANDLE_VALUE_CFM, NULL );
            }

            // Reset server info
            gattResetServerInfo( pServer );
            // Just in case if we've timed out waiting for a confirmation
            pServer->timerId = INVALID_TIMER_ID;
        }
    }
}

/*********************************************************************
    @fn          GATT_SetNextHandle

    @brief       Set the next available attribute handle.

    @param       handle - next attribute handle

    @return      void
*/
void GATT_SetNextHandle( uint16 handle )
{
    if ( handle >= nextHandle )
    {
        #if defined ( GATT_QUAL )
        // Set next available attribute handle
        nextHandle = handle;
        #endif // GATT_QUAL
    }
}

/****************************************************************************
****************************************************************************/
