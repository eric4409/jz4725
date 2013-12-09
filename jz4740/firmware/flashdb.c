/*************************************************
  
 ZEM 200                                          
 
 flashdb.c define all functions for database mangement of flash                             
 
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
 
*************************************************/

#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <sys/ioctl.h>
//#include <fcntl.h>
//#include <unistd.h>
//#include <asm/unaligned.h>
#include <sys/time.h>
#include "ccc.h"
#include "flashdb.h"
#include "utils.h"
#include "options.h" 
#include "main.h"
#include "lcdmenu.h"
#include "zkfp.h"
#include "zkfp10.h"
#include "sensor.h"
#include "lcm.h"
#if MACHINE_ID == 2
#include "ff.h"
#endif
//#include "yaffsfs.h"
//#include <sys/vfs.h> 

#define STAT_COUNT 		0
#define STAT_VALIDLEN 		1
#define STAT_CNTADMINUSER 	2	 
#define STAT_CNTADMIN 		3
#define STAT_CNTPWDUSER		4
#define STAT_CNTTEMPLATE	5

typedef struct _FSizes_{  //
	int Total, TotalSector,
	SectorCnt, SectorFree,
	UserCnt, UserFree,
	TmpCnt, TmpFree,
	AttLogCnt, AttLogFree,
	OpLogCnt, OpLogFree,
	AdminCnt, PwdCnt,
	StdTmp, StdUser, StdLog,
	ResTmp, ResUser, ResLog;
}GCC_PACKED TFSizes, *PFSizes;

static int attLogReadPos=LOG_READ_NONE;
static int opLogReadPos=LOG_READ_NONE;

//transactions
static int fdTransaction=-1;
//extend transactions
static int fdExtLog=-1;
//fingerprints template data
static int fdFingerTmp=-1;
static int fdFingerTmp1=-1;//kenny
int IsDB8M=0;

//users record
static int fdUser=-1;
static int fdExtUser=-1;
//operate log
static int fdOpLog=-1;
//user sms
static int fdSms=-1;
static int fdUData=-1;
//worcode table
static int fdWorkCode=-1;

static unsigned char gBoardSMS[2048];
static int gBoardSMSPos=0;

//transaction storage format
//4字节短时间格式 4 byte short time format
#define AttLogSize1 4
//8字节完整时间格式 8 byte long time format
#define AttLogSize2 8

//|0      |1       |2          |3      |       
//+----------------+---+--+-+----------+--------------------------------+
//    UID                             16 bits user id
//                  ST                3  bits status
//                      VT            2  bits verification type 
//                         ST         1  bit  time format，1-short time type，0-long time type
//                              Time  10 bit  times, short time type mean elapsed seconds since the last long time type(0-2048)，long time format is 0
//long time format add 4 byte UNIX times, that is total seconds from 1970-1-1 00:00:00

// ExtendAttLogFormat
//|0 	|4 	|8     		|9	|12 	|16
//+------+---------+-----------------+--------------------------------+
//PIN 							//4Bytes userid or pin2
//	DATETIME						//4Bytes UNIX times total seconds from 1970-1-1 00:00:00
//		STATUS					//1Byte	
//			VERI				//1Byte	verification type
//				Reserved			//2Bytes
//					WORKCODE		//4Bytes

//当前最高用户号码 
static int UserMaxID=1;
//当前考勤记录基准时间
time_t BaseTime=0;

static U32 *UserIDMap=NULL;

char PRIVALUES[4]={PRIVILLEGE0, PRIVILLEGE1,PRIVILLEGE2,PRIVILLEGE3};

#define IsAttLogLongPack(value) (((value & 0x04)==0))
#define IsValidUser(buf) ((((PUser)(buf))->Privilege & PRI_VALID)==PRI_VALID)
#define EmptyData(buf) ((get_unaligned((U32*)(buf)))==0xFFFFFFFF)

//data structure for file data searching 
/* chenyy
typedef struct _SearchHandle{
	int ContentType;
	char *buffer;
	int bufferlen;
	int datalen;
	int fd;
}TSearchHandle, *PSearchHandle;
*/

int UnpackAttLog(char *buf, PAttLog log);
int PackAttLog(char *buf, PAttLog log);

#ifdef WEBSERVER
int * pSelectFDFromConentType(int ContentType) /******** Add For Web Server ********/
{
        if (FCT_ATTLOG==ContentType){
                return (int*)&fdTransaction;
        }
        else if (FCT_FINGERTMP==ContentType){
                return (int*)&fdFingerTmp;
        }
        else if (FCT_USER==ContentType){
                return (int*)&fdUser;
        }
        else if (FCT_OPLOG==ContentType){
                return (int*)&fdOpLog;
        }
        else if (FCT_SMS==ContentType){
                return (int*)&fdSms;
        }
        else if (FCT_UDATA==ContentType){
                return (int*)&fdUData;
        }
	else if (FCT_EXTUSER==ContentType){
		return (int*)&fdExtUser;
	}
        return NULL;
}
#endif

//get file handle from File type
int SelectFDFromConentType(int ContentType)
{
	if (FCT_ATTLOG==ContentType){
		if(gOptions.AttLogExtendFormat)
			return fdExtLog;
		else
			return fdTransaction;
	}
	else if (FCT_FINGERTMP==ContentType){
		return fdFingerTmp;
	}
	else if (FCT_FINGERTMP1==ContentType){
		return fdFingerTmp1;//kenny
	}
	else if (FCT_USER==ContentType){
		return fdUser;
	}
	else if (FCT_OPLOG==ContentType){
		return fdOpLog;
	}	
	else if (FCT_SMS==ContentType){
		return fdSms;
	}
	else if (FCT_UDATA==ContentType){
		return fdUData;
	}
	else if (FCT_EXTUSER==ContentType){
		return fdExtUser;
	}
	else if (FCT_WorkCode==ContentType){
		return fdWorkCode;
	}
	return -1;
}

void SearchFirst(PSearchHandle sh)
{
	sh->fd=SelectFDFromConentType(sh->ContentType);
	lseek(sh->fd, 0, SEEK_SET);
	sh->bufferlen=0; 
	sh->datalen=0;  //valid data length
}

static int GetTypeSize(int ContentType)
{
	switch(ContentType){
	case FCT_ATTLOG:
		return sizeof(TAttLog);
	case FCT_USER:
		return sizeof(TUser);
	case FCT_FINGERTMP:
		if (gOptions.ZKFPVersion == ZKFPV10)
			return sizeof(TTemplate);
		else
			return (sizeof(TTemplate) - ZKFP_OFF_LEN);
	case FCT_OPLOG:
		return sizeof(TOPLog);
	case FCT_SMS:
		return sizeof(TSms);
	case FCT_UDATA:
		return sizeof(TUData);
	case FCT_EXTUSER:
		return sizeof(TExtUser);
	}
}

BOOL SearchNext(PSearchHandle sh)
{
	BOOL eof;       
	int tmplen = sizeof(TTemplate);
	
	eof = TRUE;
	sh->bufferlen=0;
	sh->datalen=0;
	switch(sh->ContentType)
	{
	case FCT_ATTLOG:
		if(gOptions.AttLogExtendFormat)
		{
			if (read(sh->fd, sh->buffer, sizeof(TExtendAttLog))==sizeof(TExtendAttLog))
			{
				sh->bufferlen=sizeof(TExtendAttLog);
				sh->datalen=sh->bufferlen;
				eof = FALSE;
			}
		}
		else
		{
			if (read(sh->fd, sh->buffer, 4)==4)
			{
				sh->bufferlen=4;
				if (IsAttLogLongPack(sh->buffer[2]))
				{
					read(sh->fd, sh->buffer+4, 4);
					sh->bufferlen+=4;
				}
				sh->datalen=sh->bufferlen;				
				eof = FALSE;
			}
		}
		break;
	case FCT_USER:
		if (read(sh->fd, sh->buffer, sizeof(TUser))==sizeof(TUser)){
			sh->bufferlen=sizeof(TUser);
			if (((PUser)sh->buffer)->PIN) //pin > 0
				sh->datalen=sh->bufferlen;
			eof = FALSE;
		}
		break;
	case FCT_FINGERTMP:
		if (gOptions.ZKFPVersion != ZKFPV10)
			tmplen -= ZKFP_OFF_LEN;
		if (read(sh->fd, sh->buffer, tmplen)==tmplen){
			if (gOptions.ZKFPVersion != ZKFPV10)
			{
				sh->bufferlen=tmplen;
				if (((PTemplate)sh->buffer)->Valid)
					sh->datalen=((PTemplate)sh->buffer)->Size;
			}
			else
			{
				if (((PTemplate)sh->buffer)->Size > 0)
					((PTemplate)sh->buffer)->Size += 6;
				sh->bufferlen=((PTemplate)sh->buffer)->Size;
				if (((PTemplate)sh->buffer)->Valid)
					sh->datalen=((PTemplate)sh->buffer)->Size;
			}
			eof = FALSE;
		}
		break;
	case FCT_OPLOG:
		if (read(sh->fd, sh->buffer, sizeof(TOPLog))==sizeof(TOPLog)){
			sh->bufferlen=sizeof(TOPLog);
			sh->datalen=sh->bufferlen;
			eof = FALSE;
		}
		break;	    
	case FCT_SMS:
		if (read(sh->fd, sh->buffer, sizeof(TSms))==sizeof(TSms)){
			sh->bufferlen=sizeof(TSms);
			if (((PSms)sh->buffer)->ID)
				sh->datalen=sh->bufferlen;
			eof = FALSE;
		}
		break;
	case FCT_UDATA:
		if (read(sh->fd, sh->buffer, sizeof(TUData))==sizeof(TUData)){
			sh->bufferlen=sizeof(TUData);
			if (((PUData)sh->buffer)->PIN)
				sh->datalen=sh->bufferlen;
			eof = FALSE;
		}
		break;
	case FCT_EXTUSER:
		if (read(sh->fd, sh->buffer, sizeof(TExtUser))==sizeof(TExtUser)){
			sh->bufferlen=sizeof(TExtUser);
			if (((PExtUser)sh->buffer)->PIN) //pin > 0
				sh->datalen=sh->bufferlen;
			eof = FALSE;
		}
		break;
	case FCT_WorkCode:
		if (read(sh->fd,sh->buffer,sizeof(TWorkCode))==sizeof(TWorkCode)){
			sh->bufferlen=sizeof(TWorkCode);
			if (((PWorkCode)sh->buffer)->WORKID)
				sh->datalen=sh->bufferlen;
			eof = FALSE;
		}
			
		break;
	}
	return eof;
}
static U8 gTemplate[2048];

//#ifdef ZEM300
#if 0	 //ucos no need this funciton
void RefreshJFFS2Node(int ContentType, int REFRESH_JFFS2_NODE)
{
	TSearchHandle sh;
	U8 buf[2048];
	char *tmpdata;
	int datasize=0;
	int offset=0;
	int count=0;
	int fd;
	
	fd = SelectFDFromConentType(ContentType);
	
	sh.ContentType=ContentType; 
	sh.buffer=buf;	
	SearchFirst(&sh);

	if(CurAttLogCount&&((CurAttLogCount%REFRESH_JFFS2_NODE)==0))
	{
		while(!SearchNext(&sh))
		{
			count++;
			if((count%REFRESH_JFFS2_NODE)==1)
			{
				offset=lseek(fd, 0, SEEK_CUR)-sh.bufferlen;
			}
		}
		
		if(count&&((count%REFRESH_JFFS2_NODE)==0)) //??? CurAttLogCount count  % REFRESH_JFFS2_NODE
		{
			datasize=lseek(fd, 0, SEEK_CUR)-offset;
			tmpdata=malloc(datasize);
			lseek(fd, offset, SEEK_SET);
			if(read(fd, tmpdata, datasize)==datasize)
			{
				lseek(fd, offset, SEEK_SET);
				write(fd, tmpdata, datasize);
			}
			DBPRINTF("count=%d datasize=%d offset=%d\n", count, datasize, offset);
			free(tmpdata);
		}
	}
}
#define FlashBlockLimit 80
void RefreshTemplate()
{
	//statfs
    long blocks_used=0;
    long blocks_percent_used=0;

    char buf[80];
    TSearchHandle sh;
    int  j, k, l;
    char *FingerCacheBuf;
    int MaxGroupFingerCnt=200;
    int FingerNumber;
    int CurGroupFingerCnt;	
#ifdef UCOS
	return;
#endif

#ifndef UCOS
    struct statfs s;
   if ((statfs("/mnt/mtdblock/data", &s) == 0) || (statfs("/mnt/mtdblock", &s)==0))
   {
	if ((s.f_blocks > 0) )
	{
            blocks_used = s.f_blocks - s.f_bfree;
            blocks_percent_used = 0;
            if (blocks_used + s.f_bavail) {
                blocks_percent_used = (((long long) blocks_used) * 100
                                       + (blocks_used + s.f_bavail)/2
                                       ) / (blocks_used + s.f_bavail);
            }
	}
	DBPRINTF("disk used: %ld\n",blocks_percent_used);	
   }
#endif

	if (blocks_percent_used >= FlashBlockLimit)	
	{
		ClockEnabled = FALSE;
	        LCDInfoShow(LoadStrByID(MID_DISKCLEAN), LoadStrByID(HID_WAITING));

		close(fdFingerTmp); 
		if (gOptions.ZKFPVersion == ZKFPV10)
		{
			if (IsDB8M)
				fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "data/tempv10.dat", buf), O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
			else
				fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "tempv10.dat", buf), O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
		}
		else
		{
			if (IsDB8M)
				fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "data/template.dat", buf), O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
			else
				fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "template.dat", buf), O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
		}

		sh.ContentType=FCT_FINGERTMP; 
		sh.buffer=(char *)gTemplate;
		
		//acquired the status for operation
		FingerNumber=0;
		SearchFirst(&sh);
		while(!SearchNext(&sh))
		{
			if(sh.datalen)
				FingerNumber++;
				
		}
			//execute 
			SearchFirst(&sh);
			j=1;
			k=0;
			FingerCacheBuf=malloc(MaxGroupFingerCnt*sizeof(TTemplate));		
			while(k<FingerNumber)
			{
				CurGroupFingerCnt=MaxGroupFingerCnt;
				if ((k+MaxGroupFingerCnt)>FingerNumber)
					CurGroupFingerCnt=FingerNumber-k;			
				read(fdFingerTmp, FingerCacheBuf, CurGroupFingerCnt*sizeof(TTemplate));
				k+=CurGroupFingerCnt;			
				DBPRINTF("GROUP %d Starting......\n", j);
				for(l=0;l<CurGroupFingerCnt;l++)
				{
					sh.buffer=(char*)((PTemplate)FingerCacheBuf+l);
				}
				DBPRINTF("GROUP %d Writting......\n", j);
				lseek(fdFingerTmp, -1*CurGroupFingerCnt*sizeof(TTemplate), SEEK_CUR);
				write(fdFingerTmp, FingerCacheBuf, CurGroupFingerCnt*sizeof(TTemplate));
				DBPRINTF("GROUP %d Endded\n", j);
				j++;
			}
			free(FingerCacheBuf);
	
        	//fsync(fdFingerTmp);
		close(fdFingerTmp);
		if (gOptions.ZKFPVersion == ZKFPV10)
		{
			if (IsDB8M)
				fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "data/tempv10.dat", buf), O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
			else	
				fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "tempv10.dat", buf), O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
		}
		else
		{
			if (IsDB8M)
				fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "data/template.dat", buf), O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
			else	
				fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "template.dat", buf), O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
		}
		yaffs_sync("/mnt"); 
		DBPRINTF("Finish Template Space Compress\n");
	}

}

#else
void RefreshJFFS2Node(int ContentType, int REFRESH_JFFS2_NODE)
{
	//nothing to do
}
void RefreshTemplate()
{
	//nothing to do
}
#endif

//append or overwrite data to file
int SearchAndSave(int ContentType, char *buffer, U32 size)
{
	int fd;
	int rc;
	
	fd = SelectFDFromConentType(ContentType);
	switch(ContentType)
	{
	case FCT_ATTLOG:
		//RefreshJFFS2Node(ContentType, 200);
		lseek(fd, 0, SEEK_END);
		break;
	case FCT_USER:
		if (FDB_GetUser(0, NULL))
			lseek(fd, -1*sizeof(TUser), SEEK_CUR);
		else
			lseek(fd, 0, SEEK_END);		
		break;
	case FCT_FINGERTMP:
		RefreshTemplate();
		if (FDB_GetTmp(0, 0, NULL))
			lseek(fd, -1*size, SEEK_CUR);
		else
			lseek(fd, 0, SEEK_END);		
		break;
	//kenny
	case FCT_FINGERTMP1:
			lseek(fd, 0, SEEK_END);		
		break;
	//kenny
	case FCT_OPLOG:
		lseek(fd, 0, SEEK_END);
		break;	    
	case FCT_SMS:
		if (FDB_GetSms(0, NULL))
			lseek(fd, -1*sizeof(TSms), SEEK_CUR);
		else
			lseek(fd, 0, SEEK_END);
		break;	    
	case FCT_UDATA:
		if (FDB_GetUData(0, NULL))
			lseek(fd, -1*sizeof(TUData), SEEK_CUR);
		else
			lseek(fd, 0, SEEK_END);
		break;
	case FCT_EXTUSER:
		if (FDB_GetExtUser(0, NULL))
			lseek(fd, -1*sizeof(TExtUser), SEEK_CUR);
		else
			lseek(fd, 0, SEEK_END);
	
	case FCT_WorkCode:
		if (FDB_GetWorkCode(0, NULL))
			lseek(fd, -1*sizeof(TWorkCode), SEEK_CUR);
		else
			lseek(fd, 0, SEEK_END);
		break;	    
	}
	rc=((write(fd, buffer, size)==size)?FDB_OK:FDB_ERROR_IO);
	yaffs_flush(fd);
	if((ContentType==FCT_ATTLOG)&&(rc==FDB_OK)) CurAttLogCount++;
	return rc;
}


BOOL FDB_IsEmpty(int ContentType)
{
	return (lseek(SelectFDFromConentType(ContentType), 0, SEEK_END)?TRUE:FALSE);	
}

int FDB_InitDBs(BOOL OpenSign)
{
	int i,id;
	U8 buf[2048];
	TSearchHandle sh;
		
	UserMaxID=0;
	BaseTime=0;
	CurAttLogCount=0;
	memset(gBoardSMS, 0, 2048);	
	if (OpenSign)
	{
		fdTransaction=open(GetEnvFilePath("USERDATAPATH", "transaction.dat", buf), O_RDWR|O_CREAT, S_IREAD|S_IWRITE);
		fdExtLog=open(GetEnvFilePath("USERDATAPATH", "extlog.dat", buf), O_RDWR|O_CREAT, S_IREAD|S_IWRITE);
//                fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "data/template.dat", buf), O_RDWR|O_CREAT, S_IREAD|S_IWRITE);
		if (gOptions.ZKFPVersion == ZKFPV10)
                	fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "tempv10.dat", buf), O_RDWR|O_CREAT, S_IREAD|S_IWRITE);
		else	
                	fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "template.dat", buf), O_RDWR|O_CREAT, S_IREAD|S_IWRITE);
/*
                if (fdFingerTmp < 0)
                {
                        fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "template.dat", buf), O_RDWR|O_CREAT, S_IREAD|S_IWRITE);
                        IsDB8M=0;
                }
                else
                {
                        IsDB8M=1;                 }
*/
                IsDB8M=0;	
		fdUser=open(GetEnvFilePath("USERDATAPATH", "user.dat", buf), O_RDWR|O_CREAT, S_IREAD|S_IWRITE);
		fdOpLog=open(GetEnvFilePath("USERDATAPATH", "oplog.dat", buf), O_RDWR|O_CREAT, S_IREAD|S_IWRITE);	
		fdSms=open(GetEnvFilePath("USERDATAPATH", "sms.dat", buf), O_RDWR|O_CREAT, S_IREAD|S_IWRITE);
		fdUData=open(GetEnvFilePath("USERDATAPATH", "udata.dat", buf), O_RDWR|O_CREAT, S_IREAD|S_IWRITE);
		fdExtUser=open(GetEnvFilePath("USERDATAPATH", "extuser.dat", buf),O_RDWR|O_CREAT, S_IREAD|S_IWRITE);
		fdWorkCode=open(GetEnvFilePath("USERDATAPATH", "workcode.dat", buf), O_RDWR|O_CREAT, S_IREAD|S_IWRITE);
	}

	//get base time from transactions	
	sh.ContentType=FCT_ATTLOG; 
	sh.buffer=buf;	
	SearchFirst(&sh);
		
	while(!SearchNext(&sh))
	{
		CurAttLogCount++;
		if(!gOptions.AttLogExtendFormat)
		{
			if (IsAttLogLongPack(sh.buffer[2]))
				memcpy(&BaseTime, sh.buffer+4, 4);
		}		
	}




//解决丢记录问题 2007.01.24
	if (CurAttLogCount)
	{
		int TotalAttLogSize=0;
		int CurAttLogSize=0;
	
	
		if(gOptions.AttLogExtendFormat)
		{
			CurAttLogSize = CurAttLogCount*(sizeof(TExtendAttLog));
			TotalAttLogSize = lseek(fdExtLog,0,SEEK_END);
		
		}
		else
		{
			CurAttLogSize = CurAttLogCount*(sizeof(TAttLog));
			TotalAttLogSize = lseek(fdTransaction,0,SEEK_END);
		}
		//printf("logcount: %d-%d\n",CurAttLogCount,sizeof(TAttLog));
		printf("attlogsize:%d-%d\n",CurAttLogSize, TotalAttLogSize);

		//may be bad  record
		//printf("TotalAttLogSize:%d\n",TotalAttLogSize-CurAttLogSize);

		

		if (CurAttLogSize < TotalAttLogSize)
		{
			time_t t;
			
				
		        GetTime(&gCurTime);
		        t=OldEncodeTime(&gCurTime);
			if(gOptions.AttLogExtendFormat)
			{
				TExtendAttLog fixextattlog;

				memset(&fixextattlog,0,sizeof(TExtendAttLog));
				fixextattlog.time_second = t;
				lseek(fdExtLog,-1*(TotalAttLogSize-CurAttLogSize),SEEK_CUR);
				if (write(fdExtLog,&fixextattlog,sizeof(TExtendAttLog))==sizeof(TExtendAttLog))
				{
					yaffs_sync("/mnt");
					CurAttLogCount++;
				}
			}
			else
			{
				TAttLog fixattlog;

				memset(&fixattlog,0,sizeof(TAttLog));
				fixattlog.time_second = t;
				lseek(fdTransaction,-1*(TotalAttLogSize-CurAttLogSize),SEEK_CUR);
				if (write(fdTransaction,&fixattlog,sizeof(TAttLog))==sizeof(TAttLog))
				{
					yaffs_sync("/mnt");
					CurAttLogCount++;
					printf("fixok\n");
				}
			}
		}

	}

	//get max user id
	sh.ContentType=FCT_USER; 
	sh.buffer=buf;	
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{	    
		id=((PUser)sh.buffer)->PIN;
		if(id>UserMaxID) UserMaxID=id;
	}
	
	if(UserIDMap==NULL) 
		UserIDMap=(U32*)malloc(0x10000);
	i=FDB_CheckIntegrate();
	FDB_UpdateUserIDMap();
	return i;
}

void FDB_FreeDBs(void)
{
	close(fdTransaction);
	close(fdExtLog);
	close(fdFingerTmp);
	close(fdUser);
	close(fdOpLog);
	close(fdSms);
	close(fdUData);
	close(fdExtUser);
	close(fdWorkCode);
	if (UserIDMap) free(UserIDMap);
}

int GetDataInfo(int ContentType, int StatType, int Value)
{
	int tmp;	
	unsigned char buf[2048];
	TSearchHandle sh;
	
	sh.ContentType=ContentType; 
	sh.buffer=buf;
	
	tmp = 0;
	SearchFirst(&sh);
	while(!SearchNext(&sh)){
		switch(StatType)
		{
		case STAT_COUNT:
			if (sh.datalen>0) tmp++;
			break;
		case STAT_VALIDLEN:
			tmp+=sh.datalen;
			break;
		case STAT_CNTADMINUSER:	 
			if ((sh.datalen>0)&&(ISADMIN(((PUser)sh.buffer)->Privilege))) tmp++;
			break;
		case STAT_CNTADMIN:   
			if ((sh.datalen>0)&&(Value & (((PUser)sh.buffer)->Privilege))) tmp++;
			break;
		case STAT_CNTPWDUSER:   
			if ((sh.datalen>0)&&(((PUser)sh.buffer)->Password[0])) tmp++;
			break;
		case STAT_CNTTEMPLATE:   
			if ((sh.datalen>0)&&((((PTemplate)sh.buffer)->PIN)==Value)) tmp++;
			break;
		}
	}
	return tmp;
}

int TruncFDAndSaveAs(int fd, char *filename, char *buffer, int size)
{
	if (fd > 0) close(fd); 
	unlink(filename);
	fd = open(filename, O_RDWR|O_CREAT|O_TRUNC, S_IREAD | S_IWRITE);
	if (buffer!=NULL)
	{
		write(fd, buffer, size);
	}
	//sync();
	yaffs_flush(fd);
	yaffs_sync("/flash");
	return fd;
}

int FDB_ClearData(int ContentType)
{	
	char buf[80];
	
	if ((ContentType==FCT_ALL) || (ContentType==FCT_ATTLOG))
	{
		fdTransaction = TruncFDAndSaveAs(fdTransaction, GetEnvFilePath("USERDATAPATH", "transaction.dat", buf), NULL, 0);
		fdExtLog = TruncFDAndSaveAs(fdExtLog, GetEnvFilePath("USERDATAPATH", "extlog.dat", buf), NULL, 0);
		CurAttLogCount=0;
		BaseTime=0;
	}
	if ((ContentType==FCT_ALL) || (ContentType==FCT_FINGERTMP)) 
	{
		if (gOptions.ZKFPVersion == ZKFPV10)
		{
			if (IsDB8M)
			{
				if (ContentType==FCT_ALL)
					TruncFDAndSaveAs(fdFingerTmp, GetEnvFilePath("USERDATAPATH", "data/template.dat", buf), NULL, 0);
				fdFingerTmp = TruncFDAndSaveAs(fdFingerTmp, GetEnvFilePath("USERDATAPATH", "data/tempv10.dat", buf), NULL, 0);
			}
			else
			{
				if (ContentType==FCT_ALL)
					TruncFDAndSaveAs(fdFingerTmp, GetEnvFilePath("USERDATAPATH", "template.dat", buf), NULL, 0);
				fdFingerTmp = TruncFDAndSaveAs(fdFingerTmp, GetEnvFilePath("USERDATAPATH", "tempv10.dat", buf), NULL, 0);
			}
		}
		else
		{
			if (IsDB8M)
			{
				if (ContentType==FCT_ALL)
					TruncFDAndSaveAs(fdFingerTmp, GetEnvFilePath("USERDATAPATH", "data/tempv10.dat", buf), NULL, 0);
				fdFingerTmp = TruncFDAndSaveAs(fdFingerTmp, GetEnvFilePath("USERDATAPATH", "data/template.dat", buf), NULL, 0);
			}
			else
			{
				if (ContentType==FCT_ALL)
					TruncFDAndSaveAs(fdFingerTmp, GetEnvFilePath("USERDATAPATH", "tempv10.dat", buf), NULL, 0);
				fdFingerTmp = TruncFDAndSaveAs(fdFingerTmp, GetEnvFilePath("USERDATAPATH", "template.dat", buf), NULL, 0);
			}
		}
	}
	if ((ContentType==FCT_ALL) || (ContentType==FCT_USER))
	{ 
		fdUser = TruncFDAndSaveAs(fdUser, GetEnvFilePath("USERDATAPATH", "user.dat", buf), NULL, 0);
		fdExtUser = TruncFDAndSaveAs(fdExtUser, GetEnvFilePath("USERDATAPATH", "extuser.dat", buf), NULL, 0);
		if (ContentType==FCT_USER)
		{
			if (gOptions.ZKFPVersion == ZKFPV10)
			{
				if (IsDB8M)
					fdFingerTmp = TruncFDAndSaveAs(fdFingerTmp, GetEnvFilePath("USERDATAPATH", "data/tempv10.dat", buf), NULL, 0);
				else
					fdFingerTmp = TruncFDAndSaveAs(fdFingerTmp, GetEnvFilePath("USERDATAPATH", "tempv10.dat", buf), NULL, 0);
			}
			else
			{
				if (IsDB8M)
					fdFingerTmp = TruncFDAndSaveAs(fdFingerTmp, GetEnvFilePath("USERDATAPATH", "data/template.dat", buf), NULL, 0);
				else
					fdFingerTmp = TruncFDAndSaveAs(fdFingerTmp, GetEnvFilePath("USERDATAPATH", "template.dat", buf), NULL, 0);
			}

		}
	}
	if ((ContentType==FCT_ALL) || (ContentType==FCT_OPLOG)) 
		fdOpLog = TruncFDAndSaveAs(fdOpLog, GetEnvFilePath("USERDATAPATH", "oplog.dat", buf), NULL, 0);
	if ((ContentType==FCT_ALL) || (ContentType==FCT_SMS)) 
	{
		fdSms = TruncFDAndSaveAs(fdSms, GetEnvFilePath("USERDATAPATH", "sms.dat", buf), NULL, 0);
		fdUData = TruncFDAndSaveAs(fdUData, GetEnvFilePath("USERDATAPATH", "udata.dat", buf), NULL, 0);
	}
	if ((ContentType==FCT_ALL) || (ContentType==FCT_WorkCode)) 
	{
		fdWorkCode = TruncFDAndSaveAs(fdWorkCode, GetEnvFilePath("USERDATAPATH", "workcode.dat", buf), NULL, 0);
	}
	if ((ContentType==FCT_ALL) || (ContentType==FCT_UDATA)) 
	{
		fdUData = TruncFDAndSaveAs(fdUData, GetEnvFilePath("USERDATAPATH", "udata.dat", buf), NULL, 0);
	}
	
	if(ContentType==FCT_ALL)
	{
		BaseTime=0;
		UserMaxID=0;
	}
	
	if(ContentType==FCT_ALL || ContentType==FCT_USER)
	{	
		FDB_UpdateUserIDMap();
		UserMaxID=0;
	}
	
	if(ContentType==FCT_ALL || ContentType==FCT_USER || ContentType==FCT_FINGERTMP)
	{
		if (!gOptions.IsOnlyRFMachine) 
		 {
			 if (gOptions.ZKFPVersion == ZKFPV10)
				 BIOKEY_DB_CLEAR_10(fhdl);
			 else
				 BIOKEY_DB_CLEAR(fhdl);
		 }
	}
	//flush the cached data to disk
	yaffs_sync("/flash"); 
	return FDB_OK;
}

int FDB_GetSizes(char* Sizes)
{
	PFSizes p=(PFSizes)Sizes;
	
	memset((void*)p, 0, sizeof(TFSizes));
	
	p->AttLogCnt=GetDataInfo(FCT_ATTLOG, STAT_COUNT, 0);
	p->UserCnt=GetDataInfo(FCT_USER, STAT_COUNT, 0); 
	p->TmpCnt=GetDataInfo(FCT_FINGERTMP, STAT_COUNT, 0); 
	p->OpLogCnt=GetDataInfo(FCT_OPLOG, STAT_COUNT, 0);
	p->AdminCnt=FDB_CntAdminUser();
	p->PwdCnt=FDB_CntPwdUser();
	p->StdTmp=gOptions.MaxFingerCount*100; 
	p->StdUser=gOptions.MaxUserCount*100;
	p->StdLog=gOptions.MaxAttLogCount*10000;
	p->ResTmp=p->StdTmp-p->TmpCnt;
	p->ResUser=p->StdUser-p->UserCnt;
	p->ResLog=p->StdLog-p->AttLogCnt;
	
	return sizeof(TFSizes);
}

PUser FDB_CreateUser(PUser user, U16 pin, char *name, char *passwd, int privillege)
{
	memset((void*)user,0,sizeof(TUser));
	if(name) nstrcpy(user->Name, name, MAXNAMELENGTH);
	if(passwd) nstrcpy(user->Password, passwd, 5);
	user->PIN=pin;
	user->Privilege=privillege;
	user->Group=1;		//默认属于组1
	user->TimeZones=0;	//默认没有设置时间段
	return user;
}

int FDB_AddUser(PUser user)
{
	if(user->PIN>MAX_PIN) return FDB_ERROR_DATA;
	if(SearchAndSave(FCT_USER, (char*)user, sizeof(TUser))==FDB_OK)
	{
		SetBit((BYTE*)UserIDMap, user->PIN);
		return FDB_OK;
	}
	else
		return FDB_ERROR_IO;
}

int FDB_ChgUser(PUser user)
{
	PUser u;
	
	if((u=FDB_GetUser(user->PIN, NULL))==NULL) return FDB_ERROR_NODATA;
	if(0==memcmp((void*)user, (void*)u, sizeof(TUser))) return FDB_OK;
	//overwrite user 
	lseek(fdUser, -1*sizeof(TUser), SEEK_CUR);
	if (write(fdUser, (void*)user, sizeof(TUser))==sizeof(TUser))
	{
		yaffs_flush(fdUser);
		return FDB_OK;
	}
	else
		return FDB_ERROR_IO;
}

int FDB_DelUser(U16 pin)
{
	PUser u;
	int uid;
	
	if ((u=FDB_GetUser(pin, NULL))==NULL) return FDB_ERROR_NODATA;
	uid=u->PIN;
	//overwrite user 
	lseek(fdUser, -1*sizeof(TUser), SEEK_CUR);
	u->PIN=0;
	if (write(fdUser, (void*)u, sizeof(TUser))==sizeof(TUser))
	{
		if(gOptions.UserExtendFormat) FDB_DelExtUser((U16)uid);
		ClearBit((BYTE*)UserIDMap, pin);
		yaffs_flush(fdUser);
		return FDB_DeleteTmps((U16)uid);
	}
	return FDB_ERROR_IO;
}

static TUser gUser;

PUser FDB_GetUser(U16 pin, PUser user)
{
	TSearchHandle sh;
	
	sh.ContentType=FCT_USER; 
	//maybe modified user value so do the following changes
	//if (user==NULL)
	//    sh.buffer=(unsigned char*)&gUser;
	//else
	//    sh.buffer=(unsigned char*)user;
	sh.buffer=(unsigned char*)&gUser;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if(((PUser)sh.buffer)->PIN==pin)
		{
			if (user)
			{
				memcpy(user, sh.buffer, sizeof(TUser));
				return user;
			}
			else
				return (PUser)sh.buffer;
		}
	}
	return NULL;
}

PUser FDB_GetUserByPIN2(U32 pin2, PUser user)
{
	TSearchHandle sh;
	
	if(pin2<=0) return NULL;
	if(pin2>MAX_PIN2) return NULL;
	sh.ContentType=FCT_USER; 
	sh.buffer=(unsigned char*)&gUser;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if ((sh.datalen>0) && (((PUser)sh.buffer)->PIN2==pin2))
		{
			if (user)
			{
				memcpy(user, sh.buffer, sizeof(TUser));
				return user;
			}
			else
				return (PUser)sh.buffer;
		}
	}
	return NULL;
}

PUser FDB_GetUserByCard(BYTE *card, PUser user)
{
	TSearchHandle sh;
	int value;
	
	sh.ContentType=FCT_USER; 
	sh.buffer=(unsigned char*)&gUser;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		memcpy(&value,((PUser)sh.buffer)->IDCard,4);
		if((sh.datalen>0) && (nmemcmp((((PUser)sh.buffer)->IDCard), card, sizeof(((PUser)sh.buffer)->IDCard))==0))
		{
			if (user)
			{
				memcpy(user, sh.buffer, sizeof(TUser));
				return user;
			}
			else
				return (PUser)sh.buffer;
		}
	}
	return NULL;
}

int FDB_CntAdminUser(void)
{
	return GetDataInfo(FCT_USER, STAT_CNTADMINUSER, 0);
}

int FDB_CntAdmin(int Privilege)
{
	return GetDataInfo(FCT_USER, STAT_CNTADMIN, Privilege);
}

int FDB_CntPwdUser(void)
{
	return GetDataInfo(FCT_USER, STAT_CNTPWDUSER, 0);
}

int FDB_ClrUser(void)
{
	return FDB_ClearData(FCT_USER);
}

int FDB_CntUser(void)
{
	return GetDataInfo(FCT_USER, STAT_COUNT, 0);
}

void CopyTemplate(PTemplate dest, PTemplate src)
{
	dest->PIN=src->PIN;
	dest->FingerID=src->FingerID;
	memcpy(dest->Template, src->Template, MAXTEMPLATESIZE);
	dest->Valid=src->Valid;
	dest->Size=src->Size;
}

PTemplate FDB_CreateTemplate(PTemplate tmp, U16 pin, char FingerID, char *Template, int TmpLen)
{
	tmp->PIN=pin;
	tmp->FingerID=(BYTE)FingerID;

	if (gOptions.ZKFPVersion == ZKFPV10)
	{
		if (TmpLen > ZKFPV10_MAX_LEN)
			TmpLen = ZKFPV10_MAX_LEN;
	}

	memcpy(tmp->Template, (BYTE*)Template, TmpLen);
	tmp->Valid=1;
	if (gOptions.ZKFPVersion == ZKFPV10)
	{
		tmp->Size=TmpLen;
	}
	else
	{
		if (TmpLen>MAXVALIDTMPSIZE)
			TmpLen=BIOKEY_SETTEMPLATELEN(Template, MAXVALIDTMPSIZE);
		//Make a 4-Byte alignment for all template data
		tmp->Size=((TmpLen+3+6)/4)*4-6;
	}
	return tmp;
}

int FDB_AddTmp(PTemplate tmp)
{
        int tmplen = sizeof(TTemplate);
	if (gOptions.ZKFPVersion != ZKFPV10)
	{
		tmplen -= ZKFP_OFF_LEN;
		tmp->Size+=6;
	}
	return SearchAndSave(FCT_FINGERTMP, (char*)tmp, tmplen);
}

int DeleteFingerData(void)
{
	int size=sizeof(TTemplate);
	if (gOptions.ZKFPVersion != ZKFPV10)
		size -= ZKFP_OFF_LEN;
	TTemplate deltmp;

	memset(&deltmp,0,sizeof(TTemplate));
	//overwrite templates set valid = 0 
	//lseek(fdFingerTmp, -1*size + 5, SEEK_CUR);
	lseek(fdFingerTmp, -1*size, SEEK_CUR);
	//if (write(fdFingerTmp, (void*)&valid, 1)==1)
	if (write(fdFingerTmp, (void*)&deltmp, size)==size)
	{
		//lseek(fdFingerTmp, size - 6, SEEK_CUR);	    
		//lseek(fdFingerTmp, size, SEEK_CUR);	    
		yaffs_flush(fdFingerTmp);
		return FDB_OK;
	}
	else
	{
		//lseek(fdFingerTmp, size - 5, SEEK_CUR);
		lseek(fdFingerTmp, size, SEEK_CUR);	    
		return FDB_ERROR_IO;
	}
}

int FDB_DelTmp(U16 UID, char FingerID)
{
	int j = -1;

	if (FDB_GetTmp(UID, FingerID, NULL)==0)
		return FDB_ERROR_NODATA;
	else
	{
		j = DeleteFingerData();
		if(FDB_OK ==j && fhdl)
		{
			if (gOptions.ZKFPVersion == ZKFPV10)
				BIOKEY_DB_DEL_10(fhdl, UID|(FingerID<<16));
			else
				BIOKEY_DB_DEL(fhdl, UID|(FingerID<<16));
		}
		return j;
	}
}

char *GetUserTmpsV10(int *Size, U16 pin)
{
	int i, cnt, fingerCount=0;
	int lengthTmp=0;
	TTemplate tmp;
	BYTE *blockBuf;
	int validLen=0;
	int *UserIDs = NULL;

	if (!fhdl)
		return NULL;
	UserIDs = (int *)malloc(gOptions.MaxUserCount*1000*sizeof(int));
	if (UserIDs == NULL)
		return NULL;
	memset((char*)UserIDs, 0, (gOptions.MaxUserCount*1000)*sizeof(int));
	BIOKEY_GET_PARAMETER_10(fhdl,ZKFP_PARAM_CODE_USERCOUNT,&fingerCount);
	BIOKEY_GET_PARAMETER_10(fhdl,ZKFP_PARAM_CODE_USERIDS,UserIDs);

	*Size = 4;
	if(fingerCount > 0)
	{
		blockBuf = (BYTE*)malloc(10*ZKFPV10_MAX_LEN+4);
		if(!blockBuf)
		{
			free(UserIDs);
			return NULL;
		}
	}
	else
	{
		free(UserIDs);
		return NULL;
	}

	cnt = 0;
	for (i=0;i<fingerCount&&cnt<10;i++)
	{
		if ((UserIDs[i]&0xffff) != pin)
			continue;
		memset(&tmp,0,sizeof(TTemplate));
		lengthTmp=FDB_GetTmp(UserIDs[i]&0xffff,(UserIDs[i]>>16)&0xf,(char *)&tmp);
		if(lengthTmp<=0)
			continue;
		tmp.Size+=6;
		memcpy(blockBuf+*Size,(char*)&tmp,6);
		*Size +=6;
		memcpy(blockBuf+*Size,tmp.Template,tmp.Size-6);
		*Size += tmp.Size-6;
		cnt++;
	}
	validLen = *Size -4;
	memcpy(blockBuf,&validLen,4);
	free(UserIDs);
	return blockBuf;
}

int FDB_VerifyFinger(U16 UID, int *FingerID, PTemplate LastTemplate)
{
	TSearchHandle sh;
	int score = 0;

	sh.ContentType=FCT_FINGERTMP; 
	sh.buffer=(char *)gTemplate;
	if (LastTemplate==NULL || UID==0)
		return 0;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if(((PTemplate)sh.buffer)->PIN == UID)
		{
			if (gOptions.ZKFPVersion == ZKFPV10)
				score=BIOKEY_VERIFY_10(fhdl, ((PTemplate)gTemplate)->Template, LastTemplate);
			else
				score=BIOKEY_VERIFY(fhdl, ((PTemplate)gTemplate)->Template, LastTemplate);
			
			if(gOptions.VThreshold<=score)
			{
				*FingerID = ((PTemplate)sh.buffer)->FingerID;
				return score;
			}
		}
	}

	*FingerID = gOptions.MaxUserFingerCount;
	return 0;
}

U32 FDB_GetTmp(U16 UID, char FingerID, PTemplate tmp)
{
	TSearchHandle sh;
	
	sh.ContentType=FCT_FINGERTMP; 
	if (tmp==NULL)
		sh.buffer=(char *)gTemplate;
	else
		sh.buffer=(char *)tmp;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (sh.datalen==0){
			if (UID==0)
				return sh.bufferlen;
			else
				continue;
		}
		if((((PTemplate)sh.buffer)->PIN==UID) && (((PTemplate)sh.buffer)->FingerID==FingerID))
		{
			((PTemplate)sh.buffer)->Size-=6;
			return sh.datalen;
		}
	}
	return 0;
}

int FDB_DeleteTmps(U16 UID)
{
	TSearchHandle sh;
	
	sh.ContentType=FCT_FINGERTMP; 
	sh.buffer=(char *)gTemplate;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (sh.datalen==0) continue; 
		if (((PTemplate)sh.buffer)->PIN==UID)
		{
			if(FDB_OK!=DeleteFingerData())
				return FDB_ERROR_IO;
		}
	}
	return FDB_OK;
}

int FDB_ClrDuressTagTmpAll(void)
{
	TSearchHandle sh;
	PTemplate tmp;
	int size=sizeof(TTemplate);
	
	sh.ContentType=FCT_FINGERTMP; 
	sh.buffer=(char *)gTemplate;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (sh.datalen==0) continue; 
		tmp=(PTemplate)sh.buffer;
		tmp->Valid &= ~DURESSFINGERTAG;
		
		lseek(fdFingerTmp, -1*size, SEEK_CUR);
		if (write(fdFingerTmp, (void*)tmp, size)!=size)
			return FDB_ERROR_IO;
	}
	return FDB_OK;	
}

U32 FDB_GetTmpCnt(U16 UID)
{
	return GetDataInfo(FCT_FINGERTMP, STAT_CNTTEMPLATE, UID);
}

int FDB_ChgTmpValidTag(PTemplate tmp, BYTE SetTag, BYTE ClearTag)
{
	U8 valid=0;
	int size=sizeof(TTemplate);
	
	if (FDB_GetTmp(tmp->PIN, tmp->FingerID, NULL)==0)
		return FDB_ERROR_NODATA;
	else
	{
		tmp->Valid |= SetTag; 
		tmp->Valid &= (BYTE)~ClearTag;
		valid=tmp->Valid;		
		lseek(fdFingerTmp, -1*size + 5, SEEK_CUR);
		if (write(fdFingerTmp, (void*)&valid, 1)==1)
		{
			yaffs_flush(fdFingerTmp);
			lseek(fdFingerTmp, size - 6, SEEK_CUR);	    
			return FDB_OK;
		}
		else
		{
			lseek(fdFingerTmp, size - 5, SEEK_CUR);
			return FDB_ERROR_IO;
		}					
	}
}

int FDB_IsDuressTmp(U16 PIN, char FingerID)
{
	TTemplate tmp;
	if(FDB_GetTmp(PIN, FingerID, &tmp))
		return ISDURESSFP(&tmp);
	else
		return FALSE;
}

int g1ToNTemplates=0;
int GroupFpCount[5]={0,0,0,0,0};
char *gGroupPin;
PGroupUserRec GroupUsers;
static Groupusercount=0;

int GetGroupUserPin()
{
        TSearchHandle sh;
        TUser User;
        int i=0;

	gGroupPin = malloc(35*2048);
	GroupUsers=(PGroupUserRec)gGroupPin;

        sh.ContentType=FCT_USER;
        sh.buffer=(unsigned char*)&User;   
	SearchFirst(&sh);
        while(!SearchNext(&sh))
        {
                if(sh.datalen>0)
                {              
			GroupUsers[i].Group = ((PUser)sh.buffer)->Group&0x0F;
			GroupUsers[i].PIN = ((PUser)sh.buffer)->PIN;
			i++;
                }         
	}
	return i;
}

int QueryGroupUser(U16 Pin)
{
	int ret= 0;
	int i=0;

	while (i>=0 && i < Groupusercount)
	{
		if (GroupUsers[i].PIN == Pin)
		{
			ret = GroupUsers[i].Group;			
			return ret;
		}
		i++;
	}
	return ret;	
}


int FDB_LoadTmp(void *Handle)
{
	TSearchHandle sh;
	PTemplate tmp;
	U32 pin2;
        struct timeval tv;
        struct timezone tz;
        unsigned int s_msec, e_msec;
	U8 Group=0;
	int tmplen;
	BYTE *template;
	int MinTemplateLen=gOptions.MinMinutiae*5;
	
	g1ToNTemplates=0;
	GroupFpCount[0] = 0;
	GroupFpCount[1] = 0;
	GroupFpCount[2] = 0;
	GroupFpCount[3] = 0;
	GroupFpCount[4] = 0;
	if(!gOptions.Must1To1 || gOptions.I1ToG || gOptions.I1ToH)
	{
	//	gettimeofday(&tv, &tz);
	//	s_msec = tv.tv_sec*1000*1000 + tv.tv_usec;

		//先取用户pin,Group
		if (gOptions.I1ToG &&(gOptions.GroupFpLimit != gOptions.LimitFpCount))
		{
			 Groupusercount = GetGroupUserPin();
			 //DBPRINTF("Groupusercount: %d\n",Groupusercount);
		}

		
		sh.ContentType=FCT_FINGERTMP; 
		sh.buffer=(char *)gTemplate;
		SearchFirst(&sh);
		while(!SearchNext(&sh))
		{
			if(((U8*)sh.buffer)[5]) //valid
			{
				tmp=(PTemplate)sh.buffer;
				//增加模板size合法性判断 2007.01.30
				if ((tmp->Size-6) > MinTemplateLen)
				{

					//定义了1:N用户的号码范围
					if(gOptions.I1ToNTo>0)
					{
						if(gOptions.PIN2Width>PIN_WIDTH)
							pin2=(FDB_GetUser(tmp->PIN, NULL))->PIN2;
						else
							pin2=tmp->PIN;
						if(pin2==0) continue;
						if(pin2<gOptions.I1ToNFrom) continue;
						if(pin2>gOptions.I1ToNTo) continue;
					}
					//Fix template if have some errors
                                        tmplen=tmp->Size-6;
					if (gOptions.ZKFPVersion == ZKFPV10)
					{
						if(tmplen<BIOKEY_TEMPLATELEN_10(tmp->Template))
						{
							//we will ignore block property and recalculate template length
							template=tmp->Template+3;
							*template=*template&0x7F; //highest bit is block
							tmplen=BIOKEY_TEMPLATELEN_10(tmp->Template);
							if(tmplen>(tmp->Size-6)) continue;
						}
					}
					else
					{
						if(tmplen<BIOKEY_TEMPLATELEN(tmp->Template))
						{
							//we will ignore block property and recalculate template length
							template=tmp->Template+3;
							*template=*template&0x7F; //highest bit is block
							tmplen=BIOKEY_TEMPLATELEN(tmp->Template);
							if(tmplen>(tmp->Size-6)) continue;
						}
					}
					if (gOptions.I1ToG &&(gOptions.GroupFpLimit != gOptions.LimitFpCount))
					{
						Group = QueryGroupUser(tmp->PIN);
						if (Group && (GroupFpCount[Group-1] <= gOptions.GroupFpLimit))
						{
							if (gOptions.ZKFPVersion == ZKFPV10)
								BIOKEY_DB_ADD_10(Handle, tmp->PIN|(tmp->FingerID<<16), tmp->Size-6, (BYTE*)tmp->Template); 
							else
								BIOKEY_DB_ADD(Handle, tmp->PIN|(tmp->FingerID<<16), tmp->Size-6, (BYTE*)tmp->Template); 
							GroupFpCount[Group-1]++;
							g1ToNTemplates++;
							
						}				
					
					}
					else
					{
						if (gOptions.ZKFPVersion == ZKFPV10)
                                                	BIOKEY_DB_ADD_10(Handle, tmp->PIN|(tmp->FingerID<<16), tmplen, (BYTE*)tmp->Template);
						else
                                                	BIOKEY_DB_ADD(Handle, tmp->PIN|(tmp->FingerID<<16), tmplen, (BYTE*)tmp->Template);
                                                g1ToNTemplates++;
                                        }


//					DBPRINTF("PIN = %d SIZE =%d\t tmplatelen: %d\n", tmp->PIN, tmp->Size-6,tmplen);
				}
			}
		}
		
	//	gettimeofday(&tv, &tz);
	//	e_msec = tv.tv_sec*1000*1000 + tv.tv_usec;
	//	DBPRINTF("DELAY US=%d\n", e_msec-s_msec); 

		if (gOptions.I1ToG &&(gOptions.GroupFpLimit != gOptions.LimitFpCount))
			free(gGroupPin);
	

		return g1ToNTemplates;
	}
	else
		return 0;
}

int FDB_ClrTmp(void)
{
	return FDB_ClearData(FCT_FINGERTMP);
}

int FDB_CntTmp(void)
{
	return GetDataInfo(FCT_FINGERTMP, STAT_COUNT, 0);
}

int PackAttLog(char *buf, PAttLog log)
{
	U32 t=log->time_second-BaseTime;
	
	memset(buf, 0, 4);
	memcpy(buf, &log->PIN, 2);
	buf[2]=((log->status & 0x07) << 5) | ((log->verified & 0x03) << 3);
	if(gOptions.CompressAttlog && (t>0) && (t<2048))//short time format
	{
		buf[2]=buf[2] | 4 | ((t & 0x300) >>8);
		buf[3]=t & 0xff;
		return AttLogSize1;
	}
	else
	{
		memcpy(buf+4,&log->time_second, 4);
		BaseTime=log->time_second;
		return AttLogSize2;
	}
}

int UnpackAttLog(char *buf, PAttLog log)
{
	memcpy(&log->PIN, buf, 2);
	log->status=(buf[2] >> 5) & 7;
	log->verified=(buf[2] >> 3) & 3;
	if(buf[2] & 4)//short time format
	{
		log->time_second=(U8)buf[3]+((buf[2] & 3) << 8);
		return AttLogSize1;
	}
	else
	{
		memcpy(&log->time_second, buf+4, 4);
		return AttLogSize2;
	}
}

int FDB_AddAttLog(U16 pin, time_t t, char verified, char status, U32 pin2, U32 workcode, U8 SensorNo)
{
	int s;
	TAttLog log;
	TExtendAttLog extlog;
	char buf[16];
	if(gOptions.AttLogExtendFormat)
	{
		extlog.PIN=(pin2?pin2:pin);
		extlog.time_second=t;
		extlog.status=status;
		extlog.verified=verified;
		extlog.reserved[0]=SensorNo;
		extlog.reserved[1]=0;
		extlog.workcode=workcode;
		s=16;
		memcpy(buf, (void *)&extlog, s);
	}
	else
	{
		log.status=status;
		log.time_second=t;
		log.PIN=pin;
		log.verified=verified;
		s=PackAttLog(buf,&log);
	}
	return SearchAndSave(FCT_ATTLOG, (void*)buf, s);
}

int FDB_ClrAttLog(void)
{
	return FDB_ClearData(FCT_ATTLOG);
}

int FDB_CntAttLog(void)
{
	return GetDataInfo(FCT_ATTLOG, STAT_COUNT, 0);
}

//二分法查找
int SearchInLogs(PAttLog logs, time_t t, int count)
{
        int found=0, start=0, end=count-1;
        U32 tt;
        while(start<=end)
        {
                int i=(start+end)/2,j;
                j=0;
                tt=logs[i].time_second;
                if(tt>t)
                        end=i-1;
                else if(tt<t)
                        start=i+1;
                else
                {
                        start=i;
                        found=1;
                        break;
                }
        }
        return start;
}

int SearchInExtLogs(PExtendAttLog logs, time_t t, int count)
{
        int found=0, start=0, end=count-1;
        U32 tt;
        while(start<=end)
        {
                int i=(start+end)/2,j;
                j=0;
                tt=logs[i].time_second;
                if(tt>t)
                        end=i-1;
                else if(tt<t)
                        start=i+1;
                else
                {
                        start=i;
                        found=1;
                        break;
                }
        }
        return start;
}

//查询考勤记录
void AddToOrderedLogs(PAttLog logs, PAttLog log, int count)
{
        int index=SearchInLogs(logs, log->time_second, count);

        int i;
        for(i=count;i>index;i--)
        {
                logs[i]=logs[i-1];
        }

        logs[index]=*log;
}

void AddToOrderedExtLogs(PExtendAttLog logs, PExtendAttLog log, int count)
{
        int index=SearchInExtLogs(logs, log->time_second, count);

        int i;
        for(i=count;i>index;i--)
        {
                logs[i]=logs[i-1];
        }

        logs[index]=*log;
}

static TAttLog gattlog;
static TExtendAttLog gattextlog;

int FDB_GetAttLog(U16 pin, time_t StartTime,PAttLog logs, int MaxCount)
{
        TSearchHandle sh;
        int count=0;
        TAttLog log;
        time_t lastt=0;
        sh.ContentType=FCT_ATTLOG;
        sh.buffer = (unsigned char *)&gattlog;
        SearchFirst(&sh);
        while(!SearchNext(&sh))
        {
                int size=UnpackAttLog(sh.buffer, &log);
                if(size==AttLogSize2)
                        lastt=log.time_second;
                else
                        log.time_second+=lastt;
                if((pin==0 || pin==log.PIN) && (log.time_second>=StartTime))
                {
                       if (log.time_second > 0)
                                 AddToOrderedLogs(logs, &log, count++);
                        if(count>=MaxCount) break;
                }
        }
        return count;
}

int FDB_GetAttExtLog(U16 pin, time_t StartTime,PExtendAttLog logs, int MaxCount)
{
        TSearchHandle sh;
        int count=0;
        TExtendAttLog log;
        sh.ContentType=FCT_ATTLOG;
        sh.buffer = (unsigned char *)&gattextlog;
        SearchFirst(&sh);
        while(!SearchNext(&sh))
        {
		memcpy(&log, sh.buffer, sizeof(TExtendAttLog));
                if((pin==0 || pin==log.PIN) && (log.time_second>=StartTime))
                {
                       if (log.time_second > 0)
                                 AddToOrderedExtLogs(logs, &log, count++);
                        if(count>=MaxCount) break;
                }
        }
        return count;
}

char* FDB_ReadBlockByFD(int *size, int ContentType, int fd) 
{
	TSearchHandle sh;
	U8 buf[2048];
	char data[4];
	int validLen;
	int cnt=0;
	U8 *validBuf;
	U8 *p;

	validLen=GetDataInfo(ContentType, STAT_VALIDLEN, 0);
	*size=validLen+4;
	if (validLen>0)
	{
		if (fd > 0)
		{
			memcpy(data, &validLen, 4);
			write(fd, data, 4);
		}
		else
		{
			validBuf=malloc(validLen+4);
			memcpy(validBuf, &validLen, 4);
			p=validBuf+4;
		}

		sh.ContentType=ContentType; 
		sh.buffer=buf;
		SearchFirst(&sh);
		while(!SearchNext(&sh))
		{
			if(sh.datalen>0)
			{
				cnt++;
				if (fd > 0)
					write(fd, sh.buffer, sh.datalen);
				else
				{
					memcpy(p, sh.buffer, sh.datalen);
					p+=sh.datalen;
				}

				if (cnt%500 == 0)
					SwitchUDCAndDelay();	
			}
		}

		if (fd > 0)
		{
			yaffs_flush(fd);
			return malloc(16);
		}
		else
			return validBuf;
	} 

	return NULL;
}

char* FDB_ReadBlock(int *size, int ContentType) 
{
	TSearchHandle sh;
	U8 buf[2048];
	int validLen;
	U8 *validBuf;
	U8 *p;
	
	validLen=GetDataInfo(ContentType, STAT_VALIDLEN, 0);
	*size=validLen+4;
	if (validLen>0)
	{
		validBuf=malloc(validLen+4);
		memcpy(validBuf, &validLen, 4);
		p=validBuf+4;
		
		sh.ContentType=ContentType; 
		sh.buffer=buf;
		SearchFirst(&sh);
		while(!SearchNext(&sh))
		{
			if(sh.datalen>0)
			{
				memcpy(p, sh.buffer, sh.datalen);
				p+=sh.datalen;
			}
		}
		return validBuf; 
	} 
	else
		return NULL;
}
/*
char* FDB_ReadUserBlock(int *size)
{
	return FDB_ReadBlock(size, FCT_USER);	
}

char* FDB_ReadAttLogBlock(int *size)
{
	return FDB_ReadBlock(size, FCT_ATTLOG);
}
*/
int IsFreePIN(char *pin)
{
	U32 PIN;
	if(0==strtou32(pin, &PIN))
	{
		if(PIN>MAX_PIN) return FALSE;
		if(PIN==0) return FALSE;
		return !GetBit((BYTE*)UserIDMap, PIN);
	}
	else
		return FALSE;
}

int IsUsedPIN(char *pin)
{
	U32 PIN;
	if(0!=strtou32(pin, &PIN))
		return FALSE;
	else
		return !IsFreePIN(pin);
}

int IsFreePIN2(char *pin)
{
	U32 PIN;
	if(0==strtou32(pin, &PIN))
	{
		if(PIN>MAX_PIN2) return FALSE;
		if(PIN==0) return FALSE;
		return (NULL==FDB_GetUserByPIN2(PIN,NULL));
	}
	else
		return FALSE;
}

int IsUsedPIN2(char *pin)
{
	U32 PIN;
	if(0!=strtou32(pin, &PIN))
		return FALSE;
	else
		return !IsFreePIN2(pin);
}


U16 GetNextPIN(int From, int Free)
{
	U16 i=From;
	do{
		if(Free^GetBit((BYTE*)UserIDMap, i))
		{
			return i;
		}
	}
	while(++i<=MAX_PIN);
	return 0;
}

int FDB_UpdateUserIDMap(void)
{
	TSearchHandle sh;
	U8 buf[128];
	int ret=0;
	
	memset((char*)UserIDMap, 0, 0x10000);	
	
	sh.ContentType=FCT_USER;
	sh.buffer=buf;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if(sh.datalen==0) continue;
		SetBit((BYTE*)UserIDMap, ((PUser)sh.buffer)->PIN);
		ret++;
	}
	return ret;
}


//查找指定用户可用（未登记）的指纹编号
//若指定的指纹编号已经登记，则返回错误
//若指定的指纹编号>=gOptions.MaxUserFingerCount，则表示要查找一个未用（没有登记）的指纹编号
int FDB_GetFreeFingerID(U16 PIN, BYTE *FID)
{
	int i;
		
	if(FDB_CntTmp()==gOptions.MaxFingerCount*100)
	{
		return FDB_OVER_FLIMIT;
	}
	if(*FID>=gOptions.MaxUserFingerCount)
	{
		if(FDB_CntTmp()==gOptions.MaxFingerCount*100)
		{
			return FDB_OVER_FLIMIT;
		}
		for(i=0;i<gOptions.MaxUserFingerCount;i++)
		{
			if(0==FDB_GetTmp(PIN, (char)i, NULL))
			{
				*FID=i;
				return FDB_OK;
			}
		}
		return FDB_OVER_UFLIMIT;
	}
	else if(FDB_GetTmp(PIN, *FID, NULL))
	{
		return FDB_FID_EXISTS;
	}
	return FDB_OK;
}

int FDB_AddOPLog(U16 Admin, BYTE OP, U16 Objs1, U16 Objs2, U16 Objs3, U16 Objs4)
{
	int ret;
	TOPLog log;
	
	if(OP<32)
	{
		if((gOptions.OPLogMask1 & (1<<OP))==0) return FDB_ERROR_DATA;
	}
	else if(OP<64)
	{
		if((gOptions.OPLogMask2 & (1<<(OP-32)))==0) return FDB_ERROR_DATA;
	}
	else
		return FDB_ERROR_DATA;
	log.Admin=Admin;
	log.OP=OP;
	log.time_second=OldEncodeTime(&gCurTime);
	log.Users[0]=Objs1;log.Users[1]=Objs2;log.Users[2]=Objs3;log.Users[3]=Objs4;
	if(FDB_CntOPLog()>=64*2048/sizeof(TOPLog)) FDB_ClrOPLog();
	ret=SearchAndSave(FCT_OPLOG, (char*)&log, sizeof(TOPLog));
	if(FDB_CntOPLog()+gOptions.AlarmOpLog>=MAX_OPLOG_COUNT)
	{
		char buf[50];
		sprintf(buf, LoadStrByID(MID_ADMIN_REC), MAX_OPLOG_COUNT-FDB_CntOPLog());
		LCDInfoShow(LoadStrByID(MID_OR_AADMINLOG), buf);
		DelayMS(2*1000);
	}
	return ret;
}

int FDB_ClrOPLog(void)
{
	return FDB_ClearData(FCT_OPLOG);
}

int FDB_CntOPLog(void)
{
	return GetDataInfo(FCT_OPLOG, STAT_COUNT, 0);
}

char* FDB_ReadOPLogBlock(int *size)
{
	return FDB_ReadBlock(size, FCT_OPLOG);
}

int FDB_ClrAdmin(void)
{
	char *buf;
	char tmp[80];
	int size, buflen;
	PUser usr;
	
	if((buf=FDB_ReadBlock(&size, FCT_USER))==NULL) return FDB_ERROR_IO;
	buflen=size;
	usr=(PUser)(buf+4);
	while((usr->PIN<=MAX_PIN) && (size>=sizeof(TUser)))
	{
		usr->Privilege=usr->Privilege & 1;
		usr++;
		size-=sizeof(TUser);
	}
	fdUser=TruncFDAndSaveAs(fdUser, GetEnvFilePath("USERDATAPATH", "user.dat", tmp), buf+4, buflen-4);
	free(buf);
	return FDB_OK;
}

int FDB_CheckIntegrate(void)		//进行数据库正确性、完整性检查
{
	return FDB_OK;
}

#if MACHINE_ID == 2
BOOL FDB_Download(int ContentType, char *dstFileName) 
{
	TSearchHandle sh;
	U8 buf[2048];
	char fmt[64];
	FIL dstHandle;
	FRESULT fat_err;
	int datalen=0;
	
	TAttLog log;
	time_t t=0;
	int s;
	TTime tt;
	U32 WorkCode, PIN;
	BYTE status, verified;
	int pos, plen;
	int iCnt=0, iStart=0, probar=0;
	int typesize;

	f_unlink(dstFileName);
	fat_err=f_open(&dstHandle, dstFileName, FA_READ|FA_WRITE|FA_CREATE_ALWAYS);
	if (fat_err) return FALSE;
	f_lseek(&dstHandle, 0);
	sh.ContentType=ContentType; 
	sh.buffer=buf;
	SearchFirst(&sh);

	iCnt = lseek(sh.fd, 0, SEEK_END);
	typesize = GetTypeSize(ContentType);
	if (ContentType == FCT_USER)
	{
		if (!gOptions.IsOnlyRFMachine)
			iCnt /= (typesize*PROGBAR_USER_NUM);
		else
			iCnt /= (typesize*PROGBAR_MAX_NUM);
	}
	else if (ContentType == FCT_FINGERTMP)
	{
		iCnt /= (typesize*PROGBAR_FP_NUM);
		iStart = PROGBAR_USER_NUM;
	}
	else
		iCnt /= (typesize*PROGBAR_MAX_NUM);

	if (0 == iCnt)
		iCnt = 1;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		wdt_set_count(0);
		if(sh.datalen>0)
		{	
			if (ContentType==FCT_ATTLOG)
			{
				if(gOptions.AttLogExtendFormat)
				{
					PIN=((PExtendAttLog)(sh.buffer))->PIN;
					log.time_second=((PExtendAttLog)(sh.buffer))->time_second;
					status=((PExtendAttLog)(sh.buffer))->status;
					verified=((PExtendAttLog)(sh.buffer))->verified;                 
					WorkCode=((PExtendAttLog)(sh.buffer))->workcode;
				}
				else
				{
					s=UnpackAttLog(sh.buffer, &log);
					if(s==AttLogSize2)
						t=log.time_second;
					else
						log.time_second+=t;
					PIN=log.PIN;
					status=log.status;
					verified=log.verified;
					WorkCode=0;
				}
				OldDecodeTime(log.time_second, &tt);

				sprintf(fmt, "%%%dd%s", gOptions.PIN2Width,
						"\t%4.4d-%02d-%02d %02d:%02d:%02d\t%d\t%d\t%d\t%d\r\n");
				sprintf(sh.buffer,
						fmt,
						PIN,
						tt.tm_year+1900,tt.tm_mon+1,tt.tm_mday,tt.tm_hour, tt.tm_min, tt.tm_sec,
						gOptions.DeviceID,
						status,
						verified,
						WorkCode
				       );
				sh.bufferlen=strlen(sh.buffer);   
			}			
		
			pos = 0;
			plen = 512;
			while (sh.bufferlen > 0)
			{
				if (sh.bufferlen > plen)
				{
					sh.bufferlen -= plen;
				}
				else
				{
					plen = sh.bufferlen;
					sh.bufferlen = 0;
				}

				if (f_write(&dstHandle, sh.buffer + pos, plen, &datalen) || datalen != plen)
				{
					f_close(&dstHandle);
					return FALSE;
				}
				pos += plen;
			}
			
			if (ContentType == FCT_USER)
			{
				if (probar < PROGBAR_USER_NUM*iCnt)
					probar++;
			}
			else
				probar++;
			DrawProgbar(iStart + probar/iCnt);
		}
	}
	f_close(&dstHandle);
	if (FCT_USER != ContentType)
		DrawProgbar(PROGBAR_MAX_NUM);
	return TRUE;
}
#else
BOOL FDB_Download(int ContentType, char *dstFileName) 
{
	return TRUE;
}
#endif

int FDB_AddTemplate_New(PTemplate tmp)
{
	int fd;
	int rc;
        int tmplen = sizeof(TTemplate);
	if (gOptions.ZKFPVersion != ZKFPV10)
	{
		tmplen -= ZKFP_OFF_LEN;
		tmp->Size+=6;
	}
	
	fd = SelectFDFromConentType(FCT_FINGERTMP);
	lseek(fd, 0, SEEK_END);		
	rc=((write(fd, (char*)tmp, tmplen)==tmplen)?FDB_OK:FDB_ERROR_IO);
	yaffs_flush(fd);
	yaffs_sync("/flash");
	return rc;
}

//按指定用户号码、姓名和指纹编号新增指纹
//若指定的指纹存在，则覆盖
//若指定的指纹编号大于允许范围（gOptions.MaxUserFingerCount-1），则尝试使用一个未用的指纹编号来保存
//当name==NULL时，表示仅仅新增指纹，而且用户必须事先存在
//成功时返回 FDB_OK
int AppendUserTemp(int pin, char *name, int fingerid, char *temp, int tmplen)
{
	int ret;
	TUser user;

	if(pin>0 && FDB_GetUser(pin,&user))
	{
		BYTE fid=(char)fingerid;
		if (gOptions.ZKFPVersion != ZKFPV10)
		{
			if((ret=FDB_GetFreeFingerID(user.PIN, &fid))!=FDB_OK)
			{
				if(ret!=FDB_FID_EXISTS) return ret;
				if(name==NULL || (FDB_DelTmp(user.PIN, fid)!=FDB_OK))
				{
					return FDB_FID_EXISTS; //old 3
				}
			}
		}
		fingerid=fid;
		if(FDB_CntTmp()>=gOptions.MaxFingerCount*100)
			return FDB_OVER_FLIMIT;
	}
	else if(name)
	{
		if(FDB_CntTmp()>=gOptions.MaxFingerCount*100)
			return FDB_OVER_FLIMIT;
		if(!FDB_CreateUser(&user, pin, name, NULL, 0))
		{
			return FDB_ERROR_OP;
		}
		if(FDB_AddUser(&user)!=FDB_OK)
		{
			return FDB_ERROR_IO;
		}
	}
	else
		return FDB_ERROR_NODATA;
	if (gOptions.ZKFPVersion == ZKFPV10)
	{
		FDB_DelTmp(user.PIN,fingerid);
		TTemplate tmpv10;
		tmpv10.PIN = user.PIN; 
		tmpv10.FingerID = fingerid;
		tmpv10.Size = tmplen;
		tmpv10.Valid = 1;
		memcpy(tmpv10.Template,temp,tmplen);
		return FDB_AddTemplate_New(&tmpv10);
	}
	else
	{
		TTemplate tmp;
		if(!FDB_CreateTemplate(&tmp, user.PIN, fingerid, temp, tmplen))
		{
			return FDB_ERROR_IO;
		}
		return FDB_AddTemplate_New(&tmp);
	}
}

int FDB_AddUser_New(PUser user)
{
	int fd;
	int rc;
	if(user->PIN>MAX_PIN) return FDB_ERROR_DATA;
	fd = SelectFDFromConentType(FCT_USER);
	lseek(fd, 0, SEEK_END);		
	rc=((write(fd, (char*)user, sizeof(TUser))==sizeof(TUser))?FDB_OK:FDB_ERROR_IO);
	yaffs_flush(fd);
	yaffs_sync("/flash");
	if (rc == FDB_OK)
		SetBit((BYTE*)UserIDMap, user->PIN);
	return rc;
}

int AppendUser(int pin, char *name, char *password, int privillege)
{
	int ret=FDB_OK;
	TUser user;
	
	if(FDB_GetUser((U16)pin, &user))
	{
		memcpy((BYTE*)user.Name, name, MAXNAMELENGTH);
		memcpy((BYTE*)user.Password, password, 5);
		user.Privilege=privillege;
		ret=FDB_ChgUser(&user);
	}
	else if(FDB_CntUser()==gOptions.MaxUserCount*100)
		ret=FDB_OVER_ULIMIT;
	else if(FDB_CreateUser(&user, pin, name, password, privillege))
		ret=FDB_AddUser_New(&user);
	else
		ret=FDB_ERROR_OP;
	return ret;
}

int AppendFullUser(PUser user)
{
	int ret=FDB_OK;
	
	if(FDB_GetUser((U16)user->PIN, NULL))
		ret=FDB_ChgUser(user);
	else if(FDB_CntUser()==gOptions.MaxUserCount*100)
		ret=FDB_OVER_ULIMIT;
	else 
		ret=FDB_AddUser(user);
	return ret;	
}

PSms FDB_CreateSms(PSms sms, BYTE tag, U16 id, BYTE *content, U16 validminutes, time_t start)
{
	memset((void *)sms, 0, sizeof(TSms));
	sms->Tag=tag;
	sms->ID=id;
	sms->ValidMinutes=validminutes;
	sms->StartTime=start;
	nstrcpy(sms->Content, content, MAX_SMS_CONTENT_SIZE);
	return sms;
}

int FDB_AddSms(PSms sms)
{
	if (FDB_CntSms()>=MAX_SMS_COUNT)
		return FDB_ERROR_NOSPACE;
	return SearchAndSave(FCT_SMS, (char*)sms, sizeof(TSms));	
}

static TSms gSms;

PSms FDB_GetSms(U16 id, PSms sms)
{
	TSearchHandle sh;
	
	sh.ContentType=FCT_SMS; 
	sh.buffer=(unsigned char*)&gSms;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (((PSms)sh.buffer)->ID==id)
		{
			if (sms)
				memcpy(sms, sh.buffer, sizeof(TSms));
			return (PSms)sh.buffer;
		}
	}
	return NULL;	
}

int FDB_ChgSms(PSms sms)
{
	PSms s;
	
	if ((s=FDB_GetSms(sms->ID, NULL))==NULL) return FDB_ERROR_NODATA;
	if (0==memcmp((void*)sms, (void*)s, sizeof(TSms))) return FDB_OK;
	//overwrite
	lseek(fdSms, -1*sizeof(TSms), SEEK_CUR);
	if (write(fdSms, (void*)sms, sizeof(TSms))==sizeof(TSms))
		return FDB_OK;
	else
		return FDB_ERROR_IO;	
}

int FDB_DelSms(U16 id)
{
	PSms s;
	U16 smsid;
	
	if ((s=FDB_GetSms(id, NULL))==NULL) return FDB_ERROR_NODATA;
	//overwrite
	lseek(fdSms, -1*sizeof(TSms), SEEK_CUR);
	smsid=s->ID;
	s->ID=0;
	if (write(fdSms, (void*)s, sizeof(TSms))==sizeof(TSms))
		return FDB_DelUData(0, smsid);
	else
		return FDB_ERROR_IO;	
}

int FDB_ClrSms(void)
{
	return FDB_ClearData(FCT_SMS);
}

int FDB_CntSms(void)
{
	return GetDataInfo(FCT_SMS, STAT_COUNT, 0);
}

int FDB_AddUData(PUData udata)
{
	return SearchAndSave(FCT_UDATA, (char*)udata, sizeof(TUData));	
}

static TUData gUData;

PUData FDB_GetUData(U16 id, PUData udata)
{
	TSearchHandle sh;
	
	sh.ContentType=FCT_UDATA; 
	sh.buffer=(unsigned char*)&gUData;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (((PUData)sh.buffer)->PIN==id)
		{
			if (udata)
				memcpy(udata, sh.buffer, sizeof(TUData));
			return (PUData)sh.buffer;
		}
	}
	return NULL;	
}

PUData FDB_GetUDataByPINSMSID(U16 pin, U16 id, PUData udata)
{
	TSearchHandle sh;
	
	sh.ContentType=FCT_UDATA; 
	sh.buffer=(unsigned char*)&gUData;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if ((((PUData)sh.buffer)->PIN==pin)&&(((PUData)sh.buffer)->SmsID==id))
		{
			if (udata)
				memcpy(udata, sh.buffer, sizeof(TUData));
			return (PUData)sh.buffer;
		}
	}
	return NULL;	
}

//删除PIN OR SMSID指定的数据项
int FDB_DelUData(U16 PIN, U16 smsID)
{
	TUData u;
	
	lseek(fdUData, 0, SEEK_SET);	
	while(TRUE)
	{
		if (read(fdUData, (void *)&u, sizeof(TUData))==sizeof(TUData))
		{
			if (u.PIN&&((PIN&&smsID&&(u.SmsID==smsID)&&(u.PIN==PIN))||
				    (smsID&&!PIN&&(u.SmsID==smsID))||(!smsID&&PIN&&(u.PIN==PIN))))
			{
				lseek(fdUData, -1*sizeof(TUData), SEEK_CUR);
				u.PIN=0;
				u.SmsID=0;
				if (write(fdUData, (void*)&u, sizeof(TUData))!=sizeof(TUData))
				{
					yaffs_flush(fdUData);
					return FDB_ERROR_IO;
				}
			}
		}
		else break;
	}
	return FDB_OK;
}

int IsValidTimeDuration(U32 TestTime, PSms sms)
{
	U32 StartTime, EndTime;
	TTime st;	
	OldDecodeTime(sms->StartTime, &st);
	st.tm_isdst=-1; 
	//DBPRINTF("Year=%d Month=%d Day=%d Hour=%d Min=%d Sec=%d\n", 
	//	 st.tm_year, st.tm_mon, st.tm_mday, st.tm_hour, st.tm_min, st.tm_sec);    
	StartTime=mktime(&st);
	EndTime=StartTime+60*sms->ValidMinutes;	
	if(TestTime<StartTime)
		return -1;
	else if((TestTime>EndTime)&&(sms->ValidMinutes!=VALIDMINUTE_UNLIMITED))
		return 1;
	else 
		return 0;
}

void FDB_CheckSmsByStamp(U32 CurTime)
{
	TSearchHandle sh;
	U16 smsid;
	
	sh.ContentType=FCT_SMS; 
	sh.buffer=(unsigned char*)&gSms;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (gSms.ID&&(IsValidTimeDuration(CurTime, &gSms)==1))
		{
			//overwrite
			smsid=gSms.ID;
			gSms.ID=0;
			lseek(fdSms, -1*sizeof(TSms), SEEK_CUR);
			if (write(fdSms, (void*)sh.buffer, sizeof(TSms))==sizeof(TSms))
				FDB_DelUData(0, smsid);
		}
	}
}

int FDB_PackSms(void)
{
	char *buf;
	char tmp[80];
	int size;	
	if ((buf=FDB_ReadBlock(&size, FCT_SMS))==NULL) return FDB_ERROR_IO;
	fdSms=TruncFDAndSaveAs(fdSms, GetEnvFilePath("USERDATAPATH", "sms.dat", tmp), buf+4, size-4);
	free(buf);
	return FDB_OK;
}

BYTE *FDB_ReadUserSms(U16 *smsid, int smsnum, BYTE *content)
{
	PSms s;	
	int i=0,j,k;
	
	memset(content, 0, MAX_SMS_CONTENT_SIZE+1);
	k=smsnum-1;
	while(k>=0)
	{
		if((s=FDB_GetSms(smsid[k], NULL))!=NULL)
		{
			i=IsValidTimeDuration(EncodeTime(&gCurTime), s);
			//overdate so overwrite
			if (i==1)
			{
				lseek(fdSms, -1*sizeof(TSms), SEEK_CUR);
				s->ID=0;
				if (write(fdSms, (void*)s, sizeof(TSms))==sizeof(TSms))
					FDB_DelUData(0, smsid[k]);
			}
			//show information
			else if (i==0)
			{
				j=0;
				while((j<MAX_SMS_CONTENT_SIZE)&&s->Content[j])
				{
					*(content+j)=s->Content[j];
					j++;
				}
				*(content+j)='\0';
				return content;
			}
		}
		k--;
	}
	return NULL;
}

int FDB_ReadBoardSms(BYTE *content)
{
	TSearchHandle sh;
	int valid,len=0,i=1;
	int smsid;
	
	sh.ContentType=FCT_SMS; 
	sh.buffer=(unsigned char*)&gSms;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (gSms.ID==0) continue;
		//DBPRINTF("Year=%d Month=%d Day=%d Hour=%d Min=%d Sec=%d\n", 
		//	 gCurTime.tm_year, gCurTime.tm_mon, gCurTime.tm_mday, gCurTime.tm_hour, gCurTime.tm_min, gCurTime.tm_sec);
		valid=IsValidTimeDuration(EncodeTime(&gCurTime), &gSms);
		if (valid==1)
		{
			lseek(fdSms, -1*sizeof(TSms), SEEK_CUR);
			smsid=gSms.ID;
			gSms.ID=0;
			if (write(fdSms, sh.buffer, sizeof(TSms))==sizeof(TSms))
				FDB_DelUData(0, smsid);
		}
		else if ((gSms.Tag==UDATA_TAG_ALL)&&(valid==0))
		{
			if(i>1)
			{
				sprintf(content+len, "%d.%s", i, gSms.Content);
				len+=strlen(gSms.Content)+2;
			}
			else
			{
				sprintf(content+len, "%s", gSms.Content);
				len+=strlen(gSms.Content);
			}
			i++;
		}
		if (i>=10) break;
	}
	//DBPRINTF("%s\n", content);
	return strlen(content);
}

BOOL CheckBoardSMS(void)
{
	memset(gBoardSMS, 0, 2048);
	gBoardSMSPos=0;	
	return (FDB_ReadBoardSms(gBoardSMS)>0);
}

extern int gLCDCharWidth;

void DisplayBoardSMS(void)
{
	unsigned char s[20]; //16bytes
	int i=0;
	
	if (!gBoardSMS[0]) return;
	if (gBoardSMSPos>=strlen(gBoardSMS)) gBoardSMSPos=0;    
	while(i<gLCDCharWidth)
	{
		if (gBoardSMS[gBoardSMSPos])
		{
			if (gBoardSMS[gBoardSMSPos]>128) //Chinese font
			{
				if ((i+1)>=gLCDCharWidth)
				{
					s[i++]=' ';
					break;
				}
				s[i]=gBoardSMS[gBoardSMSPos++];
				s[i+1]=gBoardSMS[gBoardSMSPos++];
				i++;
			}
			else
				s[i]=gBoardSMS[gBoardSMSPos++];
		}
		else
			s[i]=' ';
		i++;
	}
	s[i]='\0';
	LCDWriteStr(0,0,s,0);    
}

BOOL CheckUserSMS(U16 pin, BYTE *smsContent)
{
	U16 sms[10];
	TUData u;
	int i=0;
	
	memset(sms, 0, 20);
	lseek(fdUData, 0, SEEK_SET);	
	while(TRUE)
	{
		if (read(fdUData, (void *)&u, sizeof(TUData))==sizeof(TUData))
		{
			if (u.PIN&&(u.PIN==pin))
			{
				sms[i++]=u.SmsID;
				if (i>=10) break;
			}
		}
		else break;
	}
	return ((i>0)&&FDB_ReadUserSms(sms, i, smsContent));
}

enum Record_mode {
	OP_DEL = 0,
	OP_ADD_ONLY = 1,
	OP_ADD_OVERWRITE = 2,
	OP_UPDATE = 3,
	OP_IGNORE = 4		   
};

//buffer format description
//USER    FINGER   TEMPLATE   length(bytes)
// 4        4         4 
//TUSERS
//TFINGERS  offset is based on TEMPLATE 
//TEMPLATE(LEN 2B + template)

int ReadTemplateInfo(int fdCacheData, int tmpPos, char *pTemplate)
{
	unsigned short tmplen;

	lseek(fdCacheData, tmpPos, SEEK_SET);
	read(fdCacheData, &tmplen, 2);
	read(fdCacheData, pTemplate, tmplen);

	return tmplen;
}

void SwitchUDCAndDelay(void)
{
	ENABLE_UDC_IRQ(0);
	DelayMS(2*1000);
	ENABLE_UDC_IRQ(1);
}

void BatchOPUserData(char *dataBuffer, int fdCacheData)
{
	char buf[80];
	PUserS UserS;
	PFingerS FingerS;
	BYTE *pTemplate;
	U32 len;
	U32 records;
	TSearchHandle sh;
	int i, j, k, l;
	U16 tmplen;
	U16 UserCount=0, FingerCount=0;
	BOOL AddSign;
	char *FingerCacheBuf;
        int MaxFingerCacheCnt=50;
	int CurFingerCacheCnt;
	char *UserCacheBuf=NULL;
	char *tmpBuf=NULL;
	int CurUserCacheSize;
	PUser user;
	int usernumber;
        int MaxGroupFingerCnt=50;
	int FingerNumber;
	int CurGroupFingerCnt;
	BOOL ChangeSign;
	int templen = sizeof(TTemplate);

	if (gOptions.ZKFPVersion != ZKFPV10)
		templen -= ZKFP_OFF_LEN;
	char *buffer;
	char readBuf[2048];
	int UserAndFingerLen = 0;
	int TemplatePos = 0;

	if (fdCacheData)
	{
		lseek(fdCacheData, 0, SEEK_SET);
		read(fdCacheData, &len, 4); //user
		UserAndFingerLen += len;
		read(fdCacheData, &len, 4); //Template
		UserAndFingerLen += len;
		TemplatePos = UserAndFingerLen + 12;   
		lseek(fdCacheData, 0, SEEK_SET);
		buffer = malloc(TemplatePos);
		read(fdCacheData, buffer, TemplatePos);
	}
	else
		buffer = dataBuffer;
	memcpy(&len, buffer, 4);
	if(len)
	{
		UserS=(PUserS)(buffer+12);
		records=(U32)(len/sizeof(TUserS));		
		//read data to memory
		CurUserCacheSize=lseek(fdUser, 0, SEEK_END);
		if(CurUserCacheSize)
		{
			UserCacheBuf=malloc(CurUserCacheSize);
			lseek(fdUser, 0, SEEK_SET);
			read(fdUser, UserCacheBuf, CurUserCacheSize);
		}
		usernumber=CurUserCacheSize/sizeof(TUser);
		user=(PUser)UserCacheBuf;				
		//acquired the status for operation
		for(j=0;j<usernumber;j++)
		{
			wdt_set_count(0);
			if(user[j].PIN)
			{
				UserCount++;
				for(i=0;i<records;i++)
				{
					if(user[j].PIN==UserS[i].User.PIN)
					{
						if(UserS[i].OpSign==OP_ADD_ONLY)
							UserS[i].OpSign=OP_IGNORE;
						else if(UserS[i].OpSign==OP_ADD_OVERWRITE)
							UserS[i].OpSign=OP_UPDATE;						
					}
				}
			}
		}
		//execute 
		AddSign=TRUE;
		for(j=0;j<usernumber;j++)
		{
			wdt_set_count(0);
                        for(i=0;i<records;i++)
                        {
				if(user[j].PIN==UserS[i].User.PIN)
                                {
                                        if(UserS[i].OpSign==OP_DEL)
                                        {
                                                //delete user
                                                user[j].PIN=0;
						UserCount--;
					}
					else if(UserS[i].OpSign==OP_UPDATE)
					{
						//update
						memcpy((void*)&(user[j]), (void*)&(UserS[i].User), sizeof(TUser));
					}
					//there maybe have two records for one ID.  
					break;
				} 
			}
			//append record
			if(AddSign&&(user[j].PIN==0)&&(UserCount<gOptions.MaxUserCount*100))
                        {
				wdt_set_count(0);
                                AddSign=FALSE;
                                for(i=0;i<records;i++)
                                {
                                        //append record
					if(UserS[i].User.PIN&&((UserS[i].OpSign==OP_ADD_ONLY)||(UserS[i].OpSign==OP_ADD_OVERWRITE)))
					{
						memcpy((void*)&(user[j]), (void*)&(UserS[i].User), sizeof(TUser));
						UserS[i].User.PIN=0;
						UserCount++;
						AddSign=TRUE;
						break;
					}						
				}
			}	
		}
		//calculate all record that need append
		if(AddSign)
		{
			wdt_set_count(0);
                        for(i=0;i<records;i++)
                        {
                                if(UserS[i].User.PIN &&((UserS[i].OpSign==OP_ADD_ONLY)||(UserS[i].OpSign==OP_ADD_OVERWRITE)))
                                        usernumber++;
                        }
                }
                //append records
                if(j<usernumber)
		{
			tmpBuf = malloc(usernumber*sizeof(TUser));
			if (UserCacheBuf)
			{
				memcpy(tmpBuf, UserCacheBuf, j*sizeof(TUser));
				free(UserCacheBuf);
			}
			UserCacheBuf = tmpBuf;
                        wdt_set_count(0);
			user=(PUser)UserCacheBuf;
			for(i=0;i<records;i++)
			{
				//append record
				if(UserS[i].User.PIN&&((UserS[i].OpSign==OP_ADD_ONLY)||(UserS[i].OpSign==OP_ADD_OVERWRITE)))
				{
					if(UserCount>=gOptions.MaxUserCount*100) break;
					memcpy((void*)&(user[j]), (void*)&(UserS[i].User), sizeof(TUser));
					UserS[i].User.PIN=0;
					UserCount++;
					j++;
					if(j>=usernumber) break;
				}						
			}			
		}
		//write data with O_TRUNC AND NO O_SYNC
		if(j)
		{
			close(fdUser);
			fdUser=open(GetEnvFilePath("USERDATAPATH", "user.dat", buf), O_RDWR|O_TRUNC);
			write(fdUser, UserCacheBuf, j*sizeof(TUser));
			yaffs_flush(fdUser);
			close(fdUser);		
			fdUser=open(GetEnvFilePath("USERDATAPATH", "user.dat", buf), O_RDWR);
		}
		if(UserCacheBuf) free(UserCacheBuf);		
		DBPRINTF("USER DATA WTITE FINISHED!\n");
	}
	SwitchUDCAndDelay();
	//template table
        wdt_set_count(0);
	FingerS=(PFingerS)(buffer+12+len);
	memcpy(&len, buffer+4, 4);
	if(len)
	{
		records=(U32)(len/sizeof(TFingerS));
		pTemplate=(BYTE *)FingerS+len;
		
		close(fdFingerTmp); 
		if (gOptions.ZKFPVersion == ZKFPV10)
		{
			if (IsDB8M)
				fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "data/tempv10.dat", buf), O_RDWR);
			else
				fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "tempv10.dat", buf), O_RDWR);
		}
		else
		{
			if (IsDB8M)
				fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "data/template.dat", buf), O_RDWR);
			else
				fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "template.dat", buf), O_RDWR);
		}

		sh.ContentType=FCT_FINGERTMP; 
		sh.buffer=(char *)gTemplate;
		wdt_set_count(0);
		//acquired the status for operation
		FingerNumber=0;
		SearchFirst(&sh);
		while(!SearchNext(&sh))
		{
			if(sh.datalen)
			{
				FingerCount++;
				for(i=0;i<records;i++)
				{
					if((((PTemplate)sh.buffer)->Valid)&&
					   (((PTemplate)sh.buffer)->PIN==FingerS[i].PIN)&&
                                           (((PTemplate)sh.buffer)->FingerID==(FingerS[i].FingerID&0x0f)))
					{
						
						if(FingerS[i].OpSign==OP_ADD_ONLY)
							FingerS[i].OpSign=OP_IGNORE;
						else if(FingerS[i].OpSign==OP_ADD_OVERWRITE)
							FingerS[i].OpSign=OP_UPDATE;
					}
				}
			}
			FingerNumber++;
		}		
		SwitchUDCAndDelay();
		//execute 
		SearchFirst(&sh);
		AddSign=TRUE;
		j=1;
		k=0;
		wdt_set_count(0);
		FingerCacheBuf=malloc(MaxGroupFingerCnt*templen);			     memset(FingerCacheBuf, 0, MaxGroupFingerCnt*templen);
		while(k<FingerNumber)
		{
			CurGroupFingerCnt=MaxGroupFingerCnt;
			if ((k+MaxGroupFingerCnt)>FingerNumber)
				CurGroupFingerCnt=FingerNumber-k;			
			read(fdFingerTmp, FingerCacheBuf, CurGroupFingerCnt*templen);
			k+=CurGroupFingerCnt;			
			DBPRINTF("GROUP %d Starting......\n", j);
			ChangeSign=FALSE;
			for(l=0;l<CurGroupFingerCnt;l++)
			{
				sh.buffer=(char*)(FingerCacheBuf+l*templen);
				for(i=0;i<records;i++)
				{
					if((((PTemplate)sh.buffer)->Valid)&&
					   (((PTemplate)sh.buffer)->PIN==FingerS[i].PIN))
					{
						if((FingerS[i].OpSign==OP_DEL)&&
						   ((FingerS[i].FingerID==0xFF)||(((PTemplate)sh.buffer)->FingerID==FingerS[i].FingerID)))
						{
							//delete template
							((PTemplate)sh.buffer)->Valid=0;
							FingerCount--;
							ChangeSign=TRUE;
						}
						else if((FingerS[i].OpSign==OP_UPDATE)&&
                                                        (((PTemplate)sh.buffer)->FingerID==(FingerS[i].FingerID&0x0f)))
						{
							if (fdCacheData)
							{
								tmplen = ReadTemplateInfo(fdCacheData, TemplatePos + FingerS[i].OffSet, readBuf);
								FDB_CreateTemplate(sh.buffer, FingerS[i].PIN, FingerS[i].FingerID&0x0f, readBuf, tmplen);
							}
							else
							{
								memcpy(&tmplen, pTemplate+FingerS[i].OffSet, 2);
								FDB_CreateTemplate((PTemplate)sh.buffer,FingerS[i].PIN, FingerS[i].FingerID&0x0f, \
			(char *)(pTemplate+FingerS[i].OffSet+2),tmplen);
							}
//2007.03.02 fix template 					
							((PTemplate)sh.buffer)->Size+=6;
							ChangeSign=TRUE;
						}
					}
				}
                                //append record
                                wdt_set_count(0);
				if(AddSign&&(((PTemplate)sh.buffer)->Valid==0)&&(FingerCount<gOptions.MaxFingerCount*100))
				{
					AddSign=FALSE;
					for(i=0;i<records;i++)
					{
						//append record
						if(FingerS[i].PIN&&((FingerS[i].OpSign==OP_ADD_ONLY)||(FingerS[i].OpSign==OP_ADD_OVERWRITE)))
						{
							if (fdCacheData)
							{
								tmplen = ReadTemplateInfo(fdCacheData, TemplatePos + FingerS[i].OffSet, readBuf);
								FDB_CreateTemplate(sh.buffer, FingerS[i].PIN, FingerS[i].FingerID&0x0f, readBuf, tmplen);
							}
							else
							{
								memcpy(&tmplen, pTemplate+FingerS[i].OffSet, 2);
								FDB_CreateTemplate((PTemplate)sh.buffer,FingerS[i].PIN, FingerS[i].FingerID&0x0f, \
			(char *)(pTemplate+FingerS[i].OffSet+2),tmplen);
							}
							((PTemplate)sh.buffer)->Size+=6;
//2007.03.02 fix template 					
							FingerS[i].PIN=0;
							FingerCount++;
							AddSign=TRUE;
							ChangeSign=TRUE;
							break;
						}						
					}
				}
			}
			wdt_set_count(0);
			if(ChangeSign)
			{
				DBPRINTF("GROUP %d Writting......\n", j);
				lseek(fdFingerTmp, -1*CurGroupFingerCnt*templen, SEEK_CUR);
				write(fdFingerTmp, FingerCacheBuf, CurGroupFingerCnt*templen);
			}
			DBPRINTF("GROUP %d Endded\n", j);
			j++;
		}
		SwitchUDCAndDelay();
		wdt_set_count(0);
                free(FingerCacheBuf);
                sh.buffer=(char *)gTemplate;//2009.12.11 fix upload finger count error
		if(AddSign)
		{		
			//append all record that left
			lseek(fdFingerTmp, 0, SEEK_END);
			FingerCacheBuf=malloc(MaxFingerCacheCnt*templen);
			CurFingerCacheCnt=0;
			for(i=0;i<records;i++)
			{
				//append record
				if(FingerS[i].PIN&&((FingerS[i].OpSign==OP_ADD_ONLY)||(FingerS[i].OpSign==OP_ADD_OVERWRITE)))
				{
					if(FingerCount>=gOptions.MaxFingerCount*100) break;
					if (fdCacheData)
					{
						tmplen = ReadTemplateInfo(fdCacheData, TemplatePos + FingerS[i].OffSet, readBuf);
						FDB_CreateTemplate(sh.buffer, FingerS[i].PIN, FingerS[i].FingerID&0x0f, readBuf, tmplen);
					}
					else
					{
						memcpy(&tmplen, pTemplate+FingerS[i].OffSet, 2);
						FDB_CreateTemplate((PTemplate)sh.buffer,FingerS[i].PIN, FingerS[i].FingerID&0x0f, \
								(char *)(pTemplate+FingerS[i].OffSet+2),tmplen);
					}
					((PTemplate)sh.buffer)->Size+=6;
					memcpy(FingerCacheBuf+CurFingerCacheCnt*templen, (void*)sh.buffer, templen);
					CurFingerCacheCnt++;
					if(CurFingerCacheCnt==MaxFingerCacheCnt)
					{
						write(fdFingerTmp, FingerCacheBuf, CurFingerCacheCnt*templen);
						CurFingerCacheCnt=0;
						DBPRINTF("Write cache data to disk!\n");
					}
					//write(fdFingerTmp, (void*)sh.buffer, templen);
					FingerS[i].PIN=0;
					FingerCount++;
				}						
			}
			if(CurFingerCacheCnt)
			{
				write(fdFingerTmp, FingerCacheBuf, CurFingerCacheCnt*templen);
				DBPRINTF("Write cache data to disk finished!\n");
			}
			free(FingerCacheBuf);
		}
		wdt_set_count(0);
                yaffs_flush(fdFingerTmp);
		close(fdFingerTmp); 
		if (gOptions.ZKFPVersion == ZKFPV10)
		{
			if (IsDB8M)
				fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "data/tempv10.dat", buf), O_RDWR);
			else
				fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "tempv10.dat", buf), O_RDWR);
		}
		else
		{
			if (IsDB8M)
				fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "data/template.dat", buf), O_RDWR);
			else
				fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "template.dat", buf), O_RDWR);
		}
	if (fdCacheData)
		free(buffer);

		yaffs_sync("/flash");
	}	
}
/*
#define upuserbuf 200*2048
#define uptmpopbuf 100*2048
#define uptmpsbuf 400*2048
#define Maxupsize 600*2048
#define MaxUpusersize 300*2048
*/
int UploadDataFromDisk(int ContentType)
{
#if 0
	char buffer[80];
	int sign=FALSE;
	int mount,ret;
	int fd;
	TUser tmp;
	TTemplate curtmp;
	TSms sms;
	TUData udata;

	char *TmpOpBuf, *UserInfoOpBuf, *TmpsBuf;
	int  TmpOpBufLen,UserInfoOpBufLen,TmpsBufLen;
	char *Updata=NULL;
	int len = 0;

	
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

	//ini batchbuf;
	UserInfoOpBufLen = 0;
	UserInfoOpBuf = NULL;
	TmpOpBufLen = 0;
	TmpOpBuf = NULL;
	TmpsBufLen = 0;
	TmpsBuf = NULL;
	
	//start upload data
	if (ContentType==FCT_USER)
	{
		sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "user.dat");
		if ((fd=open(buffer, O_RDONLY))!=-1)
		{
			lseek(fd, 0, SEEK_SET); 
			UserInfoOpBuf = malloc(upuserbuf);
			memset(UserInfoOpBuf,0,upuserbuf);
			while(TRUE)
			{
				if ((read(fd, &tmp, sizeof(TUser))==sizeof(TUser))&&(tmp.PIN>0))
				{

					*(U8*)(UserInfoOpBuf+UserInfoOpBufLen)=2;	//操作标志
					memcpy(UserInfoOpBuf+UserInfoOpBufLen+1,&tmp,sizeof(TUser));//用户信息
					UserInfoOpBufLen+=sizeof(TUser)+1;
					len = 12+UserInfoOpBufLen;
					if (len >=(MaxUpusersize-512))
					{
						Updata = malloc(MaxUpusersize);
						//*(U32*)Updata=UserInfoOpBufLen;
						memcpy(Updata,&UserInfoOpBufLen,4);
						//((U32*)Updata)[1]=TmpOpBufLen;
						memcpy(Updata+4,&TmpOpBufLen,4);
						//((U32*)Updata)[2]=TmpsBufLen;
						memcpy(Updata+4+4,&TmpsBufLen,4);
						memcpy(Updata+12,UserInfoOpBuf,UserInfoOpBufLen);
						BatchOPUserData(Updata);
						free(UserInfoOpBuf);
						UserInfoOpBuf = NULL;
						UserInfoOpBufLen = 0;
						free(Updata);
						UserInfoOpBuf = malloc(upuserbuf);		
						len = 0;
					}
					
				}
				else
				{
					if (len > 0)
					{
						Updata = malloc(600*2048);
						//*(U32*)Updata=UserInfoOpBufLen;
						memcpy(Updata,&UserInfoOpBufLen,4);
						memcpy(Updata+4,&TmpOpBufLen,4);
						//((U32*)Updata)[2]=TmpsBufLen;
						memcpy(Updata+4+4,&TmpsBufLen,4);
						memcpy(Updata+12,UserInfoOpBuf,UserInfoOpBufLen);
						BatchOPUserData(Updata);
						free(UserInfoOpBuf);
						UserInfoOpBuf = NULL;
						UserInfoOpBufLen = 0;
						free(Updata);
						len = 0;					
					}
					else
						break;
				}
			}
			close(fd);
//			DBPRINTF("finish upuser\n");
			if (!gOptions.IsOnlyRFMachine)
			{
				if (gOptions.ZKFPVersion == ZKFPV10)
					sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "tempv10.dat");
				else
					sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "template.dat");
				if ((fd=open(buffer, O_RDONLY))!=-1)
				{
					lseek(fd, 0, SEEK_SET); 
					TmpOpBuf = malloc(uptmpopbuf);
					TmpsBuf = malloc(uptmpsbuf);
					len = 0;
					while(TRUE)
					{
						if ((read(fd, &curtmp, sizeof(TTemplate))==sizeof(TTemplate))&&curtmp.Valid)
						{

							(TmpOpBuf+TmpOpBufLen)[0]=2;//操作标志
							//*(U16*)(TmpOpBuf+TmpOpBufLen+1)=curtmp.PIN; //UserID
							memcpy(TmpOpBuf+TmpOpBufLen+1,&(curtmp.PIN),2); //UserID
							(TmpOpBuf+TmpOpBufLen)[3]=(char)curtmp.FingerID;	//FingerID
							//*(U32*)(TmpOpBuf+TmpOpBufLen+4)=TmpsBufLen;//Offset
							memcpy(TmpOpBuf+TmpOpBufLen+4,&TmpsBufLen,4);//Offset
							TmpOpBufLen+=8;		
							//*(U16*)(TmpsBuf+TmpsBufLen)=(curtmp.Size-6);	//模板长度
							memcpy(TmpsBuf+TmpsBufLen,&curtmp.Size-6,2);	//模板长度

							memcpy(TmpsBuf+TmpsBufLen+2,curtmp.Template,(curtmp.Size-6));//模板
							TmpsBufLen+=(curtmp.Size-6)+2;								//模板区总长度

							len=12+UserInfoOpBufLen+TmpOpBufLen+TmpsBufLen;
				//			DBPRINTF("tmp len: %d\n",len);
							if (len>= (uptmpsbuf-2048))
							{
				//				DBPRINTF("ready malloc Updata\n");
								Updata = malloc(Maxupsize);
memset(Updata,0,Maxupsize);
				//				DBPRINTF("after malloc Updata\n");
								//*(U32*)Updata=UserInfoOpBufLen;
								memcpy(Updata,&UserInfoOpBufLen,4);
								//((U32*)Updata)[1]=TmpOpBufLen;
								memcpy(Updata+4,&TmpOpBufLen,4);
								//((U32*)Updata)[2]=TmpsBufLen;
								memcpy((Updata+4+4),&TmpsBufLen,4);
								//memcpy(Updata+12,UserInfoOpBuf,UserInfoOpBufLen);
								memcpy(Updata+12+UserInfoOpBufLen,TmpOpBuf,TmpOpBufLen);
								memcpy(Updata+12+UserInfoOpBufLen+TmpOpBufLen,TmpsBuf,TmpsBufLen);
							
								BatchOPUserData(Updata);
								//DBPRINTF("after batchopuserdata\n");
								free(TmpOpBuf);
								free(TmpsBuf);
								free(Updata);
								TmpOpBufLen = 0;
								TmpsBufLen = 0;
								TmpOpBuf = malloc(uptmpopbuf);
								TmpsBuf = malloc(uptmpsbuf);
								len = 0;
							}

							
						}
						else
						{
							if (len>0)
							{
								Updata = malloc(Maxupsize);
memset(Updata,0,Maxupsize);
								//*(U32*)Updata=UserInfoOpBufLen;
								memcpy(Updata,&UserInfoOpBufLen,4);
								//((U32*)Updata)[1]=TmpOpBufLen;
								memcpy((Updata+4),&TmpOpBufLen,4);
								//((U32*)Updata)[2]=TmpsBufLen;
								memcpy((Updata+4+4),&TmpsBufLen,4);
								//memcpy(Updata+12,UserInfoOpBuf,UserInfoOpBufLen);
								memcpy(Updata+12+UserInfoOpBufLen,TmpOpBuf,TmpOpBufLen);
								memcpy(Updata+12+UserInfoOpBufLen+TmpOpBufLen,TmpsBuf,TmpsBufLen);
							
								BatchOPUserData(Updata);
								free(TmpOpBuf);
								free(TmpsBuf);
								free(Updata);
								TmpOpBufLen = 0;
								TmpsBufLen = 0;
								TmpOpBuf = malloc(uptmpopbuf);
								TmpsBuf = malloc(uptmpsbuf);
								len = 0;
								

							}
							else
								break;
						}
					}
					sign=TRUE;
					close(fd);
				}
			}
			else
				sign=TRUE;
		}
	}
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
	if (mount==0) DoUmountUdisk();
	
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
#endif
	return 0;
}


typedef struct _PIN2Rec_{
	U16 PIN;
	U32 PIN2;
}TPIN2Rec, *PPIN2Rec;

void UpdateAttLog(void)
{
#if 0
	char buf[16];
	TAttLog log;
	time_t t=0;
	int s;
	PPIN2Rec pin2rec=NULL;
	TSearchHandle sh;
	U8 databuf[64];
	int I=0;
	int UserCount=0;
	U32 pin2;

//	printf("fstat transaction ... \n");
	struct yaffs_stat attlog; 
	fstat(fdTransaction, &attlog);	//fstat is a linux system function
	if(gOptions.AttLogExtendFormat&&(attlog.yst_size>0))
	{
		LCDWriteCenterStr(1, "Updating");
		LCDWriteCenterStr(2, "Attlog data");
		if(gOptions.PIN2Width!=PIN_WIDTH)
		{
			UserCount=GetDataInfo(FCT_USER, STAT_COUNT, 0);
			pin2rec=(PPIN2Rec)malloc(UserCount*sizeof(TPIN2Rec));
			memset(pin2rec, 0, UserCount*sizeof(TPIN2Rec));
			sh.ContentType=FCT_USER; 
			sh.buffer=databuf;
			SearchFirst(&sh);
			while(!SearchNext(&sh))
			{
				if(sh.datalen>0)
				{
					pin2rec[I].PIN=((PUser)(sh.buffer))->PIN;
					pin2rec[I].PIN2=((PUser)(sh.buffer))->PIN2;
					I++;
				}
			}		
		}
		lseek(fdTransaction, 0, SEEK_SET);
		while(TRUE)
		{
			if (read(fdTransaction, buf, 4)==4)
			{
				if (IsAttLogLongPack(buf[2]))
					read(fdTransaction, buf+4, 4);
				s=UnpackAttLog(buf, &log);
				if(s==AttLogSize2)
					t=log.time_second;
				else
					log.time_second+=t;
				pin2=0;
				//search pin2
				if(gOptions.PIN2Width!=PIN_WIDTH)
				{
					for(I=0;I<UserCount;I++)
					{
						if(pin2rec[I].PIN==log.PIN)
						{
							pin2=pin2rec[I].PIN2;
							break;
						}
					}
				}
				FDB_AddAttLog(log.PIN, log.time_second, log.verified, log.status, pin2, 0, 0);
			}
			else
				break;
		}
		fdTransaction=TruncFDAndSaveAs(fdTransaction, GetEnvFilePath("USERDATAPATH", "transaction.dat", buf), NULL, 0);
	//	printf("fdTransaction path=%s\n",buf);
		if(pin2rec) free(pin2rec);
		//sync();
	}
#endif 
}

void GetFilterGroupInfo(int inputpin, PFilterRec filterbuf)
{
	TSearchHandle sh;
	TUser User;
	int i=0;
	memset(filterbuf, 0, gOptions.MaxUserCount*100*sizeof(TFilterRec));
	sh.ContentType=FCT_USER; 	
	sh.buffer=(unsigned char*)&User;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if(sh.datalen>0)
		{
			if((((PUser)sh.buffer)->Group&0x0F)==inputpin)
				filterbuf[i++].PIN=((PUser)sh.buffer)->PIN;
		}
	}
}

void GetFilterHeadInfo(int inputpin, PFilterRec filterbuf)
{
	TSearchHandle sh;
	TUser User;
	int i=0;
	memset(filterbuf, 0, gOptions.MaxUserCount*100*sizeof(TFilterRec));
	sh.ContentType=FCT_USER; 	
	sh.buffer=(unsigned char*)&User;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if(sh.datalen>0)
		{
			filterbuf[i].PIN=((PUser)sh.buffer)->PIN;
			filterbuf[i].PIN2=((PUser)sh.buffer)->PIN2;
			i++;
		}
	}	
}

PExtUser FDB_CreateExtUser(PExtUser extuser, U16 pin, U8 verifystyle)
{
	memset((void*)extuser, 0, sizeof(TExtUser));
	extuser->PIN=pin;
	extuser->VerifyStyle=verifystyle;
	return extuser;
}

int FDB_AddExtUser(PExtUser extuser)
{
	return SearchAndSave(FCT_EXTUSER, (char*)extuser, sizeof(TExtUser));
}

int FDB_ChgExtUser(PExtUser extuser)
{
	PExtUser u;
	
	if((u=FDB_GetExtUser(extuser->PIN, NULL))==NULL) return FDB_ERROR_NODATA;
	if(0==memcmp((void*)extuser, (void*)u, sizeof(TExtUser))) return FDB_OK;
	//overwrite user 
	lseek(fdExtUser, -1*sizeof(TExtUser), SEEK_CUR);
	if(write(fdExtUser, (void*)extuser, sizeof(TExtUser))==sizeof(TExtUser))
	{
		yaffs_flush(fdExtUser);
		return FDB_OK;
	}
	else
		return FDB_ERROR_IO;
}

int FDB_DelExtUser(U16 pin)
{
	PExtUser u;
	int uid;
	
	if((u=FDB_GetExtUser(pin, NULL))==NULL) return FDB_ERROR_NODATA;
	uid=u->PIN;
	//overwrite user 
	lseek(fdExtUser, -1*sizeof(TExtUser), SEEK_CUR);
	u->PIN=0;
	if(write(fdExtUser, (void*)u, sizeof(TExtUser))==sizeof(TExtUser))
	{
		yaffs_flush(fdExtUser);
		return FDB_OK;
	}
	else
		return FDB_ERROR_IO;
}

static TExtUser gExtUser;

PExtUser FDB_GetExtUser(U16 pin, PExtUser extuser)
{
	TSearchHandle sh;
	
	sh.ContentType=FCT_EXTUSER; 
	sh.buffer=(unsigned char*)&gExtUser;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if(((PExtUser)sh.buffer)->PIN==pin)
		{
			if (extuser)
			{
				memcpy(extuser, sh.buffer, sizeof(TExtUser));
				return extuser;
			}
			else
				return (PExtUser)sh.buffer;
		}
	}
	return NULL;
}

static TWorkCode gWorkCode;

PWorkCode FDB_GetWorkCode(U16 id, PWorkCode workcode)
{
        TSearchHandle sh;

        sh.ContentType=FCT_WorkCode;
        sh.buffer=(unsigned char*)&gWorkCode;
        SearchFirst(&sh);
        while(!SearchNext(&sh))
        {
		//DBPRINTF("read wid: %d\n",((PWorkCode)sh.buffer)->WORKID);
                if (((PWorkCode)sh.buffer)->WORKID==id)
                {
                        if (workcode)
                                memcpy(workcode, sh.buffer, sizeof(TWorkCode));
                        return (PWorkCode)sh.buffer;
                }
        }
        return NULL;
}
int IsValidWorkCode(char *value)
{
	U32 workcode;
	int ret=0;
	if(0!=strtou32(value, &workcode))
		ret=FALSE;
	else
	{
	        TSearchHandle sh;
	        sh.ContentType=FCT_WorkCode;
	        sh.buffer=(unsigned char*)&gWorkCode;
        	SearchFirst(&sh);
	        while(!SearchNext(&sh))
        	{
                	if (((PWorkCode)sh.buffer)->WORKCODE==workcode && workcode > 0)
			{
                		ret = TRUE;
				break;
			}
        	}
	}
	return ret;
}

PWorkCode FDB_CreateWorkCode(PWorkCode workcode,U16 id, U32 jobcode)
{

	memset((void *)workcode, 0, sizeof(TWorkCode));
	workcode->WORKID=id;
	workcode->WORKCODE=jobcode;
	return workcode;
}

int FDB_AddWorkCode(PWorkCode workcode)
{
	if (FDB_CntWorkCode()>=MAX_WorkCode_COUNT)
		return FDB_ERROR_NOSPACE;
	return SearchAndSave(FCT_WorkCode, (char*)workcode, sizeof(TWorkCode));	
}


int FDB_ChgWorkCode(PWorkCode workcode)
{
	PWorkCode workcode1;
	
	if ((workcode1=FDB_GetWorkCode(workcode->WORKID, NULL))==NULL) return FDB_ERROR_NODATA;
	if (0==memcmp((void*)workcode, (void*)workcode1, sizeof(TWorkCode))) return FDB_OK;
	//overwrite
	lseek(fdWorkCode, -1*sizeof(TWorkCode), SEEK_CUR);
	if (write(fdWorkCode, (void*)workcode, sizeof(TWorkCode))==sizeof(TWorkCode))
		return FDB_OK;
	else
		return FDB_ERROR_IO;	
}

int FDB_CntWorkCode(void)
{
	return GetDataInfo(FCT_WorkCode, STAT_COUNT, 0);
}

int FDB_DelWorkCode(U16 id)
{
        PWorkCode s;
        U16 workid;

        if ((s=FDB_GetWorkCode(id, NULL))==NULL) return FDB_ERROR_NODATA;
        //overwrite
        lseek(fdWorkCode, -1*sizeof(TWorkCode), SEEK_CUR);
        workid=s->WORKID;
        s->WORKID=0;
        if (write(fdWorkCode, (void*)s, sizeof(TWorkCode))==sizeof(TWorkCode))
                return FDB_OK;
        else
                return FDB_ERROR_IO;
}

int FDB_ClrWorkCode(void)
{
	return FDB_ClearData(FCT_WorkCode);
}

//dsl
void SearchNewFirstEx(PSearchHandle sh, int start)
{
	int pos = 0;
	sh->fd=SelectFDFromConentType(sh->ContentType);
	if(sh->ContentType==FCT_ATTLOG)
	{
		lseek(sh->fd, 0,SEEK_SET );
		if(gOptions.AttLogExtendFormat)
		{
			pos = start * sizeof(TExtendAttLog);
		}
		else
		{
			pos = start * 8; //dsl.the size is 8 in saveattlog
		}
		lseek(sh->fd, pos, SEEK_CUR);
	}
	sh->bufferlen=0; 
	sh->datalen=0;  //valid data length
}



//dsl
int GetDataInfoEx(int ContentType, int StatType, int Value)
{
	int tmp;	
	unsigned char buf[2048];
	TSearchHandle sh;
	sh.ContentType=ContentType; 
	sh.buffer=buf;

	tmp = 0;
	SearchNewFirstEx(&sh, Value);
	while(!SearchNext(&sh)){
		switch(StatType)
		{
		case STAT_VALIDLEN:
			tmp+=sh.datalen;
			break;
		}
	}
	return tmp;
}



void FDB_SetAttLogReadAddr(U32 addr)
{
	if(attLogReadPos!=addr)
	{
		attLogReadPos=addr;
	}
}

void FDB_SetOpLogReadAddr(U32 addr)
{
	if(opLogReadPos!=addr)
	{
		opLogReadPos=addr;
	}
}

BYTE *ReadNewDataBlock(BYTE ContentType, int *size, int LogReadPos) 
{
	TSearchHandle sh;
	U8 buf[2048];
	int validLen, lastcount;
	U8 *validBuf;
	U8 *p;
	//validLen=GetDataInfo(ContentType, STAT_VALIDLEN, 0);
	lastcount = *size;
	validLen=GetDataInfoEx(ContentType, STAT_VALIDLEN, *size);
	//dsl
	if (!validLen)
	{
		return NULL;
	}
	*size=validLen+4;
	//DBPRINTF("validlen=%d, size=%d\n", validLen, *size);
	if (validLen>0)
	{
		validBuf=malloc(validLen+4);
		memcpy(validBuf, &validLen, 4);
		p=validBuf+4;
		
		sh.ContentType=ContentType; 
		sh.buffer=buf;
		//SearchNewFirst(&sh, *size);
		SearchNewFirstEx(&sh, lastcount);
		while(!SearchNext(&sh))
		{
			if(sh.datalen>0)
			{
				memcpy(p, sh.buffer, sh.datalen);
				p+=sh.datalen;
			}
		}
		return validBuf; 
	} 
	else
		return NULL;	
}

char* FDB_ReadNewAttLogBlock(int *size)
{
	return (char*)ReadNewDataBlock(FCT_ATTLOG, size, attLogReadPos);
}

char* FDB_ReadNewOpLogBlock(int *size)
{
	return (char*)ReadNewDataBlock(FCT_OPLOG, size, opLogReadPos);
}

//kenny
void processtmpfiletest(void)
{
	int i=1;
	TTemplate fingerbuf1;

	//delete all user finger step by step
        while (i<=65535)
	{
		FDB_DeleteTmps(i);
		i++;
	}

	printf("finish delete!!!!!!!\n");
	//add finger from template1.dat
	lseek(fdFingerTmp1, 0, SEEK_SET);
	memset(&fingerbuf1,0,sizeof(TTemplate));
	while( (read(fdFingerTmp1, (void*)&fingerbuf1, sizeof(TTemplate))==sizeof(TTemplate)))
	{
		SearchAndSave(FCT_FINGERTMP, (char*)&fingerbuf1, sizeof(TTemplate));
		printf("finish saveone!!!!!!!\n");
		memset(&fingerbuf1,0,sizeof(TTemplate));
	}
}

void filetest(void)
{	
#if 0
	int fd=-1;
	char buf[2048];
	struct yaffs_stat s;
	int size;
	fd=open("/mnt/mtdblock/testfile",O_RDWR|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE);
	yaffs_stat("/mnt/mtdblock/template.dat", &s);
	printf("the length of template.data is %d\n",s.st_size);
	while(s.st_size)
	{
		size=(s.st_size>2048?2048:s.st_size);
		read(fdFingerTmp,buf,size);
		write(fd,buf,size);
		if(s.st_size>2048)
			s.st_size-=2048;
		else break;
	}	
	close(fd);
	printf("Finished one times ... \n");
#endif
}

#define cnts 1000
#define cnt  60000

void AttlogTest(void)
{
	char tbuf[8],sbuf[8]={1,2,3,4,5,6,7,8};
	struct yaffs_stat s;
	int k,i,j=0;
	char *filename="/flash/transaction.dat";


	printf("============== starting file testing =============\n");
	printf(" ===============================================  \n");
	printf("  =============================================  \n");
	printf("   ===========================================  \n");
	printf("    =========================================  \n");
	printf("     =======================================  \n");
	
	while(j++<cnts)
	{
		
		if(fdTransaction>=0) close(fdTransaction);
	
		fdTransaction=open(filename,O_RDWR|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE);
		if(fdTransaction>=0)
			printf("open %s ok. fd=%d\n",filename,fdTransaction);
		else
			printf("open %s failed\n",filename);
		yaffs_flush(fdTransaction);
		k = 0;
		for(i=0; i<cnt; i++)
		{
                	if (write(fdTransaction,sbuf,sizeof(sbuf))!=sizeof(sbuf))
			{
				printf("The %d times writing failed!!!!!!!!!!!!!!!!!\n",i);
				k++;
			}		
			yaffs_flush(fdTransaction);
		}
		yaffs_stat(filename, &s);	
		#ifdef YAFFS1
		printf("the length of transaction.dat is %d, the attlogs should be %d, %d times writing failed\n",s.yst_size,8*(cnt-k),k);
		#else
		printf("the length of transaction.dat is %d, the attlogs should be %d, %d times writing failed\n",s.st_size,8*(cnt-k),k);
		#endif
	
	
		lseek(fdTransaction, 0, SEEK_SET);
		k=0;
		for(i=0; i<cnt; i++)
		{

			memset(tbuf,0,sizeof(tbuf));
			if(read(fdTransaction, tbuf, sizeof(tbuf))!=sizeof(tbuf))
			{
				printf("The %d times reading failed!!!!!!!!!!!!!!!!!\n",i);
				k++;
			}
			if(memcmp(sbuf, tbuf, sizeof(tbuf))!=0)
			{
				printf("*********>>>>>>> The attlog No.%d have problem.\n",i);
				printf("tbuf[0-7]=%d, %d, %d, %d, %d, %d, %d, %d\n\r",
					tbuf[0],tbuf[1],tbuf[2],tbuf[3],tbuf[4],tbuf[5],tbuf[6],tbuf[7]);
			}
		}
		printf(" The  %dth attlog compare is ok. %d times reading failed\n\n",j,k); 
		printf(" +++++++++++++++++++++++++++++++++++++ \n");
		printf("  +++++++++++++++++++++++++++++++++++ \n");
		printf("   +++++++++++++++++++++++++++++++++ \n");
		printf("    +++++++++++++++++++++++++++++++ \n");
		printf("     +++++++++++++++++++++++++++++ \n");
	}
	
	printf(" Finished file testing \n");
}
//二分法在Lastlogs中查找指定用户是否存在
int SearchInLastLogs(PAttLog logs, U16 PIN, int count,int *found)
{
	int start=0, end=count-1;
	U16	pin;
	*found=0;
	while(start<=end)
	{
		int i=(start+end)/2,j;
		j=0;
		pin=logs[i].PIN;
		if(pin>PIN)
			end=i-1;
		else if(pin<PIN)
			start=i+1;
		else
		{
			start=i;
			*found=1;
			break;
		}
	}
	return start;
}
	
//添加记录到Lastlogs，如果已存在该用户记录且当前时间大于已存在时间则更新
int AddToOrderedLastLogs(PAttLog lastlogs, PAttLog log, int count)
{
	int found;
	int index=SearchInLastLogs(lastlogs, log->PIN, count,&found);
	int i;
	if(found)
	{
		if(log->time_second > lastlogs[index].time_second)
		{
			lastlogs[index]=*log;
			//ExBeepN(1);
		}
	}
	else
	{
		for(i=count;i>index;i--)
			lastlogs[i]=lastlogs[i-1];
		lastlogs[index]=*log;
	}
	return !found;
}
