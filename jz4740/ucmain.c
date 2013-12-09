/**************************************************************************
*                                                                         *
*   PROJECT     : MIPS port for uC/OS-II                                  *
*                                                                         *
*   MODULE      : EX1.c                                                   *
*                                                                         *
*   AUTHOR      : Michael Anburaj                                         *
*                 URL  : http://geocities.com/michaelanburaj/             *
*                 EMAIL: michaelanburaj@hotmail.com                       *
*                                                                         *
*   PROCESSOR   : Any                                                     *
*                                                                         *
*   TOOL-CHAIN  : Any                                                     *
*                                                                         *
*   DESCRIPTION :                                                         *
*   This is a sample code (Ex1) to test uC/OS-II.                         *
*                                                                         *
**************************************************************************/

/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                           (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                               EXAMPLE #1
*********************************************************************************************************
*/

#define LCD_TEST 0
#define RTC_TEST 0
#define WAVTEST 0
#define KEY_TEST 0
#define NAND_TEST 0
#define LCM_TEST 0
#define CIM_TEST 0
#define YAFFS_TEST 0
#define UPDATE_TEST 0
#define MIDWARE_TEST 0

#include "includes.h"
#include "dm.h"
#include "threadprio.h"

/* ********************************************************************* */
/* File local definitions */
//#define  TASK_START_PRIO 80
#define  TASK_STK_SIZE   (1024*40)     /* Size of each task's stacks (# of WORDs) */
#define  NO_TASKS        30             /* Number of identical tasks */
#if DRAM_SIZE == 16
  #define  MEM_END_ADDR	(0x81000000-4)	/* the sdram size is 16M, 0x80000000 to 0x80ffffff */
#else
  #define  MEM_END_ADDR  (0x80800000-4)  /* the sdram size is 8M, 0x80000000 to 0x807fffff */
#endif

//OS_STK   TaskStk[TASK_STK_SIZE];      /* Tasks stacks */
//OS_STK   TaskStartStk[TASK_STK_SIZE];
//char     TaskData[NO_TASKS];                    /* Parameters to pass to each task */

#if MIDWARE_TEST
//#include <sample/midwaretest.c>
#endif

#if KEY_TEST
#include <sample/keytest.c>
#endif

#if WAVTEST
#include <sample/sample_vplay.c>
#endif

#if RTC_TEST

#include <sample/rtctest.c>
#endif

#if LCM_TEST
#include <sample/lcmtest.c>
#endif

#if UPDATE_TEST
#include <sample/updatefile.c>
#endif

unsigned char os_init_completed = 0;
/* ********************************************************************* */
/* Local functions */

void TaskStart (void *data)
{
	data = data;                            /* Prevent compiler warning */
		 
	JZ_StartTicker(OS_TICKS_PER_SEC);	/* os_cfg.h */
        printf("uC/OS-II, The Real-Time Kernel MIPS Ported version\n");
        printf("EXAMPLE #1 %s %s\n",__DATE__,__TIME__);

//      printf("Determining  CPU's capacity ...\n");
//	OSStatInit();                           /* Initialize uC/OS-II's statistics */

#if MIDWARE_TEST
	//MidwareTest();
	InitUdcRam();
	//InitUdcNand();
#endif

#if CIM_TEST
	capture();
#endif 

#if YAFFS_TEST
	yaffstest();
#endif

#if KEY_TEST
        wdt_enable(0);
	KeyTest();
#endif

#if LCM_TEST
	LCMTest(); 
#endif

#if RTC_TEST
     	rtc_test();
	jz_rtc_init();
     	jz_rtc_test();
#endif	

#if WAVTEST
	playwav();
#endif

	/* Initialise the driver */
//	InitializeLCD();
//	LCDShowLogo();
//	KeyInit();
        Debug_Mode();
//        pcm_init();
	pcf_rtc_init();
        jz_rtc_init();
#ifdef UDC_UART
	udc_init();
#endif
//	OSTaskStkCheck(TASK_START_PRIO);
//	OSTaskStkCheck(KEY_TASK_PRIO);
	/* run fimrwae */
	fwmain();

	while(1)
        {
   
#if 0

             printf(" OSTaskCtr:%d", OSTaskCtr);    /* Display #tasks running */
//	     printf(" OSCPUUsage:%d", OSCPUUsage);   /* Display CPU usage in % */
	     printf(" OSCtxSwCt:%d\n", OSCtxSwCtr); /* Display #context switches per second */
	     OSCtxSwCtr = 0;
#endif
	     OSTimeDlyHMSM(0, 0, 1, 0);     /* Wait one second */
        }
}


/* ********************************************************************* */
/* Global functions */
////add by cn 2009-03-22
//OS_EVENT *MainEvent;
void APP_vMain (void)
{
	OSInit();
//	heapInit();
	mem_init();	/* Init the memory head */

	//add by cn 2009-03-22
//	MainEvent=OSSemCreate(0);
	OSTaskCreate(TaskStart, (void *)0,(void *)MEM_END_ADDR/*(void *)&TaskStartStk[TASK_STK_SIZE - 1]*/, TASK_START_PRIO);
        OSStart();                              /* Start multitasking */
	while(1);
}

/* ********************************************************************* */
