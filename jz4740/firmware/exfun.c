/*************************************************
  
 ZEM 200                                          
 
 exfun.c time and voice and access control function                              
 
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
 
 $Log: exfun.c,v $
 Revision 5.14  2006/03/04 17:30:09  david
 Add multi-language function

 Revision 5.13  2005/12/22 08:54:23  david
 Add workcode and PIN2 support

 Revision 5.12  2005/08/15 13:00:22  david
 Fixed some Minor Bugs

 Revision 5.11  2005/08/13 13:26:14  david
 Fixed some minor bugs and Modify schedule bell

 Revision 5.10  2005/08/07 08:13:15  david
 Modfiy Red&Green LED and Beep

 Revision 5.9  2005/08/04 15:42:53  david
 Add Wiegand 26 Output&Fixed some minor bug

 Revision 5.8  2005/08/02 16:07:51  david
 Add Mifare function&Duress function

 Revision 5.7  2005/07/14 16:59:53  david
 Add update firmware by SDK and U-disk

 Revision 5.6  2005/06/10 17:11:01  david
 support tcp connection

 Revision 5.5  2005/05/13 23:19:32  david
 Fixed some minor bugs

 Revision 5.4  2005/04/27 00:15:37  david
 Fixed Some Bugs

 Revision 5.3  2005/04/24 11:11:26  david
 Add advanced access control function

*************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "arca.h"
#include "lcm.h"
#include "exfun.h"
#include "wavmain.h"
#include "options.h"
#include "msg.h"
#include "main.h"
#include "rtc.h"
#include "sensor.h"
#include "wiegand.h"
#include "serial.h"
#include "accapi.h"
#include "kb.h"
#include "threadprio.h"
#include "jz4740.h"
#include <ucos_ii.h>

static TMyBuf *buff_in=NULL, *buff_out1=NULL, *buff_out2=NULL;
static char WavFilePath[80]="NONE";

#define NUMBERWAV 15
static unsigned int SoundLoaded=0;
static unsigned char *wavbuf[NUMBERWAV];

int gAlarmDelay=0,gAlarmDelayIndex=0,gBellDelay=0;
unsigned char udc_prio = UDCPRIO_HIGH;


#ifndef URU
int PlayWavFileAsync(char *wavname)
{
	return PlayWavFile(wavname);	
}
#else
int PlayWavFileAsync(char *wavname)
{
	return PlayWavFile(wavname);	
}
#endif

int ExPlayVoice(int VoiceIndex)
{
	int rc;
#ifdef PLAYSOUNDMEM
	printf("ExPlayVoice %dth wav file. Address:%p\n",VoiceIndex,wavbuf[VoiceIndex]);
/*
	int i;
        for(i=0; i<NUMBERWAV; i++)
                if(SoundLoaded&(1<<i))
                        printf("the %dth wav file address: %p\n",i,wavbuf[i]);
*/
	if( (SoundLoaded&(1<<VoiceIndex)) && wavbuf[VoiceIndex]!=NULL )
	rc=PlayWavFileAsync((void *)wavbuf[VoiceIndex]);
#else
	char *VoiceWavCmd[3] = {"main", "", NULL};
	char buffer[80];
	
	if (strcmp(WavFilePath, "NONE")==0)
		if (!LoadStr("WAVFILEPATH", WavFilePath)) memset(WavFilePath, 0, 80);  
	//play X_Y.wav audio file
	sprintf(buffer, "%s%c_%d.wav", WavFilePath, gOptions.Language, VoiceIndex);	
	VoiceWavCmd[1] = buffer;
	rc=PlayWavFileAsync(buffer);
	if (rc)
	{

		if (LoadInteger(CustomVoice, 0))
		{
			sprintf(buffer, "%s%c_%d.wav", WavFilePath, gOptions.Language+32, VoiceIndex);	
			VoiceWavCmd[1] = buffer;
			rc=PlayWavFileAsync(buffer);			
		}
	}
	if (rc)
	{
		//Retry to play Y.wav audio file
		sprintf(buffer, "%s%d.wav", WavFilePath, VoiceIndex);	
		VoiceWavCmd[1] = buffer;
		rc=PlayWavFileAsync(buffer);
	}
#endif
	return rc;
}

void ExPlayVoiceFrom(int VoiceStart, int VoiceEnd)
{
	int i;
	
	for(i=VoiceStart; i<=VoiceEnd; i++) ExPlayVoice(i);
}

void ExBeep(int delay)
{
#ifdef PLAYSOUNDMEM
	if( (SoundLoaded&(1<<(NUMBERWAV-1))) && wavbuf[NUMBERWAV-1]!=NULL )
	PlayWavFileAsync((void *)wavbuf[NUMBERWAV-1]);
#else
	char *VoiceWavCmd[3] = {"main", "beep.wav", NULL};
	char buffer[80];
	int i=0;
	
	if (strcmp(WavFilePath, "NONE")==0)
		if (!LoadStr("WAVFILEPATH", WavFilePath)) memset(WavFilePath, 0, 80);
	sprintf(buffer, "%sbeep.wav", WavFilePath);
	VoiceWavCmd[1]=buffer;
	for(i=0;i<delay;i++)
	{
		PlayWavFileAsync(buffer);
		if(i<(delay-1)) DelayMS(20);
	}
#endif
}

#ifdef MP3PLAY
int ExPlayMP3(int mp3index, BOOL IsTestMP3)
{
	return 0;
}
#else
int ExPlayMP3(int mp3index, BOOL IsTestMP3)
{
	return 0;
}
#endif

void ExLightLED(int LEDIndex, int Light)
{
	if (gOptions.RFCardFunOn&&(!gOptions.IsWiegandKeyPad)&&(!gOptions.MifareAsIDCard)) return;
	
	if (LEDIndex==LED_RED)
	{	
		if (gOptions.IsWiegandKeyPad)
			GPIOSetLevel(IO_FUN_RED_LED, Light);
		else
		{
			if (gOptions.IsFlashLed == 4)
				GPIO_WIEGAND_LED(WEI_DN, !Light);
			else
				GPIO_WIEGAND_LED(WEI_DN, Light); 

		}
	}
	else
	{
		if (gOptions.IsWiegandKeyPad)
			GPIOSetLevel(IO_FUN_GREEN_LED, Light);
		else
		{
			if (gOptions.IsFlashLed==4)
				GPIO_WIEGAND_LED(WEI_DP, !Light);
			else
				GPIO_WIEGAND_LED(WEI_DP, Light);
		}
	}
}

BOOL ExPowerOff(int Cmd)
{

	if(Cmd==FALSE) //shutdown
	{
	        FDB_FreeDBs();
        	LCDClear();
        	ExLCDClose();
        	yaffs_sync("/mnt");
        	yaffs_unmount("/mnt");
        	yaffs_unmount("/flash");
        	DelayUS(100*1000);
        	GPIO_SYS_POWER(gOptions.DevID);
        	return TRUE;

	}else if(Cmd==TRUE) //sleep
		GoToSleepStatus();	
	return TRUE;
}                                                                                                                                                                                                                             

void GetTime(TTime *tm)
{
	TTime *t = (tm == NULL) ? &gCurTime : tm;
#if 1
	ReadRTCClockToSyncSys(t);
#else
	GetOSTime(t);
#endif
}

time_t GetSecond(void)
{
	TTime t;
	GetTime(&t);
	return EncodeTime(&t);
}

void SetTime(TTime *t)
{
	time_t tt;
	
	//fix tm_wday tm_yday
//	tt = EncodeTime(t);
	
	//setup RTC CLOCK 
//	SetRTCClock(t);
	SetRTCClock(t);
	DelayUS(100*1000);
	
	//synochronize system time from RTC
	ReadRTCClockToSyncSys(t);
	GetTime(t);
}

time_t EncodeTime(TTime *t)
{	
	time_t tt;
	
	//夏令时 = 没有信息
	t->tm_isdst = -1; 
	tt = mktime_1(t);
	return tt;
}

time_t OldEncodeTime(TTime *t)
{
	time_t tt;
	
	tt=((t->tm_year-100)*12*31+((t->tm_mon)*31)+t->tm_mday-1)*(24*60*60)+
	   (t->tm_hour*60+t->tm_min)*60+t->tm_sec;
	return tt;
}

TTime * OldDecodeTime(time_t t, TTime *ts)
{
	ts->tm_sec=t % 60;
	t/=60;
	ts->tm_min=t % 60;
	t/=60;
	ts->tm_hour=t % 24;
	t/=24;
	ts->tm_mday=t % 31+1;
	t/=31;
	ts->tm_mon=t % 12;
	t/=12;
	ts->tm_year=t+100;
	return ts;
}
/*
TTime * DecodeTime(time_t t, TTime *ts)
{
	memcpy(ts, localtime(&t), sizeof(TTime));
	return ts;
}
*/
int TimeDiffSec(TTime t1, TTime t2)
{
	return (EncodeTime(&t1) - EncodeTime(&t2)); 
}

void GoToSleepStatus(void)
{
	char i = 0;
	LCDClear();
	EnableMsgType(MSG_TYPE_FINGER, FALSE);
	FreeSensor();
        GPIO_HY7131_Power(FALSE);
        GPIO_LCD_USB0_Power(FALSE);
	GPIO_AC97_Mute(FALSE);

        //whether display clock ":" or not
	ClockEnabled = FALSE;		
	//whether display main windows or not
	ShowMainLCDEnabled = FALSE;
//houwx20110726 for fpsensor Hibernation
        for(i=0;i<7;i++){
		__gpio_as_output(GPC*32+i);
                __gpio_clear_pin(GPC*32+i);
	}

}

/*
 * Very simple buffer management.
 *
 * io = 0 : request for input buffer
 * io = 1 : request for output buffer
 *
 * set buffer length to 0 to free the buffer.
 *
 */
PMyBuf bget(int io)
{
	if(buff_in==NULL)
	{
		buff_out2=(TMyBuf*)malloc(sizeof(TMyBuf));
		buff_out2->len=0;
		buff_out1=(TMyBuf*)malloc(sizeof(TMyBuf));
		buff_out1->len=0;
		buff_in=(TMyBuf*)malloc(sizeof(TMyBuf));
		buff_in->len=0;
	}
	if( io == 1) {
		if(buff_out1->len == 0) return buff_out1;
		else return buff_out2;
	}else return buff_in;
	
	//      DBPRINTF("%s:can't get buffer\n",__FUNCTION__);
	return 0;
}

void FreeCommuCache(void)
{
	if(buff_in != NULL) free(buff_in);
	if(buff_out1 != NULL) free(buff_out1);
	if(buff_out2 != NULL) free(buff_out2);
}

unsigned short in_chksum(unsigned char *p, int len)
{
	unsigned int sum = 0;
	unsigned short *tmp;

	tmp = (unsigned short *)p;

	while(len > 1) {
		sum += *tmp++;
		if( sum & 0x80000000 )
			sum = (sum & 0xFFFF) + (sum >> 16);
		len -= 2;
	}
	
	if(len)
	{
		sum += (unsigned short) *(unsigned char*)tmp;
	}
	
	while(sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);
	
	return ~sum;
}

void ExOpenRF(void)
{

}

void ExCloseRF(void)
{

}

BOOL iClassReadSN(int *in_data)
{
/*
	const int MaxChars=32;
  	int rdindex = 0;
	U8 data[MaxChars];
	int rdcnt = 200;
	
	if(ttl232.poll()) //data arrived
	{
		while(rdcnt&&rdindex<MaxChars)
		{
			if(ttl232.poll())
			{	
				data[rdindex++] = ttl232.read();
				//printf("%d\n", data[rdindex-1]);
			}
			else
			{
				DelayUS(1000);
				rdcnt--;				
			}
		}
		ttl232.flush_input();
		if(rdindex>=16)
		{
			*in_data=(int)((data[rdindex-2]*256+data[rdindex-1])/2);
			return TRUE;
		}
	}
*/
	return FALSE;
}

BOOL HIDCheckCard(char *buffer)
{	
/*
	int in_data=0;
	extern int gHIDiClassOpened;


	int res;
	
	if(gHIDiClassOpened&&iClassReadSN(&in_data))
	{
		memset(buffer, 0, 5);
		memcpy(buffer, &in_data, 4);
		return TRUE;
	}
	else if ((fd_wiegand>=0)&&(read(fd_wiegand, &in_data, 4)==4))
	{
		memset(buffer, 0, 5);
		memcpy(buffer, &in_data, 4);
               DBPRINTF("wiegand = %d \n", in_data);

		return TRUE;
	}
	else
*/
		return FALSE; 
}

void ExOpenWiegand(void)
{

}

void ExCloseWiegand(void)
{

}

BOOL WiegandSend(U32 deviceID, U32 CardNum, U32 DuressID)
{
/*
	U8 data[256];
	int c;		
	if(fd_dummy>=0)
	{
		if (gWGOEMCode <=0)
			c=CalcWiegandData(deviceID, CardNum, DuressID, data, &gWiegandDef);
		else
			c=CalcHID37WiegandData(gWGOEMCode,deviceID,CardNum,DuressID,data);
		data[c]='\0';
		DBPRINTF("Wiegand Data Sending......%s\n", data);
		write(fd_dummy, data, c);
		return TRUE;
	}
*/
	return FALSE;
}

extern int gAuxOutDelay;

void ExAuxOut(int AuxOnTime, int OpenDoorDelay)
{
	gAuxOutDelay=(int)((AuxOnTime*20)/1000)+2;
	GPIOSetLevel(IO_FUN_LOCK, TRUE);
}

extern int gAlarmDelay;

int ExAlarm(int Index, int Delay)
{
	if(gOptions.DoorAlarmMode==RELAY_NO)
		GPIOSetLevel(IO_FUN_WOD0, TRUE); //Output a LOW pulse
	else if(gOptions.DoorAlarmMode==RELAY_NC)		
		GPIOSetLevel(IO_FUN_WOD0, FALSE); //Output a HIGH pulse
	else if((gOptions.DoorAlarmMode==RELAY_NONE)&&Index) //BELL
		GPIOSetLevel(IO_FUN_WOD0, TRUE); //Output a LOW pulse		
	//Do ALARM
	if(gOptions.DoorAlarmMode==RELAY_NONE)
		gAlarmDelay=0;
	else
		gAlarmDelay=Delay;
	//Do BELL
	if(Index&&!Delay) //if delay time is 0 then delay 500ms to bell
	{
		DelayUS(500*1000); //500ms
		//Output a HIGH pulse
		GPIOSetLevel(IO_FUN_WOD0, FALSE);
	}
	return TRUE;
}

int ExAlarmOff(int Index)
{
	if(Index==0)
	{
		if(gOptions.DoorAlarmMode==RELAY_NO)
			GPIOSetLevel(IO_FUN_WOD0, FALSE); //Output a HIGH pulse
		else if(gOptions.DoorAlarmMode==RELAY_NC)
			GPIOSetLevel(IO_FUN_WOD0, TRUE); //Output a LOW pulse
	}
	return TRUE;
}

BYTE ExCheckStrip(void)
{
	int i=200,c=0;
	while(--i)
	{
		if(!GPIOGetLevel(IO_FUN_SENSOR)) 
			if(++c>20) 
				return (gOptions.DoorSensorMode?DOOR_SENSOR_OPEN:DOOR_SENSOR_CLOSE);
		DelayUS(5);
	}
	return (gOptions.DoorSensorMode?DOOR_SENSOR_CLOSE:DOOR_SENSOR_OPEN);
}

BYTE ExGetIOStatus(void)
{
	BYTE ret=0;	
	//sensor status
	ret|=ExCheckStrip()-DOOR_SENSOR_OPEN;
	//button
	ret|=ExCheckGPI(IO_FUN_BUTTON)<<1;
	//alarm
	ret|=ExCheckGPI(IO_FUN_WOD0)<<2;
	//alarm strip
	ret|=ExCheckGPI(IO_FUN_ALARMSTRIP)<<3;
	//Lock
	ret|=ExCheckGPI(IO_FUN_LOCK)<<4;
	return ret;
}

extern int gDoorSensorDelay;
extern int gAlarmStrip;

BOOL CheckDOOR(char *buffer)
{
	static BYTE DoorSensorStatus=DOOR_UNKNOWN;
	static BYTE DoorOpenSign=DOOR_UNKNOWN;
	BYTE status;	
			
	if(!gOptions.IsSupportC2)
	{
		if((gOptions.LockFunOn&LOCKFUN_DOORSENSOR)&&(gOptions.DoorSensorMode<2))
		{
			//Door sensor
			status=ExCheckStrip();
			if (status!=DoorSensorStatus)
			{
				DoorSensorStatus=status;
				if(status==DOOR_SENSOR_OPEN)
				{
					if(gAuxOutDelay)
					{
						buffer[0]=DOOR_SENSOR_OPEN;
						//gDoorSensorDelay=gOptions.OpenDoorDelay;
						DoorOpenSign=DOOR_SENSOR_OPEN;							
					}
					else	
					{
						buffer[0]=DOOR_SENSOR_BREAK;
						DoorOpenSign=DOOR_SENSOR_BREAK;
					}
					return TRUE;
				}
				if(status==DOOR_SENSOR_CLOSE)
				{
					gDoorSensorDelay=0;
					buffer[0]=DOOR_SENSOR_CLOSE;
					DoorOpenSign=DOOR_SENSOR_CLOSE;
					return TRUE;			
				}
			}
		}
		else
		{
			gDoorSensorDelay=0;
			DoorSensorStatus=DOOR_UNKNOWN;
			DoorOpenSign=DOOR_UNKNOWN;
		}
		//Door button
		if((gAuxOutDelay==0)&&ExCheckGPI(IO_FUN_BUTTON))
		{
			buffer[0]=DOOR_BUTTON;
			buffer[1]=0;
			return TRUE;	
		}
	}
	else
	{
		//nothing to do
	}
	//Door strip
	if((gAlarmDelay==0)&&ExCheckGPI(IO_FUN_ALARMSTRIP))
	{
		buffer[0]=DOOR_BREAK;
		buffer[1]=0;
		return TRUE;		
	}

	return FALSE;
}
/*
void Switch_mode(U32 RS232Mode)
{
	//Output a LOW pulse
	//Switch RJ45/RS232
	GPIOSetLevel(IO_FUN_BUTTON, RS232Mode); 
}
*/ 
int DoAlarm(int Index, int DelayMS)
{
	int ret =0;
        if(gOptions.IsSupportC2)
        {
                if(gOptions.DoorAlarmMode==0)
                        ret = ExAlarmC2(Index,DelayMS);
                else if(gOptions.DoorAlarmMode==1)
                        ret = ExAlarmC2(Index,0);
        }
        else
        {
                ret = ExAlarm(Index, DelayMS);
        }
	return ret;
}
int DoAlarmOff(int Index)
{
	int ret=0;
        if(gOptions.IsSupportC2)
        {
                if(gOptions.DoorAlarmMode==0)
                        ret = ExAlarmC2(Index,0);
                else if(gOptions.DoorAlarmMode==1)
                        ret = ExAlarmC2(Index,1);
        }
        else
        {
        	ret= ExAlarmOff(Index);
        }
	return ret;
}

void DoAuxOut(int AuxOnTime, int OpenDoorDelay)
{
        if(gOptions.IsSupportC2)
        {
                ExAuxOutC2(AuxOnTime,OpenDoorDelay);
                gAuxOutDelay=AuxOnTime/25;
		if (AuxOnTime && !gAuxOutDelay)
			gAuxOutDelay=1;
        }
        else
                ExAuxOut(AuxOnTime, OpenDoorDelay);
}
int DoBell(int DelaySec)
{
	if(DelaySec)
 		gBellDelay=DelaySec;
	return ExBellC2(DelaySec);
}

int DoBellOff(void)
{
	return ExBellC2(0);
}



/*
void CaptureImgAndProcess(U32 pin, U8 method)
{
	char *buffer;
	int size;
	
	//Take a photo by camera
	//...
	
	if(gOptions.CameraFunOn)
	{
		if(gOptions.VoiceOn) 
			ExPlayVoice(VOICE_CAMERACLICK);
		else
			ExBeep(2);						
	}
	
	switch(method)
	{
	case METHOD_AUTHSERVER:
		SendPhotoToAuthServer(pin, buffer, size);
		break;
	case METHOD_SAVETODISK:
		break;
	}
}
*/

void LoadSound(void)
{
#ifdef PLAYSOUNDMEM
	unsigned char i;
	char buffer[50];
	int fd=-1;
	int fsize, cnt;
        struct yaffs_stat s;

	SoundLoaded = 0;
        if(strcmp(WavFilePath, "NONE")==0)
                if (!LoadStr("WAVFILEPATH", WavFilePath)) memset(WavFilePath, 0, 80);

	cnt = 0;
	for(i=0; i<NUMBERWAV; i++)
	{
		wavbuf[i] = NULL;
		if(i==NUMBERWAV-1)
			sprintf(buffer, "%sbeep.wav", WavFilePath);
		else
			sprintf(buffer, "%s%c_%d.wav", WavFilePath, gOptions.Language, i);
		fd=open(buffer,O_RDONLY);
		if(fd<0)
		{
			printf("Open file %s  failed. fd=%d\n",buffer,fd);
			continue;
		}
                yaffs_stat(buffer, &s);
	#ifdef YAFFS1
		fsize = s.yst_size;
	#else
		fsize = s.st_size; //yaffs2
	#endif
		wavbuf[i] = (unsigned char *)malloc(fsize);
		if(wavbuf[i]==NULL)	
		{
			printf("malloc failed\n");
			continue;
		}
		if(read(fd, wavbuf[i],fsize)!=fsize)
		{
			free(wavbuf[i]);
			close(fd);
			printf("reading file %s failed\n",buffer);
			continue;
		}

		SoundLoaded |= (1<<i);
		cnt++;
	//	printf("%s have been Loaded to 0x%p, the file size is %d, fd=%d\n",buffer,wavbuf[i],fsize,fd);
		if(close(fd)<0)
			printf("close %s failed\n",buffer);
	}
#if 0
	printf("LoadSound from %p = %x %x %x %x %x\n",wavbuf[NUMBERWAV-1],wavbuf[NUMBERWAV-1][0],wavbuf[NUMBERWAV-1][1],wavbuf[NUMBERWAV-1][2],wavbuf[NUMBERWAV-1][3],wavbuf[NUMBERWAV-1][4]);
	for(i=0; i<NUMBERWAV; i++)
		if(SoundLoaded&(1<<i))
			printf("the %dth wav file address: %p\n",i,wavbuf[i]);
#endif
	printf("%d sound files have been loaded. SoundLoaded=%X\n", cnt,SoundLoaded);
#endif
}

void FreeSound(void)
{
	unsigned char i;

        for(i=0; i<NUMBERWAV; i++)
                if( (SoundLoaded&(1<<i)) && (wavbuf[i]!=NULL))
			free(wavbuf[i]);
	SoundLoaded = 0;
}

void yaffs_format_partition(const char *disk, char force);
char GetKeyPin(void);

int GetKeyValue(void)
{
	unsigned char KeyChar;
	int SecondFun;

	if(GetKeyPadValue(&KeyChar))
		return GetKeyChar(KeyChar, &SecondFun);
	else 
		return 0;
}

void Format_Data(char c)
{
	if(c&INQUIRING)
	{
		int key;
		int PreTime=0;
		char buf[8];
		unsigned char i;
		

		CheckKeyPad();
		if(GetKeyValue()!=IKeyMenu)
			return;

		LCDClear();
		LCDWriteCenterStr(0,"Press password");
		LCDWriteCenterStr(1,"to format data.");
		LCDWriteLine(3, "ESC	          OK");
		i = 0;
		while(1)
		{
			CheckKeyPad();
			key = GetKeyValue();
			if(key)
			{
				PreTime = OSTimeGet();
				if(key==IKeyOK)
					break;
				if(key==IKeyESC)
				{
					LCDClear();
					return;
				}
				if(key<'0' || key>'9')
					continue;
				buf[i++]=key;
				buf[i]='\0';
				LCDWriteCenterStr(2,buf);
				if(i>4)
					i=4;
			}
			if(OSTimeGet()-PreTime>1000)
				return;
		}
	
//		printf(" Format Data buf=%s\n",buf);
		if(strcmp(buf,"13577")==0)
		{
			LCDWriteCenterStr(2,"Formatting ...");
			yaffs_format_partition("/flash", 0);
		}
		else if(strcmp(buf,"24668")==0)
		{
			LCDWriteCenterStr(2,"Formatting ...");
			yaffs_format_partition("/flash", 1);
		}
		else
		{
			LCDWriteCenterStr(2,"Error! Try again");
			OSTimeDly(100);
		}
		return;
	}
	else
	{
		if(GetKeyPin()>=0)
		{	
			LCDClear();
			LCDWriteCenterStr(1,"Formatting data");
			LCDWriteCenterStr(2,"wait please ...");
		}
		else
			return;
		if(GetKeyPin()==0)
			yaffs_format_partition("/flash", 0);
		else if(GetKeyPin()==1)
			yaffs_format_partition("/flash", 1);	//Dont use this way to format flash in general

		return;
	}
}

void InitializeUDC(void)
{
#ifdef UDC
        /* Initialize UDC */
        InitUdcRam();
#endif
}



 //add by cn 2009-03-18
#define GPIO_UDC_DETE_PIN (32 * 3 + 29)
#define IOVSYNC (GPC*32+23)

void EnableIRQ_Com(int flag)
{
	if(flag)
		__gpio_unmask_irq(IOVSYNC);
	else
		__gpio_mask_irq(IOVSYNC);
}

void Close_UDC_IRQ(void)
{
#ifdef UDC
	 __intc_ack_irq(IRQ_UDC);
	 __intc_mask_irq(IRQ_UDC);
	__gpio_mask_irq(GPIO_UDC_DETE_PIN);
#endif
}


void ENABLE_UDC_IRQ(unsigned char e)
{
#ifdef UDC
        if(e)
        {
        //        __gpio_unmask_irq(IRQ_UDC);
                __intc_unmask_irq(IRQ_UDC);
        }
        else
        {
                __intc_ack_irq(IRQ_UDC);
        //        __cpm_stop_udc();
                __intc_mask_irq(IRQ_UDC);
        }
#endif
}
/*
void OSChangePrio(unsigned char x)
{
	if(x&UDCPRIO_LOW)
	{
		OSTaskChangePrio(UDC_THREAD_PRIO,UDC_THREAD_PRIO+80);
		udc_prio = UDCPRIO_LOW;
		printf("*** The prio of UDC have been changed to be low\n");
	}
	if(x&UDCPRIO_HIGH)
	{
		OSTaskChangePrio(UDC_THREAD_PRIO+80,UDC_THREAD_PRIO); 
		udc_prio = UDCPRIO_HIGH;
		printf("*** The prio of UDC have been changed to be high\n");
	}
}
*/

void OSTaskStkCheck(unsigned char prio)
{
	OS_STK_DATA  stk_data;
    	INT8U        err;

	err = OSTaskStkChk(prio, &stk_data);
	if (err == OS_NO_ERR) 
		printf("Checking stack of priority %d ok\n",prio);
	else
		printf("Checking stack of priority %d error: %d\n",prio,err);
}


void CheckRTCAdjust(void)
{
}

void SetCurrentTime(TTime *tt)
{
}

TTime *CalcDays(TTime *t)
{
	int y,c;
	y=t->tm_year-2000;
	c=1+(y-1)/4;
	c=c+365*(y-1);  //Total days count
	t->tm_yday=0;
	for(y=1;y<t->tm_mon;y++)
	{
		if(y==2)
		{
			if(t->tm_year%4==0) t->tm_yday+=29; else t->tm_yday+=28;
		}
		else if(y==4 || y==6 || y==9 || y==11)
			t->tm_yday+=30;
		else
			t->tm_yday+=31;
	}
	t->tm_yday+=t->tm_mday;
	c+=t->tm_yday;
	t->tm_wday=(c%7+6)%7;
	//2000-1-1==6
	return t;
}


int Buff2Time(char *buffer, TTime *Value)
{
	int v;
	TTime t;
	v=BCD2HEX(buffer[0]);
	if((v<0) || (v>59)) return 0;
	t.tm_sec=v;
	v=BCD2HEX(buffer[1]);
	if((v<0) || (v>59)) return 0;
	t.tm_min=v;
	v=BCD2HEX(buffer[2]);
	if((v<0) || (v>23)) return 0;
	t.tm_hour=v;
	v=BCD2HEX(buffer[3]);
	if((v<0) || (v>31)) return 0;
	t.tm_mday=v;
	v=BCD2HEX(buffer[4]);
	if((v<0) || (v>12)) return 0;
	t.tm_mon=v;
	v=BCD2HEX(buffer[6])%100+2000;
	if((v<0) || (v>2036)) return 0;
	t.tm_year=v;
	CalcDays(&t);
	*Value=t;
	return 1;
}

int AdjustRTC(TTime *t)
{
	return 0;	//暂时取消校准功能
}
