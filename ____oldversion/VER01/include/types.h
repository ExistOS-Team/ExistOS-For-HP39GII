/*
 * Copyright 2009 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */


#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include "assert.h"
//! \brief TBD
//! \todo [PUBS] Add definition(s)...
//! \todo Where does this really go?
typedef struct
{
    //! \brief TBD
    uint32_t val[4];
} uint128_t;

//! \brief TBD
#ifndef RETCODE
#define RETCODE int
#endif

//------------------------------------------------------------------------------
// All of the following defs are included for compatability.  Please use the
// ANSI C99 defs for all new work.
//------------------------------------------------------------------------------

//! \brief TBD
#if !defined(FALSE)
#define FALSE false
#endif

//! \brief TBD
#if !defined(TRUE)
#define TRUE  true
#endif

//! \brief TBD
#if !defined(NULL)
#define NULL 0
#endif

//! \brief TBD
typedef bool        BOOL;

//! \brief TBD
typedef uint8_t     UINT8;
//! \brief TBD
typedef uint8_t     BYTE;
//! \brief TBD
typedef uint8_t     PACKED_BYTE;

//! \brief TBD
typedef uint16_t    UINT16;
//! \brief TBD
typedef uint16_t    USHORT;
//! \brief TBD
typedef uint16_t    WCHAR;
//! \brief TBD
typedef uint16_t    UCS3;
//! \brief TBD
typedef int16_t     SHORT;

//! \brief TBD
typedef uint32_t    UINT32;
//! \brief TBD
typedef uint32_t    WORD;
//! \brief TBD
typedef uint32_t    SECTOR_BUFFER;
//! \brief TBD
typedef uint32_t *  P_SECTOR_BUFFER;

//! \brief TBD
typedef uint64_t    DWORD;
//! \brief TBD
typedef int64_t     INT64;
//! \brief TBD
typedef int64_t     UINT64;

//! \brief TBD
typedef uint128_t   UINT128;

//! \brief TBD
typedef float       FLOAT;

//! \brief TBD
#define FRACT       _fract
//! \brief TBD
#define CIRC        _circ

//! \brief Provides a default of 16 bytes (128 bits / 8 bits per byte)
#ifndef MAX_NUM_RAW_SERIAL_NUMBER_BYTES
    #define MAX_NUM_RAW_SERIAL_NUMBER_BYTES (16)
#endif

//! \brief Provides a default value that allows each nibble of raw to be
//! converted to its
//! ASCII hex character (1 extra for NULL termination)
#ifndef MAX_NUM_ASCII_SERIAL_NUMBER_CHARS
   #define MAX_NUM_ASCII_SERIAL_NUMBER_CHARS (2*MAX_NUM_RAW_SERIAL_NUMBER_BYTES)
#endif

//! \brief Serial number.
typedef struct SerialNumber
{
    //! \brief TBD
    uint8_t rawSizeInBytes;
    //! \brief TBD
    uint8_t asciiSizeInChars;
    //! \brief TBD
    uint8_t raw[MAX_NUM_RAW_SERIAL_NUMBER_BYTES];
    // One extra for NULL termination
    char ascii[MAX_NUM_ASCII_SERIAL_NUMBER_CHARS+1];
} SerialNumber_t;

//------------------------------------------------------------------------------
// Huh?
//------------------------------------------------------------------------------

//! \brief TBD
#define REENTRANT

//------------------------------------------------------------------------------
// Debug macros in types.h?
//------------------------------------------------------------------------------

//! \brief TBD
#ifdef DEBUG
#define IFDEBUG(x) x
#else
#define IFDEBUG(x)
#endif

//------------------------------------------------------------------------------
// This sets the default build of the target
//------------------------------------------------------------------------------

//! \brief TBD
#if !defined(HW_TARGET_ASIC) && !defined(HW_TARGET_SIMULATOR)
#define HW_TARGET_BRAZOS 1
#endif

//------------------------------------------------------------------------------
// Win32 compatibility?
//------------------------------------------------------------------------------

//! \brief TBD
#ifdef _WIN32
#define inline __inline
#endif

#endif // #ifndef _TYPES_H

///////////////////////////////////////////////////////////////////////////////
// End of file
///////////////////////////////////////////////////////////////////////////////
//! @}
