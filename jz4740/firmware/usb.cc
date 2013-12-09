#include "commu.h"
#include "exfun.h"

int SendUSBData(void *buf, int size, void *param)
{
#ifdef UDC
        return sendtousb(buf, size);
#endif
}

int USBCommCheck(void)
{
       unsigned  char buf[1032];
        int cmd,len;
        //Maximize data packet size is 1032
	#ifdef UDC
        len = recvfromusb(buf, 1032);
#if 0
	if(len>0)
	{
		unsigned char tmp;
		int timeout = 5000000;
		printf(" rrrrrr>>>>> received data from usb len: %d  data: %x  %x  %x\n",len,buf[0],buf[1],buf[2]);
		if(buf[0]==0x0A && buf[1]==0x0A)
		{
			tmp = buf[2];
			memset(buf, 0, 1032);
			buf[0] = 0xA0;
			buf[1] = 0xA0;
			buf[2] = 0xFF-tmp;
			while(!sendtousb(buf,1032)&&timeout--);
			if(timeout)
				printf("wwwwwww>>>>>> finished one times communication r:%x s:%x\n",tmp, 0xFF-tmp);
			else
				printf("communication timeout\n");
		}
		return;
	}
	else
	{
		return 0;
	}
	#endif
#endif
        if(len>0){
	printf(" rrrrrr>>>>> received data from usb,data len: %d. data: %2X,%2X,%2X,%2X,%2X,%2X,%2X,%2X,%2X,%2X\n",len,buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9]);
                //▒▒▒▒ݰ▒▒▒RunCommand▒▒▒▒
                //DBPRINTF("RunCommand be called to process data packet\n");
                if ((cmd = RunCommand(buf+8, len, TRUE)) == 0) 
		{
			printf("RunCommand return error\n");
			return -2;
		}
                //▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ӻỰ▒▒▒▒▒▒▒▒ͨѶ▒▒▒▒
                //the client ask to make a connection session
                if (cmd == CMD_CONNECT){
			printf("COMMAND: CMC_CONNECT\n");
                        extern int RTLogNum;
                        char Sender[SENDER_LEN];
                        PCmdHeader chdr=(PCmdHeader)buf;

                        //▒▒▒ͨѶ▒▒▒▒▒ͨѶ▒Ự
                        //Ҳ▒▒▒▒SendUDPData▒У▒▒▒▒Խ▒˲▒▒▒▒öԷ▒▒▒ip▒▒▒Ͷ˿ںţ▒▒▒▒▒▒▒▒▒▒
                        memset(Sender, 0, SENDER_LEN);
                 //       memcpy(Sender, (void*)&from, sizeof(struct sockaddr));
                        PCommSession session=CreateSession(Sender);

                        RTLogNum=0;
                        //▒▒Է▒▒▒▒▒▒▒ӳɹ▒▒ĻỰ▒▒ʶID
                        chdr->SessionID=session->SessionID;
                        chdr->CheckSum=0;
                        chdr->CheckSum=in_chksum(buf, sizeof(TCmdHeader));
                        session->Send=SendUSBData;
                        session->Speed=10*1024*1024;
                        //ע▒▒session->Sender▒▒Ϊ▒▒▒▒▒ͨѶ▒▒▒▒▒▒▒Ժ▒▒ͨѶ▒▒▒▒▒▒▒Է▒▒▒▒▒▒▒▒ʱʹ▒ã▒
                        SendUSBData(chdr, sizeof(TCmdHeader), session->Sender);
                }
		printf("Finished RunCommand\n");
        }
        return 0;
}

