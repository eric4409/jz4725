/*
 * ucosii/jz4730/drv/cim.c
 *
 * Camera Interface Module (CIM) driver for JzSOC
 * This driver is independent of the camera sensor
 *
 * Copyright (C) 2005  JunZheng semiconductor
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "includes.h" //ucosii
#include "jz4740.h"
#include "camera.h" // Select one of the sensors

#ifndef u8
#define u8	unsigned char
#endif

#ifndef u16
#define u16	unsigned short
#endif

#ifndef u32
#define u32	unsigned int
#endif

#ifndef NULL
#define NULL	0
#endif

#define CIM_NAME        "cim"

#define GPB	1
#define GPC	2

#if 0	//old board
#define B_VSYNC 0x40000
#define B_HSYNC 0x20000
#define B_PCLK  0x400000

#define IOVSYNC (GPB*32+18)
#define IOHSYNC (GPB*32+17)
#define IOPCLK  (GPC*32+22)
#else	//new board
#define B_VSYNC 0x800000
#define B_HSYNC 0x400000
#define B_PCLK  0x8000000

#define IOVSYNC (GPC*32+23)
#define IOHSYNC (GPC*32+22)
#define IOPCLK  (GPC*32+27)
#endif

/*
 * Define the Max Image Size
 */
#define MAX_IMAGE_WIDTH  640
#define MAX_IMAGE_HEIGHT 480
#define MAX_IMAGE_BPP    16  
#define MAX_FRAME_SIZE   (MAX_IMAGE_WIDTH * MAX_IMAGE_HEIGHT * MAX_IMAGE_BPP / 8)

/* Actual image size, must less than max values */
int img_width = IMG_WIDTH, img_height = IMG_HEIGHT, img_bpp = IMG_BPP;
int frame_size;

extern unsigned char keyint;

static unsigned int cim_start(unsigned char *ubuf)
{
	unsigned int tmp,count,pp=0;
	int line,col;
	int flag;
	int j;
	int timeout=1000000;

	count=0;
	keyint=0;
        do{
                tmp=__gpio_get_port(GPC);
                if(tmp&B_VSYNC)
		{
                	do	
			{
				tmp=__gpio_get_port(GPC);
				if(!(tmp&B_VSYNC) )
                        		break;			//begin to capture a frame
				if(!timeout--)
				{
					printf("timeour when waiting VSYNC valid!\n");
					return 0;
				}
			}
			while(1);
		
			break;
		}
                else
                        if(count++>frame_size*150)
                        {
                                printf("waiting vsync timeout\n");
                                return 0;
                        }
        }while(1);

	line = 0;
	col = 0;
	flag = 0;

        do{
                tmp=__gpio_get_port(GPC);
                if(tmp&B_HSYNC)
		{                // Frame output begin
			for(j=0;j<img_width;j++)
			{
//				while((tmp&B_PCLK))
//	                		tmp=__gpio_get_port(GPC);
				do{
	                		tmp=__gpio_get_port(GPC);
				}while((tmp&B_PCLK));

                        	ubuf[pp++]=tmp&0xff;

				do{
	                		tmp=__gpio_get_port(GPC);
				}while(!(tmp&B_PCLK));
			}
			flag=1;
                }
                else if(tmp&B_VSYNC)                // Frame output end
                {
			break;
                }
		else if(!(tmp&B_HSYNC))
		{
			if(flag)
			{
				line++;				//to read the next line
				pp = img_width*line;
				flag=0;
				if(line==frame_size) break;
			} 
		}
        }while(1);
//	printf("No. %d line  No.count %d \n",line,pp);
/* If there is key interrup when captureing,variable keyint will be 1,The image must not good, ignore it */ 
	if(keyint)
	{
		keyint=0;
		return 0;
	}
	return pp;
}

/*==========================================================================
 * File operations
 *========================================================================*/

unsigned int cim_read(unsigned char *buf, int size)
{
	unsigned int rel;
//	OSSchedLock();
//	JZ_StopTicker();
	rel=cim_start(buf);
//	JZ_StartTicker(OS_TICKS_PER_SEC);
//	OSSchedUnlock();
	return rel;
}

void GPIOINIT(void)
{
	int i;
	for(i=32*GPC; i<32*GPC+8;i++)
		__gpio_as_input(i);
        __gpio_as_input(IOPCLK);
        __gpio_as_input(IOHSYNC);
        __gpio_as_input(IOVSYNC);
}

int cim_init(int w, int h)
{
	GPIOINIT();
	
	img_width = w;
	img_height = h;
  	frame_size = img_width * img_height;

	printf("Camera Interface Initialized.The frame size is %d\n",frame_size);
	return 0;
}

