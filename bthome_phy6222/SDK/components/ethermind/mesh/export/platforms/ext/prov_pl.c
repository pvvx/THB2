
/**
    \file prov_pl.c


*/

/*
    Copyright (C) 2013. Mindtree Limited.
    All rights reserved.
*/

/* --------------------------------------------- Header File Inclusion */
#include "prov_pl.h"

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */
/** Out of Band Public Key information */
DECL_STATIC UCHAR prov_dev_oob_pubkey[PROV_PUBKEY_SIZE_PL];

/** Static Out of Band Authentication information */
DECL_STATIC UCHAR prov_static_oob_auth[PROV_AUTHVAL_SIZE_PL];

/* --------------------------------------------- Functions */

void prov_set_device_oob_pubkey_pl(UCHAR* key, UINT16 size)
{
    EM_mem_copy(prov_dev_oob_pubkey, key, size);
}

void prov_read_device_oob_pubkey_pl (UCHAR* key, UINT16 size)
{
    EM_mem_copy(key, prov_dev_oob_pubkey, size);
}

void prov_set_static_oob_auth_pl(UCHAR* key, UINT16 size)
{
    EM_mem_copy(prov_static_oob_auth, key, size);
}

void prov_read_static_oob_auth_pl(UCHAR* key, UINT16 size)
{
    EM_mem_copy(key, prov_static_oob_auth, size);
}

