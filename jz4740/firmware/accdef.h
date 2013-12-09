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

//����ʱ���
int LoadTimeZone(int Index, PTimeZone TZ);
//����ʱ���
int SaveTimeZone(int Index, PTimeZone TZ);

//��ѯ�û�����������
int GetUserGrp(int UserID);
//�����û������ţ��ɹ�ʱ����TRUE, ʧ�ܷ���FALSE
int SaveUserGrp(int UserID, int GrpID);
//ȡ�û���ʱ������ã��������õ�ʱ���������0��ʾû�����ã�����0��ʾ�û����ã�С��0��ʾ������
int GetUserTZ(int UserID, int *TZs);
//ȡ���ʱ������ã��������õ�ʱ���������0��ʾû�����ã�����0��ʾ���õ�ʱ��α��
int GetGrpTZ(int GrpID, int *TZs);

//����ʱ�������
int SaveGrpTZ(int ID, int *TZs, int Count);
int SaveUserTZ(int UserID, int *TZs, int Count);

//����û�ʱ�������
int ClearUserTZ(int UserID);

int SetUserTZValue(PUser User, int *TZs, int UseGroup);
int GetUserTZValue(PUser User, int *TZs);

//�����û���ʱ���Ƿ�Ϸ����Ϸ����ط��㣬���Ϸ�����0
int TestUserTZ(int UserID, TTime t);

//�������TestGrp�Ƿ�ƥ��Ϸ���� ValidGrps
int TestGrp(char *ValidGrps, char *TestGrp);
//�������TestGrp�Ƿ�ƥ�����еĺϷ����
int TestAllGrp(char *TestGrp);

int InputTimeInterval(int row, int col, int *times);

//�����ǰ���û�������ϵĻ����¼
int ClearMultiOpen();

#endif
