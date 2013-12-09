/*
 * PCF8563 Real Time Clock interface for Linux
 *
 * It only support 24Hour Mode, And the stored values are in BCD format.
 * The alarm register is start at minute reg, no second alarm register.
 */

#include <jz4740.h>
#include <gpio.h>
#include "ucos_ii.h"
#include "rtc_d.h"

/**********************************************************************
 * register summary
 **********************************************************************/
#define RTC_SECONDS             0
#define RTC_MINUTES             1
#define RTC_HOURS               2
#define RTC_DAY_OF_MONTH        3
#define RTC_MONTH               4
#define RTC_DAY_OF_WEEK         5
#define RTC_YEAR                6

#define RTC_MINUTES_ALARM	9
#define RTC_HOURS_ALARM		0x0a
#define RTC_DAY_ALARM           0x0b
#define RTC_WEEKDAY_ALARM	0x0c


/* control registers - Moto names
 */
#define RTC_CONTROL             0x00
#define RTC_STATUS              0x01
#define RTC_CLKOUT		0x0d
#define RTC_TIMERCTL		0x0e
#define RTC_TIMERCOUNTDOWN	0x0f


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

#define DEVCLK  	11200000
#define TIMEOUT         1000

#define I2C_SDA		(32*3+23)	
#define I2C_SCK		(32*3+24)
#define I2C_RST		RTC_EN		//please define RTC_EN in gpio.c also

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
/*
 *	If this driver ever becomes modularised, it will be really nice
 *	to make the epoch retain its value across module reload...
 */
static unsigned int epoch = 1900;
static const unsigned char days_in_mo[] = 
{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static unsigned char rtcframe[16];

unsigned char HT1380_Read(void)
{
	unsigned char val;
	int i;

	val = 0;
	for(i=0; i<8; i++)
	{
		if(__gpio_get_pin(I2C_SDA))
			val |= (1<<i);
		__gpio_set_pin(I2C_SCK);
		udelay(1);
		__gpio_clear_pin(I2C_SCK);
		udelay(1);
        }

	return val;
}

void HT1380_Write(unsigned char ucDa)
{
	unsigned char val,i;

	val=ucDa;

        for(i=0; i<8; i++)
        {
		if(val&0x1)
			__gpio_set_pin(I2C_SDA);
		else
			__gpio_clear_pin(I2C_SDA);

                __gpio_set_pin(I2C_SCK);
		udelay(1);
                __gpio_clear_pin(I2C_SCK);
		udelay(1);

                val>>=1;
        }

	return;
}

void HT1380_WriteByte(unsigned char addr,unsigned char d)
{
	__gpio_clear_pin(I2C_RST);
	__gpio_clear_pin(I2C_SCK);
	__gpio_set_pin(I2C_RST);
        HT1380_Write(addr);
	mdelay(1);
        HT1380_Write(d);
	__gpio_set_pin(I2C_SCK);
	__gpio_clear_pin(I2C_RST);
	mdelay(1);

}

static int IsValidTime(void)
{

        if((rtcframe[3]==0) || (rtcframe[4]==0))
                return 0;
        if((rtcframe[0]>0x59) || (rtcframe[1]>0x59) || (rtcframe[2]>0x24))
                return 0;
        if((rtcframe[3]>0x31) || (rtcframe[4]>0x12))
                return 0;

        return 1;
}

static void read_rtcframe(void)
{
	int i;

	__gpio_as_output(I2C_RST);
	__gpio_as_output(I2C_SCK);
	__gpio_as_output(I2C_SDA);

	__gpio_clear_pin(I2C_RST);
	__gpio_clear_pin(I2C_SCK);
	__gpio_set_pin(I2C_RST);

	HT1380_Write(0xbf);

	__gpio_as_input(I2C_SDA);
	for(i=0;i<8;i++)
		rtcframe[i]=HT1380_Read();
	HT1380_Read();
	__gpio_set_pin(I2C_SCK);
	__gpio_clear_pin(I2C_RST);

	if(!IsValidTime())
		printf("reading time is invalid\n");

	__gpio_as_output(I2C_SDA);
	__gpio_clear_pin(I2C_SDA);

	return;
}

static void write_rtcframe(void)
{
	int i;

	__gpio_as_output(I2C_RST);
	__gpio_as_output(I2C_SDA);
	__gpio_as_output(I2C_SCK);

	if(IsValidTime())
	{
		HT1380_WriteByte(0x8e, 0x00);                   // Write Enable
		udelay(100);
		__gpio_clear_pin(I2C_RST);
		__gpio_clear_pin(I2C_SCK);
		__gpio_set_pin(I2C_RST);
		HT1380_Write(0xbe);                             // Clock Burst Write
		mdelay(1);
		for(i=0;i<8;i++)
                	HT1380_Write(rtcframe[i]);

		HT1380_Write(0);
		mdelay(1);
		HT1380_WriteByte(0x8e, 0x80);                   // Write Disable

		__gpio_set_pin(I2C_SCK);
//		__gpio_clear_pin(I2C_RST);
	}
	else
		printf("the writing time and date is invald\n");

	__gpio_clear_pin(I2C_SDA);
	__gpio_clear_pin(I2C_RST);

	return;
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

	read_rtcframe();
	sec	= CMOS_READ(RTC_SECONDS) & ~0x80;
	min	= CMOS_READ(RTC_MINUTES) & ~0x80;
	hour	= CMOS_READ(RTC_HOURS) & ~0xc0;
	mday	= CMOS_READ(RTC_DAY_OF_MONTH) & ~0xc0;
	wday	= CMOS_READ(RTC_DAY_OF_WEEK) & ~0xf8;
	mon	= CMOS_READ(RTC_MONTH) & ~0xe0;
	year	= CMOS_READ(RTC_YEAR)  ;

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

int rtc_ioctl(unsigned int cmd,struct rtc_time *val,unsigned int epo)
{
	struct rtc_time wtime; 
	switch (cmd) {
	case RTC_ALM_READ:	/* Read the present alarm time */
	{
		/*
		 * This returns a struct rtc_time. Reading >= 0xc0
		 * means "don't care" or "match all". Only the tm_hour,
		 * tm_min, and tm_sec values are filled in.
		 */

	//	get_rtc_alm_time(val);
		break; 
	}
	case RTC_ALM_SET:	/* Store a time into the alarm */
	{
	/*
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
		CMOS_WRITE(RTC_STATUS, CMOS_READ(RTC_STATUS) | 0x02);//open alarm int
		write_rtcframe();
	*/

		return 0;
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
		yrs = rtc_tm.tm_year +1900;
		mon = rtc_tm.tm_mon+1;   /* tm_mon starts at zero */
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
		hrs = BIN2BCD(hrs);
		mon = BIN2BCD(mon);
		yrs = BIN2BCD(yrs);
		date = BIN2BCD(date);
 	
		read_rtcframe();

		CMOS_WRITE(RTC_SECONDS, sec );
		CMOS_WRITE(RTC_MINUTES, min);
		CMOS_WRITE(RTC_HOURS, hrs);
		CMOS_WRITE(RTC_DAY_OF_MONTH, date);
		CMOS_WRITE(RTC_DAY_OF_WEEK, day);
		CMOS_WRITE(RTC_MONTH, mon);
		CMOS_WRITE(RTC_YEAR, yrs);

		write_rtcframe();

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
	/*
		 if (epo < 1900)
                       return -EINVAL;

                epoch = epo;
	*/
		return 0;
	} 
	default:
		return -EINVAL;
	}
}

