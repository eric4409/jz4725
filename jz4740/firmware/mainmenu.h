/*************************************************
                                           
 ZEM 200                                          
                                                    
 mainmenu.h                                
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef __MAINMENU_H_
#define __MAINMENU_H_

#include "msg.h"
#include "flashdb.h"

extern int WaitAdminVerifyCount;
extern PUser AdminUser;
extern int HackerNumber;

#define ADMINPIN ((AdminUser==NULL)?0:(AdminUser->PIN))

#define ISHACKER (HackerNumber==2)
#define TESTPRIVILLEGE(priv) ((HackerNumber==1) || ISHACKER || (AdminUser==NULL) || ((AdminUser->Privilege & priv)!=0))

void ShowMainLCD(void);

int DoHideMenu(void *p);
int DoAdvanceMenu(void *p);
int DoHideAdvMenu(void *p);
int DoSetAccess(void *p);
int DoMainMenu(void);
int DoSetLock(void *p);

int DoShowSysInfo(void *p);

int EnrollAFinger(char *tmp, int *len, int pin, int fingerid);

char *ShowTimeValue(char *buf, int id, int TimeValue);
int InputValueOfItem(PMsg p, int width, int minv, int maxv, int *OptionValue);
int InputYesNoItem(PMsg p, int *OptionValue);
int InputTime(int row, int col, int *vhour, int *vsecond);
int InputIPAddress(char *Title, int *ipa);

int GetCustValueCount(void);
int DoSetCustValues(void *p);

int ViewAttLogByUser(U16 PIN, int TimeOut);
int GetModeName(char *name,int modenames, int Mode);
int DoSetPrinterMode(void *p);
int DoSetWorkCodeMode(void *p);
int DoUploadVoice(void *p);
int DoRestoreVoice(void *p);
//SPANISH
int DoSetFpOpenOff(void *p);
int DoSetAutoOpenOff(void *p);
int DoSetAutoOpenTimes(void *p);

int DoShowGroupFpInfo(void *p);
int DoSetDefaultGroup(void *p);

#endif
