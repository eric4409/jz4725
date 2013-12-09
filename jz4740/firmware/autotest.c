/*************************************************
                                           
 ZEM 200                                          
                                                    
 autotest.c                            
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "arca.h"
#include "main.h"
#include "lcm.h"
#include "exfun.h"
#include "msg.h"
#include "lcdmenu.h"
#include "flashdb.h"
#include "finger.h"
#include "zkfp.h"
#include "utils.h"
#include "options.h"
#include "autotest.h"
#include "kb.h"
#include "yaffscfg.h"

extern int nand_flash_query_block(int blockNo);

int DoAutoTestMenu(void *p)
{
	int ret;
	static PMenu m23=NULL;
	m23=CreateMenu(LoadStrByID(MID_AUTOTEST),MenuStyle_OLD,NULL, m23);
	AddMenuItem(1, m23, LoadStrByID(MID_AT_ALL), DoCheckAll, NULL);
	AddMenuItem(1, m23, LoadStrByID(MID_AT_FLASH), DoCheckFlash, NULL);
	AddMenuItem(1, m23, LoadStrByID(MID_AT_LCD), DoCheckLCD, NULL);
	AddMenuItem(1, m23, LoadStrByID(MID_AT_VOICE), DoCheckVoice, NULL);
	if (!gOptions.IsOnlyRFMachine)
		AddMenuItem(1, m23, LoadStrByID(MID_AT_FINGER), DoCheckFinger, NULL);
	AddMenuItem(1, m23, LoadStrByID(MID_AT_KEYPAD), DoCheckKeypad, NULL);
	AddMenuItem(1, m23, LoadStrByID(MID_AT_RTC), DoCheckRTC, NULL);
#ifdef MP3PLAY	
	if(gOptions.IsSupportMP3)
		AddMenuItem(1, m23, LoadStrByID(MID_AT_MP3), DoCheckMP3, NULL);
#endif	
	ret=RunMenu(m23);
	DestroyMenu(m23);
	return ret;
}

int DoCheckAll(void *p)
{
	int ret;
	ret=DoCheckFlash(p);
	if(ret==News_CancelInput || ret==News_TimeOut) return ret;
	ret=DoCheckLCD(p);
	if(ret==News_CancelInput || ret==News_TimeOut) return ret;
	ret=DoCheckVoice(p);
	if(ret==News_CancelInput || ret==News_TimeOut) return ret;
	if (!gOptions.IsOnlyRFMachine)
	{
	    ret=DoCheckFinger(p);
	    if(ret==News_CancelInput || ret==News_TimeOut) return ret;
	}
	ret=DoCheckKeypad(p);
	if(ret==News_CancelInput || ret==News_TimeOut) return ret;
	ret=DoCheckRTC(p);
#ifdef MP3PLAY	
	if(gOptions.IsSupportMP3)
	{
		if(ret==News_CancelInput || ret==News_TimeOut) return ret;
		ret=DoCheckMP3(p);
	}
#endif	
	return ret;
}

int TestFlashSector(int index)
{
	return nand_flash_query_block(index);
}

#define  FREE_FLASH_COUNT  NAND_FLASH_TOTAL_BLOCKS
int DoCheckFlash(void *p)
{
	char buffer[MAX_CHAR_WIDTH];
        int ret,i,val;

        LCD_Clear();
        LCDWriteStr(0,0,LoadStrByID(MID_AT_FLASH),0);
        LCDWriteCenterStrID(1,HID_TEST_FLASH_CQ1);
        LCDWriteCenterStrID(2,HID_TEST_FLASH_CQ2);
        LCDWriteStr(3,0,LoadStrByID(HID_CONTINUEESC), 0);
        ret=InputLine(0,0,0,NULL);
        if(ret==News_CancelInput || ret==News_TimeOut) return ret;
        LCDWriteCenterStrID(1,HID_TESTING);
        ret=0;
        for(i=0;i<FREE_FLASH_COUNT;i++)
        {
                sprintf(buffer, "%8d/%-8d", i+1,FREE_FLASH_COUNT);
                LCDWriteStr(2,0,buffer,0);
                val = TestFlashSector(i);
		#ifndef YAFFS1
		val -= 0xFF;
		if(val!=0)
		#else
		if(val!=0xFF)
		#endif
		{
			printf("The %dth block is bad\n",i);
                	ret++;
		}
        }
        LCDWriteCenterStrID(1,HID_FINISH);
        sprintf(buffer, LoadStrByID(HID_TEST_FLASH_RES), FREE_FLASH_COUNT, ret);
        LCDWriteStr(2,0,buffer,0);
        ret=InputLine(0,0,0,NULL);
        return ret;
}

int TheDoCheckLCD(char StartFillChar, int StartID)
{
	BYTE buf[MAX_CHAR_WIDTH];
	int col, row, ret;
	int FontStyle=0;
	
	if(StartFillChar) FontStyle=LCD_HIGH_LIGHT;
	/*if(gOptions.IsLeaveFactory)
	{
		LCDWriteStr(0,0,LoadStrByID(HID_TSTLCD_1), FontStyle);
		LCDWriteStr(1,0,LoadStrByID(HID_TSTLCD_1), FontStyle);
		LCDWriteStr(2,0,LoadStrByID(HID_TSTLCD_1), FontStyle);
		LCDWriteStr(3,0,LoadStrByID(HID_TSTLCD_1), FontStyle);
		ret=InputLine(0,0,0,NULL);
		if(ret==News_CancelInput || ret==News_TimeOut) return ret;
		LCDWriteStr(0,0,LoadStrByID(HID_TSTLCD_2), FontStyle);
		LCDWriteStr(1,0,LoadStrByID(HID_TSTLCD_2), FontStyle);
		LCDWriteStr(2,0,LoadStrByID(HID_TSTLCD_2), FontStyle);
		LCDWriteStr(3,0,LoadStrByID(HID_TSTLCD_2), FontStyle);
		ret=InputLine(0,0,0,NULL);
		if(ret==News_CancelInput || ret==News_TimeOut) return ret;
		return ret;
	}*/

	memset(buf, StartFillChar, gLCDCharWidth);
	for(row=0;row<8;row++)
	for(col=0;col<gLCDCharWidth/2;col++)
		LCD_OutDotsX(row,col*8,buf,16);
	memset(buf, ~StartFillChar, gLCDCharWidth);
	for(row=0;row<8;row++)
	for(col=gLCDCharWidth/2;col<gLCDCharWidth;col++)
		LCD_OutDotsX(row,col*8,buf,16);
	LCDWriteStr(0,0,LoadStrByID(HID_TSTLCD), FontStyle);
	LCDWriteStr(1,0,LoadStrByID(StartID+1), FontStyle);
	LCDWriteStr(2,0,LoadStrByID(HID_ESC), FontStyle);
	LCDWriteStr(3,0,LoadStrByID(HID_CONTINUEOK), FontStyle);
	ret=InputLine(0,0,0,NULL);
	if(ret==News_CancelInput || ret==News_TimeOut) return ret;

	LCDClear();
	memset(buf, ~StartFillChar, gLCDCharWidth);
	for(row=0;row<8;row++)
	for(col=0;col<gLCDCharWidth/2;col++)
		LCD_OutDotsX(row,col*8,buf,16);
	memset(buf, StartFillChar, gLCDCharWidth);
	for(row=0;row<8;row++)
	for(col=gLCDCharWidth/2;col<gLCDCharWidth;col++)
		LCD_OutDotsX(row,col*8,buf,16);
	LCDWriteStr(0,gLCDCharWidth/2,LoadStrByID(HID_TSTLCD), FontStyle);
	LCDWriteStr(1,gLCDCharWidth/2,LoadStrByID(StartID), FontStyle);
	LCDWriteStr(2,gLCDCharWidth/2,LoadStrByID(HID_ESC), FontStyle);
	LCDWriteStr(3,gLCDCharWidth/2,LoadStrByID(HID_CONTINUEOK), FontStyle);
	ret=InputLine(0,0,0,NULL);
	if(ret==News_CancelInput || ret==News_TimeOut) return ret;
	
	if(gLCDHeight>32)
	{
		memset(buf, StartFillChar, gLCDCharWidth);
		for(row=0;row<4;row++)
		for(col=0;col<gLCDCharWidth;col++)
			LCD_OutDotsX(row,col*8,buf,16);
		memset(buf, ~StartFillChar, gLCDCharWidth);
		for(row=4;row<8;row++)
		for(col=0;col<gLCDCharWidth;col++)
			LCD_OutDotsX(row,col*8,buf,16);
		sprintf((char*)buf,"%s %s", LoadStrByID(HID_TSTLCD), LoadStrByID(StartID+3));
		LCDWriteStr(0,0,(char*)buf, FontStyle);
		LCDWriteStr(1,0,LoadStrByID(HID_CONTINUEESC), FontStyle);
		ret=InputLine(0,0,0,NULL);
		if(ret==News_CancelInput || ret==News_TimeOut) return ret;
		
		
		memset(buf, ~StartFillChar, gLCDCharWidth);
		for(row=0;row<4;row++)
		for(col=0;col<gLCDCharWidth;col++)
			LCD_OutDotsX(row,col*8,buf,16);
		memset(buf, StartFillChar, gLCDCharWidth);
		for(row=4;row<8;row++)
		for(col=0;col<gLCDCharWidth;col++)
			LCD_OutDotsX(row,col*8,buf,16);
		sprintf((char*)buf,"%s %s", LoadStrByID(HID_TSTLCD), LoadStrByID(StartID+2));
		LCDWriteStr(2,0,(char*)buf, FontStyle);
		LCDWriteStr(3,0,LoadStrByID(HID_CONTINUEESC), FontStyle);
		ret=InputLine(0,0,0,NULL);
		if(ret==News_CancelInput || ret==News_TimeOut) return ret;
	}
	return ret;
}

int DoCheckLCD(void *p)
{
	int ret;
	LCDClear();
	ret=TheDoCheckLCD(0,HID_TSTLCD_LF);
	if(ret==News_CancelInput || ret==News_TimeOut) return ret;
	ret=TheDoCheckLCD(255,HID_TSTLCD_LE);
	return ret;
}

int RunKeyTestMsg(PMsg msg)
{
	int *TimeOut=msg->Object, oldkey;
	
	if(MSG_TYPE_TIMER==msg->Message && *TimeOut>=0)
	{
		msg->Message=0;
		msg->Param1=0;
		if(*TimeOut>=InputTimeOutSec)
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_TimeOut);
		}
		return 1;
	}
	else if(!(MSG_TYPE_BUTTON==msg->Message)) 
		return 0;
	oldkey=msg->Param1;	
	msg->Param1=0;
	msg->Message=0;
	if(*TimeOut>=0) *TimeOut=0;
	
	DBPRINTF("oldkey=%d\n", oldkey);
	if(IKeyESC==oldkey)
	{
		ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_CancelInput);
		return 1;
	}
	else if(IKeyOK==oldkey)
	{
		ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_CommitInput);
		return 1;
	}
	else if(oldkey>='0' && oldkey<='9')
	{
		char buf[24];
		sprintf(buf, "%8s: %d       ", LoadStrByID(HID_KEYNAME_NUMBER), oldkey-'0');
		LCDWriteStr(gLCDRowCount/2,0,buf,LCD_HIGH_LIGHT);
	}
	else if((oldkey>9) && (oldkey<(9+FunKeyCount-10)))
	{
		if(gOptions.KeyLayout==KeyLayout_A6)
		{
			if(oldkey<=IKeyPower)
				LCDWriteCenterStrID(gLCDRowCount/2, HID_KEYNAME_MENU+oldkey-10);
			else if(oldkey==IKeyOTIn)
				LCDWriteCenterStrID(gLCDRowCount/2, HID_SOCIN);
			else if(oldkey==IKeyOTOut)
				LCDWriteCenterStrID(gLCDRowCount/2, HID_SOCOUT);
			else if(oldkey==IKeyTIn)
				LCDWriteCenterStrID(gLCDRowCount/2, HID_SOUT);
			else if(oldkey==IKeyTOut)
				LCDWriteCenterStrID(gLCDRowCount/2, HID_SBACK);
			else if(oldkey==IKeyIn)
				LCDWriteCenterStrID(gLCDRowCount/2, HID_SCIN);
			else if(oldkey==IKeyOut)
				LCDWriteCenterStrID(gLCDRowCount/2, HID_SCOUT);
			else if(oldkey==IKeyBell)
				LCDWriteCenterStr(gLCDRowCount/2, "BELL");
		}
		else
			{
				if (oldkey == IKeyPower)
				{
					char buf[24] = "                ";
					char *txt;
					if (gOptions.IsSupportC2)
						txt = LoadStrByID(MID_PUTBELL);			
					else
						txt = LoadStrByID(HID_KEYNAME_MENU+oldkey-10);			
					strncpy(buf+(16-strlen(txt))/2, txt, strlen(txt));
					LCDWriteStr(gLCDRowCount/2,0,buf,LCD_HIGH_LIGHT);
				}
				else
				{
					char buf[24] = "                ";
					char *txt = LoadStrByID(HID_KEYNAME_MENU+oldkey-10);
					strncpy(buf+(16-strlen(txt))/2, txt, strlen(txt));
					LCDWriteStr(gLCDRowCount/2,0,buf,LCD_HIGH_LIGHT);
				}
			}
	}
	else if(oldkey>0)
	{
		char buf[24];
		sprintf(buf,"   KEY: 0x%02X    ", oldkey);
		if(oldkey!='Q')
			LCDWriteStr(gLCDRowCount/2,0,buf,LCD_HIGH_LIGHT);
	}
	return 1;
}

int DoCheckKeypad(void *p)
{
	char buf[MAX_CHAR_WIDTH];
	int ret,i=RegMsgProc(RunKeyTestMsg), TimeOut=0;
	U32 mm=SelectNewMsgMask(MSG_TYPE_BUTTON);
	LCD_Clear();
	LCDWriteStr(0,0,LoadStrByID(MID_AT_KEYPAD),0);
	LCDWriteLineStrID(gLCDRowCount/2-1, HID_KEYNAME_ANYKEY);
	sprintf(buf, "                 ");
	LCDWriteStr(gLCDRowCount/2,0,buf,LCD_HIGH_LIGHT);
	LCDWriteLineStrID(3, HID_CONTINUEESC);
	ret=DoMsgProcess(&TimeOut, News_Exit_Input);
	SelectNewMsgMask(mm);
	UnRegMsgProc(i);
	return ret;
}

int DoCheckFinger(void *p)
{
	LCD_Clear();
	LCDWriteStr(0,0,LoadStrByID(MID_AT_FINGER),0);
	LCDWriteLineStrID(3, HID_CONTINUEESC);
	LCDWriteLineStrID(1, HID_TEST_OK);
	return InputLine(0,0,0,NULL);
}

int DoCheckVoice(void *p)
{
	int i, ret=News_CancelInput;
	char buf[MAX_CHAR_WIDTH];
	int voicenum;
	LCD_Clear();
	memset(buf,0,MAX_CHAR_WIDTH);
	LCDWriteStr(0,0,LoadStrByID(MID_AT_VOICE),0);
	LCDWriteLineStrID(3, HID_CONTINUEESC);
	if (gOptions.RFCardFunOn||gOptions.MifareAsIDCard)
		voicenum=VOICE_NOENROLLED_CARD;
	else
		voicenum=VOICE_ALREADY_LOG;		
	for(i=0;i<=voicenum;i++)
	{
		sprintf(buf, LoadStrByID(HID_PLAYVOICEINDEX), i+1);
		LCDWriteLine(1, buf);		
		ExPlayVoice(i);
		ret=InputLine(0,0,0,NULL);
		if(ret==News_CancelInput || ret==News_TimeOut) return ret;
	}
	return ret;
}

int DoCheckMP3(void *p)
{
	int i, ret=News_CancelInput;
	char buf[MAX_CHAR_WIDTH];
	LCD_Clear();
	LCDWriteLineStrID(3, HID_CONTINUEESC);
	for(i=0;i<MAX_AUTOALARM_CNT;i++)
	{
		LCDClearLine(0);
		LCDClearLine(2);
		LCDWriteStr(0,0,LoadStrByID(MID_AT_MP3),0);
		sprintf(buf, LoadStrByID(HID_PLAYVOICEINDEX), i+1);
		LCDWriteLine(1, buf);
		ExPlayMP3(i+1, TRUE);
		ret=InputLine(0,0,0,NULL);
		if(ret==News_CancelInput || ret==News_TimeOut) return ret;
	}
	return ret;
}

int DoCheckRTC(void *p)
{
	time_t t1=EncodeTime(&gCurTime), t2;
	LCD_Clear();
	LCDWriteStr(0,0,LoadStrByID(MID_AT_RTC),0);
	LCDWriteLineStrID(3, HID_CONTINUEESC);
	LCDWriteLineStrID(1, HID_TESTING);
	DelayUS(1000*1000);
	GetTime(&gCurTime);
	t2=EncodeTime(&gCurTime);
	if(t1>=t2)
	{
		LCDWriteLineStrID(1, HID_TEST_BAD);
		return InputLine(0,0,0,NULL);
	}
	LCDWriteLineStrID(1, HID_TEST_OK);
	return InputLine(0,0,0,NULL);	
}
