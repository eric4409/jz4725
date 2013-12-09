#ifndef _L3000DRV
#define	_L3000DRV

#include	"jz4740.h"
#include	"ccc.h"
#include	"app.h"
#include	"gpio.h"

#define LOCK_L6000 0
#define LOCK_L7000 1

/*
//#define	_OLD_ZEM700
#ifdef   _OLD_ZEM700
#define		L3000_PWR_EN          106
#define		L3000_BEEP            92
#define		L3000_OPENM       83
#define		L3000_CLOSEM      82
#define		L3000_RED_LED     73
#define		L3000_GREEN_LED     72
#define		L3000_FG_PWR_CRL    89
#define		L3000_RST           300
#else
*/

#define		L3000_PWR_EN      (32*2 + 20) //106
#define		L3000_FG_PWR_CRL  89
#define		L3000_RST           300
//#define		L3000_PWR_LOW       35
//#define		L3000_PWR_LOW_ALM   34
//#define		L3000_PWR_LOW_OFF   17
#define		L3000_BAK_PWR       (32*1 + 18)//91
#define		L3000_LCD_BLIGHT    36
#define		L3000_SENSOR        23

#if (LOCK_L6000==1)
#define		L3000_USBMF_PWREN   37
#define		L3000_CLOSE_DOOR()   {__gpio_clear_pin(L3000_OPENM); __gpio_set_pin(L3000_CLOSEM);}
#define		L3000_OPEN_DOOR()   {__gpio_set_pin(L3000_OPENM); __gpio_clear_pin(L3000_CLOSEM);}
#define		L3000_STOP_DOOR()   {__gpio_set_pin(L3000_OPENM); __gpio_set_pin(L3000_CLOSEM);}
#define     L3000_BEEP_ON()       {__gpio_set_pin(L3000_BEEP);}
#define		L3000_BEEP_OFF()      {__gpio_clear_pin(L3000_BEEP);}
#define		L3000_USBMFPWR_ON()     {__gpio_set_pin(L3000_USBMF_PWREN);}
#define		L3000_USBMFPWR_OFF()     {__gpio_clear_pin(L3000_USBMF_PWREN);}

#define		L3000_GPIO_INIT()     {__gpio_as_output(L3000_PWR_EN);  \
				       __gpio_as_output(L3000_BEEP);    \
				       __gpio_as_output(L3000_OPENM);   \
				       __gpio_as_output(L3000_CLOSEM);  \
				       __gpio_set_pin(L3000_OPENM);	\
				       __gpio_set_pin(L3000_CLOSEM);	\
				       __gpio_as_output(L3000_RED_LED);  \
				       __gpio_as_output(L3000_GREEN_LED);  \
				       __gpio_as_output(L3000_FG_PWR_CRL);  \
				       __gpio_as_input(L3000_BAK_PWR);  \
				       __gpio_as_input(L3000_PWR_LOW_ALM);  \
				       __gpio_as_input(L3000_PWR_LOW);  \
				       __gpio_as_input(L3000_PWR_LOW_OFF);  \
				       __gpio_as_input(L3000_SENSOR);\
				       __gpio_as_output(L3000_USBMF_PWREN);\
				       __gpio_set_pin(L3000_USBMF_PWREN);\
				       __gpio_as_output(L3000_LCD_BLIGHT);  }
#elif (LOCK_L7000==1)
#define		L3000_USBMFPWR_ON()     {__gpio_as_output(L3000_M_EN);}
#define		L3000_USBMFPWR_OFF()     {__gpio_as_output(L3000_M_EN);}

#define		L3000_GPIO_INIT()     {__gpio_as_output(L3000_PWR_EN);  \
				       __gpio_as_output(L3000_BEEP);    \
				       __gpio_as_output(L3000_M_EN);   \
				       __gpio_as_output(L3000_OPENM);   \
				       __gpio_as_output(L3000_CLOSEM);  \
				      	L3000_STOP_DOOR();               \
					__gpio_as_output(L3000_RED_LED);  \
				       __gpio_as_output(L3000_GREEN_LED);  \
				       __gpio_as_output(L3000_FG_PWR_CRL);  \
				       __gpio_as_input(L3000_BAK_PWR);  \
				       __gpio_as_input(L3000_PWR_LOW_ALM);  \
				       __gpio_as_input(L3000_PWR_LOW);  \
				       __gpio_as_input(L3000_PWR_LOW_OFF);  \
				       __gpio_as_input(L3000_SENSOR);\
				       __gpio_as_output(L3000_M_EN);\
				       __gpio_set_pin(L3000_M_EN);\
				       __gpio_as_output(L3000_LCD_BLIGHT);  }

#endif

#define     MTRPWR_ON()		__gpio_set_pin(MTRPWR_EN)
#define     MTRPWR_OFF()	__gpio_clear_pin(MTRPWR_EN)

#define		L3000_CLOSE_DOOR()  do{MTRPWR_ON();  Write373(1, M1_IN);Write373(0, M2_IN);}while(0)
#define		L3000_OPEN_DOOR()   do{MTRPWR_ON();  Write373(0, M1_IN);Write373(1, M2_IN);}while(0)
#define		L3000_STOP_DOOR()   do{MTRPWR_OFF(); Write373(0, M1_IN);Write373(0, M2_IN);}while(0)
			
#define     L3000_BEEP_ON()     Write373(1, BEEP)	
#define		L3000_BEEP_OFF()    Write373(0, BEEP)
#define		L3000_RLED_ON()  	Write373(0, LEDR)
#define		L3000_RLED_OFF() 	Write373(1, LEDR)
#define		L3000_GLED_ON()  	Write373(0, LEDG)
#define		L3000_GLED_OFF() 	Write373(1, LEDG)

#define		L3000_POWER_ON()      {__gpio_set_pin(L3000_PWR_EN);}
#define		L3000_POWER_OFF()     {__gpio_clear_pin(L3000_PWR_EN);}

#define		L3000_FG_PWR_ON()       {__gpio_clear_pin(L3000_FG_PWR_CRL);}
#define		L3000_FG_PWR_OFF()      {__gpio_set_pin(L3000_FG_PWR_CRL);}


#define		L3000_PowerOff()      {while(1){           \
				       L3000_POWER_OFF();		\
				       }}

//#define		L3000_Reset()		(__gpio_clear_pin(L3000_RST))
#define		L3000_READ_PORT(a, port)   ((a) = __gpio_get_pin(port))
#define		LCDBL_OFF()           {__gpio_set_pin(L3000_LCD_BLIGHT);}
#define		LCDBL_ON()            {__gpio_clear_pin(L3000_LCD_BLIGHT);}
#endif
