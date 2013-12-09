/*************************************************
                                           
 ZEM 200                                          
                                                    
 finger.c                              
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "flashdb.h"
#include "finger.h"
#include "sensor.h"

int FPDBInit(void)
{
	if(fhdl)
	{
		if (gOptions.ZKFPVersion == ZKFPV10)
			BIOKEY_DB_CLEAR_10(fhdl);
		else
			BIOKEY_DB_CLEAR(fhdl);
		return (FDB_LoadTmp(fhdl));
	}
	return 0;
}

void FPFree(void)
{
	if (fhdl)
	{
		if (gOptions.ZKFPVersion == ZKFPV10)
			BIOKEY_DB_CLEAR_10(fhdl);
		else
			BIOKEY_DB_CLEAR(fhdl);
		free(fhdl);
	}
}

int FPInit(char *FingerCacheBuf)
{
	if(fhdl == NULL)	
		FPBaseInit(FingerCacheBuf);
	return FPDBInit();
}

