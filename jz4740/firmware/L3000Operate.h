#ifndef  __L3000OPEARTE
#define  __L3000OPEARTE
//#include		"uboot-jz/jz47xx.h"
#include	"ccc.h"
#include	"app.h"
#include	"time.h"
#include	"msg.h"
#include	"options.h"
#include	"flashdb.h"
#include 	"stdlib.h"
#include 	"string.h"
//#include	"OLedDisplay.h"
#include	"L3000KeyScan.h"
#include	"L3000RTCCheck.h"
#include	"L3000Msg.h"
#include	"L3000Drv.h"
#include	"mainmenu.h"
#include	"L3000Switch.h"
#include  	"L3000Drv.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define		L3000_TEST   1
#define		SYS_CLK      336
#define		SYS_LOW_CLK  80
#define		RUN_TICK 		50	//20
//#define	FP_READER  1 //have film
#define		FP_READER  2 //no film
#define		PWD_MODE   4 // 4: four key for passward; 3: three key for passwared;  
extern		int	L3000PwdMode;
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define		DFT_USER_OPERATE_MAX_TIME    (8*(1000/RUN_TICK))

#if(LOCK_L6000==1)
#define		LOCK_OPEN_TICK     (250/RUN_TICK)
#define		LOCK_CLOSE_TICK    (250/RUN_TICK)
#elif(LOCK_L7000==1)
#define		LOCK_OPEN_TICK     (950/RUN_TICK)
#define		LOCK_CLOSE_TICK    (1000/RUN_TICK)
#endif

#define		LOCK_STATE_CLOSE  0
#define		LOCK_STATE_OPEN   1 

#define		ADJUST_CLK(clk) (clk)// (((clk) * 600) /1000)


#define	DIS_VRY_TICK    (400/RUN_TICK)

#define	MAX_FINGER_CNT     (500/100)
#define	MAX_PWD_CNT        100
#define	MAX_USER_CNT       500     
#define	MAX_LOG_CNT        (30000/10000)

#define	VRY_METHOD_BYFP     0 
#define	VRY_METHOD_BYCARD   1
#define	VRY_METHOD_BYPWD    2

/*************************************************************************/
#define	L3000_STR_REPLACE1    LoadStrByID(HID_BAK_PWR)//"Backup Power"
#define	L3000_STR_REPLACE2    LoadStrByID(HID_PLS_SET_DATE)//"PLS Set Date"
#define	L3000_STR_REPLACE3    LoadStrByID(HID_RTC_ERR)//"Date Error!"
#define	L3000_STR_REPLACE4    LoadStrByID(HID_NORMAL_OPEN)//"Nor Unlock!"
#define	L3000_STR_REPLACE5    LoadStrByID(HID_LOCKED)//"Already Lock!"
#define	L3000_STR_REPLACE6    LoadStrByID(HID_WARNING)//"Vry fail!"
#define	L3000_STR_REPLACE7    LoadStrByID(HID_BACK_ESC)//"ESC Back"
#define	L3000_STR_REPLACE8    LoadStrByID(HID_CHECK_DATE)//"Check Date..."
#define	L3000_STR_REPLACE9    LoadStrByID(HID_YES_OK)//"OK YES"
#define	L3000_STR_REPLACE11   LoadStrByID(HID_LOCK)//"Lock On"
//#define	L3000_STR_REPLACE12   LoadStrByID(HID_NORMAL_OPEN)//"Nor Unlock"
#define	L3000_STR_REPLACE13   LoadStrByID(HID_NORMAL_OPEN_M)//"Normal M"
#define	L3000_STR_REPLACE14   LoadStrByID(MID_OPTION_LOCK)//"Lock Control" // MID_OPTION_LOCK
//#define	L3000_STR_REPLACE15   "PWD INVALID "
#define	L3000_STR_REPLACE16   LoadStrByID(HID_PASSWARD_FULL)//"PWD FULL" // 
#define	L3000_STR_REPLACE18   LoadStrByID(HID_LOCK_M)//"Lock M" // 
////#define	L3000_STR_REPLACE19   "Pwd" // 
#define	L3000_STR_REPLACE20   LoadStrByID(HID_FIRST_VERIFY)//"first verify" // 
#define	L3000_STR_REPLACE21   LoadStrByID(HID_PRI_INVALID)//"no permitted" // 
#define	L3000_STR_REPLACE22   LoadStrByID(HID_REPEAT_VERIFY)//"Repeat Verify" // 
#define	L3000_STR_REPLACE23   LoadStrByID(HID_USER_FULL)//"User Full"  //LoadStrByID(HID_REPEAT_VERIFY)//"Repeat Verify" // 
#define	L3000_STR_REPLACE24   LoadStrByID(MID_NO_ADMIN)//"User Full"  //LoadStrByID(HID_REPEAT_VERIFY)//"Repeat Verify" // 
#define	L3000_STR_REPLACE25   LoadStrByID(HID_PAFFIRM)//"Pwd Affirm OK"  //LoadStrByID(HID_REPEAT_VERIFY)//"Repeat Verify" // 
//
/*************************************************************************/
typedef	struct	__L3000SELECTBOX
{
	char    *content;
	int	timeout;
	int	style;	
}TL3000SelectBox;

#define CARTOON_FREE    0
#define	CARTOON_SMILE   1
#define	CARTOON_UNLOCK  2
#define	CARTOON_LOCK    3
#define	CARTOON_DEPAND  4
#define	CARTOON_VRY_VALID_UNLOCK  8
typedef struct  __CARTOON 
{
	int	type;
	int	tick;
	int	cur_tick;
	int	step_tick;
}TCartoon, *PCartoon;
extern	TCartoon    gCartoon;

typedef	struct __VRY_BIND
{
	U8	vry_cnt;
	U8	old_mth;
	U8	mth;	
	U16 	old_pin;	
	U16	pin;							
}TVryBind, *PVryBind;
extern	TVryBind VryBind;

#define	STA_DFT     0
#define	STA_DIALOG  1
#define	STA_VRY_END 2
#define	STA_NOR_OPEN 4
#define	STA_COMM    8
typedef	struct	__L3000STA
{
	int	voltage;
	int	pwr_bak;
	int	vry_unlock;
	int dis_vry_tick;
	int	state;
}TL3000Sta;
extern	TL3000Sta     gState;
/********************************************************************************/

extern	int	L3000LowClk;
//extern  int	L3000VryFailCnt;



typedef	struct	_LOCK_CTL
{
//	int 	  cur_state;  //current lock state, 0:off, 1:on
	U32       open_ms;
	U32	  close_ms;
	U32	  open_delay_ms;
	int       nor_open;  // current lock is nor open state?
        int	  nor_open_key ;	
	
}TLockCtl;

typedef struct _BEEP_CTL
{
	U32   on_ms;
	U32   off_ms;
	U32   cycle;   //0: ÎÞ£¬<<0XFF:times£»  OXFF: always; 
	U32   cycle_on_ms;
	U32   cycle_off_ms;
}TBeepCtl;

/////////////
#define		PWD_LEN_MAX      10
#define		PWD_LEN_MIN      6
typedef	struct	_INPWD
{
	int	bits;
	char buf[PWD_LEN_MAX];
	char pwd[5];
					
}TInputPwd, *PInputPwd;
extern	TInputPwd VryPwd;
extern	int	L3000InputPassward(PInputPwd pwd, char key);
extern	int	L3000ProcInputPwd(U16 *pin, PInputPwd pwd);
extern	void	L3000BoxToPwd(char *buf, char *pwd);
extern	PUser	L3000SearchUserByPwd(char *pwd);
extern	int	L3000MaxPwdCnt;


/////////////////
extern	TLockCtl gLock;
extern	TBeepCtl gBeep;
extern	TBeepCtl gRLed;
extern	TBeepCtl gGLed;

extern  int	EnFPVry ;
extern	int	VryFailCnt;
extern	int	gUserOperateMaxTime;
extern	int	gUserOperateTime;
extern	int	gShowLowPwrAlm;
extern	void	L3000PortInit(void);
extern	void	L3000OperateInit(void);
extern	void	L3000ExRLed(int	ms);
extern	void	L3000ExGLed(int	ms);
extern	void	L3000RunRLed(U32	light_ms, U32	interval_ms,  U32	cycle);
extern	void	L3000RunGLed(U32	light_ms, U32	interval_ms,  U32	cycle);
extern	void	L3000ExBeep(int	ms);
extern	void	L3000RunBeep(U32	tweet_ms, U32	interval_ms,  U32	cycle);
extern	void	L3000ExLock(int	onoff);
extern	int	L3000RunLock(int	onoff);
extern	void    L3000ProcTickMsg(PMsg msg);
extern	void	L3000ProcSecondMsg(PMsg msg);



extern	void	L3000ProcUnLock(int	method);
extern	void	L3000ShowInputPwd(PInputPwd pwd);
extern	void	L3000ShowMainLcd(int	show_first_line);
extern	void	L3000ShowVrySuccess(PUser pu, int method, char *s);
extern  void	L3000ShowVerifyInValid(int method);
extern  void	L3000ShowFingerTouch();
extern	void	L3000ShowWarn(char *content1, char *content2, int now_dly, int after_dly);





extern	int	L3000SetNumberAt(int row,int col, int width, int *number, int minv, int maxv);
extern	void pll_init(int type);

//extern   int     L3000ShowUnlockLogPage(PDataViewer dv);
extern   int     L3000RunBrowseMsg(PMsg msg);
extern	 void	 L3000CheckVryFailCnt();	
#endif
