
#define GPA	0
#define GPB	1
#define GPC	2
#define GPD	3

#define	GPIO_NULL     (32*4+19)  ////////////////////////////////////////////////

#define  LV373_LE        GPIO_NULL //(32*1+17)
#define  RTC_EN  	GPIO_NULL //(32*1+18)

#define  RS485_SEND     GPIO_NULL //(32*2+8)
#define  REDLED         GPIO_NULL //(32*2+10)
#define  GREENLED       GPIO_NULL//(32*2+9)
#define  POWER          GPIO_NULL//(32*2+11)
#define  SOUNDEN        GPIO_NULL//(32*2+12)
#define  LCMPOWER       GPIO_NULL//(32*2+13)
#define  HV7131POWER    GPIO_NULL//(32*2+14)
#define  SW_RS232_RS485	GPIO_NULL//(32*2+15)
#define  WEI_DP         GPIO_NULL//GREENLED
#define  WEI_DN         GPIO_NULL//REDLED
#define  KEY_POWER_PIN		GPIO_NULL//(GPC*32+21)
#define  KEY_POWER_PIN_U100	GPIO_NULL//(GPD*32+8)

/*============================373-1=============================================*/
#define LE_373_1      (32*GPC+19)
#define M1_IN		  (1<<0)
#define M2_IN		  (1<<1)
#define LEDR          (1<<2)
#define LEDG          (1<<3)
#define BEEP          (1<<4)
#define CON3V3     	  (1<<5)
#define DATA_164      (1<<6)
#define CLK_164       (1<<7)

/*============================373-2=============================================*/
/*
#define LE_373_2       (32*GPC+23)
#define FGPWER_EN      (1<<0)
#define LCDBL_EN       (1<<1)
#define RFPWR_EN       (1<<2)
#define SDPWR_EN       (1<<3)
#define SOUND_EN       (1<<4)
*/
/*============================244=============================================*/
/*
#define LE_244           (32*GPB+26)
#define DOOR_SENSOR      (1<<0)
#define DOOR_SPUARE      (1<<1)
#define SD_ON            (1<<2)
#define BAT_CAPACITY     (1<<3)
#define BAT_ALM          (1<<4)
#define BAT_OFF          (1<<5)
#define BAT_EXTER        (1<<6)
*/
/*============================oled display=============================================*/
#define	LCD_CS  (32*GPC+16)
#define LCD_WR  (32*GPC+17)
#define LCD_AO  (32*GPC+18)

/*============================other=============================================*/
#define	MTRPWR_EN        (32*GPC+21)
#define GP_KEYSCAN       (32*GPD+28)
#define GP_KEYESC		 (32*GPB+26)
#define	PIN_KEY_DEL    	 (32*GPB + 17) 



