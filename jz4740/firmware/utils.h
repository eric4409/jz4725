/*************************************************
                                           
 ZEM 200                                          
                                                    
 utils.h 
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef _utils_h
#define _utils_h

#include "stdarg.h"
#include "arca.h"

#define BYTE2M (2*1024*1024)

typedef struct {
    unsigned char *buffer;
    int bufferSize;
    unsigned char *bufPtr, *bufEnd;
	int isRom;
}TBuffer;

U32 GetTickCount(void);
U32 GetUS();
char *GetEnvFilePath(const char *EnvName, const char *filename, char *fullfilename);
void SetIPAddress(char *Action, unsigned char *ipaddress);
void RebootMachine(void);
BOOL SetGateway(char *Action, unsigned char *ipaddress);
void SetNetworkIP_MASK(BYTE *ipaddress, BYTE *netmask);

int Decode16(char *String, char* Data);
char *ConvertMonth(char *version, char *iversion);

void doPrint(char * buffer, char * fmt, va_list ap);

int Hex2Char(char *s);

char *nstrcpy(char *Dest, const char *Source, int size);
int nstrcmp(const char *s1, const char *s2, int size);
BYTE *nmemcpy(BYTE *Dest, const BYTE *Source, int size);
int nmemcmp(const BYTE *s1, const BYTE *s2, int size);
int nmemset(BYTE *Dest, BYTE Data, int Size);

char* SPadCenterStr(char *buf, int size, const char *s);
char* SPadRightStr(char *buf, int size, const char *s);
char *Pad0Num(char *buf, int size, int value);

char *TrimRightStr(char *buf);
char *TrimLeftStr(char *buf);
char *TrimStr(char *buf);

int SearchIndex(char **Items, const char *Text, int ItemCount);

//--------�ַ���ת������-------------
//�ַ���strת�����޷�������*value���ɹ�����0��ʧ�ܷ��ط���
int strtou32(const char *str, unsigned int *value);
//�ַ���ת����IP��ַ
int str2ip(char* str, BYTE* ip);
//�ַ���ת����MAC��ַ
int str2mac(char* str, BYTE* mac);
int str2int(char *buf, int DefaultValue);
//16�����ַ���strת�����޷�������*value���ɹ�����0��ʧ�ܷ��ط���
int HexStrToInt(const char *str, int *value);
//�ַ���pת�����޷����������أ�Next����p����ֵ�����ĵ�ַ
int StrValue(const char *p, int *Next);
//16�����ַ���pת�����޷����������أ�Next����p����ֵ�����ĵ�ַ
int HexStrValue(const char *p, int *Next);

//-- ������ �ָ���������-----------
//��p�и��Ƶ�index��ֵ��buf��,���ظ��Ƶ��ַ����ĳ���
int SCopyStrFrom(char *buf, char *p, int index);
//�������б����ַ�����InvalidInt��ʾҪȥ����������PackInvalidInt��ʾ�Ƿ�ѹ������ķָ�����
int SaveIntList(char *buf, int *INTs, int Count, int InvalidIntO_RDONLY, int PackInvalidInt);
//ȥ���ַ����п�ͷ����β���ظ��ķָ����ţ������ش����ĳ���
int SPackStr(char *buf);
//����p�б��ָ���ַ����ĸ���
int SCountStr(char *p);
//����p�е�index��ֵ��������ʽ������Next�з��ظ���ֵ�Ľ�����ַ
int SIntValueFrom(char *p, int index, int *Next);
//����p�е�index��ֵ��16��ֵ����������ʽ������Next�з��ظ���ֵ�Ľ�����ַO_RDONLY
int SHexValueFrom(char *p, int index, int *Next);

//����̶����ȵ��ַ�����Ϊ0�������ַ��������ر����ĳ���
void StringEncode(char *buf, char *str, int size);
//����0�������ַ���Ϊ�̶������ַ��������ؽ����ĳ���
int StringDecode(char *buf, char *str);
//��16�����ַ�����
int EncodeHex(BYTE *String, BYTE *Data, int Size);


void SetBit(BYTE *Buffer, int Index);
void ClearBit(BYTE *Buffer, int Index);
int GetBit(BYTE *Buffer, int Index);
void memor(char *s1, char *s2, int len);

BOOL SaveMACToFlash(const char *MAC);
void RedBootMac(const char *serialnumber);
void SetRootPwd(int comkey);

int GetSensorSerialNumber(char *TitleStr, char *IDStr);
//void SaveMACAddr(void);

int GetMonDay(int year, int month);

void SyncTimeByTimeServer(BYTE *TimeServerIP);

int str2cardkey(char *str, BYTE* cardkey);

void ExportProxySetting(void);
TBuffer *createRomBuffer(unsigned int Address, int size);
TBuffer *createRamBuffer(int size);
void freeBuffer(TBuffer *buffer);

#ifdef ZEM300
BOOL SaveLOGOToFlash(int offset, char *config, int size);
BOOL SaveUBOOTToFlash(char *data, int size);
#endif

#endif

