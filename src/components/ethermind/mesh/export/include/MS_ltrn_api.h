
/**
    \file MS_ltrn_api.h

    \brief This file defines the Mesh Lower Transport Application Interface - includes
    Data Structures and Methods.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_LTRN_API_
#define _H_MS_LTRN_API_


/* --------------------------------------------- Header File Inclusion */
/* Network API Header File */
#include "MS_net_api.h"

/* --------------------------------------------- Global Definitions */

/**
    \defgroup ltrn_module LTRANSPORT (Mesh Lower Transport Layer)
    \{
    This section describes the interfaces & APIs offered by the EtherMind
    Mesh Lower Transport (LTRANSPORT) module to the Application and other upper
    layers of the stack.
*/

/**
    \defgroup ltrn_defines Defines
    \{
    Describes defines for the module.
*/

/**
    \defgroup ltrn_constants Constants
    \{
    Describes Constants defined by the module.
*/

/** Unsgemented Access Message */
#define MS_UNSEGMENTED_ACCESS_MSG     0x00

/** Segmented Access Message */
#define MS_SEGMENTED_ACCESS_MSG       0x01

/** Unsegmented Control Message */
#define MS_UNSEGMENTED_CTRL_MSG       0x02

/** Segmented Control Message */
#define MS_SEGMENTED_CTRL_MSG         0x03

/** Transport Message Type */
typedef UCHAR   MS_TRN_MSG_TYPE;

/** Transport Layer Control Packet */
#define MS_TRN_CTRL_PKT            0x01

/** Access Layer Packet */
#define MS_TRN_ACCESS_PKT          0x00

/** \} */

/**
    \defgroup ltrn_events Events
    \{
    This section lists the Asynchronous Events notified to Application by the
    Module.
*/
/** \} */
/** \} */

/**
    \defgroup ltrn_marcos Utility Macros
    \{
    Initialization and other Utility Macros offered by the module.
*/

/** \} */

/* --------------------------------------------- Data Types/ Structures */

/**
    \addtogroup ltrn_defines Defines
    \{
*/

/**
    \addtogroup ltrn_structures Structures
    \{
*/
/** LPN Handle */
typedef UINT8 LPN_HANDLE;


/** \} */

/** \} */

/**
    \defgroup ltrn_cb Application Callback
    \{
    This Section Describes the module Notification Callback interface offered
    to the application
*/
/**
    Lower TRANSPORT Application Asynchronous Notification Callback.

    Lower TRANSPORT calls the registered callback to indicate events occurred to the
    application.

    \param net_hdr           Network Header.
    \param subnet_handle     Associated Subnet Handle.
    \param data_param        Data associated with the event if any or NULL.
    \param data_len          Size of the event data. 0 if event data is NULL.
*/
typedef API_RESULT (*LTRN_NTF_CB)
(
    MS_NET_HEADER*      net_hdr,
    MS_SUBNET_HANDLE    subnet_handle,
    UCHAR               szmic,
    UCHAR*              data_param,
    UINT16              data_len
) DECL_REENTRANT;
/** \} */

/**
    \addtogroup ltrn_defines Defines
    \{
*/

/**
    \addtogroup ltrn_structures Structures
    \{
*/

/** \} */

/** \} */

/** TCF (Transport Control Field) - Transport Field Value */


/* --------------------------------------------- Function */

/**
    \defgroup ltrn_api_defs API Definitions
    \{
    This section describes the EtherMind Mesh Lower Transport Layer APIs.
*/
#ifdef __cplusplus
extern "C" {
#endif

/**
    \brief Register Inerface with Lower Transport Layer

    \par Description
    This routine registers interface with the Lower Transport Layer.
    Transport Layer supports single Application, hence this rouine shall be called once.

    \param [in] ltrn_cb
           Upper Layer Notification Callback

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_ltrn_register
(
    /* IN */ LTRN_NTF_CB    ltrn_cb
);

/**
    \brief API to send transport PDUs

    \par Description
    This routine sends transport PDUs to peer device.

    \param [in] src_addr
           Source Address

    \param [in] dst_addr
           Destination Address

    \param [in] subnet_handle
           Handle identifying the Subnet

    \param [in] msg_type
           Transport Message Type

    \param [in] ttl
           Time to Live

    \param [in] akf
           Application Key Flag

    \param [in] aid
           Application Key Identifier

    \param [in] seq_num
           Sequence Number to be used for the Packet

    \param [in] buffer
           Transport Packet

    \param [in] buffer_len
           Transport Packet Length

    \param [in] reliable
           If requires lower transport Ack, set reliable as TRUE

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_ltrn_send_pdu
(
    /* IN */ MS_NET_ADDR               src_addr,
    /* IN */ MS_NET_ADDR               dst_addr,
    /* IN */ MS_SUBNET_HANDLE          subnet_handle,
    /* IN */ MS_TRN_MSG_TYPE           msg_type,
    /* IN */ UINT8                     ttl,
    /* IN */ UINT8                     akf,
    /* IN */ UINT8                     aid,
    /* IN */ UINT32                    seq_num,
    /* IN */ UCHAR*                    buffer,
    /* IN */ UINT16                    buffer_len,
    /* IN */ UINT8                     reliable
);

/**
    \brief To clear all Segmentation and Reassembly Contexts

    \par Description
    This routine clears all Segmentation and Reassembly Contexts.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_ltrn_clear_sar_contexts(void);

#ifdef __cplusplus
};
#endif

/** \} */

/** \} */

#endif /* _H_MS_LTRN_API_ */

