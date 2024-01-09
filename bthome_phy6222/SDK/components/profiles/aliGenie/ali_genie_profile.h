/*******************************************************************************
    @file     ali_genie_profile.h
    @brief    Contains all functions support for ali genie profile
    @version  1.0
    @date     28. Feb. 2019
    @author   Zhongqi Yang

 SDK_LICENSE

*******************************************************************************/
#ifndef _ALI_GENIE_PROFILE_
#define _ALI_GENIE_PROFILE_




/*******************************************************************************
    INCLUDES
*/

#include <stddef.h>
#include <stdint.h>





/*******************************************************************************
    Global Var
*/
extern unsigned char ali_genie_pid[4];
extern unsigned char ali_genie_mac[6];
extern unsigned char ali_genie_macStr[12];
extern unsigned char ali_genie_sec[16];
extern unsigned char ali_genie_auth[16];


/*******************************************************************************
    MACRO
*/

int gen_aligenie_auth_val( void );


#endif
