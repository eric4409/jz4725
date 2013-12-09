/*************************************************
                                           
 ZEM 200                                          
                                                    
 rtc.h 
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef _RTC_H_
#define _RTC_H_

#include "arca.h"
#include "exfun.h"

#define RTC_ALM_READ 0
#define RTC_ALM_SET 1
#define RTC_RD_TIME 2
#define RTC_SET_TIME 3
#define RTC_EPOCH_READ 4
#define RTC_EPOCH_SET 5

BOOL SetRTCClock(TTime *tm);
BOOL ReadRTCClockToSyncSys(TTime *tm);
BOOL ReadJzRTC(TTime *tm);
void GetOSTime(TTime *tm);

#endif

