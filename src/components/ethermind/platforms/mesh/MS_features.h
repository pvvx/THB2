/**
    \file MS_features.h

    \brief EtherMind Compilation Switches Configuration File.

    This file lists all the Compilation Flags available in various
    EtherMind Mesh Stack modules.
*/

/*
    Copyright (C) 2016. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_FEATURES_
#define _H_MS_FEATURES_

/* ----------------------------------------------------------------------- */
/* ========== Platform =================================================== */
/* ----------------------------------------------------------------------- */
/*
    WINDOWS

    This flag is used when the EtherMind stack is being compiled on
    Windows User Mode platforms.

    Dependency: None.
*/
#define WINDOWS


/* ----------------------------------------------------------------------- */
/* ==== Mesh Specification Flags ========================================= */
/* ----------------------------------------------------------------------- */
/*
    MS_1_0

    This flag is used when the EtherMind stack is being compiled for
    Mesh specification version 1.0.
*/
#define MS_1_0

/* ----------------------------------------------------------------------- */
/* ==== Mesh v1.0 Feature Flags ========================================== */
/* ----------------------------------------------------------------------- */
/*
    MS_PROVISIONING_SUPPORT

    This flag is used to enable support for Provisioning feature.

    Dependency: None.
*/
#define MS_PROVISIONING_SUPPORT

/*
    MS_RELAY_SUPPORT

    This flag is used to enable support for Relay feature.

    Dependency: None.
*/
#define MS_RELAY_SUPPORT

/*
    MS_PROXY_SUPPORT

    This flag is used to enable support for Proxy feature.

    Dependency: None.
*/
#define MS_PROXY_SUPPORT

/*
    MS_PROXY_SERVER

    This flag is used to enable support for Proxy Server feature.

    Dependency: MS_PROXY_SUPPORT.
*/
#ifdef MS_PROXY_SUPPORT
    #define MS_PROXY_SERVER
#endif /* MS_PROXY_SUPPORT */

/*
    MS_PROXY_CLIENT

    This flag is used to enable support for Proxy Client feature.

    Dependency: MS_PROXY_SUPPORT.
*/
#ifdef MS_PROXY_SUPPORT
    #define MS_PROXY_CLIENT
#endif /* MS_PROXY_SUPPORT */

/**
    Check if the relevant roles for Proxy are Enabled
*/
#ifdef MS_PROXY_SUPPORT
    #if !((defined MS_PROXY_SERVER) || (defined MS_PROXY_CLIENT))
        #error "Server or Client SHALL be supported when MS_PROXY_SUPPORT enabled"
    #endif /* !((defined MS_PROXY_SERVER) || (defined MS_PROXY_CLIENT)) */
#endif /* MS_PROXY_SUPPORT */

/*
    MS_FRIEND_SUPPORT

    This flag is used to enable support for Friend feature.

    Dependency: None.
*/
#undef MS_FRIEND_SUPPORT

/*
    MS_LPN_SUPPORT

    This flag is used to enable support for Low Power feature.

    Dependency: None.
*/
#undef MS_LPN_SUPPORT

#ifndef CFG_HEARTBEAT_MODE
    #define CFG_HEARTBEAT_MODE 0
#endif


/* ----------------------------------------------------------------------- */
/* ==== Stack Architecture Flags ========================================= */
/* ----------------------------------------------------------------------- */


/* ----------------------------------------------------------------------- */
/* ==== Stack Feature Flags ============================================== */
/* ----------------------------------------------------------------------- */
/*
    MS_SUPPORT_STACK_VERSION_INFO

    This flag enables EtherMind MS_get_version_number() API.

    This API is used to retrieve the current stack build version
    information

    Dependency: None.
*/
#define MS_SUPPORT_STACK_VERSION_INFO

/*
    MS_HAVE_DYNAMIC_CONFIG

    This flag enables dynamic configuration of some of the parameters defined in MS_limits.h.

    Dependency: None.
*/
#define MS_HAVE_DYNAMIC_CONFIG

/*
    MS_DEBUG_ASSERT

    This flag enables EtherMind MS_assert() macro, for evaluating truth
    value of an expression. For details, refer to BT_assert.h for the
    platform.

    The MS_assert() macro is used for debugging during development, and
    can be excluded in builds for the final product.

    Dependency: None.
*/
#define MS_DEBUG_ASSERT

/*
    MS_STATUS

    This flag enables EtherMind Status Flag APIs for reporting various
    Status, as described in BT_status_api.h. The EtherMind Status APIs
    are designed and to be used for debugging purposes during development
    and can be excluded in builds for final products.

    Dependency: None.
*/
/* #define MS_STATUS */

/*
    MS_DISABLE_MUTEX

    This flag disables the use of mutex and conditional variables from all
    stack modules (protocols and profiles).
    This should be defined in EtherMind builds where synchronization
    primitives are not required (such as, single task build of EtherMind).

    Dependency: None.
*/
#define MS_DISABLE_MUTEX

/*
    EM_HAVE_STATIC_DECL

    This flag enables provision for declaring functions and/or globals in
    a file as 'static'.
*/
#define EM_HAVE_STATIC_DECL

/*
    EM_HAVE_CONST_DECL

    This flag enables provision for declaring globals in a file as 'const'.
*/
#define EM_HAVE_CONST_DECL

/*
    VAR_ARG_IN_MACRO_NOT_SUPPORTED

    This flag should be enabled if the compiler tool-chain does not support
    variable argument in macro.
*/
/* #define VAR_ARG_IN_MACRO_NOT_SUPPORTED */

/*
    MS_STORAGE

    This flag enables support of the Storage Module

    Dependency: None
*/
#define MS_STORAGE

/* ----------------------------------------------------------------------- */
/* ==== Module Inclusion Flags for EtherMind Mesh Modules ================ */
/* ----------------------------------------------------------------------- */
/* Mesh Models */
#define MS_MODEL_CONFIG


/* ----------------------------------------------------------------------- */
/* ==== Module Inclusion Flags for EtherMind Mesh Modules ================ */
/* ----------------------------------------------------------------------- */
/*
    Module inclusion flags for various EtherMind Mesh Modules, and should be
    defined according to the modules included in the build.

    Dependency: None.
*/
#define MS_BRR
#define MS_NET
#define MS_LTRN
#define MS_TRN
#define MS_ACCESS


/*
    Topology - Roles.
    Only one of the role shall be defined. Edge or Relay.
*/
/*
    An Edge node is capable of transmitting and receiving mesh packets,
    but does not relay incoming packets.
*/
/* #define MS_ROLE_EDGE_NODE */

/*
    A Relay node is capable of transmitting and receiving mesh packets,
    and contributes to the multi-hop network infrastructure by relaying
    incoming packets, which are authenticated against the network security
    credentials.
*/
#define MS_ROLE_RELAY_NODE

/*
    Topology - Sub-Roles.
    Low Power Node or Friend Relay Node.
*/
/*
    A Low Power node is an Edge Node performing low duty cycle scanning.
*/
/* #define MS_SUB_ROLE_LOW_POWER */

/*
    A Friend Relay node is a Relay node.
    Friend Relay nodes will also allow Low Power nodes to establish friend
    relationships with it, and once established will cache packets destined
    for the Low Power node.
*/
#define MS_SUB_ROLE_FRIEND_RELAY_NODE


/* ----------------------------------------------------------------------- */
/* ==== Bearer Module Specific Flags ===================================== */
/* ----------------------------------------------------------------------- */

/*
    MS_BEARER_ADV

    An advertising bearer.
*/
#define MS_BEARER_ADV

/*
    MS_BEARER_GATT

    A GATT bearer.
*/
/* #define MS_BEARER_GATT */

/*
    BLEBRR_LP_SUPPORT

    Enables Low Power Mode interfaces in the bearer.
*/
#undef BLEBRR_LP_SUPPORT


/* ----------------------------------------------------------------------- */
/* ==== Network Module Specific Flags ==================================== */
/* ----------------------------------------------------------------------- */


/* ----------------------------------------------------------------------- */
/* ==== Transport Module Specific Flags ================================== */
/* ----------------------------------------------------------------------- */
/*
    MS_TRN_SEND_UNSEG_MSG_SUPPORT

    Enables support for sending Unsegmented Transport Message procedure.
*/
#define MS_TRN_SEND_UNSEG_MSG_SUPPORT

/*
    MS_TRN_SEND_CTRL_PKT_SUPPORT

    Enables support for sending Transport Control Packet procedure.
*/
#define MS_TRN_SEND_CTRL_PKT_SUPPORT

#ifdef MS_TRN_SEND_CTRL_PKT_SUPPORT
    /*
    MS_TRN_CTRL_FRND_PING_SUPPORT

    Enables support for sending Transport Control Packet - Friend Poll procedure.
    */
    #define MS_TRN_CTRL_FRND_POLL_SUPPORT

    /*
    MS_TRN_CTRL_FRND_UPDATE_SUPPORT

    Enables support for sending Transport Control Packet - Friend Update procedure.
    */
    #define MS_TRN_CTRL_FRND_UPDATE_SUPPORT

    /*
    MS_TRN_CTRL_FRND_REQ_SUPPORT

    Enables support for sending Transport Control Packet - Friend Request procedure.
    */
    #define MS_TRN_CTRL_FRND_REQ_SUPPORT

    /*
    MS_TRN_CTRL_FRND_OFFER_SUPPORT

    Enables support for sending Transport Control Packet - Friend Offer procedure.
    */
    #define MS_TRN_CTRL_FRND_OFFER_SUPPORT

    /*
    MS_TRN_CTRL_FRND_CLEAR_SUPPORT

    Enables support for sending Transport Control Packet - Friend Clear procedure.
    */
    #define MS_TRN_CTRL_FRND_CLEAR_SUPPORT

    /*
    MS_TRN_CTRL_FRND_CLEAR_CNF_SUPPORT

    Enables support for sending Transport Control Packet - Friend Clear Confirmation procedure.
    */
    #define MS_TRN_CTRL_FRND_CLEAR_CNF_SUPPORT

    /*
    MS_TRN_CTRL_FRND_SUBSCRN_LIST_ADD_SUPPORT

    Enables support for sending Transport Control Packet - Friend Subscription List Add procedure.
    */
    #define MS_TRN_CTRL_FRND_SUBSCRN_LIST_ADD_SUPPORT

    /*
    MS_TRN_CTRL_FRND_SUBSCRN_LIST_REMOVE_SUPPORT

    Enables support for sending Transport Control Packet - Friend Subscription List Remove procedure.
    */
    #define MS_TRN_CTRL_FRND_SUBSCRN_LIST_REMOVE_SUPPORT

    /*
    MS_TRN_CTRL_FRND_SUBSCRN_LIST_CNF_SUPPORT

    Enables support for sending Transport Control Packet - Friend Subscription List Confirmation procedure.
    */
    #define MS_TRN_CTRL_FRND_SUBSCRN_LIST_CNF_SUPPORT

    /*
    MS_TRN_CTRL_HEARTBEAT_SUPPORT

    Enables support for sending Transport Control Packet - Heartbeat procedure.
    */
    #define MS_TRN_CTRL_HEARTBEAT_SUPPORT

#endif /* MS_TRN_SEND_CTRL_PKT_SUPPORT */

/*
    MS_TRN_NO_NULL_PARAM_CHECK

    This flag disables 'null' parameter check in Mesh Transport APIs

    When application using Mesh Transport APIs are validated and
    it is ensured that none of the API call path is using
    an unexpected 'null' parameter, this flag can be enabled
    to reduce the code size.

    Dependency: None.
*/
/* #define MS_TRN_NO_NULL_PARAM_CHECK */

/*
    MS_TRN_NO_RANGE_CHECK

    This flag disables the check in Mesh Transport APIs to verify if the parameter
    values are within specification defined valid range

    When application using Mesh Transport APIs are validated and
    it is ensured that none of the API call path is using
    parameters with invalid value, this flag can be enabled
    to reduce the code size.

    Dependency: None.
*/
/* #define MS_TRN_NO_RANGE_CHECK */

/* ----------------------------------------------------------------------- */
/* ==== Access Module Specific Flags ===================================== */
/* ----------------------------------------------------------------------- */
/*
    MS_ACCESS_PUBLISH_TIMER_SUPPORT

    This flag enables Publication Timer Support.

    Dependency: None.
*/
#undef MS_ACCESS_PUBLISH_TIMER_SUPPORT

/* ----------------------------------------------------------------------- */
/* ==== Model Specific Flags ============================================= */
/* ----------------------------------------------------------------------- */
/*
    Support Generic OnOff Model.
*/
#define HAVE_GENERIC_ONOFF_MODEL

/*
    Support Generic Level Model.
*/
#define HAVE_GENERIC_LEVEL_MODEL


/* ----------------------------------------------------------------------- */
/* ==== Module Profiling Flags =========================================== */
/* ----------------------------------------------------------------------- */
/*
    Profiling related definitions for all the layers (protocol and profiles)
    shall be defined in this section.
*/

/*
    MS_ENABLE_SPY

    If defined, this flag enables module profiling.

    Dependency: None
*/
#define MS_ENABLE_SPY

#ifdef MS_ENABLE_SPY
    #define BRR_ENABLE_SPY
    #define NET_ENABLE_SPY
    #define TRN_ENABLE_SPY
    #define APP_ENABLE_SPY
#endif /* MS_ENABLE_SPY */


/* ----------------------------------------------------------------------- */
/* ==== Debug Specification Flags ======================================== */
/* ----------------------------------------------------------------------- */
/*
    Debug definitions for all the layers (protocol and profiles) should be
    defined in this section.
*/

/*
    By default, the Error Logs of all the layers are enabled.
    To disable error logging of a module, define <module>_NO_DEBUG flag.
    Example: Define BRR_NO_DEBUG to disable error logging of Bearer layer.

    By default, the Trace, Information, Data and other Logs
    of all the layers are disabled.
    To enable debug logging of a module, define <module>_DEBUG flag.
    Example: Define BRR_DEBUG to enable debug logging (Trace and Information)
    of Bearer layer.
*/

/* Protocol Modules */
#define COMMON_NO_DEBUG
/* #define COMMON_DEBUG */

//#define BRR_DEBUG
#define BRR_NO_DEBUG
/* #define BRR_DEBUG */

//#define NET_DEBUG
#define NET_NO_DEBUG
/* #define NET_DEBUG */

#define LTRN_NO_DEBUG
//#define LTRN_DEBUG
/* #define LTRN_DEBUG */

#define TRN_NO_DEBUG
//#define TRN_DEBUG
/* #define TRN_DEBUG */

#define APP_NO_DEBUG
//#define APP_DEBUG

/* #define APP_DEBUG */

#define STBX_NO_DEBUG
//#define STBX_DEBUG
/* #define STBX_DEBUG */

#define ACCESS_NO_DEBUG
//#define ACCESS_DEBUG
/* #define ACCESS_DEBUG */

#define PROV_NO_DEBUG
//#define PROV_DEBUG
/* #define PROV_DEBUG */

#define CONFIG_NO_DEBUG
//#define CONFIG_DEBUG
/* #define CONFIG_DEBUG */

#endif /* _H_MS_FEATURES_ */

