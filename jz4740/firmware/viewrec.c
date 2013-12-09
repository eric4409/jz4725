#include <stdlib.h>
#include <string.h>
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
#include "kb.h"
#include "main.h"
#include "sensor.h"

typedef struct _DataSet_{
	void *Data;
	int CurrentRec;
	int RecordCount;
}TDataSet, *PDataSet;

typedef int (*FormatorFun)(void *);

typedef struct _DataViewer_{
	TDataSet DataSet;
	int ViewCount;
	int Descent;
	int Left;
	int Zoom;
	int Packed;
	int TimeOut;
	char Title[MAX_CHAR_WIDTH];
	FormatorFun Formator;
}TDataViewer, *PDataViewer;

int IsPrint=FALSE;
char *FormatStatus(BYTE status)
{
	static char *StaName[]={"I","O","T","B","i","o","N"," "};
	if(gOptions.ShowState)
	{
		if(status<6)
			return StaName[status];
		else
			return StaName[6];
	}
	else
		return StaName[7];
}

char *FormatVerified(BYTE verified)
{
	static char *vername[]={"P","F","C","M"};
	if(verified<3) return vername[verified];
	return vername[3];
}

char *FormatPIN(U16 PIN)
{
        static char buffer[MAX_CHAR_WIDTH];
        static U16 LastPin=0;
        char format[20]="%0.5d";
        U32 uid=PIN;
        if(LastPin==PIN) return buffer;
        if(gOptions.PIN2Width>5)
        {
                PUser u=FDB_GetUser(PIN, NULL);
                if(u && u->PIN2)
                {
                        uid=u->PIN2;
                }
                sprintf(format,"%%0.%dd", gOptions.PIN2Width);
        }
        sprintf(buffer, format, uid);
        LastPin=PIN;
        return buffer;

}

int AttLogFormator(void *dataviewer)
{
        PDataViewer dv=(PDataViewer)dataviewer;
        PDataSet Data=&dv->DataSet;
        int cw, row=0, rowc=gLCDRowCount, showc=0, i=Data->CurrentRec, inc=dv->Descent?-1:1;
        char line[MAX_CHAR_WIDTH];
        TTime tt;
        LCDBufferStart(TRUE);
        cw=gROMDriver->CharWidth;
        if(dv->Zoom && gLangDriver->CharHeight>8)
        {
                rowc=2*rowc;
                gROMDriver->CharWidth=6;
        }
        LCD_Clear();

	if (gOptions.AttLogExtendFormat)	
	{		
		PExtendAttLog logs =((PExtendAttLog)(Data->Data));	
	        if(dv->Zoom)
	        {
        	        sprintf(line,"%d/%d", dv->Descent?Data->RecordCount-Data->CurrentRec:Data->CurrentRec, Data->RecordCount);
                	LCDWriteStrLng(gROMDriver, row++, gLCDWidth/gROMDriver->CharWidth-strlen(line), line, 0);
        	}
	        while(i>=0 && i<Data->RecordCount)
        	{
                	OldDecodeTime(logs[i].time_second, &tt);
	                if(dv->Packed)
        	                sprintf(line, "%s %02d %02d:%02d%s%s", FormatPIN(logs[i].PIN), tt.tm_mday,
                	                tt.tm_hour, tt.tm_min, FormatStatus(logs[i].status), FormatVerified(logs[i].verified));
	                else
        	                sprintf(line, "%s %02d-%02d %02d:%02d:%02d%s%s", FormatPIN(logs[i].PIN), tt.tm_mon+1, tt.tm_mday,
                	                tt.tm_hour, tt.tm_min, tt.tm_sec, FormatStatus(logs[i].status), FormatVerified(logs[i].verified));
	                if(dv->Zoom)
        	                LCDWriteStrLng(gROMDriver, row, dv->Left, line, 0);
                	else
                        	LCDWriteStr(row, dv->Left, line, 0);
	                if(IsPrint)
        	        {
                	        SerialOutputString(&ff232, line);
                        	SerialOutputString(&ff232, "\r\n");
                	}
	                showc++;
        	        row++;
                	if(row>=rowc) break;
	                i+=inc;
        	}
	}
	else
	{
	        PAttLog logs=((PAttLog)(Data->Data));
		
	        if(dv->Zoom)
	        {
        	        sprintf(line,"%d/%d", dv->Descent?Data->RecordCount-Data->CurrentRec:Data->CurrentRec, Data->RecordCount);
                	LCDWriteStrLng(gROMDriver, row++, gLCDWidth/gROMDriver->CharWidth-strlen(line), line, 0);
        	}
	        while(i>=0 && i<Data->RecordCount)
        	{
                	OldDecodeTime(logs[i].time_second, &tt);
	                if(dv->Packed)
        	                sprintf(line, "%s %02d %02d:%02d%s%s", FormatPIN(logs[i].PIN), tt.tm_mday,
                	                tt.tm_hour, tt.tm_min, FormatStatus(logs[i].status), FormatVerified(logs[i].verified));
	                else
        	                sprintf(line, "%s %02d-%02d %02d:%02d:%02d%s%s", FormatPIN(logs[i].PIN), tt.tm_mon+1, tt.tm_mday,
                	                tt.tm_hour, tt.tm_min, tt.tm_sec, FormatStatus(logs[i].status), FormatVerified(logs[i].verified));
	                if(dv->Zoom)
        	                LCDWriteStrLng(gROMDriver, row, dv->Left, line, 0);
                	else
                        	LCDWriteStr(row, dv->Left, line, 0);
	                if(IsPrint)
        	        {
                	        SerialOutputString(&ff232, line);
                        	SerialOutputString(&ff232, "\r\n");
                	}
	                showc++;
        	        row++;
                	if(row>=rowc) break;
	                i+=inc;
        	}
		
	}
        gROMDriver->CharWidth=cw;
        LCDBufferStart(FALSE);
        dv->ViewCount=rowc-(dv->Zoom?1:0);
        return i-Data->CurrentRec;
}

int AttLogFormatorPacked(void *dataviewer)
{
        PDataViewer dv=(PDataViewer)dataviewer;
        PDataSet Data=&dv->DataSet;
        int pcol, lastday, cw, row=0, rowc=gLCDRowCount, showc=0, i=Data->CurrentRec, inc=dv->Descent?-1:1;
        char line[MAX_CHAR_WIDTH];
        TTime tt;
        LCDBufferStart(TRUE);
        cw=gROMDriver->CharWidth;
        if(dv->Zoom && gLangDriver->CharHeight>8)
        {
                rowc=2*rowc;
                gROMDriver->CharWidth=6;
        }
        LCD_Clear();
	if (gOptions.AttLogExtendFormat)	
	{
		PExtendAttLog logs=((PExtendAttLog)(Data->Data));	
	        if(dv->Zoom)
	        {
        	        sprintf(line," %d/%d", dv->Descent?Data->RecordCount-Data->CurrentRec:Data->CurrentRec, Data->RecordCount);
                	pcol=gLCDWidth/gROMDriver->CharWidth-strlen(line);
	                if(dv->Packed)
        	        {
                	        int startp;
                        	char line[MAX_CHAR_WIDTH];
	                        OldDecodeTime(logs[i].time_second, &tt);
        	                sprintf(line, "%s %d-%d", FormatPIN(logs[i].PIN), tt.tm_year+1900, tt.tm_mon+1);
                	        startp=pcol-strlen(line);
                        	if(startp>0)startp=0;
	                        LCDWriteStrLng(gROMDriver, row, startp, line, 0);
        	        }
                	LCDWriteStrLng(gROMDriver, row++, pcol, line, 0);
        	}
	        pcol=0;
        	lastday=0;
	        while(i>=0 && i<Data->RecordCount)
        	{
                	OldDecodeTime(logs[i].time_second, &tt);
	                if(dv->Packed)
        	        {
                	        if(pcol && (lastday!=tt.tm_mday))
                        	{
                                	pcol=0;
	                                row++;
        	                        if(row>=rowc) break;
                	        }
                        	if(pcol==0)
                        	{
                                	if(lastday!=tt.tm_mday)
	                                        sprintf(line, "%02d %02d:%02d", tt.tm_mday, tt.tm_hour, tt.tm_min);
        	                        else
                	                        sprintf(line, "   %02d:%02d", tt.tm_hour, tt.tm_min);
                        	}
	                        else
	                                sprintf(line, " %02d:%02d", tt.tm_hour, tt.tm_min);
        	                lastday=tt.tm_mday;
	                }
        	        else
	                {
	                        sprintf(line, "%s %02d-%02d %02d:%02d:%02d%s%s", FormatPIN(logs[i].PIN), tt.tm_mon+1, tt.tm_mday,
	                        tt.tm_hour, tt.tm_min, tt.tm_sec, FormatStatus(logs[i].status), FormatVerified(logs[i].verified));
	                }
	                if(dv->Zoom)
	                        LCDWriteStrLng(gROMDriver, row, pcol+dv->Left, line, 0);
	                else
	                        LCDWriteStr(row, pcol+dv->Left, line, 0);
	                if(IsPrint)
	                {
	                        SerialOutputString(&ff232, line);
	                }
	                showc++;
	                pcol+=strlen(line);
	                if(pcol+6>=gLCDWidth/(dv->Zoom?6:gLangDriver->CharWidth))
	                {
	                        pcol=0;
	                        row++;
	                        if(IsPrint)
                                SerialOutputString(&ff232, "\r\n");
	                        if(row>=rowc) break;
	                }
	                i+=inc;
        	}
	}
	else
	{
	       PAttLog logs=((PAttLog)(Data->Data));
		
	        if(dv->Zoom)
	        {
        	        sprintf(line," %d/%d", dv->Descent?Data->RecordCount-Data->CurrentRec:Data->CurrentRec, Data->RecordCount);
                	pcol=gLCDWidth/gROMDriver->CharWidth-strlen(line);
	                if(dv->Packed)
        	        {
                	        int startp;
                        	char line[MAX_CHAR_WIDTH];
	                        OldDecodeTime(logs[i].time_second, &tt);
        	                sprintf(line, "%s %d-%d", FormatPIN(logs[i].PIN), tt.tm_year+1900, tt.tm_mon+1);
                	        startp=pcol-strlen(line);
                        	if(startp>0)startp=0;
	                        LCDWriteStrLng(gROMDriver, row, startp, line, 0);
        	        }
                	LCDWriteStrLng(gROMDriver, row++, pcol, line, 0);
        	}
	        pcol=0;
        	lastday=0;
	        while(i>=0 && i<Data->RecordCount)
        	{
                	OldDecodeTime(logs[i].time_second, &tt);
	                if(dv->Packed)
        	        {
                	        if(pcol && (lastday!=tt.tm_mday))
                        	{
                                	pcol=0;
	                                row++;
        	                        if(row>=rowc) break;
                	        }
                        	if(pcol==0)
                        	{
                                	if(lastday!=tt.tm_mday)
	                                        sprintf(line, "%02d %02d:%02d", tt.tm_mday, tt.tm_hour, tt.tm_min);
        	                        else
                	                        sprintf(line, "   %02d:%02d", tt.tm_hour, tt.tm_min);
                        	}
	                        else
	                                sprintf(line, " %02d:%02d", tt.tm_hour, tt.tm_min);
        	                lastday=tt.tm_mday;
	                }
        	        else
	                {
	                        sprintf(line, "%s %02d-%02d %02d:%02d:%02d%s%s", FormatPIN(logs[i].PIN), tt.tm_mon+1, tt.tm_mday,
	                        tt.tm_hour, tt.tm_min, tt.tm_sec, FormatStatus(logs[i].status), FormatVerified(logs[i].verified));
	                }
	                if(dv->Zoom)
	                        LCDWriteStrLng(gROMDriver, row, pcol+dv->Left, line, 0);
	                else
	                        LCDWriteStr(row, pcol+dv->Left, line, 0);
	                if(IsPrint)
	                {
	                        SerialOutputString(&ff232, line);
	                }
	                showc++;
	                pcol+=strlen(line);
	                if(pcol+6>=gLCDWidth/(dv->Zoom?6:gLangDriver->CharWidth))
	                {
	                        pcol=0;
	                        row++;
	                        if(IsPrint)
                                SerialOutputString(&ff232, "\r\n");
	                        if(row>=rowc) break;
	                }
	                i+=inc;
        	}

	}

        gROMDriver->CharWidth=cw;
        LCDBufferStart(FALSE);
        dv->ViewCount=(dv->Packed?showc:rowc);
        if(dv->ViewCount<rowc) dv->ViewCount=rowc;
        return i-Data->CurrentRec;
}

int RunBrowseMsg(PMsg msg)
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
	if(IKeyOK==oldkey)
	{
		dv->Left=0;
		//输出当前屏幕显示到232
		IsPrint=TRUE;
	}
	else if('1'==oldkey)
	{
		dv->Left-=1;
	}
	else if('3'==oldkey)
	{
		if(dv->Left<0)
			dv->Left+=1;
	}
	else if('0'==oldkey)
	{
		i=0;
	}
	else if('9'==oldkey)
	{
		i=dv->DataSet.RecordCount-1;
	}
	else if('4'==oldkey)
	{
		dv->Packed=!dv->Packed;
	}
	else if('6'==oldkey)
	{
		dv->Zoom=!dv->Zoom;
	}
	else if('2'==oldkey)
	{
		i=dv->DataSet.CurrentRec+1;
	}
	else if('5'==oldkey)
	{
		i=dv->DataSet.CurrentRec-1;
	}
	else if(IKeyUp==oldkey)
	{
		i=dv->DataSet.CurrentRec+dv->ViewCount;
	}
	else if(IKeyDown==oldkey)
	{
		i=dv->DataSet.CurrentRec-dv->ViewCount;
	}
	else if(IKeyESC==oldkey)
	{
		ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, 0);
		return 1;
	}
	if(i<0)
		dv->DataSet.CurrentRec=0;
	else if(i<dv->DataSet.RecordCount)
		dv->DataSet.CurrentRec=i;
	else if(i>=dv->DataSet.RecordCount)
		dv->DataSet.CurrentRec=dv->DataSet.RecordCount-1;
	if(oldkey)
		dv->Formator(dv);
	msg->Message=0;
	msg->Param1=0;
	return 1;
}

int RunBrowse(PDataViewer DataViewer)
{
	int ret,i=RegMsgProc(RunBrowseMsg);
	U32 mm=SelectNewMsgMask(MSG_TYPE_BUTTON);
	DataViewer->Formator(DataViewer);
	InputTimeOut=0;
	ret=DoMsgProcess(DataViewer, News_Exit_Input);	
	SelectNewMsgMask(mm);
	UnRegMsgProc(i);
	LCDClear();
	return ret;	
}

int ViewAttLogByUser(U16 PIN, int TimeOut)
{
        TDataViewer dv;
        int ret,c;
	int imagesize = 800*1024;

	unsigned char *buffer = malloc(imagesize);
	
	if(buffer==NULL)
		return News_ErrorInput;
	
        PAttLog  logs=(PAttLog)buffer;
        PExtendAttLog extlogs=(PExtendAttLog)buffer;
	
	LCDClear();
	
	if (gOptions.AttLogExtendFormat)
	{
		c=imagesize/16;
		c=FDB_GetAttExtLog(PIN,0,extlogs,c);
	}
	else
	{
		c=imagesize/8;
	        c=FDB_GetAttLog(PIN,0,logs,c);
	}
        if(c==0)
        {
		free(buffer);
                LCDInfo(LoadStrByID(HID_NOATTLOG), 2);
                return News_ErrorInput;
        }
	
        dv.DataSet.CurrentRec=c-1;
        dv.DataSet.RecordCount=c;

	if (gOptions.AttLogExtendFormat)
		dv.DataSet.Data=(void*)extlogs;
	else		
		dv.DataSet.Data=(void*)logs;
        dv.Descent=TRUE;
        dv.Formator=PIN>0?AttLogFormatorPacked:AttLogFormator;
        dv.ViewCount=0;
        dv.Left=0;
        dv.Zoom=1;
        dv.Packed=PIN>0;
        dv.TimeOut=TimeOut;
        ret=RunBrowse(&dv);
	free(buffer);
        return ret;
}

