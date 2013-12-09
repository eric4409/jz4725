/*************************************************
                                           
 ZEM 200                                          
                                                    
 FPCardMng.c  all functions for Mifare manage                                
                                                      
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
#include "zlg500b.h"
#include "fpcardmng.h"
#include "lcm.h"
#include "kb.h"

//把 Templates 指定的数个指纹，压缩放到Temp中，然后返回新的长度
int PackTemplates(U8* Temp, U8 *Templates[], int TempCount, int ResSize)
{
	int TempLen[10],NewLen[10];
	int i, size=0, newsize=0;
	U8 tmp[1024];
	for(i=0;i<TempCount;i++)
	{
		if (gOptions.ZKFPVersion == ZKFPV10)
			TempLen[i]=BIOKEY_TEMPLATELEN_10(Templates[i]);
		else
			TempLen[i]=BIOKEY_TEMPLATELEN(Templates[i]);
		NewLen[i]=TempLen[i];
		size+=TempLen[i];
	}
	newsize=0;
	if(size>ResSize)
	{
		for(i=0;i<TempCount;i++)
		{
			NewLen[i]=TempLen[i]*ResSize/size;
			memcpy(tmp, Templates[i], TempLen[i]);
			if (gOptions.ZKFPVersion == ZKFPV10)
				NewLen[i]=BIOKEY_SETTEMPLATELEN_10(tmp, NewLen[i]);
			else
				NewLen[i]=BIOKEY_SETTEMPLATELEN(tmp, NewLen[i]);
			memcpy(Temp+newsize, tmp, NewLen[i]);
			newsize+=NewLen[i];
		}
	}
	else
		for(i=0;i<TempCount;i++)
		{
			memcpy(Temp+newsize, Templates[i], TempLen[i]);
			newsize+=TempLen[i];
		}
	return newsize;
}

int RunWaitCardAndWriteTemp(PMsg msg)
{
	int i;
	PFPCardOP tmp=(PFPCardOP)(msg->Object);
	char buf[40];
	if(MSG_TYPE_TIMER==msg->Message && InputTimeOut>=0)
	{
		msg->Message=0;
		msg->Param1=0;
		if(++InputTimeOut>=InputTimeOutSec)
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_TimeOut);
		}
		else if(gMachineState!=STA_WRITEMIFARE)
                {
                        ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input,News_CancelInput);
                }
		return 1;
	}
	else if(MSG_TYPE_MF==msg->Message)
	{
		ExLightLED(LED_RED, FALSE);
		ExLightLED(LED_GREEN, FALSE);
		InputTimeOut=0;
		if(tmp->OP==OP_WRITE)
			i=MFWrite(tmp);
		else if(tmp->OP==OP_EMPTY)
			i=MFEmpty();
		else
			i=MFRead(tmp, FALSE);
		if(i>=0)
		{
			ExLightLED(LED_GREEN, TRUE);
			MFFinishCard();
			ExBeep(1);
			//ExPlayVoice(VOICE_OK);
			if(tmp->OP!=OP_READ)
			{
				LCDWriteCenterStrID(2, HID_WRITE_OK);
				DelayMS(2*1000);
			}
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input,News_CommitInput);
		}
		else
		{
			ExLightLED(LED_RED, TRUE);
			msg->Message=0;
			if(tmp->OP!=OP_READ)
			{
				sprintf(buf, "%s: %d", LoadStrByID(HID_WRITE_ERROR), i);
				LCDWriteCenterStr(2,buf);
				DelayMS(100);
			}
			else
			{
				if(i!=MFCARD_ERROR_READ)
				{
					ExBeep(2);
					LCDWriteCenterStrID(2,HID_INVALIDCARD);	
					DelayMS(500);
					LCDWriteCenterStrID(2,HID_SHOWCARD);
				}
			}
		}
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

//Enroll a fingerprint template. store it to tmp and return the length of template.
int WaitCardAndWriteTemp(PFPCardOP tmp)
{
	U32 mm;
	int ret,i;
	int OldState=gMachineState;
	InputTimeOut=0;
	gMachineState=STA_WRITEMIFARE;
	LCDWriteCenterStrID(2,HID_SHOWCARD);
	LCDWriteCenterStrID(3,HID_ESC);
	i=RegMsgProc(RunWaitCardAndWriteTemp);
	mm=SelectNewMsgMask(MSG_TYPE_BUTTON|MSG_TYPE_MF|MSG_TYPE_TIMER);
	ret=DoMsgProcess(tmp, News_Exit_Input);
	SelectNewMsgMask(mm);
	UnRegMsgProc(i);
	gMachineState=OldState;
	return ret;
}

int DoCreateCard(void *p)
{
	PUser u;
	TUser user;
	int ret;
	user.PIN=1;
	do
	{
		if(FDB_CntUser()==0)
		{
			LCDWriteCenterStrID(1, HID_NOTENROLLED);
			DelayMS(2000);
			return News_ErrorInput;
		}
		LCDWriteCenterStr(1,"");
		ret=InputPINBox(LoadStrByID(MID_DC_CREATE),LoadStrByID(HID_OKCANCEL), 0, &user);
		if(News_CancelInput==ret || News_TimeOut==ret) return ret;
		if(ret==News_ErrorInput) continue;
		else if(ret==News_CommitInput)
		{
			u=FDB_GetUser(user.PIN, NULL);
			if(u)
			{
				int fid, fc=0;
				U8 Templates[4][1024];
				TFPCardOP tmp;
				U8 TMP[10240];
				TTemplate Temp;
				
				tmp.Templates=TMP;
				tmp.OP=OP_WRITE;
				if(gOptions.PIN2Width==PIN_WIDTH)
					tmp.PIN=user.PIN;
				else
					tmp.PIN=user.PIN2;
				memset(tmp.Finger, 0xFF, 4);
				for(fid=0;fid<gOptions.MaxUserFingerCount;fid++)
				{
					if(FDB_GetTmp(user.PIN, (char)fid, &Temp))
					{
						memcpy(Templates[fc], Temp.Template, Temp.Size);
						tmp.Finger[fc]=fid;
						fc++;
						if(fc>=gOptions.RFCardFPC) break;
					}
				}	
				if(fc>0)
				{
					U8 *(t[4]);
					int j;
					t[0]=Templates[0];t[1]=Templates[1];t[2]=Templates[2];t[3]=Templates[3];
					tmp.TempSize=PackTemplates(tmp.Templates, t, fc, MFGetResSize()-8);
					j=WaitCardAndWriteTemp(&tmp);
					if(News_TimeOut==j) return News_TimeOut;
					FDB_AddOPLog(ADMINPIN, OP_MF_CREATE, tmp.PIN, j, fc, tmp.TempSize);
					break;
				}
				else
				{
					LCDWriteCenterStrID(2, HID_NOFINGER);
					DelayMS(5*1000);
				}
			}
			else
				ExBeep(2);
		}
	}
	while(1);
	//ret==News_CommitInput
	return ret;
}

int DoCardMng(void *p)
{
	//卡管理
	int ret;
	static PMenu m23;
	m23=CreateMenu(LoadStrByID(MID_DATA_CARD),gOptions.MenuStyle,NULL,m23);
	//生成号码卡
	AddMenuItem(1, m23, LoadStrByID(MID_DC_PIN), DoCreatePINCard, NULL);
	if(gOptions.RFCardFPC>0)
	{
		//登记指纹卡，新登记指纹，并把指纹存到卡上
		AddMenuItem(1, m23, LoadStrByID(MID_DC_ENROLL), DoEnrollCard, NULL);
		//生成指纹卡 - 把已经登记的指纹存到卡上
		AddMenuItem(1, m23, LoadStrByID(MID_DC_CREATE), DoCreateCard, NULL);
	}
	//注册指纹卡 - 把其他地方登记的指纹卡注册到本机
	AddMenuItem(1, m23, LoadStrByID(MID_DC_REG), DoRegisterCard, NULL);
	//注销指纹卡 - 把指纹卡从本机上注销
	AddMenuItem(1, m23, LoadStrByID(MID_DC_UNREG), DoUnRegCard, NULL);
	//清空指纹卡 - 清除指纹卡上的指纹数据，恢复默认的扇区密码
	AddMenuItem(1, m23, LoadStrByID(MID_DC_EMPTY), DoEmptyCard, NULL);
	if(gOptions.RFCardFPC>0)
	{
		//复制卡内数据 - 把卡上的指纹复制到机器中
		AddMenuItem(1, m23, LoadStrByID(MID_DC_DUMPFP), DoDumpFPCard, NULL);
		//转移指纹到卡内 - 把卡上的指纹复制到卡内, 然后删除机器内的指纹
		AddMenuItem(1, m23, LoadStrByID(MID_DC_MOVEFP), DoMoveToCard, NULL);
	}
	ret=RunMenu(m23);
	DestroyMenu(m23);
	return ret;
}

int DoEnrollCard(void *p)
{
	int i, j, ret, fc, tmplens[4];
	U8 tmps[4][1024];
	TUser u, user;
	TFPCardOP tmp;
	while(1)
	{
		memset((void*)&u, 0, sizeof(TUser));
		memset((void*)&user, 0, sizeof(TUser));
		LCDWriteCenterStr(1, "");
		i=InputPINBox(LoadStrByID(MID_DC_ENROLL),LoadStrByID(HID_OKCANCEL), INPUT_USER_NEW, &user);
		if(i==News_ErrorInput)
		{
			LCDWriteCenterStrID(3,HID_ERRORPIN);
			DelayMS(3*1000);
			continue;
		}
		else if(i==News_CancelInput || i==News_TimeOut)
			return i;
		if(gOptions.PIN2Width!=PIN_WIDTH)
			user.PIN=GetNextPIN(1, TRUE);
		if(user.PIN2==0) user.PIN2=user.PIN;
		fc=0;
		memset(tmp.Finger, 0xff, 4);
		while(fc<gOptions.RFCardFPC)
		{
			tmplens[fc]=0;
			ret=EnrollAFinger((char*)tmps[fc], tmplens+fc, user.PIN2, fc);
			if(ret!=News_CommitInput) return ret;
			if(tmplens[fc]==0) //enroll fail
			{
				if(gOptions.VoiceOn) ExPlayVoice(VOICE_RETRY_FP);
				LCDWriteCenterStrID(2,HID_INPUTAGAIN);
				DelayMS(3*1000);
				continue;
			}
			tmp.Finger[fc]=fc;
			fc++;
			if(fc>=gOptions.RFCardFPC) break;
			ret=LCDSelectOK(LoadStrByID(MID_DC_ENROLL), LoadStrByID(HID_CONTINUE), LoadStrByID(HID_YESNO));
			if(ret==News_CommitInput) continue;
			if(ret==News_CancelInput) break; else return ret;
		}
		if(fc>0)
		{
			U8 *(t[4]);
			U8 TMP[10240];
			char buf[20];
			char fmt[20];
			t[0]=tmps[0]; t[1]=tmps[1]; t[2]=tmps[2]; t[3]=tmps[3];
			tmp.Templates=TMP;
			tmp.TempSize = PackTemplates(tmp.Templates, t, fc, MFGetResSize()-8);
			tmp.OP=OP_WRITE;
			if(gOptions.PIN2Width==PIN_WIDTH)
				tmp.PIN=user.PIN;
			else
				tmp.PIN=user.PIN2;
			sprintf(fmt, "%%0%dd:%%d", gOptions.PIN2Width);
			sprintf(buf, fmt, tmp.PIN, fc);
			LCDWriteCenterStr(1, buf);
			ret=WaitCardAndWriteTemp(&tmp);
			if(ret==News_TimeOut) return ret;
			if(ret==News_CancelInput) return ret;
			FDB_CreateUser(&u, user.PIN, NULL, NULL, 0);
			if(gOptions.PIN2Width==PIN_WIDTH)
				u.PIN2=user.PIN;
			else
				u.PIN2=user.PIN2;
			nmemcpy(u.IDCard, user.IDCard, sizeof(u.IDCard));
			j=FDB_AddUser(&u);
			FDB_AddOPLog(ADMINPIN, OP_MF_ENROLL, u.PIN, j, fc, 0);
			if(FDB_OK!=j)
			{
				LCDWriteCenterStrID(2, HID_FLASHERROR);
				DelayMS(3*1000);
				ExBeep(2);
				break;
			}
		}
	}
	return ret;
}

int DoRegisterCard(void *p)
{
	TFPCardOP tmp;
	int ret;
	TUser u;
	BYTE TMP[10240];
	tmp.Templates=TMP;
	char fmt[32];
	BOOL rc=TRUE;
	
	LCDWriteCenterStr(1,NULL);
	LCDWriteLineStrID(0,MID_DC_REG);
	while(1)
	{
		tmp.OP=OP_READ;
		ret=WaitCardAndWriteTemp(&tmp);
		if(ret==News_TimeOut || ret==News_CancelInput) return ret;
		if(gOptions.PIN2Width==PIN_WIDTH)
		{
			if(!FDB_GetUser(tmp.PIN, NULL))
			{
				FDB_CreateUser(&u, tmp.PIN, NULL, NULL, 0);
				rc=(FDB_AddUser(&u)==FDB_OK);
			}
		}
		else
		{
			if(!FDB_GetUserByPIN2(tmp.PIN, NULL))
			{
				FDB_CreateUser(&u, GetNextPIN(1, TRUE), NULL, NULL, 0);
				u.PIN2=tmp.PIN;
				rc=(FDB_AddUser(&u)==FDB_OK);
			}
		}		
		if(rc)
		{
			FDB_AddOPLog(ADMINPIN, OP_MF_REG, tmp.PIN, 0, 0, 0);
			if(gOptions.PIN2Width==PIN_WIDTH)			
				sprintf((char*)TMP,"%s:%05d", LoadStrByID(HID_ENROLLNUM), tmp.PIN);
			else
			{
				sprintf(fmt, "%%s:%%0%dd", gOptions.PIN2Width);
				sprintf((char*)TMP, fmt, LoadStrByID(HID_PIN2), tmp.PIN);
			}
			LCDWriteCenterStr(2, (char*)TMP);
			DelayMS(2*1000);
			break;
		}
		else
		{
			LCDWriteCenterStrID(2, HID_FLASHERROR);
			FDB_AddOPLog(ADMINPIN, OP_MF_REG, tmp.PIN, 1, 0, 0);
		}
		DelayMS(2*1000);				
	}
	return ret;
}

int DoUnRegCard(void *p)
{
	TFPCardOP tmp;
	int ret;
	BYTE TMP[1024];
	BOOL rc=FALSE;
	char fmt[32];
	TUser u;
	tmp.Templates=TMP;
	LCDWriteCenterStr(1,NULL);
	LCDWriteLineStrID(0,MID_DC_UNREG);
	while(1)
	{
		tmp.OP=OP_READ;
		ret=WaitCardAndWriteTemp(&tmp);
		if(ret==News_TimeOut || ret==News_CancelInput) return ret;
		if(gOptions.PIN2Width==PIN_WIDTH)
			rc=(FDB_OK==FDB_DelUser(tmp.PIN));
		else
			rc=(FDB_GetUserByPIN2(tmp.PIN, &u)&&(FDB_OK==FDB_DelUser(u.PIN)));
		if(rc)
		{
			FDB_AddOPLog(ADMINPIN, OP_MF_UNREG, tmp.PIN, 0, 0, 0);
			if(gOptions.PIN2Width==PIN_WIDTH)
				sprintf((char*)TMP,"%s:%05d", LoadStrByID(HID_ENROLLNUM), tmp.PIN);
			else
			{
				sprintf(fmt, "%%s:%%0%dd", gOptions.PIN2Width);
				sprintf((char*)TMP, fmt, LoadStrByID(HID_PIN2), tmp.PIN);
			}
			LCDWriteCenterStr(2, (char*)TMP);
			DelayMS(2*1000);
			break;
		}
		else
		{
			FDB_AddOPLog(ADMINPIN, OP_MF_UNREG, tmp.PIN, 1, 0, 0);
			LCDWriteCenterStrID(2, HID_FLASHERROR);
		}
		DelayMS(2*1000);			
	}
	return ret;
}

int DoEmptyCard(void *p)
{
	TFPCardOP tmp;
	int ret;
	BYTE TMP[1024];
	tmp.Templates=TMP;
	LCDWriteCenterStr(1,NULL);
	LCDWriteLineStrID(0,MID_DC_EMPTY);
	tmp.OP=OP_EMPTY;
	ret=WaitCardAndWriteTemp(&tmp);
	FDB_AddOPLog(ADMINPIN, OP_MF_CLEAR, tmp.PIN, ret, 0, 0);
	if(ret!=News_CommitInput) return ret;
	return ret;
}

int DoDumpFPCard(void *p)
{
	TFPCardOP tmp;
	int ret;
	BYTE TMP[10240];
	TUser u;
	BOOL rc=TRUE;
	char fmt[32];
	
	tmp.Templates=TMP;
	LCDWriteCenterStr(1,NULL);
	LCDWriteLineStrID(0,MID_DC_DUMPFP);
	tmp.OP=OP_READ;
	ret=WaitCardAndWriteTemp(&tmp);
	if(ret!=News_CommitInput) return ret;
	
	if(gOptions.PIN2Width==PIN_WIDTH)
	{
		if(!FDB_GetUser(tmp.PIN, &u))
		{
			FDB_CreateUser(&u, tmp.PIN, NULL, NULL, 0);
			rc=(FDB_AddUser(&u)==FDB_OK);
		}
	}
	else
	{
		if(!FDB_GetUserByPIN2(tmp.PIN, &u))
		{
			FDB_CreateUser(&u, GetNextPIN(1, TRUE), NULL, NULL, 0);
			u.PIN2=tmp.PIN;
			rc=(FDB_AddUser(&u)==FDB_OK);
		}
	}		
	if(rc)
	{
		TTemplate t;
		int i,offset=0,len,fc=0;
		for(i=0;i<4;i++)
		if(tmp.TempSize>0)
		{
			if (gOptions.ZKFPVersion == ZKFPV10)
				len=BIOKEY_TEMPLATELEN_10(tmp.Templates+offset);
			else
				len=BIOKEY_TEMPLATELEN(tmp.Templates+offset);
			FDB_CreateTemplate(&t, u.PIN, tmp.Finger[i], (char*)tmp.Templates+offset, len);
			if(FDB_OK==FDB_AddTmp(&t)) fc++;
			offset+=len;
			tmp.TempSize-=len;
		}
		else
			break;
		FDB_AddOPLog(ADMINPIN, OP_MF_DUMP, tmp.PIN, 0, fc, 0);
		if(fc)
		{
			if(gOptions.PIN2Width==PIN_WIDTH)
				sprintf((char*)TMP,"%s:%05d", LoadStrByID(HID_ENROLLNUM), tmp.PIN);
			else
			{
				sprintf(fmt, "%%s:%%0%dd", gOptions.PIN2Width);
				sprintf((char*)TMP, fmt, LoadStrByID(HID_PIN2), tmp.PIN);
			}
			LCDWriteCenterStr(2, (char*)TMP);
			sprintf((char*)TMP,"Count: %d", fc);
			LCDWriteCenterStr(3, (char*)TMP);
		}
		else
			LCDWriteCenterStrID(2, HID_FLASHERROR);			
		FPDBInit();  //reload data
	}
	else
	{
		FDB_AddOPLog(ADMINPIN, OP_MF_DUMP, tmp.PIN, 1, 0, 0);
		LCDWriteCenterStrID(2, HID_FLASHERROR);
	}
	DelayMS(2*1000);			
	return ret;
}

int DoMoveToCard(void *p)
{
	PUser u;
	TUser user;
	int ret;
	while(1)
	{
		LCDClear();
		if(FDB_CntUser()==0)
		{
			LCDWriteCenterStrID(1, HID_NOTENROLLED);
			DelayMS(2000);
			return News_ErrorInput;
		}
		LCDWriteCenterStrID(0, MID_DC_MOVEFP);
		user.PIN=GetNextPIN(1, FALSE);
		if(!FDB_GetUser(user.PIN, &user))
		{
			LCDWriteCenterStrID(2, HID_NOTENROLLED);
			DelayMS(2000);
			return News_ErrorInput;
		}
		ret=InputPINBox(LoadStrByID(MID_DC_MOVEFP),LoadStrByID(HID_OKCANCEL), 0, &user);
		if(News_CancelInput==ret || News_TimeOut==ret) break;
		if(ret==News_ErrorInput) continue;
		else if(ret==News_CommitInput)
		{
			u=FDB_GetUser(user.PIN,NULL);
			if(u)
			{
				int fid, fc=0;
				U8 Templates[4][1024];
				TFPCardOP tmp;
				U8 TMP[10240];
				TTemplate Temp;
				tmp.Templates=TMP;
				tmp.OP=OP_WRITE;
				if(gOptions.PIN2Width==PIN_WIDTH)
					tmp.PIN=user.PIN;
				else
					tmp.PIN=user.PIN2;					
				memset(tmp.Finger, 0xFF, 4);
				for(fid=0;fid<gOptions.MaxUserFingerCount;fid++)
				{
					if(FDB_GetTmp(user.PIN, (char)fid, &Temp))
					{
						memcpy(Templates[fc], Temp.Template, Temp.Size);
						tmp.Finger[fc]=fid;
						fc++;
						if(fc>=gOptions.RFCardFPC) break;
					}
				}
				if(fc>0)
				{
					U8 *(t[4]);
					t[0]=Templates[0];t[1]=Templates[1];t[2]=Templates[2];t[3]=Templates[3];
					tmp.TempSize=PackTemplates(tmp.Templates, t, fc, MFGetResSize()-8);
					ret=WaitCardAndWriteTemp(&tmp);
					FDB_AddOPLog(ADMINPIN, OP_MF_MOVE, tmp.PIN, ret, fc, 0);
					if(ret==News_CommitInput)
					{
						while(fc--)
							FDB_DelTmp(user.PIN, (char)tmp.Finger[fc]);
					}
					break;
				}
				else
				{
					ExBeep(2);
					FDB_AddOPLog(ADMINPIN, OP_MF_DUMP, tmp.PIN, 0, 0, 0);
					LCDWriteCenterStrID(2, HID_NOFINGER);
					DelayMS(2000);
				}
			}
			else
				ExBeep(2);
		}
	}
	FPDBInit();  //reload data
	return ret;
}

int DoCreatePINCard(void *p)
{
	PUser u;
	TUser user;
	int ret;
	do
	{
		if(FDB_CntUser()==0)
		{
			LCDWriteCenterStrID(1, HID_NOTENROLLED);
			DelayMS(2000);
			return News_ErrorInput;
		}
		memset((void*)&user, 0, sizeof(user));
		user.PIN=1;
		LCDWriteCenterStr(1, "");
		ret=InputPINBox(LoadStrByID(MID_DC_PIN),LoadStrByID(HID_OKCANCEL), 0, &user);
		if(News_CancelInput==ret || News_TimeOut==ret) return ret;
		if(ret==News_ErrorInput) continue;
		else if(ret==News_CommitInput)
		{
			u=FDB_GetUser(user.PIN,NULL);
			if(u)
			{
				TFPCardOP tmp;
				int j;
				if(gOptions.PIN2Width==PIN_WIDTH)
					tmp.PIN=user.PIN;
				else
					tmp.PIN=user.PIN2;
				tmp.Finger[0]=(char)NOFINGERTAG;
				tmp.TempSize=0;
				tmp.OP=OP_WRITE;
				j=WaitCardAndWriteTemp(&tmp);
				FDB_AddOPLog(ADMINPIN, OP_MF_CREATE, tmp.PIN, j, 0, 0);
				if(News_TimeOut==j) 
					return News_TimeOut;
			}
			else
				ExBeep(2);
		}
	}
	while(1);
	//ret==News_CommitInput
	return ret;
}
