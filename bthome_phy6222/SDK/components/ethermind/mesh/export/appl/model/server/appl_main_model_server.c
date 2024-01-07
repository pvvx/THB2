
/**
    \file appl_main_model_server.c

    This File contains the "main" function for the Test Application
    to test the Model Servers of Mindtree Mesh stack.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

/* ------------------------------- Header File Inclusion */
#include "appl_main.h"

#if 0
/* ------------------------------- Global Variables */
char main_model_server_options[] = " \n\
================ M A I N   M E N U ================ \n\
   0.  Exit. \n\
   1.  Refresh this Menu. \n\
 \n\
  10.  Health Server Operations. \n\
 \n\
  20.  Generic Onoff Server Operations. \n\
  21.  Generic Level Server Operations. \n\
  22.  Generic Default Transition Time Server Operations. \n\
  23.  Generic Power OnOff Server Operations. \n\
  24.  Generic Power Level Server Operations. \n\
  25.  Generic Battery Server Operations. \n\
  26.  Generic Location Server Operations. \n\
  27.  Generic Property Server Operations. \n\
 \n\
  40.  Sensor Server Operations. \n\
 \n\
  50.  Time Server Operations. \n\
  51.  Scene Server Operations. \n\
  52.  Scheduler Server Operations. \n\
 \n\
  60.  Light Lightness Server Operations. \n\
  61.  Light CTL Server Operations. \n\
  62.  Light HSL Server Operations. \n\
  63.  Light xyL Server Operations. \n\
  64.  Light LC Server Operations. \n\
 \n\
Your Option ? \0";

/* ------------------------------- Functions */
int main_model_server_operations (int argc, char** argv)
{
    int choice;
    MS_LOOP_FOREVER()
    {
        printf("\n");
        printf("%s", main_model_server_options);
        scanf("%d", &choice);

        switch(choice)
        {
        case 0:
            return 0;

        case 1:
            printf("\nRefreshing ...\n\n");
            break;

        case 10: /* Health Server Operations */
            main_health_server_operations(MS_TRUE);
            break;

        case 20: /* Generic Onoff Server Operations */
            main_generic_onoff_server_operations(MS_TRUE);
            break;

        case 21: /* Generic Level Server Operations */
            main_generic_level_server_operations(MS_TRUE);
            break;

        case 22: /* Generic Default Transition Time Server Operations */
            main_generic_default_transition_time_server_operations(MS_TRUE);
            break;

        case 23: /* Generic Power OnOff Server Operations */
            main_generic_power_onoff_server_operations(MS_TRUE);
            break;

        case 24: /* Generic Power Level Server Operations */
            main_generic_power_level_server_operations(MS_TRUE);
            break;

        case 25: /* Generic Battery Server Operations */
            main_generic_battery_server_operations(MS_TRUE);
            break;

        case 26: /* Generic Location Server Operations */
            main_generic_location_server_operations(MS_TRUE);
            break;

        case 27: /* Generic Property Server Operations */
            main_generic_property_server_operations(MS_TRUE);
            break;

        case 40: /* Sensor Server Operations */
            main_sensor_server_operations(MS_TRUE);
            break;

        case 50: /* Time Server Operations */
            main_time_server_operations(MS_TRUE);
            break;

        case 51: /* Scene Server Operations */
            main_scene_server_operations(MS_TRUE);
            break;

        case 52: /* Scheduler Server Operations */
            main_scheduler_server_operations(MS_TRUE);
            break;

        case 60: /* Light Lightness Server Operations */
            main_light_lightness_server_operations(MS_TRUE);
            break;

        case 61: /* Light CTL Server Operations */
            main_light_ctl_server_operations(MS_TRUE);
            break;

        case 62: /* Light HSL Server Operations */
            main_light_hsl_server_operations(MS_TRUE);
            break;

        case 63: /* Light xyL Server Operations */
            main_light_xyl_server_operations(MS_TRUE);
            break;

        case 64: /* Light LC Server Operations */
            main_light_lc_server_operations(MS_TRUE);
            break;

        default:
            printf("Invalid Option : %d.\n", choice);
            break;
        }
    }
    return 0;
}
#endif /* 0 */
