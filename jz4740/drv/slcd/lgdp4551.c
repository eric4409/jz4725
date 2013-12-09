/*
 * jz4740/drv/slcd/lgdp4554.c -- Ingenic On-Chip SLCD frame buffer device
 *
 * Copyright (C) 2005-2007, Ingenic Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#if SLCDTYPE == 1

#include <jz4740.h>
#include <slcdc.h>

#define PIN_CS_N 	(32*2+18)	/* Chip select      :SLCD_WR: GPC18 */ 
#define PIN_RESET_N 	(32*2+21)	/* LCD reset        :SLCD_RST: GPC21*/ 
#define PIN_RS_N 	(32*2+19)

/* Set the start address of screen, for example (0, 0) */
void Mcupanel_SetAddr(u16 x, u16 y)
{
	Mcupanel_RegSet(0x20,x) ;
	udelay(1);
	Mcupanel_RegSet(0x21,y) ;
	udelay(1);
	Mcupanel_Command(0x22);	

}
#if 0
#define	__slcd_special_pin_init() \
do { \
	__gpio_as_output(PIN_CS_N); 	\
	__gpio_as_output(PIN_RESET_N); 	\
	__gpio_clear_pin(PIN_CS_N); /* Clear CS */\
	mdelay(100);			\
} while(0)

#define __slcd_special_on() 		\
do {	/* RESET# */			\
	__gpio_set_pin(PIN_RESET_N);	\
	mdelay(10);			\
	__gpio_clear_pin(PIN_RESET_N);	\
	mdelay(10);			\
	__gpio_set_pin(PIN_RESET_N);	\
	mdelay(100);			\
	Mcupanel_RegSet(0x0015,0x0050);	\
	Mcupanel_RegSet(0x0011,0x0000);	\
	Mcupanel_RegSet(0x0010,0x3628);	\
	Mcupanel_RegSet(0x0012,0x0002);	\
	Mcupanel_RegSet(0x0013,0x0E47);	\
	udelay(100);			\
	Mcupanel_RegSet(0x0012,0x0012);	\
	udelay(100);			\
	Mcupanel_RegSet(0x0010,0x3620);	\
	Mcupanel_RegSet(0x0013,0x2E47);	\
	udelay(50);			\
	Mcupanel_RegSet(0x0030,0x0000);	\
	Mcupanel_RegSet(0x0031,0x0502);	\
	Mcupanel_RegSet(0x0032,0x0307);	\
	Mcupanel_RegSet(0x0033,0x0304);	\
	Mcupanel_RegSet(0x0034,0x0004);	\
	Mcupanel_RegSet(0x0035,0x0401);	\
	Mcupanel_RegSet(0x0036,0x0707);	\
	Mcupanel_RegSet(0x0037,0x0303);	\
	Mcupanel_RegSet(0x0038,0x1E02);	\
	Mcupanel_RegSet(0x0039,0x1E02);	\
	Mcupanel_RegSet(0x0001,0x0000);	\
	Mcupanel_RegSet(0x0002,0x0300);	\
	if (jzfb_slcd.bpp == 16)		\
		Mcupanel_RegSet(0x0003,0x10B8); /*8-bit system interface two transfers
						  up:0x10B8 down:0x1088 left:0x1090 right:0x10a0*/\
	else				\
		if (jzfb_slcd.bpp == 32)	\
			Mcupanel_RegSet(0x0003,0xD0B8);/*8-bit system interface three transfers,666
							 up:0xD0B8 down:0xD088 left:0xD090 right:0xD0A0*/\
	Mcupanel_RegSet(0x0008,0x0204);\
	Mcupanel_RegSet(0x000A,0x0008);\
	Mcupanel_RegSet(0x0060,0x3100);\
	Mcupanel_RegSet(0x0061,0x0001);\
	Mcupanel_RegSet(0x0090,0x0052);\
	Mcupanel_RegSet(0x0092,0x000F);\
	Mcupanel_RegSet(0x0093,0x0001);\
	Mcupanel_RegSet(0x009A,0x0008);\
	Mcupanel_RegSet(0x00A3,0x0010);\
	Mcupanel_RegSet(0x0050,0x0000);\
	Mcupanel_RegSet(0x0051,0x00EF);\
	Mcupanel_RegSet(0x0052,0x0000);\
	Mcupanel_RegSet(0x0053,0x018F);\
	/*===Display_On_Function=== */ \
	Mcupanel_RegSet(0x0007,0x0001);\
	Mcupanel_RegSet(0x0007,0x0021);\
	Mcupanel_RegSet(0x0007,0x0023);\
	Mcupanel_RegSet(0x0007,0x0033);\
	Mcupanel_RegSet(0x0007,0x0133);\
	Mcupanel_Command(0x0022);/*Write Data to GRAM	*/  \
	udelay(1);		\
	Mcupanel_SetAddr(0,0);	\
	mdelay(100);		\
} while (0)

#define __slcd_special_off() 		\
do { \
} while(0)
#endif
#if 1
static void __slcd_special_pin_init(void)
{
	__gpio_as_output(PIN_CS_N);
	__gpio_as_output(PIN_RESET_N);
	__gpio_clear_pin(PIN_CS_N); /* Clear CS */
	mdelay(100);
}

static void __slcd_special_on(void)
{	/* RESET# */			
	__gpio_set_pin(PIN_RESET_N);	
	mdelay(10);			
	__gpio_clear_pin(PIN_RESET_N);	
	mdelay(10);			
	__gpio_set_pin(PIN_RESET_N);	
	mdelay(100);			
	Mcupanel_RegSet(0x0015,0x0050);	
	Mcupanel_RegSet(0x0011,0x0000);	
	Mcupanel_RegSet(0x0010,0x3628);	
	Mcupanel_RegSet(0x0012,0x0002);	
	Mcupanel_RegSet(0x0013,0x0E47);
	udelay(100);			
	Mcupanel_RegSet(0x0012,0x0012);	
	udelay(100);			
	Mcupanel_RegSet(0x0010,0x3620);
	Mcupanel_RegSet(0x0013,0x2E47);	
	udelay(50);			
	Mcupanel_RegSet(0x0030,0x0000);	
	Mcupanel_RegSet(0x0031,0x0502);	
	Mcupanel_RegSet(0x0032,0x0307);	
	Mcupanel_RegSet(0x0033,0x0304);	
	Mcupanel_RegSet(0x0034,0x0004);	
	Mcupanel_RegSet(0x0035,0x0401);	
	Mcupanel_RegSet(0x0036,0x0707);	
	Mcupanel_RegSet(0x0037,0x0303);	
	Mcupanel_RegSet(0x0038,0x1E02);	
	Mcupanel_RegSet(0x0039,0x1E02);	
	Mcupanel_RegSet(0x0001,0x0000);
	Mcupanel_RegSet(0x0002,0x0300);
	if (jzfb_slcd.bpp == 16)	
		Mcupanel_RegSet(0x0003,0x10B8); /*8-bit system interface two transfers*/\
	else				
		if (jzfb_slcd.bpp == 32)	
			Mcupanel_RegSet(0x0003,0xD0B8);/*8-bit system interface three transfers,666*/
	Mcupanel_RegSet(0x0008,0x0204);
	Mcupanel_RegSet(0x000A,0x0008);
	Mcupanel_RegSet(0x0060,0x3100);
	Mcupanel_RegSet(0x0061,0x0001);
	Mcupanel_RegSet(0x0090,0x0052);
	Mcupanel_RegSet(0x0092,0x000F);
	Mcupanel_RegSet(0x0093,0x0001);
	Mcupanel_RegSet(0x009A,0x0008);
	Mcupanel_RegSet(0x00A3,0x0010);
	Mcupanel_RegSet(0x0050,0x0000);
	Mcupanel_RegSet(0x0051,0x00EF);
	Mcupanel_RegSet(0x0052,0x0000);
	Mcupanel_RegSet(0x0053,0x018F);
	/*===Display_On_Function=== */ 
	Mcupanel_RegSet(0x0007,0x0001);
	Mcupanel_RegSet(0x0007,0x0021);
	Mcupanel_RegSet(0x0007,0x0023);
	Mcupanel_RegSet(0x0007,0x0033);
	Mcupanel_RegSet(0x0007,0x0133);
	Mcupanel_Command(0x0022);/*Write Data to GRAM	*/  
	udelay(1);	       
	Mcupanel_SetAddr(0,0); 
	mdelay(100);		
}
void __slcd_special_off(void)
{;
}
#endif
/*
 * Platform specific definition
 */

#define __slcd_dma_enable() (REG_SLCD_CTRL |= SLCD_CTRL_DMA_EN)
#if 0
#define __slcd_dma_disable() \
do {\
	while (REG_SLCD_STATE & SLCD_STATE_BUSY); 	\
	REG_SLCD_CTRL &= ~SLCD_CTRL_DMA_EN;		\
} while(0)
#endif
void __slcd_dma_disable(void)
{
	while (REG_SLCD_STATE & SLCD_STATE_BUSY);
	REG_SLCD_CTRL &= ~SLCD_CTRL_DMA_EN;
}
#define GPIO_PWM    123		/* GP_D27 */
#define PWM_CHN 4    /* pwm channel */
#define PWM_FULL 101
/* 100 level: 0,1,...,100 */
#if 0
void __slcd_set_backlight_level(int n)
{
	__gpio_as_pwm(4);
        __tcu_disable_pwm_output(PWM_CHN);
        __tcu_stop_counter(PWM_CHN);
        __tcu_init_pwm_output_high(PWM_CHN);
        __tcu_set_pwm_output_shutdown_abrupt(PWM_CHN);
        __tcu_select_clk_div1(PWM_CHN);
        __tcu_mask_full_match_irq(PWM_CHN);
        __tcu_mask_half_match_irq(PWM_CHN);
        __tcu_set_count(PWM_CHN,0);
        __tcu_set_full_data(PWM_CHN,__cpm_get_extalclk()/1000);
        __tcu_set_half_data(PWM_CHN,__cpm_get_extalclk()/1000*n/100);
        __tcu_enable_pwm_output(PWM_CHN);
        __tcu_select_extalclk(PWM_CHN);
        __tcu_start_counter(PWM_CHN);
}
#endif

void __slcd_set_backlight_level(int n)
{
	__gpio_as_output(32*3+27);
	__gpio_set_pin(32*3+27); 
}
void  slcd_set_backlight(void)
{
	__slcd_set_backlight_level(80); 
}

void __slcd_close_backlight(void)
{
	__gpio_as_output(GPIO_PWM);
	__gpio_clear_pin(GPIO_PWM);
}

void __slcd_display_pin_init(void)
{
	__slcd_special_pin_init();
}

void __slcd_display_on() {
	__slcd_special_on();
}

void __slcd_display_off(){
	__slcd_special_off(); 
	__slcd_close_backlight();
}

void slcd_board_init(void)
{
	__slcd_display_pin_init();
	__slcd_special_on();
	__slcd_set_backlight_level(80); 
}

void  lcd_set_backlight(int level)
{
	__slcd_set_backlight_level(level); 
}

void lcd_close_backlight()
{
	__slcd_close_backlight();
}


#endif /* LGDP4551 */


