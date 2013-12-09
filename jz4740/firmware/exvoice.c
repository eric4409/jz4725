/*************************************************
                                           
 ZEM 200                                          
                                                    
 exvoice.c Define voice index when verified successful  
 
 Copyright (C) 2003-2006, ZKSoftware Inc.
 
*************************************************/
#include <stdlib.h>
#include <string.h>
#include "arca.h"
#include "lcdmenu.h"
#include "utils.h"
#include "options.h"
#include "accdef.h"
#include "lcm.h"
#include "exvoice.h"

static TVTimeZone VTZ;
static int iValueModified=FALSE;

int DoSetVInterval(void *p)
{
	PMenu menu=((PMenu)((PMsg)p)->Object);
	int ret, row=CalcMenuItemOffset(menu, menu->ItemIndex);
	int col=gLCDCharWidth-11, times[4], i=menu->ItemIndex;
	
	times[0]=CVTZ->intervals[i][TZ_START][TZ_HOUR]; 
	times[1]=CVTZ->intervals[i][TZ_START][TZ_MINUTE];
	times[2]=CVTZ->intervals[i][TZ_END][TZ_HOUR];
	times[3]=CVTZ->intervals[i][TZ_END][TZ_MINUTE];
	
	if(News_CommitInput==(ret=InputTimeInterval(row, col, times)))
	{
		if(CVTZ->intervals[i][TZ_START][TZ_HOUR]!=times[0])
		{
			CVTZ->intervals[i][TZ_START][TZ_HOUR]=times[0];
			iValueModified=TRUE;
		}
		if(CVTZ->intervals[i][TZ_START][TZ_MINUTE]!=times[1])
		{
			CVTZ->intervals[i][TZ_START][TZ_MINUTE]=times[1];
			iValueModified=TRUE;
		}
		if(CVTZ->intervals[i][TZ_END][TZ_HOUR]!=times[2])
		{
			CVTZ->intervals[i][TZ_END][TZ_HOUR]=times[2];
			iValueModified=TRUE;
		}
		if(CVTZ->intervals[i][TZ_END][TZ_MINUTE]!=times[3])
		{
			CVTZ->intervals[i][TZ_END][TZ_MINUTE]=times[3];
			iValueModified=TRUE;
		}
		if(iValueModified)
		{
			char format[40];
			sprintf(format, "%%02d:%%02d-%%02d:%%02d");
                        sprintf(menu->Items[menu->ItemIndex].Caption+gLCDCharWidth-11-MenuIndicatorWidth,format,
                                CVTZ->intervals[i][TZ_START][TZ_HOUR], CVTZ->intervals[i][TZ_START][TZ_MINUTE],
                                CVTZ->intervals[i][TZ_END][TZ_HOUR], CVTZ->intervals[i][TZ_END][TZ_MINUTE]); 			
		}
	}
	return ret;
}

int LoadVTimeZone(PVTimeZone TZ)
{
	char buf[200], p[256];
	int i;
	
	for(i=0;i<VOICE_TZ_NUM;i++)
	{
		TZ->intervals[i][TZ_START][TZ_HOUR]=0;TZ->intervals[i][TZ_START][TZ_MINUTE]=0;
		TZ->intervals[i][TZ_END][TZ_HOUR]=0;TZ->intervals[i][TZ_END][TZ_MINUTE]=0;
	}
	//morning
	TZ->intervals[0][TZ_START][TZ_HOUR]=0;TZ->intervals[0][TZ_START][TZ_MINUTE]=0;
	TZ->intervals[0][TZ_END][TZ_HOUR]=11;TZ->intervals[0][TZ_END][TZ_MINUTE]=59;
	//noon
	TZ->intervals[1][TZ_START][TZ_HOUR]=12;TZ->intervals[1][TZ_START][TZ_MINUTE]=0;
	TZ->intervals[1][TZ_END][TZ_HOUR]=13;TZ->intervals[1][TZ_END][TZ_MINUTE]=59;
	//afternoon
	TZ->intervals[2][TZ_START][TZ_HOUR]=14;TZ->intervals[2][TZ_START][TZ_MINUTE]=0;
	TZ->intervals[2][TZ_END][TZ_HOUR]=17;TZ->intervals[2][TZ_END][TZ_MINUTE]=59;
	//evening
	TZ->intervals[3][TZ_START][TZ_HOUR]=18;TZ->intervals[3][TZ_START][TZ_MINUTE]=0;
	TZ->intervals[3][TZ_END][TZ_HOUR]=23;TZ->intervals[3][TZ_END][TZ_MINUTE]=59;
	
	if (!LoadStr("VTZ", p)) return FALSE;
	i=StringDecode(buf, p);
	memcpy((void*)TZ, buf, i);
	return TRUE;
}

int SaveVTimeZone(PVTimeZone TZ)
{
	char buf[200];
	
	StringEncode(buf, (char *)TZ, sizeof(TVTimeZone));
	return SaveStr("VTZ", buf, TRUE);
}

int DoDefVerOKVoice(void *p)
{
	int ret, i;
	char cbuf[10][MAX_CHAR_WIDTH];
	PMenu m21=NULL;
	
	LCD_Clear();
	sprintf(cbuf[0], "%s", LoadStrByID(MID_ADV_VOICETZ));
	m21=CreateMenu(cbuf[0],gOptions.MenuStyle,NULL, m21);
	iValueModified=FALSE;
	for(i=0;i<VOICE_TZ_NUM;i++)
	{
		char format[MAX_CHAR_WIDTH];
		sprintf(format, "%%0%dd %%02d:%%02d-%%02d:%%02d", gLCDCharWidth-12-MenuIndicatorWidth);
		sprintf(cbuf[i+1],format, i+1, 
			CVTZ->intervals[i][TZ_START][TZ_HOUR], CVTZ->intervals[i][TZ_START][TZ_MINUTE],
			CVTZ->intervals[i][TZ_END][TZ_HOUR], CVTZ->intervals[i][TZ_END][TZ_MINUTE]);
		AddMenuItem(0, m21, cbuf[i+1], DoSetVInterval, NULL);
	}	
	ret=RunMenu(m21);
	DestroyMenu(m21);
	if(iValueModified && ret!=News_TimeOut)
	{
		ret=LCDSelectOK(cbuf[0],LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret) SaveVTimeZone(CVTZ);
	}
	gOptions.Saved=TRUE;
	return ret;	
}

void LoadVoiceInfo(void)
{
	CVTZ=&VTZ;
	LoadVTimeZone(CVTZ);
}

void PlayVoiceByTimeZone(TTime t, int group, int defIndex)
{
	int i, ret=0;
	int CurMins;
	
	if(gOptions.PlayTZVoice)
	{
		CurMins=t.tm_hour*60+t.tm_min;
		for(i=0;i<VOICE_TZ_NUM;i++)
		{
			if((CurMins>=(CVTZ->intervals[i][TZ_START][TZ_HOUR]*60+CVTZ->intervals[i][TZ_START][TZ_MINUTE]))&&
			   (CurMins<=(CVTZ->intervals[i][TZ_END][TZ_HOUR]*60+CVTZ->intervals[i][TZ_END][TZ_MINUTE])))
			{
				ret=i+1;
				break;
			}
		}
		if(ExPlayVoice(VOICE_TZ_INDEX+ret)) ExPlayVoice(defIndex);
	}
	else if(gOptions.PlayGroupVoice)
	{
		if(ExPlayVoice(VOICE_GROUP_INDEX+group)) ExPlayVoice(defIndex);		
	}
	else
		ExPlayVoice(defIndex);	
}
