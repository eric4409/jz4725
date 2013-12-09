#include	"L3000KeyScan.h"
                                                                  //ESC, UP, DOWN, MENU, OK
const	int      KEY_VALID[L3000_KEY_MAX] = {1, 0, 0, 0, 0};
const	char     KeyCharGroup[L3000_KEY_MAX] = {'2', '3', '4', 'A', '1'};
//const	char     KeyCharGroup[L3000_KEY_MAX] = {'1', '3', '4', '2', 'A'};
//const   int	 KeyPinNum[L3000_KEY_MAX] = {PIN_KEY_MENU, PIN_KEY_UP, PIN_KEY_DOWN, PIN_KEY_ESC, PIN_KEY_OK};
void	L3000KeyInit(void)
{
	L3000_KEY_INIT();
}

int KeySample(int *key)
{
	int res;

	res = Scan164();
	if(res > 0)
	{
		*key = res;
		return 1;
	}

	*key = 0;
	return	1;
}

extern	int	gMachineState;
int	CheckMenuKey(int	key)
{
	static	U32	ms = 0;
//	char k = 'A';

//	if(L3000PwdMode == 3) k = '4';
	if(key && KeyCharGroup[key-1] == 'A' && gMachineState==STA_VERIFYING)
	{
		if(!ms)
		{
			ms = GetMS();
		}
		else	if(GetMS() >= (ms + 2000))
		{
			ms = 0;
			return	1;
		}
	}
	else
	{
		ms = 0;
	}	
	return	0;
}


#define	ACCELERATE_STEP_MIN  20 
#define	ACCELERATE_STEP_MAX   (400)
#define	ACCELERATE_STEP    50
#define	ACCELERATE_START  (800+ACCELERATE_STEP_MAX)

int 	L3000KeyCheck(char *k)
{
	int	ret = 0;
	int	key=0;
	static	int	key_status = 0;
	static	U32	old_ms = 0;
	U32     ms;

	static	U32 durative_ms = 0;
	static	U32 durative_cnt = 0;
	static	int	old_key = 0;
	//static	U32 accelerate = ACCELERATE_START;

	ms = GetMS();
	*k = 0;
	if(KeySample(&key))
	{
		if(key_status == 0)
		{
			if(key) 
			{
				if(ms >= old_ms + 30 || old_ms >= ms+20)
				{	
					old_ms = ms;
					key_status = 1;
					L3000RunBeep(60, 0,0);
					*k = KeyCharGroup[key-1];
					old_key = key;
					durative_ms = ms + ACCELERATE_START;
					durative_cnt = 0;
					ret = 1;
				}
			}	
		}
		else	 
		{
			if(key == 0)
			{		
				key_status = 0;					
				old_key = 0;
			}
			else if(old_key && key == old_key && (KeyCharGroup[key-1] == '2' || KeyCharGroup[key-1] == '3'))
			{
				U32	m = 0;								
				U32     dm = 0;


				m = ACCELERATE_STEP_MAX-(ACCELERATE_STEP*durative_cnt);
				if(m > ACCELERATE_STEP_MAX || m < ACCELERATE_STEP_MIN){
				       	m = ACCELERATE_STEP_MIN;
				//	durative_cnt;// = ACCELERATE_STEP_MAX/ACCELERATE_STEP;
				}
				dm = durative_ms + m;
				if(ms >= dm || ms+ACCELERATE_START < durative_ms){
					ret = 1;
					if(KeyCharGroup[key-1] == '2') *k = 'U';
					else	if(KeyCharGroup[key-1] == '3') *k = 'D';//do	
					else	{*k = 0; ret = 0; old_key = 0;}
					durative_ms = ms;
					if(m > ACCELERATE_STEP_MIN) durative_cnt++;
				}
			}
			else	
			{
				old_key = 0;
//				durative_ms = 0;
			}
		}
	}
	else	
	{
		key = 0;
		old_key = 0;
	}

	if(CheckMenuKey(key)) 
	{
		*k = 'M';
		key_status = 1;
		old_key = 0;
		L3000RunBeep(60, 0,0);
		old_ms  = ms;
		ret = 1;
	}	

	return  ret;	
}


