/********************** BEGIN LICENSE BLOCK ************************************
 *
 * JZ4740  mobile_tv  Project  V1.0.0
 * INGENIC CONFIDENTIAL--NOT FOR DISTRIBUTION IN SOURCE CODE FORM
 * Copyright (c) Ingenic Semiconductor Co. Ltd 2005. All rights reserved.
 * 
 * This file, and the files included with this file, is distributed and made 
 * available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER 
 * EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS ALL SUCH WARRANTIES, 
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS 
 * FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. 
 * 
 * http://www.ingenic.cn 
 *
 ********************** END LICENSE BLOCK **************************************
 *
 *  Author:  <dsqiu@ingenic.cn>  <jgao@ingenic.cn> <xyzhang@ingenic.cn> 
 *
 *  Create:   2008-06-26, by dsqiu
 *            
 *  Maintain: 2008-06-26, by jgao
 *            
 *  Maintain: 2008-08-27, by xyzhang
 *            
 *
 *******************************************************************************
 */

#ifndef __STRUCTURE_H__
#define __STRUCTURE_H__

typedef enum  __FLASH_TYPE__
{
	NANDFLASH,
	NORFLASH
} FLASH_TYPE;
typedef struct __NAND_INFO__ *PNAND_INFO;
typedef struct __FLASH_INFO__
{
	FLASH_TYPE		flashType;
	unsigned int	dwBytesPerBlock;
	unsigned int	dwSectorsPerBlock;
	unsigned int	dwDataBytesPerSector;

	unsigned int	dwBLStartBlock;
	unsigned int	dwBLTotalBlocks;
	unsigned int	dwBLTotalSectors;
	
	unsigned int	dwFSStartBlock;
	unsigned int	dwFSTotalBlocks;
	unsigned int	dwFSTotalSectors;

	unsigned int	dwTotalPhyBlocks;
	unsigned int	dwMaxBadBlocks;

	unsigned int	dwCylinders;
	unsigned int	dwHeads;
	unsigned int	dwSectors;
	unsigned int	dwFlags;
	PNAND_INFO pnandinfo;
} flash_info_t, *pflash_info_t;

typedef struct __BLOCK_CACHE__
{
	int	in_use;
	int	virt_block; 
	int	old_phys_block;
	int	new_phys_block;

	// unsigned char 8bit because max pagesize = 4(2048 / 512) bit
	unsigned char	*dirty_page;

	//unsigned short dirty_sector[CONFIG_SSFDC_NAND_PAGE_PER_BLOCK][SECTORS_PER_PAGE];
	unsigned char	*data;
	unsigned char	*ext_data;
} block_cache_t;

typedef struct __PAGE_CACHE__
{
	unsigned long	page_num;
	unsigned char	*data;
} page_cache_t;

typedef struct __BLOCK_INFO__
{
	unsigned int	lifetime;
	unsigned int	tag;
} block_info_t;


typedef struct __attribute__ ((packed)) __NAND_PAGE_INFO__ 
{
#if 1
	unsigned char	reserved0;
	unsigned char	reserved1;
	unsigned char	reserved2;
	unsigned char	reserved3;
	unsigned char	page_status;
	unsigned char	block_status;	// sixth byte is the block status
	unsigned char	ecc_field[9];
	unsigned char	reserved4;
#else
	unsigned char tagByte0;
	unsigned char tagByte1;
	unsigned char tagByte2;
	unsigned char tagByte3;
	unsigned char pageStatus;	/* set to 0 to delete the chunk */
	unsigned char block_status;
	unsigned char tagByte4;
	unsigned char tagByte5;
	unsigned char eccfield1[3];
	unsigned char tagByte6;
	unsigned char tagByte7;
	unsigned char ecc_field[3];
#endif
//nsigned char	ecc_field[9];
} nand_page_info_t;

typedef struct __NAND_ZONE__
{
	unsigned int	dwSignature;
	unsigned int	dwStartBlock;
	unsigned int	dwTotalBlocks;
	unsigned int	dwTotalSectors;
	unsigned int	dwMaxBadBlocks;
} NAND_ZONE, *PNAND_ZONE;


typedef struct __VENUS_NAND_INFO__
{
	unsigned int	dwBytesPerBlock;
	unsigned int	dwSectorsPerBlock;
	unsigned int	dwDataBytesPerSector;
	unsigned int	dwTotalPhyBlocks;

	unsigned int	dwNumOfZone;
	NAND_ZONE		NandZoneList[1];
} VENUS_NAND_INFO, *PVENUS_NAND_INFO;

typedef struct __NAND_INFO__
{
	unsigned int	dwNandID;
    unsigned int    dwNandExtID;        //  32 - 24 bit is chipnumber
    unsigned int	dwPlaneNum;		//  Plane numbers to use	 
	// Essential Timing
	unsigned int	Tals;				// ALE Setup Time(Unit: ns)
	unsigned int	Talh;				// ALE Hold Time(Unit: ns)
	unsigned int	Trp;				// /RE Pulse Width(Unit: ns)
	unsigned int	Twp;				// /WE Pulse Width(Unit: ns)
	unsigned int	Trhw;				// /RE High to /WE Low(Unit: ns)
	unsigned int	Twhr;				// /WE High to /RE Low(Unit: ns)

	// Essential Nand Size
	unsigned int	dwPageSize;				// 512 or 2048
	unsigned int	dwBlockSize;			// 16K or 128K
	unsigned int	dwPageAddressCycle;		// 2 or 3

	unsigned int	dwMinValidBlocks;
	unsigned int	dwMaxValidBlocks;
} NAND_INFO;
typedef struct __NAND_CHIP_INFO__
{
	int id;
	unsigned int chipcs;
	unsigned int start_pages;
	unsigned int end_pages;
	
}NandChipInfo,*PNandChipInfo;
#endif	// __STRUCTURE_H__
