#ifndef __WIEGAND_H__
#define __WIEGAND_H__

#include "arca.h"

typedef struct _WiegandBitsDef_{
	U8 DeviceIDLen,CardIDLen,DuressIDLen;
	U8 EvenParityStart, EvenParityLen, EvenParityPos;
	U8 OddParityStart, OddParityLen, OddParityPos;
} TWiegandBitsDef, *PWiegandBitsDef;

extern TWiegandBitsDef gWiegandDef;
extern int gWGFailedID;
extern int gWGDuressID;
extern int gWGSiteCode;
extern int gWGOEMCode;
extern int gWGPulseWidth;
extern int gWGPulseInterval;
extern int gWGOutputType;

#define DUMMY_PULSE_WIDTH       0x10
#define DUMMY_PULSE_INTERVAL    0x20
#define DUMMY_OUTPUT_TYPE       0x30

//≥ı ºªØWiegand
int InitWiegandDef(PWiegandBitsDef WiegandBitsDef, char *BitDef);
int CalcWiegandData(U32 DeviceID, U32 CardNum,U32 DuressID, U8 *data1, PWiegandBitsDef WiegandBitsDef);

int CalcHID37WiegandData(U32 OEMCode, U32 DeviceID, U32 CardNum, U32 DuressID, U8 *data1);

#endif
