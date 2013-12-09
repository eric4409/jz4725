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

/* mtd interface for YAFFS2 */

const char *yaffs_mtdif2_c_version =
    "$Id: yaffs_mtdif2.c,v 1.17 2007/02/14 01:09:06 wookey Exp $";

#include "yportenv.h"
#include "yaffs_mtdif2.h"
#include "yaffs_packedtags2.h"

int nandmtd2_WriteChunkWithTagsToNAND(yaffs_Device * dev, int chunkInNAND,
				      const __u8 * data,
				      const yaffs_ExtendedTags * tags)
{
	int retval = 0;
	yaffs_PackedTags2 pt;

	T(YAFFS_TRACE_MTD,
	  (TSTR
	   ("nandmtd2_WriteChunkWithTagsToNAND chunk %d data %p tags %p"
	    TENDSTR), chunkInNAND, data, tags));
	
	if (tags) {
		yaffs_PackTags2(&pt, tags);
	}

	if (data && tags) 	/* use NAND ECC always when reading data */
		retval= nand_flash_program_buf(chunkInNAND,(unsigned char *)data, (unsigned char *)&pt);
	else {
		if (data)
			retval= nand_flash_program_buf(chunkInNAND,(unsigned char *)data, NULL);
		if (tags)
			retval= nand_flash_program_buf(chunkInNAND,NULL, (unsigned char *)&pt);
	}

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

int nandmtd2_ReadChunkWithTagsFromNAND(yaffs_Device * dev, int chunkInNAND,
				       __u8 * data, yaffs_ExtendedTags * tags)
{
	size_t dummy;
	int retval = 0;

	yaffs_PackedTags2 pt;

	T(YAFFS_TRACE_MTD,
	  (TSTR
	   ("nandmtd2_ReadChunkWithTagsFromNAND chunk %d data %p tags %p"
	    TENDSTR), chunkInNAND, data, tags));

	if (data && tags) 	/* use NAND ECC always when reading data */
		retval = nand_flash_read_buf(chunkInNAND, (unsigned char*)data, (unsigned char *)&pt);
	else 
	{
		if (data)
			retval = nand_flash_read_buf(chunkInNAND, (unsigned char*)data, NULL);
		if (tags)
			retval = nand_flash_read_buf(chunkInNAND, NULL, (unsigned char *)&pt);
	}

	if (tags)
		yaffs_UnpackTags2(tags, &pt);
	
//	if(tags && retval == -EBADMSG && tags->eccResult == YAFFS_ECC_RESULT_NO_ERROR)	//treckle
	if(tags && retval != 0 && tags->eccResult == YAFFS_ECC_RESULT_NO_ERROR)
	{
		tags->eccResult = YAFFS_ECC_RESULT_UNFIXED;
	}

	if(retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

int nandmtd2_MarkNANDBlockBad(struct yaffs_DeviceStruct *dev, int blockNo)
{
	int retval;
	yaffs_PackedTags2 pt;

	T(YAFFS_TRACE_MTD,
	  (TSTR("nandmtd2_MarkNANDBlockBad %d" TENDSTR), blockNo));

	retval=nand_flash_mark_badblock(blockNo);

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

int nandmtd2_QueryNANDBlock(struct yaffs_DeviceStruct *dev, int blockNo,
			    yaffs_BlockState * state, int *sequenceNumber)
{
	int retval;
	yaffs_PackedTags2 pt;
	
	T(YAFFS_TRACE_MTD,
	  (TSTR("nandmtd2_QueryNANDBlock %d" TENDSTR), blockNo));
	
	retval=nand_flash_query_block(blockNo);

	if (retval) {
		T(YAFFS_TRACE_MTD, (TSTR("block is bad" TENDSTR)));

		*state = YAFFS_BLOCK_STATE_DEAD;
		*sequenceNumber = 0;
	} else {
		yaffs_ExtendedTags t;
		nandmtd2_ReadChunkWithTagsFromNAND(dev,
						   blockNo *
						   dev->nChunksPerBlock, NULL,
						   &t);  

		if (t.chunkUsed) {
			*sequenceNumber = t.sequenceNumber;
			*state = YAFFS_BLOCK_STATE_NEEDS_SCANNING;
		} else {
			*sequenceNumber = 0;
			*state = YAFFS_BLOCK_STATE_EMPTY;
		}
	}
	T(YAFFS_TRACE_MTD,
	  (TSTR("block  seq %d state %d" TENDSTR), *sequenceNumber,
	   *state));

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

int nandmtd2_EraseBlockInNAND(yaffs_Device *dev, int blockNumber)
{
        int err;
	
        if(blockNumber < dev->startBlock || blockNumber > dev->endBlock)
        {
                return YAFFS_FAIL;
        }

        err = nand_flash_erase_block(blockNumber);
        if (err != 0)
        {
                return YAFFS_FAIL;
        }
        else
        {
                return YAFFS_OK;
        }
//      printf("erase block: %d", blockNumber);
}

int nandmtd2_InitialiseNAND(yaffs_Device *dev)
{
	//dev->useNANDECC = 1; // force on useNANDECC which gets faked.
        return YAFFS_OK;
}

