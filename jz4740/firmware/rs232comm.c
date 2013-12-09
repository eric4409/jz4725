/*************************************************
                                           
 ZEM 200                                          
                                                    
 rs232comm.c                                
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "arca.h"
#include "serial.h"
#include "utils.h"
#include "commu.h"
#include "options.h"
#include "exfun.h"
#include "rs232comm.h"
#include <ucos_ii.h>

#define CMD_RS 0x81
#define WORD 	unsigned short

#define THEBR gOptions.RS232BaudRate
#define THEDID gOptions.DeviceID

typedef struct _RSHeader_{
	unsigned char HeaderTag[2];
	unsigned char Cmd;
	unsigned char CmdCheck;
	WORD Len;
	WORD CheckSum;
}GCC_PACKED TRSHeader, *PRSHeader;

void RS_SendDataPack(serial_driver_t *rs, unsigned char *buf, unsigned short size)
{
	unsigned short cs;
	
	TMyBuf *packet=bget(1);
	PRSHeader ch=(PRSHeader)(packet->buf);
	memset(packet->buf, 0, sizeof(TRSHeader));
	packet->len=sizeof(TRSHeader)+size;
	ch->HeaderTag[0]=THEDID; ch->HeaderTag[1]=THEDID;
	ch->Cmd=(char)CMD_RS; 
	ch->CmdCheck=0xFF-CMD_RS;
	ch->Len=size;
	memcpy(packet->buf+sizeof(TRSHeader), buf, size);
	ch->CheckSum=in_chksum(packet->buf, packet->len);
	if(gOptions.RS485On) 
	{
		RS485_setmode(TRUE);
		cs=0;
		while(cs<packet->len)
		{ 
			rs->write(packet->buf[cs++]);

#ifdef ZEM300
			if(cs%256==0)
			{
				DelayUS(100*1000);
			}
#endif
		}
		//i don't know how to do here
		DBPRINTF("Data SEND %d\n", cs);

#ifdef ZEM300
		rs->tcdrain();
		DelayMS(3*115200/gOptions.RS232BaudRate);		
#else
		DelayMS(1.5*cs*9600/gOptions.RS232BaudRate);
#endif
		RS485_setmode(FALSE);
	}
	else
	{
		for(cs=0;cs<packet->len;cs++) 
		{
			rs->write(packet->buf[cs]);
		}
		//for(cs=0;cs<packet->len;cs++) rs->write(packet->buf[cs]);
	}
        rs->flush_output();
	packet->len=0;
}

int CloseHangUp(void *param)		
{
	DelayUS(1000*1000);
	SerialOutputString(*(void **)param, "+++");
	DelayUS(1000*1000);
	SerialOutputString(*(void **)param, "ATH0\r");
	return 0;
}

int SendRSData(void *buf, int size, void *param)
{
	RS_SendDataPack(*(void **)param, (unsigned char *)buf, (WORD)size);
	return 0;
}

WORD rsc=0, PS1[256], PS2[256], PSIndex=0;

int RS232Check(serial_driver_t *rs)
{
	int i, j;
	U32 ch,sum;
	TMyBuf *packet;
	PRSHeader pch;
	int timeout;
	unsigned char Receive;
	
	if(rs->poll()==0) return 0;
	i=0;sum=0;
	packet=bget(0);
	pch=(PRSHeader)(packet->buf);
	rsc++;

	while(1)
	{
		j=200;
		while(rs->poll()==0) 
		{
			DelayUS(200);
		
			if(++j>=2200)
			{
				if(i>8)	//FIFO溢出，要求重发
				{
					PCmdHeader chdr=(PCmdHeader)(packet->buf+sizeof(TRSHeader));
					PCommSession session=GetSession(chdr->SessionID);
					if(session)
					{
						chdr->Command=CMD_ACK_RETRY;
						chdr->CheckSum=0;
						chdr->CheckSum=in_chksum((BYTE*)chdr,sizeof(TCmdHeader));
						session->Send((char *)chdr,sizeof(TCmdHeader),session->Sender);
						PS1[PSIndex]=i;
						PS2[PSIndex++]=rsc;
						PSIndex &= 0xFF;
						j=0;	//等待重发
						i=0;
						sum=0;
						DBPRINTF("FIFO overflow, SO SEND CMD_ACK_RETRY\n");
						continue;
					}
				}
				return -1;
			}
		}
		ch = rs->read();
		packet->buf[i] = ch;
		//TESTING RS232 MODEM		
		if(i<=1)
		{
			if(gOptions.RS485On && (ch!=(U32)THEDID)) 
			{
			//	EnableDevice(TRUE);
				i=0;
				break;
			}
		}
		else if(i==3)
		{
			if(pch->Cmd+pch->CmdCheck!=0xFF) break;
			if(pch->Cmd!=CMD_RS) break;
		}
		else if(i==sizeof(TRSHeader)-1)
		{
			if(pch->Len>BUFSZ-sizeof(TRSHeader)) break;
		}
		else if(i==(int)(pch->Len+sizeof(TRSHeader)-1))
		{
			
			sum=pch->CheckSum;
			pch->CheckSum=0;
			
			if(sum==in_chksum(packet->buf, pch->Len+sizeof(TRSHeader)))
			{
				PCommSession session;

			//	DBPRINTF("CHECKSUM OK DataLength=%d\n", pch->Len);
				int cmd=RunCommand(packet->buf+sizeof(TRSHeader), pch->Len, TRUE);
				if(cmd==0) 
					return -2;
				if(cmd==CMD_CONNECT)
				{
					char Sender[SENDER_LEN];
					PCmdHeader chdr=(PCmdHeader)(packet->buf+sizeof(TRSHeader));
					memset(Sender, 0, SENDER_LEN);
					memcpy(Sender, (void*)&rs, sizeof(rs));
					session=CreateSession(Sender);
					chdr->SessionID=session->SessionID;
					chdr->CheckSum=0;
					chdr->CheckSum=in_chksum((void*)chdr,sizeof(TCmdHeader));
					session->Send=SendRSData;
					session->Speed=THEBR;
					SendRSData((void*)chdr, sizeof(TCmdHeader), Sender);
				}				
			}
			else
			{
				DBPRINTF("CHECKSUM ERROR!\n");
			}
			return pch->Cmd;
		}
		i++;
	}
	while(rs->poll()) ch=rs->read();
	return -1;	
}

int SerialInputStr(serial_driver_t *serial, char *s, const int len)
{
	int c;
	int i;
	int numRead;
	int skipNewline = 1;
	int maxRead = len;
	
	for(numRead=0, i=0; numRead<maxRead;)
	{
		c = serial->read();

		/* check for errors */
		if(c < 0) 
		{
			s[i++] = '\0';
			return c;
		}
		/* eat newline characters at start of string */
		if((skipNewline == 1) && (c != '\r') && (c != '\n'))
			skipNewline = 0;
		if(skipNewline == 0)
		{
			if((c == '\r') || (c == '\n'))
			{
				s[i++] = '\0';
				return(numRead);
			} 
			else 
			{
				s[i++] = (char)c;
				numRead++;
			}
		}
	}
	return(numRead);
}
/*
int CheckModemReady(serial_driver_t *serial)
{
	static int WaitInitModem=0; //Wait n seconds and init modem
	int WaitChars, i;
	char WaitResult[128];
	char *tmp;
	
	if (IsModemReady) return 1;
	if(WaitInitModem)
	{
		if(!--WaitInitModem)
		{
			WaitChars=serial->poll();
			//printf("WaitChars=%d\n", WaitChars);
			while(WaitChars)
			{
				memset(WaitResult, 0, 128);
				i=SerialInputStr(serial, WaitResult, WaitChars>=128?128:WaitChars);
				if (nstrcmp("OK", WaitResult, 2)==0)
				{
					IsModemReady=1;
					if ((tmp=LoadStrOld("ModemInitStr"))!=NULL)
						SerialOutputString(serial, tmp);
					else
						SerialOutputString(serial, "ATS0=3 &D0 S7=45 E0 V1 Q1\r");
					break;
				}
				if(i<0) break;
				WaitChars=serial->poll();
			}
	       }
	}
	else
	{	
		serial->flush_input();
		SerialOutputString(serial, "AT\r");
		WaitInitModem=3;
	}
	return IsModemReady;
}
*/
