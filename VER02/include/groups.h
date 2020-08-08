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

#ifndef _GROUPS_H
#define _GROUPS_H

///////////////////////////////////////////////////////////////////////////////
//! \addtogroup common
//! @{
//
// Copyright (c) 2004-2005 SigmaTel, Inc.
//
//! \file groups.h
//! \brief  Contains group codes.
//!
//! \code
//! 0x00000000, 0xFFFFFFFF, and 0x8000000 - 0xAFFFFFFF reserved for Sigmatel
//! 0xB0000000 - 0xFFFFFFFE available for customer use
//! \endcode
//! 768 Major groups for Sigmatel, 1280 Major groups for customers\n
//! 256 Minor groups per Major group\n
//! \code
//! Bit            3322 2222 2222 1111 1111 11
//!                1098 7654 3210 9876 5432 1098 7654 3210
//!                ---------------------------------------
//! Major Groups:  ---- MMMM MMMM ---- ---- ---- ---- ----
//! Minor Groups:  ---- ---- ---- mmmm mmmm ---- ---- ----
//! Errors:        ---- ---- ---- ---- ---- eeee eeee eeee
//! \endcode
//!


//the below is ugly, but its the only way to solve the int typedef type checking
// where errors show up as negative
#define HW_GROUP                  (0x00100000) //0x80100000
#define DDI_GROUP                 (0x00200000) //0x80200000
#define OS_GROUP                  (0x00300000) //0x80300000
#define MIDDLEWARE_GROUP          (0x00400000) //0x80400000
#define ROM_GROUP                 (0x00500000) //0x80500000
#define BM_GROUP                  (0x00600000) //0x80600000
#define APPS_GROUP                (0x00700000) //0x80700000
#define UTILITY_GROUP             (0x00800000) //0x80800000
#endif//_GROUPS_H

