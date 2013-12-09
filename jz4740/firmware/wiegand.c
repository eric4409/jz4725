/*************************************************
                                           
 ZEM 200                                          
                                                    
 wiegand.c define wiegand format and output wiegand data  
 
                                                      
*************************************************/
#include <stdlib.h>
#include <string.h>
#include "arca.h"
#include "wiegand.h"
#include "options.h"

TWiegandBitsDef gWiegandDef={8,16,0,1,12,0,13,12,25};

int gWGFailedID;	//Wiegand失败ID
int gWGDuressID;	//Wiegand胁迫ID
int gWGSiteCode;	//Wiegand区码
int gWGOEMCode;		//OEM Code or Manufacture Code
int gWGPulseWidth;	//脉冲宽度
int gWGPulseInterval;	//脉冲间隔
int gWGOutputType;	
		
static TWiegandBitsDef Wiegand26Def={8,16,0,1,12,0,13,12,25};
static TWiegandBitsDef Wiegand34Def={8,24,0,1,12,0,13,20,33};

int InitWiegandDef(PWiegandBitsDef WiegandBitsDef, char *BitDef)
{
/*
	----------------------Wiegand 26-------------------
	0                 1                   2            
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6
	P e e e e e e e e E E E E O O O O O O O O O O O O P
	PeeeeeeeeEEEEOOOOOOOOOOOOP
	---------------------------------------------------
	-------------------------Wiegand 34--------------------------------
	0                 1                   2                   3        
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4
	P e e e e e e e e E E E E O O O O O O O O O O O O O O O O O O O O P
	PeeeeeeeeEEEEOOOOOOOOOOOOOOOOOOOOP
	-------------------------------------------------------------------
*/
	int i,len,EvenFirst=FALSE,p1=-1,p2=-1;
	TWiegandBitsDef w, *old=WiegandBitsDef;
	
	gWGFailedID=LoadInteger("WGFailedID", -1);
	gWGDuressID=LoadInteger("WGDuressID", -1);
	gWGSiteCode=LoadInteger("WGSiteCode", -1);
	gWGOEMCode=LoadInteger("WGOEMCode", -1);
	gWGPulseWidth=LoadInteger("WGPulseWidth", 100);
	gWGPulseInterval=LoadInteger("WGPulseInterval", 1000);
	gWGOutputType=LoadInteger("WGOutputType", 2); //default value is output directly
	
	if(!BitDef || BitDef[0]==0 || 0==strcmp(BitDef,"26")||0==strcmp(BitDef,"WIEGAND26")||0==strcmp(BitDef,"Wiegand26"))
	{
		memcpy((void*)WiegandBitsDef, (void*)&Wiegand26Def, sizeof(TWiegandBitsDef));
		return TRUE;
	}
	else if(0==strcmp(BitDef,"34")||0==strcmp(BitDef,"WIEGAND34")||0==strcmp(BitDef,"Wiegand34"))
	{
		memcpy((void*)WiegandBitsDef, (void*)&Wiegand34Def, sizeof(TWiegandBitsDef));
		return TRUE;
	}
	WiegandBitsDef=&w;
	len=strlen(BitDef);
	memset((void*)WiegandBitsDef, 0, sizeof(TWiegandBitsDef));
	for(i=0;i<len;i++)
	{
		switch(BitDef[i])
		{
		case 'P':
		{
			if(p1==-1) p1=i; else if(p2==-1) p2=i; else return FALSE;
			break;
		}
		case 'E':
		case 'e':
		{
			if(BitDef[i]=='e')
				WiegandBitsDef->DeviceIDLen++;
			else
				WiegandBitsDef->CardIDLen++;
			if(WiegandBitsDef->EvenParityLen==0)
			{
				WiegandBitsDef->EvenParityStart=i;
				if(WiegandBitsDef->OddParityLen==0)
					EvenFirst=TRUE;
			}
			WiegandBitsDef->EvenParityLen++;
			break;
		}
		case 'O':
		case 'o':
		{
			if(BitDef[i]=='o')
				WiegandBitsDef->DeviceIDLen++;
			else
				WiegandBitsDef->CardIDLen++;
			if(WiegandBitsDef->OddParityLen==0)
				WiegandBitsDef->OddParityStart=i;
			WiegandBitsDef->OddParityLen++;
			break;
		}
		case 'D':
		case 'd':
		{
			WiegandBitsDef->DuressIDLen++;
			if(BitDef[i]=='D')
			{
				if(WiegandBitsDef->EvenParityLen==0)
				{
					WiegandBitsDef->EvenParityStart=i;
					if(WiegandBitsDef->OddParityLen==0)
						EvenFirst=TRUE;
				}
				WiegandBitsDef->EvenParityLen++;
			}else
			{
				if(WiegandBitsDef->OddParityLen==0)
					WiegandBitsDef->OddParityStart=i;
				WiegandBitsDef->OddParityLen++;
			}
			break;
		}
		default:
			return FALSE;
		}
	}
	if(EvenFirst)
	{
		WiegandBitsDef->EvenParityPos=p1;
		WiegandBitsDef->OddParityPos=p2;
	}
	else
	{
		WiegandBitsDef->EvenParityPos=p2;
		WiegandBitsDef->OddParityPos=p1;
	}
	memcpy((void*)old, (void*)&w, sizeof(w));
	return TRUE;
}

int CalcWiegandData(U32 DeviceID, U32 CardNum,U32 DuressID, U8 *data1, PWiegandBitsDef WiegandBitsDef)
{
	int Test,i,pos,EvenParityFirst,p1c,p2c,p1s,p2s,p1p,p2p;
	
	memset(data1,'0',WiegandBitsDef->CardIDLen+WiegandBitsDef->DeviceIDLen+4);
	EvenParityFirst=(WiegandBitsDef->EvenParityLen>0) && (WiegandBitsDef->EvenParityStart<WiegandBitsDef->OddParityStart);
	p1c=(EvenParityFirst?WiegandBitsDef->EvenParityLen:WiegandBitsDef->OddParityLen);
	p2c=(WiegandBitsDef->CardIDLen+WiegandBitsDef->DeviceIDLen-p1c);
	p1s=EvenParityFirst?WiegandBitsDef->EvenParityStart:WiegandBitsDef->OddParityStart;
	p2s=(!EvenParityFirst)?WiegandBitsDef->EvenParityStart:WiegandBitsDef->OddParityStart;
	p1p=EvenParityFirst?WiegandBitsDef->EvenParityPos:WiegandBitsDef->OddParityPos;
	p2p=(!EvenParityFirst)?WiegandBitsDef->EvenParityPos:WiegandBitsDef->OddParityPos;
	pos=0;
	//生成DeviceID的位串
	Test=1<<(WiegandBitsDef->DeviceIDLen-1);
	for(i=0;i<WiegandBitsDef->DeviceIDLen;i++)
	{
		if(pos==p1p || pos==p2p) pos++; //越过校验位的位置
		if(DeviceID & Test)
			data1[pos]='1';
		pos++;
		Test>>=1;
	}
	//生成CardNum的位串
	Test=1<<(WiegandBitsDef->CardIDLen-1);
	for(i=0;i<WiegandBitsDef->CardIDLen;i++)
	{
		if(pos==p1p || pos==p2p) pos++; //越过校验位的位置
		if(CardNum & Test)
			data1[pos]='1';
		pos++;
		Test>>=1;
	}
	//生成Duress的位串
	Test=1<<(WiegandBitsDef->DuressIDLen -1);
	for(i=0;i<WiegandBitsDef->DuressIDLen;i++)
	{
		if(pos==p1p || pos==p2p) pos++; //越过校验位的位置
		if(DuressID & Test)
			data1[pos]='1';
		pos++;
		Test>>=1;
	}
	//计算奇检验
	if(WiegandBitsDef->OddParityLen>0)
	{
		Test=0;
		pos=WiegandBitsDef->OddParityStart;
		for(i=0;i<WiegandBitsDef->OddParityLen;i++)
		{
			Test+=data1[pos++];
		}
		if((Test & 1)==0) data1[WiegandBitsDef->OddParityPos]='1';
	}
	//计算偶检验
	if(WiegandBitsDef->EvenParityLen>0)
	{
		Test=0;
		pos=WiegandBitsDef->EvenParityStart;
		for(i=0;i<WiegandBitsDef->EvenParityLen;i++)
		{
			Test+=data1[pos++];
		}
		if(Test & 1) data1[WiegandBitsDef->EvenParityPos]='1';
	}
	return WiegandBitsDef->CardIDLen+WiegandBitsDef->DeviceIDLen+WiegandBitsDef->DuressIDLen+(WiegandBitsDef->OddParityLen>0?1:0)+(WiegandBitsDef->EvenParityLen>0?1:0);
}

//HID Wiegand 37
int CalcHID37WiegandData(U32 OEMCode, U32 DeviceID, U32 CardNum, U32 DuressID, U8 *data1)
{
	int OEMCodeLen=4;
	int SiteCodeLen=12;
	int CardNumLen=19;
	int OddStart=2, OddEnd=20;
	int EvenStart=21, EvenEnd=36;
	int i, parity;
		
	for(i=0;i<OEMCodeLen;i++)
	{
		if(OEMCode&(1<<i))
			data1[OEMCodeLen-i]='1';
		else
			data1[OEMCodeLen-i]='0';	
	}

	for(i=0;i<SiteCodeLen;i++)
	{
		if(DeviceID&(1<<i))
			data1[SiteCodeLen+OEMCodeLen-i]='1';
		else
			data1[SiteCodeLen+OEMCodeLen-i]='0';		
	}

	for(i=0;i<CardNumLen;i++)
	{
		if(CardNum&(1<<i))
			data1[CardNumLen+OEMCodeLen+SiteCodeLen-i]='1';
		else
			data1[CardNumLen+OEMCodeLen+SiteCodeLen-i]='0';
	}

	parity=0;
	for(i=OddStart;i<=OddEnd;i++)
	{
		if(data1[i-1]=='1') parity++;
	}
	if(parity&1)
		data1[0]='0';
	else
		data1[0]='1';

	
	parity=0;
	for(i=EvenStart;i<=EvenEnd;i++)
	{
		if(data1[i-1]=='1') parity++;
	}
	if((parity&1)==0)
		data1[OEMCodeLen+SiteCodeLen+CardNumLen+1]='0';
	else
		data1[OEMCodeLen+SiteCodeLen+CardNumLen+1]='1';

	return (OEMCodeLen+SiteCodeLen+CardNumLen+2);
}

