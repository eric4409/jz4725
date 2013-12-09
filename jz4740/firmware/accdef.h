/*************************************************
                                           
 ZEM 200                                          
                                                    
 accdef.h                              
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef __ACCDEF_H__
#define __ACCDEF_H__

#include "exfun.h"

#define TZ_MAX	50

#define TZ_START 0
#define TZ_END 1

#define TZ_HOUR 0
#define TZ_MINUTE 1

#define TZ_SUN 0
#define TZ_MON 1
#define TZ_TUE 2
#define TZ_WED 3
#define TZ_THU 4
#define TZ_FRI 5
#define TZ_STA 6

enum __VERIFY_STYLE
{
	VS_FP_OR_PW_OR_RF=0,
	VS_FP,
	VS_PIN,
	VS_PW,
	VS_RF,
	VS_FP_OR_PW,
	VS_FP_OR_RF,
	VS_PW_OR_RF,
	VS_PIN_AND_FP,
	VS_FP_AND_PW,
	VS_FP_AND_RF,
	VS_PW_AND_RF,
	VS_FP_AND_PW_AND_RF,
	VS_PIN_AND_FP_AND_PW,
	VS_FP_AND_RF_OR_PIN,
	VS_NUM=15
};

typedef struct _TZ_{
	unsigned char intervals[7][2][2];
}TTimeZone, *PTimeZone;

//载入时间段
int LoadTimeZone(int Index, PTimeZone TZ);
//保存时间段
int SaveTimeZone(int Index, PTimeZone TZ);

//查询用户所属的组编号
int GetUserGrp(int UserID);
//保存用户的组编号，成功时返回TRUE, 失败返回FALSE
int SaveUserGrp(int UserID, int GrpID);
//取用户的时间段设置，返回设置的时间段数量，0表示没有设置，大于0表示用户设置，小于0表示组设置
int GetUserTZ(int UserID, int *TZs);
//取组的时间段设置，返回设置的时间段数量，0表示没有设置，大于0表示设置的时间段编号
int GetGrpTZ(int GrpID, int *TZs);

//保存时间段设置
int SaveGrpTZ(int ID, int *TZs, int Count);
int SaveUserTZ(int UserID, int *TZs, int Count);

//清除用户时间段设置
int ClearUserTZ(int UserID);

int SetUserTZValue(PUser User, int *TZs, int UseGroup);
int GetUserTZValue(PUser User, int *TZs);

//测试用户的时间是否合法，合法返回非零，不合法返回0
int TestUserTZ(int UserID, TTime t);

//测试组合TestGrp是否匹配合法组合 ValidGrps
int TestGrp(char *ValidGrps, char *TestGrp);
//测试组合TestGrp是否匹配所有的合法组合
int TestAllGrp(char *TestGrp);

int InputTimeInterval(int row, int col, int *times);

//清除当前多用户开锁组合的缓冲记录
int ClearMultiOpen();

#endif
