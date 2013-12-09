/*************************************************
                                           
 ZEM 200                                          
                                                    
 flash.c                             
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include "arca.h"
#include "flash.h" 

#define FLASH_RESET_WAIT_TIME 10   //10us
#define FLASH_DEVICECODE_WAIT_TIME 30   //30us

#define FLASH_SECTOR_ERASE_WAIT_TIME 500*1000  //500*1000us
#define FLASH_SECTOR_ERASE_LOOP_WAIT_TIME 1000  //1000us
#define FLASH_SECTOR_ERASE_LOOP_TIMES 500  
#define FLASH_SECTOR_ERASE_TRY_TIMES 3

#define FLASH_SECTOR_WRITE_WAIT_TIME 20  //10us
#define FLASH_SECTOR_WRITE_LOOP_WAIT_TIME 10 //10us
#define FLASH_SECTOR_WRITE_LOOP_TIMES 100  
#define FLASH_SECTOR_WRITE_TRY_TIMES 10

//DEVICE CODE
int GetFlashID(void)		
{
	int i;
	
	FlashBaseAddr[0x555]=0xAA;  FlashBaseAddr[0x2AA]=0x55;
	FlashBaseAddr[0x555]=0x90;  
	DelayUS(FLASH_DEVICECODE_WAIT_TIME);
	i = FlashBaseAddr[1];	
	FlashBaseAddr[0]=0xF0;
	DelayUS(FLASH_RESET_WAIT_TIME);		
	return i;
}

void FlashReset(void)
{
	GetFlashID();
}

//涉及到的地址是Flash的绝对地址 BYTE
U32 GetFlashStartAddress(int SecoterIndex)
{
	U32 a = (U32)gFlash16;
	int i = 0;
	
	while(i < SecoterIndex)
	{
		a+=FlashSectorSizes[i];
		i++;
	}
	return a;
}

//涉及到的地址是Flash的绝对地址 BYTE
int GetFlashSectorIndex(U32 Address)
{
	U32 a = (U32)gFlash16;
	int i = -1;
	
	while(a <= Address)
	{
		i++;
		if (i >= FLASH_SECTOR_COUNT) return -2;
		a += FlashSectorSizes[i];
	}
	return i;	
}

//涉及到的地址是Flash的绝对地址 BYTE 不支持跨扇区操作
int DeleteDataInSector(U32 Addr, U32 size)
{
	int i,index;
	U32 a;
	U16 *buf;
	
	index = GetFlashSectorIndex(Addr);
	if (index < 0) return -1;
	a = GetFlashStartAddress(index);
	if ((size + Addr) > (a + FlashSectorSizes[index]))
		size = a + FlashSectorSizes[index] - Addr;
	//复制数据
	buf = (U16*)malloc(FlashSectorSizes[index]);
	for(i = 0; i < FlashSectorSizes[index]/2; i++)
		buf[i] = ((U16*)a)[i];

	//移除数据
	for(i=Addr-a;i<FlashSectorSizes[index]-size;i++)
		((U8*)buf)[i]=((U8*)buf)[i+size];
	for(i=FlashSectorSizes[index]-size;i<FlashSectorSizes[index];i++)
		((U8*)buf)[i]=0xFF;
	//回写数据
	i=FlashSaveBuffer(a, buf, FlashSectorSizes[index]);
	free(buf);
	return i;
}

int FlashSectorEraseByIndex(int i)
{	
    return FlashSectorErase(GetFlashStartAddress(i));
}

//涉及到的地址是Flash的绝对地址 BYTE
int FlashSectorErase(U32 Addr)
{
	int i, j;
	
	for(j=0; j<FLASH_SECTOR_ERASE_TRY_TIMES; j++){
	    FlashBaseAddr[0x555]=0xAA;  FlashBaseAddr[0x2AA]=0x55;
	    FlashBaseAddr[0x555]=0x80;  FlashBaseAddr[0x555]=0xAA;
	    FlashBaseAddr[0x2AA]=0x55;  *((U16*)Addr)=0x30;
	    DelayUS(FLASH_SECTOR_ERASE_WAIT_TIME);
	
	    i = FLASH_SECTOR_ERASE_LOOP_TIMES;
	    while(i--)
	    { 
		if(*((U16*)Addr)==0xFFFF) 
		    if(*((U16*)Addr+0x10)==0xFFFF) //16 WORD OFFSET
			if(*((U16*)Addr+0x100)==0xFFFF)  //256 WORD OFFSET
			    if(*((U16*)Addr+0x200)==0xFFFF)  //512 WORD OFFSET
				return 1;
		DelayUS(FLASH_SECTOR_ERASE_LOOP_WAIT_TIME);
	    }
	    //Error so it should be reset before try it again
	    FlashBaseAddr[0]=0xF0;
	    DelayUS(FLASH_RESET_WAIT_TIME);		
	}
	return 0;
}

//涉及到的地址是Flash的绝对地址 BYTE
int FlashWriteWord(U32 Addr, U16 Data)
{
	int i, j;
	
	for(j=0; j<FLASH_SECTOR_WRITE_TRY_TIMES; j++)
	{
		FlashBaseAddr[0x555]=0xAA;  FlashBaseAddr[0x2AA]=0x55;
		FlashBaseAddr[0x555]=0xA0;  *((U16*)Addr)=Data;
		
		DelayUS(FLASH_SECTOR_WRITE_WAIT_TIME);
		
		i=FLASH_SECTOR_WRITE_LOOP_TIMES;
		while(i--)
		{ 
			if(*((U16*)Addr)==Data)
			{ 
				return 1;
			}
			DelayUS(FLASH_SECTOR_WRITE_LOOP_WAIT_TIME);
		}
		//Error so it should be reset before try it again
		FlashBaseAddr[0]=0xF0;
		DelayUS(FLASH_RESET_WAIT_TIME);		
	}
	return 0;
}

//涉及到的地址是Flash的绝对地址 BYTE 不支持跨扇区操作!!!
//奇数地址, 则向后移写入(避免删除SECTOR 追加方式)
int FlashSaveBuffer(U32 Addr, void *Buffer, U32 Size)
{
	U32 i,SectorStartAddr,StartAddr,EndAddr,SecSize,Erased,ThisSize;
	U16 *TempBuffer;
	
	TempBuffer=(U16 *)malloc(STD_SECTOR_SIZE); 
	
	//get flash sector index and size
	i=GetFlashSectorIndex(Addr);
	SecSize=FlashSectorSizes[i];
	//get flash start address
	SectorStartAddr=GetFlashStartAddress(i);

	//read from flash to buffer
	for(i=0;i<SecSize/2;i++)
	{
		TempBuffer[i]=((U16*)SectorStartAddr)[i];
		//if(i%150==0) printf("%d %x\n", i, SectorStartAddr+i*2);
	}			
	//Determine the bound of tempbuffer and buffer		
	if(SectorStartAddr < Addr) 
	    StartAddr=Addr-SectorStartAddr;
	else 
	    StartAddr=0;
		
	EndAddr=StartAddr + Size;		
	if(EndAddr > SecSize) EndAddr = SecSize;
	ThisSize=EndAddr-StartAddr;
		
	//check diff of buffer
	if(memcmp((U8*)TempBuffer+StartAddr, Buffer, ThisSize))
	{
		//check if erase this section
		Erased=0;
		for(i=StartAddr;i<EndAddr;i+=2)
		{
			if(!(TempBuffer[i/2]==0xFFFF))
			{
			      if (!FlashSectorErase(SectorStartAddr)){
			           free(TempBuffer);
				    return -1;
				}
				Erased=1;
				break;
			}
		}
		memcpy((U8*)TempBuffer+StartAddr, Buffer, ThisSize);
		if(Erased)  //if erased old data, write back all data of the section
		{ 
			i=0; 
			EndAddr=SecSize;
		}
		else
		{ 
			i=(StartAddr/2)*2; //adjust odd address to previous
			EndAddr=((EndAddr+1)/2)*2; 			
		}
		
		while(i < EndAddr)
		{
			if (!FlashWriteWord(SectorStartAddr + i, TempBuffer[i/2]))
			{
				free(TempBuffer);
				return -2;
			}
			i+=2;
		}
	}
	
	free(TempBuffer);
	return 1;
}

//涉及到的地址是Flash的绝对地址 BYTE
//WORD的保存格式 低8位在前 高8位在后
int FlashReadBuffer(U32 Addr, void *Buffer, U32 Size)
{
	int SAddr;
	U32 v;
	
	Addr=Addr-(U32)gFlash16; //得到相对地址(gFlash16)	
	SAddr=Addr >> 1;
	if(!(Addr==(SAddr<<1))) //Odd number of Address 奇数
	{
		v=gFlash16[SAddr++];
		*(U8*)Buffer=v >> 8; Buffer=(U8*)Buffer+1;
		Size--;
		while(Size>1)
		{
			v=gFlash16[SAddr++];
			*(U8*)Buffer=v & 0xFF; Buffer=(U8*)Buffer+1;
			*(U8*)Buffer=v >> 8; Buffer=(U8*)Buffer+1;
			Size-=2;
		}
	}
	else
	while(Size>1)
	{
		*(U16*)Buffer=gFlash16[SAddr++];
		Buffer=(U8*)Buffer+2;
		Size--;Size--;
	}
	
	if(Size)  //odd size
	{
		*(U8*)Buffer=gFlash16[SAddr] & 0xFF;
	}
	return 1;
}
