/*************************************************
                                           
 ZEM 200                                          
                                                    
 rs232comm.c                                
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef __FPCardMng_h__
#define __FPCardMng_h__

//Enroll a fingerprint template. store it to tmp and return the length of template.
int EnrollAFinger(char *tmp, int *len, int pin, int fingerid);

int DoCreateCard(void *p);
int DoEnrollCard(void *p);
int DoRegisterCard(void *p);
int DoUnRegCard(void *p);
int DoEmptyCard(void *p);
int DoDumpFPCard(void *p);
int DoMoveToCard(void *p);
int DoCreatePINCard(void *p);

int DoCardMng(void *p);


#endif
