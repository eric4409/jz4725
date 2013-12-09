/*************************************************
                                           
 ZEM 200                                          
                                                    
 msg.c message process and gather functions                               
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
 
 $Log: msg.c,v $
 Revision 5.20  2006/03/04 17:30:09  david
 Add multi-language function

 Revision 5.19  2005/12/22 08:54:23  david
 Add workcode and PIN2 support

 Revision 5.18  2005/11/06 02:41:34  david
 Fixed RTC Bug(Synchronize time per hour)

 Revision 5.17  2005/09/19 10:01:59  david
 Add AuthServer Function

 Revision 5.16  2005/08/18 07:16:56  david
 Fixed firmware update flash error

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
l
 Revision 5.5  2005/04/24 11:11:26  david
 Add advanced access control function

 Revision 5.4  2005/04/21 16:46:44  david
 Modify for HID Cardusernumber++;

 Revision 5.3  2005/04/07 17:01:45  david

 Modify to support A&C and 2 row LCD

*************************************************/

#include <stdlib.h>
#include <string.h>
#include "arca.h"
#include "msg.h"
#include "exfun.h"
#include "options.h"
#include "kb.h"
#include "main.h"
#include "sensor.h"
#include "commu.h"
#include "rs232comm.h"
#include "serial.h"
#include "zlg500b.h"
#include <ucos_ii.h>
#include <jz4740.h>
#include	"L3000Operate.h"

#define IDLE_LOOP_TIME 10*1000*1000

//#define TESTFP
//#define TESTYAFFS	
int picfd=-1;	//just for testing

extern int WaitAdminRemainCnt;
extern int C2connect;
static U32 TimerCount=0;
static U32 EnabledMsg=0;

//extern OS_EVENT key_sem;
//OS_SEM_DATA key_data;
extern unsigned char KeyUp;
#define KEY_WAIT_TIME	5

static PMsgProc *MessageProcs=NULL;
static int MsgProcCount=0;

//add by cn 2009-03-22
int pretime = 0;
static unsigned char  First = 1;

TSensorBufInfo SensorBufInfo;

int HasInputControl(void)
{
	return MsgProcCount>1;
}

int TestEnabledMsg(int MsgType)
{
	return EnabledMsg & MsgType;
}

void EnableMsgType(int MsgType, int Enabled)
{
	if(Enabled)
	{
		EnabledMsg=EnabledMsg|MsgType;
		switch(MsgType)
		{
		case MSG_TYPE_TIMER: 
			TimerCount=0;
			break;
		}
	}
	else
	{
		EnabledMsg=(EnabledMsg & ~MsgType);
	}			
}

int GetMsg(PMsg msg)
{
	int m;
	while((m=GatherMsgs(msg))==0);
	return m;
}

extern int KeyBufferIndex;
extern TFPCardOP FPData;
extern int CommSessionCount;
extern int gLocalCorrectionImage;	
extern int gEthOpened;
extern serial_driver_t *gSlave232;
extern int gHaveRTC;
int gFPDirectProc=0;

char *TestBuffer;

static	U32 MFTick = 0;
static	U32 MFTickOld = 0;
static	int	MFEnable = 0;
static U32 LastTick=0;

static  U32 cfpDelay=0, cfpDelayStart=0;
#define DELAYCAPFP(delay_ms) \
	do{cfpDelay=delay_ms; cfpDelayStart=GetMS();}while(0)

TTime MachineBaseTime;

int GatherMsgs(PMsg msg)
{        
	unsigned char CurKeyChar; 
	int i=MCU_CMD_NONE,ret=0,value=0;
	static int ErrorCount=0;
	int status;
	U8 buffer[10];
	static int Motor=0;

//	error = OSSemQuery(&key_sem,&key_data);
//	printf(" ###### error=%d, cnt=%d\n",error,key_data.OSCnt);
	/* switch the task to be key task for scan key is up or not */
	/* delete by chenyy
	if(!KeyUp)
		OSTimeDly(KEY_WAIT_TIME);

	//check the key be pressed or not
	if(!gOptions.IsWiegandKeyPad) CheckKeyPad();
	*/
	MFTick = GetMS();
	if(MFTick >= (MFTickOld + 500) || MFTick < MFTickOld)
	{
		MFTickOld = MFTick;
		MFEnable = 1;		
	}
	else	MFEnable = 0;
	{
		if(gOptions.MasterSlaveFunOn && gOptions.MasterSlaveOn==1)
		{
			//MODCheck(gSlave232);
		}
	//	if(gOLED_En == 0)	i=BT232Check(buffer);
	 	if(MFEnable == 0)	i = L3000MsgCheck(buffer);
	}
#if 0
	//if(i==MCU_CMD_RTC)
	{
	//	DebugOutput1("sec :%d", buffer[0]);
		GetTime(NULL);
		if(MachineBaseTime.tm_sec != gCurTime.tm_sec)
		{
			ExFeedWatchDog(10);	
			ConstructMSG(msg, MSG_TYPE_TIMER,  msg->Param1,
				(gCurTime.tm_min == MachineBaseTime.tm_min) ? Timer_Second:Timer_Minute);
			MachineBaseTime=gCurTime;
//				ResetBaseTime();
			CheckSessionTimeOut();
			return MSG_TYPE_TIMER;
		}
	}
#endif
	if(EnabledMsg & MSG_TYPE_BUTTON && i==MCU_CMD_KEY)
	{     
		int	key = 0;
		gUserOperateTime = 0;
	//	DebugOutput1("\n process one key %d\n", buffer[0]);
		//msg->Message=MSG_TYPE_BUTTON;
//		if(gOLED_En == 0)	msg->Param1 = GetKeyChar(buffer[0], &msg->Param2);
//		else
		key = L3000GetKeyChar(buffer[0]);
		if(key)
		{
			msg->Message = MSG_TYPE_BUTTON;
			msg->Param1 = key;
			CheckSessionSend(EF_BUTTON,(char*)&(msg->Param1),sizeof(msg->Param1));
			return MSG_TYPE_BUTTON;
		}		
	}
	else	if(/*EnabledMsg & MSG_TYPE_SWITCH &&*/ i == MCU_CMD_SWITCH)
	{
		msg->Message = MSG_TYPE_SWITCH;
		msg->Param1 = buffer[0];
		msg->Param2 = buffer[1];
	//	{
	//		char str[100];

	//		sprintf(str, "\nswi_msg: %d   %d\n", buffer[0], buffer[1]);
	//		L3000Debug(str);

	//	}
		return	MSG_TYPE_SWITCH;	
	}

	if(L3000CheckKeyDel())
	{
		ConstructMSG(msg, MSG_TYPE_BUTTON, IKeyDel, 0);
		return MSG_TYPE_BUTTON;
	}

	//timer event
	if(EnabledMsg & MSG_TYPE_TIMER)
	{
		TTime t;
		BYTE TimerType;
		static int ms = 0;

		if(GetMS() - ms > 500)
		{
			ms = GetMS();
			GetTime(&t);
		}

		if(t.tm_sec != gCurTime.tm_sec)
		{
			TimerType=(gCurTime.tm_min==t.tm_min?Timer_Second:Timer_Minute);
			if ((TimerType==Timer_Minute)&&(gCurTime.tm_hour!=t.tm_hour))
				TimerType=Timer_Hour;
			ConstructMSG(msg, MSG_TYPE_TIMER,  msg->Param1, TimerType);
			memcpy(&gCurTime, &t, sizeof(TTime));
			CheckSessionTimeOut();
	 		wdt_set_count(0);
			pretime++;
		/*	if(pretime++>30 && First)
			{
				OSChangePrio(1);
				First = 0;
			} */
		//	printf(" >>>>>>>>> %d ,EnabledMsg=%d\n",t.tm_sec, EnabledMsg & MSG_TYPE_FINGER);
			return MSG_TYPE_TIMER;
		}
	}
	else
		wdt_set_count(0);

	//NOTE: RS232/RS485复用同一个UART PORT 不能同时使用
	//下面用到gCurTime 因此在MSG_TYPE_TIMER下面执行!!!
	if(gOptions.RS232On||gOptions.RS485On)
	{
		//2007.03.01 进菜单的话， 不检测通讯
		if (gMachineState != STA_MENU)
			RS232Check(&ff232);
	}
	#ifdef UDC
	USBCommCheck();
	#endif

	#ifdef UDC_UART
        RS232Check(&usb232);
	#endif
#if 0
	if(EnabledMsg & MSG_TYPE_BUTTON && (GetKeyPadValue(&CurKeyChar)))
	{
		ConstructMSG(msg, MSG_TYPE_BUTTON, GetKeyChar(CurKeyChar, &msg->Param2), msg->Param2);
		if(msg->Param1)
			CheckSessionSend(EF_BUTTON,(char*)&(msg->Param1),sizeof(msg->Param1));
		if (gOptions.Must1To1) FlushSensorBuffer();
		return MSG_TYPE_BUTTON;	
	}
#endif
/*
	else if((gOptions.IsWiegandKeyPad||gOptions.RFCardFunOn) && HIDCheckCard(buffer))
	{

		memcpy(&value, buffer, 4);
		if(value)
		{
			if (gOptions.IsWiegandKeyPad&&(((value>>16)&0xff)==255)&&((value&0xffff)<=FunKeyCount)) //KEY
			{
				GetKeyFromWiegand(value&0xffff);
			}
			else if ((EnabledMsg & MSG_TYPE_HID) && (gOptions.RFCardFunOn)) //HID
			{
				ConstructMSG(msg, MSG_TYPE_HID, status, value);
				if(msg->Param2)
					CheckSessionSend(EF_HIDNUM, (char*)&(msg->Param2), sizeof(msg->Param2));
				return MSG_TYPE_HID;
			}
		}
	}

	else if((gOptions.LockFunOn&LOCKFUN_ADV)&& (gOptions.LockFunOn != LOCKFUN_ADV) && CheckDOOR(buffer))
	{
		if(buffer[0]==DOOR_SENSOR_BREAK)        //门磁报警
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Door_Sensor, DOOR_SENSOR_BREAK);
			return MSG_TYPE_CMD;
		}
		else if(buffer[0]==DOOR_SENSOR_OPEN)	//门磁:正常开门
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Door_Sensor, DOOR_SENSOR_OPEN);
			return MSG_TYPE_CMD;
		}
		else if(buffer[0]==DOOR_SENSOR_CLOSE)   //门磁：正常关门
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Door_Sensor, DOOR_SENSOR_CLOSE);
			return MSG_TYPE_CMD;
		}
		else if(buffer[0]==DOOR_BUTTON)		//出门开关
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Door_Button, buffer[1]);
			return MSG_TYPE_CMD;
		}
		else if(buffer[0]==DOOR_BREAK) 		//拆机报警
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Alarm_Strip, buffer[1]);
			return MSG_TYPE_CMD;
		}
	}	
	else if(gMFOpened&&(EnabledMsg&MSG_TYPE_MF)&&(i=MFCheckCard(buffer)))
	{
		if(gOptions.MifareAsIDCard)
		{
			memcpy(&value, buffer, 4);
			ConstructMSG(msg, MSG_TYPE_HID, status, value);
			if(msg->Param2)
				CheckSessionSend(EF_HIDNUM, (char*)&(msg->Param2), sizeof(msg->Param2));
			return MSG_TYPE_HID;			
		}
		else
		{
			ConstructMSG(msg, MSG_TYPE_MF, i, 0);
			return MSG_TYPE_MF;
		}
	}
	
*/ 	
//	return 0;
#if 1
	if( !gState.dis_vry_tick
		&& (!gState.state || gState.state == STA_COMM) 
		&& (!EnFPVry)	
//		&& gLock.nor_open!=LOCK_STATE_OPEN
		&&(EnabledMsg & MSG_TYPE_FINGER) 
		//&& ExIsFPReaderReady()
       	&& ((gMachineState!=STA_IDLE) ||WaitAdminVerifyCount)
       	&& (HasInputControl() 
			|| !gOptions.Must1To1 
			|| KeyBufferIndex 
			|| FPData.TempSize>0)
		&& cfpDelay==0 )
	{
		if((EnabledMsg & MSG_TYPE_FINGER) && (pretime>3) )
			ENABLE_UDC_IRQ(0);

		if((EnabledMsg & MSG_TYPE_FINGER) && ((gMachineState!=STA_IDLE) || WaitAdminRemainCnt) && (!gOptions.IsOnlyRFMachine))
		{
			DELAYCAPFP(10);
			if(gLocalCorrectionImage) status=ONLY_LOCAL;

#ifndef TESTFP
			//	if((gFPDirectProc&&AutoGainImage(3))||
			//		((!gFPDirectProc)&&CaptureSensor(gImageBuffer, status, &SensorBufInfo)&&FPTest(gImageBuffer)))
			if(CaptureSensor(gImageBuffer, status, &SensorBufInfo)&&FPTest(gImageBuffer))
#else
#ifdef TESTYAFFS
				static int FileLen;
			if(picfd<0)
			{
				picfd =	open("/mnt/mtdblock/picture",O_RDWR|O_CREAT, S_IREAD|S_IWRITE);
				FileLen = lseek(picfd, 0, SEEK_END);
				if(FileLen)
					Motor = 1;
			}
#endif	
			if(CaptureSensor(gImageBuffer, status, &SensorBufInfo)&&(Motor||FPTest(gImageBuffer)))
#endif


			{
#ifdef TESTFP
				if(Motor==0)
				{
#ifndef TESTYAFFS
					//TestBuffer=malloc(SensorBufInfo.RawImgLen);
					TestBuffer=malloc(2*gOptions.OImageWidth*gOptions.OImageHeight);
					memcpy(TestBuffer, gImageBuffer, SensorBufInfo.RawImgLen);
#else
					if(picfd)
					{
						lseek(picfd, 0, SEEK_SET);
						write(picfd, gImageBuffer, SensorBufInfo.RawImgLen);
						FileLen = SensorBufInfo.RawImgLen;
					}
#endif
				}
				else
				{
#ifndef TESTYAFFS
					SensorBufInfo.RawImgPtr=TestBuffer;
					SensorBufInfo.DewarpedImgPtr=TestBuffer;      
#else
					if(picfd)
					{
						lseek(picfd, 0, SEEK_SET);
						read(picfd,SensorBufInfo.RawImgPtr,FileLen);
						lseek(picfd, 0, SEEK_SET);
						read(picfd,SensorBufInfo.DewarpedImgPtr,FileLen);
					}
#endif
				}
				Motor++;
				DBPRINTF("Motor=%d\n", Motor);

				if(((Motor%2)==0))
				{
					DBPRINTF("Got finger\n");
					ConstructMSG(msg, MSG_TYPE_FINGER, status, (int)&SensorBufInfo);
					CheckSessionSend(EF_FINGER, NULL, 0);			
					return MSG_TYPE_FINGER;
				} 
#else
				ConstructMSG(msg, MSG_TYPE_FINGER, status, (int)&SensorBufInfo);
				CheckSessionSend(EF_FINGER, NULL, 0);			
				if(pretime>3)
					ENABLE_UDC_IRQ(1);
				return MSG_TYPE_FINGER;
#endif
			}
		}
		if((EnabledMsg & MSG_TYPE_FINGER) && (pretime>3) )
			ENABLE_UDC_IRQ(1);
	}
#endif
	if(CommSessionCount==0) DelayNS(IDLE_LOOP_TIME);

	//
	U32	tick_run;
	U32	tick_500;
	static	U32 LastTick50ms = 0;
	U32 CurTick=GetMS();

	if(cfpDelay)
	{
		if(CurTick-cfpDelayStart>cfpDelay || CurTick < cfpDelayStart) 
			cfpDelay=0;
	}

	if(L3000LowClk == 0){
		tick_run = RUN_TICK;
		tick_500 = 500;		
	}
	else{
		tick_run = ADJUST_CLK(RUN_TICK);//	tick_run = 12;//  = 16/27 * 20
		tick_500 = ADJUST_CLK(500);//tick_500 = 296;
	}

	if((CurTick-LastTick)>tick_500 || CurTick < LastTick)	//大于500毫秒
	{
		ConstructMSG(msg, MSG_TYPE_TIMER_500MS,  msg->Param1,
				Timer_Second);
		LastTick=CurTick;
		return MSG_TYPE_TIMER;
	}

	if(CurTick - LastTick50ms >= tick_run || CurTick < LastTick50ms) {
		if(gMachineState != STA_VERIFYING)  WriteWDT(25000);
		else	WriteWDT(8000);
		ConstructMSG(msg, MSG_TYPE_TIMER_BASEMS, CurTick, LastTick50ms);
		LastTick50ms  =  CurTick;
		return	MSG_TYPE_TIMER_BASEMS;
	}
	return 0;
}

int TranslateMsg(int MsgType, PMsg msg)
{
	return 1;
}

U32 SelectNewMsgMask(U32 newmsk)
{
	U32 oldmsk=EnabledMsg;
	EnabledMsg=newmsk;
	return oldmsk;
}

int ProcessMsg(PMsg msg)
{
	int i=MsgProcCount;
	while(i>0)
	{
		i--;
		if (MessageProcs[i](msg)) break;
	}
	return i>=0;
}

int DoMsgProcess(void *Obj, int ExitCommand)
{
	TMsg msg;
	int i;
	msg.Object=Obj;
	do
	{
		msg.Message=0;
		i=GetMsg(&msg);
		if(TranslateMsg(i, &msg))
		{
			while(msg.Message && !((msg.Message==MSG_TYPE_CMD) && (msg.Param1==ExitCommand)))
			{
				if(!ProcessMsg(&msg)) break;
			}
		}		
	}while(!((msg.Message==MSG_TYPE_CMD) && (msg.Param1==ExitCommand)));
	return msg.Param2;
}

int RegMsgProc(PMsgProc MsgProc)
{
	if(MessageProcs==NULL)
		MessageProcs=(PMsgProc*)malloc(sizeof(PMsgProc)*200);
	MessageProcs[MsgProcCount++]=MsgProc;
	return MsgProcCount-1;
}

int RegMsgProcBottom(PMsgProc MsgProc)
{
	int i=MsgProcCount-1;
	if(MessageProcs==NULL)
		MessageProcs=(PMsgProc*)malloc(sizeof(PMsgProc)*200);
	while(i)
		MessageProcs[i+1]=MessageProcs[i];
	MessageProcs[0]=MsgProc;
	MsgProcCount++;
	return 0;
}

int UnRegMsgProc(int index)
{
	int i=index;
	while(i<MsgProcCount-1)
	{
		MessageProcs[i]=MessageProcs[i+1];
	}
	MsgProcCount--;
	return MsgProcCount;
}
