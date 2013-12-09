/*
 * PCF8563 Real Time Clock interface for Linux
 *
 * It only support 24Hour Mode, And the stored values are in BCD format.
 * The alarm register is start at minute reg, no second alarm register.
 */

#include <jz4740.h>
#include "ucos_ii.h"
#include "rtc_d.h"
//#include "libc.h"

/**********************************************************************
 * register summary
 **********************************************************************/

#define RTC_SECONDS		0
#define RTC_MINUTES		1
#define RTC_HOURS		2
#define RTC_DAY_OF_WEEK		3
#define RTC_DAY_OF_MONTH	4
#define RTC_MONTH		5
#define RTC_YEAR		6

#define RTC_MINUTES_ALARM	8
#define RTC_HOURS_ALARM		0x09
#define RTC_DAY_ALARM           0x0b
#define RTC_WEEKDAY_ALARM	0x0a

/* control registers - Moto names
 */
#define RTC_CTR1                0x0f
#define RTC_CTR2                0x10
#define RTC_CTR3                0x11

#define RTC_ALM_READ 0
#define RTC_ALM_SET 1
#define RTC_RD_TIME 2
#define RTC_SET_TIME 3
#define RTC_EPOCH_READ 4
#define RTC_EPOCH_SET 5

#define EINVAL 1
#define EACCES 2
#define EFAULT 3

/* example: !(CMOS_READ(RTC_CONTROL) & RTC_DM_BINARY) 
 * determines if the following two #defines are needed
 */
#ifndef BCD2BIN
#define BCD2BIN(val) (((val) & 0x0f) + ((val) >> 4) * 10)
#endif

#ifndef BIN2BCD
#define BIN2BCD(val) ((((val) / 10) << 4) + (val) % 10)
#endif

#define ADDR_WRITE	0x64
#define ADDR_READ	0x65
extern int i2c_read(unsigned char device, unsigned char *buf,
                    unsigned char offset, int count);

extern int i2c_write(unsigned char device, unsigned char *buf,
              unsigned char offset, int count);
extern int i2c_read_2403(unsigned char device, unsigned char *buf,
             unsigned char offset, int count);
/*
 *	We sponge a minor off of the misc major. No need slurping
 *	up another valuable major dev number for this. If you add
 *	an ioctl, make sure you don't conflict with SPARC's RTC
 *	ioctls.
 */


/*
 * rtc_status is never changed by rtc_interrupt, and ioctl/open/close is
 * protected by the big kernel lock. However, ioctl can still disable the timer
 * in rtc_status and then with del_timer after the interrupt has read
 * rtc_status but before mod_timer is called, which would then reenable the
 * timer (but you would need to have an awful timing before you'd trip on it)
 */
static unsigned long rtc_status = 0;	/* bitmapped status byte.	*/

/*
 *	If this driver ever becomes modularised, it will be really nice
 *	to make the epoch retain its value across module reload...
 */
static unsigned int epoch = 1900;
static const unsigned char days_in_mo[] = 
{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static unsigned char rtcframe[16];

static void read_rtcframe(void)
{
	i2c_read(ADDR_READ>>1, rtcframe, 0, 16);
}

static void write_rtcframe(void)
{
	i2c_write(ADDR_WRITE>>1, rtcframe, 0, 16);
}

static void write_rtc(unsigned char addr, unsigned char val)
{
	unsigned char v = val;
	i2c_write(ADDR_WRITE>>1, &v, addr, 1);
}

static unsigned char read_rtc(unsigned char addr)
{
	 unsigned char v;
	i2c_read(ADDR_READ>>1, &v, addr, 1);
	return v;
}

static void CMOS_WRITE(unsigned char addr, unsigned char val) 
{
	rtcframe[addr] = val; 
}

static unsigned char CMOS_READ(unsigned char addr) 
{
	return rtcframe[addr]; 
}

void get_rtc_time(struct rtc_time *rtc_tm)
{
	unsigned char sec,mon,mday,wday,year,hour,min;

	/*
	 * Only the values that we read from the RTC are set. We leave
	 * tm_wday, tm_yday and tm_isdst untouched. Even though the
	 * RTC has RTC_DAY_OF_WEEK, we ignore it, as it is only updated
	 * by the RTC when initially set to a non-zero value.
	 */

	int i=0;
	for(i=0;i<3;i++)
	{
		//read_rtcframe();
		i2c_read_2403(ADDR_READ>>1, rtcframe, 0,7 );
	//	printf("**************read*************\n");
	//	int i;
	//	for(i=0; i<16; i++)
	//		printf("rtcframe[%d]=%x\n",i,rtcframe[i]);
		sec	= CMOS_READ(RTC_SECONDS) & ~0x80;
		min	= CMOS_READ(RTC_MINUTES) & ~0x80;
		hour	= CMOS_READ(RTC_HOURS) & ~0xc0;
		mday	= CMOS_READ(RTC_DAY_OF_MONTH) & ~0xc0;
		wday	= CMOS_READ(RTC_DAY_OF_WEEK) & ~0xf8;
		mon	= CMOS_READ(RTC_MONTH) & ~0xe0;
		year	= CMOS_READ(RTC_YEAR);
#if 0		
		if(year<0x09 || year>0x99)
		{
			DelayMS(2);
			continue;
		}
		else
			break;
#endif
	}
#if 0
	if((year<0x09 || year>0x99))
	{
		sec=0;
		min=0;
		hour=0;
		mday=0;
		wday=0;
		mon=0;
		year=0;
	}
#endif
	rtc_tm->tm_sec = BCD2BIN(sec);
	rtc_tm->tm_min = BCD2BIN(min);
	rtc_tm->tm_hour = BCD2BIN(hour);
	rtc_tm->tm_mday = BCD2BIN(mday);
	rtc_tm->tm_wday = wday;
	/* Don't use centry, but start from year 1970 */
	rtc_tm->tm_mon = BCD2BIN(mon);
	year = BCD2BIN(year);
	if ((year += (epoch - 1900)) <= 69)
		year += 100;
	rtc_tm->tm_year = year;
	/*
	 * Account for differences between how the RTC uses the values
	 * and how they are defined in a struct rtc_time;
	 */
	rtc_tm->tm_mon--;
}

void get_rtc_alm_time(struct rtc_time *alm_tm)
{ 
	unsigned char sec, min, hour;

	/*
	 * Only the values that we read from the RTC are set. That
	 * means only tm_hour, tm_min, and tm_sec.
	 */
	read_rtcframe();
	sec = 0;
	min	= CMOS_READ(RTC_MINUTES_ALARM);
	hour	= CMOS_READ(RTC_HOURS_ALARM);

	alm_tm->tm_sec = sec;//not set sec
	alm_tm->tm_min = BCD2BIN(min);
	alm_tm->tm_hour = BCD2BIN(hour);
}

void enableRTCWrite(void)
{
	write_rtc(RTC_CTR2,0x80);
	write_rtc(RTC_CTR1,0x84);
}
void disableRTCWrite(void)
{
	write_rtc(RTC_CTR1,0x0);
	write_rtc(RTC_CTR2,0x0);
}

void RTCRegisterClear(unsigned char addr)
{
	write_rtc(addr, 0);
}

int rtc_ioctl(unsigned int cmd,struct rtc_time *val,unsigned int epo)
{
	switch (cmd) {
	case RTC_ALM_READ:	/* Read the present alarm time */
	{
		/*
		 * This returns a struct rtc_time. Reading >= 0xc0
		 * means "don't care" or "match all". Only the tm_hour,
		 * tm_min, and tm_sec values are filled in.
		 */

		get_rtc_alm_time(val);
		break; 
	}
	case RTC_ALM_SET:	/* Store a time into the alarm */
	{
		unsigned char hrs, min, sec;
		struct rtc_time alm_tm;

		alm_tm = *val;
		hrs = alm_tm.tm_hour;
		min = alm_tm.tm_min;
		sec = alm_tm.tm_sec;

		if (hrs >= 24)
			return -EINVAL;

		hrs = BIN2BCD(hrs);

		if (min >= 60)
			return -EINVAL;

		min = BIN2BCD(min);

		if (sec >= 60)
			return -EINVAL;

		read_rtcframe();
		CMOS_WRITE(RTC_HOURS_ALARM, hrs | 0x80);
		CMOS_WRITE(RTC_MINUTES_ALARM, min | 0x80);

		CMOS_WRITE(RTC_DAY_ALARM, CMOS_READ(RTC_DAY_ALARM) | 0x80); 
		CMOS_WRITE(RTC_WEEKDAY_ALARM, CMOS_READ(RTC_WEEKDAY_ALARM) | 0x80);
	//	CMOS_WRITE(RTC_STATUS, CMOS_READ(RTC_STATUS) | 0x02);/*open alarm int*/
		write_rtcframe();
		break;
	}
	case RTC_RD_TIME:	/* Read the time/date from RTC	*/
	{
		get_rtc_time(val);
		return 0;
	}
	case RTC_SET_TIME:	/* Set the RTC */
	{
		struct rtc_time rtc_tm;
		unsigned char mon, day, hrs, min, sec, leap_yr, date;
		unsigned int yrs;
		unsigned char ctr;


		rtc_tm = *val;
		yrs = rtc_tm.tm_year + 1900;
		mon = rtc_tm.tm_mon + 1;   /* tm_mon starts at zero */
		day = rtc_tm.tm_wday;
		date = rtc_tm.tm_mday;
		hrs = rtc_tm.tm_hour;
		min = rtc_tm.tm_min;
		sec = rtc_tm.tm_sec;


		if (yrs < 1970)
			return -EINVAL;

		leap_yr = ((!(yrs % 4) && (yrs % 100)) || !(yrs % 400));

		if ((mon > 12) || (date == 0))
			return -EINVAL;

		if (date > (days_in_mo[mon] + ((mon == 2) && leap_yr)))
			return -EINVAL;
			
		if ((hrs >= 24) || (min >= 60) || (sec >= 60))
			return -EINVAL;

		if ((yrs -= epoch) > 255)    /* They are unsigned */
			return -EINVAL;

		/* These limits and adjustments are independant of
		 * whether the chip is in binary mode or not.
		 */


		if (yrs > 169) {
			return -EINVAL;
		}

		if (yrs >= 100)
			yrs -= 100;

		min = BIN2BCD(min);
		sec = BIN2BCD(sec);
		hrs = BIN2BCD(hrs)|0x80; /* 24 hour show */
		mon = BIN2BCD(mon);
		yrs = BIN2BCD(yrs);
		date = BIN2BCD(date);
 	
		read_rtcframe();
		enableRTCWrite();
		CMOS_WRITE(RTC_SECONDS, sec );
		CMOS_WRITE(RTC_MINUTES, min);
		CMOS_WRITE(RTC_HOURS, hrs);
		CMOS_WRITE(RTC_DAY_OF_MONTH, date);
		CMOS_WRITE(RTC_DAY_OF_WEEK, day);
		CMOS_WRITE(RTC_MONTH, mon);
		CMOS_WRITE(RTC_YEAR, yrs);
	//	printf("**************write*************\n");
	//	int i;
	//	for(i=0; i<16; i++)
	//		printf("rtcframe[%d]=%x\n",i,rtcframe[i]);

		write_rtcframe();
		RTCRegisterClear(0x12);	//clear the register(0x12) to be 0
		disableRTCWrite();
		return 0;
	}
	case RTC_EPOCH_READ:	/* Read the epoch.	*/
	{
		epo = epoch;
		return 0;
	}
	case RTC_EPOCH_SET:	/* Set the epoch.	*/
	{
		/* 
		 * There were no RTC clocks before 1900.
		 */

		 if (epo < 1900)
                       return -EINVAL;

                epoch = epo;
		return 0;
	} 
	default:
		return -EINVAL;
	}

	return 0;
}



/*
 *	We enforce only one user at a time here with the open/close.
 *	Also clear the previous interrupt data on an open, and clean
 *	up things on a close.
 */
int pcf_rtc_init(void)
{
	unsigned char ctr;
	int i;

/*	for(i=0; i<8; i++)
		write_rtc(0x14+i, i);

	for(i=0;i<8; i++)
	{
		ctr=read_rtc(0x14+i);
		printf("read: %x\n", ctr);
	} */

	printf("SD2403 RTC installed !!!\n");
	return 0;

}

