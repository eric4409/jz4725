#include "usb.h"

//#define dprintf(x...) 
#define dprintf printf

extern u8 ep0state;

static const USB_DeviceRequest UDReq[] = {		//for what???
	{}, // get_status
	{}, // clear_feature
	{}, // RFU
	{}, // set_feature
	{}, // RFU
	{}, // set_address
	{}, // get_descriptor
	{}, // set_descriptor
	{}, // get_configuration
	{0, SET_CONFIGURATION, 1, 0, 8},
	{}, // get_interface
	{0, SET_INTERFACE, 1, 0, 8},
	{}, // SOF
};

void usbEncodeDevReq(u8 *buf, int index)		//for what??
{
	memcpy(buf, &UDReq[index], sizeof(USB_DeviceRequest));
}

#define NULL (void*)0
extern u8 *waitBuffer;
extern int waitSize;

void usbHandleClassDevReq(u8 *buf)
{
	USB_DeviceRequest *ReqData = (USB_DeviceRequest *)buf;
	int Reset=0, OutState=0;
	int DataLen = ReqData->wLength;
	int tempDataLen=0;
	char *TxBuffEp0P=NULL;
	USB_CDC_ClassReq(ReqData->bRequest, &TxBuffEp0P, &tempDataLen,
			&OutState, &Reset);
	printf("CDReq: len=%d, size=%d\n", tempDataLen, DataLen);
	if(Reset)
	{
		printf("RST. ");
		waitSize=0;
		waitBuffer=NULL;
//		HW_SendZeroPKT(0);
	}
	else if(OutState)
	{
		if(TxBuffEp0P)
		{
			waitBuffer=(u8*)TxBuffEp0P;
			waitSize=DataLen;
		}
	}

	else if(TxBuffEp0P)
	{
		if(tempDataLen==0)
			HW_SendZeroPKT(0);
		else if(tempDataLen>0)
		{
			if (DataLen > tempDataLen)  DataLen = tempDataLen;
			HW_SendPKT(0, TxBuffEp0P, DataLen);
		}
	}
}

void usbHandleDevReq(u8 *buf)
{
	dprintf("dev req:%d\n", (buf[0] & (3 << 5)) >> 5);
	switch ((buf[0] & (3 << 5)) >> 5) {
	case 0: /* Standard request */
		usbHandleStandDevReq(buf);
		break;
	case 1: /* Class request */
		usbHandleClassDevReq(buf);
		ep0state=2;
		break;
	case 2: /* Vendor request */
		break;
	}
}

