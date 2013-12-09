#ifndef _ZEM_APP_H_
#define _ZEM_APP_H_

#include "ccc.h"

#define U32  unsigned int
#define U8   unsigned char
//#define BYTE char

extern int debuging;

void InitClock(void);
void DelayUS(int us);
void DelayMS(int ms);
U32 GetUS(void);
U32 GetMS(void);
U32 SetCPUSpeed(U32 speed);

int isCheckingIdle(void);
int checkIdle(int tag);

#define	IP_HEAD		14
#define	UDP_HEAD		34


int InitCMOS(int Left,int Top,int Width,int Height, int FPReaderOpt);
int SetCMOSSize(int Left,int Top,int Width,int Height);
int SetCMOSSpeed(U32 CPUFreq);
int CaptureImage(char *Buffer, int wait);
void StopCaptureImage(void);

void InitGPIO(void);
int GPIOGet(int Index);
void GPIOSet(int Index);
void GPIOClear(int Index);
void GPIO_DirIn(int Index);
void GPIO_DirOut(int Index);


void enabledcache(void);
void disabledcache(void);

#define CMOS_WIDTH 640
#define CMOS_HEIGHT 480
#define CMOS_7620_ID	0x42
#define CMOS_7131_ID	0x22
#define CMOS_0360_ID	0xBA
#define CMOS_0303_ID	0x30
extern char CMOSDevAddress;

U32 GetPC(void);
void SetPC(U32 address);

void UDPSend(void *buf, int size);
void UDPSendStr(char *str);

#define TRACERUN DebugOutput2("%s(%d)\n", __FILE__,__LINE__)
#define TRACER(msg) DebugOutput3("%s(%d):%s\n", __FILE__, __LINE__, msg)

#ifdef PRINTF
#ifndef LINUX
//typedef int (*t_printf)(const char *fmt, ...);
//extern printf(const char *fmt, ...);
#endif
#endif

#ifndef WIN32
#define DM_CwDbgPrintf(...)
#define LOGERROR(...)

#ifdef PRINTF
#define _RPT0(_MsgType, _msg) printf(_msg)
#else
#define _RPT0(_MsgType, _msg) UDPSendStr(_msg)
#endif

#define _RPT1(_MsgType, _fmt, _data1)	\
{\
	char buf[1024*10];\
	sprintf(buf, _fmt, _data1);\
	_RPT0(_MsgType, buf);\
}
#define _RPT2(_MsgType, _fmt, _data1, _data2)	\
{\
	char buf[1024*10];\
	sprintf(buf, _fmt, _data1, _data2);\
	_RPT0(_MsgType, buf);\
}
#define _RPT3(_MsgType, _fmt, _data1, _data2, _data3)	\
{\
	char buf[1024*10];\
	sprintf(buf, _fmt, _data1, _data2, _data3);\
	_RPT0(_MsgType, buf);\
}
#define _RPT4(_MsgType, _fmt, _data1, _data2, _data3, _data4)	\
{\
	char buf[1024*10];\
	sprintf(buf, _fmt, _data1, _data2, _data3, _data4);\
	_RPT0(_MsgType, buf);\
}

#else
#undef _RPT0
#define _RPT0(a, _msg) printf(_msg)
#undef _RPT1
#define _RPT1(a, _msg, b) printf(_msg, b)
#undef _RPT2
#define _RPT2(a, _msg, b, c) printf(_msg, b, c)
#undef _RPT3
#define _RPT3(a, _msg, b, c, d) printf(_msg, b, c, d)
#undef _RPT4
#define _RPT4(a, _msg, b, c, d, e) printf(_msg, b, c, d, e)
#endif

#ifdef _DEBUG_ME_ //_DEBUGMODULE_
#define DebugOutput(_msg) _RPT0(0, _msg)
#define DebugOutput1(_msg, _data1) _RPT1(0, _msg, _data1)
#define DebugOutput2(_msg, _data1, _data2) _RPT2(0, _msg, _data1, _data2)
#define DebugOutput3(_msg, _data1, _data2, _data3) _RPT3(0, _msg, _data1, _data2, _data3)
#define DebugOutput4(_msg, _data1, _data2, _data3, _data4) _RPT4(0, _msg, _data1, _data2, _data3, _data4)
#define DebugDump(_msg, _data, _len) \
{\
	char dumpBuf[1024*10];\
	EncodeHex3(dumpBuf, _data, (_len)>3000?3000:(_len), 16);\
	DebugOutput3("%s[%d]:\n%s\n", _msg, _len, dumpBuf);\
}

#else
#define DebugOutput(_msg)
#define DebugOutput1(_msg, _data1)
#define DebugOutput2(_msg, _data1, _data2)
#define DebugOutput3(_msg, _data1, _data2, _data3)
#define DebugOutput4(_msg, _data1, _data2, _data3, _data4)
#define DebugDump(_msg, _data, _len)					  
#endif

int IIC_write(int DevAddress, U8 SubAddr, U8 *Data, int Len, int TimeOut);
int IIC_write0(int DevAddress, U8 *Data, int Len, int TimeOut);
void IIC_reset(void);

int CaptureFPImage(char *Buffer, U32 Size);

#endif

