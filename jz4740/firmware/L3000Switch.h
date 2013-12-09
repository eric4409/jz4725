#ifndef         __SWITCH_
#define		__SWITCH_
#include	"L3000Drv.h"
#include	"jz4740.h"
#include	"ccc.h"
#include	"app.h"
#include	"options.h"
#include	"L3000Operate.h"

//#define		L3000_PWR_LOW
//#define         L3000_PWR_LOW_ALM   33
//#define         L3000_PWR_LOW_OFF   17
//#define         L3000_BAK_PWR       91

#define	SWI_CNT_UPLIMIT   (400/RUN_TICK)
#define SWITCH_MAX        5 
#define SW_LOW_PWR    	  1
#define SW_LOW_PWR_ALM    2
#define SW_LOW_PWR_OFF    3
#define SW_BAK_PWR     	  0
#define	SW_SENSOR         4

//#define       L3000_READ_PORT(a, port)   ((a) = __gpio_get_pin(port))

extern	const	U8 DefSwPort[SWITCH_MAX];
extern		U8 L3000SwGroup[SWITCH_MAX];
extern	int	L3000CheckSwitch(U8 * sw);

#endif
