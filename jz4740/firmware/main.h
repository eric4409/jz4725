#ifndef	_MAIN_H_
#define	_MAIN_H_

#include "arca.h"
#include "exfun.h"
#include "ccc.h"

#define  FINGER_CACHE_SIZE	(1024*1024*2)
char *gImageBuffer;
TTime gCurTime;

typedef struct _VSStatus_{
	BOOL PIN;
	BOOL FP;
	BOOL Password;
	BOOL Card;
}GCC_PACKED TVSStatus, *PVSStatus;

void ShowMainLCD(void);
void EnableDevice(int Enabled);
void ExSetPowerSleepTime(int IdleMinute);
int ShowFPAnimate(int x, int y);
void OutputPrinterFmt2(int pin);
extern int  WaitInitSensorCount;
int Filter_Group_Run(int TID);

/* From net.h  --treckle*/
typedef struct _FPResult_{
        U32 PIN;
        char Name[20];
        char Group[20];
}GCC_PACKED TFPResult, *PFPResult;

#define FACTORY_MODE	0
#define RELEASE_MODE	1

#endif
