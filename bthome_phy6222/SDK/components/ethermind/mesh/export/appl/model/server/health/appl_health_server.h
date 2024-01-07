/**
    \file appl_health_server.h
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_APPL_HEALTH_SERVER_
#define _H_APPL_HEALTH_SERVER_


/* --------------------------------------------- Header File Inclusion */
#include "MS_health_server_api.h"
#include "appl_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* health server application entry point */
void main_health_server_operations(/* IN */ UINT8 have_menu);

/**
    \brief Health Server application Asynchronous Notification Callback.

    \par Description
    Health Server calls the registered callback to indicate events occurred to the
    application.

    \param handle        Model Handle.
    \param event_type    Health Server Event type.
    \param event_param   Parameter associated with the event if any or NULL.
    \param param_len     Size of the event parameter data. 0 if event param is NULL.
*/
API_RESULT appl_health_server_cb
(
    MS_ACCESS_MODEL_HANDLE* handle,
    UINT8                    event_type,
    UINT8*                   event_param,
    UINT16                   param_len
);

void appl_health_server_send_status(void);

#endif /*_H_APPL_HEALTH_SERVER_ */

