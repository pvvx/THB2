/*************************************************************************************************
    Filename:       gatt_client.c
    Revised:
    Revision:

    Description:    This file contains the Generic Attribute Profile Client.

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

/*********************************************************************
    CONSTANTS
*/

/*********************************************************************
    TYPEDEFS
*/
// Structure to keep Client info
typedef struct
{
    // Info maintained for Client that expecting a response back
    uint16 connHandle;          // connection message was sent out
    uint8 method;               // type of response to be received
    gattParseRsp_t pfnParseRsp; // function to parse response to be received
    uint8 timerId;              // response timeout timer id
    uint8 taskId;               // task to be notified of response

    // GATT Request message
    gattMsg_t req;              // request message

    // Info maintained for GATT Response message
    uint8 numRsps;             // number of responses received
} gattClientInfo_t;

/*********************************************************************
    GLOBAL VARIABLES
*/
// Client Info table (one entry per each physical link)
gattClientInfo_t clientInfoTbl[GATT_MAX_NUM_CONN];

// Task to be notified of Notification and Indication messages
uint8 indTaskId = INVALID_TASK_ID;

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
static void gattProcessMultiReqs( uint16 connHandle, gattClientInfo_t* pClient,
                                  uint8 method, gattMsg_t* pMsg );
static uint8 gattProcessFindInfo( gattClientInfo_t* pClient,
                                  uint8 method, gattMsg_t* pMsg );
static uint8 gattProcessFindByTypeValue( gattClientInfo_t* pClient,
                                         uint8 method, gattMsg_t* pMsg );
static uint8 gattProcessReadByType( gattClientInfo_t* pClient,
                                    uint8 method, gattMsg_t* pMsg );
static uint8 gattProcessReadLong( gattClientInfo_t* pClient,
                                  uint8 method, gattMsg_t* pMsg );
static uint8 gattProcessReadByGrpType( gattClientInfo_t* pClient,
                                       uint8 method, gattMsg_t* pMsg );
static bStatus_t gattProcessWriteLong( gattClientInfo_t* pClient,
                                       uint8 method, gattMsg_t* pMsg );
static bStatus_t gattProcessReliableWrites( gattClientInfo_t* pClient,
                                            uint8 method, gattMsg_t* pMsg );

static void gattStoreClientInfo( gattClientInfo_t* pClient, gattMsg_t* pReq,
                                 uint8 method, gattParseRsp_t pfnParseRsp, uint8 taskId );
static gattClientInfo_t* gattFindClientInfo( uint16 connHandle );
static bStatus_t gattGetClientStatus( uint16 connHandle, gattClientInfo_t** p2pClient );
static void gattResetClientInfo( gattClientInfo_t* pClient );
static void gattClientStartTimer( uint8* pData, uint16 timeout, uint8* pTimerId );

static bStatus_t gattFindInfo( uint16 connHandle, attFindInfoReq_t* pReq, uint8 taskId );
static bStatus_t gattFindByTypeValue( uint16 connHandle, attFindByTypeValueReq_t* pReq,
                                      uint8 taskId );
static bStatus_t gattReadByType( uint16 connHandle, attReadByTypeReq_t* pReq,
                                 uint8 discByCharUUID, uint8 taskId );
static bStatus_t gattRead( uint16 connHandle, attReadReq_t* pReq, uint8 taskId );
static bStatus_t gattReadLong( uint16 connHandle, attReadBlobReq_t* pReq, uint8 taskId );
static bStatus_t gattReadByGrpType( uint16 connHandle, attReadByGrpTypeReq_t* pReq, uint8 taskId );
static bStatus_t gattWrite( uint16 connHandle, attWriteReq_t* pReq, uint8 taskId );
static bStatus_t gattWriteLong( uint16 connHandle, gattPrepareWriteReq_t* pReq, uint8 taskId );

// Callback functions
static bStatus_t gattClientProcessMsgCB( uint16 connHandle,  attPacket_t* pPkt );
static void gattClientHandleTimerCB( uint8* pData );
static void gattClientHandleConnStatusCB( uint16 connectionHandle, uint8 changeType );

/*********************************************************************
    API FUNCTIONS
*/

/*  -------------------------------------------------------------------
    GATT Client Public APIs
*/

/******************************************************************************
    @fn      GATT_InitClient

    @brief   Initialize the Generic Attribute Profile Client.

    @return  SUCCESS: Client initialized successfully.
*/
bStatus_t GATT_InitClient( void )
{
    uint8 i;

    // Mark all records as unused
    for ( i = 0; i < GATT_MAX_NUM_CONN; i++ )
    {
        gattClientInfo_t* pClient = &clientInfoTbl[i];

        // Initialize connection handle
        if ( i == 0 )
        {
            pClient->connHandle = LOOPBACK_CONNHANDLE;
        }
        else
        {
            pClient->connHandle = INVALID_CONNHANDLE;
        }

        // Initialize response info
        pClient->method = 0;
        pClient->taskId = INVALID_TASK_ID;
        pClient->timerId = INVALID_TIMER_ID;
        // Initialize GATT Response message info
        pClient->numRsps = 0;
        // Initialize request info
        VOID osal_memset( &(pClient->req), 0, sizeof( gattMsg_t ) );
    }

    // Set up the client's processing function
    gattRegisterClient( gattClientProcessMsgCB );
    // Register with Link DB to receive link status change callback
    linkDB_Register( gattClientHandleConnStatusCB );
    return ( SUCCESS );
}

/******************************************************************************
    @fn      GATT_RegisterForInd

    @brief   Register to receive incoming ATT Indications or Notifications
            of attribute values.

    @param   taskId - task to forward indications or notifications to

    @return  void
*/
void GATT_RegisterForInd( uint8 taskId )
{
    indTaskId = taskId;
}

/*********************************************************************
    @fn      GATT_PrepareWriteReq

    @brief   The Prepare Write Request is used to request the server to
            prepare to write the value of an attribute.

            Note: This function is needed only for GATT testing.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_PrepareWriteReq( uint16 connHandle, attPrepareWriteReq_t* pReq, uint8 taskId )
{
    uint8 status;

    if ( pReq != NULL )
    {
        gattClientInfo_t* pClient;
        // Make sure we're allowed to send a new request
        status = gattGetClientStatus( connHandle, &pClient );

        if ( status == SUCCESS )
        {
            // Send the request
            status = ATT_PrepareWriteReq( connHandle, pReq );

            if ( status == SUCCESS )
            {
                // Store client info
                gattStoreClientInfo( pClient, NULL, ATT_PREPARE_WRITE_RSP,
                                     ATT_ParsePrepareWriteRsp, taskId );
            }
        }
    }
    else
    {
        status = INVALIDPARAMETER;
    }

    return ( status );
}

/*********************************************************************
    @fn      GATT_ExecuteWriteReq

    @brief   The Execute Write Request is used to request the server to
            write or cancel the write of all the prepared values currently
            held in the prepare queue from this client.

            Note: This function is needed only for GATT testing.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_ExecuteWriteReq( uint16 connHandle, attExecuteWriteReq_t* pReq, uint8 taskId )
{
    uint8 status;

    if ( pReq != NULL )
    {
        gattClientInfo_t* pClient;
        // Make sure we're allowed to send a new request
        status = gattGetClientStatus( connHandle, &pClient );

        if ( status == SUCCESS )
        {
            // Send the request
            status = ATT_ExecuteWriteReq( connHandle, pReq );

            if ( status == SUCCESS )
            {
                // Store client info
                gattStoreClientInfo( pClient, NULL, ATT_EXECUTE_WRITE_RSP,
                                     ATT_ParseExecuteWriteRsp, taskId );
            }
        }
    }
    else
    {
        status = INVALIDPARAMETER;
    }

    return ( status );
}

/*  -------------------------------------------------------------------
    GATT Client Sub-Procedure APIs
*/

/*********************************************************************
    @fn      GATT_ExchangeMTU

    @brief   This sub-procedure is used by the client to set the ATT_MTU
            to the maximum possible value that can be supported by both
            devices when the client supports a value greater than the
            default ATT_MTU for the Attribute Protocol. This sub-procedure
            shall only be initiated once during a connection.

            The ATT Exchange MTU Request is used by this sub-procedure.

            If the return status from this function is SUCCESS, the calling
            application task will receive an OSAL GATT_MSG_EVENT message.
            The type of the message will be either ATT_EXCHANGE_MTU_RSP or
            ATT_ERROR_RSP.

            Note: This sub-procedure is complete when either ATT_EXCHANGE_MTU_RSP
                  (with SUCCESS or bleTimeout status) or ATT_ERROR_RSP (with
                  SUCCESS status) is received by the calling application task.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_ExchangeMTU( uint16 connHandle, attExchangeMTUReq_t* pReq, uint8 taskId )
{
    gattClientInfo_t* pClient;
    uint8 status;
    // Make sure we're allowed to send a new request
    status = gattGetClientStatus( connHandle, &pClient );

    if ( status == SUCCESS )
    {
        // Send the request
        status = ATT_ExchangeMTUReq( connHandle, pReq );

        if ( status == SUCCESS )
        {
            g_attMtuClientServer.clientMTU=pReq->clientRxMTU;
            // Store client info
            gattStoreClientInfo( pClient, (gattMsg_t*)pReq, ATT_EXCHANGE_MTU_RSP,               // update 2020-03-18, the store message follow the style of other function, e.g.gattFindInfo
                                 ATT_ParseExchangeMTURsp, taskId );                             // but note that there is a potential risk read uninitial memory when invoke osal_memcpy in gattStoreClientInfo
        }
    }

    return ( status );
}

/*********************************************************************
    @fn      GATT_DiscAllPrimaryServices

    @brief   This sub-procedure is used by a client to discover all
            the primary services on a server.

            The ATT Read By Group Type Request is used with the Attribute
            Type parameter set to the UUID for "Primary Service". The
            Starting Handle is set to 0x0001 and the Ending Handle is
            set to 0xFFFF.

            If the return status from this function is SUCCESS, the calling
            application task will receive multiple OSAL GATT_MSG_EVENT messages.
            The type of the messages will be either ATT_READ_BY_GRP_TYPE_RSP
            or ATT_ERROR_RSP (if an error occurred on the server).

            Note: This sub-procedure is complete when either ATT_READ_BY_GRP_TYPE_RSP
                  (with bleProcedureComplete or bleTimeout status) or ATT_ERROR_RSP
                  (with SUCCESS status) is received by the calling application
                  task.

    @param   connHandle - connection to use
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_DiscAllPrimaryServices( uint16 connHandle, uint8 taskId )
{
    attReadByGrpTypeReq_t req;
    req.startHandle = GATT_MIN_HANDLE;
    req.endHandle   = GATT_MAX_HANDLE;
    req.type.len = ATT_BT_UUID_SIZE;
    req.type.uuid[0] = LO_UINT16( GATT_PRIMARY_SERVICE_UUID );
    req.type.uuid[1] = HI_UINT16( GATT_PRIMARY_SERVICE_UUID );
    return ( gattReadByGrpType( connHandle, &req,taskId ) );
}

/*********************************************************************
    @fn      GATT_DiscPrimaryServiceByUUID

    @brief   This sub-procedure is used by a client to discover a specific
            primary service on a server when only the Service UUID is
            known. The specific primary service may exist multiple times
            on a server. The primary service being discovered is identified
            by the service UUID.

            The ATT Find By Type Value Request is used with the Attribute
            Type parameter set to the UUID for "Primary Service" and the
            Attribute Value set to the 16-bit Bluetooth UUID or 128-bit
            UUID for the specific primary service. The Starting Handle shall
            be set to 0x0001 and the Ending Handle shall be set to 0xFFFF.

            If the return status from this function is SUCCESS, the calling
            application task will receive multiple OSAL GATT_MSG_EVENT messages.
            The type of the messages will be either ATT_FIND_BY_TYPE_VALUE_RSP
            or ATT_ERROR_RSP (if an error occurred on the server).

            Note: This sub-procedure is complete when either ATT_FIND_BY_TYPE_VALUE_RSP
                  (with bleProcedureComplete or bleTimeout status) or ATT_ERROR_RSP
                  (with SUCCESS status) is received by the calling application task.

    @param   connHandle - connection to use
    @param   pValue - pointer to value to look for
    @param   len - length of value
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_DiscPrimaryServiceByUUID( uint16 connHandle, uint8* pValue,
                                         uint8 len, uint8 taskId )
{
    if ( ( ( len == ATT_BT_UUID_SIZE ) || ( len == ATT_UUID_SIZE ) ) && ( pValue != NULL ) )
    {
        attFindByTypeValueReq_t req;
        req.startHandle = GATT_MIN_HANDLE;
        req.endHandle   = GATT_MAX_HANDLE;
        req.type.len = ATT_BT_UUID_SIZE;
        req.type.uuid[0] = LO_UINT16( GATT_PRIMARY_SERVICE_UUID );
        req.type.uuid[1] = HI_UINT16( GATT_PRIMARY_SERVICE_UUID );
        req.len = len;
        VOID osal_memcpy( req.value, pValue, len );
        return ( gattFindByTypeValue( connHandle, &req, taskId ) );
    }

    return ( INVALIDPARAMETER );
}

/*********************************************************************
    @fn      GATT_FindIncludedServices

    @brief   This sub-procedure is used by a client to find include
            service declarations within a service definition on a
            server. The service specified is identified by the service
            handle range.

            The ATT Read By Type Request is used with the Attribute
            Type parameter set to the UUID for "Included Service". The
            Starting Handle is set to starting handle of the specified
            service and the Ending Handle is set to the ending handle
            of the specified service.

            If the return status from this function is SUCCESS, the calling
            application task will receive multiple OSAL GATT_MSG_EVENT messages.
            The type of the messages will be either ATT_READ_BY_TYPE_RSP
            or ATT_ERROR_RSP (if an error occurred on the server).

            Note: This sub-procedure is complete when either ATT_READ_BY_TYPE_RSP
                  (with bleProcedureComplete or bleTimeout status) or ATT_ERROR_RSP
                  (with SUCCESS status) is received by the calling application task.

    @param   connHandle - connection to use
    @param   startHandle - starting handle
    @param   endHandle - end handle
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_FindIncludedServices( uint16 connHandle, uint16 startHandle,
                                     uint16 endHandle, uint8 taskId )
{
    attReadByTypeReq_t req;
    req.startHandle = startHandle;
    req.endHandle   = endHandle;
    req.type.len = ATT_BT_UUID_SIZE;
    req.type.uuid[0] = LO_UINT16( GATT_INCLUDE_UUID );
    req.type.uuid[1] = HI_UINT16( GATT_INCLUDE_UUID );
    return ( gattReadByType( connHandle, &req, FALSE, taskId ) );
}

/*********************************************************************
    @fn      GATT_DiscAllChars

    @brief   This sub-procedure is used by a client to find all the
            characteristic declarations within a service definition on
            a server when only the service handle range is known. The
            service specified is identified by the service handle range.

            The ATT Read By Type Request is used with the Attribute Type
            parameter set to the UUID for "Characteristic". The Starting
            Handle is set to starting handle of the specified service and
            the Ending Handle is set to the ending handle of the specified
            service.

            If the return status from this function is SUCCESS, the calling
            application task will receive multiple OSAL GATT_MSG_EVENT messages.
            The type of the messages will be either ATT_READ_BY_TYPE_RSP
            or ATT_ERROR_RSP (if an error occurred on the server).

            Note: This sub-procedure is complete when either ATT_READ_BY_TYPE_RSP
                  (with bleProcedureComplete or bleTimeout status) or ATT_ERROR_RSP
                  (with SUCCESS status) is received by the calling application task.

    @param   connHandle - connection to use
    @param   startHandle - starting handle
    @param   endHandle - end handle
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_DiscAllChars( uint16 connHandle, uint16 startHandle,
                             uint16 endHandle, uint8 taskId )
{
    attReadByTypeReq_t req;
    req.startHandle = startHandle;
    req.endHandle   = endHandle;
    req.type.len = ATT_BT_UUID_SIZE;
    req.type.uuid[0] = LO_UINT16( GATT_CHARACTER_UUID );
    req.type.uuid[1] = HI_UINT16( GATT_CHARACTER_UUID );
    return ( gattReadByType( connHandle, &req, FALSE, taskId ) );
}

/*********************************************************************
    @fn      GATT_DiscCharsByUUID

    @brief   This sub-procedure is used by a client to discover service
            characteristics on a server when only the service handle
            ranges are known and the characteristic UUID is known.
            The specific service may exist multiple times on a server.
            The characteristic being discovered is identified by the
            characteristic UUID.

            The ATT Read By Type Request is used with the Attribute Type
            is set to the UUID for "Characteristic" and the Starting
            Handle and Ending Handle parameters is set to the service
            handle range.

            If the return status from this function is SUCCESS, the calling
            application task will receive multiple OSAL GATT_MSG_EVENT messages.
            The type of the messages will be either ATT_READ_BY_TYPE_RSP
            or ATT_ERROR_RSP (if an error occurred on the server).

            Note: This sub-procedure is complete when either ATT_READ_BY_TYPE_RSP
                  (with bleProcedureComplete or bleTimeout status) or ATT_ERROR_RSP
                  (with SUCCESS status) is received by the calling application task.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_DiscCharsByUUID( uint16 connHandle, attReadByTypeReq_t* pReq, uint8 taskId )
{
    return ( gattReadByType( connHandle, pReq, TRUE, taskId ) );
}

/*********************************************************************
    @fn      GATT_DiscAllCharDescs

    @brief   This sub-procedure is used by a client to find all the
            characteristic descriptor�s Attribute Handles and Attribute
            Types within a characteristic definition when only the
            characteristic handle range is known. The characteristic
            specified is identified by the characteristic handle range.

            The ATT Find Information Request is used with the Starting
            Handle set to starting handle of the specified characteristic
            and the Ending Handle set to the ending handle of the specified
            characteristic. The UUID Filter parameter is NULL (zero length).

            If the return status from this function is SUCCESS, the calling
            application task will receive multiple OSAL GATT_MSG_EVENT messages.
            The type of the messages will be either ATT_FIND_INFO_RSP or
            ATT_ERROR_RSP (if an error occurred on the server).

            Note: This sub-procedure is complete when either ATT_FIND_INFO_RSP
                  (with bleProcedureComplete or bleTimeout status) or ATT_ERROR_RSP
                  (with SUCCESS status) is received by the calling application task.

    @param   connHandle - connection to use
    @param   startHandle - starting handle
    @param   endHandle - end handle
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_DiscAllCharDescs( uint16 connHandle, uint16 startHandle,
                                 uint16 endHandle, uint8 taskId )
{
    attFindInfoReq_t req;
    req.startHandle = startHandle;
    req.endHandle   = endHandle;
    return ( gattFindInfo( connHandle, &req, taskId ) );
}

/*********************************************************************
    @fn      GATT_ReadCharValue

    @brief   This sub-procedure is used to read a Characteristic Value
            from a server when the client knows the Characteristic Value
            Handle. The ATT Read Request is used with the Attribute Handle
            parameter set to the Characteristic Value Handle. The Read
            Response returns the Characteristic Value in the Attribute
            Value parameter.

            The Read Response only contains a Characteristic Value that
            is less than or equal to (ATT_MTU-1) octets in length. If
            the Characteristic Value is greater than (ATT_MTU-1) octets
            in length, the Read Long Characteristic Value procedure may
            be used if the rest of the Characteristic Value is required.

            If the return status from this function is SUCCESS, the calling
            application task will receive an OSAL GATT_MSG_EVENT message.
            The type of the message will be either ATT_READ_RSP or
            ATT_ERROR_RSP (if an error occurred on the server).

            Note: This sub-procedure is complete when either ATT_READ_RSP
                  (with SUCCESS or bleTimeout status) or ATT_ERROR_RSP (with
                  SUCCESS status) is received by the calling application task.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_ReadCharValue( uint16 connHandle, attReadReq_t* pReq, uint8 taskId )
{
    return ( gattRead( connHandle, pReq, taskId ) );
}

/*********************************************************************
    @fn      GATT_ReadUsingCharUUID

    @brief   This sub-procedure is used to read a Characteristic Value
            from a server when the client only knows the characteristic
            UUID and does not know the handle of the characteristic.

            The ATT Read By Type Request is used to perform the sub-procedure.
            The Attribute Type is set to the known characteristic UUID and
            the Starting Handle and Ending Handle parameters shall be set
            to the range over which this read is to be performed. This is
            typically the handle range for the service in which the
            characteristic belongs.

            If the return status from this function is SUCCESS, the calling
            application task will receive an OSAL GATT_MSG_EVENT messages.
            The type of the message will be either ATT_READ_BY_TYPE_RSP
            or ATT_ERROR_RSP (if an error occurred on the server).

            Note: This sub-procedure is complete when either ATT_READ_BY_TYPE_RSP
                  (with SUCCESS or bleTimeout status) or ATT_ERROR_RSP (with
                  SUCCESS status) is received by the calling application task.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_ReadUsingCharUUID( uint16 connHandle, attReadByTypeReq_t* pReq, uint8 taskId )
{
    return ( gattReadByType( connHandle, pReq, FALSE, taskId ) );
}

/*********************************************************************
    @fn      GATT_ReadLongCharValue

    @brief   This sub-procedure is used to read a Characteristic Value from
            a server when the client knows the Characteristic Value Handle
            and the length of the Characteristic Value is longer than can
            be sent in a single Read Response Attribute Protocol message.

            The ATT Read Blob Request is used in this sub-procedure.

            If the return status from this function is SUCCESS, the calling
            application task will receive multiple OSAL GATT_MSG_EVENT messages.
            The type of the messages will be either ATT_READ_BLOB_RSP or
            ATT_ERROR_RSP (if an error occurred on the server).

            Note: This sub-procedure is complete when either ATT_READ_BLOB_RSP
                  (with bleProcedureComplete or bleTimeout status) or ATT_ERROR_RSP
                  (with SUCCESS status) is received by the calling application task.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_ReadLongCharValue( uint16 connHandle, attReadBlobReq_t* pReq, uint8 taskId )
{
    return ( gattReadLong( connHandle, pReq, taskId ) );
}

/*********************************************************************
    @fn      GATT_ReadMultiCharValues

    @brief   This sub-procedure is used to read multiple Characteristic Values
            from a server when the client knows the Characteristic Value
            Handles. The Attribute Protocol Read Multiple Requests is used
            with the Set Of Handles parameter set to the Characteristic Value
            Handles. The Read Multiple Response returns the Characteristic
            Values in the Set Of Values parameter.

            The ATT Read Multiple Request is used in this sub-procedure.

            If the return status from this function is SUCCESS, the calling
            application task will receive an OSAL GATT_MSG_EVENT message.
            The type of the message will be either ATT_READ_MULTI_RSP
            or ATT_ERROR_RSP (if an error occurred on the server).

            Note: This sub-procedure is complete when either ATT_READ_MULTI_RSP
                  (with SUCCESS or bleTimeout status) or ATT_ERROR_RSP (with
                  SUCCESS status) is received by the calling application task.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_ReadMultiCharValues( uint16 connHandle, attReadMultiReq_t* pReq, uint8 taskId )
{
    gattClientInfo_t* pClient;
    uint8 status;
    // Make sure we're allowed to send a new request
    status = gattGetClientStatus( connHandle, &pClient );

    if ( status == SUCCESS )
    {
        // Send the request
        status = ATT_ReadMultiReq( connHandle, pReq );

        if ( status == SUCCESS )
        {
            // Store client info
            gattStoreClientInfo( pClient, NULL, ATT_READ_MULTI_RSP,
                                 ATT_ParseReadMultiRsp, taskId );
        }
    }

    return ( status );
}

/*********************************************************************
    @fn      GATT_WriteNoRsp

    @brief   This sub-procedure is used to write a Characteristic Value
            to a server when the client knows the Characteristic Value
            Handle and the client does not need an acknowledgement that
            the write was successfully performed. This sub-procedure
            only writes the first (ATT_MTU-3) octets of a Characteristic
            Value. This sub-procedure can not be used to write a long
            characteristic; instead the Write Long Characteristic Values
            sub-procedure should be used.

            The ATT Write Command is used for this sub-procedure. The
            Attribute Handle parameter shall be set to the Characteristic
            Value Handle. The Attribute Value parameter shall be set to
            the new Characteristic Value.

            No response will be sent to the calling application task for this
            sub-procedure. If the Characteristic Value write request is the
            wrong size, or has an invalid value as defined by the profile,
            then the write will not succeed and no error will be generated
            by the server.

    @param   connHandle - connection to use
    @param   pReq - pointer to command to be sent

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_WriteNoRsp( uint16 connHandle, attWriteReq_t* pReq )
{
    gattClientInfo_t* pClient;
    uint8 status;
    // Make sure we're allowed to send a new request
    status = gattGetClientStatus( connHandle, &pClient );

    if ( status != bleTimeout )
    {
        if ( ( pReq->sig == FALSE ) && ( pReq->cmd == TRUE ) )
        {
            status = ATT_WriteReq( connHandle, pReq );
        }
        else
        {
            status = INVALIDPARAMETER;
        }
    }

    return ( status );
}

/*********************************************************************
    @fn      GATT_SignedWriteNoRsp

    @brief   This sub-procedure is used to write a Characteristic Value
            to a server when the client knows the Characteristic Value
            Handle and the ATT Bearer is not encrypted. This sub-procedure
            shall only be used if the Characteristic Properties authenticated
            bit is enabled and the client and server device share a bond as
            defined in the GAP.

            This sub-procedure only writes the first (ATT_MTU-15) octets
            of an Attribute Value. This sub-procedure cannot be used to
            write a long Attribute.

            The ATT Write Command is used for this sub-procedure. The
            Attribute Handle parameter shall be set to the Characteristic
            Value Handle. The Attribute Value parameter shall be set to
            the new Characteristic Value authenticated by signing the
            value, as defined in the Security Manager.

            No response will be sent to the calling application task for this
            sub-procedure. If the authenticated Characteristic Value that is
            written is the wrong size, or has an invalid value as defined by
            the profile, or the signed value does not authenticate the client,
            then the write will not succeed and no error will be generated by
            the server.

    @param   connHandle - connection to use
    @param   pReq - pointer to command to be sent

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            bleMemAllocError: Memory allocation error occurred.
            bleLinkEncrypted: Connection is already encrypted.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_SignedWriteNoRsp( uint16 connHandle, attWriteReq_t* pReq )
{
    gattClientInfo_t* pClient;
    uint8 status;
    // Make sure we're allowed to send a new request
    status = gattGetClientStatus( connHandle, &pClient );

    if ( status != bleTimeout )
    {
        if ( ( pReq->sig == TRUE ) && ( pReq->cmd == TRUE ) )
        {
            status = ATT_WriteReq( connHandle, pReq );
        }
        else
        {
            status = INVALIDPARAMETER;
        }
    }

    return ( status );
}

/*********************************************************************
    @fn      GATT_WriteCharValue

    @brief   This sub-procedure is used to write a characteristic value
            to a server when the client knows the characteristic value
            handle. This sub-procedure only writes the first (ATT_MTU-3)
            octets of a characteristic value. This sub-procedure can not
            be used to write a long attribute; instead the Write Long
            Characteristic Values sub-procedure should be used.

            The ATT Write Request is used in this sub-procedure. The
            Attribute Handle parameter shall be set to the Characteristic
            Value Handle. The Attribute Value parameter shall be set to
            the new characteristic.

            If the return status from this function is SUCCESS, the calling
            application task will receive an OSAL GATT_MSG_EVENT message.
            The type of the message will be either ATT_WRITE_RSP
            or ATT_ERROR_RSP (if an error occurred on the server).

            Note: This sub-procedure is complete when either ATT_WRITE_RSP
                  (with SUCCESS or bleTimeout status) or ATT_ERROR_RSP (with
                  SUCCESS status) is received by the calling application task.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_WriteCharValue( uint16 connHandle, attWriteReq_t* pReq, uint8 taskId )
{
    return ( gattWrite( (connHandle), (pReq), (taskId) ) );
}

/*********************************************************************
    @fn      GATT_WriteLongCharValue

    @brief   This sub-procedure is used to write a Characteristic Value to
            a server when the client knows the Characteristic Value Handle
            but the length of the Characteristic Value is longer than can
            be sent in a single Write Request Attribute Protocol message.

            The ATT Prepare Write Request and Execute Write Request are
            used to perform this sub-procedure.

            If the return status from this function is SUCCESS, the calling
            application task will receive multiple OSAL GATT_MSG_EVENT messages.
            The type of the messages will be either ATT_PREPARE_WRITE_RSP,
            ATT_EXECUTE_WRITE_RSP or ATT_ERROR_RSP (if an error occurred on
            the server).

            Note: This sub-procedure is complete when either ATT_PREPARE_WRITE_RSP
                  (with bleTimeout status), ATT_EXECUTE_WRITE_RSP (with SUCCESS
                  or bleTimeout status), or ATT_ERROR_RSP (with SUCCESS status)
                  is received by the calling application task.

            Note: The 'pReq->pValue' pointer will be freed when the sub-procedure
                  is complete.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_WriteLongCharValue( uint16 connHandle, gattPrepareWriteReq_t* pReq, uint8 taskId )
{
    return ( gattWriteLong( connHandle, pReq, taskId ) );
}

/*********************************************************************
    @fn      GATT_ReliableWrites

    @brief   This sub-procedure is used to write a Characteristic Value to
            a server when the client knows the Characteristic Value Handle,
            and assurance is required that the correct Characteristic Value
            is going to be written by transferring the Characteristic Value
            to be written in both directions before the write is performed.
            This sub-procedure can also be used when multiple values must
            be written, in order, in a single operation.

            The sub-procedure has two phases, the first phase prepares the
            characteristic values to be written. Once this is complete,
            the second phase performs the execution of all of the prepared
            characteristic value writes on the server from this client.

            In the first phase, the ATT Prepare Write Request is used.
            In the second phase, the attribute protocol Execute Write
            Request is used.

            If the return status from this function is SUCCESS, the calling
            application task will receive multiple OSAL GATT_MSG_EVENT messages.
            The type of the messages will be either ATT_PREPARE_WRITE_RSP,
            ATT_EXECUTE_WRITE_RSP or ATT_ERROR_RSP (if an error occurred on
            the server).

            Note: This sub-procedure is complete when either ATT_PREPARE_WRITE_RSP
                  (with bleTimeout status), ATT_EXECUTE_WRITE_RSP (with SUCCESS
                  or bleTimeout status), or ATT_ERROR_RSP (with SUCCESS status)
                  is received by the calling application task.

            Note: The 'pReqs' pointer will be freed when the sub-procedure is
                  complete.

    @param   connHandle - connection to use
    @param   pReqs - pointer to requests to be sent (must be allocated)
    @param   numReqs - number of requests in pReq
    @param   flags - execute write request flags
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_ReliableWrites( uint16 connHandle, attPrepareWriteReq_t* pReqs,
                               uint8 numReqs, uint8 flags, uint8 taskId )
{
    uint8 status;

    if ( ( pReqs != NULL ) && ( numReqs > 0 ) )
    {
        gattClientInfo_t* pClient;
        // Make sure we're allowed to send a new request
        status = gattGetClientStatus( connHandle, &pClient );

        if ( status == SUCCESS )
        {
            // Send the first request
            status = ATT_PrepareWriteReq( connHandle, &(pReqs[0]) );

            if ( status == SUCCESS )
            {
                gattReliableWritesReq_t reliableWritesReq;
                // Save the number Prepare Write Requests to be sent
                reliableWritesReq.numReqs = numReqs;
                // Save the Prepare Write Requests to be sent
                reliableWritesReq.pReqs = pReqs;
                // Save the index of the last Prepare Write Request sent
                reliableWritesReq.index = 0;
                // Set Execute Write Request flags
                reliableWritesReq.flags = flags;
                // This is a reliable write request
                reliableWritesReq.reliable = TRUE;
                // Store client info
                gattStoreClientInfo( pClient, (gattMsg_t*)&reliableWritesReq,
                                     ATT_PREPARE_WRITE_RSP, ATT_ParsePrepareWriteRsp, taskId );
            }
        }
    }
    else
    {
        status = INVALIDPARAMETER;
    }

    return ( status );
}

/*********************************************************************
    @fn      GATT_ReadCharDesc

    @brief   This sub-procedure is used to read a characteristic descriptor
            from a server when the client knows the characteristic descriptor
            declaration�s Attribute handle.

            The ATT Read Request is used for this sub-procedure. The Read
            Request is used with the Attribute Handle parameter set to the
            characteristic descriptor handle. The Read Response returns the
            characteristic descriptor value in the Attribute Value parameter.

            If the return status from this function is SUCCESS, the calling
            application task will receive an OSAL GATT_MSG_EVENT message.
            The type of the message will be either ATT_READ_RSP or
            ATT_ERROR_RSP (if an error occurred on the server).

            Note: This sub-procedure is complete when either ATT_READ_RSP
                  (with SUCCESS or bleTimeout status) or ATT_ERROR_RSP (with
                  SUCCESS status) is received by the calling application task.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_ReadCharDesc( uint16 connHandle, attReadReq_t* pReq, uint8 taskId )
{
    return ( gattRead( connHandle, pReq, taskId ) );
}

/*********************************************************************
    @fn      GATT_ReadLongCharDesc

    @brief   This sub-procedure is used to read a characteristic descriptor
            from a server when the client knows the characteristic descriptor
            declaration�s Attribute handle and the length of the characteristic
            descriptor declaration is longer than can be sent in a single Read
            Response attribute protocol message.

            The ATT Read Blob Request is used to perform this sub-procedure.
            The Attribute Handle parameter shall be set to the characteristic
            descriptor handle. The Value Offset parameter shall be the offset
            within the characteristic descriptor to be read.

            If the return status from this function is SUCCESS, the calling
            application task will receive multiple OSAL GATT_MSG_EVENT messages.
            The type of the messages will be either ATT_READ_BLOB_RSP or
            ATT_ERROR_RSP (if an error occurred on the server).

            Note: This sub-procedure is complete when either ATT_READ_BLOB_RSP
                  (with bleProcedureComplete or bleTimeout status) or ATT_ERROR_RSP
                  (with SUCCESS status) is received by the calling application task.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_ReadLongCharDesc( uint16 connHandle, attReadBlobReq_t* pReq, uint8 taskId )
{
    return ( gattReadLong( connHandle, pReq, taskId ) );
}

/*********************************************************************
    @fn      GATT_WriteCharDesc

    @brief   This sub-procedure is used to write a characteristic
            descriptor value to a server when the client knows the
            characteristic descriptor handle.

            The ATT Write Request is used for this sub-procedure. The
            Attribute Handle parameter shall be set to the characteristic
            descriptor handle. The Attribute Value parameter shall be
            set to the new characteristic descriptor value.

            If the return status from this function is SUCCESS, the calling
            application task will receive an OSAL GATT_MSG_EVENT message.
            The type of the message will be either ATT_WRITE_RSP
            or ATT_ERROR_RSP (if an error occurred on the server).

            Note: This sub-procedure is complete when either ATT_WRITE_RSP
                  (with SUCCESS or bleTimeout status) or ATT_ERROR_RSP (with
                  SUCCESS status) is received by the calling application task.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_WriteCharDesc( uint16 connHandle, attWriteReq_t* pReq, uint8 taskId )
{
    return ( gattWrite( connHandle, pReq, taskId ) );
}

/*********************************************************************
    @fn      GATT_WriteLongCharDesc

    @brief   This sub-procedure is used to write a Characteristic Value to
            a server when the client knows the Characteristic Value Handle
            but the length of the Characteristic Value is longer than can
            be sent in a single Write Request Attribute Protocol message.

            The ATT Prepare Write Request and Execute Write Request are
            used to perform this sub-procedure.

            If the return status from this function is SUCCESS, the calling
            application task will receive multiple OSAL GATT_MSG_EVENT messages.
            The type of the messages will be either ATT_PREPARE_WRITE_RSP,
            ATT_EXECUTE_WRITE_RSP or ATT_ERROR_RSP (if an error occurred on
            the server).

            Note: This sub-procedure is complete when either ATT_PREPARE_WRITE_RSP
                  (with bleTimeout status), ATT_EXECUTE_WRITE_RSP (with SUCCESS
                  or bleTimeout status), or ATT_ERROR_RSP (with SUCCESS status)
                  is received by the calling application task.

            Note: The 'pReq->pValue' pointer will be freed when the sub-procedure
                  is complete.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
bStatus_t GATT_WriteLongCharDesc( uint16 connHandle, gattPrepareWriteReq_t* pReq, uint8 taskId )
{
    return ( gattWriteLong( connHandle, pReq, taskId ) );
}

/*  -------------------------------------------------------------------
    GATT Client Internal Functions
*/

/*********************************************************************
    @fn      gattFindInfo

    @brief   The Find Information Request is used to obtain the mapping
            of attribute handles with their associated types. This
            allows a client to discover the list of attributes and
            their types on a server.

            Only attributes with attribute handles between and including
            the Starting Handle parameter and the Ending Handle parameter
            will be returned. To read all attributes, the Starting Handle
            parameter shall be set to 0x0001, and the Ending Handle
            parameter shall be set to 0xFFFF.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
static bStatus_t gattFindInfo( uint16 connHandle, attFindInfoReq_t* pReq, uint8 taskId )
{
    uint8 status;
    gattClientInfo_t* pClient;
    // Make sure we're allowed to send a new request
    status = gattGetClientStatus( connHandle, &pClient );

    if ( status == SUCCESS )
    {
        // Send the request
        status = ATT_FindInfoReq( connHandle, pReq );

        if ( status == SUCCESS )
        {
            // Store client info
            gattStoreClientInfo( pClient, (gattMsg_t*)pReq, ATT_FIND_INFO_RSP,
                                 ATT_ParseFindInfoRsp, taskId );
        }
    }

    return ( status );
}

/*********************************************************************
    @fn      gattFindByTypeValue

    @brief   The Find By Type Value Request is used to obtain the
            handles of attributes that have a 16-bit UUID attribute
            type and attribute value. This allows the range of handles
            associated with a given attribute to be discovered when
            the attribute type determines the grouping of a set of
            attributes.

            Note: Generic Attribute Profile defines grouping of
                  attributes by attribute type.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
static bStatus_t gattFindByTypeValue( uint16 connHandle, attFindByTypeValueReq_t* pReq,
                                      uint8 taskId )
{
    uint8 status;
    gattClientInfo_t* pClient;
    // Make sure we're allowed to send a new request
    status = gattGetClientStatus( connHandle, &pClient );

    if ( status == SUCCESS )
    {
        // Send the request
        status = ATT_FindByTypeValueReq( connHandle, pReq );

        if ( status == SUCCESS )
        {
            // Store client info
            gattStoreClientInfo( pClient, (gattMsg_t*)pReq, ATT_FIND_BY_TYPE_VALUE_RSP,
                                 ATT_ParseFindByTypeValueRsp, taskId );
        }
    }

    return ( status );
}

/*********************************************************************
    @fn      gattReadByType

    @brief   The Read By Type Request is used to obtain the values
            of attributes where the attribute type is known but the
            handle is not known.

            Only the attributes with attribute handles between and
            including the Starting Handle and the Ending Handle with
            the attribute type that is the same as the Attribute Type
            given will be returned. To search through all attributes,
            the starting handle shall be set to 0x0001 and the ending
            handle shall be set to 0xFFFF.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   discCharsByUUID - whether it's discover characteristics by UUID
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
static bStatus_t gattReadByType( uint16 connHandle, attReadByTypeReq_t* pReq,
                                 uint8 discCharsByUUID, uint8 taskId )
{
    uint8 status;
    gattClientInfo_t* pClient;
    // Make sure we're allowed to send a new request
    status = gattGetClientStatus( connHandle, &pClient );

    if ( status == SUCCESS )
    {
        // Send the request
        if ( discCharsByUUID )
        {
            attReadByTypeReq_t req;
            // This is Discover Characteristic by UUID
            req.startHandle = pReq->startHandle;
            req.endHandle = pReq->endHandle;
            // Set Type to Characteristic UUID
            req.type.len = ATT_BT_UUID_SIZE;
            req.type.uuid[0] = LO_UINT16( GATT_CHARACTER_UUID );
            req.type.uuid[1] = HI_UINT16( GATT_CHARACTER_UUID );
            status = ATT_ReadByTypeReq( connHandle, &req );
        }
        else
        {
            status = ATT_ReadByTypeReq( connHandle, pReq );
        }

        if ( status == SUCCESS )
        {
            gattReadByTypeReq_t req;
            VOID osal_memcpy( &(req.req), pReq, sizeof( attReadByTypeReq_t ) );
            req.discCharsByUUID = discCharsByUUID;
            // Store client info
            gattStoreClientInfo( pClient, (gattMsg_t*)&req, ATT_READ_BY_TYPE_RSP,
                                 ATT_ParseReadByTypeRsp, taskId );
        }
    }

    return ( status );
}

/*********************************************************************
    @fn      gattRead

    @brief   The Read Request is used to request the server to read
            the value of an attribute and return its value in a
            Read Response.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
static bStatus_t gattRead( uint16 connHandle, attReadReq_t* pReq, uint8 taskId )
{
    gattClientInfo_t* pClient;
    uint8 status;
    // Make sure we're allowed to send a new request
    status = gattGetClientStatus( connHandle, &pClient );

    if ( status == SUCCESS )
    {
        // Send the request
        status = ATT_ReadReq( connHandle, pReq );

        if ( status == SUCCESS )
        {
            // Store client info
            gattStoreClientInfo( pClient, NULL, ATT_READ_RSP, ATT_ParseReadRsp, taskId );
        }
    }

    return ( status );
}

/*********************************************************************
    @fn      gattReadLong

    @brief   This sub-procedure shall be used to read the value of an
            attribute from a server when the client knows the attribute
            handle and the length of the value of this attribute may be
            longer than can be communicated in a single Read Response
            attribute protocol message.

            The ATT Read Blob Request is used in this sub-procedure.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
static bStatus_t gattReadLong( uint16 connHandle, attReadBlobReq_t* pReq, uint8 taskId )
{
    uint8 status;
    gattClientInfo_t* pClient;
    // Make sure we're allowed to send a new request
    status = gattGetClientStatus( connHandle, &pClient );

    if ( status == SUCCESS )
    {
        // Send the request
        status = ATT_ReadBlobReq( connHandle, pReq );

        if ( status == SUCCESS )
        {
            // Store client info
            gattStoreClientInfo( pClient, (gattMsg_t*)pReq, ATT_READ_BLOB_RSP,
                                 ATT_ParseReadBlobRsp, taskId );
        }
    }

    return ( status );
}

/*********************************************************************
    @fn      gattReadByGrpType

    @brief   The Read By Group Type Request is used to obtain the values
            of attributes where the attribute type is known, the type
            of a grouping attribute as defined by a higher layer
            specification, but the handle is not known.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
static bStatus_t gattReadByGrpType( uint16 connHandle, attReadByGrpTypeReq_t* pReq, uint8 taskId )
{
    uint8 status;
    gattClientInfo_t* pClient;
    // Make sure we're allowed to send a new request
    status = gattGetClientStatus( connHandle, &pClient );

    if ( status == SUCCESS )
    {
        // Send the request
        status = ATT_ReadByGrpTypeReq( connHandle, pReq );

        if ( status == SUCCESS )
        {
            // Store client info
            gattStoreClientInfo( pClient, (gattMsg_t*)pReq, ATT_READ_BY_GRP_TYPE_RSP,
                                 ATT_ParseReadByGrpTypeRsp, taskId );
        }
    }

    return ( status );
}

/*********************************************************************
    @fn      gattWrite

    @brief   The Write Request is used to request the server to write
            the value of an attribute and acknowledge that this has
            been achieved in a Write Response.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
static bStatus_t gattWrite( uint16 connHandle, attWriteReq_t* pReq, uint8 taskId )
{
    uint8 status;

    if ( ( pReq->sig == FALSE ) && ( pReq->cmd == FALSE ) )
    {
        gattClientInfo_t* pClient;
        // Make sure we're allowed to send a new request
        status = gattGetClientStatus( connHandle, &pClient );

        if ( status == SUCCESS )
        {
            // Send the request
            status = ATT_WriteReq( connHandle, pReq );

            if ( status == SUCCESS )
            {
                // Store client info
                gattStoreClientInfo( pClient, NULL, ATT_WRITE_RSP,
                                     ATT_ParseWriteRsp, taskId );
            }
        }
    }
    else
    {
        status = INVALIDPARAMETER;
    }

    return ( status );
}

/*********************************************************************
    @fn      gattWriteLong

    @brief   This sub-procedure shall be used to write a characteristic
            value to a server when the client knows the characteristic
            value handle but the length of the characteristic value is
            longer than can be sent in a single Write Request attribute
            protocol message.

            The ATT Prepare Write Request and Execute Write Request are
            used to perform this sub-procedure.

    @param   connHandle - connection to use
    @param   pReq - pointer to request to be sent
    @param   taskId - task to be notified of response

    @return  SUCCESS: Request was sent successfully.
            INVALIDPARAMETER: Invalid connection handle or request field.
            MSG_BUFFER_NOT_AVAIL: No HCI buffer is available.
            bleNotConnected: Connection is down.
            blePending: A response is pending with this server.
            bleMemAllocError: Memory allocation error occurred.
            bleTimeout: Previous transaction timed out.
*/
static bStatus_t gattWriteLong( uint16 connHandle, gattPrepareWriteReq_t* pReq, uint8 taskId )
{
    uint8 status;
    gattClientInfo_t* pClient;
    // Make sure we're allowed to send a new request
    status = gattGetClientStatus( connHandle, &pClient );

    if ( status == SUCCESS )
    {
        attPrepareWriteReq_t req;
        // Build Prepare Write Request message
        req.handle = pReq->handle;
        req.offset = pReq->offset;

        if ( pReq->len > ( gAttMtuSize[connHandle] - 5 ) )
        {
            req.len = gAttMtuSize[connHandle] - 5;
        }
        else
        {
            // Last part of the attribute value
            req.len = pReq->len;
        }

        VOID osal_memcpy( req.value, pReq->pValue, req.len );
        // Send the request
        status = ATT_PrepareWriteReq( connHandle, &req );

        if ( status == SUCCESS )
        {
            gattWriteLongReq_t writeLongReq;
            // This is not a reliable write request
            writeLongReq.reliable = FALSE;
            // Save the original request
            VOID osal_memcpy( &(writeLongReq.req), pReq, sizeof( gattPrepareWriteReq_t ) );
            // Save the offset just sent
            writeLongReq.lastOffset = pReq->offset;
            // Store client info
            gattStoreClientInfo( pClient, (gattMsg_t*)&writeLongReq, ATT_PREPARE_WRITE_RSP,
                                 ATT_ParsePrepareWriteRsp, taskId );
        }
    }

    return ( status );
}

/*********************************************************************
    @fn          gattProcessClientMsgCB

    @brief       GATT Client message processing function.

    @param       connHandle - connection packet was received on
    @param       pPkt - pointer to received packet

    @return      SUCCESS: Message processed successfully
                ATT_ERR_UNSUPPORTED_REQ: Unsupported message
                ATT_ERR_INVALID_PDU: Invalid PDU
                bleMemAllocError: Memory allocation error occurred
*/
static bStatus_t gattClientProcessMsgCB( uint16 connHandle, attPacket_t* pPkt )
{
    gattMsg_t msg;
    gattClientInfo_t* pClient;
    uint8 status;

    // Make sure the incoming message is supported
    if ( ( pPkt->sig != ATT_SIG_NOT_INCLUDED ) || ( pPkt->cmd == TRUE ) )
    {
        // Unsupported message
        return ( ATT_ERR_UNSUPPORTED_REQ );
    }

    // See if this is an indication or notification
    if ( ( pPkt->method == ATT_HANDLE_VALUE_NOTI ) || ( pPkt->method == ATT_HANDLE_VALUE_IND ) )
    {
        if ( indTaskId == INVALID_TASK_ID )
        {
            // Application hasn't registered for it!
            return ( ATT_ERR_UNSUPPORTED_REQ );
        }

        // Parse indication
        status = ATT_ParseHandleValueInd( pPkt->sig, pPkt->cmd, pPkt->pParams, pPkt->len, (attMsg_t*)&msg );

        if ( status == SUCCESS )
        {
            // Forward the message to upper layer application
            status = gattNotifyEvent( indTaskId, connHandle, SUCCESS, pPkt->method, &msg );
        }

        // We're done here
        return ( status );
    }

    // Make sure we have the info about the Client that initiated the request
    pClient = gattFindClientInfo( connHandle );

    if ( pClient == NULL )
    {
        // Request must have timed out
        return ( SUCCESS );
    }

    // Parse the message
    if ( pPkt->method == ATT_ERROR_RSP )
    {
        status = ATT_ParseErrorRsp( pPkt->pParams, pPkt->len, (attMsg_t*)&msg );
    }
    else
    {
        if ( ( pPkt->method == pClient->method ) && ( pClient->pfnParseRsp != NULL ) )
        {
            status = (*pClient->pfnParseRsp)( pPkt->pParams, pPkt->len, (attMsg_t*)&msg );
        }
        else
        {
            // We're not expecting this message!
            status = ATT_ERR_INVALID_PDU;
        }
    }

    // Try to process the response
    if ( status == SUCCESS )
    {
        // See if this is a response to a GATT sub-procedure with multiple requests
        if ( ( pClient->method == ATT_FIND_INFO_RSP )          ||
                ( pClient->method == ATT_FIND_BY_TYPE_VALUE_RSP ) ||
                ( pClient->method == ATT_READ_BY_TYPE_RSP )       ||
                ( pClient->method == ATT_READ_BLOB_RSP )          ||
                ( pClient->method == ATT_READ_BY_GRP_TYPE_RSP )   ||
                ( ( pClient->method == ATT_PREPARE_WRITE_RSP )    &&
                  ( pClient->req.gattReliableWritesReq.pReqs != NULL ) ) ) // needed for GATT testing
        {
            gattProcessMultiReqs( connHandle, pClient, pPkt->method, &msg );
        }
        else if ( ( pClient->method == ATT_EXCHANGE_MTU_RSP )  )       // add 2020-03-18
        {
//        ATT_MTU_SIZE_UPDATE(MIN(msg.exchangeMTURsp.serverRxMTU, pClient->req.exchangeMTUReq.clientRxMTU));
            ATT_UpdateMtuSize(connHandle, MIN(msg.exchangeMTURsp.serverRxMTU, pClient->req.exchangeMTUReq.clientRxMTU));
            // Forward the message to upper layer application
            status = gattNotifyEvent( pClient->taskId, connHandle, SUCCESS, pPkt->method, &msg );
            // Reset client info
            gattResetClientInfo( pClient );
        }
        else
        {
            // Forward the message to upper layer application
            status = gattNotifyEvent( pClient->taskId, connHandle, SUCCESS, pPkt->method, &msg );
            // Reset client info
            gattResetClientInfo( pClient );
        }
    }

    return ( status );
}

/*********************************************************************
    @fn          gattProcessMultiReqs

    @brief       Process a GATT sub-procedure that could generate multiple
                request messages.

                The received response is forwarded up to the application,
                or/and the sub-procedure complete is sent to the application
                depending on the status returned from the process function:

                         Forward Rsp    Send Complete    Reset Client Info
                         ===========    =============    =================
                Pending      Yes*            No                 No

                Failure      Yes             No                 Yes

                Success      Yes**           Yes                Yes

 *              *  = Except Prepare Write Response
 *              ** = Except Error Response that indicates end of sub-procedure

    @param       connHandle - connection event belongs to
    @param       pClient - pointer to client info
    @param       method - type of message
    @param       pMsg - pointer to received message

    @return      none
*/
static void gattProcessMultiReqs( uint16 connHandle, gattClientInfo_t* pClient,
                                  uint8 method, gattMsg_t* pMsg )
{
    uint8 status;

    switch ( pClient->method )
    {
    case ATT_FIND_INFO_RSP:
        status = gattProcessFindInfo( pClient, method, pMsg );
        break;

    case ATT_FIND_BY_TYPE_VALUE_RSP:
        status = gattProcessFindByTypeValue( pClient, method, pMsg );
        break;

    case ATT_READ_BY_TYPE_RSP:
        status = gattProcessReadByType( pClient, method, pMsg );
        break;

    case ATT_READ_BLOB_RSP:
        status = gattProcessReadLong( pClient, method, pMsg );
        break;

    case ATT_READ_BY_GRP_TYPE_RSP:
        status = gattProcessReadByGrpType( pClient, method, pMsg );
        break;

    case ATT_PREPARE_WRITE_RSP:
        if ( pClient->req.gattReliableWritesReq.reliable == TRUE )
        {
            status = gattProcessReliableWrites( pClient, method, pMsg );
        }
        else
        {
            status = gattProcessWriteLong( pClient, method, pMsg );
        }

        if ( status != FAILURE )
        {
            // Must be an intermediate Prepare Write Response so no need to forward it
            return;
        }

        break;

    default:
        // Should never get here!
        status = FAILURE;
        break;
    }

    // Do not forward an Error Response that indicates the end of a sub-procedure
    if ( ( status != SUCCESS )       ||
            ( method != ATT_ERROR_RSP ) ||
            ( pMsg->errorRsp.errCode != ATT_ERR_ATTR_NOT_FOUND ) )
    {
        // Make sure there's data to be forwarded
        if ( ( method != ATT_READ_BY_TYPE_RSP ) || ( pMsg->readByTypeRsp.numPairs > 0 ) )
        {
            // Forward the message to upper layer application
            VOID gattNotifyEvent( pClient->taskId, connHandle, SUCCESS, method, pMsg );
        }
    }

    if ( status == SUCCESS )
    {
        // Indicate sub-procedure completion to upper layer application
        VOID gattNotifyEvent( pClient->taskId, connHandle, bleProcedureComplete, pClient->method, NULL );
    }

    if ( status != blePending )
    {
        // Reset client info
        gattResetClientInfo( pClient );
    }
}

/*********************************************************************
    @fn          gattProcessFindInfo

    @brief       Process a Find Information message.

    @param       pClient - pointer to client info
    @param       method - type of message
    @param       pMsg - pointer to received

    @return      SUCCESS: Sub-procedure completed
                FAILURE: Sub-procedure failed
                blePending: Sub-procedure still in progress
*/
static uint8 gattProcessFindInfo( gattClientInfo_t* pClient,
                                  uint8 method, gattMsg_t* pMsg )
{
    // Response to Find Info Request
    if ( method == ATT_FIND_INFO_RSP )
    {
        attFindInfoRsp_t* pRsp = &(pMsg->findInfoRsp); // received response
        attFindInfoReq_t* pReq = &(pClient->req.findInfoReq); // original request
        uint16 endHandle;
        pClient->numRsps++; // Increment number of responses

        // Find out the end handle
        if ( pRsp->format == ATT_HANDLE_BT_UUID_TYPE )
        {
            endHandle = pRsp->info.btPair[pRsp->numInfo-1].handle;
        }
        else // ATT_HANDLE_UUID_TYPE
        {
            endHandle = pRsp->info.pair[pRsp->numInfo-1].handle;
        }

        // The sub-procedure is complete when the Find Information Response has
        // an Attribute Handle that is equal to the Ending Handle of the request.
        if ( endHandle < pReq->endHandle )
        {
            // Update the start handle
            pReq->startHandle = endHandle + 1;
            // Send another Find Info Request
            VOID ATT_FindInfoReq( pClient->connHandle, pReq );
            VOID osal_CbTimerUpdate( pClient->timerId, (ATT_MSG_TIMEOUT * 1000) );
            return ( blePending );
        }
    }
    else // ATT_ERROR_RSP
    {
        attErrorRsp_t* pErrorRsp = &pMsg->errorRsp;

        // See if an error occurred on the server
        if ( ( pErrorRsp->errCode != ATT_ERR_ATTR_NOT_FOUND ) || ( pClient->numRsps == 0 ) )
        {
            // Should never get here!
            return ( FAILURE );
        }

        // The sub-procedure is complete when the Error Response is received and
        // the Error Code is set to Attribute Not Found.
    }

    // No more attributes can be discovered on the server; this sub-procedure is complete.
    return ( SUCCESS );
}

/*********************************************************************
    @fn          gattProcessFindByTypeValue

    @brief       Process a Find By Type Value message.

    @param       pClient - pointer to client info
    @param       method - type of message
    @param       pMsg - pointer to received

    @return      SUCCESS: Sub-procedure completed
                FAILURE: Sub-procedure failed
                blePending: Sub-procedure still in progress
*/
static uint8 gattProcessFindByTypeValue( gattClientInfo_t* pClient,
                                         uint8 method, gattMsg_t* pMsg )
{
    // Response to Find By Type Value Request
    if ( method == ATT_FIND_BY_TYPE_VALUE_RSP )
    {
        attFindByTypeValueRsp_t* pRsp = &(pMsg->findByTypeValueRsp); // received response
        attFindByTypeValueReq_t* pReq = &(pClient->req.findByTypeValueReq); // original request
        pClient->numRsps++; // Increment number of responses

        // The sub-procedure is complete whe the Find By Type Value Response has
        // an Attribute Handle that is equal to the Ending Handle of the request.
        if ( pRsp->handlesInfo[pRsp->numInfo-1].handle < pReq->endHandle )
        {
            // Update the start handle
            pReq->startHandle = pRsp->handlesInfo[pRsp->numInfo-1].handle + 1;
            // Send another Find By Type Value Request
            VOID ATT_FindByTypeValueReq( pClient->connHandle, pReq );
            VOID osal_CbTimerUpdate( pClient->timerId, (ATT_MSG_TIMEOUT * 1000) );
            return ( blePending );
        }
    }
    else // ATT_ERROR_RSP
    {
        attErrorRsp_t* pErrorRsp = &pMsg->errorRsp;

        // See if an error occurred on the server
        if ( ( pErrorRsp->errCode != ATT_ERR_ATTR_NOT_FOUND ) || ( pClient->numRsps == 0 ) )
        {
            // The service declaration is readable and requires no authentication
            // or authorization, therefore insufficient authentication or read not
            // permitted errors shall not occur.
            // Should never get here!
            return ( FAILURE );
        }

        // The sub-procedure is complete when the Error Response is received and
        // the Error Code is set to Attribute Not Found.
    }

    // No more attributes can be discovered on the server; this sub-procedure is complete.
    return ( SUCCESS );
}

/*********************************************************************
    @fn          gattProcessReadByType

    @brief       Process a Read By Type message.

    @param       pClient - pointer to client info
    @param       method - type of message
    @param       pMsg - pointer to received

    @return      SUCCESS: Sub-procedure completed
                FAILURE: Sub-procedure failed
                blePending: Sub-procedure still in progress
*/
static uint8 gattProcessReadByType( gattClientInfo_t* pClient,
                                    uint8 method, gattMsg_t* pMsg )
{
    gattReadByTypeReq_t* pReq = &(pClient->req.gattReadByTypeReq); // original request
    uint16 lastHandle = GATT_MAX_HANDLE;

    // Response to Read By Type Request
    if ( method == ATT_READ_BY_TYPE_RSP )
    {
        attReadByTypeRsp_t* pRsp = &(pMsg->readByTypeRsp); // received response
        uint8 lastIdx = ( pRsp->numPairs - 1 ) * pRsp->len;
        pClient->numRsps++; // Increment number of responses
        // Remember the last handle
        lastHandle = BUILD_UINT16( pRsp->dataList[lastIdx], pRsp->dataList[lastIdx+1] );

        // See if this is Discover Characteristics by UUID
        if ( pReq->discCharsByUUID == TRUE )
        {
            uint8 matchedLen = 0;
            uint8 dataLen = pRsp->numPairs * pRsp->len;
            // Let's assume no match will be found!
            pRsp->numPairs = 0;

            // A list of Attribute Handle and Attribute Value pairs is returned. Each
            // Attribute Value in the list is the Attribute Value for the characteristic
            // declaration. The Attribute Value contains the characteristic properties,
            // Characteristic Value Handle and characteristic UUID.
            for ( uint8 i = 0; i < dataLen; i += pRsp->len )
            {
                // Check the Attribute Value for each Attribute Handle and Attribute
                // Value pairs for a matching characteristic UUID.
                if ( ATT_CompareUUID( pReq->req.type.uuid, pReq->req.type.len,
                                      &(pRsp->dataList[i+5]), pRsp->len-5 ) )
                {
                    if ( i != matchedLen )
                    {
                        // Keep the matched attribute handle-value pair
                        VOID osal_memcpy( &(pRsp->dataList[matchedLen]),
                                          &(pRsp->dataList[i]), pRsp->len );
                    }

                    // Increment the length of data matched so far
                    matchedLen += pRsp->len;
                    // Increment the number of the handle-value pairs found so far
                    pRsp->numPairs++;
                }
            } // for
        }
    }
    else // ATT_ERROR_RSP
    {
        attErrorRsp_t* pErrorRsp = &pMsg->errorRsp;

        // See if an error occurred on the server
        if ( ( pErrorRsp->errCode != ATT_ERR_ATTR_NOT_FOUND ) || ( pClient->numRsps == 0 ) )
        {
            // The service, include and characteristic declarations are readable and
            // require no authentication or authorization, therefore insufficient
            // authentication or read not permitted errors shall not occur.
            if ( !gattServiceType( pReq->req.type )                   &&
                    !gattIncludeType( pReq->req.type )                   &&
                    !gattCharacterType( pReq->req.type )                 &&
                    ( pErrorRsp->errCode == ATT_ERR_READ_NOT_PERMITTED   ||
                      pErrorRsp->errCode == ATT_ERR_INSUFFICIENT_AUTHEN  ||
                      pErrorRsp->errCode == ATT_ERR_INSUFFICIENT_AUTHOR  ||
                      pErrorRsp->errCode == ATT_ERR_INSUFFICIENT_ENCRYPT ||
                      pErrorRsp->errCode == ATT_ERR_INSUFFICIENT_KEY_SIZE ) )
            {
                // An attribute of the given type has been discovered but the value
                // cannot be read. We should continue discovering.
                pClient->numRsps++; // Increment number of responses
                // Remember the last handle
                lastHandle = pErrorRsp->handle;
            }
            else
            {
                // Should never get here!
                return ( FAILURE );
            }
        }

        // The sub-procedure is complete when the Error Response is received
        // with the Error Code set to Attribute Not Found.
    }

    // More attributes wish to be discovered?
    if ( lastHandle != GATT_MAX_HANDLE )
    {
        // The sub-procedure is complete whe the Read By Type Response has an
        // Attribute Handle that is equal to the Ending Handle of the request.
        if ( lastHandle < pReq->req.endHandle )
        {
            // Update the start handle
            pReq->req.startHandle = lastHandle + 1;

            // Send another Read By Type Request
            if ( pReq->discCharsByUUID == TRUE )
            {
                attReadByTypeReq_t req;
                // This is Discover Characteristic by UUID
                req.startHandle = pReq->req.startHandle;
                req.endHandle = pReq->req.endHandle;
                // Set Type to Characteristic UUID
                req.type.len = ATT_BT_UUID_SIZE;
                req.type.uuid[0] = LO_UINT16( GATT_CHARACTER_UUID );
                req.type.uuid[1] = HI_UINT16( GATT_CHARACTER_UUID );
                VOID ATT_ReadByTypeReq( pClient->connHandle, &req );
            }
            else
            {
                VOID ATT_ReadByTypeReq( pClient->connHandle, &pReq->req );
            }

            VOID osal_CbTimerUpdate( pClient->timerId, (ATT_MSG_TIMEOUT * 1000) );
            return ( blePending );
        }
    }

    // All attributes with the given type have been discovered; this sub-procedure is complete.
    return ( SUCCESS );
}

/*********************************************************************
    @fn          gattProcessReadLong

    @brief       Process Read Long message.

    @param       pClient - pointer to client info
    @param       method - type of message
    @param       pMsg - pointer to received

    @return      SUCCESS: Sub-procedure completed
                FAILURE: Sub-procedure failed
                blePending: Sub-procedure still in progress
*/
static uint8 gattProcessReadLong( gattClientInfo_t* pClient,
                                  uint8 method, gattMsg_t* pMsg )
{
    // Response to Read Blob Request
    if ( method == ATT_READ_BLOB_RSP )
    {
        attReadBlobReq_t* pReq = &(pClient->req.readBlobReq); // original request
        attReadBlobRsp_t* pRsp = &pMsg->readBlobRsp; // received response
        pClient->numRsps++; // Increment number of responses

        // The Read Blob Request is repeated until the Read Blob Response�s
        // Part Attribute Value parameter is shorter than (ATT_MTU-1).
        if ( pRsp->len >= ( gAttMtuSize[pClient->connHandle] - 1 ) )
        {
            // Update the offset. The offset for subsequent Read Blob Requests is
            // the next octet that has yet to be read.
            pReq->offset += pRsp->len;
            // Try to read the next part of the attribute value
            VOID ATT_ReadBlobReq( pClient->connHandle, pReq );
            VOID osal_CbTimerUpdate( pClient->timerId, (ATT_MSG_TIMEOUT * 1000) );
            return ( blePending );
        }
    }
    else // ATT_ERROR_RSP
    {
        attErrorRsp_t* pErrorRsp = &pMsg->errorRsp;

        // See if an error occurred on the server
        if ( ( pErrorRsp->errCode != ATT_ERR_INVALID_OFFSET ) || ( pClient->numRsps == 0 ) )
        {
            // Should never get here!
            return ( FAILURE );
        }

        // The sub-procedure is complete when the Error Response is received
        // with the Error Code set to Invalid Offset.
    }

    // This sub-procedure is complete; send the response up to application.
    return ( SUCCESS );
}

/*********************************************************************
    @fn          gattProcessReadByGrpType

    @brief       Process a Read By Group Type message.

    @param       pClient - pointer to client info
    @param       method - type of message
    @param       pMsg - pointer to received

    @return      SUCCESS: Sub-procedure completed
                FAILURE: Sub-procedure failed
                blePending: Sub-procedure still in progress
*/
static uint8 gattProcessReadByGrpType( gattClientInfo_t* pClient,
                                       uint8 method, gattMsg_t* pMsg )
{
    uint16 endGrpHandle;

    // Response to Read By Group Type Request
    if ( method == ATT_READ_BY_GRP_TYPE_RSP )
    {
        attReadByGrpTypeRsp_t* pRsp = &(pMsg->readByGrpTypeRsp); // received response
        uint8 lastGrpIdx = ( pRsp->numGrps - 1 ) * pRsp->len;
        pClient->numRsps++; // Increment number of responses
        // Remember the end group handle
        endGrpHandle = BUILD_UINT16( pRsp->dataList[lastGrpIdx+2], pRsp->dataList[lastGrpIdx+3] );
    }
    else // ATT_ERROR_RSP
    {
        attErrorRsp_t* pErrorRsp = &pMsg->errorRsp;

        // See if an error occurred on the server
        if ( ( pErrorRsp->errCode != ATT_ERR_ATTR_NOT_FOUND ) || ( pClient->numRsps == 0 ) )
        {
            // The service declaration is readable and requires no authentication
            // or authorization, therefore insufficient authentication or read not
            // permitted errors shall not occur.
            // Should never get here!
            return ( FAILURE );
        }

        // The sub-procedure is complete when the Error Response is received and the
        // Error Code is set to Attribute Not Found.
        endGrpHandle = GATT_MAX_HANDLE;
    }

    // More attributes wish to be discovered?
    if ( endGrpHandle != GATT_MAX_HANDLE )
    {
        attReadByGrpTypeReq_t* pReq = &(pClient->req.readByGrpTypeReq); // original request
        // Update the start handle
        pReq->startHandle = endGrpHandle + 1;

        // See if there're more attributes to be discovered
        if ( pReq->startHandle <= pReq->endHandle )
        {
            // Send another Read By Type Request
            VOID ATT_ReadByGrpTypeReq( pClient->connHandle, pReq );
            VOID osal_CbTimerUpdate( pClient->timerId, (ATT_MSG_TIMEOUT * 1000) );
            return ( blePending );
        }
    }

    // All attributes with the given type have been discovered; this sub-procedure is complete.
    return ( SUCCESS );
}

/*********************************************************************
    @fn          gattProcessWriteLong

    @brief       Process Write Long message.

    @param       pClient - pointer to client info
    @param       method - type of message
    @param       pMsg - pointer to received

    @return      SUCCESS: Sub-procedure completed
                FAILURE: Sub-procedure failed
                blePending: Sub-procedure still in progress
*/
static bStatus_t gattProcessWriteLong( gattClientInfo_t* pClient,
                                       uint8 method, gattMsg_t* pMsg )
{
    attExecuteWriteReq_t executeReq;
    VOID pMsg; // Not used here

    if ( method == ATT_PREPARE_WRITE_RSP )
    {
        gattWriteLongReq_t* pReq = &(pClient->req.gattWriteLongReq); // original GATT request
        uint16 transferredLen;
        // Note: The values in the Prepare Write Response do not need to be
        // verified in this sub-procedure.
        // The offset for subsequent Prepare Write Requests is the next octet
        // that has yet to be written.
        pReq->lastOffset += ( gAttMtuSize[pClient->connHandle] - 5 ); // last packet could be less than this but it's ok
        // The Prepare Write Request is repeated until the complete Characteristic
        // Value has been transferred.
        transferredLen = pReq->lastOffset - pReq->req.offset;

        if ( transferredLen < pReq->req.len )
        {
            attPrepareWriteReq_t req;

            // Build the next Prepare Write Request
            if ( ( pReq->req.len - transferredLen ) > ( gAttMtuSize[pClient->connHandle] - 5 ) )
            {
                req.len = gAttMtuSize[pClient->connHandle] - 5;
            }
            else
            {
                // Last part of the attribute value
                req.len = pReq->req.len - transferredLen;
            }

            req.handle = pReq->req.handle;
            req.offset = pReq->lastOffset;
            VOID osal_memcpy( req.value, &(pReq->req.pValue[transferredLen]), req.len );
            // Try to send the next Prepare Write Request
            VOID ATT_PrepareWriteReq( pClient->connHandle, &req );
            VOID osal_CbTimerUpdate( pClient->timerId, (ATT_MSG_TIMEOUT * 1000) );
            return ( blePending );
        }
        else
        {
            // All Prepare Write Requests have been sent successfully. Immediately
            // write all pending prepared values by transmitting an Execute Write
            // Request with the Flags parameter set to 0x01.
            executeReq.flags = ATT_WRITE_PREPARED_VALUES;
        }
    }
    else // ATT_ERROR_RSP or error in ATT_PREPARE_WRITE_RSP
    {
        // An error has occured. Abort the sub-procedure by sending an Execute
        // Write Request with the Flags parameter set to 0x00 to cancel all
        // prepared writes.
        executeReq.flags = ATT_CANCEL_PREPARED_WRITES;
    }

    // The first phase is complete. Start the second phase by sending an
    // Execute Write Request. Once the Execute Write Response has been
    // received by the client, this procedure is complete.
    VOID ATT_ExecuteWriteReq( pClient->connHandle, &executeReq );

    if ( method == ATT_ERROR_RSP )
    {
        // Send the response up to application; this sub-procedure is complete.
        return ( FAILURE );
    }

    // Wait for the Execute Write Response
    VOID osal_CbTimerUpdate( pClient->timerId, (ATT_MSG_TIMEOUT * 1000) );
    // Update the response info for this client
    pClient->method = ATT_EXECUTE_WRITE_RSP;
    pClient->pfnParseRsp = ATT_ParseExecuteWriteRsp;
    return ( blePending );
}

/*********************************************************************
    @fn          gattProcessReliableWrites

    @brief       Process Relaible Writes message.

    @param       pClient - pointer to client info
    @param       method - type of message
    @param       pMsg - pointer to received

    @return      SUCCESS: Sub-procedure completed
                FAILURE: Sub-procedure failed
                blePending: Sub-procedure still in progress
*/
static bStatus_t gattProcessReliableWrites( gattClientInfo_t* pClient,
                                            uint8 method, gattMsg_t* pMsg )
{
    gattReliableWritesReq_t* pReq = &(pClient->req.gattReliableWritesReq); // original request
    attExecuteWriteReq_t executeReq;

    if ( method == ATT_PREPARE_WRITE_RSP )
    {
        attPrepareWriteRsp_t* pRsp = &(pMsg->prepareWriteRsp);

        // Check the attribute value in the response with the attribute value
        // that was sent in the Prepare Write Request
        if ( ( pReq->pReqs[pReq->index].handle == pRsp->handle ) &&
                ( pReq->pReqs[pReq->index].offset == pRsp->offset ) &&
                ( pReq->pReqs[pReq->index].len    == pRsp->len )    &&
                ( osal_memcmp( pReq->pReqs[pReq->index].value, pRsp->value, pRsp->len ) ) )
        {
            // See if all Prepare Write Requests have been sent
            if ( ++pReq->index < pReq->numReqs )
            {
                // Try to send the next Write Prepare Request
                VOID ATT_PrepareWriteReq( pClient->connHandle, &(pReq->pReqs[pReq->index]) );
                VOID osal_CbTimerUpdate( pClient->timerId, (ATT_MSG_TIMEOUT * 1000) );
                return ( blePending );
            }
            else
            {
                // All Prepare Write Requests have been sent successfully. Immediately
                // write all pending prepared values by transmitting an Execute Write
                // Request with the Flags parameter set to 0x01.
                executeReq.flags = pReq->flags;
            }
        }
        else
        {
            // The attribute value has been corrupted during transmission. Abort
            // the sub-procedure by sending an Execute Write Request with the Flags
            // parameter set to 0x00 to cancel all prepared writes.
            executeReq.flags = ATT_CANCEL_PREPARED_WRITES;
        }
    }
    else // ATT_ERROR_RSP
    {
        // An error has occured. Abort the sub-procedure by sending an Execute
        // Write Request with the Flags parameter set to 0x00 to cancel all
        // prepared writes.
        executeReq.flags = ATT_CANCEL_PREPARED_WRITES;
    }

    // The first phase is complete. Start the second phase by sending an
    // Execute Write Request. Once the Execute Write Response has been
    // received by the client, this procedure is complete.
    VOID ATT_ExecuteWriteReq( pClient->connHandle, &executeReq );

    if ( method == ATT_ERROR_RSP )
    {
        // Send the response up to application; this sub-procedure is complete.
        return ( FAILURE );
    }

    // Wait for the Execute Write Response
    VOID osal_CbTimerUpdate( pClient->timerId, (ATT_MSG_TIMEOUT * 1000) );
    // Update the response info for this client
    pClient->method = ATT_EXECUTE_WRITE_RSP;
    pClient->pfnParseRsp = ATT_ParseExecuteWriteRsp;
    return ( blePending );
}

/*********************************************************************
    @fn      gattStoreClientInfo

    @brief   Store client info.

    @param   connHandle ? connection to use
    @param   pMsg ? pointer to message to be sent
    @param   method ? response to expect
    @param   pfnParseRsp ? parse function for response
    @param   taskId ? task to be notified of response

    @return  none
*/
static void gattStoreClientInfo( gattClientInfo_t* pClient, gattMsg_t* pReq,
                                 uint8 method, gattParseRsp_t pfnParseRsp, uint8 taskId )
{
    if ( taskId != INVALID_TASK_ID )
    {
        // Start a timeout timer for the response
        gattClientStartTimer( (uint8*)pClient, ATT_MSG_TIMEOUT, &pClient->timerId );
        // Store task id to forward the response to
        pClient->taskId = taskId;
        // Store the response method
        pClient->method = method;
        // Store parse function for the response
        pClient->pfnParseRsp = pfnParseRsp;

        // Store info for GATT request
        if ( pReq != NULL )
        {
            VOID osal_memcpy( &(pClient->req), pReq, sizeof( gattMsg_t ) );
        }

        // else pClient->req is already zero'ed out
    }
}

/*********************************************************************
    @fn          gattGetClientStatus

    @brief       Get the status for a given client.

    @param       connHandle - client connection to server
    @param       p2pClient - pointer to pointer to client info

    @return      SUCCESS: No response pending
                INVALIDPARAMETER: Invalid connection handle
                blePending: Response pending
                bleTimeout: Previous transaction timed out
*/
static bStatus_t gattGetClientStatus( uint16 connHandle, gattClientInfo_t** p2pClient )
{
    gattClientInfo_t* pClient;
    pClient = gattFindClientInfo( connHandle );

    if ( pClient != NULL )
    {
        if ( p2pClient != NULL )
        {
            *p2pClient = pClient;
        }

        // Make sure there's no response pending or timed out with this server
        return ( TIMER_STATUS( pClient->timerId ) );
    }

    // Connection handle not found
    return ( INVALIDPARAMETER );
}

/*********************************************************************
    @fn      gattFindClientInfo

    @brief   Find the client info.  Uses the connection handle to search
            the client info table.

    @param   connHandle - connection handle.

    @return  a pointer to the found item. NULL, otherwise.
*/
static gattClientInfo_t* gattFindClientInfo( uint16 connHandle )
{
    uint8 i;

    for ( i = 0; i < GATT_MAX_NUM_CONN; i++ )
    {
        if ( clientInfoTbl[i].connHandle == connHandle )
        {
            // Entry found
            return ( &clientInfoTbl[i] );
        }
    }

    return ( (gattClientInfo_t*)NULL );
}

/*********************************************************************
    @fn      gattResetClientInfo

    @brief   Reset the client info.

    @param   pClient - pointer to client info.

    @return  none
*/
static void gattResetClientInfo( gattClientInfo_t* pClient )
{
    // First cancel the response timer
    gattStopTimer( &pClient->timerId );

    // Free the buffer provided by the application
    if ( ( pClient->method == ATT_PREPARE_WRITE_RSP ) ||
            ( pClient->method == ATT_EXECUTE_WRITE_RSP ) )
    {
        uint8* pBuf;

        if ( pClient->req.gattReliableWritesReq.reliable == TRUE )
        {
            pBuf = (uint8*)(pClient->req.gattReliableWritesReq.pReqs);
        }
        else
        {
            pBuf = pClient->req.gattWriteLongReq.req.pValue;
        }

        if ( pBuf != NULL )
        {
            osal_mem_free( pBuf );
        }
    }

    // Reset response info
    pClient->method = 0;
    pClient->taskId = INVALID_TASK_ID;
    pClient->numRsps = 0;
    // Reset request info
    VOID osal_memset( &(pClient->req), 0, sizeof( gattMsg_t ) );
}

/*********************************************************************
    @fn      gattClientStartTimer

    @brief   Start a client timer to expire in n seconds.

    @param   pData - data to be passed in to callback function
    @param   timeout - in seconds.
    @param   pTimerId - will point to new timer Id (if not null)

    @return  none
*/
static void gattClientStartTimer( uint8* pData, uint16 timeout, uint8* pTimerId )
{
    gattStartTimer( gattClientHandleTimerCB, pData, timeout, pTimerId );
}

/*********************************************************************
    @fn      gattClientHandleTimerCB

    @brief   Handle a callback for a timer that has just expired.

    @param   pData - pointer to timer data

    @return  none
*/
static void gattClientHandleTimerCB( uint8* pData )
{
    gattClientInfo_t* pClient = (gattClientInfo_t*)pData;

    // Response timer has expired
    if ( ( pClient != NULL ) && ( pClient->timerId != INVALID_TIMER_ID ) )
    {
        // Notify the application about the timeout
        VOID gattNotifyEvent( pClient->taskId, pClient->connHandle, bleTimeout, pClient->method, NULL );

        if ( pClient->method == ATT_EXECUTE_WRITE_REQ )
        {
            attExecuteWriteReq_t req;
            // Cancel all prepared writes
            req.flags = ATT_CANCEL_PREPARED_WRITES;
            VOID ATT_ExecuteWriteReq( pClient->connHandle, &req );
        }

        // Timer has expired. If a transaction has not completed before it times
        // out, then this transaction shall be considered to have failed. No more
        // attribute protocol requests, commands, indications or notifications
        // shall be sent to the target device on this ATT Bearer.
//ZQ 20181216 for test
        //pClient->timerId = TIMEOUT_TIMER_ID;
        // Reset client info
        gattResetClientInfo( pClient );
    }
}

/*********************************************************************
    @fn          gattClientHandleConnStatusCB

    @brief       GATT link status change handler function.

    @param       connHandle - connection handle
    @param       changeType - type of change

    @return      none
*/
static void gattClientHandleConnStatusCB( uint16 connHandle, uint8 changeType )
{
    gattClientInfo_t* pClient = NULL;

    // Check to see if this is loopback connection
    if ( connHandle == LOOPBACK_CONNHANDLE )
    {
        return;
    }

    if ( changeType == LINKDB_STATUS_UPDATE_NEW )
    {
        // A new connection has been made
        pClient = gattFindClientInfo( connHandle );

        if ( pClient == NULL )
        {
            // Entry not found; add it to the server table
            pClient = gattFindClientInfo( INVALID_CONNHANDLE );

            if ( pClient != NULL )
            {
                // Empty entry found
                pClient->connHandle = connHandle;
            }
        }

        // We're done here!
        return;
    }

    if ( changeType == LINKDB_STATUS_UPDATE_REMOVED )
    {
        pClient = gattFindClientInfo( connHandle );

        if ( pClient != NULL )
        {
            // Entry found; remove it from the client table
            pClient->connHandle = INVALID_CONNHANDLE;
        }
    }
    else if ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS )
    {
        // Check to see if the connection has dropped
        if ( !linkDB_Up( connHandle ) )
        {
            pClient = gattFindClientInfo( connHandle );
        }
    }

    // Connection has dropped
    if ( pClient != NULL )
    {
        if ( pClient->timerId != INVALID_TIMER_ID )
        {
            if ( pClient->timerId != TIMEOUT_TIMER_ID )
            {
                // Notify the application about the link disconnect
                VOID gattNotifyEvent( pClient->taskId, connHandle, bleNotConnected,
                                      pClient->method, NULL );
            }

            // Reset client info
            gattResetClientInfo( pClient );
            // Just in case if we've timed out waiting for a response
            pClient->timerId = INVALID_TIMER_ID;
        }
    }
}


/****************************************************************************
****************************************************************************/
