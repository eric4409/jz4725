/*************************************************
  
 ZEM 200                                          
 
 net.c Simple network layer with UDP & TCP                              
 
 Copyright (C) 2003-2004, ZKSoftware Inc.      
 
 $Log: net.c,v $
 Revision 5.11  2005/12/22 08:54:23  david
 Add workcode and PIN2 support

 Revision 5.10  2005/11/06 02:41:34  david
 Fixed RTC Bug(Synchronize time per hour)

 Revision 5.9  2005/09/19 10:01:59  david
 Add AuthServer Function

 Revision 5.8  2005/08/07 08:13:15  david
 Modfiy Red&Green LED and Beep

 Revision 5.7  2005/08/02 16:07:51  david
 Add Mifare function&Duress function

 Revision 5.6  2005/07/14 16:59:53  david
 Add update firmware by SDK and U-disk
 
*************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
//#define __KERNEL__
//#include <asm/types.h>
//#include <linux/sockios.h>
//#include <linux/ethtool.h>
#include "arca.h"
#include "net.h"
#include "utils.h"
#include "commu.h"
#include "options.h"
#include "exfun.h"
#include "main.h"
#include "flashdb.h"
#include "sensor.h"

#define THEIP gOptions.IPAddress
#define THEUDP gOptions.UDPPort
#define BROADCASTPORT 65535

static int comm_socket,listenAuthServer_socket, receive_broadcast_socket; //UDP
static int server_socket; //TCP listen 
fd_set rfds;
struct timeval tv;

static int image_socket=-1; //TCP AuthServer
int AuthServerSessionCount=0;
PAuthServerSession AuthServerSessions=NULL;
int MaxAuthServerSessionCount=0;
int StaticDNSCount=0;

//send data with UDP protocol
//buf - buffer will be send
//size - length of data buffer
//param - this parameter save the ip address and port number of receiver, pls refer E TFPResult, *PFPResultthCommCheck function  
//该参数用于识别接收数据方的ip地址和端口号，该参数由 eth_rx 在一个通讯会话建立时保存下来。
int SendUDPData(void *buf, int size, void *param)
{
	return sendto(comm_socket, buf, size, 0, (struct sockaddr*)param, sizeof(struct sockaddr_in));
}

//检查是否有接收到的UDP数据，若有的话，进行处理；
//Check whether data arrival or not
int EthCommCheck(void)
{
	struct sockaddr_in from;
	char buf[1032];
	int cmd; 
	int fromlen=sizeof(from);
	//Maximize data packet size is 1032 
	int len = recvfrom(comm_socket, buf, 1032, 0, (struct sockaddr*)&from, &fromlen);
	if(len>0){
		//把数据包交由RunCommand处理
		//RunCommand be called to process data packet
		if ((cmd = RunCommand(buf, len, TRUE)) == 0) return -2;  
		//如果是请求建立连接命令，则建立连接会话，并保存通讯参数
		//the client ask to make a connection session 
		if (cmd == CMD_CONNECT){
			char Sender[SENDER_LEN];		 
			PCmdHeader chdr=(PCmdHeader)buf;
			
			//生成通讯参数并创建通讯会话
			//也即在SendUDPData中，可以借此参数获得对方的ip地址和端口号，才能发送数据
			memset(Sender, 0, SENDER_LEN);
			memcpy(Sender, (void*)&from, sizeof(struct sockaddr));
			PCommSession session=CreateSession(Sender);
			
			//向对方发送连接成功的会话标识ID
			chdr->SessionID=session->SessionID;
			chdr->CheckSum=0;
			chdr->CheckSum=in_chksum(buf, sizeof(TCmdHeader));
			session->Send=SendUDPData;
			session->Speed=10*1024*1024;
			//注意session->Sender作为保存的通讯参数，在以后的通讯过程中向对方发送数据时使用，
			SendUDPData(chdr, sizeof(TCmdHeader), session->Sender); 
		}               
	}
	return 0;
}

int EthBoradcastCheck(void)
{
	struct sockaddr_in from;
	char buf[128];
	int fromlen=sizeof(from);
	//Maximize data packet size is 1032 
	int len = recvfrom(receive_broadcast_socket, buf, 128, 0, (struct sockaddr*)&from, &fromlen);
	if(len>0)
	{
		if(buf[0]=='X')
		{
			sprintf(buf, "X%s/%s", LoadStrOld("MAC"), LoadStrOld("IPAddress"));
			DBPRINTF("Yes! I got message for X then response %s len=%d\n", buf, strlen(buf));
			return sendto(comm_socket, buf, strlen(buf)+1, 0, (struct sockaddr*)&from, sizeof(struct sockaddr_in));
		}
	}
	return 0;
}

//TCP communication then close the temp socket
int CloseTCPSocket(void *param)
{
	int tmp_server_socket;	
	memcpy(&tmp_server_socket, param, 4);
	close(tmp_server_socket);
	return 1;
}

int SendTCPData(void *buf, int size, void *param)
{
	int tmp_server_socket;	
	memcpy(&tmp_server_socket, param, 4);
	return send(tmp_server_socket, buf, size, 0);
}

int ProcessTCPPackage(void *buf, int len, int tmp_server_socket)
{	
	int cmd; 
	//把数据包交由RunCommand处理
	//RunCommand be called to process data packet
	if ((cmd = RunCommand(buf, len, TRUE)) == 0) return -2;  
	//如果是请求建立连接命令，则建立连接会话，并保存通讯参数
	//the client ask to make a connection session 
	if (cmd == CMD_CONNECT)
	{
		char Sender[SENDER_LEN];
		PCmdHeader chdr=(PCmdHeader)buf;				
		//保存会话句柄
		memset(Sender, 0, SENDER_LEN);
		memcpy(Sender, (void*)&tmp_server_socket, 4);
		PCommSession session=CreateSession(Sender);
		//向对方发送连接成功的会话标识ID
		chdr->SessionID=session->SessionID;
		chdr->CheckSum=0;
		chdr->CheckSum=in_chksum(buf, sizeof(TCmdHeader));
		session->Send=SendTCPData;
		session->Speed=10*1024*1024;
		session->Close=CloseTCPSocket;
		strcpy(session->Interface, "TCP");
		//注意session->Sender作为保存的通讯参数，在以后的通讯过程中向对方发送数据时使用
		SendTCPData(chdr, sizeof(TCmdHeader), session->Sender); 
	}
	return 0;
}

extern int CommSessionCount;
extern PCommSession CommSessions;
//check tcp communication
int EthCommTCPCheck(void)
{
	int address_size;
	static struct sockaddr_in pin;
	char buf[1032];
	long save_file_flags;
	int tmp_server_socket;
	static int cur_comm_count=0;
	int rc=0;	  
	int retval; 
	
	//whether new tcp connection is coming or not 
	retval=select(server_socket+1, &rfds, NULL, NULL, &tv);
	if((retval>0)&&FD_ISSET(server_socket, &rfds))
	{
		tmp_server_socket=accept(server_socket, (struct sockaddr *)&pin, &address_size);
		if (tmp_server_socket!=-1)
		{		
			FD_ZERO(&rfds);
			FD_SET(server_socket, &rfds);
			tv.tv_sec=0;
			tv.tv_usec=0;
			//check whether data arrival or not, Maximize data packet size is 1032 
			int len=recv(tmp_server_socket, buf, 1032, 0);
			if(len>0)
			{
				//set socket to non-blocking
				save_file_flags=fcntl(tmp_server_socket, F_GETFL);
				save_file_flags|=O_NONBLOCK;
				fcntl(tmp_server_socket, F_SETFL, save_file_flags);
				//process the data
				rc=ProcessTCPPackage(buf, len, tmp_server_socket);
			}
			else
				close(tmp_server_socket);
		}
	}
	else
	{
		if (cur_comm_count>=CommSessionCount)	
			cur_comm_count=0;	
		if ((CommSessionCount>0)&&(nstrcmp("TCP", CommSessions[cur_comm_count].Interface, 3)==0))
		{
			memcpy(&tmp_server_socket, CommSessions[cur_comm_count].Sender, 4);
			//check whether data arrival or not, Maximize data packet size is 1032 
			int len=recv(tmp_server_socket, buf, 1032, 0);
			if(len>0)
				rc=ProcessTCPPackage(buf, len, tmp_server_socket);
		}
		cur_comm_count++;
	}
	return rc;
}

int InitUDPSocket(U16 port, int *udpsocket) 
{
	struct sockaddr_in sin;
	long save_file_flags;
	
	//Initialize socket address structure for internet protocol
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(port);
	
	//create a receive UDP Scoket
	*udpsocket=socket(AF_INET, SOCK_DGRAM, 0);
	if (*udpsocket==-1) return -1; 	 
	//bind it to the port
	if (bind(*udpsocket, (struct sockaddr *)&sin, sizeof(sin))==-1)
	{
		close(*udpsocket);
		*udpsocket=-1;
		return -1;
	}
	//set socket to non-blocking
	save_file_flags = fcntl(*udpsocket, F_GETFL);
	save_file_flags |= O_NONBLOCK;
	if (fcntl(*udpsocket, F_SETFL, save_file_flags) == -1)
	{
		close(*udpsocket);
		*udpsocket=-1;
		return -1;
	}
	return 0;
}	

void ForceClearMACCacheIP(unsigned char *ipaddress)
{
	char buffer[128];
	struct sockaddr_in pin;
	char msg[128];
	int i;
    
	if(ipaddress[0])
	{
		bzero(&pin, sizeof(pin));
		sprintf(buffer, "%d.%d.%d.%d", ipaddress[0], ipaddress[1], ipaddress[2], ipaddress[3]);
		pin.sin_family=AF_INET;
		pin.sin_addr.s_addr=inet_addr(buffer);
		pin.sin_port=htons(THEUDP+1);
		if(gOptions.StartUpNotify&2)
		{
			sprintf(msg, "\"%s-%s-%d\" Started.", DeviceName, SerialNumber, gOptions.DeviceID);
			for(i=0;i<5;i++)
				sendto(comm_socket, msg, strlen(msg), 0, (struct sockaddr*)&pin, sizeof(struct sockaddr_in));
		}
	}
}

//初始化UDP通讯，一般是建立通讯的UDP socket.	
//Initilization UDP 
int EthInit(void)
{
	struct sockaddr_in sin;
	
	if(InitUDPSocket(THEUDP, &comm_socket)!=0) return -1;
	if(InitUDPSocket(THEUDP+1, &listenAuthServer_socket)!=0) return -1;
	if(InitUDPSocket(BROADCASTPORT, &receive_broadcast_socket)!=0) return -1;
	//Force gateway router clear cached IP Address infomation
	ForceClearMACCacheIP(gOptions.GATEIPAddress);	
	//create a tcp socket for monitor connection from clients
	if ((server_socket=socket(AF_INET, SOCK_STREAM, 0))==-1) return -1;
	//bind tcp socket with current parameter
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(gOptions.TCPPort);
	if (bind(server_socket, (struct sockaddr *)&sin, sizeof(sin))==-1) return -1; 
	if (listen(server_socket, 1)==-1) return -1;
	FD_ZERO(&rfds);
	FD_SET(server_socket, &rfds);
	tv.tv_sec=0;
	tv.tv_usec=0;
	return 0; 
}

void EthFree(void)
{
	if (comm_socket>0) close(comm_socket);
	if (server_socket>0) close(server_socket);	
	if (listenAuthServer_socket>0) close(listenAuthServer_socket);
	if (receive_broadcast_socket>0) close(receive_broadcast_socket);
}

/*
int ethernet_cmd(const char *ifname, u16 speed, u8 duplex)
{
	struct ethtool_cmd cmd;
	 struct ifreq ifr;
	int s, i;
	char *media[] = {
		"10BaseT-HD ", "10BaseT-FD ","100baseTx-HD ",
		"100baseTx-FD", "100baseT4", 0
	};
	
	s = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (s == -1) {
		return 0;
	}
	strcpy(ifr.ifr_name, ifname);
	
	ifr.ifr_data = (__caddr_t )&cmd;
	
	cmd.cmd = ETHTOOL_GSET;
	
	if (ioctl(s, SIOCETHTOOL, &ifr) == -1) {
		return 0;
	}
	
	DBPRINTF("Provide Mode All: ");
	for (i=0;i<5;i++)
	    if (cmd.advertising & (1<<i))
		DBPRINTF("(%d)%s",i+1,media[i]);
	DBPRINTF("\n");
	
	cmd.cmd = ETHTOOL_SSET;
	cmd.speed= speed;
	cmd.duplex= duplex;
	cmd.autoneg=(speed==0)?AUTONEG_ENABLE:AUTONEG_DISABLE;	
	
	if (ioctl(s, SIOCETHTOOL, &ifr) == -1) {
		return 0;
	}
	
	return 1;
}
*/


/*----------------------Auth Server------------------------ */

BOOL ConnectWithTimeout(int sockfd, const struct sockaddr *serv_addr,
			socklen_t addrlen, int timeout)
{	
	struct timeval tm;
	fd_set set;
	BOOL ret = FALSE;
	int error=-1, len;
	long save_file_flags;
	
	save_file_flags=fcntl(sockfd, F_GETFL);
	//set the NOBLOCK flags to file descriptor
	fcntl(sockfd, F_SETFL, save_file_flags|O_NONBLOCK);
	if(connect(sockfd, serv_addr, addrlen)==-1)
	{
		tm.tv_sec  = timeout;
		tm.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(sockfd, &set);
		if(select(sockfd+1, NULL, &set, NULL, &tm)>0)
		{
			len=sizeof(int);
			getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
			if(error==0) 
				ret=TRUE;
			else 
				ret=FALSE;
			DBPRINTF("Data arrival! and Error=%d\n", error);
		} 
		else
		{ 
			ret=FALSE;
			DBPRINTF("select timeout!\n");
		}
	}
	else 
		ret=TRUE;
	
	//restore old file status flags
	fcntl(sockfd, F_SETFL, save_file_flags);
	
	return ret;
}

BOOL SendDataToAuthServer(U16 Command, void *Header, U16 HeaderLen, 
			  void *Data, U32 DataLen)
{
	int totalSent=0;
	int byteSent=0;	
	U32 size;
	unsigned int s_msec, e_msec;
	
	size=sizeof(U16)+HeaderLen+DataLen;	
	DBPRINTF("Data sending......\n");
	s_msec=GetTickCount();       
	//Send Size Command Buffer(Header+Data)
	if((send(image_socket, &size, sizeof(U32), MSG_NOSIGNAL)==sizeof(U32))&&
	   (send(image_socket, &Command, sizeof(U16), MSG_NOSIGNAL)==sizeof(U16))&&
	   ((Header==NULL)||(send(image_socket, Header, HeaderLen, MSG_NOSIGNAL)==HeaderLen)))
	{
		e_msec=GetTickCount();
		DBPRINTF("Packet Header Sending FINISHED!time=%d\n", e_msec-s_msec);
		if(Data)
		{
			do
			{
				DBPRINTF("Sending Data....\n");
				byteSent=send(image_socket, (char *)Data+totalSent, DataLen-totalSent, MSG_NOSIGNAL);
				DBPRINTF("Sending Data....DataLen=%d byteSent=%d totalSent=%d\n", DataLen, byteSent, totalSent);
				if (byteSent==-1) break;
				totalSent+=byteSent;
			}while(totalSent<DataLen);
			e_msec=GetTickCount();
			DBPRINTF("Packet Data sending FINISHED!time=%d Data=%d\n", e_msec-s_msec, totalSent);
			if (totalSent==DataLen)
			{
				DBPRINTF("Packet Data have already sent to server successfuly!\n");
				return TRUE;
			}
		}
		else
		{
			DBPRINTF("Packet No Data!\n"); 
			return TRUE; 
		}
	}
	else
		DBPRINTF("Packet Header sending FAILED!\n"); 
	
	return FALSE;
}

BOOL GetDataFromAuthServer(void *AuthResult, U32 Len)
{
	U32 size;
	unsigned int s_msec, e_msec;
	void *buffer;
	
	DBPRINTF("Receiving......\n");
	s_msec=GetTickCount();       	
	if(recv(image_socket, &size, sizeof(U32), 0)==sizeof(U32))
	{
		DBPRINTF("Data reply Len=%d!\n", size);
		memset(AuthResult, 0, Len);
		if(size==0)
		{
			return FALSE;
		}
		if(size>Len)
		{
			buffer=malloc(size);
			if(recv(image_socket, buffer, size, 0)==size)
			{
				memcpy(AuthResult, buffer, Len);
				e_msec=GetTickCount();
				DBPRINTF("Data received!time=%d\n", e_msec-s_msec);	
				return TRUE;
			}
			free(buffer);			
		}
		else
		{
			if(recv(image_socket, AuthResult, size, 0)==size)
			{
				e_msec=GetTickCount();
				DBPRINTF("Data received!time=%d\n", e_msec-s_msec);	
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL PrcocessCMD(int sockfd, const struct sockaddr *serv_addr,
		 U16 Command, void *Header, U16 HeaderLen, void *Data, U32 DataLen,
		 void *AuthResult, U32 Len)
{
	return (ConnectWithTimeout(sockfd, (void *)serv_addr, sizeof(*serv_addr), 5)&&
		SendDataToAuthServer(Command, Header, HeaderLen, Data, DataLen)&&  
		GetDataFromAuthServer(AuthResult, Len));
}

BOOL ExecuteCMD(U16 Command, void *Header, U16 HeaderLen, void *Data, U32 DataLen,
		void *AuthResult, U32 Len)
{
	struct sockaddr_in pin;
	int i;
	BOOL rc=FALSE;
	
	if(!CheckAllAuthServerRuning()) return FALSE;
	
	bzero(&pin, sizeof(pin));
	pin.sin_family=AF_INET;
	pin.sin_port=htons(gOptions.TCPPort);
	
	if(image_socket!=-1) close(image_socket);
	image_socket=socket(AF_INET, SOCK_STREAM, 0);
	if(image_socket==-1)
		DBPRINTF("Image socket create error!\n");
	
	switch(Command)
	{
	case AUTH_CMD_IMAGE_IDENTIFY:
	case AUTH_CMD_ACQUIRE_DNSADDRS:
	case AUTH_CMD_UPLOAD_IDINFO:
	case AUTH_CMD_UPLOAD_ATTLOG:
	case AUTH_CMD_UPLOAD_PHOTO:
	case AUTH_CMD_REFRESH_USERDATA:
		for(i=0;i<AuthServerSessionCount;i++)
		{
			if(AuthServerSessions[i].Connected)
			{
				pin.sin_addr.s_addr=AuthServerSessions[i].sin.sin_addr.s_addr;
				//create a send tcp socket for auth server
				if(image_socket!=-1) close(image_socket);
				image_socket=socket(AF_INET, SOCK_STREAM, 0);
				if(image_socket==-1)
				{
					DBPRINTF("Image socket create error!\n");
					break;
				}
				else
				{
					
					if(PrcocessCMD(image_socket, (struct sockaddr *)&pin, 
						       Command, Header, HeaderLen, Data, DataLen,
						       AuthResult, Len))
					{
						rc=TRUE;
						break;
					}
					close(image_socket);
					image_socket=-1;
				}
			}
		}
		break;
	case AUTH_CMD_FINGER_REGISTER:
	case AUTH_CMD_ACQUIRE_FREEID:
	case AUTH_CMD_CHECK_ISIDLEID:
	case AUTH_CMD_SYNC_LOCALTIME:
		pin.sin_addr.s_addr=AuthServerSessions[0].sin.sin_addr.s_addr;
		if(PrcocessCMD(image_socket, (struct sockaddr *)&pin, 
			       Command, Header, HeaderLen, Data, DataLen,
			       AuthResult, Len))
			rc=TRUE;
		break;
	default:
		break;
	}	
	if(image_socket!=-1)
	{
		close(image_socket);
		image_socket=-1;
	}
	return rc;
}

/*-------------------AuthServer system fucntions-------------------*/

int CheckAuthServerSessionTimeOut(int TimeOutSec)
{
	int i;
	for(i=0;i<AuthServerSessionCount;i++)
	{
		AuthServerSessions[i].LastActive++;
		if(AuthServerSessions[i].LastActive>TimeOutSec)
		{
			if(AuthServerSessions[i].Connected)
			{
				AuthServerSessions[i].Connected=FALSE;
			}
			AuthServerSessions[i].LastActive=0;
			DBPRINTF("AuthServer %s is Down!\n", inet_ntoa(AuthServerSessions[i].sin.sin_addr));
		}
	}
	return 0;
}

int EthAuthServerCheck(void)
{
	struct sockaddr_in from;
	char buf[1032];
	int fromlen=sizeof(from);
	int len, i;	
	
	len=recvfrom(listenAuthServer_socket, buf, 1032, 0, (struct sockaddr*)&from, &fromlen);
	if(len>0)
	{
		DBPRINTF("REQ Received\n");
		for(i=0;i<AuthServerSessionCount;i++)
		{
			if(memcmp(&from.sin_addr, &(AuthServerSessions[i].sin.sin_addr), sizeof(from.sin_addr))==0)
			{
				AuthServerSessions[i].Connected=TRUE;
				AuthServerSessions[i].LastActive=0;
				DBPRINTF("AuthServer %s is Up!\n", inet_ntoa(AuthServerSessions[i].sin.sin_addr));
			}
		}		
	}
	return 0;
}

int AuthServerREQ(int TimeInterval)
{
	int i;
	
	for(i=0;i<AuthServerSessionCount;i++)
	{
		if((EncodeTime(&gCurTime)-AuthServerSessions[i].LastREQ)>TimeInterval)
		{
			sendto(comm_socket, "REQ", 3, 0, (struct sockaddr*)&(AuthServerSessions[i].sin), sizeof(struct sockaddr_in));
			AuthServerSessions[i].LastREQ=EncodeTime(&gCurTime);
			DBPRINTF("AuthServer %s REQ is sending!\n", inet_ntoa(AuthServerSessions[i].sin.sin_addr));
		}
	}
	return 0;
}

int InitAuthServer(void)
{
	int i;
	char *hostname;
	char tmp[128];
	struct sockaddr_in pin;
	void *newp;
	BYTE ip[16];
	
	if (AuthServerSessions) free(AuthServerSessions);
	AuthServerSessionCount=0;
	MaxAuthServerSessionCount=0;
	i=0;
	StaticDNSCount=0;
	while(TRUE)
	{
		if(i==0)
			sprintf(tmp, "AuthServerIP");
		else
			sprintf(tmp, "AuthServerIP%d", i);
		hostname=LoadStrOld(tmp);
		if(hostname)
		{
			if((str2ip(hostname, ip)==0)&&(ip[0]!=0))
			{
				if((AuthServerSessionCount+1)>MaxAuthServerSessionCount)
				{
					MaxAuthServerSessionCount+=5;
					newp=malloc(sizeof(TAuthServerSession)*MaxAuthServerSessionCount);
					if(AuthServerSessionCount)
					{
						memcpy(newp, AuthServerSessions, AuthServerSessionCount*sizeof(TAuthServerSession));
						free(AuthServerSessions);
					}
					AuthServerSessions=newp;
				}			
				bzero(&pin, sizeof(pin));
				pin.sin_family=AF_INET;
				pin.sin_addr.s_addr=inet_addr(hostname);
				pin.sin_port=htons(THEUDP+1);
				memcpy(&(AuthServerSessions[i].sin), &pin, sizeof(pin));
				AuthServerSessions[i].LastActive=0;
				AuthServerSessions[i].LastREQ=EncodeTime(&gCurTime)-10;
				AuthServerSessions[i].Connected=FALSE;
				AuthServerSessionCount++;
			}
		}
		else break;
		i++;
	}
	StaticDNSCount=AuthServerSessionCount;
	return 0;
}

int FreeAuthServer(void)
{
	free(AuthServerSessions);
	AuthServerSessionCount=0;
	return 0;
}

BOOL CheckAllAuthServerRuning(void)
{
	int i;
	
	for(i=0;i<AuthServerSessionCount;i++)
	{
		if(AuthServerSessions[i].Connected) return TRUE;
	}
	return FALSE;
}

/*-----------Auth Server application function--------------*/

BOOL SendImageAndIdentify(char *buffer, int size, U32 pin2, PFPResult result)
{
	TFPIdentify header;
	
	header.MachineNumber=gOptions.DeviceID;
	header.PIN=pin2;
	header.ImageSize=size;
	
	return ExecuteCMD(AUTH_CMD_IMAGE_IDENTIFY, (void *)&header, sizeof(header), 
			  buffer, size, (void *)result, sizeof(TFPResult));
}

BOOL SendImageToRegister(char *buffer, int size, U32 pin2, U8 Index, U16 *result)
{
	TFPRegister header;
	
	header.MachineNumber=gOptions.DeviceID;
	header.PIN=pin2;
	header.ImageSize=size;
	header.Index=Index;
	
	return ExecuteCMD(AUTH_CMD_FINGER_REGISTER, (void *)&header, sizeof(header), 
			  buffer, size, (void *)result, sizeof(U16));
}

BOOL GetFreeIDFromAuthServer(U32 *result)
{
	BYTE DeviceID;
		
	DeviceID=gOptions.DeviceID;
	return ExecuteCMD(AUTH_CMD_ACQUIRE_FREEID, (void *)&DeviceID, sizeof(U8),
			  NULL, 0, (void *)result, sizeof(U32));
}

BOOL CheckIsIdleIDFromAuthServer(U32 pin2, U8 *result)
{
	TFPID header;
	
	header.MachineNumber=gOptions.DeviceID;
	header.PIN=pin2;
	
	return ExecuteCMD(AUTH_CMD_CHECK_ISIDLEID, (void *)&header, sizeof(header),
			  NULL, 0, (void *)result, sizeof(U8));	
}

BOOL GetDNSAddrsFromAuthServer(U8 *result, int len)
{
	BYTE DeviceID;
		
	DeviceID=gOptions.DeviceID;
	return ExecuteCMD(AUTH_CMD_ACQUIRE_DNSADDRS, (void *)&DeviceID, sizeof(U8),
			  NULL, 0, (void *)result, len);
}

BOOL UploadIDCardToAuthServer(U32 cardnumber, PFPResult result)
{
	TFPID header;
	
	header.MachineNumber=gOptions.DeviceID;
	header.PIN=cardnumber;
	
	return ExecuteCMD(AUTH_CMD_UPLOAD_IDINFO, (void *)&header, sizeof(header),
			  NULL, 0, (void *)result, sizeof(TFPResult));	
}

BOOL UploadAttlogToAuthServer(char *buffer, int size, U8 *result)
{
	TFPID header;
	
	header.MachineNumber=gOptions.DeviceID;
	header.PIN=size;
	
	return ExecuteCMD(AUTH_CMD_UPLOAD_ATTLOG, (void *)&header, sizeof(header),
			  buffer, size, (void *)result, sizeof(U8));	
}

BOOL GetUserDataFromAuthServer(U8 *result, int len)
{
	BYTE DeviceID;
		
	DeviceID=gOptions.DeviceID;
	return ExecuteCMD(AUTH_CMD_REFRESH_USERDATA, (void *)&DeviceID, sizeof(U8),
			  (void *)&len, 4, (void *)result, len);
}

int RefreshAuthServerByDNS(U8 *result, int len)
{
	char tmp[128];
	struct sockaddr_in pin;
	void *newp;
	U8 *p;
	int i;
	BOOL bSign;
	
	AuthServerSessionCount=StaticDNSCount;
	p=result;
	while(TRUE)
	{
		if(!p[0]) break;
		sprintf(tmp, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
		DBPRINTF("%s\n", tmp);
		if(tmp)
		{		
			if((AuthServerSessionCount+1)>MaxAuthServerSessionCount)
			{
				MaxAuthServerSessionCount+=5;
				newp=malloc(sizeof(TAuthServerSession)*MaxAuthServerSessionCount);
				if(AuthServerSessionCount)
				{
					memcpy(newp, AuthServerSessions, AuthServerSessionCount*sizeof(TAuthServerSession));
					free(AuthServerSessions);
				}
				AuthServerSessions=newp;
			}			
			bzero(&pin, sizeof(pin));
			pin.sin_family=AF_INET;
			pin.sin_addr.s_addr=inet_addr(tmp);
			pin.sin_port=htons(THEUDP+1);
			
			bSign=TRUE;
			//check whether exist or not
			for(i=0;i<StaticDNSCount;i++)
			{
				if(memcmp(&pin.sin_addr, &(AuthServerSessions[i].sin.sin_addr), sizeof(pin.sin_addr))==0)
				{
					bSign=FALSE;
					break;
				}
			}	
			if(bSign)
			{
				memcpy(&(AuthServerSessions[AuthServerSessionCount].sin), &pin, sizeof(pin));
				AuthServerSessions[AuthServerSessionCount].LastActive=0;
				AuthServerSessions[AuthServerSessionCount].LastREQ=EncodeTime(&gCurTime)-10;
				AuthServerSessions[AuthServerSessionCount].Connected=FALSE;
				AuthServerSessionCount++;
			}
		}
		//next dns ip address
		if((p-result+4)>len) break;				
		p+=4;
	}
	return 0;
}

void RefreshAuthServerListFromAuthServer(void)
{
	const int MaxAuthDNSCount=15; 
	U8 buffer[MaxAuthDNSCount*4];
	
	if(GetDNSAddrsFromAuthServer(buffer, MaxAuthDNSCount*4))
		RefreshAuthServerByDNS(buffer, MaxAuthDNSCount*4);
}

void UploadAttLogByAuthServer(void)
{
	char *buffer;
	int size;
	BYTE ret=0;
	
	if(CheckAllAuthServerRuning())
	{			
		buffer=FDB_ReadBlock(&size, FCT_ATTLOG);
		if(buffer)
		{
			if(UploadAttlogToAuthServer(buffer+4, size-4, &ret)&&ret) FDB_ClearData(FCT_ATTLOG);
			free(buffer);
		}
	}
}

BOOL SendPhotoToAuthServer(U32 pin, char *buffer, U16 width, U16 height, U8 bpp, U8 type)
{
        TPhotoSize header;
        BYTE result;
        int size;

        header.MachineNumber=gOptions.DeviceID;
        header.PIN=pin;
        header.Width=width;
        header.Height=height;
        header.BPP=bpp;
        header.Type=type;
        size=width*height*bpp;

        return (ExecuteCMD(AUTH_CMD_UPLOAD_PHOTO, (void *)&header, sizeof(header),
                          buffer, size, (void *)&result, sizeof(U8))&&result);

}


BOOL RefreshUserDataFromAuthServer(void)
{
	while(TRUE)
	{
#ifdef URU
		if(GetUserDataFromAuthServer(gImageBuffer, 2*URU_IMAGE_SIZE))
#else
		if(GetUserDataFromAuthServer(gImageBuffer, 5*gOptions.OImageWidth*gOptions.OImageHeight))
#endif
		{
			BatchOPUserData((BYTE *)gImageBuffer+1);
			if(*((BYTE *)gImageBuffer)==0) return TRUE;
		}
		else
			break;
	}
	return FALSE;
}

BOOL GetTimeFromAuthServer(U32 *result)
{
        BYTE DeviceID;

        DeviceID=gOptions.DeviceID;
        return ExecuteCMD(AUTH_CMD_SYNC_LOCALTIME, (void *)&DeviceID, sizeof(U8),
                          NULL, 0, (void *)result, sizeof(U32));
}
BOOL SyncLocalTimeFromAuthServer()
{
	time_t newtm;
	TTime ltm;
	if (GetTimeFromAuthServer(&newtm))	
	{
		OldDecodeTime(newtm,&ltm);
		SetTime(&ltm);
		return TRUE;
	}
	return FALSE;
}

