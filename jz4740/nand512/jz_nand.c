/********************** BEGIN LICENSE BLOCK ************************************
 *
 * JZ4740  mobile_tv  Project  V1.0.0
 * INGENIC CONFIDENTIAL--NOT FOR DISTRIBUTION IN SOURCE CODE FORM
 * Copyright (c) Ingenic Semiconductor Co. Ltd 2005. All rights reserved.
 * 
 * This file, and the files included with this file, is distributed and made 
 * available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER 
 * EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS ALL SUCH WARRANTIES, 
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS 
 * FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. 
 * 
 * http://www.ingenic.cn 
 *
 ********************** END LICENSE BLOCK **************************************
 *
 *  Author:  <xyzhang@ingenic.cn> 
 *
 *  Create:   2008-08-27, by xyzhang
 *                        
 *******************************************************************************
 */

#include <jz4740.h>

#include "structure.h"
#include <jz_nand.h>
#include <nand_cfg.h>

#include "ucos_ii.h"


//***********************************************************************************
//**			FILE HEADER
//***********************************************************************************

//#define JZ_DEBUG

#define	TRUE	1

unsigned char HARDECC = 0;

//-------------	Page info----------------------------------

unsigned int g_dwPageSize = 512;
unsigned int g_dwBlockSize = 16 * 1024;
unsigned int g_dwOobSize = 16;
#define	NAND_PAGE_SIZE		g_dwPageSize
#define	NAND_BLOCK_SIZE		g_dwBlockSize
#define	NAND_OOB_SIZE		g_dwOobSize

#define JZSOC_ECC_STEPS		(NAND_PAGE_SIZE / JZSOC_ECC_BLOCK)
#define	CONFIG_SSFDC_NAND_PAGE_PER_BLOCK	NAND_BLOCK_SIZE / NAND_PAGE_SIZE
#define JZ_NAND_SELECT     (REG_EMC_NFCSR |=  EMC_NFCSR_NFCE1)
#define JZ_NAND_DESELECT   (REG_EMC_NFCSR &= ~(EMC_NFCSR_NFCE1))

//--------------------------------------------------------

void rs_correct(unsigned char *buf, int idx, int mask);

#define NAND_INTERRUPT 0
#define RS_INITERRUPT 1
#define MULTI_TASK_SUPPORT 1

#if NAND_INTERRUPT
	static OS_EVENT * rb_sem = NULL;
	static OS_EVENT * dma_sem = NULL;
	#define RB_GPIO_GROUP 2
	#define RB_GPIO_BIT   30
	#define RB_GPIO_PIN (32 * RB_GPIO_GROUP + RB_GPIO_BIT)  
	#define NAND_CLEAR_RB() (REG_GPIO_PXFLGC(RB_GPIO_GROUP) = (1 << RB_GPIO_BIT))
	#define JZ_NAND_ERASE_TIME (30 * 1000)
	#define nand_wait_ready()                     \
	do{                                           \
	    unsigned int timeout = (1000 * 1000 / OS_TICKS_PER_SEC);             \
		while((!(REG_GPIO_PXFLG((RB_GPIO_GROUP) & (1 << RB_GPIO_BIT))))& timeout--);\
	    REG_GPIO_PXFLGC(RB_GPIO_GROUP) = (1 << RB_GPIO_BIT); \
	}while(0)
	
	#define READDATA_TIMEOUT (100 * 1000 / OS_TICKS_PER_SEC)
	#define WRITEDATA_TIMEOUT (800 * 1000 / OS_TICKS_PER_SEC)
	#define PAGE_TIMEOUT (100 * 1000 / OS_TICKS_PER_SEC)
	
	#define CLEAR_RB()						\
	do {									\
		NAND_CLEAR_RB();					\
		__gpio_unmask_irq(RB_GPIO_PIN);		\
	} while (0)
	
	#define WAIT_RB()						\
	do {									\
		unsigned char err;					\
		OSSemPend(rb_sem,PAGE_TIMEOUT,&err);\
	} while (0)

#else
	#define CLEAR_RB()
	static inline void nand_wait_ready(void)
	{
	  unsigned int timeout = 100;
	  while ((REG_GPIO_PXPIN(2) & 0x40000000) && timeout--);
	  while (!(REG_GPIO_PXPIN(2) & 0x40000000));
	}
	#define WAIT_RB()	nand_wait_ready()
#endif

static unsigned int NANDFLASH_BASE = 0xB8000000;

#define REG_NAND_DATA  (*((volatile unsigned char *) NANDFLASH_BASE))
#define REG_NAND_CMD   (*((volatile unsigned char *) (NANDFLASH_BASE + NANDFLASH_CLE)))
#define REG_NAND_ADDR  (*((volatile unsigned char *) (NANDFLASH_BASE + NANDFLASH_ALE)))


#if MULTI_TASK_SUPPORT
	static  OS_EVENT  *SemDeviceOps;
	#define OP_NAND_LOCK() do{ \
								unsigned char  err; \
								OSSemPend(SemDeviceOps, 0, &err); \
							}while(0)
	#define OP_NAND_UNLOCK() do{ \
									unsigned char  err; \
									OSSemPost(SemDeviceOps); \
								}while(0)
#else
	#define OP_NAND_LOCK()
	#define OP_NAND_UNLOCK()
#endif


#if RS_INITERRUPT
	static OS_EVENT * rs_sem = NULL;
	#define __nand_ecc_decode_sync()     \
	do{                                  \
		unsigned char err;                 \
		REG_EMC_NFINTE  = EMC_NFINTE_ENCFE;      \
		OSSemPend(rs_sem,2,&err);          \
		if(err)                            \
			   printf("rs wait timeout!\n"); \
	}while(0)
#else
	#define __nand_ecc_decode_sync() while (!(REG_EMC_NFINTS & EMC_NFINTS_DECF))
#endif



//***********************************************************************************
//**		RS_CORRECT
//***********************************************************************************

void rs_correct(unsigned char *buf, int idx, int mask)
{
	int i, j;
	unsigned short d, d1, dm;

	i = (idx * 9) >> 3;
	j = (idx * 9) & 0x7;

	i = (j == 0) ? (i - 1) : i;
	j = (j == 0) ? 7 : (j - 1);
    	if(i >= 512)
		return;
	
	d = (buf[i] << 8) | buf[i - 1];

	d1 = (d >> j) & 0x1ff;
	d1 ^= mask;

	dm = ~(0x1ff << j);
	d = (d & dm) | (d1 << j);
    
	buf[i - 1] = d & 0xff;
	buf[i] = (d >> 8) & 0xff;
}

static int nand_rs_correct(unsigned char *tmpbuf)
{
	unsigned int stat = REG_EMC_NFINTS;
	unsigned int i = 0;
	//printf("stat = 0x%x\n", stat);
	#if 1
	for(i = 0; i < 512; i++)
	{
		printf(" %x",tmpbuf[i++]);	
		printf(" %x",tmpbuf[i++]);
		printf(" %x",tmpbuf[i++]);
		printf(" %x",tmpbuf[i++]);
		printf(" %x",tmpbuf[i++]);
		printf(" %x",tmpbuf[i++]);
		printf(" %x",tmpbuf[i++]);
		printf(" %x",tmpbuf[i]);
		printf(" \n");
	}
	#endif

	if (stat & EMC_NFINTS_ERR) {
		if (stat & EMC_NFINTS_UNCOR) {
			printf("nand_rs_correct:Uncorrectable error occurred!!\n");
			printf("stat = 0x%x\n", stat);
			
			return -1;
		}
		else {
			printf("Correctable error occurred\n");
			
			unsigned int errcnt = (stat & EMC_NFINTS_ERRCNT_MASK) >> EMC_NFINTS_ERRCNT_BIT;
//Note:rs correct one bit each time,so take away the first three break -by dsqiu
			printf("stat = 0x%x error count = %d\n", stat,errcnt);
			switch (errcnt) {
			case 4:
				rs_correct(tmpbuf, (REG_EMC_NFERR3 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT, (REG_EMC_NFERR3 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT);			
			case 3:
				rs_correct(tmpbuf, (REG_EMC_NFERR2 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT, (REG_EMC_NFERR2 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT);				
			case 2:
				rs_correct(tmpbuf, (REG_EMC_NFERR1 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT, (REG_EMC_NFERR1 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT);
			case 1:
				rs_correct(tmpbuf, (REG_EMC_NFERR0 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT, (REG_EMC_NFERR0 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT);
				break;
			default:
				
				break;
			}
		}
		
	}
	return 0;	
}


//****************************************************************
//*
//*			NAND INIT
//*
//****************************************************************

static NAND_INFO	*cur_NandInfo;
static unsigned int dwNandID = 0;

static unsigned int IsNandSupport(unsigned int dwNandID,unsigned int dwMClock)
{
	unsigned int dwData = 0, dwTime = 0, dwSMCRData;
	int i=0;

#define MCLOCK_BEAT (1000000000 / (dwMClock / 1000))
#define	NAND_TIMING_MARGIN		0	// Percent

	while ( TRUE )
	{
		if ( g_NandInfo[i].dwNandID == 0 )
			break;
		if ( g_NandInfo[i].dwNandID == dwNandID )
			break;
		i ++;
	}
	if ( g_NandInfo[i].dwNandID == 0 )
	{
		cur_NandInfo=0;
		return 0;
	}
	else
	{
		cur_NandInfo=(NAND_INFO	*)&g_NandInfo[i];
	}
	
	dwSMCRData = 0;
	// Tas
	dwTime = (cur_NandInfo->Tals * 100 + NAND_TIMING_MARGIN) / 100;
	dwData = (dwTime * 1000 + MCLOCK_BEAT - 1) / MCLOCK_BEAT;
	dwData = dwData > 7 ? 7 : dwData;
	dwSMCRData |= dwData << EMC_SMCR_TAS_BIT;
	// Tah
	dwTime = (cur_NandInfo->Talh * 100 + NAND_TIMING_MARGIN) / 100;
	dwData = (dwTime * 1000 + MCLOCK_BEAT - 1) / MCLOCK_BEAT;
	dwData = dwData > 7 ? 7 : dwData;
	dwSMCRData |= dwData << EMC_SMCR_TAH_BIT;
	// Twp
	dwTime = (cur_NandInfo->Twp * 100 + NAND_TIMING_MARGIN) / 100;
	dwData = (dwTime * 1000 + MCLOCK_BEAT - 1) / MCLOCK_BEAT;
	dwData = dwData > 12 ? 0xF : dwData;
	dwSMCRData |= dwData << EMC_SMCR_TBP_BIT;
	// Trp
	dwTime = (cur_NandInfo->Trp * 100 + NAND_TIMING_MARGIN) / 100;
	dwData = (dwTime * 1000 + MCLOCK_BEAT - 1) / MCLOCK_BEAT;
	dwData = dwData > 12 ? 0xF : dwData;
	dwSMCRData |= dwData << EMC_SMCR_TAW_BIT;

	dwTime = cur_NandInfo->Trhw > cur_NandInfo->Twhr ? cur_NandInfo->Trhw : cur_NandInfo->Twhr;
	dwTime = (dwTime * 100 + NAND_TIMING_MARGIN) / 100;
	dwData = (dwTime * 1000 + MCLOCK_BEAT - 1) / MCLOCK_BEAT;
	dwData = dwData > 15 ? 0xF : dwData;
	dwSMCRData |= dwData << EMC_SMCR_STRV_BIT;
#undef MCLOCK_BEAT
	return dwSMCRData;
}

static unsigned int nand_set_reg(void)
{
	unsigned int dwData = 0, dwSMCRData;
	dwSMCRData = IsNandSupport(dwNandID,__cpm_get_mclk());
	if (dwSMCRData == 0)
		return 0;

	dwData = REG_EMC_SMCR1;
	REG_EMC_SMCR1 = (dwSMCRData | (dwData & 0x0FF));
	REG_EMC_NFCSR |= (EMC_NFCSR_NFCE1 | EMC_NFCSR_NFE1 );

	printf("NAND Flash: Timing = 0x%x\r\n", dwSMCRData);
}

static unsigned int jz_nand_scan_id (void)
{
#ifdef JZ_DEBUG
	printf("%s()\n", __FUNCTION__);
#endif

	char	cVendorID, cDeviceID, cTemp = 0;
	
	OP_NAND_LOCK();
	JZ_NAND_SELECT;
    REG_NAND_CMD = NAND_CMD_READ_ID1;
	REG_NAND_ADDR = NAND_CMD_READ1_00;
	cVendorID = NAND_IO_ADDR;
	cDeviceID = NAND_IO_ADDR;
	JZ_NAND_DESELECT;
	
	dwNandID = ((cVendorID & 0x000000ff) << 8) | (cDeviceID & 0x000000ff);

	printf("NAND Flash: 0x%X%X is found\r\n", cVendorID, cDeviceID);

	OP_NAND_UNLOCK();	
	nand_set_reg();

	g_dwPageSize = cur_NandInfo->dwPageSize;
	g_dwBlockSize = cur_NandInfo->dwBlockSize;
	g_dwOobSize = g_dwPageSize / 512 * 16;
	
	return 	dwNandID;
}
#if NAND_INTERRUPT
static void nand_rb_handler(unsigned int arg)
{
    __gpio_mask_irq(arg);
	OSSemPost(rb_sem);	
}
#endif

#if RS_INITERRUPT
void nand_rs_decode_handler(unsigned int arg)
{
	REG_EMC_NFINTE &= (~EMC_NFINTE_ENCFE);
	OSSemPost(rs_sem);
}
#endif

extern unsigned int NANDID;     //treckle
static int jz_nand_hw_init (void)
{
    	unsigned int dat;

	REG_EMC_NFCSR |= EMC_NFCSR_NFE1;
    	dat = REG_EMC_SMCR1;
	dat &= ~0x0fffff00;

	REG_EMC_SMCR1 = dat | 0x0fff7700;

	printf("REG_EMC_SMCR1 = 0x%08x\r\n",REG_EMC_SMCR1);
	
#if NAND_INTERRUPT	
		
    	NAND_CLEAR_RB();
	
	__gpio_as_irq_rise_edge(RB_GPIO_PIN);
	__gpio_disable_pull(RB_GPIO_PIN);
    
    	if(rb_sem == NULL)
		rb_sem = OSSemCreate(0);
	request_irq(IRQ_GPIO_0 + RB_GPIO_PIN, nand_rb_handler,RB_GPIO_PIN);
#endif

#if MULTI_TASK_SUPPORT
	SemDeviceOps  = OSSemCreate(1);
#endif

#if RS_INITERRUPT
	rs_sem = OSSemCreate(0);
	ack_irq(IRQ_EMC);
	request_irq(IRQ_EMC,nand_rs_decode_handler,0);	
#endif
	NANDID=jz_nand_scan_id();

	return 0;
}
int jz_nand_init (void)
{
	static int ret = 0 ;	
	if ( ret > 0 ) return 0;
	ret = 1;
	
	jz_nand_hw_init();
	
	return 0;
}

int jz_nand_get_info(pflash_info_t pflashinfo)
{
	printf("dwBlockSize[%d]\n",cur_NandInfo->dwBlockSize);
	
	pflashinfo->pnandinfo = cur_NandInfo;
	pflashinfo->dwBytesPerBlock = cur_NandInfo->dwBlockSize;
	pflashinfo->dwDataBytesPerSector = cur_NandInfo->dwPageSize;
	pflashinfo->dwSectorsPerBlock = cur_NandInfo->dwBlockSize / cur_NandInfo->dwPageSize;
	pflashinfo->dwFSStartBlock = 4096; //IMAGE_RSV_LENTH / g_NandInfo.dwBlockSize + 1;
	pflashinfo->dwFSTotalBlocks = cur_NandInfo->dwMaxValidBlocks - pflashinfo->dwFSStartBlock;
	pflashinfo->dwFSTotalSectors = pflashinfo->dwFSTotalBlocks * pflashinfo->dwSectorsPerBlock;

	pflashinfo->dwTotalPhyBlocks = cur_NandInfo->dwMaxValidBlocks;
	pflashinfo->dwMaxBadBlocks = cur_NandInfo->dwMaxValidBlocks - cur_NandInfo->dwMinValidBlocks;
	
	return 1;
}


//****************************************************************
//*
//*			READ OPERATION
//*
//****************************************************************

static inline int nand_send_readaddr(unsigned int pageaddr, unsigned int offset)
{
	int cycle = CONFIG_SSFDC_NAND_ROW_CYCLE;
	unsigned char err;
	if(offset < 512)
	{
		if(offset < 256)	
		{	
			REG_NAND_CMD = NAND_CMD_READ1_00;
		}
		else
		{
			REG_NAND_CMD = NAND_CMD_READ1_01;
		}
	}else
	{
		REG_NAND_CMD = NAND_CMD_READ2;
	}
	REG_NAND_ADDR = (unsigned char)(offset & 0xff);
	while(cycle > 0)
	{     
		REG_NAND_ADDR = (unsigned char)(pageaddr & 0xff); // Page address.
		pageaddr >>= 8;
		cycle--;	
	}
	
	CLEAR_RB();
	
	//Wait R/B
	#if NAND_INTERRUPT	
		OSSemPend(rb_sem,READDATA_TIMEOUT,&err);
		if(err)
		{
			printf("timeout\n pageaddr=%d, offset=%d\n", pageaddr, offset);
			return -1;
		}
	#else
		nand_wait_ready();
	#endif
	
	return 0;
}

static inline int nand_read_oob(int page,nand_page_info_t *info)
{
	
	unsigned char *ptr;
	unsigned int i;
	if(nand_send_readaddr(page,NAND_PAGE_SIZE))
	    return -1;
	ptr = (unsigned char *)info;	
		
	for ( i = 0; i < NAND_OOB_SIZE; i++) 
	{
		unsigned char d;
		d =NAND_IO_ADDR;
		*ptr++ = d;
		
	}
	
	unsigned char err;
	CLEAR_RB();
	//Wait R/B
	#if NAND_INTERRUPT	
		OSSemPend(rb_sem,READDATA_TIMEOUT,&err);
		if(err)
		{
			printf("timeout\n pageaddr=%d\n", page);
			return -1;
		}
	#else
		nand_wait_ready();
	#endif
	
	return 0;
	
}

int jz_nand_read_page_info (void *dev_id, int page, nand_page_info_t *info)
{
	int ret;
	
#ifdef JZ_DEBUG    
	printf("%s() page: 0x%x\n", __FUNCTION__, page);
#endif
	OP_NAND_LOCK();
	JZ_NAND_SELECT;
	
	ret = nand_read_oob(page,info);
	
	JZ_NAND_DESELECT;
	OP_NAND_UNLOCK();
	return ret;

}

int jz_nand_read_page(void *dev_id, int page, unsigned char *data,nand_page_info_t *info)
{
	int i,j,ret = 0;
	
	unsigned char *ptr;
	unsigned char *par;
	unsigned char *tmpbuf;
	unsigned int stat;
	volatile unsigned char *paraddr = (volatile unsigned char*)EMC_NFPAR0;
	
#ifdef JZ_DEBUG
	printf("%s() page: 0x%x!!!!!!!!!!!\n", __FUNCTION__, page);
#endif

	OP_NAND_LOCK();	
	JZ_NAND_SELECT;

	if(nand_read_oob(page,info))
	{
		JZ_NAND_DESELECT;
		OP_NAND_UNLOCK();
		printf("%s page 0x%x oob error!",__FUNCTION__,page);
		return -1;    
	}
	#if 0
	printf("info->block_status[0x%x]\n",info->block_status);
	printf("info->block_addr_field[0x%x]\n",info->block_addr_field);
	printf("info->lifetime[0x%x]\n",info->lifetime);
	printf("page info->page_status[0x%x]\n",info->page_status);
	printf("info->block_erase[0x%x]\n",info->block_erase);
	for (i = 0; i < 8; i++)
		printf(" %x %x %x %x\n",info->ecc_field[i++],info->ecc_field[i++],info->ecc_field[i++],info->ecc_field[i]);
	printf(" %x\n",info->ecc_field[i]);
	#endif
	ptr = (unsigned char *) data;
	
/*	if(info->page_status == 0xff)
	{
		JZ_NAND_DESELECT;
		OP_NAND_UNLOCK();
		printf("%s page 0x%x oob error!",__FUNCTION__,page);
		return -1;   
	}
	else */
	if(1)
	{  
		par = (unsigned char *)&(info->ecc_field);
		nand_send_readaddr(page,0);
		
		for(j = 0; (j < JZSOC_ECC_STEPS) && (ret == 0); j++)
		{
			if(1)
			{
				tmpbuf = ptr;
	
				REG_EMC_NFINTS = 0x0;
				__nand_ecc_rs_decoding();
	
			
				for(i = 0; i < JZSOC_ECC_BLOCK; i++)
				{
					unsigned int dat;
					dat = NAND_IO_ADDR;
					*ptr++ = dat;
				}
		
				for(i = 0; i < PAR_SIZE; i++)
				{
				  *paraddr++ = *par++;
				}
				
				REG_EMC_NFECR |= EMC_NFECR_PRDY;
				
				__nand_ecc_decode_sync();
				__nand_ecc_disable();
                		if(HARDECC)
				{
					ret =  nand_rs_correct(tmpbuf);
				
					if(ret == -1){
						printf("%s page = 0x%x,sector = %d\n",__FUNCTION__,page,j);								  
					}
				}
			}
			else
			{
				ptr += JZSOC_ECC_BLOCK;
				par += PAR_SIZE;
				  
			}			
		}
	
	}

	JZ_NAND_DESELECT;
	OP_NAND_UNLOCK();
	return ret;
}


//****************************************************************
//*
//*			WRITE OPERATION
//*
//****************************************************************

void nand_send_writeaddr(unsigned int pageaddr,unsigned int offset)
{
	int cycle = CONFIG_SSFDC_NAND_ROW_CYCLE;

	if(offset < 512)
	{
		if(offset < 256)
		{
			REG_NAND_CMD = NAND_CMD_READ1_00;
		}
		else
		{
			REG_NAND_CMD = NAND_CMD_READ1_01;
		}
	}
	else
	{
		REG_NAND_CMD = NAND_CMD_READ2;
	}
	REG_NAND_CMD = NAND_CMD_PAGE_PROGRAM_START;// Send write command.
	REG_NAND_ADDR = (unsigned char)(offset & 0xff);
	
	while(cycle > 0)
	{
		REG_NAND_ADDR = (unsigned char)(pageaddr & 0xff);// Page address.
		pageaddr >>= 8;
		cycle--;	
	}
}

int jz_nand_write_page (void *dev_id, int page, unsigned char *data, nand_page_info_t *info)
{
	int i,j;
	unsigned char *ptr, status;
	unsigned char *par = (unsigned char *)&(info->ecc_field);
	volatile unsigned char *paraddr = (unsigned char *)EMC_NFPAR0; 
		
#ifdef JZ_DEBUG
	printf("!!!!!!!gt!!! %s() page: 0x%x 0x%x 0x%x\n", __FUNCTION__, page, *data, *(data+1));
#endif
	
	OP_NAND_LOCK();
	JZ_NAND_SELECT;
	
	nand_send_writeaddr(page,0);
	ptr = (unsigned char *) data;

	for(j = 0;j<JZSOC_ECC_STEPS; j++)
	{
		REG_EMC_NFINTS = 0;
		__nand_ecc_rs_encoding();

		for(i =0; i < JZSOC_ECC_BLOCK; i++)	
			NAND_IO_ADDR = *ptr++;

		__nand_ecc_encode_sync();

		if(HARDECC)
			for (i = 0; i < PAR_SIZE; i++)
			{
				*par++ = *paraddr++;
			}

		__nand_ecc_disable();
	}
	
	ptr = (unsigned char *) info;
	for (i = 0; i < NAND_OOB_SIZE; i++)
		NAND_IO_ADDR = *ptr++;

	CLEAR_RB();
	
	REG_NAND_CMD = NAND_CMD_PAGE_PROGRAM_STOP;
	#if NAND_INTERRUPT
		unsigned char err;
	
		OSSemPend(rb_sem,PAGE_TIMEOUT,&err);
		if(err)
		{
			printf("!!!!!!!error!!! %s() page: 0x%x \n", __FUNCTION__, page);
			JZ_NAND_DESELECT;
			goto write_error;
			
		}
	#else
		nand_wait_ready();
	#endif

	REG_NAND_CMD = NAND_CMD_READ_STATUS;

    	i = NAND_OOB_SIZE * JZ_WAIT_STAUTS_DELAY * 1000;
	while((NAND_IO_ADDR & 0x01) &&(i--)) ;
	JZ_NAND_DESELECT;
	if(i > 0)
	{	
		OP_NAND_UNLOCK();
		return 0;
	}

write_error:
	printf("write page: 0x%x failed\n", page);
	OP_NAND_UNLOCK();
	return -1;
}

int jz_nand_write_page_info (void *dev_id, unsigned int page, nand_page_info_t *info)
{
	int i;
	unsigned char *ptr, status;
	OP_NAND_LOCK();

	JZ_NAND_SELECT;

	nand_send_writeaddr(page,NAND_PAGE_SIZE);
	ptr = (unsigned char *) info;
	for (i = 0; i < NAND_OOB_SIZE; i++) 
	{
		NAND_IO_ADDR = *ptr++;
	}

	CLEAR_RB();
	
	REG_NAND_CMD = NAND_CMD_PAGE_PROGRAM_STOP;
		
	#if NAND_INTERRUPT
		unsigned char err;
		OSSemPend(rb_sem,PAGE_TIMEOUT,&err);
		if(err)
		{
			printf("!!!!!!!error!!! %s() page: 0x%x \n", __FUNCTION__, page);
			JZ_NAND_DESELECT;
			goto write_error;
		}
	#else
		nand_wait_ready();
	#endif

	REG_NAND_CMD = NAND_CMD_READ_STATUS;

	i = NAND_OOB_SIZE * JZ_WAIT_STAUTS_DELAY * 1000;
	while((NAND_IO_ADDR & 0x01) &&(i--)) ;
	
	JZ_NAND_DESELECT;
	if(i > 0)
	{
		OP_NAND_UNLOCK();
		return 0;
	}
 write_error:
	OP_NAND_UNLOCK();
	printf("write page : 0x%x 's info failed\n", page);
	return -1;
}

#define JZ_NAND_ERASE_TIME (30 * 1000)

void nand_send_eraseAddr(unsigned int pageaddr)
{
	int cycle = CONFIG_SSFDC_NAND_ROW_CYCLE;

	REG_NAND_CMD = NAND_CMD_BLOCK_ERASE_START;// Send block erase command.

	while (cycle > 0)
	{
		REG_NAND_ADDR = (unsigned char)(pageaddr & 0xff);// Page address.

		pageaddr >>= 8;
		cycle--;
	}
}

int jz_nand_erase_block (void *dev_id, unsigned int block)
{
	int i;
	unsigned char status;
	unsigned long addr = block * CONFIG_SSFDC_NAND_PAGE_PER_BLOCK;
#ifdef JZ_DEBUG
	printf("$$$$$$$$$$$ %s() block: 0x%x addr=0x%x\n", __FUNCTION__, block, addr);
#endif
	OP_NAND_LOCK();
	JZ_NAND_SELECT;
	
	nand_send_eraseAddr(addr);
	
	REG_NAND_CMD = NAND_CMD_BLOCK_ERASE_CONFIRM;
	
	CLEAR_RB();
	
	#if NAND_INTERRUPT
		unsigned char err;
		OSSemPend(rb_sem,PAGE_TIMEOUT,&err);
		if(err)
		{
			JZ_NAND_DESELECT;
			goto erase_error;
			
		}
    #else
		nand_wait_ready();
    #endif
    
	REG_NAND_CMD = NAND_CMD_READ_STATUS;
	
	i = JZ_NAND_ERASE_TIME / 2 ;
	while((NAND_IO_ADDR & 0x01) && (i--));
	JZ_NAND_DESELECT;
	if(i > 0)
	{
		OP_NAND_UNLOCK();	
		return 1;
	}

erase_error:
	printf("erase block: 0x%x failed\n", block);
	OP_NAND_UNLOCK();
	return 0;
}

void jz_nand_ecc(char en)
{
	if(en)
		HARDECC=1;
	else
		HARDECC=0;
}

