
/**
    \file MS_assigned_numbers.h

    This header file describes various definitions from
    the Mesh Assigned Numbers Specification.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_ASSIGNED_NUMBERS_
#define _H_MS_ASSIGNED_NUMBERS_

/* --------------------------------- Header File Inclusion */

/* --------------------------------- Global Definitions */
/** Service UUIDs */
/** Mesh Provisioning Service */
#define MESH_PROVISIONING_SERVICE            0x1827
/** Mesh Proxy Service */
#define MESH_PROXY_SERVICE                   0x1828

/** Characteristic UUIDs */
#define MESH_CH_PROVISIONING_DATA_IN         0x2ADB
#define MESH_CH_PROVISIONING_DATA_OUT        0x2ADC
#define MESH_CH_PROXY_DATA_IN                0x2ADD
#define MESH_CH_PROXY_DATA_OUT               0x2ADE

/** Advertising Type */
#define MESH_AD_TYPE_PB_ADV                  0x29
#define MESH_AD_TYPE_PKT                     0x2A
#define MESH_AD_TYPE_BCON                    0x2B

/** GATT PDU Types */
#define MESH_GATT_TYPE_NETWORK               0x00
#define MESH_GATT_TYPE_BEACON                0x01
#define MESH_GATT_TYPE_PROXY                 0x02
#define MESH_GATT_TYPE_PROV                  0x03

/** GATT Segmentation & Reassembly (SAR) constants */
#define MESH_GATT_SAR_COMPLETE               0x00
#define MESH_GATT_SAR_START                  0x01
#define MESH_GATT_SAR_CONTINUE               0x02
#define MESH_GATT_SAR_END                    0x03

/** Model type definitions */
#define MS_ACCESS_MODEL_TYPE_SIG                            0x00
#define MS_ACCESS_MODEL_TYPE_VENDOR                         0x01

/** Mesh Nonce Types */
#define MS_NONCE_T_NETWORK                                  0x00
#define MS_NONCE_T_APPLICATION                              0x01
#define MS_NONCE_T_DEVICE                                   0x02
#define MS_NONCE_T_PROXY                                    0x03

/** Opcode definitions for the Foundation Model */

/** 8-bit Opcodes of Model specific messages */
#define MS_ACCESS_CONFIG_APPKEY_ADD_OPCODE                                      0x00
#define MS_ACCESS_CONFIG_APPKEY_UPDATE_OPCODE                                   0x01
#define MS_ACCESS_CONFIG_COMPOSITION_DATA_STATUS_OPCODE                         0x02
#define MS_ACCESS_CONFIG_MODEL_PUBLICATION_SET_OPCODE                           0x03
#define MS_ACCESS_HEALTH_CURRENT_STATUS_OPCODE                                  0x04
#define MS_ACCESS_HEALTH_FAULT_STATUS_OPCODE                                    0x05
#define MS_ACCESS_CONFIG_HEARTBEAT_PUBLICATION_STATUS_OPCODE                    0x06

/** 16-bit Opcodes of Model specific messages */
/** Config AppKey Delete Opcode */
#define MS_ACCESS_CONFIG_APPKEY_DELETE_OPCODE                                   0x8000
/** Config AppKey Get Opcode */
#define MS_ACCESS_CONFIG_APPKEY_GET_OPCODE                                      0x8001
/** Config AppKey List Opcode */
#define MS_ACCESS_CONFIG_APPKEY_LIST_OPCODE                                     0x8002
/** Config AppKey Status Opcode */
#define MS_ACCESS_CONFIG_APPKEY_STATUS_OPCODE                                   0x8003
/** Health Attention Get Opcode */
#define MS_ACCESS_HEALTH_ATTENTION_GET_OPCODE                                   0x8004
/** Health Attention Set Opcode */
#define MS_ACCESS_HEALTH_ATTENTION_SET_OPCODE                                   0x8005
/** Health Attention Set Unacknowledged Opcode */
#define MS_ACCESS_HEALTH_ATTENTION_SET_UNACKNOWLEDGED_OPCODE                    0x8006
/** Health Attention Status Opcode */
#define MS_ACCESS_HEALTH_ATTENTION_STATUS_OPCODE                                0x8007
/** Config Composition Data Get Opcode */
#define MS_ACCESS_CONFIG_COMPOSITION_DATA_GET_OPCODE                            0x8008
/** Config Beacon Get Opcode */
#define MS_ACCESS_CONFIG_BEACON_GET_OPCODE                                      0x8009
/** Config Beacon Set Opcode */
#define MS_ACCESS_CONFIG_BEACON_SET_OPCODE                                      0x800A
/** Config Beacon Status Opcode */
#define MS_ACCESS_CONFIG_BEACON_STATUS_OPCODE                                   0x800B
/** Config Deafault TTL Get Opcode */
#define MS_ACCESS_CONFIG_DEFAULT_TTL_GET_OPCODE                                 0x800C
/** Config Deafault TTL Set Opcode */
#define MS_ACCESS_CONFIG_DEFAULT_TTL_SET_OPCODE                                 0x800D
/** Config Deafault TTL Status Opcode */
#define MS_ACCESS_CONFIG_DEFAULT_TTL_STATUS_OPCODE                              0x800E
/** Config Friend Get Opcode */
#define MS_ACCESS_CONFIG_FRIEND_GET_OPCODE                                      0x800F
/** Config Friend Set Opcode */
#define MS_ACCESS_CONFIG_FRIEND_SET_OPCODE                                      0x8010
/** Config Friend Status Opcode */
#define MS_ACCESS_CONFIG_FRIEND_STATUS_OPCODE                                   0x8011
/** Config GATT Proxy Get Opcode */
#define MS_ACCESS_CONFIG_GATT_PROXY_GET_OPCODE                                  0x8012
/** Config GATT Proxy Set Opcode */
#define MS_ACCESS_CONFIG_GATT_PROXY_SET_OPCODE                                  0x8013
/** Config GATT Proxy Status Opcode */
#define MS_ACCESS_CONFIG_GATT_PROXY_STATUS_OPCODE                               0x8014
/** Config Key Refresh Phase Get Opcode */
#define MS_ACCESS_CONFIG_KEY_REFRESH_PHASE_GET_OPCODE                           0x8015
/** Config Key Refresh Phase Set Opcode */
#define MS_ACCESS_CONFIG_KEY_REFRESH_PHASE_SET_OPCODE                           0x8016
/** Config Key Refresh Phase Status Opcode */
#define MS_ACCESS_CONFIG_KEY_REFRESH_PHASE_STATUS_OPCODE                        0x8017
/** Config Model Publication Get Opcode */
#define MS_ACCESS_CONFIG_MODEL_PUBLICATION_GET_OPCODE                           0x8018
/** Config Model Publication Status Opcode */
#define MS_ACCESS_CONFIG_MODEL_PUBLICATION_STATUS_OPCODE                        0x8019
/** Config Model Publication Virtual Address Set Opcode */
#define MS_ACCESS_CONFIG_MODEL_PUBLICATION_VIRTUAL_ADDRESS_SET_OPCODE           0x801A
/** Config Model Subscription Add Opcode */
#define MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_ADD_OPCODE                          0x801B
/** Config Model Subscription Delete Opcode */
#define MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_DELETE_OPCODE                       0x801C
/** Config Model Subscription Delete All Opcode */
#define MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_DELETE_ALL_OPCODE                   0x801D
/** Config Model Subscription Overwrite Opcode */
#define MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_OVERWRITE_OPCODE                    0x801E
/** Config Model Subscription Status Opcode */
#define MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_STATUS_OPCODE                       0x801F
/** Config Model Subscription Virtual Address Add Opcode */
#define MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_ADD_OPCODE          0x8020
/** Config Model Subscription Virtual Address Delete Opcode */
#define MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_DELETE_OPCODE       0x8021
/** Config Model Subscription Virtual Address Overwrite Opcode */
#define MS_ACCESS_CONFIG_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_OVERWRITE_OPCODE    0x8022
/** Config Network Transmit Get Opcode */
#define MS_ACCESS_CONFIG_NETWORK_TRANSMIT_GET_OPCODE                            0x8023
/** Config Network Transmit Set Opcode */
#define MS_ACCESS_CONFIG_NETWORK_TRANSMIT_SET_OPCODE                            0x8024
/** Config Network Transmit Status Opcode */
#define MS_ACCESS_CONFIG_NETWORK_TRANSMIT_STATUS_OPCODE                         0x8025
/** Config Relay Get Opcode */
#define MS_ACCESS_CONFIG_RELAY_GET_OPCODE                                       0x8026
/** Config Relay Set Opcode */
#define MS_ACCESS_CONFIG_RELAY_SET_OPCODE                                       0x8027
/** Config Relay Status Opcode */
#define MS_ACCESS_CONFIG_RELAY_STATUS_OPCODE                                    0x8028
/** Config SIG Model Subscription Get Opcode */
#define MS_ACCESS_CONFIG_SIG_MODEL_SUBSCRIPTION_GET_OPCODE                      0x8029
/** Config SIG Model Subscription List Opcode */
#define MS_ACCESS_CONFIG_SIG_MODEL_SUBSCRIPTION_LIST_OPCODE                     0x802A
/** Config Vendor Model Subscription Get Opcode */
#define MS_ACCESS_CONFIG_VENDOR_MODEL_SUBSCRIPTION_GET_OPCODE                   0x802B
/** Config Vendor Model Subscription List Opcode */
#define MS_ACCESS_CONFIG_VENDOR_MODEL_SUBSCRIPTION_LIST_OPCODE                  0x802C
/** Config Low Power Node PollTimeout Get Opcode */
#define MS_ACCESS_CONFIG_LOW_POWER_NODE_POLLTIMEOUT_GET_OPCODE                  0x802D
/** Config Low Power Node PollTimeout Status Opcode */
#define MS_ACCESS_CONFIG_LOW_POWER_NODE_POLLTIMEOUT_STATUS_OPCODE               0x802E
/** Health Fault Clear Opcode */
#define MS_ACCESS_HEALTH_FAULT_CLEAR_OPCODE                                     0x802F
/** Health Fault Clear Unacknowledged Opcode */
#define MS_ACCESS_HEALTH_FAULT_CLEAR_UNACKNOWLEDGED_OPCODE                      0x8030
/** Health Fault Get Opcode */
#define MS_ACCESS_HEALTH_FAULT_GET_OPCODE                                       0x8031
/** Health Fault Test Opcode */
#define MS_ACCESS_HEALTH_FAULT_TEST_OPCODE                                      0x8032
/** Health Fault Test Unacknowledged Opcode */
#define MS_ACCESS_HEALTH_FAULT_TEST_UNACKNOWLEDGED_OPCODE                       0x8033
/** Health Period Get Opcode */
#define MS_ACCESS_HEALTH_PERIOD_GET_OPCODE                                      0x8034
/** Health Period Set Opcode */
#define MS_ACCESS_HEALTH_PERIOD_SET_OPCODE                                      0x8035
/** Health Period Set Unacknowledged Opcode */
#define MS_ACCESS_HEALTH_PERIOD_SET_UNACKNOWLEDGED_OPCODE                       0x8036
/** Health Period Status Opcode */
#define MS_ACCESS_HEALTH_PERIOD_STATUS_OPCODE                                   0x8037
/** Config Heartbeat Publication Get Opcode */
#define MS_ACCESS_CONFIG_HEARTBEAT_PUBLICATION_GET_OPCODE                       0x8038
/** Config Heartbeat Publication Set Opcode */
#define MS_ACCESS_CONFIG_HEARTBEAT_PUBLICATION_SET_OPCODE                       0x8039
/** Config Heartbeat Subscription Get Opcode */
#define MS_ACCESS_CONFIG_HEARTBEAT_SUBSCRIPTION_GET_OPCODE                      0x803A
/** Config Heartbeat Subscription Set Opcode */
#define MS_ACCESS_CONFIG_HEARTBEAT_SUBSCRIPTION_SET_OPCODE                      0x803B
/** Config Heartbeat Subscription Status Opcode */
#define MS_ACCESS_CONFIG_HEARTBEAT_SUBSCRIPTION_STATUS_OPCODE                   0x803C
/** Config Model App Bind Opcode */
#define MS_ACCESS_CONFIG_MODEL_APP_BIND_OPCODE                                  0x803D
/** Config Model App Status Opcode */
#define MS_ACCESS_CONFIG_MODEL_APP_STATUS_OPCODE                                0x803E
/** Config Model App Unbind Opcode */
#define MS_ACCESS_CONFIG_MODEL_APP_UNBIND_OPCODE                                0x803F
/** Config NetKey Add Opcode */
#define MS_ACCESS_CONFIG_NETKEY_ADD_OPCODE                                      0x8040
/** Config NetKey Delete Opcode */
#define MS_ACCESS_CONFIG_NETKEY_DELETE_OPCODE                                   0x8041
/** Config NetKey Get Opcode */
#define MS_ACCESS_CONFIG_NETKEY_GET_OPCODE                                      0x8042
/** Config NetKey List Opcode */
#define MS_ACCESS_CONFIG_NETKEY_LIST_OPCODE                                     0x8043
/** Config NetKey Status Opcode */
#define MS_ACCESS_CONFIG_NETKEY_STATUS_OPCODE                                   0x8044
/** Config NetKey Update Opcode */
#define MS_ACCESS_CONFIG_NETKEY_UPDATE_OPCODE                                   0x8045
/** Config Node Identity Get Opcode */
#define MS_ACCESS_CONFIG_NODE_IDENTITY_GET_OPCODE                               0x8046
/** Config Node Identity Set Opcode */
#define MS_ACCESS_CONFIG_NODE_IDENTITY_SET_OPCODE                               0x8047
/** Config Node Identity Status Opcode */
#define MS_ACCESS_CONFIG_NODE_IDENTITY_STATUS_OPCODE                            0x8048
/** Config Node Reset Opcode */
#define MS_ACCESS_CONFIG_NODE_RESET_OPCODE                                      0x8049
/** Config Node Reset Status Opcode */
#define MS_ACCESS_CONFIG_NODE_RESET_STATUS_OPCODE                               0x804A
/** Config SIG Model App Get Opcode */
#define MS_ACCESS_CONFIG_SIG_MODEL_APP_GET_OPCODE                               0x804B
/** Config SIG Model App List Opcode */
#define MS_ACCESS_CONFIG_SIG_MODEL_APP_LIST_OPCODE                              0x804C
/** Config Vendor Model App Get Opcode */
#define MS_ACCESS_CONFIG_VENDOR_MODEL_APP_GET_OPCODE                            0x804D
/** Config Vendor Model App List Opcode */
#define MS_ACCESS_CONFIG_VENDOR_MODEL_APP_LIST_OPCODE                           0x804E

/** Generic OnOff */
/** Generic OnOff Get Opcode */
#define MS_ACCESS_GENERIC_ONOFF_GET_OPCODE                                      0x8201
/** Generic OnOff Set Opcode */
#define MS_ACCESS_GENERIC_ONOFF_SET_OPCODE                                      0x8202
/** Generic OnOff Set Unacknowledged Opcode */
#define MS_ACCESS_GENERIC_ONOFF_SET_UNACKNOWLEDGED_OPCODE                       0x8203
/** Generic OnOff Status Opcode */
#define MS_ACCESS_GENERIC_ONOFF_STATUS_OPCODE                                   0x8204

/** Generic Level */
/** Generic Level Get Opcode */
#define MS_ACCESS_GENERIC_LEVEL_GET_OPCODE                                      0x8205
/** Generic Level Set Opcode */
#define MS_ACCESS_GENERIC_LEVEL_SET_OPCODE                                      0x8206
/** Generic Level Set Unacknowledged Opcode */
#define MS_ACCESS_GENERIC_LEVEL_SET_UNACKNOWLEDGED_OPCODE                       0x8207
/** Generic Level Status Opcode */
#define MS_ACCESS_GENERIC_LEVEL_STATUS_OPCODE                                   0x8208
/** Generic Delta Set Opcode */
#define MS_ACCESS_GENERIC_DELTA_SET_OPCODE                                      0x8209
/** Generic Delta Set Unacknowledged Opcode */
#define MS_ACCESS_GENERIC_DELTA_SET_UNACKNOWLEDGED_OPCODE                       0x820A
/** Generic Move Set Opcode */
#define MS_ACCESS_GENERIC_MOVE_SET_OPCODE                                       0x820B
/** Generic Move Set Unacknowledged Opcode */
#define MS_ACCESS_GENERIC_MOVE_SET_UNACKNOWLEDGED_OPCODE                        0x820C

/** Generic Default Transition Time */
/** Generic Default Transition Time Get Opcode */
#define MS_ACCESS_GENERIC_DEFAULT_TRANSITION_TIME_GET_OPCODE                    0x820D
/** Generic Default Transition Time Set Opcode */
#define MS_ACCESS_GENERIC_DEFAULT_TRANSITION_TIME_SET_OPCODE                    0x820E
/** Generic Default Transition Time Set Unacknowledged Opcode */
#define MS_ACCESS_GENERIC_DEFAULT_TRANSITION_TIME_SET_UNACKNOWLEDGED_OPCODE     0x820F
/** Generic Default Transition Time Status Opcode */
#define MS_ACCESS_GENERIC_DEFAULT_TRANSITION_TIME_STATUS_OPCODE                 0x8210

/** Generic Power OnOff */
/** Generic Power OnOff Get Opcode */
#define MS_ACCESS_GENERIC_ONPOWERUP_GET_OPCODE                                  0x8211
/** Generic Power OnOff Status Opcode */
#define MS_ACCESS_GENERIC_ONPOWERUP_STATUS_OPCODE                               0x8212

/** Generic Power OnOff Setup */
/** Generic Power OnOff Setup Set Opcode */
#define MS_ACCESS_GENERIC_ONPOWERUP_SET_OPCODE                                  0x8213
/** Generic Power OnOff Setup Set Unacknowledged Opcode */
#define MS_ACCESS_GENERIC_ONPOWERUP_SET_UNACKNOWLEDGED_OPCODE                   0x8214

/** Generic Power Level */
/** Generic Power Level Get Opcode */
#define MS_ACCESS_GENERIC_POWER_LEVEL_GET_OPCODE                                0x8215
/** Generic Power Level Set Opcode */
#define MS_ACCESS_GENERIC_POWER_LEVEL_SET_OPCODE                                0x8216
/** Generic Power Level Set Unacknowledged Opcode */
#define MS_ACCESS_GENERIC_POWER_LEVEL_SET_UNACKNOWLEDGED_OPCODE                 0x8217
/** Generic Power Level Status Opcode */
#define MS_ACCESS_GENERIC_POWER_LEVEL_STATUS_OPCODE                             0x8218
/** Generic Power Last Get Opcode */
#define MS_ACCESS_GENERIC_POWER_LAST_GET_OPCODE                                 0x8219
/** Generic Power Last Status Opcode */
#define MS_ACCESS_GENERIC_POWER_LAST_STATUS_OPCODE                              0x821A
/** Generic Power Default Get Opcode */
#define MS_ACCESS_GENERIC_POWER_DEFAULT_GET_OPCODE                              0x821B
/** Generic Power Default Status Opcode */
#define MS_ACCESS_GENERIC_POWER_DEFAULT_STATUS_OPCODE                           0x821C
/** Generic Power Range Get Opcode */
#define MS_ACCESS_GENERIC_POWER_RANGE_GET_OPCODE                                0x821D
/** Generic Power Range Status Opcode */
#define MS_ACCESS_GENERIC_POWER_RANGE_STATUS_OPCODE                             0x821E

/** Generic Power Level Setup */
/** Generic Power Default Set Opcode */
#define MS_ACCESS_GENERIC_POWER_DEFAULT_SET_OPCODE                              0x821F
/** Generic Power Default Set Unacknowledged Opcode */
#define MS_ACCESS_GENERIC_POWER_DEFAULT_SET_UNACKNOWLEDGED_OPCODE               0x8220
/** Generic Power Range Set Opcode */
#define MS_ACCESS_GENERIC_POWER_RANGE_SET_OPCODE                                0x8221
/** Generic Power Range Set Unacknowledged Opcode */
#define MS_ACCESS_GENERIC_POWER_RANGE_SET_UNACKNOWLEDGED_OPCODE                 0x8222

/** Generic Battery */
/** Generic Battery Get Opcode */
#define MS_ACCESS_GENERIC_BATTERY_GET_OPCODE                                    0x8223
/** Generic Battery Status Opcode */
#define MS_ACCESS_GENERIC_BATTERY_STATUS_OPCODE                                 0x8224

/** Generic Location */
/** Generic Location Global Get Opcode */
#define MS_ACCESS_GENERIC_LOCATION_GLOBAL_GET_OPCODE                            0x8225
/** Generic Location Global Status Opcode */
#define MS_ACCESS_GENERIC_LOCATION_GLOBAL_STATUS_OPCODE                         0x40
/** Generic Location Local Get Opcode */
#define MS_ACCESS_GENERIC_LOCATION_LOCAL_GET_OPCODE                             0x8226
/** Generic Location Local Status Opcode */
#define MS_ACCESS_GENERIC_LOCATION_LOCAL_STATUS_OPCODE                          0x8227

/** Generic Location Setup */
/** Generic Location Global Set Opcode */
#define MS_ACCESS_GENERIC_LOCATION_GLOBAL_SET_OPCODE                            0x41
/** Generic Location Global Set Unacknowledged Opcode */
#define MS_ACCESS_GENERIC_LOCATION_GLOBAL_SET_UNACKNOWLEDGED_OPCODE             0x42
/** Generic Location Local Set Opcode */
#define MS_ACCESS_GENERIC_LOCATION_LOCAL_SET_OPCODE                             0x8228
/** Generic Location Local Set Unacknowledged Opcode */
#define MS_ACCESS_GENERIC_LOCATION_LOCAL_SET_UNACKNOWLEDGED_OPCODE              0x8229

/** Generic Manufacturer Property */
/** Generic Manufacturer Properties Get Opcode */
#define MS_ACCESS_GENERIC_MANUFACTURER_PROPERTIES_GET_OPCODE                    0x822A
/** Generic Manufacturer Properties Status Opcode */
#define MS_ACCESS_GENERIC_MANUFACTURER_PROPERTIES_STATUS_OPCODE                 0x43
/** Generic Manufacturer Property Get Opcode */
#define MS_ACCESS_GENERIC_MANUFACTURER_PROPERTY_GET_OPCODE                      0x822B
/** Generic Manufacturer Property Set Opcode */
#define MS_ACCESS_GENERIC_MANUFACTURER_PROPERTY_SET_OPCODE                      0x44
/** Generic Manufacturer Property Set Unacknowledged Opcode */
#define MS_ACCESS_GENERIC_MANUFACTURER_PROPERTY_SET_UNACKNOWLEDGED_OPCODE       0x45
/** Generic Manufacturer Property Status Opcode */
#define MS_ACCESS_GENERIC_MANUFACTURER_PROPERTY_STATUS_OPCODE                   0x46

/** Generic Admin Property */
/** Generic Admin Properties Get Opcode */
#define MS_ACCESS_GENERIC_ADMIN_PROPERTIES_GET_OPCODE                           0x822C
/** Generic Admin Properties Status Opcode */
#define MS_ACCESS_GENERIC_ADMIN_PROPERTIES_STATUS_OPCODE                        0x47
/** Generic Admin Property Get Opcode */
#define MS_ACCESS_GENERIC_ADMIN_PROPERTY_GET_OPCODE                             0x822D
/** Generic Admin Property Set Opcode */
#define MS_ACCESS_GENERIC_ADMIN_PROPERTY_SET_OPCODE                             0x48
/** Generic Admin Property Set Unacknowledged Opcode */
#define MS_ACCESS_GENERIC_ADMIN_PROPERTY_SET_UNACKNOWLEDGED_OPCODE              0x49
/** Generic Admin Property Status Opcode */
#define MS_ACCESS_GENERIC_ADMIN_PROPERTY_STATUS_OPCODE                          0x4A

/** Generic User Property */
/** Generic User Properties Get Opcode */
#define MS_ACCESS_GENERIC_USER_PROPERTIES_GET_OPCODE                            0x822E
/** Generic User Properties Status Opcode */
#define MS_ACCESS_GENERIC_USER_PROPERTIES_STATUS_OPCODE                         0x4B
/** Generic User Property Get Opcode */
#define MS_ACCESS_GENERIC_USER_PROPERTY_GET_OPCODE                              0x822F
/** Generic User Property Set Opcode */
#define MS_ACCESS_GENERIC_USER_PROPERTY_SET_OPCODE                              0x4C
/** Generic User Property Set Unacknowledged Opcode */
#define MS_ACCESS_GENERIC_USER_PROPERTY_SET_UNACKNOWLEDGED_OPCODE               0x4D
/** Generic User Property Status Opcode */
#define MS_ACCESS_GENERIC_USER_PROPERTY_STATUS_OPCODE                           0x4E

/** Generic Client Property */
/** Generic Client Properties Get Opcode */
#define MS_ACCESS_GENERIC_CLIENT_PROPERTIES_GET_OPCODE                          0x4F
/** Generic Client Properties Status Opcode */
#define MS_ACCESS_GENERIC_CLIENT_PROPERTIES_STATUS_OPCODE                       0x50

/** Sensor */
/** Sensor Descriptor Get Opcode */
#define MS_ACCESS_SENSOR_DESCRIPTOR_GET_OPCODE                                  0x8230
/** Sensor Descriptor Status Opcode */
#define MS_ACCESS_SENSOR_DESCRIPTOR_STATUS_OPCODE                               0x51
/** Sensor Get Opcode */
#define MS_ACCESS_SENSOR_GET_OPCODE                                             0x8231
/** Sensor Status Opcode */
#define MS_ACCESS_SENSOR_STATUS_OPCODE                                          0x52
/** Sensor Column Get Opcode */
#define MS_ACCESS_SENSOR_COLUMN_GET_OPCODE                                      0x8232
/** Sensor Column Status Opcode */
#define MS_ACCESS_SENSOR_COLUMN_STATUS_OPCODE                                   0x53
/** Sensor Series Get Opcode */
#define MS_ACCESS_SENSOR_SERIES_GET_OPCODE                                      0x8233
/** Sensor Series Status Opcode */
#define MS_ACCESS_SENSOR_SERIES_STATUS_OPCODE                                   0x54

/** Sensor Setup */
/** Sensor Cadence Get Opcode */
#define MS_ACCESS_SENSOR_CADENCE_GET_OPCODE                                     0x8234
/** Sensor Cadence Set Opcode */
#define MS_ACCESS_SENSOR_CADENCE_SET_OPCODE                                     0x55
/** Sensor Cadence Set Unacknowledged Opcode */
#define MS_ACCESS_SENSOR_CADENCE_SET_UNACKNOWLEDGED_OPCODE                      0x56
/** Sensor Cadence Status Opcode */
#define MS_ACCESS_SENSOR_CADENCE_STATUS_OPCODE                                  0x57
/** Sensor Settings Get Opcode */
#define MS_ACCESS_SENSOR_SETTINGS_GET_OPCODE                                    0x8235
/** Sensor Settings Status Opcode */
#define MS_ACCESS_SENSOR_SETTINGS_STATUS_OPCODE                                 0x58
/** Sensor Setting Get Opcode */
#define MS_ACCESS_SENSOR_SETTING_GET_OPCODE                                     0x8236
/** Sensor Setting Set Opcode */
#define MS_ACCESS_SENSOR_SETTING_SET_OPCODE                                     0x59
/** Sensor Setting Set Unacknowledged Opcode */
#define MS_ACCESS_SENSOR_SETTING_SET_UNACKNOWLEDGED_OPCODE                      0x5A
/** Sensor Setting Status Opcode */
#define MS_ACCESS_SENSOR_SETTING_STATUS_OPCODE                                  0x5B

/** Time */
/** Time Get Opcode */
#define MS_ACCESS_TIME_GET_OPCODE                                               0x8237
/** Time Set Opcode */
#define MS_ACCESS_TIME_SET_OPCODE                                               0x5C
/** Time Status Opcode */
#define MS_ACCESS_TIME_STATUS_OPCODE                                            0x5D
/** Time Role Get Opcode */
#define MS_ACCESS_TIME_ROLE_GET_OPCODE                                          0x8238
/** Time Role Set Opcode */
#define MS_ACCESS_TIME_ROLE_SET_OPCODE                                          0x8239
/** Time Role Status Opcode */
#define MS_ACCESS_TIME_ROLE_STATUS_OPCODE                                       0x823A
/** Time Zone Get Opcode */
#define MS_ACCESS_TIME_ZONE_GET_OPCODE                                          0x823B
/** Time Zone Set Opcode */
#define MS_ACCESS_TIME_ZONE_SET_OPCODE                                          0x823C
/** Time Zone Status Opcode */
#define MS_ACCESS_TIME_ZONE_STATUS_OPCODE                                       0x823D
/** Time - TAI UTC Delta Get Opcode */
#define MS_ACCESS_TAI_UTC_DELTA_GET_OPCODE                                      0x823E
/** Time - TAI UTC Delta Set Opcode */
#define MS_ACCESS_TAI_UTC_DELTA_SET_OPCODE                                      0x823F
/** Time - TAI UTC Delta Status Opcode */
#define MS_ACCESS_TAI_UTC_DELTA_STATUS_OPCODE                                   0x8240

/** Scene */
/** Scene Get Opcode */
#define MS_ACCESS_SCENE_GET_OPCODE                                              0x8241
/** Scene Recall Opcode */
#define MS_ACCESS_SCENE_RECALL_OPCODE                                           0x8242
/** Scene Recall Unacknowledged Opcode */
#define MS_ACCESS_SCENE_RECALL_UNACKNOWLEDGED_OPCODE                            0x8243
/** Scene Status Opcode */
#define MS_ACCESS_SCENE_STATUS_OPCODE                                           0x5E
/** Scene Register Get Opcode */
#define MS_ACCESS_SCENE_REGISTER_GET_OPCODE                                     0x8244
/** Scene Register Status Opcode */
#define MS_ACCESS_SCENE_REGISTER_STATUS_OPCODE                                  0x8245

/** Scene Setup */
/** Scene Store Opcode */
#define MS_ACCESS_SCENE_STORE_OPCODE                                            0x8246
/** Scene Store Unacknowledged Opcode */
#define MS_ACCESS_SCENE_STORE_UNACKNOWLEDGED_OPCODE                             0x8247
/** Scene Delete Opcode */
#define MS_ACCESS_SCENE_DELETE_OPCODE                                           0x829E
/** Scene Delete Unacknowledged Opcode */
#define MS_ACCESS_SCENE_DELETE_UNACKNOWLEDGED_OPCODE                            0x829F

/** Scheduler */
/** Scheduler Action Get Opcode */
#define MS_ACCESS_SCHEDULER_ACTION_GET_OPCODE                                   0x8248
/** Scheduler Action Status Opcode */
#define MS_ACCESS_SCHEDULER_ACTION_STATUS_OPCODE                                0x5F
/** Scheduler Get Opcode */
#define MS_ACCESS_SCHEDULER_GET_OPCODE                                          0x8249
/** Scheduler Status Opcode */
#define MS_ACCESS_SCHEDULER_STATUS_OPCODE                                       0x824A

/** Scheduler Setup */
/** Scheduler Action Set Opcode */
#define MS_ACCESS_SCHEDULER_ACTION_SET_OPCODE                                   0x60
/** Scheduler Action Set Unacknowledged Opcode */
#define MS_ACCESS_SCHEDULER_ACTION_SET_UNACKNOWLEDGED_OPCODE                    0x61

/** Light Lightness */
/** Light Lightness Get Opcode */
#define MS_ACCESS_LIGHT_LIGHTNESS_GET_OPCODE                                    0x824B
/** Light Lightness Set Opcode */
#define MS_ACCESS_LIGHT_LIGHTNESS_SET_OPCODE                                    0x824C
/** Light Lightness Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_LIGHTNESS_SET_UNACKNOWLEDGED_OPCODE                     0x824D
/** Light Lightness Status Opcode */
#define MS_ACCESS_LIGHT_LIGHTNESS_STATUS_OPCODE                                 0x824E
/** Light Lightness Linear Get Opcode */
#define MS_ACCESS_LIGHT_LIGHTNESS_LINEAR_GET_OPCODE                             0x824F
/** Light Lightness Linear Set Opcode */
#define MS_ACCESS_LIGHT_LIGHTNESS_LINEAR_SET_OPCODE                             0x8250
/** Light Lightness Linear Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_LIGHTNESS_LINEAR_SET_UNACKNOWLEDGED_OPCODE              0x8251
/** Light Lightness Linear Status Opcode */
#define MS_ACCESS_LIGHT_LIGHTNESS_LINEAR_STATUS_OPCODE                          0x8252
/** Light Lightness Last Get Opcode */
#define MS_ACCESS_LIGHT_LIGHTNESS_LAST_GET_OPCODE                               0x8253
/** Light Lightness Last Status Opcode */
#define MS_ACCESS_LIGHT_LIGHTNESS_LAST_STATUS_OPCODE                            0x8254
/** Light Lightness Default Get Opcode */
#define MS_ACCESS_LIGHT_LIGHTNESS_DEFAULT_GET_OPCODE                            0x8255
/** Light Lightness Default Status Opcode */
#define MS_ACCESS_LIGHT_LIGHTNESS_DEFAULT_STATUS_OPCODE                         0x8256
/** Light Lightness Range Get Opcode */
#define MS_ACCESS_LIGHT_LIGHTNESS_RANGE_GET_OPCODE                              0x8257
/** Light Lightness Range Status Opcode */
#define MS_ACCESS_LIGHT_LIGHTNESS_RANGE_STATUS_OPCODE                           0x8258

/** Light Lightness Setup */
/** Light Lightness Range Set Opcode */
#define MS_ACCESS_LIGHT_LIGHTNESS_DEFAULT_SET_OPCODE                            0x8259
/** Light Lightness Range Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_LIGHTNESS_DEFAULT_SET_UNACKNOWLEDGED_OPCODE             0x825A
/** Light Lightness Range Set Opcode */
#define MS_ACCESS_LIGHT_LIGHTNESS_RANGE_SET_OPCODE                              0x825B
/** Light Lightness Range Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_LIGHTNESS_RANGE_SET_UNACKNOWLEDGED_OPCODE               0x825C

/** Light CTL */
/** Light CTL Get Opcode */
#define MS_ACCESS_LIGHT_CTL_GET_OPCODE                                          0x825D
/** Light CTL Set Opcode */
#define MS_ACCESS_LIGHT_CTL_SET_OPCODE                                          0x825E
/** Light CTL Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_CTL_SET_UNACKNOWLEDGED_OPCODE                           0x825F
/** Light CTL Status Opcode */
#define MS_ACCESS_LIGHT_CTL_STATUS_OPCODE                                       0x8260
/** Light CTL Temperature Get Opcode */
#define MS_ACCESS_LIGHT_CTL_TEMPERATURE_GET_OPCODE                              0x8261
/** Light CTL Temperature Range Get Opcode */
#define MS_ACCESS_LIGHT_CTL_TEMPERATURE_RANGE_GET_OPCODE                        0x8262
/** Light CTL Temperature Range Status Opcode */
#define MS_ACCESS_LIGHT_CTL_TEMPERATURE_RANGE_STATUS_OPCODE                     0x8263
/** Light CTL Temperature Set Opcode */
#define MS_ACCESS_LIGHT_CTL_TEMPERATURE_SET_OPCODE                              0x8264
/** Light CTL Temperature Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_CTL_TEMPERATURE_SET_UNACKNOWLEDGED_OPCODE               0x8265
/** Light CTL Temperature Status Opcode */
#define MS_ACCESS_LIGHT_CTL_TEMPERATURE_STATUS_OPCODE                           0x8266
/** Light CTL Default Get Opcode */
#define MS_ACCESS_LIGHT_CTL_DEFAULT_GET_OPCODE                                  0x8267
/** Light CTL Default Status Opcode */
#define MS_ACCESS_LIGHT_CTL_DEFAULT_STATUS_OPCODE                               0x8268

/** Light CTL Setup */
/** Light CTL Default Set Opcode */
#define MS_ACCESS_LIGHT_CTL_DEFAULT_SET_OPCODE                                  0x8269
/** Light CTL Default Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_CTL_DEFAULT_SET_UNACKNOWLEDGED_OPCODE                   0x826A
/** Light CTL Default Range Set Opcode */
#define MS_ACCESS_LIGHT_CTL_TEMPERATURE_RANGE_SET_OPCODE                        0x826B
/** Light CTL Default Range Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_CTL_TEMPERATURE_RANGE_SET_UNACKNOWLEDGED_OPCODE         0x826C

/** Light HSL */
/** Light HSL Get Opcode */
#define MS_ACCESS_LIGHT_HSL_GET_OPCODE                                          0x826D
/** Light HSL HUE Get Opcode */
#define MS_ACCESS_LIGHT_HSL_HUE_GET_OPCODE                                      0x826E
/** Light HSL HUE Set Opcode */
#define MS_ACCESS_LIGHT_HSL_HUE_SET_OPCODE                                      0x826F
/** Light HSL HUE Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_HSL_HUE_SET_UNACKNOWLEDGED_OPCODE                       0x8270
/** Light HSL HUE Status Opcode */
#define MS_ACCESS_LIGHT_HSL_HUE_STATUS_OPCODE                                   0x8271
/** Light HSL Saturation Get Opcode */
#define MS_ACCESS_LIGHT_HSL_SATURATION_GET_OPCODE                               0x8272
/** Light HSL Saturation Set Opcode */
#define MS_ACCESS_LIGHT_HSL_SATURATION_SET_OPCODE                               0x8273
/** Light HSL Saturation Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_HSL_SATURATION_SET_UNACKNOWLEDGED_OPCODE                0x8274
/** Light HSL Saturation Status Opcode */
#define MS_ACCESS_LIGHT_HSL_SATURATION_STATUS_OPCODE                            0x8275
/** Light HSL Set Opcode */
#define MS_ACCESS_LIGHT_HSL_SET_OPCODE                                          0x8276
/** Light HSL Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_HSL_SET_UNACKNOWLEDGED_OPCODE                           0x8277
/** Light HSL Status Opcode */
#define MS_ACCESS_LIGHT_HSL_STATUS_OPCODE                                       0x8278
/** Light HSL Target Get Opcode */
#define MS_ACCESS_LIGHT_HSL_TARGET_GET_OPCODE                                   0x8279
/** Light HSL Target Status Opcode */
#define MS_ACCESS_LIGHT_HSL_TARGET_STATUS_OPCODE                                0x827A
/** Light HSL Default Get Opcode */
#define MS_ACCESS_LIGHT_HSL_DEFAULT_GET_OPCODE                                  0x827B
/** Light HSL Default Status Opcode */
#define MS_ACCESS_LIGHT_HSL_DEFAULT_STATUS_OPCODE                               0x827C
/** Light HSL Range Get Opcode */
#define MS_ACCESS_LIGHT_HSL_RANGE_GET_OPCODE                                    0x827D
/** Light HSL Range Status Opcode */
#define MS_ACCESS_LIGHT_HSL_RANGE_STATUS_OPCODE                                 0x827E

/** Light HSL Setup */
/** Light HSL Default Set Opcode */
#define MS_ACCESS_LIGHT_HSL_DEFAULT_SET_OPCODE                                  0x827F
/** Light HSL Default Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_HSL_DEFAULT_SET_UNACKNOWLEDGED_OPCODE                   0x8280
/** Light HSL Range Set Opcode */
#define MS_ACCESS_LIGHT_HSL_RANGE_SET_OPCODE                                    0x8281
/** Light HSL Range Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_HSL_RANGE_SET_UNACKNOWLEDGED_OPCODE                     0x8282

/** Light xyL */
/** Light xyL Get Opcode */
#define MS_ACCESS_LIGHT_XYL_GET_OPCODE                                          0x8283
/** Light xyL Set Opcode */
#define MS_ACCESS_LIGHT_XYL_SET_OPCODE                                          0x8284
/** Light xyL Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_XYL_SET_UNACKNOWLEDGED_OPCODE                           0x8285
/** Light xyL Status Opcode */
#define MS_ACCESS_LIGHT_XYL_STATUS_OPCODE                                       0x8286
/** Light xyL Target Get Opcode */
#define MS_ACCESS_LIGHT_XYL_TARGET_GET_OPCODE                                   0x8287
/** Light xyL Target Status Opcode */
#define MS_ACCESS_LIGHT_XYL_TARGET_STATUS_OPCODE                                0x8288
/** Light xyL Default Get Opcode */
#define MS_ACCESS_LIGHT_XYL_DEFAULT_GET_OPCODE                                  0x8289
/** Light xyL Default Status Opcode */
#define MS_ACCESS_LIGHT_XYL_DEFAULT_STATUS_OPCODE                               0x828A
/** Light xyL Range Get Opcode */
#define MS_ACCESS_LIGHT_XYL_RANGE_GET_OPCODE                                    0x828B
/** Light xyL Range Status Opcode */
#define MS_ACCESS_LIGHT_XYL_RANGE_STATUS_OPCODE                                 0x828C

/** Light xyL Setup */
/** Light xyL Default Set Opcode */
#define MS_ACCESS_LIGHT_XYL_DEFAULT_SET_OPCODE                                  0x828D
/** Light xyL Default Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_XYL_DEFAULT_SET_UNACKNOWLEDGED_OPCODE                   0x828E
/** Light xyL Range Set Opcode */
#define MS_ACCESS_LIGHT_XYL_RANGE_SET_OPCODE                                    0x828F
/** Light xyL Range Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_XYL_RANGE_SET_UNACKNOWLEDGED_OPCODE                     0x8290

/** Light Control */
/** Light LC Mode Get Opcode */
#define MS_ACCESS_LIGHT_LC_MODE_GET_OPCODE                                      0x8291
/** Light LC Mode Set Opcode */
#define MS_ACCESS_LIGHT_LC_MODE_SET_OPCODE                                      0x8292
/** Light LC Mode Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_LC_MODE_SET_UNACKNOWLEDGED_OPCODE                       0x8293
/** Light LC Mode Status Opcode */
#define MS_ACCESS_LIGHT_LC_MODE_STATUS_OPCODE                                   0x8294
/** Light LC Occupancy Mode Get Opcode */
#define MS_ACCESS_LIGHT_LC_OM_GET_OPCODE                                        0x8295
/** Light LC Occupancy Mode Set Opcode */
#define MS_ACCESS_LIGHT_LC_OM_SET_OPCODE                                        0x8296
/** Light LC Occupancy Mode Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_LC_OM_SET_UNACKNOWLEDGED_OPCODE                         0x8297
/** Light LC Occupancy Mode Status Opcode */
#define MS_ACCESS_LIGHT_LC_OM_STATUS_OPCODE                                     0x8298
/** Light LC Light OnOff Get Opcode */
#define MS_ACCESS_LIGHT_LC_LIGHT_ONOFF_GET_OPCODE                               0x8299
/** Light LC Light OnOff Set Opcode */
#define MS_ACCESS_LIGHT_LC_LIGHT_ONOFF_SET_OPCODE                               0x829A
/** Light LC Light OnOff Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_LC_LIGHT_ONOFF_SET_UNACKNOWLEDGED_OPCODE                0x829B
/** Light LC Light OnOff Status Opcode */
#define MS_ACCESS_LIGHT_LC_LIGHT_ONOFF_STATUS_OPCODE                            0x829C
/** Light LC Property Get Opcode */
#define MS_ACCESS_LIGHT_LC_PROPERTY_GET_OPCODE                                  0x829D
/** Light LC Property Set Opcode */
#define MS_ACCESS_LIGHT_LC_PROPERTY_SET_OPCODE                                  0x62
/** Light LC Property Set Unacknowledged Opcode */
#define MS_ACCESS_LIGHT_LC_PROPERTY_SET_UNACKNOWLEDGED_OPCODE                   0x63
/** Light LC Property Status Opcode */
#define MS_ACCESS_LIGHT_LC_PROPERTY_STATUS_OPCODE                               0x64

/** Invalid Opcode */
#define MS_ACCESS_INVALID_OPCODE                                                0xFFFFFFFF


/** ---------------------------------------------------------------------------- Model IDs */
/** SIG defined model IDs */
/** Model ID - Config Server */
#define MS_MODEL_ID_CONFIG_SERVER                                               0x0000
/** Model ID - Config Client */
#define MS_MODEL_ID_CONFIG_CLIENT                                               0x0001
/** Model ID - Health Server */
#define MS_MODEL_ID_HEALTH_SERVER                                               0x0002
/** Model ID - Health Client */
#define MS_MODEL_ID_HEALTH_CLIENT                                               0x0003

/** Generic */
/** Model ID - Generic OnOff Server */
#define MS_MODEL_ID_GENERIC_ONOFF_SERVER                      0x1000
/** Model ID - Generic OnOff Client */
#define MS_MODEL_ID_GENERIC_ONOFF_CLIENT                      0x1001
/** Model ID - Generic Level Server */
#define MS_MODEL_ID_GENERIC_LEVEL_SERVER                      0x1002
/** Model ID - Generic Level Client */
#define MS_MODEL_ID_GENERIC_LEVEL_CLIENT                      0x1003
/** Model ID - Generic Default Transition Time Server */
#define MS_MODEL_ID_GENERIC_DEFAULT_TRANSITION_TIME_SERVER    0x1004
/** Model ID - Generic Default Transition Time Client */
#define MS_MODEL_ID_GENERIC_DEFAULT_TRANSITION_TIME_CLIENT    0x1005
/** Model ID - Generic Power OnOff Server */
#define MS_MODEL_ID_GENERIC_POWER_ONOFF_SERVER                0x1006
/** Model ID - Generic Power OnOff Setup Server */
#define MS_MODEL_ID_GENERIC_POWER_ONOFF_SETUP_SERVER          0x1007
/** Model ID - Generic Power OnOff Client */
#define MS_MODEL_ID_GENERIC_POWER_ONOFF_CLIENT                0x1008
/** Model ID - Generic Power Level Server */
#define MS_MODEL_ID_GENERIC_POWER_LEVEL_SERVER                0x1009
/** Model ID - Generic Power Level Setup Server */
#define MS_MODEL_ID_GENERIC_POWER_LEVEL_SETUP_SERVER          0x100A
/** Model ID - Generic Power Level Client */
#define MS_MODEL_ID_GENERIC_POWER_LEVEL_CLIENT                0x100B
/** Model ID - Generic Battery Server */
#define MS_MODEL_ID_GENERIC_BATTERY_SERVER                    0x100C
/** Model ID - Generic Battery Client */
#define MS_MODEL_ID_GENERIC_BATTERY_CLIENT                    0x100D
/** Model ID - Generic Location Server */
#define MS_MODEL_ID_GENERIC_LOCATION_SERVER                   0x100E
/** Model ID - Generic Location Setup Server */
#define MS_MODEL_ID_GENERIC_LOCATION_SETUP_SERVER             0x100F
/** Model ID - Generic Location Client */
#define MS_MODEL_ID_GENERIC_LOCATION_CLIENT                   0x1010
/** Model ID - Generic Admin Property Server */
#define MS_MODEL_ID_GENERIC_ADMIN_PROPERTY_SERVER             0x1011
/** Model ID - Generic Manufacturer Property Server */
#define MS_MODEL_ID_GENERIC_MANUFACTURER_PROPERTY_SERVER      0x1012
/** Model ID - Generic User Property Server */
#define MS_MODEL_ID_GENERIC_USER_PROPERTY_SERVER              0x1013
/** Model ID - Generic Client Property Server */
#define MS_MODEL_ID_GENERIC_CLIENT_PROPERTY_SERVER            0x1014
/** Model ID - Generic Property Client */
#define MS_MODEL_ID_GENERIC_PROPERTY_CLIENT                   0x1015

/** Sensors */
/** Model ID - Sensor Server */
#define MS_MODEL_ID_SENSOR_SERVER                             0x1100
/** Model ID - Sensor Setup Server */
#define MS_MODEL_ID_SENSOR_SETUP_SERVER                       0x1101
/** Model ID - Sensor Client */
#define MS_MODEL_ID_SENSOR_CLIENT                             0x1102

/** Time and Scenes */
/** Model ID - Time Server */
#define MS_MODEL_ID_TIME_SERVER                               0x1200
/** Model ID - Time Setup Server */
#define MS_MODEL_ID_TIME_SETUP_SERVER                         0x1201
/** Model ID - Time Client */
#define MS_MODEL_ID_TIME_CLIENT                               0x1202
/** Model ID - Scene Server */
#define MS_MODEL_ID_SCENE_SERVER                              0x1203
/** Model ID - Scene Setup Server */
#define MS_MODEL_ID_SCENE_SETUP_SERVER                        0x1204
/** Model ID - Scene Client */
#define MS_MODEL_ID_SCENE_CLIENT                              0x1205
/** Model ID - Scheduler Server */
#define MS_MODEL_ID_SCHEDULER_SERVER                          0x1206
/** Model ID - Scheduler Setup Server */
#define MS_MODEL_ID_SCHEDULER_SETUP_SERVER                    0x1207
/** Model ID - Scheduler Client */
#define MS_MODEL_ID_SCHEDULER_CLIENT                          0x1208

/** Lighting */
/** Model ID - Light Lightness Server */
#define MS_MODEL_ID_LIGHT_LIGHTNESS_SERVER                    0x1300
/** Model ID - Light Lightness Setup Server */
#define MS_MODEL_ID_LIGHT_LIGHTNESS_SETUP_SERVER              0x1301
/** Model ID - Light Lightness Client */
#define MS_MODEL_ID_LIGHT_LIGHTNESS_CLIENT                    0x1302
/** Model ID - Light CTL Server */
#define MS_MODEL_ID_LIGHT_CTL_SERVER                          0x1303
/** Model ID - Light CTL Setup Server */
#define MS_MODEL_ID_LIGHT_CTL_SETUP_SERVER                    0x1304
/** Model ID - Light CTL Client */
#define MS_MODEL_ID_LIGHT_CTL_CLIENT                          0x1305
/** Model ID - Light CTL Temperature Server */
#define MS_MODEL_ID_LIGHT_CTL_TEMPERATURE_SERVER              0x1306
/** Model ID - Light HSL Server */
#define MS_MODEL_ID_LIGHT_HSL_SERVER                          0x1307
/** Model ID - Light HSL Setup Server */
#define MS_MODEL_ID_LIGHT_HSL_SETUP_SERVER                    0x1308
/** Model ID - Light HSL Client */
#define MS_MODEL_ID_LIGHT_HSL_CLIENT                          0x1309
/** Model ID - Light HSL HUE Server */
#define MS_MODEL_ID_LIGHT_HSL_HUE_SERVER                      0x130A
/** Model ID - Light HSL Saturation Server */
#define MS_MODEL_ID_LIGHT_HSL_SATURATION_SERVER               0x130B
/** Model ID - Light xyL Server */
#define MS_MODEL_ID_LIGHT_XYL_SERVER                          0x130C
/** Model ID - Light xyL Setup Server */
#define MS_MODEL_ID_LIGHT_XYL_SETUP_SERVER                    0x130D
/** Model ID - Light xyL Client */
#define MS_MODEL_ID_LIGHT_XYL_CLIENT                          0x130E
/** Model ID - Light LC Server */
#define MS_MODEL_ID_LIGHT_LC_SERVER                           0x130F
/** Model ID - Light LC Setup Server */
#define MS_MODEL_ID_LIGHT_LC_SETUP_SERVER                     0x1310
/** Model ID - Light LC Client */
#define MS_MODEL_ID_LIGHT_LC_CLIENT                           0x1311

#endif /* _H_MS_ASSIGNED_NUMBERS_ */

