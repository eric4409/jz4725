#ifndef _ZFPRINT_H_
#define _ZFPRINT_H_

#ifndef BYTE
#define BYTE	unsigned char
#endif

#define SENSOR_HASFILM_TRINITY	1
#define SENSOR_NOFILM_REVERSE	2
#define SENSOR_UPEK		3
#define SENSOR_HASFILM_ZK6000	4

#define MEMERY_SIZE		0x6000

int CalcVar(BYTE *img, int width, int height, int *var, int *mean, int FrameWidth);
int DetectFP(BYTE *ImgBuf,int Width,int Height, int HasFingerThreshold,int NoFingerThreshold,int Reverse,int DetectCount,int IsSingle);

#endif
