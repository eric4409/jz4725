/**********************************************************************

Author:			Andy.Wu
Date:			2006.6.8

Description:	this file is the driver of nand_flash. 

***********************************************************************/

//#include "header.h"
#include "jz_nand.h"
#include "structure.h"
#include "yaffs_guts.h"

#if 0
unsigned char PairedPage[64]=
{
0x00,0x01,
0x02,0x03,0x06,0x07,0x0a,0x0b,0x0e,0x0f,
0x12,0x13,0x16,0x17,0x1a,0x1b,0x1e,0x1f,
0x22,0x23,0x26,0x27,0x2a,0x2b,0x2e,0x2f,
0x32,0x33,0x36,0x37,0x3a,0x3b,0x3e,0x3f,
0x42,0x43,0x46,0x47,0x4a,0x4b,0x4e,0x4f,
0x52,0x53,0x56,0x57,0x5a,0x5b,0x5e,0x5f,
0x62,0x63,0x66,0x67,0x6a,0x6b,0x6e,0x6f,
0x72,0x73,0x76,0x77,0x7a,0x7b
};
#else
unsigned char PairedPage[64]=
{
0x04,0x05,0x08,0x09,0x0c,0x0d,
0x10,0x11,0x14,0x15,0x18,0x19,0x1c,0x1d,
0x20,0x21,0x24,0x25,0x28,0x29,0x2c,0x2d,
0x30,0x31,0x34,0x35,0x38,0x39,0x3c,0x3d,
0x40,0x41,0x44,0x45,0x48,0x49,0x4c,0x4d,
0x50,0x51,0x54,0x55,0x58,0x59,0x5c,0x5d,
0x60,0x61,0x64,0x65,0x68,0x69,0x6c,0x6d,
0x70,0x71,0x74,0x75,0x78,0x79,0x7c,0x7d,
0x7e,0x7f
};
#endif
extern int jz_nand_init (void);
extern int jz_nand_erase_block (void *dev_id, unsigned int block);
extern int jz_nand_write_page (void *dev_id, int page, unsigned char *data, nand_page_info_t *info);
extern int jz_nand_read_page (void *dev_id, int page, unsigned char *data,nand_page_info_t *info); 

#define  debug  0
/*************************************************
Initialize the  hardware of nandflash
*************************************************/
int nand_flash_hwr_init(void)
{
	jz_nand_init (); 
	return 0;
}



/************************************************
Erase a block on nandflash;
block:	the number of block;

*************************************************/
int nand_flash_erase_block(int block)
{
	if(jz_nand_erase_block (0,block))
	{
	//	#if debug
		printf("*****>>>>>Nand erased block No. %d\n OK",block);
	//	#endif
		return 0;
	}
	else
	{
		printf("Nand erase block No. %d\n failed!\n",block);
		return -1;
	}
}

/************************************************
Write data to blockpage;
blockpage:
data:	the first 512bytes
spare:	the next 16bytes
*************************************************/
//int nand_flash_program_buf(int blockpage, const unsigned char *data, const unsigned char *spare)
int nand_flash_program_buf(int blockpage, unsigned char *data, unsigned char *spare)
{
	int i;
	unsigned char buf[64];
	unsigned char dbuf[CONFIG_SSFDC_NAND_PAGE_SIZE];	//2048
	nand_page_info_t *info=(nand_page_info_t *)buf;
	yaffs_Spare *spare_t=(yaffs_Spare *)spare;

	unsigned char page;
	unsigned int block;

/*	for(i=0;i<10;i++)
	{
		printf("data[%d]=%d\n",i,data[i]);//treckle
	} */
	/* if(spare[0]==0xff*/
//	printf("Writing:Before map, blockpage = %d\n",blockpage);
	block=blockpage/YAFFS_CHUNKS_PER_BLOCK;
	page=blockpage%YAFFS_CHUNKS_PER_BLOCK;
	blockpage=block*YAFFS_CHUNKS_PER_BLOCK*2 + PairedPage[page];
//	printf("Writing:After map, blockpage = %d,PariedPage=%d,block=%d,page=%d\n",blockpage,PairedPage[page],block,page);

#if 1
	memset(buf,0xff,64);

	if(data)
	{
/*
		for(i=0;i<3+16;i++)
		{
			printf("write data[%x]=%x\n",i,data[i]);//treckle
		}
*/
	
		if(spare)
		{
			memcpy(info->spare_yaffs,spare,16);
		//the first byte of 64bytes spare is the flag for nand flash
			info->block_status=spare_t->blockStatus; 
		//	printf("blockstatus=%x\n",info->block_status);//treckle
		}
		else
		{	
			printf("yaffs try to write data only, it's invalid!\n");
			return -1;
		}

		memset(dbuf,0xff,CONFIG_SSFDC_NAND_PAGE_SIZE);	//2048
		memcpy(dbuf,data,YAFFS_BYTES_PER_CHUNK);	//512
		if(jz_nand_write_page(0, blockpage, dbuf, info)!=0)
		{
		//	#if debug
			printf("Nand program failed!\n");
		//	#endif
			return -1;
		}
		else
		{
			#if debug
			printf("Nand program page No. %d ok\n",blockpage);//treckle
			#endif
		}
	}
/*
	else
	{
		printf(" yaffs try to write spare only !\n");//treckle
		return -1;
	}
*/
#if 1
	else if(spare)
	{
		if(jz_nand_read_page_info(0, blockpage,info)!=0)
		{
			printf("Nand Read oob failed!\n");
			return -1;
		}
		else
		{
			memcpy(info->spare_yaffs,spare,16);
		//the first byte of 64bytes spare is the flag for nand flash
			info->block_status=spare_t->blockStatus; 
	//	printf("blockstatus=%x\n",info->block_status);//treckle
	//	printf("block status when writing oob=%x\n",spare[0]);
			if(jz_nand_write_page_info(0, blockpage, info)!=0)		
			{
			//	#if debug
				printf("Nand write oob failed!\n");
			//	#endif
				return -1;
			}	
			else
			{
			//	#if debug
				printf("Nand write oob of page No. %d ok\n",blockpage);//treckle
			//	#endif
			}
		}
	}
#endif
#endif
	return 0;
}



/************************************************
Write data to blockpage;
blockpage:
data:	the first 512 bytes
spare:	the next 16 bytes
*************************************************/

int nand_flash_read_buf(int blockpage, unsigned char *data,unsigned char *spare)
{
	unsigned char buf[64];
	unsigned char dbuf[CONFIG_SSFDC_NAND_PAGE_SIZE];

	nand_page_info_t *info=(nand_page_info_t *)buf;
	yaffs_Spare *spare_t=(yaffs_Spare *)spare;
	int i;

        unsigned char page;
        unsigned int block;

//        printf("Reading:Before map, blockpage = %d\n",blockpage);
        block=blockpage/YAFFS_CHUNKS_PER_BLOCK;
        page=blockpage%YAFFS_CHUNKS_PER_BLOCK;
        blockpage=block*YAFFS_CHUNKS_PER_BLOCK*2 + PairedPage[page];
//        printf("Reading:After map, blockpage = %d,PariedPage=%d,block=%d,page=%d\n",blockpage,PairedPage[page],block,page);


	memset(buf,0xff,64);

	if(data)
	{
		memset(dbuf,0xff,YAFFS_BYTES_PER_CHUNK);	//512
		if(jz_nand_read_page (0, blockpage, dbuf, info)!=0)		
		{
			#if debug
			printf("Nand read failed!\n");
			#endif
			return -1;
		}
		else	
		{		
			#if debug
			printf("Nand read page No. %d ok\n",blockpage);//treckle
			#endif
		}

		memcpy(data,dbuf,YAFFS_BYTES_PER_CHUNK);	//512
		if(spare) 
		{
			memcpy(spare,info->spare_yaffs,16);
	//the first byte of 64bytes spare is the flag for nand flash, we must tell it to yaffs
			spare_t->blockStatus=info->block_status; 
			return 0;
		}

	/*	for(i=0;i<3;i++)
		{
			printf("Nand read data[%x]=%x\n",i,data[i]);//treckle
		} 
	*/
	}

	if(spare)
	{
		if(jz_nand_read_page_info(0, blockpage,info)!=0)		
		{
			#if debug
			printf("Nand read oob failed!\n");
			#endif
			return -1;
		}	
		else
		{
			#if debug
			printf("Nand read oob of page No. %d ok\n",blockpage);//treckle
			#endif
		}

		memcpy(spare,info->spare_yaffs,16);
		//the first byte of 64bytes spare is the flag for nand flash, we must tell it to yaffs
		spare_t->blockStatus=info->block_status; 
	}
#if 0
	if(spare)
	{
		for(i=0;i<16;i++)
			printf("spare of yaffs[%d]=%d\n",i,info->spare_yaffs[i]);
	}
#endif
	return 0;
}


