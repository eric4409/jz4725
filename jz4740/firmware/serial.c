/*************************************************
                                           
 ZEM 200                                          
                                                    
 serial.c 
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>
#include "arca.h"
#include "serial.h"
#include "utils.h"
#include "options.h"
#include "uart.h"

// Initialise serial port at the request baudrate. 
static int arca_serial_init(int BaudRate, int DataSize, int Parity, int FlowControl)
{
	serial_init(BaudRate, DataSize, Parity, FlowControl);   //Only baudrate is valid
    	return 0;
}

/* Flush serial input queue. 
 * Returns 0 on success or negative error number otherwise
 */
static int arca_serial_flush_input(void)
{
    return 0;
}

/* Flush output queue. 
 * Returns 0 on success or negative error number otherwise
 */
static int arca_serial_flush_output(void)
{
    return 0;
}

/* Check if there is a character available to read. 
 * Returns 1 if there is a character available, 0 if not, 
 */
static int arca_serial_poll(void)
{
	return serial_tstc();
}

/* read one character from the serial port. return character (between
 * 0 and 255) on success, or negative error number on failure. this
 * function is not blocking */
static int arca_serial_read(void)
{
	return serial_getc()&0xff;
}

/* write character to serial port. return 0 on success, or negative
 * error number on failure. this function is blocking
 */
static int arca_serial_write(int c)
{
	serial_put(c&0xff);
    	return 0;
}

static int arca_serial_tcdrain(void)
{
	return 0;
}

static void arca_serial_free(void)
{
	return;
}
	
static int arca_serial_write_buf(unsigned char* Data,int Size)
{
	unsigned int i;
	
	for(i=0;i<Size;i++)
		serial_put(*Data++);
	
	return 0;
}

static int arca_serial_read_buf(unsigned char* Data,int Size)
{
	unsigned int i;
	
	for(i=0;i<Size;i++)
		*Data++=serial_getc()&0xff;

	return i;
}
/* export serial driver */
serial_driver_t ff232 = {
	arca_serial_init,
	arca_serial_read,
	arca_serial_write,
	arca_serial_poll,
	arca_serial_flush_input,
	arca_serial_flush_output,
	arca_serial_tcdrain,
	arca_serial_free,
	arca_serial_write_buf,
    	arca_serial_read_buf
};

#ifdef UDC_UART
int USB_PutBuffer(char *buf, int size);
int USB_GetChar(void);
int USB_Poll(void);
static int usb_serial_init(int BaudRate, int DataSize, int Parity, int FlowControl)
{
	return 0;
}

static int usb_serial_read(void)
{
	int ret=USB_GetChar();
	return ret;
}

#define WRITE_BUF_SIZE 1040
static char usb_write_buff[WRITE_BUF_SIZE];
static int usb_write_index=0;

static usb_serial_flush_output(void);

static int usb_serial_write(int c)
{
	usb_write_buff[usb_write_index++]=c;
	if(usb_write_index>=WRITE_BUF_SIZE)
		usb_serial_flush_output();
   	return 0;
}

static int usb_serial_write_buf(unsigned char* Data,int Size)
{
	USB_PutBuffer(Data, Size);
	return 0;
}

static usb_serial_flush_output(void)
{
	if(usb_write_index>0)
	{
		usb_serial_write_buf(usb_write_buff, usb_write_index);
	}
	usb_write_index=0;
}

static int usb_serial_poll()
{
	return USB_Poll();
}

static int usb_serial_read_buf(unsigned char* Data,int Size)
{
	unsigned int i;
	for(i=0;i<Size;i++)
	{
		int ch=USB_GetChar();
		if(ch==-1) break;
		*Data++=ch;
	}
	return i;
}

/* export serial driver */
serial_driver_t usb232 = {
	usb_serial_init,
	usb_serial_read,
	usb_serial_write,
	usb_serial_poll,
	arca_serial_flush_input,
	usb_serial_flush_output,
	arca_serial_tcdrain,
	arca_serial_free,
	usb_serial_write_buf,
    	usb_serial_read_buf
};
#endif

void RS485_setmode(U32 SendMode)
{
	GPIO_RS485_Status(SendMode);
}

/*
 * Write a null terminated string to the serial port.
 */
void SerialOutputString(serial_driver_t *serial, const char *s)
{
        while(*s != 0)
                serial->write(*s++);
} /* SerialOutputString */

void SerialOutputData(serial_driver_t *serial, char *s,int  _size_)
{
        char *p=s;
        int size=_size_;
        while(size--) (serial)->write(*p++);
}

