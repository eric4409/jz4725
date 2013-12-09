#include	"L3000Switch.h"


const	U8 DefSwPort[SWITCH_MAX] = {1,1,1,1,0};
U8 L3000SwGroup[SWITCH_MAX];

void	L3000SwitchInit(void)
{
	memset(L3000SwGroup, 0, sizeof(L3000SwGroup));
}

/*
 * 电池电压
 * > 4.8V 满格
 * > 4.5  2格
 * > 4.3  1格
 * < 4.3  空
 */
#define VOL_LOW			4800
#define VOL_LOW_ALM		4500
#define VOL_LOW_OFF		4300

int	L3000CheckSwitch(U8 * sw)
{
	int	flag;
	int vol;

	//电池电源(mv)/AD采样值 = 3.8
	vol = (Read_Battery() * 38 + 5)/10;

	//L3000_READ_PORT(flag, L3000_PWR_LOW); //2格
	flag = vol > VOL_LOW;
	if(flag == DefSwPort[SW_LOW_PWR] && sw[SW_LOW_PWR])
		--sw[SW_LOW_PWR];
	else	if(flag != DefSwPort[SW_LOW_PWR] && sw[SW_LOW_PWR] < SWI_CNT_UPLIMIT)
		++sw[SW_LOW_PWR];
	else	;
	
	//__gpio_as_input(L3000_PWR_LOW_ALM); //1 
	//L3000_READ_PORT(flag, L3000_PWR_LOW_ALM);	
	flag = vol > VOL_LOW_ALM;
	if(flag == DefSwPort[SW_LOW_PWR_ALM] && sw[SW_LOW_PWR_ALM])
		--sw[SW_LOW_PWR_ALM];
	else	if(flag != DefSwPort[SW_LOW_PWR_ALM] && sw[SW_LOW_PWR_ALM] < SWI_CNT_UPLIMIT)
		++sw[SW_LOW_PWR_ALM];
	else;
	if(sw[SW_LOW_PWR_ALM] == SWI_CNT_UPLIMIT-1){
		L3000Debug("\nPower low2 is bad!\n");
	}

	//L3000_READ_PORT(flag, L3000_PWR_LOW_OFF); //空
	flag = vol > VOL_LOW_OFF;
	if(flag == DefSwPort[SW_LOW_PWR_OFF] && sw[SW_LOW_PWR_OFF])
		--sw[SW_LOW_PWR_OFF];
	else	if(flag != DefSwPort[SW_LOW_PWR_OFF] && sw[SW_LOW_PWR_OFF] < SWI_CNT_UPLIMIT)
		++sw[SW_LOW_PWR_OFF];
	else;

	__gpio_as_input(L3000_BAK_PWR);  
	L3000_READ_PORT(flag, L3000_BAK_PWR); //外部电源
	if(flag == DefSwPort[SW_BAK_PWR] && sw[SW_BAK_PWR])
		--sw[SW_BAK_PWR];
	else	if(flag != DefSwPort[SW_BAK_PWR] && sw[SW_BAK_PWR] < SWI_CNT_UPLIMIT)
		++sw[SW_BAK_PWR];	
	else;

#if 0	
	L3000_READ_PORT(flag, L3000_SENSOR);
	if(flag == DefSwPort[SW_SENSOR] && sw[SW_SENSOR])
		--sw[SW_SENSOR];
	else	if(flag != DefSwPort[SW_SENSOR] && sw[SW_SENSOR] < SWI_CNT_UPLIMIT)
		++sw[SW_SENSOR];	
	else;
#endif

	return	0;
}
