#include <rtc_d.h>
void rtc_test()
{

	struct rtc_time testtime,test;
	int i;
/*
    //set alarm clock time
	rtc_ioctl(0,&testtime,0);
        printf("Alarm time before set:%d:%d:%d\n",testtime.tm_hour,testtime.tm_min,testtime.tm_sec);
	testtime.tm_hour=12;
	testtime.tm_min=23;
	testtime.tm_sec=56;
	udelay(70);
	rtc_ioctl(1,&testtime,0);
	udelay(70);
	rtc_ioctl(0,&testtime,0);
	printf("Alarm time after set:%d:%d:%d\n",testtime.tm_hour,testtime.tm_min,testtime.tm_sec);
*/
        //set real time clock
	testtime.tm_year=2008;
	testtime.tm_mon=7;
	testtime.tm_wday=1;
	testtime.tm_mday=28;
	testtime.tm_hour=17;
	testtime.tm_min=40;
	testtime.tm_sec=00;
	udelay(70);
//	rtc_ioctl(3,&testtime,0);
	udelay(70);
	rtc_ioctl(2,&testtime,0);
	printf("DS1302 Real time after set: %d:%d:%d %d/%d/%d\n",testtime.tm_hour,testtime.tm_min,testtime.tm_sec,testtime.tm_mon,testtime.tm_mday,testtime.tm_year);
        for(i =0; i< 5000;i++)
		udelay(1000);
	rtc_ioctl(2,&testtime,0);
	printf("DS1302 Real time after 5 seconds: %d:%d:%d %d/%d/%d\n",testtime.tm_hour,testtime.tm_min,testtime.tm_sec,testtime.tm_mon,testtime.tm_mday,testtime.tm_year);

}

void jz_rtc_test()
{

	struct rtc_time testtime,test;
	int i;

	testtime.tm_year=2008;
	testtime.tm_mon=7;
	testtime.tm_wday=1;
	testtime.tm_mday=28;
	testtime.tm_hour=17;
	testtime.tm_min=55;
	testtime.tm_sec=00;
	udelay(70);
	rtc_ioctl(2,&testtime,0);	//read time from ds1302
	jz_rtc_ioctl(3,&testtime,0);	//set time to jz rtc
	udelay(70);
	jz_rtc_ioctl(2,&testtime,0);
	printf("Real time after set: %d:%d:%d %d/%d/%d\n",testtime.tm_hour,testtime.tm_min,testtime.tm_sec,testtime.tm_mon,testtime.tm_mday,testtime.tm_year);
	while(1)
	{
        	for(i =0; i< 5000;i++)
        		udelay(1000);
		jz_rtc_ioctl(2,&testtime,0);
		printf("Real time after 5 seconds: %d:%d:%d %d/%d/%d\n",testtime.tm_hour,testtime.tm_min,testtime.tm_sec,testtime.tm_mon,testtime.tm_mday,testtime.tm_year);
	}
}
