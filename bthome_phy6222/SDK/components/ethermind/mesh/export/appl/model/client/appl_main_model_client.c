
/**
    \file appl_main_model_client.c

    This File contains the "main" function for the Test Application
    to test the Model Clients of Mindtree Mesh stack.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

/* ------------------------------- Header File Inclusion */
#include "appl_main.h"

/* ------------------------------- Global Variables */
char main_model_client_options[] = " \n\
================ M A I N   M E N U ================ \n\
   0.  Exit. \n\
   1.  Refresh this Menu. \n\
 \n\
  10.  Config Client Operations. \n\
  20.  Health Client Operations. \n\
 \n\
  30.  Generic Onoff Client Operations. \n\
  31.  Generic Level Client Operations. \n\
  32.  Generic Default Transition Time Client Operations. \n\
  33.  Generic Power OnOff Client Operations. \n\
  34.  Generic Power Level Client Operations. \n\
  35.  Generic Battery Client Operations. \n\
  36.  Generic Location Client Operations. \n\
  37.  Generic Property Client Operations. \n\
 \n\
  40.  Sensor Client Operations. \n\
 \n\
  50.  Time Client Operations. \n\
  51.  Scene Client Operations. \n\
  52.  Scheduler Client Operations. \n\
 \n\
  60.  Light Lightness Client Operations. \n\
  61.  Light CTL Client Operations. \n\
  62.  Light HSL Client Operations. \n\
  63.  Light xyL Client Operations. \n\
  64.  Light LC Client Operations. \n\
 \n\
Your Option ? \0";

/* ------------------------------- Functions */
int main_model_client_operations (int argc, char** argv)
{
    int choice;
    API_RESULT retval;

    while (1)
    {
        printf("\n");
        printf("%s", main_model_client_options);
        scanf("%d", &choice);

        switch(choice)
        {
        case 0:
            return 0;

        case 1:
            printf("\nRefreshing ...\n\n");
            break;

        case 10: /* Config Client Operations */
            main_config_client_operations();
            break;

        case 20: /* Health Client Operations */
            main_health_client_operations();
            break;

        case 30: /* Generic Onoff Client Operations */
            main_generic_onoff_client_operations();
            break;

        case 31: /* Generic Level Client Operations */
            main_generic_level_client_operations();
            break;

        case 32: /* Generic Default Transition Time Client Operations */
            main_generic_default_transition_time_client_operations();
            break;

        case 33: /* Generic Power OnOff Client Operations */
            main_generic_power_onoff_client_operations();
            break;

        case 34: /* Generic Power Level Client Operations */
            main_generic_power_level_client_operations();
            break;

        case 35: /* Generic Battery Client Operations */
            main_generic_battery_client_operations();
            break;

        case 36: /* Generic Location Client Operations */
            main_generic_location_client_operations();
            break;

        case 37: /* Generic Property Client Operations */
            main_generic_property_client_operations();
            break;

        case 40: /* Sensor Client Operations */
            main_sensor_client_operations();
            break;

        case 50: /* Time Client Operations */
            main_time_client_operations();
            break;

        case 51: /* Scene Client Operations */
            main_scene_client_operations();
            break;

        case 52: /* Scheduler Client Operations */
            main_scheduler_client_operations();
            break;

        case 60: /* Light Lightness Client Operations */
            main_light_lightness_client_operations();
            break;

        case 61: /* Light CTL Client Operations */
            main_light_ctl_client_operations();
            break;

        case 62: /* Light HSL Client Operations */
            main_light_hsl_client_operations();
            break;

        case 63: /* Light xyL Client Operations */
            main_light_xyl_client_operations();
            break;

        case 64: /* Light LC Client Operations */
            main_light_lc_client_operations();
            break;

        default:
            printf("Invalid Option : %d.\n", choice);
            break;
        }
    }

    return 0;
}

