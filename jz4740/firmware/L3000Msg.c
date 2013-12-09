#include	"app.h"
#include	"exfun.h"
#include	"L3000Operate.h"
#if 0
#include	"ccc.h"
#include	"app.h"
#include	"time.h"
#include	"msg.h"
#include	"options.h"
#include	"flashdb.h"
#include 	"stdlib.h"
#include 	"string.h"
#include	"OLedDisplay.h"
#include	"L3000KeyScan.h"
#include	"L3000RTCCheck.h"
#include	"L3000Msg.h"
#include	"L3000Switch.h"
#include	"exfun.h"
//#include	"L3000Drv.h"

//extern	int	L3000LowClk;

#endif


int	gInputNumKey = 1;

int	L3000GetKeyChar(char key)
{
	int	ret = 0;
#if 1
	if(gInputNumKey)
	{

		if(gLock.nor_open == LOCK_STATE_OPEN){
			if(gState.state == STA_NOR_OPEN && gMachineState == STA_VERIFYING){
				if(key == KEY_AVOID_NOR_UNLOCK || key == 'A'){
					//		L3000Debug("\nGet a avoidnorunlock key!!!!!!!!!!!\n");
					return	 IKeyAvoidNorUnLock;
				}
				else	if(key == '1') return	IKeyESC;
				else	;				
			}
			// else	L3000Debug("\n get avoidnorunlock key error!!!!!!!");
		}
		else	if(gState.state == STA_VRY_END){
			if(gMachineState == STA_VERIFYING
					&& gLock.nor_open == LOCK_STATE_CLOSE 
					&& gLock.open_delay_ms && gLock.close_ms && gLock.nor_open_key){
				if(key == KEY_NOR_UNLOCK || key == 'A'){
					//		L3000Debug("\nGet a norunlock key!!!!!!!!!!!\n");
					if(gOptions.NorOpenOn)	return	 IKeyNorUnLock;
					//		gLock.nor_open_key = 0;
				}	
				else	if(key == '1') return	IKeyESC;
				else	;				
			}
			// else	L3000Debug("\n get norunlock key error!!!!!!!");
		}
		else	{
			if(L3000PwdMode == 4)	
			{     
				if(key == 'A') ret = IKeyOK;
				else	if(key == 'M') ret = IKeyMenu;
				else	if(key >= '1' && key <= '4')	ret = key;
				else    ret = 0;
			}
			else	
			{
				if(key == '4' || key == 'A') ret = IKeyOK;
				else	if(key == 'M') ret = IKeyMenu;
				else	if(key >= '1' && key <= '3')	ret = key;
				else    ret = 0;			
			}
			/*	if(ret)
				{
				char str[50];
				sprintf(str, "\nhave a key %c", (char)ret);
				L3000Debug(str);
				}*/
		}		
	}
	else	
	{
		if(key == '1') ret = IKeyESC;
		else	if(key == '2') ret = IKeyUp;
		else	if(key == '3') ret = IKeyDown;		
		//	else	if(key == '4') ret = IKeyOK;
		else	if(key == 'A')  ret= IKeyOK;
		else	if(key == 'U')  {
			ret = IKeyUpUp;
			//	L3000Debug("\nhave a upup key msg");
		}
		else	if(key == 'D') {
			ret = IKeyDownDown;
			//	L3000Debug("\nhave a downdown key msg!!!");
		}
		else    ret = 0;
	}
#endif
	return	ret;
}

int	L3000CheckSwitchMsg(U8 *swi_grp, char *buf)
{
	static	U32	old_swi = 0;
	int	i;

	for(i=0; i<SWITCH_MAX; i++){
		if((old_swi & (1<<i)) == 0){
			if(swi_grp[i] >= SWI_CNT_UPLIMIT){
				old_swi |= 1<<i;
				buf[0] = i;
				buf[1] = 1;
				return	1;	
			}
		}
		else	{		
			if(swi_grp[i] == 0){
				old_swi &= ~(1<<i);
				buf[0] = i;
				buf[1] = 0;
				return	1;	
			}
		}
	}
	return	0;
}

int	L3000MsgCheck(char *buf)
{
	int	ret = 0;
	static	unsigned	int last_tick = 0;
	unsigned	int	current_tick = 0;
	static	U32    last_tick2;


	BYTE time[7];	
	memset(time, 0, 7);
//	if(L3000KeyCheck(buf)) 
	if(buf[0] = GetKey())
	{
		return	MCU_CMD_KEY;	
	}
	current_tick = GetMS();
#if 0
	if(current_tick - last_tick > 200 || last_tick > current_tick){
		last_tick = current_tick;
		HT1380_ReadTime(time);

		if(memcmp(time, buf, 7) != 0) {
			memcpy(buf, time, 7);

			return	MCU_CMD_RTC;		
		}
	}
#endif
	if(current_tick - last_tick2 >= RUN_TICK || last_tick2 > current_tick){
		last_tick2 = current_tick;
		L3000CheckSwitch(L3000SwGroup);
		if(L3000CheckSwitchMsg(L3000SwGroup, buf))
		{
			return	MCU_CMD_SWITCH;
		}
	}

	return	0;
}



