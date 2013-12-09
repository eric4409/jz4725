/*************************************************
                                           
 ZEM 200                                          
                                                    
 flashdb.h define all functions for database mangement of flash                             
                                                      
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef _FLASHDB_H_
#define _FLASHDB_H_

#include <time.h>
#include "arca.h"
#include "ccc.h"

#define MAX_USER_FINGER_COUNT 10	/* Max Finger Count for a User */
//������
#define FCT_EMPTY (U8)0xFF
//���ڼ�¼
#define FCT_ATTLOG (U8)1
//ָ������
#define FCT_FINGERTMP (U8)2
#define FCT_FINGERTMP1 (U8)99
//������¼
#define FCT_OPLOG (U8)4
//�û���¼
#define FCT_USER (U8)5
//SMS
#define FCT_SMS (U8)6
//UDATA
#define FCT_UDATA (U8)7
//System Options
#define FCT_SYSOPTIONS (U8)0x0a
//System Reserved
#define FCT_SYSTEM	(U8)0x0b
//Not for used
#define FCT_SYSTEM_NONE (U8)0xF0
//Web pages
#define FCT_WEBPAGES (U8)0x0c

#define FCT_ALL	-1
//Extend user infomation
#define FCT_EXTUSER	(U8) 8
//workcode infomation
#define FCT_WorkCode	(U8)9

#define PIN_WIDTH 5
#define MAX_PIN ((U16)0xFFFE)
#define MAX_PIN2 ((U32)0x7FFFFFFE)

#define FDB_OK 0				/* Success */
#define FDB_ERROR_NOTINIT	-1  		/* Database Not initialized */
#define FDB_ERROR_OP -2			/* Wrong Operation */
#define FDB_ERROR_IO -3			/* Flash I/O Error */
#define FDB_ERROR_NODATA -4		/* No (matched) Data */
#define FDB_ERROR_NOSPACE -5		/* No more space */
#define FDB_ERROR_DATA	-6		/* Data is not correct */

#define FDB_OVER_FLIMIT 1
#define FDB_OVER_UFLIMIT 2
#define FDB_OVER_ULIMIT	3
#define	FDB_FID_EXISTS 4
typedef struct _SearchHandle{
	int ContentType;
	char *buffer;
	int bufferlen;
	int datalen;
	int fd;
}TSearchHandle, *PSearchHandle;
#define PROGBAR_MAX_NUM		64
#define PROGBAR_USER_NUM	14
#define PROGBAR_FP_NUM		(PROGBAR_MAX_NUM - PROGBAR_USER_NUM)

//database initialization, return Fingertemplate count
int FDB_InitDBs(BOOL OpenSign);
void FDB_FreeDBs(void);
int FDB_GetSizes(char* Sizes);
//Clear all data
int FDB_ClearData(int ContentType);

#define PRI_VALIDUSER 	0
#define PRI_INVALIDUSER 1
#define PRI_VALIDBIT	1
#define PRI_ENROLL	2
#define PRI_OPTIONS 	4
#define PRI_SUPERVISOR 	8
#define ISADMIN(p)	(((p) & ~PRI_VALIDBIT)!=0)

#define PRIVILLEGE0 PRI_VALIDUSER
#define PRIVILLEGE1 (PRI_ENROLL)
#define PRIVILLEGE2 (PRI_ENROLL|PRI_OPTIONS)
#define PRIVILLEGE3 (PRI_ENROLL|PRI_OPTIONS|PRI_SUPERVISOR)

#define ISINVALIDUSER(user) (((user).Privilege & PRI_VALIDBIT)!=PRI_VALIDUSER)

#define ZKFPV10 10
#define MAXTEMPLATESIZE 602
#define MAXVALIDTMPSIZE 598 //for 4 bytes aligned
#define ZKFPV10_MAX_LEN 1664
#define ZKFPV10_VALID_LEN 1660 //for 4 bytes aligned
#define ZKFP_OFF_LEN    (ZKFPV10_MAX_LEN - MAXTEMPLATESIZE)

#define MAXTMPCOUNT	1500  //treckle

extern char PRIVALUES[];

#define MAXNAMELENGTH  8

#if 0
typedef struct _User_{
	U16 PIN;
	U8 Privilege;
	char Password[5];
	char Name[MAXNAMELENGTH];
	U8 Card[5];		//�����룬���ڴ洢��Ӧ��ID���ĺ���
	U8 Group;		//�û������ķ���
	U16 TimeZones;		//�û����õ�ʱ��Σ�λ��־
	U32 PIN2;		//32λ����û��ڶ�ʶ���
}GCC_PACKED TUser, *PUser;
#else
//�°汾���û����ݽṹ
typedef struct _User_{
	U16 PIN;
	U8 Privilege;
	char Password[5];
	char Name[8];
	U8 IDCard[3];		//�����룬���ڴ洢��Ӧ��ID���ĺ���
	U8 Verify;		//�������֤��ʽ
	U8 Reserved;		//����
	U8 Group;		//�û������ķ���. 4Bit
	U16 TimeZones;		//�û����õ�ʱ��Σ�λ��־//6 BIT Per TimeZone
	U32 PIN2;		//32λ����û��ڶ�ʶ���
}GCC_PACKED TUser, *PUser;
#endif

#define CARD_IS_ZERO(u) ((u)->IDCard[0] == 0 && (u)->IDCard[1] == 0 && (u)->IDCard[2] == 0)

U32 CurAttLogCount;

PUser FDB_CreateUser(PUser user, U16 pin, char *name, char *passwd, int privillege);
int FDB_AddUser(PUser user);
int FDB_ChgUser(PUser user);
int FDB_DelUser(U16 PIN);
PUser FDB_GetUser(U16 PIN, PUser user);
PUser FDB_GetUserByPIN2(U32 pin2, PUser user);
PUser FDB_GetUserByCard(BYTE *card, PUser user);

int FDB_ClrUser(void);
int FDB_ClrAdmin(void);	//this function to clear all user's privillege
int FDB_CntAdmin(int Privillege);
int FDB_CntUser(void);
int FDB_CntAdminUser(void);
int FDB_CntPwdUser(void);

typedef struct _Template_{
	U16 Size;
	U16 PIN;
	BYTE FingerID;
	BYTE Valid;
	BYTE Template[ZKFPV10_MAX_LEN]; //maximize template length
}GCC_PACKED TTemplate, *PTemplate;

PTemplate FDB_CreateTemplate(PTemplate tmp, U16 PIN, char FingerID, char *Template, int TmpLen);
int FDB_AddTmp(PTemplate tmp);
int FDB_DelTmp(U16 PIN, char FingerID);
int FDB_DeleteTmps(U16 PIN);
U32 FDB_GetTmp(U16 PIN, char FingerID, PTemplate tmp);
int FDB_ClrTmp(void);
int FDB_CntTmp(void);
int FDB_LoadTmp(void *Handle);
U32 FDB_GetTmpCnt(U16 UID);

int FDB_ChgTmpValidTag(PTemplate tmp, BYTE SetTag, BYTE ClearTag);
int FDB_ClrDuressTagTmpAll(void);
int FDB_IsDuressTmp(U16 PIN, char FingerID);
void RefreshTemplate();

#define DURESSFINGERTAG 2
#define ISDURESSFP(tmp) (0!=((tmp)->Valid & DURESSFINGERTAG))

#define FDB_SetDuressTmp(TmpAddress) FDB_ChgTmpValidTag(TmpAddress, DURESSFINGERTAG, 0)
#define FDB_ClearDuressTmp(TmpAddress) FDB_ChgTmpValidTag(TmpAddress, 0, DURESSFINGERTAG)

int FDB_PackData(int ContentType);

typedef struct _AttLog_{
	U16 PIN;
	BYTE verified;
	time_t time_second;
	BYTE status;
}GCC_PACKED TAttLog, *PAttLog;

typedef struct _ExtendAttLog_{
	U32 PIN;
	time_t time_second;
	BYTE status;
	BYTE verified;
	BYTE reserved[2];
	U32 workcode;
}GCC_PACKED TExtendAttLog, *PExtendAttLog;

int FDB_AddAttLog(U16 PIN, time_t t, char verified, char status, U32 pin2, U32 workcode, U8 SensorNo);

int FDB_ClrAttLog(void);
int FDB_CntAttLog(void);
void UpdateAttLog(void);

int FDB_GetSectorLayout(char *p);
char* FDB_ReadAttLogBlock(int *size);
char* FDB_ReadOPLogBlock(int *size);
char* FDB_ReadUserBlock(int *size);

typedef struct _OPLog_{
	U16 Admin;		//2
	BYTE OP;		//2
	time_t time_second;	//4
	U16 Users[4];		//2*4
}TOPLog, *POPLog;

typedef struct _ExtUser_{
	U16 PIN;
	BYTE VerifyStyle;
	U8 reserved[21];
}GCC_PACKED TExtUser, *PExtUser;

#define OP_POWER_ON    	0    //����
#define OP_POWER_OFF 	1    //�ػ�
#define OP_ALARM_VERIFY 2    //��֤ʧ��
#define OP_ALARM 	3    //����
#define OP_MENU 	4    //����˵�
#define OP_CHG_OPTION 	5    //��������, Users[3]����Ϊ������޸���Ŀ
#define OP_ENROLL_FP   	6    //�Ǽ�ָ��
#define OP_ENROLL_PWD  	7    //�Ǽ�����
#define OP_ENROLL_RFCARD 	8//�Ǽ�HID��
#define OP_DEL_USER   	9    //ɾ���û�
#define OP_DEL_FP      	10   //ɾ��ָ��
#define OP_DEL_PWD     	11   //ɾ������
#define OP_DEL_RFCARD  	12   //ɾ����Ƶ��
#define OP_CLEAR_DATA  	13   //�������
#define OP_MF_CREATE   	14	//����MF��
#define OP_MF_ENROLL   	15	//�Ǽ�MF��
#define OP_MF_REG      	16	//ע��MF��
#define OP_MF_UNREG    	17	//ɾ��MF��ע��
#define OP_MF_CLEAR    	18	//���MF������
#define OP_MF_MOVE     	19	//�ѵǼ������Ƶ�����
#define OP_MF_DUMP     	20	//�ѿ��е����ݸ��Ƶ�������
#define OP_SET_TIME    	21   //����ʱ��
#define OP_RES_OPTION  	22   //�ָ���������
#define OP_CLEAR_LOG   	23   //ɾ��������¼
#define OP_CLEAR_ADMIN 	24   //�������ԱȨ��}
#define OP_ACC_GRP     	25   //�޸��Ž�������
#define OP_ACC_USER    	26   //�޸��û��Ž�����
#define OP_ACC_TZ      	27   //�޸��Ž�ʱ���
#define OP_ACC         	28   //�޸Ŀ����������
#define OP_UNLOCK      	29   //����
#define OP_ENROLL_USER 	30   //�Ǽ����û�
#define OP_CHG_FP       31   //����ָ������
#define OP_DURESS       32   //в�ȱ���

int FDB_AddOPLog(U16 Admin, BYTE OP, U16 Objs1, U16 Objs2, U16 Objs3, U16 Objts4);
int FDB_ClrOPLog(void);
int FDB_CntOPLog(void);

#define MAX_OPLOG_COUNT (64*1024/sizeof(TOPLog))

int IsFreePIN(char *PIN);
int IsUsedPIN(char *PIN);
int IsFreePIN2(char *PIN);
int IsUsedPIN2(char *PIN);

U16 GetNextPIN(int From, int Free);
//update the userid map for test if a userid is used
int FDB_UpdateUserIDMap(void);
//search a pin in sorted pins buf(create by FDB_GetSortedPINs). 
//return 1 if found, else return 0. the ">=" position is stored in pos.
int FDB_SearchInPINs(char *buf, U16 pin, int start, int end, int *pos);

char* FDB_ReadBlockByFD(int *size, int ContentType, int fd);
char* FDB_ReadBlock(int *size, int ContentType);

int FDB_GetFreeFingerID(U16 PIN, BYTE *FID);

int FDB_CheckUpdata(void);	//�������ݿ��������

int FDB_CheckIntegrate(void);	//�������ݿ���ȷ�ԡ������Լ��

BOOL FDB_Download(int ContentType, char *dstFileName);

int AppendUserTemp(int pin, char *name, int fingerid, char *temp, int tmplen);
int AppendUser(int pin, char *name, char *password, int privillege);
int AppendFullUser(PUser user);

#define MAX_SMS_CONTENT_SIZE 	60 //������ʾ ����ÿ�а����������
#define MAX_SMS_COUNT 		1024

//�������ݱ�ʶ
#define UDATA_TAG_USERSMS 	0xFE    //��ʾ�û�����Ϣ
#define UDATA_TAG_ALL   	0xFD 	 //��ʾ����֪ͨ����Ϣ
#define UDATA_TAG_TEMP 		0xFF    //��ʾϵͳ��������ʱ��

#define STARTTIME_UNLIMITED 	0xFFFFFFFF
#define VALIDMINUTE_UNLIMITED 	0


#define MAX_WorkCode_COUNT 2048

typedef struct _SMS_{
	BYTE Tag;		//���
	U16 ID; 		//�������ݱ�ʶ 0��ʾ��¼�Ѿ���Ч}
	U16 ValidMinutes; 	//��Ч������   ��������
	U16 Reserved;
	U32 StartTime; 		//��ʼʱ��
	BYTE Content[MAX_SMS_CONTENT_SIZE+1];   //����Ϣ����
}GCC_PACKED TSms, *PSms;    //60 Bytes

//user->sms
typedef struct _UData_{
	U16 PIN;        //����ʾ��Ч��¼
	U16 SmsID;
}GCC_PACKED TUData, *PUData;  //4Bytes

PSms FDB_CreateSms(PSms sms, BYTE tag, U16 id, BYTE *content, U16 validminutes, time_t start);
int FDB_AddSms(PSms sms);
int FDB_ChgSms(PSms sms);
int FDB_DelSms(U16 id);
PSms FDB_GetSms(U16 id, PSms sms);
int FDB_ClrSms(void);
int FDB_CntSms(void);
void FDB_CheckSmsByStamp(U32 CurTime);
int FDB_PackSms(void);

int FDB_AddUData(PUData udata);
PUData FDB_GetUData(U16 id, PUData udata);
int FDB_DelUData(U16 PIN, U16 smsID);
PUData FDB_GetUDataByPINSMSID(U16 pin, U16 id, PUData udata);

//BYTE *FDB_ReadUserSms(U16 *smsid, BYTE *content);
int FDB_ReadBoardSms(BYTE *content);
BOOL CheckBoardSMS(void);
void DisplayBoardSMS(void);
BOOL CheckUserSMS(U16 pin, BYTE *smsContent);

//Update user dara 
typedef struct _UserS_{
	U8 OpSign;
	TUser User;
}GCC_PACKED TUserS, *PUserS;

typedef struct _FingerS_{
	U8 OpSign;
	U16 PIN;
	U8 FingerID;
	U32 OffSet;
}GCC_PACKED TFingerS, *PFingerS;

void BatchOPUserData(char *buffer, int fdCacheData);

typedef struct _AlarmRec_{
	U16 PIN;
	U32 LastTime;
}TAlarmRec, *PAlarmRec;

typedef struct _FilterRec_{
	U16 PIN;
	U32 PIN2;
}TFilterRec, *PFilterRec;

void GetFilterGroupInfo(int inputpin, PFilterRec filterbuf);
void GetFilterHeadInfo(int inputpin, PFilterRec filterbuf);

PExtUser FDB_CreateExtUser(PExtUser extuser, U16 pin, U8 verifystyle);
int FDB_AddExtUser(PExtUser extuser);
int FDB_ChgExtUser(PExtUser extuser);
int FDB_DelExtUser(U16 pin);
PExtUser FDB_GetExtUser(U16 pin, PExtUser extuser);

int FDB_GetAttLog(U16 pin, time_t StartTime,PAttLog logs, int MaxCount);
int FDB_GetAttExtLog(U16 pin, time_t StartTime,PExtendAttLog logs, int MaxCount);

//workcode.dat�� 8byte 2006.10.17
typedef struct _WORKCODE_{
	U16 WORKID;
	U32 WORKCODE;
	U8  Reserver[2];
}GCC_PACKED TWorkCode,*PWorkCode;

PWorkCode FDB_GetWorkCode(U16 id, PWorkCode workcode);
int IsValidWorkCode(char *value);
PWorkCode FDB_CreateWorkCode(PWorkCode workcode,U16 id, U32 jobcode);
int FDB_AddWorkCode(PWorkCode workcode);
int FDB_ChgWorkCode(PWorkCode WorkCode);
int FDB_DelWorkCode(U16 id);
int FDB_CntWorkCode(void);
int FDB_ClrWorkCode(void);
int UploadDataFromDisk(int ContentType);

typedef struct _GroupUserRec_{
	U8 Group;
	U16 PIN;
}TGroupUserRec, *PGroupUserRec;
int GetGroupUserPin();
int QueryGroupUser(U16 Pin);

#define RTLOGDATASIZE	16 //14->16 dsl 2007.7.30
#define LOG_READ_ALL 0
#define LOG_READ_NONE  -1
typedef struct _RTLog_{
	U16 EventType;	
	char Data[RTLOGDATASIZE];
} TRTLog, *PRTLog;

struct TRTLogNode;
typedef struct TRTLogNode *PRTLogNode;
struct TRTLogNode{
	TRTLog log;
	PRTLogNode Next;
};

void FDB_SetAttLogReadAddr(U32 addr);
void FDB_SetOpLogReadAddr(U32 addr);
char* FDB_ReadNewAttLogBlock(int *size);
char* FDB_ReadNewOpLogBlock(int *size);
//kenny
void processtmpfiletest(void);

#endif
