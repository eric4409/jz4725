/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "usb_drv.h"
#include <time.h>

/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
/* Note that Tiny-FatFs supports only single drive and always            */
/* accesses drive number 0.                                              */

#define ATA		0
#define MMC		1
#define USB		2



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber (0..) */
)
{
	DSTATUS stat;
	int result;

//	printf("%s, drv:%d\n",__func__,drv);

	switch (drv) {
	case ATA :
		//result = ATA_disk_initialize();
		// translate the reslut code here

		return stat;
#if DEVTYPE == 2
	case MMC :
		result = MMC_disk_initialize();
		// translate the reslut code here
		if(!result)
			stat = RES_OK;
		else
			stat = RES_ERROR;
		return stat;
#endif
	case USB :
		result = USB_disk_initialize();
		// translate the reslut code here
		if(!result)
			stat = RES_OK;
		else
			stat = RES_ERROR;

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
	DSTATUS stat;
	int result;

//	printf("%s, drv:%d\n",__func__,drv);
	switch (drv) {
	case ATA :
		//result = ATA_disk_status();
		// translate the reslut code here

		return stat;
#if DEVTYPE == 2
	case MMC :
		result = MMC_disk_status();
		// translate the reslut code here
		if(!result)
			stat = RES_OK;
		else
			stat = RES_ERROR;

		return stat;
#endif
	case USB :
		result = USB_disk_status();
		// translate the reslut code here
		if(!result)
		{	
			stat = RES_OK;
		}
		else
		{
			stat = RES_ERROR;
		}

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{
	DRESULT res;
	int result;

//	printf("%s, drv:%d\n",__func__,drv);
	switch (drv) {
	case ATA :
		//result = ATA_disk_read(buff, sector, count);
		// translate the reslut code here

		return res;
#if DEVTYPE == 2
	case MMC :
		result = MMC_disk_read(buff, sector, count);
		// translate the reslut code here
		if(!result)
			res = RES_OK;
		else
			res = RES_ERROR;

		return res;
#endif
	case USB :
		result = USB_disk_read(buff, sector, count);
		// translate the reslut code here
		if(!result)
			res = RES_OK;
		else
			res = RES_ERROR;

		return res;
	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
	DRESULT res;
	int result;
	unsigned char tbuf[512];
	int cmp=0;
	int timeout=50;
	switch (drv) {
	case ATA :
		//result = ATA_disk_write(buff, sector, count);
		// translate the reslut code here

		return res;
#if DEVTYPE == 2
	case MMC :
		result = MMC_disk_write(buff, sector, count);
		// translate the reslut code here
	//	printf("%s s:%d, c:%d, rel:%d\n",__func__,sector, count, result);
		if(!result)
			res = RES_OK;
		else
			res = RES_ERROR;

		return res;
#endif
	case USB :
		//result = USB_disk_write(buff, sector, count);
		do{
			result = USB_disk_write(buff, sector, count);
			if (!result)
				if (!USB_disk_read(tbuf, sector, count))
					cmp=memcmp(tbuf, buff, count);
		}while(cmp&&--timeout);
		// translate the reslut code here
		if(!result)
			res = RES_OK;
		else
			res = RES_ERROR;

		return res;
	}
	return RES_PARERR;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

//	printf("%s, drv:%d\n",__func__,drv);
	switch (drv) {
	case ATA :
		// pre-process here

		//result = ATA_disk_ioctl(ctrl, buff);
		// post-process here

		return res;
#if DEVTYPE == 2
	case MMC :
		// pre-process here

		result = MMC_disk_ioctl(ctrl, buff);
		// post-process here
		if(!result)
			res = RES_OK;
		else
			res = RES_ERROR;

		return res;
#endif
	case USB :
		// pre-process here

		result = USB_disk_ioctl(ctrl, buff);
		// post-process here
		if(!result)
			res = RES_OK;
		else
			res = RES_ERROR;

		return res;
	}
	return RES_PARERR;
}

//void GetTime(TTime *t)
//void GetTime(struct tm *t)
//{
//      return;
//}

DWORD get_fattime(void)
{
        struct tm ctime;
        DWORD tt=0;
        GetTime(&ctime);
        ctime.tm_year -= 80;
        tt =(ctime.tm_year<<25) &0xfe000000;
        tt |= ((ctime.tm_mon+1) <<21) & 0x01e00000;
        tt |= (ctime.tm_mday << 16) &0x001f0000;
        tt |= (ctime.tm_hour << 11) &0x0000f800;
        tt |= (ctime.tm_min << 5) &0x000007e0;
        tt |= (ctime.tm_sec)&0x0000001f; 
        return tt; 
}

