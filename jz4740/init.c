/*
 * init.c 
 *
 * Perform the early initialization. Include CP0.status, install exception
 * handlers, fill all interrupt entries.
 *
 * Author: Seeger Chin
 * e-mail: seeger.chin@gmail.com
 *
 * Copyright (C) 2006 Ingenic Semiconductor Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
 /*
#pragma	weak	pcm_init
void pcm_init(void) {}
*/
#include <string.h>
#include <archdefs.h>
#include <mipsregs.h>
#include <jz4740_4725.h>
#include <ucos_ii.h>
#include "libc.h"
#include "uart.h"
#include "cache.h"
#include "clock.h"
#include "intc.h"

//#define debug
extern char DebugMode; 
unsigned int g_stack[2049];

extern void except_common_entry(void);

void CONSOL_SendCh(unsigned char ch)
{
#ifdef debug
	if(DebugMode)
		serial_putc(ch);
#endif
}

void CONSOL_GetChar(unsigned char *ch)
{
	int r;
	r = serial_getc();
	if (r > 0)
		*ch = (unsigned char)r;
	else
		*ch = 0;
}

typedef void (*pfunc)(void);

extern pfunc __CTOR_LIST__[];
extern pfunc __CTOR_END__[];
void LCDShowLogo(void);
void gpio_init(void);
void APP_vMain (void);

void c_main(void)
{
	pfunc *p;

        LCDShowLogo();
	write_c0_status(0x10000400);

	memcpy((void *)A_K0BASE, except_common_entry, 0x20);
	memcpy((void *)(A_K0BASE + 0x180), except_common_entry, 0x20);
	memcpy((void *)(A_K0BASE + 0x200), except_common_entry, 0x20);

	__dcache_writeback_all();
	__icache_invalidate_all();

	intc_init();
	sys_pll_init(CFG_CPU_SPEED);
	detect_clock();
	gpio_init();
	serial_init (57600, 8, 0, 0);	//Only baudrate is valid
	OSInit();
	
    /* Invoke constroctor functions when needed. */
#if 0	
	for (p=&__CTOR_END__[-1]; p >= __CTOR_LIST__; p--)
	{
		printf("create class %08x\n",p);
		
		(*p)();
    	}
	
#endif

#ifdef MOBILE_TV
	Mobile_TV_Main();
#else
	APP_vMain();
#endif	
	while(1);
}

//#define NULL (void *)0
//typedef unsigned int U32;
#if DRAM_SIZE == 16
  #define  MEM_END_ADDR   (0x81000000-4)      /* the sdram size is 16M, 0x80000000 to 0x80ffffff */
#else
  #define  MEM_END_ADDR   (0x80800000-4)      /* the sdram size is 8M, 0x80000000 to 0x807fffff */
#endif

//#define TASK_STK_SIZE    (1024*1024*2)
//#define TASK_STK_SIZE    (1024*1024)
#define TASK_STK_SIZE    (700*1024)
#define sbrk_end 	(MEM_END_ADDR-TASK_STK_SIZE)

U32 heapStart;
extern int _app_end;

void mem_init(void)
{
	heapStart=(U32)&_app_end;
}

void* sbrk(int size)
{
        U32 next;
        int tmp_size;
        if(size < 0)
        {
         tmp_size=(size-15)/16*16;
        }
        else
        {
         tmp_size=(size+15)/16*16;
        }
        next=heapStart+tmp_size;//size;//(size+3)/4*4;
     	printf("\tsbrk:size=%d tmp_size=%d\n", size, tmp_size);
//     	printf("\tsbrk(%d): next=%X\n", size, &next);
        if(tmp_size < 0)
        {
                //if(next>=(U32)&heapStart)
		if(next>=(U32)&_app_end)
                {
                        heapStart=next;
                        return (void*)next;
                }
                else
                {
                        printf("\t\tBottom of heap.\n");
                        return NULL;
                }
        }
//        printf("#####>>>>> sbrk memory. Total:%d bytes, Free:%d, Used:%d bytes,  Distributing:%d\n",(U32)&next-(U32)&_app_end, (U32)&next-next, next-(U32)&_app_end, size);
        printf("#####>>>>> sbrk memory. Total:%d bytes, Free:%d, Used:%d bytes,  Distributing:%d\n",(U32)sbrk_end-(U32)&_app_end, (U32)sbrk_end-next, next-(U32)&_app_end, size);

        if(next<(U32)sbrk_end)
//        if(next<(U32)&next)
        {
                void *ret=(void*)heapStart;
                heapStart=next;
                return ret;
        }
        else
        {
              printf("\t\tTop of heap.\n");
                return NULL;
        }
}

