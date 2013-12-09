/*************************************************
                                           
 ZEM 200                                          
                                                    
 exvoice.h                              
                                                      
 Copyright (C) 2003-2006, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef __EXVOICE_H__
#define __EXVOICE_H__

#define VOICE_TZ_NUM  8

#define VOICE_TZ_INDEX 1000
#define VOICE_GROUP_INDEX 2000

typedef struct _VTZ_{
	unsigned char intervals[VOICE_TZ_NUM][2][2];
}TVTimeZone, *PVTimeZone;

PVTimeZone CVTZ;

void LoadVoiceInfo(void);
int DoDefVerOKVoice(void *p);
void PlayVoiceByTimeZone(TTime t, int group, int defIndex);

#endif
