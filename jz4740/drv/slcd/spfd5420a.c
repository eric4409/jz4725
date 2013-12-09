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

#if SLCDTYPE == 2

#include <jz4740.h>
#include <slcdc.h>

//#define PIN_CS_N 	(32*2+18)	/* Chip select | GPC18 */
#define PIN_CS_N 	(32*2+22)	/* Chip select | GPC22;*/
#define PIN_RESET_N 	(32*1+18)	/* LCD reset   | GPB18 */
#define PIN_RS_N 	(32*2+19)	/* LCD RS      | GPC19 */
//#define PIN_POWER_N	(32*3+0)	/* Power off   | GPD0  */
//#define PIN_FMARK_N	(32*3+1)	/* fmark       | GPD1  */

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
#if 0
static void detect_fmark_status(void *arg)
{

	unsigned int gpio_state = 0;
	unsigned int count = 0;
	static unsigned int i;
	u8 byte1 , byte2 ,err;
	
	enum {
			FMARK_RISE,
			FMARK_FALL,
		}stat;
	
	gpio_state = __gpio_get_pin(PIN_FMARK_N);
	
	while(1)
	{
		udelay(1);
		if(gpio_state == __gpio_get_pin(PIN_FMARK_N))
		{
			count++;
			if(count > 2000)
				break;
		}else
		{
			count = 0;
			gpio_state = __gpio_get_pin(PIN_FMARK_N);
		}
		
	}
	
	if (gpio_state == 0)
	{
		__gpio_as_irq_rise_edge(PIN_FMARK_N);
		stat = FMARK_RISE; // here is rise edge
	}
	else if (gpio_state == 1)
	{
		__gpio_as_irq_fall_edge(PIN_FMARK_N);	// here is fall edge
		stat = FMARK_FALL;
	}	
}
#endif
int GAMMA()
{
    
	Mcupanel_RegSet(0x0300,0x0101);
	Mcupanel_RegSet(0x0301,0x0b27);
	Mcupanel_RegSet(0x0302,0x132a);
	Mcupanel_RegSet(0x0303,0x2a13);
	Mcupanel_RegSet(0x0304,0x270b);
	Mcupanel_RegSet(0x0305,0x0101);
	Mcupanel_RegSet(0x0306,0x1205);
	Mcupanel_RegSet(0x0307,0x0512);
	Mcupanel_RegSet(0x0308,0x0005);
	Mcupanel_RegSet(0x0309,0x0003);
	Mcupanel_RegSet(0x030a,0x0f04);
	Mcupanel_RegSet(0x030b,0x0f00);
	Mcupanel_RegSet(0x030c,0x000f);
	Mcupanel_RegSet(0x030d,0x040f);
	Mcupanel_RegSet(0x030e,0x0300);
	Mcupanel_RegSet(0x030f,0x0500);
	//**************secorrect gamma2 *********
	Mcupanel_RegSet(0x0400,0x3500);
	Mcupanel_RegSet(0x0401,0x0001);
	Mcupanel_RegSet(0x0404,0x0000);
	
	Mcupanel_RegSet(0x0500,0x0000);
	Mcupanel_RegSet(0x0501,0x0000);
	Mcupanel_RegSet(0x0502,0x0000);
	Mcupanel_RegSet(0x0503,0x0000);      //
	Mcupanel_RegSet(0x0504,0x0000);      //
	Mcupanel_RegSet(0x0505,0x0000);

	Mcupanel_RegSet(0x0600,0x0000);
	Mcupanel_RegSet(0x0606,0x0000);
	Mcupanel_RegSet(0x06f0,0x0000);
	
	Mcupanel_RegSet(0x07f0,0x5420);      //
	Mcupanel_RegSet(0x07f3,0x288a);      //
	Mcupanel_RegSet(0x07f4,0x0022);      //
	Mcupanel_RegSet(0x07f5,0x0001);      //
	Mcupanel_RegSet(0x07f0,0x0000);      //

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
	if (jzfb_slcd.bus == 18) {
		Mcupanel_RegSet(0x0606,0x0000);
		udelay(10);
		Mcupanel_RegSet(0x0007,0x0001);
		udelay(10);
		Mcupanel_RegSet(0x0110,0x0001);
		udelay(10);
		Mcupanel_RegSet(0x0100,0x17b0);
		Mcupanel_RegSet(0x0101,0x0147);
		Mcupanel_RegSet(0x0102,0x019d);
		Mcupanel_RegSet(0x0103,0x8600);
		Mcupanel_RegSet(0x0281,0x0010);
		udelay(10);
		Mcupanel_RegSet(0x0102,0x01bd);
		udelay(10);
		
		
		//************initia************
		Mcupanel_RegSet(0x0000,0x0000);
		Mcupanel_RegSet(0x0001,0x0000);//0100
		Mcupanel_RegSet(0x0002,0x0400);//0100
		/*up:0x1288 down:0x12B8 left:0x1290 right:0x12A0*/
		Mcupanel_RegSet(0x0003,0x1288);
		Mcupanel_RegSet(0x0006,0x0000);
		Mcupanel_RegSet(0x0008,0x0503);
		Mcupanel_RegSet(0x0009,0x0001);//0001
		Mcupanel_RegSet(0x000b,0x0010);
		Mcupanel_RegSet(0x000c,0x0000);
		Mcupanel_RegSet(0x000f,0x0000);
		Mcupanel_RegSet(0x0007,0x0001);
		Mcupanel_RegSet(0x0010,0x0010);/////////
		Mcupanel_RegSet(0x0011,0x0202);
		Mcupanel_RegSet(0x0012,0x0300);
		Mcupanel_RegSet(0x0020,0x021e);
		Mcupanel_RegSet(0x0021,0x0202);
		Mcupanel_RegSet(0x0022,0x0100);
		Mcupanel_RegSet(0x0090,0x0000);
		Mcupanel_RegSet(0x0092,0x0000);
		Mcupanel_RegSet(0x0100,0x16b0);
		Mcupanel_RegSet(0x0101,0x0147);
		Mcupanel_RegSet(0x0102,0x01bd);
		//Mcupanel_RegSet(0x0103,0x3600);
		Mcupanel_RegSet(0x0103,0x2c00);//////////////
		
		Mcupanel_RegSet(0x0107,0x0000);
		Mcupanel_RegSet(0x0110,0x0001);
		
		Mcupanel_RegSet(0x0210,0x0000);
		Mcupanel_RegSet(0x0211,0x00ef);
		Mcupanel_RegSet(0x0212,0x0000);
		Mcupanel_RegSet(0x0213,0x018f);
		Mcupanel_RegSet(0x0280,0x0000);
		Mcupanel_RegSet(0x0281,0x0001);////////////////
		
		Mcupanel_RegSet(0x0282,0x0000);
		GAMMA();
		
		Mcupanel_RegSet(0x0007,0x0173);
		
	}
	else {
		Mcupanel_RegSet(0x0600, 0x0001);   //soft reset    
		mdelay(10);  
		Mcupanel_RegSet(0x0600, 0x0000);   //soft reset   
		mdelay(10);  
		Mcupanel_RegSet(0x0606, 0x0000);   //i80-i/F Endian Control
		
		//===User setting===    
#ifdef LCD_MODULE_G240400_REV
		Mcupanel_RegSet(0x0001, 0x0100);   //Driver Output Control-------------0x0100 SM(bit10) | 0x400
#else
		Mcupanel_RegSet(0x0001, 0x0000);//0x0100);   //Driver Output Control-------------0x0100 SM(bit10) | 0x400
#endif
		Mcupanel_RegSet(0x0002, 0x0100);   //LCD Driving Wave Control      0x0100 
#ifdef LCD_MODULE_G240400_REV
		if (jzfb_slcd.bpp == 16)
			Mcupanel_RegSet(0x0003, 0x50A8);//0x1020 | 0x8);   //Entry Mode 0x1030
		else /*bpp = 18*/
			Mcupanel_RegSet(0x0003, 0xD0A8);//0x1020 | 0x8);   //Entry Mode 0x1030
#else
		if (jzfb_slcd.bpp == 16)
			Mcupanel_RegSet(0x0003, 0x50A8);//0x1010 | 0x8);   //Entry Mode 0x1030
		else /*bpp = 18*/
			Mcupanel_RegSet(0x0003, 0x1010 | 0xC8);   //Entry Mode 0x1030
#endif
		Mcupanel_RegSet(0x0006, 0x0000);   //Outline Sharpening Control     
		Mcupanel_RegSet(0x0008, 0x0808);   //Sets the number of lines for front/back porch period       
		Mcupanel_RegSet(0x0009, 0x0001);   //Display Control 3    
		Mcupanel_RegSet(0x000B, 0x0010);   //Low Power Control
		Mcupanel_RegSet(0x000C, 0x0000);   //External Display Interface Control 1 //0x0001
		Mcupanel_RegSet(0x000F, 0x0000);   //External Display Interface Control 2         
#ifdef LCD_MODULE_G240400_REV
		Mcupanel_RegSet(0x0400, 0x3100);   //Base Image Number of Line--------------GS(bit15) | 0x8000
#else
		Mcupanel_RegSet(0x0400, 0xB104);//0x3100);   //Base Image Number of Line--------------GS(bit15) | 0x8000
#endif
		Mcupanel_RegSet(0x0401, 0x0001);   //Base Image Display        0x0001
		Mcupanel_RegSet(0x0404, 0x0000);   //Base Image Vertical Scroll Control    0x0000
		
		Mcupanel_RegSet(0x0500, 0x0000);   //Partial Image 1: Display Position
		Mcupanel_RegSet(0x0501, 0x0000);   //RAM Address (Start Line Address) 
//#ifdef LCD_MODULE_G240400_REV
		Mcupanel_RegSet(0x0502, 0x0000);   //RAM Address (End Line Address)  
//#else
//		Mcupanel_RegSet(0x0502, 0x018f);   //RAM Address (End Line Address)  
//#endif
		
		Mcupanel_RegSet(0x0503, 0x0000);   //Partial Image 2: Display Position  RAM Address
		Mcupanel_RegSet(0x0504, 0x0000);   //RAM Address (Start Line Address) 
		Mcupanel_RegSet(0x0505, 0x0000);   //RAM Address (End Line Address)
		
		//Panel interface control===
		Mcupanel_RegSet(0x0010, 0x0011);   //Division Ratio,Clocks per Line  14  
		mdelay(10); 
		Mcupanel_RegSet(0x0011, 0x0202);   //Division Ratio,Clocks per Line
		Mcupanel_RegSet(0x0012, 0x0300);   //Sets low power VCOM drive period.   
		mdelay(10); 
		Mcupanel_RegSet(0x0020, 0x021e);   //Panel Interface Control 4  
		Mcupanel_RegSet(0x0021, 0x0202);   //Panel Interface Control 5 
		Mcupanel_RegSet(0x0022, 0x0100);   //Panel Interface Control 6 
		Mcupanel_RegSet(0x0090, 0x0000);   //Frame Marker Control  
		Mcupanel_RegSet(0x0092, 0x0000);   //MDDI Sub-display Control  
		
		//===Gamma setting===    
		Mcupanel_RegSet(0x0300, 0x0101);   //γ Control
		Mcupanel_RegSet(0x0301, 0x0000);   //γ Control
		Mcupanel_RegSet(0x0302, 0x0016);   //γ Control
		Mcupanel_RegSet(0x0303, 0x2913);   //γ Control
		Mcupanel_RegSet(0x0304, 0x260B);   //γ Control
		Mcupanel_RegSet(0x0305, 0x0101);   //γ Control
		Mcupanel_RegSet(0x0306, 0x1204);   //γ Control
		Mcupanel_RegSet(0x0307, 0x0415);   //γ Control
		Mcupanel_RegSet(0x0308, 0x0205);   //γ Control
		Mcupanel_RegSet(0x0309, 0x0303);   //γ Control
		Mcupanel_RegSet(0x030a, 0x0E05);   //γ Control
		Mcupanel_RegSet(0x030b, 0x0D01);   //γ Control
		Mcupanel_RegSet(0x030c, 0x010D);   //γ Control
		Mcupanel_RegSet(0x030d, 0x050E);   //γ Control
		Mcupanel_RegSet(0x030e, 0x0303);   //γ Control
		Mcupanel_RegSet(0x030f, 0x0502);   //γ Control
		
		//===Power on sequence===
		Mcupanel_RegSet(0x0007, 0x0001);   //Display Control 1
		Mcupanel_RegSet(0x0110, 0x0001);   //Power supply startup enable bit
		Mcupanel_RegSet(0x0112, 0x0060);   //Power Control 7
		Mcupanel_RegSet(0x0100, 0x16B0);   //Power Control 1 
		Mcupanel_RegSet(0x0101, 0x0115);   //Power Control 2
		Mcupanel_RegSet(0x0102, 0x0119);   //Starts VLOUT3,Sets the VREG1OUT.
		mdelay(50); 
		Mcupanel_RegSet(0x0103, 0x2E00);   //set the amplitude of VCOM
		mdelay(50);
		Mcupanel_RegSet(0x0282, 0x0093);//0x008E);//0x0093);   //VCOMH voltage
		Mcupanel_RegSet(0x0281, 0x000A);   //Selects the factor of VREG1OUT to generate VCOMH. 
		Mcupanel_RegSet(0x0102, 0x01BE);   //Starts VLOUT3,Sets the VREG1OUT.
		mdelay(10);
		
		//Address 
		Mcupanel_RegSet(0x0210, 0x0000);   //Window Horizontal RAM Address Start
		Mcupanel_RegSet(0x0211, 0x00ef);   //Window Horizontal RAM Address End
		Mcupanel_RegSet(0x0212, 0x0000);   //Window Vertical RAM Address Start
		Mcupanel_RegSet(0x0213, 0x018f);   //Window Vertical RAM Address End 
	
		Mcupanel_RegSet(0x0200, 0x0000);   //RAM Address Set (Horizontal Address)
//#ifdef LCD_MODULE_G240400_REV
		Mcupanel_RegSet(0x0201, 0x0000);   //RAM Address Set (Vertical Address)
//#else
//		Mcupanel_RegSet(0x0201, 0x018f);   //RAM Address Set (Vertical Address)
//#endif
		//===Display_On_Function===
		Mcupanel_RegSet(0x0007, 0x0021);   //Display Control 1 
		mdelay(50);   //40
		Mcupanel_RegSet(0x0007, 0x0061);   //Display Control 1 
		mdelay(50);   //100
		Mcupanel_RegSet(0x0007, 0x0173);   //Display Control 1 
		mdelay(50);   //300
	}
	Mcupanel_Command(0x0202);                  //Write Data to GRAM	  
	udelay(10);
	Mcupanel_SetAddr(0,0);
	udelay(100);
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



