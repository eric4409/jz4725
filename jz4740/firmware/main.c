/************************************************
  
 ZEM 200 
 
 main.c Main source file                             
 
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
 
 $Log: main.c,v $
 Revision 5.24  2006/03/04 17:30:09  david
 Add multi-language function

 Revision 5.23  2005/12/22 08:54:23  david
 Add workcode and PIN2 support

 Revision 5.22  2005/11/06 02:41:34  david
 Fixed RTC Bug(Synchronize time per hour)

 Revision 5.21  2005/09/19 10:01:59  david
 Add AuthServer Function

 Revision 5.20  2005/08/18 07:16:56  david
 Fixed firmware update flash error

 Revision 5.19  2005/08/15 13:00:22  david
 Fixed some Minor Bugs

 Revision 5.18  2005/08/13 13:26:14  david
 Fixed some minor bugs and Modify schedule bell

 Revision 5.17  2005/08/07 08:13:15  david
 Modfiy Red&Green LED and Beep

 Revision 5.16  2005/08/04 15:42:53  david
 Add Wiegand 26 Output&Fixed some minor bug


 Revision 5.15  2005/08/02 16:07:51  david
 Add Mifare function&Duress function

 Revision 5.14  2005/07/14 16:59:53  david
 Add update firmware by SDK and U-disk

 Revision 5.13  2005/07/07 08:09:02  david
 Fixed AuthServer&Add remote register

 Revision 5.12  2005/06/29 20:21:43  david
 Add MultiAuthServer Support

 Revision 5.11  2005/06/16 23:27:51  david
 Add AuthServer function

 Revision 5.10  2005/06/10 17:11:01  david
 support tcp connection

 Revision 5.9  2005/06/02 20:11:12  david
 Fixed SMS bugs and Add Power Button Control function

 Revision 5.8  2005/05/20 23:41:04  david
 Add USB support for SMS

 Revision 5.7  2005/05/13 23:19:32  david
 Fixed some minor bugs

 Revision 5.6  2005/04/27 00:15:37  david

 Fixed Some Bugs

 Revision 5.5  2005/04/24 11:11:26  david
 Add advanced access control function

 Revision 5.4  2005/04/21 16:46:44  david
 Modify for HID Card

 Revision 5.3  2005/04/07 17:01:45
 Modify to support A&C and 2 row LCD

 Revision 5.2  2005/04/05 21:38:11  david
 Add Support to update firmware by USB Flash Disk
 
 Revision 5.1  2005/03/17 18:48:00  david
 Add CVS Log KeyWord
 
*************************************************/
#include <stdlib.h>
#include <string.h>
#include "arca.h"
#include "serial.h"
#include "exfun.h"
#include "msg.h"
#include "lcdmenu.h"
#include "flashdb.h"
#include "finger.h"
#include "zkfp.h"
#include "zkfp10.h"
#include "utils.h"
#include "options.h"
#include "mainmenu.h"
#include "zlg500b.h"
#include "commu.h"
#include "accdef.h"
#include "tempdb.h"
#include "rtc.h"
#include "main.h"
#include "lcm.h"
#include "sensor.h"
#include "kb.h"
#include "rs232comm.h"
#include "wavmain.h"
#include "exvoice.h"
#include "sensor.h"
#include "wiegand.h"
#include "accapi.h"
#include <ucos_ii.h>
#include "L3000Operate.h"
#include "exfun.h"

#define CPUHISPEEDDELAY             20
#define CHECK_C2_INTERVAL   10  //在500毫秒的事件中执行，其两次事件做一次算

#define DAYLIGHTSAVINGTIME      1           //夏令时
#define STANDARDTIME    2                   //非夏令时
#define COUNT_RETRY_ADMIN           3       //重新验证身份的重试次数

#define LOG_VALID	0
#define LOG_REPEAT	1
#define LOG_INVALIDTIME	2
#define LOG_INVALIDCOMBO	4
#define LOG_INVALIDUSER	8
#define LOG_UNSAVED	16

#define LOG_WAITCOMBO	32		//多用户验证
#define LOG_NOSPACE		64		//空间已满
#define LOG_ALARMLEFTSPACE	128		//记录警告
#define LOG_MUSTCHOICEINOUT	256		//必须选择出入状态
#define LOG_ANTIPASSBACK		512		//反潜
#define LOG_NOREGISTEREDUSER	1024	//未注册用户
#define LOG_NOTINCONTROLTIME	2048	//不在有效的控制时间内

#define ANTIPASSBACK_OUT	1
#define ANTIPASSBACK_IN		2
#define ANTIPASSBACK_INOUT	3
#define	STATE_IN		0
#define STATE_OUT		1

static	int	 IsVryMF = 0;
static	int	 MfVryCnt = 0;
static	int	 IsVryFgID = 0xf0;

extern int gHaveRTC;
extern int gExternalRTC;
int gMachineState=STA_IDLE;
int WaitAdminVerifyCount=0; //等待进入管理功能可尝试验证的次数，为零时表示不在管理状态
//static int WaitAdminVerifySecond=0; //等待即如管理功能超时时间，为零时表示
//int WaitAdminRemainCnt=1;   //等待进入管理功能需要验证的管理员数
int SpeedHiDelay=CPUHISPEEDDELAY;
//int gShowMainLCDDelay=0;        //等待延迟显示主界面时间
//U32 gShowMainLCDDelayStart=0;   //开始延迟显示主界面时间
//int SecondPos=14;
int FlashGreenLED=0;     
int gLockDelay = 0;
int gWaitSlaveEndIdle=0;    //等待自动结束Idle的时间，防止CMD_CANCELCAPTURE导致的Idle问题
int gLockForceAction=0; //锁强制动作
int gLockForceState=0;      //当前锁状态
int gMasterConnected=FALSE;    
int gFlashLED=0;    //FlashLED快闪时间
int gPowerState=BATTERY_External;
int gContinuousVerifyFailed=0;	//连续失败次数累计
static int IsOpenDoorAction=0;	//本次验证是否有开锁动作
int	gMasterSlaveProcess=FALSE;	//主从机处理
PAttLog lastlogs;
int LastLogsCount=0;

#define TIMEOUT_POWER_OFF		3	//电源按钮按下后等待延迟的时间
#define TIMEOUT_INPUT_PIN		(gOptions.TimeOutPin)	//等待用户输入验证考勤号码的时间
//#define TIMEOUT_WAIT_MAINLCD		3	//显示验证成功或失败信息后延迟显示主见面的时间
#define TIMEOUT_SHOWSTATE		(gOptions.TimeOutState)	//显示临时考勤状态时间
#define MaxAdminVerify 			10
#define DELAYNEXTUSER     		30 	//多人验证时等待下一个用户验证的时间
#define TIMEOUT_WAIT_ADMIN_VERIFY       30      //等待管理员验证身份的超时时间

//2006.10.16
#define WorkCode_Base 1
#define WorkCode_ADV 2

PRTLogNode rtloglist;
PRTLogNode rtlogheader;
#define MAXRTLOGSCACHE 32       //max count for real time logs
int gRTLogListCount=0;

int MainProcMsg(PMsg msg);

int gEthOpened=FALSE;
int gMFOpened=FALSE;
int gHIDiClassOpened=FALSE;
void ShowMainLCD(void);

int ShowMainLCDDelay=0;		//等待延迟显示主界面时间

static char KeyBuffer[20];	//当前输入的考勤号码——（密码验证）
int  KeyBufferIndex=0;		//当前输入考勤号码位置
static char KeyBufferTimeOut=0;	//当前输入考勤号码等待超时时间

int WaitInitSensorCount=0;	//Wait n seconds and then init sensor
int WaitSleepCount=0;		//Wait n seconds and then sleep

int WaitAdminRemainCnt=0;       //等待进入管理功能需要验证的管理员数
int WaitAdmins[MaxAdminVerify]={0};
int gLocalCorrectionImage=FALSE;
static int WaitAdminVerifySecond=0; //等待管理功能超时时间

int HackerNumber=0, HackerWait=5;
int WaitShowState=0;
int WaitPowerOff=0;		//按了电源键后等待关机的时间
PUser AdminUser;		//当前进入管理功能的用户
TUser gAdminUser;
int PowerSuspend=FALSE;
int TestOpenLock(int UID, TTime t, int VerifyMethod);

int PrepareSecondFun=0;
int SecondFunction=0;
int SecondPos=14;
int FlashRedLED=0;

#define ALARMSTRIPTAG 0x10000
int gAlarmStrip=0;  //拆机报警已经起动>=ALARMSTRIPTAG
//int gAlarmDelayIndex=0;
//int gAlarmDelay=0;
int gAuxOutDelay=0;
int gDoorSensorDelay=0;
int gCloseDoorDelay=0;

int WaitDuressAlarm=0;          //胁迫报警发生后延迟产生报警信号的时间
int DelayTriggerDuress=0;       //按键触发胁迫报警的有效时间

int DelayNextUser=0;

int DelayScheduleBell=0; 
int ScheduleBellInterval=0;

int AuthServerDNSListInterval=0; // seconds
int AuthServerUploadAttlogInterval=0; //seconds
int AuthServerRefreshUserData=0; //seconds
int SyncTmFromAuthServerInterval=0;//seconds

int gSetGatewayWaitCount=0;

int g_slaveormain=0;

BOOL RTCTimeValidSign=TRUE;

PAlarmRec CurAlarmRec=NULL;

//Filter 1:H 1:G
U32 gInputPin=0;
U32 g1ToG=FALSE;
PFilterRec gFilterBuf=NULL;

int gErrTimes=0; //press finger 5 times trigger alarm
int gErrTimesCLearDelayCnt=0;

//Authentication Modes(Verify Type)
#define WAIT_VERIFY_IDLE_TIME  10
#define COUNT_RETRY_USER	3 //重新验证身份的重试次数

#define NidekaTimeOut 30
int gNidekaTimeOut=0;

int WaitVerifyTypeIdleTime=0;
int WaitVerifyRetryCnt=COUNT_RETRY_USER;

static BYTE FingerTemplate[10240];
TFPCardOP FPData={0,0,{0,0,0,0},FingerTemplate,OP_READ};
extern int CommSessionCount;
extern int gFPDirectProc;
static TFPResult AuthFPData={0,"",""};
static TVSStatus VSStatus={FALSE,FALSE,FALSE,FALSE};

BYTE gSensorNo=0;

static time_t LastTime=-1;
static int LastUID=-1;
//workcode
static int LastWorkCode=0;
int gWaitworkcodeTimeOut=0;
//cardno
U32 LastCardNo=0;
int gIgnoreCardTm=0;
int gIgnoreCardTimeOut=0;

//C2 2006.10.23
serial_driver_t *gSlave232=NULL;
U16 gBindID=0;
U16 WaitCheckC2=1;
int C2connect=0;
void ShowTestFinger(void);
extern TSensorBufInfo SensorBufInfo;

extern int fd_sensor;
extern char DebugMode;

void AppendRTLog(int EventType,char *Data, int Len)
{
}


void DoAlarmByErrTime(void)
{

	if(gOptions.ErrTimes)
	{
		gErrTimesCLearDelayCnt=gOptions.ErrTimes*15; //seconds=times*15
		gErrTimes++;
		if(gErrTimes>=gOptions.ErrTimes)
                        DoAlarm(0, 24*60*60);
	}
}

int GetNowState(void)
{
	int State[4]={HID_SCIN,HID_SCOUT,HID_SOCIN,HID_SOCOUT};
	int i, res=0;
	int dmin=24*60+1, nowmin=gCurTime.tm_min+gCurTime.tm_hour*60;
	for(i=0;i<4*4;i++)
	{
		int min=60*((gOptions.AutoState[i] >> 8) & 0xFF)+(gOptions.AutoState[i] & 0xFF);
		if(!((gOptions.AutoState[i] & 0xFF)>59))
		{
			min=nowmin<min?24*60+nowmin-min:nowmin-min;
			if(min<dmin)
			{
				res=State[i/4]-HID_SCIN;
				dmin=min;
			}
		}
	}
	return res;
}

extern int GetOptionNameAndValue(char *p, int size);
int fwmain(void)
{
	int NewLng;
	int keyval=0;

	DBPRINTF("Starting....\n");

	if(!GPIO_IO_Init()) DBPRINTF("GPIO Initialize Error!\n");
	llock_gpio_init();
	//指纹图像缓冲区
	gImageBuffer=NULL;
	//指纹算法内存句柄
	fhdl=0;
	//是否显示时钟":"
	ClockEnabled = TRUE;
	ShowMainLCDEnabled = TRUE;	

	/* Initialise yaffs file system */
	yaffs_StartUp();

	/* partition /mnt  is for store options.cfg LANGUAGE.*  *.wav ffiso.dat etc */
        yaffs_mount("/mnt");	
	/* make sure directory /mnt/mtdblock is existent on  yaffs partition /mnt  before starting device*/
//	yaffs_mkdir("/mnt/mtdblock", 0);

	//初始化全局参数
	InitOptions();

	/* Init Key */
	KeyInit(gOptions.DevID);

	//初始化LCDa
	if(!ExLCDOpen())
	{
		DBPRINTF("LCD Open Error!\n");
		ExLCDClose();	
	}

	//setting font and language
	DBPRINTF("Starting ... FONT\n");
	NewLng=LoadInteger("NewLng", gOptions.Language);
	if(NewLng!=gOptions.Language)
	{		
		if(!NewLng)
			NewLng=gOptions.Language;
		else
			gOptions.Language=NewLng;
		SaveInteger("Language", gOptions.Language);
		SaveInteger("NewLng", gOptions.Language);
	}
	SelectLanguage(gOptions.Language);
	NewLng=GetDefaultLocaleID();
	if(!SetDefaultLanguage(NewLng, gRowHeight))
	{
		//如果该语言的字库不存在，则切换到英文
		gOptions.Language=LanguageEnglish;
		SaveInteger("Language", gOptions.Language);
		SaveInteger("NewLng", gOptions.Language);
		SelectLanguage(gOptions.Language);
		SetDefaultLanguage(LID_ENGLISH, gRowHeight);
	}

	if(gLCDHeight<64)
		gLCDCharWidth=gLCDWidth/gLangDriver->CharWidth;

	/* Formating data directly */
//	Format_Data(DIRECTLY);
	/* Formating data if it's allowable */
	Format_Data(INQUIRING);

	/* partition /flash is for store template.dat user.dat transaction.dat etc */
        yaffs_mount("/flash");
	
	/* Display the content of flash partition in  debug mode */
	if(DebugMode)
	{
		ListDir("/mnt");
		ListDir("/mnt/mtdblock");
		ListDir("/flash");
	}

	//Init Sensor	     
	if (!gOptions.IsOnlyRFMachine)
	{
		//初始化SENSOR
		InitSensor(gOptions.OLeftLine,
				gOptions.OTopLine,
				gOptions.OImageWidth,
				gOptions.OImageHeight,
				gOptions.NewFPReader
				);

		SaveOptions(&gOptions);
		gImageBuffer=(char *)malloc(gOptions.OImageWidth*gOptions.OImageHeight*2+128*64*2);
	}
	
	//Synchronize Linux system time from RTC clock
	RTCTimeValidSign=ReadRTCClockToSyncSys(&gCurTime);
	DBPRINTF("Synchronize system time from RTC\n");

	//初始化RS232 COM1
	if(ff232.init(gOptions.RS232BaudRate, V10_8BIT, V10_NONE, 0)!=0)
	{
		printf("Initialise RS232 failed\n");
	}
	
	if(gOptions.RS485On)
	{
		GPIO_SWITCH_RS232_RS485(RS485_EN);
		RS485_setmode(FALSE);
	}
	else
		GPIO_SWITCH_RS232_RS485(RS232_EN);

	//自动关机检测
	if(gOptions.PowerMngFunOn)
		WaitSleepCount=gOptions.IdleMinute*60;
	//空闲时间到后, 关机 = 87 或者 休眠 = 88
	PowerSuspend=(gOptions.IdlePower==HID_SUSPEND);  
	EnableMsgType(MSG_TYPE_TIMER, 1);
	EnableMsgType(MSG_TYPE_BUTTON, 1); 
	EnableMsgType(MSG_TYPE_FINGER, 1); 
	EnableMsgType(MSG_TYPE_MF, 0);
	EnableMsgType(MSG_TYPE_HID, 0);
	EnableMsgType(MSG_TYPE_DOOR, 0);
	if (gOptions.Nideka)
	{
		EnableMsgType(MSG_TYPE_FINGER, 0); 
		EnableMsgType(MSG_TYPE_HID, 0);
	}

	//Alarm for user punch when repeat
	CurAlarmRec=(PAlarmRec)malloc(gOptions.MaxUserCount*100*sizeof(TAlarmRec));
	memset(CurAlarmRec, 0, gOptions.MaxUserCount*100*sizeof(TAlarmRec));
/*
	//Filter buf
	gFilterBuf=malloc(gOptions.MaxUserCount*100*sizeof(TFilterRec));
	memset(gFilterBuf, 0, gOptions.MaxUserCount*100*sizeof(TFilterRec));
*/
	if(gOptions.ShowState || gOptions.ShowCheckIn)
	{
		gOptions.AttState=GetNowState();
	}
	if(gOptions.MustChoiceInOut) gOptions.AttState=-1;
	
	DBPRINTF("FP Engine initializing....\n");
	//初始化指纹系统和模版
	FDB_InitDBs(TRUE);
	//指纹超过限定的数量时强制为1:1验证方式  2006.11.30
        if ((FDB_CntTmp()>gOptions.LimitFpCount) && (!gOptions.I1ToG))
        {
                SaveInteger("Must1To1",1);
		gOptions.Must1To1 = 1;
        }

	if (!gOptions.IsOnlyRFMachine) 
	{
		if (gOptions.ZKFPVersion == ZKFPV10)
			FPInit(NULL);
		else
			FPInit(malloc(FINGER_CACHE_SIZE));
	}
	DBPRINTF("Finish Fp Engine init\n");

	//update compressed log data if the extend log format is true
	//UpdateAttLog();
	//printf("finished update attlog\n");
	FDB_AddOPLog(0, OP_POWER_ON, 0,0,0,0);

	//setting mixer
	SetAudioVol(gOptions.AudioVol);
				
	if(gOptions.IsModule)
		gMachineState=STA_IDLE;
	else
		gMachineState=STA_VERIFYING;
	
	//Calcuate delay based value
	CalibrationTimeBaseValue();
	
	//Initialize Alarm relay state
	//ExAlarmOff(0);
//	DoAlarmOff(0);

	/* Loading Sound files from nand flash to memory */
	//LoadSound();

	/* Update current time from system */
	GetTime(&gCurTime);

	LCDClear();

	if(gOptions.IsTestMachine)
	{
		ShowTestFinger();
	}
	else
	{
		DBPRINTF("STARTING MAIN DISPLAY...\n");
		ShowMainLCD();
		InitKeyBuffer(); 
	}

	L3000OperateInit();
	JudgeVryFailWait();
	Battery_Init();
	FirstCheckPower();

	InitializeUDC();
//	wdt_enable(TRUE);
	//llock_test();
	RegMsgProc(MainProcMsg);
	DoMsgProcess(NULL, News_Exit_Program);

	wdt_enable(FALSE);
	FreeSound();
	free(CurAlarmRec);
	if (gFilterBuf)
		free(gFilterBuf);	

	//关闭RS232
	ff232.sfree();
	//关闭LCD
	LCDClear();
	
	FDB_AddOPLog(0, OP_POWER_OFF, 0,0,0,0);                                                                                                            
	if (!gOptions.IsOnlyRFMachine)
	{
		FPFree();
		free(gImageBuffer);
	}

	ExPowerOff(FALSE);
	
	return 0;
}

void ExSetPowerSleepTime(int IdleMinute)
{
	if(gOptions.PowerMngFunOn)
		WaitSleepCount=IdleMinute*60;
}

void EnableDevice(int Enabled)
{
	static int DeviceEnabled=TRUE;
	
	if(DeviceEnabled!=Enabled) //确保成对执行Enable/Disable
	{
		if(!Enabled || !WaitAdminRemainCnt) 
		{
			EnableMsgType(MSG_TYPE_FINGER, Enabled);
			EnableMsgType(MSG_TYPE_MF, Enabled);
			EnableMsgType(MSG_TYPE_HID, Enabled);
		}
		EnableMsgType(MSG_TYPE_BUTTON, Enabled);
		ShowMainLCD();
		// //add by cn 2009-03-22
		EnableIRQ_Com(Enabled);

		DeviceEnabled=Enabled;
		wdt_enable(Enabled);
	}
	FlushSensorBuffer();
}

//--------------------------Show Main LCD Screen--------------------------
void GetNumberChar(char *line1, char *line2, int Number)
{
	*line1=(char)254;
	line1[1]=(Number*2+161);
	*line2=(char)254;
	line2[1]=(Number*2+162);
}

void GetTimePixel(char *line1, char *line2, int Hour, int Minute)
{
	memset(line1, 32, gLCDCharWidth);
	memset(line2, 32, gLCDCharWidth);
	GetNumberChar(line1+3, line2+3, Hour/10); 
	GetNumberChar(line1+5, line2+5, Hour%10);
	line1[7]=32;line1[8]=32;line2[7]=32;line2[8]=32;//冒号Position
	GetNumberChar(line1+9, line2+9, Minute/10);
	GetNumberChar(line1+11, line2+11, Minute%10);
}

char *GetAttName(int attstate)
{
	switch(attstate)
	{
	case 0:
		return LoadStrByID(HID_SCIN);	
	case 1:
		return LoadStrByID(HID_SCOUT);	
//2006.10.08

	case 2:
		return LoadStrByID(HID_SOCIN);	
	case 3:
		return LoadStrByID(HID_SOCOUT);	
	case 4:
		return LoadStrByID(HID_SOUT);	
	case 5:
		return LoadStrByID(HID_SBACK);

//2006.08.29
/*

	case 2:
		return LoadStrByID(HID_SOUT);	
	case 3:
		return LoadStrByID(HID_SBACK);	
	case 4:
		return LoadStrByID(HID_SOCIN);	
	case 5:
		return LoadStrByID(HID_SOCOUT);
*/
	default:
		return NULL;
	}
}

void DrawNum2(int x, int y, int dig, int width)
{
        int x2=x+width;
        if(gLangDriver && gLangDriver->RightToLeft)
        {
                x+=width;
                x2=x-width;
        }
        LCD_OutBMP1Bit(x, y, numBmpData, width*(dig/10), 0, width, -1, 0);
        LCD_OutBMP1Bit(x2, y, numBmpData, width*(dig%10), 0, width, -1, 0);
}

int FirstLine=0;
void ShowMainLCD(void)
{
//	static	int	once = 0;
	int	show_first_line = 0;
	char tmp1[MAX_CHAR_WIDTH], tmp2[MAX_CHAR_WIDTH];
	int i;
	//printf("%s:%d[%d]\n", __func__, __LINE__, gCurTime.tm_sec);
//	ExEnableClock(1);
//	DrawCircleClock(gCurTime.tm_hour,gCurTime.tm_min, gCurTime.tm_sec, 31,31,32);
	memset((U8 *)&VryPwd, 0, sizeof(TInputPwd));
	FirstLine=0;
	if(gOptions.IsTestMachine) return;
	if(gMachineState==STA_MENU)	return;	//避免操作菜单出现报警时菜单错乱，两点闪
	if(gState.state & STA_DIALOG)   return;
	if(gCartoon.type)   return;
	DelayMS(1);
	//ExEnableClock(1);
	g1ToG=FALSE;
	ExLightLED(LED_GREEN, FALSE);
	ExLightLED(LED_RED, FALSE);
	LCDBufferStart(LCD_BUFFER_ON);
	LCD_Clear();
#if 1
	if(0==fd_sensor && !WaitAdminVerifyCount)// && !once)
	{
	//	ExBeep(5);
	//	once = 1;
		LCDWriteStr(0,0," FPReader Error! ",LCD_HIGH_LIGHT);
	}
	else if(!TestEnabledMsg(MSG_TYPE_BUTTON) ||
		(gState.state & STA_COMM))
	{
		LCDWriteCenterStrID(0,HID_WORKING);
	}
	else if(WaitAdminVerifyCount)
	{
		if(WaitAdminVerifyCount>1)
		{
			LCDWriteStr(0,0,LoadStrByID(HID_VADMIN),0);
		//	printf(tmp2, "%d", WaitAdminRemainCnt);
		//	PadRightStrStr(tmp1, LoadStrByID(HID_VADMIN), tmp2, gLCDCharWidth);
		//	LCDWriteStr(0,0,tmp1,0);
		}
		else
			LCDWriteStr(0,0,LoadStrByID(HID_VADMIN),0);
	}
	else if (gOptions.ShowState && (gOptions.AttState>=0) && 
			(gOptions.ShowCheckIn || (gOptions.AttState>1)))
	{
		char *p=LoadStrByID(HID_SCIN+gOptions.AttState);
		if(p)
			i=gLangDriver->GetTextWidthFun(gLangDriver, p)/gLangDriver->CharWidth;
		else
			i=0;
		LCDClearLine(0);
		LCDWriteLineStrID(0,HID_WELCOME);
		if(p)
			LCDWriteStr(0,gLCDCharWidth-i,p,0);
	}
	else if(DelayNextUser)
		LCDWriteLineStrID(0,HID_MUSER_OPEN1);
//	else if(gLCDRowCount>2)
//		LCDWriteLineStrID(0,HID_WELCOME);
	else	show_first_line = 1;
//	else
//		firstline=1;
#endif
	if(gLCDHeight>=64)
	{
		char *p;
		if(gOptions.MenuStyle==MenuStyle_ICON)
		{
		/*	int x1=2,x2=2*13+10,x3=2*12+6;
			i=FormatDate(tmp1, gOptions.DateFormat, gCurTime.tm_year, gCurTime.tm_mon, gCurTime.tm_mday);
			if(gLangDriver && gLangDriver->RightToLeft)
			{
				x1=gLCDWidth-x2-4+13; x2=x1-8-2*13; x3=x1-6;
				LCDWriteStr(gLCDRowCount-1,gLCDCharWidth-strlen(tmp1), tmp1, 0);
			}
			else
				LCDWriteLine(gLCDRowCount-1,tmp1);
			DrawNum2(x1, 16+4, gCurTime.tm_hour, 12);
			DrawNum2(x2, 16+4, gCurTime.tm_min, 12);
			LCD_Bar(x3, 16+4+4+2, x3+2, 16+8+2+2);
			LCD_Bar(x3, 16+4+4+10, x3+2, 16+8+2+10);*/
			L3000ShowMainLcd(show_first_line);
		}
		else
		{
			L3000ShowMainLcd(show_first_line);
			/*
			GetTimePixel(tmp1,tmp2,gCurTime.tm_hour,gCurTime.tm_min);
			tmp1[gLCDCharWidth]=0;tmp2[gLCDCharWidth]=0; //sprintf(tmp2+14,"%02d",gCurTime.tm_sec);
			LCDWriteStrLng(gSymbolDriver, 1,0,tmp1, 0);
			LCDWriteStrLng(gSymbolDriver, 2,0,tmp2, 0);
	//		LCD_ClearBar(0, 16, 128, 16+32);
	//		DrawNum2(28, 16, gCurTime.tm_hour, 14);
	//		DrawNum2(64+8, 16, gCurTime.tm_min, 14);
			i=FormatDate(tmp1, gOptions.DateFormat, gCurTime.tm_year, gCurTime.tm_mon, gCurTime.tm_mday);
			LCDWriteLine(gLCDRowCount-1,tmp1);
			p=LoadStrByID(HID_DAY0+gCurTime.tm_wday);
			LCDWriteStr(gLCDRowCount-1,gLCDCharWidth-GetTextWidth(p)/gLangDriver->CharWidth,p,0);
			SecondPos=gLCDCharWidth-2;
			if(gOptions.ShowSecond)
				ExShowDevIcon(1,0);*/
		}
	}
#if 0
	else if(gLCDRowCount==4)
	{
		if(gOptions.ShowSecond)
			sprintf(tmp1, " %02d:%02d:%02d", gCurTime.tm_hour,gCurTime.tm_min, gCurTime.tm_sec);
		else
			sprintf(tmp1, " %02d:%02d", gCurTime.tm_hour,gCurTime.tm_min);
		LCDWriteCenterStr(1,"");
		LCDWriteCenterStr(2,tmp1);
		SecondPos=gLCDCharWidth-(gLCDCharWidth-strlen(tmp1))/2-3;
		i=FormatDate(tmp1, gOptions.DateFormat, gCurTime.tm_year, gCurTime.tm_mon, gCurTime.tm_mday);
		PadRightStrStr(tmp2,tmp1,LoadStrByID(HID_DAY0+gCurTime.tm_wday), gLCDCharWidth);
		LCDWriteStr(3,0,tmp2, 0);
	}
	else
	{
		if(firstline)
		{
			if(gOptions.ShowSecond)
				sprintf(tmp1, "%02d:%02d:%02d", gCurTime.tm_hour,gCurTime.tm_min, gCurTime.tm_sec);
			else
				sprintf(tmp1, "%02d:%02d", gCurTime.tm_hour,gCurTime.tm_min);
			SecondPos=gLCDCharWidth-(gLCDCharWidth-strlen(tmp1))/2-3;
			LCDWriteCenterStr(0,tmp1);
			i=FormatDate(tmp1, gOptions.DateFormat, gCurTime.tm_year, gCurTime.tm_mon, gCurTime.tm_mday);
			sprintf(tmp2, "%%%ds", gLCDCharWidth-i);
			sprintf(tmp1+i,tmp2,LoadStrByID(HID_DAY0+gCurTime.tm_wday));
			LCDWriteStr(1,0,tmp1, 0);
		}
		else
		{
			char sday[10]="        ";
			i=FormatDate2(tmp1, gOptions.DateFormat, gCurTime.tm_mon, gCurTime.tm_mday);
			sprintf(tmp1+i, " %02d:%02d", gCurTime.tm_hour,gCurTime.tm_min);
			sprintf(tmp2, "%%%ds", gLCDCharWidth-i-6);
			SCopyStrFrom(sday, LoadStrByID(HID_SHORTWEEK), gCurTime.tm_wday);
			sprintf(tmp1+i+6,tmp2,sday);
			LCDWriteStr(1,0,tmp1, 0);
		}
	}
#endif
//	LCDWriteStrLng(gSymbolDriver,0,10,"\x12\x12\x13", 0);
//	sprintf(tmp1, "%d",GetIntSec());
//	LCDWriteStr(0,0,tmp1,0);
//	LCD_Line(10,10,gCurTime.tm_sec+30,10);
	DebugOutput("\n show main lcd!\n");
//	LCDClearLine(3);

	LCDBufferStart(LCD_BUFFER_OFF);
}

void TriggerDuress(U16 pin, int verified)
{
        U32 d[3];
        WaitDuressAlarm=gOptions.DuressAlarmDelay;
        d[0]=0xFFFFFFFF;
        d[1]=OP_DURESS | (pin<<16);
        d[2]=verified;
        CheckSessionSend(EF_ALARM, (void*)d, 12);
        FDB_AddOPLog(pin, OP_DURESS, verified, 0, 0, 0);
}

int Filter_Group_Run(int TID)
{
	int i, count=gOptions.MaxUserCount*100;
        int pin=TID & 0xFFFF;
	
	for(i=0;i<count;i++)
	{
		if(gFilterBuf[i].PIN==pin) return TRUE;
		if(gFilterBuf[i].PIN==0) break;	
	}
	return FALSE;
}

int Filter_Head_Run(int TID)
{
	int i, count=gOptions.MaxUserCount*100;
        U32 pin=TID & 0xFFFF;
        U32 ipin=gInputPin, res=1;
	
	for(i=0;i<count;i++)
	{
		if(gFilterBuf[i].PIN==pin)
		{
			if(gOptions.PIN2Width>PIN_WIDTH)
				pin=gFilterBuf[i].PIN2;
			if(pin==0) break;
			while(ipin<=pin)
			{
				U32 ipn=ipin*10;
				if(ipn>pin) break;
				if(ipin>(U32)0xFFFFFFFD/10) break;
				ipin=ipn;
				res*=10;
			}
			if(ipin<=pin)
			{
				if(res>pin-ipin) return TRUE;
			}
			break;
		}
		if(gFilterBuf[i].PIN==0) break;	
	}
	return FALSE;	
}

U16 GetUserPIN_16(U32 pin)
{
	PUser u;	
	
	if(gOptions.PIN2Width>PIN_WIDTH)
		u=FDB_GetUserByPIN2(pin, NULL);
	else
		u=FDB_GetUser(pin, NULL);
	if(u)
		return u->PIN;
	else
		return 0;
}

#define LOWQLT -100

extern int gFPDirectProc;
extern int GroupFpCount[5];


int IdentifyFinger(char *InputPin, U32 PIN, BYTE *Temp, BYTE* Image)
{
	int result=-1, score=0, i, FingerID=0;
	char Buffer[32];
	int LastTempLen=0;
	BYTE LastTemplate[1024];

	LCDClear();
	
	if(InputPin[0]||Temp[0]||PIN)
	{
		LCDWriteLineStrID(0, HID_1TO1);
	}
	else
	{
		LCDWriteLineStrID(0, HID_VF);
	}
	LCDWriteCenterStrID(gLCDRowCount/2, HID_LEAVEFINGER);
	if (gOptions.Nideka)
		LCDWriteCenterStrID(gLCDRowCount-1, NID_LEAVEFINGER);


//如果打开分组比对的话，默认为系统默认组进行分组比对
	if (gOptions.I1ToG && (gOptions.GroupFpLimit != gOptions.LimitFpCount) && (!InputPin[0]))
	{
		sprintf(InputPin,"%d",gOptions.DefaultGroup);
		g1ToG = TRUE;
	}
	
	unsigned int s_msec, e_msec;	
//	s_msec=GetTickCount();
	if (gOptions.ZKFPVersion == ZKFPV10)
		LastTempLen=BIOKEY_EXTRACT_10(fhdl, Image, LastTemplate,  EXTRACT_FOR_IDENTIFICATION);
	else
		LastTempLen=BIOKEY_EXTRACT(fhdl, Image, LastTemplate,  EXTRACT_FOR_IDENTIFICATION);
//	e_msec=GetTickCount();
//	DBPRINTF("EXTRACT FINISHED! time=%d tmplen=%d\n", e_msec-s_msec, LastTempLen);
	
	if(LastTempLen>0)
	{			
		if(!InputPin[0]&&!Temp[0]&&!PIN)  //1:many
		{
			score=gOptions.MThreshold;		       
//			s_msec=GetTickCount();
			if (gOptions.ZKFPVersion == ZKFPV10)
			{
				BIOKEY_DB_FILTERID_ALL_10(fhdl);
				if(!BIOKEY_IDENTIFYTEMP_10(fhdl, (BYTE*)LastTemplate, &result, &score))
					result=-1;
				else
				{
					//High byte = FingerID;
					FingerID = (result>>16) & 0xf;
					result &= 0xFFFF;
				}			
			}
			else
			{
				BIOKEY_DB_FILTERID_ALL(fhdl);
				if(!BIOKEY_IDENTIFYTEMP(fhdl, (BYTE*)LastTemplate, &result, &score))
					result=-1;
				else
				{
					//High byte = FingerID;
					FingerID = (result>>16) & 0xf;
					result &= 0xFFFF;
				}			
			}
//			e_msec=GetTickCount();
//			DBPRINTF("IDENTIFY FINISHED! time = %d ID=%d\n", e_msec-s_msec, result);
		}
		else if(Temp[0])  //1:1 fingerprint - MIFARE
		{
			if (gOptions.ZKFPVersion == ZKFPV10)
				BIOKEY_MATCHINGPARAM_10(fhdl, SPEED_LOW, gOptions.VThreshold); //use low speed matching for verification
			else
				BIOKEY_MATCHINGPARAM(fhdl, SPEED_LOW, gOptions.VThreshold); //use low speed matching for verification
			result =-1;
			i=0;
			while(i<gOptions.RFCardFPC)
			{
				int TmpLen;
				if (gOptions.ZKFPVersion == ZKFPV10)
					score=BIOKEY_VERIFY_10(fhdl, Temp, LastTemplate);
				else
					score=BIOKEY_VERIFY(fhdl, Temp, LastTemplate);
				if(gOptions.VThreshold<=score)
				{
					result=GetUserPIN_16(PIN); 
					break;
				}
				if (gOptions.ZKFPVersion == ZKFPV10)
					TmpLen=BIOKEY_TEMPLATELEN_10(Temp);
				else
					TmpLen=BIOKEY_TEMPLATELEN(Temp);
				if(TmpLen>0)
					Temp+=TmpLen;
				else
					break;
			}
			if (gOptions.ZKFPVersion == ZKFPV10)
				BIOKEY_MATCHINGPARAM_10(fhdl, IDENTIFYSPEED, gOptions.MThreshold); //restore identification speed
			else
				BIOKEY_MATCHINGPARAM(fhdl, IDENTIFYSPEED, gOptions.MThreshold); //restore identification speed
		}
		else if(PIN)  //1:1 fingerprint - PIN
		{			
			result=GetUserPIN_16(PIN);
			if(result)
			{
				TTemplate tmp;
				//use low speed matching for verification
				if (gOptions.ZKFPVersion == ZKFPV10)
					BIOKEY_MATCHINGPARAM_10(fhdl, SPEED_LOW, gOptions.VThreshold);
				else
					BIOKEY_MATCHINGPARAM(fhdl, SPEED_LOW, gOptions.VThreshold);
				
#if  0
				for(i=0;i<gOptions.MaxUserFingerCount;i++)
				{
					if(FDB_GetTmp((U16)result, (char)i, &tmp))
					{	
						if (gOptions.ZKFPVersion == ZKFPV10)
							score=BIOKEY_VERIFY_10(fhdl, (BYTE*)tmp.Template, LastTemplate);
						else
							score=BIOKEY_VERIFY(fhdl, (BYTE*)tmp.Template, LastTemplate);
						if(gOptions.VThreshold<=score)
							break;
					}	
				}
#else
				//score=FDB_VerifyFinger((U16)result,&i,LastTemplate);
				TSearchHandle sh;
				TTemplate tmpp;
				sh.ContentType=FCT_FINGERTMP;
				sh.buffer=&tmpp;
				SearchFirst(&sh);
				while(!SearchNext(&sh))
				{
					if(((PTemplate)sh.buffer)->PIN == (U16)result)
					{
						if (gOptions.ZKFPVersion == ZKFPV10)
							score=BIOKEY_VERIFY_10(fhdl, ((PTemplate)sh.buffer)->Template, LastTemplate);
						else
							score=BIOKEY_VERIFY(fhdl, ((PTemplate)sh.buffer)->Template, LastTemplate);
						if(gOptions.VThreshold<=score)
						{
							i = ((PTemplate)sh.buffer)->FingerID;
							break;
						}
						else
							i = gOptions.MaxUserFingerCount;
					}
				}
#endif
				//restore identification speed
				if (gOptions.ZKFPVersion == ZKFPV10)
					BIOKEY_MATCHINGPARAM_10(fhdl, IDENTIFYSPEED, gOptions.MThreshold);
				else
					BIOKEY_MATCHINGPARAM(fhdl, IDENTIFYSPEED, gOptions.MThreshold);
				if(i>=gOptions.MaxUserFingerCount)
				{
					KeyBufferTimeOut=TIMEOUT_INPUT_PIN;
					result=-1;
				}
				else
					FingerID=i;
			}
			else
				result=-2;	
		}
		else //1:1 1:G 1:H user
		{
			if(0==strtou32(InputPin, (U32*)&gInputPin))
			{
				if(gInputPin)
				{
					if(gOptions.I1ToH||g1ToG)
					{
						int fcount;
						if(g1ToG)
						{
							GetFilterGroupInfo(gInputPin, gFilterBuf);
							if (gOptions.ZKFPVersion == ZKFPV10)
								fcount=BIOKEY_DB_FILTERID_10(fhdl, Filter_Group_Run);
							else
								fcount=BIOKEY_DB_FILTERID(fhdl, Filter_Group_Run);
						}
						else
						{
							GetFilterHeadInfo(gInputPin, gFilterBuf);
							if (gOptions.ZKFPVersion == ZKFPV10)
								fcount=BIOKEY_DB_FILTERID_10(fhdl, Filter_Head_Run);
							else
								fcount=BIOKEY_DB_FILTERID(fhdl, Filter_Head_Run);
						}
						if(fcount>=1)
						{
							score=fcount<10?gOptions.VThreshold:gOptions.MThreshold;
							//use low speed matching for verification
							if (gOptions.ZKFPVersion == ZKFPV10)
							{
								BIOKEY_MATCHINGPARAM_10(fhdl, IDENTIFYSPEED, score);
								if(!BIOKEY_IDENTIFYTEMP_10(fhdl, LastTemplate, &result, &score))
									result =-1;
								else
								{
									//高字节是FingerID;
									FingerID = (result>>16) & 0xf;
									result &= 0xFFFF;
								}
								//restore identification speed
								BIOKEY_MATCHINGPARAM_10(fhdl, IDENTIFYSPEED, gOptions.MThreshold);
							}
							else
							{
								BIOKEY_MATCHINGPARAM(fhdl, IDENTIFYSPEED, score);
								if(!BIOKEY_IDENTIFYTEMP(fhdl, LastTemplate, &result, &score))
									result =-1;
								else
								{
									//高字节是FingerID;
									FingerID = (result>>16) & 0xf;
									result &= 0xFFFF;
								}
								//restore identification speed
								BIOKEY_MATCHINGPARAM(fhdl, IDENTIFYSPEED, gOptions.MThreshold);
							}	
						}
						else
							result=-2;
					}
					else
					{		
						result=GetUserPIN_16(gInputPin);
						if(result)
						{
							TTemplate tmp;
							//use low speed matching for verification
							if (gOptions.ZKFPVersion == ZKFPV10)
								BIOKEY_MATCHINGPARAM_10(fhdl, SPEED_LOW, gOptions.VThreshold);
							else
								BIOKEY_MATCHINGPARAM(fhdl, SPEED_LOW, gOptions.VThreshold);
#if 0
							for(i=0;i<gOptions.MaxUserFingerCount;i++)
							{
								if(FDB_GetTmp((U16)result, (char)i, &tmp))
								{	
									if (gOptions.ZKFPVersion == ZKFPV10)
										score=BIOKEY_VERIFY_10(fhdl, (BYTE*)tmp.Template, LastTemplate);
																		     else
										score=BIOKEY_VERIFY(fhdl, (BYTE*)tmp.Template, LastTemplate);
									if(gOptions.VThreshold<=score)
										break;
								}	
							}
#else
	//score=FDB_VerifyFinger((U16)result,&i,LastTemplate);
	TSearchHandle sh;
	TTemplate tmpp;
	sh.ContentType=FCT_FINGERTMP;
	sh.buffer=&tmpp;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if(((PTemplate)sh.buffer)->PIN == (U16)result)
		{
			if (gOptions.ZKFPVersion == ZKFPV10)
				score=BIOKEY_VERIFY_10(fhdl, ((PTemplate)sh.buffer)->Template, LastTemplate);
			else
				score=BIOKEY_VERIFY(fhdl, ((PTemplate)sh.buffer)->Template, LastTemplate);
			if(gOptions.VThreshold<=score)
			{
				i = ((PTemplate)sh.buffer)->FingerID;
				break;
			}
			else
				i = gOptions.MaxUserFingerCount;
		}
	}
#endif
							//restore identification speed
							if (gOptions.ZKFPVersion == ZKFPV10)
								BIOKEY_MATCHINGPARAM_10(fhdl, IDENTIFYSPEED, gOptions.MThreshold);
							else
								BIOKEY_MATCHINGPARAM(fhdl, IDENTIFYSPEED, gOptions.MThreshold);
							if(i>=gOptions.MaxUserFingerCount)
							{
								KeyBufferTimeOut=TIMEOUT_INPUT_PIN;
								result=-1;
							}
							else
								FingerID=i;
						}
						else
							result=-2;
					}
				}
				else
					result=-3;
			}	
			else
				result =-3;
		}
/*		if(result>0)
		{			
			if(DelayTriggerDuress||FDB_IsDuressTmp(result, FingerID)||
			   ((!InputPin[0]&&!Temp[0]) && gOptions.Duress1ToN)||((InputPin[0]||Temp[0]) && gOptions.Duress1To1)||((InputPin[0] && !Temp[0]) && gOptions.I1ToG && gOptions.Duress1ToN))
			{
                                DelayTriggerDuress=10;	
                        }
			//PIN2
			if(gOptions.PIN2Width>PIN_WIDTH)
				result=FDB_GetUser(result, NULL)->PIN2;
		}
*/		if(gOptions.ShowScore)
		{
			sprintf(Buffer,"%2d", CalcNewThreshold(score));
			LCDWriteStr(0, 14, Buffer, LCD_HIGH_LIGHT);
		}
                if(gFPDirectProc==-2)
                        gFPDirectProc=0;
                if(gFPDirectProc==-3 && result>0)
                        gFPDirectProc=0;
                else if(gFPDirectProc>0)
                        gFPDirectProc--;
	}
	else
	{
		result=LOWQLT;
		LCDWriteLineStrID(0, HID_LOWQLT);
		LCDWriteCenterStrID(gLCDRowCount/2, HID_TRYAGAIN);
		*Buffer=0;
		if(gFPDirectProc==0) CheckSessionSend(EF_FPFTR, Buffer, 1);

	}

	memcpy(Buffer, &result, 4);
	Buffer[4]=1;
	CheckSessionSend(EF_VERIFY, Buffer, 5);
	if(gOptions.RS232Fun==2)//简单输出ID号
	{
		if(result<=0)
		{
			if(PIN)
				sprintf((char*)Buffer,"-F%d\r\n", PIN);
			else if(InputPin)
				sprintf((char*)Buffer,"-F%s\r\n", InputPin);
			else
				sprintf((char*)Buffer,"-F\r\n");
			SerialOutputString(&ff232, (char*)Buffer);
		}
	}

	LastUserID=result;
	IsVryFgID = FingerID;	
	return result;
}

int InputWorkCode(U16 pin, PUser u, int *workcode)
{
	BOOL ret;
	char name[20];	
	int delay;
	U32 mm=SelectNewMsgMask(MSG_TYPE_BUTTON|MSG_TYPE_TIMER);	
	LCDClear();
	if(gOptions.ShowName && u && u->Name[0])
	{
		memset(name, 0, 20);
		nstrcpy(name, u->Name, MAXNAMELENGTH);
		LCDWriteCenterStr(0, name);
	}
	else
	{
		if(u && u->PIN2)
			ShowUserHint(0, TRUE, u->PIN2);
		else
			ShowUserHint(0, FALSE, pin);
	}
	LCDWriteCenterStrID(1, MID_WORKCODE);
	LCDWriteCenterStrID(gLCDRowCount-1, HID_SAVECANCEL);
	delay=gOptions.TimeOutMenu;
        gOptions.TimeOutMenu=LoadInteger("TimeOutWC", 5);	
	//ret=InputNumber(gLCDRowCount/2, 3, 9, workcode, 0, 0x7FFFFFFF, FALSE);
        ret=InputWCNumber(gLCDRowCount/2, 3, 9, workcode);
	gOptions.TimeOutMenu=delay;
	FlushSensorBuffer();
	SelectNewMsgMask(mm);
	ShowMainLCDDelay=1;
	return ret;
}

int InputAdvWorkCode(int *workcode)
{
        BOOL ret;
	int delay;
        U32 mm=SelectNewMsgMask(MSG_TYPE_BUTTON|MSG_TYPE_TIMER);

        LCDClear();
        LCDWriteCenterStrID(0, MID_WORKCODE);
        LCDWriteCenterStrID(gLCDRowCount-1, HID_SAVECANCEL);
	delay=gOptions.TimeOutMenu;
        gOptions.TimeOutMenu=LoadInteger("TimeOutWC", 5);	
        ret=InputWCNumber(gLCDRowCount/2, 3, 9, workcode);
        if (ret==News_CommitInput)
        {
		LCDClearLine(gLCDRowCount-1);
        	LCDWriteCenterStrID(gLCDRowCount-1, HID_PLACEFINGER);
		
                if(gOptions.VoiceOn)
                        ExPlayVoice(VOICE_WK_HINTFP);
                else
                        ExBeep(1);

        }
	gOptions.TimeOutMenu=delay;
        FlushSensorBuffer();
        SelectNewMsgMask(mm);
	ClockEnabled = FALSE;
        ShowMainLCDDelay = 5;
        return ret;

}
int InputAttState()
{
	int ret=0;
        int i=0, v[6]={0,1,2,3,4,5};
        char buf[MAX_CHAR_WIDTH*2];

	U32 mm=SelectNewMsgMask(MSG_TYPE_BUTTON|MSG_TYPE_TIMER);	
        LCD_Clear();
        sprintf(buf,"%s:%s", LoadStrByID(HID_SCIN), LoadStrByID(HID_SCOUT));

        LCDWriteLineStrID(0,MID_AUTO_STATE);
        LCDWriteCenterStrID(3, HID_OKCANCEL);

        if(gLCDRowCount>2)
        {
                LCDWriteCenterStrID(1, HID_STATE);
                ret=LCDSelectItemValue(2, (gLCDCharWidth-8)/2, 8, buf, v, &i);
        }
        else
        {
                LCDWriteLineStrID(1, HID_STATE);
                ret=LCDSelectItemValue(1, (gLCDCharWidth-8)/2, 8, buf, v, &i);
        }
	FlushSensorBuffer();
	SelectNewMsgMask(mm);
	//EnableMsgType(MSG_TYPE_FINGER, 0); 
	printf("Attstate: %d\tfpdata.pin: %d\n",i,FPData.PIN);
	return ret;


}

#define LOG_VALID		0
#define LOG_REPEAT		1
#define LOG_INVALIDTIME		2
#define LOG_INVALIDCOMBO   	4
#define LOG_INVALIDUSER		8

void OutputPrinterFmt2(int pin)
{
        char buf1[30]={0x1B,0x40,0x1B,0x61,0x01,0x1C,0x70,0x01,0x00,0x1B,0x61,0x00,0x1B,0x21,0x8};
        char buf2[30]={0x1B,0x21,0x00};
        char buf3[10] ={0x0A,0x0B,0x0A,0x0B,0x1B,0x21,0x08};
        char buf4[10] ={0x1B,0x21,0x00,0x0A,0x0B};
        char buf5[20] ={0x0A,0x0B,0x0A,0x0B,0x0A,0x0B,0x0A,0x0B,0x0A,0x0B,0x0A,0x0B,0x0A,0x0B,0x1B,0x69};
        char buf6[30] ={0};     //"Enroll Number"
        char buf7[30] ={0}; //Enroll number
        char buf8[30] ={0};     //"Date time    CheckIn"
        char buf9[60] ={0};     //Date time

        sprintf(buf6,"%s%s",LoadStrByID(HID_ENROLLNUM),":  ");
        sprintf(buf7,"%s",FormatPIN(pin));
        sprintf(buf8,"%s      %s",LoadStrByID(MID_OS_TIME),LoadStrByID(HID_SCIN+gOptions.AttState));        FormatDate(buf9, gOptions.DateFormat, gCurTime.tm_year, gCurTime.tm_mon+1, gCurTime.tm_mday);
        sprintf(buf9+8,"        %02d:%02d:%02d",gCurTime.tm_hour,gCurTime.tm_min,gCurTime.tm_sec);
        SerialOutputData(&ff232, buf1,15);
        SerialOutputString(&ff232, buf6);
        SerialOutputData(&ff232,buf2,3);
        SerialOutputString(&ff232,buf7);
        SerialOutputData(&ff232,buf3,7);
        SerialOutputString(&ff232,buf8);
        SerialOutputData(&ff232,buf4,5);
        SerialOutputString(&ff232,buf9);
        SerialOutputData(&ff232,buf5,16);
}

int CheckAC(int pin,int VerifiedMethod,PUser u, U32 t,int LogState)
{
	int c;
	PAttLog lastlog=NULL;
	//是否指令控制开门
/*	if(gOptions.LockFunOn && gOptions.ControlOn>0)
	{
		TTime ControlStartTime;
		int i;
		DecodeTime(gOptions.ControlStartTime,&ControlStartTime);
		i=TimeDiffSec(gCurTime,ControlStartTime);
		if(!(i>0 && i<=gOptions.ControlMinute*60))
		{
			LogState=LOG_NOTINCONTROLTIME;
		}
	}*/
	//判断时间段是否允许
/*	if((gOptions.LockFunOn & LOCKFUN_ADV) && LogState==LOG_VALID)
		if(!TestUserTZ(pin, gCurTime))
			LogState|=LOG_INVALIDTIME;
	//检查反潜
	if(LogState==LOG_VALID && gOptions.AntiPassbackFunOn &&
		gOptions.AntiPassbackOn)
	{
		lastlog=GetUserLastLog(lastlogs,pin,LastLogsCount);
		switch (gOptions.AntiPassbackOn)
		{
		case ANTIPASSBACK_OUT:
			if(!lastlog || (lastlog->status==STATE_IN && (getCurrentState()==STATE_IN)))
				LogState=LOG_ANTIPASSBACK;
			break;
		case ANTIPASSBACK_IN:
			if(!lastlog || (lastlog->status==STATE_OUT && (getCurrentState()==STATE_OUT)))
				LogState=LOG_ANTIPASSBACK;
			break;
		case ANTIPASSBACK_INOUT:
			if(lastlog)
			if((lastlog->status==STATE_IN && (getCurrentState()==STATE_IN)) ||
				(lastlog->status==STATE_OUT && (getCurrentState()==STATE_OUT)))
				LogState=LOG_ANTIPASSBACK;
			break;
		}
	}
	//消费机检查
	if(LogState==LOG_VALID && gOptions.ConsumerFunOn)
	{
		if(!ValidConsumer(pin))
			LogState=LOG_INVALIDTIME;
	}
	if(gOptions.LockFunOn&8 && gLockForceAction==2)
		LogState=LOG_INVALIDTIME;*/
	IsOpenDoorAction=0;
	//是否开门
	if(LogState==LOG_VALID)
	if(gOptions.LockFunOn)
//	if(!(gOptions.MustChoiceInOut && (gOptions.AttState<0)))
	{
		DelayNextUser=0;
		c=1;//TestOpenLock(pin, gCurTime, VerifiedMethod);
		DebugOutput1("TestOpenLock %d\n", c);
		if(c>0)
		{
			if(gOptions.LockOn && gLockForceAction==0)
			{
				L3000Debug("FP VERIFY: START UNLOCK");
				IsOpenDoorAction=1;
				//开门并保存记录
		/*		if(gOptions.LockOn<=254)
					DoAuxOut(gOptions.LockOn, gOptions.OpenDoorDelay);
				else
				{
					DoAuxOut(0xFF,gOptions.OpenDoorDelay);
					gLockDelay=gOptions.LockOn/25;
				}*/
				gLockDelay=gOptions.LockOn/25;
				
				if(gOptions.SaveUnlockLog)
				if((gOptions.LockFunOn & LOCKFUN_ADV) || (gOptions.UnlockPerson>1))
					if(FDB_OK!=FDB_AddAttLog(0, t, 0, c, 0, 0, 0))
						LogState|=LOG_UNSAVED;
	//			L3000RunBeep(200, 0, 0);	
	//			L3000RunGLed(3000, 0,0);
	//			L3000RunLock(1);
			//	L3000ShowUNLock(0);
				gState.vry_unlock = 1;
				L3000Debug("FP VERIFY: UNLOCK END");
			}
		}
		else
		{
			if(c<0)
			{	//需要多用户验证
				//LCDWriteCenterStrID(hintrow,HID_MUSER_OPEN1);
				LogState|=LOG_WAITCOMBO;
				DelayNextUser=DELAYNEXTUSER;
			}
			else	//组合不正确
				//LCDWriteCenterStrID(hintrow,HID_MUSER_OPEN2);
				LogState|=LOG_INVALIDCOMBO;
			//hintrow=5;
		}
	}else
		LogState|=LOG_MUSTCHOICEINOUT;
	return LogState;
}


int SaveLog(U16 pin, int VerifiedMethod,U32 t,int LogState)
{
	int ret=0,c=FDB_CntAttLog();
	if(gOptions.MaxAttLogCount>0 && c>=gOptions.MaxAttLogCount*10000)	//超过最大记录容量
	{
	/*	if(gOptions.AlarmAttLog||
			(DeleteOldLog()==0))//记录警告为零时，自动删除最旧的记录扇区
		{
			LCDWriteCenterStrID(gLCDRowCount/2,HID_EXCEED);
			LCDClearLine(3);
			if(gOptions.VoiceOn)
				ExPlayVoice(VOICE_NO_LOG_RECSPACE);
			else
				ExBeepN(5);
			LogState|=LOG_NOSPACE;
		}
        	c=FDB_CntAttLog();*/
		LogState |= LOG_NOSPACE;
	}
	else	if(c+gOptions.AlarmAttLog>=gOptions.MaxAttLogCount*10000)	//提示记录快满信息
	{
		/*LCDWriteCenterStrID(gLCDRowCount/2,HID_LEFTSPACE);
		sprintf(name, "%d",gOptions.MaxAttLogCount*10000-FDB_CntAttLog());
		LCDWriteCenterStr(gLCDRowCount-1, name);*/
		ret|=LOG_ALARMLEFTSPACE;
		LogState |= LOG_ALARMLEFTSPACE;
	}
	else	;

	//保存记录
#ifdef MODULE	
	if(!gIsSetTime)
		;//不保存记录,用于锁
	else
#endif
	if(gOptions.SaveAttLog
		//&& (LogState!=LOG_REPEAT) && (LogState!=LOG_INVALIDUSER)
		&& (!(LogState & (LOG_REPEAT+LOG_INVALIDUSER+LOG_NOSPACE))) 
		&& (gHaveRTC==1 || gExternalRTC) 
		&& (!(gOptions.UnSaveLogDeny && ((LogState&LOG_INVALIDCOMBO) || (LogState&LOG_INVALIDTIME))))
		)
	{
		int state=getCurrentState();
		if(gOptions.TagLogDoorAction)
		{
			state|=IsOpenDoorAction<<2;	//状态为3个字节，最高字节为是否开锁动作
		}
		if(FDB_OK!=FDB_AddAttLog((U16)pin, t, (char)VerifiedMethod, (char)state, 0, 0, 0))
			LogState|=LOG_UNSAVED;

			/*if(FDB_OK!=FDB_AddAttLog((U16)pin, t, (char)VerifiedMethod, (char)7))
				LogState|=LOG_UNSAVED;*/
		if(LogState&LOG_UNSAVED)
		{
		}else
		{	//更新最后打卡记录集
			if(gOptions.AlarmReRec || (gOptions.AntiPassbackFunOn && gOptions.AntiPassbackOn))
			{
				TAttLog log;
				log.status=state;
				log.time_second=t;
				log.PIN=pin;
				log.verified=(char)VerifiedMethod;
				if(AddToOrderedLastLogs(lastlogs,&log,LastLogsCount))
					LastLogsCount++;
			}
		}
	}
	return LogState;
}

static	int	VryMethod = 0;
void	L3000ChangeMethod(int	VerifiedMethod)
{
	if(VerifiedMethod == 0)
		VryMethod = VRY_METHOD_BYPWD;
	else	if(VerifiedMethod == 2)
		VryMethod =  VRY_METHOD_BYCARD;
	else	VryMethod =  VRY_METHOD_BYFP;
}


int ShowResult(U16 pin,PUser u,int LogState)
{	
	int	i = 1000;
	char *s = NULL;
	char name[20];
	int c,hintrow=gLCDRowCount-1;
	if(gLCDRowCount<4) hintrow=0;
	if(LogState==LOG_NOREGISTEREDUSER)	//未注册用户
	{
		i += 1;
//		LCDWriteCenterStrID(gLCDRowCount-1, HID_UIDERROR);
//		ShowVerifyFail(0,News_VerifiedByCard, FALSE, 0,-1);
		L3000CheckVryFailCnt(VryMethod);
		L3000ShowVerifyInValid(101);
		return 0;
	}
/*	else if(LogState==LOG_REPEAT)
	{	//记录重复
		if(gOptions.VoiceOn) ExPlayVoice(VOICE_ALREADY_LOG);
		LCDWriteCenterStrID(hintrow,HID_ALREADY);
		return 0;
	}*/
	else if(LogState==LOG_INVALIDUSER)	//禁止用户
	{/*
#ifndef MODULE
		LCDWriteCenterStrID(gLCDRowCount-1,HID_PRI_INVALID);
		if(u && (gOptions.PIN2Width>PIN_WIDTH) && u->PIN2)
			ShowUserHint(gLCDRowCount/2, TRUE, u->PIN2);
		else
			ShowUserHint(gLCDRowCount/2, FALSE, pin);
#endif*/
				
		L3000CheckVryFailCnt(VryMethod);
		L3000ShowVerifyInValid(102);
		return 0;
	}

	if(LogState!=LOG_VALID)
	{
	/*	if(LogState&LOG_NOTINCONTROLTIME)	//不在有效的控制时间内
		{
			LCDWriteCenterStrID(hintrow, MID_CONTROLON);
			ExLightLED(LED_RED, TRUE);
		}
		if(LogState&LOG_INVALIDTIME)	//非法时间段
		{
			LCDWriteCenterStrID(hintrow, HID_INVALIDTIME);
			ExLightLED(LED_RED, TRUE);
		}
		if(LogState&LOG_ANTIPASSBACK)
		{
			LCDWriteCenterStrID(hintrow,HID_LOG_ANTIPASSBACK);
			ExLightLED(LED_RED, TRUE);
		}
		if(LogState&LOG_INVALIDCOMBO)
		{
			LCDWriteCenterStrID(hintrow,HID_MUSER_OPEN2);
			ExLightLED(LED_RED, TRUE);
		}
		if(LogState&LOG_WAITCOMBO)
		{
			LCDWriteCenterStrID(hintrow,HID_MUSER_OPEN1);
			ExLightLED(LED_RED, TRUE);
		}*/

		if(LogState&LOG_NOSPACE)
		{
			/*LCDWriteCenterStrID(gLCDRowCount/2,HID_EXCEED);
			LCDClearLine(3);
			if(gOptions.VoiceOn)
				ExPlayVoice(VOICE_NO_LOG_RECSPACE);
			else
				ExBeepN(5);*/
			s = LoadStrByID(HID_EXCEED);
		}
		if(LogState&LOG_ALARMLEFTSPACE)
		{
	/*		LCDWriteCenterStrID(gLCDRowCount/2,HID_LEFTSPACE);
			sprintf(name, "%d",gOptions.MaxAttLogCount*10000-FDB_CntAttLog());
			LCDWriteCenterStr(gLCDRowCount-1, name);*/
			
			s = LoadStrByID(HID_LEFTSPACE);
		}
/*		if(LogState&LOG_MUSTCHOICEINOUT)	//必须按状态
		{
//			LCDClear();
			LCDWriteCenterStrID(gLCDRowCount/2,HID_MUSTINOUT);
			DelayMS(6);
			ExBeep(3);
		}*/
		if(LogState&LOG_UNSAVED)
		{
		/*	if(gOptions.VoiceOn)
				ExPlayVoice(VOICE_NO_LOG_RECSPACE);
			else
				ExBeep(15);
			LCDWriteCenterStrID(gLCDRowCount/2,HID_EXCEED);*/
			s = LoadStrByID(HID_EXCEED);
		}
/*		if(LogState&LOG_ANTIPASSBACK)
		{
			LCDWriteCenterStrID(hintrow,MID_ANTIPASSBACK);
			DelayMS(6);
			ExBeep(2);
		}*/
	}
	else
	{
	/*	if(gOptions.VoiceOn) ExPlayVoice(VOICE_THANK);	//验证通过
		ExLightLED(LED_GREEN, TRUE);
		LCDWriteCenterStrID(hintrow,HID_VSUCCESS);*/
	}

	memset(name, 0, sizeof(name));
/*	if(LogState&LOG_MUSTCHOICEINOUT)
	{
	}
	else if(gOptions.ShowName && u->Name[0])	//显示姓名,验证通过
	{
		if(gLCDRowCount>2)
		{
#ifndef MODULE
			LCDClearLine(gLCDRowCount/2);
			if((gOptions.PIN2Width>PIN_WIDTH) && (u->PIN2>0))
				ShowUserHint(gLCDRowCount/2, TRUE, u->PIN2);
			else
				ShowUserHint(gLCDRowCount/2, FALSE, pin);
#endif
		}
		nstrcpy(name, u->Name, 8);
		LCDWriteCenterStr(1, name);
		if(gOptions.RS232Fun==13)
		{
			char Buffer[32];
			Buffer[0]=0;
			SendExCommand3(98, Buffer);
			Buffer[1]=0;
			Buffer[0]=6;
			sprintf(Buffer+2,"    %8s    ", name);
			SendExCommand3(100, Buffer);
			Buffer[0]=4;
			sprintf(Buffer+2,"    验证通过   ");
			SendExCommand3(100, Buffer);
		}
	}*/
//	else				//显示编号,验证通过*/

	{
#ifndef MODULE
	/*	LCDClearLine(gLCDRowCount/2);
		if(u && (gOptions.PIN2Width>PIN_WIDTH) && (u->PIN2>0))
			ShowUserHint(gLCDRowCount/2, TRUE, u->PIN2);
		else    {
			//	ShowUserHint(gLCDRowCount/2, FALSE, pin);
				L3000ShowVrySuccess(u, 0);
				L3000Debug("FP VERIFY: SHOW VRY RESULT");
			}*/
#endif
/*		if(gOptions.RS232Fun==13)
		{
			char Buffer[32];
			Buffer[0]=0;
			SendExCommand3(98, Buffer);
			Buffer[1]=0;
			if(u->PIN2)
				sprintf(Buffer+2,"    %08d    ", u->PIN2);
			else
				sprintf(Buffer+2,"    %08d    ", pin);
			Buffer[0]=6;
			SendExCommand3(100, Buffer);
			Buffer[0]=4;
			sprintf(Buffer+2,"    验证通过   ");
			SendExCommand3(100, Buffer);
		}*/
	}
	if(gState.vry_unlock){
		if(s) L3000ShowWarn(NULL, s, 3, 0);	
		L3000ShowVrySuccess(u, VryMethod, s);	
		L3000ProcUnLock(0);	
		gState.vry_unlock = 0;
	}
	else	{
		ShowMainLCDDelay = 3;
	}
	if(gFlashLED>0) gFlashLED=0;

	return 1;
}

int PrecheckLog(U16 pin, int VerifiedMethod, PUser u,U32 t)
{
	PAttLog lastlog;
	int LogState=LOG_VALID;
	//检查重复签到
//	if(gOptions.AlarmReRec && (lastlog=GetUserLastLog(lastlogs,pin,LastLogsCount)))
//	{
//		if(lastlog->status==gOptions.AttState && (t-lastlog->time_second)<gOptions.AlarmReRec*60)
//			LogState=LOG_REPEAT;
//	}
//	if(!gOptions.MustEnroll || u)
//	{
		if(u==NULL || ISINVALIDUSER(*u))
			LogState=LOG_INVALIDUSER;		//判断是否为禁止用户
//		if(gOptions.MustChoiceInOut && (gOptions.AttState<0))
//		{
//			LogState=LOG_MUSTCHOICEINOUT;
//		}

//	}
//	else
//	{
//		LogState=LOG_NOREGISTEREDUSER;
//	}
	return LogState;
}

int getCurrentState()
{
		int state=0;
		if(gOptions.MasterSlaveFunOn && gOptions.MasterSlaveOn==1 && gOptions.MasterState>=0)
		{
			if(gOptions.MasterState==0)
				if(gMasterSlaveProcess)
					state=STATE_OUT;	
				else
					state=STATE_IN;	
			else if(gOptions.MasterState==1)
				if(gMasterSlaveProcess)
					state=STATE_IN;	
				else
					state=STATE_OUT;	
			else
				state=gOptions.AttState;
		}else
			state=gOptions.AttState;
		return state;
}


void OutputPrinterFmt3(int pin)
{
#ifndef MODULE
	char buf[32];
	memset(buf,0,sizeof(buf));
	sprintf((char*)buf,"%s %3d ",FormatPIN(pin),gOptions.DeviceID);
	FormatDate(buf+8, gOptions.DateFormat, gCurTime.tm_year, gCurTime.tm_mon, gCurTime.tm_mday);
	sprintf(buf+16," %02d:%02d:%02d %s\r\n",gCurTime.tm_hour,gCurTime.tm_min,gCurTime.tm_sec, FormatStatus(gOptions.AttState));
	SerialOutputString(&ff232, buf);
	SerialOutputString(&ff232, "\r\n");
	SerialOutputString(&ff232, "\r\n");
	SerialOutputString(&ff232, "\r\n");
	SerialOutputString(&ff232, "\r\n");
#endif
}

int SaveAttLog(U16 pin, int VerifiedMethod)
{ 	
	PUser user = NULL;
	char name[20],buf[32];
	int LogState=LOG_VALID,tempValue,state;
	PUser u;
	U32 t;
	if((gOptions.ErrTimes>0) && gContinuousVerifyFailed)
		gContinuousVerifyFailed=0;
/*	if(gOptions.RS232Fun==2)//简单输出ID号,malysia 项目
	{
		sprintf((char*)buf,"+C%d\r\n", pin);
		SerialOutputString(&ff232, buf);
	}*/
	t=EncodeTime(&gCurTime);
	u=FDB_GetUser(pin,0);
	LogState=PrecheckLog(pin,VerifiedMethod,u,t);

/*	if(u && gOptions.RS232Fun==8)//输出ID号和密码
	{
		int i=0;
		int tempbuf[32]={0};
		sprintf((char*)tempbuf,"%5d;%s", pin,u->Password );
		while(tempbuf[i] && (tempbuf[i] = tempbuf[i]^i))
			i++;
		SerialOutputString(&ff232, (char*)tempbuf);
		SerialOutputString(&ff232, "\r\n");
	}*/

	if(LogState==LOG_VALID)
	{

		LogState=CheckAC(pin,VerifiedMethod, u,t,LogState);		//检查门禁规则
		LogState=SaveLog(pin,VerifiedMethod,t,LogState);
		LCDClear();
		L3000Debug("FP VERIFY: LOG SAVED");
#ifndef MODULE
		if(!(LogState&LOG_UNSAVED) && gOptions.PrinterFunOn)
			if(gOptions.PrinterOn==0)
			{
				
			}else if(gOptions.PrinterOn==2)
			{
				OutputPrinterFmt2(pin);
			}else if(gOptions.PrinterOn==3)
			{
				OutputPrinterFmt3(pin);
			}else
			{
				memset(buf,0,sizeof(buf));
				sprintf((char*)buf,"%s %3d ",FormatPIN(pin),gOptions.DeviceID);
				FormatDate(buf+8, gOptions.DateFormat, gCurTime.tm_year, gCurTime.tm_mon, gCurTime.tm_mday);
				sprintf(buf+16," %02d:%02d:%02d %s\r\n",gCurTime.tm_hour,gCurTime.tm_min,gCurTime.tm_sec, FormatStatus(gOptions.AttState));
				SerialOutputString(&ff232, buf);
			}
#endif //MODULE
	}

	if (gOptions.I1ToG && g1ToG)                                        
	{
		if (GroupFpCount[gInputPin-1] > gOptions.GroupFpLimit)
		{
			LCDClearLine(1);
			LCDWriteCenterStrID(1,MID_GROUPFPOVER);
		}
	}
	g1ToG = FALSE;

	if(u && LogState!=LOG_INVALIDUSER)
	{
		char *sms;
		sms=GetSMS(u);
		if(sms)	//显示短消息
		{
			LCDWriteLine(0,sms);
			LCDWriteStr(0,0,sms, LCD_WRAP|LCD_BOTTOM_LINE);
		}
	}
	//发送给Wiegand
/*	memcpy(&tempValue, (void*)(u->IDCard), 3);
	if(!gOptions.WiegandID || !tempValue )
	{
		if((gOptions.PIN2Width>PIN_WIDTH) && (u->PIN2>0))
			WiegandSend(gWGSiteCode<0?gOptions.DeviceID:gWGSiteCode, u->PIN2 ,0, NULL);
		else
			WiegandSend(gWGSiteCode<0?gOptions.DeviceID:gWGSiteCode, pin,0, NULL);
	}else
		WiegandSend(gWGSiteCode<0?gOptions.DeviceID:gWGSiteCode, tempValue,0, NULL);*/
	//发送事件消息
	state=getCurrentState();
	if(gOptions.TagLogDoorAction)
	{
		state|=IsOpenDoorAction<<2;	//状态为3个字节，最高字节为是否开锁动作
	}
	if (gOptions.PIN2Width <= PIN_WIDTH)
	{   
	    SET_WORD(buf,(U16)pin,0);
	    buf[2]=VerifiedMethod;
	    buf[3]=state|(char)(!LogState?LOG_VALID:(1<<7));
	    buf[4]=gCurTime.tm_year-2000;
	    buf[5]=gCurTime.tm_mon;
	    buf[6]=gCurTime.tm_mday;
	    buf[7]=gCurTime.tm_hour;
	    buf[8]=gCurTime.tm_min;
	    buf[9]=gCurTime.tm_sec;
	    CheckSessionSend(EF_ATTLOG,buf,10);
	}
	else
	{
	    //memcpy(buf,&(u->PIN2),4);
	    SET_DWORD(buf,(U32)(u->PIN2),0);
	    buf[4]=VerifiedMethod;
	    buf[5]=state|(char)(!LogState?LOG_VALID:(1<<7));
	    buf[6]=gCurTime.tm_year-2000;
	    buf[7]=gCurTime.tm_mon;
	    buf[8]=gCurTime.tm_mday;
	    buf[9]=gCurTime.tm_hour;
	    buf[10]=gCurTime.tm_min;
	    buf[11]=gCurTime.tm_sec;
	    CheckSessionSend(EF_ATTLOG,buf,12);
	    
	}
	if(gOptions.MustChoiceInOut) gOptions.AttState=-1;
	LastTime=t;
	LastUID=pin;
	if(user && u)
		memcpy((void*)user,(void*)u,sizeof(TUser));
//	if(!gMasterSlaveProcess || gOptions.ShowSlaveOperation)
	L3000ChangeMethod(VerifiedMethod);
	ShowResult(pin,u,LogState);

//	GetTime(NULL, TRUE);	//更新时间，避免连续按指纹时产生相同时间记录的问题
	return LogState;
}

int SaveNidekaAttLog(U16 pin, int VerifiedMethod)
{
	BYTE AttStateTable[6]={0,1,4,5,2,3};	
	char name[20],buf[32], format[32];
	char tmp1[50],tmp2[50],tmp3[12];
	int LogState=LOG_VALID;
	PUser u;
	time_t t;
	int c,hintrow=gLCDRowCount-1;
	unsigned char smsContent[64];
	int workcode;
	int wcret=News_CommitInput;
	int CurPos;
	BYTE CurAttState;
	U32 pin2;
	int group=0;
	BOOL OpenDoorSign=FALSE;
		
	GetTime(&gCurTime);
	t=OldEncodeTime(&gCurTime);

	
	for(CurPos=0;CurPos<gOptions.MaxUserCount*100;CurPos++)
	{
		if(CurAlarmRec[CurPos].PIN==pin)
		{
			if(gOptions.AlarmReRec&&(t-CurAlarmRec[CurPos].LastTime)<gOptions.AlarmReRec*60)
				LogState|=LOG_REPEAT;
			break;
		}
		else if(CurAlarmRec[CurPos].PIN==0)
		{
			break;
		}
	}
	DBPRINTF("CurPos=%d CurPin=%d\n", CurPos, CurAlarmRec[CurPos].PIN);

	u=FDB_GetUser(pin, NULL);
	if(u) group=u->Group&0x0F;
	LCDClear();
	LCDWriteLineStrID(0, MID_OA_VSHINT);
	if(!gOptions.MustEnroll || u)
	{
		if(gOptions.MustEnroll)
		{
			if((u&&ISINVALIDUSER(*u))||
			   (gOptions.DisableAdminUser&&(u&&ISADMIN(u->Privilege)))||
			   (gOptions.DisableNormalUser&&(u&&!ISADMIN(u->Privilege))))
			{
				LCDWriteCenterStrID(gLCDRowCount-1,HID_PRI_INVALID);
				if(u && (u->PIN2))
					ShowUserHint(gLCDRowCount/2, TRUE, u->PIN2);
				else
					ShowUserHint(gLCDRowCount/2, FALSE, pin);
				LogState|=LOG_INVALIDUSER;
			}
		}
		if(gLCDRowCount<4) hintrow=0;
		if(!(LogState&LOG_INVALIDUSER))
		{
			//show user info    
			if(gOptions.ShowName && u && u->Name[0])
			{


		               if(gOptions.ShowSecond)
                		        sprintf(tmp1, " %02d:%02d:%02d", gCurTime.tm_hour,gCurTime.tm_min, gCurTime.tm_sec);
                		else
		                        sprintf(tmp1, " %02d:%02d", gCurTime.tm_hour,gCurTime.tm_min);
                		//PadRightStrStr(tmp2,GetAttName(gOptions.AttState),tmp1, gLCDCharWidth);
				if (gOptions.AttState >=1)
                			PadRightStrStr(tmp2,LoadStrByID(NID_COUT1),tmp1, gLCDCharWidth);
				else
                			PadRightStrStr(tmp2,LoadStrByID(NID_CIN1),tmp1, gLCDCharWidth);
				LCDClearLine(0);
				LCDWriteStr(0,0,tmp2,0);
				LCDClearLine(1);
				if (gOptions.AttState >=1)
	                                LCDWriteStr(1,0,LoadStrByID(NID_COUT),0);
				else
					LCDWriteStr(1,0,LoadStrByID(NID_CIN),0);


				if (gLCDRowCount>2)
				{
					LCDClearLine(2);
					Pad0Num(tmp3,gOptions.PIN2Width,pin);
					if(u->PIN2)
						Pad0Num(tmp3,gOptions.PIN2Width,u->PIN2);
					else
						Pad0Num(tmp3,gOptions.PIN2Width,pin);
					LCDWriteStr(2,0,tmp3,0);
				}
				memset(name, 0, 20);
				nstrcpy(name, u->Name, MAXNAMELENGTH);
				LCDClearLine(3);
				LCDWriteCenterStr(3, name);
			}
			else
			{

				
                               if(gOptions.ShowSecond)
                                        sprintf(tmp1, " %02d:%02d:%02d", gCurTime.tm_hour,gCurTime.tm_min, gCurTime.tm_sec);
                                else
                                        sprintf(tmp1, " %02d:%02d", gCurTime.tm_hour,gCurTime.tm_min);
                                //PadRightStrStr(tmp2,GetAttName(gOptions.AttState),tmp1, gLCDCharWidth);
				if (gOptions.AttState >=1)
                			PadRightStrStr(tmp2,LoadStrByID(NID_COUT1),tmp1, gLCDCharWidth);
				else
                			PadRightStrStr(tmp2,LoadStrByID(NID_CIN1),tmp1, gLCDCharWidth);
                                LCDClearLine(0);
                                LCDWriteStr(0,0,tmp2,0);
                                LCDClearLine(1);
				if (gOptions.AttState >=1)
	                                LCDWriteStr(1,0,LoadStrByID(NID_COUT),0);
				else
					LCDWriteStr(1,0,LoadStrByID(NID_CIN),0);


                                if (gLCDRowCount>2)
                                {
                                        LCDClearLine(2);
                                        if(u->PIN2)
                                                Pad0Num(tmp3,gOptions.PIN2Width,u->PIN2);
                                        else
                                                Pad0Num(tmp3,gOptions.PIN2Width,pin);
                                        LCDWriteCenterStr(2,tmp3);
                                }

				LCDClearLine(3);
			}
			if(gOptions.LockFunOn&LOCKFUN_ADV)
			{
				if(!TestUserTZ(pin, gCurTime))
				{
					LogState|=LOG_INVALIDTIME;
					LCDWriteCenterStrID(hintrow, HID_INVALIDTIME);
				}
			}
			
			workcode=0;
			if((LogState<=LOG_REPEAT)&&gOptions.WorkCode)
			{
				wcret=InputWorkCode(pin, u, &workcode);
			}
			if((LogState<=LOG_REPEAT)&&gOptions.LockFunOn)
			{
				if(!(gOptions.MustChoiceInOut && (gOptions.AttState<0)))
				{
					DelayNextUser=0;
					c=TestOpenLock(pin, gCurTime, VerifiedMethod);
					if(c>0)	
					{
						OpenDoorSign=TRUE;
						if(gOptions.LockOn)
							//ExAuxOut(gOptions.LockOn, gOptions.OpenDoorDelay);
							DoAuxOut(gOptions.LockOn, gOptions.OpenDoorDelay);
					}
					else
					{
						if(c<0)
						{
							LCDWriteCenterStrID(hintrow,HID_MUSER_OPEN1);
							DelayNextUser=DELAYNEXTUSER;
						}
						else
							LCDWriteCenterStrID(hintrow,HID_MUSER_OPEN2);
						LogState|=LOG_INVALIDCOMBO;
					}
				}
			}
		}	
		
		if(LogState>LOG_REPEAT)
		{
			ExLightLED(LED_GREEN, FALSE);
			ExLightLED(LED_RED, TRUE);
			ExBeep(2);
		}	
		else		
		{
			if(gOptions.MustChoiceInOut && (gOptions.AttState<0))
			{
				LCDWriteCenterStrID(hintrow, HID_MUSTINOUT);
				ExBeep(1);
			}
			else
			{
				if(LogState==LOG_REPEAT)
				{
					//LCDWriteCenterStrID(hintrow, HID_ALREADY);
					LCDClearLine(0);
					memset(tmp2,0,50);
                                	PadRightStrStr(tmp2,LoadStrByID(HID_ALREADY),GetAttName(gOptions.AttState),gLCDCharWidth);
					LCDClearLine(1);
					LCDWriteStr(1,0,tmp2,0);
                                        LCDClearLine(2);
					memset(tmp3,0,50);
                                        if(u->PIN2)
                                                Pad0Num(tmp3,gOptions.PIN2Width,u->PIN2);
                                        else
                                                Pad0Num(tmp3,gOptions.PIN2Width,pin);
                                        LCDWriteCenterStr(2,tmp3);

					LCDClearLine(3);
					if(gOptions.ShowName && u && u->Name[0])	
					{		
						memset(name, 0, 20);
						nstrcpy(name, u->Name, MAXNAMELENGTH);
						LCDWriteCenterStr(3, name);
					}
					

					if(gOptions.VoiceOn) 
						ExPlayVoice(VOICE_ALREADY_LOG);
					else
						ExBeep(1);
				}
				else
				{
					//LCDWriteCenterStrID(hintrow, HID_VSUCCESS);
					if(gOptions.VoiceOn) 
						PlayVoiceByTimeZone(gCurTime, group, VOICE_THANK);
					else
						ExBeep(1);
				}
			}
		}	
				
		if(!(LogState&LOG_INVALIDUSER))
		{
			if(CurAttLogCount>=gOptions.MaxAttLogCount*10000)
			{
				DelayMS(2*1000);
				LCDWriteCenterStrID(gLCDRowCount/2,HID_EXCEED);
				if(gOptions.VoiceOn) ExPlayVoice(VOICE_NO_LOG_RECSPACE);
				return 0;
			}
			if(CurAttLogCount+gOptions.AlarmAttLog>=gOptions.MaxAttLogCount*10000)
			{
				DelayMS(2*1000);
				LCDWriteCenterStrID(gLCDRowCount/2,HID_LEFTSPACE);
				sprintf(name, "%d",gOptions.MaxAttLogCount*10000-FDB_CntAttLog());
				LCDWriteCenterStr(gLCDRowCount-1, name);
			}
			if(u) 
				pin2=u->PIN2;
			else
				pin2=0;			 
			if(gOptions.SaveAttLog&&(LogState!=LOG_REPEAT)&&
			   (!gOptions.WorkCode||(wcret==News_CommitInput)||(wcret==News_TimeOut)))
			{
				if((LogState==LOG_VALID) && !(gOptions.MustChoiceInOut && (gOptions.AttState<0)))
				{
					CurAttState=(char)gOptions.AttState;
					if(CurAttState<=5)
						CurAttState=AttStateTable[CurAttState];
				}	
				else
					CurAttState=(char)7;
				
				if(FDB_AddAttLog((U16)pin, t, (char)VerifiedMethod, CurAttState, pin2, workcode, gSensorNo)==FDB_OK)
				{
					//Attlog already saved
					if(CurPos<gOptions.MaxUserCount*100)
					{
						CurAlarmRec[CurPos].PIN=pin;
						CurAlarmRec[CurPos].LastTime=t;
					}
				}							
				//sync(); //flush buffer data to disk
			}	
			
			//发送给Wiegand
			if((gOptions.PIN2Width>PIN_WIDTH) && (pin2>0))
				WiegandSend(gWGSiteCode<0?gOptions.DeviceID:gWGSiteCode, pin2, 0);
			else
				WiegandSend(gWGSiteCode<0?gOptions.DeviceID:gWGSiteCode, pin, 0);
			
			if(gOptions.RS232Fun==2)//简单输出ID号
			{				
				sprintf(format, "+%%0%dd\r\n", gOptions.PIN2Width);
				if(u->PIN2)
					sprintf((char*)buf, format, u->PIN2);
				else
					sprintf((char*)buf, format, u->PIN);
				SerialOutputString(&ff232, (char*)buf);
			}		
			LastTime=t;
			LastUID=pin;		
			//Display SMS	
			if(gOptions.IsSupportSMS)
			{
				memset(smsContent, 0, 64);
				if (CheckUserSMS(pin, smsContent))
				{
					DelayMS(500);
					LCDClear();
					LCDWriteStr(0,0,smsContent, LCD_WRAP);
					ShowMainLCDDelay=10;		
				}
			}
		}
		memcpy(buf, &pin, 2);
		buf[2]=VerifiedMethod;
		buf[3]=gOptions.AttState|(char)((LogState<=LOG_REPEAT)?LOG_VALID:(1<<7));
		buf[4]=gCurTime.tm_year-100;
		buf[5]=gCurTime.tm_mon+1;
		buf[6]=gCurTime.tm_mday;
		buf[7]=gCurTime.tm_hour;
		buf[8]=gCurTime.tm_min;
		buf[9]=gCurTime.tm_sec;	
		CheckSessionSend(EF_ATTLOG, buf, 10);		
	}
	else
	{
		ExBeep(2);
		ExLightLED(LED_RED, TRUE);		
		ExLightLED(LED_GREEN, FALSE);	
		LCDWriteCenterStrID(gLCDRowCount-1, HID_UIDERROR);
		if(gOptions.RS232Fun==2)//简单输出ID号
		{
			sprintf((char*)buf, "-%d\r\n", pin);
			SerialOutputString(&ff232, (char*)buf);
		}
	}
	if(gOptions.MustChoiceInOut) gOptions.AttState=-1;
	return 0;
}


int SaveAuthServerLog(U32 pin,int VerifiedMethod)
{
	BYTE AttStateTable[6]={0,1,4,5,2,3};	
	int workcode=0;
	time_t t;
	BYTE CurAttState;
		
	GetTime(&gCurTime);
	t=OldEncodeTime(&gCurTime);

	if(gOptions.SaveAttLog)
	{
		if(!(gOptions.MustChoiceInOut && (gOptions.AttState<0)))
		{
			CurAttState=(char)gOptions.AttState;
			if(CurAttState<=5)
				CurAttState=AttStateTable[CurAttState];
		}	
		else
			CurAttState=(char)7;
		FDB_AddAttLog(pin, t, (char)VerifiedMethod, CurAttState, pin, workcode, gSensorNo);
		//sync();
	}
	if(gOptions.MustChoiceInOut) gOptions.AttState=-1;
	return 0;
}

void CheckAdmin(void)
{
    int i;

    if (FDB_CntUser() > 0)
    {
        if (gState.pwr_bak)
        {
            L3000ShowWarn(NULL, L3000_STR_REPLACE1, 0, 2);			
            WaitAdminVerifyCount=0;
            WaitAdminVerifySecond=0;			
            KeyBufferTimeOut=0;
            HackerNumber=0;
            HackerWait=0;
            KeyBuffer[0]=0;
            return ;
        }
        else if (gState.voltage >= SW_LOW_PWR_ALM)
        {
            L3000ShowWarn(NULL, LoadStrByID(HID_ALARM_LOWBATTERY), 0, 2);
            WaitAdminVerifyCount=0;
            WaitAdminVerifySecond=0;			
            KeyBufferTimeOut=0;
            HackerNumber=0;
            HackerWait=0;
            KeyBuffer[0]=0;
            gShowLowPwrAlm = 0;
            return ;
        }
        else ;
    }

    for (i=0;i<MaxAdminVerify;i++)
        WaitAdmins[i]=0;

    WaitAdminVerifyCount=COUNT_RETRY_ADMIN;
    WaitAdminVerifySecond=TIMEOUT_WAIT_ADMIN_VERIFY;
    i=FDB_CntAdminUser();
    WaitAdminRemainCnt=i;
    if (WaitAdminRemainCnt>gOptions.AdminCnt)
	{
        WaitAdminRemainCnt=gOptions.AdminCnt;	
	}
    if ((i==0) || HackerWait)//Hacker状态,以超级管理员进入菜单的后门程序
    {
        if((gAlarmStrip>ALARMSTRIPTAG+30) && (gAlarmStrip<ALARMSTRIPTAG+40))
            HackerNumber=1;
        AdminUser=NULL;
        WaitAdminVerifySecond=0;
        ShowMainLCDDelay=0;
#ifndef MODULE
        if (gOptions.C2FunOn)
            ExSetMonitorC2(FALSE);
        DoMainMenu();
        if (gOptions.C2FunOn)
            ExSetMonitorC2(TRUE);
#endif
        ExSetAuxOutDelay(gOptions.LockOn,gOptions.OpenDoorDelay, gOptions.DoorSensorMode);
        if (!gOptions.PageState)
            gOptions.AttState=GetNowState();
        if(gOptions.MustChoiceInOut)
            gOptions.AttState=-1;
        WaitAdminVerifyCount=0;
        KeyBufferTimeOut=0;
    }
    else
    {
#ifndef MODULE
        ShowMainLCD();
#endif
    }
    HackerNumber=0;
    HackerWait=0;
    KeyBuffer[0]=0;
}

int SumNum(int i)
{
	int ret=0;
	while(i)
	{
		ret+=i % 10;
		i/=10;
	}
	return ret;
}

static int OldAttState=0;

BOOL ProcStateKey(int i)
{
	if(WaitShowState==0) OldAttState=gOptions.AttState;
	if(i>=IKeyIn)
	{
		int MaxState=i-IKeyIn;
		if(gOptions.AttState!=MaxState)
		{
			gOptions.AttState=MaxState;
			if(gOptions.MustChoiceInOut) WaitShowState=TIMEOUT_SHOWSTATE;
			ShowMainLCD();
                        return TRUE;
		}					
	}
	else
	{
		gOptions.AttState=2+i-IKeyOTIn;
		WaitShowState=TIMEOUT_SHOWSTATE;
		ShowMainLCD();
		return TRUE;		
	}
	DBPRINTF("ATTSTATE=%d\n", gOptions.AttState);
	return FALSE;
}

void POWEROFFSTART(void)
{

	LCDClear();
	HackerNumber=0;
	HackerWait=0;
	ExBeep(2);
	LCDWriteCenterStrID(1,HID_PREPOWEROFF);
	WaitPowerOff=3;
	ClockEnabled=FALSE;
	PowerSuspend=FALSE;
}

void WakeUpFromSleepStatus(void)
{
	if (!ShowMainLCDEnabled)
	{

		//whether display clock ":" or not
		ClockEnabled = TRUE;		
		//whether display main windows or not
		ShowMainLCDEnabled = TRUE;
		GPIO_HY7131_Power(TRUE);
		GPIO_LCD_USB0_Power(TRUE);
		EnableMsgType(MSG_TYPE_FINGER, TRUE);					
		WaitInitSensorCount=LOADDRIVERTIME;
		ShowMainLCD();
		
		mdelay(100);
		//enabled mute
		GPIO_AC97_Mute(TRUE);	
	}
}

U32 CheckUserPIN(char *pin)
{
	U32 p;
	PUser u;
	
	if(0==strtou32(pin, &p))
	{
		if(gOptions.PIN2Width==PIN_WIDTH)
		{
			u=FDB_GetUser((U16)p, NULL);
		}
		else
		{
			u=FDB_GetUserByPIN2(p, NULL);
		}
	}
	else
		u=NULL;
	if(u)
		return p;
	else
		return -1;
}

BYTE GetExtUserVSByPIN(U32 pin, int msgtype)
{
	U16 p;	
	PExtUser extuser=NULL;
	char tmp[16];
	BYTE vs;
	
	p=GetUserPIN_16(pin);
	if(gOptions.UserExtendFormat)
	{
		if(p)
		{
			extuser=FDB_GetExtUser(p, NULL);
		}
		if(extuser)
		{	
			if(extuser->VerifyStyle&0x80)
				return extuser->VerifyStyle&0x7F;
		}
	}
	//default value
	vs=VS_FP_OR_PW_OR_RF;	
	//use Group Verify Type
	if(p&&gOptions.UserExtendFormat)
	{
		sprintf(tmp, "GVS%d", GetUserGrp(p));
		vs=LoadInteger(tmp, vs);
	}
	else
	{
                //如果不使用extend模式的，默认原来 2006.08.24
		switch(msgtype)
		{
			case News_VerifiedByPwd:
				vs = VS_PW;
				break;
			case News_VerifiedByFP:
			case News_VerifiedByFPRIS:
                                if ((!gOptions.OnlyPINCard) && (gOptions.Nideka))
                                        vs=VS_FP_AND_RF;
                                else
					vs = VS_FP;
				break;
			case News_VerifiedByIDCard:
			case News_VerifiedByIDCardRIS:
			case News_VerifiedByMFCard:
				if(!gOptions.OnlyPINCard)
					vs=VS_FP_AND_RF;
				else
					vs = VS_RF;
				break;			
		}
	}
	return vs;
}

int CheckInputPassword(U32 pin2)
{
	PUser u;
	int result=-1,ret;
	unsigned char box_t[sizeof(TInputBox)];
	PInputBox box = (PInputBox)box_t;
		
	LCDClear();
	LCDWriteLineStrID(0, HID_VERINPUTPWD);
	u=FDB_GetUser(GetUserPIN_16(pin2), NULL);	
	memset(box, 0, sizeof(TInputBox));
	box->MaxLength=5;
	box->Row=gLCDRowCount/2;
	box->Col=strlen(LoadStrByID(HID_INPUTPWD))+2;
	box->PasswordChar='*';
	box->Width=5;
	box->AutoCommit=TRUE;
	ret=-1;
	LCDClearLine(gLCDRowCount/2);
	LCDWriteStr(gLCDRowCount/2, 1, LoadStrByID(HID_INPUTPWD), 0);
	ShowMainLCDDelay=1000;
	EnableMsgType(MSG_TYPE_FINGER, FALSE);
	EnableMsgType(MSG_TYPE_MF, FALSE);
	EnableMsgType(MSG_TYPE_HID, FALSE);
	ret=RunInput(box);
	EnableMsgType(MSG_TYPE_FINGER, TRUE);
	EnableMsgType(MSG_TYPE_MF, TRUE);
	EnableMsgType(MSG_TYPE_HID, TRUE);
	if(News_CommitInput==ret)
	{
		if(u->Password[0]&&nstrcmp(box->Text, u->Password, 5)==0)
			result=pin2;
		else 
			result=-2;
	}
	return result;
}

void ClearVerifyType(void)
{
	FPData.TempSize=0;
	FPData.PIN=0;
	memset(FingerTemplate, 0, sizeof(FingerTemplate));
	KeyBufferTimeOut=0;
	KeyBufferIndex=0;
	memset(KeyBuffer, 0, 20);
//	memset((void*)&AuthFPData, 0, sizeof(TFPResult)); //treckle
	memset((void*)&VSStatus, 0, sizeof(TVSStatus));
	WaitVerifyRetryCnt=COUNT_RETRY_USER;
	WaitVerifyTypeIdleTime=0;
}

BOOL CheckNextVerifyType(PMsg msg, TVSStatus VSStatus, TVSStatus CurVSStatus, BOOL IsANDOperator)
{
	BOOL result=FALSE;
	int rc;
	
//	LCDClear();
	//PIN CARD PWD FP
	if(IsANDOperator||!((CurVSStatus.PIN&&VSStatus.PIN)||(CurVSStatus.Card&&VSStatus.Card)||
			    (CurVSStatus.Password&&VSStatus.Password)||(CurVSStatus.FP&&VSStatus.FP)))
	{
		ShowMainLCDDelay=10;
		if(CurVSStatus.PIN&&!VSStatus.PIN)
		{
			LCDClearLine(gLCDRowCount/2);
			ShowUserHint(gLCDRowCount/2, gOptions.PIN2Width>PIN_WIDTH, FPData.PIN);
			LCDWriteLineStrID(0, HID_ENROLLNUM);
			LCDWriteCenterStrID(gLCDRowCount-1, HID_OS_MUST1TO1);
		}				
		else if(CurVSStatus.Password&&!VSStatus.Password&&
			(IsANDOperator||(FDB_GetUser(GetUserPIN_16(FPData.PIN), NULL)->Password[0])))
		{
			rc=CheckInputPassword(FPData.PIN);
			if(rc>0)
				ConstructMSG(msg, MSG_TYPE_CMD, News_VerifiedByPwd, FPData.PIN);
			else
			{
				if(rc==-1)
				{
					ShowMainLCDDelay=0;
					ClearVerifyType();
					ShowMainLCD();
				}
				else ConstructMSG(msg, MSG_TYPE_CMD, News_FailedByPwd, 0);
			}
		}
		else if(CurVSStatus.FP&&!VSStatus.FP&&!gOptions.IsOnlyRFMachine)
		{	
			LCDClearLine(gLCDRowCount/2);
			ShowUserHint(gLCDRowCount/2, gOptions.PIN2Width>PIN_WIDTH, FPData.PIN);
			if (!gOptions.Nideka)
			{
				LCDWriteLineStrID(0, HID_VF);
				LCDWriteCenterStrID(gLCDRowCount-1, HID_PLACEFINGER);
			}
			else
			{
				char tmp1[32];
				char tmp2[32];
				if (gOptions.AttState >=1)
				{
					sprintf(tmp1,"%s %s",LoadStrByID(NID_OUT),LoadStrByID(NID_FP));
	                                LCDWriteCenterStr(0,tmp1);
					sprintf(tmp2,"%s %s",LoadStrByID(NID_OUT1),"Punch FP");
	                                LCDWriteCenterStr(3,tmp2);
				}
				else
				{
					sprintf(tmp1,"%s %s",LoadStrByID(NID_IN),LoadStrByID(NID_FP));
	                                LCDWriteCenterStr(0,tmp1);
					sprintf(tmp2,"%s %s",LoadStrByID(NID_IN1),"Punch FP");
	                                LCDWriteCenterStr(3,tmp2);
				}
					
			}
		}
		else if(CurVSStatus.Card&&!VSStatus.Card&&!gOptions.IsOnlyRFMachine)
		{
			LCDClearLine(gLCDRowCount/2);
			ShowUserHint(gLCDRowCount/2, gOptions.PIN2Width>PIN_WIDTH, FPData.PIN);
			LCDWriteLineStrID(0, HID_CARD);
			LCDWriteCenterStrID(gLCDRowCount-1, HID_SHOWCARD);
		}
		else
		{
			LCDClearLine(gLCDRowCount/2);
			ShowUserHint(gLCDRowCount/2, gOptions.PIN2Width>PIN_WIDTH, FPData.PIN);
			if(CurVSStatus.Password&&!VSStatus.Password)
			{
				LCDWriteLineStrID(0, HID_VERINPUTPWD);
				WaitVerifyRetryCnt=1;
				ConstructMSG(msg, MSG_TYPE_CMD, News_FailedByPwd, 0);				
			}
			else
			{
				result=TRUE;
				ShowMainLCDDelay=0;
			}
		}
	}
	else
		result=TRUE;
	return result;
}

void DisplayUserInfo(U32 pin, char *name)
{
	int hintrow=0;

	char tmp1[50],tmp2[50],tmp3[50];

	if (gOptions.Nideka)
	{

			        if(gOptions.ShowSecond)
                                        sprintf(tmp1, " %02d:%02d:%02d", gCurTime.tm_hour,gCurTime.tm_min, gCurTime.tm_sec);
                                else
                                        sprintf(tmp1, " %02d:%02d", gCurTime.tm_hour,gCurTime.tm_min);
                                if (gOptions.AttState >=1)
                                	PadRightStrStr(tmp2,LoadStrByID(NID_COUT1),tmp1, gLCDCharWidth);
                                else
                                	PadRightStrStr(tmp2,LoadStrByID(NID_CIN1),tmp1, gLCDCharWidth);
                                LCDClearLine(0);
                                LCDWriteStr(0,0,tmp2,0);
                                LCDClearLine(1);
                                if (gOptions.AttState >=1)
                                        LCDWriteStr(1,0,LoadStrByID(NID_COUT),0);
                                else
                                        LCDWriteStr(1,0,LoadStrByID(NID_CIN),0);


                                if (gLCDRowCount>2)
                                {
                                        LCDClearLine(2);
                                        Pad0Num(tmp3,gOptions.PIN2Width,pin);
                                        LCDWriteStr(2,0,tmp3,0);
                                }
                                LCDClearLine(3);
                                LCDWriteCenterStr(3, name);
 		               if(gOptions.MustChoiceInOut && (gOptions.AttState<0))
                		{
		                        LCDWriteCenterStrID(hintrow, HID_MUSTINOUT);
                		        ExBeep(1);
		                }
				else
				{
					if(gOptions.VoiceOn)
						ExPlayVoice(VOICE_THANK);
					else
						ExBeep(1);
					
				}

	}
	else
	{
	
		LCDWriteLineStrID(0, MID_OA_VSHINT);
		LCDWriteCenterStr(1, name);
		if (gLCDRowCount>2)
		{
			LCDClearLine(gLCDRowCount/2);
			ShowUserHint(gLCDRowCount/2, gOptions.PIN2Width>PIN_WIDTH, pin);
			hintrow=gLCDRowCount-1;
		}
		else
			hintrow=0;
		if(gOptions.MustChoiceInOut && (gOptions.AttState<0))
		{
			LCDWriteCenterStrID(hintrow, HID_MUSTINOUT);
			ExBeep(1);
		}
		else
		{
			if(gOptions.MustChoiceInOut) gOptions.AttState=-1;
		
			LCDWriteCenterStrID(hintrow, HID_VSUCCESS);
			if(gOptions.LockFunOn)
			{
				if(gOptions.LockOn)
					//ExAuxOut(gOptions.LockOn, gOptions.OpenDoorDelay);
					DoAuxOut(gOptions.LockOn, gOptions.OpenDoorDelay);
			}
			//发送给Wiegand
			WiegandSend(gWGSiteCode<0?gOptions.DeviceID:gWGSiteCode, pin, 0);
			if(gOptions.VoiceOn)
				ExPlayVoice(VOICE_THANK);
			else
				ExBeep(1);
		}
	}
}

int MainProcMsg(PMsg msg)
{
	static	int oled_tick = 0;
	static	int led_flash = 0;
	int message=msg->Message;
	int i;
	static int LastKey=0;
	static int ii=0;
	U16 pin;
	ii++;
	msg->Message=0;
	if(message!=MSG_TYPE_TIMER)
	{
		SpeedHiDelay=CPUHISPEEDDELAY;
		SetCPUSpeed(gOptions.CPUFreq);
	}

	switch(message)
	{
#ifdef WEBSERVER
        case MSG_WEB_PROCESS:  /******** Add For Web Server ********/
                WebProcess();
                break;
#endif		
	case MSG_TYPE_TIMER:
		L3000ProcSecondMsg(msg); //connecting....... don't sleep and power off
		if (gOptions.PowerMngFunOn && (CommSessionCount>0)) 
		{
			WaitPowerOff=0;
		}
        if(gCloseDoorDelay)
        {
			if((gCloseDoorDelay%5)==0) 
			{
				//char buf[20];
				LCDWriteCenterStrID(0,HID_DOOR_OPEN);
				//sprintf(buf,"gclosedd=%d",gCloseDoorDelay);
				//LCDWriteStr(1, 0, buf, 0);
				//ExBeep(10);
			}
            if(!--gCloseDoorDelay)
            {
				ConstructMSG(msg, MSG_TYPE_CMD, News_Door_Sensor, DOOR_SENSOR_BREAK);
            }
        }
  		if((gOptions.CPUFreq==-1) && SpeedHiDelay)
		{
			if(!--SpeedHiDelay)
			{
				SetCPUSpeed(0);
			}
		}
		else if(KeyBufferTimeOut==0 && ShowMainLCDDelay==0)
		{
			ShowMainLCD();
#ifdef MODULE
			if(gMachineState!=STA_IDLE)
#else
			if(gMachineState==STA_IDLE)
#endif
			{
				/*ExLightLED(LED_GREEN, FlashGreenLED);
				ExLightLED(LED_RED, FlashGreenLED);
				FlashGreenLED=!FlashGreenLED;*///not flash led after cancel operation
				//DebugOutput1("IDLE FlashLED=%d\n", FlashGreenLED);
			}
			else if((gOptions.FlashLED & 0xFF) ==1)
			{
				ExLightLED(LED_RED, 0!=(gOptions.FlashLED & 0x100));
				ExLightLED(LED_GREEN, FlashGreenLED);
				FlashGreenLED=!FlashGreenLED;
				//DebugOutput2("1:FlashLED=%d, %d\n", FlashGreenLED, gOptions.FlashLED);
			}
			else if((gOptions.FlashLED & 0xFF) ==2)
			{
				ExLightLED(LED_GREEN, 0!=(gOptions.FlashLED & 0x100));
				ExLightLED(LED_RED, FlashGreenLED);
				FlashGreenLED=!FlashGreenLED;
				//DebugOutput1("2:FlashLED=%d\n", FlashGreenLED);
			}
		}
#ifndef MODULE
		if(PrepareSecondFun && SecondFunction)
		{
			if(!--PrepareSecondFun)
			{
/*				switch(SecondFunction)
				{
				case IKeyPower:
					//作为第二功能启动
					if(!gOptions.NoPowerKey && !gOptions.LockPWRButton)
					{
						POWEROFFSTART
					}
					break;
				case IKeyMenu:
					CheckAdmin();
					break;
				case IKeyDuress:
					TriggerDuress(0,0);
					break;
				}
				KeyBufferTimeOut=0;
				KeyBufferIndex=0;
*/
			}
		}
		if(KeyBufferTimeOut)
		{
			if(!--KeyBufferTimeOut)
			{
				ExBeep(6);
				KeyBufferIndex=0;
				ShowMainLCD();
			}
		}
	/*	if(WaitAdminVerifySecond)
		{
			if(WaitAdminVerifyCount && !--WaitAdminVerifySecond)
			{
				ExBeep(6);
				WaitAdminVerifyCount=0;
				ShowMainLCD();
			}
		}
	*/
		if(DelayNextUser)
		{
			if(!--DelayNextUser)
			{
				ClearMultiOpen();
				ShowMainLCD();
			}
		}
		if(WaitPowerOff)
		{
			if(!--WaitPowerOff)
			{
				if(gOptions.C2FunOn && gOptions.ExternalAC==0)
				{
					if(ExSetMonitorC2(FALSE))	//停止C2监控
					{
						if((gBellDelay<=0) || (gBellDelay && ExBellC2(0)))	//如果正在响铃则关闭，避免一直响铃
						{
							ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Program, 0);
							return 1;
						}
					}
				}else
				{
					ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Program, 0);
					return 1;
				}
			}
			else if(WaitPowerOff<10)
			{
				char buffer[400];
				if(PowerSuspend)
					sprintf(buffer, LoadStrByID(HID_SUSPENDING), WaitPowerOff);
				else
					sprintf(buffer, LoadStrByID(HID_SHUTDOWNING), WaitPowerOff);
				LCDWriteCenterStr(0,buffer);
				//ExShowDevIcon(1,0);
			}
		}
		if(WaitShowState)
		{
			if(!--WaitShowState)	//状态等待超时
			{
				WaitShowState=0;
				if(gOptions.MustChoiceInOut)
					gOptions.AttState=-1;
				else if (!gOptions.PageState)
					gOptions.AttState=OldAttState;
				ShowMainLCD();
			}
		}
		if(gAlarmDelay)
		{
			if(!--gAlarmDelay)
			{
				DoAlarmOff(gAlarmDelayIndex);
			}
		}
		if(gBellDelay)
		{
			if(!--gBellDelay)
			{
				DoBellOff();
			}
		}
		if(WaitDuressAlarm)
		{
			if(!--WaitDuressAlarm)
			{
				DoAlarm(0, 24*60*60);
			}
		}
		if(DelayTriggerDuress) DelayTriggerDuress--;

		if(HackerWait) HackerWait--;

		if(gAlarmStrip>=ALARMSTRIPTAG)
		{//拆机报警已经起动，则开始计时
			if(++gAlarmStrip>ALARMSTRIPTAG+60) //计时满60秒则重置标志
				gAlarmStrip=0;
		}

		if(gLockDelay)
		{
			if(!--gLockDelay)
			{
				DoAuxOut(0,gOptions.OpenDoorDelay);
			}
		}
		if(gDoorSensorDelay)
		{
			if(!--gDoorSensorDelay)
			{
				//开始倒数计时60秒，逾期还不关门将产生报警信号
				gCloseDoorDelay=gOptions.DoorSensorTimeout;
				//ExBeep(10);
			}
		}
		if(gWaitSlaveEndIdle)
		{
			if(!--gWaitSlaveEndIdle && gMachineState==STA_IDLE)
				gMachineState=STA_VERIFYING;
		}
		if(gOptions.C2FunOn && WaitCheckC2 && !--WaitCheckC2)
		{
			char retbuf[10]={0};
			int i=QueryC2(retbuf);
			WaitCheckC2=CHECK_C2_INTERVAL;
			if(i>0)
			{
				if(i==DOOR_SENSOR_OPEN)
				{
					ConstructMSG(msg, MSG_TYPE_CMD, News_Door_Sensor, DOOR_SENSOR_OPEN);
					return 	MSG_TYPE_CMD;					
				}
				else if(i==DOOR_SENSOR_CLOSE)	//门磁：正常关门
				{
					ConstructMSG(msg, MSG_TYPE_CMD, News_Door_Sensor, DOOR_SENSOR_CLOSE);
					return MSG_TYPE_CMD;
				}
				else if(i==DOOR_BUTTON)
				{
					ConstructMSG(msg, MSG_TYPE_CMD, News_Door_Button,0);
					return MSG_TYPE_CMD;
				}else if(i==DOOR_SENSOR_BREAK)
				{
					ConstructMSG(msg, MSG_TYPE_CMD, News_Door_Sensor, DOOR_SENSOR_BREAK);
					return MSG_TYPE_CMD;
				}
			}
		}

		if(msg->Param2==Timer_Minute)
		{
			if(gCurTime.tm_min==0) //整点
				CheckRTCAdjust();
			//分钟数改变，更新液晶屏显示
		//	if(ClockEnabled)
		//	{
		//		ShowMainLCD();
		//		if(gOptions.MenuStyle==MenuStyle_ICON) if(TestEnabledMsg(MSG_TYPE_BUTTON)) ShowFPAnimate(80, 16);
		//	}
			if(gOptions.PowerMngFunOn)
			{
				//自动关机
				if((gOptions.AutoPowerOff & 0xFF)==gCurTime.tm_min &&
					(gOptions.AutoPowerOff >> 8)==gCurTime.tm_hour)
				{
					WaitPowerOff=10;
					PowerSuspend=FALSE;
				}
				//自动待机
				if((gOptions.AutoPowerSuspend & 0xFF)==gCurTime.tm_min &&
					(gOptions.AutoPowerSuspend >> 8)==gCurTime.tm_hour)
				{
					WaitPowerOff=10;
					PowerSuspend=TRUE;
				}
				//自动响铃
				if(gOptions.AutoAlarmFunOn && gOptions.AutoAlarmDelay)
				for(i=0;i<MAX_AUTOALARM_COUNT;i++)
				if((gOptions.AutoAlarm[i] & 0xFF)==gCurTime.tm_min &&
					(gOptions.AutoAlarm[i] >> 8)==gCurTime.tm_hour)
				{
					if(gOptions.AutoAlarmDelay) DoBell(gOptions.AutoAlarmDelay);
				}

				//自动转换状态
				if(gOptions.AutoStateFunOn)
				for(i=0;i<4*4;i++)
				if((gOptions.AutoState[i] & 0xFF)==gCurTime.tm_min &&
					(gOptions.AutoState[i] >> 8)==gCurTime.tm_hour)
				{
					int State[4]={HID_SCIN,HID_SCOUT,HID_SOCIN,HID_SOCOUT};
					gOptions.AttState=State[i/4]-HID_SCIN;
					if(gOptions.MustChoiceInOut) WaitShowState=TIMEOUT_SHOWSTATE;
					ShowMainLCD();
				}
				//夏令时支持
				if(gOptions.DaylightSavingTimeFun && gOptions.DaylightSavingTimeOn)
				if(gOptions.CurTimeMode==DAYLIGHTSAVINGTIME)
				{
					if(!IsDaylightSavingTime())
					{
						gCurTime.tm_hour-=1;
						SetCurrentTime(&gCurTime);
						gOptions.CurTimeMode=STANDARDTIME;
						SaveOptions(&gOptions);
						ShowMainLCD();	//更新液晶屏
					}
				}
				else if(gOptions.CurTimeMode==STANDARDTIME)
				{
					if(IsDaylightSavingTime())
					{
						gCurTime.tm_hour+=1;
						SetCurrentTime(&gCurTime);
						gOptions.CurTimeMode=DAYLIGHTSAVINGTIME;
						SaveOptions(&gOptions);
						ShowMainLCD();
					}
				}else if(gOptions.CurTimeMode==0)
				{
					if(IsDaylightSavingTime())
						gOptions.CurTimeMode=DAYLIGHTSAVINGTIME;
					else
						gOptions.CurTimeMode=STANDARDTIME;
					SaveOptions(&gOptions);
				}

				//若为从机且没有建立连接则重试连接。
//				if(gOptions.MasterSlaveFunOn && gOptions.MasterSlaveOn==2 && !gMasterConnected)
//					gMasterConnected=ConnectMaster(gSlave232);
			}
		}
		else if(msg->Param2==Timer_Second)
		{
			CheckSessionSendMsg();
		//	if(ClockEnabled) if(gOptions.MenuStyle==MenuStyle_ICON) if(TestEnabledMsg(MSG_TYPE_BUTTON)) ShowFPAnimate(80, 16);
//			DebugOutput2("S: %02d, %d\n", gCurTime.tm_sec, gOptions.ShowSecond);
#if 0
			if(ClockEnabled && gOptions.ShowSecond && (gOptions.MenuStyle!=MenuStyle_ICON))
			{
				char Buf[20];
//				DrawCircleClock(gCurTime.tm_hour,gCurTime.tm_min, gCurTime.tm_sec, 31,31,32);
				sprintf(Buf,"%02d", gCurTime.tm_sec);
				DebugOutput1("S%s \n", Buf);
				if(gLCDRowCount>2)
				{
					if(gLCDCharWidth==20)
						LCDWriteStr(gLCDRowCount-2, SecondPos, Buf, 0);
					else
						LCDWriteStr(gLCDRowCount-2, SecondPos, Buf, 0);
				}
				else if(firstline && !gShowMainLCDDelay && (WaitPowerOff==0 || !(WaitPowerOff<10)))
					LCDWriteStr(0, (gLCDCharWidth-8)/2+6, Buf, 0);
				DebugOutput1("S%d \n", gCurTime.tm_sec);
			}
#endif
		}
#endif
		if(gFlashLED) gFlashLED--;
		break;

	case	MSG_TYPE_SWITCH:
		switch(msg->Param1)
		{
		case	SW_LOW_PWR:
			if(gState.voltage < SW_LOW_PWR) 
				gState.voltage = SW_LOW_PWR;
			break;
		case	SW_LOW_PWR_ALM:
			if(gState.voltage < SW_LOW_PWR_ALM) 
			{
				gState.voltage = SW_LOW_PWR_ALM;
				gShowLowPwrAlm = 1;				
			}	
			break;
		case	SW_LOW_PWR_OFF:
			if(gState.voltage < SW_LOW_PWR_OFF) 
			{	
				gState.voltage = SW_LOW_PWR_OFF;
				if(!(gState.state & STA_VRY_END) && gMachineState == STA_VERIFYING)
				{
					FinishProgram();
				}
				else if(gMachineState == STA_MENU && gUserOperateTime >= 6000/RUN_TICK)
				{
					printf("%s:%d FinishProgram\n", __func__, __LINE__);
					FinishProgram();
				}
				else ;
			}
			break;
		case	SW_BAK_PWR:
			gState.pwr_bak = 1;
			break;

		default: 
			break;
		}
		break;
	case MSG_TYPE_BUTTON:		
	{	
#ifndef URU
			if(gOptions.IsTestMachine)
			{
				extern int ShowVar;
			int Step=4;
				static int BlackThreshold=20;
				int Draw=FALSE, ReCalc=FALSE, Value,
					OldW=gOptions.OImageWidth,OldH=gOptions.OImageHeight;
				char buf[40];
				strcpy(buf,"       ");
				switch(msg->Param1)
				{
					case IKeyPower:
						ShowVar=FALSE;
						if(CalcFingerCenter(480,480))
							Draw=TRUE;
						LastIndex=0;				
						ReCalc=FALSE;
						break;
					case '0':
						ShowVar=FALSE;
						if(CalcFingerCenter(280,330))
							Draw=TRUE;
						LastIndex=0;				
						ReCalc=FALSE;
						break;
					case '2':
						{
							int XDPI, YDPI;
							ii=0;
							CaptureSensor(gImageBuffer, ONLY_LOCAL, &SensorBufInfo);
							if (gOptions.ZKFPVersion == ZKFPV10)
								BIOKEY_GETFINGERLINEAR_10(fhdl, (BYTE*)gImageBuffer, (BYTE*)gImageBuffer+(gOptions.OImageWidth*gOptions.OImageHeight));
							else
								BIOKEY_GETFINGERLINEAR(fhdl, (BYTE*)gImageBuffer, (BYTE*)gImageBuffer+(gOptions.OImageWidth*gOptions.OImageHeight));
							if(CalcImageDPI(gImageBuffer+(gOptions.OImageWidth*gOptions.OImageHeight),
										gOptions.ZF_WIDTH, gOptions.ZF_HEIGHT, &XDPI, &YDPI))
							{
								if(gLCDRowCount>3)
								{
									sprintf(buf, "X=%d", XDPI);
									LCDWriteStr(2,10,buf,0);
									sprintf(buf, "Y=%d", YDPI);
									LCDWriteStr(3,10,buf,0);
								}
								else
								{
									sprintf(buf, "X=%d, Y=%d", XDPI, YDPI);
									LCDWriteStr(1,0,buf,0);
								}
								strcpy(buf,"Auto DPI");
								//						gOptions.OImageWidth=((gOptions.OImageWidth*XDPI+250)/500+2)/4*4;
								//						gOptions.OImageHeight=(gOptions.OImageHeight*YDPI+250)/500;
								XDPI = (gOptions.ZF_WIDTH*500+XDPI/2)/XDPI;
								gOptions.ZF_WIDTH=((XDPI+gOptions.ZF_WIDTH+1)/2+2)/4*4;
								YDPI= (gOptions.ZF_HEIGHT*500+YDPI/2)/YDPI;
								gOptions.ZF_HEIGHT=(YDPI+gOptions.ZF_HEIGHT+1)/2;
								Draw = TRUE;
								ReCalc=TRUE;
							}
							else
							{
								ExBeep(2);
								LCDWriteStr(1,12,"Error DPI",0);
								ExBeep(2); DelayMS(200); ExBeep(2); DelayMS(200);
							}
							break;
						}
					case IKeyUp:
						if(BlackThreshold<=94) BlackThreshold+=4;
						Draw=TRUE;
						sprintf(buf, "%d%% ", BlackThreshold);
						ShowVar=FALSE;
						break;
					case IKeyDown:
						if(BlackThreshold>=4) BlackThreshold-=4;
						Draw=TRUE;
						sprintf(buf, "%d%% ", BlackThreshold);
						ShowVar=FALSE;
						break;
					case '1':
						gOptions.OImageWidth+=Step;
						strcpy(buf,"<-> ");
						Draw=TRUE;
						ReCalc=TRUE;
						break;
					case '3':
						gOptions.OImageWidth-=Step;
						strcpy(buf,">-< ");
						Draw=TRUE;
						ReCalc=TRUE;
						break;
					case '4':
						gOptions.OImageHeight+=Step;
						strcpy(buf,"|<->");
						Draw=TRUE;
						ReCalc=TRUE;
						break;
					case '6':
						gOptions.OImageHeight-=Step;
						strcpy(buf,"|>-<");
						Draw=TRUE;
						ReCalc=TRUE;
						break;
					case '5':
						gOptions.OTopLine+=Step;
						strcpy(buf,"/-> ");
						Draw=TRUE;
						ReCalc=TRUE;
						break;
					case '8':
						gOptions.OTopLine-=Step;
						strcpy(buf,"/<- ");
						Draw=TRUE;
						ReCalc=TRUE;
						break;
					case '9':
						gOptions.OLeftLine+=Step;
						Draw=TRUE;
						ReCalc=TRUE;
						strcpy(buf,"->  ");
						break;
					case '7':
						gOptions.OLeftLine-=Step;
						strcpy(buf,"<-  ");
						Draw=TRUE;
						ReCalc=TRUE;
						break;
					case IKeyOK:
						{
							TOptions opt;
							Draw=TRUE;
							LCDWriteStr(0,8,"  Writen",0);
							SaveOptions(&gOptions);
							if(!WriteSensorOptions(&gOptions,FALSE))//不能写入EEPROM
							{
								ExBeep(10);
								LCDWriteStr(0,10,"NoROM",0);
								ExBeep(10);
								DelayMS(100);
								ExBeep(10);
							}
							LoadOptions(&opt);
							if(opt.OTopLine!=gOptions.OTopLine)
								LCDWriteStr(0,10,"TLError",0);
							if(opt.OLeftLine!=gOptions.OLeftLine)
								LCDWriteStr(0,10,"LLError",0);
							if(opt.OImageWidth!=gOptions.OImageWidth)
								LCDWriteStr(0,10,"IWError",0);
							if(opt.OImageHeight!=gOptions.OImageHeight)
								LCDWriteStr(0,10,"IHError",0);
							if(opt.ZF_WIDTH!=gOptions.ZF_WIDTH)
								LCDWriteStr(0,10,"ZWError",0);
							if(opt.ZF_HEIGHT!=gOptions.ZF_HEIGHT)
								LCDWriteStr(0,10,"ZHError",0);
							DelayMS(10*1000);
							strcpy(buf,"    ");
							break;
						}
					case IKeyESC:
						gOptions.IsTestMachine=0;
						SaveInteger("~IsTestMachine", gOptions.IsTestMachine);
						LoadOptions(&gOptions);
						ShowMainLCDDelay=1;
						break;
					case IKeyMenu:
						{
							ShowVar=!ShowVar;
							strcpy(buf,"    ");
							Draw=TRUE;
						}
				}
				if(Draw)
				{
					if(ReCalc)
					{
						ShowVar=FALSE;
						if(gOptions.OTopLine<0) gOptions.OTopLine=0;
						if(gOptions.OLeftLine<0) gOptions.OLeftLine=0;
						if(gOptions.OImageWidth<220) gOptions.OImageWidth=220;
						if(gOptions.OImageWidth>620) gOptions.OImageWidth=620;
						if(gOptions.OImageHeight<160) gOptions.OImageHeight=160;
						if(gOptions.OImageHeight>450) gOptions.OImageHeight=450;
						if(gOptions.OImageWidth+gOptions.OLeftLine>CMOS_WIDTH) gOptions.OLeftLine=CMOS_WIDTH-gOptions.OImageWidth;
						if(gOptions.OImageHeight+gOptions.OTopLine>CMOS_HEIGHT) gOptions.OTopLine=CMOS_HEIGHT-gOptions.OImageHeight;
						if(OldW!=gOptions.OImageWidth)
						{
							Value=(gOptions.CPX[0]-gOptions.CPX[1])*gOptions.OImageWidth/OldW;
							gOptions.CPX[1]=gOptions.OImageWidth/2-Value/2;
							gOptions.CPX[0]=gOptions.CPX[1]+Value;
							Value=(gOptions.CPX[2]-gOptions.CPX[3])*gOptions.OImageWidth/OldW;
							gOptions.CPX[3]=gOptions.OImageWidth/2-Value/2;
							gOptions.CPX[2]=gOptions.CPX[3]+Value;
						}
						if(OldH!=gOptions.OImageHeight)
						{
							gOptions.CPY[1]=gOptions.OImageHeight;
							gOptions.CPY[0]=gOptions.CPY[1];
							gOptions.CPY[3]=0;
							gOptions.CPY[2]=0;
						}
						FPInit(NULL);
						InitSensor(gOptions.OLeftLine,gOptions.OTopLine,gOptions.OImageWidth,gOptions.OImageHeight, gOptions.NewFPReader);
						CaptureSensor(gImageBuffer, ONLY_LOCAL, &SensorBufInfo);
					}
					CaptureSensor(gImageBuffer, ONLY_LOCAL, &SensorBufInfo);
					LCDWriteStr(0,8,"        ",0);
					LCDWriteStr(1,12,buf,0);
					if (gOptions.ZKFPVersion == ZKFPV10)
						BIOKEY_GETFINGERLINEAR_10(fhdl, (BYTE*)gImageBuffer, (BYTE*)gImageBuffer+(gOptions.OImageWidth*gOptions.OImageHeight));
					else
						BIOKEY_GETFINGERLINEAR(fhdl, (BYTE*)gImageBuffer, (BYTE*)gImageBuffer+(gOptions.OImageWidth*gOptions.OImageHeight));
					if(gLCDHeight>40) DrawImage(gImageBuffer+(gOptions.OImageWidth*gOptions.OImageHeight), gOptions.ZF_WIDTH, gOptions.ZF_HEIGHT, BlackThreshold);
				}
				break;
			}		
#endif
			if(msg->Param1) ShowMainLCDDelay = 0;

			if(FPData.TempSize) //msg->Param1=IKeyESC;
			{
				ShowMainLCD();
				msg->Param1=IKeyESC;
			}

			if(gOptions.PowerMngFunOn) WaitPowerOff=gOptions.IdleMinute*60; 
			else WaitPowerOff=0;

			PowerSuspend=gOptions.IdlePower==HID_SUSPEND;
			FPData.TempSize=0;

			DBPRINTF("msg param1=%d param2=%d\n", msg->Param1, msg->Param2);
			if(DelayScheduleBell)
			{
				DelayScheduleBell=0;
				break;
			}

			if(gOptions.ShowState && (WaitAdminVerifyCount==0))
			{
				if((msg->Param2>=IKeyOTIn) && (msg->Param2<=IKeyOut) && (KeyBufferIndex==0)) //没有输入号码时
				{//上升为第一功能
					msg->Param1=msg->Param2;
					msg->Param2=0;
				}
			}
			else if((msg->Param2>=IKeyOTIn) && (msg->Param2<=IKeyOut))
				msg->Param2 =0;
			else if((msg->Param1>=IKeyOTIn) && (msg->Param1<=IKeyOut))
			{
				msg->Param1=msg->Param2;
				msg->Param2=0;
			}

			if(msg->Param2)
			{
				PrepareSecondFun=2;
			}
			else
				PrepareSecondFun=0;
			if((msg->Param1>=IKeyOTIn) && (msg->Param1<=IKeyOut))
			{
				if(gOptions.ShowState)
				{
					if(ProcStateKey(msg->Param1))
						KeyBufferTimeOut=0;
				}
			}
			else if(msg->Param1==IKeyPower)
			{
				if(!gOptions.NoPowerKey)
				{
					if(!gOptions.LockPWRButton)
					{
						POWEROFFSTART();
					}
				}
				else if(gOptions.NoPowerKey==1)
				{
					if(gOptions.AutoAlarmDelay) DoBell(gOptions.AutoAlarmDelay);			
				}
			}
			else	if(msg->Param1 == IKeyAvoidNorUnLock)
			{
				L3000Debug("\nhave a avoid nor unlock key!!!");
				if(L3000AvoidNorUnLock()){
					//		if(L3000ProcMsgBox(L3000_STR_REPLACE11, 0, 5) == News_CommitInput){
					int	init = 0;				
					L3000RunBeep(300, 0, 0);
					L3000RunLock(0);
					ConstructCartoon(CARTOON_LOCK, 1000/RUN_TICK, 4);

					if(gLock.nor_open == LOCK_STATE_OPEN){
						gLock.nor_open = LOCK_STATE_CLOSE;
						SetLockNorStateParam(LOCK_STATE_CLOSE);
					}
					while(1){
						while(IsStepTick(&init)){
							ControlLock();
							ControlBeepOutput();
							ShowLockCartoon();
						}
						if(gLock.close_ms == 0){
							FinishProgram();
						}
					}
					//		}
					//		else	{
					//			ShowMainLCD();					
					//		}
				}
			}
			else	if(msg->Param1 == IKeyNorUnLock)
			{
				L3000Debug("\nhave a nor unlock key!!!");
				memset(&gCartoon, 0, sizeof(TCartoon));
				if(L3000CheckNormalUnLock()){
					L3000Debug("\nnor unlock ok!!!");

					//	gLock.open_delay_ms = 10000/RUN_TICK;
					//	if(L3000ProcMsgBox(L3000_STR_REPLACE4, 0, 5) == News_CommitInput){
					L3000ExBeep(100);
					if(gLock.nor_open != LOCK_STATE_OPEN){
						gLock.nor_open = LOCK_STATE_OPEN;
						SetLockNorStateParam(LOCK_STATE_OPEN);
					}
					L3000ShowWarn(NULL, L3000_STR_REPLACE4, 2, 0);					
					FinishProgram();
					//		}
					//			else	{
					//				gLock.open_delay_ms = 3000/RUN_TICK;
					//				ShowMainLCD();
					//			}

				}
			}
			else if(msg->Param1==IKeyMenu)
			{

				/*	if(gOLED_En){
					BYTE time_ls[7] = {11, 10, 9, 2, 1, 3, 8};
					BYTE time_ls2[7] = {0,0,0,0,0,0,0};
					HT1380_WriteTime(time_ls);
					DebugOutput("\n Write time RTC!!! \n");
					HT1380_ReadTime(time_ls2);
					DebugOutput1("\n min is %d\n", time_ls2[1]);	
					if(time_ls[2] ==  time_ls2[2]){
					DebugOutput("\n time Write sucess!!!");
					}
					else	DebugOutput("\n time Write invalid!");
					}*/
				/*	if(KeyBufferTimeOut && !(KeyBufferIndex == 1 && KeyBuffer[0] == '4'))
					{
					U32 p;
					PUser u;
					HackerNumber=0;
					HackerWait=0;
					KeyBufferTimeOut=0;
					KeyBufferIndex=0;
					memset(KeyBuffer, 0, sizeof(KeyBuffer));
					if(gOptions.ViewAttLogFun & 2)
					{
					u=GetPINUser(KeyBuffer, &p);
					if(u)
					{
					ViewAttLogByUser(u->PIN, gOptions.TimeOutMenu);
					ShowMainLCD();
					}
					}
					}*/

				{
					//	U32 t=EncodeTime(&gCurTime);
					//	if(gOptions.ViewAttLogFun && (t-LastTime)<10 && !ClockEnabled)
					//	{
					//		ViewAttLogByUser(LastUID, gOptions.TimeOutMenu);//TIMEOUT_INPUT_PIN;
					//		ShowMainLCD();
					//	}
					//	else
					//	if(L3000LowClk == 0){
					KeyBufferTimeOut=0;
					KeyBufferIndex=0;
					memset(KeyBuffer, 0, sizeof(KeyBuffer));
					memset(&VryPwd, 0, sizeof(TInputPwd));
					CheckAdmin();
					//	}
					//	else	if(L3000CheckNormalUnLock()){
					//		;
					//	}

				}
			}
			else if(msg->Param1==IKeyDel)			
			{
				//		if(KeyBufferTimeOut>0)// && (KeyBufferIndex==4))
				//		{
				//			char str[30];
				//			KeyBuffer[KeyBufferIndex]=0;
				//	if(0==strcmp(KeyBuffer,"5151"))
				//	{
				//		gOptions.ShowSecond=!gOptions.ShowSecond;
				//		SaveOptions(&gOptions);
				//		ShowMainLCD();
				//	}
				//	else
				//			KeyBuffer[4] = '\0';
				//			sprintf(str, "\nKeyBuffer: %s", KeyBuffer);
				//			DebugOutput(str);
				//			if(0==strcmp(KeyBuffer,"1111"))
				//			{
				/*	if(HackerWait)
					{
					DoShowSysInfo(NULL);
					HackerWait=0;
					KeyBufferTimeOut = 0;
					KeyBufferIndex = 0;
					memset(KeyBuffer, 0, sizeof(KeyBuffer));
					ShowMainLCD();
					}
				 */
				//	else	
				{
					HackerWait = 10;
					HackerNumber=1;
					KeyBufferTimeOut = 0;
					KeyBufferIndex = 0;
					memset(KeyBuffer, 0, sizeof(KeyBuffer));
					memset(&VryPwd, 0, sizeof(TInputPwd));
					CheckAdmin();						
				}

				//			}
				/*			else	{
							HackerWait = 0;
							HackerNumber=0;
							KeyBufferTimeOut = 0;
							KeyBufferIndex = 0;
							memset(KeyBuffer, 0, sizeof(KeyBuffer));
							memset(&VryPwd, 0, sizeof(TInputPwd));	
				//		WaitAdminVerifyCount=0;
				//		WaitAdminVerifySecond=0;
				}*/
				//		}
			}
	/*		else if(0==strcmp(KeyBuffer, "8882"))
				ResetSys(0);
			else if(HackerWait)
				{
					char buf[20];
					sprintf(buf,"%02d%02d", gCurTime.tm_hour+1,gCurTime.tm_min);
					HackerWait=0;
					if(0==strcmp(KeyBuffer,"8888"))
					{
						HackerNumber=0xFFFF & GetUS();
						HackerWait=50;
						sprintf(buf,"%6d", HackerNumber);
						LCDWriteStr(0,10,buf,LCD_HIGH_LIGHT);
						DelayMS(5000);
					}
					else if(0==strcmp(KeyBuffer, buf)// && (gLCDHeight>=64))
					{
						gOptions.IsTestMachine=1;
						ShowTestFinger();
						SaveOptions(&gOptions);
					}
					else if(0==strcmp(KeyBuffer,"8880"))
						DoHideAdvMenu(NULL);
					else if(HackerNumber>10 && !(LOCKFUN_DOORSENSOR & gOptions.LockFunOn))
					{
						HackerNumber=SumNum(HackerNumber);
						sprintf(buf, "7%d0%d",HackerNumber/10, HackerNumber%10);
						if(0==strcmp(buf, KeyBuffer))
						{
							//获得超级管理员权限
							HackerWait=1000;
							HackerNumber=1;
							KeyBufferTimeOut=0;
						}
						else
						{
							sprintf(buf, "%d%d%d",
								gCurTime.tm_hour%10,
								gCurTime.tm_year%10+gCurTime.tm_mon%10+gCurTime.tm_mday%10,
								HackerNumber%10);
							if(0==strcmp(buf, KeyBuffer))
							{
								//获得黑客权限
								LCDWriteStr(0,12,"HAHA",LCD_HIGH_LIGHT);
								HackerWait=1000;
								HackerNumber=2;
								KeyBufferTimeOut=0;
							}
							else
							{
								sprintf(buf, "73%2d",HackerNumber);
								if(0==strcmp(buf, KeyBuffer))
								{
									LCDWriteCenterStr(0,DeviceVender);
									DelayMS(5000);
								}
								else
								{
									sprintf(buf, "91%2d",HackerNumber);
									if(0==strcmp(buf, KeyBuffer))
									{
										LCDWriteCenterStr(0,AuthorName);
										DelayMS(5000);
									}
								}
							}
						}
					}
				}
#ifdef FACTORY_TEST
			else if(KeyBufferTimeOut>0 && (KeyBufferIndex>=1))
			{
				KeyBuffer[KeyBufferIndex]=0;
				if(0==strcmp(KeyBuffer,"1"))
				{
					if(!WriteSensorOptions(&gOptions,TRUE))//不能写入EEPROM
					{
						ExBeep(50);
						LCDWriteStr(0,0," EEPROM Error!",LCD_HIGH_LIGHT);
						DelayMS(1200);
					}else
					{
						LCDWriteStr(0,0," EEPROM OK!",LCD_HIGH_LIGHT);
						DelayMS(1200);
					}
				}
				else if(0==strcmp(KeyBuffer,"2"))
				{
					DoCheckLCD(NULL);
				}else if(0==strcmp(KeyBuffer,"3"))
				{
					gOptions.IsTestMachine=1;
					ShowTestFinger();
					SaveOptions(&gOptions);
				}

			}
#endif
			WaitAdminVerifyCount=0;
			WaitAdminVerifySecond=0;
			WaitAdminRemainCnt=0;
			KeyBufferIndex=0;
			ShowMainLCD();
		*/
		else if(msg->Param1==IKeyDown)
		{
			if(gOptions.I1ToG && KeyBufferIndex)
			{
				g1ToG=TRUE;
				LCDWriteLineStrID(0,HID_1TOG);
			}
		}
		else if(msg->Param1==IKeyESC)
		{
/*
			{
				extern int DMRec, DMPRec;
				char buf[3][20];
				sprintf(buf[0], "OK/ESC Exit");
				sprintf(buf[1], "Recv=%d", DMRec);
				sprintf(buf[2], "Recv Pkg=%d", DMPRec);
				LCDSelectOK(buf[1],buf[2],buf[0]);
			}
*/
			
			if(gState.state & STA_VRY_END){
				if(gLock.open_ms == 0 && gLock.close_ms && gLock.open_delay_ms > 1){
					gLock.open_delay_ms = 2;
				}
			}
			else	if(gState.state & STA_NOR_OPEN){
				FinishProgram();
			}
			else;	
/*
			HackerNumber=0;
			HackerWait=0;
			WaitAdminVerifyCount=0;
			WaitAdminVerifySecond=0;
			WaitAdminRemainCnt=0;
			KeyBufferIndex=0;
			KeyBufferTimeOut=0;
			DelayNextUser=0;
			ClearMultiOpen();
			ShowMainLCD();*/
		}
		else if(msg->Param1==IKeyOK)
		{
			int	ret = -1;
			U16     pin = 0;
			
		  if(KeyBufferIndex && VryPwd.bits){
			HackerNumber=0;
			HackerWait=0;
			ExFeedWatchDog(20);	//避免连续密码比对的重启
			KeyBufferTimeOut = 0;
			KeyBufferIndex = 0;
			memset(KeyBuffer, 0, sizeof(KeyBuffer));
			ret = L3000ProcInputPwd(&pin, &VryPwd);
			memset(&VryPwd, 0, sizeof(TInputPwd));
			DebugOutput1("\nVryPwd: %d\n", ret);
		//	if(ret == -1) {				
		//		L3000ShowVerifyInValid(2);
		//	}
			if(ret == -4 || ret == -1){
		//		if(ret != -1) {
					L3000CheckVryFailCnt(2);
					L3000ShowVerifyInValid(2);
		//		}
		//		else	L3000ShowInfo(NULL, L3000_STR_REPLACE15, 0, 2);
			//	ShowMainLCDDelay(2);
			}
			else	if(ret == 0){
		//		ExVryBind(&VryBind, pin, 2, 0);
				ConstructMSG(msg, MSG_TYPE_CMD, News_VerifiedByPwd, pin);
			}
			else	;/*
			if(KeyBufferIndex)
			{
				ExFeedWatchDog(20);	//避免连续密码比对的重启
				if(gLCDRowCount>2) LCDWriteLineStrID(0,HID_VERINPUTPWD);
				i=CheckInputNumber(KeyBuffer);
				if(i>=0)
					ConstructMSG(msg, MSG_TYPE_CMD, News_VerifiedByPwd, i);
				else
				{
					int p;
					if(0!=strtou32(KeyBuffer, (U32*)&p)) p=0;
					OutputFailData(i,News_VerifiedByPwd,FALSE,p);
					if(WaitAdminVerifyCount)
					WaitAdminVerifyCount--;
				}
				KeyBufferIndex=0;
				KeyBufferTimeOut=0;
			}
			else
				ShowMainLCD();*/
		    }
		    else	{                        
			memset(&VryPwd, 0, sizeof(TInputPwd));
			KeyBufferIndex=0;
			KeyBufferTimeOut=0;
		    	ShowMainLCD();
		    }		  		  	
		}
		else if(msg->Param1>='0' && msg->Param1<='9')
		{
			
	//		KeyBufferTimeOut= 5; //TIMEOUT_INPUT_PIN;
			if(gState.state) {
				memset((U8 *)&VryPwd, 0, sizeof(TInputPwd));				
				KeyBufferIndex = 0;
				KeyBufferTimeOut = 0;
				KeyBuffer[0] = 0;
			}
			else	{
				L3000InputPassward(&VryPwd, msg->Param1);
                       		 if(VryPwd.bits){
					KeyBufferTimeOut = 5;
					KeyBufferIndex = VryPwd.bits;
					KeyBuffer[KeyBufferIndex-1] = VryPwd.buf[KeyBufferIndex-1];
				}
			}
		/*	int PinWidth=PIN_WIDTH;
			U32 uid;
			if(g1ToG)
			{
				g1ToG=FALSE;
				if(gOptions.One2OneFunOn)
					LCDWriteLineStrID(0,HID_1TO1);
			}
			if(gOptions.PIN2Width>PinWidth)
				PinWidth=gOptions.PIN2Width;
			ExLightLED(LED_RED, FALSE);
			ExLightLED(LED_GREEN, FALSE);
			if(WaitShowState>0)
				WaitShowState=TIMEOUT_SHOWSTATE;
			if(KeyBufferTimeOut==0)
				KeyBufferIndex=0;
			if(KeyBufferIndex==0)
			{
				LCDClear();
				if(gOptions.One2OneFunOn)
				{
					LCDInfoShow(LoadStrByID(HID_1TO1),"",NULL);
					LCDFullALine(gLCDRowCount-1,LoadStrByID(HID_OKPWD));
				}
				else
					LCDInfoShow(LoadStrByID(HID_VERINPUTPWD),"",NULL);
			}
			KeyBuffer[KeyBufferIndex++]=msg->Param1;
			KeyBuffer[KeyBufferIndex]=0;
			strtou32(KeyBuffer, &uid);
			//如果输人1-5号就做为分组比对
            if (gOptions.I1ToG)
            {
				if (uid >= 1 && uid <= 5)
				{
					g1ToG=TRUE;
					LCDWriteLineStrID(0, HID_1TOG);
				}
				else
				{
					g1ToG=FALSE;
					if(gOptions.One2OneFunOn)
						LCDWriteLineStrID(0, HID_1TO1); //1:G 1:H
				}

            }
			ShowUserHint(gLCDRowCount-2, KeyBufferIndex>5, uid);
			if(KeyBufferIndex>=PinWidth)
			{
				char test[]="99999999999";
				test[PinWidth]=0;
				if(0==strcmp(KeyBuffer,test))
				{
					KeyBuffer[0]=0;
					KeyBufferIndex=0;
					CheckAdmin();
				}
				else if(!gOptions.One2OneFunOn)
				{
					i=CheckInputNumber(KeyBuffer);
					if(i>=0)
						ConstructMSG(msg, MSG_TYPE_CMD, News_VerifiedByPwd, i);
					else if(WaitAdminVerifyCount)
						WaitAdminVerifyCount--;
					KeyBufferIndex=0;
					KeyBufferTimeOut=0;
				}
				else
					KeyBufferIndex=PinWidth-1;
			}
			else
				KeyBufferTimeOut=TIMEOUT_INPUT_PIN;*/
		}
		else
		{
			if(SecondFunction || LastKey==IKeyPower)
			{
				if(SecondFunction==IKeyPower && (WaitPowerOff>0 && WaitPowerOff<=3))	//当0和电源按钮共用时导致按了Menu按钮后只要输入0则验证无法进入菜单,故加 && WaitPowerOff
				{
					WaitAdminVerifyCount=0;
					WaitAdminVerifySecond=0;
				}
				else if(SecondFunction==IKeyDuress)
				{
					DelayTriggerDuress=3;
				}else if(SecondFunction==-1)
				{
					gOptions.AttState=-1;
					if(gOptions.MustChoiceInOut) WaitShowState=TIMEOUT_SHOWSTATE;
					ShowMainLCD();
				}
				else if(SecondFunction==-1)
				{
					gOptions.AttState=-1;
					//if(gOptions.MustChoiceInOut)
				}
				if(gOptions.PowerMngFunOn) WaitPowerOff=gOptions.IdleMinute*60; else WaitPowerOff=0;
				PowerSuspend=gOptions.IdlePower==HID_SUSPEND;
			}
			if(0==ShowMainLCDDelay && (!(((LastKey>='0') && (LastKey<='9'))||g1ToG) || KeyBufferIndex==0))
			{
//				PrepareSecondFun=0;
				KeyBufferIndex=0;
				ShowMainLCD();
			}
		}
		SecondFunction=msg->Param2;
		LastKey=msg->Param1;
//		DebugOutput2("KeyBuffer=""%s"",%d\r\n", KeyBuffer, KeyBufferIndex);
	}
		break;
#ifndef MODULE
	case MSG_TYPE_MF:

		if(VryPwd.bits){
                        KeyBufferTimeOut = 0;
                        KeyBufferIndex = 0;
                        memset(KeyBuffer, 0, sizeof(KeyBuffer));
                        memset(&VryPwd, 0, sizeof(TInputPwd));
                }
		gState.dis_vry_tick = DIS_VRY_TICK;
		gUserOperateTime = 0;
		ExFeedWatchDog(10);
		if(FPData.TempSize>0) break;
		memset(FingerTemplate, 0, sizeof(FingerTemplate));
		if(msg->Param2==0)
		{
			FPData.TempSize=MFRead(&FPData, gOptions.OnlyPINCard);
			ExBeep(3);
			MFFinishCard();
		}
		else
		{
			//FPData.TempSize=MMAPIRead(&st232, &FPData.PIN, FPData.Templates);
			i=0;
			ExBeep(333);
		}
		if(FPData.TempSize>0)
		{
			char buf[30];
		//	LCDClear();
		//	LCDInfoShow(LoadStrByID(HID_CARDFP),"",NULL);
			
			sprintf(buf, "%05d", FPData.PIN);
			//sprintf(buf, "%s %05d", LoadStrByID(HID_CARDFP), FPData.PIN);
		//	LCDWriteCenterStr(1,buf);
		//	LCDWriteCenterStrID(2,HID_PLACEFINGER);
		//	LCDWriteCenterStrID(3,HID_CANCELKEY);
		//	if(gLCDWidth==0)
		//	{
		//		ShowMainLCDDelay(5);
		//		gFlashLED=5;
		//		ExLightLED(LED_RED, FALSE);
		//	}
		//	else
		//		ShowMainLCDDelay(6);
			MfVryCnt = 3;
			L3000ShowCardVry(LoadStrByID(HID_CARDFP), buf, LoadStrByID(HID_PLACEFINGER), 0, 6);
		}
		else if(FPData.TempSize==0)
		{
			char * buf[30];
		//	LCDClear();
	/*		if(!gOptions.OnlyPINCard)
			{
				KeyBufferTimeOut=TIMEOUT_INPUT_PIN;
				gFlashLED=KeyBufferTimeOut;
				ExLightLED(LED_RED, FALSE);
		//		LCDInfoShow(LoadStrByID(HID_1TO1),"", NULL);
			}*/
	//		else
	//		{
	//	;//	LCDInfoShow(LoadStrByID(HID_CARD),"", NULL);
	//		}
	//		sprintf(KeyBuffer, "%d", FPData.PIN);
	//		KeyBufferIndex=strlen(KeyBuffer);
			//LCDFullALine(3,LoadStrByID(HID_OKPWD));
			//ShowUserHint(2, FALSE, FPData.PIN);
			
	//		sprintf(buf, "%s %05d", LoadStrByID(HID_1TO1), FPData.PIN);
	//		L3000ShowCardVry(NULL, buf, LoadStrByID(HID_PLACEFINGER), 0, 6);
			if(gOptions.OnlyPINCard)
			{
				LCDClear();
				KeyBufferIndex=0;
				KeyBufferTimeOut=0;
		//		ExVryBind(&VryBind, FPData.PIN, 1, 0);
				ConstructMSG(msg, MSG_TYPE_CMD, News_VerifiedByCard, FPData.PIN);
			}
			else	{
				L3000CheckVryFailCnt(1);
				L3000ShowVerifyInValid(100);				
			}
		}
		else if(KeyBufferIndex==0)
		{
		//	LCDClear();
			L3000CheckVryFailCnt(1);
			L3000ShowVerifyInValid(109);
		//	LCDInfoShow(LoadStrByID(HID_CARDFP),"", NULL);
		//	LCDWriteCenterStrID(2,HID_INVALIDCARD);
		//	ShowMainLCDDelay(2);
		//	ShowVerifyFail(0,News_VerifiedByCard, FALSE, 0,-1);
		}
		break;
	case MSG_TYPE_HID:
		{
			TUser user;
			BYTE card[5];
			if(VryPwd.bits)
			{
				KeyBufferTimeOut = 0;
				KeyBufferIndex = 0;
				memset(KeyBuffer, 0, sizeof(KeyBuffer));
				memset(&VryPwd, 0, sizeof(TInputPwd));
			}
			SET_DWORD(card, msg->Param2,0);
			card[4]=msg->Param1 & 0xFF;
			ExFeedWatchDog(10);

			if(!WaitAdminVerifySecond && gOptions.MasterSlaveFunOn && gOptions.MasterSlaveOn==2)
			{
				SendMasterProcess(CMD_SLAVE_HID,(char*)card,5,1000);
				return -1;
			}

			LCDClear();
			if(!FDB_GetUserByCard(card, &user))
			{
				L3000CheckVryFailCnt(1);
				L3000ShowVerifyInValid(100);				
				
				/*LCDInfoShow(LoadStrByID(HID_CARD),LoadStrByID(HID_NOTENROLLED),Icon_Warning);
				ExLightLED(LED_RED, TRUE);
				ExBeep(6);
			
				ShowVerifyFail(0,News_VerifiedByCard, FALSE, 0,VOICE_INVALID_ID);
				//if(gOptions.VoiceOn) ExPlayVoice(VOICE_INVALID_ID);
				ShowMainLCDDelay(2);
				SET_DWORD(card, -1, 0);card[4]=0xff;
				CheckSessionSend(EF_VERIFY,(char*)card,5);*/
				break;
			}
			ExBeep(3);
			ShowMainLCDDelay = 2;
			if(gOptions.PIN2Width>PIN_WIDTH)
				FPData.PIN=user.PIN2;
			else
				FPData.PIN=user.PIN;
			FPData.TempSize=0;
			if(!gOptions.OnlyPINCard)
			{
				KeyBufferTimeOut=TIMEOUT_INPUT_PIN;
				gFlashLED=KeyBufferTimeOut;
				ExLightLED(LED_RED, FALSE);
				LCDInfoShow(LoadStrByID(HID_1TO1),"");
			}
			/*else
			{
				LCDInfoShow(LoadStrByID(HID_CARD),"",NULL);
			}

			sprintf(KeyBuffer,"%s     ", LoadStrByID(HID_CARD_NO));
			//F10的gLCDCharWidth=0;
			if(card[4]==0)
				sprintf(KeyBuffer+gLCDCharWidth-10,"%d",GETDWORD(card));
			else
				EncodeHex((BYTE*)KeyBuffer+gLCDCharWidth-10, card, 5);

			LCDWriteCenterStr(1, KeyBuffer);
			//F10导致此值为0,因为F10的gLCDCharWidth=0;
			//KeyBufferIndex=strlen(KeyBuffer);

			sprintf(KeyBuffer, "%d", FPData.PIN);
			KeyBufferIndex=strlen(KeyBuffer);
			LCDFullALine(3,LoadStrByID(HID_OKPWD));
//			ShowUserHint(2, FALSE, FPData.PIN);
*/
			if(gOptions.OnlyPINCard)
			{
				LCDClear();
				KeyBufferIndex=0;
				KeyBufferTimeOut=0;
				ConstructMSG(msg, MSG_TYPE_CMD, News_VerifiedByCard, FPData.PIN);
				SET_DWORD(card, FPData.PIN, 0);card[4]=0xff;
				CheckSessionSend(EF_VERIFY,(char*)card,5);
			}
		}
		break;
#endif
	case MSG_TYPE_FINGER:
		{
			PSensorBufInfo SensorInfo=(PSensorBufInfo)msg->Param2;
			int bSign=0;
			ClockEnabled=FALSE;
			U32 pin2=0;

			ExLightLED(TRUE, FALSE);
			ExLightLED(LED_GREEN, FALSE);

		#if 0	
			IsVryMF = 0;
			IsVryFgID = 0xf0;
			if(VryPwd.bits){
				KeyBufferTimeOut = 0;
				KeyBufferIndex = 0;
				memset(KeyBuffer, 0, sizeof(KeyBuffer));
				memset(&VryPwd, 0, sizeof(TInputPwd));
			}
			L3000ShowFingerTouch();
			gUserOperateTime = 0;
		#endif
			
			if(!HasInputControl()&&gOptions.Must1To1&&(FPData.PIN==0))
			{
				LCDWriteLineStrID(0, HID_VF);
				LCDWriteCenterStrID(gLCDRowCount/2, HID_OS_MUST1TO1);
				if (gOptions.Nideka)
					LCDWriteCenterStrID(gLCDRowCount-1, NID_PSPUTCARD);

				ShowMainLCDDelay=3;
				break;
			}

			if(gOptions.MustChoiceInOut && (gOptions.AttState<0))
			{
				LCDWriteLineStrID(0, HID_VF);
				LCDWriteCenterStrID(gLCDRowCount/2, HID_MUSTINOUT);
				ExBeep(1);
				ShowMainLCDDelay=3;
				break;
			}
			
			i=0;
		//	if((msg->Param1==ONLY_LOCAL)||(msg->Param1==LOCAL_NETWORK)) //dewarped image
			if((msg->Param1==ONLY_LOCAL)) //dewarped image
			{
				#if 0 //treckle
				if(gOptions.SaveBitmap)
				{
#ifndef URU   	
					char *fingerbuf;
#endif					
					if ((tmp=LoadStrOld("FONTFILEPATH"))!=NULL)
						sprintf(bmpFileName, "%sfinger.bmp", tmp);
					else
						sprintf(bmpFileName, "finger.bmp");	
					
#ifdef URU   	
					write_bitmap(bmpFileName, SensorInfo->DewarpedImgPtr, gOptions.ZF_WIDTH, gOptions.ZF_HEIGHT);
#else	
					write_bitmap(bmpFileName, SensorInfo->DewarpedImgPtr, gOptions.OImageWidth, gOptions.OImageHeight);
					fingerbuf=malloc(150*1024);
					BIOKEY_GETFINGERLINEAR(fhdl, SensorInfo->DewarpedImgPtr, fingerbuf);
					write_bitmap("fingerdewarped.bmp", fingerbuf, gOptions.ZF_WIDTH, gOptions.ZF_HEIGHT);
					free(fingerbuf);
#endif					
					DBPRINTF("Image be Saved!\n");
				}
				#endif
//test
/*
				int tempfd;
				char *fingerbuf1;
				
				fingerbuf1 = malloc(150*1024);				
				tempfd = open("finger.bmp",O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
				if (tempfd)
					read(tempfd,fingerbuf1,292*362);

				i=IdentifyFinger(KeyBuffer, FPData.PIN, FingerTemplate, fingerbuf1);
				close(tempfd);
				free(fingerbuf1);
*/

				i=IdentifyFinger(KeyBuffer, FPData.PIN, FingerTemplate, SensorInfo->DewarpedImgPtr);
				if(i>0)
				{
					gSensorNo=SensorInfo->SensorNo;
					ConstructMSG(msg, MSG_TYPE_CMD, News_VerifiedByFP, i);
					break;
				}
				else
					bSign=1;
			}
			#if 0	//treckle
			if(((msg->Param1==LOCAL_NETWORK)&&(bSign==1))||(msg->Param1==ONLY_NETWORK)||(msg->Param1==NETWORK_LOCAL))
			{
				
				char *fingerbuf=NULL;
				
				if(gOptions.ASDewarpedImage)
				{


#ifdef ZEM300
					#ifdef URU   	
						iImageLen=SensorInfo->DewarpedImgLen;
						pImagePtr=SensorInfo->DewarpedImgPtr;
					#else
						U16 zfwidth,zfheight;
						zfwidth = gOptions.ZF_WIDTH;
						zfheight = gOptions.ZF_HEIGHT;
						iImageLen=gOptions.ZF_WIDTH*gOptions.ZF_HEIGHT;
						iImageLen+= 4;
						//iImageLen=280*330;
						DBPRINTF("DewarpedImage: RawImgLen: %d\tdewlen:%d\n",SensorInfo->RawImgLen,iImageLen);
						fingerbuf=malloc(150*1024);
						memcpy(fingerbuf,&zfwidth,2);
						memcpy(fingerbuf+2,&zfheight,2);
						BIOKEY_GETFINGERLINEAR(fhdl, SensorInfo->RawImgPtr, fingerbuf+4);

						pImagePtr=fingerbuf;
						//write_bitmap("fingerdewarped.bmp", fingerbuf+4, gOptions.ZF_WIDTH, gOptions.ZF_HEIGHT);

					#endif
#else
						iImageLen=SensorInfo->DewarpedImgLen;
						pImagePtr=SensorInfo->DewarpedImgPtr;
					
#endif

					;
				}
				else
				{
					iImageLen=SensorInfo->RawImgLen;
					pImagePtr=SensorInfo->RawImgPtr;
				}
				
				if(FPData.PIN)
				{
					KeyBufferTimeOut=TIMEOUT_INPUT_PIN;
					sprintf(KeyBuffer, "%d", FPData.PIN);
					KeyBufferIndex=strlen(KeyBuffer);
				}						
				if(KeyBuffer&&(strtou32(KeyBuffer, (U32*)&pin2)==0))
				{
					if(!bSign)
						LCDWriteLineStrID(0, HID_1TO1);
				}
				else //1:many
				{
					if(!bSign)
						LCDWriteLineStrID(0, HID_VF);
				}
				if(!bSign)
					LCDWriteCenterStrID(gLCDRowCount/2, HID_LEAVEFINGER);
				
				memset(&AuthFPData, 0, sizeof(TFPResult));	
				//test

				if(gOptions.SaveBitmap)
				{
					write_bitmap("RISFP.bmp", pImagePtr+64, 384,289);
					printf("imgsize: %d\t w: %d\th:%d\n" ,iImageLen,gOptions.ZF_WIDTH,gOptions.ZF_HEIGHT);
				}

				if (SendImageAndIdentify(pImagePtr, iImageLen, pin2, &AuthFPData)&&AuthFPData.PIN)
				{
					gSensorNo=2;
					ConstructMSG(msg, MSG_TYPE_CMD, News_VerifiedByFPRIS, AuthFPData.PIN);
                                	if (fingerbuf)
	                                {
        	                                free(fingerbuf);
                	                        DBPRINTF("FREE fingerbuf\n");
                        	        }
					break;
				}
				else if(msg->Param1==NETWORK_LOCAL)
				{
					#if 0
					i=IdentifyFinger(KeyBuffer, FPData.PIN, FingerTemplate, SensorInfo->DewarpedImgPtr);
					if(i>0)
					{
						gSensorNo=SensorInfo->SensorNo;
						ConstructMSG(msg, MSG_TYPE_CMD, News_VerifiedByFP, i);
						break;
					}
					else
						bSign=2;						
					#endif //treckle
				}
				else	
					bSign=2;


                                if (fingerbuf)
                                {
                                        free(fingerbuf);
                                        DBPRINTF("FREE fingerbuf\n");
                                }


			}	
			#endif //treckle
			//verification failed
			if(bSign)
			{
				ConstructMSG(msg, MSG_TYPE_CMD, News_FailedByFP, 0);
			}
		}
		break;
	case MSG_TYPE_CMD:
		switch(msg->Param1)
		{
			case News_Door_Sensor:
				if((gOptions.LockFunOn&LOCKFUN_DOORSENSOR)&&(gOptions.DoorSensorMode<2))
				{
					ShowMainLCDDelay=10;
					if(msg->Param2==DOOR_SENSOR_BREAK)
					{
						LCDWriteCenterStrID(0,HID_ALARM_DOOR);
						//ExAlarm(0, 24*60*60);
						DoAlarm(0, 24*60*60);
						FDB_AddOPLog(0, OP_ALARM, msg->Param1, msg->Param2, 0,0);
					}
					else if(msg->Param2==DOOR_SENSOR_OPEN)
					{
						//LCDWriteCenterStrID(0,HID_DOOR_OPEN);
						gDoorSensorDelay=gOptions.OpenDoorDelay;

						ShowMainLCDDelay=2;
					}
					else if(msg->Param2==DOOR_SENSOR_CLOSE)
					{
						LCDWriteCenterStrID(0,HID_DOOR_CLOSE);
						gCloseDoorDelay=0;
					}
					CheckSessionSend(EF_ALARM, (void*)&msg->Param1, 8);
				}
				break;
			case News_Door_Button:
				LCDWriteCenterStrID(0,HID_ALARM_INDOOROPEN);
				CheckSessionSend(EF_ALARM, (void*)&msg->Param1, 4);
				FDB_AddOPLog(0, OP_ALARM, msg->Param1, msg->Param2, 0,0);
				//ExAuxOut(gOptions.LockOn, gOptions.OpenDoorDelay);
				DoAuxOut(gOptions.LockOn, gOptions.OpenDoorDelay);
				ShowMainLCDDelay=5;
				break;
			case News_Alarm_Strip:
				LCDWriteCenterStrID(0,HID_ALARM_STRIPE);
				CheckSessionSend(EF_ALARM, (void*)&msg->Param1, 4);
				FDB_AddOPLog(0, OP_ALARM, msg->Param1, msg->Param2, 0,0);
				//ExAlarm(0, 24*60*60);
				DoAlarm(0, 24*60*60);
				ShowMainLCDDelay=10;
				gAlarmStrip=0x10000;
				break;
			case News_Battery:
				if(gPowerState!=msg->Param2)
				{
					gPowerState=msg->Param2;
					CheckSessionSend(EF_ALARM, (void*)&msg->Param1, 8);
					FDB_AddOPLog(0, OP_ALARM, msg->Param1, msg->Param2, 0,0);
					ExBeep(3);
					if(ShowMainLCDDelay==0)
						ShowMainLCD();
					if(gPowerState==BATTERY_None)
					{
						LCDWriteCenterStrID(0,HID_ALARM_LOWBATTERY);
						WaitPowerOff=4;
						PowerSuspend=FALSE;
					}
					else if(gPowerState==BATTERY_External)//恢复外部供电
					{
						if(gOptions.PowerMngFunOn) WaitPowerOff=gOptions.IdleMinute*60; else WaitPowerOff=0;
						PowerSuspend=gOptions.IdlePower==HID_SUSPEND;
					}
				}
				return 1;
			case News_Reset_Options:
				//			RestoreDefaultOptions();
				break;
			case News_VerifiedByPIN:
			case News_VerifiedByPwd:
			case News_VerifiedByFP:
			case News_VerifiedByIDCard:		
			case News_VerifiedByMFCard:			
			case News_VerifiedByIDCardRIS:			
			case News_VerifiedByFPRIS:
				{
					int	method = 0;
					BYTE str[4];
					str[0] = 8;

					if(gState.state & STA_VRY_END+STA_COMM+STA_NOR_OPEN){
						IsVryMF = 0;       
						break;	
					}
					if(msg->Param1 == News_VerifiedByPwd) method = 2;
					else	if(msg->Param1 == News_VerifiedByFP) method = 1;
					else	method = 0;

					FPData.TempSize=0;
					ExLightLED(LED_RED, FALSE);
					ExLightLED(LED_GREEN, TRUE);
					LastIndex-=1;
					pin=GetUserPIN_16(msg->Param2);

#ifndef MODULE
					if(WaitAdminVerifyCount)
					{
						IsVryMF = 0;	
						memset(&VryBind, 0, sizeof(TVryBind));
						AdminUser=FDB_GetUser(pin, &gAdminUser);
						if(AdminUser && ISADMIN(AdminUser->Privilege))
						{
							if(ISINVALIDUSER(*AdminUser))
							{
								i=-1;
								L3000CheckVryFailCnt(method);
								L3000ShowVerifyInValid(1000);
								//	char buf[20];
								//	LCDWriteCenterStrID(3,HID_PRI_INVALID);
								//	sprintf(buf, "%s %05d",LoadStrByID(HID_ENROLLNUM),AdminUser->PIN);
								//	LCDWriteCenterStr(2, buf);
							}
							else if(WaitAdminRemainCnt==1)
							{
								WaitAdminVerifySecond=0;
								ShowMainLCDDelay=0;

								if(gOptions.C2FunOn)	
									ExSetMonitorC2(FALSE);

								if(gState.pwr_bak)
								{ 
									L3000ShowWarn(NULL, L3000_STR_REPLACE1, 0, 2); 		    
								}
								else	if(gState.voltage >=  SW_LOW_PWR_ALM)
								{
									L3000ShowWarn(NULL, LoadStrByID(HID_ALARM_LOWBATTERY), 0, 2);
								}
								else	DoMainMenu();

								if(gOptions.C2FunOn)	ExSetMonitorC2(TRUE);
								ExSetAuxOutDelay(gOptions.LockOn,gOptions.OpenDoorDelay, gOptions.DoorSensorMode);
								if(!gOptions.PageState) gOptions.AttState=GetNowState();
								KeyBufferIndex=0;
							}
							else if(WaitAdminRemainCnt>0)
							{
								for(i=0;i<MaxAdminVerify;i++)
									if(WaitAdmins[i]==AdminUser->PIN) break;
								if(i==MaxAdminVerify)
								{
									WaitAdminRemainCnt--;
								}
								WaitAdminVerifyCount=COUNT_RETRY_ADMIN;
								WaitAdminVerifySecond=TIMEOUT_WAIT_ADMIN_VERIFY;
							}
						}
						else
						{
							i=-1;
							L3000CheckVryFailCnt(method);
							L3000ShowVerifyInValid(1000);
							//ShowVerifyFail(i,msg->Param1,FALSE,i>0?i:0,VOICE_INVALID_ADMIN);
						}
					}
					else
#endif
					{
						int	tmp = 0;

						if(msg->Param1 == News_VerifiedByCard)           ExVryBind(&VryBind, pin, 1, 0);
						else	if(msg->Param1 == News_VerifiedByPwd)    ExVryBind(&VryBind, pin, 2, 0);
						else    if(msg->Param1 == News_VerifiedByFP)
						{
							if(IsVryMF) ExVryBind(&VryBind, pin, 1, 0);
							else	if(IsVryFgID < 10) ExVryBind(&VryBind, pin, 0, IsVryFgID);
							else	tmp = -8;
						}
						else	tmp = -8;
						IsVryMF = 0;	
						IsVryFgID = 0xff;
						if(tmp == 0) tmp = L3000CheckVryBind(&VryBind);
						if(tmp < 0)
						{
							if(tmp == -8){
								L3000CheckVryFailCnt(0);
								gState.dis_vry_tick = DIS_VRY_TICK;
								L3000ShowForbid(NULL, L3000_STR_REPLACE21, 0, 3);
							}				
							else	if(tmp == -1 || tmp==-2){
								char str[50];
								PUser user = FDB_GetUser(pin, NULL);

								sprintf(str, "%05d", user->PIN2);
								gState.dis_vry_tick = DIS_VRY_TICK;
								L3000ShowInfo(str, L3000_STR_REPLACE20, 0, 3);
							}
							else	if(tmp == -4){
								L3000CheckVryFailCnt(0);
								L3000ShowVerifyInValid(110);
							}
							else	L3000ShowVerifyInValid(110);
						}
						else	
						{
							memset(&VryBind, 0, sizeof(TVryBind));
							SaveAttLog(pin, (char)(msg->Param1-News_VerifiedByPwd));
						}
					}
					break;
				}
				if(gOptions.PowerMngFunOn) WaitPowerOff=gOptions.IdleMinute*60; else WaitPowerOff=0;
				PowerSuspend=gOptions.IdlePower==HID_SUSPEND;
				ShowMainLCDDelay = 3;
				gFlashLED=0;
				break;
			case News_FailedByPIN:
			case News_FailedByPwd:
			case News_FailedByFP:
			case News_FailedByIDCard:
			case News_FailedByMFCard:
				{
					printf("[%s:%d] News_Failed\n", __func__, __LINE__);
					if(gOptions.MasterSlaveFunOn && gOptions.MasterSlaveOn==2)
					{
						//ShowMainLCDDelay(10);
						break;
					}
					L3000CheckVryFailCnt(0);
					L3000ShowVerifyInValid(0);
		printf("%s:%d\n", __func__, __LINE__);
					//	ShowVerifyFail(i,News_VerifiedByFP, FALSE, (FPData.TempSize>0)?FPData.PIN:0,VOICE_RETRY_FP);
					if(FPData.TempSize || (KeyBufferTimeOut && KeyBufferIndex && gOptions.One2OneFunOn))
					{
						/*		if(gLCDWidth==0)
								{
								ShowMainLCDDelay(6);
								gFlashLED=5;
								ExLightLED(LED_RED, FALSE);
								}else
								ShowMainLCDDelay(2);*/
		printf("%s:%d\n", __func__, __LINE__);
						if(MfVryCnt){
		printf("%s:%d\n", __func__, __LINE__);
							char buf[30];					
							L3000ExBeep(100);
							DelayMS(80);
							L3000ExBeep(100);
							DelayMS(100);
							sprintf(buf, "%05d", FPData.PIN);
							L3000ShowCardVry(LoadStrByID(HID_CARDFP), buf, LoadStrByID(HID_PLACEFINGER), 0, 6);
						}

					}
					else
						ShowMainLCDDelay = 2;
				printf("%s:%d\n", __func__, __LINE__);
				}
		}
		break;
	case MSG_TYPE_TIMER_500MS:
		led_flash = !led_flash;
		if(led_flash){
			//__gpio_clear_pin(96);
		}
		else{
			//__gpio_set_pin(96);				
		}
			

		if(gFlashLED>0)
		{
			ExLightLED(LED_GREEN, FlashGreenLED);
			FlashGreenLED=!FlashGreenLED;
		}
		
		{
			static int sec=0;
			if(sec==1)
			{
				sec=0;
				#if 1
				if(ShowMainLCDDelay)
				{
					if(0==--ShowMainLCDDelay)
					{
						ShowDelayMainLCD();
					}
				}
				#endif
			}
			else
				sec++;
		}
		break;
	case	MSG_TYPE_TIMER_BASEMS: 
//		L3000ProcTickMsg(NULL);
	/*
		if(L3000CheckKeyDel())
		{
			ConstructMSG(msg, MSG_TYPE_BUTTON, IKeyDel, 0);
//			DebugOutput("\nhave a IKeyDel msg!\n");
		}
	*/
		break;
	}
	return 1;
}

#ifndef URU					  
int CalcFingerCenter(int MaxWidth, int MaxHeight)
{
	int Corners[8], VHeight, VWidth, GridValue, result=0;
	char buf[40];
	DBPRINTF("Start Calibrating...");
	LCDClear();
	strcpy(buf,"Calibrating...");
	LCDWriteCenterStr(0, buf);
	InitSensor(0, 0, CMOS_WIDTH, CMOS_HEIGHT, gOptions.NewFPReader);						
	if(!CaptureSensor(gImageBuffer, ONLY_LOCAL, &SensorBufInfo))
		CaptureSensor(gImageBuffer, ONLY_LOCAL, &SensorBufInfo);
	if (0)
	//if(ZFSearchValidArea((BYTE*)gImageBuffer, CMOS_WIDTH, CMOS_HEIGHT, 
	//	MaxWidth, MaxHeight,
	//	Corners, &VHeight, &VWidth, &GridValue)>0)	
	{
		DBPRINTF("SearchValidArea OK.");
		VWidth=((VWidth+2)/4)*4;
		gOptions.ZF_HEIGHT=VHeight;
		gOptions.ZF_WIDTH=VWidth;
		gOptions.OImageWidth=((Corners[2]-Corners[0]+2)/4)*4;
		gOptions.OImageHeight=Corners[1]-Corners[7];
		gOptions.OLeftLine=CMOS_WIDTH-Corners[2];
		gOptions.OTopLine=CMOS_HEIGHT-Corners[1];
		gOptions.OLeftLine=(gOptions.OLeftLine+2)/4*4;
		gOptions.CPX[1]=Corners[4]-Corners[0];
		gOptions.CPX[0]=Corners[6]-Corners[0];
		gOptions.CPX[3]=0;
		gOptions.CPX[2]=gOptions.OImageWidth-1;
		gOptions.CPY[0]=gOptions.OImageHeight-1;
		gOptions.CPY[1]=gOptions.OImageHeight-1;
		gOptions.CPY[2]=0;
		gOptions.CPY[3]=0;
		FPInit(NULL);
		DBPRINTF("Write Result to LCD.");
		strcpy(buf,"OK            ");
		LCDWriteStr(0,0,buf,0);
		if(gLCDRowCount>3)
		{
			sprintf(buf, "W=%d", VWidth);
			LCDWriteStr(2,10,buf,0);
			sprintf(buf, "H=%d", VHeight);
			LCDWriteStr(3,10,buf,0);
		}
		else
		{
			sprintf(buf, "W=%d,H=%d", VWidth, VHeight);
			LCDWriteStr(1,0,buf,0);
		}
		result=1;
		ExBeep(1); DelayMS(140); ExBeep(1);
	}
	else
	{
		LCDWriteStr(0,0,"Calibrate Fail! ", 0);
		ExBeep(2);
	}
	DBPRINTF("Reset CMOS Parameters...");
	InitSensor(gOptions.OLeftLine,gOptions.OTopLine,gOptions.OImageWidth,gOptions.OImageHeight, gOptions.NewFPReader);
	CaptureSensor(gImageBuffer, ONLY_LOCAL, &SensorBufInfo);
	DBPRINTF("Finished calibrate.");
	return result;
}
#endif

void ShowTestFinger(void)
{
	LCDClear();
	LCDWriteStr(0,0,"Testing", 0);
	LCDWriteStr(1,0,"FPSensor", 0);
	LCDWriteStr(3,0,"ESC-Exit", 0);
}

int ExFeedWatchDog(int i)
{    
    return i;
}        

TTime getMonthDayTime(int MonthDayTime)
{
    TTime tt={0,0,0,1,1,2004,4,0,0};
    tt.tm_year=gCurTime.tm_year;
    tt.tm_min=MonthDayTime & 0xFF;
    tt.tm_hour=MonthDayTime>>8&0xFF;
    tt.tm_mday=MonthDayTime>>16&0xFF;
    tt.tm_mon=MonthDayTime>>24&0xFF;
    return tt;   
}                  

int IsDaylightSavingTime()
{   
    //如果当前时间在夏令开始和结束(即非夏令开始时间)的区间内，则为夏令，否则不是。
    TTime DaylightSavingTime,StandardTime;
    //int c1;
    DaylightSavingTime=getMonthDayTime(gOptions.DaylightSavingTime);
    StandardTime=getMonthDayTime(gOptions.StandardTime);
    //c1=TimeDiffSec(gCurTime,StandardTime);
    if(TimeDiffSec(gCurTime,DaylightSavingTime)>=0 && TimeDiffSec(gCurTime,StandardTime)<0)
        return 1;  
    else
        return 0;
}

void ShowDelayMainLCD(void)
{       
    ShowMainLCDDelay=0;
    FPData.TempSize=0;
    KeyBufferTimeOut=0;
    KeyBufferIndex=0;
    HackerWait=0;
#ifndef MODULE
    ShowMainLCD();
#endif
    FlashGreenLED=0;
    if(gOptions.RS232Fun==13)
    {
        char tmp1[20];
        *tmp1=1;
//        SendExCommand3(98, tmp1);
    }
}
void	L6000Debug(char *s, int va)
{
#ifdef PRINTF
	char	ss[200];
	
	sprintf(ss, "\n%s: 0x%x\n", s, va);
	DebugOutput(ss);
#endif
}
int	L3000CheckShowProgTime()
{
  	return	0;
}
int     L3000CheckShowFgTime()
{
		return 0;
}

void llock_test(void)
{
	int	ret;
	char buf[16];

	Battery_Init();

	while(1)
	{
		ret =  (Read_Battery() * 38 + 5)/10;
		sprintf(buf, "%d", ret);
		LCDWriteCenterStr(2, buf);
		OSTimeDly(10); 
	}
}

