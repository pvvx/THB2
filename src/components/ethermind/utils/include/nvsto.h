
/**
    \file nvsto.h


*/

/*
    Copyright (C) 2013. Mindtree Limited.
    All rights reserved.
*/

#ifndef _H_NVSTO_
#define _H_NVSTO_

/* --------------------------------------------- Header File Inclusion */
#include "EM_os.h"
#include "nvs.h"

/* --------------------------------------------- Global Definitions */
/** Number of partitions per bank in platform */
#define NVSTO_NUM_PARTITIONS                5

/* --------------------------------------------- Structures/Data Types */
/** NVSTO Handle type */
typedef UINT8   NVSTO_HANDLE;

/* --------------------------------------------- Macros */
/** Persistent storage access wrappers */
#define nvsto_register_ps(size, handle) \
    nvsto_register(NVS_BANK_PERSISTENT, (size), (handle))

#define nvsto_open_pswrite(handle) \
    nvsto_open(NVS_BANK_PERSISTENT, (handle), NVS_ACCESS_WRITE)

#define nvsto_open_psread(handle) \
    nvsto_open(NVS_BANK_PERSISTENT, (handle), NVS_ACCESS_READ)

#define nvsto_close_ps(handle) \
    nvsto_close(NVS_BANK_PERSISTENT, (handle))

#define nvsto_write_ps(handle, buffer, length) \
    nvsto_write(NVS_BANK_PERSISTENT, (handle), (buffer), (length))

#define nvsto_read_ps(handle, buffer, length) \
    nvsto_read(NVS_BANK_PERSISTENT, (handle), (buffer), (length))

#define nvsto_seek_ps(handle, offset) \
    nvsto_seek(NVS_BANK_PERSISTENT, (handle), (offset))

//by hq
#define nvsto_erase_ps(handle) \
    nvsto_erase(NVS_BANK_PERSISTENT, (handle))

#define nvsto_write_header_ps(handle,value) \
    nvsto_write_header(NVS_BANK_PERSISTENT, (handle),(value))

#define nvsto_read_crc16_ps(handle,buffer,length) \
    nvsto_read_crc16(NVS_BANK_PERSISTENT, (handle), (buffer),(length))





/* --------------------------------------------- Internal Functions */

/* --------------------------------------------- API Declarations */
/**
    \brief

    \Description


    \param void

    \return void
*/
void nvsto_init (UINT32 base1,UINT32 base2);

/**
    \brief

    \Description


    \param void

    \return void
*/
void nvsto_shutdown (void);

/**
    \fn nvsto_register

    \brief

    \Description


    \param storage
    \param size
    \param handle

    \return void
*/
INT8 nvsto_register
(
    /* IN */  UINT8     storage,
    /* IN */  UINT16    size,
    /* OUT */ UINT8*    handle
);

/**
    \fn nvsto_open

    \brief

    \Description


    \param storage
    \param handle
    \param access

    \return void
*/
INT16 nvsto_open
(
    /* IN */ UINT8    storage,
    /* IN */ UINT8    handle,
    /* IN */ UINT8    access
);

/**
    \fn nvsto_close

    \brief

    \Description


    \param storage
    \param handle

    \return void
*/
INT16 nvsto_close
(
    /* IN */ UINT8    storage,
    /* IN */ UINT8    handle
);

/**
    \fn nvsto_write

    \brief

    \Description


    \param storage
    \param handle
    \param buffer
    \param length

    \return Number of bytes written
*/
INT16 nvsto_write
(
    /* IN */ UINT8    storage,
    /* IN */ UINT8    handle,
    /* IN */ void*    buffer,
    /* IN */ UINT16   length
);

/**
    \fn nvsto_read

    \brief

    \Description


    \param storage
    \param handle
    \param buffer
    \param length

    \return Number of bytes read
*/
INT16 nvsto_read
(
    /* IN */ UINT8    storage,
    /* IN */ UINT8    handle,
    /* IN */ void*    buffer,
    /* IN */ UINT16   length
);

/**
    \fn nvsto_read_crc16

    \brief

    \Description


    \param storage
    \param handle
    \param buffer
    \param length

    \return Number of bytes read
*/
INT16 nvsto_read_crc16
(
    /* IN */ UINT8    storage,
    /* IN */ UINT8    handle,
    /* IN */ UINT16*    buffer,
    /* IN */ UINT16   length
);


/**
    \fn nvsto_seek

    \brief

    \Description


    \param storage
    \param handle
    \param offset

    \return void
*/
INT16 nvsto_seek
(
    /* IN */ UINT8    storage,
    /* IN */ UINT8    handle,
    /* IN */ UINT32   offset
);


//by hq
/**
    \fn nvsto_write_flash

    \brief

    \Description


    \param storage
    \param handle

    \return void
*/

INT16 nvsto_erase
(
    /* IN */ UINT8    storage,
    /* IN */ UINT8    handle
);

/**
    \fn nvsto_erase

    \brief

    \Description


    \param storage
    \param handle

    \return void
*/
INT16 nvsto_write_header
(
    /* IN */ UINT8    storage,
    /* IN */ UINT8    handle,
    /* IN */ UINT32    value
);

#endif /* _H_NVSTO_ */

