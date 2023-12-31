/**
    \file appl_health_client.h

    \brief This file defines the Mesh Health Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_APPL_HEALTH_CLIENT_
#define _H_APPL_HEALTH_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_health_client_api.h"
#include "appl_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* health client application entry point */
void main_health_client_operations(void);

/* Send Health Attention Get */
void appl_send_health_attention_get(void);

/* Send Health Attention Set */
void appl_send_health_attention_set(void);

/* Send Health Attention Set Unacknowledged */
void appl_send_health_attention_set_unacknowledged(void);

/* Send Health Fault Clear */
void appl_send_health_fault_clear(void);

/* Send Health Fault Clear Unacknowledged */
void appl_send_health_fault_clear_unacknowledged(void);

/* Send Health Fault Get */
void appl_send_health_fault_get(void);

/* Send Health Fault Test */
void appl_send_health_fault_test(void);

/* Send Health Fault Test Unacknowledged */
void appl_send_health_fault_test_unacknowledged(void);

/* Send Health Period Get */
void appl_send_health_period_get(void);

/* Send Health Period Set */
void appl_send_health_period_set(void);

/* Send Health Period Set Unacknowledged */
void appl_send_health_period_set_unacknowledged(void);

/* Get Model Handle */
void appl_health_client_get_model_handle(void);

/* Set Publish Address */
void appl_health_client_set_publish_address(void);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Health client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT appl_health_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_APPL_HEALTH_CLIENT_ */
