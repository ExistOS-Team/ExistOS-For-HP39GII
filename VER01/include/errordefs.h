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

#ifndef _ERRORDEFS_H
#define _ERRORDEFS_H

///////////////////////////////////////////////////////////////////////////////
//! \addtogroup common
//! @{
//
// Copyright (c) 2004-2005 SigmaTel, Inc.
//
//! \file error.h
//! \brief  Contains error codes.
//!
//! \code
//! 0x00000000, 0xFFFFFFFF, and 0x8000000 - 0xAFFFFFFF reserved for Sigmatel
//! 0xB0000000 - 0xFFFFFFFE available for customer use
//! \endcode
//! 768 Major groups for Sigmatel, 1280 Major groups for customers\n
//! 256 Minor groups per Major group\n
//! 4096 errors per Minor group\n
//! \code
//! Bit            3322 2222 2222 1111 1111 11
//!                1098 7654 3210 9876 5432 1098 7654 3210
//!                ---------------------------------------
//! Major Groups:  ---- MMMM MMMM ---- ---- ---- ---- ----
//! Minor Groups:  ---- ---- ---- mmmm mmmm ---- ---- ----
//! Errors:        ---- ---- ---- ---- ---- eeee eeee eeee
//! \endcode
//!

#ifndef __LANGUAGE_ASM__
#ifndef RT_STATUS_T_DEFINED
#define RT_STATUS_T_DEFINED
typedef int RtStatus_t;
#endif
#endif

#define SUCCESS                         (0x00000000)
#define ERROR_GENERIC                          (-1)

#define ERROR_MASK                      (-268435456)

#endif//_ERRORDEFS_H

