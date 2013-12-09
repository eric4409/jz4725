#include <ucos_ii.h>
#include <jz4740.h>
#include <key.h>
#include "fs_api.h"
#include "threadprio.h"
#include "midware.h"

#define MIDWARE_TASK_PRIO	 MIDI_THREAD_PRIO //4
#define MID_TASK_STK_SIZE	1024 * 4
#ifndef JZ4740_LYRA
#define CHARG_DETE_PIN          (15 + 3 * 32)	//GPD15
#define DC_DETE_PIN             (6  + 3 * 32)	//UDC_DETE
#else
#define DC_DETE_PIN              124	//UDC_DETE
#define CHARG_DETE_PIN          (21 + 2 * 32)	//GPC21
#endif


#define LCD_ON                  0
#define LCD_OFF                 1
#define CHARG_ON                0
#define CHARG_OFF               1

volatile static PMIDSRC midsrc[MAX_MID_SRC] = {0 ,0 ,0};
volatile static PMIDOBJ midobj[MAX_MID_OBJ] = {0 };
u8 cursrcnum = 1 , curobjnum = 0 ,  istaskcreat = 0,ishavekey = 0 , err,islcdoff = 0;
OS_EVENT *MidEvent, *MidEvent1, *MidEvent2;
void *MidQ[10];
static u8 chargstate;
static int bk_play_state=0;
static int Save_CPCCR=0;
static int KeepLow=0;

static bk_light_level;
static OS_STK MidTaskStack[MID_TASK_STK_SIZE];
u32 midsrcid, LCDTIME = 0, chargid ;
u32 LcdTimeTick = 0,LcdTimeOut = 0,PendTime = 0,OffTime=0,LcdTime=0;
u8 LcdState = LCD_ON;

//PFN_KEYHANDLE oldkeydown,oldkeyup;
//extern PFN_KEYHANDLE UpKey, DownKey;

volatile static MIDSRC chargsrc;


static int Bat_to_volt(int bat)
{
	return (bat*986*75)/40960+33;
}

u32 MidCheckBatState(void)
{
#ifndef USE_AD_CHARGE
	int dc_pin,charg_pin;
	dc_pin = __gpio_get_pin(DC_DETE_PIN);
	charg_pin = __gpio_get_pin(CHARG_DETE_PIN);
	//printf("DC %d CHARG %d \n",dc_pin,charg_pin);
	if ( dc_pin == 1 && charg_pin == 0 )  //charging
		return BAT_CHARGING;
	else if ( dc_pin == 1 && charg_pin == 1 )  //full
		return BAT_FULL;
	else if ( dc_pin == 0)                //not charging
		return BAT_NOT_CHARGING;

#else
	int bat_val = 2300;
	int dc_pin,charge_pin;;
	bat_val = Read_Battery();
	bat_val = Bat_to_volt(bat_val);
	dc_pin = __gpio_get_pin(DC_DETE_PIN);
	if ( dc_pin == 0)                //not charging
		return BAT_NOT_CHARGING;
	else if ( dc_pin == 1 && bat_val >= BAT_FULL_VALUE && charge_pin == 1)  //full
		return BAT_FULL;
	else               //not charging
		return BAT_CHARGING;

#endif
	
}

/**************************set alarm start*************************************/

static int set_rtc_alm_time(u32 alarm_time)
{
	u32 tick = alarm_time + __rtc_get_second();
	printf("rtc alarm will be %d second late!\n",alarm_time);
	while ( !__rtc_write_ready() ) ;
	REG_RTC_RSAR = tick;

	while ( !__rtc_write_ready() ) ; /* set alarm function */
	if ( !((REG_RTC_RCR>>2) & 0x1) ) {
		__rtc_enable_alarm();
	}

	while ( !__rtc_write_ready() ) ;
	if ( !(REG_RTC_RCR & RTC_RCR_AIE) ) { /* Enable alarm irq */
		__rtc_enable_alarm_irq();
	}
	return 0;
}

static u32 get_rtc_alm_time(void)
{ 
	unsigned long lval;

	while ( !__rtc_write_ready() ) ;
	lval = REG_RTC_RSAR;
	return lval;
}

static u32 calc_new_time(int bat_val)
{
	int now_capa;
	now_capa = bat_val - MIN_BAT_VAL;
	if ( BAT_CAPACITY < now_capa ) now_capa = BAT_CAPACITY;
	return FULL_BAT_TIME / ( BAT_CAPACITY / now_capa ) / TIME_PERCENT ;
}

//-------------------------Old One------------------------------------
#if 0
static void rtc_alarm_set(void)
{
	int bat_val = 2300;
	u32 alarm_time;
	if(MidCheckBatState()!=1)
		bat_val = Read_Battery();
	printf("read battery in rtc! now:%d---min:%d\n",bat_val,MIN_BAT_VAL);
	if ( bat_val <= MIN_BAT_VAL )
	{
		rtc_alarm_stop();
		do_bat_low();
	}
	alarm_time = calc_new_time(bat_val);
	printf("set rtc alarm time! now:%d---min:%d--alarm_time:%d\n",bat_val,MIN_BAT_VAL,alarm_time);
	
	if(alarm_time>0)
		set_rtc_alm_time(alarm_time);
	else
		set_rtc_alm_time(60);
}
#endif
/*
static void tcu_battery_check_set(void)
{
	int bat_val = 2300;
	u32 tcu_time;
	if(MidCheckBatState()!=1)
		bat_val = Read_Battery();
	printf("read battery! now:%d---min:%d\n",bat_val,MIN_BAT_VAL);
	if ( bat_val <= MIN_BAT_VAL )
	{
		JZ_StopTimer2();
		rtc_alarm_stop();
		printf("Battery low!!!!Power off!!!! \n");
		do_bat_low();
		return;
	}
	tcu_time = calc_new_time(bat_val);
	printf("tcu_time = %d \n",tcu_time);
	if(tcu_time > 0)
		tcu_timer_set(2, tcu_time);
	else
		tcu_timer_set(2, 60);
} */
//-------------------------------------------------------------------

//-------------------------------------------------------------------

static void auto_hibernate(void)
{
	PMIDOBJ curobj ;
	
	curobj = midobj[0];
	if ( curobj->Notify( EVENT_AUTOOFF ) ) //Do auto power off
	{
		printf("Now do auto power off! \n");
		jz_pm_hibernate();
	}
}
/*
static void rtc_alarm_set(void)
{
	rtc_alarm_stop();
	auto_hibernate();
}
*/
//-------------------------------------------------------------------

/**************************set alarm end*************************************/

/*
void SetBkLightLevel(int level)
{
	bk_light_level = level;
	lcd_set_backlight(bk_light_level);
}
*/
void SetKeepLow(int keep_low)
{
	KeepLow = keep_low;
}

void SaveStatus()
{
	printf("save:%x\n",REG_CPM_CPCCR);
	Save_CPCCR=REG_CPM_CPCCR;
}

void SetStatus()
{
	printf("before set:%x\n",REG_CPM_CPCCR);
	REG_CPM_CPCCR=Save_CPCCR;
	printf("after set:%x\n",REG_CPM_CPCCR);
}

void SetBkPlayStatus(int status)
{

	 bk_play_state = status;
}
void SdramPowerDown()
{

	REG_EMC_DMCR |= (1 << 18);
}
void SdramPowerUp()
{

	REG_EMC_DMCR |= (0 << 18);
}
/*
void ShutLcd(int type)
{
	int x=0;
	int i;

	lcd_close_backlight();
	__lcd_set_dis();
//	jz_pm_control(3);
	LcdState = LCD_OFF;
	oldkeydown = DownKey;
	oldkeyup = UpKey;
	UpKey = NULL;
	DownKey = NULL;
	printf("LCD Off!!!! \n");

	//x=sys_get_bk_play_status();
	x = sys_get_sleep_enable();

	printf("x:%d\n",x);
	SaveStatus();

#define POWER_OFF_PIN    (29 + 3 * 32)	//GPD29
	
	if(__gpio_get_pin(POWER_OFF_PIN) ==1)
	{
		if(type)
		{
			if(x==1)
			{
				for(i = 0; i < KEY_NUM; i++)
				{
					__gpio_ack_irq(KEY_PIN + i);
					__gpio_unmask_irq(KEY_PIN + i);
			
				}
				jz_pm_control(3);
				for(i = 0; i < KEY_NUM; i++)
					__gpio_mask_irq(KEY_PIN + i);
			}
			else
			{

		
//				jz_pm_control(2);

	
//				SdramPowerDown();
			}
		}
	}
}

void TurnonLcd()
{
	lcdstart();
	lcd_set_backlight(bk_light_level);
	LcdState = LCD_ON;
	UpKey = oldkeyup;
	DownKey = oldkeydown;
	printf("LCD On!!!! \n");

	SetStatus();
	SdramPowerUp();
}
*/
unsigned int RegisterMidSrc(PMIDSRC  psrc)
{
	if(psrc)
	{
		if(cursrcnum > MAX_MID_SRC)
		{
			printf("warning:Max midware source number is %d\r\n",MAX_MID_SRC);
			return -1;
		}
		cli();
		(*psrc).ID = cursrcnum;
		(*psrc).CurEvent = MidEvent;
		(*psrc).CurEvent1 = MidEvent1;
		(*psrc).CurEvent2 = MidEvent2;
		midsrc[cursrcnum] = psrc;
		cursrcnum ++ ;
		sti();
	}
	return 0;
} 

void MidGetState(u32 *dat)
{
	* dat = 0;
}

//------------------------------------------------------------

void CaluPendTime(void)
{
	if ( LcdTime > 0)           //Only shut lcd
	{
		LcdTimeTick = 0;
		LcdTimeOut = 1;
		PendTime = LcdTime;
	}
	else if ( OffTime == 0 && LcdTime == 0 )
	{
		LcdTimeTick = 0;
		LcdTimeOut = 0;
		PendTime = 0;         //never timeout!
	}
	else if ( LcdTime == 0 && OffTime > 0 )           //Do not shut lcd
	{
		LcdTimeTick = 0;
		LcdTimeOut = 0;
		PendTime = OffTime;
	}
	
	printf("Pendtime set to %d %d %d %d\n",
	       PendTime,OffTime,LcdTimeOut);

}

void SetLcdBLTime(u32 dat)
{
	LcdTime = dat;
	CaluPendTime();
	OSQPost(MidEvent1 , (void *)&midsrcid);
	OSSemPost(MidEvent);
}

void SetAutoOffTime(u32 dat)
{
	printf("SetAutoOffTime: %d!\r\n",dat);
	OffTime = dat;
	CaluPendTime();
/*
	if (dat != 0)
		set_rtc_alm_time(dat);
	else
		rtc_alarm_stop();
*/
}

//------------------------------------------------------------

unsigned int RegisterMidObj(PMIDOBJ pobj)
{
	if(pobj)
	{
		if(curobjnum > MAX_MID_OBJ)
		{
			printf("warning:Max midware object number is %d\r\n",MAX_MID_OBJ);
			return -1;
		}
		cli();
		(*pobj).ID = curobjnum;
		(*pobj).GetState = MidGetState;
		(*pobj).SetLcdBLTime = SetLcdBLTime;
		(*pobj).SetAutoOffTime = SetAutoOffTime;
		midobj[curobjnum++] = pobj;
		sti();
	}
	return 0;
} 



static void MidWareTask(void *arg)
{
	volatile u32 eventid;
	PMIDOBJ curobj ;
	PMIDSRC cursrc;
	MIDSRCDTA srcdat;
	u8 err;
	void *p;

	while(1)
	{
		OSSemPend(MidEvent, PendTime, &err);

		p = OSQAccept(MidEvent1,&err);
		if ( p )
			eventid = (u32) (* (u32 *) p) ;
		else eventid = 0;

		if ( !midobj[0] || eventid == MIDWARE_SELF ) continue;
		curobj = midobj[0];

		if ( err == OS_TIMEOUT || eventid == 0)         //timeout ! 
		{
			printf("Event time out ! \n");

			if ( LcdTimeOut > 0 && LcdState == LCD_ON )                //Do shut lcd!
			{
			//	ShutLcd(1);
				curobj->Notify( EVENT_LCD_OFF );
			}

		}
		else
		{
			printf("Event occur id: %d src: %d event %d \n",
			       eventid,midsrc[eventid]->Src,midsrc[eventid]->Event);

			//Not time out!
			if ((TCU_SET_TIMER !=midsrc[eventid]->Src) && ( LcdState == LCD_OFF ) && 
				(SRC_KEY_DOWN !=midsrc[eventid]->Src)) //Do lcd on!
			{
		//		TurnonLcd();
				curobj->Notify( EVENT_LCD_ON );
				LcdState = LCD_ON;
			}
			
			switch (midsrc[eventid]->Src)
			{
			case SRC_KEY:
			case SRC_KEY_DOWN:
			
				printf("Key event ! \n");
				if ((OffTime != 0) && (LcdState == LCD_OFF))
					set_rtc_alm_time(OffTime);
				break;
				
			case SRC_UDC:
				printf("Udc event ! \n");
#if 0				
				if ( midsrc[eventid]->Event == EVENT_USB_IN)
				{
					FS_IoCtl("mmc:",FS_CMD_UNMOUNT,0,0);
					FS_IoCtl("nfl:",FS_CMD_UNMOUNT,0,0);
				}
				else if ( midsrc[eventid]->Event == EVENT_USB_OUT || 
					  midsrc[eventid]->Event == EVENT_UNINSTALL)
				{
					NAND_LB_FLASHCACHE();
					FS_IoCtl("mmc:",FS_CMD_MOUNT,0,0);
					FS_IoCtl("nfl:",FS_CMD_MOUNT,0,0);
				}
#endif
				cursrc = midsrc[eventid];
				srcdat.Val = curobj->Notify( midsrc[eventid]->Event );
				cursrc->Response( &srcdat );
				OSSemPost(MidEvent2);
				break;
			case SRC_MMC:
				printf("MMC event ! \n");
				cursrc = midsrc[eventid];
				srcdat.Val = curobj->Notify( midsrc[eventid]->Event );
				cursrc->Response( &srcdat );
				break;
			case SRC_POWER_OFF:
				printf("Power off event ! \n");
				cursrc = midsrc[eventid];
				srcdat.Val = curobj->Notify( midsrc[eventid]->Event );
				cursrc->Response( &srcdat );
				break;
			case SRC_POWER_LOW:
				printf("Power low event ! \n");
				cursrc = midsrc[eventid];
				srcdat.Val = curobj->Notify( midsrc[eventid]->Event );
				cursrc->Response( &srcdat );
				break;
/*			case SRC_SET_ALM:
				printf("Auto power off ! \n");
				cursrc = midsrc[eventid];
				rtc_alarm_set();
				srcdat.Val = 0;
				cursrc->Response( &srcdat );
				break; */
			case TCU_SET_TIMER:
				printf("Auto battery check ! \n");
				cursrc = midsrc[eventid];
		//		tcu_battery_check_set();
				srcdat.Val = 0;
				cursrc->Response( &srcdat );
			default:
				printf("Unexpect Src! \n");
			}
			printf("Mid ware pass information finish! \n");
		}
	}
}

u32 MidWareInit( PMIDOBJ pobj )
{
	RegisterMidObj( pobj);
//	InitUdcNand();
//	InitUdcMMC();
//	InitUdcRam();
//	KeyInit();
//	mmc_detect_init();
//	rtc_power_check_init();
	//	power_off_init();           //add event source device!
	bk_light_level = 0;
	return 1;
}
 
void RunMidWare()
{
	printf("Run Midware %s %s\n",__DATE__,__TIME__);
	MidEvent1 = OSQCreate(&MidQ[0],10);
	MidEvent = OSSemCreate(0);
	MidEvent2 = OSSemCreate(0);

	__gpio_as_input(CHARG_DETE_PIN);
	__gpio_as_input(DC_DETE_PIN);
	__gpio_disable_pull(DC_DETE_PIN);

//	__gpio_disable_pull(CHARG_DETE_PIN);

	midsrcid = MIDWARE_SELF;
	OSTaskCreate(MidWareTask, (void *)0,
		     (void *)&MidTaskStack[MID_TASK_STK_SIZE - 1],
		     MIDWARE_TASK_PRIO);

}

