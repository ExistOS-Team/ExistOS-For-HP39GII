/*
 * A simple user interface for this project
 *
 * Copyright 2020 Creep_er
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifndef _FLASH_MAPPING_CONST_H
#define _FLASH_MAPPING_CONST_H
#ifdef __cplusplus 
extern "C" { 
#endif


#define STMP_FINGERPRINT    0x504d5453		//!< 'STMP'

//! \name 37xx DBBT fingerprint constants
//@{
#define DBBT_FINGERPRINT1   STMP_FINGERPRINT   //!< 'STMP'
#define DBBT_FINGERPRINT2   0x54424244   //!< 'DBBT' - Discovered Bad Block Table.
#define DBBT_FINGERPRINT3   0x44494252   //!< 'RBID' - ROM Boot Image Block - D
//@}

//! \name 37xx NCB fingerprint constants
//@{
#define NCB_FINGERPRINT1    STMP_FINGERPRINT    //!< 'STMP'
#define NCB_FINGERPRINT2    0x2042434E    //!< 'NCB<space>' - NAND Control Block
#define NCB_FINGERPRINT3    0x4E494252    //!< 'RBIN' - ROM Boot Image Block - N
//@}

//! \name 37xx LDLB fingerprint constants
//@{
#define LDLB_FINGERPRINT1   STMP_FINGERPRINT   //!< 'STMP'
#define LDLB_FINGERPRINT2   0x424C444C   //!< 'LDLB' - Logical Device Layout Block
#define LDLB_FINGERPRINT3   0x4C494252   //!< 'RBIL' - ROM Boot Image Block - L
//@}

//! \name 37xx BBRC (BadBlocksPerRegionCounts_t) fingerprint constants
//@{
#define BBRC_FINGERPRINT1   STMP_FINGERPRINT   //!< 'STMP'
#define BBRC_FINGERPRINT2   0x52434242   //!< 'BBRC' - Bad Block per Region Counts
#define BBRC_FINGERPRINT3   0x42494252   //!< 'RBIB' - ROM Boot Image Block - B
//@}

#define CONFIG_BLOCK_SECTOR_OFFSET  1
#define NAND_CONFIG_BLOCK_MAGIC_COOKIE  0x00010203
#define NAND_CONFIG_BLOCK_VERSION       0x0000000b
#define NAND_MAGIC_COOKIE_WORD_POS      0
#define NAND_VERSION_WORD_POS           1

/* STMP codes */
/* Metadata STMP code value for zone map pages. */
#define LBA_STRING_PAGE1            (('L'<<24)|('B'<<16)|('A'<<8)|'M')	//0x4C42414D

/* Metadata STMP code value for phys map pages. */
#define PHYS_STRING_PAGE1           (('E'<<24)|('X'<<16)|('M'<<8)|'A')	//0x45584D41

/* Map section header constants */
/* Signature shared by all map types, used to identify a valid map header. */
const uint32_t kNandMapHeaderSignature = (('x'<<24)|('m'<<16)|('a'<<8)|('p')); /* 'xmap' 0x786D6170 */

/* Unique signature used for the zone map. */
const uint32_t kNandZoneMapSignature = (('z'<<24)|('o'<<16)|('n'<<8)|('e')); /* 'zone'   0x7A6F6E65*/

/* Unique signature used for the phy map. */
const uint32_t kNandPhysMapSignature = (('p'<<24)|('h'<<16)|('y'<<8)|('s')); /* 'phys'   0x70687973*/

/* 
** Current version of the map header.
**
** The low byte is the minor version, all higher bytes form the major version.
**
** Version history:
** - Version 1.0 was the original map section format that had a very basic two-word "header"
** with no signature.
** - Version 2.0 is the first version with a real header. 
*/
const uint32_t kNandMapSectionHeaderVersion = 0x00000200;

typedef enum FMAP_RESULT{
	RES_OK	=	0,
	RES_FAIL
}FMAP_RESULT;
#ifdef __cplusplus 
};
#endif

#endif