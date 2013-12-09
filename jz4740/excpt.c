/*
 * excpt.c
 *
 * Handle exceptions, dump all generic registers for debug.
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

#include <mipsregs.h>

const static char *regstr[] = {
	"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
	"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
	"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};
unsigned int Process_RA = 0;
unsigned int Process_SP = 0;

typedef void (*PFun_Exception_Modal_Handler)(void);
typedef struct
{
	unsigned int sp;
	unsigned int ra;
 	PFun_Exception_Modal_Handler except_handle;
}Except_Struct,*PExcept_Struct;
static PFun_Exception_Modal_Handler main_except_handle = 0;
#if 0
#define malloc(x) alloc(x)
#define free(x) deAlloc(x)
#else	
//#define malloc(x) mem_malloc(x)			//treckle
//#define free(x) mem_free(x)
#endif

static PExcept_Struct g_pExcept = 0;
static unsigned int g_MaxExceptNum = 0;
static unsigned int g_CurExcept = 0;
void setmainexcpt(void *p)
{
    main_except_handle = p;
}
int InitExcept(unsigned int x)
{
	g_pExcept = (PExcept_Struct)malloc(x * sizeof(Except_Struct));
	if(g_pExcept == 0)
		return 0;
	memset(g_pExcept,0,x * sizeof(Except_Struct));	
	g_MaxExceptNum = x;
  g_CurExcept = 0;
	return 1;	
}
void DeinitExcept()
{
	if(g_pExcept)
		free(g_pExcept);
}
int AddExcept(PFun_Exception_Modal_Handler pFun)
{
	
	if(g_CurExcept >= g_MaxExceptNum)
	{
		g_CurExcept++;
		return 0;
	}
	g_pExcept->sp = Process_SP;
	g_pExcept->ra = Process_RA;
	g_pExcept->except_handle = pFun;
	g_CurExcept++;
	return 1; 
}
void DecExcept(void)
{
    if(g_CurExcept <= 0) return;
    g_CurExcept--;
    g_pExcept[g_CurExcept].except_handle = 0;
    g_pExcept[g_CurExcept].sp            = 0;
    g_pExcept[g_CurExcept].ra            = 0;
}
void c_except_handler(unsigned int *sp)
{
	unsigned int i;
	unsigned int epc = read_c0_epc();
	printf("-----------------------------------------------------\n");
	for(i = 0;i < 4;i++)
	{
		printf("%08x:\t%08x\n",(epc - 4*4 + i*4),*(unsigned int *)((epc - 4 * 4 + i * 4)| 0xa0000000));
  	}
  	for(i = 0;i < 4;i++)
	{
		printf("%08x:\t%08x\n",(epc + i*4),*(unsigned int *)((epc + i*4)| 0xa0000000));
  	}
	printf("-----------------------------------------------------\n");
	printf("CAUSE=%08x EPC=%08x\n", read_c0_cause(), read_c0_epc());
	for (i=0;i<32;i++) {
		if (i % 4 == 0)
			printf("\n");
		printf("%4s %08x ", regstr[i], sp[i]);
	}
	printf("\n");
	g_CurExcept--;
	if(g_CurExcept < g_MaxExceptNum)
	{
		i = (unsigned int)g_pExcept[g_CurExcept].except_handle;
		if(i)
		{
			Process_SP = g_pExcept[g_CurExcept].sp;
			Process_RA = g_pExcept[g_CurExcept].ra;
	  	}
    		}else if(main_except_handle){
		i = (unsigned int)main_except_handle;
    	}else{
        	printf("c_except_handler: while(1)");
		while(1);
    	}
	write_32bit_cp0_register(CP0_EPC,i);
	__asm__ __volatile__("eret\n\t");	
}
/*
void *alloc(int size)
{
	return mem_malloc(size);
}

void deAlloc(void *rmem)
{
	mem_free(rmem);
}
*/
