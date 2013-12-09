#include	"oled.h"
#include	"gpio.h"
void llock_gpio_init(void)
{
	L3000PortInit();
	OLED_PIN_INIT();
	__gpio_as_input(PIN_KEY_DEL);
	__gpio_as_output(LE_373_1);
	__gpio_clear_pin(LE_373_1);
	//__gpio_as_output(LE_373_2);
	//__gpio_clear_pin(LE_373_2);
	//__gpio_as_output(LE_244);
	//__gpio_set_pin(LE_244);

	__gpio_set_pin(OLED_PIN_DATA0);
	__gpio_set_pin(OLED_PIN_DATA0+1);
	__gpio_set_pin(OLED_PIN_DATA0+2);
	__gpio_set_pin(OLED_PIN_DATA0+3);
	__gpio_set_pin(OLED_PIN_DATA0+4);
	__gpio_set_pin(OLED_PIN_DATA0+5);
	__gpio_set_pin(OLED_PIN_DATA0+6);
	__gpio_set_pin(OLED_PIN_DATA0+7);
	
	__gpio_as_output(MTRPWR_EN);
	__gpio_set_pin(MTRPWR_EN);
	__gpio_as_input(GP_KEYSCAN);

	Write373(1, LEDR);
	Write373(1, LEDG);
	Write373(1, BEEP);
}
/*
void	Enable244(int on)
{
	if(on) __gpio_clear_pin(LE_244);
	else   __gpio_set_pin(LE_244);
}
*/
/*
//============================373-1=============================================
#define LE_373_1      (GPC+22)
#define LOCKMTR1      (1<<0)
#define LOCKCTL1      (1<<1)
#define COVERMTR1     (1<<2)
#define COVERMTR2     (1<<3)
#define DATA_164      (1<<4)
#define CLK_164       (1<<5)
//============================373-2=============================================
#define LE_373_2       (GPC+23)
#define FGPWER_EN      (1<<0)
#define LCDBL_EN       (1<<1)
#define RFPWR_EN       (1<<2)
#define SDPWR_EN       (1<<3)
#define SOUND_EN       (1<<4)
*/
#define Write373_1 Write373
void Write373(U8 val, U8 mask)
{
	static	U8 old_data = 0;
	U8 tmp = 0;

	if(val) old_data |= mask;
	else old_data &= ~mask;

	__gpio_clear_pin(LE_373_1);
	__gpio_clear_pin(LE_373_1);
	
	__gpio_clear_port(2) = 0xff << 8;
	__gpio_set_port(2) = old_data << 8;
	
	__gpio_set_pin(LE_373_1);
	__gpio_set_pin(LE_373_1);
	__gpio_set_pin(LE_373_1);
	__gpio_clear_pin(LE_373_1);
}

#if 0
void	Write373_2(U8 val, U8 mask)
{
	static  U8 old_data = 0;
        U8 data;
        U8 tmp = 0;

        data = old_data;
        data &= ~mask;
        tmp = val & mask;
        data |= tmp;
        old_data = data;
	
	__gpio_as_output(LE_373_2);
 
        __gpio_clear_pin(LE_373_2);
        __gpio_clear_pin(LE_373_2);

        __gpio_clear_port(2) = 0xff;
        __gpio_set_port(2) = old_data;

        __gpio_set_pin(LE_373_2);
        __gpio_set_pin(LE_373_2);
        __gpio_set_pin(LE_373_2);
        __gpio_clear_pin(LE_373_2);
}
#endif

static void	Clear164DataPin(void)
{
	Write373_1(0, DATA_164);	
}

static void	Set164DataPin(void)
{
	Write373_1(DATA_164, DATA_164);	
}

static void Clear164ClkPin(void)
{
	Write373_1(0, CLK_164);
}

static void Set164ClkPin(void)
{
    Write373_1(CLK_164, CLK_164);
}

static void	Write164Bit(U8 bit)
{
    if(bit == 0)
        Clear164DataPin();
    else 
        Set164DataPin();
    Set164ClkPin();
    Clear164ClkPin();
}

static void	Write164Byte(U8 va)
{
    int i = 0;
    
    for(i=0; i<8; i++)
    {
        Write164Bit(va&(1<<(7-i)));
    }
}

static int Get164Input(void)
{
    __gpio_as_input(GP_KEYSCAN);
    return __gpio_get_pin(GP_KEYSCAN);
}

static int GetEscPin(void)
{
    __gpio_as_input(GP_KEYESC);
    return __gpio_get_pin(GP_KEYESC);
}

U32  Scan164(void)
{
    U32 val = 0;
    int i = 0;

	if(GetEscPin() == 1)
	{
		return 5;
	}

	__gpio_clear_pin(LCD_CS);	
	Write164Byte(0xff);
	if(Get164Input() == 0)
	{
		return 4;
	}

	Write164Bit(0);

	for(i=0; i<3; i++)
	{
		if(Get164Input() == 0)
		{
			return i + 1;
		}
		Write164Bit(1);
	}

    return 0;
}

#if 0
void	EnableAudio(void)
{
	Write373_2(0, SOUND_EN);
}
#endif
