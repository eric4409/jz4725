#ifndef __KEY_H__
#define __KEY_H__
#include <gpio.h>

#define KEY_PIN	(GPC*32+18)	//key input pin from GPC18 TO GPC21

int GetKey(void);
void InitKeyBuffer(void);
void KeyInit(int DeviceID);
char GetKeyPin(void);

#if MACHINE_ID == MID_K18
#define DATAPIN (GPC*32+18)
#define CLKPIN  (GPC*32+19)
#define KLED    (GPC*32+21)
#define KEYPOW  (GPD*32+8)

#define KEYINPUT0 (GPC*32+20)
#define KEYINPUT1 (GPC*32+21)
#define PINMASK0  (0x100000)
#define PINMASK1  (0x200000)  
#else
/* for keypad with 74hc164 */
#define DATAPIN (GPD*32+10)
#define CLKPIN  (GPD*32+9)
#define KLED	(GPD*32+28)
#define KEYPOW  (GPD*32+8)

#define KEYINPUT0 (GPC*32+18)
#define KEYINPUT1 (GPC*32+19)
#define PINMASK0  (0x40000)
#define PINMASK1  (0x80000)	
#endif

/* end */

#endif //__KEY_H__
