#include "diskio.h"
#include "integer.h"
#include "CH376INC.h"
#include "FILE_SYS.h"

int USB_disk_initialize(void)
{
	return InitCH376();
}

int USB_disk_status(void)
{
	return (CH376DiskConnect( ) == USB_INT_SUCCESS)?0:-1;
}

int USB_disk_read(buff, sector, count)
{
	return (CH376DiskReadSec((PUINT8)buff, (UINT32)sector, (UINT8)count)==USB_INT_SUCCESS)?0:-1;
}

int USB_disk_write(buff, sector, count)
{
	return (CH376DiskWriteSec((PUINT8)buff, (UINT32)sector, (UINT8)count)==USB_INT_SUCCESS)?0:-1;
}

DWORD usb_GetSecterCount(void)
{
	DWORD DiskCap=0;	
	CH376DiskCapacity(&DiskCap );
	printf("-Whole sector :%d,free:%d\n",DiskCap);

	return DiskCap; 
}

int USB_disk_ioctl(BYTE cmd, void *buff)
{
//        if (mmc_status!=0)
  //              return RES_NOTRDY;

        switch (cmd)
        {
                case CTRL_SYNC:
                        return RES_OK;

                case GET_SECTOR_SIZE:
                        *(WORD *)buff = (WORD *)512;
                        return RES_OK;

                case GET_SECTOR_COUNT:
                {

                         *(DWORD *)buff = (DWORD *)usb_GetSecterCount();
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
