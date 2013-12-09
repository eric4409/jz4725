#ifndef	_KEYSCAN
#define	_KEYSCAN
//#include	"uboot-jz/jz47xx.h"
#include	"ccc.h"
#include	"app.h"
#include	"options.h"
#include	"L3000Operate.h"

//#define	PIN_KEY_DEL    (32 + 17) 
#define	L3000_KEY_MAX              5

#define	L3000_KEY_INIT()  __gpio_as_input(PIN_KEY_DEL)

#define	GET_KEY_INPUTPIN(num)  __gpio_get_pin(num)  

extern	void	L3000KeyInit(void);
extern	int 	L3000KeyCheck(char *k);	

#endif




