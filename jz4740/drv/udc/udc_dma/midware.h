#ifndef _MIDWARE_H_
#define _MIDWARE_H_

#include <jz4740.h>
#define MAX_MID_SRC 10
#define MAX_MID_OBJ 1

#define EVENT_KEY_UP        0x1
#define EVENT_USB_IN        0x2
#define EVENT_USB_OUT       0x4
#define EVENT_POWER_IN      0x8
#define EVENT_POWER_OUT     0x10
#define EVENT_MMC_IN        0x20
#define EVENT_MMC_OUT       0x40
#define EVENT_POWER_OFF     0x80
#define EVENT_CHARG_ON      0x100
#define EVENT_CHARG_OFF     0x200
#define EVENT_LCD_ON        0x400
#define EVENT_LCD_OFF       0x800
#define EVENT_UNINSTALL     0x1000
#define EVENT_AUTOOFF       0x2000
#define EVENT_BAT_LOW       0x4000
#define EVENT_BAT_FULL      0x8000
#define EVENT_SET_ALM       0x10000
#define EVENT_SET_TIMER			0x12000


#define SRC_KEY        1
#define SRC_UDC        2
#define SRC_MMC        3
#define SRC_POWER_OFF  4
#define SRC_CHARG_DETE 5
#define SRC_POWER_LOW  6
#define SRC_SET_ALM    7
#define TCU_SET_TIMER  8
#define SRC_KEY_DOWN   9

#define MIDWARE_SELF   ( MAX_MID_SRC + 1 )

#define BAT_CHARGING       1
#define BAT_NOT_CHARGING   2
#define BAT_FULL           3
#define BAT_VERY_LOW       4
#define BAT_LOW_1          5

//#define USE_AD_CHARGE    1
#define BAT_FULL_VALUE     4150

#define MIN_BAT_VAL      1830
#define MAX_BAT_VAL      2280  

#define BAT_CAPACITY    ( MAX_BAT_VAL - MIN_BAT_VAL )
#define FULL_BAT_TIME   ( 2 * 60 * 60 )
#define TIME_PERCENT    4

typedef struct
{
	u32 Obj;
	u32 Val;
}MIDSRCDTA;

typedef struct
{
	volatile u32 ID;
	u8 *Name;
	volatile u32 Src;
	volatile u32 Event;
	void (*GetRequest) (MIDSRCDTA *);
	void (*Response) (MIDSRCDTA *);
	OS_EVENT *CurEvent, *CurEvent1, *CurEvent2;

} MIDSRC,* PMIDSRC;

typedef struct
{
	u32 ID;
	u8 *Name;
	u32 (*Notify) (u32);
	void (*GetState) (u32 *);
	void (*SetLcdBLTime) (u32);
	void (*SetAutoOffTime)(u32 );

} MIDOBJ,* PMIDOBJ;

unsigned int RegisterMidSrc(PMIDSRC );

unsigned int RegisterMidObj(PMIDOBJ );

u32 MidWareInit( PMIDOBJ pobj );

#endif
