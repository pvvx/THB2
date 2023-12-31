
/**
    \file MS_error.h

    This file lists all the Error Codes returned by EtherMind
    Mesh APIs.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_ERROR_
#define _H_MS_ERROR_

/**
    \addtogroup ms_common_defines Defines
    \{
*/

/**
    \defgroup MS_ERROR_CODES Mesh Error Codes
    \{
    This section contains all the error codes defined for Mesh stack
    and profiles.
    <p>
    \anchor error_code_overview
    <b>Theory:</b>
    <p>
      Every API under Mesh Protocol Suite returns \ref API_RESULT,
      which is a 2 Byte Unsigned Short variable. The higher order byte
      signifies the Module from which the Error has been generated, and
      the lower order byte encodes the actual reason of Error.
      <p>
      Each module under Mesh Stack is given unique
      Error ID (the higher order byte). Also, for each module the Error
      Code (the lower order byte) signifies an unique error situation.
      <p>
      For Mesh Protocol Modules (eg, Transport, Network etc.), Error IDs are
      assigned from the range 0x10 to 0x4F. For Profiles, the range
      is from 0x50 to 0x7F.
      <p>
      The definition of \ref API_SUCCESS is 0x0000 - which is the 'Success'
      return value for an API returning \ref API_RESULT. All other values for
      should be treated as Errors.
      <p>
      The definition of \ref API_FAILURE is 0xFFFF - which stands for
      "Unknown Error Situation".
      <p>
    <b>Note:</b>
      <p>
      The applications using native/profile Mesh API should NOT do
      any check on the basis of \ref API_FAILURE - rather, the correct way to
      detect an error situation is by doing a negative check on \ref
      API_SUCCESS.
      <p>
      For example,
      \code if ( API_FAILURE == MS_access_register_model(x, y, z) ) \endcode
      ... Wrong !
      <p>
      <p>
      \code if ( API_SUCCESS != MS_access_register_model(x, y, z) ) \endcode
      ... Correct !
*/

/**
    \defgroup ms_error_codes_defines Defines
    \{
*/

/** Definition of API_RESULT */

#ifndef API_RESULT_DEFINED
    typedef UINT16    API_RESULT;
    #define API_RESULT_DEFINED
#endif /* API_RESULT_DEFINED */

/* Definitions of API_SUCCESS & API_FAILURE */
#ifdef API_SUCCESS
    #undef API_SUCCESS
#endif /* API_SUCCESS */
/** Status - 'Success' */
#define API_SUCCESS             0x0000

#ifdef API_FAILURE
    #undef API_FAILURE
#endif /* API_FAILURE */
/** Status - 'Failure' */
#define API_FAILURE             0xFFFF

/** \} */

/* ====================== EtherMind Module Error IDs ====================== */

/**
    \defgroup ms_error_codes_groups Error Grouping
    \{
*/

/**
    \defgroup ms_error_codes_groups_std Specification Error Codes (0x00 - 0x0F)
    Error IDs for Bluetooth Specification Defined Error Codes (0x00 - 0x0F).
    \{
*/

/** Error Codes for Mesh - \ref ms_error_codes_module_mesh */
#define MS_ERR_ID                               0x0000
/** \cond ignore */
/** \endcond */

/** \} */

/**
    \defgroup ms_error_codes_groups_core Core Modules (0x10 - 0x1F)
    Error IDs for Mesh Core Modules (0x10 - 0x1F).
    \{
*/

/** Error Codes for MS Common - \ref ms_error_codes_module_common */
#define MS_COMMON_ERR_ID                        0x1000
/** Error Codes for Timer - \ref ms_error_codes_module_timer */
#define TIMER_ERR_ID                            0x1200

/** \} */

/**
    \defgroup ms_error_codes_groups_protocols Protocols (0x20 - 0x3F)
    Error IDs for Mesh Protocol Modules (0x20 - 0x3F).
    \{
*/

/** Error Codes for Bearer - \ref ms_error_codes_module_brr */
#define BRR_ERR_ID                              0x2000
#define NET_ERR_ID                              0x2100
#define LTRN_ERR_ID                             0x2200
#define TRN_ERR_ID                              0x2300
#define ACCESS_ERR_ID                           0x2400

#define PROV_ERR_ID                             0x3000

#define HEALTH_ERR_ID                           0x8000

/** \} */


/** \} */

/** \} */
/** \} */

/* ================== EtherMind Common Reason Error Codes ================= */

/**
    \addtogroup ms_common_defines Defines
    \{
*/

/**
    \defgroup ms_error_codes_module_common Common
    \{
*/

#define MUTEX_INIT_FAILED                       0x0001
#define COND_INIT_FAILED                        0x0002
#define MUTEX_LOCK_FAILED                       0x0003
#define MUTEX_UNLOCK_FAILED                     0x0004
#define MEMORY_ALLOCATION_FAILED                0x0005

/** \} */
/** \} */

/* ======================================= Section 'Mesh' */
/**
    \addtogroup MS_ERROR_CODES
    \{
*/

/**
    \defgroup ms_error_codes_module_mesh Mesh Result Codes
    \{
*/
#define MS_SUCCESS                              (0x0000 | MS_ERR_ID)
/** Invalid Address */
#define MS_INVALID_ADDRESS                      (0x0001 | MS_ERR_ID)
/** Invalid Model */
#define MS_INVALID_MODEL                        (0x0002 | MS_ERR_ID)
/** Invalid AppKey Index */
#define MS_INVALID_APPKEY_INDEX                 (0x0003 | MS_ERR_ID)
/** Invalid NetKey Index */
#define MS_INVALID_NETKEY_INDEX                 (0x0004 | MS_ERR_ID)
/** Insufficient Resources */
#define MS_INSUFFICIENT_RESOURCES               (0x0005 | MS_ERR_ID)
/** Key Index Already Stored */
#define MS_KEY_INDEX_ALREADY_STORED             (0x0006 | MS_ERR_ID)
/** Invalid Publish Parameters */
#define MS_INVALID_PUBLISH_PARAMETER            (0x0007 | MS_ERR_ID)
/** Not a Subscribe Model */
#define MS_NOT_A_SUBSCRIBE_MODEL                (0x0008 | MS_ERR_ID)
/** Storage Failure */
#define MS_STORAGE_FAILURE                      (0x0009 | MS_ERR_ID)
/** Feature Not Supported */
#define MS_FEATURE_NOT_SUPPORTED                (0x000A | MS_ERR_ID)
/** Cannot Update */
#define MS_CANNOT_UPDATE                        (0x000B | MS_ERR_ID)
/** Cannot Remove */
#define MS_CANNOT_REMOVE                        (0x000C | MS_ERR_ID)
/** Cannot Bind */
#define MS_CANNOT_BIND                          (0x000D | MS_ERR_ID)
/** Temporarily Unable to Change State */
#define MS_TEMP_UNABLE_TO_CHANGE_STATE          (0x000E | MS_ERR_ID)
/** Cannot Set */
#define MS_CANNOT_SET                           (0x000F | MS_ERR_ID)
/** Unspecified Error */
#define MS_UNSPECIFIED_ERROR                    (0x0010 | MS_ERR_ID)
/** Invalid Binding */
#define MS_INVALID_BINDING                      (0x0011 | MS_ERR_ID)

/** \} */
/** \} */

/* ===================== EtherMind Module Error Codes ===================== */

/* ======================================= Section 'Timer' */
/**
    \cond ignore_this
    \{
*/

/**
    \defgroup ms_error_codes_module_timer Timer
    \{
*/

#define TIMER_MUTEX_INIT_FAILED                 \
    (MUTEX_INIT_FAILED | TIMER_ERR_ID)
#define TIMER_COND_INIT_FAILED                  \
    (COND_INIT_FAILED | TIMER_ERR_ID)
#define TIMER_MUTEX_LOCK_FAILED                 \
    (MUTEX_LOCK_FAILED | TIMER_ERR_ID)
#define TIMER_MUTEX_UNLOCK_FAILED               \
    (MUTEX_UNLOCK_FAILED | TIMER_ERR_ID)
#define TIMER_MEMORY_ALLOCATION_FAILED          \
    (MEMORY_ALLOCATION_FAILED | TIMER_ERR_ID)

#define TIMER_HANDLE_IS_NULL                    (0x0011 | TIMER_ERR_ID)
#define TIMER_CALLBACK_IS_NULL                  (0x0012 | TIMER_ERR_ID)
#define TIMER_QUEUE_EMPTY                       (0x0013 | TIMER_ERR_ID)
#define TIMER_QUEUE_FULL                        (0x0014 | TIMER_ERR_ID)
#define TIMER_ENTITY_SEARCH_FAILED              (0x0015 | TIMER_ERR_ID)
#define TIMER_NULL_PARAMETER_NOT_ALLOWED        (0x0016 | TIMER_ERR_ID)
#define TIMER_TIMEOUT_ZERO_NOT_ALLOWED          (0x0017 | TIMER_ERR_ID)

/** \} */
/** \} */

/* ======================================= Section 'Bearer' */
/**
    \addtogroup brr_defines
    \{
*/

/**
    \defgroup ms_error_codes_module_brr Error Code
    \{
*/

#define BRR_MUTEX_INIT_FAILED                    \
    (MUTEX_INIT_FAILED | BRR_ERR_ID)
#define BRR_COND_INIT_FAILED                     \
    (COND_INIT_FAILED | BRR_ERR_ID)
#define BRR_MUTEX_LOCK_FAILED                    \
    (MUTEX_LOCK_FAILED | BRR_ERR_ID)
#define BRR_MUTEX_UNLOCK_FAILED                  \
    (MUTEX_UNLOCK_FAILED | BRR_ERR_ID)
#define BRR_MEMORY_ALLOCATION_FAILED             \
    (MEMORY_ALLOCATION_FAILED | BRR_ERR_ID)

#define BRR_INVALID_PARAMETER_VALUE             (0x0011 | BRR_ERR_ID)
#define BRR_PARAMETER_OUTSIDE_RANGE             (0x0012 | BRR_ERR_ID)
#define BRR_NULL_PARAMETER_NOT_ALLOWED          (0x0013 | BRR_ERR_ID)
#define BRR_INTERFACE_NOT_READY                 (0x0014 | BRR_ERR_ID)
#define BRR_API_NOT_SUPPORTED                   (0x00FF | BRR_ERR_ID)

/** \} */
/** \} */

/* ======================================= Section 'Network' */
/**
    \addtogroup net_defines
    \{
*/

/**
    \defgroup ms_error_codes_module_net Error Code
    \{
*/

#define NET_MUTEX_INIT_FAILED                    \
    (MUTEX_INIT_FAILED | NET_ERR_ID)
#define NET_COND_INIT_FAILED                     \
    (COND_INIT_FAILED | NET_ERR_ID)
#define NET_MUTEX_LOCK_FAILED                    \
    (MUTEX_LOCK_FAILED | NET_ERR_ID)
#define NET_MUTEX_UNLOCK_FAILED                  \
    (MUTEX_UNLOCK_FAILED | NET_ERR_ID)
#define NET_MEMORY_ALLOCATION_FAILED             \
    (MEMORY_ALLOCATION_FAILED | NET_ERR_ID)

#define NET_INVALID_PARAMETER_VALUE             (0x0011 | NET_ERR_ID)
#define NET_PARAMETER_OUTSIDE_RANGE             (0x0012 | NET_ERR_ID)
#define NET_NULL_PARAMETER_NOT_ALLOWED          (0x0013 | NET_ERR_ID)
#define NET_TX_QUEUE_FULL                       (0x0014 | NET_ERR_ID)
#define NET_TX_QUEUE_EMPTY                      (0x0015 | NET_ERR_ID)

/**
    Error Codes returned by Network Callback, indicating if it detected
    an invalid packet format or if the packet to be further processed,
    by the network layer like to be relayed or proxied etc.
*/
#define NET_INVALID_RX_PKT_FORMAT               (0x0016 | NET_ERR_ID)
#define NET_RX_LOCAL_SRC_ADDR_PKT               (0x0017 | NET_ERR_ID)
#define NET_POST_PROCESS_RX_PKT                 (0x0018 | NET_ERR_ID)

#define NET_API_NOT_SUPPORTED                   (0x00FF | NET_ERR_ID)

/** \} */
/** \} */

/* ======================================= Section 'Lower Transport' */
/**
    \addtogroup ltrn_defines
    \{
*/

/**
    \defgroup ms_error_codes_module_ltrn Error Code
    \{
*/

#define LTRN_MUTEX_INIT_FAILED                    \
    (MUTEX_INIT_FAILED | LTRN_ERR_ID)
#define LTRN_COND_INIT_FAILED                     \
    (COND_INIT_FAILED | LTRN_ERR_ID)
#define LTRN_MUTEX_LOCK_FAILED                    \
    (MUTEX_LOCK_FAILED | LTRN_ERR_ID)
#define LTRN_MUTEX_UNLOCK_FAILED                  \
    (MUTEX_UNLOCK_FAILED | LTRN_ERR_ID)
#define LTRN_MEMORY_ALLOCATION_FAILED             \
    (MEMORY_ALLOCATION_FAILED | LTRN_ERR_ID)

#define LTRN_INVALID_PARAMETER_VALUE             (0x0011 | LTRN_ERR_ID)
#define LTRN_PARAMETER_OUTSIDE_RANGE             (0x0012 | LTRN_ERR_ID)
#define LTRN_NULL_PARAMETER_NOT_ALLOWED          (0x0013 | LTRN_ERR_ID)
#define LTRN_SAR_CTX_ALLOCATION_FAILED           (0x0014 | LTRN_ERR_ID)

/** \} */
/** \} */

/* ======================================= Section 'Transport' */
/**
    \addtogroup trn_defines
    \{
*/

/**
    \defgroup ms_error_codes_module_trn Error Code
    \{
*/

#define TRN_MUTEX_INIT_FAILED                    \
    (MUTEX_INIT_FAILED | TRN_ERR_ID)
#define TRN_COND_INIT_FAILED                     \
    (COND_INIT_FAILED | TRN_ERR_ID)
#define TRN_MUTEX_LOCK_FAILED                    \
    (MUTEX_LOCK_FAILED | TRN_ERR_ID)
#define TRN_MUTEX_UNLOCK_FAILED                  \
    (MUTEX_UNLOCK_FAILED | TRN_ERR_ID)
#define TRN_MEMORY_ALLOCATION_FAILED             \
    (MEMORY_ALLOCATION_FAILED | TRN_ERR_ID)

#define TRN_INVALID_PARAMETER_VALUE             (0x0011 | TRN_ERR_ID)
#define TRN_PARAMETER_OUTSIDE_RANGE             (0x0012 | TRN_ERR_ID)
#define TRN_NULL_PARAMETER_NOT_ALLOWED          (0x0013 | TRN_ERR_ID)
#define TRN_QUEUE_FULL                          (0x0014 | TRN_ERR_ID)
#define TRN_QUEUE_EMPTY                         (0x0015 | TRN_ERR_ID)
#define TRN_INCOMPLETE_PKT_RECEIVED             (0x0016 | TRN_ERR_ID)
#define TRN_INVALID_FRNDSHIP_STATE              (0x0017 | TRN_ERR_ID)

#define TRN_API_NOT_SUPPORTED                   (0x00FF | TRN_ERR_ID)

/** \} */
/** \} */

/* ======================================= Section 'Access' */
/**
    \addtogroup access_defines
    \{
*/

/**
    \defgroup ms_error_codes_module_access Error Code
    \{
*/

#define ACCESS_MUTEX_INIT_FAILED                    \
    (MUTEX_INIT_FAILED | ACCESS_ERR_ID)
#define ACCESS_COND_INIT_FAILED                     \
    (COND_INIT_FAILED | ACCESS_ERR_ID)
#define ACCESS_MUTEX_LOCK_FAILED                    \
    (MUTEX_LOCK_FAILED | ACCESS_ERR_ID)
#define ACCESS_MUTEX_UNLOCK_FAILED                  \
    (MUTEX_UNLOCK_FAILED | ACCESS_ERR_ID)
#define ACCESS_MEMORY_ALLOCATION_FAILED             \
    (MEMORY_ALLOCATION_FAILED | ACCESS_ERR_ID)

#define ACCESS_INVALID_PARAMETER_VALUE             (0x0011 | ACCESS_ERR_ID)
#define ACCESS_PARAMETER_OUTSIDE_RANGE             (0x0012 | ACCESS_ERR_ID)
#define ACCESS_NULL_PARAMETER_NOT_ALLOWED          (0x0013 | ACCESS_ERR_ID)

#define ACCESS_NO_RESOURCE                         (0x0020 | ACCESS_ERR_ID)
#define ACCESS_NO_MATCH                            (0x0021 | ACCESS_ERR_ID)
#define ACCESS_INVALID_HANDLE                      (0x0022 | ACCESS_ERR_ID)
#define ACCESS_MODEL_ALREADY_REGISTERED            (0x0023 | ACCESS_ERR_ID)
#define ACCESS_INVALID_SRC_ADDR                    (0x0024 | ACCESS_ERR_ID)
#define ACCESS_DEV_KEY_TABLE_FULL                  (0x0025 | ACCESS_ERR_ID)
#define ACCESS_MASTER_NID_ON_LPN                   (0x0026 | ACCESS_ERR_ID)
#define ACCESS_INVALID_PUBLICATION_STATE           (0x0027 | ACCESS_ERR_ID)

#define ACCESS_API_NOT_SUPPORTED                   (0x00FF | ACCESS_ERR_ID)

/** \} */
/** \} */

/* ======================================= Section 'Provisioning' */
/**
    \addtogroup prov_defines
    \{
*/

/**
    \defgroup ms_error_codes_module_provisioning Error Code
    \{
*/

#define PROV_MUTEX_INIT_FAILED                    \
    (MUTEX_INIT_FAILED | PROV_ERR_ID)
#define PROV_COND_INIT_FAILED                     \
    (COND_INIT_FAILED | PROV_ERR_ID)
#define PROV_MUTEX_LOCK_FAILED                    \
    (MUTEX_LOCK_FAILED | PROV_ERR_ID)
#define PROV_MUTEX_UNLOCK_FAILED                  \
    (MUTEX_UNLOCK_FAILED | PROV_ERR_ID)
#define PROV_MEMORY_ALLOCATION_FAILED             \
    (MEMORY_ALLOCATION_FAILED | PROV_ERR_ID)

#define PROV_INVALID_STATE                          (0x0011 | PROV_ERR_ID)
#define PROV_INVALID_PARAMETER                      (0x0012 | PROV_ERR_ID)
#define PROV_CONTEXT_ALLOC_FAILED                   (0x0013 | PROV_ERR_ID)
#define PROV_CONTEXT_ASSERT_FAILED                  (0x0014 | PROV_ERR_ID)
#define PROV_CONTEXT_LINK_OPEN                      (0x0015 | PROV_ERR_ID)
#define PROV_BEARER_ASSERT_FAILED                   (0x0016 | PROV_ERR_ID)
#define PROV_PROCEDURE_TIMEOUT                      (0x0017 | PROV_ERR_ID)

/** \} */
/** \} */

/* ======================================= Section 'Health Server' */
/**
    \addtogroup health_server_defines
    \{
*/

/**
    \defgroup ms_error_codes_module_health_server Error Code
    \{
*/

#define HEALTH_MUTEX_INIT_FAILED                    \
    (MUTEX_INIT_FAILED | HEALTH_ERR_ID)
#define HEALTH_COND_INIT_FAILED                     \
    (COND_INIT_FAILED | HEALTH_ERR_ID)
#define HEALTH_MUTEX_LOCK_FAILED                    \
    (MUTEX_LOCK_FAILED | HEALTH_ERR_ID)
#define HEALTH_MUTEX_UNLOCK_FAILED                  \
    (MUTEX_UNLOCK_FAILED | HEALTH_ERR_ID)
#define HEALTH_MEMORY_ALLOCATION_FAILED             \
    (MEMORY_ALLOCATION_FAILED | HEALTH_ERR_ID)

#define HEALTH_INVALID_STATE                          (0x0011 | HEALTH_ERR_ID)
#define HEALTH_INVALID_PARAMETER                      (0x0012 | HEALTH_ERR_ID)
#define HEALTH_CONTEXT_ALLOC_FAILED                   (0x0013 | HEALTH_ERR_ID)
#define HEALTH_CONTEXT_ASSERT_FAILED                  (0x0014 | HEALTH_ERR_ID)

/** \} */
/** \} */

#endif /* _H_MS_ERROR_ */

