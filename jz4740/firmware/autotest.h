/*************************************************
                                           
 ZEM 200                                          
                                                    
 autotest.h                            
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef __AUTOTEST_H_
#define __AUTOTEST_H_

int DoAutoTestMenu(void *p);
int DoCheckAll(void *p);

//键盘检测(让用户按压)
int DoCheckKeypad(void *p);

//语音检测(液晶显示播放内容提示)
int DoCheckVoice(void *p);

//MP3语音检测(液晶显示播放内容提示)
int DoCheckMP3(void *p);

//液晶检查(在各个位置显示内容，可让用户判断是否有短横等) 由用户按键盘，程序自动显示，类似以前的DOS中的VGA的显示测试
int DoCheckLCD(void *p);

//FLASH冗余检查，例如全部所有可用空间写入读出一次或者两次
int DoCheckFlash(void *p);

//sensor test
int DoCheckFinger(void *p);

int DoCheckRTC(void *p);

#endif
