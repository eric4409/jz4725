#ifndef __LCDM_H__
#define __LCDM_H__

#include <asm/jzsoc.h>

#define GPC	2
#define LCD_EN	(32*GPC + 17)
#define LCD_RS	(32*GPC + 16)

/* IOCTL commands */
#define  LCD_DISPLAY_ON          0x01
#define  LCD_DISPLAY_OFF         0x02
#define  LCD_RESET               0x03
#define  LCD_DISPLAY_LINE_START  0x04
#define  LCD_WRITE_DATA          0x05
#define  LCD_READ_DATA           0x06
#define  LCD_SET_PAGE            0x07
#define  LCD_SET_ADDRESS         0x08

/*******COB*********/
#define _12864_TIMEOUT               	0x1000
#define  CMD_SET_PAGE            	0xb8
#define  CMD_SET_ADDRESS         	0x40
#define  CMD_DISPLAY_ON          	0x3f
#define  CMD_DISPLAY_OFF         	0x3e
#define  CMD_DISPLAY_LINE_START  	0xc0
#define  STATUS_BUSY                	0x80
#define  STATUS_DISPLAY_ON          	0x20
#define  STATUS_RESET               	0x10


/******* COG*********/
#define  CMD_COG_SET_PAGE            	0xb0
#define  CMD_COG_SET_ADDRESS_MSB     	0x10
#define  CMD_COG_SET_ADDRESS_LSB     	0x00
#define  CMD_COG_DISPLAY_ON          	0xaf
#define  CMD_COG_DISPLAY_OFF         	0xae
#define  CMD_COG_DISPLAY_LINE_START  	0x40
#define  CMD_COG_RESET                  0xe2

#define  LCD_WIDTH               	128
#define  LCD_HEIGHT              	64

/******OLED*******/
/* GPIO define */
#define OLED_RD		79
#define OLED_WR		80
#define OLED_A0		81
#define OLED_RESET	82
#define OLED_CS		83	

#define PAGE		0xb0
#define LOW_COL		0x00
#define HIGH_COL	0x10

int lcd_ioctl(unsigned int cmd, unsigned int *arg);

#endif

