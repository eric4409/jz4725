/*************************************************
                                           
 ZEM 200                                          
                                                    
 commu.h communication for PC                             
                                                      
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef _COMMU_H_
#define _COMMU_H_

#include "arca.h"
#include "ccc.h"
#include "utils.h"

//command for data management
#define CMD_USERTMPS_RRQ	6
#define CMD_DB_RRQ 		7			/* read request */
#define CMD_USER_WRQ 		8			/* write request */
#define CMD_USERTEMP_RRQ 	9			/* read request */
#define CMD_USERTEMP_WRQ 	10			/* write request */
#define CMD_OPTIONS_RRQ		11			/* read request */
#define CMD_OPTIONS_WRQ 	12			/* write request */
#define CMD_ATTLOG_RRQ 		13			/* read request */

#define CMD_CLEAR_DATA		14			/* command */
#define CMD_CLEAR_ATTLOG	15

#define CMD_APPEND_USER		16
#define CMD_APPEND_USERTEMP 	17
#define CMD_DELETE_USER		18
#define CMD_DELETE_USERTEMP	19
#define CMD_CLEAR_ADMIN	20

//门禁相关命令
#define CMD_USERGRP_RRQ		21				//读用户分组
#define CMD_USERGRP_WRQ		22				//设置用户分组
#define CMD_USERTZ_RRQ		23				//读用户时间段设置
#define CMD_USERTZ_WRQ		24				//写用户时间段设置
#define CMD_GRPTZ_RRQ		25				//读组时间段设置
#define CMD_GRPTZ_WRQ		26				//写组时间段设置
#define CMD_TZ_RRQ		27				//读时间段设置
#define CMD_TZ_WRQ		28				//写时段设置
#define CMD_ULG_RRQ		29				//读开锁组合
#define CMD_ULG_WRQ		30				//写开锁组合
#define CMD_UNLOCK		31				//打开锁
#define CMD_CLEAR_ACC		32				//清除门禁设置

#define CMD_CLEAR_OPLOG		33
#define CMD_OPLOG_RRQ 		34			/* read request */

#define CMD_TEMPDB_CLEAR	48
#define CMD_TEMPDB_ADD		49

#define CMD_GET_FREE_SIZES  	50
#define CMD_GET_DATA_LAYOUT 	51
#define CMD_UPDATE_USERS	52			/* update all users data if it's empty */
#define CMD_UPDATE_TEMP		53		/* update all users template data if it's empty */
#define CMD_USER_RRQ		54
#define CMD_FREEID_RRQ		55
#define CMD_SENSOROPT_WRQ	56
#define CMD_ENABLE_CLOCK	57
#define CMD_SENSOROPT_RRQ       58

//模块相关命令
#define CMD_STARTVERIFY		60
#define	CMD_STARTENROLL		61
	#define ERR_OK		0
	#define ERR_STATE	8
	#define ERR_PARAM	1
	#define ERR_ENROLLED	2
	#define ERR_SAVE	3
	#define ERR_FAIL	4
	#define ERR_REPEAT	5
	#define ERR_CANCEL	6
	#define ERR_NOTENROLLED	7

#define	CMD_CANCELCAPTURE	62
#define CMD_TRANSSTATE		63
#define CMD_STATE_RRQ		64
#define CMD_LASTTEMP_RRQ	65
#define CMD_WRITE_LCD		66
#define CMD_CLEAR_LCD		67
#define CMD_LCDSIZE_RRQ		68
#define CMD_GET_PINWIDTH 	69

//SMS command
#define CMD_SMS_WRQ 		70
#define CMD_SMS_RRQ 		71
#define CMD_DELETE_SMS 		72
#define CMD_UDATA_WRQ   	73
#define CMD_DELETE_UDATA 	74

#define CMD_GET_IOSTATUS	75

//Mifare card command
#define CMD_WRITE_MIFARE        76
#define CMD_READ_MIFARE         77
#define CMD_EMPTY_MIFARE        78

//EXTUSER
#define CMD_EXTUSER_WRQ		79
#define CMD_EXTUSER_RRQ		80
#define CMD_DELETE_EXTUSER	81

//Workcode command
#define CMD_WorkCode_WRQ 	82
#define CMD_WorkCode_RRQ 	83
#define CMD_DELETE_WorkCode 	84

#define CMD_RTLOG_RRQ		90

//template
#define CMD_USERTEMP_EX_WRQ     87
#define CMD_USERTEMP_EX_RRQ     88

//command for fireware update
#define CMD_UPDATE_FIREWARE	110		/* update firmware */
#define UPDATE_BOOTLOADER	0
#define UPDATE_FIRMWARE		1
#define UPDATE_CFIRMWARE	2
#define UPDATE_FONT		3
#define UPDATE_OPTIONS		4
#define UPDATE_USERS		5
#define UPDATE_TEMPS		6
#define UPDATE_FLASH		7		/* WRITE FLASH FROM AN ADDRESS */
#define UPDATE_SOUND		8		/* update wav files */
#define UPDATE_ALGORITHM	9	       /* update algorithm files */
#define UPDATE_LANGUAGE		10	       /* update language files*/
#define UPDATE_AUTOSHELL	11	       /* update AUTO.SH files */
#define UPDATE_BATCHUSERDATA	12
#define UPDATE_RUN		100

#define CMD_QUERY_FIRWARE	111		/*  */
#define CMD_CPU_REG			112
#define CMD_CMOS_GAIN		113
#define CMD_UADATA_RRQ		114		/*  */
#define CMD_UADATA_WRQ		115		/*  */
#define CMD_APPEND_INFO		116
#define CMD_USER_INFO		117
#define CMD_MTHRESHOLD		118
#define CMD_HASH_DATA		119
#define CMD_READ_NEW		120
#define CMD_UPDATE_READALL	121

//command for device options
#define CMD_GET_TIME 		201		/* write request */
#define CMD_SET_TIME 		202
#define CMD_GET_COUNT 		203

//避免与ZEM200冲突
#define CMD_HTZ_RRQ			300
#define CMD_HTZ_WRQ			301

//command for register a event notification
#define CMD_REG_EVENT		500	

// command for connect and device control
#define CMD_CONNECT 		1000	/* connect request */
#define CMD_EXIT 		1001	/* disconnect request */
#define CMD_ENABLEDEVICE 	1002	/* enabled device operation */
#define CMD_DISABLEDEVICE	1003	/* disable device opreation */
#define CMD_RESTART		1004	/* restart device */
#define CMD_POWEROFF		1005	/* power off device */
#define CMD_SLEEP		1006    /* sleep device */
#define CMD_RESUME		1007	/* Resume sleeping device */

#define CMD_AUXCOMMAND		1008    /* Aux Board Command */
#define CMD_CAPTUREFINGER	1009    /* Capture a fingerprint image */
#define CMD_ENROLL		1010	/* Capture a fingerprint to enroll template */
#define CMD_TEST_TEMP		1011
#define CMD_CAPTUREIMAGE	1012    /* Capture a full fingerprint image */
#define CMD_REFRESHDATA		1013	/* Reload data from rom to cache */
#define CMD_REFRESHOPTION	1014	/* Refresh Options */
#define CMD_CALC_FINGER		1015	
#define CMD_RUN_PRG		1016
#define CMD_TESTVOICE		1017
#define CMD_GET_FLASHID		1018
#define CMD_GET_MCU_VERSION	1019
#define CMD_MCU_COMMAND		1020
#define CMD_SETFPDIRECT         1021    //Direct capture finger and not detect`
#define CMD_GET_VERSION		1100
#define CMD_CHANGE_SPEED	1101
#define CMD_AUTH		1102	/* 授权 */
#define CMD_ALARM_TRIGER        1104
#define CMD_WIEGAND 		1105
#define CMD_SLAVE_ALARM     1111    //报警事件
#define CMD_SLAVE_HID       1112    //card number

#define CMD_SERURITY_KEY_RRQ	1200
#define CMD_SERURITY_DATA_RRQ	1201
#define CMD_SERURITY_KEY_WRQ	1202

//2008.08.08 use for update file
#define CMD_UPDATEFILE	1700

#define CMD_PREPARE_DATA	1500	/* Prepare data transfer */
#define CMD_DATA		1501	/* Send a data packet */
#define CMD_FREE_DATA		1502	/* Free the transfered data */
#define	CMD_QUERY_DATA		1503
#define CMD_READ_DATA		1504
//dsl 2008.4.16
#define CMD_UPDATEFILE		1700
#define CMD_UPDATELANBYID	1701
#define CMD_READFILE		1702
#define CMD_CUSTATTSTATE_WRQ	1703
#define CMD_CUSTVOICE_WRQ	1704
#define CMD_USECUSTATTSTATE	1705
#define CMD_USECUSTVOICE	1706
#define CMD_DELCUSTATTSTATE	1707
#define CMD_DELCUSTVOICE	1708

//response from device
#define CMD_ACK_OK 		2000	/* Return OK to execute */
#define CMD_ACK_ERROR 		2001	/* Return Fail to execute command */
#define CMD_ACK_DATA		2002	/* Return Data */
#define CMD_ACK_RETRY		2003	/* Regstered event occorred */
#define CMD_ACK_REPEAT		2004
#define CMD_ACK_UNAUTH		2005	/* 连接尚未授权 */
#define CMD_ACK_UNKNOWN		0xffff	/* Return Unknown Command */

typedef int (*SendDataProc)(void *buf, int size, void *param);
typedef int (*CloseSessionProc)(void *param);

#define EF_ATTLOG	1
#define EF_FINGER	(1<<1)
#define EF_ENROLLUSER	(1<<2)
#define EF_ENROLLFINGER	(1<<3)
#define EF_BUTTON	(1<<4)
#define EF_UNLOCK	(1<<5)		//开锁
#define EF_STARTUP	(1<<6)		//起动系统
#define EF_VERIFY	(1<<7)		//验证指纹
#define EF_FPFTR	(1<<8)		//提取指纹特征点
#define EF_ALARM 	(1<<9)          //报警信号
#define EF_HIDNUM	(1<<10)		//射频卡号码
#define EF_WRITECARD    (1<<11)         //写卡成功
#define EF_EMPTYCARD    (1<<12)         //清除卡成功

#define SENDER_LEN	16
#define MAX_BUFFER_MSG  80           //最大缓存事件个数
#define MAX_MSGBUFFER_SIZE      1024 //事件缓存区的大小
/*
typedef struct _comm_session_{
	int SessionID;
	int StartTime;
	int LastActive;
	int LastCommand;
	int LastReplyID;
	int LastSendLen;
	char LastSendData[1032];
	void *Buffer;
	int BufferPos;
	int BufferLen;
	int Speed;
	int RegEvents;
	int Authen;
	WORD VerifyUserID;
	BYTE VerifyFingerID;
	SendDataProc Send;
	char Sender[SENDER_LEN];
	int TimeOutSec;
	CloseSessionProc Close;
	char Interface[16];
        char MsgBuffer[MAX_MSGBUFFER_SIZE]; //??????????
        int MsgLength[MAX_BUFFER_MSG];  //???????????
        int MsgStartAddress[MAX_BUFFER_MSG];//??????????????
        int MsgLast;                    //????????????????
        int MsgCount;                   //?????????????
        int MsgCached;                  //???????????
}TCommSession, *PCommSession;
*/

typedef struct _comm_session_{
	int SessionID;
	int StartTime;
	int LastActive;
	int LastCommand;
	int LastReplyID;
	int LastSendLen;
	char LastSendData[1032];
	TBuffer *Buffer;
	int Speed;
	int RegEvents;
	int Authen;
	WORD VerifyUserID;
	BYTE VerifyFingerID;
	SendDataProc Send;
	char Sender[SENDER_LEN];
	char MsgBuffer[MAX_MSGBUFFER_SIZE];			//事件内容缓存
	int MsgLength[MAX_BUFFER_MSG];	//事件内容的长度
	int MsgStartAddress[MAX_BUFFER_MSG];//事件内容的起始地址
	int MsgLast;			//正在传送的事件的序号
	int MsgCount;			//等待传送的事件个数
	int MsgCached;			//是否缓存事件数据
	int TimeOutSec;
	
	CloseSessionProc Close;
	char Interface[16];	
	// Batch cached file handle
	int fdCacheData;
}TCommSession, *PCommSession;

/*  PC->Device 

Reply ID is a flag to identify the replayed data from device. So while send data
to device, let ReplyID++ to keep every cmd have a different id.

|--------|--------|--------|--------|--------|--------|--------|--------|---
|       CMD       |    Check Sum    |    Session ID   |    Reply ID     |Data

*/

/* Device->PC

|--------|--------|--------|--------|--------|--------|--------|--------|---
|       CMD       |    Check Sum    |      Data1      |     Reply ID    |Data2
CMD=CMD_ACK_OK,CMD_ACK_ERROR,CMD_DATA,CMD_UNKNOWN
*/
typedef struct _CmdHdr_{
	unsigned short Command, CheckSum, SessionID, ReplyID;
}GCC_PACKED TCmdHeader, *PCmdHeader;

int CheckSessionSend(int EventFlag, char *Data, int Len);
PCommSession CheckSessionVerify(int *PIN, int *FingerID);

int RunCommand(void *buf, int size, int CheckSecury);
PCommSession CreateSession(void *param);
int CheckSessionTimeOut(void);
PCommSession GetSession(int SessionID);
int FreeSession(int SessionID);
int CheckSessionSendMsg(void);

#endif
