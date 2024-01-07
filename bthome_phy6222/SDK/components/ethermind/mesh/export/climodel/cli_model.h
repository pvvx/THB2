/**************************************************************************************************
*******
**************************************************************************************************/


#ifndef _CLI_VENDOR_H
#define _CLI_VENDOR_H

/*********************************************************************
    INCLUDES
*/
#include "types.h"
#include "rf_phy_driver.h"

#include "bleMesh.h"

#include "MS_common.h"

#include "MS_prov_api.h"

#include "nvs.h"

#include "cliface.h"

#include "mesh_clients.h"
#include "access_extern.h"
#include "MS_config_api.h"
#include "vendormodel_server.h"

/*********************************************************************
    LOCAL FUNCTIONS
*/
API_RESULT cli_raw_data(UINT32 argc, UCHAR* argv[]);

API_RESULT cli_get_information(UINT32 argc, UCHAR* argv[]);

API_RESULT cli_disp_key(UINT32 argc, UCHAR* argv[]);

API_RESULT cli_demo_reset(UINT32 argc, UCHAR* argv[]);

API_RESULT cli_internal_status(UINT32 argc, UCHAR* argv[]);

API_RESULT cli_modelc_config_heartbeat_publication_set(UINT32 argc, UCHAR* argv[]);

API_RESULT cli_start(UINT32 argc, UCHAR* argv[]);

API_RESULT cli_on(UINT32 argc, UCHAR* argv[]);

API_RESULT cli_off(UINT32 argc, UCHAR* argv[]);

API_RESULT cli_seek(UINT32 argc, UCHAR* argv[]);

API_RESULT cli_group_select(UINT32 argc, UCHAR* argv[]);

API_RESULT cli_core_modelc_config_key_refresh_phase_set(UINT32 argc, UCHAR* argv[]);

API_RESULT cli_core_modelc_config_netkey_update(UINT32 argc, UCHAR* argv[]);

API_RESULT cli_demo_help(UINT32 argc, UCHAR* argv[]);

#endif

