
/**
    \file model_state_handler_pl.c


*/

/*
    Copyright (C) 2013. Mindtree Limited.
    All rights reserved.
*/

/* --------------------------------------------- Header File Inclusion */
#include "model_state_handler_pl.h"
#include "led_light.h"

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */

/* --------------------------------------------- Functions */
void mesh_model_platform_init_pl(void)
{
    /* Map Platform related initializations of GPIOs/LEDs etc here */
}

void mesh_model_device_bootup_ind_pl(void)
{
    /* LED ON/OFF for BOOT UP Indication to be mapped here */
}

void mesh_model_device_provisioned_ind_pl(void)
{
    /* LED ON/OFF for Provisioning Indication to be mapped here */
}

void generic_onoff_set_pl (UINT8 state)
{
    /* LED ON/OFF for GENERIC ONOFF to be mapped here */
    if (state)
    {
        light_ctrl(LIGHT_RED, LIGHT_TOP_VALUE-1);
        light_ctrl(LIGHT_GREEN, LIGHT_TOP_VALUE-1);
        light_ctrl(LIGHT_BLUE, LIGHT_TOP_VALUE-1);
    }
    else
    {
        light_ctrl(LIGHT_RED, 0);
        light_ctrl(LIGHT_GREEN, 0);
        light_ctrl(LIGHT_BLUE, 0);
    }
}

void vendor_mode_mainlight_onoff_set_pl (UINT8 state)
{
    /* LED ON/OFF for vendor model mainlight ONOFF to be mapped here */
    if (state)
    {
        light_ctrl(LIGHT_GREEN, LIGHT_TOP_VALUE-1);
    }
    else
    {
        light_ctrl(LIGHT_GREEN, 0);
    }
}

void vendor_mode_backlight_onoff_set_pl (UINT8 state)
{
    /* LED ON/OFF for vendor model backlight ONOFF to be mapped here */
    if (state)
    {
        light_ctrl(LIGHT_RED, LIGHT_TOP_VALUE-1);
    }
    else
    {
        light_ctrl(LIGHT_RED, 0);
    }
}



void light_lightness_set_pl (uint16_t ligtnessValue)
{
    light_ctrl(LIGHT_RED, ligtnessValue>>10);
    light_ctrl(LIGHT_GREEN, ligtnessValue>>10);
    light_ctrl(LIGHT_BLUE, ligtnessValue>>10);
}

void light_ctl_set_pl (uint16_t ctlValue,uint16_t dltUV)
{
    if(ctlValue<6600)
    {
        light_ctrl(LIGHT_RED, 255);
    }
    else
    {
        light_ctrl(LIGHT_RED, 255-(ctlValue-6600)*(255-160)/(20000-6600));
    }

    if(ctlValue<6600)
    {
        light_ctrl(LIGHT_GREEN, 50+200*(6600-ctlValue)/6600);
    }
    else
    {
        light_ctrl(LIGHT_GREEN, 255-(ctlValue-6600)*(255-190)/(20000-6600));
    }

    if(ctlValue<2000)
    {
        light_ctrl(LIGHT_BLUE, 0);
    }
    else if(ctlValue<6500)
    {
        light_ctrl(LIGHT_BLUE, 255*(6600-ctlValue)/(6500-2000));
    }
    else
    {
        light_ctrl(LIGHT_BLUE, 255);
    }
}

static float Hue_2_RGB( float v1, float v2, float vH ) //Function Hue_2_RGB
{
    if ( vH < 0 ) vH += 1;

    if ( vH > 1 ) vH -= 1;

    if (( 6 * vH ) < 1 ) return ( v1 + ( v2 - v1 ) * 6 * vH );

    if (( 2 * vH ) < 1 ) return ( v2 );

    if (( 3 * vH ) < 2 ) return ( v1 + ( v2 - v1 ) * ( ( 2/3.0 ) - vH ) * 6 );

    return ( v1 );
}

void light_hsl_set_pl (uint16_t H_int,uint16_t S_int,uint16_t L_int)
{
    float H = (float)H_int / 65535.0;
    float S = (float)S_int / 65535.0;
    float L = (float)L_int / 65535.0;
    float R,G,B,var_1,var_2;

    if ( S == 0 )
    {
        R = L;
        G = L;
        B = L;
    }
    else
    {
        if ( L < 0.5 )
            var_2 = L * ( 1 + S );
        else
            var_2 = ( L + S ) - ( S * L );

        var_1 = 2 * L - var_2;
        R = Hue_2_RGB( var_1, var_2, H + ( 1/3.0 ));
        G = Hue_2_RGB( var_1, var_2, H );
        B = Hue_2_RGB( var_1, var_2, H - ( 1/3.0 ));
    }

    uint16_t R_int = (uint16_t)(R*LIGHT_TURN_ON);
    uint16_t G_int = (uint16_t)(G*LIGHT_TURN_ON);
    uint16_t B_int = (uint16_t)(B*LIGHT_TURN_ON);
//    printf("[HSL_f] %f %f %f\n",R,G,B);
//    printf("[HSL_I] %02x %02x %02x\n",R_int,G_int,B_int);
    light_ctrl(LIGHT_RED, R_int);
    light_ctrl(LIGHT_GREEN, G_int);
    light_ctrl(LIGHT_BLUE, B_int);
}

//    light_hsl_set_pl(0x5555,0xffff,0x8000); // 0x00 0xFF 0x00 Green
//    light_hsl_set_pl(0xaaaa,0xffff,0x8000); // 0x00 0x00 0xFF Blue
//    light_hsl_set_pl(0x0000,0xffff,0x8000);// 0xFF 0x00 0x00 Red
//

