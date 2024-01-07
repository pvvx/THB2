/**
    \file appl_model_state_handler.c
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/



/* --------------------------------------------- Header File Inclusion */
#include "appl_model_state_handler.h"
#include "MS_common.h"
#include "MS_generic_property_api.h"

/* --------------------------------------------- Global Definitions */


/* --------------------------------------------- Static Global Variables */


/* --------------------------------------------- Data Types/Structures */
#define MS_MAX_NUM_STATES    2

#define MS_MAX_USER_PROPERTIES            1 /* 3 */
#define MS_MAX_ADMIN_PROPERTIES           1 /* 3 */
#define MS_MAX_MANUFACTURER_PROPERTIES    1 /* 3 */
#define MS_MAX_CLIENT_PROPERTIES          1 /* 3 */

static MS_STATE_GENERIC_USER_PROPERTY_STRUCT appl_generic_user_property[MS_MAX_NUM_STATES][MS_MAX_USER_PROPERTIES];
static MS_STATE_GENERIC_ADMIN_PROPERTY_STRUCT appl_generic_admin_property[MS_MAX_NUM_STATES][MS_MAX_ADMIN_PROPERTIES];
static MS_STATE_GENERIC_MANUFACTURER_PROPERTY_STRUCT appl_generic_manufacturer_property[MS_MAX_NUM_STATES][MS_MAX_MANUFACTURER_PROPERTIES];
static MS_STATE_GENERIC_USER_PROPERTY_STRUCT appl_generic_client_property[MS_MAX_NUM_STATES][MS_MAX_CLIENT_PROPERTIES];


/* --------------------------------------------- Function */
void appl_model_states_initialization(void)
{
    UINT32 index;
    UINT8  access_type;
    EM_mem_set (appl_generic_user_property, 0, sizeof(appl_generic_user_property));
    EM_mem_set (appl_generic_admin_property, 0, sizeof(appl_generic_admin_property));
    EM_mem_set (appl_generic_manufacturer_property, 0, sizeof(appl_generic_manufacturer_property));
    EM_mem_set(appl_generic_client_property, 0, sizeof(appl_generic_client_property));
    /* For the PTS testing configure Properies */
    /* Will use all three kinds of user access one after another - for the example configuration */
    /* User Properties */
    access_type = MS_GENERIC_USER_ACCESS_READ;

    for (index = 0; index < MS_MAX_USER_PROPERTIES; index++)
    {
        appl_generic_user_property[0][index].property_id = index + 1;
        appl_generic_user_property[0][index].user_access = access_type;
        /* Allocate and keep some property value */
        appl_generic_user_property[0][index].property_value_len = index + 1;
        appl_generic_user_property[0][index].property_value = EM_alloc_mem(index + 1);
        /* TODO: Not checking for memeory allocation failure */
        EM_mem_set(appl_generic_user_property[0][index].property_value, (index + 1), (index + 1));

        if (MS_GENERIC_USER_ACCESS_READ_WRITE == access_type)
        {
            access_type = MS_GENERIC_USER_ACCESS_READ;
        }
        else
        {
            access_type++;
        }
    }

    /* Admin Properties */
    access_type = MS_GENERIC_USER_ACCESS_READ;

    for (index = 0; index < MS_MAX_USER_PROPERTIES; index++)
    {
        appl_generic_admin_property[0][index].property_id = index + 1;
        appl_generic_admin_property[0][index].user_access = access_type;
        /* Allocate and keep some property value */
        appl_generic_admin_property[0][index].property_value_len = index + 1;
        appl_generic_admin_property[0][index].property_value = EM_alloc_mem(index + 1);
        /* TODO: Not checking for memeory allocation failure */
        EM_mem_set(appl_generic_admin_property[0][index].property_value, (index + 1), (index + 1));

        if (MS_GENERIC_USER_ACCESS_READ_WRITE == access_type)
        {
            access_type = MS_GENERIC_USER_ACCESS_READ;
        }
        else
        {
            access_type++;
        }
    }

    /* Manufacturer Properties */
    access_type = MS_GENERIC_USER_ACCESS_READ;

    for (index = 0; index < MS_MAX_USER_PROPERTIES; index++)
    {
        appl_generic_manufacturer_property[0][index].property_id = index + 1;
        appl_generic_manufacturer_property[0][index].user_access = access_type;
        /* Allocate and keep some property value */
        appl_generic_manufacturer_property[0][index].property_value_len = index + 1;
        appl_generic_manufacturer_property[0][index].property_value = EM_alloc_mem(index + 1);
        /* TODO: Not checking for memeory allocation failure */
        EM_mem_set(appl_generic_manufacturer_property[0][index].property_value, (index + 1), (index + 1));

        if (MS_GENERIC_USER_ACCESS_READ_WRITE == access_type)
        {
            access_type = MS_GENERIC_USER_ACCESS_READ;
        }
        else
        {
            access_type++;
        }
    }

    /* Client Properties */
    /* Manufacturer Properties */
    access_type = MS_GENERIC_USER_ACCESS_READ;

    for (index = 0; index < MS_MAX_CLIENT_PROPERTIES; index++)
    {
        appl_generic_client_property[0][index].property_id = index + 1;
        appl_generic_client_property[0][index].user_access = access_type;
        /* Allocate and keep some property value */
        appl_generic_client_property[0][index].property_value_len = index + 1;
        appl_generic_client_property[0][index].property_value = EM_alloc_mem(index + 1);
        /* TODO: Not checking for memeory allocation failure */
        EM_mem_set(appl_generic_client_property[0][index].property_value, (index + 1), (index + 1));

        if (MS_GENERIC_USER_ACCESS_READ_WRITE == access_type)
        {
            access_type = MS_GENERIC_USER_ACCESS_READ;
        }
        else
        {
            access_type++;
        }
    }
}

void appl_model_state_get(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    UINT32 index;

    switch(state_t)
    {
    case MS_STATE_GENERIC_USER_PROPERTY_IDS_T:
    {
        MS_STATE_GENERIC_PROPERTY_IDS_STRUCT* param_p;
        UINT8 count;
        param_p = (MS_STATE_GENERIC_PROPERTY_IDS_STRUCT*)param;
        count = 0;

        for (index = 0; index < MS_MAX_USER_PROPERTIES; index++)
        {
            /* Check if Admin Property is not disabled */
            if (MS_GENERIC_USER_ACCESS_PROHIBITED != appl_generic_user_property[0][index].user_access)
            {
                param_p->property_ids[index] = appl_generic_user_property[0][index].property_id;
                count++;
            }
        }

        /* TODO: Not checking 'property_ids_count' in input */
        param_p->property_ids_count = count;
    }
    break;

    case MS_STATE_GENERIC_ADMIN_PROPERTY_IDS_T:
    {
        MS_STATE_GENERIC_PROPERTY_IDS_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_PROPERTY_IDS_STRUCT*)param;

        for (index = 0; index < MS_MAX_ADMIN_PROPERTIES; index++)
        {
            param_p->property_ids[index] = appl_generic_admin_property[0][index].property_id;
        }

        /* TODO: Not checking 'property_ids_count' in input */
        param_p->property_ids_count = MS_MAX_ADMIN_PROPERTIES;
    }
    break;

    case MS_STATE_GENERIC_MANUFACTURER_PROPERTY_IDS_T:
    {
        MS_STATE_GENERIC_PROPERTY_IDS_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_PROPERTY_IDS_STRUCT*)param;

        for (index = 0; index < MS_MAX_MANUFACTURER_PROPERTIES; index++)
        {
            param_p->property_ids[index] = appl_generic_manufacturer_property[0][index].property_id;
        }

        /* TODO: Not checking 'property_ids_count' in input */
        param_p->property_ids_count = MS_MAX_MANUFACTURER_PROPERTIES;
    }
    break;

    case MS_STATE_GENERIC_CLIENT_PROPERTY_IDS_T:
    {
        MS_STATE_GENERIC_PROPERTY_IDS_STRUCT* param_p;
        UINT8 count;
        param_p = (MS_STATE_GENERIC_PROPERTY_IDS_STRUCT*)param;
        count = 0;

        for (index = 0; index < MS_MAX_CLIENT_PROPERTIES; index++)
        {
            /* Check if Admin Property is not disabled */
            /* if (MS_GENERIC_USER_ACCESS_PROHIBITED != appl_generic_client_property[0][index].user_access) */
            {
                param_p->property_ids[index] = appl_generic_client_property[0][index].property_id;
                count++;
            }
        }

        /* TODO: Not checking 'property_ids_count' in input */
        param_p->property_ids_count = count;
    }
    break;

    case MS_STATE_GENERIC_USER_PROPERTY_T:
    {
        MS_STATE_GENERIC_USER_PROPERTY_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_PROPERTY_IDS_STRUCT*)param;
        /* Check for property ID match */
        /* Mark the access as prohibited, to indicate invalid Property ID */
        param_p->user_access = MS_GENERIC_USER_ACCESS_PROHIBITED;

        for (index = 0; index < MS_MAX_USER_PROPERTIES; index++)
        {
            if (appl_generic_user_property[0][index].property_id == param_p->property_id)
            {
                param_p->user_access = appl_generic_user_property[0][index].user_access;

                if (MS_GENERIC_USER_ACCESS_WRITE == param_p->user_access)
                {
                    param_p->property_value_len = 0;
                }
                else
                {
                    param_p->property_value = appl_generic_user_property[0][index].property_value;
                    param_p->property_value_len = appl_generic_user_property[0][index].property_value_len;
                }

                printf("GGGGEEETTTTTT::::USER::::: [%d] ID:0x%04X. Access:0x%02X. Len:%d\n",
                       index, appl_generic_admin_property[0][index].property_id,
                       appl_generic_admin_property[0][index].user_access,
                       appl_generic_admin_property[0][index].property_value_len);
                break;
            }
        }
    }
    break;

    case MS_STATE_GENERIC_ADMIN_PROPERTY_T:
    {
        MS_STATE_GENERIC_USER_PROPERTY_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_PROPERTY_IDS_STRUCT*)param;
        /* Check for property ID match */
        /* Mark the access as prohibited, to indicate invalid Property ID */
        param_p->user_access = MS_GENERIC_USER_ACCESS_PROHIBITED;
        param_p->property_value_len = 0;

        for (index = 0; index < MS_MAX_ADMIN_PROPERTIES; index++)
        {
            if (appl_generic_admin_property[0][index].property_id == param_p->property_id)
            {
                param_p->user_access = appl_generic_admin_property[0][index].user_access;

                if (MS_GENERIC_USER_ACCESS_WRITE == param_p->user_access)
                {
                    param_p->property_value_len = 0;
                }
                else
                {
                    param_p->property_value = appl_generic_admin_property[0][index].property_value;
                    param_p->property_value_len = appl_generic_admin_property[0][index].property_value_len;
                }

                printf("GGGGEEETTTTTT::::ADMIN::::: [%d] ID:0x%04X. Access:0x%02X. Len:%d\n",
                       index, appl_generic_admin_property[0][index].property_id,
                       appl_generic_admin_property[0][index].user_access,
                       appl_generic_admin_property[0][index].property_value_len);
                break;
            }
        }
    }
    break;

    case MS_STATE_GENERIC_MANUFACTURER_PROPERTY_T:
    {
        MS_STATE_GENERIC_USER_PROPERTY_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_PROPERTY_IDS_STRUCT*)param;

        /* Check for property ID match */

        for (index = 0; index < MS_MAX_MANUFACTURER_PROPERTIES; index++)
        {
            if (appl_generic_manufacturer_property[0][index].property_id == param_p->property_id)
            {
                param_p->user_access = appl_generic_manufacturer_property[0][index].user_access;

                if (MS_GENERIC_USER_ACCESS_WRITE == param_p->user_access)
                {
                    param_p->property_value_len = 0;
                }
                else
                {
                    param_p->property_value = appl_generic_manufacturer_property[0][index].property_value;
                    param_p->property_value_len = appl_generic_manufacturer_property[0][index].property_value_len;
                }

                printf("GGGGEEETTTTTT::::MANU::::: [%d] ID:0x%04X. Access:0x%02X. Len:%d\n",
                       index, appl_generic_manufacturer_property[0][index].property_id,
                       appl_generic_manufacturer_property[0][index].user_access,
                       appl_generic_manufacturer_property[0][index].property_value_len);
                break;
            }
        }

        /* Mark the access as prohibited, to indicate invalid Property ID */
        if (MS_MAX_MANUFACTURER_PROPERTIES == index)
        {
            param_p->user_access = MS_GENERIC_USER_ACCESS_INVALID_PROPERTY_ID;
            param_p->property_value_len = 0;
        }
    }
    break;
    }
}


void appl_model_state_set(UINT16 state_t, UINT16 state_inst, void* param, UINT8 direction)
{
    UINT32 index;

    switch(state_t)
    {
    case MS_STATE_GENERIC_USER_PROPERTY_T:
    {
        MS_STATE_GENERIC_USER_PROPERTY_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_PROPERTY_IDS_STRUCT*)param;
        /* Check for property ID match */
        /* Mark the access as prohibited, to indicate invalid Property ID */
        param_p->user_access = MS_GENERIC_USER_ACCESS_PROHIBITED;

        for (index = 0; index < MS_MAX_USER_PROPERTIES; index++)
        {
            if (appl_generic_user_property[0][index].property_id == param_p->property_id)
            {
                /* Check properties - write is permitted */
                if (MS_GENERIC_USER_ACCESS_READ != appl_generic_user_property[0][index].user_access)
                {
                    /* Update Value */
                    /* Free Memory */
                    if (NULL != appl_generic_user_property[0][index].property_value)
                    {
                        EM_free_mem(appl_generic_user_property[0][index].property_value);
                    }

                    /* Allocate Memory */
                    appl_generic_user_property[0][index].property_value_len = param_p->property_value_len;
                    appl_generic_user_property[0][index].property_value = EM_alloc_mem(param_p->property_value_len);
                    /* TODO: Not checking memory allocation failure */
                    /* Copy */
                    EM_mem_copy(appl_generic_user_property[0][index].property_value, param_p->property_value, param_p->property_value_len);
                }

                param_p->user_access = appl_generic_user_property[0][index].user_access;
                param_p->property_value = appl_generic_user_property[0][index].property_value;
                param_p->property_value_len = appl_generic_user_property[0][index].property_value_len;
                printf("SSSEEETTTTTT::::USER::::: [%d] ID:0x%04X. Access:0x%02X. Len:%d\n",
                       index, appl_generic_admin_property[0][index].property_id,
                       appl_generic_admin_property[0][index].user_access,
                       appl_generic_admin_property[0][index].property_value_len);
                break;
            }
        }
    }
    break;

    case MS_STATE_GENERIC_ADMIN_PROPERTY_T:
    {
        MS_STATE_GENERIC_USER_PROPERTY_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_PROPERTY_IDS_STRUCT*)param;

        /* Check for property ID match */
        /* Mark the access as prohibited, to indicate invalid Property ID */
        /* param_p->user_access = MS_GENERIC_USER_ACCESS_PROHIBITED; */

        for (index = 0; index < MS_MAX_ADMIN_PROPERTIES; index++)
        {
            if (appl_generic_admin_property[0][index].property_id == param_p->property_id)
            {
                appl_generic_admin_property[0][index].user_access = param_p->user_access;

                /* Check properties - write is permitted */
                if (MS_GENERIC_USER_ACCESS_READ != appl_generic_admin_property[0][index].user_access)
                {
                    /* Update Value */
                    /* Free Memory */
                    if (NULL != appl_generic_admin_property[0][index].property_value)
                    {
                        EM_free_mem(appl_generic_admin_property[0][index].property_value);
                    }

                    /* Allocate Memory */
                    appl_generic_admin_property[0][index].property_value_len = param_p->property_value_len;
                    appl_generic_admin_property[0][index].property_value = EM_alloc_mem(param_p->property_value_len);
                    /* TODO: Not checking memory allocation failure */
                    /* Copy */
                    EM_mem_copy(appl_generic_admin_property[0][index].property_value, param_p->property_value, param_p->property_value_len);
                }

                param_p->user_access = appl_generic_admin_property[0][index].user_access;
                param_p->property_value = appl_generic_admin_property[0][index].property_value;
                param_p->property_value_len = appl_generic_admin_property[0][index].property_value_len;
                printf("SSSSSSSEEETTTTTT::::ADMIN::::: [%d] ID:0x%04X. Access:0x%02X. Len:%d\n",
                       index, appl_generic_admin_property[0][index].property_id,
                       appl_generic_admin_property[0][index].user_access,
                       appl_generic_admin_property[0][index].property_value_len);
                break;
            }
        }
    }
    break;

    case MS_STATE_GENERIC_MANUFACTURER_PROPERTY_T:
    {
        MS_STATE_GENERIC_USER_PROPERTY_STRUCT* param_p;
        param_p = (MS_STATE_GENERIC_PROPERTY_IDS_STRUCT*)param;

        /* Check for property ID match */
        /* Mark the access as prohibited, to indicate invalid Property ID */
        /* param_p->user_access = MS_GENERIC_USER_ACCESS_PROHIBITED; */
        for (index = 0; index < MS_MAX_MANUFACTURER_PROPERTIES; index++)
        {
            if (appl_generic_manufacturer_property[0][index].property_id == param_p->property_id)
            {
                appl_generic_manufacturer_property[0][index].user_access = param_p->user_access;

                /* Check properties - write is permitted */
                if (MS_GENERIC_USER_ACCESS_READ != appl_generic_manufacturer_property[0][index].user_access)
                {
                    /* Update Value */
                    /* Free Memory */
                    if ((0 != param_p->property_value_len) && (NULL != appl_generic_manufacturer_property[0][index].property_value))
                    {
                        EM_free_mem(appl_generic_manufacturer_property[0][index].property_value);
                        /* Allocate Memory */
                        appl_generic_manufacturer_property[0][index].property_value_len = param_p->property_value_len;
                        appl_generic_manufacturer_property[0][index].property_value = EM_alloc_mem(param_p->property_value_len);
                        /* TODO: Not checking memory allocation failure */
                        /* Copy */
                        EM_mem_copy(appl_generic_manufacturer_property[0][index].property_value, param_p->property_value, param_p->property_value_len);
                    }
                }

                {
                    appl_generic_user_property[0][index].user_access = param_p->user_access;
                }

                param_p->user_access = appl_generic_manufacturer_property[0][index].user_access;
                param_p->property_value = appl_generic_manufacturer_property[0][index].property_value;
                param_p->property_value_len = appl_generic_manufacturer_property[0][index].property_value_len;
                printf("SSSSSSSEEETTTTTT::::MANUFACTURER::::: [%d] ID:0x%04X. Access:0x%02X. Len:%d\n",
                       index, appl_generic_manufacturer_property[0][index].property_id,
                       appl_generic_manufacturer_property[0][index].user_access,
                       appl_generic_manufacturer_property[0][index].property_value_len);
                break;
            }
        }

        /* Mark the access as prohibited, to indicate invalid Property ID */
        if (MS_MAX_MANUFACTURER_PROPERTIES == index)
        {
            param_p->user_access = MS_GENERIC_USER_ACCESS_INVALID_PROPERTY_ID;
            param_p->property_value_len = 0;
        }
    }
    break;
    }
}


