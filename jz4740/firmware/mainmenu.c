/*************************************************
  
 ZEM 200                                          
 
 mainmenu.c                                
 
 Copyright (C) 2003-2005, ZKSoftware Inc.   
 
 $Log: mainmenu.c,v $
 Revision 5.23  2006/03/04 17:30:09  david
 Add multi-language function

 Revision 5.22  2005/12/22 08:54:23  david
 Add workcode and PIN2 support

 Revision 5.21  2005/11/06 02:41:34  david
 Fixed RTC Bug(Synchronize time per hour)

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

 Revision 5.3  2005/04/07 17:01:45  david
 Modify to support A&C and 2 row LCD
 
 Revision 5.2  2005/04/06 11:56:48  david
 Fixed some bugs on USB
 
 Revision 5.1  2005/04/05 21:38:11  david
 Add Support to update firmware by USB Flash Disk
 
*************************************************/

#include <stdlib.h>
#include <string.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
#include "arca.h"
#include "serial.h"
#include "exfun.h"
#include "msg.h"
#include "lcdmenu.h"
#include "flashdb.h"
#include "finger.h"
#include "zkfp.h"
//#include "net.h"
#include "utils.h"
#include "options.h"
#include "autotest.h"
#include "mainmenu.h"
#include "fpcardmng.h"
#include "commu.h"
#include "lcm.h"
#include "main.h"
#include "kb.h"
#include "yaffs_guts.h"

#if MACHINE_ID == 2
#include "ff.h"
#endif
//#include "usb_helper.h"
#include "sensor.h"
#include "exvoice.h"
#include	"L3000Msg.h"
#include	"L3000Operate.h"

extern unsigned char Icon_Menu_Option[];
extern unsigned char Icon_Menu_AttLog[];
extern unsigned char Icon_Menu_User[];
extern unsigned char Icon_Menu_Info[];
extern unsigned char Icon_Menu_Finger[];

char *g_usbpath = "2:";
static int gtmpver=0, gtmpkbeep=0;
char* MenuFmtStr(char *buf, int StrID, char *Value)
{
        return PadRightStrSID(buf, StrID, Value, MenuCharWidth);
}

char* MenuFmtInt(char *buf, int StrID, int Value)
{
        return PadRightIntSID(buf, StrID, Value, MenuCharWidth);
}

char *MenuFmtStrStr(char *buf, int width, char *Value)
{
/* 
       char *p=GetNextText(buf, (MenuCharWidth-width)*gLangDriver->CharWidth);
        if(p) *p=0;
        return PadRightStrStr(buf, buf, Value, MenuCharWidth);
*/
        if(gLangDriver->RightToLeft)
        {
                char *p=GetNextText(buf, width*gLangDriver->CharWidth);
                if(p) while(*p==' ') p++;
                return PadRightStrStr(buf, p, Value, MenuCharWidth);
        }
        else
        {
                char *p=GetNextText(buf, (MenuCharWidth-width)*gLangDriver->CharWidth);
                if(p) *p=0;
                return PadRightStrStr(buf, buf, Value, MenuCharWidth);
        }

}

char *MenuFmtStrInt(char *buf, int width, int Value)
{
/* 
       char vbuf[20];
        char *p=GetNextText(buf, (MenuCharWidth-width)*gLangDriver->CharWidth);
        if(p) *p=0;
        sprintf(vbuf,"%d",Value);
        return PadRightStrStr(buf, buf, vbuf, MenuCharWidth);
*/
        char vbuf[20];
        char *p;
        if(gLangDriver->RightToLeft)
        {
                p=GetNextText(buf, width*gLangDriver->CharWidth);
                if(p) while(*p==' ') p++;
                sprintf(vbuf,"%d",Value);
                return PadRightStrStr(buf, p, vbuf, MenuCharWidth);
        }
        else
        {
                p=GetNextText(buf, (MenuCharWidth-width)*gLangDriver->CharWidth);
                if(p) *p=0;
                sprintf(vbuf,"%d",Value);
                return PadRightStrStr(buf, buf, vbuf, MenuCharWidth);
        }


}

static char format[MAX_CHAR_WIDTH];

int DoShowResInfo(void *p)
{
	int ret;
	char buf[3][MAX_CHAR_WIDTH];
	PMenu m=NULL;
	m=CreateMenu(LoadStrByID(MID_INFO_RES),gOptions.MenuStyle,NULL, m);
	if (!gOptions.IsOnlyRFMachine)
		AddMenuItem(1,m,MenuFmtInt(buf[0],MID_SYSINFO_FINGER,gOptions.MaxFingerCount*100-FDB_CntTmp()),NULL,NULL);
	else
		AddMenuItem(1,m,MenuFmtInt(buf[0],MID_SYSINFO_USER,gOptions.MaxUserCount*100-FDB_CntUser()),NULL,NULL);
        AddMenuItem(1,m,MenuFmtInt(buf[1],MID_SYSINFO_ATTLOG,gOptions.MaxAttLogCount*10000-FDB_CntAttLog()),NULL,NULL);
        AddMenuItem(1,m,MenuFmtInt(buf[2],MID_SYSINFO_ADMINLOG,MAX_OPLOG_COUNT-FDB_CntOPLog()),NULL,NULL);

	ret=RunMenu(m);
	DestroyMenu(m);
	return ret;
}

int DoShowProTime(void *p)
{
	char buf[MAX_CHAR_WIDTH];
	int i;
	
	LCD_Clear();
	strcpy(buf, ProductTime);
        if(buf[10]==' ')
                i=10;
        else
                i=11;
        buf[i]=0;
	LCDWriteLineStrID(0,MID_OI_PT);
	LCDWriteCenterStr(1, buf);
	LCDWriteCenterStr(2, buf+i+1);
	LCDWriteCenterStrID(3,HID_CONTINUEESC);
	return InputLine(0,0,0,NULL);
}

int DoShowProOEM(void *p)
{
	LCD_Clear();
	LCDWriteLineStrID(0,MID_OI_OEM);
	LCDWriteCenterStr(1, OEMVendor);
	LCDWriteCenterStrID(3,HID_CONTINUEESC);
	return InputLine(0,0,0,NULL);
}

int DoShowProName(void *p)
{
	LCD_Clear();
	LCDWriteLineStrID(0,MID_OI_PN);
	LCDWriteCenterStr(1, DeviceName);
	LCDWriteCenterStrID(3,HID_CONTINUEESC);
	return InputLine(0,0,0,NULL);
}

int DoShowFWVer(void *p)
{
	char buf[10];
	LCD_Clear();
        strncpy(buf, MAINVERSION, 8); buf[8]=0;
        if(gLCDRowCount>2)
        {
                LCDWriteLineStrID(0,MID_OI_FWVER);
                LCDWriteCenterStr(1, buf);
                LCDWriteCenterStr(2, MAINVERSION+9);
        }
        else
        {
                LCDWriteCenterStr(0, buf);
                LCDWriteCenterStr(1, MAINVERSION+9);
        }
        LCDWriteCenterStrID(3,HID_CONTINUEESC);
        return InputLine(0,0,0,NULL);
}

int DoShowAlVer(void *p)
{
	char szInfo[64];
	LCD_Clear();
	if(!gOptions.IsOnlyRFMachine) 
	{
		LCDWriteLineStrID(0,MID_OI_ALGVER);
		memset(szInfo, 0, sizeof(szInfo));
		sprintf(szInfo, "%s%d%s", AlgVer, gOptions.ZKFPVersion, ".0");
		LCDWriteCenterStr(1, szInfo);
	}
	LCDWriteCenterStrID(3,HID_CONTINUEESC);
	return InputLine(0,0,0,NULL);
}

int DoShowProSN(void *p)
{
	char buf[MAX_CHAR_WIDTH];	
	LCD_Clear();
	strcpy(buf, SerialNumber);
	LCDWriteLineStrID(0,MID_OI_SN);
	LCDWriteCenterStr(1, buf);
	if(strlen(buf)>16)
		LCDWriteCenterStr(2, buf+16);
	LCDWriteCenterStrID(3,HID_CONTINUEESC);
	return InputLine(0,0,0,NULL);
}

int DoShowDevInfo(void *p)
{
	int ret;
	char buf[6][MAX_CHAR_WIDTH];
	PMenu m=NULL;
	m=CreateMenu(LoadStrByID(MID_INFO_DEV),gOptions.MenuStyle,NULL,m);
	if (!gOptions.IsOnlyRFMachine)
		AddMenuItem(1,m,MenuFmtInt(buf[0],MID_OI_ENNUM,gOptions.MaxFingerCount),NULL,NULL);
	else
		AddMenuItem(1,m,MenuFmtInt(buf[0],MID_OI_USERNUM,gOptions.MaxUserCount),NULL,NULL);
	AddMenuItem(1,m,MenuFmtInt(buf[1],MID_OI_ALNUM,gOptions.MaxAttLogCount),NULL,NULL);
        AddMenuItem(1,m,MenuFmtInt(buf[2],MID_SYSINFO_ADMINLOG,MAX_OPLOG_COUNT),NULL,NULL);
        AddMenuItem(1,m,LoadStrByID(MID_OI_PT),DoShowProTime,NULL);
        AddMenuItem(1,m,LoadStrByID(MID_OI_SN),DoShowProSN,NULL);
        if('?'!=*(OEMVendor))
                AddMenuItem(1,m,LoadStrByID(MID_OI_OEM),DoShowProOEM,NULL);
        AddMenuItem(1,m,LoadStrByID(MID_OI_PN),DoShowProName,NULL);
	if (!gOptions.IsOnlyRFMachine)
		AddMenuItem(1,m,LoadStrByID(MID_OI_ALGVER),DoShowAlVer,NULL);
        AddMenuItem(1,m,LoadStrByID(MID_OI_FWVER),DoShowFWVer,NULL);
	ret=RunMenu(m);
	DestroyMenu(m);
	return ret;
}

int DoShowSysInfo(void *p)
{
	char buf[10][MAX_CHAR_WIDTH];
	int ret;
	extern int g1ToNTemplates;
	PMenu m=NULL;
	m=CreateMenu(LoadStrByID(MID_SYSINFO),gOptions.MenuStyle,NULL,m);	
        AddMenuItem(1,m,MenuFmtInt(buf[0],MID_SYSINFO_USER,FDB_CntUser()),NULL,NULL);
	if (!gOptions.IsOnlyRFMachine)
		AddMenuItem(1,m,MenuFmtInt(buf[1],MID_SYSINFO_FINGER,FDB_CntTmp()),NULL,NULL);
        AddMenuItem(1,m,MenuFmtInt(buf[2],MID_SYSINFO_ATTLOG,FDB_CntAttLog()),NULL,NULL);
	//加入分组指纹统计功能
	if (gOptions.I1ToG)
	{
        	AddMenuItem(1,m,LoadStrByID(MID_GROUPFPINFO), DoShowGroupFpInfo, NULL);
	}
        AddMenuItem(1,m,MenuFmtInt(buf[3],MID_SYSINFO_ADMIN,FDB_CntAdminUser()),NULL,NULL);
	if (!gOptions.IsOnlyRFMachine)
		AddMenuItem(1,m,MenuFmtInt(buf[4],MID_SYSINFO_PWD,FDB_CntPwdUser()),NULL,NULL);
        AddMenuItem(1,m,MenuFmtInt(buf[5],MID_SYSINFO_ADMINLOG,FDB_CntOPLog()),NULL,NULL);

//	if(gOptions.AdvanceMatch)
//		AddMenuItem(0,m,MenuFmtInt(buf[6],MID_OI_1TON,g1ToNTemplates),NULL,NULL);
	
        AddMenuItem(1,m,LoadStrByID(MID_INFO_RES), DoShowResInfo, NULL);
        AddMenuItem(1,m,LoadStrByID(MID_INFO_DEV), DoShowDevInfo, NULL);
	ret=RunMenu(m);
	DestroyMenu(m);
	return ret;
}

int InputYesNoItem(PMsg p, int *OptionValue)
{
	char Items[MAX_CHAR_WIDTH];
	PMenu menu=((PMenu)p->Object);
	int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret;
	int col=gLCDCharWidth-3, index, values[]={1,0}; 
	sprintf(Items, "%s:%s", LoadStrByID(HID_YES), LoadStrByID(HID_NO));
	index=*OptionValue;
	ret=LCDSelectItemValue(row,col,3,Items,values,&index);
	if(ret==News_CommitInput)
	{
		if(index!=*OptionValue)
		{
			*OptionValue=index;
			gOptions.Saved=FALSE;
			MenuFmtStrStr(menu->Items[menu->ItemIndex].Caption, 3, GetYesNoName(index));
		}
	}
	return ret;
}

int InputValueOfItem(PMsg p, int width, int minv, int maxv, int *OptionValue)
{
	PMenu menu=((PMenu)p->Object);
	int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret, Value;
	int col=gLCDCharWidth-width;
	Value=*OptionValue;
	do
	{
		ret=InputNumber(row,col,width,&Value,minv,maxv,FALSE);
		if(News_CancelInput==ret || News_TimeOut==ret) return ret;
	}while(ret==News_ErrorInput);
	// News_CommitInput
	if(Value!=*OptionValue)
	{
		/*	
                MenuFmtStrInt(menu->Items[menu->ItemIndex].Caption,width,Value);
		*OptionValue=Value;
		gOptions.Saved=FALSE;
		*/
                if(!gLangDriver->RightToLeft||
                        (gLangDriver->GetTextWidthFun(gLangDriver,menu->Items[menu->ItemIndex].Caption)/gLangDriver->CharWidth>=MenuCharWidth))
                        MenuFmtStrInt(menu->Items[menu->ItemIndex].Caption,width,Value);
                else
                {
                        char value[20];
                        sprintf(value, "%d", Value);
                        PadRightStrStr(menu->Items[menu->ItemIndex].Caption, menu->Items[menu->ItemIndex].Caption, value, MenuCharWidth);
                }
                *OptionValue=Value;
                gOptions.Saved=FALSE;

	}
	return ret;
}

//--------------------------Enroll A User ON AuthServer--------------------------
#if 0 //treckle
typedef struct _RemoteEnrollSession_{
	int Index;
	int Enrolled;
	U32 pin2;
}TRemoteEnrollSession, *PRemoteEnrollSession;

int RunRemoteEnrollFinger(PMsg msg)
{
	U16 result;			
	PRemoteEnrollSession es=(PRemoteEnrollSession)(msg->Object);
	PSensorBufInfo SensorInfo=(PSensorBufInfo)msg->Param2;

	if(MSG_TYPE_TIMER==msg->Message && InputTimeOut>=0)
	{
		msg->Message=0;
		msg->Param1=0;
		if(++InputTimeOut>=InputTimeOutSec)
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_TimeOut);
		}
		else if(gMachineState!=STA_ENROLLING)
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input,News_CancelInput);
		}
		return 1;
	}
	else if(MSG_TYPE_FINGER==msg->Message)
	{
	    
	    	char *fingerbuf=NULL;
		int ptrlen;
		char *imgptr;
		
		InputTimeOut=0;
		msg->Message=0;
		LCDWriteCenterStrID(gLCDRowCount/2,HID_LEAVEFINGER);
		
				ptrlen = SensorInfo->RawImgLen;
		imgptr = SensorInfo->RawImgPtr;
#ifdef ZEM300
	#ifndef URU
	
		fingerbuf=malloc(150*1024);
		//ptrlen = 280*330;
                U16 zfwidth,zfheight;
                zfwidth = gOptions.ZF_WIDTH;
                zfheight = gOptions.ZF_HEIGHT;
                ptrlen=gOptions.ZF_WIDTH*gOptions.ZF_HEIGHT;
                ptrlen += 4;
                memcpy(fingerbuf,&zfwidth,2);
                memcpy(fingerbuf+2,&zfheight,2);
                BIOKEY_GETFINGERLINEAR(fhdl, SensorInfo->RawImgPtr, fingerbuf+4);


		imgptr = fingerbuf;	

	#endif
#endif

		//if(SendImageToRegister(SensorInfo->RawImgPtr, SensorInfo->RawImgLen, es->pin2, es->Index, &result)&&
		if(SendImageToRegister(imgptr, ptrlen, es->pin2, es->Index, &result)&&
		   ((result&0xFF)==0))
		{
			es->Index++;
			if((result>>8)<=1)
			{
				es->Enrolled=result>>8;
				ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input,News_CommitInput);
			}
			else if(es->Index==2)
				LCDWriteCenterStrID(gLCDRowCount/2,HID_PLACEFINGER2);
			else if(es->Index==3)
				LCDWriteCenterStrID(gLCDRowCount/2,HID_PLACEFINGER3);
			else if(es->Index==4)
				LCDWriteCenterStrID(gLCDRowCount/2,HID_PLACEFINGER4);
			else 
				LCDWriteCenterStrID(gLCDRowCount/2,HID_PLACEFINGER);				
		}
		else
		{
			LCDWriteCenterStrID(gLCDRowCount/2,HID_VFFAIL);
		}
                if (fingerbuf)
                        free(fingerbuf);

		if(msg->Param1==News_Exit_Input) DelayMS(100);
		return 1;
	}
	else if(MSG_TYPE_BUTTON==msg->Message)
	{
		InputTimeOut=0;
		if(IKeyESC==msg->Param1)
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input,News_CancelInput);
		else
			msg->Message=0;
		return 1;
	}
	else
		return 0;
}


int RemoteEnrollAFinger(int *enrolled, int pin)
{
	TRemoteEnrollSession es;
	char buf[MAX_CHAR_WIDTH], fbuf[MAX_CHAR_WIDTH];
	U32 mm;
	int ret,i;
	int OldState=gMachineState;
	
	gMachineState=STA_ENROLLING;
	es.Index=1;
	es.Enrolled=1;
	es.pin2=pin;
	InputTimeOut=0;
	sprintf(fbuf,"%%0%dd", gOptions.PIN2Width);
	sprintf(buf, fbuf, pin);
	FlushSensorBuffer(); //Clear sensor buffer
	LCDWriteCenterStr(gLCDRowCount/4, buf);
	LCDWriteCenterStrID(gLCDRowCount/2,HID_PLACEFINGER);
	LCDWriteCenterStrID(3,HID_ESC); 
	i=RegMsgProc(RunRemoteEnrollFinger);
	mm=SelectNewMsgMask(MSG_TYPE_BUTTON|MSG_TYPE_FINGER);
	ret=DoMsgProcess(&es, News_Exit_Input);
	SelectNewMsgMask(mm);
	UnRegMsgProc(i);
	if(ret==News_CommitInput) 
	{
		if(es.Enrolled==1)
		{
			//if(gOptions.VoiceOn) ExPlayVoice(VOICE_RETRY_FP);
			//LCDWriteCenterStrID(gLCDRowCount/2,HID_INPUTAGAIN);
		}
	}
	if(gMachineState==STA_ENROLLING) gMachineState=OldState;
	*enrolled=!es.Enrolled;
	return ret;	
}

int DoRemoteEnrollFinger(void *p)
{
#if 0 //treckle
	int pin2,ret;
	char buf[MAX_CHAR_WIDTH], *name;
	int l1, l2, w1, w2=5;
	int FPEnrolled;
	BOOL IsNewEnroll=TRUE;
	BYTE result;
	
	pin2=0;
	gLocalCorrectionImage=FALSE;	
	do
	{
		IsNewEnroll=TRUE;				
		FPEnrolled=0;
		if(pin2)
		{
			ret=LCDSelectOK(LoadStrByID(MID_DATA_EU_FINGER), LoadStrByID(HID_ENROLLNEWQ), LoadStrByID(HID_YESNO));
			if(ret==News_TimeOut) return ret;
			if(ret==News_CancelInput) IsNewEnroll=FALSE;
		}		
		//select ID for enrollment
		LCD_Clear();
		LCDWriteCenterStrID(0, MID_DATA_EU_FINGER);			
		if(IsNewEnroll)
		{
			pin2=0;
			//Acquire a idle ID for register
			if(!GetFreeIDFromAuthServer(&pin2)||!pin2)
			{
				LCDWriteCenterStrID(gLCDRowCount/2, MID_AUTHSERVER_ERROR);
				DelayMS(2*1000);
				return News_Exit_Input;			
			}		
		}		
		if(gLCDRowCount>2)
		{
			LCDWriteCenterStrID(gLCDRowCount-1, HID_OKCANCEL);		
		}
		//display format by Pin2Width
		if(gOptions.PIN2Width>5)
		{
			name=LoadStrByID(HID_PIN2);
			w2=gOptions.PIN2Width;
			sprintf(buf, "%d", pin2);
			if(w2<strlen(buf)) w2=strlen(buf);
		}
		else 
			name=LoadStrByID(HID_ENROLLNUM);
		w1=strlen(name);
		if(w1+w2>=gLCDCharWidth-1)
		{
			l1=0;
			l2=gLCDCharWidth-w2;
		}
		else
		{
			l1=(gLCDCharWidth-(w1+w2+1)+1)/2;
			l2=l1+w1+1;
		}		if(!CheckIsIdleIDFromAuthServer(pin2, &result))
		{
			LCDWriteCenterStrID(gLCDRowCount/2, MID_AUTHSERVER_ERROR);
			DelayMS(3*1000);
			return News_CancelInput;
		}	
		LCDWriteStr(gLCDRowCount/2,l1,name,0);
		//user enter a ID 
		if(gOptions.PIN2Width==5)
			ret=InputTextNumber(gLCDRowCount/2, l2, gOptions.PIN2Width, &pin2, 1, 0xFFFF, InputStyle_ANumber);
		else
			ret=InputTextNumber(gLCDRowCount/2, l2, gOptions.PIN2Width, &pin2, 1, 0x7FFFFFFF, InputStyle_ANumber);
		if(ret==News_CancelInput||ret==News_TimeOut) return ret;
		
		//register fingerprint process
		LCDClear();
		if(!CheckIsIdleIDFromAuthServer(pin2, &result))
		{
			LCDWriteCenterStrID(gLCDRowCount/2, MID_AUTHSERVER_ERROR);
			DelayMS(2*1000);
			return News_CancelInput;
		}				
		if(result)
			LCDWriteCenterStrID(0, HID_ENROLLBACKUP);
		else 
			LCDWriteCenterStrID(0, HID_ENROLLNEW);
		//fingerprint enrollment
		ret=RemoteEnrollAFinger(&FPEnrolled, pin2);
		if(ret==News_TimeOut) return ret;		
		//continue or exit
		if(FPEnrolled)
			ret=LCDSelectOK(LoadStrByID(MID_DATA_EU_FINGER), LoadStrByID(HID_ENROLLOK), LoadStrByID(HID_CONTINUECANCEL));
		else		
			ret=LCDSelectOK(LoadStrByID(MID_DATA_EU_FINGER), LoadStrByID(HID_ENROLLFAIL), LoadStrByID(HID_CONTINUECANCEL));
		if((ret==News_CancelInput)||(News_TimeOut==ret)) return ret;
	}while(1);
	
	return ret;
#endif
	return 0;//treckle
}
#endif //treckle
//--------------------------Enroll A User--------------------------
typedef struct _EnrollSession_{
	BYTE *(Tmps[3]);
	BYTE *GTmp;
	int Index;
	int len;
}TEnrollSession, *PEnrollSession;

extern int AnimateIndex;
extern int gLocalCorrectionImage;

int RunEnrollFinger(PMsg msg)
{
	BYTE qlt=0;
	PEnrollSession es=(PEnrollSession)(msg->Object);
	PSensorBufInfo SensorInfo=(PSensorBufInfo)msg->Param2;

	if(MSG_TYPE_TIMER==msg->Message && InputTimeOut>=0)
	{
		if((gOptions.MenuStyle==MenuStyle_ICON) && (gLCDHeight>=64))
		{
                        AnimateIndex=0;
		}
		msg->Message=0;
		msg->Param1=0;
		if(++InputTimeOut>=InputTimeOutSec)
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_TimeOut);
		}
		else if(gMachineState!=STA_ENROLLING)
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input,News_CancelInput);
		}
		return 1;
	}
	else if(MSG_TYPE_FINGER==msg->Message)
	{
		InputTimeOut=0;
		msg->Message=0;
		LCDWriteCenterStrID(gLCDRowCount/2,HID_LEAVEFINGER);
#ifdef URU		
		if(gOptions.VoiceOn)
		{
			if(ExPlayVoice(VOICE_REMOVE_FP)) ExBeep(1);
		}
		else
			ExBeep(1);
#endif
		if (gOptions.ZKFPVersion == ZKFPV10)
		{
			if(BIOKEY_EXTRACT_10(fhdl, SensorInfo->DewarpedImgPtr, es->Tmps[es->Index], EXTRACT_FOR_IDENTIFICATION))
			{
				DBPRINTF("FTR_%d=%X\n", es->Index, (U32)GetTmpCnt(es->Tmps[es->Index]));
				LastIndex=0;
				if(++(es->Index)>=gOptions.EnrollCount)
				{
					es->len=BIOKEY_GENTEMPLATE_10(fhdl, es->Tmps, gOptions.EnrollCount, es->GTmp);
					ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input,News_CommitInput);
				}
				else if(es->Index==1)
					LCDWriteCenterStrID(gLCDRowCount/2,HID_PLACEFINGER2);
				else if(es->Index==2)
					LCDWriteCenterStrID(gLCDRowCount/2,HID_PLACEFINGER3);
				else 
					LCDWriteCenterStrID(gLCDRowCount/2,HID_PLACEFINGER);
				qlt=100;
				//增加动态进度条 2006.08
				if((gLCDHeight>=64) &&(gOptions.MenuStyle==MenuStyle_ICON))
				{
					LCD_ClearBar(0, 0, 128, 16);
					LCD_DrawProgress(64, 3, 64, 10, gOptions.EnrollCount, es->Index, 1);                                		  LCDInvalid();
				}

			}
			else
			{
				LCDWriteCenterStrID(gLCDRowCount/2,HID_VFFAIL);
			}
		}
		else
		{
			if(BIOKEY_EXTRACT(fhdl, SensorInfo->DewarpedImgPtr, es->Tmps[es->Index], EXTRACT_FOR_IDENTIFICATION))
			{
				DBPRINTF("FTR_%d=%X\n", es->Index, (U32)GetTmpCnt(es->Tmps[es->Index]));
				LastIndex=0;
				if(++(es->Index)>=gOptions.EnrollCount)
				{
					es->len=BIOKEY_GENTEMPLATE(fhdl, es->Tmps, gOptions.EnrollCount, es->GTmp);
					ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input,News_CommitInput);
				}
				else if(es->Index==1)
					LCDWriteCenterStrID(gLCDRowCount/2,HID_PLACEFINGER2);
				else if(es->Index==2)
					LCDWriteCenterStrID(gLCDRowCount/2,HID_PLACEFINGER3);
				else 
					LCDWriteCenterStrID(gLCDRowCount/2,HID_PLACEFINGER);
				qlt=100;
				//增加动态进度条 2006.08
				if((gLCDHeight>=64) &&(gOptions.MenuStyle==MenuStyle_ICON))
				{
					LCD_ClearBar(0, 0, 128, 16);
					LCD_DrawProgress(64, 3, 64, 10, gOptions.EnrollCount, es->Index, 1);                                		  LCDInvalid();
				}

			}
			else
			{
				LCDWriteCenterStrID(gLCDRowCount/2,HID_VFFAIL);
			}
		}
		CheckSessionSend(EF_FPFTR, (void*)&qlt, 1);
		if(msg->Param1==News_Exit_Input) DelayMS(100);
		return 1;
	}
	else if(MSG_TYPE_BUTTON==msg->Message)
	{
		InputTimeOut=0;
		if(IKeyESC==msg->Param1)
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input,News_CancelInput);
		else
			msg->Message=0;
		return 1;
	}
	else
		return 0;
}





int ugroup=0;
extern PFilterRec gFilterBuf;
//Enroll a fingerprint template. store it to tmp and return the length of template.
//return value:
//	News_CommitInput	-	len>0	Enroll OK
//							len==0	Enroll fail
//	News_ErrorInput		-	the enrolled finger is repeat 
//	News_TimeOut		-LCDSelectItem
//	News_CancelInput	-
int EnrollAFinger(char *tmp, int *len, int pin, int fingerid)
{
	static TEnrollSession es;
	static BYTE tmps[3][2048];
	char buf[MAX_CHAR_WIDTH], fbuf[MAX_CHAR_WIDTH];
	U32 mm;
	int ret,i;
	int OldState=gMachineState;
	int fpcount=0;
	
	gMachineState=STA_ENROLLING;
	es.Index=0;
	es.len=0;
	es.Tmps[0]=tmps[0];es.Tmps[1]=tmps[1];es.Tmps[2]=tmps[2];
	es.GTmp=(BYTE*)tmp;
	InputTimeOut=0;
	sprintf(fbuf,"%%0%dd-%d", gOptions.PIN2Width, fingerid);
	sprintf(buf, fbuf, pin);
	FlushSensorBuffer(); //Clear sensor buffer
	LCDWriteCenterStr(gLCDRowCount/4, buf);
	LCDWriteCenterStrID(gLCDRowCount/2,HID_PLACEFINGER);

        //增加动态进度条 2006.08
        if((gLCDHeight>=64) &&(gOptions.MenuStyle==MenuStyle_ICON))
        {
                LCD_ClearBar(0, 0, 128, 16);
                LCD_DrawProgress(64, 3, 64, 10, gOptions.EnrollCount, 0, 1);
        }


	LCDWriteCenterStrID(3,HID_ESC); 
	if (gOptions.ZKFPVersion == ZKFPV10)
		BIOKEY_MATCHINGPARAM_10(fhdl, SPEED_LOW, gOptions.EThreshold);
	else
		BIOKEY_MATCHINGPARAM(fhdl, SPEED_LOW, gOptions.EThreshold);
	i=RegMsgProc(RunEnrollFinger);
	mm=SelectNewMsgMask(MSG_TYPE_BUTTON|MSG_TYPE_FINGER);
	ret=DoMsgProcess(&es, News_Exit_Input);
	SelectNewMsgMask(mm);
	UnRegMsgProc(i);
	if(ret==News_CommitInput) 
	{
		if(es.len>0) //enroll success, check if this fingerprint is in database
		{
			int result,score=55;

			//如果是分组比对的话， 应该是在所在组判断指纹是否重复
			if (gOptions.I1ToG && (gOptions.GroupFpLimit != gOptions.LimitFpCount))
			{
				GetFilterGroupInfo(ugroup,gFilterBuf);
				if (gOptions.ZKFPVersion == ZKFPV10)
					fpcount = BIOKEY_DB_FILTERID_10(fhdl, Filter_Group_Run);
				else
					fpcount = BIOKEY_DB_FILTERID(fhdl, Filter_Group_Run);
				if (fpcount >= 1)
				{
					 score=fpcount<10?gOptions.VThreshold:gOptions.MThreshold;
                                        //use low speed matching for verification
					if (gOptions.ZKFPVersion == ZKFPV10)
                                        	BIOKEY_MATCHINGPARAM_10(fhdl, IDENTIFYSPEED, score);
					else
                                        	BIOKEY_MATCHINGPARAM(fhdl, IDENTIFYSPEED, score);
				}

			}

			if (gOptions.ZKFPVersion == ZKFPV10)
			{
				if(BIOKEY_IDENTIFYTEMP_10(fhdl, (BYTE*)tmp, &result, &score))
				{
					if(gOptions.VoiceOn) ExPlayVoice(VOICE_REPEAT_FP);
					LCDWriteCenterStrID(gLCDRowCount/2,HID_REFINGER);
					ret=News_ErrorInput;
				}
			}
			else
			{
				if(BIOKEY_IDENTIFYTEMP(fhdl, (BYTE*)tmp, &result, &score))
				{
					if(gOptions.VoiceOn) ExPlayVoice(VOICE_REPEAT_FP);
					LCDWriteCenterStrID(gLCDRowCount/2,HID_REFINGER);
					ret=News_ErrorInput;
				}
			}

			if (gOptions.I1ToG && (gOptions.GroupFpLimit != gOptions.LimitFpCount))
			{
				if (gOptions.ZKFPVersion == ZKFPV10)
					BIOKEY_MATCHINGPARAM_10(fhdl, IDENTIFYSPEED, gOptions.MThreshold);				
				else
					BIOKEY_MATCHINGPARAM(fhdl, IDENTIFYSPEED, gOptions.MThreshold);				
			}

		}
		else //es.len==0 //enroll fail
		{
			if(gOptions.VoiceOn) ExPlayVoice(VOICE_RETRY_FP);
			LCDWriteCenterStrID(gLCDRowCount/2,HID_INPUTAGAIN);
		}
		*len=es.len;
	}
	else 
		*len=0;
	if (gOptions.ZKFPVersion == ZKFPV10)
		BIOKEY_MATCHINGPARAM_10(fhdl, IDENTIFYSPEED, gOptions.MThreshold);
	else
		BIOKEY_MATCHINGPARAM(fhdl, IDENTIFYSPEED, gOptions.MThreshold);
	if(gMachineState==STA_ENROLLING) gMachineState=OldState;
	return ret;	
}

extern int GroupFpCount[5];

int EnrollAPwd(char *pwd)
{
	int ret;
	char temp_pwd[12];
	int	err_cnt = 0;
	PUser	enroll_user = NULL;

	PInputBox box;
	box=(PInputBox)malloc(sizeof(TInputBox));
	memset(box,0,sizeof(TInputBox));
	box->MaxLength=PWD_LEN_MAX;
	box->Row=1;
	box->Col=3;
	box->PasswordChar='*';
	box->AutoCommit=TRUE;
	box->Width=PWD_LEN_MAX;
	ret=-1;
	memset(pwd, 0, 5);
	LCDClearLine(1);
	LCDClearLine(2);
	LCDClearLine(3);
//	L3000ShowPwdKeypad();
//	LCDWriteStrID(1,0,HID_INPUTPWD);
#if 0
		enroll_user = L3000CheckPwdFree(temp_pwd);
		if(enroll_user == &L3000MaxPwdCnt){
			L3000ShowInfo(NULL, L3000_STR_REPLACE16, 2, 0);
			ret = News_CancelInput;
			return	ret;
		}
		enroll_user = NULL;
#endif
	do
	{ 
		if(enroll_user != NULL || (box->SelectStart > 0 && box->SelectStart < PWD_LEN_MIN)){			
	//		if(++err_cnt >= 3){
				L3000ExBeep(200);
				L3000ShowError(NULL, LoadStrByID(HID_VPFAIL), 2, 0);			
				ret = News_CancelInput;
				return	ret;
	//		}
		}
		box->SelectStart = 0;
		memset(box->Text, 0, sizeof(box->Text));
		enroll_user = NULL;	
		ret=RunInput(box);
		if(ret==News_CancelInput || ret==News_TimeOut) return ret;
		box->Text[box->SelectStart]=0;
		if(box->SelectStart >= PWD_LEN_MIN){
			memset(temp_pwd, 0, sizeof(temp_pwd));
			L3000BoxToPwd(box->Text, temp_pwd);
			enroll_user = L3000SearchUserByPwd(temp_pwd);
		}
	}while(box->SelectStart==0 || box->SelectStart < PWD_LEN_MIN || enroll_user != NULL);
	memset(temp_pwd, 0, sizeof(temp_pwd));
	strcpy(temp_pwd,box->Text);	
	{
		char str[] = "*************";
		int	len = box->SelectStart;
		str[len] = '\0';
		LCDClearLine(1);
	//	LCDWriteStr(2, 3, "          ", LCD_HIGH_LIGHT);
		//str[box->SelectStart] = '\0';
		LCDWriteStr(1,3, str,  0);//LCD_HIGH_LIGHT);		
		LCD_Rectangle(24, 16, 103, 30);
		LCDClearLine(2);
	}
//	LCDWriteLineStrID(gLCDRowCount-1, HID_VERINPUTPWD);
//	LCDWriteLine(gLCDRowCount-1, "AFM:");
	
	box->Row=2;
	err_cnt = 0;

	do
	{
		if(++err_cnt > 2){
			{
		//	char str[10];
		//	sprintf(str, "%s", box->Text);
			L3000Debug(box->Text);
			}
			L3000ExBeep(200);
			L3000ShowError(NULL, LoadStrByID(HID_VPFAIL), 2, 0);			
			ret = News_CancelInput;
			return	ret;
		}
	
		box->SelectStart = 0;
		memset(box->Text, 0, sizeof(box->Text));
		//box->Text[0]=0;
		//box->SelectStart=0;
		ret=RunInput(box);
		if(ret==News_CancelInput || ret==News_TimeOut) return ret;
	}while(box->SelectStart < PWD_LEN_MIN || nstrcmp(box->Text, temp_pwd, box->SelectStart));
	memset(pwd, 0, 5);
	
	{
	//	char str[10];
	//	sprintf(str, "%s", box->Text);
		L3000Debug(box->Text);
	}

	L3000BoxToPwd(temp_pwd, pwd);
	free(box);
	return ret;
}

int EnrollAUser(int EnrollFinger, int EnrollPwd, int Privilege)
{
	int i, j, fingerid, PwdEnrolled, FPEnrolled, tmplen, ret=News_ErrorInput;
	char tmp[2048], *Title, Pwd[10];
	char buf[MAX_CHAR_WIDTH], fbuf[MAX_CHAR_WIDTH];
	char IsNewEnroll;
	TUser u, user;
	
	i=0;
	memset((void*)&u, 0, sizeof(TUser));
	memset((void*)&user, 0, sizeof(TUser));
	user.PIN=1;
	
	if((EnrollFinger && (FDB_CntTmp()>=gOptions.MaxFingerCount*100))||
	   (FDB_CntUser()>=gOptions.MaxUserCount*100))
	{
		LCDClear();
		LCDWriteCenterStrID(gLCDRowCount/2, HID_EXCEED);
		DelayMS(3*1000);			
		return News_CancelInput;
	}
	
	if(FDB_CntUser()==0||gOptions.IsOnlyRFMachine) 
	{
		IsNewEnroll=TRUE;
	}
	else
	{
		i=MID_DATA_EU_FINGER;		
		if(EnrollPwd && EnrollFinger) 
			i=MID_DATA_EU_FP;
		else if(EnrollPwd)
			i=MID_DATA_EU_PWD;
		else if(!EnrollFinger)
			i=MID_CARD_REG;
		ret=LCDSelectOK(LoadStrByID(i), LoadStrByID(HID_ENROLLNEWQ), LoadStrByID(HID_YESNO));
		if(ret==News_TimeOut) return ret;
		IsNewEnroll=(News_CommitInput==ret);
	}
	LCDWriteCenterStr(1, "");
	ugroup = gOptions.DefaultGroup;
	do
	{
	   	if (IsNewEnroll && (FDB_CntUser()>=gOptions.MaxUserCount*100))
		{
			LCD_Clear();
			LCDWriteCenterStrID(gLCDRowCount/2, HID_EXCEED);
			DelayMS(3*1000);
			return News_CancelInput;
		}

		user.PIN=GetNextPIN(user.PIN, IsNewEnroll);
		if(EnrollFinger && (FDB_CntTmp()>=gOptions.MaxFingerCount*100))
		{
			LCD_Clear();
			LCDWriteCenterStrID(gLCDRowCount/2, HID_EXCEED);
			DelayMS(3*1000);
			return News_CancelInput;
		}

		if(IsNewEnroll)
		{
			Title=LoadStrByID(HID_ENROLLNEW);
		}
		else
		{			
			if(EnrollFinger)
				Title=LoadStrByID(HID_ENROLLBACKUP);
			else if(EnrollPwd)
				Title=LoadStrByID(HID_CHGPWD);
			else
				Title=LoadStrByID(HID_CHGCARDNUM);
		}
		i=InputPINBox(Title,LoadStrByID(HID_OKCANCEL), (IsNewEnroll?INPUT_USER_NEW:0),&user);
		if(i==News_CancelInput || i==News_TimeOut)	
			return i;
//========================输人组 2006.12.28==============================
		if (gOptions.I1ToG && IsNewEnroll)
		{
			if(gLCDRowCount>2)
			{
				LCDClearLine(1);
				LCDClearLine(2);
			        LCDWriteCenterStrID(1, MID_INPUT_GROUP);
		        	LCDWriteCenterStrID(gLCDRowCount-1, HID_OKCANCEL);
			}
			else
			{
				LCDClearLine(0);
				LCDClearLine(1);
			        LCDWriteCenterStrID(0, MID_INPUT_GROUP);
				
			}	
	        	i=InputNumber(gLCDRowCount/2, 8, 1, &ugroup, 1, 5, FALSE);
			if(i==News_CancelInput || i==News_TimeOut)	
				return i;
		}
//==============================================================

		u.PIN=0;
		if(!IsNewEnroll) 
		{
			FDB_GetUser(user.PIN, &u);
			if (gOptions.I1ToG)
				ugroup = u.Group&0x0F;
//修正下级权限用户可以备份登记上级用户指纹问题
			if (AdminUser  && u.PIN)
			{
				int ret1=0;
				if ISADMIN(u.Privilege)	
				{
					if (u.Privilege > AdminUser->Privilege)	
						ret1 = 1;
					else if (u.Privilege == AdminUser->Privilege)	
						if (u.PIN != AdminUser->PIN)
							ret1 = 1;
					if (ret1)
					{
		                                ExBeep(2);
        		                        LCDWriteCenterStrID(gLCDRowCount-1, HID_ACCESSDENY);
                        		        DelayMS(3*1000);
						return News_CancelInput;

					}
				}
			}
		}

		if (gOptions.I1ToG)
		{
			if (GroupFpCount[ugroup-1] >= gOptions.GroupFpLimit)
			{
				LCDWriteCenterStrID(gLCDRowCount-1, MID_GROUPFPREACH);
				DelayMS(2000);				
				return 0;
			}	
		}
		
		fingerid=0; 
		if(EnrollFinger)
		{
			if(u.PIN!=0)
			{
				fingerid=-1;
				for(i=0;i<gOptions.MaxUserFingerCount;i++)
				{
					if(0==FDB_GetTmp((U16)u.PIN,(char)i,NULL))
					{
						fingerid=i;
						break;
					}
				}
				if(fingerid==-1)
				{
					sprintf(buf, LoadStrByID(HID_EXCEEDFINGER), gOptions.MaxUserFingerCount);
					LCDInfo(buf,5);	
					return News_CancelInput;
				}
			}
			else 
				fingerid=0;
		}
		else if(!EnrollPwd)	//既不登记指纹，也不登记密码，就是登记射频卡
		{
		//	if(!IsNewEnroll || (*(int*)user.Card==0 && user.Card[4]==0))	//检查是否输入了新的射频卡
			if(!IsNewEnroll || CARD_IS_ZERO(&user))	//检查是否输入了新的射频卡
			{
			//	while((*(int*)user.Card==0 && user.Card[4]==0)) 
				while(CARD_IS_ZERO(&user)) //treckle 
				{
					TUser u1;
					u1.PIN=user.PIN;
					u1.PIN2=user.PIN2;
					LCDWriteCenterStrID(1, HID_SHOWCARD);
					if(gOptions.VoiceOn&&gOptions.IsOnlyRFMachine) 
						ExPlayVoice(VOICE_RETRY_FP);
					else
						ExBeep(1);
					i=InputHIDCardNumber(Title,LoadStrByID(HID_OKCANCEL), 0, &u1);
					if(i==News_CancelInput || i==News_TimeOut)  return i;
					user.PIN=u1.PIN;
					user.PIN2=u1.PIN2;
					nmemcpy(user.IDCard, u1.IDCard, sizeof(user.IDCard));
				}	
			}
		}
		
		PwdEnrolled=0; FPEnrolled=0;
		if(EnrollFinger)
		{
			do
			{
				tmplen=0;
				i=user.PIN2;
				if(i==0) i=user.PIN;
				ret=EnrollAFinger(tmp, &tmplen, i, fingerid);
				if(ret==News_TimeOut) return ret;
				if(ret==News_CancelInput) break;
				if(ret==News_ErrorInput) tmplen=0;
				if(tmplen==0)
					DelayMS(500);
			}while(tmplen==0);
			FPEnrolled=tmplen>0;
		}
		
		Pwd[0]=0;
		if(EnrollPwd)
		{
			gInputNumKey = 1;
			ret=EnrollAPwd(Pwd);
			gInputNumKey = 0;
			PwdEnrolled=ret==News_CommitInput;
			if(ret==News_TimeOut) return ret;
		}
		
		i=user.PIN2;
		j=gOptions.PIN2Width;
		if(i==0)
		{
			j=PIN_WIDTH;
			i=user.PIN;
		}
		
		if(FPEnrolled && PwdEnrolled)
			sprintf(fbuf,"%%0%dd-%dP",j,fingerid);
		else if(PwdEnrolled)
			sprintf(fbuf,"%%0%dd-P",j);
		else if(FPEnrolled)
			sprintf(fbuf,"%%0%dd-%d",j,fingerid);
		//else if((*(int*)user.Card!=0 || user.Card[4]!=0))//treckle
		else if(CARD_IS_ZERO(&user))
			sprintf(fbuf,"%%0%dd-C",j);
		else
		{
			return ret;
		}
		sprintf(buf, fbuf, i);
		
		ret=LCDSelectOK(Title, buf, LoadStrByID(HID_SAVECANCEL));
		if(ret==News_TimeOut) return ret;
		//if((FPEnrolled || PwdEnrolled || ((*(int*)user.Card!=0 || user.Card[4]!=0))) && News_CommitInput==ret)
		if((FPEnrolled || PwdEnrolled || CARD_IS_ZERO(&user)) && News_CommitInput==ret)
		{
			j=FDB_OK;
			if(IsNewEnroll)
			{
				FDB_CreateUser(&u, user.PIN, NULL, Pwd, Privilege);
				if (gOptions.I1ToG)
				{
					 u.Group=(u.Group & 0xf0) | (ugroup &0x0f);
					 //printf("ugroup: %d\tu.Group: %d\n",ugroup,u.Group);
				}
				nmemcpy(u.IDCard, user.IDCard, sizeof(u.IDCard));
				u.PIN2=user.PIN2;
				if (gOptions.PIN2Width==PIN_WIDTH) u.PIN2=u.PIN;
				j=FDB_AddUser(&u);
				if(j==FDB_OK)
				{
					TUser myuser;

					memset(&myuser,0,sizeof(TUser));
					FDB_GetUser(1,&myuser);
					//CheckSessionSend(EF_ENROLLUSER, (char*)&u, sizeof(u));//treckle
				}
				FDB_AddOPLog(ADMINPIN, OP_ENROLL_USER, u.PIN,j,0,0);//treckle
			}
			else if(PwdEnrolled)
			{
				if(nstrcmp(Pwd,u.Password,5))
				{
					memset(u.Password, 0, 5);
					nstrcpy(u.Password,Pwd,5);
					j=FDB_ChgUser(&u);
					FDB_AddOPLog(ADMINPIN, OP_ENROLL_PWD, u.PIN, j, 0,0);
				}
			}
		//	else if((*(int*)user.Card!=0 || user.Card[4]!=0))//treckle
			else if(CARD_IS_ZERO(&user))
			{
				if(nmemcmp(user.IDCard,u.IDCard, sizeof(u.IDCard)))
				{ 
					nmemcpy(u.IDCard, user.IDCard, sizeof(u.IDCard));
					j=FDB_ChgUser(&u);
					FDB_AddOPLog(ADMINPIN, OP_ENROLL_RFCARD, u.PIN, j,0,0);
				}
			}						
			if(FDB_OK==j && FPEnrolled)
			{
				TTemplate t;
				j=FDB_AddTmp(FDB_CreateTemplate(&t, (U16)u.PIN, (char)fingerid, tmp, tmplen));
				if(j==FDB_OK)
				{
					char Buffer[sizeof(TTemplate)+4];
					((U16*)Buffer)[2]=0;
					memcpy(Buffer+4, &t, sizeof(t));
					CheckSessionSend(EF_ENROLLFINGER, Buffer+2, sizeof(t)+2);
				}
				FDB_AddOPLog(ADMINPIN, OP_ENROLL_FP, u.PIN,j,fingerid,tmplen);
			}
			//flush the cached data to disk
			//sync();
			if(FDB_OK!=j)
			{
				LCDWriteCenterStrID(gLCDRowCount/2, HID_FLASHERROR);
				DelayMS(5*1000);
				break;
			}
			else if(IsNewEnroll || FPEnrolled) 
				FPDBInit(); 
			ret=LCDSelectOK(Title,LoadStrByID(HID_CONTINUE), LoadStrByID(HID_YESNO));
			if(News_TimeOut==ret) return ret;
			if(gOptions.IsOnlyRFMachine&&(ret==News_CancelInput)) return ret;
			if(ret==News_CancelInput) IsNewEnroll=!IsNewEnroll;
		}
	}while(1);
	return News_CommitInput;
}

int DoEnrollFingerPwd(void *p)
{
	PMsg msg=p;
	PMenu menu=((PMenu)(msg->Object));
	PMenuItem m=&menu->Items[menu->ItemIndex];
	int EnrollFinger=FALSE, EnrollPwd=FALSE, Privilege=0;

	if(strcmp(menu->Title, LoadStrByID(MID_DATA_EADMIN))==0)
	{
		//Let user select privillege
//		char buf[MAX_CHAR_WIDTH];
		char *(items[3]);
		int ret, c=1;
	//	items[0]=LoadStrByID(HID_PRI_ENROLL);
		items[0]=LoadStrByID(HID_PRI_ADMIN);
		items[1]=LoadStrByID(HID_PRI_SUPERVISOR);
		Privilege=0;
	//	if(TESTPRIVILLEGE(PRI_OPTIONS) || (FDB_CntAdmin(PRI_OPTIONS|PRI_SUPERVISOR)==0))
	//	{
	//		Privilege=2;
	//		c=2;
	//	}
		if(TESTPRIVILLEGE(PRI_SUPERVISOR) || (FDB_CntAdmin(PRI_SUPERVISOR)==0)){
			c=2;
			Privilege  = 1;
		}
		LCDWriteCenterStr(0,"");
		LCDWriteCenterStr(gLCDRowCount/2,"");
		LCDWriteCenterStrID(gLCDRowCount/4,HID_ADMINPRIV);
		if(gLCDRowCount>2)
		{
			LCDWriteCenterStr(gLCDRowCount-1,LoadStrByID(HID_OKCANCEL));
		}
		do{ret=LCDSelectItem(gLCDRowCount/2,(gLCDCharWidth-12)/2,12,items,c,&Privilege);}while(ret==News_ErrorInput);
		if(News_CancelInput==ret || News_TimeOut==ret) return ret;
		Privilege=PRIVALUES[Privilege+1+1];
	}
	if(strcmp(m->Caption, LoadStrByID(MID_DATA_EU_FINGER))==0)
		EnrollFinger=TRUE;
	else if(strcmp(m->Caption, LoadStrByID(MID_DATA_EU_PWD)) == 0)
		EnrollPwd=TRUE;
	else if(strcmp(m->Caption, LoadStrByID(MID_DATA_EU_FP)) == 0)
	{
		EnrollPwd=TRUE; 
		EnrollFinger=TRUE;
	}
	else if(strcmp(m->Caption, LoadStrByID(MID_CARD_REG)) != 0)
	{
		return News_CancelInput;
	}

	return EnrollAUser(EnrollFinger, EnrollPwd, Privilege);
}
		
int DoEnrollUser(void *p)
{
	PMsg msg=p;
	int ret;
	PMenuItem m=&((PMenu)(msg->Object))->Items[((PMenu)(msg->Object))->ItemIndex];
	PMenu enm=NULL;

	if (strcmp(m->Caption, LoadStrByID(MID_DATA_EUSER))==0)
	{
		U32   admin_cnt = FDB_CntAdminUser();
		if(admin_cnt)
			enm=CreateMenu(LoadStrByID(MID_DATA_EUSER),gOptions.MenuStyle,NULL, NULL);
		else	
		{
			L3000ShowInfo(NULL, L3000_STR_REPLACE24, 2, 0);
		        return	News_CancelInput;	
		}
	}
	else if(strcmp(m->Caption, LoadStrByID(MID_DATA_EADMIN)) == 0)
	{
		enm=CreateMenu(LoadStrByID(MID_DATA_EADMIN),gOptions.MenuStyle,NULL,NULL);
	}
	else
	{
		return News_CancelInput;
	}

	AddMenuItem(1, enm, LoadStrByID(MID_DATA_EU_FINGER), DoEnrollFingerPwd, NULL);
	AddMenuItem(1, enm, LoadStrByID(MID_DATA_EU_PWD), DoEnrollFingerPwd, NULL);
	//AddMenuItem(enm, LoadStrByID(MID_DATA_EU_FP), DoEnrollFingerPwd, NULL);
	DebugOutput1("RFCardFunOn = %d", gOptions.RFCardFunOn);
	if(gOptions.RFCardFunOn)
		AddMenuItem(1, enm, LoadStrByID(MID_CARD_REG), DoEnrollFingerPwd, NULL);
	ret=RunMenu(enm);
	DestroyMenu(enm);
	return ret;
}

extern TUser gAdminUser;

int DoDelUser(void *p)
{
	int Canceled=FALSE, DeleteCount=0, i,ret,AdminPIN=0;
	PUser u;
	TUser user;

	LCD_Clear();
	user.PIN=GetNextPIN(0, FALSE);
	if(user.PIN==0)
		return News_ErrorInput;
	ret=InputPINBox(LoadStrByID(MID_DATA_DEL),LoadStrByID(HID_OKCANCEL), FALSE, &user);
	if(News_CancelInput==ret || News_TimeOut==ret) return ret;
	//ret==News_CommitInput
	u=FDB_GetUser(user.PIN, NULL);
	if(u)
	{
		char buf[MAX_CHAR_WIDTH], fbuf[MAX_CHAR_WIDTH];
		U32 pin;
		int left;
		if(AdminUser)
		{
			if(u->Privilege >= AdminUser->Privilege)
			{
				ExBeep(2);
				LCDWriteCenterStrID(gLCDRowCount-1, HID_ACCESSDENY);
				DelayMS(3*1000);
				return News_CancelInput;
			}
			AdminPIN=AdminUser->PIN;
		}
		pin=u->PIN2;
		i=gOptions.PIN2Width;
		if(pin==0)
		{
			i=PIN_WIDTH;
			pin=u->PIN;
		}
		left=(gLCDCharWidth-i-2)/2+i;	//start position of pin
		sprintf(fbuf, "%%%ds%%0%dd", left-i, i);
		sprintf(buf, fbuf, "", pin);
		for(i=gOptions.MaxUserFingerCount-1;i>=0;i--)
		{
			if(FDB_GetTmp((U16)u->PIN,(char)i,NULL))
			{
				sprintf(buf+left,"-%d", i);
				ret=LCDSelectOK(LoadStrByID(HID_DEL_FP),buf,LoadStrByID(HID_OKCANCEL));
				if(ret==News_TimeOut) return ret;
				if(News_CommitInput==ret)
				{
					int j=FDB_DelTmp((U16)u->PIN,(char)i);
					if(j==FDB_OK) DeleteCount++;
					FDB_AddOPLog(ADMINPIN, OP_DEL_FP, u->PIN,j,i,0);
				}
				else if(ret==News_TimeOut) 
					return ret;
				else
				{
					Canceled=TRUE; 
				}
			}
		}
		if(u->Password[0])
		{
			sprintf(buf+left,"-%s", "P");
			ret=LCDSelectOK(LoadStrByID(HID_DEL_PWD),buf,LoadStrByID(HID_OKCANCEL));
			if(ret==News_TimeOut) return ret;
			if(News_CommitInput==ret)
			{
				TUser user;
				int j;
				user=*u;
				nmemset((BYTE*)user.Password, 0, sizeof(u->Password));
				j=FDB_ChgUser(&user);
				FDB_AddOPLog(ADMINPIN, OP_DEL_PWD, u->PIN,j,0,0);
			}
			else
				Canceled=TRUE;
		}
	//	if(!((*(int*)(u->Card)==0) && (u->Card[4]==0)))
		if(!((u->IDCard[0]==0) && (u->IDCard[2]==0))) //treckle
		{
			sprintf(buf+left,"-%s", "C");
			ret=LCDSelectOK(LoadStrByID(MID_CARD_UNREG),buf,LoadStrByID(HID_OKCANCEL));
			if(ret==News_TimeOut) return ret;
			if(News_CommitInput==ret)
			{
				TUser user;
				int j;
				user=*u;
				nmemset(user.IDCard, 0, sizeof(u->IDCard));
				j=FDB_ChgUser(&user);
				FDB_AddOPLog(ADMINPIN, OP_DEL_RFCARD, u->PIN,j,0,0);
			}
			else
				Canceled=TRUE;
		}
		if(!Canceled)
		{
			sprintf(buf+left,"%s", "");
			ret=LCDSelectOK(LoadStrByID(HID_DEL_USR),buf,LoadStrByID(HID_OKCANCEL));
			if(News_CommitInput==ret)
			{
				ret=LCDSelectOK(LoadStrByID(HID_DEL_USR),LoadStrByID(HID_VERDELETE),LoadStrByID(HID_OKCANCEL));
				if(News_CommitInput==ret)
				{
					int pin=u->PIN;
					int j=FDB_DelUser(pin);
					if(FDB_OK==j) 
					{
						DeleteCount++;
						LCDWriteCenterStrID(gLCDRowCount/2,HID_NOTENROLLED);
						LCDClearLine(3);
					}
					FDB_AddOPLog(ADMINPIN, OP_DEL_USER, pin, j, i,0);
				}
				else if(ret==News_TimeOut) return ret;
			}
			else if(ret==News_TimeOut) return ret;
		}
		if(DeleteCount>0)
		{
			FPDBInit();
			if(AdminUser)
			{
				AdminUser=FDB_GetUser((U16)AdminPIN, &gAdminUser);
			}
		}
		//flush the cached data to disk
		//sync();
	}
	return ret;
}

int DoClearLog(void *p)
{
	int ret=LCDSelectOK(LoadStrByID(MID_DATA_DELLOG),LoadStrByID(HID_VERDELETE),LoadStrByID(HID_OKCANCEL));
	if(News_CommitInput==ret)
	{
		int j;
		LCDInfo(LoadStrByID(HID_WAITING),0);
		j=FDB_ClrAttLog();
		FDB_AddOPLog(ADMINPIN, OP_CLEAR_LOG, 0, j, 0, 0);
	}
	return ret;
}

int DoViewAttLog(void *p)
{
        int ret;
        TUser user;
        LCDClear();
        user.PIN=0;
        ret=InputPINBox(LoadStrByID(MID_DATA_VATTLOG), LoadStrByID(HID_OKCANCEL), 0, (void*)&user);
        if(ret!=News_CommitInput)
        {
                if(ret!=News_ErrorInput)
                        return ret;
                else
                        user.PIN=0;
        }
        return ViewAttLogByUser(user.PIN, InputTimeOutSec);

}

int DoViewAdminLog(void *p)
{
	return News_CancelInput;
}

#define INTIMEP(row,col,minv,maxv,e) {\
 newm.e=gCurTime->e;\
       do{	\
		ret=InputNumber(row,col,2,&newm.e,minv,maxv);\
		    if(ret==News_CancelInput || ret==News_TimeOut) return;\
	    }while(ret==News_ErrorInput);\
sprintf(buf, "%02d", newm.e);\
LCDWriteStr(row,col,buf,0);\
}

int DoSetTime(void *p)
{
	TTime newm;
	int ret,ii,row,col,RightToLeft;
	char buf[MAX_CHAR_WIDTH];
	
	LCD_Clear();
	if (gLCDRowCount>2)
	{
		LCDWriteCenterStr(0, "YYYY-MM-DD 24H");
		LCDWriteCenterStrID(3, HID_OKCANCEL);
	}		
	row=gLCDRowCount/4;
	col=(gLCDCharWidth-10)/2;	
	sprintf(buf,"%4d-%2d-%2d",gCurTime.tm_year+1900, gCurTime.tm_mon+1,gCurTime.tm_mday);
	LCDWriteStr(row,col,buf,0);
	sprintf(buf,"%2d:%2d:%2d",gCurTime.tm_hour, gCurTime.tm_min,gCurTime.tm_sec);
	LCDWriteStr(row+1,col+1,buf,0);
	
        RightToLeft=gLangDriver->RightToLeft;
        gLangDriver->RightToLeft=FALSE;
	memcpy(&newm, &gCurTime, sizeof(struct tm));
	newm.tm_year=gCurTime.tm_year+1900; 
	newm.tm_mon=gCurTime.tm_mon+1;
	ii=0;
	ret=News_ErrorInput;
	do
	{
		switch(ii)
		{
		case 0:
			ret=InputNumberAt(row,col,4,&newm.tm_year,2000,2037); break;
		case 1:
			ret=InputNumberAt(row,col+5,2,&newm.tm_mon,1,12); break;
		case 2:
			ret=InputNumberAt(row,col+5+3,2,&newm.tm_mday,1,GetMonDay(newm.tm_year, newm.tm_mon)); break;
		case 3:
			ret=InputNumberAt(row+1,col+1,2,&newm.tm_hour,0,23); break;
		case 4:
			ret=InputNumberAt(row+1,col+1+3,2,&newm.tm_min,0,59); break;
		case 5:
			ret=InputNumberAt(row+1,col+1+3+3,2,&newm.tm_sec,0,59); break;
		}
		if(ret==News_ErrorInput) continue;
		//if(News_CancelInput==ret || News_TimeOut==ret) break;
		if(News_TimeOut==ret) break;

		if(ret == News_CancelInput)
		{
			ii -= 1;
		}

		if(ret == News_CommitInput)
		{
			ii += 1;
		}

		/*
		if(ret==News_NextInput)
		{
			if(++ii>5) ii=0;
		}
		else if(ret==News_PrevInput)
		{
			if(--ii<0) ii=5;
		}
		*/
	}while(ii >= 0 && ii <= 5);

	if(ret==News_CommitInput)
	{
		FDB_AddOPLog(ADMINPIN, OP_SET_TIME, newm.tm_year, newm.tm_mon, newm.tm_mday, newm.tm_hour*newm.tm_min);
		newm.tm_year-=1900;
		newm.tm_mon-=1;
		SetTime(&newm);
	}
        gLangDriver->RightToLeft=RightToLeft;
	return ret;
}

int NewLng=0;

int DoSetLng(void *p)
{
	char *cp, Items[MAX_CHAR_WIDTH*10];
	int Lngs[50];
	PMenu menu=((PMenu)((PMsg)p)->Object);
	int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret;
	int col=gLCDCharWidth-6, index=gOptions.Language;
	int i, LngCnt=GetSupportedLangByYaffs(Lngs, 50);
        index=Lngs[0];
        cp=Items;
        for(i=0;i<LngCnt;i++)
        {
                char *ln=GetLangName(Lngs[i]);
                sprintf(cp,"%s:",ln);
                cp+=strlen(ln)+1;
                if(Lngs[i]==gOptions.Language) index=gOptions.Language;
        }
        cp-=1;*cp=0;
	ret=LCDSelectItemValue(row, col, 6, Items, Lngs, &index);
	if(ret==News_CommitInput)
	{
		if(index!=gOptions.Language)
		{
			NewLng=index;
			MenuFmtStr(menu->Items[menu->ItemIndex].Caption,MID_OS_LANGUAGE,GetLangName(NewLng));

			gOptions.Saved=FALSE;
			ShowMenu((PMenu)((PMsg)p)->Object);
	       }
	}
	return ret;
}

int DoSetLock(void *p)
{
	return InputValueOfItem((PMsg)p, 3, 0, 500, &gOptions.LockOn);
}


int DoSetLockUsers(void *p)
{
	return InputValueOfItem((PMsg)p, 1, 1, 5, &gOptions.UnlockPerson);
}

int DoSetDevNumber(void *p)
{
	return InputValueOfItem((PMsg)p, 3, 1, 255, &gOptions.DeviceID);
}

char *ShowTimeValue0(char *buf, int v)
{
	char vbuf[10];
	v=v & 0xFFFF;
	if(((v & 0xFF)>59) || ((v>>8)>23))
	{
		sprintf(vbuf,"%5s", LoadStrByID(HID_NO));
	}
	else
	{
		sprintf(vbuf,"%02d:%02d", v>>8, v & 0xFF);
	}
	return MenuFmtStrStr(buf, 5, vbuf);	
}

char *ShowTimeValueIndex(char *buf, int id, int v)
{
	char format[MAX_CHAR_WIDTH], b[MAX_CHAR_WIDTH];
	sprintf(format, "%%-%ds", gLCDCharWidth);
	sprintf(b, LoadStrByID(HID_NUM), id);
	sprintf(buf, format, b);
	return ShowTimeValue0(buf, v);
}

char *ShowTimeValue(char *buf, int id, int v)
{
/*	char format[MAX_CHAR_WIDTH], b[MAX_CHAR_WIDTH];
	sprintf(format, "%%-%ds", gLCDCharWidth);
	sprintf(b, "%s", LoadStrByID(id));
	sprintf(buf, format, b);
	return ShowTimeValue0(buf, v);
*/
        MenuFmtStr(buf, id, "");
        return ShowTimeValue0(buf, v);

}

int InputTime(int row, int col, int *vhour, int *vsecond)
{
	int ii=0, ret,RightToLeft;
        RightToLeft=gLangDriver->RightToLeft;
        gLangDriver->RightToLeft=FALSE;
        if(RightToLeft)
                col=gLCDCharWidth-col-5;

	
	ret=News_ErrorInput;
	do
	{
		char buf[MAX_CHAR_WIDTH];
		sprintf(buf, "%2d:%2d",*vhour,*vsecond);
		LCDWriteStr(row, col, buf, 0);
		switch(ii)
		{
		case 0:
			ret=InputNumberAt(row,col,2,vhour,0,23); break;
		case 1:
			ret=InputNumberAt(row,col+3,2,vsecond,0,59); break;
		}
		if(ret==News_ErrorInput) continue;
		if(News_CancelInput==ret || News_TimeOut==ret)  break;
		if(ret==News_NextInput)
		{
			if(++ii>1) ii=0;
		}
		else if(ret==News_PrevInput)
		{
			if(--ii<0) ii=1;
		}
	}while(ret!=News_CommitInput);
        gLangDriver->RightToLeft=RightToLeft;
	return ret;
}

static int *TimeValues=NULL;

int DoSetAutoPower(void *p)
{
	PMenu menu=((PMenu)((PMsg)p)->Object);
	int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret, ii;
	int col=gLCDCharWidth-5, vhour, vsecond, *v;
	if(TimeValues)
	{
		v=TimeValues+menu->ItemIndex;
	}
	else
	{
		v=&gOptions.AutoPowerOff;
		//if(menu->ItemIndex==1) v=&gOptions.AutoPowerOn;
		//else if(menu->ItemIndex==2) v=&gOptions.AutoPowerSuspend;
		if(menu->ItemIndex==1) v=&gOptions.AutoPowerSuspend;		
	}
	*v&=0xffff;
	vhour=(*v)>>8; if(vhour>=24) vhour=23;
	vsecond=(*v)&0xFF; if(vsecond>=60) vsecond=59;
	//设置定时功能吗？
	ii=MID_OSA_POWEROFF;
	if(v==&gOptions.AutoPowerOn) ii=MID_OSA_POWERON;
	else if(v==&gOptions.AutoPowerSuspend) ii=MID_OSA_SUSPEND;
	else if(TimeValues==gOptions.AutoAlarm) ii=MID_OSA_ALARM;
	else if(TimeValues==gOptions.AutoState) ii=MID_AUTO_STATE;
	ret=LCDSelectOK(LoadStrByID(ii), LoadStrByID(HID_OTAPOWER), LoadStrByID(HID_YESNO));
	if(News_TimeOut==ret) return ret;
	if(News_CommitInput==ret)
	{
		ShowMenu(menu);
		//重绘制菜单项避免反显
		LCDWriteStr(row, MenuIndicatorWidth, menu->Items[menu->ItemIndex].Caption, 0);		
		ret=InputTime(row, col, &vhour, &vsecond);
		if(News_CancelInput==ret || News_TimeOut==ret) return ret;
		ii=(vhour<<8) | vsecond;
	}
	else
	{
		ii=(24<<8) | 60;
		ret=News_CommitInput;
	}
	if(ii!=*v)
	{
		*v=ii;
		if(ret==News_CommitInput)
		{
			ShowTimeValue0(menu->Items[menu->ItemIndex].Caption, *v);
			gOptions.Saved=FALSE;
		}
	}
	return ret;
}

int DoSetIdle(void *p)
{
	char *(Items[2]), format[MAX_CHAR_WIDTH];
	int idles[2];
	PMenu menu=((PMenu)((PMsg)p)->Object);
	int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret;
	int col=gLCDCharWidth-5, index; 
	idles[0]=HID_POWEROFF;idles[1]=HID_SUSPEND;
	Items[0]=LoadStrByID(idles[0]);
	Items[1]=LoadStrByID(idles[1]); 
	index=0;
	if(gOptions.IdlePower==idles[1]) index=1;
	do
	{
		ret=LCDSelectItem(row,col,5,Items,2,&index);
		if(News_CancelInput==ret || News_TimeOut==ret) return ret;
	}while(ret==News_ErrorInput);
	// News_CommitInput
	if(idles[index]!=gOptions.IdlePower)
	{
	sprintf(format, "%%-%ds%%5s", gLCDCharWidth-5-MenuIndicatorWidth);
	sprintf(menu->Items[menu->ItemIndex].Caption,format,LoadStrByID(MID_OSA_IDLE),LoadStrByID(idles[index]));
	gOptions.IdlePower=idles[index];
	gOptions.Saved=FALSE;
	}
	return ret;
}


int DoSetDateFormat(void *p)
{
	PMenu menu=((PMenu)((PMsg)p)->Object);
	int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret;
	int col=gLCDCharWidth-8, index=gOptions.DateFormat; 
	do
	{
		ret=LCDSelectItem(row,col,8,DateFormats,10,&index);
		if(News_CancelInput==ret || News_TimeOut==ret) return ret;
	}while(ret==News_ErrorInput);
	// News_CommitInput
	if(index!=gOptions.DateFormat)
	{
		gOptions.DateFormat=index;
		MenuFmtStr(menu->Items[menu->ItemIndex].Caption,322,DateFormats[gOptions.DateFormat%10]);
		gOptions.Saved=FALSE;
	}
	return ret;
}


int DoSetIdleMinute(void *p)
{
	return InputValueOfItem((PMsg)p, 3, 0, 999, &gOptions.IdleMinute);	
}

int DoSetAutoState(void *p)
{
	int ret, i=0, v[6]={0,1,2,3,4,5}, index=0;
	char cbuf[8][MAX_CHAR_WIDTH], buf[MAX_CHAR_WIDTH*2], Buffer[MAX_CHAR_WIDTH];
	static PMenu m21=NULL;
	
	LCD_Clear();
	sprintf(buf,"%s:%s:%s:%s", LoadStrByID(HID_SCIN), LoadStrByID(HID_SCOUT), LoadStrByID(HID_SOCIN), LoadStrByID(HID_SOCOUT));
	
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
	if(ret!=News_CommitInput) return ret;
	
	SCopyStrFrom(Buffer, buf, i);
	index=i;
	sprintf(cbuf[0], LoadStrByID(HID_AUTO_STATE), Buffer);
	m21=CreateMenu(cbuf[0],gOptions.MenuStyle,NULL, m21);
	TimeValues=gOptions.AutoState+4*index;
	for(i=0;i<4;i++)
	{
		ShowTimeValueIndex(cbuf[i+1], i+1, gOptions.AutoState[i+index*4]);
		AddMenuItem(1,m21, cbuf[i+1], DoSetAutoPower, NULL);
	}
	ret=RunMenu(m21);
	DestroyMenu(m21);
	if(!gOptions.Saved && ret!=News_TimeOut)
	{
		ret=LCDSelectOK(cbuf[0],LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret)
		{
			SaveOptions(&gOptions);
		}
		else
			LoadOptions(&gOptions);
	}
	gOptions.Saved=TRUE;
	TimeValues=NULL;
	return ret;	
}

int DoSetAutoAlarmDelay(void *p)
{
	return InputValueOfItem((PMsg)p, 3, 0, 999, &gOptions.AutoAlarmDelay);	
}

int DoSetAutoAlarm(void *p)
{
	int ret, i=0;
	char cbuf[20][MAX_CHAR_WIDTH];
	static PMenu m21=NULL;
	LCD_Clear();
	m21=CreateMenu(LoadStrByID(MID_OSA_ALARM),gOptions.MenuStyle,NULL, m21);
	for(i=0;i<MAX_AUTOALARM_CNT;i++)
	{
		ShowTimeValueIndex(cbuf[i+1], i+1, gOptions.AutoAlarm[i]);
		AddMenuItem(1, m21, cbuf[i+1], DoSetAutoPower, NULL);
	}
	TimeValues=gOptions.AutoAlarm;
	ret=RunMenu(m21);
	DestroyMenu(m21);
	if(!gOptions.Saved && ret!=News_TimeOut)
	{
		ret=LCDSelectOK(LoadStrByID(MID_OSA_ALARM),LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret) 
		{
			SaveOptions(&gOptions);
		}
		else
			LoadOptions(&gOptions);
	}
	gOptions.Saved=TRUE;
	TimeValues=NULL;
	return ret;	
}

int DoSetWEBSERVERIP(void *p)
{
#if 0 //treckle
	int ii,ret,ipa[4];
	ipa[0]=gOptions.WebServerIP[0];
	ipa[1]=gOptions.WebServerIP[1];
	ipa[2]=gOptions.WebServerIP[2];
	ipa[3]=gOptions.WebServerIP[3];
	ret=InputIPAddress(LoadStrByID(MID_OSA_WEBSERVERIP), ipa);
	for(ii=0;ii<4;ii++)
		if(ipa[ii]!=gOptions.WebServerIP[ii])
		{
		      if(gOptions.Saved) gOptions.Saved=FALSE;
		      gOptions.WebServerIP[ii]=ipa[ii];
	       }
	return ret;
#endif
	return 0;
}

int DoSetLockPower(void *p)
{
	return InputYesNoItem((PMsg)p, &gOptions.LockPowerButton);
}

int DoShutDown(void *p)
{
	LCDClear();
	ExPowerOff(FALSE);
	return 0;
}
/*
int DoSetAutoAlarmAudioVol(void *p)
{
	return InputValueOfItem((PMsg)p, 2, 0, 99, &gOptions.AutoAlarmAudioVol);
}
*/
char autobuf[3][32];
int DoAutoPower(void *p)
{
	//电源管理
	char cbuf[10][MAX_CHAR_WIDTH];
	int ret;
	static PMenu m21=NULL;
	m21=CreateMenu(LoadStrByID(MID_OS_AUTOPOWER),gOptions.MenuStyle,NULL, m21);
	if (!LoadInteger(NOPOWERKEY,0))
	{
		ShowTimeValue(cbuf[0],MID_OSA_POWEROFF,gOptions.AutoPowerOff);
		AddMenuItem(1,m21, cbuf[0], DoSetAutoPower, NULL);
	}
	//ShowTimeValue(cbuf[1],MID_OSA_POWERON,gOptions.AutoPowerOn);
	//AddMenuItem(m21, cbuf[1], DoSetAutoPower, NULL);

	//adv access not for suspend funtion 2006.10.09
	if(!(gOptions.LockFunOn&LOCKFUN_ADV))
	{
		
		ShowTimeValue(cbuf[2],MID_OSA_SUSPEND,gOptions.AutoPowerSuspend);
		AddMenuItem(1,m21, cbuf[2], DoSetAutoPower, NULL);
	
		AddMenuItem(1, m21, MenuFmtStr(cbuf[3],MID_OSA_IDLE,LoadStrByID(gOptions.IdlePower)), DoSetIdle, NULL);
		AddMenuItem(1, m21, MenuFmtInt(cbuf[4],MID_OSA_IDLETIME,gOptions.IdleMinute), DoSetIdleMinute, NULL);
	}
	if(gOptions.AutoAlarmFunOn)
	{
		AddMenuItem(1, m21, MenuFmtInt(cbuf[6], MID_AUTOBELL_DELAY, gOptions.AutoAlarmDelay), DoSetAutoAlarmDelay, NULL);		
		AddMenuItem(1, m21, LoadStrByID(MID_OSA_ALARM), DoSetAutoAlarm, NULL);

		//span custom A11
		 if (gOptions.AutoOpenRelayFunOn)
                {
                        AddMenuItem(1, m21, MenuFmtStr(autobuf[0],MID_AUTO_FPOPEN,GetYesNoName(gOptions.FPOpenRelay)), DoSetFpOpenOff, NULL);
                        AddMenuItem(1, m21, MenuFmtStr(autobuf[1],MID_AUTO_OPENDOOR,GetYesNoName(gOptions.AutoOpenRelay)), DoSetAutoOpenOff, NULL);
                        AddMenuItem(1, m21, MenuFmtInt(autobuf[2],MID_AUTO_TIMES,gOptions.AutoOpenRelayTimes), DoSetAutoOpenTimes, NULL);

                }

	}
/*
	//web server ip
	if(gOptions.IsSupportMP3)
	{
		AddMenuItem(1, m21, LoadStrByID(MID_OSA_WEBSERVERIP), DoSetWEBSERVERIP, NULL);
		AddMenuItem(0, m21, MenuFmtInt(cbuf[7], MID_ADV_AUDIOVOL, gOptions.AutoAlarmAudioVol), DoSetAutoAlarmAudioVol, NULL);
	}
*/	//treckle
	if(gOptions.AutoStateFunOn)
		if(gOptions.ShowState || gOptions.ShowCheckIn)
			AddMenuItem(1, m21, LoadStrByID(MID_AUTO_STATE), DoSetAutoState, NULL);	
	//关机按钮
	AddMenuItem(1, m21, MenuFmtStr(cbuf[5],MID_LOCK_POWER,GetYesNoName(gOptions.LockPowerButton)), DoSetLockPower, NULL);
	if (gOptions.LockPowerButton)
		AddMenuItem(1, m21, LoadStrByID(MID_POWER_OFF), DoShutDown, NULL);		
	gOptions.Saved=TRUE;
	ret=RunMenu(m21);
	DestroyMenu(m21);
	if(!gOptions.Saved && ret!=News_TimeOut)
	{
		ret=LCDSelectOK(LoadStrByID(MID_OS_AUTOPOWER),LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret) 
		{
			ExSetPowerSleepTime(gOptions.IdleMinute); //setup idle time again
			SaveOptions(&gOptions);
		}
		else
			LoadOptions(&gOptions);
	}
	return ret;
}

int DoSetOnlyPINCard(void *p)
{
	return InputYesNoItem((PMsg)p, &gOptions.OnlyPINCard);
}

int DoSetMustEnroll(void *p)
{
	return InputYesNoItem((PMsg)p, &gOptions.MustEnroll);
}

int DoSetTIMESERVERIP(void *p)
{
	int ii,ret,ipa[4];
	ipa[0]=gOptions.TimeServerIP[0];
	ipa[1]=gOptions.TimeServerIP[1];
	ipa[2]=gOptions.TimeServerIP[2];
	ipa[3]=gOptions.TimeServerIP[3];
	ret=InputIPAddress(LoadStrByID(MID_TIME_SERVER), ipa);
	for(ii=0;ii<4;ii++)
		if(ipa[ii]!=gOptions.TimeServerIP[ii])
		{
		      if(gOptions.Saved) gOptions.Saved=FALSE;
		      gOptions.TimeServerIP[ii]=ipa[ii];
	       }
	return ret;
}

int DoSetAutoSyncTime(void *p)
{
#if 0 //treckle
	PMenu menu=((PMenu)((PMsg)p)->Object);
	int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret, ii;
	int col=gLCDCharWidth-5, vhour, vsecond, *v;
	
	v=&gOptions.AutoSyncTime;
	*v&=0xffff;
	vhour=(*v)>>8; if(vhour>=24) vhour=23;
	vsecond=(*v)&0xFF; if(vsecond>=60) vsecond=59;
	ret=LCDSelectOK(LoadStrByID(MID_TIME_SYNC), LoadStrByID(HID_TIME_SYNCHINT), LoadStrByID(HID_YESNO));
	if(News_TimeOut==ret) return ret;
	if(News_CommitInput==ret)
	{
		ShowMenu(menu);
		//重绘制菜单项避免反显
		LCDWriteStr(row, MenuIndicatorWidth, menu->Items[menu->ItemIndex].Caption, 0);		
		ret=InputTime(row, col, &vhour, &vsecond);
		if(News_CancelInput==ret || News_TimeOut==ret) return ret;
		ii=(vhour<<8) | vsecond;
	}
	else
	{
		ii=(24<<8) | 60;
		ret=News_CommitInput;
	}
	if(ii!=*v)
	{
		*v=ii;
		if(ret==News_CommitInput)
		{
			ShowTimeValue0(menu->Items[menu->ItemIndex].Caption, *v);
			gOptions.Saved=FALSE;
		}
	}
	return ret;
#endif
	return 0;
}

int DoSetTimeMenu(void *p)
{
	//记录设置
	char cbuf[3][MAX_CHAR_WIDTH];
	int ret;
	static PMenu m22=NULL;
	m22=CreateMenu(LoadStrByID(MID_OS_TIME), gOptions.MenuStyle, NULL, m22);
	AddMenuItem(1, m22, LoadStrByID(MID_TIME_SET), DoSetTime, NULL);
	ShowTimeValue(cbuf[0], MID_TIME_SYNC, gOptions.AutoSyncTime);
	AddMenuItem(1, m22, cbuf[0], DoSetAutoSyncTime, NULL);	
	AddMenuItem(1, m22, LoadStrByID(MID_TIME_SERVER), DoSetTIMESERVERIP, NULL);	
	gOptions.Saved=TRUE;
	ret=RunMenu(m22);
	DestroyMenu(m22);
	if(!gOptions.Saved && ret!=News_TimeOut)
	{
		ret=LCDSelectOK(LoadStrByID(MID_OS_TIME),LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret) SaveOptions(&gOptions);
	}
	return ret;	
}

int DoUSBDiskMng(void *p)
{
	int	ret = 0;
	static PMenu m3=NULL;	

	L3000ShowQuestion(LoadStrByID(MID_PENDRV_MNG), LoadStrByID(HID_YESNO), 0, 10);
	ret = InputLine(0,0,0,NULL);
	return 0;
}

int DoOptionSystem(void *p)
{
	//系统设置
	char cbuf[10][MAX_CHAR_WIDTH];
	int format[20];
	int ret;

	static PMenu m21;
	m21=CreateMenu(LoadStrByID(MID_OPTIONS_SYSTEM),gOptions.MenuStyle,NULL, m21);
	if(gOptions.AutoSyncTimeFunOn)
		AddMenuItem(1, m21, LoadStrByID(MID_OS_TIME), DoSetTimeMenu, NULL);
	else
		AddMenuItem(1, m21, LoadStrByID(MID_OS_TIME), DoSetTime, NULL);

	if(gOptions.MultiLanguage&&(GetSupportedLangByYaffs((int*)format, 2)>1))
	{
		AddMenuItem(1, m21, MenuFmtStr(cbuf[3], MID_OS_LANGUAGE, GetLangName(gOptions.Language)), DoSetLng, NULL);
	}

	AddInputOptMenu(m21,&gOptions.DeviceID);
	if(gOptions.LockFunOn&LOCKFUN_BASE)
	{
		AddMenuItem(1, m21, MenuFmtInt(cbuf[4],MID_OS_LOCK,gOptions.LockOn), DoSetLock, NULL);
		AddMenuItem(1, m21, MenuFmtInt(cbuf[5],MID_OS_LOCKUSERS,gOptions.UnlockPerson), DoSetLockUsers, NULL);
	}	
	//AddMenuItem(1, m21, MenuFmtStr(cbuf[8],322,DateFormats[gOptions.DateFormat%10]), DoSetDateFormat, NULL);
	
	if(TESTPRIVILLEGE(PRI_SUPERVISOR))
	{
		AddMenuItem(1, m21, LoadStrByID(MID_OS_ADVANCE), DoAdvanceMenu, NULL);
	//	if(GetCustValueCount()>0)
	//		AddMenuItem(1, m21, LoadStrByID(MID_OS_CUST), DoSetCustValues, NULL);
	}
	
	gOptions.Saved=TRUE;
	NewLng=gOptions.Language;
	ret=RunMenu(m21);
	DestroyMenu(m21);
	if(!gOptions.Saved && ret!=News_TimeOut)
	{
		ret=LCDSelectOK(LoadStrByID(MID_OPTIONS_SYSTEM),LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret) 
		{
			SaveOptions(&gOptions); 
			if(NewLng!=gOptions.Language)
			{
				SaveInteger("NewLng", NewLng);
				LCDInfoShow(LoadStrByID(MID_OS_LANGUAGE),LoadStrByID(HID_RESTART));
				DelayMS(2*1000);
			}
		}
		else 
			LoadOptions(&gOptions);
	}
	return ret;
}

int DoSetAlarmOpLog(void *p)
{
	return InputValueOfItem((PMsg)p, 2, 0, 99, &gOptions.AlarmOpLog);
}
int DoSetAlarmAttLog(void *p)
{
	return InputValueOfItem((PMsg)p, 2, 0, 99, &gOptions.AlarmAttLog);
}
int DoSetAlarmReRec(void *p)
{
	return InputValueOfItem((PMsg)p, 2, 0, 99, &gOptions.AlarmReRec);
}

int DoOptionRec(void *p)
{
	//记录设置
	char cbuf[3][MAX_CHAR_WIDTH];
	int ret;
	static PMenu m22=NULL;
	m22=CreateMenu(LoadStrByID(MID_OPTIONS_REC),gOptions.MenuStyle,NULL, m22);
	AddMenuItem(1, m22, MenuFmtInt(cbuf[0],MID_OR_AADMINLOG,gOptions.AlarmOpLog), DoSetAlarmOpLog, NULL);
	AddMenuItem(1, m22, MenuFmtInt(cbuf[1],MID_OR_AATTLOG,gOptions.AlarmAttLog), DoSetAlarmAttLog, NULL);
	AddMenuItem(1, m22, MenuFmtInt(cbuf[2],MID_OR_REREC,gOptions.AlarmReRec), DoSetAlarmReRec, NULL);
	gOptions.Saved=TRUE;
	ret=RunMenu(m22);
	DestroyMenu(m22);
	if(!gOptions.Saved && ret!=News_TimeOut)
	{
		ret=LCDSelectOK(LoadStrByID(MID_OPTIONS_REC),LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret) 
			SaveOptions(&gOptions);
		else 
			LoadOptions(&gOptions);
	}
	return ret;	
}

int DoSetBaudRate(void *p)
{
	char *Items="9600:19200:38400:57600:115200";
	int Values[]={9600,19200,38400,57600,115200};
	PMenu menu=((PMenu)((PMsg)p)->Object);
	int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret;
	int col=gLCDCharWidth-6, index; 
	index=gOptions.RS232BaudRate;
	ret=LCDSelectItemValue(row,col,6,Items,Values,&index);
	if(ret==News_CommitInput)
	{
		if(index!=gOptions.RS232BaudRate)
		{
			sprintf(menu->Items[menu->ItemIndex].Caption+gLCDCharWidth-6-MenuIndicatorWidth,"%6d",index);
			gOptions.RS232BaudRate=index;
			gOptions.Saved=FALSE;
		}
	}
	return ret;
}

int InputIPAddress(char *Title, int *ipa)
{
#if 0
	int ii,ret,RightToLeft;
	char buf[32];
	LCD_Clear();
	LCDWriteStr(0,0, Title, 0);
        RightToLeft=gLangDriver->RightToLeft;
        gLangDriver->RightToLeft=FALSE;

	sprintf(buf,"%3d.%3d.%3d.%3d ",ipa[0],ipa[1],ipa[2],ipa[3]);
	LCDWriteStr(gLCDRowCount/2,0,buf,0);
	LCDWriteCenterStrID(3, HID_OKCANCEL);
	ii=0;
	do
	{
		//if(ii==0) ret=1; else ret=0;
		ret=0;
		ret=InputNumberAt(gLCDRowCount/2,ii*4,3,ipa+ii,ret,255);
		if(ret==News_ErrorInput) continue;
		if(News_CancelInput==ret || News_TimeOut==ret)  break;
		if(ret==News_NextInput)
		{
			if(++ii>3) ii=0;
		}
		else if(ret==News_PrevInput)
		{
			if(--ii<0) ii=3;
		}
	}while(ret!=News_CommitInput);
       gLangDriver->RightToLeft=RightToLeft;
	return ret;
#endif
	return 0;//treckle
}

int DoSetIPAddress(void *p)
{
#if 0
	int ii,ret,ipa[4];
	ipa[0]=gOptions.IPAddress[0];
	ipa[1]=gOptions.IPAddress[1];
	ipa[2]=gOptions.IPAddress[2];
	ipa[3]=gOptions.IPAddress[3];
	ret=InputIPAddress(LoadStrByID(MID_NET_IP), ipa);
	for(ii=0;ii<4;ii++)
		if(ipa[ii]!=gOptions.IPAddress[ii])
		{
		      if(gOptions.Saved) gOptions.Saved=FALSE;
		      gOptions.IPAddress[ii]=ipa[ii];
	       }
	return ret;
#endif
	return 0;
}

int DoSetGATEIPAddress(void *p)
{
#if 0
	int ii,ret,ipa[4];
	ipa[0]=gOptions.GATEIPAddress[0];
	ipa[1]=gOptions.GATEIPAddress[1];
	ipa[2]=gOptions.GATEIPAddress[2];
	ipa[3]=gOptions.GATEIPAddress[3];
	ret=InputIPAddress(LoadStrByID(MID_GATEWAY_IP), ipa);
	for(ii=0;ii<4;ii++)
		if(ipa[ii]!=gOptions.GATEIPAddress[ii])
		{
		      if(gOptions.Saved) gOptions.Saved=FALSE;
		      gOptions.GATEIPAddress[ii]=ipa[ii];
	       }
	return ret;
#endif
	return 0;//treckle
}

int DoSetNetMask(void *p)
{
#if 0
	int ii,ret,ipa[4];
	ipa[0]=gOptions.NetMask[0];
	ipa[1]=gOptions.NetMask[1];
	ipa[2]=gOptions.NetMask[2];
	ipa[3]=gOptions.NetMask[3];
	ret=InputIPAddress(LoadStrByID(MID_NETMASK_ADDR), ipa);
	for(ii=0;ii<4;ii++)
		if(ipa[ii]!=gOptions.NetMask[ii])
		{
		    if(gOptions.Saved) gOptions.Saved=FALSE;
		    gOptions.NetMask[ii]=ipa[ii];
	       }
	return ret;
#endif
	return 0;//treckle
}

char cbuf[20][MAX_CHAR_WIDTH];

int DoSetNetworkOff(void *p)
{
#if 0
	int ret=InputYesNoItem((PMsg)p, &gOptions.NetworkOn);
	if(gOptions.NetworkOn && (gOptions.RS232On || gOptions.RS485On) && LoadInteger(IsNetSwitch, 0))
	{
		gOptions.RS232On=0;
		gOptions.RS485On=0;
		sprintf(format, "%%-%ds%%3s", gLCDCharWidth-3-MenuIndicatorWidth);
		sprintf(cbuf[3],format,LoadStrByID(MID_OC_RS232OFF),GetYesNoName(gOptions.RS232On));
		sprintf(cbuf[4],format,LoadStrByID(MID_OC_RS485OFF),GetYesNoName(gOptions.RS485On));
	}
	return ret;
#endif
	return 0;//treckle
}

int DoSetRS232Off(void *p)
{
	int ret=InputYesNoItem((PMsg)p, &gOptions.RS232On);
	if(gOptions.NetworkOn && (gOptions.RS232On || gOptions.RS485On) && LoadInteger(IsNetSwitch, 0))
	{
		gOptions.NetworkOn=0;
		sprintf(format, "%%-%ds%%3s", gLCDCharWidth-3-MenuIndicatorWidth);
		sprintf(cbuf[2],format,LoadStrByID(MID_OC_NETOFF),GetYesNoName(gOptions.NetworkOn));
	}
	gOptions.RS485On = !gOptions.RS232On;
	if(gOptions.IsSupportModem&&gOptions.RS485On)
	{
		gOptions.IsConnectModem=0;
		sprintf(cbuf[7],format,LoadStrByID(MID_MODEM),GetYesNoName(gOptions.IsConnectModem));
	}
	sprintf(format, "%%-%ds%%3s", gLCDCharWidth-3-MenuIndicatorWidth);
	sprintf(cbuf[3],format,LoadStrByID(MID_OC_RS232OFF),GetYesNoName(gOptions.RS232On));
	sprintf(cbuf[4],format,LoadStrByID(MID_OC_RS485OFF),GetYesNoName(gOptions.RS485On));
	return ret;
}

int DoSetRS485Off(void *p)
{
	int ret=InputYesNoItem((PMsg)p, &gOptions.RS485On);
	if(gOptions.NetworkOn && (gOptions.RS232On || gOptions.RS485On) && LoadInteger(IsNetSwitch, 0))
	{
		gOptions.NetworkOn=0;
		sprintf(format, "%%-%ds%%3s", gLCDCharWidth-3-MenuIndicatorWidth);
		sprintf(cbuf[2],format,LoadStrByID(MID_OC_NETOFF),GetYesNoName(gOptions.NetworkOn));
	}
	gOptions.RS232On =!gOptions.RS485On;
	if(gOptions.IsSupportModem&&gOptions.RS485On)
	{
		gOptions.IsConnectModem=0;
		sprintf(cbuf[7],format,LoadStrByID(MID_MODEM),GetYesNoName(gOptions.IsConnectModem));
	}
	sprintf(format, "%%-%ds%%3s", gLCDCharWidth-3-MenuIndicatorWidth);
	sprintf(cbuf[3],format,LoadStrByID(MID_OC_RS232OFF),GetYesNoName(gOptions.RS232On));
	sprintf(cbuf[4],format,LoadStrByID(MID_OC_RS485OFF),GetYesNoName(gOptions.RS485On));
	return ret;
}

int DoSetFpOpenOff(void *p)
{
        int ret=InputYesNoItem((PMsg)p, &gOptions.FPOpenRelay);

        gOptions.AutoOpenRelay =!gOptions.FPOpenRelay;
        sprintf(format, "%%-%ds%%3s", gLCDCharWidth-3-MenuIndicatorWidth);
        sprintf(autobuf[0],format,LoadStrByID(MID_AUTO_FPOPEN),GetYesNoName(gOptions.FPOpenRelay));
        sprintf(autobuf[1],format,LoadStrByID(MID_AUTO_OPENDOOR),GetYesNoName(gOptions.AutoOpenRelay));
        return ret;

}
int DoSetAutoOpenOff(void *p)
{
        int ret=InputYesNoItem((PMsg)p, &gOptions.AutoOpenRelay);

        gOptions.FPOpenRelay =!gOptions.AutoOpenRelay;
        sprintf(format, "%%-%ds%%3s", gLCDCharWidth-3-MenuIndicatorWidth);
        sprintf(autobuf[0],format,LoadStrByID(MID_AUTO_FPOPEN),GetYesNoName(gOptions.FPOpenRelay));
        sprintf(autobuf[1],format,LoadStrByID(MID_AUTO_OPENDOOR),GetYesNoName(gOptions.AutoOpenRelay));
        return ret;

}


int DoSetAutoOpenTimes(void *p)
{
        return InputValueOfItem((PMsg)p, 3, 1, 20, &gOptions.AutoOpenRelayTimes);
}

int DoSetDHCP(void *p)
{
//	return InputYesNoItem((PMsg)p, &gOptions.DHCP);	
	return 0;//treckle
}

//static char *NetNames[]={"10M-H","100M-H", "10M-F", "100M-F", "AUTO"};

char *GetNetSpeedName(int Speed)
{
#if 0
	if(Speed==ETH_100MHD) return NetNames[1];
	if(Speed==ETH_10MFD) return NetNames[2];
	if(Speed==ETH_100MFD) return NetNames[3];
	if(Speed==ETH_AUTO) return NetNames[4];
	return NetNames[0];
#endif
	return NULL;//treckle
}

int DoSetHighNet(void *p)
{
#if 0
	char Items[100];
	int Values[]={ETH_10MHD,ETH_100MHD,ETH_10MFD,ETH_100MFD,ETH_AUTO};
	PMenu menu=((PMenu)((PMsg)p)->Object);
	int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret;
	int col=gLCDCharWidth-6, index; 
	sprintf(Items, "%s:%s:%s:%s:%s", GetNetSpeedName(ETH_10MHD),GetNetSpeedName(ETH_100MHD),
		GetNetSpeedName(ETH_10MFD),GetNetSpeedName(ETH_100MFD),GetNetSpeedName(ETH_AUTO));
	index=gOptions.HiSpeedNet;
	ret=LCDSelectItemValue(row,col,6,Items,Values,&index);
	if(ret==News_CommitInput)
	{
		if(index!=gOptions.HiSpeedNet)
		{
			sprintf(menu->Items[menu->ItemIndex].Caption+gLCDCharWidth-6-MenuIndicatorWidth,"%6s",GetNetSpeedName(index));
			gOptions.HiSpeedNet=index;
			gOptions.Saved=FALSE;
		}
	}
	return ret;	
#endif
	return 0;//treckle
}

int DoSetCommKey(void *p)
{
	return InputValueOfItem((PMsg)p, 6, 0, 999999, &gOptions.ComKey);
}

int DoSetModem(void *p)
{
#if 0
	int ret=InputYesNoItem((PMsg)p, &gOptions.IsConnectModem);
	if(gOptions.IsConnectModem)
	{
		gOptions.RS232On = gOptions.IsConnectModem;
		gOptions.RS485On = !gOptions.RS232On;
		sprintf(format, "%%-%ds%%3s", gLCDCharWidth-3-MenuIndicatorWidth);
		sprintf(cbuf[3],format,LoadStrByID(MID_OC_RS232OFF),GetYesNoName(gOptions.RS232On));
		sprintf(cbuf[4],format,LoadStrByID(MID_OC_RS485OFF),GetYesNoName(gOptions.RS485On));
	}
	return ret;
#endif
	return 0;//treckle
}

int DoSetProxyIPAddress(void *p)
{
#if 0
	int ii,ret,ipa[4];
	ipa[0]=gOptions.ProxyServerIP[0];
	ipa[1]=gOptions.ProxyServerIP[1];
	ipa[2]=gOptions.ProxyServerIP[2];
	ipa[3]=gOptions.ProxyServerIP[3];
	ret=InputIPAddress(LoadStrByID(MID_PROXY_IP), ipa);
	for(ii=0;ii<4;ii++)
	{
		if(ipa[ii]!=gOptions.ProxyServerIP[ii])
		{
			if(gOptions.Saved) gOptions.Saved=FALSE;
			gOptions.ProxyServerIP[ii]=ipa[ii];
		}
	}
	return ret;
#endif
	return 0;//treckle
}

int DoSetProxyPort(void *p)
{
//	return InputValueOfItem((PMsg)p, 5, 0, 65535, &gOptions.ProxyServerPort);
	return 0;//treckle
}

int DoSetEnableProxyServer(void *p)
{
	return InputYesNoItem((PMsg)p, &gOptions.EnableProxyServer);		
}

int	DoOptionLock(void *p)
{
	int ret;
	char cbuf[4][MAX_CHAR_WIDTH], dsmname[10];

	PMenu m23=CreateMenu(L3000_STR_REPLACE14, gOptions.MenuStyle, NULL, NULL);
	AddYNOptMenu(m23,&gOptions.VryFailWait);
	AddInputOptMenu(m23,&gOptions.VryFailWaitCnt);
	//L3000Dbg("\nLockOn:%d DevID:%d\n", gOptions.LockOn, gOptions.DeviceID);
	AddInputOptMenu(m23,&gOptions.LockOn);
	//AddMenuItem(m23, MenuFmtStr(cbuf[2],MID_AC_DSM,dsmname), DoSetDoorSensorMode, NULL);
	//AddMenuItem(m23, LoadStrByID(MID_AC_DSM), DoSetDoorSensorMode, NULL);
	//AddYNOptMenu(m23, &gOptions.DoorSensorMode);
	AddInputOptMenu(m23,&gOptions.VryBind);
	AddYNOptMenu(m23, &gOptions.NorOpenOn);
	//AddYNOptMenu(m23, &gOptions.CloseDoorHint);
	//if( ((gOptions.RFCardFunOn || gMFOpened)) )
	if( gMFOpened )
	{
		AddYNOptMenu(m23, &gOptions.OnlyPINCard);
	}

	gOptions.Saved=TRUE;
	ret=RunMenu(m23);
	DestroyMenu(m23);
	if(!gOptions.Saved && ret!=News_TimeOut)
	{
		ret=LCDSelectOK(L3000_STR_REPLACE14 ,LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret)
		{
			SaveOptions(&gOptions);
		}
		else
			LoadOptions(&gOptions);
	}

	return ret;
}

int gbaudrate = 0;
int DoOptionCOMM(void *p)
{
	extern int gEthOpened;
	//通讯设置
	int ret;
	static PMenu m23=NULL;
	m23=CreateMenu(LoadStrByID(MID_OPTIONS_COMM),gOptions.MenuStyle,NULL, m23);
	if(gEthOpened)
	{
		if (LoadInteger(IsNetSwitch, 0))
			AddMenuItem(1, m23, MenuFmtStr(cbuf[2],MID_OC_NETOFF,GetYesNoName(gOptions.NetworkOn)), DoSetNetworkOff, NULL);
		AddMenuItem(1, m23, LoadStrByID(MID_NET_IP), DoSetIPAddress, NULL); 
		AddMenuItem(1, m23, LoadStrByID(MID_NETMASK_ADDR), DoSetNetMask, NULL); 
		AddMenuItem(1, m23, LoadStrByID(MID_GATEWAY_IP), DoSetGATEIPAddress, NULL); 			    if (gOptions.DHCPFunOn)
			AddMenuItem(1, m23, MenuFmtStr(cbuf[8],MID_NET_DHCP,GetYesNoName(gOptions.DHCP)), DoSetDHCP, NULL);
		AddMenuItem(1, m23, MenuFmtStr(cbuf[5],MID_OS_HIGHSPEED,GetNetSpeedName(gOptions.HiSpeedNet)), DoSetHighNet, NULL);
	}
	AddMenuItem(1, m23, MenuFmtInt(cbuf[1],MID_OS_DEVNUMBER,gOptions.DeviceID), DoSetDevNumber, NULL);	
	if(gOptions.RS232On||gOptions.RS485On||LoadInteger(IsNetSwitch, 0))
	{
		AddMenuItem(1, m23, MenuFmtInt(cbuf[0],MID_OC_BAUDRATE,gOptions.RS232BaudRate), DoSetBaudRate, NULL);
		AddMenuItem(0, m23, MenuFmtStr(cbuf[3],MID_OC_RS232OFF,GetYesNoName(gOptions.RS232On)), DoSetRS232Off, NULL);
		AddMenuItem(0, m23, MenuFmtStr(cbuf[4],MID_OC_RS485OFF,GetYesNoName(gOptions.RS485On)), DoSetRS485Off, NULL);
	}
	if(TESTPRIVILLEGE(PRI_SUPERVISOR))
		AddMenuItem(1, m23, MenuFmtInt(cbuf[6],MID_OS_COMKEY, gOptions.ComKey), DoSetCommKey, NULL);
	if (gOptions.IsSupportModem)
		AddMenuItem(1, m23, MenuFmtStr(cbuf[7],MID_MODEM, GetYesNoName(gOptions.IsConnectModem)), DoSetModem, NULL);
	if(gOptions.IClockFunOn)
	{
		AddMenuItem(1, m23, MenuFmtStr(cbuf[10],MID_PROXY_SERVER,GetYesNoName(gOptions.EnableProxyServer)), DoSetEnableProxyServer, NULL);
		AddMenuItem(1, m23, LoadStrByID(MID_PROXY_IP), DoSetProxyIPAddress, NULL); 		
		AddMenuItem(1, m23, MenuFmtInt(cbuf[9],MID_PROXY_PORT, gOptions.ProxyServerPort), DoSetProxyPort, NULL);
	}	
	gbaudrate = gOptions.RS232BaudRate;
	gOptions.Saved=TRUE;
	ret=RunMenu(m23);
	DestroyMenu(m23);
	if(!gOptions.Saved && ret!=News_TimeOut)
	{
		ret=LCDSelectOK(LoadStrByID(MID_OPTIONS_COMM),LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret)
		{ 
			SaveOptions(&gOptions);
			if (gbaudrate != gOptions.RS232BaudRate)
			{
				LCDInfoShow(LoadStrByID(MID_OC_BAUDRATE), LoadStrByID(HID_RESTART));
				DelayMS(2*1000);
			}
		/*
//for iclock688
                if(gOptions.IsSupportModem&&gOptions.IsConnectModem)
                {
			
                        int fdrs232=0;
                        char buf[1024];
                        fdrs232=open(GetEnvFilePath("USERDATAPATH", "RS232FunOn.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
                close(fdrs232);
			
                }
                else
                {
                        char buf[1024];
                        unlink(GetEnvFilePath("USERDATAPATH", "RS232FunOn.dat", buf));
                }

		*/ //treckle


		}
		else 
			LoadOptions(&gOptions);
	}
	return ret;
}

int DoSetMAC(void *p)
{
#if 0
	char buf[MAX_CHAR_WIDTH];
	int ii, ret, mac[6];
	LCDWriteCenterStrID(0, MID_NET_MAC);
        LCDWriteStr(1,0,SPadCenterStr(buf,gLCDCharWidth,0),0);
	sprintf(buf,"%3d:%3d:%3d",gOptions.MAC[0],gOptions.MAC[1],gOptions.MAC[2]);
	LCDWriteStr(1,2,buf,0);
	LCDWriteStr(2,0,SPadCenterStr(buf,gLCDCharWidth,0),0);
	sprintf(buf,"%3d:%3d:%3d",gOptions.MAC[3],gOptions.MAC[4],gOptions.MAC[5]);
	LCDWriteStr(2,2,buf,0);
	LCDWriteCenterStrID(3, HID_OKCANCEL);
	for(ii=0;ii<6;ii++)	mac[ii]=gOptions.MAC[ii];
	ii=0;
	do
	{
		if(ii==2 || ii==4)	ret=1;	else ret=0;
		ret=InputNumberAt(1+ii/3,2+ii%3*4,3,mac+ii,ret,255); 
		if(ret==News_ErrorInput) continue;
		if(News_CancelInput==ret || News_TimeOut==ret) return ret;
		if(ret==News_NextInput)
		{
			if(++ii>5) ii=0;
		}
		else if(ret==News_PrevInput)
		{
			if(--ii<0) ii=5;
		}
	}while(ret!=News_CommitInput);
	for(ii=0;ii<6;ii++)
		if(mac[ii]!=gOptions.MAC[ii])
		{
		if(gOptions.Saved) gOptions.Saved=FALSE;
		gOptions.MAC[ii]=mac[ii];
	}
	return ret;
#endif
	return 0;//treckle
}

int DoSetUDPPort(void *p)
{
	return InputValueOfItem((PMsg)p, 4, 20, 9999, &gOptions.UDPPort);
}
int DoSetWebPort(void *p)
{
	return InputValueOfItem((PMsg)p, 4, 80, 9999, &gOptions.WEBPort);
}

int DownloadData(int ContentType)
{
#if MACHINE_ID == 2
	char buffer[80];
	int sign=FALSE;
	int mount;
	
	LCD_Clear();
	
	LCDWriteCenterStr(1, LoadStrByID(HID_MOUNTING_PENDRV));
	
	mount=fatfile_Setup_usb();
	
	if (mount==0) //successful
		LCDWriteCenterStr(1, LoadStrByID(HID_DOWNLOADING_DATA));
	else if (mount==2) //pls plug pen driver
		LCDWriteCenterStr(1, LoadStrByID(HID_PLUGPENDRIVE));	
	else
		LCDWriteCenterStr(1, LoadStrByID(HID_PENDRIVE_NOEXIST));	
	
	if (mount!=0) 
	{
		return InputLine(0,0,0,NULL);
	}
	
	wdt_set_count(0);
	ExLightLED(LED_GREEN, TRUE);
	ExLightLED(LED_RED, FALSE);
	//start download data
	if (ContentType==FCT_ATTLOG)
	{
		sprintf(buffer, "%s%d_%s", g_usbpath, gOptions.DeviceID, "attlog.dat");
		//convert bianry data to text data
		if (FDB_Download(FCT_ATTLOG, buffer))
			sign=TRUE;
	}
	else if (ContentType==FCT_USER)
	{
		sprintf(buffer, "%s%s", g_usbpath, "user.dat");
		if (FDB_Download(FCT_USER, buffer))
		{ 
			if (!gOptions.IsOnlyRFMachine)
			{
				if (gOptions.ZKFPVersion == ZKFPV10)
					sprintf(buffer, "%s%s", g_usbpath, "template.fp10.1");
				else
					sprintf(buffer, "%s%s", g_usbpath, "template.dat");
				FDB_Download(FCT_FINGERTMP, buffer);
			}
			
			sign=TRUE;
		}
	}
#if 0
	else if (ContentType==FCT_SMS)
	{
		//SMS DATA
		sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "sms.dat");
		if (FDB_Download(FCT_SMS, buffer))
		{
			sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "udata.dat");
			if (FDB_Download(FCT_UDATA, buffer))
				sign=TRUE;
		}
	}
#endif
		
	//DoShellCMD("sync");
	
	if (mount==0) fatfile_TearDown_usb();
	
	//Display download result
	if (sign)
		LCDWriteCenterStr(1, LoadStrByID(HID_COPYDATA_SUCCEED));		
	else
		LCDWriteCenterStr(1, LoadStrByID(HID_COPYDATA_FAILURE));    
	
	return InputLine(0,0,0,NULL);
#else
	return 0;
#endif
}

#if 0	//treckle
int UploadVoiceFromDisk()
{
	int ret;
        char buffer[150];
        int mount;

        LCD_Clear();

        LCDWriteCenterStr(1, LoadStrByID(HID_MOUNTING_PENDRV));

        mount=DoMountUdisk();

        if (mount==0) //successful
                LCDWriteCenterStr(1, LoadStrByID(HID_DOWNLOADING_DATA));
        else if (mount==2) //pls plug pen driver
                LCDWriteCenterStr(1, LoadStrByID(HID_PLUGPENDRIVE));
        else
                LCDWriteCenterStr(1, LoadStrByID(HID_PENDRIVE_NOEXIST));

        if (mount!=0)
        {
                return InputLine(0,0,0,NULL);
        }
	
#ifdef ZEM300
        sprintf(buffer, "cp %s/%c_*.wav /mnt/mtdblock/ ", USB_MOUNTPOINT, gOptions.Language);
#else
        sprintf(buffer, "cd %s && tar czvf res1.tgz  *.wav && sync && cp res1.tgz /mnt/mtdblock/ &&sync && rm -rf res1.tgz && sync", USB_MOUNTPOINT);
#endif
        //sprintf(buffer, "tar czvf /mnt/mtdblock/res1.tgz  %s/*.wav", USB_MOUNTPOINT);
        //sprintf(buffer, "tar czvf /mnt/mtdblock/res1.tgz  %s/%c_*.wav", USB_MOUNTPOINT, gOptions.Language);

	//DBPRINTF("voice: %s\n",buffer);
	ret = system(buffer);
        if (mount==0) DoUmountUdisk();
        if (ret==EXIT_SUCCESS)	 
	        LCDWriteCenterStr(1, LoadStrByID(HID_COPYDATA_SUCCEED));
	else
		LCDWriteCenterStr(1, LoadStrByID(HID_COPYDATA_FAILURE));    
	
	return InputLine(0,0,0,NULL);
}
#endif

int UploadData(int ContentType)
{
#if MACHINE_ID == 2
	char buffer[80];
	int sign=FALSE;
	int mount,ret;
	FIL fd;
	U16 pin=0;
	BYTE fpindex=0;
	int iCnt=0, probar=0;
	int tmpsize,readlen=0;
	char *tmpbuf;
	unsigned char head[6];
	TUser user;
	TTemplate curtmp;
	TSms sms;
	TUData udata;
	FRESULT fat_err;
	
	LCD_Clear();
	
	LCDWriteCenterStr(1, LoadStrByID(HID_MOUNTING_PENDRV));

	mount=fatfile_Setup_usb();
	
	if (mount==0) //successful
		LCDWriteCenterStr(1, LoadStrByID(HID_DOWNLOADING_DATA));
	else if (mount==2) //pls plug pen driver
		LCDWriteCenterStr(1, LoadStrByID(HID_PLUGPENDRIVE));	
	else
		LCDWriteCenterStr(1, LoadStrByID(HID_PENDRIVE_NOEXIST));	
	
	if (mount!=0) 
	{
		return InputLine(0,0,0,NULL);
	}
	
	wdt_set_count(0);
	ExLightLED(LED_GREEN, TRUE);
	ExLightLED(LED_RED, FALSE);
	//start upload data
	if (ContentType==FCT_USER)
	{
		sprintf(buffer, "%s%s", g_usbpath, "user.dat");
		if ((fat_err=f_open(&fd, buffer, FA_READ))==0)
		{
			iCnt = fd.fsize;
			if (!gOptions.IsOnlyRFMachine)
				iCnt /= (PROGBAR_USER_NUM*sizeof(TUser));
			else
				iCnt /= (PROGBAR_MAX_NUM*sizeof(TUser));

			if (0 == iCnt)
				iCnt = 1;

			f_lseek(&fd, 0); 
			while(TRUE)
			{
				wdt_set_count(0);
				if ((f_read(&fd, &user, sizeof(TUser), &readlen)==0) && (readlen==sizeof(TUser))&&(user.PIN>0))
				{
					AppendFullUser(&user);
				}
				else break;

				probar++;
				DrawProgbar(probar/iCnt);
			}
			if (!gOptions.IsOnlyRFMachine)
				DrawProgbar(PROGBAR_USER_NUM);
			else
				DrawProgbar(PROGBAR_MAX_NUM);

			f_close(&fd);
			if (!gOptions.IsOnlyRFMachine)
			{
				int tmplen = sizeof(TTemplate);
				if (gOptions.ZKFPVersion == ZKFPV10)
				{
					tmplen = 900;
					sprintf(buffer, "%s%s", g_usbpath, "template.fp10.1");	
				}
				else
				{
					tmplen -= ZKFP_OFF_LEN;
					sprintf(buffer, "%s%s", g_usbpath, "template.dat");	
				}

				tmpbuf = malloc(20*1024);
				if (tmpbuf!=NULL && (fat_err=f_open(&fd, buffer, FA_READ))==0)
				{
					probar = 0;
					iCnt = fd.fsize;
					iCnt /= (PROGBAR_FP_NUM*tmplen);
					if (0 == iCnt)
						iCnt = 1;

					f_lseek(&fd, 0); 
					while(TRUE)
					{
						wdt_set_count(0);
						if (gOptions.ZKFPVersion==ZKFPV10)
						{
							memset(head,0,sizeof(head));
							if (f_read(&fd, head, sizeof(head), &readlen) || readlen!=sizeof(head))
							{
								printf("read head of template failed\n");
								DrawProgbar(PROGBAR_MAX_NUM);
								break;
							}

							pin=GETWORD(head+2);
							fpindex=*(head+4);
							tmpsize=GETWORD(head)-6;
							if (f_read(&fd, tmpbuf, tmpsize, &readlen)==0 && readlen==tmpsize)
							{
								if (tmpsize > ZKFPV10_MAX_LEN)
									tmpsize = ZKFPV10_MAX_LEN;
								AppendUserTemp(pin, NULL, fpindex, tmpbuf, tmpsize);
							}
							else 
							{
								DrawProgbar(PROGBAR_MAX_NUM);
								break;
							}
						}
						else
						{
							if ((f_read(&fd, &curtmp, tmplen, &readlen)==0 && readlen==tmplen)&&curtmp.Valid)
							{
								AppendUserTemp(curtmp.PIN, NULL, curtmp.FingerID, curtmp.Template, curtmp.Size-6);
							}
							else 
							{
								DrawProgbar(PROGBAR_MAX_NUM);
								break;
							}
						}

						probar++;
						DrawProgbar(PROGBAR_USER_NUM + probar/iCnt);
					}
					f_close(&fd);
				}
				
				if (tmpbuf)
					free(tmpbuf);
			}

			sign=TRUE;
		}
	}
#if 0
	else if (ContentType==FCT_SMS)
	{
		//SMS DATA
		sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "sms.dat");
		if ((fd=open(buffer, O_RDONLY))!=-1)
		{
			lseek(fd, 0, SEEK_SET); 
			while(TRUE)
			{
				if (read(fd, &sms, sizeof(TSms))==sizeof(TSms))
				{
					ret=FDB_ChgSms(&sms); 
					if(ret==FDB_ERROR_NODATA)
					{
						if (FDB_AddSms(&sms)!=FDB_OK) break;
					}
				}
				else break;
			}
			close(fd);
			sign=TRUE;
		}
		sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "udata.dat");
		if ((fd=open(buffer, O_RDONLY))!=-1)
		{
			lseek(fd, 0, SEEK_SET); 
			while(TRUE)
			{
				if (read(fd, &udata, sizeof(TUData))==sizeof(TUData))
				{
					if (FDB_DelUData(udata.PIN, 0)==FDB_OK)
						if (FDB_AddUData(&udata)!=FDB_OK) break;
				}
				else break;
			}
			close(fd);
			sign=TRUE;
		}
		if(gOptions.IsSupportSMS) CheckBoardSMS();
	}
#endif	
	if (mount==0) fatfile_TearDown_usb();
	
	//Display upload result
	if (sign)
	{
		LCDWriteCenterStr(1, LoadStrByID(HID_COPYDATA_SUCCEED));		
		if (ContentType==FCT_USER)
		{
			FDB_InitDBs(FALSE);
			FPInit(NULL);
		}
	}
	else
		LCDWriteCenterStr(1, LoadStrByID(HID_COPYDATA_FAILURE));    
	
	return InputLine(0,0,0,NULL);
#else
	return 0;
#endif	
}

int DoDownloadAttLog(void *p)
{    
	return DownloadData(FCT_ATTLOG);
}

int DoDownloadUser(void *p)
{
	return DownloadData(FCT_USER);    
}

int DoUploadUser(void *p)
{
	return UploadData(FCT_USER);    
}

/* //treckle
int DoDownloadSMS(void *p)
{
	return DownloadData(FCT_SMS);    
}

int DoUploadSMS(void *p)
{
	return UploadData(FCT_SMS);    
}

int DoUploadVoice(void *p)
{
	return UploadVoiceFromDisk();
}
*/
int DoRestoreVoice(void *p)
{
	return 0;	//treckle
}

char *LoadFirmware(char *FirmwareFile, char *Version, int *Length)
{
#if 0
	FILE *fh;
	char line[4000], linename[20];
	int i=0, dataline=0, checksum=0, position=0;
	char *fdata=NULL;
	char platform[30];
	char platformvalue[10];

	if (LoadStr(PLATFORM,platformvalue))
			sprintf(platform,"%s%s",platformvalue,"_FirmwareVersion=");
	else
			sprintf(platform,"%s%s","ZEM200","_FirmwareVersion=");

	
		
	fh=fopen(FirmwareFile, "r");
	if(fh!=NULL)
	{
		sprintf(linename,"Data%d=",dataline);
		while(fgets(line, 4000, fh))
		{
			//if((i==0)&&(strncmp(line,"ZEM200_FirmwareVersion=", strlen("ZEM200_FirmwareVersion="))==0))
			if((i==0)&&(strncmp(line,platform, strlen(platform))==0))
			{
				//strcpy(Version, line+strlen("ZEM200_FirmwareVersion="));
				strcpy(Version, line+strlen(platform));
			}
			else if(strncmp(line, "FirmwareLength=",strlen("FirmwareLength="))==0)
			{				
				*Length=atoi(line+strlen("FirmwareLength="));
				fdata=malloc(4000+*Length);
				i++;
			}
			else if(strncmp(line, "FirmwareCheckSum=",strlen("FirmwareCheckSum="))==0)
			{
				checksum=atoi(line+strlen("FirmwareCheckSum="));
				i++;
			}
			else if(i==2)
			{
				if(strncmp(line, linename, strlen(linename))==0)
				{
					line[strlen(line)-1]='\0';
					position+=Decode16(line+strlen(linename), fdata+position);
					sprintf(linename,"Data%d=",++dataline);
					if(position==*Length) break;
				}
			}
		}
		fclose(fh);
	}
	if(position==*Length)
	{
		if(position) 
			if(in_chksum(fdata, position)==checksum)
				return fdata;
	}
	if(fdata) free(fdata);
	return NULL;
#endif
	return NULL;//treckle
}

/*
 * copy the file to nand flash from udisk
 * @fname:  file name
 * @offset: offset blocks
 * @blocks: the size of data in block
 */
int udisk2nand(char *fname, int offset, int blocks)
{
	char *buf = NULL;
	int  buf_size = YAFFS_BYTES_PER_BLOCK;
	int  i;
	int len;
	off_t file_size;
	FRESULT fat_err;	
	FIL fd;
	char *p;

  	fat_err=f_open(&fd, fname, FA_READ);
	if(fat_err)
	{
		if(fat_err == FR_NO_FILE)
		{
			return TRUE;
		}

		return FALSE;
	}

	LCDWriteCenterStr(2, fname);

	buf = malloc(buf_size);
	if(buf == NULL)
	{
		f_close(&fd);
		return FALSE;
	}

	for(i=0; i<blocks; i++)
	{
		memset(buf, 0, buf_size);
		fat_err = f_read(&fd, buf, buf_size, &len);
		if(fat_err)
		{
			free(buf);	
			f_close(&fd);
			return FALSE;
		}

		if(len == 0)
		{
			break;
		}

		update_ucos(buf, offset + i, buf_size);

		sprintf(buf, "%d/%d", i + 1, blocks);	
		LCDWriteCenterStr(3, buf);
		wdt_set_count(0);
	}

	free(buf);	
	f_close(&fd);
	return TRUE;
}

/*
 * copy the file to yaffs file system from udisk
 * @fname:  file name
 */
int udisk2yaffs(char *file_dest, char *file_src)
{
	char *buf = NULL;
	int  buf_size = 16*1024;
	FRESULT fat_err;	
	FIL fsrc;
	int fdest;
	int len;
	int cnt = 0;

  	fat_err=f_open(&fsrc, file_src, FA_READ);
	if(fat_err)
	{
		if(fat_err == FR_NO_FILE)
		{
			return TRUE;
		}

		return FALSE;
	}

	LCDWriteCenterStr(2, file_src);

	fdest = yaffs_open(file_dest, O_TRUNC | O_CREAT | O_RDWR, 0777);
	if(fdest < 0)
	{
		f_close(&fsrc);
		return FALSE;
	}

	buf = malloc(buf_size);
	if(buf == NULL)
	{
		f_close(&fsrc);
		yaffs_close(fdest);
	}

	while(1)
	{
		fat_err = f_read(&fsrc, buf, buf_size, &len);
		if(fat_err)
		{
			f_close(&fsrc);
			yaffs_close(fdest);
			free(buf);
			return FALSE;
		}

		if(len == 0)
		{
			break;
		}

		yaffs_write(fdest, buf, len);
		wdt_set_count(0);
	}

	f_close(&fsrc);
	yaffs_close(fdest);
	free(buf);

	return TRUE;
}

int UpdateFirmware(char *filename)
{
	int i, ret;
	char fsrc[128];
	char fdest[128];
	const char *fw_dir = "2:emfw";	
	const char *files[] = {
		"options.cfg",
		"beep.wav",
		"ffiso.dat",
		"GB2312.FT",
		"help_cn.pdf",
		"help_en.pdf",
		"LANGUAGE.E",
		"LANGUAGE.S",
		"S_0.wav",
		"S_10.wav",
		"S_11.wav",
		"S_1.wav",
		"S_2.wav",
		"S_3.wav",
		"S_4.wav",
		"S_5.wav",
		"S_6.wav",
		"S_7.wav",
		"S_8.wav",
		"S_9.wav",
		"SYM.FT",
	};

	sprintf(fsrc, "%s/ucos.bin", fw_dir, files[i]);
	if( udisk2nand(fsrc, 1, 128) == FALSE)
	{
		return FALSE;
	}

	for(i=0; i < sizeof(files)/sizeof(files[0]); i++)
	{
		sprintf(fsrc, "%s/%s", fw_dir, files[i]);
		sprintf(fdest, "/mnt/mtdblock/%s", files[i]);

		if( udisk2yaffs(fdest, fsrc) == FALSE )
		{
			return FALSE;
		}

		sprintf(fsrc, "%d/%d", i + 1, sizeof(files)/sizeof(files[0]));	
		LCDWriteCenterStr(3, fsrc);
	}

	LCDClearLine(2);
	LCDClearLine(3);
	
	return TRUE;
#if 0
  	fat_err=f_open(&fd, filename, FA_READ);
	if(fat_err)
	{
		LCDWriteCenterStr(3, "open err");
		DelayMS(1000);
		return FALSE;
	}

	buf = malloc(buf_size);
	if(buf == NULL)
	{
		LCDWriteCenterStr(3, "malloc err");
		DelayMS(1000);
		f_close(&fd);
		return FALSE;
	}

	ret = TRUE;
	fat_err = f_read(&fd, buf, buf_size, &len);
	if(fat_err != 0)
	{
		LCDWriteCenterStr(3, "read err");
		DelayMS(1000);

		ret = FALSE;
	}
	else
	{
		offset = CFG_NAND_UCOS_OFFS/YAFFS_BYTES_PER_BLOCK;

		if(update_ucos(buf, offset, buf_size))
		{
			LCDWriteCenterStr(3, "update err");
			DelayMS(1000);

			ret = FALSE;
		}
	}

	f_close(&fd);
	free(buf);
	return ret;
#endif
}

int DoUpdateFirmware(void *p)
{
	int sign=FALSE;
	int mount;
	char fw_name[100];
	//sprintf(fw_name, "%semfw.cfg", g_usbpath);
	sprintf(fw_name, "%sucos.bin", g_usbpath);

	LCD_Clear();    
	LCDWriteCenterStr(1, LoadStrByID(HID_MOUNTING_PENDRV)); 

	mount=fatfile_Setup_usb();
	if (mount==0) //successful
	{
		//start update file
		LCDWriteCenterStr(1, LoadStrByID(HID_DOWNLOADING_DATA));
		sign=UpdateFirmware(fw_name);
		//Display upload result
		if (sign)
		{
			LCDWriteCenterStr(1, LoadStrByID(HID_COPYDATA_SUCCEED));
			DelayMS(1000);
			FinishProgram();
		}
		else
		{
			LCDWriteCenterStr(1, LoadStrByID(HID_COPYDATA_FAILURE));    		    
		}

		fatfile_TearDown_usb();
	}
	else if (mount==2) //pls plug pen driver
		LCDWriteCenterStr(1, LoadStrByID(HID_PLUGPENDRIVE));	
	else
		LCDWriteCenterStr(1, LoadStrByID(HID_PENDRIVE_NOEXIST));		

	InputLine(0,0,0,NULL);

	return 0;
}

extern int gAlarmDelay;
extern int gErrTimes;
extern int C2connect;

int DoSetAlarmOff(void *p)
{
        //ExAlarmOff(0);
	DoAlarmOff(0);
	gErrTimes=0;
        FDB_AddOPLog(ADMINPIN, OP_ALARM, 0xffff,0xffff,0xffff,0xffff);
        gAlarmDelay=0;
        return News_CommitInput;
}

int DoMainMenu(void)
{
	static PMenu mainmenu=NULL;
	static PMenu m1=NULL;	
	static PMenu m2=NULL;	
	static PMenu m3=NULL;	
	int ret, oldMachineState;
	gInputNumKey = 0;
	oldMachineState=gMachineState;
	//unsigned int s1,e1;
	
	gMachineState=STA_MENU;
	ExLightLED(LED_GREEN, FALSE);
	ExLightLED(LED_RED, FALSE);
	ClockEnabled=FALSE;
	
	LCDClear();

	//s1 = GetTickCount();
	mainmenu=CreateMenu(LoadStrByID(MID_MENU),gOptions.MenuStyle,NULL, mainmenu);
	
	//if(ISHACKER||!gOptions.IsInit)
	//	AddMenuItem(1, mainmenu, LoadStrByID(MID_OS_INIT), DoHideMenu, NULL);
	//data manage
	m1=CreateMenu(LoadStrByID(MID_DATA),gOptions.MenuStyle,mainmenu, m1);
	AddMenuItem(1, m1, LoadStrByID(MID_DATA_EUSER), DoEnrollUser, NULL);
	if(gMFOpened&&!gOptions.MifareAsIDCard)
		AddMenuItem(1, m1, LoadStrByID(MID_DATA_CARD), DoCardMng, NULL);
	AddMenuItem(1, m1, LoadStrByID(MID_DATA_EADMIN), DoEnrollUser, NULL);
	AddMenuItem(1, m1, LoadStrByID(MID_DATA_DEL), DoDelUser, NULL);
	if(gOptions.ViewAttlogFunOn)
		AddMenuItem(1, m1, LoadStrByID(MID_DATA_VATTLOG), DoViewAttLog, NULL);
	AddMenuItem(1, mainmenu, LoadStrByID(MID_DATA), NULL, m1)->Icon=Icon_Menu_User;
#if 1	
	//setup
	if ((TESTPRIVILLEGE(PRI_OPTIONS))||(TESTPRIVILLEGE(PRI_SUPERVISOR)))
	{ 
		m2=CreateMenu(LoadStrByID(MID_OPTIONS),gOptions.MenuStyle,mainmenu, m2);
		if(TESTPRIVILLEGE(PRI_OPTIONS))
		{
			AddMenuItem(1, m2, L3000_STR_REPLACE14, DoOptionLock, NULL);
			AddMenuItem(1, m2, LoadStrByID(MID_OPTIONS_SYSTEM), DoOptionSystem, NULL);
		//	if(gOptions.PowerMngFunOn) AddMenuItem(1, m2, LoadStrByID(MID_OS_AUTOPOWER), DoAutoPower, NULL);
		//	AddMenuItem(1, m2, LoadStrByID(MID_OPTIONS_COMM), DoOptionCOMM, NULL);
		//	AddMenuItem(1, m2, LoadStrByID(MID_OPTIONS_REC), DoOptionRec, NULL);
		}
		if(TESTPRIVILLEGE(PRI_SUPERVISOR))
		{
			if(gOptions.LockFunOn&LOCKFUN_ADV)
				AddMenuItem(1, m2, LoadStrByID(MID_OA_OPTION), DoSetAccess, NULL);
		//	AddMenuItem(1, m2, LoadStrByID(MID_AUTOTEST), DoAutoTestMenu, NULL);
		}
		AddMenuItem(1, mainmenu, LoadStrByID(MID_OPTIONS), NULL, m2)->Icon=Icon_Menu_Option;	
	}

	//USB Manage
//	AddMenuItem(1, mainmenu, LoadStrByID(MID_PENDRV_MNG), DoUSBDiskMng, NULL)->Icon = Icon_Menu_AttLog; 
	//if(gOptions.IsSupportUSBDisk&&TESTPRIVILLEGE(PRI_OPTIONS))
	{
		m3=CreateMenu(LoadStrByID(MID_PENDRV_MNG), gOptions.MenuStyle, NULL, mainmenu);
		AddMenuItem(1, m3, LoadStrByID(MID_DOWNLOAD_ATT), DoDownloadAttLog, NULL);
		AddMenuItem(1, m3, LoadStrByID(MID_DOWNLOAD_USR), DoDownloadUser, NULL);
		AddMenuItem(1, m3, LoadStrByID(MID_UPLOAD_USR), DoUploadUser, NULL);	
		AddMenuItem(1, m3, LoadStrByID(MID_UPDATE_FIRMWARE), DoUpdateFirmware, NULL);	

		AddMenuItem(1, mainmenu, LoadStrByID(MID_PENDRV_MNG), NULL, m3)->Icon=Icon_Menu_AttLog;	
	}

	//System info
	AddMenuItem(1, mainmenu, LoadStrByID(MID_SYSINFO), DoShowSysInfo, NULL)->Icon=Icon_Menu_Info;
	//	AddMenuItem(mainmenu, "测试", DoMoveToCard, NULL);
	if(gAlarmDelay) //解除报警
		AddMenuItem(1, mainmenu, LoadStrByID(MID_AO_ALARMOFF), DoSetAlarmOff, NULL);
	FDB_AddOPLog(ADMINPIN, OP_MENU, 0,0,0,0);
#endif
	//e1= GetTickCount();
	//printf("finish   mainmenu:%d\n",e1-s1);
	LCDClear();
	ret=RunMenu(mainmenu);
	
	DestroyMenu(mainmenu);
	DestroyMenu(m1);
	if ((TESTPRIVILLEGE(PRI_OPTIONS))||(TESTPRIVILLEGE(PRI_SUPERVISOR)))
		DestroyMenu(m2);
	
	//LCDClear();
	ClockEnabled=TRUE;

	WaitAdminVerifyCount=0;
	if(ret==News_TimeOut) ExBeep(2);
	gMachineState=oldMachineState;
	FlushSensorBuffer();
	gInputNumKey = 1;
	ShowMainLCD();
	return ret;
}

int DoClearAllData(void *p)
{
	int ret;
	if((ret=LCDSelectOK(LoadStrByID(MID_CLEAR_DATA),LoadStrByID(HID_CONTINUE),LoadStrByID(HID_CONTINUEESC)))==News_CommitInput)
	{
		int j;
		LCDInfoShow(LoadStrByID(MID_CLEAR_DATA), LoadStrByID(HID_WAITING));
		j=FDB_ClearData(FCT_ALL);
		if (gOptions.I1ToG)
		{
			GroupFpCount[0] =0;
			GroupFpCount[1] =0;
			GroupFpCount[2] =0;
			GroupFpCount[3] =0;
			GroupFpCount[4] =0;
		}
		FDB_AddOPLog(ADMINPIN, OP_CLEAR_DATA, 0, j, 0, 0);
	}
	return ret;
}

int DoClearAllAdmin(void *p)
{
	int ret;
	if((ret=LCDSelectOK(LoadStrByID(MID_CLEAR_ADMIN),LoadStrByID(HID_CONTINUE),LoadStrByID(HID_CONTINUEESC)))==News_CommitInput)
	{
		int j;
		LCDInfoShow(LoadStrByID(MID_CLEAR_ADMIN), LoadStrByID(HID_WAITING));
		j=FDB_ClrAdmin();
		FDB_AddOPLog(ADMINPIN, OP_CLEAR_ADMIN, 0, j, 0, 0);
	}
	return ret;
}

int DoSetShowScore(void *p)
{
	return InputYesNoItem((PMsg)p, &gOptions.ShowScore);
}

int DoSetVoice(void *p)
{
	return InputYesNoItem((PMsg)p, &gOptions.VoiceOn);
}

int DoSetMatchScore(void *p)
{
	int ret;
	int score=CalcNewThreshold(gOptions.MThreshold);
	ret=InputValueOfItem((PMsg)p, 2, 15, 50, &score);
	gOptions.MThreshold=CalcThreshold(score);
	return ret;
}

int DoSetNoiseThreshold(void *p)
{
	return InputValueOfItem((PMsg)p, 3, 100, 150, &gOptions.MaxNoiseThr);
}

int DoSetMSpeed(void *p)
{
	return InputValueOfItem((PMsg)p, 1, 0, 2, &gOptions.MSpeed);
}

int DoSetShowState(void *p)
{
	return InputYesNoItem((PMsg)p, &gOptions.ShowState);
}

int DoSetPowerMngFunOn(void *p)
{
	return InputYesNoItem((PMsg)p, &gOptions.PowerMngFunOn);
}

int DoSetMaxFingerCount(void *p)
{
	return InputValueOfItem((PMsg)p, 2, 1, 25, &gOptions.MaxFingerCount);
}

int DoSetMaxAttLogCount(void *p)
{
	return InputValueOfItem((PMsg)p, 2, 1, 15, &gOptions.MaxAttLogCount);
}

int DoSetVerifyScore(void *p)
{	
	int ret;
	int score=CalcNewThreshold(gOptions.VThreshold);
	if(score>50) score=50; else if(score<0) score=0;
	ret=InputValueOfItem((PMsg)p, 2, 5, 50, &score);
	gOptions.VThreshold=CalcThreshold(score);
	return ret;
}
/*
int DoDevInit(void *p)
{
	char buf[20];
	int ret;
	LCD_Clear();
	LCDWriteStr(0,0,LoadStrByID(MID_OI_INITDEV),0);
	LCDWriteCenterStrID(3,HID_CONTINUEESC);
	LCDWriteCenterStrID(1,HID_CONTINUE);
	ret=InputLine(0,0,0,NULL);
	if(ret==News_CommitInput)
	{
		if(!gOptions.Saved) SaveOptions(&gOptions);
		ret=DoClearAllData(p);
		sprintf(buf, "%02d%02d%02d%02d%02d%02d%04d", (gCurTime.tm_year+1900)%100, gCurTime.tm_mon+1,
			gCurTime.tm_mday, gCurTime.tm_hour, gCurTime.tm_min, gCurTime.tm_sec,
			(unsigned short) (OSTimeGet()%10000));
			//(unsigned short)(time(NULL)%10000));//treckle
		SaveStr("~SerialNumber", buf, FALSE);
		SaveInteger("~IsInit", TRUE);
		sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", gCurTime.tm_year, gCurTime.tm_mon,
			gCurTime.tm_mday, gCurTime.tm_hour, gCurTime.tm_min, gCurTime.tm_sec);
		SaveStr("~ProductTime", buf, FALSE);
		gOptions.IsInit=TRUE;
	}
	return ret;
}
*/ //treckle
int DoRestoreDefaultOptions(void *p)
{
	int ret=LCDSelectOK(LoadStrByID(MID_OS_RESTORE),"",LoadStrByID(HID_OKCANCEL));
	if(News_CommitInput==ret)
	{
		SaveOptions(GetDefaultOptions(&gOptions));
		ClearOptionItem("NONE");
		FDB_AddOPLog(ADMINPIN, OP_RES_OPTION, 0, 0, 0, 0);
		LCDInfoShow(LoadStrByID(MID_OS_RESTORE),LoadStrByID(HID_FINISH));
		DelayMS(3*1000);
		ShowMenu((PMenu)((PMsg)p)->Object);
	}
	return ret;
}

/*
int DoDevType(void *p)
{
	char *(Items[6]);
	PMenu menu=(PMenu)((PMsg)p)->Object;
	int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret;
	int col=gLCDCharWidth-12, index; 
	Items[0]="BioClock I";
	Items[1]="BioClock II";
	Items[2]="BioClock III";
	Items[3]="F4/1";
	Items[4]="F4/2";
	Items[5]="Old I";
	index=0;
	do
	{
		ret=LCDSelectItem(row,col,12,Items,6,&index);
		if(News_CancelInput==ret || News_TimeOut==ret) return ret;
	}while(ret==News_ErrorInput);
	// News_CommitInput
	switch(index)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		gOptions.KeyLayout=KeyLayout_BioClockIII;
		gOptions.ShowState=index=2;
		break;
	case 5:
		gOptions.KeyLayout=KeyLayout_BioClockII;
		gOptions.ShowState=FALSE;
		break;
	}
	SaveStr("~DeviceName", Items[index], FALSE);
	gOptions.Saved=FALSE;
	sprintf(menu->Items[menu->ItemIndex].Caption,"%-2s%12s",LoadStrByID(HMID_DEVTYPE),Items[index]);
	LCDWriteStr(row, 2, menu->Items[menu->ItemIndex].Caption, 0);
	return ret;
	
}
int DoHideMenu(void *p)
{
	char buf[10][MAX_CHAR_WIDTH];
	int ret;
	PMenu m=CreateMenu(LoadStrByID(MID_OS_INIT),gOptions.MenuStyle,NULL);
	AddMenuItem(0, m,MenuFmtInt(buf[0],MID_OI_ENNUM, gOptions.MaxFingerCount),DoSetMaxFingerCount,NULL);
	AddMenuItem(0, m,MenuFmtInt(buf[1],MID_OI_ALNUM, gOptions.MaxAttLogCount),DoSetMaxAttLogCount,NULL);
	AddMenuItem(0, m,MenuFmtStr(buf[7],MID_OI_NET,GetYesNoName(gOptions.NetworkFunOn)),DoSetShowState,NULL);
	AddMenuItem(0, m,MenuFmtStr(buf[8],MID_OI_POWERMNG,GetYesNoName(gOptions.PowerMngFunOn)),DoSetPowerMngFunOn,NULL);
	AddMenuItem(1,m,LoadStrByID(HMID_DEVTYPE),DoDevType,NULL);
	AddMenuItem(1,m,LoadStrByID(MID_OI_INITDEV),DoDevInit,NULL);
	
	ret=RunMenu(m);
	DestroyMenu(m);
	if(!gOptions.Saved && ret!=News_TimeOut) SaveOptions(&gOptions); else LoadOptions(&gOptions);
	return ret;
}
*/ //treckle
int DoSetCardKey(void *p)
{
	int ret;
	int value, OptionValue;
	char buffer[12];	
	if(0!=strtou32((char*)GetCardKeyStr((BYTE*)buffer, gOptions.CardKey), (U32*)&value)) value=0;
	OptionValue=value;
	LCD_Clear();
	LCDWriteLineStrID(0,MID_OA_FPKEY);
	LCDWriteCenterStrID(3,HID_OKCANCEL);
	do
	{
		ret=InputNumber(2,5,6,&value,0,999999,FALSE);
		if(News_CancelInput==ret || News_TimeOut==ret) return ret;
	}while(ret==News_ErrorInput);
	// News_CommitInput
	if(value!=OptionValue)
	{
		sprintf(buffer,"%d",value);
		memset(gOptions.CardKey,0xff,6);
		if(value)
			strncpy((char*)gOptions.CardKey,buffer,6);
		gOptions.Saved=FALSE;
	}
	return ret;
}

int DoSet1To1(void *p)
{
	return InputYesNoItem((PMsg)p, &gOptions.Must1To1);
}

int DoSetFPVer(void *p)
{
	PMsg pmsg = (PMsg)p;
	char Items[MAX_CHAR_WIDTH];
	PMenu menu=((PMenu)pmsg->Object);
	int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret;
	int col=gLCDCharWidth-3, index, oldindex, values[]={0,1}; 
	sprintf(Items, "%s:%s", "9", "10");
	if (gtmpver == ZKFPV10)
		index = oldindex = 1;
	else
		index = oldindex = 0;	
	ret=LCDSelectItemValue(row,col,3,Items,values,&index);
	if(ret==News_CommitInput)
	{
		if(index!=oldindex)
		{
			gtmpver = (index==1)?10:9;
			gOptions.Saved=FALSE;
			MenuFmtStrStr(menu->Items[menu->ItemIndex].Caption, 3, index==1?"10":"9");
		}
	}
	return ret;
}
/*
int DoSetTwoSensor(void *p)
{
	int ret;
	gOptions.IsOnlyOneSensor=!gOptions.IsOnlyOneSensor;
	ret=InputYesNoItem((PMsg)p, &gOptions.IsOnlyOneSensor);   
	gOptions.IsOnlyOneSensor=!gOptions.IsOnlyOneSensor;	
	return ret;
}
*/ //treckle
static char *AuthNames[]={"LO","NL", "NO", "LN"};
char *GetAutoServerMethod(int AuthServerEnabled)
{
	return AuthNames[AuthServerEnabled];
}

int DoSetAuthServer(void *p)
{
#if 0 //treckle
	char Items[32];
	int Values[]={ONLY_LOCAL,NETWORK_LOCAL,ONLY_NETWORK,LOCAL_NETWORK};
	PMenu menu=((PMenu)((PMsg)p)->Object);
	int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret;
	int col=gLCDCharWidth-3, index; 
	sprintf(Items, "%s:%s:%s:%s", AuthNames[ONLY_LOCAL], AuthNames[NETWORK_LOCAL],AuthNames[ONLY_NETWORK], AuthNames[LOCAL_NETWORK]);
	index=gOptions.AuthServerEnabled;
	ret=LCDSelectItemValue(row,col,3,Items,Values,&index);
	if(ret==News_CommitInput)
	{
		if(index!=gOptions.AuthServerEnabled)
		{
			sprintf(menu->Items[menu->ItemIndex].Caption+gLCDCharWidth-3-MenuIndicatorWidth,"%6s",GetAutoServerMethod(index));
			gOptions.AuthServerEnabled=index;
			gOptions.Saved=FALSE;
		}	
	}
	return ret;	
#endif
	return 0;
}

int DoSetAuthServerIPAddress(void *p)
{
#if 0 //treckle
	int ii,ret,ipa[4];
	ipa[0]=gOptions.AuthServerIP[0];
	ipa[1]=gOptions.AuthServerIP[1];
	ipa[2]=gOptions.AuthServerIP[2];
	ipa[3]=gOptions.AuthServerIP[3];
	ret=InputIPAddress(LoadStrByID(MID_AUTHSERVER_IP), ipa);
	for(ii=0;ii<4;ii++)
		if(ipa[ii]!=gOptions.AuthServerIP[ii])
		{
		      if(gOptions.Saved) gOptions.Saved=FALSE;
		      gOptions.AuthServerIP[ii]=ipa[ii];
	       }
	return ret;
#endif
	return 0;
}

int DoSetWorkCode(void *p)
{
	return InputYesNoItem((PMsg)p, &gOptions.WorkCode);
}

int DoSetKeyPadBeep(void *p)
{
	gtmpkbeep = gOptions.KeyPadBeep;
	return InputYesNoItem((PMsg)p, &gtmpkbeep);	
}

int DoSetAudioVol(void *p)
{
	return InputValueOfItem((PMsg)p, 2, 0, 99, &gOptions.AudioVol);
//	return InputValueOfItem((PMsg)p, 1, 0, 4, &gOptions.AudioVol);	//treckle
}

int DoSet1ToNFrom(void *p)
{
        int ret=InputValueOfItem((PMsg)p, 5, 0, 0xFFFE, &gOptions.I1ToNFrom);
        return ret;
}

int DoSet1ToNTo(void *p)
{
        int ret=InputValueOfItem((PMsg)p, 5, 0, 99999, &gOptions.I1ToNTo);
        return ret;
}

int DoSet1ToH(void *p)
{
        int ret=InputYesNoItem((PMsg)p, &gOptions.I1ToH);
        return ret;
}

int DoSet1ToG(void *p)
{
        int ret=InputYesNoItem((PMsg)p, &gOptions.I1ToG);
        return ret;
}

int GetModeName(char *name,int modenames, int Mode)
{
        char *p=LoadStrByID(modenames);
        if(p)
        {
                return SCopyStrFrom(name,p,Mode);
        }
        return 0;
}

int DoSetPrinterMode(void *p)
{
        PMenu menu=((PMenu)((PMsg)p)->Object);
        int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret;
        int col=gLCDCharWidth-8, index=gOptions.PrinterOn;
        char name[10];
        int values[11]={0,1,2,3,4,5,6,7,8,9,10};
        ret=LCDSelectItemValue(row, col, 8, LoadStrByID(MID_PRINTERMODE), values, &index);
        if(ret==News_CommitInput)
        if(index!=gOptions.PrinterOn)
        {
                char format[40];
                sprintf(format, "%%-%ds%%4s", gLCDCharWidth-4-MenuIndicatorWidth);
                GetModeName(name,MID_PRINTERMODE, index);
                MenuFmtStr(menu->Items[menu->ItemIndex].Caption,MID_PRINTERON,name);
                gOptions.PrinterOn=index;
                gOptions.Saved=FALSE;
                //ShowMenu(menu);
        }
        return ret;
}

int DoSetWorkCodeMode(void *p)
{
        PMenu menu=((PMenu)((PMsg)p)->Object);
        int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret;
        int col=gLCDCharWidth-8, index=gOptions.WorkCode;
        char name[10];
        int values[11]={0,1,2,3,4,5,6,7,8,9,10};
        ret=LCDSelectItemValue(row, col, 8, LoadStrByID(MID_WORKCODEMODE), values, &index);
        if(ret==News_CommitInput)
        if(index!=gOptions.WorkCode)
        {
                char format[40];
                sprintf(format, "%%-%ds%%4s", gLCDCharWidth-4-MenuIndicatorWidth);
                GetModeName(name,MID_WORKCODEMODE, index);
                MenuFmtStr(menu->Items[menu->ItemIndex].Caption,MID_WORKCODE,name);
                gOptions.WorkCode=index;
                gOptions.Saved=FALSE;
                //ShowMenu(menu);
        }
        return ret;
}

#define FPSensitivityStep 30

int DoSetFPSensitivity(void *p)
{
	int ret, i=(gOptions.TopThr+FPSensitivityStep/2)/FPSensitivityStep;
	i=11-i;
	ret=InputValueOfItem((PMsg)p, 3, 1, 10, &i);
	if(ret==News_CommitInput)
	{
		i=11-i;
		gOptions.TopThr=i*FPSensitivityStep;
		gOptions.IncThr=(15*gOptions.TopThr+25)/50;
		if(gOptions.MinThr>gOptions.TopThr)
			gOptions.MinThr=gOptions.TopThr*2/3;
	}
}

static char must1to1;
int DoAdvanceMenu(void *p)
{
	char buf[20][MAX_CHAR_WIDTH];
	int ret;
	static PMenu m=NULL;
	m=CreateMenu(LoadStrByID(MID_OS_ADVANCE),gOptions.MenuStyle,NULL, m);
	AddMenuItem(1,m,LoadStrByID(MID_OS_RESTORE),DoRestoreDefaultOptions,NULL);
	AddMenuItem(1,m,LoadStrByID(MID_DATA_DELLOG),DoClearLog, NULL);
	AddMenuItem(1,m,LoadStrByID(MID_CLEAR_DATA),DoClearAllData,NULL);
	AddMenuItem(1,m,LoadStrByID(MID_CLEAR_ADMIN),DoClearAllAdmin,NULL);	
#if 0
	if (!gOptions.IsOnlyRFMachine)
	{
		AddMenuItem(1,m,MenuFmtStr(buf[0],HMID_SHOWSCORE,GetYesNoName(gOptions.ShowScore)),DoSetShowScore,NULL);
		AddMenuItem(1,m,MenuFmtInt(buf[1],HMID_MATCHSCORE,CalcNewThreshold(gOptions.MThreshold)),DoSetMatchScore,NULL);
		if(!gOptions.UserExtendFormat)
			AddMenuItem(1,m,MenuFmtStr(buf[2],MID_OS_MUST1TO1,GetYesNoName(gOptions.Must1To1)),DoSet1To1,NULL);
		AddMenuItem(1,m,MenuFmtInt(buf[3],MID_OS_VERSCORE,CalcNewThreshold(gOptions.VThreshold)),DoSetVerifyScore,NULL);
		//Two Sensor
/*		if(gOptions.IsSupportUSBDisk && gOptions.OutSensorFunOn)
			AddMenuItem(0,m,MenuFmtStr(buf[4],MID_TWOSENSOR,GetYesNoName(!gOptions.IsOnlyOneSensor)),DoSetTwoSensor,NULL); */ //treckle
	}
	AddMenuItem(1,m,MenuFmtStr(buf[5],HMID_VOICEON,GetYesNoName(gOptions.VoiceOn)),DoSetVoice,NULL);
	if((gOptions.RFCardFunOn||gMFOpened)&&(!gOptions.IsOnlyRFMachine))
	{
		
		if(!gOptions.UserExtendFormat)
			AddMenuItem(1,m,MenuFmtStr(buf[6],MID_OC_PINCARD,GetYesNoName(gOptions.OnlyPINCard)), DoSetOnlyPINCard, NULL);
	}
	//Now it don't support
	//if(gMFOpened&&!gOptions.MifareAsIDCard)
	//	AddMenuItem(0,m,MenuFmtStr(buf[7],MID_OC_MUSTENROLL,GetYesNoName(gOptions.MustEnroll)), DoSetMustEnroll, NULL);
/*	if(gMFOpened&&gOptions.CanChangeCardKey&&!gOptions.MifareAsIDCard)
		AddMenuItem(1,m,LoadStrByID(MID_OA_FPKEY),DoSetCardKey,NULL);
	if(gOptions.IsSupportUSBDisk)
		AddMenuItem(1,m,LoadStrByID(MID_UPDATE_FIRMWARE),DoUpdateFirmware,NULL);	
	if(gOptions.IsSupportAuthServer)
	{
		AddMenuItem(0,m,MenuFmtStr(buf[8],MID_AUTHSERVER,GetAutoServerMethod(gOptions.AuthServerEnabled)),DoSetAuthServer,NULL);
		AddMenuItem(1, m, LoadStrByID(MID_AUTHSERVER_IP), DoSetAuthServerIPAddress, NULL);
	} */ //treckle
	if(gOptions.WorkCodeFunOn)
	{
                char name1[10];
                GetModeName(name1,MID_WORKCODEMODE,gOptions.WorkCode);
                AddMenuItem(1,m, MenuFmtStr(buf[9],MID_WORKCODE,name1), DoSetWorkCodeMode, NULL);
		//AddMenuItem(0,m,MenuFmtStr(buf[9],MID_WORKCODE,GetYesNoName(gOptions.WorkCode)),DoSetWorkCode,NULL);
	}
	AddMenuItem(1,m,MenuFmtStr(buf[10],MID_BUTTONBEEP,GetYesNoName(gOptions.KeyPadBeep)),DoSetKeyPadBeep,NULL);	
	if(gOptions.PlayTZVoice)
		AddMenuItem(1,m,LoadStrByID(MID_ADV_VOICETZ),DoDefVerOKVoice,NULL);
	AddMenuItem(1,m,MenuFmtInt(buf[11],MID_ADV_AUDIOVOL,gOptions.AudioVol),DoSetAudioVol,NULL);	
	if(gOptions.AdvanceMatch)
	{
		//AddMenuItem(0,m,MenuFmtInt(buf[12],MID_OS_1TON_FROM,gOptions.I1ToNFrom),DoSet1ToNFrom,NULL);
		//AddMenuItem(0,m,MenuFmtInt(buf[13],MID_OS_1TON_TO,gOptions.I1ToNTo),DoSet1ToNTo,NULL);
		//AddMenuItem(0,m,MenuFmtStr(buf[14],MID_OS_1TOH,GetYesNoName(gOptions.I1ToH)),DoSet1ToH,NULL);
		//if(gOptions.LockFunOn&LOCKFUN_ADV)//具有高级门禁功能才能有分组支持
		if(gOptions.LockFunOn)//具有门禁功能才能有分组支持
			AddMenuItem(1,m,MenuFmtStr(buf[15],MID_OS_1TOG,GetYesNoName(gOptions.I1ToG)),DoSet1ToG,NULL);
		if (gOptions.I1ToG)
			AddMenuItem(1,m,MenuFmtInt(buf[16],MID_DEFAULTGROUP,(gOptions.DefaultGroup)),DoSetDefaultGroup,NULL);
			
			
	}
        if(gOptions.PrinterFunOn)
        {
                char name[10];
                GetModeName(name,MID_PRINTERMODE,gOptions.PrinterOn);
                AddMenuItem(1,m, MenuFmtStr(buf[16],MID_PRINTERON,name), DoSetPrinterMode, NULL);
        }
        //2006.11.28
        if (LoadInteger(CustomVoice, 0))
        {
	        AddMenuItem(1, m, LoadStrByID(MID_RESTORE_VOICE), DoRestoreVoice, NULL);

        }
#ifndef URU 
	if (LoadInteger(FPSENSITIFUNON,0))
	{
		AddMenuItem(1, m, MenuFmtInt(buf[17],MID_AO_FPS, 11-(gOptions.TopThr+FPSensitivityStep/2)/FPSensitivityStep), DoSetFPSensitivity, NULL);
	}
#endif
	gtmpver = gOptions.ZKFPVersion;
	must1to1 = gOptions.Must1To1;
	if (gOptions.MulAlgVer)
		AddMenuItem(1,m,MenuFmtInt(buf[18],MID_OI_ALGVER, gOptions.ZKFPVersion),DoSetFPVer, NULL);
#endif
	ret=RunMenu(m);
	DestroyMenu(m);
	if(!gOptions.Saved && ret!=News_TimeOut)
	{
		if (gtmpver!=gOptions.ZKFPVersion)
			ret=LCDSelectOK(LoadStrByID(MID_OPTIONS_SYSTEM),LoadStrByID(HID_CHRVER_DELALL), LoadStrByID(HID_SAVECANCEL));
		else
			ret=LCDSelectOK(LoadStrByID(MID_OPTIONS_SYSTEM),LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret) 
		{
			gOptions.KeyPadBeep = gtmpkbeep;
			SaveOptions(&gOptions); 
			if (gtmpver!=gOptions.ZKFPVersion)
			{
				SaveInteger("ChangeVersion", gtmpver);
				FDB_ClearData(FCT_ALL);
				LCDInfoShow(LoadStrByID(MID_OI_ALGVER),LoadStrByID(HID_AUTORESTART));
				DelayMS(2*1000);
				wdt_initialize(0);
				DelayMS(5*1000);
			}
			if (must1to1==1 && gOptions.Must1To1==0)
			{
				FPDBInit();
			}
		}
		else 
			LoadOptions(&gOptions);
	}
	return ret;
}

int DoSetNEWFPR(void *p)
{
	return InputYesNoItem((PMsg)p, &gOptions.NewFPReader);
}

int DoHideAdvMenu(void *p)
{
	char buf[10][MAX_CHAR_WIDTH];
	int ret;
	PMenu m=NULL;
	m=CreateMenu(LoadStrByID(MID_OS_ADVANCE),gOptions.MenuStyle,NULL, m);
	AddMenuItem(1,m,MenuFmtStr(buf[4],HMID_NEWFPR,GetYesNoName(gOptions.NewFPReader)),DoSetNEWFPR,NULL);
	AddMenuItem(1,m,MenuFmtInt(buf[5],HMID_MATCHSPEED,gOptions.MSpeed),DoSetMSpeed,NULL);
	ret=RunMenu(m);
	DestroyMenu(m);
	if(!gOptions.Saved && ret!=News_TimeOut)
	{
		ret=LCDSelectOK(LoadStrByID(MID_OPTIONS_SYSTEM),LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret) 
			SaveOptions(&gOptions); 
		else 
			LoadOptions(&gOptions);
	}
	return ret;
}

int DoShowGroupFpInfo(void *p)
{
        int ret;
        char buf[5][MAX_CHAR_WIDTH];
        PMenu m=NULL;
	m=CreateMenu(LoadStrByID(MID_GROUPFPINFO),gOptions.MenuStyle,NULL, m);
        AddMenuItem(1,m,MenuFmtInt(buf[0],MID_ONEGROUPINFO,GroupFpCount[0]),NULL,NULL);
        AddMenuItem(1,m,MenuFmtInt(buf[1],MID_TWOGROUPINFO,GroupFpCount[1]),NULL,NULL);
        AddMenuItem(1,m,MenuFmtInt(buf[2],MID_THREEGROUPINFO,GroupFpCount[2]),NULL,NULL);
        AddMenuItem(1,m,MenuFmtInt(buf[3],MID_FOURGROUPINFO,GroupFpCount[3]),NULL,NULL);
        AddMenuItem(1,m,MenuFmtInt(buf[4],MID_FIVEGROUPINFO,GroupFpCount[4]),NULL,NULL);
        ret=RunMenu(m);
        DestroyMenu(m);
        return ret;
}

int DoSetDefaultGroup(void *p)
{
	return InputValueOfItem((PMsg)p, 1, 1, 5, &gOptions.DefaultGroup);
	
}
