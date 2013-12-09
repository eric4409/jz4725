#include	"oled.h"
#include	"options.h"
#include	"jz4740.h"
//#include	"CharTable.h"
//#if 0
/************ SHOW PICTURE DATA*************/

int	gOLED_En = 0;
void	OLED_Clear(void);
void	OLED_ShowImg16_V(uint8	row,  uint8 col, uint8 *buf);
void	OLED_ShowImg(uint8	row, uint8	col, uint8	count,  uint8 *buf);

//#endif
// MODE: 8080

static	uint8	OLED_DTick = 0;
void	OLED_Dly(void)
{
	OLED_DTick = 8;
	while(--OLED_DTick);
}

void	OLED_HWInit(void)
{
	OLED_PIN_INIT();
	OLED_Dly();
	OLED_RW_1;
	OLED_AO_1;        
	OLED_CS_1;
	OLED_RS_1;
	OLED_RST_1;
	OLED_Dly();
	/*
         __gpio_as_output(OLED_PIN_RS);
         __gpio_as_output(OLED_PIN_WR);      
         __gpio_as_output(OLED_PIN_CS);      
         __gpio_as_output(OLED_PIN_A0);      
	 __gpio_as_output(OLED_PIN_RES);      
	 __gpio_as_output(OLED_PIN_DATA0 +0);      
	 __gpio_as_output(OLED_PIN_DATA0 +1);      
	 __gpio_as_output(OLED_PIN_DATA0 +2);      
	 __gpio_as_output(OLED_PIN_DATA0 +3);      
	 __gpio_as_output(OLED_PIN_DATA0 +4);      
	 __gpio_as_output(OLED_PIN_DATA0 +5);      
	 __gpio_as_output(OLED_PIN_DATA0 +6);      
	 __gpio_as_output(OLED_PIN_DATA0 +7)I;*/      						 
}


void OLED_HardwareReset(void)
{   
    OLED_RST_0;
    DelayMS(200);
    OLED_RST_1;
    DelayMS(200);
}   

void	OLED_WData(uint8	data)
{
	unsigned	int	va ;
	unsigned	int	port ;

	//port =  __gpio_port_data(2);
	va = data;
	va <<= OLED_PIN_DATA_NUM;
	va &= (0x000000ff<<OLED_PIN_DATA_NUM);
	__gpio_clear_port(2) = (0x000000ff<<OLED_PIN_DATA_NUM);
	__gpio_set_port(2) = va;

	/*
	p = __gpio_port_data(2);
        p &= 0xf00fffff;
        p |= (d<<20);
        __gpio_port_data(2) = p;
	*/
}
	//__gpio_set_pin(23);
static	void OLED_WCom(uint8 cmd1)
{
	
        OLED_RW_1;

         OLED_AO_0;        
         OLED_AO_0;        
         OLED_AO_0;        
         OLED_AO_0;        
         OLED_AO_0;        
	 OLED_CS_0;
	 OLED_CS_0;
	 OLED_CS_0;
	 
	 OLED_RW_0;			
	 //OLED_Dly();
	 OLED_WData(cmd1);       	 
	 //OLED_Dly();
	 //OLED_RW_0;			
	 //OLED_Dly();

	// OLED_Dly();
	 OLED_RW_1;
	 OLED_CS_1;	
	 OLED_AO_1;	
}	

static	void OLED_WDta(uint8 data1)
{
	 OLED_RW_1;

	 OLED_AO_1;        
	 OLED_AO_1;        
	 OLED_AO_1;        
	 OLED_AO_1;        
	 OLED_AO_1;        
	 OLED_CS_0;
	 OLED_CS_0;
	 OLED_CS_0;

	 OLED_RW_0;			
	 //OLED_Dly();     
         OLED_WData(data1);
	 //OLED_Dly();

	 //OLED_RW_0;
	 OLED_Dly();
	 OLED_RW_1;
	 OLED_CS_1;		 
}

static	uint8 col_offset = 2;
static	void OLED_SetColumn(uint8 col)
{
        col += col_offset;
        OLED_WCom(OLED_LOW_COL + (col&0x0f));
        OLED_WCom(OLED_HIGH_COL + ((col>>4)&0x0f));
}

static void OLED_SetPage(uint8 page)
{
        OLED_WCom(OLED_START_PAGE + (page&0x07));
}

void oled_init(void)
{
	 U8 contrast = 60; //gOptions.OledContrast;
	 OLED_HWInit();
	 OLED_HardwareReset();
	
	//if(gOptions.OLEDType == 1) col_offset = 4;
	//else			   col_offset = 2; 
	col_offset = 2;
        OLED_WCom(0xAE);        //DOT MARTIX DISPLAY OFF        
        OLED_WCom(0x40);        //SET DISPLAY START LINE(40H-7FH)
        //OLED_WCom(0x7f);        //SET DISPLAY START LINE(40H-7FH)
       // OLED_WCom(0xC8);        //COM SCAN COM1-COM64(0C8H,0C0H)
        OLED_WCom(0xC0);        //COM SCAN COM1-COM64(0C8H,0C0H)
       // OLED_WCom(0xA1);        //SET SEGMENT RE-MAP(0A0H-0A1H)
        OLED_WCom(0xA0);        //SET SEGMENT RE-MAP(0A0H-0A1H)
        
        OLED_WCom(0xA4);        //ENTIRE DISPLAY OFF(0A4H-0A5H)
        OLED_WCom(0xA6);        //SET NORMAL DISPLAY(0A6H-0A7H)        
        OLED_WCom(0xA8);        //SET MULTIPLEX RATIO 64
        OLED_WCom(0x3F);                
        
        OLED_WCom(0x81);        //CONTARST CONTROL(00H-0FFH)       
        //OLED_WCom(0x3F);
        OLED_WCom(contrast);
//        DelayMS(200);
        OLED_WCom(0xAD);        //SET DC/DC BOOSTER(8AH=OFF,8BH=ON)
        OLED_WCom(0x8A);  
		
        OLED_WCom(0xD3);        //SET DISPLAY OFFSET(OOH-3FH)        
        OLED_WCom(0x00);        
        OLED_WCom(0xD5);        //SET FRAME FREQUENCY
        OLED_WCom(0x60);  		
		
        OLED_WCom(0xD9);        //SET PRE_CHARGE PERIOD        
        OLED_WCom(0x22);        
        OLED_WCom(0xDA);        //COM PIN CONFIGURATION(02H,12H)
        OLED_WCom(0x12);        
		
        OLED_WCom(0xDB);          //SET VCOM DESELECT LEVEL(035H)        
        OLED_WCom(0x35);
        OLED_Clear();
//        DelayMS(250);
        OLED_WCom(0xAF);         //DSPLAY ON
	 /*
        OLED_WriteCmd(0xAE);    //Dot martix display off
        OLED_WriteCmd(0x40);    //Set Display Start Line(0x40 - 0x7f)
        OLED_WriteCmd(0xC8);    //Com Scan com1 - com64(0xc8 - 0xc0)
        OLED_WriteCmd(0xA1);    //Set Segment RE-MAP(0xa0 - 0xa1)
        
        OLED_WriteCmd(0xA4);    //Entier Display off(0xa4 - 0xa5)
        OLED_WriteCmd(0xA6);    //Set Normal Display(0xa6 - 0xa7)
        OLED_WriteCmd(0xA8);    //Set Multiplex Ratio 64
        OLED_WriteCmd(0x3F);

        OLED_WriteCmd(0x81);    //Contarst Control(0x00 - 0xff)
        OLED_WriteCmd(0x3F);
        OLED_WriteCmd(0xAD);    //Set DC/DC Booster(0x8a=off,0x8b=on)
        OLED_WriteCmd(0x8A);

        OLED_WriteCmd(0xD3);    //Set display offset (0x00 - 0x3f)
        OLED_WriteCmd(0x00);
        OLED_WriteCmd(0xD5);    //Set frame frequency
	 OLED_WriteCmd(0x60);

        OLED_WriteCmd(0xD9);    //Set per_charge period
        OLED_WriteCmd(0x22);
        OLED_WriteCmd(0xDA);    //Com pin configuration(0x02,0x12)
        OLED_WriteCmd(0x12);
        
        OLED_WriteCmd(0xDB);    //Set vcom deselect(0x35)
        OLED_WriteCmd(0x35);
        Delay 10ms
        OLED_WriteCmd(0xAF);    //Display on*/

}
void	OLED_DisOff()
{
	OLED_WCom(0xAe);         //DSPLAY Off
	DelayMS(100);
}
void	OLED_Clear()
{
	uint8	i,j;

	for(i=0; i < 8; i++){		
	        OLED_SetPage(i);
	        OLED_SetColumn(0);
		for(j=0; j<128; j++){
			OLED_WDta(0);
		}
	}
}
/*
void OLED_DispPicture(uint8  *picturn)
{
    uint8 i,j,k;
	
        k=0;
	for(i=0; i<8; i++)
	{
	     OLED_SetPage(i);
	     OLED_SetColumn(0);
	     for(j=0;j<128;j++)
	     {
            	  OLED_WDta(picturn[k]);
	          k++;
	     }
	}
}*/


void	OLED_DispPicture(uint8 * picture)
{
	uint8	i,j,k = 0;

	for(i=0; i<8; i++)
	{
	    // OLED_SetPage(i);
	    // OLED_SetColumn(0);
	     for(j=0; j<16; j++)
	     {
            	  OLED_ShowImg(i, j*8, 8,  picture+k);
	          k += 8;
	     }
	}
}

void	OLED_ShowImg32(uint8	row,  uint8 col, uint8 *buf);
/*void	OLED_DispHanzi(void)
{
	uint8	hz[32];

	memset(hz, 0, 32);
	memcpy(hz+2, HanziGroup[1], 28);
	OLED_ShowImg32(0, 0, hz);
}*/

void	OLED_ShowImg(uint8	row, uint8	col, uint8	count,  uint8 *buf)
{
	uint8 i;
	OLED_SetColumn(col);
	OLED_SetPage(row);
	for(i=0; i<count; i++){
		OLED_WDta(buf[i]);
	}
}

void	OLED_ShowImg16_V(uint8	row,  uint8 col, uint8 *buf)
{
	OLED_ShowImg(row, col, 8,  buf);
	++row;
	OLED_ShowImg(row, col, 8,  buf+8);
}

void	OLED_ShowImg16_H(uint8	row,  uint8 col, uint8 *buf)
{
	OLED_ShowImg(row, col, 16, buf);
}

void	OLED_ShowImg32(uint8	row,  uint8 col, uint8 *buf)
{
	OLED_ShowImg(row, col, 16, buf);
	++row;
	OLED_ShowImg(row,  col, 16,  buf+16);
}

