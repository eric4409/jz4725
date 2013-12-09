/*************************************************
                                           
 DZEM 200                                          
                                                    
 options.h header file for options                               
                                                      
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
                                                      
 $Log: options.h,v $
 Revision 5.19  2006/03/04 17:30:09  david
 Add multi-language function

 Revision 5.18  2005/12/22 08:54:23  david
 Add workcode and PIN2 support

 Revision 5.17  2005/11/06 02:41:34  david
 Fixed RTC Bug(Synchronize time per hour)

 Revision 5.16  2005/09/19 10:01:59  david
 Add AuthServer Function

 Revision 5.15  2005/08/13 13:26:14  david
 Fixed some minor bugs and Modify schedule bell

 Revision 5.14  2005/08/04 15:42:53  david
 Add Wiegand 26 Output&Fixed some minor bug

 Revision 5.13  2005/08/02 16:07:51  david
 Add Mifare function&Duress function

 Revision 5.12  2005/07/14 16:59:50  david
 Add update firmware by SDK and U-disk

 Revision 5.11  2005/07/07 08:09:05  david
 Fixed AuthServer&Add remote register

 Revision 5.10  2005/06/29 20:21:46  david
 Add MultiAuthServer Support

 Revision 5.9  2005/06/16 23:27:49  david
 Add AuthServer function

 Revision 5.8  2005/06/10 17:10:59  david
 support tcp connection

 Revision 5.7  2005/06/02 20:11:09  david
 Fixed SMS bugs and Add Power Button Control function

 Revision 5.6  2005/05/20 23:41:04  david
 Add USB support for SMS

 Revision 5.5  2005/04/27 00:15:34  david
 Fixed Some Bugs

 Revision 5.4  2005/04/24 11:11:28  david
 Add advanced access control function

 Revision 5.3  2005/04/07 17:01:43  david
 Modify to support A&C and 2 row LCD

*************************************************/

#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include "arca.h" 

#define STA_IDLE	0	//	等待状态；
#define	STA_ENROLLING	1	//	登记指纹状态；
#define	STA_VERIFYING	2	//	识别指纹状态；
#define	STA_MENU	3	//	执行人机界面菜单
#define	STA_BUSY	4	//	正忙于处理其他工作
#define STA_WRITEMIFARE 5       // 	等待写卡状态

#define VALUE_BUFFERLEN 	1024
#define VALUE_BUFFERCACHE 	4096
#define MAX_OPTION_SIZE		8*1024
#define MAX_AUTOALARM_CNT 	8

extern int gMachineState;

#define MAINVERSION "Ver 6.60 "__DATE__
//#define MAINVERSION "Ver 6.20 Sep 12 2008" 
#define DeviceVender "ZKSoftware Inc."
#define AuthorName "Chen Shukai"
#define ProductTime (LoadStrOld("~ProductTime")?LoadStrOld("~ProductTime"):"2004-01-01 20:20:20")
#define SerialNumber (LoadStrOld("~SerialNumber")?LoadStrOld("~SerialNumber"):"0")
#define OEMVendor (LoadStrOld("~OEMVendor")?LoadStrOld("~OEMVendor"):DeviceVender)
#define DeviceName (LoadStrOld("~DeviceName")?LoadStrOld("~DeviceName"):"A5")
#define AlgVer (LoadStrOld("~AlgVer")?LoadStrOld("~AlgVer"):"ZKF2004-5.04")

/*
L/_XXX_=RRR

        L为多国语言资源标识符，用于定义一种语言字符串资源。不同的语言使用不同的标识符
作为一个字符串的引导字符。其范围是：0x21~0x7E 中，除去["],[/],[=]三个字符以外的92
个字符；XXX 是字符串编号；RRR 是对应的字符串。

下面是已经分配的标识符
*/

#define LanguageTraditionalChinese 	'T'  //BIG5内码
#define LanguageSimplifiedChinese 	'S'
#define LanguageEnglish			'E'
#define LanguageThailand		'L'
#define LanguageIndonesian 		'I'
#define LanguageJapanese 		'J'
#define LanguageKorean 			'K'
#define LanguageMalaysia 		'M'
#define LanguageVietnamese 		'V'
#define LanguageTurkish			't'

//字符串编号表
#define MSG_LANG_ID 		0       //该资源的语言内码编号，参见"locale.h"
#define MSG_LANG_NAME           1       //该资源的语言名称，<=4 BYTE
#define MSG_LANG_NAME_ENG       2       //该资源的语言名称的英文表示，<=4 BYTE
#define MSG_LANG_REPLACE        3       //该语言的首要替换语言。

//LCD Hint messages
#define HINT_START	10
#define HID_SYSTEM	(HINT_START+1)		//"指纹考勤系统"
#define HID_PLACEFINGER	(HINT_START+2)		//"请按手指"
#define HID_MATCHING	(HINT_START+3)		//"比对指纹……"
#define HID_MSUCCESS	(HINT_START+4)		//"成功：%s"
#define HID_TRYAGAIN	(HINT_START+5)		//"请再试一次！"
#define HID_LOWQLT	(HINT_START+6)		//"指纹质量不够！"
#define HID_SYSINFO	(HINT_START+7)		//"人数 指纹   考勤"
#define HID_ENROLL	(HINT_START+8)		//"登记指纹"
#define HID_PIN2	(HINT_START+9)		//"号码"
#define HID_PLACECNT	(HINT_START+10)		//"第 %d 次"
#define HID_FINGEREXISTS	(HINT_START+11)
#define HID_ENROLLFAIL	(HINT_START+12)
#define HID_ENROLLOK	(HINT_START+13)
#define HID_NOSPACE	(HINT_START+14)
#define HID_UIDERROR	(HINT_START+15)		//"错误！"
#define HID_YESNO	(HINT_START+16)		//"否ESC   是OK"
#define HID_SAVECANCEL	(HINT_START+17)		//"无效ESC  保存OK"
#define HID_OKCANCEL	(HINT_START+18)		//退出 ESC  设置OK
#define HID_WELCOME	(HINT_START+19)		//"欢迎"
#define HID_DAY0	(HINT_START+20)		//"星期日"
#define HID_DAY1	(HINT_START+21)		//"星期一"
#define HID_DAY2	(HINT_START+22)		//"星期二"
#define HID_DAY3	(HINT_START+23)		//"星期三"
#define HID_DAY4	(HINT_START+24)		//"星期四"
#define HID_DAY5	(HINT_START+25)		//"星期五"
#define HID_DAY6	(HINT_START+26)		//"星期六"
#define HID_VADMIN	(HINT_START+27)		//"管理者确认"
#define HID_ENROLLNEWQ	(HINT_START+28)		//"新登记？"
#define HID_ENROLLNEW	(HINT_START+29)		//"新登记"
#define HID_ENROLLNUM	(HINT_START+30)		//"登记号码"
#define HID_LEAVEFINGER	(HINT_START+31)		//"请离开手指"
#define HID_PLACEFINGER2	(HINT_START+32)	//"请第二次按手指"
#define HID_PLACEFINGER3	(HINT_START+33)	//"请第三次按手指"
#define HID_FINGERIDFMT	(HINT_START+34)		//"%05d-%d"
#define HID_CONTINUE	(HINT_START+35)		//"继续？"
#define HID_UIDFMT	(HINT_START+36)		//"%05d"
#define HID_ENROLLBACKUP	(HINT_START+37)	//"备份登记"
#define HID_USERPWDFMT	(HINT_START+38)		//"%05d-P"
#define HID_INPUTPWD	(HINT_START+39)		//"输入密码"
#define HID_VERINPUTPWD	(HINT_START+40)		//"密码确认"
#define HID_ADMINPRIV	(HINT_START+41)		//"管理者授权"
#define HID_VERDELETE	(HINT_START+42)		//"删除？"
#define HID_NOTENROLLED	(HINT_START+43)		//"无登记数据!"
#define HID_NOATTLOG	(HINT_START+44)		//"无考勤记录"
#define HID_HINTTIME	(HINT_START+45)		//"设置ESC  上一个OK"
#define HID_WAITING	(HINT_START+46)		//"工作中，请稍等"
#define HID_ERRORPIN	(HINT_START+47)		//"登记号码出错"
#define HID_REFINGER	(HINT_START+48)		//"指纹重复"
#define HID_INPUTAGAIN	(HINT_START+49)		//"请重新输入！"
#define HID_LNGSCH	(HINT_START+50)		//"简体"
#define HID_LNGTCH	(HINT_START+51)		//"繁体"
#define HID_LNGENG	(HINT_START+52)		//"ENG"
#define HID_YES		(HINT_START+53)		//"是"
#define HID_NO		(HINT_START+54)		//"否"
#define HID_NOTHING	(HINT_START+55)		//"无"
#define HID_THANK	(HINT_START+56)		//"谢谢！"
#define HID_ALREADY	(HINT_START+57)		//"已签到，谢谢！"
#define HID_AGAIN	(HINT_START+58)		//"请重新输入！"
#define HID_EXCEEDFINGER	(HINT_START+59)	//"已登记10枚指纹"
#define HID_SCIN	(HINT_START+60)		//"上班签到
#define HID_SCOUT	(HINT_START+61)		//"下班签退"
#define HID_SOUT	(HINT_START+62)		//"外出"
#define HID_SBACK	(HINT_START+63)		//"外出返回"
#define HID_SOCIN	(HINT_START+64)		//"加班签到"
#define HID_SOCOUT	(HINT_START+65)		//"加班签退"
#define HID_SOPENLOCK	(HINT_START+66)		//"开门"
#define HID_SVERIFY	(HINT_START+67)		//"验证"
#define HID_VF	(HINT_START+68)			//"指纹确认"
#define HID_VFFAIL	(HINT_START+69)		//"请重新按手指！"
#define HID_VP	(HINT_START+70)			//"密码确认"
#define HID_VPFAIL	(HINT_START+71)		//"密码错误！"
#define HID_VREP	(HINT_START+72)		//"已确认！"
#define HID_CONTINUEESC	(HINT_START+73)		//"退出ESC 继续OK"
#define HID_SAVEQ	(HINT_START+74)		//"保存？"
#define HID_VSUCCESS	(HINT_START+75)		//"确认成功！"
#define HID_ACCESSDENY	(HINT_START+76)		//"非法管理！"
#define HID_POWEROFF (HINT_START+77)		//"关机\t%s"
#define HID_SUSPEND (HINT_START+78)		//"待机\t%s"
#define HID_ESC	(HINT_START+79)			//"退出ESC"
#define HID_TSTLCD	(HINT_START+80)		//当前LCD
#define HID_TSTLCD_LF	(HINT_START+81)		//左屏充满
#define HID_TSTLCD_RF	(HINT_START+82)		//右屏充满
#define HID_TSTLCD_UF	(HINT_START+83)		//上屏充满
#define HID_TSTLCD_DF	(HINT_START+84)		//下屏充满
#define HID_TSTLCD_LE	(HINT_START+85)		//左屏空
#define HID_TSTLCD_RE	(HINT_START+86)		//右屏空
#define HID_TSTLCD_UE	(HINT_START+87)		//上屏空
#define HID_TSTLCD_DE	(HINT_START+88)		//下屏空
#define HID_CONTINUEOK	(HINT_START+89)		//继续OK
#define HID_KEYNAME_MENU (HINT_START+90)	//MENU
#define HID_KEYNAME_OK	(HINT_START+91)		//OK
#define HID_KEYNAME_ECS	(HINT_START+92)		//ESC
#define HID_KEYNAME_UP	(HINT_START+93)		//上箭头
#define HID_KEYNAME_DOWN	(HINT_START+94)	//下箭头
//#define HID_KEYNAME_LEFT	(HINT_START+95)	//左箭头
//#define HID_KEYNAME_RIGHT	(HINT_START+96)	//右箭头
#define HID_KEYNAME_POWER	(HINT_START+95)	//按下电源
#define HID_KEYNAME_ANYKEY	(HINT_START+99)	//按任意键显示
#define HID_KEYNAME_NUMBER	(HINT_START+100)//数字
#define HID_PLAYVOICEINDEX  (HINT_START+101)	//播放第 %d 段声音
#define HID_TEST_FLASH_CQ1   (HINT_START+102)	//测试FLASH时请
#define HID_TEST_FLASH_CQ2   (HINT_START+103)	//不要按电源按钮
#define HID_TESTING   (HINT_START+104)		//正在测试……
#define HID_TEST_FLASH_RES   (HINT_START+105)	//总数:%d坏块:%d
#define HID_FINISH   (HINT_START+106)		//完成！
#define HID_TEST_OK    (HINT_START+107)		//正常！
#define HID_TEST_BAD    (HINT_START+108)	//不能正常工作！
#define HID_PREPOWEROFF    (HINT_START+109)	//准备关机……
#define HID_PRI_NONE   (HINT_START+110)		//普通用户
#define HID_PRI_ENROLL    (HINT_START+111)	//登记员
#define HID_PRI_ADMIN    (HINT_START+112)	//管理员
#define HID_PRI_SUPERVISOR    (HINT_START+113)	//超级管理员
#define HID_PRI_INVALID   (HINT_START+114)	//禁止用户
#define HID_PRI   (HINT_START+115)		//权限
#define HID_1TO1   (HINT_START+116)		//1:1 Verifying
#define HID_EXCEED	(HINT_START+117)	//Exceed space
#define HID_LEFTSPACE (HINT_START+118)		//left space %d
#define HID_OKPWD (HINT_START+119) 		//OK for pwd
#define HID_OTAPOWER (HINT_START+120) 		//设置定时功能吗？
#define HID_DEL_FP (HINT_START+121)		//Delete Fingerprint
#define HID_DEL_PWD (HINT_START+122) 		//Delete Password
#define HID_DEL_USR (HINT_START+123) 		//Delete User
#define HID_WORKING	(HINT_START+124)	//工作中....
#define HID_CARDFP	(HINT_START+125)	//比对卡上指纹
#define HID_INVALIDCARD	(HINT_START+126)	//无效卡
#define HID_CANCELKEY	(HINT_START+127)	//任意键取消
#define HID_SHOWCARD	(HINT_START+128)	//请出示卡
#define HID_WRITE_OK	(HINT_START+129)	//写卡成功
#define HID_WRITE_ERROR	(HINT_START+130)	//写卡失败
#define HID_NOFINGER	(HINT_START+131)	//未登记指纹
#define HID_FLASHERROR	(HINT_START+132)	//存储器错误
#define HID_CARD	(HINT_START+133)	//卡验证
#define HID_MUSTINOUT	(HINT_START+134)	//必须选择进出状态
#define HID_TZDEF	(HINT_START+135)	//时间段%d定义
#define HID_GTZ	(HINT_START+136)		//组%d默认时间段
#define HID_ULG	(HINT_START+137)		//组合%d
#define HID_TZI	(HINT_START+138)		//时间段%d
#define HID_UAOPT	(HINT_START+139)	//用户%05d门禁
#define HID_SHORTWEEK	(HINT_START+140)	//星期的短名称
#define HID_TZNUM	(HINT_START+141)	//时间段编号
#define HID_GRPNUM	(HINT_START+142)	//组编号acc
#define HID_INVALIDTIME (HINT_START+143)	//非法时段访问
#define HID_OS_MUST1TO1 (HINT_START+144)	//必须输入ID
#define HID_NEWENROLLNUM (HINT_START+145)	//新号码
#define HID_CARD_NOTENROLLED (HINT_START+146)	//卡未登记
#define HID_CARD_ENROLLED (HINT_START+147)	//卡已登记
#define HID_CARD_NO	(HINT_START+148)	//卡号:
#define HID_AUTO_STATE	(HINT_START+149)
#define HID_NUM	(HINT_START+150)
#define HID_STATE	(HINT_START+151)
#define HID_LNGTHAI	(HINT_START+152)	//泰语
#define HID_SHUTDOWNING (HINT_START+153)        //%s 秒后关机
#define HID_SUSPENDING (HINT_START+154) 	//%s 秒后待机
#define HID_ALARM_STRIPE (HINT_START+155)       //机器被拆除
#define HID_ALARM_DOOR (HINT_START+156)        	//门被意外打开
#define HID_ALARM_LOWBATTERY (HINT_START+157) 	//电池电压过低
#define HID_ALARM_INDOOROPEN (HINT_START+158)   //出门开关开门
#define HID_DSM (HINT_START+159)  		//门磁开关模式：常开;常闭;无
#define HID_DOOR_OPEN (HINT_START+160)  	//门已打开
#define HID_DOOR_CLOSE (HINT_START+161) 	//门已关闭
#define HID_MUSER_OPEN1 (HINT_START+162)        //多用户进入
#define HID_MUSER_OPEN2 (HINT_START+163)        //非法分组组合
#define HID_RESTART (HINT_START+164)        	//重新启动才能生效
#define HID_UNREGALL (HINT_START+165)   	//全部清除吗
#define HID_LNGINDON (HINT_START+166)   	//印尼语
#define HID_TSTLCD_1 (HINT_START+167) 	//LCD测试第一屏
#define HID_TSTLCD_2 (HINT_START+168) 	//LCD测试第一屏
#define HID_1TOG     (HINT_START+169) 	//1:G比对
#define HID_INVALIDWORKCODE (HINT_START+170) //workcode 无效
//#define HID_BAK_PWR         (10+173) //Backup Power    后备电源
#define	HID_PLS_SET_DATE		(10+174) //Pls Set Date    请设置时间
#define	HID_RTC_ERR			(10+175) //Invalid Date    日期错误 
//#define HID_NORMAL_OPEN         (10+176) //Normal Open     常开
#define HID_WARNING         (10+178) //Warning         非法操作
#define HID_BACK_ESC                (10+179) //Back ESC        ESC返回
#define HID_CHECK_DATE              (10+180) //Check Date...   检查日期...
#define HID_YES_OK              (10+181) //Yes OK          是 OK
#define HID_LOCK                (10+182) //Lock            上锁
#define HID_NORMAL_OPEN_M       (10+183) //Normal Open M   ESC退出  OK常开
#define HID_PASSWARD_FULL       (10+184) //Passward Full   密码已满
#define HID_LOCK_M              (10+185) //Lock M          ESC退出  OK上锁
#define HID_FIRST_VERIFY        (10+186) //First Verify    第一次比对
#define HID_REPEAT_VERIFY       (10+187) //Repeat Verify   重复验证
#define HID_USER_FULL           (10+188) //user full       用户已满
#define HID_PAFFIRM             (10+189) //Affirm          确认

#define MENU_START 200
#define MID_MENU (MENU_START+1)	//"菜单"
#define MID_DATA (MENU_START+2)	//"数据管理"
#define MID_HIDE (MENU_START+3)	
#define MID_OPTIONS (MENU_START+4)	//"设置"
#define MID_SYSINFO (MENU_START+5)	//"系统信息"
#define MID_DATA_EUSER (MENU_START+6)	//"用户登记"
#define MID_DATA_EADMIN (MENU_START+7)	//"管理者登记"
#define MID_DATA_DEL (MENU_START+8)		//"删除登记数据"
#define MID_DATA_EU_FINGER (MENU_START+9)	//"指纹登记"
#define MID_DATA_EU_PWD (MENU_START+10)		//"密码登记"
#define MID_DATA_EU_FP (MENU_START+11)		//"指纹及密码"
#define MID_OPTIONS_SYSTEM (MENU_START+12)	//"系统设置"
#define MID_OPTIONS_REC (MENU_START+13)		//"记录设置"
#define MID_OPTIONS_COMM (MENU_START+14)	//"通讯设置"
#define MID_OS_ADMINNUMBER (MENU_START+15)	//"管理者总数\t%d"
#define MID_OS_DEVNUMBER (MENU_START+16)	//"机号\t%d"
#define MID_OS_TIME (MENU_START+17)		//"时间设置"
#define MID_OS_LANGUAGE (MENU_START+18)		//"语言\t%s"
#define MID_OS_LOCK (MENU_START+19)		//"锁驱动\t%s"
#define MID_OS_AUTOPOWER (MENU_START+20)	//"自动电源管理\t%s"
#define MID_OR_AADMINLOG (MENU_START+21)	//"管理记录警告\t%d"
#define MID_OR_AATTLOG (MENU_START+22)		//"考勤记录警告\t%d"
#define MID_OR_REREC (MENU_START+23)		//"重复确认时间\t%d"
#define MID_OC_BAUDRATE (MENU_START+24)		//"波特率\t%d"
#define MID_OC_CRC (MENU_START+25)		//"奇偶校验\t%s"
#define MID_OC_STOP (MENU_START+26)		//"停止位\t%d"
#define MID_SYSINFO_USER (MENU_START+27)	//"用户登记\t%d"
#define MID_SYSINFO_FINGER (MENU_START+28)	//"指纹登记\t%d"
//#define MID_SYSINFO_ATTLOG (MENU_START+29)	//"考勤记录\t%d"
#define MID_SYSINFO_ADMIN (MENU_START+30)	//"管理者登记\t%d"
#define MID_SYSINFO_PWD (MENU_START+31)		//"密码登记\t%d"
//#define MID_SYSINFO_ADMINLOG (MENU_START+32)	//"管理记录\t%d"
#define MID_DATA_VATTLOG (MENU_START+33)	//"查看考勤记录"
#define MID_DATA_VADMINLOG (MENU_START+34)	//"查看管理记录"
#define MID_DATA_DELLOG (MENU_START+35)		//"删除全部记录"
#define MID_NET_UDPPORT (MENU_START+36)		//"服务端口"
#define MID_NET_WEBPORT (MENU_START+37)		//"Web端口"
#define MID_NET_MAC (MENU_START+38)		//"MAC地址"
#define MID_NET_IP (MENU_START+39)		//"IP地址"
#define MID_OPTIONS_NET (MENU_START+40)		//"网络设置"
#define MID_OSA_POWEROFF (MENU_START+41)	//"关机\t%s"
#define MID_OSA_SUSPEND (MENU_START+42)	//"待机\t%s"
#define MID_OSA_POWERON (MENU_START+43)	//"开机\t%s"
#define MID_OSA_IDLE (MENU_START+44)	//"空闲设置\t%s
#define MID_OSA_IDLETIME (MENU_START+45)	//"空闲时间\t%s"
#define HMID_SHOWSCORE (MENU_START+46)	//显示分数
#define HMID_MATCHSCORE (MENU_START+47) //匹配阀值
#define HMID_NOISETHRESHOLD (MENU_START+48)//噪音阀值
#define HMID_MATCHSPEED (MENU_START+49)	//高速比对
#define HMID_VOICEON (MENU_START+50)	//语音提示
#define HMID_NOFINGER_THRESHOLD (MENU_START+51)		//无指纹阀值
#define HMID_HASFINGER_THRESHOLD (MENU_START+52)	//有指纹阀值
#define HMID_TOPFINGER_THRESHOLD (MENU_START+53)	//高指纹阀值
#define MID_AUTOTEST (MENU_START+54)	//自动检测机器
#define MID_AT_FLASH (MENU_START+55)	//FLASH检测
#define MID_AT_LCD (MENU_START+56)	//液晶检查
#define MID_AT_VOICE (MENU_START+57)	//语音检测acc
#define MID_AT_KEYPAD (MENU_START+58)	//键盘检测
#define MID_AT_ALL (MENU_START+59)	//ALL
#define MID_AT_RTC (MENU_START+60)	//实时时钟
#define MID_AT_FINGER (MENU_START+61)	//指纹采集器
#define MID_OC_NETOFF (MENU_START+62)	//禁用以太网络
#define MID_OC_RS232OFF (MENU_START+63)	//禁用RS232通信
#define MID_OC_RS485OFF (MENU_START+64)	//禁用RS485通信
#define MID_OS_ADVANCE (MENU_START+65)	//高级设置
#define MID_OS_INIT	(MENU_START+66)		//出厂设置
#define MID_OI_ENNUM	(MENU_START+67)		//可登记指纹
#define MID_OI_ALNUM	(MENU_START+68)		//可保存记录
#define MID_OI_NET	(MENU_START+69)		//网络功能
#define MID_OI_LOCK	(MENU_START+70)		//锁控功能
#define MID_OI_RFCARD	(MENU_START+71)		//射频卡功能
#define MID_OI_PROTIME	(MENU_START+72)		//出厂时间
#define MID_OI_INITDEV	(MENU_START+73)		//设备初始化
#define MID_CLEAR_DATA	(MENU_START+74)		//Clear Data
#define MID_OS_RESTORE	(MENU_START+75)		//Restore Default Options
#define MID_OS_VERSCORE	(MENU_START+76)		//1:1 Verify Score
#define MID_CLEAR_ADMIN	(MENU_START+77)		//清除管理员
#define MID_INFO_RES	(MENU_START+78)		//剩余容量信息
#define MID_IR_FINGER	(MENU_START+79)		//指纹数AttLog
#define MID_IR_ATTLOG	(MENU_START+80)		//出入记录
#define MID_INFO_DEV 	(MENU_START+81)		//设备信息
#define MID_OI_PT	(MENU_START+82)		//出厂日期
#define MID_OI_SN	(MENU_START+83)		//序列号
#define MID_OI_OEM	(MENU_START+84)		//制造商
#define HMID_TDFINGER_THRESHOLD (MENU_START+85)		//HasFingerThresholdDiff
#define HMID_NEWFPR	(MENU_START+86)		//侧光采集器
#define HMID_DEVTYPE (MENU_START+87)	//机型
#define MID_OI_1TO1 (MENU_START+88)	//1:1功能
#define MID_OI_ATTSTATE (MENU_START+89)	//考勤状态功能
#define MID_OI_SHOWNAME (MENU_START+90)	//显示姓名功能
#define MID_OI_POWERMNG (MENU_START+91)	//电源管理功能
#define MID_OI_PN	(MENU_START+92)	//Device Name
#define MID_OS_LOCKUSERS (MENU_START+93)//unLock Users
#define MID_DC_ENROLL	(MENU_START+94)	//登记指纹卡
#define MID_DC_REG	(MENU_START+95)	//注册指纹卡
#define MID_DC_CREATE	(MENU_START+96)	//生成指纹卡
#define MID_DC_EMPTY	(MENU_START+97)	//清空指纹卡
#define MID_DC_UNREG	(MENU_START+98)	//注销指纹卡
#define MID_DATA_CARD	(MENU_START+99)	//指纹卡管理
#define MID_DC_DUMPFP	(MENU_START+100)//复制卡内指纹
#define MID_DC_MOVEFP	(MENU_START+101)//转移指纹到卡内
#define MID_DC_PIN		(MENU_START+102)//号码卡
#define MID_OC_PINCARD	(MENU_START+103)//仅验证号码卡
#define MID_OI_FWVER	(MENU_START+104)//固件版本号
#define MID_OI_ALGVER	(MENU_START+105)//算法版本号
#define MID_OA_FPKEY	(MENU_START+106)//指纹卡密码
#define MID_OS_CUST	(MENU_START+107)//其他设置
#define MID_OS_HIGHSPEED	(MENU_START+108)//高速网络
#define MID_OA_OPTION	(MENU_START+109)	//门禁功能设置
#define MID_OA_TZDEF	(MENU_START+110)	//时间段定义
#define MID_OA_ULGRP	(MENU_START+111)	//开锁组合定义
#define MID_OA_UAOPT	(MENU_START+112)	//用户门禁设置
#define MID_OA_GTZ	(MENU_START+113)	//组默认时间段
#define MID_OA_GRP	(MENU_START+114)	//所属分组
#define MID_OS_MUST1TO1 (MENU_START+115)	//必须输入ID
#define MID_OS_COMKEY	(MENU_START+116)	//连接密码
#define MID_OC_MUSTENROLL	(MENU_START+117)//禁止未注册卡
#define MID_OA_UDT	(MENU_START+118)//使用组时段
#define MID_CARD_REG	(MENU_START+119)	//射频卡登记
#define MID_CARD_UNREG	(MENU_START+120)	//射频卡注销
#define MID_OSA_ALARM	(MENU_START+121)	//定时响铃
#define MID_ADMIN_REC	(MENU_START+123)	
#define MID_AUTO_STATE	(MENU_START+124)	//定时状态装换
#define MID_AO_IMGCOH 	(MENU_START+125)    //指纹图像清晰度
#define MID_AO_ALARMOFF (MENU_START+126)    //解除报警
#define MID_AC_DSD 	(MENU_START+127)    //门磁延时
#define MID_AC_DSM 	(MENU_START+128)    //门磁开关模式
#define MID_OSA_ALARM_DELAY (MENU_START+129) //
#define MID_OSA_ALARM_NAME (MENU_START+130) //
#define MID_AD_DURESSFINGER (MENU_START+131)    //胁迫指纹管理
#define MID_AD_DURESSHELP (MENU_START+132)      //“~K”键求助 是/否
#define MID_AD_DURESS11 (MENU_START+133)        //1：1方式报警 是/否
#define MID_AD_DURESS1N (MENU_START+134)        //1：N方式报警 是/否
#define MID_AD_DURESSAD (MENU_START+135)        //自动报警时间 0～255秒
#define MID_ADF_ENROLL (MENU_START+136) 	//新登记胁迫指纹
#define MID_ADF_REG (MENU_START+137)    	//胁迫指纹定义
#define MID_ADF_UNREG (MENU_START+138)  	//取消胁迫指纹
#define MID_ADF_UNREGALL (MENU_START+139)       //取消全部
#define MID_AD_DURESS (MENU_START+140)  	//胁迫报警
#define MID_AD_DURESSPWD (MENU_START+141)       //PWD方式报警 是/否
#define MID_LOCK_POWER	(MENU_START+142)    	//LOCK POWER BUTTON
#define MID_POWER_OFF	(MENU_START+143)    	//Shut Down
#define MID_OS_1TON_FROM (MENU_START+144)       //1:N From
#define MID_OS_1TON_TO 	(MENU_START+145) 	//1:N To
#define MID_OS_1TOH 	(MENU_START+146) 	//"S/_346_=前部分ID号",
#define MID_OS_1TOG 	(MENU_START+147) 	//"S/_347_=允许1:G",
#define MID_OI_1TON 	(MENU_START+148) 	//"S/_348_=1：N指纹",
#define	MID_L3000_TEST      (MENU_START+201)
#define MID_L3000_LT        (MENU_START+202)
#define MID_NO_ADMIN        (MENU_START+203)
#define	MID_PLS_CLOSE_DOOR       (MENU_START+205)
#define	MID_CLOSEDR_HT       (MENU_START+206)

#define ZEM200_START 500
#define MID_WORKCODE	(ZEM200_START+1)	//WORK CODE
#define MID_BUTTONBEEP 	(ZEM200_START+2) 	//button beep
#define MID_AT_MP3	(ZEM200_START+3)	//mp3 hint
#define MID_ADV_VOICETZ	(ZEM200_START+4)	//voice time zone 
#define MID_OSA_WEBSERVERIP (ZEM200_START+5) 	//web server ip
#define MID_ADV_AUDIOVOL (ZEM200_START+6)	//adjust audio volume
#define HID_PLUGPENDRIVE (ZEM200_START+7)	//PLS PLUG PEN DRIVE
#define HID_DOWNLOADING_DATA (ZEM200_START+8)	//Downloading DATA
#define HID_PENDRIVE_NOEXIST (ZEM200_START+9)	//PEN DRIVER NO EXIST
#define HID_COPYDATA_FAILURE (ZEM200_START+10)	//COPY DATA FAILURE
#define HID_COPYDATA_SUCCEED (ZEM200_START+11)	//COPY DATA SUCCEED
#define HID_MOUNTING_PENDRV (ZEM200_START+12)	//Mounting PenDrive
#define HID_FIRMWARE_ITEM (ZEM200_START+13)	//Firmware item(files)
#define HID_SELECT_ITEM (ZEM200_START+14)	//select item for update
#define HID_PLACEFINGER4 (ZEM200_START+15)	//"请第四次按手指"
#define HID_CONTINUECANCEL (ZEM200_START+16)	//退出ESC 继续OK
#define HID_CHGPWD (ZEM200_START+17)		//change password
#define HID_CHGCARDNUM (ZEM200_START+18) 	//change card number
//#define MID_DOWNLOAD_ATT (ZEM200_START+19)	//download attlog
//#define MID_DOWNLOAD_USR (ZEM200_START+20)	//download usr info
//#define MID_UPLOAD_USR	(ZEM200_START+21)	//upload usr info
#define MID_PENDRV_MNG	(ZEM200_START+22)	//pen drive manage
#define MID_GATEWAY_IP	(ZEM200_START+23)	//gateway
#define MID_NETMASK_ADDR (ZEM200_START+24)	//Network mask
#define MID_OI_USERNUM	(ZEM200_START+25)	//Users
#define MID_TWOSENSOR	(ZEM200_START+26)	//Two sensor
#define MID_UPDATE_FIRMWARE (ZEM200_START+27)	//update firmware
#define MID_UPLOAD_SMS 	(ZEM200_START+28)    	//upload SMS
#define MID_DOWNLOAD_SMS (ZEM200_START+29)   	//download SMS
#define MID_MODEM	(ZEM200_START+30) 	//use Modem or not
#define MID_AUTHSERVER	(ZEM200_START+31)    	//use AuthServer or not
#define MID_AUTHSERVER_IP (ZEM200_START+32)    	//AuthServer IP
#define MID_AUTHSERVER_REGISTER	(ZEM200_START+33) 	//REMOTE REGISTER
#define MID_AUTHSERVER_ERROR	(ZEM200_START+34) 	//REMOTE AUTHSERVER ERROR
#define MID_AUTOBELL_DELAY	(ZEM200_START+35) 	//Auto ALARM delay times
#define MID_NET_DHCP		(ZEM200_START+36) 	//Auto ALARM delay times
#define HID_NET_DHCPHINT	(ZEM200_START+37) 	//DHCP Running
#define MID_TIME_SET		(ZEM200_START+38) 	//Set time Manualy
#define MID_TIME_SYNC		(ZEM200_START+39)	//Synchronize time
#define MID_TIME_SERVER		(ZEM200_START+40)	//Time server ip address
#define HID_TIME_SYNCHINT	(ZEM200_START+41)	//Time synchronize hint
#define MID_AD_ERRPRESS		(ZEM200_START+42)	//press finger n times failed to Alarm
#define MID_OA_VERIFYTYPE	(ZEM200_START+43)	//User verify type
#define MID_OA_GVERIFYTYPE	(ZEM200_START+44)	//Group verify type
#define MID_OA_VSHINT		(ZEM200_START+45)	//
#define HID_OA_NOEQUAL		(ZEM200_START+46)	//
#define MID_OA_GRPVS		(ZEM200_START+47)	//whether use Group Verify Type or not
#define MID_PROXY_IP		(ZEM200_START+48)	//proxy server ip address
#define MID_PROXY_PORT		(ZEM200_START+49)	//proxy server port
#define MID_PROXY_SERVER	(ZEM200_START+50)	//PROXY SERVER enable
#define MID_AC_DSAD		(ZEM200_START+51)	//Door sensor alarm delay
#define MID_AC_DLM		(ZEM200_START+52)	//Door alarm MODE
//spanish
#define MID_AUTO_FPOPEN         (ZEM200_START+53)       //FP OpenDoor
#define MID_AUTO_OPENDOOR       (ZEM200_START+54)       //BELL OpenDoor
#define MID_AUTO_TIMES          (ZEM200_START+55)       //AUTOOPENTIMES


#define MID_PRINTERON           (ZEM200_START+63)                //打印机功能
#define MID_PRINTERMODE         (ZEM200_START+64)                //模式

#define MID_WORKCODEMODE	(ZEM200_START+65)		//workecode模式

#define MID_C2CONFAIL		(ZEM200_START+66)	//C2通讯失败
#define MID_C2NOFOUND		(ZEM200_START+67)	//C2不匹配
#define MID_C2LOCKED		(ZEM200_START+67)	//C2锁死
#define MID_UPLOAD_VOICE	(ZEM200_START+68)	//导入语音文件
#define MID_RESTORE_VOICE	(ZEM200_START+69)	//恢复默认语音
#define MID_DISKCLEAN (ZEM200_START+70)		//diskclean
#define MID_PUTBELL (ZEM200_START+71)		//按下门铃健


#define MID_AO_FPS	(ZEM200_START+72)
#define MID_INPUT_GROUP (ZEM200_START+73)
#define MID_GROUPFPOVER (ZEM200_START+74)
#define MID_GROUPFPINFO (ZEM200_START+75)
#define MID_ONEGROUPINFO (ZEM200_START+76)
#define MID_TWOGROUPINFO (ZEM200_START+77)
#define MID_THREEGROUPINFO (ZEM200_START+78)
#define MID_FOURGROUPINFO (ZEM200_START+79)
#define MID_FIVEGROUPINFO (ZEM200_START+80)
#define MID_DEFAULTGROUP (ZEM200_START+81)
#define MID_GROUPFPREACH (ZEM200_START+82)

#define MID_SYSINFO_ATTLOG 		(770)	//"记录"
#define MID_SYSINFO_ADMINLOG	(771)	//"查看记录"
#define MID_OPTION_LOCK 		(772)		//操作设置
#define MID_VRYFAIL_WAIT     	(773)	//非法操作报警
#define	MID_NOR_OPEN_ON     	(774) 	//常开功能
#define	MID_VRYFAIL_WAITCNT    	(775) //S/_775_=失败次数限制    
#define	MID_VRY_BIND       		(776) //S/_776_=验证模式
#define HID_NORMAL_OPEN         (777) //Normal Open     常开
#define HID_BAK_PWR         	(778) //Backup Power    后备电源
#define MID_DOWNLOAD_ATT 		(779)  //download attlog
#define MID_DOWNLOAD_USR 		(780)  //download usr info
#define MID_UPLOAD_USR  		(781)   //upload usr info

#define Nideka_start 600
#define NID_CIN (Nideka_start+0)
#define NID_COUT (Nideka_start+1)
#define NID_WELCOME (Nideka_start+2)
#define NID_IN (Nideka_start+3)
#define NID_OUT (Nideka_start+4)
#define NID_PCARD (Nideka_start+5)
#define NID_FP (Nideka_start+6)
#define NID_PUNCHCARD (Nideka_start+7)
#define NID_IN1 (Nideka_start+8)
#define NID_OUT1 (Nideka_start+9)
#define NID_CIN1 (Nideka_start+10)
#define NID_COUT1 (Nideka_start+11)
#define NID_VFFAIL (Nideka_start+12)//请重按手指
#define NID_NOREC (Nideka_start+13)//无员工记录
#define NID_PSPUTCARD (Nideka_start+14) //请先拍卡
#define NID_LEAVEFINGER (Nideka_start+15)//请离开手指

#define HID_AUTORESTART         1619
#define HID_CHRVER_DELALL       1620

typedef struct __OPTIONS__{
	int Language;					//Language of display messages
	BYTE MAC[6];					//Ethrenet MAC
	BYTE CardKey[6];
	BYTE IPAddress[4];				//Network IP Address
	int DeviceID;					//Device ID for 485 connection
	int MThreshold, EThreshold, VThreshold;		//Fingerprint Matching threshold, Fingerprint Enroll threshold
	unsigned int LastAttLog;			//Last AttLog index
	int UDPPort;					//UDP Port Number for communication
	int OImageWidth, OImageHeight, OTopLine, OLeftLine;  //Original fingerprint size and corner for capturing
	int CPX[4],CPY[4];				//correct distorted image parameters
	int ZF_WIDTH,ZF_HEIGHT;
	int MSpeed;					//fingerprint match speed 0-low, 1-high, 2-auto
	int AttState;
	int MaxUserCount;				//unit is 100
	int MaxAttLogCount;				//unit is 10000
	int MaxFingerCount;				//unit is 100
	int LockOn;					//设置锁控时长
	int AlarmAttLog;
	int AlarmOpLog;
	int AlarmReRec;
	int RS232BaudRate;				//0x23
	int RS232CRC; 
	int RS232Stop;
	int WEBPort;
	int ShowState;
	int KeyLayout;
	int VoiceOn;
	int AutoPowerOff; 				//自动关机时间
	int AutoPowerOn;				//自动开机时间
	int AutoPowerSuspend;				//自动待机时间
	int AutoAlarm[MAX_AUTOALARM_CNT];		//自动响铃时间
	int IdlePower;					//空闲自动待机-1，自动关机-0
	int IdleMinute;					//空闲时间
	int ShowScore;					//show the verification score for fingerprint matching.
	int NetworkOn, RS232On, RS485On;		//是否用以太网络、RS232、RS485功能
	int NetworkFunOn, LockFunOn, RFCardFunOn; 	//是否打开网络/门禁（0-无，1-简单门禁，2-高级门禁）/射频卡功能
	int One2OneFunOn, PowerMngFunOn;	 	//是否打开1:1功能,电源管理功能
	int NewFPReader;
	int ShowName;
	int UnlockPerson;				//同时开锁的人数
	int ShowCheckIn;				//是否显示上下班状态
	int OnlyPINCard;				//仅验证号码卡
	int IsTestMachine;				//是否测试用机器
	int MustChoiceInOut;				//是否必须选择进出状态
	//能够自动测试	int CMOSSensorDevAddr;		//CMOS Sensor Chip IIC Device Address: 0x42-OV7620, 0x22-Hy7131
	int HiSpeedNet;					//100M Ethernet
	int MenuStyle;					//菜单风格
	int CanChangeCardKey;				//是否允许改变卡密码
	int Must1To1;					//是否只允许一对一比对
	int LCDModify;                              	
	int ComKey;					//连接密码
	int MustEnroll;					//必须是注册用户,比对后才能有效。用于别的机器上登记的指纹卡可以不在本机上注册即可使用
	int TimeOutMenu;				//菜单的超时时间
	int TimeOutPin;					//输考勤号码的超时时间
	int TimeOutState;				//考勤状态的超时时间
	int SaveAttLog;					//是否保存验证记录
	int RS232Fun;					//RS232接口功能：0-无；1-完整API通讯；2-简单ID输出
	int IsModule;					//是否模块
	int ShowSecond;					//是否显示秒
	int RFCardSecStart;				//Mifare Card 起始存放指纹数据的扇区
	int RFCardSecLen;				//Mifare Card 存放指纹数据的扇区数
	int RFCardFPC;					//Mifare Card 存放指纹的个数
	int PIN2Width;					//PIN2码的字符宽度 <=5表示不支持PIN2码  2147483647=0x7FFFFFF
	int DateFormat;					//Date Format
	int OPLogMask1;					//
	int OPLogMask2;					//
	int AutoState[4*4];				//自动状态转换的时间
	int DelayCount, IncThr, TopThr, MinThr;		//指纹检测的阀值参数
	int MaxNoiseThr, MinMinutiae, MaxTempLen;	//最大容许指纹图像噪音阀值, 
	int SaveBitmap;					//Save bmp format finger image or not 
	BYTE GATEIPAddress[4];				//Gateway IP Address
	int TCPPort;					//TCP port for communication
	BYTE NetMask[4];				//Network mask address	
	BYTE AuthServerIP[4];				//Identification Server
	int AuthServerEnabled;				//Identification Server enabled 0-only local 1-network->local 2-only network 3-local->network
	int IsOnlyRFMachine;				//Only RF machine not fingerprint
	int IsOnlyOneSensor;				//Only One Sensor
	int OS;						//Current OS 0=NONE 1=LINUX
	int IsWiegandKeyPad;				//use wiegand keypad or not
	int AutoAlarmFunOn;				//Auto alarm function enabled or disable
	int AutoStateFunOn;				//自动状态转换功能
	int EnrollCount;  	                     	//register finger count	
	int OpenDoorDelay;          	              	//delay time after open the door
	int DoorSensorMode;                         	//door sensor 门磁开关方式 0-NO 1-NC 其他表示不检测门磁
	int IsSupportSMS;				//SMS Enable or Disable
	int IsSupportUSBDisk;				//Support USB DISK
	int AdminCnt;                               	//同时验证管理员的个数
	int AutoAlarmDelay;				//Auto Alarm delay time(second)
	int MultiLanguage;				//support select language
	int LockPowerButton;				//Lock Power Button
	int IsSupportModem;				//support modem connection
	int IsConnectModem;				//connect or not
	int IsSupportAuthServer;			//support AuthServer
	int AuthServerTimeOut;				//TimeOut for AuthServer
	int IsSupportMF;				//support mifare
        int DuressHelpKeyOn;                            //“~K”键求助 是/否
        int Duress1To1;                                 //1：1方式报警 是/否
        int Duress1ToN;                                 //1：N方式报警 是/否
        int DuressPwd;                                  //Password方式产生报警
        int DuressAlarmDelay;                   	//自动报警时间 0～255秒
	int AutoAlarmTimes;				//Auto Alarm Times
	int IsACWiegand;				//whether output access control wiegand 26bit
	int DNSCheckInterval;				//Refresh AuthServer List interval times unit:minute
	int AutoUploadAttlog;				//Automate upload Attlog seconds
	int DisableNormalUser;				//Disable normal user
	int KeyPadBeep;					//press key to play beep voice.
	int WorkCode;					//support work code
	int MaxUserFingerCount;				//default=10
	int AttLogExtendFormat;				//Extend attendance logs storage format
	int CompressAttlog;				//whether compress attendance logs or not(only valid for old AttLog format)
	int ASDewarpedImage;				//whether dewarp the image for auth server
	int ProcessImage;				//processing image with light check
	int IsFlashLed;					//whether flash green led or not
	int DisableRootPassword;			//disabled Setup root password by CommKey
	int IsSupportMP3;				//support MP3
	int MifareAsIDCard;				//Mifare Card as ID Card
	int PlayGroupVoice;				//when verified play voice by its group
	int PlayTZVoice;				//when verified play voice by time zone
	BYTE WebServerIP[4];				//Network IP Address
	int AudioVol;					//Audio volumn
	int AutoAlarmAudioVol;				//Auto alarm audio volume
	int DHCP;					//DHCP enable or disable
	int AutoSyncTimeFunOn;				//Synchronize time from remote time server
	int AutoSyncTime;				//Schedule time for synchronize
	BYTE TimeServerIP[4];				//Time Server IP Address
	int CameraFunOn;				//take a photo from camera 
        int StartUpNotify;                              //起动广播，BIT0-BT232, BIT1-NETWORK
	int AdvanceMatch;				//support 1:G 1:H	
        int I1ToNFrom;                                  //1：N From (minimum user PIN for identification)
        int I1ToNTo;                                    //1：N To (maximum user PIN for identification)
        int I1ToH;                                    	//1：H
        int I1ToG; 					//1：G
	int TwoLineLCM;					//TWO LINE SMALL LCM
	int ErrTimes;					//Error times for pressing finger
	int UserExtendFormat;				//extend user data information
	U32 CMOSGC; 					//0-AUTO; 1-255 for RGB
	int RefreshUserData;				//Auto refresh user data from authserver
	int DisableAdminUser;				//Disable administrator verification
	int IClockFunOn;				//iClock functions 
	BYTE ProxyServerIP[4];				//Proxy server ip address
	int ProxyServerPort;				//Proxy server port
	int IsSupportC2;				//support C2 controller or not
	int EnableProxyServer;				//Enable ProxyServer
	int WorkCodeFunOn;				//work code funtion
	int ViewAttlogFunOn;				//View attlog funtion
	int IsInit;
	int Saved;
	int DHCPFunOn;					//DHCP menu visible or invisible
	int OutSensorFunOn;				//outsensorfunction menu visible or invisible
	int SaveAuthServerLog;
	int SetGatewayWaitCount; 			//Wait seconds to retry setup gateway
	int IsSupportHID;				//support iClass
	int DoorSensorTimeout;				//Alarm signal will be raise when door sensor open 
	int DoorAlarmMode;				//door alarm 开关方式 0-NO 1-NC 其他表示不检测
	int PrinterFunOn;                       //是否有打印机功能
	int PrinterOn;                       //打印机功能：0：无；1：输出到串口；2：ESC、Star打印
	int Nideka; 
	int CheckOutWc;//按chekcout 输人workcode	
	int SyncTimeFromAuthServer;  //get time from authserver interval
        int AutoOpenRelayFunOn;//是否显示定时响铃开门功能菜单        
	int FPOpenRelay;        //按指纹是否开门
        int AutoOpenRelay;      //是否启用定时响铃功能         
	int AutoOpenRelayTimes; //定时响铃后几秒开门
	int DefaultGroup;//系统默认组 用于分组比对
	int GroupFpLimit;//单组指纹最大数
	int LimitFpCount;//系统指纹组大数
	int WireLessBell;//无线门铃
	int DevID;	//similar to DeviceName.0 - h1plus, 1 - u100
	int RunMode;	// 0 - factory mode; 1 - release mode
	int ZKFPVersion;
	int MulAlgVer;

	//-- add by chenyy
	int NorOpenOn;
	int MSAnimation;
	int CPUFreq;
	int FlashLED                ;                   //0不闪，1绿灯，2红灯
	int C2FunOn                 ;           //是否支持C2
	int ExternalAC              ;           //外置门禁控制器
	int PageState               ;                   //翻页状态  , 1-四个状态 2-六个状态

	int DaylightSavingTimeFun   ,           //是否显示支持夏令时制,
		DaylightSavingTimeOn    ,           //是否支持夏令时制
		CurTimeMode             ,           //1当前为夏令时，2当前不是夏令时
		DaylightSavingTime      ,           //进入夏令时的时间
		StandardTime            ;           //进入非夏令时的时间

	int MasterSlaveFunOn        ;           //是否支持主从机功能
	int SlaveMustRegester       ;           //从机是否必须注册,0不必注册，1必须注册
	int MasterSlaveOn;                      //0,无,1主机，2从机
	int NoPowerKey              ;           //No power key 0-have power key,1-power key is bell,2-No power key
	int LockPWRButton           ;               //锁定关机按钮
	int SaveUnlockLog           ;               //If Saved the Unlock Log
	int VryFailWait;
	int VryFailWaitCnt;
	int MifareFunOn;
	int VryBind				;			//Verify mode select
	int TagLogDoorAction		;			//tag log as door action that whether user open door.
	int MasterState				;			//主从机通讯中，主机的状态，若主机为0，则从机为1，若主机为1则从机为0，若为-1则主从都为主机当前的状态gOptions.AttState。
	int UnSaveLogDeny;
	int	AntiPassbackFunOn;					//0支持反潜,1不支持反潜
	int AntiPassbackOn;						//0 不反潜;1、出反潜，必须有入才能出；2、入反潜，必须有出才能入；
	int CloseDoorHint;
	int TestFunOn;
	int ExtWGInFunOn			;
	int VoiceFunOn				;					//语音功能
	int PwdBitsMode;
}TOptions, *POptions;

#define optoffset(field) offsetof(TOptions, field)

POptions LoadOptions(POptions opts);
POptions SaveOptions(POptions opts);
POptions GetDefaultOptions(POptions opts);

typedef int (*StrConvertFun)(char *str, BYTE *value);
typedef char *(*StrFormatFun)(char *str, BYTE *value);
//new options
typedef struct __OPTIONSRESSTR__{
	char *OptionName;
	int OptionLong;
	char DefaultValue[256];
	int IsNeedRestoreFactory;
	int Offset;
	StrConvertFun Convertor;
	StrFormatFun Formator;
}TOptionsResStr,*POptionsResStr;
typedef struct __OPTIONSRESINT__{
	char *OptionName;
	int DefaultValue;
	int IsNeedRestoreFactory;
	int Offset;
	int MenuResID;
	int MaxValue;
	int MinValue;
}TOptionsResInt,*POptionsResInt;

#define OPTIONSRESSTRCOUNT		(sizeof(OptionsResStr)/sizeof(OptionsResStr[0]))
extern TOptionsResStr OptionsResStr[];
#define OPTIONSRESINTCOUNT		(sizeof(OptionsResInt)/sizeof(OptionsResInt[0]))
extern TOptionsResInt OptionsResInt[];
POptionsResInt QueryOptResByOffset(int Offset);

BOOL SaveStr(const char *name, const char *value, int SaveTrue);
BOOL LoadStr(const char *name, char *value);
int LoadStrFromFile(int fd, const char *name, char *value, BOOL ExitSign, int offset);
char *LoadStrOld(const char *name);
char* LoadStrByID(int ID); 
char* LoadStrByIDPad(int ID, int Len);
char* LoadStringLng(const char *Name, int LngID);
int LoadInteger(const char *Name, int DefaultValue);
int SaveInteger(const char *Name, int Value);
//从内存串列表中删除指定字符串
U32 PackStrBuffer(char *Buffer, const char *name, int size);
char* GetYesNoName(int Yes);
int InitOptions(void);

extern TOptions gOptions;

#define IsNetSwitch "IsNetSwitch"
#define CustomVoice "CustomVoice"
//2006.11.28
//~Platform=ZEM200
//~Platform=ZEM300
#define PLATFORM "~Platform"
#define IGNORECARDTM "IgNoreCardTm"
#define NOPOWERKEY   "NoPowerKey"
#define FPSENSITIFUNON  "FPSensitivity"

#define MAX_AUTOALARM_COUNT 10 //最多的定时响铃次数
int FormatDate(char *buf, int index, int y, int m, int d);
int FormatDate2(char *buf, int index, int m, int d);
extern char *DateFormats[];

int GetFileLength(int fd);
void TruncOptionAndSaveAs(char *buffer, int size);

void SelectLanguage(char Language);
int GetDefaultLocaleID(void);
char *GetLangName(char LngID);
char GetLangFileType(char Language);
int GetSupportedLang(int *LngID, int MaxLngCnt);

char *GetSMS(int UserID);

int ClearAllACOpt(int All);
int ClearOptionItem(char *name);

BOOL UpdateNetworkInfoByDHCP(char *dhcp);

#ifndef URU
int ReadSensorOptions(POptions opts);
int WriteSensorOptions(POptions opts, int Rewrite);
int EEPROMWriteOpt(BYTE * data, int size, int Rewrite);
#endif

char *GetCardKeyStr(char *Buffer, BYTE *Key);

//----- add by chenyy -----
#define MAX_ENROLL_COUNT	10
#define MAX_TEMPLATE_LEN	2048	//800

#define IKeyMenu 10
#define IKeyOK  11
#define IKeyESC 12
#define IKeyUp  13
#define IKeyDown 14
#define IKeyAvoidNorUnLock  24
#define IKeyNorUnLock       25
#define IKeyDel 23
#define IKeyUpUp            26
#define IKeyDownDown        27

#endif
