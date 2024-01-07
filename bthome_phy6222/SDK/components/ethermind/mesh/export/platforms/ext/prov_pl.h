
/**
    \file prov_pl.h


*/

/*
    Copyright (C) 2013. Mindtree Limited.
    All rights reserved.
*/

#ifndef _H_PROV_PL_
#define _H_PROV_PL_

/* --------------------------------------------- Header File Inclusion */
#include "MS_common.h"

/* --------------------------------------------- Global Definitions */
#define PROV_PUBKEY_SIZE_PL                     64
#define PROV_AUTHVAL_SIZE_PL                    16

/* --------------------------------------------- Structures/Data Types */

/* --------------------------------------------- Macros */

/* --------------------------------------------- Internal Functions */

/* --------------------------------------------- API Declarations */
void prov_set_device_oob_pubkey_pl(UCHAR* key, UINT16 size);
void prov_set_static_oob_auth_pl(UCHAR* key, UINT16 size);
void prov_read_device_oob_pubkey_pl (UCHAR* key, UINT16 size);
void prov_read_static_oob_auth_pl(UCHAR* key, UINT16 size);

#endif /* _H_PROV_PL_ */

