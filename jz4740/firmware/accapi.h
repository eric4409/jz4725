#ifndef _ACC_API_H_
#define _ACC_API_H_

#include "serial.h"
#include "msg.h"
#include "exfun.h"

/*********************************************************************
	         			命令号定义
*********************************************************************/
#define	  COM_C2_PARAM     (0x0001<<1)      //主机设置参数
#define	  COM_C2_MON        (0x0001<<2)     //主机启动/关闭监控
#define	  COM_C2_QUERY      (0x0001<<3)     //主机查询从机状态命令
#define	  COM_C2_ERR         (0x0001<<4)    //主机异常或通讯故障
#define	  COM_C2_BIND        (0x0001<<5)   //主机设置捆绑号
#define	  COM_C2_ACK         (0x0001<<6)   //主机应答
#define	  COM_C2_STATUS      (0x0001<<7)   //从机发送状态消息
#define	  COM_C2_OUT_LOCK   (0x0001<<8)   //开关锁命令
#define	  COM_C2_OUT_WARN   (0x0001<<9)   //报警命令
#define	  COM_C2_OUT_BELL   (0x0001<<10)   //响铃命令
#define	  COM_C2_OUT_CH   (0x0001<<11)  //TCP/IP 和 RS485/RS232 通道选择

/*********************************************************************
	         			数据值定义
*********************************************************************/
#define    C2_OUT_OFF          0x01		    //输出关闭
#define    C2_OUT_OPEN   		(0x01+1)		    //输出启动
#define    C2_MON_OFF   	    (0x01+2)			//监控关闭
#define    C2_MON_OPEN    	(0x01+3)			//监控启动
#define    C2_DETECT_COM   	(0x01+4)			//主机检测
#define    C2_QUERY_COM   	(0x01+5)			//主机查询C2状态
#define    C2_RUN_ERR   			(0x01+6)			//运行错误
#define    C2_ACK_YES			(0x01+7)			//回应确认
#define    C2_ACK_NO   		(0x01+8)			//回应错误	
#define    C2_NO_ERR				(0x01+9）		//C2没有故障。
#define    C2_ERR_FLASH 		    (0x01+10)		//C2故障类型: FLASH存储。
#define   C2_ERR_OTHER 			(0x01+11)  		//C2有其它原因的故障。
#define	  C2_RUN__NO_ERR     (0x01+12)		//解除错误


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
