/******************************************************************************\
|*                                                                            *|
|*            ZKEM fingerprint recognition system firmware                    *|
|*                                                                            *|
|*  Copyright (C) 1998-2007 ZKSoftware Ltd.                                   *|
|*  Author: Chen Shukai                                                       *|
|*  Log   :                                                                   *|
|*		2006-10-24 Create (Chen Shukai)                                       *|
\******************************************************************************/
#include "serial.h"
#include "usbtypes.h"



//#define dprintf(...)
#define dprintf printf

// CDC Device
static char CDCDeviceDesc[]=
{    
    18,    					// Size of this descriptor in bytes
    UsbDescTypeDevice,                // DEVICE descriptor type
	0x10,		            	// USB 1.1 (LSB)
	1,			            // USB 1.1 (MSB)
    0x02,             		// Class Code
    0x00,                   // Subclass code
    0x00,                   // Protocol code
    16, //UDC_MAX_PACKET_SIZE_CTL,// Max packet size for EP0, see usbcfg.h
    0x55,                 	// Vendor ID: ZKSoftware
    0x1B,
    0x00,                 	// Product ID: CDC RS-232 Emulation Demo
    0xB4,
    0x00,                 	// Device release number in BCD format
    0x00,
    0x00,                   // Manufacturer string index
    0x00,                   // Product string index
    0x00,                   // Device serial number string index
    0x01                    // Number of possible configurations
};
#define SETCTLLEN(l) CDCDeviceDesc[7]=l

static char CDCConfigDesc[]={
	9, UsbDescTypeConfiguration, // Configuration descriptor
	9+9+5+4+5+5+7+9+7+7,	    // Total len (LSB)
	0,			            // Total len (MSB)
	2,			            // Number interfaces
	1,			            // Configuration value
	0,			            // Configuration string index
#ifdef MODULE
	0x80,			            // Attributes, BIT6 indicator if it is a self-power device
#else
	0xC0,			            // Attributes, BIT6 indicator if it is a self-power device
#endif
	150,			            // Max Power/2

	9, UsbDescTypeInterface, 0, 0, 1, 2, 2, 1, 0,	 

	// CDC Class-Specific Descriptors 
	5, 0x24, 0, 0x10, 1,
	4, 0x24, 2, 2,
	5, 0x24, 6, 0, 1,
	5, 0x24, 1, 0, 1,

	7,UsbDescTypeEndpoint,0x85, UsbInterrupt, 8, 0, 2,			            // Polling interval
	9,UsbDescTypeInterface, 1, 0, 2, 0x0A, 0, 0, 0,	/* Endpoint Descriptors */
	7,UsbDescTypeEndpoint,2,UsbBulk,64,0x00,0,
	7,UsbDescTypeEndpoint,0x81,UsbBulk,64,0x00,0
};
#define SETINTINLEN(l) CDCConfigDesc[9+9+5+4+5+5+4]=l
#define SETBULKOUTLEN(l) CDCConfigDesc[9+9+5+4+5+5+7+9+4]=l
#define SETBULKINLEN(l) CDCConfigDesc[9+9+5+4+5+5+7+9+7+4]=l
#define SETINTINEP(ep) CDCConfigDesc[9+9+5+4+5+5+2]=((ep)|0x80)
#define SETBULKOUTEP(ep) CDCConfigDesc[9+9+5+4+5+5+7+9+2]=ep
#define SETBULKINEP(ep)  CDCConfigDesc[9+9+5+4+5+5+7+9+7+2]=((ep)|0x80)
#define GETBULKINEP()   (CDCConfigDesc[9+9+5+4+5+5+7+9+7+2]&0x0F)

/* CDC specific */
#define SEND_ENCAPSULATED_COMMAND   0x00
#define GET_ENCAPSULATED_RESPONSE   0x01
#define SET_COMM_FEATURE            0x02
#define GET_COMM_FEATURE            0x03
#define CLEAR_COMM_FEATURE          0x04
#define SET_LINE_CODING             0x20
#define GET_LINE_CODING             0x21
#define SET_CONTROL_LINE_STATE      0x22
#define SEND_BREAK                  0x23
/* CDC Bulk IN transfer states */
#define CDC_TX_READY                0
#define CDC_TX_BUSY                 1
#define CDC_TX_BUSY_ZLP             2       // ZLP: Zero Length Packet
#define CDC_TX_COMPLETING           3

/* Line Coding Structure */
#define LINE_CODING_LENGTH          0x07

static BYTE line_coding[LINE_CODING_LENGTH]={
	115200 & 0xFF, (115200/256)& 0xFF, (115200/256/256)& 0xFF, (115200/256/256/256)& 0xFF, 
	0, 0, 8};    
//1 stop bit, None Parity, 8 Data Bits

/*
 * SEND_ENCAPSULATED_COMMAND and GET_ENCAPSULATED_RESPONSE are required
 * requests according to the CDC specification.
 * However, it is not really being used here, therefore a dummy buffer is
 * used for conformance.
 */
#define dummy_length    0x08
static BYTE dummy_encapsulated_cmd_response[dummy_length];

int USBInit_CDC(int ctlLen, int intInLen, int bulkInLen, int bulkOutLen, int intInEP, int bulkInEP, int bulkOutEP)
{
	SETCTLLEN(ctlLen);
	SETINTINLEN(intInLen);
	SETBULKINLEN(bulkInLen);
	SETBULKOUTLEN(bulkOutLen);
	SETINTINEP(intInEP);
	SETBULKINEP(bulkInEP);
	SETBULKOUTEP(bulkOutEP);
	return TRUE;
}

int USB_CDC_GetDescriptorID(int Value, char * *TxBuff0P, int *DataLenP)
{
	int tempDataLen;
    switch (Value)
    {
        case UsbDescTypeDevice:
            // Read the length of the device descriptor    
            // and compare with number of bytes in the data stage
            tempDataLen = sizeof(CDCDeviceDesc);
            if (*DataLenP > tempDataLen)
              *DataLenP = tempDataLen;
            // Initialize the buffer pointer
            *TxBuff0P = CDCDeviceDesc;
            break;
        case UsbDescTypeConfiguration:
            // Read the total length of the configuration descriptor    
            // and compare with number of bytes in the data stage
            tempDataLen = sizeof(CDCConfigDesc);
            if (*DataLenP > tempDataLen)
              *DataLenP = tempDataLen;
            
            // Initialize the buffer pointer
            *TxBuff0P = CDCConfigDesc;
			break;
	}
	return tempDataLen;
}

int USB_CDC_ClassReq(int Request, char **TxBuffOP, int *tempDataLen, int *OutState, int *Reset)
{
    switch(Request)
    {
        case SEND_ENCAPSULATED_COMMAND:
			*TxBuffOP = (void*)&dummy_encapsulated_cmd_response;
			*tempDataLen=dummy_length;
			dprintf("SEND_ENCAP_CMD  ");
            break;
        case GET_ENCAPSULATED_RESPONSE:
            break;
        case SET_COMM_FEATURE:                  // Optional
            break;
        case GET_COMM_FEATURE:                  // Optional
            break;
        case CLEAR_COMM_FEATURE:                // Optional
            break;
        case SET_LINE_CODING:
            *TxBuffOP = (void*)line_coding;
            *OutState = TRUE;
			dprintf("SET_LINE_CODING  ");
            break;
        case GET_LINE_CODING:
            *tempDataLen = sizeof(line_coding);
            *TxBuffOP = (void*)line_coding;
			dprintf("GET_LINE_CODING  ");
            break;
        case SET_CONTROL_LINE_STATE:
            *Reset = TRUE;          	
			dprintf("SET_CTL_LINE_ST  ");
            break;
        case SEND_BREAK:                        // Optional
            break;
        default:
			dprintf("unknown request %d.\n", Request);
            break;
    }
}

int SendUSBData(const char *buf, int size)
{
	return USBBulkInSend(GETBULKINEP(), buf, size);
}

int SendUSBRSData(void *param, const char *buf, int size)
{
	return SendUSBData(buf, size);
}

#define UDC_IN_SIZE 2048
static char udcInBuffer[UDC_IN_SIZE];
static int InInIndex = -1;
static int InOutIndex = -1;

int USB_Poll(void)
{
	int value,i;
	if(InInIndex==InOutIndex) return 0;
	if(InInIndex>InOutIndex) return InInIndex-InOutIndex;
	return UDC_IN_SIZE+InInIndex-InOutIndex;
}

int USB_GetChar(void)
{
	int value,i;
	if(InInIndex==InOutIndex) return -1;
	InOutIndex++;
	value=udcInBuffer[InOutIndex];
	if(InOutIndex>=UDC_IN_SIZE-1) InOutIndex=-1;
	return value;
}

int udcPutChar(char ch)
{
	udcInBuffer[++InInIndex]=ch;
	if(InInIndex>=UDC_IN_SIZE-1) InInIndex=-1;
	return 0;
}

int udcPutBuffer(char *buf, int size)
{
	printf("PutBuffer(In-Out): %d=%d\n", InInIndex, InOutIndex);
	while(size-->0) udcPutChar(*buf++);
	return size;
}

int USB_PutBuffer(char *buf, int size)
{
	SendUSBData(buf, size);
}

#include "commu.h"

int USB_CDC_Service(BYTE *p, int len)
{
	udcPutBuffer(p, len);
	return len;
/*	dprintf("CDC_Service: %d, %02X %02X %02X %02X %02X %02X %02X\n", len, p[0],p[1],p[2],p[3],p[4],p[5],p[6]);
	if(len>=16 && p[0]==p[1] && p[2]==0x81 && p[3]==0x7E && (unsigned char)(p[4])+(unsigned char)(p[5])*256+8==len)
	{//16: 01 01 81 7E 08 00 75 80 E8 03 17 FC 00 00 00 00 
		char Sender[SENDER_LEN];
		void *proc=(void*)&SendUSBRSData;
		dprintf("USB Data OK: %d\n", len);
		memset(Sender, 0, SENDER_LEN);
		SendUSBRSData(NULL, p, len);
		memcpy(Sender+4, &proc, sizeof(void*));
		//RS232Proc(p, Sender, 115200*100);
		return len;
	}
	else if(len<16 || len>=p[4]+p[5]*256+8 || p[4]+p[5]*256>1024+56)
	{
		dprintf("Invalid USB Data.\n");
		SendUSBRSData(NULL, p, len);
		return len;
	}
	return 0;
*/
}
