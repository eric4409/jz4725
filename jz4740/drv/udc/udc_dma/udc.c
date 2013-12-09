#include <ucos_ii.h>
#include <jz4740.h>
#include "usb.h"
#include "udc.h"
#include "udcbus.h"
#include "threadprio.h"
#include "midware.h"
//#include "libc.h"
//#include "cache.h"
//#include "intc.h"
 
#define EP1_INTR_BIT 2
#define EP_FIFO_NOEMPTY 2

#ifdef JZ4740_LYRA
#define GPIO_UDC_DETE_PIN		124 /* GPD6 */
#else
//#define GPIO_UDC_DETE_PIN (32 * 3 + 10)
//#define GPIO_UDC_DETE_PIN (32 * 2 + 23) //GPC23
#define GPIO_UDC_DETE_PIN (32 * 3 + 29) //GPD29
#endif

#define GPIO_UDC_DETE GPIO_UDC_DETE_PIN
#define IRQ_GPIO_UDC_DETE (IRQ_GPIO_0 + GPIO_UDC_DETE)
#define MAX_CABLE_DETE  3
#define CABLE_DETE_TIME  500                //wait time in ms
#define CABLE_CONNECTED          0
#define CABLE_DISCONNECT         1

#define DEBUGMSG 1
#if DEBUGMSG
#undef  dprintf
#define dprintf printf
#else
#undef  dprintf
#define dprintf(x...)
#endif


//add by cn 2009-03-22
#define UDC_GPIO
#define UDC_GPIO_LOW
extern int pretime;



#define IS_CACHE(x) (x < 0xa0000000)

#ifdef USE_MIDWARE
volatile static MIDSRC udcsrc;
volatile static u32 udcid;
#endif
volatile static MIDSRCDTA res;
volatile static u32 cable_stat;
volatile static u32 protocool_stat;

typedef struct
{
	unsigned char ep;
	unsigned char state;
	unsigned short fifosize;	
	unsigned int curlen;
	unsigned int totallen;
	unsigned int data_addr;
	unsigned int fifo_addr;
}EPSTATE,*PEPSTATE;

#define CPU_READ     0
#define CPU_WRITE     1

#define DMA_READ     2
#define DMA_WRITE     3


#define WRITE_FINISH  4
#define READ_FINISH  5

#define TXFIFOEP0 USB_FIFO_EP0

EPSTATE epstat[3] = {
	{0x00, 0,0,0,0,0,TXFIFOEP0 + 0},
	{0x81, 0,0,0,0,0,TXFIFOEP0 + 4},
	{0x01, 0,0,0,0,0,TXFIFOEP0 + 4}
};

#define GetEpState(x) (&epstat[x])

static OS_EVENT *udcEvent;
u8 USB_Version;
volatile static unsigned char udc_irq_type = 0;

//void (*tx_done)(void) = NULL;

#define UDC_TASK_PRIO	UDC_THREAD_PRIO  //7
#define UDC_TASK_STK_SIZE	1024 * 2
static OS_STK udcTaskStack[UDC_TASK_STK_SIZE];

#define PHYADDR(x) (x & 0x1fffffff)

static void DMA_ReceiveData(PEPSTATE pep)
{
	unsigned int size;
	size = pep->totallen - (pep->totallen % pep->fifosize);
	if(IS_CACHE(pep->data_addr))
		dma_cache_wback_inv(pep->data_addr, pep->totallen);
	//IntrOutMask = 0x0;
	
	
	jz_writel(USB_REG_ADDR2,PHYADDR(pep->data_addr));
	jz_writel(USB_REG_COUNT2,size);
	jz_writel(USB_REG_CNTL2,0x001d);

	jz_writeb(USB_REG_INDEX,1);	
	usb_setw(USB_REG_OUTCSR,0xa000);
//	printf("dma receive %x\n",size);    
	pep->curlen = size;
	
	
}
static void DMA_SendData(PEPSTATE pep)
{
	  unsigned int size;
	  
	  size = pep->totallen - (pep->totallen % pep->fifosize);
	  
	  if(IS_CACHE(pep->data_addr))
		  dma_cache_wback_inv(pep->data_addr, size);
	  jz_writel(USB_REG_ADDR1,PHYADDR(pep->data_addr));
	  jz_writel(USB_REG_COUNT1,size);//fifosize[ep]);
	  jz_writel(USB_REG_CNTL1,0x001f);
	  jz_writeb(USB_REG_INDEX, 1);
	  usb_setw(USB_REG_INCSR,0x9400);
	  
	  //printf("dma send %x\n",size);
	  pep->curlen = size;
}
static inline void DMA_SendData_Finish()
{
	jz_writel(USB_REG_CNTL1,0x0);
	jz_writeb(USB_REG_INDEX,1);
	usb_clearw(USB_REG_INCSR,0x9400);
	
}

static void DMA_ReceieveData_Finish()
{
//	dprintf("\n Disable DMA!\n");
	
	jz_writel(USB_REG_CNTL2,0x0);
	jz_writeb(USB_REG_INDEX,1);
	usb_clearw(USB_REG_OUTCSR,0xa000);
}



static void DisableDevice(unsigned int handle);
static void udc_reset(unsigned int handle)
{
//	PUDC_BUS pBus = (PUDC_BUS) handle;
	u8 byte;
	
	//data init
	
	//ep0state = USB_EP0_IDLE;
//	IntrOutMask = 0x2;

	// __cpm_stop_udc();
	/* Enable the USB PHY */
	//REG_CPM_SCR |= CPM_SCR_USBPHY_ENABLE;
	/* Disable interrupts */

	jz_writew(USB_REG_INTRINE,0);
	jz_writew(USB_REG_INTROUTE, 0);
	jz_writeb(USB_REG_INTRUSBE, 0);

	jz_writeb(USB_REG_FADDR,0);
//	jz_writeb(USB_REG_POWER,0x60);   //High speed

	jz_writeb(USB_REG_INDEX,0);
	jz_writeb(USB_REG_CSR0,0xc0);

	jz_writeb(USB_REG_INDEX,1);
	jz_writew(USB_REG_INCSR,0x2048);
	jz_writew(USB_REG_INMAXP,512);
	
	jz_writeb(USB_REG_INDEX,1);
	jz_writew(USB_REG_OUTCSR,0x0090);
	jz_writew(USB_REG_OUTMAXP,512);
	byte = jz_readb(USB_REG_POWER);
	dprintf("REG_POWER: %02x\r\n",byte);
	epstat[0].fifosize = 64;
	
	if(epstat[1].state != WRITE_FINISH)
		jz_readw(USB_REG_INTRIN);
	if(epstat[2].state != READ_FINISH)
		jz_readw(USB_REG_INTROUT);
	
	epstat[0].state = 0;
	epstat[1].state = WRITE_FINISH;
	epstat[2].state = READ_FINISH;
	epstat[0].curlen = 0;
	epstat[1].curlen = 0;
	epstat[2].curlen = 0;
	epstat[0].totallen = 0;
	epstat[1].totallen = 0;
	epstat[2].totallen = 0;
	
//	if ((byte & 0x10)==0) 
	if (1) 
	{
		jz_writeb(USB_REG_INDEX,1);
		jz_writew(USB_REG_INMAXP,64);
		jz_writew(USB_REG_INCSR,0x2048);
		jz_writeb(USB_REG_INDEX,1);
		jz_writew(USB_REG_OUTMAXP,64);
		jz_writew(USB_REG_OUTCSR,0x0090);
		USB_Version=USB_FS;

		//fifosize[1]=64;
      
		epstat[1].fifosize = 64;
		epstat[2].fifosize = 64;
		printf("usb1.1\n");
		

	}
	else
	{
		jz_writeb(USB_REG_INDEX,1);
		jz_writew(USB_REG_INMAXP,512);
		jz_writew(USB_REG_INCSR,0x2048);
		jz_writeb(USB_REG_INDEX,1);
		jz_writew(USB_REG_OUTMAXP,512);
		jz_writew(USB_REG_OUTCSR,0x0090);
		USB_Version=USB_HS;
		epstat[1].fifosize = 512;
		epstat[2].fifosize = 512;
		printf("usb2.0\n");
		
	}
//DMA USE
	jz_writel(USB_REG_CNTL1,0);
	
	jz_writel(USB_REG_ADDR1,0);
	jz_writel(USB_REG_COUNT1,0);//fifosize[ep]);

	jz_writel(USB_REG_CNTL2,0);
	jz_writel(USB_REG_ADDR2,0);
	jz_writel(USB_REG_COUNT2,0);//fifosize[ep]);

	jz_readb(USB_REG_INTR); // clear dma interrupt
	jz_writew(USB_REG_INTRINE,0x1);   //enable ep0 intr
	jz_writeb(USB_REG_INTRUSBE,0x6);
//	DisableDevice();	
}


static void udcReadFifo(PEPSTATE pep, int size)
{
	unsigned int *ptr = (unsigned int *)(pep->data_addr + pep->curlen);
	unsigned int fifo = pep->fifo_addr;
	unsigned char *c;
	int s = size / 4;
	unsigned int x;
	if(((unsigned int )ptr & 3) == 0)
	{
		while(s--)
			*ptr++ = REG32(fifo);
			
	}
	else
	{
		while(s--)
		{
			x = REG32(fifo);
			*ptr++ = (x >> 0)& 0x0ff;
			*ptr++ = (x >> 8)  & 0x0ff;
			*ptr++ = (x >> 16) & 0x0ff;
			*ptr++ = (x >> 24) & 0xff;
		}
	}
	
	s = size & 3;
	c = (unsigned char *)ptr;
	while(s--)
		*c++ = REG8(fifo);
	pep->curlen += size;
	
	
#if 0

	c = (unsigned char *)(pep->data_addr + pep->curlen - size);
	dprintf("recv:(%d)", size);
	for (s=0;s<size;s++) {
		if (s % 16 == 0)
			dprintf("\n");
		dprintf(" %02x", *(c+s));
	}
	dprintf("\n");
#endif
}

static void udcWriteFifo(PEPSTATE pep, int size)
{
	unsigned int *d = (unsigned int *)(pep->data_addr + pep->curlen);
	unsigned int fifo = pep->fifo_addr;
	u8 *c;
	int s, q;

#if 0
	unsigned char *ptr =(unsigned char *)d;
	
	dprintf("send:fifo(%x) = (%d)",fifo, size);
	for (s=0;s<size;s++) {
		if (s % 16 == 0)
			dprintf("\n");
		dprintf(" %02x", ptr[s]);
	}
	dprintf("\n");
#endif
	
	if (size > 0) {
		s = size >> 2;
		while (s--)
			REG32(fifo) = *d++;
		q = size & 3;
		if (q) {
			c = (u8 *)d;
			while (q--)
				REG8(fifo) = *c++;
		}
	} 
	pep->curlen += size;
}
void EP0_Handler (unsigned int handle)
{
    PEPSTATE pep = GetEpState(0);
	unsigned char byCSR0;

/* Read CSR0 */
	jz_writeb(USB_REG_INDEX, 0);
	byCSR0 = jz_readb(USB_REG_CSR0);
   
//	printf("EP0 CSR = %x\n",byCSR0);
    if(byCSR0 == 0)
		return;
	
/* Check for SentStall 
   if sendtall is set ,clear the sendstall bit*/
	if (byCSR0 & USB_CSR0_SENTSTALL) 
	{
		jz_writeb(USB_REG_CSR0, (byCSR0 & ~USB_CSR0_SENDSTALL));
		pep->state = CPU_READ;
		
		printf("\nSentstall!\n");
		return;
	}
/* Check for SetupEnd */
	if (byCSR0 & USB_CSR0_SETUPEND) 
	{
		jz_writeb(USB_REG_CSR0, (byCSR0 | USB_CSR0_SVDSETUPEND));
		pep->state = CPU_READ;
		printf("\nSetupend!\n");
		//return;
	}
/* Call relevant routines for endpoint 0 state */
	if (pep->state == CPU_READ) 
	{
		if (byCSR0 & USB_CSR0_OUTPKTRDY)   //There are datas in fifo
		{
            USB_DeviceRequest dreq;
			
            *(unsigned int *) &dreq =  REG32(pep->fifo_addr);
			*(unsigned int *) ((unsigned int)&dreq + 4)=  REG32(pep->fifo_addr);
		
			
		/*	dprintf("\nbmRequestType:%02x\nbRequest:%02x\n"
					"wValue:%04x\nwIndex:%04x\n"
					"wLength:%04x\n",
					dreq.bmRequestType,
					dreq.bRequest,
					dreq.wValue,
					dreq.wIndex,
					dreq.wLength);
		*/
			if ( dreq.bRequest == SET_CONFIGURATION )
				protocool_stat = CABLE_CONNECTED;

			if ( dreq.bRequest == SET_ADDRESS || dreq.bRequest == SET_CONFIGURATION || dreq.bRequest == CLEAR_FEATURE )
				usb_setb(USB_REG_CSR0, 0x40 | USB_CSR0_DATAEND);//clear OUTRD bit and DATAEND
			else
				usb_setb(USB_REG_CSR0, 0x40);//clear OUTRD bit

			BusNotify(handle,UDC_SETUP_PKG_FINISH,(unsigned char *)&dreq,8);
		}
		else 
		{
			dprintf("0:R DATA\n");
			
		}
		
	}
	
	if (pep->state == CPU_WRITE) 
	{
		int sendsize;
		sendsize = pep->totallen - pep->curlen;
//		printf("send size = %d\r\n",sendsize);
		
		if (sendsize < 64) 
		{
			udcWriteFifo(pep,sendsize);
			pep->curlen = pep->totallen;
			usb_setb(USB_REG_CSR0, USB_CSR0_INPKTRDY | USB_CSR0_DATAEND);
			pep->state = CPU_READ;
			
		} else 
		{
			if(sendsize)
			{
				udcWriteFifo(pep, 64);
				usb_setb(USB_REG_CSR0, USB_CSR0_INPKTRDY);
				pep->curlen += 64;
				
			}else
				pep->state = CPU_READ;
			
		}
	}
//	printf("pep state = %d %d\r\n",CPU_WRITE,pep->state);

	return;
}

void EPIN_Handler(unsigned int handle,PEPSTATE pep)
{
	unsigned int size;
   
	
	//fifo = fifoaddr[EP];
	size = pep->totallen - pep->curlen;
//	printf("EPIN: pep->totallen = %d pep->curlen=%d, size = %d \n",pep->totallen,pep->curlen,size);
	if(size == 0)
	{
		
		pep->state = WRITE_FINISH;
		usb_clearw(USB_REG_INTRINE,EP1_INTR_BIT);  // close ep1 in intr
		BusNotify(handle,UDC_PROTAL_SEND_FINISH,(unsigned char *)pep->data_addr,pep->curlen);
//		printf("Send finish \n"); 
		return;
		
	}
	
	if(size < pep->fifosize)
	{
		udcWriteFifo(pep,size);
	}else
		udcWriteFifo(pep,pep->fifosize);
	usb_setb(USB_REG_INCSR, USB_INCSR_INPKTRDY);
	
}

void EPOUT_Handler(unsigned int handle,PEPSTATE pep)
{
    unsigned int size;
	
	jz_writeb(USB_REG_INDEX, 1);

	size = jz_readw(USB_REG_OUTCOUNT);	
 	udcReadFifo(pep, size);
	usb_clearb(USB_REG_OUTCSR,USB_OUTCSR_OUTPKTRDY);
	pep->state = CPU_READ;
//	dprintf("EPOUT: totallen = %d curlen = %d,size=%d\n",pep->totallen,pep->curlen,size);
	
	if(pep->totallen == pep->curlen)
	{
		pep->state = READ_FINISH;
		usb_clearw(USB_REG_INTROUTE,EP1_INTR_BIT);
		BusNotify(handle,UDC_PROTAL_RECEIVE_FINISH,
			  (unsigned char *)pep->data_addr,pep->curlen);
		
		
	}
	
	//	USB_HandleUFICmd();
//	dprintf("\nEPOUT_handle return!\n");
}

//add by cn 2009-03-22
#ifdef  UDC_GPIO
static int check_gpio(u32 pin)
{
	int i ,j  = 0,k = 0;
	for ( i = 0 ; i < 1000 ; i ++ )
	{
		if (__gpio_get_pin(pin))
			j ++;
		else k ++;
	}
	if ( j >= k ) return 1;
	else return 0;
}
#endif

void udcIntrbhandle(unsigned int handle,unsigned char val)
{
	unsigned char byte;
	
	if (val & USB_INTR_RESET) 
	{
		printf("UDC reset intrupt!\r\n");  
		if ( protocool_stat == CABLE_DISCONNECT && 
		     cable_stat == CABLE_CONNECTED )
		{
			printf("USB cable insert! \n");
			cable_stat = CABLE_CONNECTED;
			#ifdef USE_MIDWARE
			udcsrc.Src = SRC_UDC;
			udcsrc.Event = EVENT_USB_IN;
			OSQPost(udcsrc.CurEvent1 , (void *)&udcid);
			OSSemPost(udcsrc.CurEvent);
			#endif
			if ( res.Val == 1 )  //|| res.Val == 0xffff) //up layer said yes!
			{
//				protocool_stat = CABLE_CONNECTED;
//				BusNotify((unsigned int)pBus,stat,0,0);
				;
			}
			else           //up layer said no!
			{
				protocool_stat = CABLE_DISCONNECT;
				jz_writeb(USB_REG_POWER,0x00);   //High speed	
				printf("As power cable insert! \n");
			}

		}

		udc_reset(handle);
		
		byte = jz_readb(USB_REG_POWER);
		//dprintf("REG_POWER: %02x\r\n",byte);

		if ((byte&0x10) ==0 ) 
		{
			BusNotify(handle,UDC_FULLSPEED,0,0);
		}else
			BusNotify(handle,UDC_HIGHSPEED,0,0);
		// enable USB Suspend interrupt
		
		byte = jz_readb(USB_REG_INTRUSBE);
		jz_writeb(USB_REG_INTRUSBE,byte | USB_INTR_SUSPEND);
      
		BusNotify(handle,UDC_RESET,0,0);
		
	}
	if(val & USB_INTR_SUSPEND)
	{
		BusNotify(handle,UDC_SUSPEND,0,0);
		
		byte = jz_readb(USB_REG_INTRUSBE);
		jz_writeb(USB_REG_INTRUSBE,(byte & (~USB_INTR_SUSPEND) & 7));
		printf("udc suspend %x\n",byte);
#ifdef USE_MIDWARE
		if ( protocool_stat == CABLE_CONNECTED )
		{
			printf("USB uninstall ! %d \n",protocool_stat);
			udcsrc.Src = SRC_UDC;
			udcsrc.Event = EVENT_UNINSTALL;
			protocool_stat = CABLE_DISCONNECT;
			OSQPost(udcsrc.CurEvent1 , (void *)&udcid);
			OSSemPost(udcsrc.CurEvent);
			OSSemPend(udcsrc.CurEvent2, 0 , &err);
			OSTimeDly(350);			
			if ( check_gpio(GPIO_UDC_DETE) )   //cable have not disconnect
			{
				printf("Reas power cable insert! \n");
				udcsrc.Src = SRC_UDC;
				udcsrc.Event = EVENT_POWER_IN;
				OSQPost(udcsrc.CurEvent1 , (void *)&udcid);
				OSSemPost(udcsrc.CurEvent);
				OSSemPend(udcsrc.CurEvent2, 0 , &err);
			}
		}
		else 
			jz_writeb(USB_REG_POWER,0x60);   //High speed	
#endif
	}
	if(val & 2)
	{
		printf("udc resume\n");
	}
	
	
}

void udc4740Proc (unsigned int handle)
{
	u8	IntrUSB = 0;
	u16	IntrIn = 0;
	u16	IntrOut = 0;
	u16 IntrDMA = 0;
	PEPSTATE pep;
	cli();
/* Read interrupt regiters */
	IntrUSB = jz_readb(USB_REG_INTRUSB);
/* Check for resume from suspend mode */
	if(IntrUSB != 8)
		udcIntrbhandle(handle,IntrUSB);
	/* Check for endpoint 0 interrupt */
	
	IntrIn  = jz_readw(USB_REG_INTRIN);
//	if(jz_readw(USB_REG_INTROUTE))
	IntrOut = jz_readw(USB_REG_INTROUT);
//	printf("IntrIn = %x IntrOut = %x\n",IntrIn,IntrOut);
	
	if (IntrIn & USB_INTR_EP0) 
	{
		//dprintf("\nUDC EP0 operations!\n");
		EP0_Handler(handle);
	}	

	pep = GetEpState(1);
	if(pep->state == CPU_WRITE)
	{
		if (IntrIn & 2) 
		{
	//		dprintf("\nUDC EP1 IN operation!\n");
			EPIN_Handler(handle,pep);
			//return;
		}
	}
	
	IntrDMA = jz_readb(USB_REG_INTR);
	//printf("IntrDMA = %x\n",IntrDMA);
	
	if(pep->state == DMA_WRITE)
	{
	//	dprintf("dma write intr = %x\n",IntrDMA);

				
		if(IntrDMA & 1)
		{
			
		////	dprintf("addr %x,count %x,cntl %x\r\n",
		//		   jz_readl(USB_REG_ADDR1),jz_readl(USB_REG_COUNT1),jz_readl(USB_REG_CNTL1));
			DMA_SendData_Finish();
		//	printf("DMA writing: curlen=%d, totallen=%d\n",pep->curlen, pep->totallen);		
			if(pep->curlen != pep->totallen)
			{
				/*??????????????*/
		////		printf("cur_len %d,totallen %d\r\n",pep->curlen,pep->totallen);
				usb_setw(USB_REG_INTRINE,EP1_INTR_BIT);  // open ep1 in intr
				pep->state = CPU_WRITE;
				EPIN_Handler(handle,pep);
				
			   
                 		//jz_writeb(USB_REG_INDEX, 1);
				//usb_setb(USB_REG_INCSR, USB_INCSR_INPKTRDY);
			}else 
			{
				pep->state = WRITE_FINISH;
				usb_clearw(USB_REG_INTRINE,EP1_INTR_BIT);  // close ep1 in intr
				BusNotify(handle,UDC_PROTAL_SEND_FINISH,
						   (unsigned char *)(pep->data_addr),pep->curlen);
				
				
			}
			//return;
		}
		

	}
	
	pep = GetEpState(2);
	if(pep->state == CPU_READ)
	{
		
		if ((IntrOut /*& IntrOutMask*/ ) & 2) 
		{
////			dprintf("UDC EP1 OUT operation!\n");
			EPOUT_Handler(handle,pep);
			//return;		
		}
	}
	if(pep->state == DMA_READ)
	{
		if(IntrDMA == 0)
			IntrDMA = jz_readb(USB_REG_INTR);

////		dprintf("\nDMA_REA intrDMA = %x\n",IntrDMA);
		
		if (IntrDMA & 0x2)     //channel 2 :OUT
		{
	//		dprintf("\n INTR 2!\n");
			DMA_ReceieveData_Finish();
 		//	printf("DMA reading: curlen=%d, totallen=%d, cur/total=%d\n",pep->curlen, pep->totallen,
		//							pep->totallen % pep->fifosize);		
			if((pep->totallen % pep->fifosize) != 0)
			{
				usb_setw(USB_REG_INTROUTE,EP1_INTR_BIT);	//Enable Ep Out
				pep->state = CPU_READ;
				EPOUT_Handler(handle,pep);
			}else
			{

				pep->state = READ_FINISH;
				BusNotify(handle,UDC_PROTAL_RECEIVE_FINISH,
						  (unsigned char *)(pep->data_addr),pep->curlen);
			}
			//return;
			

		}
	}
	sti();
	//dprintf("\n UDCProc finish!\n");
	
	//return;
}

//add by cn 2009-03-22
#ifdef  UDC_GPIO
void GPIO_Handle(unsigned int arg)
{
//	__gpio_ack_irq(GPIO_UDC_DETE);
	__gpio_mask_irq(GPIO_UDC_DETE);
	udc_irq_type |= 0x10;
	OSSemPost(udcEvent);
}
#endif

static void EnableDevice(unsigned int handle);

static void GPIO_IST(void * arg)
{
	PUDC_BUS pBus = (PUDC_BUS) arg;
	static unsigned int i;
	u8 byte1 , byte2;

	dprintf("\n GPIO IRQ!!\n");
	if ( cable_stat == CABLE_CONNECTED )    //will disconnect
	{

#ifdef UDC_GPIO //add by cn 2009-03-22
		OSTimeDly(30);
	#ifdef UDC_GPIO_LOW
		if ( !check_gpio(GPIO_UDC_DETE) )  //false disconnect
	#else
		if ( check_gpio(GPIO_UDC_DETE) )  //false disconnect
	#endif
		{
			return ;
		}
#endif		
		if ( protocool_stat == CABLE_CONNECTED )  //info suspend
		{
			printf("cable USE uninstall ! \n");
			#ifdef USE_MIDWARE
			udcsrc.Src = SRC_UDC;
			udcsrc.Event = EVENT_UNINSTALL;
			protocool_stat = CABLE_DISCONNECT;
			OSQPost(udcsrc.CurEvent1 , (void *)&udcid);
			OSSemPost(udcsrc.CurEvent);
			OSSemPend(udcsrc.CurEvent2, 0 ,&err);
			#endif
		}
		//Do disabledevice
		cable_stat = CABLE_DISCONNECT;
		#ifdef USE_MIDWARE
		udcsrc.Src = SRC_UDC;
		udcsrc.Event = EVENT_POWER_OUT;
		OSQPost(udcsrc.CurEvent1 , (void *)&udcid);
		OSSemPost(udcsrc.CurEvent);
		OSSemPend(udcsrc.CurEvent2, 0 ,&err);
		#endif
		BusNotify((unsigned int)pBus,UDC_REMOVE,0,0);
		DisableDevice(0);
//add by cn 2009-03-22
#ifdef UDC_GPIO
	#ifdef UDC_GPIO_LOW
		__gpio_as_irq_fall_edge(GPIO_UDC_DETE);
	#else
		__gpio_as_irq_rise_edge(GPIO_UDC_DETE);
	#endif
#endif
	}
	else                                    // will connect
	{
		//for test USB or POWER cable!!
		EnableDevice(0);

//add by cn 2009-03-22
#ifdef UDC_GPIO
		OSTimeDly(30);
	#ifdef UDC_GPIO_LOW
		if ( check_gpio(GPIO_UDC_DETE) )  //false connect
	#else
		if ( !check_gpio(GPIO_UDC_DETE) )  //false connect
	#endif
		{
	#ifdef UDC_GPIO_LOW
			__gpio_as_irq_fall_edge(GPIO_UDC_DETE);
	#else
			__gpio_as_irq_rise_edge(GPIO_UDC_DETE);
	#endif
			return ;
		}
		i=0;
		cable_stat = CABLE_CONNECTED;
#else

		for ( i = 0; i <= MAX_CABLE_DETE; i ++)
		{
			//sleep wait a while!
			OSTimeDly(CABLE_DETE_TIME/10);
 			byte1 = jz_readb(USB_REG_POWER);
			byte2 = jz_readb(USB_REG_INTRUSB);
			if ( ( byte1 & 0x08 )|| ( byte2 & 0x04 ) ) //reset occur!
				break;
			else 
				printf("Wait reset time out! \n");
		}
//		DisableDevice(0);
		cable_stat = CABLE_CONNECTED;
	//	__gpio_as_irq_fall_edge(GPIO_UDC_DETE);
#if 0
		if ( i > MAX_CABLE_DETE ) //power cable!
		{
			printf("Power cable insert! \n");
			cable_stat = CABLE_DISCONNECT;
			BusNotify((unsigned int)pBus,UDC_REMOVE,0,0);
	                DisableDevice(0);
		/*	CloseDevice();
			printf("Close Device\n");
			OSTaskDel(OS_PRIO_SELF);
			printf("Deleted UDC task\n");
			OSSemDel(udcEvent, OS_DEL_ALWAYS, &err); */
			#ifdef USE_MIDWARE
			udcsrc.Src = SRC_UDC;
			udcsrc.Event = EVENT_POWER_IN;
			OSQPost(udcsrc.CurEvent1 , (void *)&udcid);
			OSSemPost(udcsrc.CurEvent);
			OSSemPend(udcsrc.CurEvent2, 0 ,&err);
			#endif
			return ;
		}
		else                      //usb cable!!
#endif
#endif
		{
			#ifdef USE_MIDWARE
			udcsrc.Src = SRC_UDC;
			udcsrc.Event = EVENT_USB_IN;
			OSQPost(udcsrc.CurEvent1 , (void *)&udcid);
			OSSemPost(udcsrc.CurEvent);
			OSSemPend(udcsrc.CurEvent2 , 0, &err);
			#endif
			printf("USB cable insert! \n");
			printf("read val %d \n",res.Val);
			if ( res.Val == 1 )  //|| res.Val == 0xffff) //up layer said yes!
			{
//				protocool_stat = CABLE_CONNECTED;
				BusNotify((unsigned int)pBus,UDC_JUDGE,0,0);
//add by cn 2009-03-22
#ifdef UDC_GPIO
       		 	#ifdef UDC_GPIO_LOW
 		               __gpio_as_irq_rise_edge(GPIO_UDC_DETE);
        		#else
                		__gpio_as_irq_fall_edge(GPIO_UDC_DETE);
        		#endif
#endif
				pretime=-5;


			}
			else           //up layer said no!
			{
			    	DisableDevice(0);   
				protocool_stat = CABLE_DISCONNECT;
				printf("As power cable insert! \n");
			}
		}

	}
	
}

//add by cn 2009-03-22
#ifdef UDC_GPIO
void GPIO_IRQ_init(PUDC_BUS pBus)
{
	int err = 0;
#ifdef UDC_GPIO_LOW
	__gpio_as_irq_fall_edge(GPIO_UDC_DETE);
#else
	__gpio_as_irq_rise_edge(GPIO_UDC_DETE);
#endif
	__gpio_as_irq_rise_edge(GPIO_UDC_DETE);
	request_irq(IRQ_GPIO_UDC_DETE, GPIO_Handle, pBus);
	__gpio_disable_pull(GPIO_UDC_DETE);
	REG_CPM_SCR &= ~CPM_SCR_USBPHY_ENABLE;  //disable UDC_PHY
	jz_writeb(USB_REG_POWER,0x60);   //High speed	
}
#endif
static void udcIntrHandler(unsigned int arg)
{
	__intc_mask_irq(IRQ_UDC);
	udc_irq_type |= 0x1;
	//dprintf("UDC irq\r\n");
	OSSemPost(udcEvent);
}

//add by cn 2009-03-22
//unsigned char UDCDataReceived=0;
//extern OS_EVENT *MainEvent;
static void udcTaskEntry(void *arg)
{
	u8 err;
	
	GPIO_IST(arg);
	while (1) {
		OSSemPend(udcEvent, 0, &err);
	//	dprintf("udc_irq_type = %x\r\n",udc_irq_type);
		wdt_set_count(0);
		if(udc_irq_type & 0x10)
		{
			GPIO_IST(arg);
			udc_irq_type &= ~0x10;
		//	__gpio_unmask_irq(GPIO_UDC_DETE);
		}
		if(udc_irq_type & 1)
		{	
			udc4740Proc((unsigned int)arg);
		/*	if(UDCDataReceived ==1)
			{
                        	OSSemPend(MainEvent, 0, &err);
			}
		*/
			udc_irq_type &= ~0x1;
			__intc_unmask_irq(IRQ_UDC);
			
		}
	}
}


/*   interface   */
static void SetAddress(unsigned int handle,unsigned short value)
{
	dprintf("Set address %d\r\n",value);
#if 1
	protocool_stat = CABLE_CONNECTED;
#endif
	jz_writeb(USB_REG_FADDR,value);
}

static void EnableDevice(unsigned int handle)
{
	__cpm_start_udc();
	REG_CPM_SCR |= CPM_SCR_USBPHY_ENABLE;
	jz_writeb(USB_REG_POWER,0x60);     //enable sofeconnect
	__intc_unmask_irq(IRQ_UDC);
	printf("Enable USB Phy!\r\n");
	
}
static void DisableDevice(unsigned int handle)
{
	u8 err;
  	REG_CPM_SCR &= ~CPM_SCR_USBPHY_ENABLE;
//	jz_writeb(USB_REG_POWER,0x00);      //disable sofeconnet!
	jz_readb(USB_REG_INTRUSB);
	jz_readw(USB_REG_INTRIN);
	jz_readw(USB_REG_INTROUT);
	udc_irq_type = 0;
	OSSemSet(udcEvent,0,&err);
	__intc_ack_irq(IRQ_UDC);
	__cpm_stop_udc();
	__intc_mask_irq(IRQ_UDC);

//add by cn 2009-03-22
#ifdef UDC_GPIO
	#ifdef UDC_GPIO_LOW
        __gpio_as_irq_fall_edge(GPIO_UDC_DETE);
	#else
        __gpio_as_irq_rise_edge(GPIO_UDC_DETE);
	#endif
#endif

	printf("Disable USB Phy!\r\n");
}

void StartTransfer(unsigned int handle,unsigned char ep,unsigned char *buf,unsigned int len)
{
	PEPSTATE pep;
////	dprintf("StartTransfer ep = %x buf:%p len = %d\r\n\n\n",ep,buf,len);
	unsigned char state;
	switch(ep)
	{
	case 0:
		pep = GetEpState(0);
		pep->totallen = len;
		pep->curlen = 0;
		pep->data_addr = (unsigned int)buf;		
		pep->state = CPU_WRITE;
		
		break;
	case 0x81:
		pep = GetEpState(1);
		pep->totallen = len;
		pep->curlen = 0;
		pep->data_addr = (unsigned int)buf;
		usb_setw(USB_REG_INTRINE,EP1_INTR_BIT); //open ep1 in intr

		if(len < pep->fifosize)
		{
			pep->state = CPU_WRITE;
			
			jz_writeb(USB_REG_INDEX,1);
			state =  jz_readw(USB_REG_INCSR);
			if(!(state & EP_FIFO_NOEMPTY))
			{
				len = len > pep->fifosize ? pep->fifosize :len;
				udcWriteFifo(pep,len);
				usb_setw(USB_REG_INCSR, USB_INCSR_INPKTRDY);		
				
			}
					
				
		}
		else
		{
			jz_writeb(USB_REG_INDEX,1);
			state =  jz_readw(USB_REG_INCSR);
			pep->state = DMA_WRITE;
			if(!(state & EP_FIFO_NOEMPTY))
				DMA_SendData(pep);	
		}
		break;
		
	case 0x1:
		pep = GetEpState(2);
		pep->totallen = len;
		pep->curlen = 0;
		pep->data_addr = (unsigned int)buf;
		if(len < pep->fifosize)
		{
			
			pep->state = CPU_READ;
			usb_setw(USB_REG_INTROUTE,EP1_INTR_BIT);	//Enable Ep Out
			
		}
		else
		{
			pep->state = DMA_READ;
			DMA_ReceiveData(pep);
			
		}

		break;
	case 0xff:             //mean send stall!
//		printf("Send Stall! %x \n",jz_readw(USB_REG_INCSR));
		usb_setw( USB_REG_INCSR, 0x10);	//set stall
		while( ! (jz_readw(USB_REG_INCSR) & 0x20 ) );                //wait stall sent!
		usb_setw( USB_REG_INCSR, 0x60);	             //clear datatag!
		usb_clearw( USB_REG_INCSR, 0x10);	             //clear sendstall
		usb_clearw( USB_REG_INCSR, 0x20);	             //clear sentstall
//		printf("Clear stall! %x \n",jz_readw(USB_REG_INCSR));

		break;
	}
}
void InitEndpointSuppost(unsigned int handle,unsigned char *ep,USB_ENDPOINT_TYPE ep_type,unsigned short *ep_max_pkg)
{
	PEPSTATE pep;

	if(ep_type == ENDPOINT_TYPE_CONTROL)
	{
		*ep = 0;
		*ep_max_pkg = MAX_EP0_SIZE;
		
	}
	if(ep_type == ENDPOINT_TYPE_BULK)
	{
		if(*ep & 0x80)
			pep = GetEpState(1);
		else
			pep = GetEpState(2);
		
		*ep = pep->ep;
		*ep_max_pkg = pep->fifosize;
	}
//	printf("ep = %x ep_type = %x epmax = %x\r\n",*ep,ep_type,pep->fifosize);	
}

#ifdef USE_MIDWARE
static void GetRequest(MIDSRCDTA *dat)
{
	dat->Val = res.Val;
//	printf("Up layer get :%d \n",res.Val);
}

static void Response(MIDSRCDTA *dat)
{

	res.Val = dat->Val;
	printf("Up layer said :%d \n",res.Val);

}
#endif

void InitUDC(PUDC_BUS pBus)
{
	pBus->EnableDevice = EnableDevice;
	pBus->SetAddress = SetAddress;
	pBus->StartTransfer = StartTransfer;
	pBus->InitEndpointSuppost = InitEndpointSuppost;
	pBus->DisableDevice = DisableDevice;
	printf("Init UDC %s %s\n",__DATE__,__TIME__);

#ifdef USE_MIDWARE
	udcsrc.GetRequest = GetRequest;
	udcsrc.Response = Response;
	udcsrc.Name = "UDC";
	printf("Register Midware SRC udc! \n");
	RegisterMidSrc((PMIDSRC)&udcsrc);
	udcid = udcsrc.ID;
//	res.Val = 0xffff;
	cable_stat = CABLE_DISCONNECT;
	protocool_stat = CABLE_DISCONNECT;
#endif	
	dprintf("Init UDC\n");
	res.Val = 1;
	cable_stat = CABLE_DISCONNECT;
	protocool_stat = CABLE_DISCONNECT;
	USB_Version=USB_HS;
	__intc_mask_irq(IRQ_UDC);

//add by cn 2009-03-22
#ifdef UDC_GPIO
	__gpio_mask_irq(GPIO_UDC_DETE);
#endif
	
	udcEvent = OSSemCreate(0);
	dprintf("UDC with DMA reset!!\r\n");
	request_irq(IRQ_UDC, udcIntrHandler, 0);

//add by cn 2009-03-22
#ifdef UDC_GPIO
	GPIO_IRQ_init(pBus);
#endif
	udc_reset((unsigned int)pBus);
	dprintf("UDC with DMA reset finish!!\r\n");
	dprintf("Create UDC Task!!\r\n");
	OSTaskCreate(udcTaskEntry, (void *)pBus,
		     (void *)&udcTaskStack[UDC_TASK_STK_SIZE - 1],
		     UDC_TASK_PRIO);
	
//	__gpio_unmask_irq(GPIO_UDC_DETE);
//	__intc_unmask_irq(IRQ_UDC);
#if 0
	if ( __gpio_get_pin(GPIO_UDC_DETE) == 1 )
	{
		__gpio_mask_irq(GPIO_UDC_DETE);
		udc_irq_type |= 0x10;
		OSSemPost(udcEvent);
	}
#endif
}

int UDC_DetectStatus(void)
{
	if ( protocool_stat == CABLE_CONNECTED )
		return 0;
	else return 1;
}

