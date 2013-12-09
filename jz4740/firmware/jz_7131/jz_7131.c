/*************************************************
                                              
 ZEM 200                                          
                                                    
 jz4730.c init funtions for GPIO/AC97 MUTE/USB POWER
                                                      
 Copyright (C) 2003-2006, ZKSoftware Inc.      	
 
*************************************************/

#include <stdio.h>
#include "arca.h"
#include "jz4740.h"
#include "jz4740_4725.h"
#include "gpio.h"
#include "ucos_ii.h"

char DebugMode = 1;
extern unsigned char KeyUp;
#define KEY_WAIT_TIME   5

/* 
if #define WTD_RTC_CLK 1, wdt_s could be 0 to 2048 second;
if #define WTD_RTC_CLK 0, wdt_s could be 0 to 5 second;
 */
#define WTD_RTC_CLK 1
#define	wdt_s	30	

#define RS485_BIT	0
#define LEDGREEN_BIT	1
#define LEDRED_BIT	2
#define POWER_BIT	3
#define SOUNDEN_BIT	4
#define LCMPOWER_BIT	5
#define HV7131POWER_BIT	6
#define SW_RS232_RS485_BIT 7
/* 	This  variable is for remembering  the status of RS485, GREENLED REDLED POSER
	The defaule value is 0x08, that mean the device is power on
*/
unsigned char StatusLV373 =
			(1<<LEDGREEN_BIT)|
			(1<<LEDRED_BIT)|
			(1<<SOUNDEN_BIT)|
			(1<<RS485_BIT)|
			(1<<POWER_BIT)|
			(1<<HV7131POWER_BIT)|
			(1<<LCMPOWER_BIT);

//because the minmize interval on linux is 10ms, use it when need
//it maybe not exatctly

void CalibrationTimeBaseValue(void)
{
/*
	int i, j, k=0;
    	struct timeval tv;
    	struct timezone tz;
    	unsigned int s_msec, e_msec;
    	int DelayTimes=200*1000;
	
	gettimeofday(&tv, &tz);
    	s_msec = tv.tv_sec*1000*1000 + tv.tv_usec;
    
    	for (i=0; i<DelayTimes; i++)
		for(j=0; j<BASELOOPTESTCNT; j++) k++; 
	
    	gettimeofday(&tv, &tz);
    	e_msec = tv.tv_sec*1000*1000 + tv.tv_usec;
    
    	DBPRINTF("DELAY US=%d BASECNT=%f\n", e_msec-s_msec, ((float)DelayTimes/(e_msec-s_msec))*BASELOOPTESTCNT);    
    	BASELOOPTESTCNT=(int)(((float)DelayTimes/(e_msec-s_msec))*BASELOOPTESTCNT+1);
    
	#if 0
    	gettimeofday(&tv, &tz);
    	s_msec = tv.tv_sec*1000*1000 + tv.tv_usec;
 
    	DelayUS(1000*1000);
    
    	gettimeofday(&tv, &tz);    
    	e_msec = tv.tv_sec*1000*1000 + tv.tv_usec;
    
	DBPRINTF("DELAY US=%d\n", e_msec-s_msec);    
	#endif    
*/
}
extern void udelay(unsigned int usec);

void DelayUS(int us)
{
	udelay(us);
}

void DelayMS(int ms)
{
	DelayUS(1000*ms);
}

void DelayNS(long ns)
{

}

BOOL GPIO_IO_Init(void)
{
	__gpio_as_output(REDLED);	//Red LED
	__gpio_as_output(GREENLED);	//Green LED
	__gpio_as_output(RS485_SEND);
	__gpio_as_output(SOUNDEN);
	__gpio_as_output(SW_RS232_RS485);
	wdt_initialize(wdt_s);

	return TRUE;
}
	
void GPIO_IO_Free(void)
{

}

void RestorStatus(void)
{
	unsigned int d;

//	printf("***>>>Sthe status of LV373 = %x\n",StatusLV373);//treckle
	d = StatusLV373;
        d =(d<<8)&0xff00;
	/* set pin */
        __gpio_set_port(GPC) = d;		
	/* clear pin */
        __gpio_clear_port(GPC) = ~d&0xff00;
}

void GPIO_PIN_CTL_SET(int IsWiegandKeyPad, int IsNetSwitch)
{

}
void GPIO_AC97_Mute(BOOL Switch)
{
        RestorStatus();
        __gpio_set_pin(LV373_LE);
        if (Switch)
        {
                StatusLV373 |= (1<<SOUNDEN_BIT);
                __gpio_set_pin(SOUNDEN);      /* output high level. power on */
        }
        else
        {
                StatusLV373 &= ~(1<<SOUNDEN_BIT);
                __gpio_clear_pin(SOUNDEN);      /* output low level. power off*/
        }
        __gpio_clear_pin(LV373_LE);

}
//control SENSOR ON or OFF 
void GPIO_HY7131_Power(BOOL Switch)
{
        RestorStatus();
        __gpio_set_pin(LV373_LE);
        if (Switch)
        {
                StatusLV373 |= (1<<HV7131POWER_BIT);
                __gpio_set_pin(HV7131POWER);      /* output high level. power on */
        }
        else
        {
                StatusLV373 &= ~(1<<HV7131POWER_BIT);
                __gpio_clear_pin(HV7131POWER);      /* output low level. power off*/
        }
        __gpio_clear_pin(LV373_LE);

}
//control the USB0 power/LCD backgound light 
void GPIO_LCD_USB0_Power(BOOL Switch)
{
        RestorStatus();
        __gpio_set_pin(LV373_LE);
        if (Switch)
        {
                StatusLV373 |= (1<<LCMPOWER_BIT);
                __gpio_set_pin(LCMPOWER);      /* output high level. power on */
        }
        else
        {
                StatusLV373 &= ~(1<<LCMPOWER_BIT);
                __gpio_clear_pin(LCMPOWER);      /* output low level. power off*/
        }
        __gpio_clear_pin(LV373_LE);

}

void GPIO_RS485_Status(U32 SendMode)
{
	RestorStatus();
	__gpio_set_pin(LV373_LE);
	if (SendMode)
	{
		StatusLV373 |= (1<<RS485_BIT);
		__gpio_set_pin(RS485_SEND);      /* output high level */
	}
	else
	{	
		StatusLV373 &= ~(1<<RS485_BIT);
		__gpio_clear_pin(RS485_SEND);      /* output low level */
	}
	__gpio_clear_pin(LV373_LE);
	DelayUS(1000);
}

void GPIO_SWITCH_RS232_RS485(BOOL sw)
{
        RestorStatus();
        __gpio_set_pin(LV373_LE);
        if (sw)
        {
                StatusLV373 |= (1<<SW_RS232_RS485_BIT);
                __gpio_set_pin(SW_RS232_RS485);      /* output high level */
        }
        else
        {
                StatusLV373 &= ~(1<<SW_RS232_RS485_BIT);
                __gpio_clear_pin(SW_RS232_RS485);      /* output low level */
        }
        __gpio_clear_pin(LV373_LE);
}

BOOL GPIOGetLevel(BYTE IOPIN)
{
	return __gpio_get_pin(IO_DOOR_SENSOR-(IOPIN&0x0f));
}

void GPIOSetLevel(BYTE IOPIN, int High)
{/*
	if (High)
		__gpio_clear_pin(IO_DOOR_SENSOR-(IOPIN&0x0f));
	else
		__gpio_set_pin(IO_DOOR_SENSOR-(IOPIN&0x0f));
*/
}

BOOL ExCheckGPI(BYTE GPIOPIN)
{
/*
	int i=200,c=0;
	while(--i)
	{
		if(!GPIOGetLevel(GPIOPIN)) if(++c>20) return FALSE;
		DelayUS(5);
	} */
	return TRUE; 
}

void GPIO_SYS_POWER(int DevID)
{
	RestorStatus();

	/* Enable lv373 */
	__gpio_set_pin(LV373_LE);

	/* Turn off backlight of LCM */
	__gpio_clear_pin(LCMPOWER);
	
	/* Turn off the backlight of sensor */
	__gpio_clear_pin(HV7131POWER);

	/* Disable the amplifier of sound */
	__gpio_clear_pin(SOUNDEN);

	/* Turn off the Green LED */
	__gpio_set_pin(WEI_DP);	

	/* Turn off the Red LED */
	__gpio_set_pin(WEI_DN);
	unlink("/mnt/mtdblock/picture");

	/* waiting key power_off_pin up before power off*/
	if(!DevID)
	{
		while(!__gpio_get_pin(KEY_POWER_PIN));
		OSTimeDly(30);  //delay 30 ms
		while(!__gpio_get_pin(KEY_POWER_PIN));
		OSTimeDly(20);  //delay 20 ms
	}
	else	//for u100
	{
		while(!__gpio_get_pin(KEY_POWER_PIN_U100));
		OSTimeDly(30);  //delay 30 ms
		while(!__gpio_get_pin(KEY_POWER_PIN_U100));
		OSTimeDly(20);  //delay 20 ms
	}
	printf("SYSTEM POWER OFF. BYE !!!\n"); 
	__gpio_clear_pin(POWER);//shut down system

	__gpio_clear_pin(LV373_LE);
}

void GPIO_WIEGAND_LED(BYTE GPIOPIN, BOOL Light)
{
	RestorStatus();
	__gpio_set_pin(LV373_LE);
	if (!Light)	
	{
		if(GPIOPIN==WEI_DN)
			StatusLV373 |= (1<<LEDRED_BIT);
		else 
			StatusLV373 |= (1<<LEDGREEN_BIT);
		__gpio_set_pin(GPIOPIN);
	}
	else
	{
		if(GPIOPIN==WEI_DN)
			StatusLV373 &= ~(1<<LEDRED_BIT);
		else 
			StatusLV373 &= ~(1<<LEDGREEN_BIT);
		__gpio_clear_pin(GPIOPIN);
	}
	__gpio_clear_pin(LV373_LE);
}

//It is used for A5 keypad scanning 
void GPIO_KEY_SET(BYTE Value)
{

}
int GPIO_KEY_GET(void)
{
	return 0;
}
/*
void InitRTC(void)
{
	jz_rtc_init();
}
*/

extern char GetKeyPin(void);
extern void KeyGPIOInit(void);
/* Make sure to call this function after KeyInit() */
void Debug_Mode(void)
{
//	if(GetKeyPin()==2)
	//KeyGPIOInit();

	if(!__gpio_get_pin(GPC*32+20))
                DebugMode = 1;
        else
                DebugMode = 0;
}

void wdt_initialize(int second)
{
#if WDT
	unsigned short d;
/*
   if select RTC clock to be wdt clock, the second could be 0 to 2048 seconds
   if select external clock to be wdt clock, the second could be 0 to 5 seconds
 */
 
#if WTD_RTC_CLK
	/* select the RTC clock to be wdt input clk */
	__wdt_select_rtcclk();

	/* wdt input clk = RTCclock/1024=32 HZ */
	__wdt_select_clk_div1024();
	
	if(second>=2048)
		d = 0xFFFF;
	else		
		d = CFG_RTCCLK*second/1024;	/* example: d=32768*5/1024 = 0xA0*/ 
#else
	/* select the external clock to be wdt input clk */
	__wdt_select_extalclk();

	/* wdt input clk = externalclk/1024 */
	__wdt_select_clk_div1024();
	
	if(second>5)
		d = 0xFFFF;
	else		
		d = CFG_EXTAL*second/1024;	/* example: d=12000000*5/1024 = 0xE4E1*/ 
#endif	
	/*  when connt  == data wdt will reset cpu */
	__wdt_set_data(d);
	__wdt_set_count(0); 
	printf("wdt initialized\n");
#endif
}

void wdt_enable(char e)
{
#if WDT
	if(e)
	{
		__wdt_select_rtcclk();
		__wdt_start();
		printf("wdt start\n");
	}
	else
	{
		__wdt_set_count(0);
		__wdt_stop();
		__wdt_stop_clk();
		printf("wdt stop\n");
	}
#endif
}

void wdt_set_count(int c)
{
#if WDT
	__wdt_set_count(c);
#endif
}

U32 GetMS(void)
{
	return 1000/OS_TICKS_PER_SEC * OSTimeGet();
}
U32 SetCPUSpeed(U32 speed)
{       
    return 1;
}       
