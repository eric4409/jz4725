/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2007 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * yaffscfg.c  The configuration for the "direct" use of yaffs.
 */

#include "yaffscfg.h"
#include "yaffsfs.h"
#include <errno.h>
#include "yaffs_tagscompat.h"
#include "yaffs_mtdif2.h"
#include "yaffs_flashif.h"

//unsigned yaffs_traceMask = 0xFFFFFFFF;
#if 1
unsigned yaffs_traceMask = 

	YAFFS_TRACE_SCAN |  
//	YAFFS_TRACE_GC | 
	YAFFS_TRACE_GC_DETAIL | 
//	YAFFS_TRACE_WRITE  | 
	YAFFS_TRACE_ERASE | 
	YAFFS_TRACE_TRACING | 
	YAFFS_TRACE_ALLOCATE | 
	YAFFS_TRACE_CHECKPOINT |
	YAFFS_TRACE_NANDACCESS |
	YAFFS_TRACE_ERROR |
//	YAFFS_TRACE_MTD |
//	YAFFS_TRACE_DELETION |
	YAFFS_TRACE_BAD_BLOCKS;
#endif
static yaffs_Device flashdev;
#ifdef CONFIG_YAFFS_NO_YAFFS1
unsigned char yaffsVersion = 2;		/* 1 means YAFFS1, 2 means YAFFS2 */
#else
unsigned char yaffsVersion = 1;		/* 1 means YAFFS1, 2 means YAFFS2 */
#endif
unsigned char inband_tags = 0;
unsigned char no_cache = 0;

static yaffsfs_DeviceConfiguration yaffsfs_config[] = {

	{ "/mnt", &flashdev},
	{(void *)0,(void *)0}
};

void yaffsfs_SetError(int err)
{
	//Do whatever to set error
	errno = err;
//	printf("!!!!!! yaffsfs  Error %d\n",err);
}

void yaffsfs_Lock(void)
{
}

void yaffsfs_Unlock(void)
{
}

__u32 yaffsfs_CurrentTime(void)
{
	/* return the time in second */
	return OSTimeGet()/100;
//	return 0;
}

//void yaffsfs_LocalInitialisation(nand_info *nand);
void yaffsfs_LocalInitialisation(void)
{
	//should be acquire Nand Flash parameters automaticly 
}

int yaffs_StartUp(void)
{
	int nBlocks;

	// Stuff to configure YAFFS
	yaffsfs_LocalInitialisation();

#ifdef CONFIG_YAFFS_AUTO_YAFFS2

	if (yaffsVersion == 1 &&
	    mtd.writesize >= 2048) {
	    T(YAFFS_TRACE_ALWAYS,("yaffs: auto selecting yaffs2\n"));
	    yaffsVersion = 2;
	}

	/* Added NCB 26/5/2006 for completeness */
	if (yaffsVersion == 2 && 
	    !inband_tags &&
	    mtd.writesize == 512){
	    T(YAFFS_TRACE_ALWAYS,("yaffs: auto selecting yaffs1\n"));
	    yaffsVersion = 1;
	}

#endif

	/* OK, so if we got here, we have an valid device that's NAND and looks
	 * like it has the right capabilities
	 * Set the yaffs_Device up for YAFSS2 direct
	 */
	
	// Set up devices
	memset(&flashdev, 0, sizeof(yaffs_Device));
	
	//Set up the memory size parameters....
	flashdev.nShortOpCaches = no_cache? 0 : 10;
//	flashdev.inbandTags = inband_tags;
	flashdev.nChunksPerBlock = YAFFS_CHUNKS_PER_BLOCK;
//	flashdev.totalBytesPerChunk = YAFFS_BYTES_PER_CHUNK;
	flashdev.nDataBytesPerChunk = YAFFS_BYTES_PER_CHUNK;
	/* ... and the functions. */
	if (yaffsVersion == 2) {
		nBlocks = 128;
		flashdev.startBlock = 16;
		flashdev.endBlock = flashdev.startBlock + nBlocks - 1;
		flashdev.nReservedBlocks = 5;
		flashdev.writeChunkWithTagsToNAND = nandmtd2_WriteChunkWithTagsToNAND;
		flashdev.readChunkWithTagsFromNAND =nandmtd2_ReadChunkWithTagsFromNAND;
		flashdev.markNANDBlockBad = nandmtd2_MarkNANDBlockBad;
		flashdev.queryNANDBlock = nandmtd2_QueryNANDBlock;
		flashdev.isYaffs2 = 1;
		flashdev.useNANDECC = 1;	/* 0 means use yaffs ecc, 1 means dont use yaffs ecc */
	} else {
		nBlocks = 1919;
		flashdev.startBlock = 129;
		flashdev.endBlock = flashdev.startBlock + nBlocks - 1;
		flashdev.nReservedBlocks = 50;
		flashdev.writeChunkToNAND = yflash_WriteChunkToNAND;
		flashdev.readChunkFromNAND = yflash_ReadChunkFromNAND;
		flashdev.isYaffs2 = 0;
		flashdev.useNANDECC = 0; 	/* 0 means use yaffs ecc, 1 means dont use yaffs ecc */
	}
	/* ... and common functions */
	flashdev.eraseBlockInNAND = nandmtd2_EraseBlockInNAND;
	flashdev.initialiseNAND = nandmtd2_InitialiseNAND;



#ifdef CONFIG_YAFFS_DISABLE_WIDE_TNODES
	flashdev.wideTnodesDisabled = 1;
#endif
/*
	flashdev.skipCheckpointRead = 1;
	flashdev.skipCheckpointWrite = 1;
*/
	yaffs_initialise(yaffsfs_config);
        nand_flash_hwr_init();
        mem_init();
	
	return 0;
}

void yaffs_format_partition(const char *disk, char force)
{

}
