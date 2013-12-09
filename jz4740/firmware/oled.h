
#ifndef      _OLED
#define	    _OLED
#include "jz4740.h"
#include	"gpio.h"
/*#define	LCD_CS  (GPC+16)
#define LCD_WR  (GPC+17)
#define LCD_A0  (GPC+18)
*/
/****************About OLED Configuration function****************/
#define	uint8	unsigned char
/*********************************************/
#define   OLED_START_PAGE  0xB0
#define   OLED_END_PAGE      0xB7
#define   OLED_LOW_COL       0x00
#define   OLED_HIGH_COL      0x10
/*********************PORT SETTING**************************/
//#define	OLED_PIN_RS       108//79
#define	OLED_PIN_WR       LCD_WR//80
#define	OLED_PIN_CS       LCD_CS//110//81
#define	OLED_PIN_A0       LCD_AO//76
#define	OLED_PIN_RES      (64+21) 
#define	OLED_PIN_DATA0    (64 + 8)
#define	OLED_PIN_DATA_NUM 8 //98~105  98-96=2
/***********************PORT OPERATE***************************/
#define	OLED_RST_0  __gpio_clear_pin(OLED_PIN_RES)
#define	OLED_RST_1  __gpio_set_pin(OLED_PIN_RES)
#define	OLED_RS_0   //__gpio_clear_pin(OLED_PIN_RS)
#define	OLED_RS_1   //__gpio_set_pin(OLED_PIN_RS)
#define	OLED_CS_0   __gpio_clear_pin(OLED_PIN_CS)
#define	OLED_CS_1   __gpio_set_pin(OLED_PIN_CS)
#define	OLED_RW_0   __gpio_clear_pin(OLED_PIN_WR)
#define	OLED_RW_1   __gpio_set_pin(OLED_PIN_WR) 	
#define	OLED_AO_0   __gpio_clear_pin(OLED_PIN_A0)
#define	OLED_AO_1   __gpio_set_pin(OLED_PIN_A0)
#define	OLED_PIN_INIT() {                   		 __gpio_as_output(OLED_PIN_WR);      \
							 __gpio_as_output(OLED_PIN_CS);      \
							 __gpio_as_output(OLED_PIN_RES);      \
							 __gpio_as_output(OLED_PIN_A0);      \
							 __gpio_as_output(OLED_PIN_DATA0 +0);      \
							 __gpio_as_output(OLED_PIN_DATA0 +1);      \
							 __gpio_as_output(OLED_PIN_DATA0 +2);      \
							 __gpio_as_output(OLED_PIN_DATA0 +3);      \
							 __gpio_as_output(OLED_PIN_DATA0 +4);      \
							 __gpio_as_output(OLED_PIN_DATA0 +5);      \
							 __gpio_as_output(OLED_PIN_DATA0 +6);      \
							 __gpio_as_output(OLED_PIN_DATA0 +7);      \
							 }
/*****************************************************************/
extern	int     gOLED_En;
extern	const   uint8 OLED_PIC01[];
extern	const   uint8 OLED_PIC02[];
extern	void    OLED_HardwareReset(void);/*software reset*/
extern	void    oled_Init(void);	
extern	void	OLED_Clear(void);
extern	void	OLED_ShowImg32(uint8	row,  uint8 col, uint8 *buf);
extern	void	OLED_ShowImg16_H(uint8	row,  uint8 col, uint8 *buf);
extern	void	OLED_ShowImg16_V(uint8	row,  uint8 col, uint8 *buf);
extern	void	OLED_DispPicture(uint8 *picture);
#endif
