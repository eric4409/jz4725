/******************************************************************************\
|*                                                                            *|
|*            ZKEM fingerprint recognition system firmware                    *|
|*                                                                            *|
|*  Copyright (C) 1998-2007 ZKSoftware Ltd.                                   *|
|*  Author: Chen Shukai                                                       *|
|*  Log   :                                                                   *|
|*		2006-10-24 Create (Chen Shukai)                                       *|
\******************************************************************************/
#ifndef _USB_TYPES_
#define _USB_TYPES_

// Enumerate Request types
typedef enum RequestTypeE
{
    StandardReq = 0x00,
    ClassReq = 0x01,
    VendorReq = 0x02
} RequestTypeT;

// Enumerate Standard Request types	value
typedef enum UsbStandardReqTypeE
{
    GetStatusId = 0x00,
    ClearFeatureId = 0x01,
    SetFeatureId = 0x03,
    SetAddressId = 0x05,
    GetDescriptorId = 0x06,
    SetDescriptorId = 0x07,
    GetConfigurationId = 0x08,
    SetConfigurationId =0x09,
    GetInterfaceId = 0x0a,
    SetInterfaceId = 0x0b,
    SynchFrameId = 0x0c
} UsbStandardReqTypeT;

// Descriptor types
typedef enum UDCDescriptorTypeE
{
    UsbDescTypeDevice = 0x01,
    UsbDescTypeConfiguration = 0x02,
    UsbDescTypeString = 0x03,
    UsbDescTypeInterface =0x04,
    UsbDescTypeEndpoint = 0x05
}   UDCDescriptorTypeT;

// Usb transfer types
typedef enum UDCTransferTypeE {
    UsbControl = 0,
    UsbIsochronous = 1,
    UsbBulk = 2,
    UsbInterrupt = 3
}   UDCTransferTypeT;

// Vendor Requests
typedef enum UdcVendorReqTypeE {
    UsbSetupInEp = 0x01,
    UsbSetupOutEp = 0x02,
    UsbSetupIntEp = 0x03,
    UsbSetupLoopback = 0x04
} UdcVendorReqTypeT;

typedef struct UdcSetupDataS
{
    unsigned char bmRequestType;
    unsigned char bRequest;
    unsigned short wValue;
    unsigned short wIndex;
    unsigned short wLength;
} UdcSetupDataT;

#endif

