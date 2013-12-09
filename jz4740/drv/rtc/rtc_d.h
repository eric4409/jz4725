#ifndef __RTC_H__
#define __RTC_H__

typedef struct{
unsigned short int u16_year;
unsigned char u8_month;
unsigned char u8_week;
unsigned char u8_day;
unsigned char u8_hour;
unsigned char u8_minute;
unsigned char u8_second;
}ST_datetime; //type define a struct of date time

struct rtc_time {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};
int rtc_ioctl(unsigned int cmd,struct rtc_time *val,unsigned int epo);
int jz_rtc_ioctl(unsigned int cmd,struct rtc_time *val,unsigned int epo);
/*
void get_rtc_time (struct rtc_time *rtc_tm);
void get_rtc_alm_time (struct rtc_time *alm_tm);
unsigned int jz_mktime(int year, int mon, int day, int hour, int min, int sec);
void jz_gettime(unsigned int rtc, int *year, int *mon, int *day, int *hour,
				int *min, int *sec, int *weekday);
*/
#endif
