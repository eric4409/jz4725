/*
 *  linux/drivers/char/jzchar/lcm.c
 *
 *  LCD Module Driver.
 *  a dot-matrix graphic LCD module, it includes row driver/line driver 128x64 pixels LCD.
 *  It can display graphic picture and English text, also can show 8x4 (16x16 pixels)Chinese words.
 *  Author: lqs@zksoftware.com
 *  Copyright 2004,Sinovee Microsystems.
 */
#include "dlcm.h"
#include "jz4740.h"
#include "gpio.h"

#undef  LCD_DEBUG

#define  t_page			 7
#define  t_address		 32
#define  cob			 1
#define  cog			 2
#define  oled 			 5

static int lcdstyle = cog;
static int lcdheight=64;
static int lcdconvert=0;
int getLCDType(void)
{
	__gpio_as_input(GPB*32+18);
	return !(__gpio_get_pin(GPB*32+18));
}

/*====================== COG support start ==================*/
static void LCDWRITE(unsigned char data)
{
        unsigned int d;

        d = data;
	d =(d<<8)&0xff00;
        __gpio_set_port(GPC) = d;
	__gpio_clear_port(GPC) = ~d&0xff00;
        __gpio_set_pin(LCD_EN);	
//	udelay(5);
        __gpio_clear_pin(LCD_EN);
}

int lcm_write_cmd(unsigned char cmd)
{
        __gpio_clear_pin(LCD_RS);
        LCDWRITE(cmd);
//        printf("OLED Write cmd = %x\n",cmd);
	return 0;
}

int lcm_write_data(unsigned char data)
{
        __gpio_set_pin(LCD_RS);
        LCDWRITE(data);
//        printf("OLED Write data = %x\n",data);
	return 0;
}

static unsigned char lcm_read_data(void)
{

}

/*
static void lcd_setbright(int bright)
{
	lcm_write_cmd(0x81);
	lcm_write_cmd(bright);
}
*/
static void LCDGPIOINIT(void)
{
	int i;

	for(i=32*GPC+8;i<32*GPC+8+10;i++)
		__gpio_as_output(i);

	return;
}
static void COGLCDInit(void)
{
	LCDGPIOINIT();
	lcm_write_cmd(0xAE);    //Display off
        lcm_write_cmd(0xE2);    //Reset
        lcm_write_cmd(0xA2);    //set LCD Bias(1/9)
	if (lcdconvert)
		lcm_write_cmd(0xC0);    //SHL select COM1-COM64
	else	
		lcm_write_cmd(0xC8);    //SHL select COM1-COM64
        lcm_write_cmd(0xA6);    //Normal display
        lcm_write_cmd(0x2F);    //set power control(VB VR VF)
        lcm_write_cmd(0x81);    //set reference voltage mode
//      lcm_write_cmd(0x2c);    //SET REFENENCE VOLTAGE REGISTER 30H
        if(lcdheight == 32) {
		lcm_write_cmd(0x20);    //SET REFENENCE VOLTAGE REGISTER 30H
                lcm_write_cmd(0x40);    //set line start
                lcm_write_cmd(0xA1);    //ADC Select S1-S132
        }
        else { 
		lcm_write_cmd(0x2c);    //SET REFENENCE VOLTAGE REGISTER 30H
                lcm_write_cmd(0x60);    //set line start
		if (lcdconvert)
			lcm_write_cmd(0xA1);    //ADC Select S1-S128
		else	
			lcm_write_cmd(0xA0);    //ADC Select S1-S128
       }
       lcm_write_cmd(0xAF);    //Display On
}

/*========================== COG end ===========================*/

/*============================OLED Support Start==================*/
#if 1
static void GPIOINIT(void)
{
	int i;

	for(i=32*GPC+8;i<=32*GPC+8+10;i++)	
		__gpio_as_output(i);	

	__gpio_set_pin(OLED_CS);
	__gpio_set_pin(OLED_A0);
	__gpio_set_pin(OLED_RD);	
	__gpio_set_pin(OLED_WR);
	__gpio_set_pin(OLED_RESET);
}
#endif
static void OLEDWRITE(unsigned char data)
{
	unsigned int d;
	
        d = data;
	d =(d<<8)&0xff00;
        __gpio_set_port(GPC) = d;
	__gpio_clear_port(GPC) = ~d&0xff00;
}

static unsigned char OLEDREAD(void)
{
	unsigned int data;

	data = (__gpio_get_port(2)>>8)&0xff;

	return (unsigned char)data;
}

static void OLED_WriteCmd(unsigned char cmd)
{
	__gpio_clear_pin(OLED_CS);
	__gpio_clear_pin(OLED_A0);
	__gpio_clear_pin(OLED_WR);	
	OLEDWRITE(cmd);
	__gpio_set_pin(OLED_WR);
	__gpio_set_pin(OLED_CS);
//	printk("OLED Write cmd = %x\n",cmd);
}

static void OLED_WriteData(unsigned char data)
{
	__gpio_clear_pin(OLED_CS);
	__gpio_set_pin(OLED_A0);
	__gpio_clear_pin(OLED_WR);	
	OLEDWRITE(data);
	__gpio_set_pin(OLED_WR);
	__gpio_set_pin(OLED_CS);
//	printk("OLED Write data = %x\n",data);
}

static unsigned char OLED_ReadData(void)
{
	unsigned char data,i;

/* set d0 - d7 to be input */
	for(i=32*GPC+8;i<=32*GPC+8+10;i++)	
		__gpio_as_input(i);	

	__gpio_set_pin(OLED_A0);
	__gpio_clear_pin(OLED_CS);
	__gpio_clear_pin(OLED_RD);
	data =	OLEDREAD();
	__gpio_set_pin(OLED_RD);
	__gpio_set_pin(OLED_CS);
//	printk("OLED Read data = %x\n",data);

/* restore d0 - d7 to be output */
	for(i=84;i<=91;i++)	
		__gpio_as_output(i);	

	return data;
}

static void OLED_HardwareReset(void)
{
	__gpio_clear_pin(OLED_RESET);
	udelay(5);
	__gpio_set_pin(OLED_RESET);
	udelay(1);
}



static void OLED_Init(void)
{
	GPIOINIT();
	OLED_HardwareReset();

	OLED_WriteCmd(0xAE);	//Dot martix display off
	OLED_WriteCmd(0x40);	//Set Display Start Line(0x40 - 0x7f)
	OLED_WriteCmd(0xC8);	//Com Scan com1 - com64(0xc8 - 0xc0)	
	OLED_WriteCmd(0xA1);	//Set Segment RE-MAP(0xa0 - 0xa1)
	OLED_WriteCmd(0xA4);	//Entier Display off(0xa4 - 0xa5)
	OLED_WriteCmd(0xA6);	//Set Normal Display(0xa6 - 0xa7)

	OLED_WriteCmd(0xA8);	//Set Multiplex Ratio 64
	OLED_WriteCmd(0x3F);	

	OLED_WriteCmd(0x81);	//Contarst Control(0x00 - 0xff)
	OLED_WriteCmd(0x3F);	
	OLED_WriteCmd(0xAD);	//Set DC/DC Booster(0x8a=off,0x8b=on)
	OLED_WriteCmd(0x8A);	

	OLED_WriteCmd(0xD3);	//Set display offset (0x00 - 0x3f)
	OLED_WriteCmd(0x00);	

	OLED_WriteCmd(0xD5);	//Set frame frequency
	OLED_WriteCmd(0x60);	

	OLED_WriteCmd(0xD9);	//Set per_charge period
	OLED_WriteCmd(0x22);

	OLED_WriteCmd(0xDA);	//Com pin configuration(0x02,0x12)
	OLED_WriteCmd(0x12);	

	OLED_WriteCmd(0xDB);	//Set vcom deselect(0x35)
	OLED_WriteCmd(0x35);	
	//delay 10ms
	OLED_WriteCmd(0xAF);	//Display on 
}

static void OLED_SetColumn(unsigned char col)
{
	col += 2;
	OLED_WriteCmd(LOW_COL|(col&0x0f));
	OLED_WriteCmd(HIGH_COL|((col>>4)&0x0f));
}

static void OLED_SetPage(unsigned char page)
{
	OLED_WriteCmd(PAGE|(page&0x07));
}
	
/*============================OLED end==================*/

/*
static void lcd_detect(void)
{
        unsigned char data[4] = {0xab,0xcd,0xef,'\0'},result[4] = {0 ,0 ,0 ,'\0'};
        unsigned char cs = 1,i;

//	if ( (lcd.style == cob) || (lcd.style == cog) ) {
	if(lcd.style!=0)
	{
                printk("Lcd style is appointed!\n");
                return;
        }
        __lcdc_sync(cs);
        __lcdc_write(CMD_DISPLAY_OFF, cs);
        setting(t_page,t_address);
        __lcdc_sync(cs);
        __lcdc_cdhi();
#if 1                                   //Write 3 datas lcd
        __lcdc_write(data[0], cs);
        udelay(5);
        __lcdc_write(data[1], cs);
        udelay(5);
        __lcdc_write(data[2], cs);
#endif
        setting(t_page,t_address);
        __lcdc_sync(cs);
        __lcdc_cdhi();
        __lcdc_read(cs);
        for(i=0;i<3;i++)                //Read 3 datas from lcd
        {
                __lcdc_sync(cs);
                __lcdc_cdhi();
                result[i] = (unsigned char)__lcdc_read(cs);
//              printk("result[%d]=%x\n",i,result[i]);
        }
	if(strcmp(data,result) == 0) { //Compare result data with wrote data to determine lcd style

                lcd.style = cob;
                printk("Detecting lcd style is cob!\n");
        }
        else {
                lcd.style =cog;                                           //default LCD is cog
                printk("Default lcd style is cog!\n");
        }
}
*/

int lcd_ioctl(unsigned int cmd, unsigned int *arg)
{
	unsigned int data;
	
	data = *arg;
/*
	if (lcd.height == 32) {		// 132*32 lcd support start
		int cs, ret = 0;
        	unsigned char c,tmp,i;

        	switch (cmd) {
        	case LCD_DISPLAY_ON:
                	cs = data & 0x3;
                	if(cs == 1)
                        	ret = lcm_write_cmd(CMD_COG_DISPLAY_ON);
                	return ret;
        	case LCD_DISPLAY_OFF:
                	cs = data & 0x3;
                	if(cs == 1)
                		ret =  lcm_write_cmd(CMD_COG_DISPLAY_OFF);
                	return ret;
        	case LCD_SET_PAGE:
                	c = (char)(data&0x0f);
                	switch(c)
			{
                		case 0:c=3;break;
                		case 1:c=2;break;
                		case 2:c=1;break;
                		case 3:c=0;break;
                	}
                	return lcm_write_cmd(CMD_COG_SET_PAGE|c);
        	case LCD_SET_ADDRESS:
               		cs = (data >> 16)&0x3;
                	c = (char)(data&0xff);
                	if(cs == 2)
                        	c += 61;
                	lcm_write_cmd(CMD_COG_SET_ADDRESS_MSB|((c&0xf0) >> 4));
                	lcm_write_cmd(CMD_COG_SET_ADDRESS_LSB|(c&0x0f));
                	return 0;
        	case LCD_RESET:
                	return lcm_write_cmd(CMD_COG_RESET);
        	case LCD_DISPLAY_LINE_START:
			c = (char)(data&0x03f);
                	return lcm_write_cmd(CMD_COG_DISPLAY_LINE_START|c);
        	case LCD_WRITE_DATA:
                	c = data & 0x0ff;
                	tmp=0;
                	for(i=0;i<8;i++)
                	{
                        	if(c&(1<<i))
                                	tmp+=(1<<(7-i));
                	}
                	return lcm_write_data(tmp);
        	case LCD_READ_DATA:
//                	data = (int)lcm_read_data();
                	return;
        	default:
                	printk("JZ LCD:*** Do not support\n");
                	return -EINVAL;
		}
        }
*/
	if(lcdstyle==cog) {		//Cog lcd support start
 
		int cs, ret = 0;
		char c;

		switch (cmd) {
		case LCD_DISPLAY_ON:
			cs = data & 0x3;
			if(cs == 1)
				ret = lcm_write_cmd(CMD_COG_DISPLAY_ON);
				return 0;
		case LCD_DISPLAY_OFF:
			cs = data & 0x3;
			if(cs == 1)
				ret =  lcm_write_cmd(CMD_COG_DISPLAY_OFF);
				return ret;
		case LCD_SET_PAGE:
			c = (char)(data&0x0f);
			return lcm_write_cmd(CMD_COG_SET_PAGE|c);
		case LCD_SET_ADDRESS:
			cs = (data >> 16)&0x3;
			c = (char)(data&0xff);
			if (lcdconvert)
				c += 0x03;	
			else
				c++;
			if(cs == 2)
				c += 64;
			lcm_write_cmd(CMD_COG_SET_ADDRESS_MSB|((c&0xf0) >> 4));
			lcm_write_cmd(CMD_COG_SET_ADDRESS_LSB|(c&0x0f));
			return 0;
		case LCD_RESET:
			return 0;
			return lcm_write_cmd(CMD_COG_RESET);
		case LCD_DISPLAY_LINE_START:
#if 0
			error = get_user(data, (int *)arg);
			if (error)
				return error;
			c = (char)(data&0x03f);
				return lcm_write_cmd(CMD_DISPLAY_LINE_START|c);
#endif 
		return lcm_write_cmd(0x60);
		case LCD_WRITE_DATA:
			c = data & 0x0ff;
			return lcm_write_data(c);
		case LCD_READ_DATA:
//			data = (int)lcm_read_data();
			return;
		default:
			printf("JZ LCD:*** Do not support\n");
			return -1;
		}
	}					//Cog lcd support end
	else if(lcdstyle==oled) {		//OLED support start

		switch(cmd){

		case LCD_DISPLAY_ON:
			OLED_WriteCmd(0xAF);
			break;

		case LCD_DISPLAY_OFF:
			OLED_WriteCmd(0xAE);
			break;

		case LCD_SET_PAGE:
			OLED_SetPage((unsigned char)data&0x0f);
			break;

		case LCD_SET_ADDRESS:
			if(((data>>16)&0x3)==2)
                                data += 64;
			OLED_SetColumn((unsigned char)data&0xff);
			break;
		case LCD_DISPLAY_LINE_START:
			break;

		case LCD_RESET:
		/*
			OLED_HardwareReset();
			OLED_Init();
		*/
			break;

		case LCD_WRITE_DATA:
			OLED_WriteData((unsigned char)data&0xff);
			break;

		case LCD_READ_DATA:
//			data = (int)OLED_ReadData();
			break;

		default:
			printf("LCD:*** Do not support\n");
			return -1;
		}

	}	//OLED support end
	return 0;
}

void InitializeLCD(void)
{
	lcdconvert = getLCDType();
	if(lcdstyle==cog)
		COGLCDInit();		
	else if(lcdstyle==cob)
		OLED_Init();
}

