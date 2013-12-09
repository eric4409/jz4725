#if DEVTYPE == 2
#include "diskio.h"
#include "integer.h"
#include "mmc_api.h"


static int mmc_status = -1;
int MMC_disk_initialize(void)
{
	mmc_status = MMC_Initialize();	//return 0 if successful
	if(!mmc_status)
		printf("MMC size:%d Mbytes\n", MMC_GetSize());
//	printf("%s st:%d\n",__func__, mmc_status);
	return mmc_status;
}

int MMC_disk_status(void)
{
	return (mmc_status | MMC_DetectStatus());		//return 0|0 if successful
}

#define SD_DEFAULT_BLOCKLEN 512

int MMC_disk_read(	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */)
{
	if (mmc_status!=0)
		return RES_NOTRDY;

/*	while (count--)
	{
//		printf("%s sector:%d\n",__func__,sector);
		if(MMC_ReadBlock((unsigned int)sector, (unsigned char *)buff)!= 0)
			return RES_ERROR;
		buff += SD_DEFAULT_BLOCKLEN;
		sector++;
	}
	return RES_OK; */

	return MMC_ReadMultiBlock( (unsigned int)sector, 
					(unsigned int)count, (unsigned char *)buff );

}

int MMC_disk_write(BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */)
{
        if (mmc_status!=0)
                return RES_NOTRDY;

/*        while (count--)
        {
//		printf("%s sector:%d\n",__func__,sector);
                if(MMC_WriteBlock((unsigned int)sector, (unsigned char *)buff)!= 0)
                        return RES_ERROR;
                buff += SD_DEFAULT_BLOCKLEN;
                sector++;
        }
        return RES_OK; */

	//	printf("%s sector:%d, count:%d\n",__func__,sector, count);
	return MMC_WriteMultiBlock( (unsigned int)sector, 
					(unsigned int)count, (unsigned char *)buff );

}

int MMC_disk_ioctl(	BYTE cmd,		/* Control cmd */
	void *buff		/* Buffer to send/receive control data */)
{
	if (mmc_status!=0)
		return RES_NOTRDY;

	switch (cmd)
	{
		case CTRL_SYNC:
			return RES_OK;

		case GET_SECTOR_SIZE:
			*(WORD *)buff = (WORD *)MMC_GetSecterSize();
			return RES_OK;

		case GET_SECTOR_COUNT:
		{
			
			 *(DWORD *)buff = (WORD *)MMC_GetSecterCount();
				return RES_OK;
		}


		case GET_BLOCK_SIZE:
			*(DWORD *)buff = 1;
			return RES_OK;

		default:
			printf("unknown command: [%d]\n", cmd);
			return RES_PARERR;
	}
}
#endif
