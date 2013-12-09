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

#define STA_IDLE	0	//	�ȴ�״̬��
#define	STA_ENROLLING	1	//	�Ǽ�ָ��״̬��
#define	STA_VERIFYING	2	//	ʶ��ָ��״̬��
#define	STA_MENU	3	//	ִ���˻�����˵�
#define	STA_BUSY	4	//	��æ�ڴ�����������
#define STA_WRITEMIFARE 5       // 	�ȴ�д��״̬

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

        LΪ���������Դ��ʶ�������ڶ���һ�������ַ�����Դ����ͬ������ʹ�ò�ͬ�ı�ʶ��
��Ϊһ���ַ����������ַ����䷶Χ�ǣ�0x21~0x7E �У���ȥ["],[/],[=]�����ַ������92
���ַ���XXX ���ַ�����ţ�RRR �Ƕ�Ӧ���ַ�����

�������Ѿ�����ı�ʶ��
*/

#define LanguageTraditionalChinese 	'T'  //BIG5����
#define LanguageSimplifiedChinese 	'S'
#define LanguageEnglish			'E'
#define LanguageThailand		'L'
#define LanguageIndonesian 		'I'
#define LanguageJapanese 		'J'
#define LanguageKorean 			'K'
#define LanguageMalaysia 		'M'
#define LanguageVietnamese 		'V'
#define LanguageTurkish			't'

//�ַ�����ű�
#define MSG_LANG_ID 		0       //����Դ�����������ţ��μ�"locale.h"
#define MSG_LANG_NAME           1       //����Դ���������ƣ�<=4 BYTE
#define MSG_LANG_NAME_ENG       2       //����Դ���������Ƶ�Ӣ�ı�ʾ��<=4 BYTE
#define MSG_LANG_REPLACE        3       //�����Ե���Ҫ�滻���ԡ�

//LCD Hint messages
#define HINT_START	10
#define HID_SYSTEM	(HINT_START+1)		//"ָ�ƿ���ϵͳ"
#define HID_PLACEFINGER	(HINT_START+2)		//"�밴��ָ"
#define HID_MATCHING	(HINT_START+3)		//"�ȶ�ָ�ơ���"
#define HID_MSUCCESS	(HINT_START+4)		//"�ɹ���%s"
#define HID_TRYAGAIN	(HINT_START+5)		//"������һ�Σ�"
#define HID_LOWQLT	(HINT_START+6)		//"ָ������������"
#define HID_SYSINFO	(HINT_START+7)		//"���� ָ��   ����"
#define HID_ENROLL	(HINT_START+8)		//"�Ǽ�ָ��"
#define HID_PIN2	(HINT_START+9)		//"����"
#define HID_PLACECNT	(HINT_START+10)		//"�� %d ��"
#define HID_FINGEREXISTS	(HINT_START+11)
#define HID_ENROLLFAIL	(HINT_START+12)
#define HID_ENROLLOK	(HINT_START+13)
#define HID_NOSPACE	(HINT_START+14)
#define HID_UIDERROR	(HINT_START+15)		//"����"
#define HID_YESNO	(HINT_START+16)		//"��ESC   ��OK"
#define HID_SAVECANCEL	(HINT_START+17)		//"��ЧESC  ����OK"
#define HID_OKCANCEL	(HINT_START+18)		//�˳� ESC  ����OK
#define HID_WELCOME	(HINT_START+19)		//"��ӭ"
#define HID_DAY0	(HINT_START+20)		//"������"
#define HID_DAY1	(HINT_START+21)		//"����һ"
#define HID_DAY2	(HINT_START+22)		//"���ڶ�"
#define HID_DAY3	(HINT_START+23)		//"������"
#define HID_DAY4	(HINT_START+24)		//"������"
#define HID_DAY5	(HINT_START+25)		//"������"
#define HID_DAY6	(HINT_START+26)		//"������"
#define HID_VADMIN	(HINT_START+27)		//"������ȷ��"
#define HID_ENROLLNEWQ	(HINT_START+28)		//"�µǼǣ�"
#define HID_ENROLLNEW	(HINT_START+29)		//"�µǼ�"
#define HID_ENROLLNUM	(HINT_START+30)		//"�ǼǺ���"
#define HID_LEAVEFINGER	(HINT_START+31)		//"���뿪��ָ"
#define HID_PLACEFINGER2	(HINT_START+32)	//"��ڶ��ΰ���ָ"
#define HID_PLACEFINGER3	(HINT_START+33)	//"������ΰ���ָ"
#define HID_FINGERIDFMT	(HINT_START+34)		//"%05d-%d"
#define HID_CONTINUE	(HINT_START+35)		//"������"
#define HID_UIDFMT	(HINT_START+36)		//"%05d"
#define HID_ENROLLBACKUP	(HINT_START+37)	//"���ݵǼ�"
#define HID_USERPWDFMT	(HINT_START+38)		//"%05d-P"
#define HID_INPUTPWD	(HINT_START+39)		//"��������"
#define HID_VERINPUTPWD	(HINT_START+40)		//"����ȷ��"
#define HID_ADMINPRIV	(HINT_START+41)		//"��������Ȩ"
#define HID_VERDELETE	(HINT_START+42)		//"ɾ����"
#define HID_NOTENROLLED	(HINT_START+43)		//"�޵Ǽ�����!"
#define HID_NOATTLOG	(HINT_START+44)		//"�޿��ڼ�¼"
#define HID_HINTTIME	(HINT_START+45)		//"����ESC  ��һ��OK"
#define HID_WAITING	(HINT_START+46)		//"�����У����Ե�"
#define HID_ERRORPIN	(HINT_START+47)		//"�ǼǺ������"
#define HID_REFINGER	(HINT_START+48)		//"ָ���ظ�"
#define HID_INPUTAGAIN	(HINT_START+49)		//"���������룡"
#define HID_LNGSCH	(HINT_START+50)		//"����"
#define HID_LNGTCH	(HINT_START+51)		//"����"
#define HID_LNGENG	(HINT_START+52)		//"ENG"
#define HID_YES		(HINT_START+53)		//"��"
#define HID_NO		(HINT_START+54)		//"��"
#define HID_NOTHING	(HINT_START+55)		//"��"
#define HID_THANK	(HINT_START+56)		//"лл��"
#define HID_ALREADY	(HINT_START+57)		//"��ǩ����лл��"
#define HID_AGAIN	(HINT_START+58)		//"���������룡"
#define HID_EXCEEDFINGER	(HINT_START+59)	//"�ѵǼ�10öָ��"
#define HID_SCIN	(HINT_START+60)		//"�ϰ�ǩ��
#define HID_SCOUT	(HINT_START+61)		//"�°�ǩ��"
#define HID_SOUT	(HINT_START+62)		//"���"
#define HID_SBACK	(HINT_START+63)		//"�������"
#define HID_SOCIN	(HINT_START+64)		//"�Ӱ�ǩ��"
#define HID_SOCOUT	(HINT_START+65)		//"�Ӱ�ǩ��"
#define HID_SOPENLOCK	(HINT_START+66)		//"����"
#define HID_SVERIFY	(HINT_START+67)		//"��֤"
#define HID_VF	(HINT_START+68)			//"ָ��ȷ��"
#define HID_VFFAIL	(HINT_START+69)		//"�����°���ָ��"
#define HID_VP	(HINT_START+70)			//"����ȷ��"
#define HID_VPFAIL	(HINT_START+71)		//"�������"
#define HID_VREP	(HINT_START+72)		//"��ȷ�ϣ�"
#define HID_CONTINUEESC	(HINT_START+73)		//"�˳�ESC ����OK"
#define HID_SAVEQ	(HINT_START+74)		//"���棿"
#define HID_VSUCCESS	(HINT_START+75)		//"ȷ�ϳɹ���"
#define HID_ACCESSDENY	(HINT_START+76)		//"�Ƿ�����"
#define HID_POWEROFF (HINT_START+77)		//"�ػ�\t%s"
#define HID_SUSPEND (HINT_START+78)		//"����\t%s"
#define HID_ESC	(HINT_START+79)			//"�˳�ESC"
#define HID_TSTLCD	(HINT_START+80)		//��ǰLCD
#define HID_TSTLCD_LF	(HINT_START+81)		//��������
#define HID_TSTLCD_RF	(HINT_START+82)		//��������
#define HID_TSTLCD_UF	(HINT_START+83)		//��������
#define HID_TSTLCD_DF	(HINT_START+84)		//��������
#define HID_TSTLCD_LE	(HINT_START+85)		//������
#define HID_TSTLCD_RE	(HINT_START+86)		//������
#define HID_TSTLCD_UE	(HINT_START+87)		//������
#define HID_TSTLCD_DE	(HINT_START+88)		//������
#define HID_CONTINUEOK	(HINT_START+89)		//����OK
#define HID_KEYNAME_MENU (HINT_START+90)	//MENU
#define HID_KEYNAME_OK	(HINT_START+91)		//OK
#define HID_KEYNAME_ECS	(HINT_START+92)		//ESC
#define HID_KEYNAME_UP	(HINT_START+93)		//�ϼ�ͷ
#define HID_KEYNAME_DOWN	(HINT_START+94)	//�¼�ͷ
//#define HID_KEYNAME_LEFT	(HINT_START+95)	//���ͷ
//#define HID_KEYNAME_RIGHT	(HINT_START+96)	//�Ҽ�ͷ
#define HID_KEYNAME_POWER	(HINT_START+95)	//���µ�Դ
#define HID_KEYNAME_ANYKEY	(HINT_START+99)	//���������ʾ
#define HID_KEYNAME_NUMBER	(HINT_START+100)//����
#define HID_PLAYVOICEINDEX  (HINT_START+101)	//���ŵ� %d ������
#define HID_TEST_FLASH_CQ1   (HINT_START+102)	//����FLASHʱ��
#define HID_TEST_FLASH_CQ2   (HINT_START+103)	//��Ҫ����Դ��ť
#define HID_TESTING   (HINT_START+104)		//���ڲ��ԡ���
#define HID_TEST_FLASH_RES   (HINT_START+105)	//����:%d����:%d
#define HID_FINISH   (HINT_START+106)		//��ɣ�
#define HID_TEST_OK    (HINT_START+107)		//������
#define HID_TEST_BAD    (HINT_START+108)	//��������������
#define HID_PREPOWEROFF    (HINT_START+109)	//׼���ػ�����
#define HID_PRI_NONE   (HINT_START+110)		//��ͨ�û�
#define HID_PRI_ENROLL    (HINT_START+111)	//�Ǽ�Ա
#define HID_PRI_ADMIN    (HINT_START+112)	//����Ա
#define HID_PRI_SUPERVISOR    (HINT_START+113)	//��������Ա
#define HID_PRI_INVALID   (HINT_START+114)	//��ֹ�û�
#define HID_PRI   (HINT_START+115)		//Ȩ��
#define HID_1TO1   (HINT_START+116)		//1:1 Verifying
#define HID_EXCEED	(HINT_START+117)	//Exceed space
#define HID_LEFTSPACE (HINT_START+118)		//left space %d
#define HID_OKPWD (HINT_START+119) 		//OK for pwd
#define HID_OTAPOWER (HINT_START+120) 		//���ö�ʱ������
#define HID_DEL_FP (HINT_START+121)		//Delete Fingerprint
#define HID_DEL_PWD (HINT_START+122) 		//Delete Password
#define HID_DEL_USR (HINT_START+123) 		//Delete User
#define HID_WORKING	(HINT_START+124)	//������....
#define HID_CARDFP	(HINT_START+125)	//�ȶԿ���ָ��
#define HID_INVALIDCARD	(HINT_START+126)	//��Ч��
#define HID_CANCELKEY	(HINT_START+127)	//�����ȡ��
#define HID_SHOWCARD	(HINT_START+128)	//���ʾ��
#define HID_WRITE_OK	(HINT_START+129)	//д���ɹ�
#define HID_WRITE_ERROR	(HINT_START+130)	//д��ʧ��
#define HID_NOFINGER	(HINT_START+131)	//δ�Ǽ�ָ��
#define HID_FLASHERROR	(HINT_START+132)	//�洢������
#define HID_CARD	(HINT_START+133)	//����֤
#define HID_MUSTINOUT	(HINT_START+134)	//����ѡ�����״̬
#define HID_TZDEF	(HINT_START+135)	//ʱ���%d����
#define HID_GTZ	(HINT_START+136)		//��%dĬ��ʱ���
#define HID_ULG	(HINT_START+137)		//���%d
#define HID_TZI	(HINT_START+138)		//ʱ���%d
#define HID_UAOPT	(HINT_START+139)	//�û�%05d�Ž�
#define HID_SHORTWEEK	(HINT_START+140)	//���ڵĶ�����
#define HID_TZNUM	(HINT_START+141)	//ʱ��α��
#define HID_GRPNUM	(HINT_START+142)	//����acc
#define HID_INVALIDTIME (HINT_START+143)	//�Ƿ�ʱ�η���
#define HID_OS_MUST1TO1 (HINT_START+144)	//��������ID
#define HID_NEWENROLLNUM (HINT_START+145)	//�º���
#define HID_CARD_NOTENROLLED (HINT_START+146)	//��δ�Ǽ�
#define HID_CARD_ENROLLED (HINT_START+147)	//���ѵǼ�
#define HID_CARD_NO	(HINT_START+148)	//����:
#define HID_AUTO_STATE	(HINT_START+149)
#define HID_NUM	(HINT_START+150)
#define HID_STATE	(HINT_START+151)
#define HID_LNGTHAI	(HINT_START+152)	//̩��
#define HID_SHUTDOWNING (HINT_START+153)        //%s ���ػ�
#define HID_SUSPENDING (HINT_START+154) 	//%s ������
#define HID_ALARM_STRIPE (HINT_START+155)       //���������
#define HID_ALARM_DOOR (HINT_START+156)        	//�ű������
#define HID_ALARM_LOWBATTERY (HINT_START+157) 	//��ص�ѹ����
#define HID_ALARM_INDOOROPEN (HINT_START+158)   //���ſ��ؿ���
#define HID_DSM (HINT_START+159)  		//�Ŵſ���ģʽ������;����;��
#define HID_DOOR_OPEN (HINT_START+160)  	//���Ѵ�
#define HID_DOOR_CLOSE (HINT_START+161) 	//���ѹر�
#define HID_MUSER_OPEN1 (HINT_START+162)        //���û�����
#define HID_MUSER_OPEN2 (HINT_START+163)        //�Ƿ��������
#define HID_RESTART (HINT_START+164)        	//��������������Ч
#define HID_UNREGALL (HINT_START+165)   	//ȫ�������
#define HID_LNGINDON (HINT_START+166)   	//ӡ����
#define HID_TSTLCD_1 (HINT_START+167) 	//LCD���Ե�һ��
#define HID_TSTLCD_2 (HINT_START+168) 	//LCD���Ե�һ��
#define HID_1TOG     (HINT_START+169) 	//1:G�ȶ�
#define HID_INVALIDWORKCODE (HINT_START+170) //workcode ��Ч
//#define HID_BAK_PWR         (10+173) //Backup Power    �󱸵�Դ
#define	HID_PLS_SET_DATE		(10+174) //Pls Set Date    ������ʱ��
#define	HID_RTC_ERR			(10+175) //Invalid Date    ���ڴ��� 
//#define HID_NORMAL_OPEN         (10+176) //Normal Open     ����
#define HID_WARNING         (10+178) //Warning         �Ƿ�����
#define HID_BACK_ESC                (10+179) //Back ESC        ESC����
#define HID_CHECK_DATE              (10+180) //Check Date...   �������...
#define HID_YES_OK              (10+181) //Yes OK          �� OK
#define HID_LOCK                (10+182) //Lock            ����
#define HID_NORMAL_OPEN_M       (10+183) //Normal Open M   ESC�˳�  OK����
#define HID_PASSWARD_FULL       (10+184) //Passward Full   ��������
#define HID_LOCK_M              (10+185) //Lock M          ESC�˳�  OK����
#define HID_FIRST_VERIFY        (10+186) //First Verify    ��һ�αȶ�
#define HID_REPEAT_VERIFY       (10+187) //Repeat Verify   �ظ���֤
#define HID_USER_FULL           (10+188) //user full       �û�����
#define HID_PAFFIRM             (10+189) //Affirm          ȷ��

#define MENU_START 200
#define MID_MENU (MENU_START+1)	//"�˵�"
#define MID_DATA (MENU_START+2)	//"���ݹ���"
#define MID_HIDE (MENU_START+3)	
#define MID_OPTIONS (MENU_START+4)	//"����"
#define MID_SYSINFO (MENU_START+5)	//"ϵͳ��Ϣ"
#define MID_DATA_EUSER (MENU_START+6)	//"�û��Ǽ�"
#define MID_DATA_EADMIN (MENU_START+7)	//"�����ߵǼ�"
#define MID_DATA_DEL (MENU_START+8)		//"ɾ���Ǽ�����"
#define MID_DATA_EU_FINGER (MENU_START+9)	//"ָ�ƵǼ�"
#define MID_DATA_EU_PWD (MENU_START+10)		//"����Ǽ�"
#define MID_DATA_EU_FP (MENU_START+11)		//"ָ�Ƽ�����"
#define MID_OPTIONS_SYSTEM (MENU_START+12)	//"ϵͳ����"
#define MID_OPTIONS_REC (MENU_START+13)		//"��¼����"
#define MID_OPTIONS_COMM (MENU_START+14)	//"ͨѶ����"
#define MID_OS_ADMINNUMBER (MENU_START+15)	//"����������\t%d"
#define MID_OS_DEVNUMBER (MENU_START+16)	//"����\t%d"
#define MID_OS_TIME (MENU_START+17)		//"ʱ������"
#define MID_OS_LANGUAGE (MENU_START+18)		//"����\t%s"
#define MID_OS_LOCK (MENU_START+19)		//"������\t%s"
#define MID_OS_AUTOPOWER (MENU_START+20)	//"�Զ���Դ����\t%s"
#define MID_OR_AADMINLOG (MENU_START+21)	//"�����¼����\t%d"
#define MID_OR_AATTLOG (MENU_START+22)		//"���ڼ�¼����\t%d"
#define MID_OR_REREC (MENU_START+23)		//"�ظ�ȷ��ʱ��\t%d"
#define MID_OC_BAUDRATE (MENU_START+24)		//"������\t%d"
#define MID_OC_CRC (MENU_START+25)		//"��żУ��\t%s"
#define MID_OC_STOP (MENU_START+26)		//"ֹͣλ\t%d"
#define MID_SYSINFO_USER (MENU_START+27)	//"�û��Ǽ�\t%d"
#define MID_SYSINFO_FINGER (MENU_START+28)	//"ָ�ƵǼ�\t%d"
//#define MID_SYSINFO_ATTLOG (MENU_START+29)	//"���ڼ�¼\t%d"
#define MID_SYSINFO_ADMIN (MENU_START+30)	//"�����ߵǼ�\t%d"
#define MID_SYSINFO_PWD (MENU_START+31)		//"����Ǽ�\t%d"
//#define MID_SYSINFO_ADMINLOG (MENU_START+32)	//"�����¼\t%d"
#define MID_DATA_VATTLOG (MENU_START+33)	//"�鿴���ڼ�¼"
#define MID_DATA_VADMINLOG (MENU_START+34)	//"�鿴�����¼"
#define MID_DATA_DELLOG (MENU_START+35)		//"ɾ��ȫ����¼"
#define MID_NET_UDPPORT (MENU_START+36)		//"����˿�"
#define MID_NET_WEBPORT (MENU_START+37)		//"Web�˿�"
#define MID_NET_MAC (MENU_START+38)		//"MAC��ַ"
#define MID_NET_IP (MENU_START+39)		//"IP��ַ"
#define MID_OPTIONS_NET (MENU_START+40)		//"��������"
#define MID_OSA_POWEROFF (MENU_START+41)	//"�ػ�\t%s"
#define MID_OSA_SUSPEND (MENU_START+42)	//"����\t%s"
#define MID_OSA_POWERON (MENU_START+43)	//"����\t%s"
#define MID_OSA_IDLE (MENU_START+44)	//"��������\t%s
#define MID_OSA_IDLETIME (MENU_START+45)	//"����ʱ��\t%s"
#define HMID_SHOWSCORE (MENU_START+46)	//��ʾ����
#define HMID_MATCHSCORE (MENU_START+47) //ƥ�䷧ֵ
#define HMID_NOISETHRESHOLD (MENU_START+48)//������ֵ
#define HMID_MATCHSPEED (MENU_START+49)	//���ٱȶ�
#define HMID_VOICEON (MENU_START+50)	//������ʾ
#define HMID_NOFINGER_THRESHOLD (MENU_START+51)		//��ָ�Ʒ�ֵ
#define HMID_HASFINGER_THRESHOLD (MENU_START+52)	//��ָ�Ʒ�ֵ
#define HMID_TOPFINGER_THRESHOLD (MENU_START+53)	//��ָ�Ʒ�ֵ
#define MID_AUTOTEST (MENU_START+54)	//�Զ�������
#define MID_AT_FLASH (MENU_START+55)	//FLASH���
#define MID_AT_LCD (MENU_START+56)	//Һ�����
#define MID_AT_VOICE (MENU_START+57)	//�������acc
#define MID_AT_KEYPAD (MENU_START+58)	//���̼��
#define MID_AT_ALL (MENU_START+59)	//ALL
#define MID_AT_RTC (MENU_START+60)	//ʵʱʱ��
#define MID_AT_FINGER (MENU_START+61)	//ָ�Ʋɼ���
#define MID_OC_NETOFF (MENU_START+62)	//������̫����
#define MID_OC_RS232OFF (MENU_START+63)	//����RS232ͨ��
#define MID_OC_RS485OFF (MENU_START+64)	//����RS485ͨ��
#define MID_OS_ADVANCE (MENU_START+65)	//�߼�����
#define MID_OS_INIT	(MENU_START+66)		//��������
#define MID_OI_ENNUM	(MENU_START+67)		//�ɵǼ�ָ��
#define MID_OI_ALNUM	(MENU_START+68)		//�ɱ����¼
#define MID_OI_NET	(MENU_START+69)		//���繦��
#define MID_OI_LOCK	(MENU_START+70)		//���ع���
#define MID_OI_RFCARD	(MENU_START+71)		//��Ƶ������
#define MID_OI_PROTIME	(MENU_START+72)		//����ʱ��
#define MID_OI_INITDEV	(MENU_START+73)		//�豸��ʼ��
#define MID_CLEAR_DATA	(MENU_START+74)		//Clear Data
#define MID_OS_RESTORE	(MENU_START+75)		//Restore Default Options
#define MID_OS_VERSCORE	(MENU_START+76)		//1:1 Verify Score
#define MID_CLEAR_ADMIN	(MENU_START+77)		//�������Ա
#define MID_INFO_RES	(MENU_START+78)		//ʣ��������Ϣ
#define MID_IR_FINGER	(MENU_START+79)		//ָ����AttLog
#define MID_IR_ATTLOG	(MENU_START+80)		//�����¼
#define MID_INFO_DEV 	(MENU_START+81)		//�豸��Ϣ
#define MID_OI_PT	(MENU_START+82)		//��������
#define MID_OI_SN	(MENU_START+83)		//���к�
#define MID_OI_OEM	(MENU_START+84)		//������
#define HMID_TDFINGER_THRESHOLD (MENU_START+85)		//HasFingerThresholdDiff
#define HMID_NEWFPR	(MENU_START+86)		//���ɼ���
#define HMID_DEVTYPE (MENU_START+87)	//����
#define MID_OI_1TO1 (MENU_START+88)	//1:1����
#define MID_OI_ATTSTATE (MENU_START+89)	//����״̬����
#define MID_OI_SHOWNAME (MENU_START+90)	//��ʾ��������
#define MID_OI_POWERMNG (MENU_START+91)	//��Դ������
#define MID_OI_PN	(MENU_START+92)	//Device Name
#define MID_OS_LOCKUSERS (MENU_START+93)//unLock Users
#define MID_DC_ENROLL	(MENU_START+94)	//�Ǽ�ָ�ƿ�
#define MID_DC_REG	(MENU_START+95)	//ע��ָ�ƿ�
#define MID_DC_CREATE	(MENU_START+96)	//����ָ�ƿ�
#define MID_DC_EMPTY	(MENU_START+97)	//���ָ�ƿ�
#define MID_DC_UNREG	(MENU_START+98)	//ע��ָ�ƿ�
#define MID_DATA_CARD	(MENU_START+99)	//ָ�ƿ�����
#define MID_DC_DUMPFP	(MENU_START+100)//���ƿ���ָ��
#define MID_DC_MOVEFP	(MENU_START+101)//ת��ָ�Ƶ�����
#define MID_DC_PIN		(MENU_START+102)//���뿨
#define MID_OC_PINCARD	(MENU_START+103)//����֤���뿨
#define MID_OI_FWVER	(MENU_START+104)//�̼��汾��
#define MID_OI_ALGVER	(MENU_START+105)//�㷨�汾��
#define MID_OA_FPKEY	(MENU_START+106)//ָ�ƿ�����
#define MID_OS_CUST	(MENU_START+107)//��������
#define MID_OS_HIGHSPEED	(MENU_START+108)//��������
#define MID_OA_OPTION	(MENU_START+109)	//�Ž���������
#define MID_OA_TZDEF	(MENU_START+110)	//ʱ��ζ���
#define MID_OA_ULGRP	(MENU_START+111)	//������϶���
#define MID_OA_UAOPT	(MENU_START+112)	//�û��Ž�����
#define MID_OA_GTZ	(MENU_START+113)	//��Ĭ��ʱ���
#define MID_OA_GRP	(MENU_START+114)	//��������
#define MID_OS_MUST1TO1 (MENU_START+115)	//��������ID
#define MID_OS_COMKEY	(MENU_START+116)	//��������
#define MID_OC_MUSTENROLL	(MENU_START+117)//��ֹδע�Ῠ
#define MID_OA_UDT	(MENU_START+118)//ʹ����ʱ��
#define MID_CARD_REG	(MENU_START+119)	//��Ƶ���Ǽ�
#define MID_CARD_UNREG	(MENU_START+120)	//��Ƶ��ע��
#define MID_OSA_ALARM	(MENU_START+121)	//��ʱ����
#define MID_ADMIN_REC	(MENU_START+123)	
#define MID_AUTO_STATE	(MENU_START+124)	//��ʱ״̬װ��
#define MID_AO_IMGCOH 	(MENU_START+125)    //ָ��ͼ��������
#define MID_AO_ALARMOFF (MENU_START+126)    //�������
#define MID_AC_DSD 	(MENU_START+127)    //�Ŵ���ʱ
#define MID_AC_DSM 	(MENU_START+128)    //�Ŵſ���ģʽ
#define MID_OSA_ALARM_DELAY (MENU_START+129) //
#define MID_OSA_ALARM_NAME (MENU_START+130) //
#define MID_AD_DURESSFINGER (MENU_START+131)    //в��ָ�ƹ���
#define MID_AD_DURESSHELP (MENU_START+132)      //��~K�������� ��/��
#define MID_AD_DURESS11 (MENU_START+133)        //1��1��ʽ���� ��/��
#define MID_AD_DURESS1N (MENU_START+134)        //1��N��ʽ���� ��/��
#define MID_AD_DURESSAD (MENU_START+135)        //�Զ�����ʱ�� 0��255��
#define MID_ADF_ENROLL (MENU_START+136) 	//�µǼ�в��ָ��
#define MID_ADF_REG (MENU_START+137)    	//в��ָ�ƶ���
#define MID_ADF_UNREG (MENU_START+138)  	//ȡ��в��ָ��
#define MID_ADF_UNREGALL (MENU_START+139)       //ȡ��ȫ��
#define MID_AD_DURESS (MENU_START+140)  	//в�ȱ���
#define MID_AD_DURESSPWD (MENU_START+141)       //PWD��ʽ���� ��/��
#define MID_LOCK_POWER	(MENU_START+142)    	//LOCK POWER BUTTON
#define MID_POWER_OFF	(MENU_START+143)    	//Shut Down
#define MID_OS_1TON_FROM (MENU_START+144)       //1:N From
#define MID_OS_1TON_TO 	(MENU_START+145) 	//1:N To
#define MID_OS_1TOH 	(MENU_START+146) 	//"S/_346_=ǰ����ID��",
#define MID_OS_1TOG 	(MENU_START+147) 	//"S/_347_=����1:G",
#define MID_OI_1TON 	(MENU_START+148) 	//"S/_348_=1��Nָ��",
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
#define HID_PLACEFINGER4 (ZEM200_START+15)	//"����Ĵΰ���ָ"
#define HID_CONTINUECANCEL (ZEM200_START+16)	//�˳�ESC ����OK
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


#define MID_PRINTERON           (ZEM200_START+63)                //��ӡ������
#define MID_PRINTERMODE         (ZEM200_START+64)                //ģʽ

#define MID_WORKCODEMODE	(ZEM200_START+65)		//workecodeģʽ

#define MID_C2CONFAIL		(ZEM200_START+66)	//C2ͨѶʧ��
#define MID_C2NOFOUND		(ZEM200_START+67)	//C2��ƥ��
#define MID_C2LOCKED		(ZEM200_START+67)	//C2����
#define MID_UPLOAD_VOICE	(ZEM200_START+68)	//���������ļ�
#define MID_RESTORE_VOICE	(ZEM200_START+69)	//�ָ�Ĭ������
#define MID_DISKCLEAN (ZEM200_START+70)		//diskclean
#define MID_PUTBELL (ZEM200_START+71)		//�������彡


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

#define MID_SYSINFO_ATTLOG 		(770)	//"��¼"
#define MID_SYSINFO_ADMINLOG	(771)	//"�鿴��¼"
#define MID_OPTION_LOCK 		(772)		//��������
#define MID_VRYFAIL_WAIT     	(773)	//�Ƿ���������
#define	MID_NOR_OPEN_ON     	(774) 	//��������
#define	MID_VRYFAIL_WAITCNT    	(775) //S/_775_=ʧ�ܴ�������    
#define	MID_VRY_BIND       		(776) //S/_776_=��֤ģʽ
#define HID_NORMAL_OPEN         (777) //Normal Open     ����
#define HID_BAK_PWR         	(778) //Backup Power    �󱸵�Դ
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
#define NID_VFFAIL (Nideka_start+12)//���ذ���ָ
#define NID_NOREC (Nideka_start+13)//��Ա����¼
#define NID_PSPUTCARD (Nideka_start+14) //�����Ŀ�
#define NID_LEAVEFINGER (Nideka_start+15)//���뿪��ָ

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
	int LockOn;					//��������ʱ��
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
	int AutoPowerOff; 				//�Զ��ػ�ʱ��
	int AutoPowerOn;				//�Զ�����ʱ��
	int AutoPowerSuspend;				//�Զ�����ʱ��
	int AutoAlarm[MAX_AUTOALARM_CNT];		//�Զ�����ʱ��
	int IdlePower;					//�����Զ�����-1���Զ��ػ�-0
	int IdleMinute;					//����ʱ��
	int ShowScore;					//show the verification score for fingerprint matching.
	int NetworkOn, RS232On, RS485On;		//�Ƿ�����̫���硢RS232��RS485����
	int NetworkFunOn, LockFunOn, RFCardFunOn; 	//�Ƿ������/�Ž���0-�ޣ�1-���Ž���2-�߼��Ž���/��Ƶ������
	int One2OneFunOn, PowerMngFunOn;	 	//�Ƿ��1:1����,��Դ������
	int NewFPReader;
	int ShowName;
	int UnlockPerson;				//ͬʱ����������
	int ShowCheckIn;				//�Ƿ���ʾ���°�״̬
	int OnlyPINCard;				//����֤���뿨
	int IsTestMachine;				//�Ƿ�����û���
	int MustChoiceInOut;				//�Ƿ����ѡ�����״̬
	//�ܹ��Զ�����	int CMOSSensorDevAddr;		//CMOS Sensor Chip IIC Device Address: 0x42-OV7620, 0x22-Hy7131
	int HiSpeedNet;					//100M Ethernet
	int MenuStyle;					//�˵����
	int CanChangeCardKey;				//�Ƿ�����ı俨����
	int Must1To1;					//�Ƿ�ֻ����һ��һ�ȶ�
	int LCDModify;                              	
	int ComKey;					//��������
	int MustEnroll;					//������ע���û�,�ȶԺ������Ч�����ڱ�Ļ����ϵǼǵ�ָ�ƿ����Բ��ڱ�����ע�ἴ��ʹ��
	int TimeOutMenu;				//�˵��ĳ�ʱʱ��
	int TimeOutPin;					//�俼�ں���ĳ�ʱʱ��
	int TimeOutState;				//����״̬�ĳ�ʱʱ��
	int SaveAttLog;					//�Ƿ񱣴���֤��¼
	int RS232Fun;					//RS232�ӿڹ��ܣ�0-�ޣ�1-����APIͨѶ��2-��ID���
	int IsModule;					//�Ƿ�ģ��
	int ShowSecond;					//�Ƿ���ʾ��
	int RFCardSecStart;				//Mifare Card ��ʼ���ָ�����ݵ�����
	int RFCardSecLen;				//Mifare Card ���ָ�����ݵ�������
	int RFCardFPC;					//Mifare Card ���ָ�Ƶĸ���
	int PIN2Width;					//PIN2����ַ���� <=5��ʾ��֧��PIN2��  2147483647=0x7FFFFFF
	int DateFormat;					//Date Format
	int OPLogMask1;					//
	int OPLogMask2;					//
	int AutoState[4*4];				//�Զ�״̬ת����ʱ��
	int DelayCount, IncThr, TopThr, MinThr;		//ָ�Ƽ��ķ�ֵ����
	int MaxNoiseThr, MinMinutiae, MaxTempLen;	//�������ָ��ͼ��������ֵ, 
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
	int AutoStateFunOn;				//�Զ�״̬ת������
	int EnrollCount;  	                     	//register finger count	
	int OpenDoorDelay;          	              	//delay time after open the door
	int DoorSensorMode;                         	//door sensor �Ŵſ��ط�ʽ 0-NO 1-NC ������ʾ������Ŵ�
	int IsSupportSMS;				//SMS Enable or Disable
	int IsSupportUSBDisk;				//Support USB DISK
	int AdminCnt;                               	//ͬʱ��֤����Ա�ĸ���
	int AutoAlarmDelay;				//Auto Alarm delay time(second)
	int MultiLanguage;				//support select language
	int LockPowerButton;				//Lock Power Button
	int IsSupportModem;				//support modem connection
	int IsConnectModem;				//connect or not
	int IsSupportAuthServer;			//support AuthServer
	int AuthServerTimeOut;				//TimeOut for AuthServer
	int IsSupportMF;				//support mifare
        int DuressHelpKeyOn;                            //��~K�������� ��/��
        int Duress1To1;                                 //1��1��ʽ���� ��/��
        int Duress1ToN;                                 //1��N��ʽ���� ��/��
        int DuressPwd;                                  //Password��ʽ��������
        int DuressAlarmDelay;                   	//�Զ�����ʱ�� 0��255��
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
        int StartUpNotify;                              //�𶯹㲥��BIT0-BT232, BIT1-NETWORK
	int AdvanceMatch;				//support 1:G 1:H	
        int I1ToNFrom;                                  //1��N From (minimum user PIN for identification)
        int I1ToNTo;                                    //1��N To (maximum user PIN for identification)
        int I1ToH;                                    	//1��H
        int I1ToG; 					//1��G
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
	int DoorAlarmMode;				//door alarm ���ط�ʽ 0-NO 1-NC ������ʾ�����
	int PrinterFunOn;                       //�Ƿ��д�ӡ������
	int PrinterOn;                       //��ӡ�����ܣ�0���ޣ�1����������ڣ�2��ESC��Star��ӡ
	int Nideka; 
	int CheckOutWc;//��chekcout ����workcode	
	int SyncTimeFromAuthServer;  //get time from authserver interval
        int AutoOpenRelayFunOn;//�Ƿ���ʾ��ʱ���忪�Ź��ܲ˵�        
	int FPOpenRelay;        //��ָ���Ƿ���
        int AutoOpenRelay;      //�Ƿ����ö�ʱ���幦��         
	int AutoOpenRelayTimes; //��ʱ������뿪��
	int DefaultGroup;//ϵͳĬ���� ���ڷ���ȶ�
	int GroupFpLimit;//����ָ�������
	int LimitFpCount;//ϵͳָ�������
	int WireLessBell;//��������
	int DevID;	//similar to DeviceName.0 - h1plus, 1 - u100
	int RunMode;	// 0 - factory mode; 1 - release mode
	int ZKFPVersion;
	int MulAlgVer;

	//-- add by chenyy
	int NorOpenOn;
	int MSAnimation;
	int CPUFreq;
	int FlashLED                ;                   //0������1�̵ƣ�2���
	int C2FunOn                 ;           //�Ƿ�֧��C2
	int ExternalAC              ;           //�����Ž�������
	int PageState               ;                   //��ҳ״̬  , 1-�ĸ�״̬ 2-����״̬

	int DaylightSavingTimeFun   ,           //�Ƿ���ʾ֧������ʱ��,
		DaylightSavingTimeOn    ,           //�Ƿ�֧������ʱ��
		CurTimeMode             ,           //1��ǰΪ����ʱ��2��ǰ��������ʱ
		DaylightSavingTime      ,           //��������ʱ��ʱ��
		StandardTime            ;           //���������ʱ��ʱ��

	int MasterSlaveFunOn        ;           //�Ƿ�֧�����ӻ�����
	int SlaveMustRegester       ;           //�ӻ��Ƿ����ע��,0����ע�ᣬ1����ע��
	int MasterSlaveOn;                      //0,��,1������2�ӻ�
	int NoPowerKey              ;           //No power key 0-have power key,1-power key is bell,2-No power key
	int LockPWRButton           ;               //�����ػ���ť
	int SaveUnlockLog           ;               //If Saved the Unlock Log
	int VryFailWait;
	int VryFailWaitCnt;
	int MifareFunOn;
	int VryBind				;			//Verify mode select
	int TagLogDoorAction		;			//tag log as door action that whether user open door.
	int MasterState				;			//���ӻ�ͨѶ�У�������״̬��������Ϊ0����ӻ�Ϊ1��������Ϊ1��ӻ�Ϊ0����Ϊ-1�����Ӷ�Ϊ������ǰ��״̬gOptions.AttState��
	int UnSaveLogDeny;
	int	AntiPassbackFunOn;					//0֧�ַ�Ǳ,1��֧�ַ�Ǳ
	int AntiPassbackOn;						//0 ����Ǳ;1������Ǳ������������ܳ���2���뷴Ǳ�������г������룻
	int CloseDoorHint;
	int TestFunOn;
	int ExtWGInFunOn			;
	int VoiceFunOn				;					//��������
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
//���ڴ洮�б���ɾ��ָ���ַ���
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

#define MAX_AUTOALARM_COUNT 10 //���Ķ�ʱ�������
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
