/*************************************************
                                           
 ZEM 200                                          
                                                    
 commu.c communication for PC                             
                                                      
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
                                                      
*************************************************/
#include <stdlib.h>
#include <string.h>
#include <asm/unaligned.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <sys/ioctl.h>
//#include <fcntl.h>
#include "arca.h"
#include "main.h"
#include "utils.h"
#include "commu.h"
#include "msg.h"
#include "flashdb.h"
#include "lcdmenu.h"
#include "options.h"
#include "exfun.h"
#include "sensor.h"
#include "serial.h"
#include "accdef.h"
#include "tempdb.h"
#include "lcm.h"
#include "kb.h"
#include "finger.h"
#if MACHINE_ID == 2
#include "integer.h"
#endif
#include "minilzo.h"
#include "zlg500b.h"

#define FCT_SYSTEM_NONE (U8)0xF0
#define MAX_CACHE_SIZE	(64*1024)

int CommSessionCount=0;
PCommSession CommSessions;
unsigned short in_chksum(unsigned char *p, int len);
void SendLargeData(char *in_buf, PCommSession session, char *buf, int size);
static int MaxCommSessionCount=0;

extern PRTLogNode rtloglist;
extern PRTLogNode rtlogheader;

int RTLogNum=0;
char* LastRTLogBuf=NULL;
int LastRTLogBufLen=0;
static unsigned char Reboot_Power = 0;

int SessionSendMsg(PCommSession session)
{
        char buf[1024];
        int start, isize, size;
        if(session->MsgCount>0)
        {
                start=session->MsgStartAddress[session->MsgLast];
                size=session->MsgLength[session->MsgLast];
                isize=(MAX_MSGBUFFER_SIZE-start);
                if(isize>=size) 
                        memcpy(buf, session->MsgBuffer+start, size);
                else
                {
                        memcpy(buf, session->MsgBuffer+start, isize);
                        memcpy(buf+isize, session->MsgBuffer, size-isize);
                }
                DBPRINTF("SEND MSG: Index=%d, isize=%d, size=%d, Start=%d\n",session->MsgLast, isize, size, start);
                session->Send(buf, size, session->Sender);
        }
	return TRUE;
}

int CheckSessionSendMsg(void)
{
        int i;
        for(i=0;i<CommSessionCount;i++)
                if(CommSessions[i].MsgCached)
                {
                        SessionSendMsg(CommSessions+i);
                }
	return TRUE;
}

//消息的缓冲可以看成一个先进先出队列
//把消息压入，返回-1表示错误，其他表示正确
int SessionPushInAMsg(PCommSession session, PCmdHeader Data, int Len)
{
        int Res=MAX_MSGBUFFER_SIZE, index, Address, bsize, i; //缓存区剩余的字节数
        if(session->MsgCount==0)
        {
                index=session->MsgLast;
                Address=0;
        }
        else if(session->MsgCount>=MAX_BUFFER_MSG)      //已缓存的消息太多
        {
                session->MsgCached=0;	
                return -1;
        }
        else
        {
                Res=0;
                index=session->MsgLast;
                for(i=0;i<session->MsgCount;i++)
                {
                        Res+=session->MsgLength[index];
                        index++;
                        if(index>=MAX_BUFFER_MSG) index-=MAX_BUFFER_MSG;
                }
                Address=Res+session->MsgStartAddress[session->MsgLast];
                if(Address>=MAX_MSGBUFFER_SIZE) Address-=MAX_MSGBUFFER_SIZE;
                Res=MAX_MSGBUFFER_SIZE-Res;
        }

        if(Res<Len)     //缓存区空间不够
        {//释放旧的消息
                int sindex=0;
                for(i=0;i<session->MsgCount;i++)
                {
                        sindex=session->MsgLast+i;
                        if(sindex>=MAX_BUFFER_MSG) sindex-=MAX_BUFFER_MSG;
                        Res+=session->MsgLength[sindex];
                        if(Res>=Len) break;
                }
                if(i>=session->MsgCount) return -1;
                sindex+=1;
                if(sindex>=MAX_BUFFER_MSG) sindex-=MAX_BUFFER_MSG;
                session->MsgLast=sindex;                
        }

        Data->ReplyID=index;
        Data->CheckSum=0;
        Data->CheckSum=in_chksum((void*)Data,Len);

        session->MsgStartAddress[index]=Address;
        session->MsgLength[index]=Len;
        session->MsgCount++;
        bsize=MAX_MSGBUFFER_SIZE-Address;
        if(Len<bsize)
        {
                memcpy(session->MsgBuffer+Address, (void*)Data, Len);
        }
        else
        {
                memcpy(session->MsgBuffer+Address, (void*)Data, bsize);
                memcpy(session->MsgBuffer, (char*)Data+bsize, Len-bsize);
        }	
        DBPRINTF("Session PushMsg: Index=%d, Address=%d\n", index, Address);
        return index;
}

int SessionTakeOutMsg(PCommSession session, int Index)
{
        if(Index==session->MsgLast)
        {
                DBPRINTF("Session TakeOutMsg: %d\n",Index);
                session->MsgLast++;
                if(session->MsgLast>=MAX_BUFFER_MSG) session->MsgLast=0;
                if(session->MsgCount>0)
                        session->MsgCount--;
                SessionSendMsg(session);
                return Index;
        }
        else
        {
                DBPRINTF("Session TakeOutMsg BAD: Index=%d, Last=%d\n",Index, session->MsgLast);
                return -1;
        }
}

int SessionClearMsg(PCommSession session)
{
        session->MsgCount=0;
        session->MsgLast=0;
	return TRUE;
}

PCommSession CreateSession(void *param)
{
	PCommSession res=NULL;
	int i;
	for(i=0;i<CommSessionCount;i++)
	{
		if(0==memcmp(CommSessions[i].Sender, param, SENDER_LEN))
		{
			res=CommSessions+i;
			break;
		}
	}
	if(res==NULL)
	{
		if(CommSessionCount+1>MaxCommSessionCount)
		{
			void *newp;
			MaxCommSessionCount+=5;
			newp=malloc(sizeof(TCommSession)*MaxCommSessionCount);
			if(CommSessionCount)
			{
				memcpy(newp, CommSessions, sizeof(TCommSession)*CommSessionCount);
				free(CommSessions);
			}
			CommSessions=newp;
		}
		res=CommSessions+CommSessionCount;
		CommSessionCount++;
		memset((void*)res,0,sizeof(TCommSession));
		do
		{
			res->SessionID=GetSecond()& 0xFFFF;
			for(i=0;i<CommSessionCount-1;i++) if(res->SessionID==CommSessions[i].SessionID) break;
		}while(i<CommSessionCount-1);
		memcpy(res->Sender,param, SENDER_LEN);
		res->Close=NULL;
		memset(res->Interface, 0, 16);
	}
	res->StartTime=EncodeTime(&gCurTime);
	res->LastReplyID=0;
	res->LastSendLen=0;
	res->LastActive=res->StartTime;
	res->LastCommand=CMD_CONNECT;
	SessionClearMsg(res);
	return res;
}

int CheckSessionTimeOut(void)
{
	int i;
	for(i=0;i<CommSessionCount;i++)
	{
		int sec;		
		if (CommSessions[i].TimeOutSec>0)
		{
			sec=EncodeTime(&gCurTime)-CommSessions[i].LastActive;
			if(CommSessions[i].TimeOutSec<=sec)
			{
				EnableDevice(TRUE);
				//FreeSession(CommSessions[i].SessionID);
				return 1;
			}   
		}
		else if(CommSessions[i].MsgCached && CommSessions[i].MsgCount>0)
			SessionSendMsg(CommSessions+i);
        }
        return 0;
}

PCommSession GetSession(int SessionID)
{
	int i;
	for(i=0;i<CommSessionCount;i++)
	if(CommSessions[i].SessionID==SessionID)
	{
		return CommSessions+i;
	}
	return NULL;
}

int FreeSession(int SessionID)
{
	int i,j;
	for(i=0;i<CommSessionCount;i++)
	if(CommSessions[i].SessionID==SessionID)
	{
		if(CommSessions[i].Close)
			CommSessions[i].Close((void *)CommSessions[i].Sender);
		if(CommSessions[i].Buffer)
			freeBuffer(CommSessions[i].Buffer);
		if(CommSessions[i].fdCacheData > 0)
		{
			char fdbuf[80];
			close(CommSessions[i].fdCacheData);
			unlink(GetEnvFilePath("USERDATAPATH", "CacheData.dat", fdbuf));
		}
		for(j=i+1;j<CommSessionCount;j++)
			memcpy(CommSessions+j-1,CommSessions+j,sizeof(TCommSession));
		CommSessionCount--;
		if(CommSessionCount==0)
		{
			EnableDevice(TRUE);
		}
		return 1;
	}
	return 0;
}

void SendEvent(PCommSession session, int EventFlag, char *Data, int Len)
{
	int size;
	char buf[1024];
	PCmdHeader chdr=(PCmdHeader)buf;
	chdr->Command=CMD_REG_EVENT;
	chdr->ReplyID=0;
	chdr->SessionID=EventFlag;
	memcpy((void *)(chdr+1), Data, Len);
	size=sizeof(TCmdHeader)+Len;
	chdr->CheckSum=0;
	chdr->CheckSum=in_chksum((void*)buf,size);
        if(!session->MsgCached)
                SessionClearMsg(session);	
        if(SessionPushInAMsg(session, chdr, size)>=0)
		SessionSendMsg(session);	
}

int CheckSessionSend(int EventFlag, char *Data, int Len)
{
	int i;
	AppendRTLog(EventFlag,Data,Len);
	for(i=0;i<CommSessionCount;i++)
		if((CommSessions[i].RegEvents & EventFlag)==EventFlag)
			SendEvent(CommSessions+i, EventFlag, Data, Len);
	return 0;
}

PCommSession CheckSessionVerify(int *PIN, int *FingerID)
{
	int i;
	for(i=0;i<CommSessionCount;i++)
	{
		if(CommSessions[i].VerifyUserID>0)
		{
			*PIN=CommSessions[i].VerifyUserID;
			*FingerID=CommSessions[i].VerifyFingerID;
			CommSessions[i].RegEvents |= EF_VERIFY;
			return CommSessions+i;
		}
	}	
	return FALSE;
}

int MakeKey(int Key, int SessionID)
{
	int k,i;
//	BYTE B;
	WORD swp;

	k=0;
	for(i=0;i<32;i++)
		if(Key & (1<<i))
			k=(k<<1 | 1);
		else
			k=k<<1;
	k+=SessionID;

	((BYTE*)&k)[0]^='Z';
	((BYTE*)&k)[1]^='K';
	((BYTE*)&k)[2]^='S';
	((BYTE*)&k)[3]^='O';

	swp=(k>>16);
	k=(k<<16)+swp;
	
	//swp=((WORD *)&k)[0];
	//((WORD *)&k)[0]=((WORD *)&k)[1];
	//((WORD *)&k)[1]=swp;

	DBPRINTF("Key: %d,%d->%d \n", Key, SessionID, k);
	
	return k;

/*	B=(BYTE)(0xFF & GetUS());
	((BYTE*)&k)[0]^=B;
	((BYTE*)&k)[1]^=B;
	((BYTE*)&k)[2]=B;
	((BYTE*)&k)[3]^=B;

	return k;
*/
}


//验证连接密码是否正确
int CheckCommKey(int Key, WORD SessionID, int AuthKey)
{
//AuthKey的得到方法
//Key按位反序，与SessionID相加，
//与“ZKSO”异或，
//比较高字和低字，如果高字>低字则交换字顺序
//取1-255的随机数B，用B异或每一字节，然后用B替代第三个字节
	BYTE B=((BYTE*)&AuthKey)[2];
	Key=MakeKey(Key, SessionID);
	if((((BYTE*)&AuthKey)[0]^B)==((BYTE*)&Key)[0])
	if((((BYTE*)&AuthKey)[1]^B)==((BYTE*)&Key)[1])
	if((((BYTE*)&AuthKey)[3]^B)==((BYTE*)&Key)[3])
		return TRUE;
	return FALSE;
}

/*
|--------|--------|--------|--------|--------|--------|--------|--------|
|       CMD       |    Check Sum    |    Session ID   |    Reply ID     |
*/


extern int PowerSuspend;
extern int WaitPowerOff;
extern int gFPDirectProc;
extern int ShowMainLCDDelay;
extern int EnrollAFinger(char *tmp, int *len, int pin, int fingerid);

int ProcessEnroll(int pin, int fingerid, int *tmplen)
{
	char tmpbuf[2048];
	int ret;
	TTemplate tmp;
	
	ClockEnabled=FALSE;
	LCDClear();
	ret=EnrollAFinger(tmpbuf, tmplen, pin, fingerid);
	ShowMainLCDDelay=1;
	if(News_CommitInput==ret)
	{
		if(*tmplen>0)
		{
			if(FDB_CntTmp()>=gOptions.MaxFingerCount*100)
				return ERR_FAIL;
			if(FDB_OK==FDB_AddTmp(FDB_CreateTemplate(&tmp, (U16)pin, (char)fingerid, tmpbuf, *tmplen)))
			{
				TUser usr;
				if(!FDB_GetUser((U16)pin, &usr))
					FDB_AddUser(FDB_CreateUser(&usr, (U16)pin, NULL, NULL, PRI_VALIDUSER));
				FDB_InitDBs(FALSE);
				FPInit(NULL);
				ret=ERR_OK;
			}
			else
				ret=ERR_SAVE;
		}
		else
			ret=ERR_FAIL;
	}
	else if(News_ErrorInput==ret)
		ret=ERR_REPEAT;
	else
		ret=ERR_CANCEL;
	return ret;
}

extern int WaitCardAndWriteTemp(PFPCardOP tmp);
extern int PackTemplates(U8* Temp, U8 *Templates[], int TempCount, int ResSize);

int ProcessWriteCard(char *p, int *Pin, int *Len)
{
        int fc=0,ret;
        char *tmpsinfobuf=p+2;
        char *tmpsbuf=p+2+3*4;
        U8 Templates[4][1024];
        TFPCardOP tmp;
        U8 TMP[10240];
        //TTemplate Temp;
        U8 *(t[4]);
        int i,TmpLen=0,TmpsLen=0;	
	//生成写卡数据
	tmp.Templates=TMP;
        tmp.OP=OP_WRITE;
	tmp.PIN=0;
	if(gOptions.PIN2Width==PIN_WIDTH) //ID 2bytes
	{
		tmpsinfobuf=p+2;
		tmpsbuf=p+2+3*4;
		memcpy(&(tmp.PIN), p, 2);
	}
	else //ID 4bytes
	{
		tmpsinfobuf=p+4;
		tmpsbuf=p+4+3*4;		
		memcpy(&(tmp.PIN), p, 4);
	}
        memset(tmp.Finger, 0xFF, 4);
        for(i=0;i<4;i++)
        {
                TmpLen=get_unaligned((U16*)(tmpsinfobuf+i*3));
		DBPRINTF("Template %d=%d\n", i, TmpLen);
                if(TmpLen>0)
                {
                        memcpy(Templates[i], tmpsbuf+TmpsLen, TmpLen);
                        tmp.Finger[fc]=tmpsinfobuf[i*3+2];
                        TmpsLen+=TmpLen;
                        fc++;
                }
        }
        t[0]=Templates[0];t[1]=Templates[1];t[2]=Templates[2];t[3]=Templates[3];
        tmp.TempSize=PackTemplates(tmp.Templates, t, fc, MFGetResSize()-8);
        //需要返回
        *Pin=tmp.PIN; *Len=tmp.TempSize;
        LCDClear();
        ret=WaitCardAndWriteTemp(&tmp);	
        if(News_CommitInput==ret)
        {
                ret=ERR_OK;
        }
        else if(News_ErrorInput==ret)
                ret=ERR_REPEAT;
        else
                ret=ERR_CANCEL;
        return ret;
}

int ProcessEmptyCard()
{
        TFPCardOP tmp;
        int ret;
        BYTE TMP[10240];	
        tmp.Templates=TMP;
        LCDWriteCenterStr(1, "");
        LCDWriteLineStrID(0, MID_DC_EMPTY);
        tmp.OP=OP_EMPTY;
        LCDClear();
        ret=WaitCardAndWriteTemp(&tmp);
        if(News_CommitInput==ret)
        {
                ret=ERR_OK;
        }
        else if(News_ErrorInput==ret)
                ret=ERR_REPEAT;
        else
                ret=ERR_CANCEL;
        return ret;
}

extern char *ReadDataBlockByFD(int *size, int ContentType, int fd);
extern char *ReadDataBlock(int *size, int ContentType);
/*
QueryData - Query
CompressMethod - IN for specifying a compression method

*/
TBuffer *QueryData(char *CompressMethod, U16 DataType, char *Param, int *OriLen, PCommSession session)
{
	char *data=NULL;
	char fdbuf[80];
	U32 Start, Size=BYTE2M;
	BYTE ram=1;
	TBuffer *ret;
	//read data
	//DBPRINTF("QueryData: 0x%X, 0x%X, 0x%X\n", DataType, GETDWORD(Param), GETDWORD(Param+4));
	//DBPRINTF("QueryData2: %d, %d, %d, %d\n",*CompressMethod, DataType, GETDWORD(Param), GETDWORD(Param+4));
	switch(DataType)
	{
		case CMD_QUERY_FIRWARE:
			Start=GETDWORD(Param);
			Size=GETDWORD(Param+4);
			/*ZEM400 use
			 * Size=ConvertFWAddress(&Start, Size);
			data=(char*)Start;*/
			ram=0;
			break;
		case CMD_USERTMPS_RRQ:
			session->fdCacheData = TruncFDAndSaveAs(session->fdCacheData, GetEnvFilePath("USERDATAPATH", "CacheData.dat", fdbuf), NULL, 0);
			close(session->fdCacheData);
			session->fdCacheData = 0;
			if (gOptions.ZKFPVersion == ZKFPV10)
				data = GetUserTmpsV10((int*)&Size,GETWORD(Param));
			else
				Size = 0;
			break;
		case CMD_DB_RRQ:
			session->fdCacheData = TruncFDAndSaveAs(session->fdCacheData, GetEnvFilePath("USERDATAPATH", "CacheData.dat", fdbuf), NULL, 0);
			data=ReadDataBlockByFD((int*)&Size, *Param, session->fdCacheData);
			break;
		case CMD_READ_NEW:
			Size=BYTE2M;
			if(FCT_ATTLOG==*Param)
			{
				Size = GETDWORD(Param+4);//dsl
				data=FDB_ReadNewAttLogBlock((int*)&Size);
			}
			else if(FCT_OPLOG==*Param)
				data=FDB_ReadNewOpLogBlock((int*)&Size);
			break;
		case CMD_USERTEMP_RRQ:
			session->fdCacheData = TruncFDAndSaveAs(session->fdCacheData, GetEnvFilePath("USERDATAPATH", "CacheData.dat", fdbuf), NULL, 0);
			close(session->fdCacheData);
			session->fdCacheData = 0;
				data=ReadDataBlock((int*)(&Size), *Param);
			break;
		case CMD_ATTLOG_RRQ:
			Start=GETDWORD(Param);
			if(Start==0)
			{
				session->fdCacheData = TruncFDAndSaveAs(session->fdCacheData, GetEnvFilePath("USERDATAPATH", "CacheData.dat", fdbuf), NULL, 0);
				data=ReadDataBlockByFD((int*)&Size, FCT_ATTLOG, session->fdCacheData);
			}
			else
			{
				session->fdCacheData = TruncFDAndSaveAs(session->fdCacheData, GetEnvFilePath("USERDATAPATH", "CacheData.dat", fdbuf), NULL, 0);
				close(session->fdCacheData);
				session->fdCacheData = 0;
				int count=FDB_CntAttLog();
				Size=GETDWORD(Param+4);
				data=malloc(count*sizeof(TAttLog));
				if(data)
					Size=FDB_GetAttLog(Size, Start, (TAttLog*)data, count)*sizeof(TAttLog);
				else
					Size=0;
			}
			break;
		case CMD_OPLOG_RRQ:
			session->fdCacheData = TruncFDAndSaveAs(session->fdCacheData, GetEnvFilePath("USERDATAPATH", "CacheData.dat", fdbuf), NULL, 0);
			close(session->fdCacheData);
			session->fdCacheData = 0;
			data=ReadDataBlock((int*)&Size, FCT_OPLOG);
			break;
		case CMD_OPTIONS_RRQ:
			/*ZEM400 use
			 * data=(char*)OPTStart;
			Size=OptionSize;
			ram=0;*/
			break;
		default:
			Size=0;
	}
	*OriLen=Size;
	if(data==NULL || Size==0) return NULL;
	ret=createRomBuffer((U32)data, Size);
	ret->isRom=!ram;
	*CompressMethod=0;
	//compress?	
	//if(data && (Size<=6*640*480) && (((Size>4*1024)&&(0xFF==*CompressMethod)) || (1==*CompressMethod)))//compress by lzo
	if (0)
	{
		char wrkmem[LZO1X_MEM_COMPRESS];
		int DataLen=0;
		DBPRINTF("compress data at QueryData()\n");
		if(LZO_E_OK==lzo1x_1_compress((const lzo_byte *)data, Size, (lzo_byte*)gImageBuffer, (lzo_uintp)&DataLen, wrkmem))
		{
			if((int)Size>DataLen) //large than source
			{
				*CompressMethod=1;
				if(ram==0)
				{
					data=malloc(DataLen);
					if(data==NULL)
						return ret;
				}
				memcpy(data, gImageBuffer, DataLen);
				ret->buffer=(void*)data; 
				ret->bufEnd=(void*)(data+DataLen);
				ret->bufPtr=(void*)data;
				ret->isRom=0;
			}
		}
	}
	return ret;
}

int ProcessCommand(PCommSession session, int cmd, PCmdHeader chdr, int size);

int RunCommand(void *buf, int size, int CheckSecury)
{
	PCmdHeader chdr=(PCmdHeader)buf;
	int checksum,cmd,len, LastReplyID;
	PCommSession session;
		
	cmd=chdr->Command;
	if(size<sizeof(TCmdHeader))
		 return 0;
	session=GetSession(chdr->SessionID);
	if((session==NULL) && ! (cmd==CMD_CONNECT))
 		return 0;
	checksum=chdr->CheckSum;
	chdr->CheckSum=0;

	if(CheckSecury && !(checksum==in_chksum(buf,size))) 
		return 0;
	
	if(cmd==CMD_CONNECT) WakeUpFromSleepStatus();
	
	if(cmd==CMD_CONNECT&&HasInputControl())
	{
		InputTimeOut=InputTimeOutSec;
		return 0;
	}
	
	chdr->Command=CMD_ACK_OK;
	len=sizeof(TCmdHeader);
	size-=len;
	
	if(session)	//已经建立了连接
	{
		if(chdr->ReplyID==session->LastReplyID)	//重复发送的数据包
		{
			if(session->LastSendLen)
			{
				DBPRINTF("SEND PACKET AGAIN!\n");
				session->Send(session->LastSendData, session->LastSendLen, session->Sender);
				return cmd;
			}
		}
		else if((chdr->ReplyID<session->LastReplyID)) //非递增顺序出现的数据包非法
			return 0;
	}
	LastReplyID=chdr->ReplyID;
	if(CheckSecury && 
		session &&			//已经建立了连接
		gOptions.ComKey &&		//需要验证连接密码
		!(session->Authen) &&		//还没有验证连接密码
		(cmd!=CMD_AUTH))		//不是授权命令
	{
		chdr->Command=CMD_ACK_UNAUTH;
	}
	else 
		len=ProcessCommand(session, cmd, chdr, size);
	if(len)
	{
		session->LastCommand=cmd;
		session->LastReplyID=LastReplyID;
		chdr->CheckSum=0;
		if(CheckSecury) chdr->CheckSum=in_chksum(buf,len);
		session->Send(buf,len,session->Sender);
		memcpy(session->LastSendData, buf, len);
		session->LastSendLen=len;
		session->LastActive=EncodeTime(&gCurTime);
		if(cmd==CMD_EXIT)
			FreeSession(chdr->SessionID);
		else if(cmd==CMD_STARTENROLL && chdr->Command==CMD_ACK_OK)
		{
			int fingerid, tmplen;
			U16 pin, ret;
			char *p=(char*)(chdr+1);
			session->RegEvents|=EF_FINGER|EF_FPFTR;
			memcpy(&pin, p, 2);
			fingerid=((U8 *)p)[2];
			ret=ProcessEnroll(pin, fingerid, &tmplen);
			memcpy(p, &ret, 2);
			if(ret==ERR_OK)
			{			    
				memcpy(p+2, &tmplen, 2);
				memcpy(p+4, &pin, 2);
				p[6]=fingerid;
				len=7;
			}
			else
				len=2;
			SendEvent(session, EF_ENROLLFINGER, p, len);
		}
		else if(cmd==CMD_WRITE_MIFARE && chdr->Command==CMD_ACK_OK)
                {
                        int ret;
			U32 pin=0, len=0, writeLen=session->Buffer->bufferSize;
                        char w[16];
                        session->RegEvents|=EF_WRITECARD;
                        ret=ProcessWriteCard((char *)session->Buffer->buffer, &pin, &writeLen);
			memcpy(w, &ret, 2);
                        if(ret==ERR_OK)
                        {
				memcpy(w+2, &writeLen, 2);
				memcpy(w+4, &pin, 4);
                                len=8;
                        }
                        else
                                len=2;
                        SendEvent(session, EF_WRITECARD, w, len);
			ShowMainLCDDelay=1;
                }
		else if(cmd==CMD_EMPTY_MIFARE && chdr->Command==CMD_ACK_OK)
                {
                        int ret,len=0;
                        char w[2];
                        session->RegEvents|=EF_EMPTYCARD;
                        ret=ProcessEmptyCard();
			memcpy(w, &ret, 2);
                        len=2;
                        SendEvent(session, EF_WRITECARD, w, len);
			ShowMainLCDDelay=1;
                }
	}
	return cmd;
}

char *ReadDataBlockByFD(int *size, int ContentType, int fd)
{
	char *buffer;
	buffer=FDB_ReadBlockByFD(size, ContentType, fd);
	return buffer;
}
char *ReadDataBlock(int *size, int ContentType)
{
	char *buffer;
	buffer=FDB_ReadBlock(size, ContentType);
	return buffer;
}

int ReadBlockAndSend(PCmdHeader chdr, PCommSession session, int ContentType)
{
	int size;
	char *buffer;
	
	buffer = FDB_ReadBlockByFD(&size, ContentType, session->fdCacheData);
	if(buffer)
	{
		SendLargeData((void*)chdr, session, buffer, size);
		free(buffer);
		return size;
	}

	return 0;
}

int GetOptionNameAndValue(char *p, int size)
{
	char value[1024];
	int l,vl;
	p[size]=0;

	if(!LoadStr(p, value))  
		GetDefaultOption(p,value);

	if(value)
	{
		l=strlen(p);
		vl=strlen(value);
		if(value && vl<1024)
		{
			p[l]='=';
			strcpy(p+l+1,value);
			return l+2+vl;
		}
		else
			return 0;
	}
	else
		return 0;
}

extern TSensorBufInfo SensorBufInfo;

int ProcessCommand(PCommSession session, int cmd, PCmdHeader chdr, int size)
{
	TUser user;
	TSms sms;
	TWorkCode workcode;
	TExtUser extuser;	
	char fdbuf[80];
	char *p=(char*)(chdr+1);
	int i, len;

	len=sizeof(TCmdHeader);
	switch(cmd)
	{
		case CMD_CONNECT: 
			if(gOptions.ComKey)
				chdr->Command=CMD_ACK_UNAUTH;
			EnableDevice(FALSE);
			return 0;
		case CMD_EXIT:
			FlushSensorBuffer();
			EnableDevice(TRUE);
			if(Reboot_Power&1)
				RebootMachine();
			if(Reboot_Power&2)
				ExPowerOff(FALSE);
			break;
		case CMD_ACK_OK:
			session->MsgCached=TRUE;
			SessionTakeOutMsg(session, chdr->ReplyID);
			len=0;  //不需发送回应
			break;		
		case CMD_AUTH:
			session->Authen=CheckCommKey(gOptions.ComKey,session->SessionID, get_unaligned((int *)p));
			if(!(session->Authen))
			{
				chdr->Command=CMD_ACK_UNAUTH;
				EnableDevice(TRUE);
			}
			break;
		case CMD_REG_EVENT:		
			memcpy(&session->RegEvents, p, 4);
			break;
		case CMD_RTLOG_RRQ:
			{
				extern int gRTLogListCount;
				int n=0,rtlognum;
				//DebugOutput1("Send rtlog %d",n);

				if(size>0 && RTLogNum>(rtlognum=GETDWORD(p)))
				{
					n=LastRTLogBufLen;
					SET_DWORD(p,n,0);
					if(n>0)
						memcpy(p+4,LastRTLogBuf,n);
				}
				else if(gRTLogListCount>0)
				{
					char *buf,*cbuf;
					PRTLogNode prtlognode,tmprtlognode;
					n=gRTLogListCount*sizeof(TRTLog);

					if(LastRTLogBuf)
						free(LastRTLogBuf);

					SET_DWORD(p,n,0);
					//DebugOutput1("Send rtlog %d",n);
					buf=malloc(n);
					cbuf=buf;
					prtlognode=rtlogheader->Next;
					while(prtlognode)
					{
						tmprtlognode=prtlognode;
						memcpy(cbuf,(char*)&(prtlognode->log),sizeof(TRTLog));					
						//	DebugOutput1("get rtlog node %d",prtlognode->log.EventType);
						prtlognode=prtlognode->Next;
						cbuf+=sizeof(TRTLog);
						free(tmprtlognode);
					}
					gRTLogListCount=0;
					rtloglist=rtlogheader;
					memcpy(p+4,buf,n);

					LastRTLogBuf=malloc(n);
					memcpy(LastRTLogBuf,buf,n);
					LastRTLogBufLen=n;
					RTLogNum++;

					free(buf);
				}else
					SET_DWORD(p,0,0);
				len=len+4+n;
				//DebugOutput1("Send rtlog finish %d",len);
			}
			break;		
		case CMD_DISABLEDEVICE:
			/*  //add by cn 2009-03-22
			    if(size==sizeof(U32))
			    {
			    session->TimeOutSec=get_unaligned((U32*)p);
			    }
			 */
			//	EnableDevice(FALSE);
			break;
		case CMD_ENABLEDEVICE:
			//	EnableDevice(TRUE);
			break;
		case CMD_USERTEMP_EX_WRQ:
			{
				char *fptemp;
				U16 PIN=get_unaligned((U16*)p);
				char fingerID=p[2];
				U16 tempLen=0;

				if (gOptions.ZKFPVersion == ZKFPV10)
				{
					fptemp = (char *)session->Buffer->buffer;
					tempLen = session->Buffer->bufferSize;
					if (tempLen > ZKFPV10_MAX_LEN)
					{
						chdr->Command=CMD_ACK_ERROR;
						break;
					}
				}
				else
				{
					fptemp =p+6;
					tempLen = get_unaligned((U16*)(p+4));
				}
				if(FDB_OK!=(chdr->SessionID=AppendUserTemp(PIN, NULL, fingerID, fptemp, tempLen)))
					chdr->Command=CMD_ACK_ERROR;
			}
			break;
		case CMD_USERTEMP_EX_RRQ:
			if(size==0)
			{
				if(!(chdr->SessionID=ReadBlockAndSend(chdr, session, FCT_USER)))
					chdr->Command=CMD_ACK_ERROR;
			}
			else if(size==1)
			{
				if(!(chdr->SessionID=ReadBlockAndSend(chdr, session, *p)))
					chdr->Command=CMD_ACK_ERROR;
			}
			else if(size==3)
			{
				PUser u;
				TTemplate tmp;
				int ret;
				BYTE duress=0;
				u=FDB_GetUser(get_unaligned((U16*)p),NULL);
				memset(&tmp,0,sizeof(TTemplate));
				if(u && (ret=FDB_GetTmp(u->PIN,p[2],(char *)&tmp)))
				{
					if (gOptions.ZKFPVersion == ZKFPV10)
					{
						duress=tmp.Valid;
						memcpy(tmp.Template+ret,&duress,1);
						SendLargeData((void*)chdr,session,(char*)tmp.Template,ret+1);
					}
					else
					{
						duress =tmp.Valid;
						memcpy(p,tmp.Template, tmp.Size);
						memcpy((void*)(p+tmp.Size), &duress,1);
						len+=tmp.Size+1;
						chdr->SessionID=tmp.Size+1;
					}
				}
				else
					chdr->Command=CMD_ACK_ERROR;
			}
			break;
	case CMD_APPEND_USERTEMP:
		if(FDB_OK!=(chdr->SessionID=AppendUserTemp(get_unaligned((U16*)p), p+3, *(p+2), p+13, get_unaligned((U16*)(p+11)))))
			chdr->Command=CMD_ACK_ERROR;
		break;
	case CMD_USERTEMP_WRQ:
		if(FDB_OK!=(chdr->SessionID=AppendUserTemp(get_unaligned((U16*)p), NULL, *(p+2), p+5, get_unaligned((U16*)(p+3)))))
			chdr->Command=CMD_ACK_ERROR;
		break;
	case CMD_USER_WRQ:
		chdr->Command=CMD_ACK_ERROR;
		if((size==sizeof(TUser)))
		{
			TUser u;
			memcpy((void*)&u, (void*)p, sizeof(TUser));
			if(AppendUser(u.PIN, u.Name, u.Password, u.Privilege)==FDB_OK) 
				chdr->Command=CMD_ACK_OK;
		}
		else if(AppendUser((int)get_unaligned((U16*)p), p+2, p+2+8, (int)(p[2+8+5]))==FDB_OK) 
			chdr->Command=CMD_ACK_OK;
		break;
	case CMD_EXTUSER_WRQ:
		chdr->Command=CMD_ACK_ERROR;
		if((size==sizeof(TExtUser)))
		{
			nmemcpy((void*)&extuser, (void*)p, sizeof(TExtUser));
			if(FDB_GetExtUser(get_unaligned((U16*)p), NULL))
			{
				if(FDB_ChgExtUser(&extuser)!=FDB_OK)
					chdr->SessionID=1;
				else
					chdr->Command=CMD_ACK_OK;
			}
			else if(FDB_AddExtUser(&extuser)!=FDB_OK)
				chdr->SessionID=2;
			else 
				chdr->Command=CMD_ACK_OK;
		}
		break;

	
	case CMD_WorkCode_WRQ:
		//DBPRINTF("Receive Worcde wrq  Comand\n");
		chdr->Command=CMD_ACK_ERROR;
		if((size==sizeof(TWorkCode)))
		{
			int ret;			
			nmemcpy((void*)&workcode, (void*)p, sizeof(TWorkCode));
			//DBPRINTF("wid: %d\twc: %d\n",workcode.WORKID,workcode.WORKCODE);
			ret=FDB_ChgWorkCode(&workcode); 
			if(ret==FDB_ERROR_NODATA)
			{
			       if (FDB_AddWorkCode(&workcode)!=FDB_OK)
				       chdr->SessionID=2;
			       else
					chdr->Command=CMD_ACK_OK;
			}
			else if (ret==FDB_ERROR_IO)
			       chdr->SessionID=1;
			else 
				chdr->Command=CMD_ACK_OK;
		}
		break;
	case CMD_SMS_WRQ:
		chdr->Command=CMD_ACK_ERROR;
		if((size==sizeof(TSms)))
		{
			int ret;			
			nmemcpy((void*)&sms, (void*)p, sizeof(TSms));
			ret=FDB_ChgSms(&sms); 
			if(ret==FDB_ERROR_NODATA)
			{
			       if (FDB_AddSms(&sms)!=FDB_OK)
				       chdr->SessionID=2;
			       else
					chdr->Command=CMD_ACK_OK;
			}
			else if (ret==FDB_ERROR_IO)
			       chdr->SessionID=1;
			else 
				chdr->Command=CMD_ACK_OK;
		}
		break;
        case CMD_UDATA_WRQ:
		if(size==sizeof(TUData))
		{
			TUData udata;
			int res;
			memcpy((void*)&udata, p, sizeof(TUData));
			res=FDB_AddUData(&udata);
			chdr->SessionID=res;
			if(res!=FDB_OK)
				chdr->Command=CMD_ACK_ERROR;
			
		}
		else
			chdr->Command=CMD_ACK_ERROR;
		break;
        case CMD_WRITE_MIFARE:
                {
                        chdr->Command=CMD_ACK_ERROR;
                        if(gMachineState==STA_VERIFYING)
                        {
                                chdr->Command=CMD_ACK_OK;
                        }
                        else
                                chdr->SessionID=ERR_STATE;
                }
                break;
        case CMD_EMPTY_MIFARE:
                {
                        chdr->Command=CMD_ACK_ERROR;
                        if(gMachineState==STA_VERIFYING)
                        {
                                chdr->Command=CMD_ACK_OK;
                        }
                        else
                                chdr->SessionID=ERR_STATE;
                }
                break;		
	case CMD_FREEID_RRQ:
		if(size==0)
		{
			i=1;
			while(i<MAX_PIN)
			{
				if(!FDB_GetUser(i,&user)) break;
				i++;
			}
			if(i<MAX_PIN)
				chdr->SessionID=(WORD)i;
			else
				chdr->Command=CMD_ACK_ERROR;
		}
		else
		{
			BYTE fid=0xff;
			i=get_unaligned((U16*)p);
			i=FDB_GetFreeFingerID(i,&fid);
			if(i!=FDB_OK)
			{
				chdr->Command=CMD_ACK_ERROR;
				chdr->SessionID=(WORD)i;
			}
			else
				chdr->SessionID=fid;
		}
		break;
	case CMD_USER_RRQ:
		if(FDB_GetUser(get_unaligned((U16*)p),&user))
		{
			len+=sizeof(TUser);
			memcpy(p, (void*)&user, sizeof(TUser));

		}
		else
			chdr->Command=CMD_ACK_ERROR;
		break;
	case CMD_EXTUSER_RRQ:
		if(FDB_GetExtUser(get_unaligned((U16*)p), &extuser))
		{
			len+=sizeof(TExtUser);
			memcpy(p, (void*)&extuser, sizeof(TExtUser));
		}
		else
			chdr->Command=CMD_ACK_ERROR;
		break;

	case CMD_WorkCode_RRQ:
		//DBPRINTF("Receive Worcde rrq  Comand\n");
		if(FDB_GetWorkCode(get_unaligned((U16*)p), &workcode))
		{
			len+=sizeof(TWorkCode);
			memcpy(p, (void*)&workcode, sizeof(TWorkCode));
		}
		else
			chdr->Command=CMD_ACK_ERROR;
		break;

	case CMD_SMS_RRQ:
		if(FDB_GetSms(get_unaligned((U16*)p), &sms))
		{
			len+=sizeof(TSms);
			memcpy(p, (void*)&sms, sizeof(TSms));
		}
		else
			chdr->Command=CMD_ACK_ERROR;
		break;
	case CMD_DB_RRQ:
		if(size>0)
		if(!(chdr->SessionID=ReadBlockAndSend(chdr, session, *p)))
			chdr->Command=CMD_ACK_ERROR;
		break;
	case CMD_USERTEMP_RRQ:
		if(size==0)
		{
			if(!(chdr->SessionID=ReadBlockAndSend(chdr, session, FCT_USER)))
				chdr->Command=CMD_ACK_ERROR;
		}
		else if(size==1)
		{
			if(!(chdr->SessionID=ReadBlockAndSend(chdr, session, *p)))
				chdr->Command=CMD_ACK_ERROR;
		}
		else if(size==3)
		{
			TTemplate tmp;
			PUser u;
			u=FDB_GetUser(get_unaligned((U16*)p),NULL);
			if(u && FDB_GetTmp(u->PIN,p[2],&tmp))
			{
				memcpy(p,tmp.Template, tmp.Size);
				len+=tmp.Size;
				chdr->SessionID=tmp.Size;
			}
			else
				chdr->Command=CMD_ACK_ERROR;
		}		
		break;	
	case CMD_DELETE_USER:
		if(FDB_OK!=FDB_DelUser(get_unaligned((U16*)p))) chdr->Command=CMD_ACK_ERROR;
		break;
	case CMD_DELETE_EXTUSER:
		if(FDB_OK!=FDB_DelExtUser(get_unaligned((U16*)p))) chdr->Command=CMD_ACK_ERROR;
		break;
	case CMD_DELETE_WorkCode:
		if(FDB_OK!=FDB_DelWorkCode(get_unaligned((U16*)p))) chdr->Command=CMD_ACK_ERROR;
		break;
	case CMD_DELETE_SMS:
		if(FDB_OK!=FDB_DelSms(get_unaligned((U16*)p))) chdr->Command=CMD_ACK_ERROR;
		break;
	case CMD_DELETE_UDATA:
		if(size==sizeof(TUData))
		{
			TUData udata;
			memcpy((void*)&udata, p, sizeof(TUData));
			if(FDB_OK!=FDB_DelUData(udata.PIN, udata.SmsID))
				chdr->Command=CMD_ACK_ERROR; 			
		}
		else
			chdr->Command=CMD_ACK_ERROR; 
		break;
	case CMD_CLEAR_ADMIN:
		if(FDB_OK!=FDB_ClrAdmin()) chdr->Command=CMD_ACK_ERROR;
		break;
	case CMD_REFRESHDATA:
		FDB_InitDBs(FALSE);
		FPInit(NULL);
		if(gOptions.IsSupportSMS) CheckBoardSMS();
		break;
	case CMD_DELETE_USERTEMP:
		chdr->Command=CMD_ACK_ERROR;
		if(FDB_GetUser(get_unaligned((U16*)p),&user))
		{
			i=FDB_DelTmp(user.PIN,p[2]);
			if(i==FDB_ERROR_NODATA) chdr->SessionID=2;
			else if(i!=FDB_OK) chdr->SessionID=1; 
			else chdr->Command=CMD_ACK_OK;
		}
		else
			chdr->SessionID=2;
		break;
	case CMD_TEST_TEMP:
		{
			int result,score=55;
			PUser u;
			if (gOptions.ZKFPVersion == ZKFPV10)
			{
				if(BIOKEY_IDENTIFYTEMP_10(fhdl, (BYTE*)p, &result, &score))
				{
					u=FDB_GetUser((U16)result, NULL);
					if(u)
					{
						memcpy(p, &u->PIN, 2);
					}
				}
				else
					*p=0;
			}
			else
			{
				if(BIOKEY_IDENTIFYTEMP(fhdl, (BYTE*)p, &result, &score))
				{
					u=FDB_GetUser((U16)result, NULL);
					if(u)
					{
						memcpy(p, &u->PIN, 2);
					}
				}
				else
					*p=0;
			}
			len+=2;
		}
		break;
	case CMD_RESTART:
	//	RebootMachine()
		Reboot_Power = 1;
		break;
	case CMD_POWEROFF:
		DBPRINTF("Power off!\n");
	//	ExPowerOff(FALSE);
		Reboot_Power = 2;
		break;
	case CMD_SLEEP:
		ExPowerOff(TRUE);
		break;
	case CMD_GET_FREE_SIZES:
		{
			int Flag=0;
			int Len=FDB_GetSizes(p);
			if(size>0) Flag=*(BYTE*)p;
			if((size>0) && ((U32)Flag<Len/sizeof(int)))
			{
				memcpy(p, p+Flag*sizeof(int),sizeof(int));
				len+=sizeof(int);
			}
			else
				len+=Len;
			break;
		}
#ifdef URU
	case CMD_CAPTUREIMAGE:
		{
		       int is=gOptions.ZF_WIDTH*gOptions.ZF_HEIGHT;
		       char *image=malloc(is+2*sizeof(int));		
		       memcpy(image, &gOptions.ZF_WIDTH, 4);
		       memcpy(image+4, &gOptions.ZF_HEIGHT, 4);
		       memcpy(image+2*sizeof(int), gImageBuffer, is);
		       is+=2*sizeof(int);
		       SendLargeData((void*)chdr, session, image, is);       
		       free(image);
		}
		break;
#else
	case CMD_CAPTUREFINGER:
		{
			char *sbuf=gImageBuffer, *dbuf=gImageBuffer, *img;
			int i=0,j,w=gOptions.ZF_WIDTH,c=gOptions.OImageHeight*gOptions.OImageWidth;
			if(size>=sizeof(int))
			{
				dbuf=(char*)malloc(512*512);
				i=*(int*)p;
				if(500==i)
				{
					if(size>sizeof(int))
						w=((int*)p)[1];
				}
				else if(501==i)
					sbuf=gImageBuffer+c*4;
				else if(502==i)
				{
					w=gOptions.OImageWidth;
					sbuf=gImageBuffer+c*4;
				}
			}
			memset(gImageBuffer, 0, sizeof(gImageBuffer));
			if(sbuf==gImageBuffer && !TestEnabledMsg(MSG_TYPE_FINGER))
			{
				j = 50000;
				while( !CaptureSensor(gImageBuffer, ONLY_LOCAL, &SensorBufInfo)&&--j );
				if(j<=0)
					break;
			}
			if(dbuf!=gImageBuffer)
			{
				extern HANDLE fhdl;
				*(int *)dbuf=500; 
				((int *)dbuf)[1]=w;
				((int *)dbuf)[2]=i==502?gOptions.OImageHeight:w*gOptions.ZF_HEIGHT/gOptions.ZF_WIDTH;
				c=((int *)dbuf)[1]*((int *)dbuf)[2]+sizeof(int)*3;
				if(i!=502)
				{
					DBPRINTF("width=%d ZF_WIDTH=%d i=%d buf[1]=%d buf[2]=%d\n", w, gOptions.ZF_WIDTH, i, ((int *)dbuf)[1], ((int *)dbuf)[2]);
					if (gOptions.ZKFPVersion == ZKFPV10)
						BIOKEY_GETFINGERLINEAR_10(fhdl, (BYTE*)sbuf, (BYTE*)dbuf+sizeof(int)*3);
					else
						BIOKEY_GETFINGERLINEAR(fhdl, (BYTE*)sbuf, (BYTE*)dbuf+sizeof(int)*3);
					if(w!=gOptions.ZF_WIDTH)
					{
						dbuf+=sizeof(int)*3;
						img=sbuf;
						for(i=0;i<gOptions.ZF_HEIGHT*w/gOptions.ZF_WIDTH;i++)
						{
							char *row=dbuf+gOptions.ZF_WIDTH*(i*gOptions.ZF_WIDTH/w);
							for(j=0;j<w;j++)
							{
								*img=row[j*gOptions.ZF_WIDTH/w];
								img++;
							}
						}
						memcpy(dbuf, sbuf, c);
						dbuf-=sizeof(int)*3;
					}
				}
				else
					memcpy((BYTE*)dbuf+sizeof(int)*3,sbuf,((int *)dbuf)[1]*((int *)dbuf)[2]);
			}
			SendLargeData((void*)chdr, session, dbuf, c);
			if(dbuf!=gImageBuffer) free(dbuf);
		}
		break;
        case CMD_CAPTUREIMAGE:
                {
                        int is=CMOS_WIDTH*CMOS_HEIGHT;
                        char *image=malloc(is);
			memset(image, 0, is);
                        InitSensor(0,0,CMOS_WIDTH,CMOS_HEIGHT, gOptions.NewFPReader);
                        CaptureSensor(image, ONLY_LOCAL, &SensorBufInfo);
                        CaptureSensor(image, ONLY_LOCAL, &SensorBufInfo);
	/*
			memset(image, 0, is);
			k = 500000;
			while(k--)
			{
				if(CaptureSensor(image, ONLY_LOCAL, &SensorBufInfo)) 
					break;
			}
			if(k<=0)
			{
				ExBeep(1);
				free(image);
                        	InitSensor(gOptions.OLeftLine,gOptions.OTopLine,gOptions.OImageWidth,gOptions.OImageHeight, gOptions.NewFPReader);
				break;
			} */
                        if(size>=sizeof(int))
                        {
                                int width=*(int*)p;
                                int i,j;
                                char *img=gImageBuffer, *row;
                                if(width*CMOS_HEIGHT*width/CMOS_WIDTH<=5*gOptions.OImageWidth*gOptions.OImageHeight)
                                {
                                        is=width*CMOS_HEIGHT*width/CMOS_WIDTH;
                                        for(i=0;i<CMOS_HEIGHT*width/CMOS_WIDTH;i++)
                                        {
                                                row=image+CMOS_WIDTH*(i*CMOS_WIDTH/width);
                                                for(j=0;j<width;j++)
                                                {
                                                        *img=row[j*CMOS_WIDTH/width];
                                                        img++;
                                                }
                                        }
                                        ((int*)image)[0]=500*width/CMOS_WIDTH;
                                        ((int*)image)[1]=width;
                                        ((int*)image)[2]=CMOS_HEIGHT*width/CMOS_WIDTH;
                                        memcpy(image+3*sizeof(int), gImageBuffer, is);
                                        is+=3*sizeof(int);
                                }
                        }
                        SendLargeData((void*)chdr, session, image, is);
			DBPRINTF("Send IMAGE OK! Length=%d\n", is); 
                        InitSensor(gOptions.OLeftLine,gOptions.OTopLine,gOptions.OImageWidth,gOptions.OImageHeight, gOptions.NewFPReader);
                        free(image);
                        CaptureSensor(gImageBuffer, ONLY_LOCAL, &SensorBufInfo);
                }
                break;
#endif
	case CMD_CLEAR_ATTLOG:
		FDB_ClrAttLog();
		break;
	case CMD_CLEAR_OPLOG:
		FDB_ClrOPLog();
		break;
	case CMD_CLEAR_DATA:
		if(size==1)
			FDB_ClearData(*p);
		else
			FDB_ClearData(FCT_ALL);			
		break;
	case CMD_OPTIONS_RRQ:
		if(0==(i=GetOptionNameAndValue(p, size)))
			chdr->Command=CMD_ACK_ERROR;
		else
			len+=i;
		break;		
	case CMD_OPTIONS_WRQ:
		{
			char *value=p;
			int namel=0;
			p[size]=0;
			while(*value)
			{
				if('='==*value++)
					break;
				if(namel++>size) break;
			}
			if(namel>=size || namel<1)
			{
				chdr->Command=CMD_ACK_ERROR;
				chdr->SessionID=1;
			}
			else
			{
				p[namel]=0;
				if(!SaveStr(p,value,TRUE))
				{			
				       chdr->Command=CMD_ACK_ERROR;
				       chdr->SessionID=2;					
				}
			}
		}
		break;
	case CMD_ATTLOG_RRQ:
		if(!(chdr->SessionID=ReadBlockAndSend(chdr, session, FCT_ATTLOG)))
			chdr->Command=CMD_ACK_ERROR;
		break;
	case CMD_GET_PINWIDTH:
		p[0]=gOptions.PIN2Width;
		len++;
		break;		
	case CMD_GET_IOSTATUS:
		p[0]=ExGetIOStatus();
		len++;
		break;
	case CMD_OPLOG_RRQ:
		if(!(chdr->SessionID=ReadBlockAndSend(chdr, session, FCT_OPLOG)))
			chdr->Command=CMD_ACK_ERROR;
		break;
	case CMD_SET_TIME:
		{
			TTime t;
			memcpy(&i,p,4);
			OldDecodeTime((time_t)i, &t);
			SetTime(&t);
			ShowMainLCD();
		}
		break;
	case CMD_GET_TIME:
		{
			time_t tmp=OldEncodeTime(&gCurTime);
			memcpy(p, &tmp, sizeof(time_t));
			len+=sizeof(time_t);
		}
		break;
	case CMD_ENABLE_CLOCK:
		ClockEnabled=*p;
		if(*p)
			ShowMainLCD();
		break;
	case CMD_REFRESHOPTION:
		LoadOptions(&gOptions);
		FPInit(NULL);
		break;
	case CMD_TESTVOICE:
		if(size==2)
			ExPlayVoiceFrom(p[0],p[1]);
		else
			ExPlayVoice(p[0]);
		break;
#ifndef URU
        case CMD_SENSOROPT_WRQ:
                if(size==0)
                {
                        if(!WriteSensorOptions(&gOptions, FALSE))
                                chdr->Command=CMD_ACK_ERROR;
                }
                else if(size==2*15)
                {
                        if(!EEPROMWriteOpt((BYTE*)p, 2*15, FALSE))
                                chdr->Command=CMD_ACK_ERROR;
                }
                else
                        chdr->Command=CMD_ACK_ERROR;
                break;
        case CMD_SENSOROPT_RRQ:
                if(Read24WC02(0,p,30))
                {
                        len+=32;
                        p[31]=0;
                        p[30]=0x22;
                }
                else
                        chdr->Command=CMD_ACK_ERROR;
                break;
#endif
        case CMD_SETFPDIRECT:
                gFPDirectProc=*(BYTE*)p;
                if(gFPDirectProc==255)
                        gFPDirectProc=-1;
                else if(gFPDirectProc==254)
                        gFPDirectProc=-2;
                else if(gFPDirectProc==253)
                        gFPDirectProc=-3;
                break;
	case CMD_GET_VERSION:
		if (gOptions.ZKFPVersion == ZKFPV10)
			strcpy(p,MAINVERSION);
		else
			sprintf(p, "%s %s", "Ver 6.21", __DATE__);
		len+=strlen(p)+1;
		break;
	case CMD_QUERY_FIRWARE: 
		break; 
	case CMD_HASH_DATA:
		if(size==4)
		{
			if (session->fdCacheData)
			{
				if(GETDWORD(p)!=hashBufferByFD(session->fdCacheData))
				{
					DBPRINTF("HASH FAILED\n");
					chdr->Command=CMD_ACK_ERROR;
				}
			}
			else
			{
				if(GETDWORD(p)!=hashBuffer(session->Buffer))
				{
					DBPRINTF("HASH FAILED\n");
					chdr->Command=CMD_ACK_ERROR;
				}
			}
		}
		else if(size==0)
		{
			if (session->fdCacheData)
			{
				unsigned int hash = hashBufferByFD(session->fdCacheData);
				SET_DWORD(p, hash, 0);
			}
			else
				SET_DWORD(p, hashBuffer(session->Buffer), 0);
			len+=4;
		}
		break;
	case CMD_UPDATE_READALL:
		if(size==1)
		{
			if(*p==FCT_ATTLOG)
				FDB_SetAttLogReadAddr(LOG_READ_ALL);
			else if(*p==FCT_OPLOG)
				FDB_SetOpLogReadAddr(LOG_READ_ALL);
			else
				chdr->Command=CMD_ACK_ERROR;
		}
		else
			chdr->Command=CMD_ACK_ERROR;
		break;
	case CMD_READ_NEW:
		if(size>0)
		{
			int Size=BYTE2M;
			char *data;
			if(FCT_ATTLOG==*p)
				data=FDB_ReadNewAttLogBlock((int*)&Size);
			else if(FCT_OPLOG==*p)
				data=FDB_ReadNewOpLogBlock((int*)&Size);
			if(data)
			{
				SendLargeData((void*)chdr, session, data, Size);
				free(data);
			}
			else
				chdr->Command=CMD_ACK_ERROR;
		}
		else 
			chdr->Command=CMD_ACK_ERROR;
		break;		
	case CMD_UPDATE_FIREWARE: 
		chdr->Command=CMD_ACK_ERROR; 
		if(session->Buffer->bufferSize<=0) 
			chdr->SessionID=1; 
		else if(NULL==session->Buffer) 
			chdr->SessionID=2; 
		else if(session->Buffer->bufPtr>session->Buffer->bufEnd) 
			chdr->SessionID=3; 
		else 
		{ 
			U32 C=get_unaligned((U32*)p);
			char sFileName[128], sFirmwareFiles[128];
			U32 A=0;
                        U32 Addr=get_unaligned((U32*)p+1);
			U32 L=session->Buffer->bufferSize;
			//DBPRINTF("CMD_UPDATE_FIRMWARE C=%d L=%d Addr=%x\n", C, L, Addr);
			switch(C) 
			{ 
/*			case UPDATE_FONT: 
				GetEnvFilePath("USERDATAPATH", "hz2.dat.gz", sFileName);
				A=1;
				break;  */
			case UPDATE_OPTIONS: 
				break; 
			case UPDATE_TEMPS: 
				if (gOptions.ZKFPVersion == ZKFPV10)
					GetEnvFilePath("USERDATAPATH", "tempv10.dat", sFileName);
				else
					GetEnvFilePath("USERDATAPATH", "template.dat", sFileName);
				A=1;
				break; 
			case UPDATE_USERS: 
				GetEnvFilePath("USERDATAPATH", "user.dat", sFileName);
				A=1;
				break; 
/*			case UPDATE_FIRMWARE: 
				GetEnvFilePath("USERDATAPATH", "main.gz", sFileName);
				A=1;
				break; 
			case UPDATE_CFIRMWARE: 
				strcpy(sFileName, "/tmp/update.tgz");
				if((((BYTE *)session->Buffer->buffer)[0]==0x1f)&&(((BYTE *)session->Buffer->buffer)[1]==0x8b))
				       A=1;
				break;  */
			case UPDATE_BOOTLOADER:
				A=1; 
				break; 
			case UPDATE_FLASH: 
				A=1;
				break; 					
/*			case UPDATE_SOUND: 
				GetEnvFilePath("USERDATAPATH", "res.tgz", sFileName);
				A=1;
				break;
			case UPDATE_ALGORITHM: 
				GetEnvFilePath("USERDATAPATH", "libzkfp.so.3.5.1", sFileName);
				A=1;
				break; 		
			case UPDATE_LANGUAGE: 
                                GetEnvFilePath("USERDATAPATH", "language.tgz", sFileName);
				A=1;
				break; 		
			case UPDATE_AUTOSHELL: 
				GetEnvFilePath("USERDATAPATH", "auto.sh", sFileName);
				A=1;
				break;
*/
                        case CMD_UPDATEFILE:
                        {
                                char sTmp[40];

                               memset(sTmp,0,40);
                               memcpy(sTmp, p+4, sizeof(sTmp));
                               sprintf(sFileName,"%s%s","/mnt/mtdblock/",sTmp);
                               A=1;
                               break;
                        }
 
			case UPDATE_BATCHUSERDATA: 
				A=1;
				break; 				
			default: 
				A=0; 
			} 
			if(A==1) 
			{ 
				if(C==UPDATE_BATCHUSERDATA)
				{	
					unsigned int s_msec, e_msec;	
					
					s_msec=GetTickCount();
					BatchOPUserData(session->Buffer->buffer, session->fdCacheData);
					e_msec=GetTickCount();
					DBPRINTF("User Data Batch Command FINISHED! time = %d\n", e_msec - s_msec);
					if(session->Buffer && session->Buffer->bufferSize>0)
						freeBuffer(session->Buffer);
					session->Buffer=NULL;
					
					chdr->Command=CMD_ACK_OK; 
				}
				else
				{
					int fd = -1;
					/* update firmware(ucos.bin) */
					if(memcmp(sFileName,"/mnt/mtdblock/ucos.bin",strlen(sFileName))==0)
					{
						if(update_ucos(session->Buffer->buffer, 1,session->Buffer->bufferSize)==0)
						chdr->Command=CMD_ACK_OK;
					}
					else
						fd=open(sFileName, O_CREAT|O_RDWR|O_TRUNC, S_IREAD|S_IWRITE);
					//DBPRINTF("fd: %d\n",fd);
					if (fd>0)
					{	
						if(write(fd, session->Buffer->buffer, session->Buffer->bufferSize)==session->Buffer->bufferSize)
						{
							//DBPRINTF("Write file ok\n");
							close(fd);
							yaffs_sync("/mnt");
							#if 0 
							if(C==UPDATE_CFIRMWARE)
							{
							}
							else if(C==UPDATE_LANGUAGE)
							{
							}
							#endif 
							chdr->Command=CMD_ACK_OK;
						}
						else
						{
							close(fd);
							chdr->SessionID=4;
						}
					}
					else						
						chdr->SessionID=4; 				
				}
			} 
			else 
				chdr->SessionID=5; 
		} 
		break; 		
	case CMD_FREE_DATA:
		if(session->Buffer && session->Buffer->bufferSize>0)
			freeBuffer(session->Buffer);
		session->Buffer=NULL;
		//session->BufferPos=0;
		// clear data
		if (session->fdCacheData)
		{
			session->fdCacheData = TruncFDAndSaveAs(session->fdCacheData, GetEnvFilePath("USERDATAPATH", "CacheData.dat", fdbuf), NULL, 0);
			yaffs_flush(session->fdCacheData);
			close(session->fdCacheData);
			session->fdCacheData = 0;
		}
		break;
	case CMD_PREPARE_DATA:
		freeBuffer(session->Buffer);
		if (GETDWORD(p) >= MAX_CACHE_SIZE)
		{
			session->Buffer = createCacheRamBuffer(GETDWORD(p));
			session->fdCacheData = TruncFDAndSaveAs(session->fdCacheData, GetEnvFilePath("USERDATAPATH", "CacheData.dat", fdbuf), NULL, 0);
		}
		else
		{
			session->Buffer = createRamBuffer(GETDWORD(p));
			session->fdCacheData = 0;
		}
		break;
	case CMD_DATA:
		if (session->fdCacheData)
			i=session->Buffer->bufferSize - lseek(session->fdCacheData, 0, SEEK_END);
		else
			i=session->Buffer->bufEnd-session->Buffer->bufPtr;

		if((session->LastReplyID+1==chdr->ReplyID) && (i>=size) && session->Buffer)
		{
			if (session->fdCacheData)
				i = write(session->fdCacheData, p, size);
			else
			{
				memcpy((char*)(session->Buffer->bufPtr), p, size);
				session->Buffer->bufPtr+=size;
			}
		}
		else
			chdr->Command=CMD_ACK_ERROR;
		//printf("session->BufferLen=%d BufferPos=%d\n", session->BufferLen, session->BufferPos);
		break;
	case CMD_QUERY_DATA:
		{
			int RealSize=0, DataType = GETWORD(p+1);
			//DBPRINTF("CMD_QUERY_DATA\n");
			freeBuffer(session->Buffer);
			session->Buffer=NULL;
			if(size==11)
				session->Buffer=QueryData(p, GETWORD(p+1), p+3, &RealSize, session);
			if(session->Buffer)
			{
				DBPRINTF("bufferSize=%d\n",session->Buffer->bufferSize);
				if(session->Buffer->bufferSize<=1024) //Less than 1K send directly
				{
					len+=session->Buffer->bufferSize;
					if (session->fdCacheData)
					{
						lseek(session->fdCacheData, 0, SEEK_SET);
						read(session->fdCacheData, p, session->Buffer->bufferSize);
					}
					else	
						memcpy(p, (char*)(session->Buffer->buffer), session->Buffer->bufferSize);
					chdr->Command=CMD_DATA;
				}
				else
				{
					SET_DWORD(p, session->Buffer->bufferSize, 1);
					SET_DWORD(p, RealSize, 1+4);
					if (session->fdCacheData)
					{
						unsigned int hash = hashBufferByFD(session->fdCacheData);
						SET_DWORD(p, hash, 1+4+4);
					}
					else
						SET_DWORD(p, hashBuffer(session->Buffer), 1+4+4);
					len+=1+4+4+4;
				}
			}
			else
				chdr->Command=CMD_ACK_ERROR;
			//DBPRINTF("CMD_QUERY_DATA,chdr->Command=%d\n",chdr->Command);
			break;
		}
	case CMD_READ_DATA:
		{
			int Size=GETDWORD(p+4);
			int Addr=GETDWORD(p);
			if(size==8 && session->Buffer && session->Buffer->bufferSize>=Addr+Size)
			{
				if(Size<=1024)  //Less than 1K send directly
				{
					len+=Size;
					if (session->fdCacheData)
					{
						lseek(session->fdCacheData, Addr, SEEK_SET);
						read(session->fdCacheData, p, Size);
					}
					else
						memcpy(p, (char*)(session->Buffer->buffer)+Addr, Size);
					chdr->Command=CMD_DATA;
				}
				else
				{
					if (session->fdCacheData)
						SendLargeDataByFD((void*)chdr, session, Addr, Size);
					else
						SendLargeData((void*)chdr, session, (char*)(session->Buffer->buffer)+Addr, Size);
				}
			}
			else
				chdr->Command=CMD_ACK_ERROR;
			break;
		}				
	case CMD_USERGRP_RRQ:			//?????????
		*p=GetUserGrp(get_unaligned((U32*)p));
		len+=1;
		break;
	case CMD_USERGRP_WRQ:			//设置用户分组
		if(gOptions.LockFunOn<2)	//仅仅高级门禁功能支持该命令
			chdr->Command=CMD_ACK_ERROR;
		else
			if(!SaveUserGrp(get_unaligned((U32*)p), p[4])) chdr->Command=CMD_ACK_ERROR;
		break;
	case CMD_USERTZ_RRQ:		//读用户时间段设置
	case CMD_GRPTZ_RRQ:		//读组时间段设置	
		{
			int TZs[10];
			if(cmd==CMD_USERTZ_RRQ)
				i=GetUserTZ(get_unaligned((U32*)p), TZs+1);
			else
				i=GetGrpTZ(get_unaligned((U32*)p), TZs+1);
			if(i<0) i=-1-i;
			TZs[0]=i;
			i=sizeof(int)*(i+1);
			memcpy(p, TZs, i);
			len+=i;
		}
		break;
	case CMD_USERTZ_WRQ:			//写用户时间段设置
	case CMD_GRPTZ_WRQ:			//写组时间段设置
		if(gOptions.LockFunOn<2)	//仅仅高级门禁功能支持该命令
			chdr->Command=CMD_ACK_ERROR;
		else
		{
			U32 TZs[10];
			memcpy(TZs, p+4, size);
			if(TZs[0]>3)	//每个用户最多允许3个时间段
				chdr->Command=CMD_ACK_ERROR;
			else
			{
				for(i=TZs[0];i<3;i++)
					TZs[i+1]=0;
				if(cmd==CMD_USERTZ_WRQ)
					SaveUserTZ(get_unaligned((U32*)p), (int*)TZs+1, TZs[0]);
				else
					SaveGrpTZ(get_unaligned((U32*)p), (int*)TZs+1, TZs[0]);
			}
		}
		break;
	case CMD_TZ_RRQ:		//读时间段设置
		{
			TTimeZone TZ;
			LoadTimeZone(get_unaligned((U32*)p), &TZ);
			i=sizeof(TTimeZone);
			memcpy(p, (char*)&TZ,i);
			len+=i;
		}
		break;
	case CMD_TZ_WRQ:			//写时段设置
		if(gOptions.LockFunOn<2)	//仅仅高级门禁功能支持该命令
			chdr->Command=CMD_ACK_ERROR;
		else if(size==sizeof(int)+sizeof(TTimeZone))
		{
			TTimeZone TZ;
			i=get_unaligned((U32*)p);
			if(i<=10)		//最多可设置10个时间段
			{
				memcpy(&TZ, p+sizeof(int), sizeof(TTimeZone));
				SaveTimeZone(i, &TZ);
			}
			else
				chdr->Command=CMD_ACK_ERROR;
		}
		else
			chdr->Command=CMD_ACK_ERROR;
		break;
	case CMD_ULG_RRQ:					//读开锁组合
		{
			char ulg[1024];
			if(LoadStr("ULG", ulg))
			{
				i=strlen(ulg)+1;
				memcpy(p, ulg, i);
				len+=i;
			}
		}
		break;
	case CMD_ULG_WRQ:					//写开锁组合
		if(gOptions.LockFunOn<2)	//仅仅高级门禁功能支持该命令
			chdr->Command=CMD_ACK_ERROR;
		else
		{
			p[size]=0;
			SaveStr("ULG", p, FALSE);
		}
		break;
	case CMD_UNLOCK:					//打开锁
		//ExAuxOut(*p, p[1]);
		DoAuxOut(*p, p[1]);

		break;
	case CMD_CLEAR_ACC:
		ClearAllACOpt(TRUE);
		break;
	case CMD_CHANGE_SPEED:
		if(size==sizeof(U32))
		{
			i=get_unaligned((U32*)p);
			if(i==1)
			{
				session->Speed=session->Speed*5/4;
				if(session->Speed>50*1024*1024) session->Speed=50*1024*1024;
			}
			else if(i==0)
			{
				if(session->Speed>512*1024) 
				{
					session->Speed=session->Speed/2;
				}
			}
			else if(i>9600 && i<101*1024*1024)
			{
				session->Speed=session->Speed/2;
			}
			else
				chdr->Command=CMD_ACK_ERROR; 
		}
		else
			chdr->Command=CMD_ACK_ERROR; 
		break;
	case CMD_STARTVERIFY:
		chdr->Command=CMD_ACK_ERROR;
		if(gMachineState==STA_IDLE || gMachineState==STA_VERIFYING)
		{
			int pin;
			BYTE fingerid;
			TTemplate tmp;
			pin=get_unaligned((U16*)p);
			fingerid=p[2];
			if((size!=0 && size!=3))
				chdr->SessionID=ERR_PARAM;
			else if(size==3)
			{
				if(pin<0 || pin>MAX_PIN || ((fingerid!=0xFF) && (fingerid>=gOptions.MaxUserFingerCount)))
					chdr->SessionID=ERR_PARAM;
				else if((fingerid!=0xFF) && !FDB_GetTmp((U16)pin, (char)fingerid, &tmp))
					chdr->SessionID=ERR_NOTENROLLED;
				else
				{
					session->VerifyFingerID=fingerid;
					session->VerifyUserID=pin;
					chdr->Command=CMD_ACK_OK;
					gMachineState=STA_VERIFYING;
					session->RegEvents |= EF_VERIFY|EF_FINGER;
				}
			}
			else
			{
				session->VerifyUserID=0;
				chdr->Command=CMD_ACK_OK;
				gMachineState=STA_VERIFYING;
				session->RegEvents |= EF_VERIFY|EF_FINGER;
			}
		}
		else
			chdr->SessionID=ERR_STATE;			
		break;
	case CMD_STARTENROLL:
		chdr->Command=CMD_ACK_ERROR;
		if(gMachineState==STA_IDLE)
		{
			int pin;
			BYTE fingerid;
			TTemplate tmp;
			pin=get_unaligned((U16*)p);
			fingerid=p[2];
			if(pin<0 || pin>MAX_PIN || fingerid>=gOptions.MaxUserFingerCount)
				chdr->SessionID=ERR_PARAM;
			if(FDB_GetTmp((U16)pin, (char)fingerid, &tmp))
				chdr->SessionID=ERR_ENROLLED;
			else
			{
				chdr->Command=CMD_ACK_OK;
			}
		}
		else
		{
			chdr->SessionID=ERR_STATE;
		}
		break;
	case CMD_CANCELCAPTURE:
		gMachineState=STA_IDLE;
		break;
	case CMD_WRITE_LCD:
		LCDWriteStr(p[0], p[1], p+3, p[2]);
		break;
	case CMD_CLEAR_LCD:
		LCDClear();
		break;
	case CMD_LCDSIZE_RRQ:
		p[0]=gLCDHeight & 0xFF;
		p[1]=gLCDHeight>>8;
		p[2]=gLCDWidth & 0xFF;
		p[3]=gLCDWidth>>8;
		p[4]=gLCDCharWidth & 0xFF;
		p[5]=gLCDCharWidth>>8;
		p[6]=gLCDRowCount & 0xFF;
		p[7]=gLCDRowCount>>8;
		len+=8;
		break;
	case CMD_LASTTEMP_RRQ:
		break;
	case CMD_TRANSSTATE:
		i=get_unaligned((U32*)p);
		if(size>0 && i>=0 && i<=IKeyOut-IKeyOTIn) 
			ProcStateKey(IKeyOTIn+i);
		chdr->SessionID=gOptions.AttState;
		break;
	case CMD_STATE_RRQ:
		chdr->SessionID=gMachineState;
		DBPRINTF("gMachineState=%d\n",gMachineState);
		break;
	case CMD_TEMPDB_CLEAR:
		TDB_Clear();
		if(size>2)
		{
			TDB_ADDTemp(get_unaligned((U16*)p), (BYTE*)p+2);
		}
		break;
	case CMD_TEMPDB_ADD:
		TDB_ADDTemp(get_unaligned((U16*)p), (BYTE*)p+2);
		break;
        case CMD_WIEGAND:
		len+=WiegandSend(get_unaligned((U32*)p), get_unaligned((U32*)p+1), get_unaligned((U32*)p+2));
		break;		
	default:
		chdr->Command=CMD_ACK_UNKNOWN;
		break;
	}
	return len;
}

extern int SendUSBData(void *buf, int size, void *param);

void SendLargeData(char *in_buf, PCommSession session, char *buf, int size)
{
	PCmdHeader chdr=(PCmdHeader)in_buf;
	char *p=(char*)(chdr+1);
	int index, psize=1024, len=sizeof(TCmdHeader)+sizeof(U32)*2;
	int timeout = 5000;
	
	memcpy(p, &size, 4);
	memcpy(p+4, &psize, 4);
	chdr->Command=CMD_PREPARE_DATA;
	chdr->CheckSum=0;
	chdr->CheckSum=in_chksum((unsigned char *)in_buf,len);
	session->Send(in_buf,len,session->Sender);
	index=0;
	chdr->Command=CMD_DATA;
	while(size>0)
	{
//		if(index%4==0) DelayMS(1);
		len=size;
		if(len>psize) len=psize;
		chdr->SessionID=index;
		memcpy(p,buf, len);
		chdr->CheckSum=0;
		chdr->CheckSum=in_chksum((unsigned char *)in_buf,len+sizeof(TCmdHeader));
		if(session->Send==&SendUSBData)
		{
			if( (session->Send(in_buf,len+sizeof(TCmdHeader),session->Sender)==0)&&timeout--)
			{
				wdt_set_count(0);
				printf("SendLargeData failed,remain data size: %d. SessionID: %d.try again ...\n",size,chdr->SessionID);
				continue;
			}
		}
		else
		{
			session->Send(in_buf,len+sizeof(TCmdHeader),session->Sender);
			DelayMS(20);
		}
		wdt_set_count(0);
		buf+=len;
		index++;
		size-=len;
		if((index%(6+(GetSecond()%10))==5) && (session->Speed>=256*1024))//以太网连接，延时
		{
			int dd=200*1024/(session->Speed/(2*1024));
			if(dd>1200) dd=1200;
			DelayMS(dd);
		}
		printf("SendLargeData successful,remain data size: %d,next index: %d\n",size,index);
	}
	printf(" **** SandLargeData end ****\n");
	chdr->Command=CMD_ACK_OK;
}

void SendLargeDataByFD(char *in_buf, PCommSession session, int addr, int size)
{
	PCmdHeader chdr=(PCmdHeader)in_buf;
	char *p=(char*)(chdr+1);
	int index, psize=1024, len=sizeof(TCmdHeader)+sizeof(U32)*2;
	int offset = addr;
	int timeout = 5000;
	
	if (session->fdCacheData <= 0)
		return;

	memcpy(p, &size, 4);
	memcpy(p+4, &psize, 4);
	chdr->Command=CMD_PREPARE_DATA;
	chdr->CheckSum=0;
	chdr->CheckSum=in_chksum((unsigned char *)in_buf,len);
	session->Send(in_buf,len,session->Sender);
	index=0;
	chdr->Command=CMD_DATA;
	while(size>0)
	{
//		if(index%4==0) DelayMS(1);
		len=size;
		if(len>psize) len=psize;
		chdr->SessionID=index;
		lseek(session->fdCacheData, offset, SEEK_SET);
		read(session->fdCacheData, p, len);
		chdr->CheckSum=0;
		chdr->CheckSum=in_chksum((unsigned char *)in_buf,len+sizeof(TCmdHeader));
		if(session->Send==&SendUSBData)
		{
			if( (session->Send(in_buf,len+sizeof(TCmdHeader),session->Sender)==0)&&timeout--)
			{
				wdt_set_count(0);
				printf("SendLargeData failed,remain data size: %d. SessionID: %d.try again ...\n",size,chdr->SessionID);
				continue;
			}
		}
		else
		{
			session->Send(in_buf,len+sizeof(TCmdHeader),session->Sender);
			DelayMS(20);
		}
		wdt_set_count(0);
		offset += len;
		index++;
		size-=len;
		if((index%(6+(GetSecond()%10))==5) && (session->Speed>=256*1024))//以太网连接，延时
		{
			int dd=200*1024/(session->Speed/(2*1024));
			if(dd>1200) dd=1200;
			DelayMS(dd);
		}
		printf("SendLargeData successful,remain data size: %d,next index: %d\n",size,index);
	}
	printf(" **** SandLargeData end ****\n");
	chdr->Command=CMD_ACK_OK;
}

int SendMasterProcess(WORD Cmd, char *buf,int len,int TimeOutMS)
{
	return 0;
}
