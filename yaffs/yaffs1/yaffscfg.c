/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system.
 * yaffscfg.c  The configuration for the "direct" use of yaffs.
 *
 * This file is intended to be modified to your requirements.
 * There is no need to redistribute this file.
 */

#include "yaffscfg.h"
#include "yaffsfs.h"
//#include "mem.h"
//#include "dlmalloc.h"

// yaffs_traceMask controls printed <INFO>, see more in ydirected.h.
//unsigned yaffs_traceMask = 0x11;
#if 1
unsigned yaffs_traceMask =

        YAFFS_TRACE_SCAN |
//        YAFFS_TRACE_GC |
	YAFFS_TRACE_OS |
        YAFFS_TRACE_ALWAYS  |
//	YAFFS_TRACE_WRITE |
//        YAFFS_TRACE_ERASE |
        YAFFS_TRACE_TRACING |
        YAFFS_TRACE_ALLOCATE |
        YAFFS_TRACE_ERROR |
        YAFFS_TRACE_NANDACCESS |
//        YAFFS_TRACE_DELETION |
        YAFFS_TRACE_BAD_BLOCKS;
#endif

void yaffsfs_SetError(int err)
{
	//Do whatever to set error
	//errno = err;
}

void yaffsfs_Lock(void)
{
}

void yaffsfs_Unlock(void)
{
}

__u32 yaffsfs_CurrentTime(void)
{
	return 0;
}

void yaffsfs_LocalInitialisation(void)
{
	// Define locking semaphore.
}

// Configuration for:
// /ram  2MB ramdisk
// /boot 2MB boot disk (flash)
// /flash 14MB flash disk (flash)
// NB Though /boot and /flash occupy the same physical device they
// are still disticnt "yaffs_Devices. You may think of these as "partitions"
// using non-overlapping areas in the same device.
// 

#include "yaffs_flashif.h"
#include "libc.h"

extern int nand_flash_hwr_init(void);

static yaffs_Device bootDev;
static yaffs_Device flashDev0;
static yaffs_Device flashDev1;

static yaffsfs_DeviceConfiguration yaffsfs_config[] = {

//	{ "/boot", &bootDev},
	{ "/mnt", &flashDev0},
	{ "/flash", &flashDev1},
	{(void *)0,(void *)0}
};

/*
The bytes per page is 512
The pages per block is 32
The total size of flash is 2048x32x512 bytes.
*/

static struct mtd_partition physmap_partitions[] = {
/* Put your own partition definitions here */
        {
                .name =         "boot/ucos",
                .size =         (NAND_SIZE_PER_BLOCK*129),	//129 blocks
                .offset =       NAND_FLASH_START_BLOCK,
        },{
                .name =         "mnt",
                .size =         (NAND_SIZE_PER_BLOCK*256),	//256 blocks
                .offset =       (NAND_FLASH_START_BLOCK+129),
        }, {
                .name =         "flash",
                .size =         (NAND_SIZE_PER_BLOCK*(NAND_FLASH_TOTAL_BLOCKS-129-256)),	// 1663 blocks
                .offset =       (NAND_FLASH_START_BLOCK+129+256),
        }
};
  
int yaffs_StartUp(void)
{	
	// /boot
	bootDev.nBytesPerChunk = YAFFS_BYTES_PER_CHUNK;
	bootDev.nChunksPerBlock = YAFFS_CHUNKS_PER_BLOCK;
	bootDev.nReservedBlocks = 5;
	bootDev.startBlock = physmap_partitions[0].offset;
	bootDev.endBlock = physmap_partitions[0].offset + 
				physmap_partitions[0].size/NAND_SIZE_PER_BLOCK - 1; 
	bootDev.useNANDECC = 0; // use YAFFS's ECC
	bootDev.nShortOpCaches = 10; // Use caches
	bootDev.genericDevice = (void *) 1;	// Used to identify the device in fstat.
	bootDev.writeChunkToNAND = yflash_WriteChunkToNAND;
	bootDev.readChunkFromNAND = yflash_ReadChunkFromNAND;
	bootDev.eraseBlockInNAND = yflash_EraseBlockInNAND;
	bootDev.initialiseNAND = yflash_InitialiseNAND;

	// "/mnt"
	flashDev0.nBytesPerChunk = YAFFS_BYTES_PER_CHUNK;
	flashDev0.nChunksPerBlock = YAFFS_CHUNKS_PER_BLOCK;
	flashDev0.nReservedBlocks = 5;
	flashDev0.startBlock = physmap_partitions[1].offset; 
	flashDev0.endBlock = physmap_partitions[1].offset + 
				physmap_partitions[1].size/NAND_SIZE_PER_BLOCK - 1; 
	flashDev0.useNANDECC = 0; // use YAFFS's ECC
	flashDev0.nShortOpCaches = 10; // Use caches
	flashDev0.genericDevice = (void *) 2;	// Used to identify the device in fstat.
	flashDev0.writeChunkToNAND = yflash_WriteChunkToNAND;
	flashDev0.readChunkFromNAND = yflash_ReadChunkFromNAND;
	flashDev0.eraseBlockInNAND = yflash_EraseBlockInNAND;
	flashDev0.initialiseNAND = yflash_InitialiseNAND;

        // "/flash"
        flashDev1.nBytesPerChunk = YAFFS_BYTES_PER_CHUNK;
        flashDev1.nChunksPerBlock = YAFFS_CHUNKS_PER_BLOCK;
        flashDev1.nReservedBlocks = 20;
        flashDev1.startBlock = physmap_partitions[2].offset;
        flashDev1.endBlock = physmap_partitions[2].offset + 
				physmap_partitions[2].size/NAND_SIZE_PER_BLOCK - 1;
        flashDev1.useNANDECC = 0; // use YAFFS's ECC
        flashDev1.nShortOpCaches = 10; // Use caches
        flashDev1.genericDevice = (void *) 3;   // Used to identify the device in fstat.
        flashDev1.writeChunkToNAND = yflash_WriteChunkToNAND;
        flashDev1.readChunkFromNAND = yflash_ReadChunkFromNAND;
        flashDev1.eraseBlockInNAND = yflash_EraseBlockInNAND;
        flashDev1.initialiseNAND = yflash_InitialiseNAND;

	
	yaffs_initialise(yaffsfs_config);

	nand_flash_hwr_init();

//	mem_init();

	return 0;
}

extern int nand_flash_query_block(int blockNo);

/* 
purpose: format the disk partition.
const char *disk: the partition name 
char force: 0 - dont format the bad block.
	    1 - force format the bad block.
*/
void yaffs_format_partition(const char *disk, char force)
{
	int i;
	
	if(strcmp(disk, "/mnt")==0)
	{
		printf("yaffs formatting partition(/mnt) %d ... \n",force);
		for(i=flashDev0.startBlock; i<=flashDev0.endBlock; i++)
		{
			if(!force)
			{
				if(nand_flash_query_block(i)!=0xFF)
					continue;
			}
			yflash_EraseBlockInNAND(&flashDev0, i);
		}
	}
	
	if(strcmp(disk, "/flash")==0)
	{
		printf("yaffs formatting partition(/flash) %d ... \n",force);
		for(i=flashDev1.startBlock; i<=flashDev1.endBlock; i++)
		{
			if(!force)
			{
				if(nand_flash_query_block(i)!=0xFF)
					continue;
			}
			yflash_EraseBlockInNAND(&flashDev1, i);
		}
	}

	printf("yaffs finished format partition!\n");

	return;
}

