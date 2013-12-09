#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include "app.h"
#include "serial.h"
//#include "net.h"
#include "utils.h"
#include "commu.h"
#include "options.h"
#include "lcdmenu.h"
#include "accapi.h"

typedef struct {
	WORD Cmd, BindID, Len;
}TACC232HDR, *PACC232HDR;

/*
数据格式
0x20+0x20+TACC232HDR+Data+CheckSum
*/

int ACCSendData(void *buf, int size, void *param)
{
	int i=0;
	serial_driver_t *rs=*(void **)param;
	BYTE *p=(BYTE*)(buf);
	int sum;

	//DBPRINTF("Send: 20 20 ");
	rs->write(0x20);rs->write(0x20);
	sum=0;
	while(i<size)
	{
		//DBPRINTF("%d ",p[i]);
		sum+=p[i];
		rs->write(p[i++]);
	}
	rs->write(sum & 0xFF);
	//DBPRINTF(" %d",sum);
	//DBPRINTF("\n");
	return 0;
}

int ACCRead(serial_driver_t *rs, BYTE *Data, WORD BindID, int TimeOutMS)
{
	char buf[1024];
	int sum=0,i,len=sizeof(TACC232HDR)+2, ch;
	U32 c,clen,TimeOutUS,start=GetUS();
	PACC232HDR h=(PACC232HDR)&buf[2];
	char *data=&buf[2+sizeof(TACC232HDR)-2]; //2+(20 20) -2(保留长度）
	c=start;
	TimeOutUS=(U32)TimeOutMS*1000;
	clen=0;
	//DBPRINTF("read: ");
	do{
		if(0==rs->poll())
		{
			DelayMS(5);//return 0;
		}
		else{
			i=0;
			while(i<=len)
			{
				while(!rs->poll())
					if((GetUS()-start>1000000) || (GetUS()<start))	//原(GetUS()-start>10000)
					{
						break;
					}
				ch=rs->read();
				//DBPRINTF(" %d",ch);
				buf[i]=ch;
				start=GetUS();
				if(i<2)
				{
					if(ch==0x20)
						i++;
					else
					{
						while(rs->poll()) rs->read();
						break;
						//return 0;
					}
				}
				else
				{
					i++;
		//#ifdef WIN32
		//			DelayMS(5);	//当PC上使用虚拟串口时太快，需要延时
		//#endif
					if(i==sizeof(TACC232HDR)+2)
					{
						len=h->Len+3;	//+3:20 20 sum
					}
					if(i<len)
						sum+=ch;
					else
					{
						if((sum & 0xFF)==ch)
						{
							int clen=h->Len-sizeof(TACC232HDR)+2;	//因为头结构的最后两个字节表示长度，所以在数据中保留这两个字段备用
							//DBPRINTF("clen: %d\tbindid: %d\n",clen,h->BindID);
							if(h->BindID==BindID || h->BindID==4369)
							{
								memcpy(Data, data, clen);
								if (h->BindID == 4369)
									return 4369;
							}
							else
								return -1;
							return clen;
						}
						else
						{
							return 0;
						}
					}
				}
			}
		}
	}while((GetUS()-c)<=TimeOutUS);
	return clen;
}

int SendC2Data(serial_driver_t *rs, WORD Cmd, WORD BindID, char *Buffer, WORD BufferLen)
{
	char buf[1024];
	int len;
	PACC232HDR hdr=(PACC232HDR)buf;
	hdr->Cmd=Cmd;
	hdr->BindID=BindID;
	hdr->Len=BufferLen+sizeof(TACC232HDR);
	if(BufferLen>0)
		memcpy(hdr+1,Buffer,BufferLen);
	len=hdr->Len;
	ACCSendData(buf,hdr->Len,&rs);
	return len;
}

extern serial_driver_t *gSlave232;
extern int gBindID;
extern int DoorOpenTimeLong;
extern int CurDoorState;
extern int DoorOpenTimeLong;
extern int gAuxOutDelay;
extern int gDoorSensorDelay;

int SendC2Command(WORD Cmd, char *Data, int BufferLen, int TimeOutMS)
{
	int res,c=0;
	//如果没有回应数据的话，重复发送数据三次
	while(c++<3)
	{
		res=SendC2Data(gSlave232, Cmd, gBindID, Data, BufferLen);
		if(res>0)
		{
			res=ACCRead(gSlave232,(BYTE*)Data,gBindID, TimeOutMS);
			//DBPRINTF("\n");
			//DBPRINTF("ACCRead res: %d\n",res);
			if(res>0) break;
		}
		else
			break;
	}
	return res;
}

int ExAuxOutC2(int AuxOnTime, int OpenDoorDelay)
{
	int ret,cmd;
	char buf[1024]={0};
	cmd=COM_C2_OUT_LOCK;
	if(AuxOnTime)
	{
		*(U8*)buf=C2_OUT_OPEN;
	}
	else
		*(U8*)buf=C2_OUT_OFF;
	ret=SendC2Command(cmd,buf,1,1000);
	if(ret<=0)
	{
		return FALSE;
	}
	return TRUE;
}

int ExAlarmC2(int Index, int DelayMS)
{
	int ret,cmd;
	char buf[1024]={0};
	cmd=COM_C2_OUT_WARN;
	if(DelayMS)
	{
		*(U8*)buf=C2_OUT_OPEN;
		gAlarmDelay=DelayMS;
	}
	else
		*(U8*)buf=C2_OUT_OFF;
	ret=SendC2Command(cmd,buf,1,1000);
	if(ret<=0)
	{
		//LCDInfoShow("C2 fun","ExAlarmC2 failed");
		return FALSE;
	}
	return TRUE;
}

int ExBellC2(int DelaySec)
{
	int ret,cmd;
	char buf[1024]={0};
	cmd=COM_C2_OUT_BELL;
	if(DelaySec)
	{
		*(U8*)buf=C2_OUT_OPEN;
	}
	else
		*(U8*)buf=C2_OUT_OFF;
	ret=SendC2Command(cmd,buf,1,1000);
	if(ret<=0)
	{
		//LCDInfoShow("C2 fun","Command failed");
		return FALSE;
	}
	return TRUE;
}

int ExSwithCommuC2(int Channel)
{
	int ret,cmd;
	char buf[1024]={0};
	cmd=COM_C2_OUT_CH;
	if(Channel)
	{
		*(U8*)buf=C2_OUT_OPEN;
	}
	else
		*(U8*)buf=C2_OUT_OFF;
	ret=SendC2Command(cmd,buf,1,1000);
	if(ret<=0)
	{
		//LCDInfoShow("C2 fun","sWitch failed");
		return FALSE;
	}
	return TRUE;
}

int ExSetParamC2(int OpenDoorDelay,int MonitorTimeOut,int UnMonitorTimeOut,int BeepDelay,int AlarmDelay)
{
	int ret,cmd;
	char buf[1024]={0};
	cmd=COM_C2_PARAM;
	*(U16*)buf=OpenDoorDelay*10;
	*(U16*)(buf+2)=MonitorTimeOut*10;
	*(U16*)(buf+4)=UnMonitorTimeOut*10;
	*(U16*)(buf+6)=BeepDelay*10;
	*(U16*)(buf+8)=AlarmDelay*10;
	ret=SendC2Command(cmd,buf,10,1000);
	if(ret<=0)
	{
		//LCDInfoShow("C2 fun","SetParam failed");
		return FALSE;
	}
	return TRUE;
}
/* 
int ExSetMonitorC2(int value)
{
	int ret,cmd;
	char buf[1024]={0};
	cmd=COM_C2_MON;
	if(value)
	{
		*(U8*)buf=C2_MON_OPEN;
	}
	else
		*(U8*)buf=C2_MON_OFF;
	ret=SendC2Command(cmd,buf,1,1000);
	if(ret<=0)
	{
		return FALSE;
	}
	return TRUE;
}
*/ //treckle
int ExSetBindIDC2(int BindID)
{
	int ret,cmd;
	char buf[1024]={0};
	cmd=COM_C2_BIND;
	ret=SendC2Command(cmd,buf,0,1000);
	if(ret<=0 || ((U8)(buf[4])==C2_ACK_NO))
	{
		//if (ret<=0)
		//	LCDInfo(LoadStrByID(MID_C2CONFAIL),2);
		//if ((U8)(buf[4])==C2_ACK_NO)
		//	LCDInfo(LoadStrByID(MID_C2NOFOUND),2);
			
		return FALSE;
	}else
	{
		return TRUE;
	}
}

int GetDoorState(char *buf,char *ret)
{
        int n=0;
        static BYTE DoorSensorStatus=DOOR_UNKNOWN;
        static BYTE DoorOpenSign=DOOR_UNKNOWN;
        BYTE status;
        status=0;
        if(!(ret[0]&0x16))      //未设置捆绑号
                ExSetBindIDC2(gBindID);
        ret[2]=buf[0]&0x01;
        n=buf[2]&0x80;
        if(n)
                status=gOptions.DoorSensorMode?DOOR_SENSOR_OPEN:DOOR_SENSOR_CLOSE;
        else
        {
                status=gOptions.DoorSensorMode?DOOR_SENSOR_CLOSE:DOOR_SENSOR_OPEN;
        }

        n=buf[3]&0x7F;
        if(n>0)
                status=DOOR_BUTTON;

        if (status && status!=DoorSensorStatus)
        {
                DoorSensorStatus=status;
                if(status==DOOR_SENSOR_OPEN)
                {
			if(gAuxOutDelay)
                        {
                                DoorSensorStatus=DOOR_SENSOR_OPEN;
                                //gDoorSensorDelay=gOptions.OpenDoorDelay;
                                DoorOpenSign=DOOR_SENSOR_OPEN; 
                        }
                        else
                        {
                                DoorSensorStatus=DOOR_SENSOR_BREAK;
                                DoorOpenSign=DOOR_SENSOR_BREAK;
                        }
                }
                else if(status==DOOR_SENSOR_CLOSE)
                {
                        gDoorSensorDelay=0;
                        DoorSensorStatus=DOOR_SENSOR_CLOSE;
                        DoorOpenSign=DOOR_SENSOR_CLOSE;
                }else if(status==DOOR_BUTTON)
                        DoorSensorStatus=DOOR_BUTTON;
                ret[0]=DoorSensorStatus;
                return DoorSensorStatus;
        }else
                return 0;
}




int QueryC2(char* RetBuf)
{
        int ret,cmd;
        char buf[1024]={0};
        char retbuf[10]={0};
        cmd=COM_C2_QUERY;
        buf[0]=C2_QUERY_COM;
        ret=SendC2Command(cmd,buf,1,1000);
	//DBPRINTF("QueryC2 ret=%d\n",ret);
        if(ret==0)
        {
		LCDClear();
		LCDInfo(LoadStrByID(MID_C2CONFAIL),2);
                return 0;
        }else if(ret==-1)
        {
		LCDClear();
		LCDInfo(LoadStrByID(MID_C2NOFOUND),2);
                if(*(U16*)buf==1)
		{
			LCDClear();
			//LCDInfo(LoadStrByID(MID_C2LOCKED),2);
			LCDInfo(LoadStrByID(MID_C2CONFAIL),2);
		}
                return ret;
        }else
        {
		if (ret==4369)
			return ret;
		ret=GetDoorState(buf+2,retbuf); //去掉数据中的长度字段2字节
        	if(RetBuf)
                	memcpy(RetBuf,retbuf,4);
                return ret;
        }
}

int ACCCheck(serial_driver_t *rs)
{
	char buf[2048];
	int sum=0,i,len=sizeof(TACC232HDR)+2, ch,ret;
	U32 start=GetUS();
	//PCmdHeader chdr=(PCmdHeader)&buf;
	PACC232HDR h=(PACC232HDR)&buf[2];
	char *data=&buf[len];

	if(0==rs->poll()) return 0;
	i=0;
	while(i<=len)
	{
		while(!rs->poll())
			if((GetUS()-start>1000000) || (GetUS()<start))	//原
			{
				//if(i<len)
				//ExBeepN(4);
				return 0;
			}
		ch=rs->read();
		buf[i]=ch;
		start=GetUS();
		if(i<2)
		{
			if(ch==0x20)
				i++;
			else
			{
				while(rs->poll()) rs->read();
				return 0;
			}
		}
		else
		{
			i++;
			if(i==(sizeof(TACC232HDR)+2))
				len=h->Len+3;	//20 20 sum
			if(i<len)
				sum+=ch;
			else
			{
				if((sum & 0xFF)==ch && h->Cmd==COM_C2_STATUS )
				{
					//返回消息事件

                                        char retbuf[10]={0};
                                        ret=GetDoorState(data,retbuf);
                                        return ret;


				}
				else
					return 0;
			}
		}
	}
	return len;
}
int ExSetMonitorC2(int value)
{
    int ret,cmd,curDoorSensorState=0;
    U8 buf[1024]={0};
    cmd=COM_C2_MON;
    if(value)
    {
        buf[0]=C2_MON_OPEN;
    }
    else 
        buf[0]=C2_MON_OFF;
    ret=SendC2Command(cmd,buf,1,100);
    if(ret<=0)
    {        
        //LCDInfoShow("C2 fun","Command failed",NULL);
        return FALSE;
    }    
    return TRUE;
}
