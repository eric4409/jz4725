/*
* Header file for using yaffs in an application via
* a direct interface.
*/


#ifndef __YAFFSCFG_H__
#define __YAFFSCFG_H__


#include "devextras.h"

#define YAFFSFS_N_HANDLES 	200

#define NAND_FLASH_START_BLOCK  0
#define NAND_FLASH_TOTAL_BLOCKS 2048
#define NAND_SIZE_PER_BLOCK	(512*32)

typedef struct
{
	const char *prefix;
	struct yaffs_DeviceStruct *dev;
} yaffsfs_DeviceConfiguration;


void yaffsfs_Lock(void);
void yaffsfs_Unlock(void);

__u32 yaffsfs_CurrentTime(void);

void yaffsfs_SetError(int err);

struct mtd_partition {
        char *name;                     /* identifier string */
        unsigned int size;                 /* partition size */
        unsigned int offset;               /* offset within the master MTD space */
//      u_int32_t mask_flags;           /* master MTD flags to mask out for this partition */
//      struct nand_oobinfo *oobsel;    /* out of band layout for this partition (NAND only)*/
//      struct mtd_info **mtdp;         /* pointer to store the MTD object */
};
#endif


