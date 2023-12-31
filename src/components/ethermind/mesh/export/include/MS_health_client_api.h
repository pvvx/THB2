/**
    \file MS_health_client_api.h

    \brief This file defines the Mesh Health Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_HEALTH_CLIENT_API_
#define _H_MS_HEALTH_CLIENT_API_


/* --------------------------------------------- Header File Inclusion */
#include "MS_access_api.h"


/* --------------------------------------------- Global Definitions */
/**
    \defgroup health_module HEALTH (Mesh Health Model)
    \{
    This section describes the interfaces & APIs offered by the EtherMind
    Mesh Health Model (HEALTH) module to the Application.
*/



/* --------------------------------------------- Data Types/ Structures */
/**
    \defgroup health_cb Application Callback
    \{
    This Section Describes the module Notification Callback interface offered
    to the application
*/

/**
    Health Client application Asynchronous Notification Callback.

    Health Client calls the registered callback to indicate events occurred to the
    application.

    \param handle        Model Handle.
    \param opcode        Opcode.
    \param data_param    Data associated with the event if any or NULL.
    \param data_len      Size of the event data. 0 if event data is NULL.
*/
typedef API_RESULT (* MS_HEALTH_CLIENT_CB)
(
    MS_ACCESS_MODEL_HANDLE* handle,
    UINT32                   opcode,
    UCHAR*                   data_param,
    UINT16                   data_len
) DECL_REENTRANT;
/** \} */

/**
    \defgroup health_structures Structures
    \{
*/

/**
    Health Status message parameters
*/
typedef struct MS_health_status_struct
{
    /** Identifier of a performed test */
    UCHAR  test_id;

    /** 16-bit Bluetooth assigned Company Identifier */
    UINT16 company_id;

    /** The FaultArray field contains a sequence of 1-octet fault values */
    UCHAR* faultarray;

    /** Number of fault values in the FaultArray */
    UINT16 faultarray_len;

} MS_HEALTH_STATUS_STRUCT;

/**
    Health Fault Get clear message parameters
*/
typedef struct MS_health_fault_get_clear_struct
{
    /** 16-bit Bluetooth assigned Company Identifier */
    UINT16 company_id;

} MS_HEALTH_FAULT_GET_CLEAR_STRUCT;

/**
    Health Fault Test message parameters
*/
typedef struct MS_health_fault_test_struct
{
    /** Identifier of a performed test */
    UCHAR  test_id;

    /** 16-bit Bluetooth assigned Company Identifier */
    UINT16 company_id;

} MS_HEALTH_FAULT_TEST_STRUCT;

/**
    Health Period message parameters
*/
typedef struct MS_health_period_struct
{
    /**
        Divider for the Publish Period.
        Modified Publish Period is used for sending Current Health Status messages
        when there are active faults to communicate.
    */
    UCHAR  fastperioddivisor;

} MS_HEALTH_PERIOD_STRUCT;

/**
    Health Attention message parameters
*/
typedef struct MS_health_attention_struct
{
    /** Value of the Attention Timer state */
    UCHAR  attention;

} MS_HEALTH_ATTENTION_STRUCT;

/** \} */



/* --------------------------------------------- Function */
/**
    \defgroup health_api_defs API Definitions
    \{
    This section describes the EtherMind Mesh Health Model APIs.
*/
/**
    \defgroup health_cli_api_defs Health Client API Definitions
    \{
    This section describes the Health Client APIs.
*/

/**
    \brief API to initialize Health Client model

    \par Description
    This is to initialize Health Client model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in] appl_cb    Application Callback to be used by the Health Client.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_health_client_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE    element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*     model_handle,
    /* IN */    MS_HEALTH_CLIENT_CB appl_cb
);

/**
    \brief API to get Health client model handle

    \par Description
    This is to get the handle of Health client model.

    \param [out] model_handle   Address of model handle to be filled/returned.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_health_client_get_model_handle
(
    /* OUT */ MS_ACCESS_MODEL_HANDLE*   model_handle
);

/**
    \brief API to send acknowledged commands

    \par Description
    This is to initialize sending acknowledged commands.

    \param [in] req_opcode    Request Opcode.
    \param [in] param         Parameter associated with Request Opcode.
    \param [in] rsp_opcode    Response Opcode.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_health_client_send_reliable_pdu
(
    /* IN */ UINT32    req_opcode,
    /* IN */ void*     param,
    /* IN */ UINT32    rsp_opcode
);

/**
    \brief API to report the registered fault state

    \par Description
    The Health Fault Get is an acknowledged message used to get the current
    Registered Fault state identified by Company ID of an element.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_HEALTH_FAULT_GET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_health_fault_get(param) \
    MS_health_client_send_reliable_pdu \
    (\
     MS_ACCESS_HEALTH_FAULT_GET_OPCODE,\
     param,\
     MS_ACCESS_HEALTH_FAULT_STATUS_OPCODE\
    )

/**
    \brief API to clear the registered fault state

    \par Description
    The Health Fault Clear Unacknowledged is an unacknowledged message used
    to clear the current Registered Fault state identified by Company ID of an element.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_HEALTH_FAULT_CLEAR_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_health_fault_clear_unacknowledged(param) \
    MS_health_client_send_reliable_pdu \
    (\
     MS_ACCESS_HEALTH_FAULT_CLEAR_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to clear the registered fault state

    \par Description
    The Health Fault Clear is an acknowledged message used to clear the
    current Registered Fault state identified by Company ID of an element.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_HEALTH_FAULT_CLEAR_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_health_fault_clear(param) \
    MS_health_client_send_reliable_pdu \
    (\
     MS_ACCESS_HEALTH_FAULT_CLEAR_OPCODE,\
     param,\
     MS_ACCESS_HEALTH_FAULT_STATUS_OPCODE\
    )

/**
    \brief API to invoke a self-test procedure

    \par Description
    The Health Fault Test is an acknowledged message used to invoke a self-test
    procedure of an element. The procedure is implementation specific and may
    result in changing the Health Fault state of an element.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_HEALTH_FAULT_TEST_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_health_fault_test(param) \
    MS_health_client_send_reliable_pdu \
    (\
     MS_ACCESS_HEALTH_FAULT_TEST_OPCODE,\
     param,\
     MS_ACCESS_HEALTH_FAULT_STATUS_OPCODE\
    )

/**
    \brief API to invoke a self-test procedure

    \par Description
    The Health Fault Test Unacknowledged is an unacknowledged message used
    to invoke a self-test procedure of an element. The procedure is implementation
    specific and may result in changing the Health Fault state of an element.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_health_fault_test_unacknowledged(param) \
    MS_health_client_send_reliable_pdu \
    (\
     MS_ACCESS_HEALTH_FAULT_TEST_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to get the health period state

    \par Description
    The Health Period Get is an acknowledged message used to get the
    current Health Period state of an element.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_health_period_get() \
    MS_health_client_send_reliable_pdu \
    (\
     MS_ACCESS_HEALTH_PERIOD_GET_OPCODE,\
     NULL,\
     MS_ACCESS_HEALTH_PERIOD_STATUS_OPCODE\
    )

/**
    \brief API to set the health period state

    \par Description
    The Health Period Set Unacknowledged is an unacknowledged message used
    to set the current Health Period state of an element.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_HEALTH_PERIOD_SET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_health_period_set_unacknowledged(param) \
    MS_health_client_send_reliable_pdu \
    (\
     MS_ACCESS_HEALTH_PERIOD_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )

/**
    \brief API to set the health period state

    \par Description
    The Health Period Set is an acknowledged message used to set the
    current Health Period state of an element.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_HEALTH_PERIOD_SET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_health_period_set(param) \
    MS_health_client_send_reliable_pdu \
    (\
     MS_ACCESS_HEALTH_PERIOD_SET_OPCODE,\
     param,\
     MS_ACCESS_HEALTH_PERIOD_STATUS_OPCODE\
    )

/**
    \brief API to get the attention state

    \par Description
    The Health Attention Get is an acknowledged message used to get
    the current Attention Timer state of an element.

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_health_attention_get() \
    MS_health_client_send_reliable_pdu \
    (\
     MS_ACCESS_HEALTH_ATTENTION_GET_OPCODE,\
     NULL,\
     MS_ACCESS_HEALTH_ATTENTION_STATUS_OPCODE\
    )

/**
    \brief API to set the attention state

    \par Description
    The Health Attention Set is an acknowledged message used to set the
    Attention Timer state of an element.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_HEALTH_ATTENTION_SET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_health_attention_set(param) \
    MS_health_client_send_reliable_pdu \
    (\
     MS_ACCESS_HEALTH_ATTENTION_SET_OPCODE,\
     param,\
     MS_ACCESS_HEALTH_ATTENTION_STATUS_OPCODE\
    )

/**
    \brief API to set the attention state

    \par Description
    The Health Attention Set Unacknowledged is an unacknowledged message
    used to set the Attention Timer state of an element.

    \param [in] param
           Pointer to the structure populated as in \ref ACCESS_HEALTH_ATTENTION_SET_PARAM

    \return API_SUCCESS or an error code indicating reason for failure
*/
#define MS_health_attention_set_unacknowledged(param) \
    MS_health_client_send_reliable_pdu \
    (\
     MS_ACCESS_HEALTH_ATTENTION_SET_UNACKNOWLEDGED_OPCODE,\
     param,\
     0xFFFFFFFF\
    )
/** \} */
/** \} */
/** \} */

#endif /* _H_MS_HEALTH_CLIENT_API_ */
