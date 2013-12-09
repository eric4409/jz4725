#include "integer.h"

int MMC_disk_initialize(void);
int MMC_disk_status(void);
int MMC_disk_read(	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */);
int MMC_disk_write(	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */);
int MMC_disk_ioctl(	BYTE cmd,		/* Control cmd */
	void *buff		/* Buffer to send/receive control data */);