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

#include <stdio.h>
#include "webserver.h"
#include "convert.h"
#include "lcm.h"
#include "finger.h"
#include "utils.h"
#include "options.h"
#include "exfun.h"
#include "zkfp.h"
#include "sensor.h"
#include "main.h"

#ifdef WEBSERVER 

void WebInitializing(void)
{
	printf("Start Web Server....\n");
        WebInitializingConfig("webs.cfg");
        WebRegSaveStr(SaveStr);
        WebRegExAuxOut(ExAuxOut);
        WebRegEAOParam(gOptions.LockOn, gOptions.OpenDoorDelay);
        WebRegExPowerOff(ExPowerOff);
        WebRegFingerBuffer(gImageBuffer);
        WebRegWriteBmp(write_bitmap);
        WebRegFDBInit(FDB_InitDBs);
        WebRegLcdClear(LCDClear);
        WebRegFDBClrUser(FDB_ClrUser);
        WebRegFDBClrAdmin(FDB_ClrAdmin);
        WebRegFDBClrAttLog(FDB_ClrAttLog);
        WebRegFDBClrTmp(FDB_ClrTmp);
        WebRegFDBClrOPLog(FDB_ClrOPLog);
        WebRegFDBCntAdmin(FDB_CntAdmin);
        WebRegFDBCntUser(FDB_CntUser);
        WebRegFDBCntAdminUser(FDB_CntAdminUser);
        WebRegFDBCntPwdUser(FDB_CntPwdUser);
        WebRegFDBCntTmp(FDB_CntTmp);
        WebRegFDBCntAttLog(FDB_CntAttLog);
        WebRegFDBCntOPLog(FDB_CntOPLog);
        WebRegFDBGetTmpCnt(FDB_GetTmpCnt);
        WebRegSoftwareInfo(ProductTime,SerialNumber,OEMVendor,DeviceName,AlgVer);
        WebRegOption(&gOptions,(void *)LoadOptions);
        WebRegFDBDeleteTmp(FDB_DeleteTmps);
        WebRegDeviceID(gOptions.DeviceID);
        WebRegFPDBInit(FPDBInit);
	WebRegFDBDelUser(FDB_DelUser);
	WebRegFDBAddUser(FDB_AddUser);
	WebRegFDBHandle(pSelectFDFromConentType(FCT_FINGERTMP),pSelectFDFromConentType(FCT_USER));
	WebRegFDBChgUser(FDB_ChgUser);
	WebRegFDBGetUser(FDB_GetUser);
	WebRegFDBCreateTemplate(FDB_CreateTemplate);
	WebRegFDBAddTmp(FDB_AddTmp);
	WebRegFDBGetFreeFingerID(FDB_GetFreeFingerID);
	WebRegMaxFingerCount(gOptions.MaxFingerCount);
	WebRegStringDecode(StringDecode);
	WebRegBIOKEY_SETTEMPLATELEN(BIOKEY_SETTEMPLATELEN);
        WebRegFDBGetTmp(FDB_GetTmp);
        WebRegBIOKEY_IDENTIFYTEMP(fhdl,BIOKEY_IDENTIFYTEMP);
        WebRegExt(gOptions.WorkCode,gOptions.AttLogExtendFormat);  //Update New Expand Struct at 2006.01.16
	WebRegIsOnlyRFMachine(gOptions.IsOnlyRFMachine); //Update RFID at 2006.01.18	
        WebStart();
	printf("Web Server Start Complete\n");
}

void WebFinish(void)
{
        WebFreeConfig(); 
	WebClose();
}
#endif

