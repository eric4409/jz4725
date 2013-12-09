#if SD_UDC_DISK
#include <fs_api.h>
#include <mass_storage.h>
#include "mmc_api.h"
#include "jz4740.h"

#define UCFS_MMC_NAME "mmc:"
static UDC_LUN udcLun;
static int isinited = 0;
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int

unsigned int InitDev(unsigned int handle)
{
//	printf("mmc init \n");
	FS_IoCtl("mmc:",FS_CMD_UNMOUNT,0,0);

}

unsigned int ReadDevSector(unsigned int handle,unsigned char *buf,unsigned int startsect,unsigned int sectcount)
{
	if (!MMC_DetectStatus())
		MMC_ReadMultiBlock( startsect , sectcount , buf);
}

unsigned int WriteDevSector(unsigned int handle,unsigned char *buf,unsigned int startsect,unsigned int sectcount)
{
	if (!MMC_DetectStatus())
		MMC_WriteMultiBlock( startsect , sectcount , buf);
}

unsigned int CheckDevState(unsigned int handle)
{

	if ( MMC_DetectStatus() ) return 0;
	else return 1;

}

unsigned int GetDevInfo(unsigned int handle,PDEVINFO pinfo)
{
	pinfo->hiddennum = 0;
	pinfo->headnum = 2;
	pinfo->sectnum = 4;
	if (MMC_DetectStatus() == 0)
		pinfo->partsize = MMC_GetSize();
	else
		pinfo->partsize = 1;
	pinfo->sectorsize = 512;
}

unsigned int DeinitDev(unsigned handle)
{
	FS_IoCtl("mmc:",FS_CMD_MOUNT,0,0);
	return 1;
}

void InitUdcMMC()
{

	
	Init_Mass_Storage();

	udcLun.InitDev = InitDev;
	udcLun.ReadDevSector = ReadDevSector;
	udcLun.WriteDevSector = WriteDevSector;
	udcLun.CheckDevState = CheckDevState;
	udcLun.GetDevInfo = GetDevInfo;
	udcLun.DeinitDev = DeinitDev;
	udcLun.FlushDev = 0;
	//udcLun.DevName = (unsigned int)'MMC1';
	udcLun.DevName = ('M' << 24) | ('M' << 16) | ('C' << 8) | '1';

	if(RegisterDev(&udcLun))
		printf("UDC Register Fail!!!\r\n");
	
}
#endif
