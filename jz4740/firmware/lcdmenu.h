/*************************************************
                                           
 ZEM 200                                          
                                                    
 lcdmenu.h the header file for menu                               
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef _LCDMENU_H_
#define _LCDMENU_H_

#include "msg.h"
#include "flashdb.h"

#define InputTimeOutSec	gOptions.TimeOutMenu	//MENU display times
extern int InputTimeOut;

#define RowCount gLCDRowCount

typedef struct Menu *PMenu;
typedef struct MenuItem *PMenuItem;

typedef int (*ActionFunc)(void *p);
typedef int (*ValidValue)(char *Value);

typedef struct MenuItem{		//菜单单元结构
    char *Caption;		//显示的字符串
	char *TitleItem;		//选择项           |
	int *ValueItem;			//选择项对应值     |
	int *ValueField;		//当前值           |--现场编辑支持，
	int ValueWidth;			//值显示宽度       |
	int MinValue;			//可输入最小值     |
	int MaxValue;			//可输入最大值     |选择项数量
	ValidValue ValidFun;	//输入值的验证函数 |
    char MemStyle;		//whether malloc memory for Caption 
    unsigned char *Icon;  //动态图标
    ActionFunc Action;		//选中后执行
    PMenu SubMenu;		//选中后执行的子菜单,==0无
} TMenuItem;

typedef struct Menu{
	int ItemIndex;
	int Count;
	int OldCount; //treckle
	int TopIndex;
	U8 Style;
	PMenuItem Items;
	PMenu Parents;
	char *Title;
    	char MenuItmeLoaded;	//treckle
}TMenu;

#define MenuStyle_NEW 0
#define MenuStyle_OLD	1
#define MenuStyle_STD	2
#define MenuStyle_ICON  3


#define InputStyle_Line 0	//直接输入数据 input text directly
#define InputStyle_Select 1	//直接选择数据 select text directly
#define InputStyle_Number 2	//输入数值 input digital value
#define InputStyle_ANumber 3	//输入对齐的数值 input aligned digital value
#define InputStyle_Number2 4	//输入数值 input digital value no keyup or keydown

#define MenuIndicatorWidth      1
#define MenuCharWidth   (gLCDCharWidth-MenuIndicatorWidth)

#define Alignment_Auto	0
#define Alignment_Left	1
#define Alignment_Right 2
#define Alignment_Center 3

typedef struct InputBox{
	U8 Row, Col;		//显示坐标位置
	U8 Width;		//显示长度
	U8 MaxLength;		//最大输入长度
	U8 Style;		//输入器风格
	U8 Alignment;		//对齐方式
	char **Items;		//可供选择项目
	int ItemCount;		//可供选择项目个数
	int MinValue;		//最小数值
	int MaxValue;		//最大数值
	ValidValue ValidFun;	//输入值的验证函数 validate function for input value
	U8 PasswordChar;
	char Text[40];		//实际输入
	U8 TopIndex;		//起始显示字符序号
	U8 SelectStart;		//当前插入点位置
	U8 SelectLength;		//选择区长度
	U8 AutoCommit;
	U8 AllowNav;
}TInputBox, *PInputBox;

void ShowMenu(PMenu menu);
int RunMenu(PMenu menu);
int CalcMenuItemOffset(PMenu menu, int index);

PMenu CreateMenu(char *Title, int Style, PMenu Parents, PMenu menu);
int DestroyMenu(PMenu menu);
PMenuItem AddMenuItem(char MemStyle, PMenu menu, char *Caption, ActionFunc Action, PMenu SubMenu);

void ShowInput(PInputBox box);
int RunInput(PInputBox box);

//Input a string
int InputLine(int row, int col, int width, char *text);
//input a number between minv and maxv
int InputNumber(int row, int col, int width, int *value, int minv, int maxv, int nav);
//Repeat input a number until no error
int RepeatInputNumber(int row,int col, int width, int *number, int minv, int maxv);
int InputNumberAt(int row,int col, int width, int *number, int minv, int maxv);
int InputTextNumber(int row, int col, int width, int *value, int minv, int maxv, U8 style);

#define INPUT_USER_NEW	1
#define INPUT_USER_PIN2	2
#define INPUT_USER_CARD	4

//input a user
int InputUNumber(int row, int col, int width, int flag, void *user);
int InputHIDCardNumber(char *title, char *hint, int flag, void *u);
//Show a full screen to input user
int InputPINBox(char *title, char *hint, int IsNew, void *user);
int ShowUserHint(int row, int width, U32 pin);
char *FormatPin(char *buf, void *user, int FingerID, int FPEnrolled, int PwdEnrolled);

void LCDInfo(char *info, int DelaySec);
void LCDInfoShow(char *title, char *info);
int LCDSelectOK(char *title, char *info, char *hint);
int LCDSelectItem(int row, int col, int width, char **items, int itemcount, int *index);
int LCDSelectItemValue(int row, int col, int width, char *items, int *values, int *value);

char* MenuFmtStr(char *buf, int StrID, char *Value);
char* MenuFmtInt(char *buf, int StrID, int Value);
char *MenuFmtStrStr(char *buf, int width, char *Value);
char *MenuFmtStrInt(char *buf, int width, int Value);
int InputWCNumber(int row, int col,int width,int *workcode);

//登记指纹进度条显示使用
extern unsigned char Icon_Stop[];
extern unsigned char Icon_Information[];
extern unsigned char Icon_Warning[];
extern unsigned char Icon_Question[];

#endif
