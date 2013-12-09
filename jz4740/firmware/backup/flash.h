/*************************************************
                                           
 ZEM 200                                          
                                                    
 flash.h                            
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef _FLASH_H_
#define _FLASH_H_

#include "arca.h"

#define FLASH_SECTOR_COUNT 32 
#define STD_SECTOR_SIZE 0x400*64

//Free sectors for Application data 
#define FREE_FLASH_COUNT 30
#define FREE_FLASH_START 0

#define FLASH_TOP_BOOT       0x227E
#define FLASH_BOTTOM_BOOT   0x227D

U32 FlashSectorSizes[FLASH_SECTOR_COUNT];

//ARCA FLASH MAP MEMORY
REG16* gFlash16;	//START ADDRESS FOR 2M BYTES OFFSET	
REG16* FlashBaseAddr;	//FLASH MEMORY START ADDRESS

int GetFlashID(void);
void FlashReset(void);
U32 GetFlashStartAddress(int SecoterIndex);
int GetFlashSectorIndex(U32 Address);
int DeleteDataInSector(U32 Addr, U32 size);
int FlashSectorEraseByIndex(int i);
int FlashSectorErase(U32 SectorAddress);
int FlashWriteWord(U32 Addr, U16 Data);
int FlashSaveBuffer(U32 Addr, void *Buffer, U32 Size);
int FlashReadBuffer(U32 Addr, void *Buffer, U32 Size);

int TestFlash(void); // Test only, this function while erase data in the flash

#endif

