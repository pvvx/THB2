
/**
    \file appl_main.c

    This File contains the "main" function for the Test Application
    to test the Mindtree Mesh stack.
*/

/*
    Copyright (C) 2016. Mindtree Ltd.
    All rights reserved.
*/

/* ------------------------------- Header File Inclusion */
#include "appl_main.h"

void main_health_server_operations(/* IN */ UINT8 have_menu);

/* ------------------------------- Global Variables */

/* ------------------------------- Functions */
void main_register_models(void)
{
    MS_ACCESS_NODE_ID node_id;
    MS_ACCESS_ELEMENT_DESC   element;
    MS_ACCESS_ELEMENT_HANDLE element_handle;
    MS_ACCESS_MODEL_HANDLE   config_server_model_handle;
    MS_ACCESS_MODEL_HANDLE   health_server_model_handle;
    API_RESULT retval;
    /* Create Node */
    retval = MS_access_create_node(&node_id);
    /* Register Element */
    /**
        TBD: Define GATT Namespace Descriptions from
        https://www.bluetooth.com/specifications/assigned-numbers/gatt-namespace-descriptors

        Using 'main' (0x0106) as Location temporarily.
    */
    element.loc = 0x0106;
    retval = MS_access_register_element
             (
                 node_id,
                 &element,
                 &element_handle
             );

    if (API_SUCCESS == retval)
    {
        retval = MS_config_server_init(element_handle, &config_server_model_handle);
    }

    CONSOLE_OUT("Model Registration Status: 0x%04X\n", retval);
    #ifndef HSL_DONT_USE_MULTI_ELEMENTS
    /* Two additional elements are registerd for HSL Server */
    retval = MS_access_register_element
             (
                 node_id,
                 &element,
                 &sec_element_handle
             );

    if (API_SUCCESS == retval)
    {
        CONSOLE_OUT("Secondary Element Handle Registered @: 0x%04X\n", sec_element_handle);
    }

    retval = MS_access_register_element
             (
                 node_id,
                 &element,
                 &ter_element_handle
             );

    if (API_SUCCESS == retval)
    {
        CONSOLE_OUT("Tertiary Element Handle Registered @: 0x%04X\n", ter_element_handle);
    }

    #endif /* HSL_DONT_USE_MULTI_ELEMENTS */
    /* Health Server */
    main_health_server_operations(MS_FALSE);
}
