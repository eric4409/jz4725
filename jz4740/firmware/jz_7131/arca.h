/********************************************************************
                                           
 ZEM 200                                          
                                                    
 jz4730.h defines all the constants and others for Jz4730 CPU
                                                      
 Copyright (C) 2003-2006, ZKSoftware Inc.      		
                                                      
********************************************************************/

#ifndef	_JZ4730_H_
#define	_JZ4730_H_

#include <gpio.h>
#include "yaffsfs.h"
#include "clock.h"
#include "libc.h"

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#define	U8    	unsigned char
#define	U16   	unsigned short
#define	U32   	unsigned int
#define BOOL  	int
#define BYTE  	unsigned char
#define WORD 	unsigned short
#define DWORD	unsigned long
#define LONG	long

#define CONFIG_DEBUG

#ifdef CONFIG_DEBUG
#define DBPRINTF(format, arg...) do { printf(format, ## arg); } while (0)
#else
#define DBPRINTF(format, arg...) 
#endif

#define  USB_POWER_PIN 		(32*3+29) 	//GPD29 is free
#define  LCM_BACK_LIGHT		(32*3+29)
#define  AC97_CTL    	 	(32*3+29)	
#define  ZK_SENSOR_INT		(32+3+29) 
#define  IO_DOOR_SENSOR         (32*3+29)
/*
#define  RS485_SEND		(32*2+8)
#define  WEI_DN			(32*2+10)
#define	 WEI_DP			(32*2+9)	
#define  SHUT_KEY     		POWER_ON	//(32*2+11)
#define  SOUNDEN		(32*2+12)			
#define  LCMPOWER		(32*2+13)
#define	 HV7131POWER		(32*2+14)
*/
#define RS232_EN        1
#define RS485_EN        0

void CalibrationTimeBaseValue(void);
void DelayUS(int us);
void DelayMS(int ms);
void DelayNS(long ns);

BOOL GPIO_IO_Init(void);
void GPIO_IO_Free(void);
void GPIO_PIN_CTL_SET(int IsWiegandKeyPad, int IsNetSwitch);

void GPIO_AC97_Mute(BOOL Switch);
void GPIO_HY7131_Power(BOOL Switch);
void GPIO_LCD_USB0_Power(BOOL Switch);
void GPIO_RS485_Status(U32 SendMode);
void GPIO_SYS_POWER(int DevID);
void GPIO_WIEGAND_LED(BYTE GPIOPIN, BOOL Light);
void GPIO_KEY_SET(BYTE Value);
int GPIO_KEY_GET(void);
int InitialseMode(void);
void wdt_initialize(int second);
void wdt_set_count(int c);
void wdt_enable(char e);

void GPIOSetLevel(BYTE IOPIN, int High);
BOOL GPIOGetLevel(BYTE IOPIN);
BOOL ExCheckGPI(BYTE GPIOPIN);


//#define ZEM300 1

#if 1
#define open(x, y, z...)	yaffs_open(x, y, ## z)
#define lseek(x, y, z)		yaffs_lseek(x, y, z)
#define close(x)		yaffs_close(x)
#define unlink(x)		yaffs_unlink(x)
#define fstat(x, y)		yaffs_fstat(x, y)
#ifdef YAFFS2
//#define yaffs_sync()			yaffs_sync("/mnt")
#endif
//#define malloc(x)		mem_malloc(x)
//#define realloc(x, y)		mem_reallocm(x, y)
//#define free(x)		mem_free(x)
#endif

//typedef unsigned int mem_size_t;

extern int read(int fd, void *buf, unsigned int nbyte);
extern int write(int fd, const void *buf, unsigned int nbyte);

#endif /* _JZ4730_H_ */

