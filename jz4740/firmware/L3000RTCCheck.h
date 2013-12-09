
#ifndef     _L3000RTC
#define	    _L3000RTC


//#include	"uboot-jz/jz47xx.h"
#include	"ccc.h"
#include	"app.h"
#include	"L3000Drv.h"


#define	DS3231    1
#define HT1380    0
#define SD2403    2


#if (LOCK_L6000==1)
#define	RTC_TYPE  DS3231
#elif (LOCK_L7000==1)
#define RTC_TYPE  SD2403
#endif

#define	PIN_RTC_DAT   75
#define	PIN_RTC_CLK   74
#define	PIN_RTC_RST   95

#if (RTC_TYPE==HT1380)


#define	L3000_RTC_INIT()    			{	__gpio_as_output(PIN_RTC_RST);  \
							__gpio_clear_pin(PIN_RTC_RST);\
							__gpio_as_output(PIN_RTC_CLK); \
							__gpio_clear_pin(PIN_RTC_CLK);\
                                                        __gpio_as_output(PIN_RTC_DAT);\
							__gpio_clear_pin(PIN_RTC_DAT);}
							
#define	RTC_DAT1       { __gpio_set_pin(PIN_RTC_DAT);}
#define	RTC_DAT0       {__gpio_clear_pin(PIN_RTC_DAT);}
#define	RTC_CLK1	   {__gpio_set_pin(PIN_RTC_CLK);}
#define	RTC_CLK0        {__gpio_clear_pin(PIN_RTC_CLK);}
#define	RTC_RST1	   {__gpio_set_pin(PIN_RTC_RST);}
#define	RTC_RST0        {__gpio_clear_pin(PIN_RTC_RST);}

#define	SET_RTC_DATOUT()          {__gpio_as_output(PIN_RTC_DAT);}
#define	SET_RTC_DATIN()          {__gpio_as_input(PIN_RTC_DAT);}
#define	RTC_GET_DAT(va) {va = __gpio_get_pin(PIN_RTC_DAT);}
						
#elif(RTC_TYPE==DS3231)
#define	L3000_RTC_INIT()    			{	__gpio_as_output(PIN_RTC_RST);  \
							__gpio_clear_pin(PIN_RTC_RST);\
							__gpio_as_output(PIN_RTC_CLK); \
							__gpio_clear_pin(PIN_RTC_CLK);\
                                                        __gpio_as_output(PIN_RTC_DAT);\
							__gpio_clear_pin(PIN_RTC_DAT);}
#elif(RTC_TYPE==SD2403)
#define	L3000_RTC_INIT()    			{	__gpio_as_output(PIN_RTC_RST);  \
							__gpio_clear_pin(PIN_RTC_RST);\
							__gpio_as_output(PIN_RTC_CLK); \
							__gpio_clear_pin(PIN_RTC_CLK);\
                                                        __gpio_as_output(PIN_RTC_DAT);\
							__gpio_clear_pin(PIN_RTC_DAT);}
#endif

#endif
