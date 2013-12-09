/*
 * jz4740/drv/slcd/spfd5420a.c -- Ingenic On-Chip SLCD frame buffer device
 *
 * Copyright (C) 2005-2007, Ingenic Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#if SLCDTYPE == 3

#include <jz4740.h>
#include <slcdc.h>

#define PIN_CS_N 	(32*1+17)	/* Chip select | GPC22;*/
#define PIN_RESET_N 	(32*1+18)	/* LCD reset   | GPB18 */

unsigned int stat = 0;

/* Set the start address of screen, for example (0, 0) */
void Mcupanel_SetAddr(u32 x, u32 y) //u32
{
	Mcupanel_RegSet(0x200,x) ;
	udelay(1);
	Mcupanel_RegSet(0x201,y) ;
	udelay(1);
	Mcupanel_Command(0x202);

}

static void __slcd_special_pin_init(void)
{
	__gpio_as_output(PIN_CS_N);
	__gpio_as_output(PIN_RESET_N);
	__gpio_clear_pin(PIN_CS_N); /* Clear CS */	
//	__gpio_as_output(PIN_POWER_N);
	
	mdelay(100);
}
//#define  LCD_MODULE_G240400_REV 1
//Note: jzfb_slcd.bus = 16, jzfb_slcd.bpp = 16
static void __slcd_special_on(void)
{
	__gpio_set_pin(PIN_RESET_N);	
	mdelay(10);			
	__gpio_clear_pin(PIN_RESET_N);	
	mdelay(10);
	__gpio_set_pin(PIN_RESET_N);	
	mdelay(100);
	//************* Start Initial Sequence **********//
	Mcupanel_RegSet(0x00, 0x0000); //0001);	// Start internal OSC.
	Mcupanel_RegSet(0x01, 0x0000); //0x0100);	// set SS and SM bit
	Mcupanel_RegSet(0x02, 0x0700); //0400);	// set 1 line inversion
	Mcupanel_RegSet(0x03, 0x10b8); //0x1038); //0x1028); //0x1038);		// set GRAM write direction and BGR=1.
	Mcupanel_RegSet(0x04, 0x0000);		// Resize register
	Mcupanel_RegSet(0x08, 0x0207); //0202);	// set the back porch and front porch
	Mcupanel_RegSet(0x09, 0x0000);		// set non-display area refresh cycle ISC[3:0]
	Mcupanel_RegSet(0x0A, 0x0000);		// FMARK function
	Mcupanel_RegSet(0x0C, 0x0000);		// RGB interface setting
	Mcupanel_RegSet(0x0D, 0x0000);		// Frame marker Position
	Mcupanel_RegSet(0x0F, 0x0000);		// RGB interface polarity

	//*************Power On sequence ****************//    WCOM(0x51, 0x00, 0xEF);
	Mcupanel_RegSet(0x07, 0x0101);
	Mcupanel_RegSet(0x10, 0x10B0); //0000);     // SAP, BT[3:0], AP, DSTB, SLP, STB
	Mcupanel_RegSet(0x11, 0x0007);              // DC1[2:0], DC0[2:0], VC[2:0]
	mdelay(50);
	Mcupanel_RegSet(0x07, 0x0001);
	Mcupanel_RegSet(0x12, 0x013b); //0000);     // VREG1OUT voltage
	mdelay(50);
	Mcupanel_RegSet(0x13, 0x0500); //0000);     // VDV[4:0] for VCOM amplitude
	Mcupanel_RegSet(0x29, 0x0009); //B);
	mdelay(50);

	// ---------- Gamma Control  ---------- //
	Mcupanel_RegSet(0x30, 0x0102);
	Mcupanel_RegSet(0x31, 0x0C1B);
	Mcupanel_RegSet(0x32, 0x121F);
	Mcupanel_RegSet(0x33, 0x391A);
	Mcupanel_RegSet(0x34, 0x380B);
	Mcupanel_RegSet(0x35, 0x1004);
	Mcupanel_RegSet(0x36, 0x1701);
	Mcupanel_RegSet(0x37, 0x0A1E);
	Mcupanel_RegSet(0x38, 0x0007);
	Mcupanel_RegSet(0x39, 0x0101);
	Mcupanel_RegSet(0x3A, 0x0C06);
	Mcupanel_RegSet(0x3B, 0x0F03);
	Mcupanel_RegSet(0x3C, 0x000C);
	Mcupanel_RegSet(0x3D, 0x0D0C);
	Mcupanel_RegSet(0x3E, 0x0504);
	Mcupanel_RegSet(0x3F, 0x0601);	

	// ---------- Window Address Area  ---------- //
	Mcupanel_RegSet(0x50, 0x0000);		// Horizontal GRAM Start Address-----HSA[7:0]
	Mcupanel_RegSet(0x51, 0x00EF);		// Horizontal GRAM End Address-----HEA[7:0]
	Mcupanel_RegSet(0x52, 0x0000);		// Vertical GRAM Start Address-----VSA[8:0]
	Mcupanel_RegSet(0x53, 0x013F);		// Vertical GRAM Start Address-----VEA[8:0]
	
	Mcupanel_RegSet(0x60, 0x2700);	// GS, NL[5:0], SCN[5:0]
	Mcupanel_RegSet(0x61, 0x0001);		// NDL,VLE, REV
	Mcupanel_RegSet(0x6A, 0x0000);		// VL[8:0]

	// ---------- Partial Display Control  ---------- //
	Mcupanel_RegSet(0x80, 0x0000);		// Partial Image 1 Display Position-----PTDP0[8:0]
	Mcupanel_RegSet(0x81, 0x0000);		// Partial Image 1 Start Address-----PTSA0[8:0]
	Mcupanel_RegSet(0x82, 0x0000);		// Partial Image 1 End Address-----PTEA0[8:0]
	Mcupanel_RegSet(0x83, 0x0000);		// Partial Image 2 Display Position-----PTDP1[8:0]
	Mcupanel_RegSet(0x84, 0x0000);		// Partial Image 2 Start Address-----PTSA1[8:0]
	Mcupanel_RegSet(0x85, 0x0000);		// Partial Image 2 Start Address-----PTEA1[8:0]

	// ---------- Panel Interface Control  ---------- //
	Mcupanel_RegSet(0x90, 0x0013);	// Panel Interface Control 1-----DIVI[1:0], RTNI[4:0]
	Mcupanel_RegSet(0x92, 0x0000);		// Panel Interface Control 2-----NOWI[2:0]
	Mcupanel_RegSet(0x93, 0x0103);	// Panel Interface Control 3-----MCPI[2:0]
	Mcupanel_RegSet(0x95, 0x0110);		// Panel Interface Control 4-----DIVE[1:0], RTNE[5:0]
	Mcupanel_RegSet(0x97, 0x0000);		// Panel Interface Control 5-----NOWE[3:0]
	Mcupanel_RegSet(0x98, 0x0000);		// Panel Interface Control 6-----MCPE[2:0]
	
	Mcupanel_RegSet(0xF0, 0x5408);
	Mcupanel_RegSet(0xF3, 0x0010);
	Mcupanel_RegSet(0xF4, 0x001F);
	Mcupanel_RegSet(0xF0, 0x0000);

	Mcupanel_RegSet(0x07, 0x0001);	// Display Control 1
	mdelay(50);
	Mcupanel_RegSet(0x07, 0x0021);	// Display Control 1
	Mcupanel_RegSet(0x07, 0x0023);	// Display Control 1

	mdelay(50);
	Mcupanel_RegSet(0x07, 0x0173);		// Display Control 1-----262K color and display ON
	mdelay(50);
	
	Mcupanel_Command(0x22);
	return;
}

void __slcd_special_off(void)
{
//	Mcupanel_RegSet();
}

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

#define GPIO_PWM    123		/* GP_D27 */	//backlight, GPD27
#define PWM_CHN 4    /* pwm channel */
#define PWM_FULL 101
/* 100 level: 0,1,...,100 */

/*
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
*/

void  __slcd_set_backlight_level(n) {
__gpio_as_output(32*3+27);
__gpio_set_pin(32*3+27);
}

void __slcd_close_backlight()
{
	__gpio_as_output(GPIO_PWM);
	__gpio_clear_pin(GPIO_PWM);
}

void __slcd_display_pin_init(void)
{
	__slcd_special_pin_init();
}

//debug here
void __lcd_display_on(){
//	__gpio_clear_pin(PIN_POWER_N);
	__slcd_special_on();
}

void __lcd_display_off() {
	__slcd_special_off();
	__slcd_close_backlight();
//	__gpio_set_pin(PIN_POWER_N);
}

void  lcd_set_backlight(int level)
{
	__slcd_set_backlight_level(level); 
}

void lcd_close_backlight()
{
	__slcd_close_backlight();
}

void slcd_board_init(void)
{
	__slcd_display_pin_init();
	__lcd_display_on();
	__slcd_set_backlight_level(80);
}

#endif /* SPFD5420A */



