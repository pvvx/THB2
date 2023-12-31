
/**
    \file cli_main.h

    Header File for the CLI application to test the Mindtree
    Mesh stack.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_CLI_MAIN_
#define _H_CLI_MAIN_

/* --------------------------------------------- Header File Inclusion */
#include "cliface.h"
#include "MS_access_api.h"
#include "MS_config_api.h"
#include "MS_health_server_api.h"
#include "appl_main.h"
#include "blebrr.h"

/* --------------------------------------------- Global Definitions */

/* --------------------------------------------- Structures/Data Types */

//  ---------------- add by HZF, CLI options
#define CLI_NO_MAIN
//#define CLI_PROVISION
//#define CLI_PROXY
//#define CLI_TRANSPORT
//#define CLI_NETWORK

//#define CLI_GENERICS_SERVER_MODEL
//#define CLI_GENERICS_CLIENT_MODEL

//#define CLI_GENERICS_ONOFF_SERVER_MODEL
//#define CLI_GENERICS_ONOFF_CLIENT_MODEL

//#define CLI_CONFIG_SERVER_MODEL
//#define CLI_CONFIG_CLIENT_MODEL

//#define CLI_HEALTH_SERVER_MODEL
//#define CLI_HEALTH_CLIENT_MODEL

/*
    - CLI_GENERICS_ONOFF_CLIENT_MODEL
    - CLI_GENERICS_LEVEL_CLIENT_MODEL
    - CLI_GENERICS_TRANSITIONTIME_CLIENT_MODEL
    - CLI_GENERICS_PWRONOFF_CLIENT_MODEL
    - CLI_GENERICS_PWRLEVEL_CLIENT_MODEL
    - CLI_GENERICS_BATTERY_CLIENT_MODEL
    - CLI_GENERICS_LOCATION_CLIENT_MODEL
    - CLI_GENERICS_PROPERTY_CLIENT_MODEL

    CLI_LIGHTINGS_CLIENT_MODEL                        -> Enables lightings client related CLI menu and submodel client menu with beow flags
    - CLI_LIGHTINGS_LIGHTNESS_CLIENT_MODEL
    - CLI_LIGHTINGS_CTL_CLIENT_MODEL
    - CLI_LIGHTINGS_HSL_CLIENT_MODEL
    - CLI_LIGHTINGS_XYL_CLIENT_MODEL
    - CLI_LIGHTINGS_LC_CLIENT_MODEL

    - CLI_GENERICS_LEVEL_SERVER_MODEL
    - CLI_GENERICS_TRANSITIONTIME_SERVER_MODEL
    - CLI_GENERICS_PWRONOFF_SERVER_MODEL
    - CLI_GENERICS_PWRLEVEL_SERVER_MODEL
    - CLI_GENERICS_BATTERY_SERVER_MODEL
    - CLI_GENERICS_LOCATION_SERVER_MODEL
    - CLI_GENERICS_PROPERTY_SERVER_MODEL

    CLI_LIGHTINGS_SERVER_MODEL                        -> Enables setup of lightings server model and submodel with below flags
    - CLI_LIGHTINGS_LIGHTNESS_SERVER_MODEL
    - CLI_LIGHTINGS_CTL_SERVER_MODEL
    - CLI_LIGHTINGS_HSL_SERVER_MODEL
    - CLI_LIGHTINGS_XYL_SERVER_MODEL
    - CLI_LIGHTINGS_LC_SERVER_MODEL
*/

/* --------------------------------------------- Macros */

/* Debug Macros */
/* TBD: Mapped with debug sub-system */
#define CLI_APP_ERR(...)    printf(__VA_ARGS__)
#define CLI_APP_TRC(...)    printf(__VA_ARGS__)
#define CLI_APP_INF(...)    printf(__VA_ARGS__)

/* Console Input/Output */
#define CONSOLE_OUT(...)    printf(__VA_ARGS__)
#define CONSOLE_IN(...)     scanf(__VA_ARGS__)

/* Macro to declare a CLI Command Handler */
#define CLI_CMD_HANDLER_DECL(x) \
    API_RESULT (x) (UINT32 arc, UCHAR * arv[])

/* Macro to define an empty CLI Command Handler */
#define CLI_CMD_HANDLER_EMPTY_DEF(x) \
    API_RESULT (x) (UINT32 argc, UCHAR * argv[]) \
    { \
        UINT32 index; \
        \
        CLI_APP_TRC(#x ". Argc %d\n", argc); \
        \
        for (index = 0; index < argc; index++) \
        { \
            CLI_APP_TRC("Argv[%d]: %s\n", index, argv[index]); \
        } \
        \
        return API_SUCCESS; \
    }

/* --------------------------------------------- Internal Functions */
/* CLI Command Handler declaration for configuration client */
/* Level - Root */
CLI_CMD_HANDLER_DECL( cli_help );
CLI_CMD_HANDLER_DECL( cli_core );
CLI_CMD_HANDLER_DECL( cli_model );
CLI_CMD_HANDLER_DECL( cli_reset );
CLI_CMD_HANDLER_DECL( cli_ps );
CLI_CMD_HANDLER_DECL( cli_brr );
CLI_CMD_HANDLER_DECL( cli_set_log_level );

CLI_CMD_HANDLER_DECL(cli_back);
CLI_CMD_HANDLER_DECL(cli_root);

/* Level - Core */
CLI_CMD_HANDLER_DECL( cli_core_setup );
CLI_CMD_HANDLER_DECL( cli_core_provision );
CLI_CMD_HANDLER_DECL( cli_core_proxy );
CLI_CMD_HANDLER_DECL( cli_core_transport );
CLI_CMD_HANDLER_DECL( cli_core_network );
CLI_CMD_HANDLER_DECL( cli_core_enable_feature );
CLI_CMD_HANDLER_DECL( cli_core_disable_feature );

/* Level - Core - Network */
CLI_CMD_HANDLER_DECL(cli_core_network_send_packet);
CLI_CMD_HANDLER_DECL(cli_core_network_set_pkt_hdr);
CLI_CMD_HANDLER_DECL(cli_core_network_snb);
CLI_CMD_HANDLER_DECL(cli_core_network_get_ivindex);

/* Level - Core - Transport */
CLI_CMD_HANDLER_DECL(cli_core_transport_send_packet);
CLI_CMD_HANDLER_DECL(cli_core_transport_frndreq);
CLI_CMD_HANDLER_DECL(cli_core_transport_ctrlmsg);
CLI_CMD_HANDLER_DECL(cli_core_transport_clrreplaycache);

/* Level - Core - Provision */
CLI_CMD_HANDLER_DECL(cli_provision_uuid);
CLI_CMD_HANDLER_DECL(cli_core_provision_setup);
CLI_CMD_HANDLER_DECL(cli_core_provision_bind);
CLI_CMD_HANDLER_DECL(cli_core_provision_input_auth_val);
CLI_CMD_HANDLER_DECL(cli_core_provision_set_auth_action);
CLI_CMD_HANDLER_DECL(cli_core_provision_set_dev_pkey);
CLI_CMD_HANDLER_DECL(cli_core_provision_get_pkey);
CLI_CMD_HANDLER_DECL(cli_core_provision_get_dev_list);
CLI_CMD_HANDLER_DECL(cli_core_provision_delete_all_dev_keys);

/* Level - Core - Proxy */
CLI_CMD_HANDLER_DECL(cli_core_proxy_server_op);
CLI_CMD_HANDLER_DECL(cli_core_proxy_client_op);
CLI_CMD_HANDLER_DECL(cli_core_start_proxy_adv);
CLI_CMD_HANDLER_DECL(cli_core_stop_proxy_adv);
CLI_CMD_HANDLER_DECL(cli_core_set_wl_filter);
CLI_CMD_HANDLER_DECL(cli_core_set_bl_filter);
CLI_CMD_HANDLER_DECL(cli_core_add_addr_to_filter);
CLI_CMD_HANDLER_DECL(cli_core_rm_addr_to_filter);

/* Level - Model */
CLI_CMD_HANDLER_DECL(cli_model_server);
CLI_CMD_HANDLER_DECL(cli_model_client);

/* Level - Persistent Storage */
CLI_CMD_HANDLER_DECL(cli_ps_get_device_key);
CLI_CMD_HANDLER_DECL(cli_ps_get_app_key);
CLI_CMD_HANDLER_DECL(cli_ps_get_net_key);
CLI_CMD_HANDLER_DECL(cli_ps_get_primary_unicast_addr);

/* Level - Bearer Layer */
CLI_CMD_HANDLER_DECL(cli_create_gatt_conn);
CLI_CMD_HANDLER_DECL(cli_terminate_gatt_conn);
CLI_CMD_HANDLER_DECL(cli_discover_service);
CLI_CMD_HANDLER_DECL(cli_config_ntf);
CLI_CMD_HANDLER_DECL(cli_scan_feature);
CLI_CMD_HANDLER_DECL(cli_scan_rsp_data_set);

/* Model Server - Foundation Models */
CLI_CMD_HANDLER_DECL(cli_models_foundation);

/* Model Server - Health */
CLI_CMD_HANDLER_DECL(cli_models_health);

/* Model Server - Health - Log Fault */
CLI_CMD_HANDLER_DECL(cli_models_health_log_fault);

/* Health - TODO: Temporary till model publication period is triggered */
CLI_CMD_HANDLER_DECL(cli_health_status_publish);

/* Level - Model - Server */
/* Generics */
CLI_CMD_HANDLER_DECL(cli_models_generics);

/* Model Server - Generics - OnOff */
CLI_CMD_HANDLER_DECL(cli_models_generics_onoff);

/* Model Server - Generics - Level */
CLI_CMD_HANDLER_DECL(cli_models_generics_level);

/* Model Server - Generics - Default Transition Time */
CLI_CMD_HANDLER_DECL(cli_models_generics_default_transition_time);

/* Model Server - Generics - Power OnOff */
CLI_CMD_HANDLER_DECL(cli_models_generics_power_onoff);

/* Model Server - Generics - Power Level */
CLI_CMD_HANDLER_DECL(cli_models_generics_power_level);

/* Model Server - Generics - Battery */
CLI_CMD_HANDLER_DECL(cli_models_generics_battery);

/* Model Server - Generics - Location */
CLI_CMD_HANDLER_DECL(cli_models_generics_location);

/* Model Server - Generics - Property */
CLI_CMD_HANDLER_DECL(cli_models_generics_property);

/* Sensors */
CLI_CMD_HANDLER_DECL(cli_models_sensor);

/* Time and Scene */
/* Time */
CLI_CMD_HANDLER_DECL(cli_models_time);

/* Scene */
CLI_CMD_HANDLER_DECL(cli_models_scene);

/* Scheduler */
CLI_CMD_HANDLER_DECL(cli_models_scheduler);

/* Light */
CLI_CMD_HANDLER_DECL(cli_models_light);

/* Model Server - Light Lightness */
CLI_CMD_HANDLER_DECL(cli_models_light_lightness);

/* Model Server - Light CTL */
CLI_CMD_HANDLER_DECL(cli_models_light_ctl);

/* Model Server - Light HSL */
CLI_CMD_HANDLER_DECL(cli_models_light_hsl);

/* Model Server - Light xyL */
CLI_CMD_HANDLER_DECL(cli_models_light_xyl);

/* Model Server - Light LC */
CLI_CMD_HANDLER_DECL(cli_models_light_lc);

/* Model Server - Vendor */
CLI_CMD_HANDLER_DECL(cli_models_vendor);

/* Model Server - Set Default Transition Time in ms */
CLI_CMD_HANDLER_DECL(cli_models_light_lc_set_default_trans_timeout_in_ms);

/* Reset */
/* TODO: Shall be part of common reset called from root */
CLI_CMD_HANDLER_DECL(cli_models_reset);

/* Level - Model - Client */
CLI_CMD_HANDLER_DECL(cli_modelc_config);
CLI_CMD_HANDLER_DECL(cli_modelc_health);
CLI_CMD_HANDLER_DECL(cli_modelc_generic_onoff);
CLI_CMD_HANDLER_DECL(cli_modelc_generic_level);
CLI_CMD_HANDLER_DECL(cli_modelc_generic_default_transition_time);
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_onoff);
CLI_CMD_HANDLER_DECL(cli_modelc_generic_power_level);
CLI_CMD_HANDLER_DECL(cli_modelc_generic_battery);
CLI_CMD_HANDLER_DECL(cli_modelc_generic_location);
CLI_CMD_HANDLER_DECL(cli_modelc_generic_property);
CLI_CMD_HANDLER_DECL(cli_modelc_sensor);
CLI_CMD_HANDLER_DECL(cli_modelc_time);
CLI_CMD_HANDLER_DECL(cli_modelc_scene);
CLI_CMD_HANDLER_DECL(cli_modelc_scheduler);
CLI_CMD_HANDLER_DECL(cli_modelc_light_lightness);
CLI_CMD_HANDLER_DECL(cli_modelc_light_ctl);
CLI_CMD_HANDLER_DECL(cli_modelc_light_hsl);
CLI_CMD_HANDLER_DECL(cli_modelc_light_xyl);
CLI_CMD_HANDLER_DECL(cli_modelc_light_lc);

/* Common Utility Routines */
void cli_cmd_stack_push(/* IN */ CLI_COMMAND* cmd_list, /* IN */ UINT16 cmd_list_len);
void cli_cmd_stack_pop(void);

/* Module related Utility Routines */
void cli_gatt_bearer_iface_event_pl_cb
(
    UCHAR  ev_name,
    UCHAR  ev_param
);
#endif /* _H_CLI_MAIN_ */

