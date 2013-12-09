#ifndef __LOADCOMMON_H__
#define __LOADCOMMON_H__
#define LOAD_INIT_FINISH  1
#define LCD_INIT_HW_FINISH 2
#define CODEC_INIT_HW_FINISH 4

typedef struct{
	unsigned int mid;
	unsigned int init;
	unsigned int crc8; 
}LoadCommonType,*PLoadCommonType;
#endif
