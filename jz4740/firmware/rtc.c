/*************************************************
                                           
 ZEM 200                                          
                                                    
 rtc.c Setup system time and RTC clock 
                                                      
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
                                                      
*************************************************/
#include "arca.h"
#include "exfun.h"
#include "rtc_d.h"
#include "rtc.h"
#include "rtc_d.h"
#include "options.h"
#include <ucos_ii.h>
 
TTime STime;
int PreOSTime = 0;
int InitSensorTime = 0;
int first=1;

/* Get the time of uC/OS for system*/
void GetOSTime(TTime *tm)
{
	int OSTime = 0;
	
	if(first)
	{
		ReadRTCClockToSyncSys(&STime);
		PreOSTime=OSTimeGet();
		first=0;
	}
	OSTime=OSTimeGet();
	if((OSTime-PreOSTime)>=100)
	{
		PreOSTime = OSTime;
		STime.tm_sec++;
	}
//	printf("STime.second=%d, STime.wday=%d, OSTime=%d\n",STime.tm_sec,STime.tm_wday,OSTime-PreOSTime);
	if(STime.tm_sec>59)
	{
		ReadRTCClockToSyncSys(&STime);
		PreOSTime=OSTimeGet();
	}

	memcpy(tm,&STime,sizeof(TTime));
}

/* Get the time of RTC on CPU for system time */
BOOL ReadJzRTC(TTime *tm)
{
    int retval;
    retval = jz_rtc_ioctl(RTC_RD_TIME, (struct rtc_time *)tm, 0);
    if (retval == -1) return FALSE;
    return TRUE;
}

extern int sensor_init(int left, int top, int width, int height, int FPReaderOpt);
/* set the time to RTC ds1302 */   
BOOL SetRTCClock(TTime *tm)
{
    	int retval;
#ifdef JZRTC
    	retval = jz_rtc_ioctl(RTC_SET_TIME, (struct rtc_time *)tm, 0);
#else
    	retval = rtc_ioctl(RTC_SET_TIME, (struct rtc_time *)tm, 0);
#endif
    	if (retval == -1) return FALSE; 
    	first = 1;	/* allow GetOSTime to sync system time */
    	printf("Set RTC successful\n");
    	return TRUE;
}

/* Read time from RTC ds1302, and then set it to jz4725's RTC */
BOOL ReadRTCClockToSyncSys(TTime *tm)
{
	int retval=0;

    /* read the time from rtc DS1302 */
#ifdef JZRTC
    	retval=jz_rtc_ioctl(RTC_RD_TIME, (struct rtc_time *)tm, 0);	
#else
    	retval=rtc_ioctl(RTC_RD_TIME, (struct rtc_time *)tm, 0);	
#endif
//    	DBPRINTF("RetVal=%d Year=%d Month=%d Day=%d Hour=%d Min=%d Sec=%d\n", 
//		retval, tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);    
/*
   if(((tm->tm_year<=70)||(tm->tm_year>=137))||
       ((tm->tm_mon>=12)||(tm->tm_mon<0))||
       ((tm->tm_mday<=0)||(tm->tm_mday>=32))||
       ((tm->tm_hour<0)||(tm->tm_hour>=24))||
       ((tm->tm_min<0)||(tm->tm_min>=60))||
       ((tm->tm_sec<0)||(tm->tm_sec>=60)))
	    retval=-1;
    //Fixed RTC time 
    if((tm->tm_year<=70)||(tm->tm_year>=137)) tm->tm_year=100;
    if((tm->tm_mon>=12)||(tm->tm_mon<0)) tm->tm_mon=0;
    if((tm->tm_mday<=0)||(tm->tm_mday>=32)) tm->tm_mday=1;
    if((tm->tm_hour<0)||(tm->tm_hour>=24)) tm->tm_hour=0;
    if((tm->tm_min<0)||(tm->tm_min>=60)) tm->tm_min=0;
    if((tm->tm_sec<0)||(tm->tm_sec>=60)) tm->tm_sec=0;
    if(retval==-1)
    {
	retval=rtc_ioctl(RTC_SET_TIME, tm, 0);
	printf("The time from RTC ds1302 is invalid\n");//treckle
    }
*/
    	tm->tm_isdst=-1;          /* don't know whether it's dst */

	/* set the time to jz4725's rtc if the time from ds1302 is correct */
	if(retval==0)
	{
	#ifndef JZRTC
		retval=jz_rtc_ioctl(RTC_SET_TIME, (struct rtc_time *)tm, 0);
		if(retval!=0)
		{
			printf("--------- SYNC system time failed \n");
			tm->tm_year = 2011 - 1900;
			tm->tm_mon = 0;
			tm->tm_mday = 1;
			tm->tm_hour = 8;
			tm->tm_min = 0;
			tm->tm_sec = 0;
			SetRTCClock(tm);
		}
	#endif
	}
	else
		printf("Reading the time from RTC error !!!\n");
    	return (retval==0);
}
