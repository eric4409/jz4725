#include <bsp.h>
#include <jz4740.h>
#include <ucos_ii.h>
#include <key.h>

#define KEY_TASK_STK_SIZE	1024
#define KEY_TASK_PRIO	1

OS_EVENT *key_sem;
static OS_STK KeyTaskStack[KEY_TASK_STK_SIZE];
unsigned char err;


#define KEY_DETECT_REPEAT_TIME 500
#define KEY_REPEAT_TIME 100
#define KEY_DELAY_TIME 1
#define KEY_DOWN_TIME 2

static int keybuffer[5];
static int keyindex = -1;

unsigned char keyint=0;
unsigned char KeyUp = 1;

unsigned int keybit[4]={0xbe,0xdd,0x7b,0xe7};

unsigned int setpin[4]={0x100,0x200,0x400,0x10000000}; // D8 D9 D10 D28 of GPD
unsigned int clrpin[4]={104,105,106,124}; // D8 D9 D10 D28 of GPD
unsigned int inpin[4]={0x40000,0x80000,0x100000,0x200000}; // D18 D19 D20 D21 of GPC

static void key_interrupt_handler(unsigned int arg)
{
//#if OS_CRITICAL_METHOD==3
//	OS_CPU_SR	cpu_sr;
//#endif
//	OS_ENTER_CRITICAL();

	/*  maks the interrupt of keypad */	
	__gpio_mask_irq_port(GPC)=inpin[0]|inpin[1]|inpin[2]|inpin[3];		
			
//	OSTaskChangePrio(KEY_TASK_PRIO+10,KEY_TASK_PRIO);
	OSSemPost(key_sem);
//	printf("key is pressed \n");
//	OS_EXIT_CRITICAL();
}

static int ScanKey(void)
{
	int i,j;
	
	keyint=1;	/* this variable mean dont use the current image */
	for(i=0;i<4;i++)
	{	
		__gpio_set_port(GPD) |= (setpin[0] | setpin[1] | setpin[2] | setpin[3]);
		/* if the key is power , return */
#if 1
		if(!__gpio_get_pin(GPC*32+21))
		{
			__gpio_clear_port(GPD) |= (setpin[0] | setpin[1] | setpin[2] | setpin[3]);
			return 0x77;	/* return the value for poweroff */

		}
#endif
		__gpio_clear_pin(clrpin[i]);

		/* Delay */
		//OSTimeDly(KEY_DELAY_TIME);
		mdelay(1);
		for(j=0;j<4;j++)
			if( !(__gpio_get_port(GPC)&inpin[j]) )
			{
				__gpio_clear_port(GPD) |= (setpin[0] | setpin[1] | setpin[2] | setpin[3]);
				return ( (keybit[i]&0xf0) | keybit[j]&0x0f );
			}
	}
	
	/* Clear output pin as 0 for interrupt */
	__gpio_clear_port(GPD) |= (setpin[0] | setpin[1] | setpin[2] | setpin[3]);

	return -1;
}

#if 1
static void KeyTaskEntry(void *arg)
{
	int key,oldkey;
	int i;

	printf("Key Install \r\n");
	
	while(1)
	{
//		OSTaskChangePrio(KEY_TASK_PRIO,KEY_TASK_PRIO+10);
		OSSemPend(key_sem, 0, &err);
		oldkey = ScanKey();
		OSTimeDly(KEY_DELAY_TIME);
		KeyUp=1;
	//	printf("oldkey =%x\n",oldkey);
		if(oldkey>=0)
		{
			key = ScanKey();
			if(key==oldkey)
			{
			//	printf("key=%x\n",key);
				keybuffer[++keyindex]=key;
				if(keyindex>4)
					keyindex=4;
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
				keybuffer[++keyindex]=0;
				if(keyindex>4)
					keyindex=4;
				KeyUp=1;	
				break;
			}
		}
		#endif
		__gpio_ack_irq_port(GPC)=inpin[0]|inpin[1]|inpin[2]|inpin[3];		
		__gpio_unmask_irq_port(GPC)=inpin[0]|inpin[1]|inpin[2]|inpin[3];		

	}
}
#endif 

int GetKey(void)
{
	int value,i;
	
	if(keyindex>=0)
	{
		value = keybuffer[0];	

		for(i=0;i<keyindex;i++)
			keybuffer[i]=keybuffer[i+1];

		keyindex--;
		return value;
	}
	else
	{
	//	printf(" no key!\n");
		return -1;
	}
}

/*
 The input key pin are GPC18  GPC19 GPC20 GPC21, The output key pin are GPD8 GPD9 GPD10 ADN GPD28
*/
void KeyGPIOInit(void)
{
/* init the pin as input pin	*/
	__gpio_as_input(GPC*32+18);
        __gpio_as_input(GPC*32+19);
        __gpio_as_input(GPC*32+20);
        __gpio_as_input(GPC*32+21);

/*	init the pin as output pin	*/ 
	__gpio_as_output(GPD*32+8);
        __gpio_as_output(GPD*32+9);
        __gpio_as_output(GPD*32+10);
        __gpio_as_output(GPD*32+28);

	__gpio_enable_pull(GPD*32+8);
	__gpio_enable_pull(GPD*32+9);
	__gpio_enable_pull(GPD*32+10);
	__gpio_enable_pull(GPD*32+28);

/* 	Clear output pin as 0 for interrupt */
	__gpio_clear_port(GPD) |= (setpin[0] | setpin[1] | setpin[2] | setpin[3]);
}

void KeyInit()
{
	int i;

    	key_sem = OSSemCreate(0);
	KeyGPIOInit();

	for(i = 0;i < 4;i++)
		request_irq(IRQ_GPIO_0 + KEY_PIN + i, key_interrupt_handler, 0);

   	OSTaskCreate(KeyTaskEntry, (void *)0,
		     (void *)&KeyTaskStack[KEY_TASK_STK_SIZE - 1],
		     KEY_TASK_PRIO);

    	for(i = 0;i < 4;i++)
	{
		__gpio_as_irq_fall_edge(KEY_PIN + i);
		__gpio_ack_irq(KEY_PIN + i);
		__gpio_unmask_irq(KEY_PIN + i);
	}

}

void InitKeyBuffer(void)
{
	memset(keybuffer,-1,5);
	keyindex = -1;
}


