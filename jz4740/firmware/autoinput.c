/*************************************************
                                           
 ZEM 200                                          
                                                    
 autoinput.c Design for ZhenZhou TKS                              
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
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
#include "fpcardmng.h"
#include "commu.h"
#include "lcm.h"

/*定制输入项定义
V序号=名称:类型:宽度:最小值:最大值:值
序号：从0到99
名称：显示提示的输入项目名称，1-10个字符
类型：输入数据的类型，整型I，时间t，日期d，日期时间D，IP地址P，是否B，
宽度：输入数据的最大宽度
最小值：
最大值：
实际值：
*/
typedef struct _CUSTVALUE_{
	int Index;
	char Name[20];
	int ValueType;
	int Width;
	int Min;
	int Max;
	int Value[6];
	int Modified;
}TCustValue, *PCustValue;


int GetCustValueCount(void)
{
	int res=0,i;
	char p[128],buf[20];
	for(i=0;i<100;i++)
	{
		sprintf(buf,"V%d",i);
		if (LoadStr(buf, p))
		    if(*p) res++;
	}
	return res;
}

int FetchCustValue(PCustValue v, char *p)
{
	int i,index,j;
	strncpy(v->Name,p,20);
	for(i=0;i<20;i++)
		if(':'==v->Name[i])
		{
			v->Name[i]=0;
			break;
		}
	i++;
	if(i>=20) return FALSE;
	v->ValueType=p[i]; i+=2;
	v->Width=p[i]-'0';i+=2;
	v->Min=StrValue(p+i,&index);i+=index+1;
	v->Max=StrValue(p+i,&index);i+=index+1;
	memset(v->Value,0,sizeof(v->Value));
	for(j=0;j<6;j++)
	{
		v->Value[j]=StrValue(p+i,&index);
		if(index<=0)
			break;
		else
			i+=index+1;
	}
	return TRUE;
}

char *FormatCustValue(PCustValue v, char *p)
{
	char buf[20];
	buf[0]=v->ValueType;
	buf[1]=0;
	if(v->ValueType=='I' || v->ValueType=='B')
		sprintf(p,"%s:%s:%d:%d:%d:%d", v->Name, buf, v->Width, v->Min, v->Max, v->Value[0]);
	else if(v->ValueType=='t')
		sprintf(p,"%s:%s:%d:%d:%d:%d:%d", v->Name, buf, v->Width, v->Min, v->Max, v->Value[0], v->Value[1]);
	else
		sprintf(p,"%s:%s:%d:%d:%d:%d:%d:%d:%d:%d:%d", v->Name, buf, v->Width, v->Min, v->Max, 
			v->Value[0], v->Value[1],v->Value[2], v->Value[3],v->Value[4], v->Value[5]);
	return p;
}

char *ShowCustValue(PCustValue v, char *p)
{
	char buf[10];
	if(v->ValueType=='I')
	{
		sprintf(buf,"%%-%ds%%%dd", gLCDCharWidth-2-v->Width,v->Width); 
		sprintf(p, buf, v->Name, v->Value[0]);
	}
	else if(v->ValueType=='t')
	{
		sprintf(p, "%-9s%2d:%2d", v->Name, v->Value[0],v->Value[1]);
	}
	else if(v->ValueType=='B')
	{
		sprintf(p, "%-11s%3s", v->Name, GetYesNoName(v->Value[0]));	
	}
	else
		sprintf(p, "%-14s", v->Name);
	return p;
}

static PCustValue gCValues=NULL;

int DoSetCustValue(void *p)
{
	PCustValue cv=gCValues+(((PMenu)((PMsg)p)->Object)->ItemIndex);
	int ret, OldSaved=gOptions.Saved;
	
	gOptions.Saved=TRUE;
	ret=News_ErrorInput;
	if(cv->ValueType=='B')
		ret=InputYesNoItem((PMsg)p,&(cv->Value[0]));
	else if(cv->ValueType=='I')
		ret=InputValueOfItem((PMsg)p, cv->Width, cv->Min, cv->Max, &(cv->Value[0]));
	else if(cv->ValueType=='t')
	{
		ret=InputTime(((PMenu)((PMsg)p)->Object)->ItemIndex-((PMenu)((PMsg)p)->Object)->ItemIndex+1,
			gLCDCharWidth-5, &cv->Value[0], &cv->Value[1]);
		if(ret==News_CommitInput)
		{
			cv->Modified=TRUE;
			ShowCustValue(cv, ((PMenu)((PMsg)p)->Object)->Title);
		}
	}
	else if(cv->ValueType=='P')
	{
		ret=InputIPAddress(cv->Name, cv->Value);
		cv->Modified=ret==News_CommitInput;
	}
	if(!gOptions.Saved) cv->Modified=TRUE;
	gOptions.Saved=OldSaved;
	return ret;
}

int DoSetCustValues(void *p)
{
	//定制输入项
	char buf[32], cbuf[20][32];//最多20项 定制输入项
	TCustValue CustValues[20];
	int CVCount=0,i,ret=0;
	char cvp[128];
	static PMenu m21=NULL;
	m21=CreateMenu(LoadStrByID(MID_OS_CUST),gOptions.MenuStyle,NULL, m21);
	gCValues=CustValues;
	for(i=0;i<100;i++)
	{
		sprintf(buf,"V%d",i);
		if (LoadStr(buf, cvp))
		    if(*cvp) 
			if(FetchCustValue(&CustValues[CVCount], cvp))
			{
			     CustValues[CVCount].Index=i;
			     CustValues[CVCount].Modified=FALSE;
			     ShowCustValue(CustValues+CVCount, cbuf[CVCount]);
			     AddMenuItem(0, m21, cbuf[CVCount], DoSetCustValue, NULL);
			     CVCount++;
			 }
	}
	ret=RunMenu(m21);
	DestroyMenu(m21);
	for(i=0;i<CVCount;i++) if(CustValues[i].Modified) break;
	if(ret!=News_TimeOut && (i<CVCount))
	{
		ret=LCDSelectOK(LoadStrByID(MID_OS_CUST),LoadStrByID(HID_SAVEQ), LoadStrByID(HID_SAVECANCEL));
		if(News_CommitInput==ret)
		{
			for(;i<CVCount;i++)
			if(CustValues[i].Modified)
			{
				char VBuf[200];
				sprintf(buf,"V%d",CustValues[i].Index);
				FormatCustValue(CustValues+i, VBuf);
				SaveStr(buf, VBuf, TRUE);
			}
		}
	}
	return ret;
}
