/*************************************************
                                           
 ZEM 200                                          
                                                    
 autotest.h                            
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef __AUTOTEST_H_
#define __AUTOTEST_H_

int DoAutoTestMenu(void *p);
int DoCheckAll(void *p);

//���̼��(���û���ѹ)
int DoCheckKeypad(void *p);

//�������(Һ����ʾ����������ʾ)
int DoCheckVoice(void *p);

//MP3�������(Һ����ʾ����������ʾ)
int DoCheckMP3(void *p);

//Һ�����(�ڸ���λ����ʾ���ݣ������û��ж��Ƿ��ж̺��) ���û������̣������Զ���ʾ��������ǰ��DOS�е�VGA����ʾ����
int DoCheckLCD(void *p);

//FLASH�����飬����ȫ�����п��ÿռ�д�����һ�λ�������
int DoCheckFlash(void *p);

//sensor test
int DoCheckFinger(void *p);

int DoCheckRTC(void *p);

#endif
