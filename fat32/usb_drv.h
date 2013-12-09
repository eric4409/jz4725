#ifndef __USB_DRV_H__
#define __USB_DRV_H__ 
int USB_disk_initialize(void);
int USB_disk_status(void);
int USB_disk_read(buff, sector, count);
int USB_disk_write(buff, sector, count);
DWORD usb_GetSecterCount(void);
int USB_disk_ioctl(BYTE cmd, void *buff);

#endif
