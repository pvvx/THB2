/**************************************************************************************************
*******
**************************************************************************************************/

/*************************************************************************************************
    Filename:       gattservapp.c
    Revised:
    Revision:

    Description:    This file contains the GATT Server Application.



**************************************************************************************************/

#if ( HOST_CONFIG & ( CENTRAL_CFG | PERIPHERAL_CFG ) )

/*******************************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "linkdb.h"

#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"


/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/

/*********************************************************************
    TYPEDEFS
*/

// Structure to keep Prepare Write Requests for each Client
typedef struct
{
    uint16 connHandle;                    // connection message was received on
    attPrepareWriteReq_t* pPrepareWriteQ; // Prepare Write Request queue
} prepareWrites_t;

// GATT Structure to keep CBs information for each service being registered
typedef struct
{
    uint16 handle;                // Service handle - assigned internally by GATT Server
    CONST gattServiceCBs_t* pCBs; // Service callback function pointers
} gattServiceCBsInfo_t;

// Service callbacks list item
typedef struct _serviceCBsList
{
    struct _serviceCBsList* next;     // pointer to next service callbacks record
    gattServiceCBsInfo_t serviceInfo; // service handle/callbacks
} serviceCBsList_t;

/*********************************************************************
    GLOBAL VARIABLES
*/

/*********************************************************************
    EXTERNAL VARIABLES
*/

/*********************************************************************
    EXTERNAL FUNCTIONS
*/
extern l2capSegmentBuff_t   l2capSegmentPkt;
/*********************************************************************
    LOCAL VARIABLES
*/
uint8 GATTServApp_TaskID;   // Task ID for internal task/event processing

uint8 appTaskID = INVALID_TASK_ID; // The task ID of an app/profile that
// wants GATT Server event messages

// Server Prepare Write table (one entry per each physical link)
static prepareWrites_t prepareWritesTbl[MAX_NUM_LL_CONN];

// Maximum number of attributes that Server can prepare for writing per Client
static uint8 maxNumPrepareWrites = 0;
#ifdef PREPARE_QUEUE_STATIC
    static attPrepareWriteReq_t prepareQueue[MAX_NUM_LL_CONN*GATT_MAX_NUM_PREPARE_WRITES];
#endif
// Callbacks for services
static serviceCBsList_t* serviceCBsList = NULL;

// Globals to be used for processing an incoming request
//static uint8 attrLen;
static uint16 attrLen;
static uint8 attrValue[ATT_MTU_SIZE-1];
static attMsg_t rsp;

/*** Defined GATT Attributes ***/

// GATT Service attribute
static CONST gattAttrType_t gattService = { ATT_BT_UUID_SIZE, gattServiceUUID };

#ifndef HID_VOICE_SPEC
    // Service Changed Characteristic Properties
    static uint8 serviceChangedCharProps = GATT_PROP_INDICATE;
#endif

// Service Changed attribute (hidden). Set the affected Attribute Handle range
// to 0x0001 to 0xFFFF to indicate to the client to rediscover the entire set
// of Attribute Handles on the server.

// Client Characteristic configuration. Each client has its own instantiation
// of the Client Characteristic Configuration. Reads of the Client Characteristic
// Configuration only shows the configuration for that client and writes only
// affect the configuration of that client.
static gattCharCfg_t indCharCfg[GATT_MAX_NUM_CONN];

#if defined ( TESTMODES )
    static uint16 paramValue = 0;
#endif

/*********************************************************************
    Profile Attributes - Table
*/

// GATT Attribute Table
static gattAttribute_t gattAttrTbl[] =
{
    // Generic Attribute Profile
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
        GATT_PERMIT_READ,                         /* permissions */
        0,                                        /* handle */
        (uint8*)& gattService                     /* pValue */
    },
    #ifndef HID_VOICE_SPEC
    // Characteristic Declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &serviceChangedCharProps
    },

    // Attribute Service Changed
    {
        { ATT_BT_UUID_SIZE, serviceChangedUUID },
        0,
        0,
        NULL
    },

    // Client Characteristic configuration
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8*)indCharCfg
    }
    #endif
};

/*********************************************************************
    LOCAL FUNCTIONS
*/
static void gattServApp_ProcessMsg( gattMsgEvent_t* pMsg );
static bStatus_t gattServApp_ProcessExchangeMTUReq( gattMsgEvent_t* pMsg );
static bStatus_t gattServApp_ProcessFindByTypeValueReq( gattMsgEvent_t* pMsg, uint16* pErrHandle );
static bStatus_t gattServApp_ProcessReadByTypeReq( gattMsgEvent_t* pMsg, uint16* pErrHandle );
static bStatus_t gattServApp_ProcessReadReq( gattMsgEvent_t* pMsg, uint16* pErrHandle );
static bStatus_t gattServApp_ProcessReadBlobReq( gattMsgEvent_t* pMsg, uint16* pErrHandle );
static bStatus_t gattServApp_ProcessReadMultiReq( gattMsgEvent_t* pMsg, uint16* pErrHandle );
static bStatus_t gattServApp_ProcessReadByGrpTypeReq( gattMsgEvent_t* pMsg, uint16* pErrHandle );
static bStatus_t gattServApp_ProcessWriteReq( gattMsgEvent_t* pMsg, uint16* pErrHandle );
static bStatus_t gattServApp_ProcessPrepareWriteReq( gattMsgEvent_t* pMsg, uint16* pErrHandle );
static bStatus_t gattServApp_ProcessExecuteWriteReq( gattMsgEvent_t* pMsg, uint16* pErrHandle );

static bStatus_t gattServApp_RegisterServiceCBs( uint16 handle, CONST gattServiceCBs_t* pServiceCBs );
static bStatus_t gattServApp_DeregisterServiceCBs( uint16 handle );
static bStatus_t gattServApp_SetNumPrepareWrites( uint8 numPrepareWrites );
static uint8 gattServApp_PrepareWriteQInUse( void );
static CONST gattServiceCBs_t* gattServApp_FindServiceCBs( uint16 service );
static bStatus_t gattServApp_EnqueuePrepareWriteReq( uint16 connHandle, attPrepareWriteReq_t* pReq );
static prepareWrites_t* gattServApp_FindPrepareWriteQ( uint16 connHandle );
static gattCharCfg_t* gattServApp_FindCharCfgItem( uint16 connHandle,
                                                   gattCharCfg_t* charCfgTbl );
static pfnGATTReadAttrCB_t gattServApp_FindReadAttrCB( uint16 handle );
static pfnGATTWriteAttrCB_t gattServApp_FindWriteAttrCB( uint16 handle );
static pfnGATTAuthorizeAttrCB_t gattServApp_FindAuthorizeAttrCB( uint16 handle );

/*********************************************************************
    API FUNCTIONS
*/

// GATT App Callback functions
static void gattServApp_HandleConnStatusCB( uint16 connHandle, uint8 changeType );
static bStatus_t gattServApp_WriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                          uint8* pValue, uint16 len, uint16 offset );

/*********************************************************************
    PROFILE CALLBACKS
*/
// GATT Service Callbacks
CONST gattServiceCBs_t gattServiceCBs =
{
    NULL,                    // Read callback function pointer
    gattServApp_WriteAttrCB, // Write callback function pointer
    NULL                     // Authorization callback function pointer
};
static gattServMsgCB_t s_GATTServCB = NULL;
/*********************************************************************
    @fn      GATTServApp_RegisterForMsgs

    @brief   Register your task ID to receive event messages from
            the GATT Server Application.

    @param   taskId - Default task ID to send events

    @return  none
*/
void GATTServApp_RegisterForMsg( uint8 taskID )
{
    appTaskID = taskID;
}

/*********************************************************************
    @fn      GATTServApp_Init

    @brief   Initialize the GATT Server Application.

    @param   taskId - Task identifier for the desired task

    @return  none
*/
void GATTServApp_Init( uint8 taskId )
{
    GATTServApp_TaskID = taskId;
    // Initialize Client Characteristic Configuration attributes
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, indCharCfg );

    // Initialize Prepare Write Table
    for ( uint8 i = 0; i < MAX_NUM_LL_CONN; i++ )
    {
        // Initialize connection handle
        prepareWritesTbl[i].connHandle = INVALID_CONNHANDLE;
        // Initialize the prepare write queue
        prepareWritesTbl[i].pPrepareWriteQ = NULL;
    }

    // Set up the initial prepare write queues
    gattServApp_SetNumPrepareWrites( GATT_MAX_NUM_PREPARE_WRITES );
    // Register to receive incoming ATT Requests
    GATT_RegisterForReq( GATTServApp_TaskID );
    // Register with Link DB to receive link status change callback
    linkDB_Register( gattServApp_HandleConnStatusCB );
}

/*********************************************************************
    @fn      GATTServApp_ProcessEvent

    @brief   GATT Server Application Task event processor. This function
            is called to process all events for the task. Events include
            timers, messages and any other user defined events.

    @param   task_id - The OSAL assigned task ID.
    @param   events - events to process. This is a bit map and can
                     contain more than one event.

    @return  none
*/
uint16 GATTServApp_ProcessEvent( uint8 task_id, uint16 events )
{
    if ( events & SYS_EVENT_MSG )
    {
        osal_event_hdr_t* pMsg;

        if ( (pMsg = ( osal_event_hdr_t*)osal_msg_receive( GATTServApp_TaskID )) != NULL )
        {
            // Process incoming messages
            switch ( pMsg->event )
            {
            // Incoming GATT message
            case GATT_MSG_EVENT:
                gattServApp_ProcessMsg( (gattMsgEvent_t*)pMsg );
                break;

            default:
                // Unsupported message
                break;
            }

            // Release the OSAL message
            VOID osal_msg_deallocate( (uint8*)pMsg );
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    // Discard unknown events
    return 0;
}

/******************************************************************************
    @fn      GATTServApp_RegisterService

    @brief   Register a service's attribute list and callback functions with
            the GATT Server Application.

    @param   pAttrs - Array of attribute records to be registered
    @param   numAttrs - Number of attributes in array
    @param   pServiceCBs - Service callback function pointers

    @return  SUCCESS: Service registered successfully.
            INVALIDPARAMETER: Invalid service field.
            FAILURE: Not enough attribute handles available.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t GATTServApp_RegisterService( gattAttribute_t* pAttrs, uint16 numAttrs,
                                       CONST gattServiceCBs_t* pServiceCBs )
{
    uint8 status;

    // First register the service attribute list with GATT Server
    if ( pAttrs != NULL )
    {
        gattService_t service;
        service.attrs = pAttrs;
        service.numAttrs = numAttrs;
        status = GATT_RegisterService( &service );

        if ( ( status == SUCCESS ) && ( pServiceCBs != NULL ) )
        {
            // Register the service CBs with GATT Server Application
            status = gattServApp_RegisterServiceCBs( GATT_SERVICE_HANDLE( pAttrs ),
                                                     pServiceCBs );
        }
    }
    else
    {
        status = INVALIDPARAMETER;
    }

    return ( status );
}

/******************************************************************************
    @fn      GATTServApp_DeregisterService

    @brief   Deregister a service's attribute list and callback functions from
            the GATT Server Application.

            NOTE: It's the caller's responsibility to free the service attribute
            list returned from this API.

    @param   handle - handle of service to be deregistered
    @param   p2pAttrs - pointer to array of attribute records (to be returned)

    @return  SUCCESS: Service deregistered successfully.
            FAILURE: Service not found.
*/
bStatus_t GATTServApp_DeregisterService( uint16 handle, gattAttribute_t** p2pAttrs )
{
    uint8 status;
    // First deregister the service CBs with GATT Server Application
    status = gattServApp_DeregisterServiceCBs( handle );

    if ( status == SUCCESS )
    {
        gattService_t service;
        // Deregister the service attribute list with GATT Server
        status = GATT_DeregisterService( handle, &service );

        if ( status == SUCCESS )
        {
            if ( p2pAttrs != NULL )
            {
                *p2pAttrs = service.attrs;
            }
        }
    }

    return ( status );
}

/*********************************************************************
    @fn      GATTServApp_SetParameter

    @brief   Set a GATT Server parameter.

    @param   param - Profile parameter ID
    @param   len - length of data to right
    @param   pValue - pointer to data to write.  This is dependent on the
                     the parameter ID and WILL be cast to the appropriate
                     data type (example: data type of uint16 will be cast
                     to uint16 pointer).

    @return  SUCCESS: Parameter set successful
            FAILURE: Parameter in use
            INVALIDPARAMETER: Invalid parameter
            bleInvalidRange: Invalid value
            bleMemAllocError: Memory allocation failed
*/
bStatus_t GATTServApp_SetParameter( uint8 param, uint8 len, void* pValue )
{
    bStatus_t status = SUCCESS;

    switch ( param )
    {
    case GATT_PARAM_NUM_PREPARE_WRITES:
        if ( len == sizeof ( uint8 ) )
        {
            if ( !gattServApp_PrepareWriteQInUse() )
            {
                // Set the new nunber of prepare writes
                status = gattServApp_SetNumPrepareWrites( *((uint8*)pValue) );
            }
            else
            {
                status = FAILURE;
            }
        }
        else
        {
            status = bleInvalidRange;
        }

        break;

    default:
        status = INVALIDPARAMETER;
        break;
    }

    return ( status );
}

/*********************************************************************
    @fn      GATTServApp_GetParameter

    @brief   Get a GATT Server parameter.

    @param   param - Profile parameter ID
    @param   pValue - pointer to data to put. This is dependent on the
                     parameter ID and WILL be cast to the appropriate
                     data type (example: data type of uint16 will be
                     cast to uint16 pointer).

    @return  SUCCESS: Parameter get successful
            INVALIDPARAMETER: Invalid parameter
*/
bStatus_t GATTServApp_GetParameter( uint8 param, void* pValue )
{
    bStatus_t status = SUCCESS;

    switch ( param )
    {
    case GATT_PARAM_NUM_PREPARE_WRITES:
        *((uint8*)pValue) = maxNumPrepareWrites;
        break;

    default:
        status = INVALIDPARAMETER;
        break;
    }

    return ( status );
}

/*********************************************************************
    @fn      gattServApp_SetNumPrepareWrites

    @brief   Set the maximum number of the prepare writes.

    @param   numPrepareWrites - number of prepare writes

    @return  SUCCESS: New number set successfully.
            bleMemAllocError: Memory allocation failed.
*/
static bStatus_t gattServApp_SetNumPrepareWrites( uint8 numPrepareWrites )
{
    attPrepareWriteReq_t* pQueue;
    uint16 queueSize = ( MAX_NUM_LL_CONN * numPrepareWrites * sizeof( attPrepareWriteReq_t ) );
    // First make sure no one can get access to the Prepare Write Table
    maxNumPrepareWrites = 0;

    // Free the existing prepare write queues
    if ( prepareWritesTbl[0].pPrepareWriteQ != NULL )
    {
        #ifndef PREPARE_QUEUE_STATIC
        osal_mem_free( prepareWritesTbl[0].pPrepareWriteQ );
        #endif

        // Null out the prepare writes queues
        for ( uint8 i = 0; i < MAX_NUM_LL_CONN; i++ )
        {
            prepareWritesTbl[i].pPrepareWriteQ = NULL;
        }
    }

    // Allocate the prepare write queues
    #ifdef PREPARE_QUEUE_STATIC
    pQueue = prepareQueue;
    #else
    pQueue = osal_mem_alloc( queueSize );
    #endif

    if ( pQueue != NULL )
    {
        // Initialize the prepare write queues
        VOID osal_memset( pQueue, 0, queueSize );

        // Set up the prepare write queue for each client (i.e., connection)
        for ( uint8 i = 0; i < MAX_NUM_LL_CONN; i++ )
        {
            uint8 nextQ = i * numPrepareWrites; // Index of next available queue
            prepareWritesTbl[i].pPrepareWriteQ = &(pQueue[nextQ]);

            // Mark the prepare write request items as empty
            for ( uint8 j = 0; j < numPrepareWrites; j++ )
            {
                prepareWritesTbl[i].pPrepareWriteQ[j].handle = GATT_INVALID_HANDLE;
            }
        }

        // Set the new number of prepare writes
        maxNumPrepareWrites = numPrepareWrites;
        return ( SUCCESS );
    }

    return ( bleMemAllocError );
}

/*********************************************************************
    @fn          GATTServApp_FindAttr

    @brief       Find the attribute record within a service attribute
                table for a given attribute value pointer.

    @param       pAttrTbl - pointer to attribute table
    @param       numAttrs - number of attributes in attribute table
    @param       pValue - pointer to attribute value

    @return      Pointer to attribute record. NULL, if not found.
*/
gattAttribute_t* GATTServApp_FindAttr( gattAttribute_t* pAttrTbl, uint16 numAttrs, uint8* pValue )
{
    for ( uint16 i = 0; i < numAttrs; i++ )
    {
        if ( pAttrTbl[i].pValue == pValue )
        {
            // Attribute record found
            return ( &(pAttrTbl[i]) );
        }
    }

    return ( (gattAttribute_t*)NULL );
}

/******************************************************************************
    @fn      GATTServApp_AddService

    @brief   Add function for the GATT Service.

    @param   services - services to add. This is a bit map and can
                       contain more than one service.

    @return  SUCCESS: Service added successfully.
            INVALIDPARAMETER: Invalid service field.
            FAILURE: Not enough attribute handles available.
            bleMemAllocError: Memory allocation error occurred.
*/
bStatus_t GATTServApp_AddService( uint32 services )
{
    uint8 status = SUCCESS;

    if ( services & GATT_SERVICE )
    {
        // Register GATT attribute list and CBs with GATT Server Application
        status = GATTServApp_RegisterService( gattAttrTbl, GATT_NUM_ATTRS( gattAttrTbl ),
                                              &gattServiceCBs );
    }

    return ( status );
}

/******************************************************************************
    @fn      GATTServApp_DelService

    @brief   Delete function for the GATT Service.

    @param   services - services to delete. This is a bit map and can
                       contain more than one service.

    @return  SUCCESS: Service deleted successfully.
            FAILURE: Service not found.
*/
bStatus_t GATTServApp_DelService( uint32 services )
{
    uint8 status = SUCCESS;

    if ( services & GATT_SERVICE )
    {
        // Deregister GATT attribute list and CBs from GATT Server Application
        status = GATTServApp_DeregisterService( GATT_SERVICE_HANDLE( gattAttrTbl ), NULL );
    }

    return ( status );
}

/******************************************************************************
    @fn      gattServApp_RegisterServiceCBs

    @brief   Register callback functions for a service.

    @param   handle - handle of service being registered
    @param   pServiceCBs - pointer to service CBs to be registered

    @return  SUCCESS: Service CBs were registered successfully.
            INVALIDPARAMETER: Invalid service CB field.
            bleMemAllocError: Memory allocation error occurred.
*/
static bStatus_t gattServApp_RegisterServiceCBs( uint16 handle,
                                                 CONST gattServiceCBs_t* pServiceCBs )
{
    serviceCBsList_t* pNewItem;

    // Make sure the service handle is specified
    if ( handle == GATT_INVALID_HANDLE )
    {
        return ( INVALIDPARAMETER );
    }

    // Fill in the new service list
    pNewItem = (serviceCBsList_t*)osal_mem_alloc( sizeof( serviceCBsList_t ) );

    if ( pNewItem == NULL )
    {
        // Not enough memory
        return ( bleMemAllocError );
    }

    // Set up new service CBs item
    pNewItem->next = NULL;
    pNewItem->serviceInfo.handle = handle;
    pNewItem->serviceInfo.pCBs = pServiceCBs;

    // Find spot in list
    if ( serviceCBsList == NULL )
    {
        // First item in list
        serviceCBsList = pNewItem;
    }
    else
    {
        serviceCBsList_t* pLoop = serviceCBsList;

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
    @fn      gattServApp_DeregisterServiceCBs

    @brief   Deregister callback functions for a service.

    @param   handle - handle of service CBs to be deregistered

    @return  SUCCESS: Service CBs were deregistered successfully.
            FAILURE: Service CBs were not found.
*/
static bStatus_t gattServApp_DeregisterServiceCBs( uint16 handle )
{
    serviceCBsList_t* pLoop = serviceCBsList;
    serviceCBsList_t* pPrev = NULL;

    // Look for service
    while ( pLoop != NULL )
    {
        if ( pLoop->serviceInfo.handle == handle )
        {
            // Service CBs found; unlink it
            if ( pPrev == NULL )
            {
                // First item in list
                serviceCBsList = pLoop->next;
            }
            else
            {
                pPrev->next = pLoop->next;
            }

            // Free the service CB record
            osal_mem_free( pLoop );
            return ( SUCCESS );
        }

        pPrev = pLoop;
        pLoop = pLoop->next;
    }

    // Service CBs not found
    return ( FAILURE );
}

/*********************************************************************
    @fn      gattServApp_FindServiceCBs

    @brief   Find service's callback record.

    @param   handle - owner of service

    @return  Pointer to service record. NULL, otherwise.
*/
static CONST gattServiceCBs_t* gattServApp_FindServiceCBs( uint16 handle )
{
    serviceCBsList_t* pLoop = serviceCBsList;

    while ( pLoop != NULL )
    {
        if ( pLoop->serviceInfo.handle == handle )
        {
            return ( pLoop->serviceInfo.pCBs );
        }

        // Try next service
        pLoop = pLoop->next;
    }

    return ( (gattServiceCBs_t*)NULL );
}

/*********************************************************************
    @fn          gattServApp_ProcessMsg

    @brief       GATT Server App message processing function.

    @param       pMsg - pointer to received message

    @return      Success or Failure
*/
static void gattServApp_ProcessMsg( gattMsgEvent_t* pMsg )
{
    uint16 errHandle = GATT_INVALID_HANDLE;
    uint8 status;
    #if defined ( TESTMODES )

    if ( paramValue == GATT_TESTMODE_NO_RSP )
    {
        // Notify GATT that a message has been processed
        // Note: This call is optional if flow control is not used.
        GATT_AppCompletedMsg( pMsg );
        // Just ignore the incoming request messages
        return;
    }

    #endif

    // Process the GATT server message
    switch ( pMsg->method )
    {
    case ATT_EXCHANGE_MTU_REQ:
        status = gattServApp_ProcessExchangeMTUReq( pMsg );
        break;

    case ATT_FIND_BY_TYPE_VALUE_REQ:
        status = gattServApp_ProcessFindByTypeValueReq( pMsg, &errHandle );
        break;

    case ATT_READ_BY_TYPE_REQ:
        status = gattServApp_ProcessReadByTypeReq( pMsg, &errHandle );
        break;

    case ATT_READ_REQ:
        status = gattServApp_ProcessReadReq( pMsg, &errHandle );
        break;

    case ATT_READ_BLOB_REQ:
        status = gattServApp_ProcessReadBlobReq( pMsg, &errHandle );
        break;

    case ATT_READ_MULTI_REQ:
        status = gattServApp_ProcessReadMultiReq( pMsg, &errHandle );
        break;

    case ATT_READ_BY_GRP_TYPE_REQ:
        status = gattServApp_ProcessReadByGrpTypeReq( pMsg, &errHandle );
        break;

    case ATT_WRITE_REQ:
        status = gattServApp_ProcessWriteReq( pMsg, &errHandle );
        break;

    case ATT_PREPARE_WRITE_REQ:
        status = gattServApp_ProcessPrepareWriteReq( pMsg, &errHandle );
        break;

    case ATT_EXECUTE_WRITE_REQ:
        status = gattServApp_ProcessExecuteWriteReq( pMsg, &errHandle );
        break;

    default:
        // Unknown request - ignore it!
        status = SUCCESS;
        break;
    }

    // See if we need to send an error response back
    if ( status != SUCCESS )
    {
        // Make sure the request was not sent locally
        if ( pMsg->hdr.status != bleNotConnected )
        {
            attErrorRsp_t* pRsp = &rsp.errorRsp;
            pRsp->reqOpcode = pMsg->method;
            pRsp->handle = errHandle;
            pRsp->errCode = status;
            VOID ATT_ErrorRsp( pMsg->connHandle, pRsp );
        }
    }

    // Notify GATT that a message has been processed
    // Note: This call is optional if flow control is not used.
    GATT_AppCompletedMsg( pMsg );

    // if app task ask the gatt message, copy and send to app task
    if(s_GATTServCB)
        s_GATTServCB(pMsg);
}

/*********************************************************************
    @fn          gattServApp_ProcessExchangeMTUReq

    @brief       Process Exchange MTU Request.

    @param       pMsg - pointer to received message

    @return      Success
*/
static bStatus_t gattServApp_ProcessExchangeMTUReq( gattMsgEvent_t* pMsg )
{
    attExchangeMTURsp_t* pRsp = &rsp.exchangeMTURsp;
    // ATT_MTU shall be set to the minimum of the Client Rx MTU and Server Rx MTU values
    // Set the Server Rx MTU parameter to the maximum MTU that this server can receive
    #if defined ( TESTMODES )

    if ( paramValue == GATT_TESTMODE_MAX_MTU_SIZE )
    {
        pRsp->serverRxMTU = ATT_MAX_MTU_SIZE;
    }
    else
    #endif
        pRsp->serverRxMTU = g_ATT_MTU_SIZE_MAX;//ATT_MTU_SIZE;

    // Send response back
    VOID ATT_ExchangeMTURsp( pMsg->connHandle, pRsp );
    return ( SUCCESS );
}

/*********************************************************************
    @fn          gattServApp_ProcessFindByTypeValueReq

    @brief       Process Find By Type Value Request.

    @param       pMsg - pointer to received message
    @param       pErrHandle - attribute handle that generates an error

    @return      Success or Failure
*/
static bStatus_t gattServApp_ProcessFindByTypeValueReq( gattMsgEvent_t* pMsg, uint16* pErrHandle )
{
    attFindByTypeValueReq_t* pReq = &pMsg->msg.findByTypeValueReq;
    attFindByTypeValueRsp_t* pRsp = &rsp.findByTypeValueRsp;
    gattAttribute_t* pAttr;
    uint16 service;
    // Initialize the response
    VOID osal_memset( pRsp, 0, sizeof( attFindByTypeValueRsp_t ) );
    // Only attributes with attribute handles between and including the Starting
    // Handle parameter and the Ending Handle parameter that match the requested
    // attribute type and the attribute value will be returned.
    // All attribute types are effectively compared as 128-bit UUIDs,
    // even if a 16-bit UUID is provided in this request or defined
    // for an attribute.
    pAttr = GATT_FindHandleUUID( pReq->startHandle, pReq->endHandle,
                                 pReq->type.uuid, pReq->type.len, &service );

    while ( ( pAttr != NULL ) && ( pRsp->numInfo < g_ATT_MAX_NUM_HANDLES_INFO ) )
    {
        uint16 grpEndHandle;

        // It is not possible to use this request on an attribute that has a value
        // that is longer than (ATT_MTU - 7).
        if ( GATTServApp_ReadAttr( pMsg->connHandle, pAttr, service, attrValue,
                                   &attrLen, 0, ((gAttMtuSize[pMsg->connHandle])-7) ) == SUCCESS )
        {
            // Attribute values should be compared in terms of length and binary representation.
            if ( ( pReq->len == attrLen ) && osal_memcmp( pReq->value, attrValue, attrLen) )
            {
                // New attribute found
                // Set the Found Handle to the attribute that has the exact attribute
                // type and attribute value from the request.
                pRsp->handlesInfo[pRsp->numInfo].handle = pAttr->handle;
            }
        }

        // Try to find the next attribute
        pAttr = GATT_FindNextAttr( pAttr, pReq->endHandle, service, &grpEndHandle );

        // Set Group End Handle
        if ( pRsp->handlesInfo[pRsp->numInfo].handle != 0 )
        {
            // If the attribute type is a grouping attribute, the Group End Handle
            // shall be defined by that higher layer specification. If the attribute
            // type is not a grouping attribute, the Group End Handle shall be equal
            // to the Found Attribute Handle.
            if ( pAttr != NULL )
            {
                pRsp->handlesInfo[pRsp->numInfo++].grpEndHandle = grpEndHandle;
            }
            else
            {
                // If no other attributes with the same attribute type exist after the
                // Found Attribute Handle, the Group End Handle shall be set to 0xFFFF.
                pRsp->handlesInfo[pRsp->numInfo++].grpEndHandle = GATT_MAX_HANDLE;
            }
        }
    } // while

    if ( pRsp->numInfo > 0 )
    {
        // Send a response back
        VOID ATT_FindByTypeValueRsp( pMsg->connHandle, pRsp );
        return ( SUCCESS );
    }

    *pErrHandle = pReq->startHandle;
    return ( ATT_ERR_ATTR_NOT_FOUND );
}

/*********************************************************************
    @fn          gattServApp_ProcessReadByTypeReq

    @brief       Process Read By Type Request.

    @param       pMsg - pointer to received message
    @param       pErrHandle - attribute handle that generates an error

    @return      Success or Failure
*/
static bStatus_t gattServApp_ProcessReadByTypeReq( gattMsgEvent_t* pMsg, uint16* pErrHandle )
{
    attReadByTypeReq_t* pReq = &pMsg->msg.readByTypeReq;
    attReadByTypeRsp_t* pRsp = &rsp.readByTypeRsp;
    uint16 startHandle = pReq->startHandle;
    uint8 dataLen = 0;
    uint8 status = SUCCESS;

    // Only the attributes with attribute handles between and including the
    // Starting Handle and the Ending Handle with the attribute type that is
    // the same as the Attribute Type given will be returned.

    // Make sure there's enough room at least for an attribute handle (no value)
    while ( dataLen <= (gAttMtuSize[pMsg->connHandle]-4) )
    {
        uint16 service;
        gattAttribute_t* pAttr;
        // All attribute types are effectively compared as 128-bit UUIDs, even if
        // a 16-bit UUID is provided in this request or defined for an attribute.
        pAttr = GATT_FindHandleUUID( startHandle, pReq->endHandle, pReq->type.uuid,
                                     pReq->type.len, &service );

        if ( pAttr == NULL )
        {
            break; // No more attribute found
        }

        // Update start handle so it has the right value if we break from the loop
        startHandle = pAttr->handle;
        // Make sure the attribute has sufficient permissions to allow reading
        status = GATT_VerifyReadPermissions( pMsg->connHandle, pAttr->permissions );

        if ( status != SUCCESS )
        {
            break;
        }

        // Read the attribute value. If the attribute value is longer than
        // (ATT_MTU - 4) or 253 octets, whichever is smaller, then the first
        // (ATT_MTU - 4) or 253 octets shall be included in this response.
        status = GATTServApp_ReadAttr( pMsg->connHandle, pAttr, service, attrValue,
                                       &attrLen, 0, (((gAttMtuSize[pMsg->connHandle]))-4) );

        if ( status != SUCCESS )
        {
            break; // Cannot read the attribute value
        }

        // See if this is the first attribute found
        if ( dataLen == 0 )
        {
            // Use the length of the first attribute value for the length field
            pRsp->len = 2 + attrLen;
        }
        else
        {
            // If the attributes have attribute values that have the same length
            // then these attributes can all be read in a single request.
            if ( pRsp->len != 2 + attrLen )
            {
                break;
            }
        }

        // Make sure there's enough room for this attribute handle and value
        if ( dataLen + attrLen > (((gAttMtuSize[pMsg->connHandle]))-4) )
        {
            break;
        }

        // Add the handle value pair to the response
        pRsp->dataList[dataLen++] = LO_UINT16( pAttr->handle );
        pRsp->dataList[dataLen++] = HI_UINT16( pAttr->handle );
        VOID osal_memcpy( &(pRsp->dataList[dataLen]), attrValue, attrLen );
        dataLen += attrLen;

        if ( startHandle == GATT_MAX_HANDLE )
        {
            break; // We're done
        }

        // Update start handle and search again
        startHandle++;
    } // while

    // See what to respond
    if ( dataLen > 0 )
    {
        // Set the number of attribute handle-value pairs found
        pRsp->numPairs = dataLen / pRsp->len;
        // Send a response back
        VOID ATT_ReadByTypeRsp( pMsg->connHandle, pRsp );
        return ( SUCCESS );
    }

    if ( status == SUCCESS )
    {
        // Attribute not found -- dataLen must be 0
        status = ATT_ERR_ATTR_NOT_FOUND;
    }

    *pErrHandle = startHandle;
    return ( status );
}

/*********************************************************************
    @fn          gattServApp_ProcessReadReq

    @brief       Process Read Request.

    @param       pMsg - pointer to received message
    @param       pErrHandle - attribute handle that generates an error

    @return      Success or Failure
*/
static bStatus_t gattServApp_ProcessReadReq( gattMsgEvent_t* pMsg, uint16* pErrHandle )
{
    attReadReq_t* pReq = &pMsg->msg.readReq;
    gattAttribute_t* pAttr;
    uint16 service;
    uint8 status;
    pAttr = GATT_FindHandle( pReq->handle, &service );

    if ( pAttr != NULL )
    {
        attReadRsp_t* pRsp = &rsp.readRsp;
        // Build and send a response back. If the attribute value is longer
        // than (ATT_MTU - 1) then (ATT_MTU - 1) octets shall be included
        // in this response.
        status = GATTServApp_ReadAttr( pMsg->connHandle, pAttr, service, pRsp->value,
                                       &pRsp->len, 0, (((gAttMtuSize[pMsg->connHandle]))-1) );

        if ( status == SUCCESS )
        {
            // Send a response back
            VOID ATT_ReadRsp( pMsg->connHandle, pRsp );
        }
    }
    else
    {
        status = ATT_ERR_INVALID_HANDLE;
    }

    if ( status != SUCCESS )
    {
        *pErrHandle = pReq->handle;
    }

    return ( status );
}

/*********************************************************************
    @fn          gattServApp_ProcessReadBlobReq

    @brief       Process Read Blob Request.

    @param       pMsg - pointer to received message
    @param       pErrHandle - attribute handle that generates an error

    @return      Success or Failure
*/
static bStatus_t gattServApp_ProcessReadBlobReq( gattMsgEvent_t* pMsg, uint16* pErrHandle )
{
    attReadBlobReq_t* pReq = &pMsg->msg.readBlobReq;
    gattAttribute_t* pAttr;
    uint16 service;
    uint8 status;
    pAttr = GATT_FindHandle( pReq->handle, &service );

    if ( pAttr != NULL )
    {
        attReadBlobRsp_t* pRsp = &rsp.readBlobRsp;
        // Read part attribute value. If the attribute value is longer than
        // (Value Offset + ATT_MTU - 1) then (ATT_MTU - 1) octets from Value
        // Offset shall be included in this response.
        status = GATTServApp_ReadAttr( pMsg->connHandle, pAttr, service, pRsp->value,
                                       &pRsp->len, pReq->offset, (((gAttMtuSize[pMsg->connHandle]))-1) );

        if ( status == SUCCESS )
        {
            // Send a response back
            VOID ATT_ReadBlobRsp( pMsg->connHandle, pRsp );
        }
    }
    else
    {
        status = ATT_ERR_INVALID_HANDLE;
    }

    if ( status != SUCCESS )
    {
        *pErrHandle = pReq->handle;
    }

    return ( status );
}

/*********************************************************************
    @fn          gattServApp_ProcessReadMultiReq

    @brief       Process Read Multiple Request.

    @param       pMsg - pointer to received message
    @param       pErrHandle - attribute handle that generates an error

    @return      Success or Failure
*/
static bStatus_t gattServApp_ProcessReadMultiReq( gattMsgEvent_t* pMsg, uint16* pErrHandle )
{
    attReadMultiReq_t* pReq = &pMsg->msg.readMultiReq;
    attReadMultiRsp_t* pRsp = &rsp.readMultiRsp;
    uint8 status = SUCCESS;
    pRsp->len = 0;

    for ( uint8 i = 0; ( i < pReq->numHandles ) && ( pRsp->len < (((gAttMtuSize[pMsg->connHandle]))-1) ); i++ )
    {
        gattAttribute_t* pAttr;
        uint16 service;
        pAttr = GATT_FindHandle( pReq->handle[i], &service );

        if ( pAttr == NULL )
        {
            // Should never get here!
            status = ATT_ERR_INVALID_HANDLE;
            // The handle of the first attribute causing the error
            *pErrHandle = pReq->handle[i];
            break;
        }

        // If the Set Of Values parameter is longer than (ATT_MTU - 1) then only
        // the first (ATT_MTU - 1) octets shall be included in this response.
        status = GATTServApp_ReadAttr( pMsg->connHandle, pAttr, service, attrValue,
                                       &attrLen, 0, (((gAttMtuSize[pMsg->connHandle]))-1) );

        if ( status != SUCCESS )
        {
            // The handle of the first attribute causing the error
            *pErrHandle = pReq->handle[i];
            break;
        }

        // Make sure there's enough room in the response for this attribute value
        if ( pRsp->len + attrLen > (((gAttMtuSize[pMsg->connHandle]))-1) )
        {
            attrLen = (((gAttMtuSize[pMsg->connHandle]))-1) - pRsp->len;
        }

        // Append this value to the end of the response
        VOID osal_memcpy( &(pRsp->values[pRsp->len]), attrValue, attrLen );
        pRsp->len += attrLen;
    }

    if ( status == SUCCESS )
    {
        // Send a response back
        VOID ATT_ReadMultiRsp( pMsg->connHandle, pRsp );
    }

    return ( status );
}

/*********************************************************************
    @fn          gattServApp_ProcessReadByGrpTypeReq

    @brief       Process Read By Group Type Request.

    @param       pMsg - pointer to received message
    @param       pErrHandle - attribute handle that generates an error

    @return      Success or Failure
*/
static bStatus_t gattServApp_ProcessReadByGrpTypeReq( gattMsgEvent_t* pMsg, uint16* pErrHandle )
{
    attReadByGrpTypeReq_t* pReq = &pMsg->msg.readByGrpTypeReq;
    attReadByGrpTypeRsp_t* pRsp = &rsp.readByGrpTypeRsp;
    uint16 service;
    gattAttribute_t* pAttr;
    uint16 dataLen = 0;
    uint8 status = SUCCESS;
    // Only the attributes with attribute handles between and including the
    // Starting Handle and the Ending Handle with the attribute type that is
    // the same as the Attribute Type given will be returned.
    // All attribute types are effectively compared as 128-bit UUIDs,
    // even if a 16-bit UUID is provided in this request or defined
    // for an attribute.
    pAttr = GATT_FindHandleUUID( pReq->startHandle, pReq->endHandle,
                                 pReq->type.uuid, pReq->type.len, &service );

    while ( pAttr != NULL )
    {
        uint16 endGrpHandle;
        // The service, include and characteristic declarations are readable and
        // require no authentication or authorization, therefore insufficient
        // authentication or read not permitted errors shall not occur.
        status = GATT_VerifyReadPermissions( pMsg->connHandle, pAttr->permissions );

        if ( status != SUCCESS )
        {
            *pErrHandle = pAttr->handle;
            break;
        }

        // Read the attribute value. If the attribute value is longer than
        // (ATT_MTU - 6) or 251 octets, whichever is smaller, then the first
        // (ATT_MTU - 6) or 251 octets shall be included in this response.
        status = GATTServApp_ReadAttr( pMsg->connHandle, pAttr, service, attrValue,
                                       &attrLen, 0, (((gAttMtuSize[pMsg->connHandle]))-6) );

        if ( status != SUCCESS )
        {
            // Cannot read the attribute value
            *pErrHandle = pAttr->handle;
            break;
        }

        // See if this is the first attribute found
        if ( dataLen == 0 )
        {
            // Use the length of the first attribute value for the length field
            pRsp->len = 2 + 2 + attrLen;
        }
        else
        {
            // If the attributes have attribute values that have the same length
            // then these attributes can all be read in a single request.
            if ( pRsp->len != 2 + 2 + attrLen )
            {
                break; // We're done here
            }

            // Make sure there's enough room for this attribute handle, end group handle and value
            if ( dataLen + attrLen > (((gAttMtuSize[pMsg->connHandle]))-6) )
            {
                break; // We're done here
            }
        }

        // Add Attribute Handle to the response
        pRsp->dataList[dataLen++] = LO_UINT16( pAttr->handle );
        pRsp->dataList[dataLen++] = HI_UINT16( pAttr->handle );
        // Try to find the next attribute
        pAttr = GATT_FindNextAttr( pAttr, pReq->endHandle, service, &endGrpHandle );

        // Add End Group Handle to the response
        if ( pAttr != NULL )
        {
            // The End Group Handle is the handle of the last attribute within the
            // service definition
            pRsp->dataList[dataLen++] = LO_UINT16( endGrpHandle );
            pRsp->dataList[dataLen++] = HI_UINT16( endGrpHandle );
        }
        else
        {
            // The ending handle of the last service can be 0xFFFF
            pRsp->dataList[dataLen++] = LO_UINT16( GATT_MAX_HANDLE );
            pRsp->dataList[dataLen++] = HI_UINT16( GATT_MAX_HANDLE );
        }

        // Add Attribute Value to the response
        VOID osal_memcpy( &(pRsp->dataList[dataLen]), attrValue, attrLen );
        dataLen += attrLen;
    } // while

    // See what to respond
    if ( dataLen > 0 )
    {
        // Set the number of attribute handle, end group handle and value sets found
        pRsp->numGrps = dataLen / pRsp->len;
        // Send a response back
        VOID ATT_ReadByGrpTypeRsp( pMsg->connHandle, pRsp );
        return ( SUCCESS );
    }

    if ( status == SUCCESS )
    {
        // No grouping attribute found -- dataLen must be 0
        status = ATT_ERR_ATTR_NOT_FOUND;
    }

    *pErrHandle = pReq->startHandle;
    return ( status );
}

/*********************************************************************
    @fn          gattServApp_ProcessWriteReq

    @brief       Process Write Request or Command.

    @param       pMsg - pointer to received message
    @param       pErrHandle - attribute handle that generates an error

    @return      Success or Failure
*/
static bStatus_t gattServApp_ProcessWriteReq( gattMsgEvent_t* pMsg, uint16* pErrHandle )
{
    attWriteReq_t* pReq = &(pMsg->msg.writeReq);
    gattAttribute_t* pAttr;
    uint16 service;
    uint8 status = SUCCESS;
    // No Error Response or Write Response shall be sent in response to Write
    // Command. If the server cannot write this attribute for any reason the
    // command shall be ignored.
    pAttr = GATT_FindHandle( pReq->handle, &service );

    if ( pAttr != NULL )
    {
        // Authorization is handled by the application/profile
        if ( gattPermitAuthorWrite( pAttr->permissions ) )
        {
            // Use Service's authorization callback to authorize the request
            pfnGATTAuthorizeAttrCB_t pfnCB = gattServApp_FindAuthorizeAttrCB( service );

            if ( pfnCB != NULL )
            {
                status = (*pfnCB)( pMsg->connHandle, pAttr, ATT_WRITE_REQ );
            }
            else
            {
                status = ATT_ERR_UNLIKELY;
            }
        }

        // If everything is fine then try to write the new value
        if ( status == SUCCESS )
        {
            // Use Service's write callback to write the request
            status = GATTServApp_WriteAttr( pMsg->connHandle, pReq->handle,
                                            pReq->value, pReq->len, 0 );

            if ( ( status == SUCCESS ) && ( pReq->cmd == FALSE ) )
            {
                // Send a response back
                //VOID ATT_WriteRsp( pMsg->connHandle );
                uint8 st=ATT_WriteRsp( pMsg->connHandle );
//        if(st)
//        {
//            AT_LOG("[ATT_RSP ERR] %x %x\n",st,l2capSegmentPkt.fragment);
//        }
            }
        }
    }
    else
    {
        status = ATT_ERR_INVALID_HANDLE;
    }

    if ( status != SUCCESS )
    {
        *pErrHandle = pReq->handle;
    }

    return ( pReq->cmd ? SUCCESS : status );
}

/*********************************************************************
    @fn          gattServApp_ProcessPrepareWriteReq

    @brief       Process Prepare Write Request.

    @param       pMsg - pointer to received message
    @param       pErrHandle - attribute handle that generates an error

    @return      Success or Failure
*/
static bStatus_t gattServApp_ProcessPrepareWriteReq( gattMsgEvent_t* pMsg, uint16* pErrHandle )
{
    attPrepareWriteReq_t* pReq = &pMsg->msg.prepareWriteReq;
    gattAttribute_t* pAttr;
    uint16 service;
    uint8 status = SUCCESS;
    pAttr = GATT_FindHandle( pReq->handle, &service );

    if ( pAttr != NULL )
    {
        // Authorization is handled by the application/profile
        if ( gattPermitAuthorWrite( pAttr->permissions ) )
        {
            // Use Service's authorization callback to authorize the request
            pfnGATTAuthorizeAttrCB_t pfnCB = gattServApp_FindAuthorizeAttrCB( service );

            if ( pfnCB != NULL )
            {
                status = (*pfnCB)( pMsg->connHandle, pAttr, ATT_WRITE_REQ );
            }
            else
            {
                status = ATT_ERR_UNLIKELY;
            }
        }

        if ( status == SUCCESS )
        {
            #if defined ( TESTMODES )

            if ( paramValue == GATT_TESTMODE_CORRUPT_PW_DATA )
            {
                pReq->value[0] = ~(pReq->value[0]);
            }

            #endif
            // Enqueue the request for now
            status = gattServApp_EnqueuePrepareWriteReq( pMsg->connHandle, pReq );

            if ( status == SUCCESS )
            {
                //LOG("pre off[%d] len[%d]\n", pReq->offset, pReq->len);
                // Send a response back
                VOID ATT_PrepareWriteRsp( pMsg->connHandle, (attPrepareWriteRsp_t*)pReq );
            }
        }
    }
    else
    {
        status = ATT_ERR_INVALID_HANDLE;
    }

    if ( status != SUCCESS )
    {
        *pErrHandle = pReq->handle;
    }

    return ( status );
}

/*********************************************************************
    @fn          gattServApp_ProcessExecuteWriteReq

    @brief       Process Execute Write Request.

    @param       pMsg - pointer to received message
    @param       pErrHandle - attribute handle that generates an error

    @return      Success or Failure
*/
static bStatus_t gattServApp_ProcessExecuteWriteReq( gattMsgEvent_t* pMsg, uint16* pErrHandle )
{
    attExecuteWriteReq_t* pReq = &pMsg->msg.executeWriteReq;
    prepareWrites_t* pQueue;
    uint8 status = SUCCESS;
    // See if this client has a prepare write queue
    pQueue = gattServApp_FindPrepareWriteQ( pMsg->connHandle );

    if ( pQueue != NULL )
    {
        for ( uint8 i = 0; i < maxNumPrepareWrites; i++ )
        {
            attPrepareWriteReq_t* pWriteReq = &(pQueue->pPrepareWriteQ[i]);

            // See if there're any prepared write requests in the queue
            if ( pWriteReq->handle == GATT_INVALID_HANDLE )
            {
                break; // We're done
            }

            // Execute the request
            if ( pReq->flags == ATT_WRITE_PREPARED_VALUES )
            {
                status = GATTServApp_WriteAttr( pMsg->connHandle, pWriteReq->handle,
                                                pWriteReq->value, pWriteReq->len,
                                                pWriteReq->offset );

                // If the prepare write requests can not be written, the queue shall
                // be cleared and then an Error Response shall be sent with a high
                // layer defined error code.
                if ( status != SUCCESS )
                {
                    // Cancel the remaining prepared writes
                    pReq->flags = ATT_CANCEL_PREPARED_WRITES;
                    // The Attribute Handle in Error shall be set to the attribute handle
                    // of the attribute from the prepare write queue that caused this
                    // application error
                    *pErrHandle = pWriteReq->handle;
                }
            }
            else // ATT_CANCEL_PREPARED_WRITES
            {
                // Cancel all prepared writes - just ignore the request
            }

            // Clear the queue item
            VOID osal_memset( pWriteReq, 0, sizeof( attPrepareWriteRsp_t ) );
            // Mark this item as empty
            pWriteReq->handle = GATT_INVALID_HANDLE;
        } // for loop

        // Mark this queue as empty
        pQueue->connHandle = INVALID_CONNHANDLE;
    }

    // Send a response back
    if ( status == SUCCESS )
    {
        VOID ATT_ExecuteWriteRsp( pMsg->connHandle );
    }

    return ( status );
}

/*********************************************************************
    @fn          gattServApp_EnqueuePrepareWriteReq

    @brief       Enqueue Prepare Write Request.

    @param       connHandle - connection packet was received on
    @param       pReq - pointer to request

    @return      Success or Failure
*/
static bStatus_t gattServApp_EnqueuePrepareWriteReq( uint16 connHandle, attPrepareWriteReq_t* pReq )
{
    prepareWrites_t* pQueue;
    // First see if there's queue already assocaited with this client
    pQueue = gattServApp_FindPrepareWriteQ( connHandle );

    if ( pQueue == NULL )
    {
        // Find a queue for this client
        pQueue = gattServApp_FindPrepareWriteQ( INVALID_CONNHANDLE );

        if ( pQueue != NULL )
        {
            pQueue->connHandle = connHandle;
        }
    }

    // If a queue is found for this client then enqueue the request
    if ( pQueue != NULL )
    {
        for ( uint8 i = 0; i < maxNumPrepareWrites; i++ )
        {
            if ( pQueue->pPrepareWriteQ[i].handle == GATT_INVALID_HANDLE )
            {
                // Store the request here
                VOID osal_memcpy( &(pQueue->pPrepareWriteQ[i]), pReq, sizeof ( attPrepareWriteReq_t ) );
                //LOG("enq off[%d]len[%d]\n", pReq->offset, pReq->len);
                return ( SUCCESS );
            }
        }
    }

    return ( ATT_ERR_PREPARE_QUEUE_FULL );
}

/*********************************************************************
    @fn          gattServApp_FindPrepareWriteQ

    @brief       Find client's queue.

    @param       connHandle - connection used by client

    @return      Pointer to queue. NULL, otherwise.
*/
static prepareWrites_t* gattServApp_FindPrepareWriteQ( uint16 connHandle )
{
    // First see if this client has already a queue
    for ( uint8 i = 0; i < MAX_NUM_LL_CONN; i++ )
    {
        if ( prepareWritesTbl[i].connHandle == connHandle )
        {
            // Queue found
            return ( &(prepareWritesTbl[i]) );
        }
    }

    return ( (prepareWrites_t*)NULL );
}

/*********************************************************************
    @fn          gattServApp_PrepareWriteQInUse

    @brief       Check to see if the prepare write queue is in use.

    @param       void

    @return      TRUE if queue in use. FALSE, otherwise.
*/
static uint8 gattServApp_PrepareWriteQInUse( void )
{
    // See if any prepare write queue is in use
    for ( uint8 i = 0; i < MAX_NUM_LL_CONN; i++ )
    {
        if ( prepareWritesTbl[i].connHandle != INVALID_CONNHANDLE )
        {
            for ( uint8 j = 0; j < maxNumPrepareWrites; j++ )
            {
                if ( prepareWritesTbl[i].pPrepareWriteQ[j].handle != GATT_INVALID_HANDLE )
                {
                    // Queue item is in use
                    return ( TRUE );
                }
            } // for
        }
    } // for

    return ( FALSE );
}

/*********************************************************************
    @fn      gattServApp_FindCharCfgItem

    @brief   Find the characteristic configuration for a given client.
            Uses the connection handle to search the charactersitic
            configuration table of a client.

    @param   connHandle - connection handle (0xFFFF for empty entry)
    @param   charCfgTbl - characteristic configuration table.

    @return  pointer to the found item. NULL, otherwise.
*/
static gattCharCfg_t* gattServApp_FindCharCfgItem( uint16 connHandle,
                                                   gattCharCfg_t* charCfgTbl )
{
    for ( uint8 i = 0; i < GATT_MAX_NUM_CONN; i++ )
    {
        if ( charCfgTbl[i].connHandle == connHandle )
        {
            // Entry found
            return ( &(charCfgTbl[i]) );
        }
    }

    return ( (gattCharCfg_t*)NULL );
}

/*********************************************************************
    @fn      gattServApp_FindReadAttrCB

    @brief   Find the Read Attribute CB function pointer for a given service.

    @param   handle - service attribute handle

    @return  pointer to the found CB. NULL, otherwise.
*/
static pfnGATTReadAttrCB_t gattServApp_FindReadAttrCB( uint16 handle )
{
    CONST gattServiceCBs_t* pCBs = gattServApp_FindServiceCBs( handle );
    return ( ( pCBs == NULL ) ? NULL : pCBs->pfnReadAttrCB );
}

/*********************************************************************
    @fn      gattServApp_FindWriteAttrCB

    @brief   Find the Write CB Attribute function pointer for a given service.

    @param   handle - service attribute handle

    @return  pointer to the found CB. NULL, otherwise.
*/
static pfnGATTWriteAttrCB_t gattServApp_FindWriteAttrCB( uint16 handle )
{
    CONST gattServiceCBs_t* pCBs = gattServApp_FindServiceCBs( handle );
    return ( ( pCBs == NULL ) ? NULL : pCBs->pfnWriteAttrCB );
}

/*********************************************************************
    @fn      gattServApp_FindAuthorizeAttrCB

    @brief   Find the Authorize Attribute CB function pointer for a given service.

    @param   handle - service attribute handle

    @return  pointer to the found CB. NULL, otherwise.
*/
static pfnGATTAuthorizeAttrCB_t gattServApp_FindAuthorizeAttrCB( uint16 handle )
{
    CONST gattServiceCBs_t* pCBs = gattServApp_FindServiceCBs( handle );
    return ( ( pCBs == NULL ) ? NULL : pCBs->pfnAuthorizeAttrCB );
}

/*********************************************************************
    @fn      gattServApp_ValidateWriteAttrCB

    @brief   Validate and/or Write attribute data

    @param   connHandle - connection message was received on
    @param   pAttr - pointer to attribute
    @param   pValue - pointer to data to be written
    @param   len - length of data
    @param   offset - offset of the first octet to be written

    @return  Success or Failure
*/
static bStatus_t gattServApp_WriteAttrCB( uint16 connHandle, gattAttribute_t* pAttr,
                                          uint8* pValue, uint16 len, uint16 offset )
{
    bStatus_t status = SUCCESS;

    if ( pAttr->type.len == ATT_BT_UUID_SIZE )
    {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

        switch ( uuid )
        {
        case GATT_CLIENT_CHAR_CFG_UUID:
            status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                     offset, GATT_CLIENT_CFG_INDICATE );
            break;

        default:
            // Should never get here!
            status = ATT_ERR_INVALID_HANDLE;
        }
    }
    else
    {
        // 128-bit UUID
        status = ATT_ERR_INVALID_HANDLE;
    }

    return ( status );
}

/*********************************************************************
    @fn          GATTServApp_ReadAttr

    @brief       Read an attribute. If the format of the attribute value
                is unknown to GATT Server, use the callback function
                provided by the Service.

    @param       connHandle - connection message was received on
    @param       pAttr - pointer to attribute
    @param       service - handle of owner service
    @param       pValue - pointer to data to be read
    @param       pLen - length of data to be read
    @param       offset - offset of the first octet to be read
    @param       maxLen - maximum length of data to be read

    @return      Success or Failure
*/
uint8 GATTServApp_ReadAttr( uint16 connHandle, gattAttribute_t* pAttr,
                            uint16 service, uint8* pValue, uint16* pLen,
                            uint16 offset, uint8 maxLen )
{
    uint8 useCB = FALSE;
    bStatus_t status = SUCCESS;

    // Authorization is handled by the application/profile
    if ( gattPermitAuthorRead( pAttr->permissions ) )
    {
        // Use Service's authorization callback to authorize the request
        pfnGATTAuthorizeAttrCB_t pfnCB = gattServApp_FindAuthorizeAttrCB( service );

        if ( pfnCB != NULL )
        {
            status = (*pfnCB)( connHandle, pAttr, ATT_READ_REQ );
        }
        else
        {
            status = ATT_ERR_UNLIKELY;
        }

        if ( status != SUCCESS )
        {
            // Read operation failed!
            return ( status );
        }
    }

    // Check the UUID length
    if ( pAttr->type.len == ATT_BT_UUID_SIZE )
    {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

        switch ( uuid )
        {
        case GATT_PRIMARY_SERVICE_UUID:
        case GATT_SECONDARY_SERVICE_UUID:

            // Make sure it's not a blob operation
            if ( offset == 0 )
            {
                gattAttrType_t* pType = (gattAttrType_t*)(pAttr->pValue);
                *pLen = pType->len;
                VOID osal_memcpy( pValue, pType->uuid, pType->len );
            }
            else
            {
                status = ATT_ERR_ATTR_NOT_LONG;
            }

            break;

        case GATT_CHARACTER_UUID:

            // Make sure it's not a blob operation
            if ( offset == 0 )
            {
                gattAttribute_t* pCharValue;
                // The Attribute Value of a Characteristic Declaration includes the
                // Characteristic Properties, Characteristic Value Attribute Handle
                // and UUID.
                *pLen = 1;
                pValue[0] = *pAttr->pValue; // Properties
                // The Characteristic Value Attribute exists immediately following
                // the Characteristic Declaration.
                pCharValue = GATT_FindHandle( pAttr->handle+1, NULL );

                if ( pCharValue != NULL )
                {
                    // It can be a 128-bit UUID
                    *pLen += (2 + pCharValue->type.len);
                    // Attribute Handle
                    pValue[1] = LO_UINT16( pCharValue->handle );
                    pValue[2] = HI_UINT16( pCharValue->handle );
                    // Attribute UUID
                    VOID osal_memcpy( &(pValue[3]), pCharValue->type.uuid, pCharValue->type.len );
                }
                else
                {
                    // Should never get here!
                    *pLen += (2 + ATT_BT_UUID_SIZE);
                    // Set both Attribute Handle and UUID to 0
                    VOID osal_memset( &(pValue[1]), 0, (2 + ATT_BT_UUID_SIZE) );
                }
            }
            else
            {
                status = ATT_ERR_ATTR_NOT_LONG;
            }

            break;

        case GATT_INCLUDE_UUID:

            // Make sure it's not a blob operation
            if ( offset == 0 )
            {
                uint16 servHandle;
                uint16 endGrpHandle;
                gattAttribute_t* pIncluded;
                uint16 handle = *((uint16*)(pAttr->pValue));
                // The Attribute Value of an Include Declaration is set the
                // included service Attribute Handle, the End Group Handle,
                // and the service UUID. The Service UUID shall only be present
                // when the UUID is a 16-bit Bluetooth UUID.
                *pLen = 4;
                pValue[0] = LO_UINT16( handle );
                pValue[1] = HI_UINT16( handle );
                // Find the included service attribute record
                pIncluded = GATT_FindHandle( handle, &servHandle );

                if ( pIncluded != NULL )
                {
                    gattAttrType_t* pServiceUUID = (gattAttrType_t*)pIncluded->pValue;

                    // Find out the End Group handle
                    if ( ( GATT_FindNextAttr( pIncluded, GATT_MAX_HANDLE,
                                              servHandle, &endGrpHandle ) == NULL ) &&
                            ( !gattSecondaryServiceType( pIncluded->type ) ) )
                    {
                        // The ending handle of the last service can be 0xFFFF
                        endGrpHandle = GATT_MAX_HANDLE;
                    }

                    // Include only 16-bit Service UUID
                    if ( pServiceUUID->len == ATT_BT_UUID_SIZE )
                    {
                        VOID osal_memcpy( &(pValue[4]), pServiceUUID->uuid, ATT_BT_UUID_SIZE );
                        *pLen += ATT_BT_UUID_SIZE;
                    }
                }
                else
                {
                    // Should never get here!
                    endGrpHandle = handle;
                }

                // End Group Handle
                pValue[2] = LO_UINT16( endGrpHandle );
                pValue[3] = HI_UINT16( endGrpHandle );
            }
            else
            {
                status = ATT_ERR_ATTR_NOT_LONG;
            }

            break;

        case GATT_CLIENT_CHAR_CFG_UUID:

            // Make sure it's not a blob operation
            if ( offset == 0 )
            {
                uint16 value = GATTServApp_ReadCharCfg( connHandle,
                                                        (gattCharCfg_t*)(pAttr->pValue) );
                *pLen = 2;
                pValue[0] = LO_UINT16( value );
                pValue[1] = HI_UINT16( value );
            }
            else
            {
                status = ATT_ERR_ATTR_NOT_LONG;
            }

            break;

        case GATT_CHAR_EXT_PROPS_UUID:
        case GATT_SERV_CHAR_CFG_UUID:

            // Make sure it's not a blob operation
            if ( offset == 0 )
            {
                uint16 value = *((uint16*)(pAttr->pValue));
                *pLen = 2;
                pValue[0] = LO_UINT16( value );
                pValue[1] = HI_UINT16( value );
            }
            else
            {
                status = ATT_ERR_ATTR_NOT_LONG;
            }

            break;

        case GATT_CHAR_USER_DESC_UUID:
        {
            uint8 len = osal_strlen( (char*)(pAttr->pValue) );  // Could be a long attribute

            // If the value offset of the Read Blob Request is greater than the
            // length of the attribute value, an Error Response shall be sent with
            // the error code Invalid Offset.
            if ( offset <= len )
            {
                // If the value offset is equal than the length of the attribute
                // value, then the length of the part attribute value shall be zero.
                if ( offset == len )
                {
                    len = 0;
                }
                else
                {
                    // If the attribute value is longer than (Value Offset + maxLen)
                    // then maxLen octets from Value Offset shall be included in
                    // this response.
                    if ( len > ( offset + maxLen ) )
                    {
                        len = maxLen;
                    }
                    else
                    {
                        len -= offset;
                    }
                }

                *pLen = len;
                VOID osal_memcpy( pValue, &(pAttr->pValue[offset]), len );
            }
            else
            {
                status = ATT_ERR_INVALID_OFFSET;
            }
        }
        break;

        case GATT_CHAR_FORMAT_UUID:

            // Make sure it's not a blob operation
            if ( offset == 0 )
            {
                gattCharFormat_t* pFormat = (gattCharFormat_t*)(pAttr->pValue);
                *pLen = 7;
                pValue[0] = pFormat->format;
                pValue[1] = pFormat->exponent;
                pValue[2] = LO_UINT16( pFormat->unit );
                pValue[3] = HI_UINT16( pFormat->unit );
                pValue[4] = pFormat->nameSpace;
                pValue[5] = LO_UINT16( pFormat->desc );
                pValue[6] = HI_UINT16( pFormat->desc );
            }
            else
            {
                status = ATT_ERR_ATTR_NOT_LONG;
            }

            break;

        default:
            useCB = TRUE;
            break;
        }
    }
    else
    {
        useCB = TRUE;
    }

    if ( useCB == TRUE )
    {
        // Use Service's read callback to process the request
        pfnGATTReadAttrCB_t pfnCB = gattServApp_FindReadAttrCB( service );

        if ( pfnCB != NULL )
        {
            // Read the attribute value
            status = (*pfnCB)( connHandle, pAttr, pValue, pLen, offset, maxLen );
        }
        else
        {
            status = ATT_ERR_UNLIKELY;
        }
    }

    return ( status );
}

/*********************************************************************
    @fn      GATTServApp_WriteAttr

    @brief   Write attribute data

    @param   connHandle - connection message was received on
    @param   handle - attribute handle
    @param   pValue - pointer to data to be written
    @param   len - length of data
    @param   offset - offset of the first octet to be written

    @return  Success or Failure
*/
uint8 GATTServApp_WriteAttr( uint16 connHandle, uint16 handle,
                             uint8* pValue, uint16 len, uint16 offset )
{
    uint16 service;
    gattAttribute_t* pAttr;
    bStatus_t status;
    // Find the owner of the attribute
    pAttr = GATT_FindHandle( handle, &service );

    if ( pAttr != NULL )
    {
        // Find out the owner's callback functions
        pfnGATTWriteAttrCB_t pfnCB = gattServApp_FindWriteAttrCB( service );

        if ( pfnCB != NULL )
        {
            // Try to write the new value
            status = (*pfnCB)( connHandle, pAttr, pValue, len, offset );
        }
        else
        {
            status = ATT_ERR_UNLIKELY;
        }
    }
    else
    {
        status = ATT_ERR_INVALID_HANDLE;
    }

    return ( status );
}

/*********************************************************************
    @fn      GATTServApp_SetParamValue

    @brief   Set a GATT Server Application Parameter value. Use this
            function to change the default GATT parameter values.

    @param   value - new param value

    @return  void
*/
void GATTServApp_SetParamValue( uint16 value )
{
    #if defined ( TESTMODES )
    paramValue = value;
    #else
    VOID value;
    #endif
}

/*********************************************************************
    @fn      GATTServApp_GetParamValue

    @brief   Get a GATT Server Application Parameter value.

    @param   none

    @return  GATT Parameter value
*/
uint16 GATTServApp_GetParamValue( void )
{
    #if defined ( TESTMODES )
    return ( paramValue );
    #else
    return ( 0 );
    #endif
}

/*********************************************************************
    @fn      GATTServApp_UpdateCharCfg

    @brief   Update the Client Characteristic Configuration for a given
            Client.

            Note: This API should only be called from the Bond Manager.

    @param   connHandle - connection handle.
    @param   attrHandle - attribute handle.
    @param   value - characteristic configuration value (from NV).

    @return  Success or Failure
*/
bStatus_t GATTServApp_UpdateCharCfg( uint16 connHandle, uint16 attrHandle, uint16 value )
{
    uint8 buf[2];
    buf[0] = LO_UINT16( value );
    buf[1] = HI_UINT16( value );
    return ( GATTServApp_WriteAttr( connHandle, attrHandle, buf, 2, 0 ) );
}

/*********************************************************************
    @fn      GATTServApp_SendServiceChangedInd

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
bStatus_t GATTServApp_SendServiceChangedInd( uint16 connHandle, uint8 taskId )
{
    uint16 value = GATTServApp_ReadCharCfg( connHandle, indCharCfg );

    if ( value & GATT_CLIENT_CFG_INDICATE )
    {
        return ( GATT_ServiceChangedInd( connHandle, taskId ) );
    }

    return ( FAILURE );
}

/*********************************************************************
    @fn      GATTServApp_InitCharCfg

    @brief   Initialize the client characteristic configuration table.

            Note: Each client has its own instantiation of the Client
                  Characteristic Configuration. Reads/Writes of the Client
                  Characteristic Configuration only only affect the
                  configuration of that client.

    @param   connHandle - connection handle (0xFFFF for all connections).
    @param   charCfgTbl - client characteristic configuration table.

    @return  none
*/
void GATTServApp_InitCharCfg( uint16 connHandle, gattCharCfg_t* charCfgTbl )
{
    // Initialize Client Characteristic Configuration attributes
    if ( connHandle == INVALID_CONNHANDLE )
    {
        for ( uint8 i = 0; i < GATT_MAX_NUM_CONN; i++ )
        {
            charCfgTbl[i].connHandle = INVALID_CONNHANDLE;
            charCfgTbl[i].value = GATT_CFG_NO_OPERATION;
        }
    }
    else
    {
        gattCharCfg_t* pItem = gattServApp_FindCharCfgItem( connHandle, charCfgTbl );

        if ( pItem != NULL )
        {
            pItem->connHandle = INVALID_CONNHANDLE;
            pItem->value = GATT_CFG_NO_OPERATION;
        }
    }
}

/*********************************************************************
    @fn      GATTServApp_ReadCharCfg

    @brief   Read the client characteristic configuration for a given
            client.

            Note: Each client has its own instantiation of the Client
                  Characteristic Configuration. Reads of the Client
                  Characteristic Configuration only shows the configuration
                  for that client.

    @param   connHandle - connection handle.
    @param   charCfgTbl - client characteristic configuration table.

    @return  attribute value
*/
uint16 GATTServApp_ReadCharCfg( uint16 connHandle, gattCharCfg_t* charCfgTbl )
{
    gattCharCfg_t* pItem;
    pItem = gattServApp_FindCharCfgItem( connHandle, charCfgTbl );

    if ( pItem != NULL )
    {
        return ( (uint16)(pItem->value) );
    }

    return ( (uint16)GATT_CFG_NO_OPERATION );
}

/*********************************************************************
    @fn      GATTServApp_WriteCharCfg

    @brief   Write the client characteristic configuration for a given
            client.

            Note: Each client has its own instantiation of the Client
                  Characteristic Configuration. Writes of the Client
                  Characteristic Configuration only only affect the
                  configuration of that client.

    @param   connHandle - connection handle.
    @param   charCfgTbl - client characteristic configuration table.
    @param   value - attribute new value.

    @return  Success or Failure
*/
uint8 GATTServApp_WriteCharCfg( uint16 connHandle, gattCharCfg_t* charCfgTbl,
                                uint16 value )
{
    gattCharCfg_t* pItem;
    pItem = gattServApp_FindCharCfgItem( connHandle, charCfgTbl );

    if ( pItem == NULL )
    {
        pItem = gattServApp_FindCharCfgItem( INVALID_CONNHANDLE, charCfgTbl );

        if ( pItem == NULL )
        {
            return ( ATT_ERR_INSUFFICIENT_RESOURCES );
        }

        pItem->connHandle = connHandle;
    }

    // Write the new value for this client
    pItem->value = value;
    return ( SUCCESS );
}

/*********************************************************************
    @fn      GATTServApp_ProcessCCCWriteReq

    @brief   Process the client characteristic configuration
            write request for a given client.

    @param   connHandle - connection message was received on
    @param   pAttr - pointer to attribute
    @param   pValue - pointer to data to be written
    @param   len - length of data
    @param   offset - offset of the first octet to be written
    @param   validCfg - valid configuration

    @return  Success or Failure
*/
bStatus_t GATTServApp_ProcessCCCWriteReq( uint16 connHandle, gattAttribute_t* pAttr,
                                          uint8* pValue, uint8 len, uint16 offset,
                                          uint16 validCfg )
{
    bStatus_t status = SUCCESS;

    // Validate the value
    if ( offset == 0 )
    {
        if ( len == 2 )
        {
            uint16 value = BUILD_UINT16( pValue[0], pValue[1] );

            // Validate characteristic configuration bit field
            if ( ( value & ~validCfg ) == 0 ) // indicate and/or notify
            {
                // Write the value if it's changed
                if ( GATTServApp_ReadCharCfg( connHandle,
                                              (gattCharCfg_t*)(pAttr->pValue) ) != value )
                {
                    status = GATTServApp_WriteCharCfg( connHandle,
                                                       (gattCharCfg_t*)(pAttr->pValue),
                                                       value );

                    if ( status == SUCCESS )
                    {
                        // Notify the application
                        GATTServApp_SendCCCUpdatedEvent( connHandle, pAttr->handle, value );
                    }
                }
            }
            else
            {
                status = ATT_ERR_INVALID_VALUE;
            }
        }
        else
        {
            status = ATT_ERR_INVALID_VALUE_SIZE;
        }
    }
    else
    {
        status = ATT_ERR_ATTR_NOT_LONG;
    }

    return ( status );
}

/*********************************************************************
    @fn      GATTServApp_ProcessCharCfg

    @brief   Process Client Charateristic Configuration change.

    @param   charCfgTbl - characteristic configuration table.
    @param   pValue - pointer to attribute value.
    @param   authenticated - whether an authenticated link is required.
    @param   attrTbl - attribute table.
    @param   numAttrs - number of attributes in attribute table.
    @param   taskId - task to be notified of confirmation.

    @return  Success or Failure
*/
#include "log.h"
bStatus_t GATTServApp_ProcessCharCfg( gattCharCfg_t* charCfgTbl, uint8* pValue,
                                      uint8 authenticated, gattAttribute_t* attrTbl,
                                      uint16 numAttrs, uint8 taskId )
{
    bStatus_t status = SUCCESS;
//    for ( uint8 i = 0; i < GATT_MAX_NUM_CONN; i++ )
    {
//        gattCharCfg_t* pItem = &(charCfgTbl[i]);
        gattCharCfg_t* pItem = charCfgTbl;

        if ( ( pItem->connHandle != INVALID_CONNHANDLE ) &&
                ( pItem->value != GATT_CFG_NO_OPERATION ) )
        {
            gattAttribute_t* pAttr;
            // Find the characteristic value attribute
            pAttr = GATTServApp_FindAttr( attrTbl, numAttrs, pValue );

            if ( pAttr != NULL )
            {
                attHandleValueNoti_t noti;

                // If the attribute value is longer than (ATT_MTU - 3) octets, then
                // only the first (ATT_MTU - 3) octets of this attributes value can
                // be sent in a notification.
                if ( GATTServApp_ReadAttr( pItem->connHandle, pAttr,
                                           GATT_SERVICE_HANDLE( attrTbl ), noti.value,
                                           &noti.len, 0, (((gAttMtuSize[pItem->connHandle]))-3) ) == SUCCESS )
                {
                    noti.handle = pAttr->handle;

                    if ( pItem->value & GATT_CLIENT_CFG_NOTIFY )
                    {
                        status |= GATT_Notification( pItem->connHandle, &noti, authenticated );
                    }

                    if ( pItem->value & GATT_CLIENT_CFG_INDICATE )
                    {
                        status |= GATT_Indication( pItem->connHandle, (attHandleValueInd_t*)&noti,
                                                   authenticated, taskId );
                    }
                }
            }
        }
    } // for
    return ( status );
}

/*********************************************************************
    @fn          gattServApp_HandleConnStatusCB

    @brief       GATT Server Application link status change handler function.

    @param       connHandle - connection handle
    @param       changeType - type of change

    @return      none
*/
static void gattServApp_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{
    // Check to see if the connection has dropped
    if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )      ||
            ( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) &&
              ( !linkDB_Up( connHandle ) ) ) )
    {
        prepareWrites_t* pQueue = gattServApp_FindPrepareWriteQ( connHandle );

        // See if this client has a prepare write queue
        if ( pQueue != NULL )
        {
            for ( uint8 i = 0; i < maxNumPrepareWrites; i++ )
            {
                attPrepareWriteReq_t* pWriteReq = &(pQueue->pPrepareWriteQ[i]);

                // See if there're any prepared write requests in the queue
                if ( pWriteReq->handle == GATT_INVALID_HANDLE )
                {
                    break;
                }

                // Clear the queue item
                VOID osal_memset( pWriteReq, 0, sizeof( attPrepareWriteRsp_t ) );
            } // for loop

            // Mark this queue as empty
            pQueue->connHandle = INVALID_CONNHANDLE;
        }

        // Reset Client Char Config when connection drops
        GATTServApp_InitCharCfg( connHandle, indCharCfg );
    }
}

/*********************************************************************
    @fn      GATTServApp_SendCCCUpdatedEvent

    @brief   Build and send the GATT_CLIENT_CHAR_CFG_UPDATED_EVENT to
            the app.

    @param   connHandle - connection handle
    @param   attrHandle - attribute handle
    @param   value - attribute new value

    @return  none
*/
void GATTServApp_SendCCCUpdatedEvent( uint16 connHandle, uint16 attrHandle, uint16 value )
{
    if ( appTaskID != INVALID_TASK_ID )
    {
        // Allocate, build and send event
        gattClientCharCfgUpdatedEvent_t* pEvent =
            (gattClientCharCfgUpdatedEvent_t*)osal_msg_allocate( (uint16)(sizeof ( gattClientCharCfgUpdatedEvent_t )) );

        if ( pEvent )
        {
            pEvent->hdr.event = GATT_SERV_MSG_EVENT;
            pEvent->hdr.status = SUCCESS;
            pEvent->method = GATT_CLIENT_CHAR_CFG_UPDATED_EVENT;
            pEvent->connHandle = connHandle;
            pEvent->attrHandle = attrHandle;
            pEvent->value = value;
            VOID osal_msg_send( appTaskID, (uint8*)pEvent );
        }
    }
}

bStatus_t gattServApp_RegisterCB(gattServMsgCB_t cb)
{
    s_GATTServCB = cb;
    return SUCCESS;
}


#endif // ( CENTRAL_CFG | PERIPHERAL_CFG )

/****************************************************************************
****************************************************************************/
