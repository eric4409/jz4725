/*
 * webserver.h -- Main program for the ZK Software WebServer (LINUX version)
 *
 * Copyright (c) ZK Software Inc., 2005. All Rights Reserved.
 *
 *
 * $Id: webserver.h,v 1.0.5 2005/11/01 14:03:46 CWX Exp $
 */

/******************************** Description *********************************/

/*
 *	Main program for the ZK WebServer. This is a demonstration
 *	Main Interface to initialize and configure the web server.
 */
#ifndef _ZK_WebServer_ 
#define _ZK_WebServer_

#include "flashdb.h"
#include "arca.h"

#ifdef WEBSERVER

#define _ZK_WebServer_Version 20051110.105
#define _ZK_WebServer_Name ZKWeb

//Message Define For WebServer

#define MSG_WEB_PROCESS  (1<<11)
#define MSG_WEB_CREATE   (1<<12)
#define MSG_WEB_CLOSE    (1<<13)

//Main Function For WebServer

int WebInitializingConfig(char *CfgName);
int WebFreeConfig();
int WebStart(void);
int WebClose(void);
int WebReady(void);
int WebProcess(void);

//CallBack Function Form Firmware

void WebRegSaveStr(int (*Proc)(const char *name, const char *value, int SaveTrue));
void WebRegEAOParam(int AuxOnTime, int OpenDoorDelay);
void WebRegExAuxOut(void (*Proc)(int AuxOnTime, int OpenDoorDelay));
void WebRegExPowerOff(int (*Proc)(int Cmd));
void WebRegCaptureSensor(int (*Proc)(char *Buffer, int Sign, int *Size, int *Sensor));
void WebRegFingerBuffer(char *Buffer);
void WebRegIdentifyFinger(int (*Proc)(char *InputPin, int PIN, unsigned char *Temp));
void WebRegWriteBmp(void (*Proc)(char *img,unsigned char *buff,int Width,int Height));
void WebRegFDBInit(int (*Proc)(int OpenSign));
void WebRegLcdClear(void (*Proc)(void));
void WebRegFDBClrUser(int (*Proc)(void));
void WebRegFDBClrAdmin(int (*Proc)(void));
void WebRegFDBClrAttLog(int (*Proc)(void));
void WebRegFDBClrTmp(int (*Proc)(void));
void WebRegFDBClrOPLog(int (*Proc)(void));
void WebRegFDBCntAdmin(int (*Proc)(int Privillege));
void WebRegFDBCntUser(int (*Proc)(void));
void WebRegFDBCntAdminUser(int (*Proc)(void));
void WebRegFDBCntPwdUser(int (*Proc)(void));
void WebRegFDBCntTmp(int (*Proc)(void));
void WebRegFDBCntAttLog(int (*Proc)(void));
void WebRegFDBCntOPLog(int (*Proc)(void));
void WebRegFDBGetTmpCnt(unsigned int (*Proc)(unsigned short UID));
void WebRegSoftwareInfo(char *Pt,char *Sn,char *Ov,char *Dn,char *Av);
void WebRegFDBDeleteTmp(int (*Proc)(unsigned short PIN));
void WebRegOption(void *option,void* (*Proc)(void *opt));
void WebRegDeviceID(int DeviceID);
void WebRegFPDBInit(int (*Proc)(void));
void WebRegFDBDelUser(int (*Proc)(unsigned short PIN));
void WebRegFDBAddUser(int (*Proc)(PUser user));
void WebRegFDBHandle(int *ft,int *fu);
void WebRegFDBChgUser(int (*Proc)(PUser user));
void WebRegFDBGetUser(PUser (*Proc)(unsigned short PIN,PUser user));
void WebRegFDBCreateTemplate(PTemplate (*Proc)(PTemplate tmp, unsigned short PIN, char FingerID, char *Template, int TmpLen));
void WebRegFDBAddTmp(int (*Porc)(PTemplate tmp));
void WebRegFDBGetFreeFingerID(int (*Proc)(unsigned short PIN,unsigned char *FID));
void WebRegMaxFingerCount(int Proc);
void WebRegStringDecode(int (*Proc)(char *buf, char *str));
void WebRegBIOKEY_SETTEMPLATELEN(int (*Proc)(unsigned char *Template, int NewLength));
void WebRegFDBGetTmp(unsigned int (*Proc)(unsigned short PIN, char FingerID, PTemplate tmp));
void WebRegBIOKEY_IDENTIFYTEMP(void *fhdl,int (*Proc)(void *Handle, unsigned char *Template, int *TID, int *Score));

void WebRegExt(int IsWorkCode,int IsExtAtt);  //Update New Expand Struct at 2006.01.16
void WebRegIsOnlyRFMachine(int IsOnlyRFMachine); //Update RFID at 2006.01.18

int * pSelectFDFromConentType(int ContentType);
int FDB_GetFreeFingerID(U16 PIN, BYTE *FID);

void WebInitializing(void);
void WebFinish(void);

#endif
#endif
