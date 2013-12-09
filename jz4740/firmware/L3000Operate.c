#include	"stdio.h"
#include	"stdlib.h"
#include	"string.h"
#include	"exfun.h"
#include	"L3000Operate.h"
#include	"lcm.h"
#include	"L3000BMP.h"
#include	"lcdmenu.h"
#include    "flashdb.h"
#if 0
#include	"strres.h"
#include	"msg.h"
#include "mipsregs.h"
#include "jz4740.h"
#include	"options.h"
#include	"lcdmenu.h"
//#include	"flash.h"
#include	"serial.h"
#include   "OLedDisplay.h"
#include   "kb.h"

//#define	L3000_TEST 1
//#define	HAVE_MF_FUN 1
//#define 	HAVE_RF_FUN 0 
#endif 

extern	int	WaitAdminVerifyCount;
extern  TTime gCurTime;
extern	int gHaveRTC;
int	L3000LowClk = 0;
extern	int	ShowMainLCDDelay; //等待延迟显示主界面时间
int		gUserOperateTime = 0;
int		gUserOperateMaxTime = DFT_USER_OPERATE_MAX_TIME;
TLockCtl gLock;
TBeepCtl gBeep;
TBeepCtl gRLed;
TBeepCtl gGLed;
TInputPwd VryPwd;
TL3000Sta  gState;
TCartoon   gCartoon;
TVryBind   VryBind;
int	gShowLowPwrAlm = 0;
int	VryFailCnt = 0;
int	gVryFailCnt = 0;
int	L3000MaxPwdCnt = MAX_PWD_CNT; 
int	 L3000TestTick = 0;
int      L3000TestFg = 0;
int	 L3000PwdMode = PWD_MODE;
static	TUser VryPassUser;

void	L3000FeedWatchDog()
{
		;
}

void    L3000Debug(char *str)
{
//  DebugOutput1("\npwd: %s", str);
}           


void	L3000PortInit()
{
	//L3000_GPIO_INIT();
	__gpio_as_input(L3000_BAK_PWR);  
	__gpio_as_output(L3000_PWR_EN); 
	//L3000_KEY_INIT();
//	L3000_RTC_INIT();
//	OLED_PIN_INIT();
//	L3000_STOP_DOOR();
	L3000_POWER_ON() ;
//	L3000_BEEP_OFF();
//	L3000_GLED_OFF();
//	L3000_RLED_OFF();
//	L3000_FG_PWR_ON();
}

void	L3000RtcInit()
{
	;		
}

void	L3000InitOptions()
{
	gOptions.TimeOutMenu = 15;
	gOptions.AdminCnt = 1;
	gOptions.One2OneFunOn = 0;
	gOptions.MaxFingerCount = MAX_FINGER_CNT;
	gOptions.MaxAttLogCount = MAX_LOG_CNT;
	gOptions.IdleMinute=0;
	gOptions.AttState=0;
	gOptions.RS232BaudRate=115200;
	gOptions.ShowState=0;
	gOptions.ExtWGInFunOn=0;
	gOptions.NetworkFunOn=0;
	gOptions.LockFunOn=2;
	gOptions.VoiceOn=0;
	gOptions.ShowScore=0;
	gOptions.RS485On=0;
	gOptions.SaveAttLog=1;
	gOptions.RS232Fun==1;
	gOptions.RS232On = 1;
	gOptions.PowerMngFunOn = 0;
//	gOptions.BTBR=115200;
	gOptions.Must1To1 = 0;
	gOptions.MustEnroll = 1;
	gOptions.PIN2Width = 5;
	gOptions.MaxUserFingerCount = 10;
	gOptions.C2FunOn = 0;
	gOptions.PrinterOn = 0;
	gOptions.IsTestMachine = 0;
	gOptions.VoiceFunOn = 0;
	gOptions.EnrollCount = 3;
	//gOptions.MifareFunOn = 3;
	gOptions.MasterSlaveFunOn = 0;
	gOptions.AntiPassbackFunOn = 0;
	gOptions.MenuStyle=1;//MenuStyle_ICON;
	gOptions.I1ToG = 0;
	gOptions.I1ToH = 0;
	gOptions.CloseDoorHint = 0;
		
/*
#if (HAVE_RF_FUN == 1)
	gOptions.RFCardFunOn = 1;
	gOptions.MifareFunOn = 0;
#endif

#if (HAVE_MF_FUN == 1)
	gOptions.RFCardFunOn = 0;
	gOptions.MifareFunOn = 1;
	gOptions.CanChangeCardKey = 1;
#endif
*/
/*	if(gOptions.IsIDMFLock == 'I'){
		gOptions.RFCardFunOn = 1;
		gOptions.MifareFunOn = 0;
	}
	else	if(gOptions.IsIDMFLock == 'M'){
		gOptions.RFCardFunOn = 0;
		gOptions.MifareFunOn = 1;
		gOptions.CanChangeCardKey = 1;	
	}	
	else	;
*/
	if(gOptions.RFCardFunOn && gOptions.MifareFunOn){
		if(gOptions.MifareFunOn == 0);
	}

/*	if(gOptions.MifareFunOn == 0 && gOptions.RFCardFunOn == 0){
		gOptions.RFCardFunOn = 1;
	}
*/
	if(gOptions.RFCardFunOn) gOptions.OnlyPINCard = 1;

	L3000PwdMode = gOptions.PwdBitsMode;
	if(L3000PwdMode != 3 && L3000PwdMode != 4){
		L3000PwdMode = 3;
		gOptions.PwdBitsMode = 3;
	}
#if (FP_READER != 1) 
//	gOptions.OImageWidth = 532;
//	gOptions.OImageHeight = 424;
  //      gOptions.OTopLine = 44;
    //    gOptions.OLeftLine=64;
     //   gOptions.CPX[0]=461;
//	gOptions.CPX[1]=69;
//	gOptions.CPX[2]=531;
  //      gOptions.CPX[3]=0;
//        gOptions.CPY[0]=423;
  //      gOptions.CPY[1]=423;
//	gOptions.CPY[2]=0;
//	gOptions.CPY[3]=0;
//	gOptions.NewFPReader = 1;
//	gOptions.CMOSGC = 0;
//	gOptions.ZF_WIDTH = 292;
//	gOptions.ZF_HEIGHT=380;
//	gOptions.TopThr = 80;
//       	gOptions.MinThr = 45;	
//	gOptions.IncThr = 18;
//	gOptions.EThreshold = 45;
//	gOptions.MThreshold = 68;	
//	gOptions.MaxNoiseThr = 90;
#endif
//	gOptions.
	
	
}

void	L3000OperateInit(void)
{
//	gOLED_En = 1;
	memset(&gState, 0, sizeof(TL3000Sta));
	memset(&gCartoon, 0, sizeof(TCartoon));
	memset(&VryBind, 0, sizeof(TVryBind));
	//OLED_Init();
	LedCtlInit();
	BeepCtlInit();
	LockCtlInit();
	//L3000KeyInit();
	//L3000RtcInit();
	//L3000SwitchInit();
	L3000InitOptions();	
	//I2CInitialize();
	memset((U8 *)&VryPwd, 0, sizeof(TInputPwd));
}

void	FirstCheckPower()
{
	int	i = SWI_CNT_UPLIMIT+5;
			
	while(--i){
		L3000CheckSwitch(L3000SwGroup);
		DelayMS(1);
	}
	if(L3000SwGroup[SW_BAK_PWR] >= SWI_CNT_UPLIMIT){
		gState.pwr_bak = 1;
	}
	if(L3000SwGroup[SW_LOW_PWR_OFF] >= SWI_CNT_UPLIMIT){
		gState.voltage = SW_LOW_PWR_OFF;
		FinishProgram();		
	}
	else	if(L3000SwGroup[SW_LOW_PWR_ALM] >= SWI_CNT_UPLIMIT){
		gState.voltage = SW_LOW_PWR_ALM;		
		L3000ShowWarn(NULL, LoadStrByID(HID_ALARM_LOWBATTERY), 2, 0);
	}
	else	if(L3000SwGroup[SW_LOW_PWR] >= SWI_CNT_UPLIMIT){
		gState.voltage = SW_LOW_PWR;
	}
	else	;	
}

void	RcvDefaultRtc(TTime *t)
{
	t->tm_sec = 0;
	t->tm_min = 0;
	t->tm_hour = 0;
	t->tm_mday = 1;
	t->tm_mon = 1;
	t->tm_year = 2008;
}

int	IsStepTick(int *init)
{
	static U32 old_ms = 0;
	U32 ms = 0;
	int ret = 0;

	ms  = GetMS();
	if(*init == 0){
		*init = 1;
		old_ms = ms;
		ret = 1;
	}
	else	if(ms > old_ms + RUN_TICK || ms < old_ms){
		ret = 1;
		old_ms = ms;
	}
	return	ret;
}

int    ReadVryFailCnt(void)
{
     int	i;

     i = LoadInteger("VFCNT", 0);
     if(gOptions.VryFailWait == 1){
	if(i > gOptions.VryFailWaitCnt){
		SaveVryFailCnt(-1);
		return	0;
	}
	else	return	i;
     }
     else	if(i){
     	SaveVryFailCnt(-1);
     }	
     return  0;
}

void	SaveVryFailCnt(int cnt)
{
	int	i;

	if(cnt == -1 && gVryFailCnt){

		SaveInteger("VFCNT", 0);
		return;
	}
	if(!gOptions.VryFailWait) return	;

	if(cnt > 0 && gOptions.VryFailWait){
		i = cnt + gVryFailCnt;
		if(i <= gOptions.VryFailWaitCnt)
		{	SaveInteger("VFCNT", i);}
	}
}

#define 	VRYFAIL_WAIT_SEC   30
void	JudgeVryFailWait()
{
	int	i = VRYFAIL_WAIT_SEC; 
	
	gVryFailCnt =  ReadVryFailCnt();
	if(gVryFailCnt == gOptions.VryFailWaitCnt){
		L3000ExBeep(100);		
		ClkManage(2);
		L3000_RLED_OFF();
		L3000_GLED_OFF();
		L3000_BEEP_OFF();
		L3000_FG_PWR_OFF();
		ShowVryFailWait(i);
		do{
			ShowVryFailWait(i);
			DelayMS(253);
			L3000ExBeep(80);
			DelayMS(253);
			L3000ExBeep(80);
			DelayMS(253);
			L3000ExBeep(80);
			L3000Debug("\nVryFail count");		
			wdt_set_count(0);
		}
		while(--i);		
		SaveVryFailCnt(-1);		
		gVryFailCnt = 0;
		FinishProgram();		
	}
}

int	RtcInvalidFg = 0;
#if 1
int	GetRtcFirst()
{
	int	ret = -1;
	TTime  t;  

	gHaveRTC = -1;
	GetTime(NULL);
	if(gHaveRTC != 1){
		L3000ShowInfo(NULL, L3000_STR_REPLACE8, 0, 0);	
		gHaveRTC = -1;
		GetTime(NULL);
		if(gHaveRTC != 1){
			RcvDefaultRtc(&t);
			gHaveRTC =1;			
			SetTime(&t);
			gHaveRTC = -1;
			GetTime(NULL);
			if(gHaveRTC != 1){
				L3000ShowWarn(NULL, L3000_STR_REPLACE3, 2, 0);
			}		
			else	{
				L3000ShowWarn(NULL, L3000_STR_REPLACE2, 2, 0);
				RtcInvalidFg = 1;
			}					
				
		}
	}	
}
#endif

int	L3000CheckKeyDel()
{
	static	int 	sw_grp[3] = {0,0,0};

	if(gMachineState != STA_VERIFYING
//	|| VryPwd.bits != 4
	|| gState.pwr_bak != 0
	|| gState.voltage > SW_LOW_PWR
	|| gState.state != 0
	|| WaitAdminVerifyCount == 0){
		memset(sw_grp, 0, sizeof(sw_grp));	
		return	0;
	}

	if(GET_KEY_INPUTPIN(PIN_KEY_DEL) == 0){
		if(sw_grp[0] < 1) ++sw_grp[0];
	}
	//else	if(GET_KEY_INPUTPIN(PIN_KEY_NOR_OPEN) == 0 && sw_grp[1]<10) ++sw_grp[1];
	else	if(sw_grp[2] < 1) ++sw_grp[2];
	else	;

	if((sw_grp[0] + sw_grp[1] + sw_grp[2]) >= 2) {
		memset(sw_grp, 0, sizeof(sw_grp));	
		L3000RunBeep(60, 0, 0);
		return 1;
	}
	else	return	0;
	
#if(0)	
	static	int	tick = 0;
	static	int	start = 0;
	if(gMachineState != STA_VERIFYING
//	|| VryPwd.bits != 4
	|| gState.pwr_bak != 0
	|| gState.voltage > SW_LOW_PWR
	|| gState.state != 0
	|| WaitAdminVerifyCount == 0){
		tick = 0;
		start = 0;
		return	0;
	}
	if(GET_KEY_INPUTPIN(PIN_KEY_DEL) == 0 || GET_KEY_INPUTPIN(PIN_KEY_NOR_OPEN) == 0){
		if(start == 1 && tick){
			tick--;
			if(tick == 0){
				start = 0;
				tick = 0;
				L3000RunBeep(60, 0, 0);
				return	1;
			}
		}
		else	{
			tick = 0;
			start = 0;
		}
	}
	else{
		tick++;
		if(tick > 10) {
			tick = 10;
			start = 1;
		}
	}
	return	0;
#endif
}


/***********************************RTC*********************************************/
void	RemindResult(int fail, int back_ground)
{
	if(back_ground == 1)
	{
		if(fail == 0){
			L3000RunBeep(500, 0, 0);	
			L3000RunGLed(3000, 0,0);
		}
		else{
			L3000RunBeep(120,80, 1);
			L3000RunRLed(3000, 0, 0);
		}	
	}
	else	{
		if(fail == 0){
			L3000_GLED_ON();
			L3000ExBeep(500);
			DelayMS(200);
			L3000_GLED_OFF();
		}		
		else	{
			L3000_RLED_ON();
			L3000ExBeep(120);
			DelayMS(80);
			L3000ExBeep(120);
			DelayMS(200);
			L3000_RLED_OFF();
		}
	}
}

/***********************************Led Display*********************************************/
void	LedCtlInit()
{
	memset((U8 *)&gRLed, 0, sizeof(TBeepCtl));
	memset((U8 *)&gGLed, 0, sizeof(TBeepCtl));
}

void	L3000ExRLed(int	ms)
{
	L3000_RLED_ON();
	L3000_GLED_OFF();
	DelayMS(ms);
	L3000_RLED_OFF();	
}

void	L3000ExGLed(int	ms)
{
	L3000_GLED_ON();
	L3000_RLED_OFF();	
	DelayMS(ms);
	L3000_GLED_OFF();	
}

void L3000RunRLed(U32	light_ms, U32	interval_ms,  U32	cycle)
{
	memset(&gGLed, 0, sizeof(gGLed));
	gRLed.cycle = cycle;
	gRLed.on_ms = light_ms/RUN_TICK;
	gRLed.off_ms = interval_ms/RUN_TICK;
	if(gRLed.cycle){
		gRLed.cycle_on_ms = light_ms/RUN_TICK;
		gRLed.cycle_off_ms = interval_ms/RUN_TICK;
	}
	if(gRLed.on_ms) {
		//L3000_BEEP_ON();
	}	
}

void L3000RunGLed(U32	light_ms, U32	interval_ms,  U32	cycle)
{
	memset(&gRLed, 0, sizeof(gRLed));
	gGLed.cycle = cycle;
	gGLed.on_ms = light_ms/RUN_TICK;
	gGLed.off_ms = interval_ms/RUN_TICK;
	if(gGLed.cycle){
		gGLed.cycle_on_ms = light_ms/RUN_TICK;
		gGLed.cycle_off_ms = interval_ms/RUN_TICK;
	}
	if(gGLed.on_ms) {
		//L3000_BEEP_ON();
	}	
}

void ControlLedOutput(void)
{
	int	rled = 0;
	int gled = 0;
	if(gRLed.on_ms){
		rled = 1;
		if(--gRLed.on_ms == 0){
			;	//		rled = 0;
		}
	}
	else	if(gRLed.off_ms){		
		if(--gRLed.off_ms == 0){
			if(gRLed.cycle){
				gRLed.cycle--;
				gRLed.on_ms = gRLed.cycle_on_ms;
				gRLed.off_ms = gRLed.cycle_off_ms;
			}
		}
	}
	else;

	if(gGLed.on_ms){
		gled = 1;
		if(--gGLed.on_ms == 0){
			;//gled = 0;
		}
	}
	else	if(gGLed.off_ms){		
		if(--gGLed.off_ms == 0){
			if(gGLed.cycle){
				gGLed.cycle--;
				gGLed.on_ms = gGLed.cycle_on_ms;
				gGLed.off_ms = gGLed.cycle_off_ms;
			}
		}
	}
	else;

	if(rled) {L3000_RLED_ON();}
	else	 {L3000_RLED_OFF();}

	if(gled) {L3000_GLED_ON();}
	else	 {L3000_GLED_OFF();}
}

/*******************BEEP OPERATE*************************************************/
void	BeepCtlInit(viod)
{
	memset((U8 *)&gBeep, 0, sizeof(TBeepCtl));
}

void	L3000ExBeep(int	ms)
{
	L3000_BEEP_ON();
	DelayMS(ms);
	L3000_BEEP_OFF();	
}

void	L3000RunBeep(U32	tweet_ms, U32	interval_ms,  U32	cycle)
{
	gBeep.cycle = cycle;
	gBeep.on_ms = tweet_ms/RUN_TICK;
	gBeep.off_ms = interval_ms/RUN_TICK;
	if(gBeep.cycle){
		gBeep.cycle_on_ms = tweet_ms/RUN_TICK;
		gBeep.cycle_off_ms = interval_ms/RUN_TICK;
	}
	if(gBeep.on_ms) {
		//L3000_BEEP_ON();
	}
}

void	ControlBeepOutput(void)
{
	int	beep = 0;
	
	if(gBeep.on_ms){
		beep = 1;
		if(--gBeep.on_ms == 0){
	//;	beep = 0;
		}
	}
	else	if(gBeep.off_ms && gBeep.cycle){		
		if(--gBeep.off_ms == 0){
			--gBeep.cycle;
			gBeep.on_ms = gBeep.cycle_on_ms;
			gBeep.off_ms = gBeep.cycle_off_ms;					
		}
	}
	else	{
		gBeep.off_ms = 0;
		gBeep.cycle = 0;
	}

	if(beep) { L3000_BEEP_ON();}
	else	 { L3000_BEEP_OFF();}
}
/********************************************************************************/


/************************LOCK OPERATE***********************************/
void	SetLockNorStateParam(int onoff)
{
	if(onoff == LOCK_STATE_OPEN){
		SaveInteger("LOCK_NORMAL", LOCK_STATE_OPEN);
	}
	else	SaveInteger("LOCK_NORMAL", LOCK_STATE_CLOSE);
}

int    ReadLockNorStateParam(void)
{
	if(LoadInteger("LOCK_NORMAL", LOCK_STATE_CLOSE) == LOCK_STATE_OPEN)	return	LOCK_STATE_OPEN;
	else	return	LOCK_STATE_CLOSE;
}

int	L3000CheckNormalUnLock(void)
{
	if(gState.state == STA_VRY_END && gLock.open_delay_ms && gLock.close_ms){
		return	1;
	}
	else	return	0;
}

int	L3000AvoidNorUnLock()
{
	if(gLock.nor_open == 1 &&  gState.state & STA_NOR_OPEN){
		return	1;
	}
	return	0;
}

void	LockCtlInit(void)
{
	int	va = 0;
	memset((U8 *)&gLock, 0, sizeof(TLockCtl));
	gLock.nor_open = ReadLockNorStateParam();
	if(gLock.nor_open == LOCK_STATE_OPEN)
	{	
		gState.state  |=  STA_NOR_OPEN;
	//	ClkManage(1);
	}
/*	if(gLock.nor_open == LOCK_STATE_OPEN) {
		gLock.cur_state = LOCK_STATE_OPEN;
	}	
	else	gLock.cur_state = LOCK_STATE_CLOSE;*/
}

void	L3000ExLock(onoff)
{
	if(onoff == 0){
		L3000_CLOSE_DOOR();
		DelayMS(LOCK_CLOSE_TICK * RUN_TICK);
		L3000_STOP_DOOR();
		DelayMS(50);
//		gLock.cur_state = LOCK_STATE_CLOSE;
//		if(gLock.nor_open = LOCK_STATE_OPEN){
//			gLock.nor_open = LOCK_STATE_CLOSE;
//			SetLockNorStateParam(LOCK_STATE_CLOSE);
//		}
	}
	else	if(onoff == 1){
		L3000_OPEN_DOOR();
		DelayMS(LOCK_OPEN_TICK * RUN_TICK);
		L3000_STOP_DOOR();
		DelayMS(50);
//		gLock.cur_state = LOCK_STATE_OPEN;
//		if(gLock.nor_open = LOCK_STATE_CLOSE){
//			gLock.nor_open = LOCK_STATE_OPEN;
//			SetLockNorStateParam(LOCK_STATE_OPEN);
//		}
	}
	else	;
}


void	ClkManage(int	type)
{
	if(type == L3000LowClk) return;
	pll_init(type);	
	L3000LowClk = type;
	if(type)
	{
		//StopCaptureImage();	chenyy
       	L3000_FG_PWR_OFF();          		
	}
}

int	EnFPVry = 0;

void DisableFPVry(int type){
	if(type == EnFPVry) return;
//	if(type != L3000LowClk){
	ClkManage(type);
//	}
	if(type == 0){
		L3000_FG_PWR_ON();
		DelayMS(250);
		/* chenyy
		InitCMOS(gOptions.OLeftLine, gOptions.OTopLine,	gOptions.OImageWidth,gOptions.OImageHeight, gOptions.NewFPReader); //0x104, 340, 116, sWidth, sHeight);				
		*/
	}
	else {
;        //	L3000_FG_PWR_OFF();          
	}
	EnFPVry = type;		
}	

int	L3000RunLock(int	onoff)
{
	int	ret = -1;
	   
            
         if(gLock.close_ms == 0 && gLock.open_ms == 0){
            if(onoff == 0){
	//		if(gLock.cur_state == LOCK_STATE_OPEN){
	           gLock.close_ms = LOCK_CLOSE_TICK;		
		   gLock.open_delay_ms = 0;
		   gLock.open_ms = 0;		   
		   ret = 0;
	//		}
	    }
	    else    if(onoff == 0xff){
	  //         if(gLock.cur_state == LOCK_STATE_CLOSE){
		gLock.open_ms = LOCK_OPEN_TICK;
		gLock.open_delay_ms = 0;
		gLock.close_ms = 0;
		ret = 0;
	//		   }		   
	    }
	    else   {
	//		   if(gLock.cur_state == LOCK_STATE_CLOSE){
		gLock.open_ms = LOCK_OPEN_TICK;
		if(gOptions.LockOn > 15) gOptions.LockOn = 5;
		if(gOptions.LockOn < 4)  gOptions.LockOn = 4;
		gLock.open_delay_ms = gOptions.LockOn * 1000 / RUN_TICK;
		gLock.close_ms = LOCK_CLOSE_TICK;			
	//	L3000Debug("\nunlock time is get!!!!!!!\n");	
		ret = 0;
	//		   }
	//		   else	{
	//				gLock.open_delay_ms = gOptions.LockOn * 1000 / RUN_TICK;
	//				gLock.close_ms = LOCK_CLOSE_TICK;				
			   	        
	//		   }
	    }
	}
	return	ret;
}

void	ControlLock(void)
{
	int	lock = 0;
	
	if(gLock.open_ms){
		lock = 1;//unlock
		if(--gLock.open_ms == 0){
			lock = 0;
	//		gLock.cur_state = LOCK_STATE_OPEN;
//			if(gLock.open_delay_ms == 0){
//				gLock.nor_open = LOCK_STATE_OPEN;
		//		SetLockNorStateParam(LOCK_STATE_OPEN);				
//			}
		}
	}
	else	if(gLock.open_delay_ms && gLock.open_ms == 0){		
		if(--gLock.open_delay_ms == 0){
			;
		}
	}
	else	if(gLock.open_delay_ms == 0 && gLock.close_ms){
		lock = 2;//lock 
		if(--gLock.close_ms == 0){
	//		gLock.cur_state = LOCK_STATE_CLOSE;
//			if(gLock.nor_open = LOCK_STATE_OPEN){
//				gLock.nor_open = LOCK_STATE_CLOSE;
		//		SetLockNorStateParam(LOCK_STATE_CLOSE);
//			}
			lock = 0;
		}
	}
	else	;

#if 0
	if(lock == 0)        { L3000_STOP_DOOR();}
	else	if(lock == 1){ L3000_OPEN_DOOR();}
	else	if(lock == 2){ L3000_CLOSE_DOOR();}
#endif
}
/**************************************************************/

/***************************************************************/
void	FinishProgram(void)
{
	while(1){
		L3000_BEEP_ON();
		LCDClear();
		LCDBL_OFF();
		L3000FeedWatchDog();
		L3000_STOP_DOOR();
		L3000_FG_PWR_OFF();	
		L3000_RLED_OFF();
		L3000_GLED_OFF();
		OLED_DisOff();
		L3000_BEEP_OFF();
		DelayMS(5);
		while(1){
			L3000_POWER_OFF();		
		}
	}
}

void	L3000ShowTime(int	two_dots)
{
	char str[50];
	static	int	start = 0;
	static	int	halfs = 0;
	
	extern	int	RtcInvalidFg;

	LCDWriteStr(0,6, "          ", 0);
	if(RtcInvalidFg || gHaveRTC == 0){
		halfs = !halfs;
		if(halfs){
			LCDWriteStr(0,8, "--:--:--", 0);
			return	;
		}
		else	{
			LCDWriteStr(0,8, "-- -- --", 0);
			return	;
			
		}
	}
		
	if(start%10 <= 1){		
		sprintf(str, "%04d-%02d-%02d", gCurTime.tm_year + 1900, 
			gCurTime.tm_mon + 1, gCurTime.tm_mday);

		LCDWriteStr(0, 6, str, 0);
	}
	else{
		sprintf(str, "%02d:%02d:%02d", gCurTime.tm_hour, gCurTime.tm_min, gCurTime.tm_sec);
//	if(two_dots == 0 && gHaveRTC){
//		sprintf(str, "%02d %02d", gCurTime.tm_hour, gCurTime.tm_min, gCurTime.tm_sec);		
//	}
		LCDWriteStr(0, 8,  str, 0);		
	}
	if(++start > 10) start = 0; 
}

void	GLedFlash(int	tick)
{
	if(gRLed.off_ms == 0 
	&& gRLed.on_ms == 0 
	&& gGLed.off_ms == 0
	&& gGLed.on_ms == 0
	&& gMachineState == STA_VERIFYING
	&& !(gState.state & STA_VRY_END)){
		if(tick < 500/RUN_TICK){
			L3000_GLED_ON();
		}
		else {
	  		L3000_GLED_OFF();
		}
	}
}

static	int     GetDoorStatus(void)
{
	
//	if(gOptions.DoorSensorMode == 0) {// normal open mode
		if(L3000SwGroup[SW_SENSOR] >= SWI_CNT_UPLIMIT) return 1;
		else	if(L3000SwGroup[SW_SENSOR] == 0)       return 0;
		else 					       return 0x55; //no sure	
//	}	
/*	else	if(gOptions.DoorSensorMode == 1) {// normal open mode
		if(L3000SwGroup[SW_SENSOR] == 0) return 1;
		else				 return	0;	
	}	
	else	return	1;	//return	1: door closed  0:door opened
*/
}

int	L3000CheckSensor(void)
{	
	int tick = 0;
	if(gOptions.DoorSensorMode > 1) return	1;
	
	if(GetDoorStatus()) return	1;

	for(tick=0; tick<(3000/RUN_TICK); ++tick){
		L3000CheckSwitch(L3000SwGroup);
		DelayMS(RUN_TICK);	
	}

	if(GetDoorStatus()) return	1;
	L3000ShowWarn(NULL, LoadStrByID(MID_PLS_CLOSE_DOOR), 0, 10);
	
	for(tick=0; tick<(3000/RUN_TICK); ++tick){
		L3000CheckSwitch(L3000SwGroup);
		if(GetDoorStatus() == 0) {
			if(tick%(500/RUN_TICK) <= (100/RUN_TICK))   L3000_BEEP_ON();
			else  L3000_BEEP_OFF();
			DelayMS(RUN_TICK);
		}
		else	return	1;	
	}
	return	0;
}

//extern	serial_driver_t *gMFOpened;
void    L3000ProcTickMsg(PMsg msg)
{
	static U32 tick = 0;
//	static	U32 sec = 0;
	char str[20];

	if(++tick >= 1000/RUN_TICK)
	{
	    tick = 0;
	//	sec++;
//	    L3000Debug("time: have a second msg!!!!");
//	{	
//		char str[100];
//		gLock.nor_open = ReadLockNorStateParam();		
//		sprintf(str, "\nlock state: %d\n", gLock.nor_open);	
//		L3000Debug(str);
//	}
//	    sprintf(str, "time:%d", sec);
//	    LCDWriteCenterStr(3, str);
	}

	if(gState.dis_vry_tick) --gState.dis_vry_tick;

//	gUserOperateTime++;

	ControlLock();
	ControlBeepOutput();
	ControlLedOutput();

#if 0
	LCDBL_ON();
	if(gState.state & STA_VRY_END) 
		L3000_USBMFPWR_OFF()
	else	
		L3000_USBMFPWR_ON()

	if(gState.state & (STA_NOR_OPEN + STA_VRY_END + STA_COMM))	
		L3000_FG_PWR_OFF()	
	else				
		L3000_FG_PWR_ON()
#endif

	GLedFlash(tick);
        if(gCartoon.type){
/*		if(gCartoon.type == CARTOON_SMILE){
			if(ShowSmileCartoon(NULL)){
				gLock.nor_open_key = 1;		
				ConstructCartoon(CARTOON_UNLOCK, 1000/RUN_TICK, 3);				
			}
		}*/
		if(gCartoon.type == CARTOON_UNLOCK){
			ShowLockCartoon();
		}
		if(gCartoon.type == CARTOON_LOCK){
			ShowLockCartoon();
		}
		if(gCartoon.type == CARTOON_VRY_VALID_UNLOCK){
			if(ShowVryValidUnlockCartoon()){			
				gLock.nor_open_key = 1;		
				ShowMainLCD();			
			}
		}
	}
	else	if(gState.state & STA_COMM){
		static	int	start = 0;

		if(start == 0){
			start = 1;
			ShowMainLCD();
		}
	}

	else	{
		if(L3000CheckShowProgTime()){
		//	ShowProgress(gUserOperateTime, gUserOperateMaxTime);
			if(gShowLowPwrAlm){
				gShowLowPwrAlm = 0;
				L3000ShowWarn(NULL, LoadStrByID(HID_ALARM_LOWBATTERY), 0, 2);
			}
			else	if(tick == 0 || tick == (500/RUN_TICK)){
				L3000ShowTime(tick);
				ShowBattery(0);
			}			
		}
		if(L3000CheckShowFgTime()){
			if(gShowLowPwrAlm){
				gShowLowPwrAlm = 0;
				L3000ShowWarn(NULL, LoadStrByID(HID_ALARM_LOWBATTERY), 0, 2);
			}
			else	if(tick % (500/RUN_TICK) == (480/RUN_TICK)) {
				if(gOptions.MSAnimation){
					LCD_ClearBar(65,24, 127, 63);
			       		ShowFPAnimate(65, 24);
				}
			/*	else	{
					LCDClearLine(1);
					LCDClearLine(2);
					LCDClearLine(3);
					//LCDWriteCenterStr(2, "KEYLOCK 8800");
					LCDWriteCenterStr(2, DeviceName);
				}*/
			}	
			else	;	
		}
	}

	if(gState.state & STA_VRY_END){ 
		static	int sensor_open = 0;
		if(sensor_open == 0) {
			if(GetDoorStatus() == 0) sensor_open = 1; 
		}
	       if(gLock.open_ms == 0 && gLock.close_ms && gLock.open_delay_ms > 1){
			//if(gOptions.DoorSensorMode <= 1){
				if(sensor_open){	
					if(GetDoorStatus() == 1) {
						gLock.open_delay_ms = 2;
					}
				}
			//}
		}
	       if(gLock.open_ms == 0 && gLock.close_ms && gLock.open_delay_ms == 1){
         		if(gCartoon.type != CARTOON_LOCK){	       	
				ConstructCartoon(CARTOON_LOCK, 1000/RUN_TICK, 3);
				gLock.nor_open_key = 0;
				L3000RunBeep(60, 0, 0);
			}
	       }
	       if(gLock.close_ms  == 0 && gLock.open_delay_ms == 0 && gLock.open_ms == 0){
			if(gOptions.DoorSensorMode <= 1 && gOptions.CloseDoorHint){
				if(sensor_open){	
					L3000CheckSensor();
				}
				L3000Debug("\nwill power off\n");
				FinishProgram();		
			}
			FinishProgram();		
	       }
	}

#ifndef	L3000_TEST
	else	if(gState.state & STA_COMM){
		if(gUserOperateTime >= gUserOperateMaxTime*3){
			FinishProgram();
		}			
	}

/*	else	if(gState.state & STA_DIALOG){
	       ;	
	}*/

	else	if(gMachineState == STA_MENU){
		if(!L3000TestFg && gUserOperateTime >= ((InputTimeOutSec*1000*3)/RUN_TICK)) { //gUserOperateMaxTime * 3)
			FinishProgram();
		}	
	}
	else	if(gMachineState == STA_VERIFYING && gOptions.TestFunOn != 1){
		if(gUserOperateTime >= gUserOperateMaxTime){
			SaveVryFailCnt(VryFailCnt);
			FinishProgram();
		}
	}
#endif
/*	else	if(gUserOperateTime >= gUserOperateMaxTime && !(gState.state & (STA_VRY_END+STA_DIALOG))){
		if(gMachineState == STA_VERIFYING) {
			SaveVryFailCnt(VryFailCnt);			
		       	FinishProgram();
		}
	}*/
	if(gMachineState != STA_VERIFYING && gMachineState != STA_MENU) 
	{
		gUserOperateTime = 0;
	}
}

void	L3000ProcSecondMsg(PMsg msg)
{;
//	gUserOperateTime++;
//	if(gShowMainLCDDelay == 0){
//		ShowProgress(gUserOperateTime, gUserOperateMaxTime);
//		ShowKeyProg();
//	}
}

void	ExVryBind(PVryBind bind, U16 pin, U8 method, U8 fgid)
{	
/*	if(gOptions.VryBind <= 1 ) {
		memset(&bind, 0, sizeof(TVryBind));
		return ;
	}*/
	bind->pin = pin;
	if(method == 1){
		bind->mth = 11;	
	}
	else	if(method == 2){
		bind->mth = 12;
	}
	else	if(fgid < 10) bind->mth = fgid+1;	
	else	fgid = 0;
}

int    L3000CheckVryBind(PVryBind bind)
{
#if 1
	PUser user = FDB_GetUser(bind->pin, NULL);
	if(ISADMIN(user->Privilege)){
		return	1;
	}
	else	if(gOptions.VryBind == 0){
		return	-8;
	}
	else	if(gOptions.VryBind == 1){
		return	0;		
	}
	else	{
		if(bind->vry_cnt == 0) {		
			bind->vry_cnt = 1;	
			bind->old_pin = bind->pin;
			bind->old_mth = bind->mth;
			return	-1;
		}
		else	{
			if(bind->old_pin != bind->pin){
				bind->vry_cnt = 1;	
				bind->old_pin = bind->pin;
				bind->old_mth = bind->mth;
				return	-2;
			}
			else	if(bind->old_mth == bind->mth){
				bind->vry_cnt = 1;	
				bind->old_pin = bind->pin;				
				return	-4;
			}
			else	{
				return 0;
			}
		}		
	}
	return	-0x10;
#endif
}	
	
/***************************************************************/


/*********************************LCD DISPLAY************************************/
#define	PROG_LEN  46
#define	PROG_STARTX  (127 - 46 + 1)

extern	const unsigned char Icon_Finger[];
void	ConstructCartoon(int	type, int tick, int step)
{
	gCartoon.type = type;
	gCartoon.tick = tick;
	gCartoon.step_tick = step;
	gCartoon.cur_tick = 0;
}

int	ShowLockCartoon()
{
	U32     num = 0;
		
	if(gCartoon.cur_tick == 0){
		LCDClear();
	        if(gCartoon.type == CARTOON_UNLOCK) LCDWriteStr(3, 0, L3000_STR_REPLACE13, 0);
		else	LCDWriteCenterStrID(3, HID_THANK);		
	}
	if(gCartoon.cur_tick < gCartoon.tick) {
		if(gCartoon.cur_tick%gCartoon.step_tick == 0){
			num = gCartoon.cur_tick/gCartoon.step_tick;
			if(num < LOCK_CNT){
				LCD_ClearBar(26, 10, 26+64-1, 10+32-1);
		        	if(gCartoon.type == CARTOON_UNLOCK) LCD_OutBMP1Bit(26, 10, LockDot6432[num], 0,0, 64, 32, 1);
				else	LCD_OutBMP1Bit(26, 10, LockDot6432[LOCK_CNT-1-num], 0,0, 64, 32, 1);
			}
		}
	        ++gCartoon.cur_tick;
		LCDInvalid();
		return	0;
	}
	 else    {
 		memset(&gCartoon, 0, sizeof(TCartoon));
		return	1;
         }
}

int	ShowSmileCartoon(PUser pu)
{
	U32     num = 0;
			
	if(gCartoon.cur_tick == 0 && pu != NULL){
		LCDClear();        		
		ShowUserHint(3, FALSE, pu->PIN2);        
	}
	if(gCartoon.cur_tick < gCartoon.tick) {
		if(gCartoon.cur_tick%gCartoon.step_tick == 0){
			num = gCartoon.cur_tick/gCartoon.step_tick;
			if(num < SMILE_CNT){
				LCD_ClearBar(48, 0, 48+32-1, 0+32-1);
       			 	if(gCartoon.type == CARTOON_SMILE) 
					LCD_OutBMP1Bit(48, 0, SmileDot3232[num], 0,0, 32, 32, 1);		
			}
		}
	        ++gCartoon.cur_tick;
		LCDInvalid();
		return	0;
         }
	 else    {
 		memset(&gCartoon, 0, sizeof(TCartoon));
		return	1;
         }
}

int	ShowVryValidUnlockCartoon(PUser pu)
{
	U32     num = 0;

	if(gCartoon.cur_tick == 0 && pu != NULL)
	{
		LCDClear();        		
		ShowUserHint(3, FALSE, pu->PIN2);        
	}

	if(gCartoon.cur_tick < gCartoon.tick) 
	{
		if(gCartoon.cur_tick%gCartoon.step_tick == 0)
		{
			num = gCartoon.cur_tick/gCartoon.step_tick;
			if(num < LOCK_CNT)
			{
				LCD_ClearBar(26, 10, 26+64-1, 10+32);
				LCD_OutBMP1Bit(26,10,LockDot6432[num],0,0,64,32,1);	
			}
		}
		++gCartoon.cur_tick;
		LCDInvalid();
		return	0;
	}
	else    
	{
		memset(&gCartoon, 0, sizeof(TCartoon));
		return	1;
	}
}

/*
void	L3000ShowLock(int	cnt)
{
	if(cnt == 0){
		LCDClear();	
		LCDWriteStr(3, 0, "ESC open", 0);
	}
	if(cnt < LOCK_CNT) {
		LCD_ClearBar(26, 16, 26+64-1, 16+32-1);
		LCD_OutBMP1Bit(26, 16, LockDot6432[cnt], 0,0, 64, 32, 1);
	}
	else	;	
}
*/

void	L3000ProcUnLock(int	method)
{
//	LCD_Clear();
//	if(method == 0)	LCD_OutBMP1Bit(32, 16, UNLockDot6432, 0,0, 64, 32, 1);	
//	LCDInvalid();	
//	DelayMS(1000);
}

void	L3000ShowVrySuccess(PUser pu, int method, char *s)
{
#ifdef L3000_TEST
	return;
#endif
	printf("[L3000ShowVrySuccess][%02d-%02d-%02d %02d:%02d:%02d] %d:%s\n",
       gCurTime.tm_year, gCurTime.tm_mon, gCurTime.tm_mday,
       gCurTime.tm_hour, gCurTime.tm_min, gCurTime.tm_sec,
       pu->PIN, pu->Name);

	
	VryFailCnt = 0;
	SaveVryFailCnt(-1);
	ClkManage(1);
	
	gState.state |= STA_VRY_END;
	L3000RunBeep(500, 0, 0);	
	L3000RunGLed(3000, 0,0);
	L3000RunLock(1);
//	ConstructCartoon(CARTOON_SMILE, 1000/RUN_TICK, 1);	
//	ShowSmileCartoon(pu);
	LCDClear();
	VryPassUser = *pu;
	ConstructCartoon(CARTOON_VRY_VALID_UNLOCK, 1800/RUN_TICK, 4);	
	ShowVryValidUnlockCartoon(pu);
	gState.dis_vry_tick = DIS_VRY_TICK;
}	

void	L3000CheckVryFailCnt(int method)
{	
	static	U8 cnt = 0;
//	static	L3000VryFailCnt = 0;
	if(gMachineState == STA_VERIFYING) {
		
	//	if(method == 1 || method == 2){
		if(method >= 0){
			++VryFailCnt;			
			printf("%s: VryFailWait %d %d %d\n", __func__, 
				gOptions.VryFailWait, VryFailCnt + gVryFailCnt, gOptions.VryFailWaitCnt);
			if(gOptions.VryFailWait && (VryFailCnt+gVryFailCnt) >= gOptions.VryFailWaitCnt) {
				SaveVryFailCnt(VryFailCnt);		
				L3000_FG_PWR_OFF();	
				L3000_GLED_OFF();
				L3000_RLED_ON();			
				L3000ExBeep(200);
				L3000ShowWarn(NULL, L3000_STR_REPLACE6, 2, 0);
				FinishProgram();
			}	
		}
#ifndef	L3000_TEST
		if(++cnt >= 6){
			SaveVryFailCnt(VryFailCnt);		
			L3000_FG_PWR_OFF();	
			L3000_GLED_OFF();
			L3000_RLED_ON();			
			L3000ExBeep(200);
			FinishProgram();			
		}	
#endif
	}	
	else{
		VryFailCnt = 0;
		cnt = 0;
	}
}

void	L3000ShowVerifyInValid(int method)
{
	printf("%s:%d\n", __func__, __LINE__);
	LCDClear();
	L3000RunBeep(120,80, 1);
	L3000RunRLed(3000, 0, 0);
/*
	if(gMachineState == STA_VERIFYING) {
		if(++L3000VryFailCnt >= 3) {
			L3000ExBeep(50);
			L3000ShowWarn(NULL, L3000_STR_REPLACE6, 2, 0);			
			FinishProgram();
		}		
	}
	else	L3000VryFailCnt = 0;
*/
	if(method == 1000){
		L3000ShowForbid(NULL, LoadStrByID(HID_ACCESSDENY), 0, 3);
		return	;
	}
	else if(method == 103)
	{
		L3000ShowForbid(NULL, LoadStrByID(HID_INVALIDTIME), 0, 3);
		return	;
	}
	else if(method == 104)
	{
		L3000ShowForbid(NULL, LoadStrByID(HID_MUSER_OPEN2), 0, 3);
		return	;
	}
	else if(method == 105)
	{
		L3000ShowForbid(NULL, LoadStrByID(HID_MUSER_OPEN1), 0, 3);
		return	;
	}

	if(method == VRY_METHOD_BYFP){
		LCDWriteCenterStrID(3, HID_VFFAIL);	
	}
	else	if(method == VRY_METHOD_BYCARD){
		LCDWriteCenterStrID(3, HID_CARD_NOTENROLLED);	
	}
	else	if(method == VRY_METHOD_BYPWD){
		LCDWriteCenterStrID(3, HID_VPFAIL);	
	}
	else	if(method == 100){
//		L3000RunBeep(100, 60, 3);
		LCDWriteCenterStrID(3, HID_INVALIDCARD);				
	}
	else	if(method == 109){
	//	L3000RunBeep(100, 60, 3);// CARD Invalid
		LCDWriteCenterStrID(3, HID_INVALIDCARD);//HID_WRITE_ERROR);
	}	
	else	if(method == 101){
		LCDWriteCenterStrID(3, HID_UIDERROR);		
	}
	else	if(method == 102){
		LCDWriteCenterStrID(3, HID_PRI_INVALID);		
	}
	else	if(method == 110){
		LCDWriteCenterStr(3, L3000_STR_REPLACE22);		
	}
	else	;
	
	LCD_OutBMP1Bit(48, 10, ErrorDot3232, 0,0,32,32,1);
//	LCD_Circle(63, 16, 16);
	LCDInvalid();
	ShowMainLCDDelay = 3;
	gState.dis_vry_tick = DIS_VRY_TICK;
}

void	L3000ShowFingerTouch(void)
{
	LCDClear();
	//LCD_OutBMP1Bit(40, 10,	Icon_Finger, 0, 0, -1, -1, 0);
	LCDWriteCenterStrID(2, HID_LEAVEFINGER);	
	//LCDInvalid();
//	L3000ExBeep(40);
	gState.dis_vry_tick = DIS_VRY_TICK;
}

void	ShowBattery(int	start)
{
	static	int	flash = 0;

	LCD_ClearBar(0, 0, 31, 15);	
	flash = !flash;
	if(start) flash = 1;
	
	if(gState.voltage == 0){		
		LCD_OutBMP1Bit(0, 2, BatteryDot3209, 0,0, 32, 9, 1);
		return	;	
	}	
	if(gState.voltage == SW_LOW_PWR){
		LCD_OutBMP1Bit(0, 2, BatteryLowDot3209, 0,0,32,9, 1);
		return	;
	}
	
	if(flash){
		if(gState.voltage == SW_LOW_PWR_OFF){
			LCD_OutBMP1Bit(0, 2, BatteryOffDot3209, 0,0,32,9, 1);
		}
		else	if(gState.voltage == SW_LOW_PWR_ALM){
			LCD_OutBMP1Bit(0, 2, BatteryAlmDot3209, 0,0,32,9, 1);			
		}
		else	;
	}	
}

void	ShowProgress(int	time, int max_time)
{
	int	progress_len = 0;
	float   f = 0;

	LCD_ClearBar(PROG_STARTX, 0, PROG_STARTX+PROG_LEN-1, 15);
	if(time > max_time) time = max_time;
	progress_len = time * (PROG_LEN - 4) / max_time;
	if(progress_len > 3){
		LCD_Bar(PROG_STARTX+2, 2+2, PROG_STARTX+2-1+progress_len, 10-2);
		LCD_Bar(PROG_STARTX+2-1+progress_len-3, 0, PROG_STARTX+2-1+progress_len, 12);		
	}
	else	{
		LCD_Bar(PROG_STARTX+2, 0, PROG_STARTX+2+3, 12);
	}
	LCD_Rectangle(PROG_STARTX, 2, PROG_STARTX+PROG_LEN-1 , 10);

}
void	ShowKeyProg()
{
	int	prog;

	if(gUserOperateTime > gUserOperateMaxTime) gUserOperateTime = gUserOperateMaxTime;
	prog = gUserOperateTime*40/gUserOperateMaxTime;
	if(prog){
		LCD_ClearBar(127+1-40, 0, 127+1-40+prog-1, 15);
	}

}

void	ShowKey()
{
	LCD_ClearBar(50, 0, 127, 15);
	LCD_OutBMP1Bit(50, 0, KeyDot4016, 0,0, 40, 16, 1);
}
//extern	TTime gCurTime;
void	L3000ShowMainLcd(int	show_first_line)
{
	char	str[20];
//	LCDClearLine(0);
//	LCDClearLine(1);
//	LCDClearLine(2);
//	LCDClearLine(3);

    if(gState.state == STA_VRY_END)
	{//	LCD_ClearBar(0, 16, 96, 48);
		LCDClearLine(0);
		if(!gCartoon.type){
			if(gOptions.NorOpenOn){
				L3000ShowQuestion(L3000_STR_REPLACE4, LoadStrByID(HID_OKCANCEL), 0, 1000);
			}
			else	{
				LCD_OutBMP1Bit(26, 10, LockDot6432[LOCK_CNT-1], 0,0, 64, 32, 1);
				ShowUserHint(3, FALSE, VryPassUser.PIN2);  
				//	LCDWriteStr(3, 0, LoadStrByID(HID_ESC), 0);				
			}
			return	;
		}
		return	;
	}
		

	else	if(!TestEnabledMsg(MSG_TYPE_BUTTON)
	||(gState.state & STA_COMM)){
		L3000ShowWait();
		return;
	}

	else	if(gState.state ==  STA_NOR_OPEN){				
		LCD_OutBMP1Bit(26, 16, LockDot6432[LOCK_CNT-1], 0,0, 64, 32, 1);
		LCDClearLine(3);
		LCDWriteStr(3, 0, L3000_STR_REPLACE18, 0);
	}
	else	{
		if(gOptions.MSAnimation){			
			LCD_OutBMP1Bit(2, 24, LockDot6432_0, 0,0, 64, 32, 1);	
		}
		else	{
			LCDClearLine(2);
			LCDClearLine(1);
			LCDClearLine(3);
		//	LCDWriteCenterStr(2, "KEYLOCK 8800");
		//	LCDWriteCenterStr(2, DeviceName);
			{
				char s[100], val[100] = "";
				sprintf(s, "Logo%d", gOptions.Language);
				
				if(LoadStr(s,val) == FALSE)	LCDWriteCenterStr(2, "ZKSoftware");
				else	LCDWriteCenterStr(2, val);
				sprintf(s, "Logo2%d", gOptions.Language);

				LoadStr(s, val);
				LCDWriteCenterStr(3, val);
			}
		}
		
	}
       // LCDWriteLine(3, "123456");
//	sprintf(str, "%02d-%02d %02d:%02d\0", gCurTime.tm_mon, gCurTime.tm_mday, gCurTime.tm_hour, gCurTime.tm_min);
//	LCDWriteLine(3, str);
//	ShowProgress(gUserOperateTime, gUserOperateMaxTime);
	if(show_first_line){
//		LCDClearLine(0);
//		ShowProgress(gUserOperateTime, gUserOperateMaxTime);
//		sprintf(str, "%02d:02d", gCurTime.tm_
	//	LCD_OutBMP1Bit(0, 2, BatteryDot3209, 0,0, 32, 9, 1);
		LCDClearLine(0);
		ShowBattery(1);
		L3000ShowTime(1);
	}
//	LCD_Rectangle(PROG_STARTX, 2, PROG_STARTX+PROG_LEN-1 , 10);
//	ShowKey();
//	ShowKeyProg();
//	LCD_Rectangle(0, 2, 63, 10);
//
}

void	L3000ShowCardVry(char * title,  char * content1, char * content2,  int now_dly, int after_dly)
{
	LCD_Clear();
//	if(title)      		LCDWriteStr(0, 0, title, 0);      
	if(content1 != NULL)	LCDWriteStr(2, 5, content1, LCD_HIGH_LIGHT);
	if(content2 != NULL)	LCDWriteCenterStr(3, content2);
	LCD_OutBMP1Bit(48, 0, CardDot3232, 0, 0, 32, 32, 1);
//	LCD_OutBMP1Bit(76, 0,	Icon_Finger, 0, 0, 0x30, 0x16, 0);
	LCDInvalid();
	if(now_dly){
		DelayMS(now_dly * 1000);
		LCDClear();		
	}
	else	ShowMainLCDDelay = after_dly;
	return	;	
}

void	L3000ShowForbid(char *content1, char *content2, int now_dly, int after_dly)
{
	int	i = 6;

	LCD_Clear();
	L3000RunBeep(120,80, 1);
	L3000RunRLed(3000, 0, 0);
	if(content1 != NULL)	LCDWriteCenterStr(2, content1);
	if(content2 != NULL)	LCDWriteCenterStr(3, content2);
	if(content1 != NULL)   i = 0;
	LCD_OutBMP1Bit(48, i,  DisableDot3232, 0, 0, 32, 32, 1);
	LCDInvalid();
	if(now_dly){
		DelayMS(now_dly * 1000);
		LCDClear();		
	}
	else	ShowMainLCDDelay = after_dly;
	return	;
}

void	L3000ShowRight(char *content1, char *content2, int now_dly, int after_dly)
{
//	L3000RunBeep(500, 0, 0);	
	L3000RunGLed(3000, 0,0);
	LCD_Clear();
	if(content1 != NULL)	LCDWriteStr(2, 5, content1, LCD_HIGH_LIGHT);
	if(content2 != NULL)	LCDWriteCenterStr(3, content2);
	LCD_OutBMP1Bit(48, 0, RightDot3232, 0, 0, 32, 32, 1);
	LCDInvalid();
	if(now_dly){
		DelayMS(now_dly * 1000);
		LCDClear();		
	}
	else	ShowMainLCDDelay = after_dly;
	return	;
}

void	L3000ShowInfo(char *content1, char *content2, int now_dly, int after_dly)
{
	int	i = 6;

	LCD_Clear();
	if(content1 != NULL)	LCDWriteCenterStr(2, content1);
	if(content2 != NULL)	LCDWriteCenterStr(3, content2);
	if(content1 != NULL)   i = 0;
	LCD_OutBMP1Bit(48, i, InfoDot3232, 0, 0, 32, 32, 1);
	LCDInvalid();
	if(now_dly){
		DelayMS(now_dly * 1000);
		LCDClear();		
	}
	else	ShowMainLCDDelay = after_dly;
	return	;
}

void    L3000ShowWait2(char *content1, char *content2, int now_dly, int after_dly)
{
	        int     i = 6;

	        LCD_Clear();
	        if(content1 != NULL)    LCDWriteCenterStr(2, content1);
	        if(content2 != NULL)    LCDWriteCenterStr(3, content2);
	        if(content1 != NULL)   i = 0;
	        LCD_OutBMP1Bit(48, i, WaitDot3232, 0, 0, 32, 32, 1);
	        LCDInvalid();
	        if(now_dly){
		        DelayMS(now_dly);// * 1000);
	//	        LCDClear();
		}
		else    ShowMainLCDDelay = after_dly;
		return  ;
}

void	L3000ShowWait()
{
	L3000ShowWait2(NULL, LoadStrByID(HID_WORKING), 500, 0);
}

void	L3000ShowError(char *content1, char *content2, int now_dly, int after_dly)
{
	int	i = 6;

	LCD_Clear();
	if(content1 != NULL)	LCDWriteCenterStr(2, content1);
	if(content2 != NULL)	LCDWriteCenterStr(3, content2);
	if(content1 != NULL)   i = 0;
	LCD_OutBMP1Bit(48, i, ErrorDot3232, 0, 0, 32, 32, 1);
	LCDInvalid();
	if(now_dly){
		DelayMS(now_dly * 1000);
		LCDClear();		
	}
	else	ShowMainLCDDelay = after_dly;
	return	;
}

void	L3000ShowWarn(char *content1, char *content2, int now_dly, int after_dly)
{
	LCD_Clear();
//	if(content1 != NULL)	LCDWriteCenterStr(2, content1);
	if(content2 != NULL)	LCDWriteCenterStr(3, content2);
	LCD_OutBMP1Bit(48, 6, WarnDot3232, 0, 0, 32, 32, 1);
	LCDInvalid();
	if(now_dly){
		L3000ExBeep(100);
		DelayMS(100);
		L3000ExBeep(100);
		DelayMS(100);
		L3000ExBeep(100);
		DelayMS(100);
		L3000ExBeep(100);
		DelayMS(100);
		L3000ExBeep(100);
		DelayMS(100);
		if(now_dly > 1)	DelayMS((now_dly-1) * 1000);
		LCDClear();		
	}
	else	{
		ShowMainLCDDelay = after_dly;
		L3000RunBeep(100,100, 5);
	}
	return	;
}

void	L3000ShowQuestion(char *content1, char *content2, int now_dly, int after_dly)
{
	int	i = 6;
	LCD_Clear();
	if(content1 != NULL)	LCDWriteCenterStr(2, content1);
	if(content2 != NULL)	LCDWriteCenterStr(3, content2);
	if(content1 != NULL) i = 0;
	LCD_OutBMP1Bit(48, i, QuestionDot3232, 0, 0, 32, 32, 1);
	LCDInvalid();
	if(now_dly){
		DelayMS(now_dly * 1000);
		LCDClear();		
	}
	else	ShowMainLCDDelay = after_dly;
	return	;
}

#if 1
void	L3000ShowLock(int	onoff, char *content2, int now_dly, int after_dly)
{
	LCD_Clear();
//	if(content1 != NULL)	LCDWriteCenterStr(2, content1);
	if(content2 != NULL)	LCDWriteCenterStr(3, content2);
	if(onoff == 0)	LCD_OutBMP1Bit(26, 10, LockDot6432, 0,0, 64, 32, 1);	
	else		LCD_OutBMP1Bit(26, 10, LockDot6432[LOCK_CNT-1], 0,0, 64, 32, 1);	
	LCDInvalid();
	if(now_dly){
		DelayMS(now_dly * 1000);
		LCDClear();		
	}
	else	ShowMainLCDDelay = after_dly;
	return	;
}

void	ShowVryFailWait(int	cnt)
{
	char str[50];
	if(cnt == VRYFAIL_WAIT_SEC){
		LCDClear();		
		LCD_OutBMP1Bit(48, 16, WarnDot3232, 0,0, 32, 32, 1);	
	}
	sprintf(str, "%02d", cnt);
	LCDWriteStr(0, 15-2, str, LCD_HIGH_LIGHT);
}

void	L3000ShowPwdKeypad()
{
	 //char str[50];
	 if(L3000PwdMode == 4){
		 LCDWriteStr(2, 0, " 1 ", LCD_HIGH_LIGHT);//             LCDWriteCenter(3, L3000_STR_REPLACE25);
       		 LCDWriteStr(2, 4, " 2 ", LCD_HIGH_LIGHT);//             LCDWriteCenter(3, L3000_STR_REPLACE25);
	         LCDWriteStr(2, 8, " 3 ", LCD_HIGH_LIGHT);//             LCDWriteCenter(3, L3000_STR_REPLACE25);
       		 LCDWriteStr(2, 12, " 4 ", LCD_HIGH_LIGHT);//            LCDWriteCenter(3, L3000_STR_REPLACE25);
        	 LCDWriteStr(3, 5, "  OK  ", LCD_HIGH_LIGHT);
        	 LCD_ClearBar(0, 47, 127, 50);
		 LCD_ClearBar(0, 32, 127, 34);
	 }
	 else	{
		 LCDWriteStr(3, 0, " 1 ", LCD_HIGH_LIGHT);//             LCDWriteCenter(3, L3000_STR_REPLACE25);
       		 LCDWriteStr(3, 4, " 2 ", LCD_HIGH_LIGHT);//             LCDWriteCenter(3, L3000_STR_REPLACE25);
	         LCDWriteStr(3, 8, " 3 ", LCD_HIGH_LIGHT);//             LCDWriteCenter(3, L3000_STR_REPLACE25);
       		 LCDWriteStr(3, 12, " OK ", LCD_HIGH_LIGHT);//            LCDWriteCenter(3, L3000_STR_REPLACE25);
        //	 LCDWriteStr(3, 5, "  OK  ", LCD_HIGH_LIGHT);
//		 LCDClearLine(2);
        	 LCD_ClearBar(0, 48, 128, 50);
	//	 LCD_ClearBar(0, 32, 120, 34);	 	
	 }
}
#endif

void	L3000ShowInputPwd(PInputPwd pwd)
{
	char  str[] = "************";
	if(pwd->bits == 1){
		LCDClear();
		LCDWriteCenterStrID(0,HID_INPUTPWD);
	//	L3000ShowPwdKeypad();
          //      LCDWriteStr(2, 0, " 1 ", LCD_HIGH_LIGHT);//		LCDWriteCenter(3, L3000_STR_REPLACE25);
        //        LCDWriteStr(2, 4, " 2 ", LCD_HIGH_LIGHT);//		LCDWriteCenter(3, L3000_STR_REPLACE25);
            //    LCDWriteStr(2, 8, " 3 ", LCD_HIGH_LIGHT);//		LCDWriteCenter(3, L3000_STR_REPLACE25);
          //      LCDWriteStr(2, 12, " 4 ", LCD_HIGH_LIGHT);//		LCDWriteCenter(3, L3000_STR_REPLACE25);
	//	LCDWriteStr(3, 5, "  OK  ", LCD_HIGH_LIGHT);
		LCDWriteStr(2, 3, "          ", LCD_HIGH_LIGHT);
	//	LCD_Line(104, 16, 104, 31);
	//	LCD_ClearBar(0, 47, 120, 50);
	}
	str[pwd->bits] = '\0';
	LCDWriteStr(2, (16-PWD_LEN_MAX)/2, str, LCD_HIGH_LIGHT);
//	LCD_ClearBar(0, 16, 120, 18);
//	LCD_ClearBar(0, 32, 120, 32);
	LCDInvalid();

}


/*******************************************************************/


/***************************passward****************************************/
int	InputPwd(PInputPwd pwd,  char key)
{	
			
	if(pwd->bits >= PWD_LEN_MAX) return	-1;
	pwd->buf[pwd->bits] = key;
	pwd->bits++;

}
#if 0
PUser	L3000CheckPwdFree(char *pwd)
{
	U32 cnt = 0;
	U32 a;
	TSearchHandle sh;
	PUser pu = NULL;
	char str[5] = {0,0,0,0,0};
	sh.ContentType=FCT_USER;
	a=SearchFirst(&sh);
	
	while(a)
	{
		pu = (PUser)a;
		if(memcmp(pu->Password, str, 5) != 0){
			if(++cnt >= L3000MaxPwdCnt){
				return	(PUser) &L3000MaxPwdCnt;
			}
		}		
		a=SearchNext(&sh);
	}
	return NULL;					
}
#endif

static TUser gUser;
PUser	L3000SearchUserByPwd(char *pwd)
{
#if 1
	U32 cnt = 0;
	TSearchHandle sh;
	PUser pu = NULL;
	char str[5] = {0,0,0,0,0};
	sh.ContentType=FCT_USER;
	sh.buffer=(unsigned char*)&gUser;
	SearchFirst(&sh);

	while(!SearchNext(&sh))
	{
		pu = (PUser)sh.buffer;
		if(memcmp(pu->Password, pwd, 5) == 0){
			return pu;
		}
		if(memcmp(pu->Password, str, 5) != 0){
			if(++cnt >= L3000MaxPwdCnt){
				return	(PUser) &L3000MaxPwdCnt;
			}
		}		
	}
#endif
	return NULL;					
}

int	L3000InputPassward(PInputPwd pwd, char key)
{
	int	ret = 0;

	ret = InputPwd(pwd, key);
	if(pwd->bits){
		L3000ShowInputPwd(pwd);	
	}
	return ret;	
}

int	L3000ProcInputPwd(U16 *pin, PInputPwd pwd)
{
	PUser  u = NULL;
	if(pwd->bits == 0) return -8;	
	else	if(pwd->bits > PWD_LEN_MAX || pwd->bits < PWD_LEN_MIN){
		return	-1;
	}
	else	{
		L3000BoxToPwd(pwd->buf, pwd->pwd);
		u = L3000SearchUserByPwd(pwd->pwd);
		if(u != NULL  && u != &L3000MaxPwdCnt)	{
			*pin = u->PIN;
			return	0;
		}
	}
	return	-4;
}

void	L3000BoxToPwd(char *buf, char *pwd)
{
	int	 i = 0;

	memset(pwd, 0, 5);
	for(i = 0; i<10; i++){
		if(buf[i] > '4' || buf[i] < '0') buf[i] = '0';
	}
	for(i = 1; i < 10; i += 2){
		pwd[i/2] = ((buf[i]-'0')<<4) + (buf[i-1]-'0'); 
	}
}

/****************************************** ********************************/

/****************************************** ********************************/

/****************************************** ********************************/

void	ShowDelFg(char *str1, char *str2, char *str3)
{
	LCDWriteStr(0, 0, str1, 0);
	LCDWriteCenterStr(1, str2);
	LCDWriteCenterStr(3, str3);
}

int L3000DoGetKeyEvent(PMsg msg)
{
	int *pkm = msg->Object;
	 if(MSG_TYPE_TIMER==msg->Message && InputTimeOut>=0)
        {
                msg->Message=0;
                msg->Param1=0;
                if(++InputTimeOut>=InputTimeOutSec)
                {
			*pkm = 0;
                        ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_TimeOut);
                }
                return 1;
        }
        if(msg->Message != MSG_TYPE_BUTTON) return      0;
	*pkm = msg->Param1;
	ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_CancelInput);
	return	1;
}

int	L3000GetKeyEvent(int *event)
{
	int i, ret;
        U32 mm=SelectNewMsgMask(MSG_TYPE_TIMER|MSG_TYPE_BUTTON);
        InputTimeOut=0;
        i = RegMsgProc(L3000DoGetKeyEvent);
        ret=DoMsgProcess(event, News_Exit_Input);
        SelectNewMsgMask(mm);
        UnRegMsgProc(i);
        return ret;
}
/****************************************** ********************************/

int	GetDelFgID(int *fid, U16 uid, int addsub)
{
	U32 i = fid[0];

	if(addsub == 1){
		while(i <= gOptions.MaxUserFingerCount-1){
			if(FDB_GetTmp(uid, i, NULL)){
				 *fid = i;
				 return	1;
			}
			i++;	
		}
	}
	else	if(addsub == -1){
		 while(i>=0){
			if(FDB_GetTmp(uid, i, NULL)){
                                 *fid = i;
				 return	1;
                        }
		 	if(i <= 0) break;
			--i;
                }
	}
	else	if(FDB_GetTmp(uid, i, NULL)) return	1;
	return	0;
}
#if 0
void	ShowSetBox(PInputBox box, int revers)
{
	char str[20];
	char str1[20];
	int	i;

	memcpy((U8 *)&i, (U8 *)box->Text, sizeof(int));
	sprintf(str, "%%0%dd", box->Width);
	sprintf(str1, str, i);
	LCDWriteStr(box->Row, box->Col, str1, revers);
}


int	SetNumMsg(PMsg msg)
{
	PInputBox box;
	int	va = 0, oldva = 0;
//	char str[50];

	box = (PInputBox) msg->Object;
	memcpy((U8 *)&va, (U8 *)box->Text,  sizeof(int)); 
	oldva = va;

//	sprintf(str, "recvice a msg %d  param1: %d \n", msg->Message, msg->Param1);
//	L3000Debug(str);
	if(MSG_TYPE_TIMER==msg->Message && InputTimeOut>=0)
	{
		msg->Message=0;
		msg->Param1=0;
		if(++InputTimeOut>=InputTimeOutSec)
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_TimeOut);
		}
		return 1;
	}
	if(msg->Message != MSG_TYPE_BUTTON) return	0;
	if(msg->Param1 == IKeyUpUp || msg->Param1 == IKeyDownDown){
		if(msg->Param1 == IKeyUpUp){
			ConstructMSG(msg, MSG_TYPE_BUTTON, IKeyUp, 0);
		}
		else	{
			ConstructMSG(msg, MSG_TYPE_BUTTON, IKeyDown, 0);
		}
		return	1;
	}	
	InputTimeOut = 0;
	msg->Message = 0;
	if(msg->Param1 == IKeyUp){
		if(va < box->MaxValue) va++;
		else	va = box->MinValue;
		memcpy((U8 *)box->Text, (U8 *)&va, sizeof(int)); 
		ShowSetBox(box, LCD_HIGH_LIGHT);
	}
	else	if(msg->Param1 == IKeyDown){
		if(va > box->MinValue) va--;
		else	va = box->MaxValue;
		memcpy((U8 *)box->Text, (U8 *)&va, sizeof(int)); 
		ShowSetBox(box, LCD_HIGH_LIGHT);
	}
	else	if(msg->Param1 == IKeyOK){
		memcpy((U8 *)box->Text, (U8 *)&va,  sizeof(int)); 
		ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_CommitInput);
		return 1;			
	}
	else	if(msg->Param1 == IKeyESC){
		memcpy((U8 *)box->Text, (U8 *)&oldva, sizeof(int)); 
		ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_CancelInput);
		return 1;			
	}		

	return	1;
}


int	L3000SetNumberAt(int row,int col, int width, int *number, int minv, int maxv)
{
	int	ret;
	int i;
	PInputBox box;
	
	box=(PInputBox) malloc(sizeof(TInputBox));
	memset((U8*)box, 0, sizeof(TInputBox));
	box->Col = col;
	box->Row = row;
	box->MaxLength = width;
	box->Width = width;
	box->MaxValue = maxv;
	box->MinValue = minv;
	if(*number > maxv || *number < minv){
		*number = minv;
	}
	memcpy((U8 *)box->Text, (U8 *)number, sizeof(int)); 
	i = RegMsgProc(SetNumMsg);
	ShowSetBox(box, LCD_HIGH_LIGHT);
	L3000Debug("will into SetNumMsg7");
	ret=DoMsgProcess(box, News_Exit_Input);	
	ShowSetBox(box, 0);
//	*number = *(int *)box->Text;
	memcpy((U8 *)number, (U8 *)box->Text,  sizeof(int)); 
	UnRegMsgProc(i);
	free(box);
	return ret;	
}
/*************************************************************************************/
void	SetMFCardKeyBox(PInputBox box, int count, U8 va)
{
	U32 col;

	col = count+5;
 	memset(box,0,sizeof(TInputBox));
        box->MaxLength=1;
        box->Row=2;
        box->Col=col;
        //box->PasswordChar='*';
        //box->AutoCommit=TRUE;
        box->MinValue=0;
        box->MaxValue=9;
        box->Width=1;
        box->SelectLength=1;
        box->Style=InputStyle_Number;
        //box->AllowNav=nav;
	//box->Text[0] = va;
	memcpy(box->Text, &va, sizeof(int));
}

void	ShowSetMFCardKey(U8 *k)
{
	char str[20];
	LCD_Clear();
	LCDWriteLineStrID(0,MID_OA_FPKEY);
        LCDWriteCenterStrID(3,HID_OKCANCEL);
	sprintf(str, "%d%d%d%d%d%d", k[0],k[1],k[2],k[3],k[4],k[5]);
	LCDWriteCenterStr(2, str);
}

static	void KeytoInput(U32 key, U8 *input)
{
	input[0] = key/100000;
	key = key%100000;
	input[1] = key/10000;
	key = key%10000;
	input[2] = key/1000;
	key = key%1000;
	input[3] = key/100;
	key = key%100;
	input[4] = key/10;
	key = key%10;
	input[5] = key;
}

static	U32 InputtoKey(U8 *input)
{
	U32 key = 0;
	key += (U32)input[0]*100000;
	key += (U32)input[1]*10000;
	key += (U32)input[2]*1000;
	key += (U32)input[3]*100;
	key += (U32)input[4]*10;
	key += (U32)input[5];
	return	key;
}

int	 L3000SetMFCardKey(U32 *value, int min, int max)
{
	int     ret = 0;
        int     i = 0;
        int     j,k;
       	U32     mm = 0;
	U32     mf_key = *value;
	U8	input[6];

	if(*value > 999999) return	News_ErrorInput;	
        InputTimeOut=0;
        mm=SelectNewMsgMask(MSG_TYPE_BUTTON|MSG_TYPE_TIMER);
	KeytoInput(mf_key, input);
	do{
		U32 va = input[i];
		ShowSetMFCardKey(input);
		//SetMFCardKeyBox(box, i, input[i]);
//		ret = InputNumberAt(2, i+5, 1, &va, 0, 9);
		ret = L3000SetNumberAt(2,i+5, 1, &va, 0, 9);
        	//ret=RunInput(box);
		if(ret == News_CancelInput){
			if(i == 0) break;
			else	i--;
		}
		else	if(ret == News_CommitInput){
			if(va >= 0 && va <= 9) 	input[i] = va;
		//	memcpy(&input[i], box->Text, sizeof(int));
			if(i >= 5) break;
			else	i++;
		}	
		else	;
	}        
	while(ret != News_TimeOut && ret != News_ErrorInput);
	if(ret == News_CommitInput) mf_key = InputtoKey(input);
	*value = mf_key;
        SelectNewMsgMask(mm);
        return ret;
}
/*
void L3000SetTime(TTime *tt)
{
	char buf[10];
	int year;
	if(gHaveRTC==1)
	{
		CalcDays(tt);
		buf[0]=HEX2BCD(tt->tm_sec);
		buf[1]=HEX2BCD(tt->tm_min);
 		buf[2]=HEX2BCD(tt->tm_hour);
		buf[3]=HEX2BCD(tt->tm_mday);
		buf[4]=HEX2BCD(tt->tm_mon);
		buf[5]=(tt->tm_wday+6)%7+1;
		year=tt->tm_year%100;
		buf[6]=HEX2BCD(year);
	//	ExCommand(21,buf,7,5);
		HT1380_WriteTime(buf)
		DelayMS(2500);
  	}
	MachineBaseTime=*tt;
	ResetBaseTime();
}*/

/****************************************** ********************************/
#endif


void pll_init(int	type)
{
#if 0
	register unsigned int cfcr, plcr1;
        int n2FR[33] = { 0, 0, 1, 2, 3, 0, 4, 5, 5, 0, 0, 0, 6, 0, 0, 0,
         		 7, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0,9};
        int div[6] = {1, 3, 3, 3, 3,7}; /* divisors of I:S:P:L:M */

	int  frequency = 0;

	frequency = 180 << CPM_PLCR1_PLL1FD_BIT;   		
	if(type == 1 || type == 2){
		frequency = 41 << CPM_PLCR1_PLL1FD_BIT;   
	}
/*	else	if(type == 2){
		frequency = 24 << CPM_PLCR1_PLL1FD_BIT;   		
	}*/

        cfcr = CPM_CFCR_CKOEN1 |
//           CPM_CFCR_SSI |             /* SSI clock is USB_clk, other 3.6864M */
//           //           CPM_CFCR_LCD |             /* External LCD pixel clock select */
//           //           CPM_CFCR_I2S |             /* I2S clk is 1/2 PLL output */
//           //           CPM_CFCR_UCS |             /* External USB clock select */
//           //           CPM_CFCR_MSC |             /* MSC clock is 24.576Mhz, other 19.1692Mhz */
//           //           CPM_CFCR_UPE |
       CPM_CFCR_CKOEN2;
       cfcr |= (n2FR[div[0]] << CPM_CFCR_IFR_BIT) |
      	       (n2FR[div[1]] << CPM_CFCR_SFR_BIT) |
	       (n2FR[div[2]] << CPM_CFCR_PFR_BIT) |
               (n2FR[div[3]] << CPM_CFCR_LFR_BIT) |
               (n2FR[div[4]] << CPM_CFCR_MFR_BIT) |
               (n2FR[div[5]] << CPM_CFCR_UFR_BIT);
//      plcr1 = (215 << CPM_PLCR1_PLL1FD_BIT) | /* 217 * 1.8432 = 400 */
//      //      plcr1 = (205 << CPM_PLCR1_PLL1FD_BIT) | /* 207 * 1.8432 = 381 */
//      //      plcr1 = (194 << CPM_PLCR1_PLL1FD_BIT) | /* 196 * 1.8432 = 360 */
//      //      plcrl = (188 << CPM_PLCR1_PLL1FD_BIT) | /* 188 * 1.8432 = 350 */
/////////////////              plcr1 = (180 << CPM_PLCR1_PLL1FD_BIT) | /* 182 * 1.8432 = 336 */
//              //      plcr1 = (172 << CPM_PLCR1_PLL1FD_BIT) | /* 174 * 1.8432 = 320 */
//              //      plcr1 = (106 << CPM_PLCR1_PLL1FD_BIT) | /* 108 * 1.8432 = 200 */
//              //      plcr1 = (161 << CPM_PLCR1_PLL1FD_BIT) | /* 163 * 1.8432 = 300 */
//              //      plcr1 = (154 << CPM_PLCR1_PLL1FD_BIT) | /* 156 * 1.8432 = 288 */
              // plcr1 = (41 << CPM_PLCR1_PLL1FD_BIT) |  /* 43 * 1.8432 = 80 */	       
//	       plcr1 = (24 << CPM_PLCR1_PLL1FD_BIT)|   /*40M*/  
	       plcr1 = frequency | 
      	       (4 << CPM_PLCR1_PLL1RD_BIT) |   /* RD=0, NR=2, 1.8432 = 3.6864/2 */
               (0 << CPM_PLCR1_PLL1OD_BIT) |   /* OD=0, NO=1 */
               (0x20 << CPM_PLCR1_PLL1ST_BIT) | /* PLL stable time */
               CPM_PLCR1_PLL1EN;                /* enable PLL */
               /* init PLL */
               REG_CPM_CFCR = cfcr;
               REG_CPM_PLCR1 = plcr1;

		{
			int	i = 5000;
			while(--i);
		}	
#endif
}	       
#if 0
/****************************************** ********************************/

/************************************check unlock log********************************/
extern	int IsPrint;
int     L3000RunBrowseMsg(PMsg msg)
{
	int oldkey,oldPos,oldLeft,i;
	PDataViewer dv=(PDataViewer)msg->Object;
	if(MSG_TYPE_TIMER==msg->Message && InputTimeOut>=0)
	{
		msg->Message=0;
		msg->Param1=0;
		if(++InputTimeOut>=dv->TimeOut)
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_TimeOut);
		}
		return 1;
	}
	else if(!(MSG_TYPE_BUTTON==msg->Message))
		return 0;
	oldkey=msg->Param1;
	msg->Param1=0;
	if(InputTimeOut>=0) InputTimeOut=0;
	oldPos=dv->DataSet.CurrentRec;
	oldLeft=dv->Left;
	i=oldPos;
	IsPrint=FALSE;
	if(IKeyDown==oldkey)
	{
		if(dv->DataSet.CurrentRec < (dv->DataSet.RecordCount-1)){
				dv->DataSet.CurrentRec++;
		}
		else	dv->DataSet.CurrentRec = 0;
		L3000ShowUnlockLogPage(dv);
	}
	else if(IKeyUp==oldkey)
	{
		if(dv->DataSet.CurrentRec > 0){
			dv->DataSet.CurrentRec--;
		}
		else	dv->DataSet.CurrentRec = dv->DataSet.RecordCount-1;
		L3000ShowUnlockLogPage(dv);
	}
	else if(IKeyESC==oldkey)
	{
		ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, 0);
		return 1;
	}
	else	;
	msg->Message=0;
	msg->Param1=0;
	return	1;
}
int     L3000ShowUnlockLogPage(PDataViewer dv)
{
	char str[50];
	PUser pu;
	PAttLog plog;
	TAttLog log;//time_second
	TTime t;
	int	len;	

	LCDClear();
	sprintf(str, "%d--%d", dv->DataSet.RecordCount, dv->DataSet.RecordCount - dv->DataSet.CurrentRec);
//	len = strlen(str);
//	LCDWriteStr(0, 16-len, str, 0);
//	LCDWriteStr(0, 1, UNLOCK_LOG_TOTAL, 0);
	LCDWriteCenterStr(0, str);
	plog = (PAttLog)dv->DataSet.Data;
	log = plog[dv->DataSet.CurrentRec];
	DecodeTime(log.time_second, &t);
	sprintf(str, "%05d", FDB_GetUserPIN2(log.PIN));
	LCDWriteStrID(1, 0, HID_PIN2);
	LCDWriteStr(1, 11, str, 0);//LCD_HIGH_LIGHT);
	sprintf(str,"%04d-%02d-%02d %02d:%02d",t.tm_year, t.tm_mon,t.tm_mday,t.tm_hour, t.tm_min);
	LCDWriteCenterStr(2, str);
//	LCDWriteCenterStrID(3, HID_OKCANCEL);
	LCDWriteStr(3, 0, L3000_STR_REPLACE7, 0);
}
/****************************************************************************************/
/****************************************************************************************/
void	L3000ShowMsgBox(TL3000SelectBox *box)
{
	LCDClear();
	LCDWriteCenterStr(2, box->content);
	LCD_OutBMP1Bit(48, 0, QuestionDot3232, 0, 0, 32, 32, 1);
	if(box->style == 0)
		LCDWriteCenterStrID(3, HID_YESNO);
	else	if(box->style == 1)
		LCDWriteCenterStr(3, L3000_STR_REPLACE9);
	else	if(box->style == 2)
		LCDWriteCenterStr(3, HID_YESNO);//L3000_STR_REPLACE10);
	else	;
	LCDInvalid();
}

int	L3000SelectYN(PMsg msg)
{
	int oldkey;
	int	i;
	TL3000SelectBox *box;
	
	box = (TL3000SelectBox *) msg->Object;
	if(MSG_TYPE_TIMER==msg->Message)// && InputTimeOut>=0)
	{
		L3000Debug("\nprocmsgbox  ++inputtimeout");
		msg->Message=0;
		msg->Param1=0;
		if(++InputTimeOut>=box->timeout)
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_TimeOut);
		}
		return 1;
	}
	else if(!(MSG_TYPE_BUTTON==msg->Message))
		return 0;
	oldkey=msg->Param1;
	msg->Param1=0;
	if(InputTimeOut>=0) {
		InputTimeOut=0;
		L3000Debug("\nprocmsgbox  inputtimeout = 0!");
	}

	if(oldkey == IKeyOK){
		ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_CommitInput);
		return 1;		
	}
	else	if(oldkey == IKeyESC && box->style != 2){
		ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_CancelInput);
		return 1;			
	}
	else;
	msg->Message=0;
	msg->Param1=0;
	return	1;
}

int	L3000ProcMsgBox(char *content, int	style, int timeout)
{
	int	ret = 0;
	int	i;
	int	j,k;
	TL3000SelectBox box;
	U32 mm=SelectNewMsgMask(MSG_TYPE_BUTTON);

	j = gInputNumKey;
	gState.state |= STA_DIALOG;
	gInputNumKey = 0;
	memset(&box, 0, sizeof(TL3000SelectBox));
	box.content = content;
	box.style = style;
	box.timeout = timeout;
	L3000ShowMsgBox(&box);
	InputTimeOut=0;
	i = RegMsgProc(L3000SelectYN);
	L3000Debug("will into SetNumMsg7");
	ret=DoMsgProcess(&box, News_Exit_Input);
	SelectNewMsgMask(mm);	
	UnRegMsgProc(i);
	gInputNumKey = j;
	gState.state &= ~STA_DIALOG;
	return ret;	
}

/******************************watch dog**********************************************************/

void	EnableWDT(void)
{
	//REG_WDT_WTCSR |= 0X10;
	//REG_WDT_WTCNT = 0xffffffff - (15000 * 8 * 10);//(870000*3); //30s 	
}
#endif

void	WriteWDT(U32	ms)
{
	U32  tick = 0;

	tick  = ms * (8*10);//11.0592Mhz	
	//REG_WDT_WTCNT = 0xffffffff - tick; //30s 	
}



/****************************************************************************************/

#if 0
int	L3000TestEsc = 0;
int     L3000Test(PMsg msg)
{
	int oldkey,i;
	if(MSG_TYPE_TIMER==msg->Message && InputTimeOut>=0){
		InputTimeOut = 0;
		msg->Message=0;
		msg->Param1=0;
		if(L3000TestFg){
			if(L3000TestTick == 0) {
				LCDClearLine(1);								
				LCDWriteCenterStr(1, "UnLock...");
				L3000ExLock(1); 
				L3000TestFg = 2;
			}
			else	if(L3000TestTick == 4){
				LCDClearLine(1);								
				LCDWriteCenterStr(1, "Lock...");
				L3000ExLock(0); 
				L3000TestFg = 4;
			}
			if(++L3000TestTick >= 4*2) L3000TestTick = 0;
		}
		if(gState.voltage >= SW_LOW_PWR_ALM)
		{
			if(L3000TestFg != 4) ;//InputTimeOut = InputTimeOutSec - 1;
			else	{FinishProgram();}
			//	ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_TimeOut);
		}
		if(L3000TestEsc && L3000TestFg == 4){		
			L3000TestEsc = 0;	
         		ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, 0);
	        	return 1;
		}
		return 1;
	}
	else if(!(MSG_TYPE_BUTTON==msg->Message))
			return 0;
	oldkey=msg->Param1;
	msg->Param1=0;
	if(InputTimeOut>=0) InputTimeOut=0;
	IsPrint=FALSE;
	if(InputTimeOut>=0) InputTimeOut=0;
	IsPrint=FALSE;
	if(IKeyOK == oldkey){
		if(L3000TestFg == 0) L3000TestFg = 1;	
		LCDClear();
		LCDWriteCenterStr(1, "UnLock");
		LCDWriteCenterStr(3, LoadStrByID(HID_CONTINUEESC));
	}
	else if(IKeyESC==oldkey){
		if(L3000TestFg == 4 || L3000TestFg == 0){
         		ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, 0);
	        	return 1;
		}
		else{
			L3000TestEsc = 1;
		}	
    	}
	else	;
	msg->Message=0;
	msg->Param1=0;
	return	1;	
}


int	DoL3000Test(void *p)
{
	int	ret;
	int i;

	i = RegMsgProc(L3000Test);
	LCDClear();
	LCDWriteCenterStr(1, LoadStrByID(MID_L3000_LT));
	LCDWriteCenterStr(3, LoadStrByID(HID_YESNO));

	InputTimeOut = 0;
	L3000TestTick = 0;
	L3000TestFg = 0;
	L3000TestEsc = 0;
	L3000Debug("will into SetNumMsg7");
	ret=DoMsgProcess(NULL, News_Exit_Input);	
	L3000TestTick = 0;
	L3000TestFg = 0;
	L3000TestEsc = 0;
	UnRegMsgProc(i);
	return ret;	
}


/****************************************************************************************/


/***************************************L6000***************************************************/


/*****************************************************************************************/
/*
typedef struct {
	int BlockCount;
	int MaxBlockSize;
	int BlockSizes[MAX_FLASH_BLOCK_COUNT];
	int FreeStart;
	int FreeCount;
	int OptionStart;
	U32 FontStart;
	int SectorSize;
	int ID;
	int Size;
	int StartAddress;
	FlashSectorEraseFun EraseSector;
	FlashWriteWordFun WriteWord;
	FlashResetFun Reset;
}TFlashInfo, *PFlashInfo;
*/
//////////////////////////////////////
extern	volatile unsigned short *gFlash16; //=NULL;
//extern	PFlashInfo FlashInfo; //=NULL;


#define	ZEM400_USE_START   0xbfec0000
#define	ZEM400_USE_BLOCKS  19
#define FW_USE_BLOCKS      4
#define FONT_USE_BLOCKS    4
#define OPT_USE_OFFSET     (ZEM400_USE_BLOCKS-1)

U32 FlashResetChip(void)
{
	extern volatile unsigned short * gFlash16;
	gFlash16=(volatile unsigned short *)ZEM400_USE_START;
	*((U16 *) 0xbfc00000) = 0xF0;
	return (U32)gFlash16;			
}

void	L6000FlashAddrConfig()
{
#if 0
	int	i = 0;
// configture code
	FlashInfo->Size=0x10000*ZEM400_USE_BLOCKS; // 
	(U32)gFlash16 = ZEM400_USE_START;
	FlashInfo->StartAddress = ZEM400_USE_START; // fw start byte;
	FlashInfo->FontStart=FlashInfo->StartAddress + (0x10000*FW_USE_BLOCKS); // fw used 4 blocks
	FlashInfo->BlockCount = ZEM400_USE_BLOCKS;
	FlashInfo->MaxBlockSize = 64*1024;
	FlashInfo->SectorSize = 64*1024;
	for(i=0; i<FlashInfo->BlockCount; i++) FlashInfo->BlockSizes[i]=FlashInfo->MaxBlockSize;
//////////////////////////////////////
	FlashInfo->FreeStart = FW_USE_BLOCKS+FONT_USE_BLOCKS; //GetFlashSectorIndex(FlashInfo->FontStart)+4; //font user 4 blocks
	FlashInfo->FreeCount = ZEM400_USE_BLOCKS - FlashInfo->FreeStart;//FlashInfo->BlockCount-(GetFlashSectorIndex(FlashInfo->FreeStart - FlashInfo->StartAddress));
	FlashInfo->OptionStart = OPT_USE_OFFSET; //FlashInfo->FreeStart+FlashInfo->FreeCount-1;
/******************************************************************************************/
	DumpFlashInfo();
	L6000Debug("FlashInfo.StartAddress=0x\n", FlashInfo->StartAddress);
	L6000Debug("FlashInfo.Size=0x\n", FlashInfo->Size);
	L6000Debug("FlashInfo.ID=0x\n", FlashInfo->ID);
	L6000Debug("FlashInfo.BlockCount=0x\n", FlashInfo->BlockCount);
	L6000Debug("FlashInfo.OptionStart=0x\n", FlashInfo->OptionStart, GetFlashStartAddress(FlashInfo->OptionStart));
	L6000Debug("FlashInfo.FreeStart=0x\n", FlashInfo->FreeStart, GetFlashStartAddress(FlashInfo->FreeStart));
	L6000Debug("FlashInfo.FreeCount=0x\n", FlashInfo->FreeCount);
	L6000Debug("FlashInfo.FontStart=0x\n", FlashInfo->FontStart);
#endif
}


#define	TEST_INTOLINUX_ADDR  0xbfff0000 //0xbffd0000
#define TEST_INTOLINUX_VALUE 0x9999

#define	SPECIAL_OPT  0XAAAA

extern volatile unsigned short *gFlash16;//=NULL;
U32     GetSpecialOptStartAdd()
{
	return	(TEST_INTOLINUX_ADDR - (U32)gFlash16)/2;
}


typedef void(* funcType)(void);

int ClearIntoLinuxFlag()
{
#if 0
	U32 a = GetSpecialOptStartAdd();
//	return	FlashSaveBuffer(TEST_INTOLINUX_ADDR, &va, 2);
	FlashInfo->EraseSector(a);
#endif
}

#define	OPT_SIZE_MAX  64

int WriteStrOptToZem500(U32 *addr, char *opt, char *str)
{
	char buf[500];
	int i;
	int len = 0;

	memset(buf, 0, sizeof(buf));
	if((strlen(opt) + strlen(str)+6) >= OPT_SIZE_MAX) return	-1;
	sprintf(buf, "%s=%s", opt, str);
	return	WriteOptToZem500(addr, buf);
}

int WriteIntOptToZem500(U32 *addr, char *opt, int data)
{
        char buf[500];
        int i;

	memset(buf, 0, sizeof(buf));
	if(strlen(opt) >= 40) return	-1;
        sprintf(buf, "%s=%d", opt, data);
        return	WriteOptToZem500(addr, buf);
}

int WriteOptToZem500(U32 *addr, char *opt)
{
	return 0;
#if 0
	int i = 0;
	for(i=0; i<(OPT_SIZE_MAX/2); i++){
                FlashInfo->WriteWord(*addr+i, ((short *)opt)[i]);
        }
	*addr += (OPT_SIZE_MAX/2);
	return	;
#endif
}

int SetIntoLinuxFlag()
{
#if 0
	short buf[1000];
	//U16 va = TEST_INTOLINUX_VALUE;
	U32 a;
	U32 b;

	a = GetSpecialOptStartAdd();
	buf[0] = SPECIAL_OPT;        
	buf[1] = TEST_INTOLINUX_VALUE; //into linux flag
	buf[2] = gOptions.Language; 
	FlashInfo->WriteWord(a, buf[0]);	
	FlashInfo->WriteWord(a+1, buf[1]);	
	FlashInfo->WriteWord(a+2, buf[2]);
	FlashInfo->WriteWord(a+3, 5); // write 3 option to zem500 flash;
	b = a+32;
	WriteIntOptToZem500(&b, "DeviceID", gOptions.DeviceID); 
	WriteStrOptToZem500(&b, "~SerialNumber", SerialNumber);	
	WriteStrOptToZem500(&b, "~DeviceName", DeviceName);
	WriteIntOptToZem500(&b, "OLEDCT", gOptions.OledContrast);// 
	WriteIntOptToZem500(&b, "OLEDType", gOptions.OLEDType);// 
#endif
	return	0;	
}

#define	SYSTEM_CHG   90

void SystemChange(void)
{
	__gpio_as_output(SYSTEM_CHG);
	__gpio_clear_pin(SYSTEM_CHG);
	while(1){
		__gpio_as_output(SYSTEM_CHG);
		__gpio_clear_pin(SYSTEM_CHG);
//		DelayMS(50);
	}
}


void	L6000EnterLinux()
{
	L3000ShowWait2(NULL,  LoadStrByID(HID_WAITING), 0, 10);
	ClearIntoLinuxFlag();
	SetIntoLinuxFlag();
	pll_init(0);	
	DelayMS(600);
	SystemChange();	
	return;	
}

extern int UpdateSettings(char *data);
int CheckUpdOpt(void)
{
	return 0;
#if 0
	char opts[5000];
	char option[200]; 
	char name[200];
	char va[200]; // same size
	U32 a;
	U32 b;
	U16 *ps;
	char *pc;
	int i = 0;
	int len = 0;
	char *pchar;
	int sign = -1;
	int opts_len = 0;

	L3000Dbg("\nget upd opt\n");

	if( UpdateSettings((char *)TEST_INTOLINUX_ADDR) )
	{
		a = GetSpecialOptStartAdd();
		FlashInfo->WriteWord(a, 0);
		FlashInfo->WriteWord(a+1, 0);
		return 1;
	}

	memset(opts, 0, sizeof(opts));
	a = TEST_INTOLINUX_ADDR;	
	ps = (unsigned short *)a;
	pc = (char *)a;
	opts_len = ps[1];
	if(ps[0] == SPECIAL_OPT+1) {
		if(opts_len > 0 && opts_len < sizeof(opts)){
			memcpy(opts, pc+8, opts_len+1);
		}	
		else	return	-1;
		
	}
	else	return	0;
	a = GetSpecialOptStartAdd();
	FlashInfo->WriteWord(a, 0);
	FlashInfo->WriteWord(a+1, 0);
	opts_len++; 
	opts[opts_len-1] = 0;
	//L3000Dbg(opts);	
	i = 0;
	pc = NULL;
	pchar = opts;
	while(pchar < opts_len+opts){
		pc = strchr(pchar, '\n');
		if(pc){
			char *p = 0;
			char * temp = 0;
			*pc = 0;
			len = strlen(pchar);		
			if(len >= sizeof(option) || len < 0) return	-1;
			temp = strchr(pchar, '=');
			if(len < 3 || temp == 0 || temp <= pchar || temp >= (pchar+len-1))   {
				pchar = pc+1;
				break;	
			}
			{
				int m = 0;
				memset(name, 0, sizeof(name));
				memset(va, 0, sizeof(va));
				while(m<len){
					if(pchar[m] == '=') {
						strcpy(va, &pchar[m+1]);
						{
							int k = strlen(va);
							printf("va_end:%d--%c", va[k-1], va[k-1]);
							if(va[k-1] == '\n') va[k-1] = 0;
							if(va[k-1] == 13) va[k-1] = 0;
							

		//					char s[1000];
		//					sprintf(s, "\nopt save: *%s--%s*-\n", name, va);
		//					L3000Dbg(s);
						}
						
					//	printf("\nname_len%d:\n", strlen(name));
					//	printf("\nva_len%d:\n", strlen(va));
					//	printf("\n-%s-\n",name);
					//	printf("\n-%s-\n",va);
	/*					strcpy(name, "LockOn");
						strcpy(va, "13");
	strcpy(name, "LockOn");
        strcpy(va, "11");
        SaveStr(name, va, FALSE);
        strcpy(name, "DeviceID");
        strcpy(va, "11");
        SaveStr(name, va, FALSE);
        strcpy(name, "~DeviceName");
        strcpy(va, "lock lvzhi");
        SaveStr(name, va, FALSE);
	return	1;
*/
        				//	SaveStr(name, va, FALSE); 
					//	sign = 1;
						if(SaveStr(name, va, FALSE)) sign = 1;
						else	{
							return	-1;
						}
						break;
					}
					else	{
						name[m] = pchar[m];
					}
					++m; 
				}
			}
			pchar = pc+1;
		}
		else	return	sign;
	}
	return	sign;
#endif
}
#endif
