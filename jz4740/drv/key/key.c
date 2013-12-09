#include <stdio.h>
#include <string.h>
#include <bsp.h>
#include <jz4740.h>
#include <ucos_ii.h>
#include <key.h>
#include <libc.h>
#include "threadprio.h"
#include "clock.h"
#include "intc.h"

#define KEY_TASK_STK_SIZE	(1024*2)
//#define KEY_TASK_PRIO	1

OS_EVENT *key_sem;
static OS_STK KeyTaskStack[KEY_TASK_STK_SIZE];
unsigned char err;


#define KEY_DETECT_REPEAT_TIME 500
#define KEY_REPEAT_TIME 100
#define KEY_DELAY_TIME 1
#define KEY_DOWN_TIME 2
#define KB_SIZE	16
static int keybuffer[KB_SIZE];
static int keyindex = 0;
static int keyOut = 0;

volatile unsigned char keyint=0;
unsigned char KeyUp = 1;
static int DevID = 0;

const unsigned int keyval[4]={0xbe,0xdd,0x7b,0xe7};

const unsigned int keyval_1[16]={
	0xde,0xdd,0xdb,0xd7,
	0xee,0xed,0xeb,0xe7,
	0x7e,0x7d,0x7b,0x77,
	0xbe,0xbd,0xbb,0xb7
};

const unsigned int setpin[4]={0x100,0x200,0x400,0x10000000}; // D8 D9 D10 D28 of GPD
const unsigned int clrpin[4]={104,105,106,124}; // D8 D9 D10 D28 of GPD
const unsigned int inpin[4]={0x40000,0x80000,0x100000,0x200000}; // D18 D19 D20 D21 of GPC
#if 0
static void timerHander(unsigned int arg)
{
        __tcu_mask_full_match_irq(1);
        __tcu_clear_full_match_flag(1);
	__tcu_stop_counter(1);
	__gpio_clear_pin(KLED);
//        printf("arg = %d!\n",arg);
}
#endif

#define RTC_CLK		32768
#define tps	5	// 5 seconds
#define latch 	(tps * RTC_CLK / 1024)

static void StartTimer(void)
{
	__tcu_stop_counter(1);
	__tcu_set_count(1,0);
        __tcu_clear_full_match_flag(1);
        __tcu_unmask_full_match_irq(1);
        __tcu_start_counter(1);
}
#if 0
static void InitTimer1(void)
{
        __tcu_disable_pwm_output(1);
        __tcu_select_rtcclk(1);
        __tcu_select_clk_div1024(1);
        __tcu_mask_half_match_irq(1);
	__tcu_set_full_data(1,latch);
	__tcu_set_half_data(1,-1);
        request_irq(IRQ_TCU1,timerHander,1);
}

void _74HC164Send(unsigned char d)
{
        int j;

        for(j=0;j<8;j++)
        {
        	if (d&1)
                	__gpio_set_pin(DATAPIN);
        	else
                	__gpio_clear_pin(DATAPIN);

		__gpio_clear_pin(CLKPIN);
		__gpio_set_pin(CLKPIN);

         	d >>= 1;
        }
}
#endif

static void key_interrupt_handler(unsigned int arg)
{
	/*  maks the interrupt of keypad */	
	__gpio_mask_irq_port(GPC)=inpin[0]|inpin[1]|inpin[2]|inpin[3];		

//	OSTaskChangePrio(KEY_TASK_PRIO+10,KEY_TASK_PRIO);
//	printf("key is pressed \n");
	OSSemPost(key_sem);
}

int GetKeyPadValue2(char *key)
{
	U32 res = 0;

	res = Scan164();   
	if(res > 0 && res <= 16)
	{
		*key = res;
		return 1;
	}
	return  0;
}

/*
3 5 1
6 2 4
9 8 7
c b a
*/
static int ScanKey(void)
{
	return 0;
}

#if 1
static void KeyTaskEntry(void *arg)
{
	int key,oldkey;
	char buf[1];

	printf("Key Installed. \r\n");
	while(1)
	{
		if(L3000KeyCheck(buf)) 
		{
			PutKey(buf[0]);
			PutKey(0);
		}
		L3000ProcTickMsg(NULL);
		OSTimeDly(5);
	}

	while(1)
	{
	//	OSTaskChangePrio(KEY_TASK_PRIO,KEY_TASK_PRIO+10);

//		OSSemPend(key_sem, 0, &err);
#if 0
		if(DevID==1)
		{
			__gpio_set_pin(KLED); /* turn on key led */
			StartTimer();
		}
#endif
		oldkey = ScanKey();
		OSTimeDly(KEY_DELAY_TIME);
		KeyUp=1;
	//	printf("oldkey =%x\n",oldkey);
		if(oldkey>=0)
		{
			key = ScanKey();
			if(key==oldkey)
			{
	//			printf("key=%x\n",key);
			#if 0
				keybuffer[++keyindex]=key;
				if(keyindex>=KB_SIZE-1)
					keyindex=-1;
			#endif
				PutKey(key);

				KeyUp=0;
			}		
		}
		#if 1
		/* KeyUp == 0 mean key is down */
		while(!KeyUp)
		{
			OSTimeDly(KEY_DOWN_TIME);
			key = ScanKey();
			if(key!=oldkey)	/* mean key up */
			{
			#if 0
				keybuffer[++keyindex]=0;
				if(keyindex>=KB_SIZE-1)
					keyindex=-1;
			#endif
				PutKey(0);

				KeyUp=1;	
				break;
			}
		}
		#endif
		
//		__gpio_ack_irq_port(GPC)=inpin[0]|inpin[1]|inpin[2]|inpin[3];		
//		__gpio_unmask_irq_port(GPC)=inpin[0]|inpin[1]|inpin[2]|inpin[3];		
	}
}
#endif 

int GetKey(void)
{
	int key;

	if(keyindex == keyOut) return 0;

	key = keybuffer[keyOut];
	keyOut = (keyOut + 1) % KB_SIZE;

	return key;
}

void PutKey(int key)
{
	if( (keyindex + 1) % KB_SIZE != keyOut )
	{
		keybuffer[keyindex] = key;
		keyindex = (keyindex + 1 ) % KB_SIZE;
	}
}

/*
 The input key pin are GPC18  GPC19 GPC20 GPC21, The output key pin are GPD8 GPD9 GPD10 ADN GPD28
 DevID: 0 - h1plus
		 1 - u100
*/
#if 0
void KeyGPIOInit(void)
{
/* init the pin as input pin	*/
	__gpio_as_input(GPC*32+18);
        __gpio_as_input(GPC*32+19);
        __gpio_as_input(GPC*32+20);
        __gpio_as_input(GPC*32+21);

/*	init the pin as output pin	*/ 

	if(!DevID)
		__gpio_as_output(GPD*32+8); 
	else
		__gpio_as_input(GPD*32+8); //GPD*32+8 is KEYPOW 
		
        __gpio_as_output(GPD*32+9);
        __gpio_as_output(GPD*32+10);
        __gpio_as_output(GPD*32+28);

	if(!DevID)
		__gpio_enable_pull(GPD*32+8);

	__gpio_enable_pull(GPD*32+9);
	__gpio_enable_pull(GPD*32+10);
	__gpio_enable_pull(GPD*32+28);

/* 	Clear output pin as 0 for interrupt */
	if(!DevID)
		__gpio_clear_port(GPD) |= (setpin[0] | setpin[1] | setpin[2] | setpin[3]);
	else
		_74HC164Send(0);
}
#endif

void InitKeyBuffer(void)
{
	memset(keybuffer,-1,sizeof(keybuffer));
	keyindex = 0;
	keyOut = 0;
}

void KeyInit(int DeviceID)
{
	int i;

	InitKeyBuffer();

   	OSTaskCreate(KeyTaskEntry, (void *)0,
		     (void *)&KeyTaskStack[KEY_TASK_STK_SIZE - 1],
		     KEY_TASK_PRIO);
	return;
#if 0
	DevID = DeviceID;
    	key_sem = OSSemCreate(0);
	return	;
	KeyGPIOInit();

	for(i = 0;i < 4;i++)
		request_irq(IRQ_GPIO_0 + KEY_PIN + i, key_interrupt_handler, 0);

	if(DevID==1)
	{
		request_irq(IRQ_GPIO_0 + KEYPOW, key_interrupt_handler, 0);
		__gpio_as_irq_fall_edge(KEYPOW);
		__gpio_ack_irq(KEYPOW);
		__gpio_unmask_irq(KEYPOW);
		InitTimer1();
	}
   	OSTaskCreate(KeyTaskEntry, (void *)0,
		     (void *)&KeyTaskStack[KEY_TASK_STK_SIZE - 1],
		     KEY_TASK_PRIO);

    	for(i = 0;i < 4;i++)
	{
		__gpio_as_irq_fall_edge(KEY_PIN + i);
		__gpio_ack_irq(KEY_PIN + i);
		__gpio_unmask_irq(KEY_PIN + i);
	}

	InitKeyBuffer();
#endif
}

char GetKeyPin(void)
{
	int d;

	d = __gpio_get_port(GPC);
	if( !(d&inpin[0]) &&
	    	!(d&inpin[2]) &&
	    	(d&inpin[1]) &&
		(d&inpin[3]) )
	 	return 0;
			
	if( (d&inpin[0]) &&
	    	(d&inpin[2]) &&
	    	!(d&inpin[1]) &&
		!(d&inpin[3]) )
		return 1;

	if( (d&inpin[0]) &&
	    	(d&inpin[1]) &&
	    	!(d&inpin[2]) &&
		(d&inpin[3]) )
		return 2;

	return -1;
}


