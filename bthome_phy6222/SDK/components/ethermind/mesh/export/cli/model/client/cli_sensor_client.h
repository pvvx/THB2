/**
    \file cli_sensor_client.h

    \brief This file defines the Mesh Sensor Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2018. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_CLI_SENSOR_CLIENT_
#define _H_CLI_SENSOR_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_sensor_api.h"
#include "cli_main.h"


/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/* sensor client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_sensor);

/* sensor client CLI entry point */
CLI_CMD_HANDLER_DECL(cli_modelc_sensor_setup);

/* Send Sensor Cadence Get */
CLI_CMD_HANDLER_DECL(cli_modelc_sensor_cadence_get);

/* Send Sensor Cadence Set */
CLI_CMD_HANDLER_DECL(cli_modelc_sensor_cadence_set);

/* Send Sensor Cadence Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_sensor_cadence_set_unacknowledged);

/* Send Sensor Column Get */
CLI_CMD_HANDLER_DECL(cli_modelc_sensor_column_get);

/* Send Sensor Descriptor Get */
CLI_CMD_HANDLER_DECL(cli_modelc_sensor_descriptor_get);

/* Send Sensor Get */
CLI_CMD_HANDLER_DECL(cli_modelc_sensor_get);

/* Send Sensor Series Get */
CLI_CMD_HANDLER_DECL(cli_modelc_sensor_series_get);

/* Send Sensor Setting Get */
CLI_CMD_HANDLER_DECL(cli_modelc_sensor_setting_get);

/* Send Sensor Setting Set */
CLI_CMD_HANDLER_DECL(cli_modelc_sensor_setting_set);

/* Send Sensor Setting Set Unacknowledged */
CLI_CMD_HANDLER_DECL(cli_modelc_sensor_setting_set_unacknowledged);

/* Send Sensor Settings Get */
CLI_CMD_HANDLER_DECL(cli_modelc_sensor_settings_get);

/* Get Model Handle */
CLI_CMD_HANDLER_DECL(cli_modelc_sensor_get_model_handle);

/* Set Publish Address */
CLI_CMD_HANDLER_DECL(cli_modelc_sensor_set_publish_address);

/**
    \brief Client Application Asynchronous Notification Callback.

    \par Description
    Sensor client calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT cli_sensor_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_CLI_SENSOR_CLIENT_ */
