#include <includes.h>
#include "commu.h"
#include "exfun.h"

int SendUSBData(void *buf, int size, void *param)
{
#ifdef UDC
	int timeout = 50;
	int rel;

	//add by cn 2009-03-22
	ENABLE_UDC_IRQ(1);
	while(timeout--)
	{
        	rel = sendtousb(buf, size);
		if(rel>0)
		{
			ENABLE_UDC_IRQ(0);
			return rel;
		}
		else
			OSTimeDly(1);
	}
	ENABLE_UDC_IRQ(0);
	return rel;
#endif
}

//add by cn 2009-03-22
//extern OS_EVENT *MainEvent;
int USBCommCheck(void)
{
#ifdef UDC
	unsigned  char buf[1032];
        int cmd,len;
        //Maximize data packet size is 1032

	memset(buf, 0, 1032);
        len = recvfromusb(buf, 1032);
        if(len>0){
	//	printf(" rrrrrr>>>>> received data from usb,data len: %d. data: %2X,%2X,%2X,%2X,%2X,%2X,%2X,%2X,%2X,%2X\n",len,buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9]);
		ENABLE_UDC_IRQ(0);
                //▒▒▒▒ݰ▒▒▒RunCommand▒▒▒▒
                //DBPRINTF("RunCommand be called to process data packet\n");
                if ((cmd = RunCommand(buf, len, TRUE)) == 0) 
		{
			printf("RunCommand return error\n");
			//add by cn 2009-03-22
//                	OSSemPost(MainEvent);
			ENABLE_UDC_IRQ(1);
			return -2;
		}
                //▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ӻỰ▒▒▒▒▒▒▒▒ͨѶ▒▒▒▒
                //the client ask to make a connection session
                if (cmd == CMD_CONNECT){
                        extern int RTLogNum;
                        char Sender[SENDER_LEN];
                        PCmdHeader chdr=(PCmdHeader)buf;

                        //▒▒▒ͨѶ▒▒▒▒▒ͨѶ▒Ự
                        //Ҳ▒▒▒▒SendUDPData▒У▒▒▒▒Խ▒˲▒▒▒▒öԷ▒▒▒ip▒▒▒Ͷ˿ںţ▒▒▒▒▒▒▒▒▒▒
                        memset(Sender, 0, SENDER_LEN);
                 //     memcpy(Sender, (void*)&from, sizeof(struct sockaddr));
			strcpy(Sender,"SendUSBData");
                        PCommSession session=CreateSession(Sender);
                        RTLogNum=0;
                        //▒▒Է▒▒▒▒▒▒▒ӳɹ▒▒ĻỰ▒▒ʶID
                        chdr->SessionID=session->SessionID;
                        chdr->CheckSum=0;
                        chdr->CheckSum=in_chksum(buf, sizeof(TCmdHeader));
                        session->Send=SendUSBData;
                        session->Speed=1024;

			 //add by cn 2009-03-18
		//	session->TimeOutSec=15;
			EnableDevice(0);

		//	session->TimeOutSec = 10;
                        //ע▒▒session->Sender▒▒Ϊ▒▒▒▒▒ͨѶ▒▒▒▒▒▒▒Ժ▒▒ͨѶ▒▒▒▒▒▒▒Է▒▒▒▒▒▒▒▒ʱʹ▒ã▒
                        SendUSBData(chdr, sizeof(TCmdHeader), session->Sender);
                }
	//	printf("Finished Command %d\n",cmd);
		 //add by cn 2009-03-22
//                 OSSemPost(MainEvent);
		ENABLE_UDC_IRQ(1);
        }
#endif
        return 0;
}

