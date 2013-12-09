/*************************************************
                                           
 ZEM 200                                          
                                                    
 exfun.h define times and voice function                              
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef _EXFUN_H_
#define _EXFUN_H_

#include <time.h>
#include "arca.h"

#define BCD2HEX(a) (((a) & 0x0F) + 10*((a)>>4))
#define HEX2BCD(a) (((a)/10)<<4 | ((a)%10))
#define VOICE_THANK		0
#define VOICE_INVALID_PWD	1
#define VOICE_INVALID_ADMIN	2
#define VOICE_INVALID_ID	3
#define VOICE_RETRY_FP		4
#define VOICE_REPEAT_ID		5
#define VOICE_NO_LOG_RECSPACE	6
#define VOICE_NO_ADM_SPACE	7
#define VOICE_REPEAT_FP		8
#define VOICE_ALREADY_LOG	9
#define VOICE_NOENROLLED_CARD	10
#define VOICE_DOOR_OPEN		11
#define VOICE_OK		12 //reserved
#define VOICE_RING		13
#define VOICE_CAMERACLICK	14
#define VOICE_REMOVE_FP		15

//adv worcode use
#define VOICE_WORKCODE_INVALID 16
#define VOICE_WK_HINTFP 17
#define MCU_CMD_NONE 0
#define MCU_CMD_KEY 1
#define MCU_CMD_VERSION 20
#define MCU_CMD_RTC 22
#define MCU_CMD_LIC 25
#define RCMD_LCD_SIZE 	33
#define CMD_LCD_STR		34
#define CMD_LCD_BRIGHT	35
#define MCU_CMD_DOOR	110
	#define DOOR_UNKNOWN		0  
	#define DOOR_SENSOR_BREAK 1	//门被意外打开
	#define DOOR_BUTTON		2	//出门开关
	#define DOOR_BREAK		3	//拆机
	#define DOOR_SENSOR_OPEN 4	//正常开门
	#define DOOR_SENSOR_CLOSE 5	//正常关门

#define MCU_CMD_BATTERY 111
#define BATTERY_None    2
#define BATTERY_Internal 1
#define BATTERY_External 0

#define RCMD_WG_OPTIONS 214
#define MCU_CMD_EXTWGIN 215
#define MCU_CMD_INTWGIN 216
#define	MCU_CMD_SWITCH  219

#define LED_GREEN 0
#define LED_RED 1

#define DIRECTLY  0
#define INQUIRING 1

#define UDCPRIO_LOW 	1
#define UDCPRIO_HIGH 	2
//Transfer Data structure
#define BUFSZ	2048
typedef struct _mybuf_{
	int len;
	unsigned char buf[BUFSZ];
}TMyBuf, *PMyBuf;

//Data transfer buffer and CheckSum
PMyBuf bget(int io);
unsigned short in_chksum(unsigned char *p, int len);
void FreeCommuCache(void);

int ClockEnabled;
int ShowMainLCDEnabled;


//int WaitInitSensorCount=0;	//Wait n seconds and then init sensor
	
void ExBeep(int delay);
void ExOpenRF(void);
int ExPlayVoice(int VoiceIndex);
void ExPlayVoiceFrom(int VoiceStart, int VoiceEnd);
void ExLightLED(int LEDIndex, int Light);
BOOL ExPowerOff(int Cmd);   
int ExPlayMP3(int mp3index, BOOL IsTestMP3);
void SetAudioVol(int vol);

void ExAuxOut(int AuxOnTime, int OpenDoorDelay);
int ExAlarm(int Index, int Delay);
BOOL CheckDOOR(char *buffer);
int ExAlarmOff(int Index);
void ExBell(U32 DelayTime);

typedef struct tm TTime;

time_t mktime_1(struct tm *tmbuf);
		
void GetTime(TTime *t);
void SetTime(TTime *t);
time_t GetSecond(void);

TTime *DecodeTime(time_t t, TTime *ts);
time_t EncodeTime(TTime *t);
int TimeDiffSec(TTime t1, TTime t2);
time_t OldEncodeTime(TTime *t);
TTime * OldDecodeTime(time_t t, TTime *ts);

void GoToSleepStatus(void);
void WakeUpFromSleepStatus(void);

BOOL HIDCheckCard(char *buffer);
void ExCloseRF(void);
void ExOpenWiegand(void);
void ExCloseWiegand(void);
BOOL WiegandSend(U32 deviceID, U32 CardNum, U32 DuressID);

enum GPIO_PIN_CTL{
      IO_FUN_GREEN_LED		= 0x07, //output is 0 padr pin7
      IO_FUN_RED_LED 		= 0x06,
      IO_FUN_LOCK 		= 0x05,
      IO_FUN_ALARMSTRIP 	= 0x14,
      IO_FUN_WOD1 		= 0x03, //wiegand output 
      IO_FUN_WOD0		= 0x02, //wiegand output and alarm
      IO_FUN_BUTTON		= 0x31, //input or output(A8)
      IO_FUN_SENSOR 		= 0x10, //input is 1 padr pin0
};

BYTE ExGetIOStatus(void);

void Switch_mode(U32 RS232Mode);
extern int gAlarmDelay,gAlarmDelayIndex,gBellDelay;

#define AUX_ON  0xFF
#define AUX_OFF 0

#define DOOR_UNKNOWN		0     
#define DOOR_SENSOR_BREAK 	1     //门被意外打开
#define DOOR_BUTTON 		2     //出门开关
#define DOOR_BREAK 		3     //拆机
#define DOOR_SENSOR_OPEN 	4     //正常开门
#define DOOR_SENSOR_CLOSE 	5     //正常关门

#define LOCKFUN_BASE 1
#define LOCKFUN_ADV  2
#define LOCKFUN_DOORSENSOR 4

#define RELAY_NO 		0
#define RELAY_NC		1
#define RELAY_NONE		2

//Extend for C2
int DoAlarm(int Index, int DelayMS);
void DoAuxOut(int AuxOnTime, int OpenDoorDelay);
int DoBell(int DelaySec);
int DoAlarmOff(int Index);
int DoBellOff(void);

void LoadSound(void);
void Format_Data(char c);
void InitializeUDC(void);
void ENABLE_UDC_IRQ(unsigned char e);

#endif
