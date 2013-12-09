#ifndef _ACC_API_H_
#define _ACC_API_H_

#include "serial.h"
#include "msg.h"
#include "exfun.h"

/*********************************************************************
	         			����Ŷ���
*********************************************************************/
#define	  COM_C2_PARAM     (0x0001<<1)      //�������ò���
#define	  COM_C2_MON        (0x0001<<2)     //��������/�رռ��
#define	  COM_C2_QUERY      (0x0001<<3)     //������ѯ�ӻ�״̬����
#define	  COM_C2_ERR         (0x0001<<4)    //�����쳣��ͨѶ����
#define	  COM_C2_BIND        (0x0001<<5)   //�������������
#define	  COM_C2_ACK         (0x0001<<6)   //����Ӧ��
#define	  COM_C2_STATUS      (0x0001<<7)   //�ӻ�����״̬��Ϣ
#define	  COM_C2_OUT_LOCK   (0x0001<<8)   //����������
#define	  COM_C2_OUT_WARN   (0x0001<<9)   //��������
#define	  COM_C2_OUT_BELL   (0x0001<<10)   //��������
#define	  COM_C2_OUT_CH   (0x0001<<11)  //TCP/IP �� RS485/RS232 ͨ��ѡ��

/*********************************************************************
	         			����ֵ����
*********************************************************************/
#define    C2_OUT_OFF          0x01		    //����ر�
#define    C2_OUT_OPEN   		(0x01+1)		    //�������
#define    C2_MON_OFF   	    (0x01+2)			//��عر�
#define    C2_MON_OPEN    	(0x01+3)			//�������
#define    C2_DETECT_COM   	(0x01+4)			//�������
#define    C2_QUERY_COM   	(0x01+5)			//������ѯC2״̬
#define    C2_RUN_ERR   			(0x01+6)			//���д���
#define    C2_ACK_YES			(0x01+7)			//��Ӧȷ��
#define    C2_ACK_NO   		(0x01+8)			//��Ӧ����	
#define    C2_NO_ERR				(0x01+9��		//C2û�й��ϡ�
#define    C2_ERR_FLASH 		    (0x01+10)		//C2��������: FLASH�洢��
#define   C2_ERR_OTHER 			(0x01+11)  		//C2������ԭ��Ĺ��ϡ�
#define	  C2_RUN__NO_ERR     (0x01+12)		//�������


int ACCSendData(void *buf, int size, void *param);
int ACCRead(serial_driver_t *rs, BYTE *Data, WORD BindID, int TimeOutMS);
int ACCCheck(serial_driver_t *rs);
int SendC2Command(WORD Cmd, char *Data, int BufferLen, int TimeOutMS);
int ExAuxOutC2(int AuxOnTime, int OpenDoorDelay);
int ExAlarmC2(int Index, int DelayMS);
int ExBellC2(int DelaySec);
int ExSwithCommuC2(int Channel);
int ExSetParamC2(int OpenDoorDelay,int MonitorTimeOut,int UnMonitorTimeOut,int BeepDelay,int AlarmDelay);
int ExSetMonitorC2(int value);
int ExSetBindIDC2(int BindID);
int QueryC2(char* RetBuf);
int GetDoorState(char *buf,char *ret);

#endif
