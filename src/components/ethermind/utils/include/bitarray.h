
/**
    \file bitarray.h

    \brief This module defines interfaces to efficiently process
    an array of booleans represented as bits in an array of 32-bit integers.
*/

/*
    Copyright (C) 2017. Mindtree Limited.
    All rights reserved.
*/

#ifndef _H_BITARRAY_
#define _H_BITARRAY_

/* --------------------------------------------- Header File Inclusion */
#include "EM_os.h"

/* --------------------------------------------- Global Definitions */
/** Size of a single block in bitarray data structure */
#define BITARRAY_BLOCK_SIZE               32u

/** Number of blocks required to store a given number of bits */
#define BITARRAY_NUM_BLOCKS(bit_count)    \
    (((bit_count) + (BITARRAY_BLOCK_SIZE - 1)) / (BITARRAY_BLOCK_SIZE))

/* --------------------------------------------- Structures/Data Types */

/* --------------------------------------------- Macros */

/* --------------------------------------------- API Declarations */

#ifdef __cplusplus
extern "C" {
#endif

/**
    \brief Set boolean value of a specific bit position

    \par Description
    This routine sets the boolean value of a specific bit position in a given bitarray.

    \param [in] bitarray        The bitarray in which the bit position to be set.
    \param [in] bit_position    The bit position to be set.
*/
void bitarray_set_bit
(
    /* IN */ UINT32* bitarray,
    /* IN */ UINT32   bit_position
);

/**
    \brief Get boolean value of a specific bit position

    \par Description
    This routine returns the boolean value of a specific bit position in a given bitarray.

    \param [in] bitarray        The bitarray from which the bit position to be fetched.
    \param [in] bit_position    The bit position to be fetched.

    \return Boolean value 1 if bit is set, otherwise 0
*/
UINT8 bitarray_get_bit
(
    /* IN */ UINT32* bitarray,
    /* IN */ UINT32   bit_position
);

/**
    \brief Reset/clear boolean value of a specific bit position

    \par Description
    This routine resets/clear the boolean value of a specific bit position in a given bitarray.

    \param [in] bitarray        The bitarray in which the bit position to be reset.
    \param [in] bit_position    The bit position to be reset.
*/
void bitarray_reset_bit
(
    /* IN */ UINT32* bitarray,
    /* IN */ UINT32   bit_position
);

/**
    \brief Set boolean value of all bits

    \par Description
    This routine sets the boolean value of all bits in a given bitarray.

    \param [in] bitarray        The bitarray to be set.
    \param [in] bit_count       Number of bits in the bitarray.
*/
void bitarray_set_all
(
    /* IN */ UINT32* bitarray,
    /* IN */ UINT32   bit_count
);

/**
    \brief Reset/clear boolean value of all bits

    \par Description
    This routine resets/clear the boolean value of all bits in a given bitarray.

    \param [in] bitarray        The bitarray to be reset.
    \param [in] bit_count       Number of bits in the bitarray.
*/
void bitarray_reset_all
(
    /* IN */ UINT32* bitarray,
    /* IN */ UINT32   bit_count
);

/**
    \brief To check if all bits are set

    \par Description
    This routine checks if all bits in a given bitarray are set.

    \param [in] bitarray        The bitarray to be checked.
    \param [in] bit_count       Number of bits in the bitarray.

    \return Boolean value 1 if all bits are set, otherwise 0
*/
UINT8 bitarray_is_all_set
(
    /* IN */ UINT32* bitarray,
    /* IN */ UINT32   bit_count
);

/**
    \brief To check if all bits are reset/clear

    \par Description
    This routine checks if all bits in a given bitarray are reset/clear.

    \param [in] bitarray        The bitarray to be checked.
    \param [in] bit_count       Number of bits in the bitarray.

    \return Boolean value 1 if all bits are reset/clear, otherwise 0
*/
UINT8 bitarray_is_all_reset
(
    /* IN */ UINT32* bitarray,
    /* IN */ UINT32   bit_count
);

/**
    \brief Get the index of lowest bit that is set

    \par Description
    This routine returns the index of the lowest bit set in a given bitarray.

    \param [in] bitarray        The bitarray to be checked.
    \param [in] bit_count       Number of bits in the bitarray.
    \param [in] start_index     Index in the bitarray from where to start looking for.

    \return Index of the lowest bit that is set from the given start_index.
    Return start_index itself, if no further bits are set.
*/
UINT32 bitarray_get_lowest_bit_set
(
    /* IN */ UINT32* bitarray,
    /* IN */ UINT32   bit_count,
    /* IN */ UINT32   start_index
);

/**
    \brief Get the index of highest bit that is set

    \par Description
    This routine returns the index of the highest bit set in a given bitarray.

    \param [in] bitarray        The bitarray to be checked.
    \param [in] bit_count       Number of bits in the bitarray.
    \param [in] start_index     Index in the bitarray from where to start looking for.

    \return Index of the highest bit that is set from the given start_index.
    Return start_index itself, if no further bits are set.
*/
UINT32 bitarray_get_highest_bit_set
(
    /* IN */ UINT32* bitarray,
    /* IN */ UINT32   bit_count,
    /* IN */ UINT32   start_index
);

#ifdef __cplusplus
};
#endif

#endif /* _H_BITARRAY_ */

