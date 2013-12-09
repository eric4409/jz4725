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
#include "intc.h"
#include "arca.h"

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

#if 1	//H1
#define B_VSYNC 0x800000
#define B_HSYNC 0x400000
#define B_PCLK  0x8000000
#define B_DATA	0

#define IOVSYNC (GPC*32+23)
#define IOHSYNC (GPC*32+22)
#define IOPCLK  (GPC*32+27)
#else 	//L100
#define B_VSYNC (1<<20)
#define B_HSYNC (1<<19)
#define B_PCLK  0x8000000
#define B_DATA	8

#define IOVSYNC (GPC*32+20)
#define IOHSYNC (GPC*32+19)
#define IOPCLK  (GPC*32+27)
#endif

/*
 * Define the Max Image Size
 */
#define MAX_IMAGE_WIDTH  640
#define MAX_IMAGE_HEIGHT 480
#define MAX_IMAGE_BPP    16  
#define MAX_FRAME_SIZE   (MAX_IMAGE_WIDTH * MAX_IMAGE_HEIGHT * MAX_IMAGE_BPP / 8)

//#define COMPOUND_IMAGE

/* Actual image size, must less than max values */
int img_width = IMG_WIDTH, img_height = IMG_HEIGHT, img_bpp = IMG_BPP;
int frame_size;
extern volatile unsigned char keyint;

#ifdef COMPOUND_IMAGE
unsigned char tbuf[MAX_FRAME_SIZE/4];
unsigned char *dst=NULL;
unsigned char *src=NULL;
#else
volatile int FrameStart, FrameTime;
#endif

static unsigned int cim_start(unsigned char *ubuf)
{
	register unsigned char *pp=ubuf;
	int line,col,tline=0;
	int flag;
	register int tmp;

	keyint=0;

#ifndef COMPOUND_IMAGE  
	unsigned int count = 0;
	int timeout=1000000;
	char jk=0;

	__gpio_mask_irq(IOVSYNC);
	FrameTime = OSTimeGet();

	do{
		tmp=__gpio_get_port(GPC);

		if(tmp&B_VSYNC)
		{
			do{
				tmp=__gpio_get_port(GPC);
				if(!(tmp&B_VSYNC) )
					break;			//begin to capture a frame
				if(!timeout--)
				{
					printf("waiting vsync timeout\n");
					__gpio_unmask_irq(IOVSYNC);
					return 0;
				}
				wdt_set_count(0);
			}while(1);

			break;
		}
		else
		{
			wdt_set_count(0);

			/* if the time from VSYNC start is small then 100ms ,return */
			if( (FrameTime-FrameStart)<10)	
			{
				__gpio_unmask_irq(IOVSYNC);
				return 0;
			}

			if(tmp&B_PCLK)
				jk = 1;
			else if(jk==1)
			{
				count++;
				jk = 0;
			}

			if(count++>(frame_size*5))
			{
				printf("waiting vsync count over %d\n",count);
				__gpio_unmask_irq(IOVSYNC);
				return 0;
			} 
		}
	}while(1);
#endif
	line = 0;
	col = 0;
	flag = 0;

	do{
		tmp=__gpio_get_port(GPC);
		if( (tmp&B_HSYNC) && !(tmp&B_VSYNC) )
		{                // Frame output begin
#if 1
			unsigned char *pEnd=pp+img_width;
			while(pp<pEnd)
			{
				volatile int c=5;
				do{
					tmp=__gpio_get_port(GPC);
				}while((tmp&B_PCLK));
				*pp++=tmp >> B_DATA;
				while(c--);
			}
#else
			for(j=0;j<img_width;j++)
			{
				//				while((tmp&B_PCLK))
				//	                		tmp=__gpio_get_port(GPC);
				do{
					tmp=__gpio_get_port(GPC);
				}while((tmp&B_PCLK));

				*pp++=tmp >> B_DATA;

				do{
					tmp=__gpio_get_port(GPC);
				}while(!(tmp&B_PCLK));
			}
#endif
			flag=1;
		}
		else if(tmp&B_VSYNC)                // Frame output end
		{
#ifdef COMPOUND_IMAGE
			tline = line+1; 
#else
			break;	
#endif
		}
		else if(!(tmp&B_HSYNC))
		{
			if(flag)
			{
				line++;				//to read the next line
				pp = ubuf+img_width*line;
				flag=0;
				if(line==img_height || (pp-ubuf)>=frame_size) break;
				//	if(line==frame_size) break;
			}
		}
	}while(1);
#ifdef COMPOUND_IMAGE
	//	printf("cim start: tline=%d,tbufsize=%d \n",tline,sizeof(tbuf) );

	if(tline>5 && tline<(img_height/2))
	{
		memcpy(tbuf, ubuf, tline*img_width);
		for(dst=ubuf,src=ubuf+tline*img_width; 
				src<=ubuf+(img_height-1)*img_width;
				dst+=img_width, src+=img_width
		   )
			memcpy(dst, src, img_width);
		memcpy(ubuf+(img_height-tline)*img_width, tbuf, tline*img_width);
	}
	else if(tline>5)
	{
		memcpy(tbuf, ubuf+tline*img_width, (img_height-tline)*img_width);
		for(dst=ubuf+(img_height-1)*img_width,src=ubuf+(tline-1)*img_width; 
				src>=ubuf;
				dst-=img_width, src-=img_width
		   )
			memcpy(dst, src, img_width);
		memcpy(ubuf, tbuf,(img_height-tline)*img_width);
	}
	//	printf("cim start: dst:%p, src:%p\n",dst,src);
#endif
	//	printf("No. %d line  No.count %p \n",line,pp);
	/* If there is key interrup when captureing,variable keyint will be 1,The image must not good, ignore it */ 
	if(keyint)
	{
		keyint=0;
#ifndef COMPOUND_IMAGE
		__gpio_unmask_irq(IOVSYNC);
#endif
		return 0;
	}
#ifndef COMPOUND_IMAGE
	__gpio_unmask_irq(IOVSYNC);
#endif
	return pp-ubuf;
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

#ifndef COMPOUND_IMAGE
static void cim_interrupt_handler(unsigned int arg)
{
	FrameStart = OSTimeGet();
}
#endif

static void GPIOINIT(void)
{
	int i;
	for(i=32*GPC + B_DATA; i<32*GPC + B_DATA + 8;i++)
		__gpio_as_input(i);
        __gpio_as_input(IOPCLK);
        __gpio_as_input(IOHSYNC);
        __gpio_as_input(IOVSYNC);
}

int cim_init(int w, int h)
{
	GPIOINIT();
#ifndef COMPOUND_IMAGE
	request_irq(IRQ_GPIO_0+IOVSYNC, cim_interrupt_handler, 0);	
	__gpio_as_irq_rise_edge(IOVSYNC);
        __gpio_ack_irq(IOVSYNC);
        __gpio_unmask_irq(IOVSYNC);
#endif

	img_width = w;
	img_height = h;
  	frame_size = img_width * img_height;
	//printf("Camera Interface Initialized.The frame size is %d\n",frame_size);
	return 0;
}

