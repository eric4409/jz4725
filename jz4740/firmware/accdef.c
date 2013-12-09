/*************************************************
                                           
 ZEM 200                                          
                                                    
 accdef.c define all the access control functions  
 
 关于门禁控制的各种设置与定义功能
                                                      
 Copyright (C) 2003-2005, ZKSoftware Inc.  
 
 $Log: accdef.c,v $
 Revision 5.7  2006/03/04 17:30:09  david
 Add multi-language function

 Revision 5.6  2005/11/06 02:41:34  david
 Fixed RTC Bug(Synchronize time per hour)

 Revision 5.5  2005/08/07 08:13:15  david
 Modfiy Red&Green LED and Beep

 Revision 5.4  2005/08/02 16:07:51  david
 Add Mifare function&Duress function
                                                     
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
//#include "net.h"
#include "utils.h"
#include "options.h"
#include "autotest.h"
#include "mainmenu.h"
#include "commu.h"
#include "accdef.h"
#include "lcm.h"

static int ValueModified=FALSE;

int InputTimeInterval(int row, int col, int *times)
{
	int ii=0, ret;
	
	ret=News_ErrorInput;
	do
	{
		char buf[MAX_CHAR_WIDTH];
		sprintf(buf, "%02d:%02d-%02d:%02d",times[0],times[1],times[2],times[3]);
		LCDWriteStr(row, col, buf, 0);
		switch(ii)
		{
		case 0:
			ret=InputNumberAt(row,col,2,times,0,23); break;
		case 1:
			ret=InputNumberAt(row,col+3,2,times+1,0,59); break;
		case 2:
			ret=InputNumberAt(row,col+6,2,times+2,0,23); break;
		case 3:
			ret=InputNumberAt(row,col+9,2,times+3,0,59); break;
		}
		if(ret==News_ErrorInput) continue;
		if(News_CancelInput==ret || News_TimeOut==ret) return ret;
		/*
		if(ret==News_NextInput)
		{
			if(++ii>3) ii=0;
		}
		else if(ret==News_PrevInput)
		{
			if(--ii<0) ii=1;
		}
		*/
		if(ret==News_CommitInput)
		{
			ii += 1;
		}
	}while(ii < 4);
//	}while(ret!=News_CommitInput);
	return ret;
}

static PTimeZone CTZ;

int DoSetInterval(void *p)
{
	PMenu menu=((PMenu)((PMsg)p)->Object);
	int ret, row=CalcMenuItemOffset(menu, menu->ItemIndex);
	int col=gLCDCharWidth-11, times[4], i=menu->ItemIndex;
	times[0]=CTZ->intervals[i][TZ_START][TZ_HOUR]; 
	times[1]=CTZ->intervals[i][TZ_START][TZ_MINUTE];
	times[2]=CTZ->intervals[i][TZ_END][TZ_HOUR];
	times[3]=CTZ->intervals[i][TZ_END][TZ_MINUTE];
	if(News_CommitInput==(ret=InputTimeInterval(row, col, times)))
	{
		if(CTZ->intervals[i][TZ_START][TZ_HOUR]	!=times[0])
		{
			CTZ->intervals[i][TZ_START][TZ_HOUR]=times[0];
			ValueModified=TRUE;
		}
		if(CTZ->intervals[i][TZ_START][TZ_MINUTE]!=times[1])
		{
			CTZ->intervals[i][TZ_START][TZ_MINUTE]=times[1];
			ValueModified=TRUE;
		}
		if(CTZ->intervals[i][TZ_END][TZ_HOUR]!=times[2])
		{
			CTZ->intervals[i][TZ_END][TZ_HOUR]	=times[2];
			ValueModified=TRUE;
		}
		if(CTZ->intervals[i][TZ_END][TZ_MINUTE]	!=times[3])
		{
			CTZ->intervals[i][TZ_END][TZ_MINUTE]=times[3];
			ValueModified=TRUE;
		}
		if(ValueModified)
		{

			char format[40];
			sprintf(format, "%%02d:%%02d-%%02d:%%02d");
                        sprintf(menu->Items[menu->ItemIndex].Caption+gLCDCharWidth-11-MenuIndicatorWidth,format,
                                CTZ->intervals[i][TZ_START][TZ_HOUR], CTZ->intervals[i][TZ_START][TZ_MINUTE],
                                CTZ->intervals[i][TZ_END][TZ_HOUR], CTZ->intervals[i][TZ_END][TZ_MINUTE]); 			
		}
	}
	return ret;
}

int DoDefTZ(void *p)
{
	int ret, i;
	int TZID;
	char cbuf[8][MAX_CHAR_WIDTH], *s, buf[MAX_CHAR_WIDTH];
	static PMenu m21=NULL;
	TTimeZone TZ;
	LCD_Clear();
	LCDWriteLineStrID(0,MID_OA_TZDEF);
	LCDWriteCenterStrID(3, HID_OKCANCEL);
	LCDWriteCenterStrID(1,HID_TZNUM);
	TZID=1;
	if(gLCDRowCount>2)
	{
		LCDWriteCenterStrID(1,HID_TZNUM);
		do
		{
			ret=InputNumber(2,(gLCDCharWidth-2)/2,2,&TZID, 1, TZ_MAX, FALSE);
			if(ret!=News_CommitInput) return ret;
		}while(!((TZID>0) && (TZID<TZ_MAX+1)));
	}
	else
	{
		LCDWriteLineStrID(1,HID_TZNUM);
		do
		{
			ret=InputNumber(1,gLCDCharWidth-3,2,&TZID, 1, TZ_MAX, FALSE);
			if(ret!=News_CommitInput) return ret;
		}while(!((TZID>0) && (TZID<TZ_MAX+1)));
	}	
	sprintf(cbuf[0], LoadStrByID(HID_TZDEF), TZID);
	m21=CreateMenu(cbuf[0],gOptions.MenuStyle,NULL, m21);
	s=LoadStrByID(HID_SHORTWEEK);
	CTZ=&TZ;
	ValueModified=FALSE;
	LoadTimeZone(TZID, CTZ);
//2006.10.31
/*
	for(i=0;i<7;i++)
	{
		char format[40];
		sprintf(format, "%%-%ds%%02d:%%02d-%%02d:%%02d", gLCDCharWidth-11-MenuIndicatorWidth);
		SCopyStrFrom(buf, s, i);
		sprintf(cbuf[i+1],format, buf, 
			TZ.intervals[i][TZ_START][TZ_HOUR], TZ.intervals[i][TZ_START][TZ_MINUTE],
			TZ.intervals[i][TZ_END][TZ_HOUR], TZ.intervals[i][TZ_END][TZ_MINUTE]);
		AddMenuItem(0, m21, cbuf[i+1], DoSetInterval, NULL);
	}	
*/
        for(i=0;i<7;i++)
        {
                char timestr[MAX_CHAR_WIDTH];  
                sprintf(timestr, "%%-%ds%%02d:%%02d-%%02d:%%02d", gLCDCharWidth-11-MenuIndicatorWidth);
                SCopyStrFrom(buf, s, i);
                sprintf(timestr,"%02d:%02d-%02d:%02d",TZ.intervals[i][TZ_START][TZ_HOUR], TZ.intervals[i][TZ_START][TZ_MINUTE],
                TZ.intervals[i][TZ_END][TZ_HOUR], TZ.intervals[i][TZ_END][TZ_MINUTE]);
                PadRightStrStr(cbuf[i+1], buf, timestr, MenuCharWidth);
		AddMenuItem(1, m21, cbuf[i+1], DoSetInterval, NULL);
        }

	ret=RunMenu(m21);
	DestroyMenu(m21);
	if(ValueModified && ret!=News_TimeOut)
	{
		ret=LCDSelectOK(cbuf[0],LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret) 
		{

			int j=SaveTimeZone(TZID, CTZ);
			FDB_AddOPLog(ADMINPIN, OP_ACC_TZ, TZID, j, 0, 0);
		}
		//flush the cached data to disk
		sync();
	}
	gOptions.Saved=TRUE;
	return ret;	
}

extern char cbuf[10][MAX_CHAR_WIDTH];

int IsValidGrps(int i)
{
	char buf[MAX_CHAR_WIDTH];
	sprintf(buf,"%d", i);
	for(i=0;i<20;i++)
	{
		if(buf[i]==0) break;
		if((buf[i]=='0') || (buf[i]>'5')) return FALSE;
	}
	return TRUE;
}

int DoSetULGrp(void *p)
{
	int ret, i, v;
	PMenu menu=((PMenu)((PMsg)p)->Object);
	v=StrValue(cbuf[menu->ItemIndex], NULL);
	i=v;
	while(1)
	{
		ret=InputValueOfItem((PMsg)p, 5, 0, 99999, &i);
		if(ret!=News_CommitInput) break;
		if(i==0) 
		{
			sprintf(menu->Items[menu->ItemIndex].Caption+gLCDCharWidth-5-MenuIndicatorWidth,"%5s", " ");
			break;
		}
		else if(IsValidGrps(i))
			break;
	}
	if((ret==News_CommitInput) && (i!=v))
	{
		if(i==0)
			cbuf[menu->ItemIndex][0]=0;
		else
			sprintf(cbuf[menu->ItemIndex], "%d",i);
		ValueModified=TRUE;
	}
	return ret;
}

int DoULGrp(void *p)
{
	char buf[10][MAX_CHAR_WIDTH], name[200], s[1024];
	int i, ret;
	char format[20];
	PMenu m21=NULL;
	m21=CreateMenu(LoadStrByID(MID_OA_ULGRP),gOptions.MenuStyle,NULL,m21);
	sprintf(format, "%%-%ds%%5s", gLCDCharWidth-5-MenuIndicatorWidth);	
	LoadStr("ULG", s);
	for(i=0;i<10;i++)
	{
		sprintf(name, LoadStrByID(HID_ULG), i+1);
		cbuf[i][0]=0;
		if(s)
			SCopyStrFrom(cbuf[i], s, i);
		else
			cbuf[i][0]=0;
		cbuf[i][5]=0;
		if(cbuf[i][0]=='0') cbuf[i][0]=0;
		sprintf(buf[i], format, name, cbuf[i]);
		AddMenuItem(1, m21, buf[i], DoSetULGrp, NULL);
	}
	ValueModified=FALSE;
	ret=RunMenu(m21);
	DestroyMenu(m21);
	if(ValueModified && ret!=News_TimeOut)
	{
		ret=LCDSelectOK(LoadStrByID(MID_OA_ULGRP),LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret)
		{
			int j;
			sprintf(name, "%s:%s:%s:%s:%s:%s:%s:%s:%s:%s", cbuf[0], cbuf[1], cbuf[2], cbuf[3],
				cbuf[4], cbuf[5], cbuf[6], cbuf[7], cbuf[8], cbuf[9]);
			SPackStr(name);
			j=(int)SaveStr("ULG", name, TRUE);
			FDB_AddOPLog(ADMINPIN, OP_ACC, 0, j, 0, 0);
			ValueModified=FALSE;
		}
	}
	gOptions.Saved=TRUE;	
	return ret;	
}

typedef struct _TUTZDEF_{
	U16 UserID;
	int UserGrp;
	int TZs[10];
	int TZCount;
	int UseGrpTZ;
	BYTE VS;
	int UserGrpVS;
} TUTZDEF, *PUTZDEF;

PUTZDEF putz;

void UpdateUZTs(void)
{
	int i;
	char format[MAX_CHAR_WIDTH];
	sprintf(format, "%%-%ds%%3s", gLCDCharWidth-3-MenuIndicatorWidth);
	sprintf(cbuf[1], format, LoadStrByID(MID_OA_UDT), GetYesNoName(putz->UseGrpTZ));
	for(i=0;i<3;i++)
	{
		char buf[200];
		sprintf(buf, LoadStrByID(HID_TZI),i+1); 
		if(putz->TZs[i]==0)
		{
			sprintf(format, "%%-%ds", gLCDCharWidth-MenuIndicatorWidth);
			sprintf(cbuf[2+i], "%-14s", buf);
		}
		else
		{
			sprintf(format, "%%-%ds%%2d", gLCDCharWidth-2-MenuIndicatorWidth);
			sprintf(cbuf[2+i], format, buf, putz->TZs[i]);
		}
	}
}

int DoSetUDTZ(void *p)
{
	int udtz=putz->UseGrpTZ;
	int ret=InputYesNoItem((PMsg)p, &udtz);
	if(ret==News_CommitInput){
		if(udtz^(putz->UseGrpTZ))
		{
			ValueModified=TRUE;
			putz->UseGrpTZ=udtz;
			if(udtz)
				putz->TZCount=GetGrpTZ(putz->UserGrp, putz->TZs);
			UpdateUZTs();
		}
	}
	return ret;
}

extern int GroupFpCount[5];
int DoSetUGrp(void *p)
{
	int ret, grp=putz->UserGrp;
        char value[20];
        PMsg msg=p;
	PMenu menu=((PMenu)msg->Object);
	ret=InputValueOfItem((PMsg)p, 1, 1, 5, &grp);
	if(ret!=News_CommitInput) return ret;
	if(putz->UserGrp==grp) return ret;
	//支持分组比对时， 如果该组用户已满应不再接受用户
	if (gOptions.I1ToG)
	{
		if ((GroupFpCount[grp-1]+FDB_GetTmpCnt(putz->UserID)) > gOptions.GroupFpLimit)
			{
				grp=putz->UserGrp;
				LCD_Clear();
				LCDWriteCenterStrID(gLCDRowCount/2, MID_GROUPFPREACH);
				DelayMS(1*1000);
	                        sprintf(value, "%d", grp);
        	                PadRightStrStr(menu->Items[menu->ItemIndex].Caption, menu->Items[menu->ItemIndex].Caption, value, MenuCharWidth);
				return ret;
			}	
	}
	ValueModified=TRUE;
	putz->UserGrp=grp;
	if(putz->UseGrpTZ)
	{
		putz->TZCount=GetGrpTZ(grp, putz->TZs);
		UpdateUZTs();
	}
	return ret;
}

int DoSetUTZ(void *p)
{
	PMenu menu=((PMenu)((PMsg)p)->Object);
	int value, ret;
	value=putz->TZs[menu->ItemIndex-2];
	ret=InputValueOfItem((PMsg)p, 2, 0, TZ_MAX, &value);
	if(ret!=News_CommitInput) return ret;
	if(putz->TZs[menu->ItemIndex-2]==value) return ret;
	ValueModified=TRUE;
	putz->TZs[menu->ItemIndex-2]=value;
	putz->UseGrpTZ=FALSE;
	UpdateUZTs();	
	return ret;	
}


static char *VerifyStyle[]={"FP/PW/RF", "FP","PIN", "PW", "RF", "FP/PW", "FP/RF", "PW/RF",
			    "PIN&FP", "FP&PW", "FP&RF", "PW&RF", "FP&PW&RF", "PIN&FP&PW", "FP&RF/PIN"};

BYTE GetExtUserByVS(U16 pin)
{
	PExtUser u=FDB_GetExtUser(pin, NULL);
	if(u)
		return u->VerifyStyle;
	else
	{
		if(gOptions.Must1To1)
			return VS_PIN_AND_FP;
		else
			return VS_FP_OR_PW_OR_RF;
	}
}

char *GetVSName(int vs)
{
	if(vs==VS_FP_OR_PW_OR_RF) 
		return VerifyStyle[0];
	else if(vs==VS_FP)
		return VerifyStyle[1];		
	else if(vs==VS_PIN)
		return VerifyStyle[2];		
	else if(vs==VS_PW)
		return VerifyStyle[3];
	else if(vs==VS_RF)
		return VerifyStyle[4];
	else if(vs==VS_FP_OR_PW)
		return VerifyStyle[5];
	else if(vs==VS_FP_OR_RF)
		return VerifyStyle[6];
	else if(vs==VS_PW_OR_RF)
		return VerifyStyle[7];
	else if(vs==VS_PIN_AND_FP)
		return VerifyStyle[8];
	else if(vs==VS_FP_AND_PW)
		return VerifyStyle[9];
	else if(vs==VS_FP_AND_RF)
		return VerifyStyle[10];
	else if(vs==VS_PW_AND_RF)
		return VerifyStyle[11];
	else if(vs==VS_FP_AND_PW_AND_RF)
		return VerifyStyle[12];
	else if(vs==VS_PIN_AND_FP_AND_PW)
		return VerifyStyle[13];
	else if(vs==VS_FP_AND_RF_OR_PIN)
		return VerifyStyle[14];
	else
		return "UNKNOWN";
}

BYTE GVS_Value[20];
		
int DoSetVerifyStyle(void *p)
{
	char Items[VS_NUM*24], tmp[24];
	int Values[]={VS_FP_OR_PW_OR_RF, 
		      VS_FP, 
		      VS_PIN,
		      VS_PW,
		      VS_RF,
		      VS_FP_OR_PW,
		      VS_FP_OR_RF,
		      VS_PW_OR_RF,
		      VS_PIN_AND_FP,
		      VS_FP_AND_PW,
		      VS_FP_AND_RF,
		      VS_PW_AND_RF,
		      VS_FP_AND_PW_AND_RF,
		      VS_PIN_AND_FP_AND_PW,
		      VS_FP_AND_RF_OR_PIN
	      };
	PMenu menu=((PMenu)((PMsg)p)->Object);
	int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret;
	int col=gLCDCharWidth-9, index, i, itemindex=menu->ItemIndex; 
	
	memset(Items, 0, VS_NUM*24);
	for(i=0;i<VS_NUM;i++)
	{
		if(i!=0) strcat(Items, ":");
		sprintf(tmp, "%s", VerifyStyle[i]);
		strcat(Items, tmp);
	}
	//Group or User
	if(putz->UserID==0) 
		putz->VS=GVS_Value[itemindex];
	index=putz->VS;
	ret=LCDSelectItemValue(row, col, 9, Items, Values, &index);
	if(ret==News_CommitInput)
	{
		if(index!=putz->VS)
		{
			sprintf(menu->Items[menu->ItemIndex].Caption+gLCDCharWidth-9-MenuIndicatorWidth,"%9s",GetVSName(index));
			ValueModified=TRUE;
			putz->VS=index;
			//Group or User
			if(putz->UserID==0) 
				GVS_Value[itemindex]=putz->VS;
		}
	}
	return ret;	
}

void SaveGroupVS(char *Prefix, BYTE *Value, int Count)
{
	int i;
	char tmp[24];
	
	for(i=0;i<Count;i++)
	{
		sprintf(tmp, "%s%d", Prefix, i+1);
		if(Value[i]!=0xFF)
			SaveInteger(tmp, Value[i]); 
	}
}

int DoSetGroupVerifyStyle(void *p)
{
	char buf[5][MAX_CHAR_WIDTH];
	static PMenu m21=NULL;
	int i, ret;
	char tmp[16];
	BYTE GroupIndex;
	TUTZDEF NewValue;
	
	putz=&NewValue;
	NewValue.UserID=0;

	memset(GVS_Value, 0xFF, 20);
	m21=CreateMenu(LoadStrByID(MID_OA_GVERIFYTYPE), gOptions.MenuStyle, NULL, m21);
	for(i=1;i<=5;i++)
	{
		sprintf(tmp, "GVS%d", i);
		GroupIndex=LoadInteger(tmp, VS_FP_OR_PW_OR_RF);
		GVS_Value[i-1]=GroupIndex;
		sprintf(buf[i-1], "%d", i);
		AddMenuItem(1, m21, MenuFmtStrStr(buf[i-1], 9, GetVSName(GroupIndex)), DoSetVerifyStyle, NULL);
	}
	ValueModified=FALSE;
	ret=RunMenu(m21);
	DestroyMenu(m21);
	if(ValueModified && ret!=News_TimeOut)
	{
		ret=LCDSelectOK(LoadStrByID(MID_OA_GVERIFYTYPE), LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret) 
		{

			SaveGroupVS("GVS", GVS_Value, 5);
		}
		//flush the cached data to disk
		sync();
	}
	gOptions.Saved=TRUE;
	return ret;	
}


int DoSetIsUGrpVS(void *p)
{
	int udvs=putz->UserGrpVS;
	int ret=InputYesNoItem((PMsg)p, &udvs);
	if(ret==News_CommitInput)
	{
		if(udvs!=putz->UserGrpVS)
		{
			ValueModified=TRUE;
			putz->UserGrpVS=udvs;
		}
	}
	return ret;	
}

int DoSetUserAcc(void *p)
{
	TUTZDEF OldValue, NewValue;
	int i, ret;
	static PMenu m21=NULL;
	TUser user;
	TExtUser extuser;
	
	putz=&NewValue;

	LCD_Clear();
	user.PIN=GetNextPIN(0, FALSE);
	ret=InputPINBox(LoadStrByID(MID_OA_UAOPT), LoadStrByID(HID_OKCANCEL), 0, &user);
	if(ret!=News_CommitInput) return ret;
	OldValue.UserID=user.PIN;
	sprintf(cbuf[9], LoadStrByID(HID_UAOPT), OldValue.UserID);
	m21=CreateMenu(cbuf[9],gOptions.MenuStyle,NULL, m21);
	OldValue.UserGrp=GetUserGrp(OldValue.UserID);
	AddMenuItem(1, m21, MenuFmtInt(cbuf[0], MID_OA_GRP, OldValue.UserGrp), DoSetUGrp, NULL);
	memset((void*)OldValue.TZs, 0, sizeof(OldValue.TZs));
	i=GetUserTZ(OldValue.UserID, OldValue.TZs);
	OldValue.UseGrpTZ=(i<0);
	if(i<0)
		OldValue.TZCount=i;
	else
		OldValue.TZCount=-1-i;
	NewValue=OldValue;
	UpdateUZTs();	
	AddMenuItem(1, m21, cbuf[1], DoSetUDTZ, NULL);
	AddMenuItem(1, m21, cbuf[2], DoSetUTZ, NULL);
	AddMenuItem(1, m21, cbuf[3], DoSetUTZ, NULL);
	AddMenuItem(1, m21, cbuf[4], DoSetUTZ, NULL);
	if(gOptions.UserExtendFormat)
	{
		OldValue.VS=GetExtUserByVS(user.PIN)&0x7F;
		OldValue.UserGrpVS=!(GetExtUserByVS(user.PIN)>>7);
		AddMenuItem(1, m21, MenuFmtStr(cbuf[5],MID_OA_VERIFYTYPE,GetVSName(OldValue.VS)), DoSetVerifyStyle, NULL);		
		AddMenuItem(1, m21, MenuFmtStr(cbuf[6],MID_OA_GRPVS, GetYesNoName(OldValue.UserGrpVS)), DoSetIsUGrpVS, NULL);
		NewValue.VS=OldValue.VS;
		NewValue.UserGrpVS=OldValue.UserGrpVS;
	}
	ValueModified=FALSE;
	ret=RunMenu(m21);
	DestroyMenu(m21);
	if(ValueModified && ret!=News_TimeOut)
	{
		ret=LCDSelectOK(cbuf[9],LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret)
		{
			if(gOptions.UserExtendFormat)
			{
				if((OldValue.VS+(OldValue.UserGrpVS?0:0x80))!=(NewValue.VS+(NewValue.UserGrpVS?0:0x80)))
				{
					FDB_CreateExtUser(&extuser, OldValue.UserID, NewValue.VS+(NewValue.UserGrpVS?0:0x80));
					if(FDB_GetExtUser(OldValue.UserID, NULL))
						FDB_ChgExtUser(&extuser);						
					else
						FDB_AddExtUser(&extuser);
				}
			}
			if(OldValue.UserGrp!=NewValue.UserGrp)
			{
				int j=SaveUserGrp(OldValue.UserID, NewValue.UserGrp);
				//如果修改了组必须重新刷新组
				FPDBInit();
				FDB_AddOPLog(ADMINPIN, OP_ACC_USER, NewValue.UserID, j, 0, NewValue.UserGrp);
			}

			ValueModified=FALSE;
			if(NewValue.UseGrpTZ)
			{
				if(!OldValue.UseGrpTZ)
				{
					int j=ClearUserTZ(OldValue.UserID);
					FDB_AddOPLog(ADMINPIN, OP_ACC_USER, NewValue.UserID, j, 1, 0);
				}
			}
			else
			{
				if(OldValue.UseGrpTZ)
					ValueModified=TRUE;
				else
					for(i=0;i<3;i++)
						if(OldValue.TZs[i]!=NewValue.TZs[i])
							ValueModified=TRUE;
			}
			if(ValueModified)
			{
				int j=SaveUserTZ(NewValue.UserID, NewValue.TZs, 3);
				FDB_AddOPLog(ADMINPIN, OP_ACC_USER, NewValue.UserID, j, 2, 0);
			}
			//flush the cached data to disk
			sync();
		}
	}
	gOptions.Saved=TRUE;	
	return ret;	
}

int DoSetGTZItem(void *p)
{
	PMenu menu=((PMenu)((PMsg)p)->Object);
	int value, ret,oldvalue;
	value=StrValue(menu->Items[menu->ItemIndex].Caption+gLCDCharWidth-2-MenuIndicatorWidth,NULL);
	oldvalue = value;
	ret=InputValueOfItem((PMsg)p, 2, 0, TZ_MAX, &value);
	if(ret!=News_CommitInput) return ret;
	ValueModified=TRUE;
	if (oldvalue==value)
		ValueModified=FALSE;
	return ret;
}

int DoSetGTZ(void *p)
{
	int UserID;
	int i, ret, TZs[10], tz;
	static PMenu m21=NULL;
	char buf[MAX_CHAR_WIDTH*10], cbuf[10][MAX_CHAR_WIDTH];
	
	LCD_Clear();
	LCDWriteLineStrID(0,MID_OA_GTZ);
	LCDWriteCenterStrID(3, HID_OKCANCEL);
	LCDWriteCenterStrID(1,HID_GRPNUM);
	UserID=1;
	if(gLCDRowCount>2)
	{
		LCDWriteCenterStrID(1,HID_GRPNUM);
		do
		{
			ret=InputNumber(2,(gLCDCharWidth-2)/2,2,&UserID, 1, 5, FALSE);
			if(ret!=News_CommitInput) return ret;
		}while(!((UserID>0) && (UserID<6)));
	}
	else
	{
		LCDWriteLineStrID(1,HID_GRPNUM);
		do
		{
			ret=InputNumber(gLCDRowCount-1,gLCDCharWidth-3,2,&UserID, 1, 5, FALSE);
			if(ret!=News_CommitInput) return ret;
		}while(!((UserID>0) && (UserID<6)));
	}	
	sprintf(cbuf[0], LoadStrByID(HID_GTZ), UserID);
	m21=CreateMenu(cbuf[0],gOptions.MenuStyle,NULL, m21);
	memset((void*)TZs, 0, sizeof(TZs));
	tz=GetGrpTZ(UserID, TZs);
/*
	for(i=1;i<=3;i++)
	{
		char format[20];
		sprintf(buf, LoadStrByID(HID_TZI),i);
		if(TZs[i-1]==0)
		{
			sprintf(format, "%%-%ds", gLCDCharWidth-MenuIndicatorWidth);
			sprintf(cbuf[i], format, buf);
		}
		else
		{
			sprintf(format, "%%-%ds%%2d", gLCDCharWidth-2-MenuIndicatorWidth);
			sprintf(cbuf[i], format, buf, TZs[i-1]);
		}
		AddMenuItem(0, m21, cbuf[i], DoSetGTZItem, NULL);
	}
*/
        for(i=1;i<=3;i++)
        {
                char buf[200], value[10];
                sprintf(buf, LoadStrByID(HID_TZI),i);
                if(TZs[i-1]==0)
                {
                        sprintf(cbuf[i], LoadStrByID(HID_TZI),i);
                }
                else
                {
                        sprintf(value,"%d", TZs[i-1]);
                        PadRightStrStr(cbuf[i], buf, value, MenuCharWidth);
                }
		AddMenuItem(1, m21, cbuf[i], DoSetGTZItem, NULL);
        }

	
	ValueModified=FALSE;
	ret=RunMenu(m21);
	DestroyMenu(m21);
	if(ValueModified && ret!=News_TimeOut)
	{
		ret=LCDSelectOK(cbuf[0],LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret)
		{
			ValueModified=FALSE;
			for(i=0;i<3;i++)
			{
				tz=StrValue(cbuf[1+i]+gLCDCharWidth-4, NULL);
				if(tz!=TZs[i])
				{
					TZs[i]=tz;
					ValueModified=TRUE;
				}
			}
			if(ValueModified)
			{
				int j=SaveGrpTZ(UserID, TZs, 3);
				FDB_AddOPLog(ADMINPIN, OP_ACC_GRP, UserID, j, TZs[0], TZs[1]);
			}
			//flush the cached data to disk
			sync();
		}
	}
	gOptions.Saved=TRUE;	
	return ret;	
}

int SaveUserTZ(int UserID, int *TZs, int Count)
{
	TUser u;
	int i,j;
	for(i=0,j=0;i<Count;i++)
	{
		if(TZs[i])
		{
			if(j!=i) 
				TZs[j]=TZs[i];
			j++;
		}
	}
	if(FDB_GetUser(UserID, &u))
	{
		SetUserTZValue(&u, TZs, (Count==0));
		if(FDB_ChgUser(&u)==FDB_OK) return TRUE;
	}
	return FALSE;
}

int SaveGrpTZ(int ID, int *TZs, int Count)
{
	char *buf, cbuf[MAX_CHAR_WIDTH];
	buf=(char*)malloc(Count*12);
	SaveIntList(buf, TZs, Count, 0, TRUE);
	sprintf(cbuf, "GTZ%d", ID);
	SaveStr(cbuf, buf, TRUE);
	free(buf);
	return TRUE;
}

int GetSensorModeName(char *name, int Mode)
{
        char *p=LoadStrByID(HID_DSM);
        if(p)
        {
		return SCopyStrFrom(name,p,Mode);
        }
        return 0;
}
                                                                                                               
int DoSetOpenDoorDelay(void *p)
{
        return InputValueOfItem((PMsg)p, 3, 0, 254, &gOptions.OpenDoorDelay);
}

int DoSetDoorSensorAlarmDelay(void *p)
{
        return InputValueOfItem((PMsg)p, 3, 0, 999, &gOptions.DoorSensorTimeout);
}

int DoSetDoorSensorMode(void *p)
{
        PMenu menu=((PMenu)((PMsg)p)->Object);
        int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret;
        int col=gLCDCharWidth-4, index=gOptions.DoorSensorMode;
        char dsmname[10];
        int values[4]={0,1,2};
                                                                                                               
        ret=LCDSelectItemValue(row, col, 4, LoadStrByID(HID_DSM), values, &index);
        if(ret==News_CommitInput)
        if(index!=gOptions.DoorSensorMode)
        {
                char format[40];
		sprintf(format, "%%-%ds%%4s", gLCDCharWidth-4-MenuIndicatorWidth);
                GetSensorModeName(dsmname,index);
                sprintf(menu->Items[menu->ItemIndex].Caption,format,LoadStrByID(MID_AC_DSM),dsmname);
                gOptions.DoorSensorMode=index;
                gOptions.Saved=FALSE;
                ShowMenu(menu);
        }
        return ret;
}


int DoSetDoorAlarmMode(void *p)
{
        PMenu menu=((PMenu)((PMsg)p)->Object);
        int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret;
        int col=gLCDCharWidth-4, index=gOptions.DoorAlarmMode;
        char dsmname[10];
        int values[4]={0,1,2};
                                                                                                               
        ret=LCDSelectItemValue(row, col, 4, LoadStrByID(HID_DSM), values, &index);
        if(ret==News_CommitInput)
        if(index!=gOptions.DoorAlarmMode)
        {
                char format[40];
		sprintf(format, "%%-%ds%%4s", gLCDCharWidth-4-MenuIndicatorWidth);
                GetSensorModeName(dsmname,index);
                sprintf(menu->Items[menu->ItemIndex].Caption,format,LoadStrByID(MID_AC_DSM),dsmname);
                gOptions.DoorAlarmMode=index;
                gOptions.Saved=FALSE;
                ShowMenu(menu);
        }
        return ret;
}

int DoEnrollDuressFinger(void *p)
{
	int i, ret, tmplens, FingerID;
	BYTE tmps[1024];
	TUser user;
	char buf[128];

//2006.10.13 修正：当用户数为0时，登记胁迫指纹程序异常
	if (FDB_CntUser()==0)
	{
		return News_CancelInput;
	}

	if(FDB_CntTmp()>=gOptions.MaxFingerCount*100)
	{
		LCD_Clear();
		LCDWriteCenterStrID(gLCDRowCount/2, HID_EXCEED);
		DelayMS(3*1000);			
		return News_CancelInput;
	}

	while(1)
	{
		memset((void*)&user, 0, sizeof(TUser));
		user.PIN=1;
		LCDWriteCenterStr(1, "");
		i=InputPINBox(LoadStrByID(MID_AD_DURESSFINGER),LoadStrByID(HID_OKCANCEL), !INPUT_USER_NEW, (void*)&user);
		if(i==News_ErrorInput)
		{
			LCDWriteCenterStrID(3,HID_ERRORPIN);
			DelayMS(3*1000);
			continue;
		}
		else if(i==News_CancelInput || i==News_TimeOut)
			return i;
		FingerID=-1;
		for(i=0;i<gOptions.MaxUserFingerCount;i++)
		{
			if(0==FDB_GetTmp((U16)user.PIN,(char)i,NULL))
			{
				FingerID=i;
				break;
			}
		}
		if(FingerID==-1)
		{
			sprintf(buf, LoadStrByID(HID_EXCEEDFINGER), gOptions.MaxUserFingerCount);
			LCDInfo(buf,5);
			user.PIN++;
			continue;
		}
		break;
	}
	while(1)
	{
		tmplens=0;
		ret=EnrollAFinger((char*)tmps, &tmplens, user.PIN, FingerID);
		if(ret!=News_CommitInput) return ret;
		if(tmplens==0) //enroll fail
		{
			if(gOptions.VoiceOn) ExPlayVoice(VOICE_RETRY_FP);
			LCDWriteCenterStrID(2,HID_INPUTAGAIN);
			DelayMS(3*1000);
		}
		else
		{
			char buf[MAX_CHAR_WIDTH];
			TTemplate t;
			
			FormatPin(buf, (void*)&user.PIN, FingerID, TRUE, FALSE);
			LCDWriteCenterStr(1, buf);
			ret=LCDSelectOK(LoadStrByID(MID_AD_DURESSFINGER), buf, LoadStrByID(HID_SAVECANCEL));
			if(News_CommitInput!=ret) break;
			FDB_CreateTemplate(&t, (U16)user.PIN, (char)FingerID, (char*)tmps, tmplens);
			t.Valid=1 | DURESSFINGERTAG;
			ret=FDB_AddTmp(&t);
			if(FDB_OK!=ret)
			{
				LCDWriteCenterStrID(2, HID_FLASHERROR);
				DelayMS(3*1000);
				ExBeep(2);
			}
			else
				FPDBInit();
			FDB_AddOPLog(ADMINPIN, OP_ENROLL_FP, user.PIN, ret, FingerID, t.Valid);
			break;
		}
	}
	return ret;
}

int DoRegisterDuressFinger(void *p)
{
	int i,ret,fc,j;
	PUser u;
	TUser user;
	TTemplate Fingers[gOptions.MaxUserFingerCount];
	LCD_Clear();
	user.PIN=GetNextPIN(0, FALSE);
	if(user.PIN==0)
		return News_ErrorInput;
	ret=InputPINBox(LoadStrByID(MID_ADF_REG),LoadStrByID(HID_OKCANCEL), FALSE, (void*)&user);
	if(News_CancelInput==ret || News_TimeOut==ret) return ret;
	//ret==News_CommitInput
	u=FDB_GetUser(user.PIN, NULL);
	if(u)
	{
		fc=0;
		for(i=0;i<gOptions.MaxUserFingerCount;i++)
		{
			TTemplate tmp;
			memset((void *)&(Fingers[i]), 0, sizeof(TTemplate));
			if(FDB_GetTmp(user.PIN, i, &tmp))
			{
				fc++;
				memcpy((void *)&(Fingers[i]), &tmp, sizeof(TTemplate));
			}
		}		
		if(fc==0)
		{
			LCDInfoShow(LoadStrByID(MID_ADF_REG), LoadStrByID(HID_NOFINGER));
			DelayMS(3000);
			return News_CancelInput;
		}
		else if(fc)
		{
			for(i=0;i<gOptions.MaxUserFingerCount;i++)
				if(Fingers[i].PIN)
				{
					char buf[40];
					FormatPin(buf, (void*)&user.PIN, i, TRUE, FALSE);
					ret=LCDSelectOK(LoadStrByID(MID_ADF_REG), buf, LoadStrByID(HID_OKCANCEL));
					if(News_CancelInput==ret) continue;
					if(News_TimeOut==ret) return ret;
					j=FDB_SetDuressTmp(&(Fingers[i]));
					FDB_AddOPLog(ADMINPIN, OP_CHG_FP, u->PIN, j, i, DURESSFINGERTAG);
					break;
				}
		}
	}
	return ret;
}

int DoUnregisterDuressFinger(void *p)
{
	int i,ret,fc;
	PUser u;
	TUser user;
	TTemplate Fingers[MAX_USER_FINGER_COUNT];
	LCD_Clear();
	user.PIN=GetNextPIN(0, FALSE);
	if(user.PIN==0)
		return News_ErrorInput;
	ret=InputPINBox(LoadStrByID(MID_ADF_UNREG),LoadStrByID(HID_OKCANCEL), FALSE, (void*)&user);
	if(News_CancelInput==ret || News_TimeOut==ret) return ret;
	//ret==News_CommitInput
	u=FDB_GetUser(user.PIN, NULL);
	if(u)
	{
		fc=0;
		for(i=0;i<gOptions.MaxUserFingerCount;i++)
		{
			TTemplate tmp;
			memset((void *)&(Fingers[i]), 0, sizeof(TTemplate));
			if(FDB_GetTmp(user.PIN, i, &tmp))
			{
				if(ISDURESSFP((&tmp)))
				{
					fc++;
					memcpy((void *)&(Fingers[i]), &tmp, sizeof(TTemplate));
				}
			}
		}
		if(fc==0)
		{
			LCDInfoShow(LoadStrByID(MID_ADF_UNREG), LoadStrByID(HID_NOFINGER));
			DelayMS(3000);
			return News_CancelInput;
		}
		else if(fc)
		{
			for(i=0;i<gOptions.MaxUserFingerCount;i++)
				if(Fingers[i].PIN)
				{
					char buf[MAX_CHAR_WIDTH];
					int j;
					FormatPin(buf, (void*)&user.PIN, i, TRUE, FALSE);
					ret=LCDSelectOK(LoadStrByID(MID_ADF_UNREG), buf,LoadStrByID(HID_OKCANCEL));
					if(News_CancelInput==ret) continue;
					if(News_TimeOut==ret) return ret;
					j=FDB_ClearDuressTmp(&(Fingers[i]));
					FDB_AddOPLog(ADMINPIN, OP_CHG_FP, u->PIN, j, i, ~DURESSFINGERTAG);
					if(FDB_OK!=j)
						break;
				}
		}
	}
	return ret;
}

int DoUnregisterAll(void *p)
{
	int ret=LCDSelectOK(LoadStrByID(MID_ADF_UNREGALL), LoadStrByID(HID_UNREGALL),LoadStrByID(HID_OKCANCEL));
	if(ret==News_CommitInput)
		FDB_ClrDuressTagTmpAll();
	return ret;
}

int DoDuressFinger(void *p)
{
	int ret;
	PMenu m21=NULL;
	m21=CreateMenu(LoadStrByID(MID_AD_DURESSFINGER),gOptions.MenuStyle,NULL, m21);
	AddMenuItem(1, m21, LoadStrByID(MID_ADF_ENROLL), DoEnrollDuressFinger, NULL);
	AddMenuItem(1, m21, LoadStrByID(MID_ADF_REG), DoRegisterDuressFinger, NULL);
	AddMenuItem(1, m21, LoadStrByID(MID_ADF_UNREG), DoUnregisterDuressFinger, NULL);
	AddMenuItem(1, m21, LoadStrByID(MID_ADF_UNREGALL), DoUnregisterAll, NULL);
	ret=RunMenu(m21);
	DestroyMenu(m21);
	//flush the cached data to disk
	sync();
	return ret;
}

int DoSetDuressHelpKeyOn(void *p)
{
	return InputYesNoItem((PMsg)p, &gOptions.DuressHelpKeyOn);
}

int DoSetDuress11(void *p)
{
	return InputYesNoItem((PMsg)p, &gOptions.Duress1To1);
}

int DoSetDuress1N(void *p)
{
	return InputYesNoItem((PMsg)p, &gOptions.Duress1ToN);
}

int DoSetDuressAD(void *p)
{
	return InputValueOfItem((PMsg)p, 3, 0, 254, &gOptions.DuressAlarmDelay);
}

int DoSetDuressPwd(void *p)
{
	return InputYesNoItem((PMsg)p, &gOptions.DuressPwd);
}

int DoSetDuress(void *p)
{
	char cbuf[5][MAX_CHAR_WIDTH];
	int ret;
	PMenu m21=NULL;
	m21=CreateMenu(LoadStrByID(MID_AD_DURESS),gOptions.MenuStyle,NULL, m21);	
	
	AddMenuItem(1, m21, LoadStrByID(MID_AD_DURESSFINGER), DoDuressFinger, NULL);
        AddMenuItem(1, m21, MenuFmtStr(cbuf[0],MID_AD_DURESSHELP,GetYesNoName(gOptions.DuressHelpKeyOn)), DoSetDuressHelpKeyOn, NULL);
	AddMenuItem(1, m21, MenuFmtStr(cbuf[1],MID_AD_DURESS11,GetYesNoName(gOptions.Duress1To1)), DoSetDuress11, NULL);
        AddMenuItem(1, m21, MenuFmtStr(cbuf[2],MID_AD_DURESS1N,GetYesNoName(gOptions.Duress1ToN)), DoSetDuress1N, NULL);
        AddMenuItem(1, m21, MenuFmtStr(cbuf[3],MID_AD_DURESSPWD,GetYesNoName(gOptions.DuressPwd)), DoSetDuressPwd, NULL);
        AddMenuItem(1, m21, MenuFmtInt(cbuf[4],MID_AD_DURESSAD,gOptions.DuressAlarmDelay), DoSetDuressAD, NULL);
	gOptions.Saved=TRUE;
	ret=RunMenu(m21);
	DestroyMenu(m21);
	if(!gOptions.Saved && ret!=News_TimeOut)
	{
		ret=LCDSelectOK(LoadStrByID(MID_OPTIONS_SYSTEM),LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret) SaveOptions(&gOptions); else LoadOptions(&gOptions);
	}
	gOptions.Saved=TRUE;
	return ret;

}


int DoSetErrTimes(void *p)
{
	return(InputValueOfItem((PMsg)p, 1, 0, 9, &gOptions.ErrTimes));
}

int DoSetAccess(void *p)
{
	char cbuf[10][MAX_CHAR_WIDTH], dsmname[10];
	int ret;
	static PMenu m21=NULL;

	m21=CreateMenu(LoadStrByID(MID_OA_OPTION),gOptions.MenuStyle,NULL, m21);
	AddMenuItem(1, m21, LoadStrByID(MID_OA_TZDEF), DoDefTZ, NULL);
	AddMenuItem(1, m21, LoadStrByID(MID_OA_UAOPT), DoSetUserAcc, NULL);
	AddMenuItem(1, m21, LoadStrByID(MID_OA_GTZ), DoSetGTZ, NULL);
	AddMenuItem(1, m21, LoadStrByID(MID_OA_ULGRP), DoULGrp, NULL);
	AddMenuItem(1, m21, MenuFmtInt(cbuf[0],MID_OS_LOCK,gOptions.LockOn), DoSetLock, NULL);
	if(gOptions.LockFunOn&LOCKFUN_DOORSENSOR)
	{
                AddMenuItem(1, m21, MenuFmtInt(cbuf[1],MID_AC_DSD,gOptions.OpenDoorDelay), DoSetOpenDoorDelay, NULL);
                GetSensorModeName(dsmname,gOptions.DoorSensorMode);
                AddMenuItem(1, m21, MenuFmtStr(cbuf[2],MID_AC_DSM,dsmname), DoSetDoorSensorMode, NULL);
		AddMenuItem(1, m21, MenuFmtInt(cbuf[4],MID_AC_DSAD,gOptions.DoorSensorTimeout), DoSetDoorSensorAlarmDelay, NULL);

//暂不支持报警开关功能  2006.10.08
/*
                GetSensorModeName(dsmname,gOptions.DoorAlarmMode);
                AddMenuItem(0, m21, MenuFmtStr(cbuf[5],MID_AC_DLM,dsmname), DoSetDoorAlarmMode, NULL);
*/
	}	
	if (gOptions.LockFunOn != LOCKFUN_ADV)
	{
		AddMenuItem(1, m21, LoadStrByID(MID_AD_DURESS), DoSetDuress, NULL);
		AddMenuItem(1, m21, MenuFmtInt(cbuf[3], MID_AD_ERRPRESS, gOptions.ErrTimes), DoSetErrTimes, NULL);
	}
	if(gOptions.UserExtendFormat)
		AddMenuItem(1, m21, LoadStrByID(MID_OA_GVERIFYTYPE), DoSetGroupVerifyStyle, NULL);
	gOptions.Saved=TRUE;
	ret=RunMenu(m21);
	DestroyMenu(m21);
	if(!gOptions.Saved && ret!=News_TimeOut)
	{
		ret=LCDSelectOK(LoadStrByID(MID_OPTIONS_SYSTEM),LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret) SaveOptions(&gOptions); else LoadOptions(&gOptions);
	}
	gOptions.Saved=TRUE;
	return ret;
}


int LoadTimeZone(int Index, PTimeZone TZ)
{
	char buf[200], p[1024];
	int i;
	for(i=0;i<7;i++)
	{
		TZ->intervals[i][TZ_START][TZ_HOUR]=0;TZ->intervals[i][TZ_START][TZ_MINUTE]=0;
		TZ->intervals[i][TZ_END][TZ_HOUR]=23;TZ->intervals[i][TZ_END][TZ_MINUTE]=59;
	}
	sprintf(buf,"TZ%d", Index);
	if (!LoadStr(buf, p)) return FALSE;
	i=StringDecode(buf, p);
	memcpy((void*)TZ, buf, i);
	return TRUE;
}

int SaveTimeZone(int Index, PTimeZone TZ)
{
	char buf[200], name[20];
	StringEncode(buf, (char *)TZ, sizeof(TTimeZone));
	sprintf(name, "TZ%d", Index);
	return SaveStr(name, buf, TRUE);
}

int TestTimeZone(PTimeZone TZ, TTime t)
{
	int Start, End, 
	i=t.tm_wday;
	Start=TZ->intervals[i][TZ_START][TZ_HOUR]*60+TZ->intervals[i][TZ_START][TZ_MINUTE];
	End=TZ->intervals[i][TZ_END][TZ_HOUR]*60+TZ->intervals[i][TZ_END][TZ_MINUTE];
	i=t.tm_hour*60+t.tm_min;
	return (Start<=i) && (End>=i);
}

int IsInGrp(int UserID, int GrpID)
{
	WORD UserIDs[4000];
	char buf[20], p[1024];
	int len;
	sprintf(buf,"GRP%d", GrpID);
	if(LoadStr(buf, p))
	{
		len=StringDecode((char*)UserIDs, p);
		len/=2;
		while(len-->0)
		{
			if(UserIDs[len]==UserID) return TRUE;
		}
	}
	return FALSE;
}

int GetUserGrp(int UserID)
{
	int i;
	PUser u=FDB_GetUser(UserID, NULL);
	if(u)
	{
		i=u->Group & 0x0F;
		if(i>0)
			return i;
	}
	return 1;
}

int GetUserGrpOld(int UserID)
{
	int i;
	for(i=10;i>1;i--)
		if(IsInGrp(UserID,i)) return i;
	return 1;
}

/*
int DelUserFromGrpOld(int UserID, int GrpID)
{
	WORD UserIDs[4000];
	char buf[4000*2], *p, name[20];
	int len,i;
	if(GrpID==1) return TRUE;
	sprintf(name,"GRP%d", GrpID);
	p=LoadStr(name);
	if(p)
	{
		len=StringDecode((char*)UserIDs, p);
		len/=2;
		for(i=0;i<len;i++)
		{
			if(UserIDs[i]==UserID)
			{
				int j;
				for(j=i+1;j<len;j++)
					UserIDs[j-1]=UserIDs[j];
				len--;
				StringEncode(buf, (char*)UserIDs, len*2);
				if(SaveStr(name, buf, TRUE)) 
					return TRUE;
				else 
					return FALSE;
			}
		}
	}
	return FALSE;
}

int AppendUserToGrp(int UserID, int GrpID)
{
	WORD UserIDs[4000];
	char buf[4000*2], *p, name[20];
	int len;
	if(GrpID==1) return TRUE;
	sprintf(name,"GRP%d", GrpID);
	p=LoadStr(name);
	if(p)
	{
		len=StringDecode((char*)UserIDs, p);
		len/=2;
	}
	else
		len=0;
	UserIDs[len]=UserID;
	len++;
	StringEncode(buf, (char*)UserIDs, len*2);
	if(SaveStr(name, buf, TRUE)) 
		return TRUE; 
	else 
		return FALSE;
}
*/
int SaveUserGrp(int UserID, int GrpID)
{
	int i;
	TUser u;
	if(FDB_GetUser(UserID, &u))
	{
		i=u.Group & 0x0F;
		if(i==0)
			i=1;
	}
	else
		i=1;
	if(i==GrpID) return TRUE;

	u.Group=(u.Group & 0xf0) | (GrpID &0x0f);
	if(FDB_ChgUser(&u)==FDB_OK) return TRUE;
	return FALSE;
}

int ExtractTZs(char *p, int *TZs)
{
	int i, next, ret=0, index;
	if(p)
	for(i=0;i<10;i++)
	{
		if(*p==0) break;
		if(*p==':') p++;
		else 
		{
			index=StrValue(p, &next);
			if(index) TZs[ret++]=index;
			p+=next;
		}
	}
	return ret;
}

int GetUserTZ(int UserID, int *TZs)
{
	TUser u;
	if(FDB_GetUser(UserID, &u))
	{
		if((u.TimeZones==0) && (0==(u.Group & 0xf0)))
			return -1-GetGrpTZ(GetUserGrp(UserID), TZs);
		else
			return GetUserTZValue(&u, TZs);
	}
	return 0;
}

int GetUserTZOld(int UserID, int *TZs)
{
	char buf[20], p[1024];
	sprintf(buf, "UTZ%d", UserID);
	if(!LoadStr(buf, p))
		return -1-GetGrpTZ(GetUserGrp(UserID), TZs);
	else
		return ExtractTZs(p, TZs);
}

int GetGrpTZ(int GrpID, int *TZs)
{
	char buf[20], p[1024];
	sprintf(buf, "GTZ%d", GrpID);
	LoadStr(buf, p);
	return ExtractTZs(p, TZs);
}

int ClearUserTZ(int UserID)
{
	TUser u;
	if(FDB_GetUser(UserID, &u))
	{
		if(u.TimeZones || (u.Group & 0xF0))
		{
			u.TimeZones=0;
			u.Group &=0x0f;
			if(FDB_OK!=FDB_ChgUser(&u)) return FALSE;
		}
	}
	return TRUE;
}

int TestUserTZ(int UserID, TTime t)
{
	int TZC, TZs[10];
	TTimeZone TZ;
	if(gOptions.LockFunOn & 2)
	{
		TZC=GetUserTZ(UserID, TZs);
		if(TZC<0) TZC=-1-TZC;
		while(TZC>0)
		{
			TZC--;
			LoadTimeZone(TZs[TZC], &TZ);
			if(TestTimeZone(&TZ, t))
				return TRUE;
		}
		return FALSE;
	}
	else
		return TRUE;
}

//测试一个用户组合是否合法的开锁组合
//>0 是合法的开锁组合, 返回组合的人数
//=0 不是合法的开锁组合的一部分
//-1 是合法的开锁组合的一部分
int TestGrp(char *ValidGrps, char *TGrp)
{
        int i,j,len,ret=0;
        char g, gbuf[20], g0=0;
        len=0;
        for(i=0;i<20;i++)
        {
		g=TGrp[i];
		gbuf[i]=g;
		if(g==0) break;
		len++;
		g0=g;
	}
        for(i=0;i<20;i++)
        {
		g=ValidGrps[i];
		if(g==0) return i;
		if(len==0) break;
		for(j=0;j<len;j++)
		{
			if(g==gbuf[j])
			{
				int k;
				for(k=j+1;k<len;k++)
					gbuf[k-1]=gbuf[k];
                                break;
			}
		}
		if(j>=len)
		{
			//检查最后一个组是否合法组合的一部分
			if(ret==0)
			{
				for(j=0;j<20;j++)
				{
					g=ValidGrps[j];
					if(g==0) break;
					if(g0==ValidGrps[j])
					{
						ret=-1;
						break;
					}
				}
			}
			break;
		}
		len--;
		ret=-1;
	}
	return ret;
}

int SetUserTZValue(PUser User, int *TZs, int UseGroup)
{
	int value=0, i=0;
	while(i<3)
	{
		if(TZs[i]==0) break;
		value|=((TZs[i] & 0x3f)<<(i*6)); //6 BIT Per TimeZone
		i++;
	}
	User->TimeZones=value & 0xFFFF;
	User->Group=(User->Group & 0xF) | (((value>>16) & 0x0F) << 4) | (UseGroup?0:0x80);
	return i;
}

int GetUserTZValue(PUser User, int *TZs)
{
	TZs[0]=User->TimeZones & 0x3F;
	TZs[1]=User->TimeZones>>6 & 0x3F;
	TZs[2]=(User->TimeZones>>12)+16*(0x3 & (User->Group>>4));
	if(TZs[0]==0) return 0;
	if(TZs[1]==0) return 1;
	if(TZs[2]==0) return 2;
	if(TZs[2]) TZs[3]=0;
	return 3;
}

int TestAllGrp(char *TGrp)
{
	char grps[128], buf[20];
	int i,ret,last=0;
	
	if (!LoadStr("ULG", grps)) return FALSE;  //没有定义开锁组合
	for(i=0;i<10;i++)
	{
		if(SCopyStrFrom(buf,grps,i)>0)
		{
			ret=TestGrp(buf, TGrp);
			if(ret>0) return ret;
			if(ret<0) last=ret;
		}
	}
	return last;
}

typedef struct _VerifyRecord_{
	U16 PIN;
	int VerifyMethod;
	TTime VerTime;
}TVerifyRecord, *PVerifyRecord;

#define VerRecSetCnt 5
static TVerifyRecord LastVerRecSet[VerRecSetCnt];
static int VerRecCount=0;

#define UnlockInterval ((gOptions.WorkCode==0)?LoadInteger("~UnlockInterval", 8):LoadInteger("~UnlockInterval", 12))

int ClearMultiOpen(void)
{
        return VerRecCount=0;
}

int IsMultiOpen(void)
{
        return VerRecCount;
}

int TestOpenLock(int UID, TTime t, int VerifyMethod)
{
	int i,j;
	//delete the old records
	for(i=0;i<VerRecCount;i++)
	{
		j=TimeDiffSec(t, LastVerRecSet[i].VerTime);
		if(j<=(UnlockInterval*VerRecCount-1))
		{
			if(i>0)
			{
				for(j=i;j<VerRecCount;j++)
					LastVerRecSet[j-i]=LastVerRecSet[j];
			}
			break;
		}
	}
	VerRecCount-=i;
	//delete old record for one person
	for(i=0;i<VerRecCount;i++)
	{
		if(LastVerRecSet[i].PIN==UID)
		{
			for(j=i+1;j<VerRecCount;j++)
				LastVerRecSet[j-1]=LastVerRecSet[j];
			VerRecCount--;
		}
	}
	//only save lastest VerRecSetCnt-1 records
	if(VerRecCount>=VerRecSetCnt)
	{
		j=0;
		for(i=VerRecCount-VerRecSetCnt+1;i<VerRecCount;i++)
			LastVerRecSet[j++]=LastVerRecSet[i];
		VerRecCount=VerRecSetCnt-1;
	}
	//set current record value 
	LastVerRecSet[VerRecCount].VerTime=t;
	LastVerRecSet[VerRecCount].VerifyMethod=VerifyMethod;
	LastVerRecSet[VerRecCount].PIN=UID;
	
	if(gOptions.LockFunOn&LOCKFUN_ADV)
	{
		char GRPs[20];
		int ret;
		memset(GRPs, 0, 20);
		for(i=0;i<VerRecCount+1;i++)
			GRPs[i]='0'+GetUserGrp(LastVerRecSet[i].PIN);
		ret=TestAllGrp(GRPs);
		if(ret>0)
		{
			VerRecCount=0;
			return ret;
		}
		
		VerRecCount++;
		return ret;
	}
	else //gOptions.LockFunOn==LOCKFUN_BASE
	{
		if(VerRecCount+1>=gOptions.UnlockPerson)
		{
			VerRecCount=0;
			return gOptions.UnlockPerson;
		}
		VerRecCount++;
		return -1;
	}
	return 0;
}
