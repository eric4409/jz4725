/**********************************************************************

Author:			Andy.Wu
Date:			2006.6.8

Description:	this file is the driver of nand_flash. 

***********************************************************************/

//#include "header.h"
#include "jz_nand.h"
#include "structure.h"
#include "yaffs_guts.h"

#ifndef YAFFS1
#include "yaffs_packedtags2.h"
#endif

struct NAND_INFO
{
        unsigned int    NandID;
	unsigned char	PosBlock;	/* the bad block ID position in block */
	unsigned char	PosPage;	/* the bad block ID position in page */
};

const struct NAND_INFO NandInfo[]=
{
	{0xECD3,	127,	0},	//SAMSUNG_K9G8G08U0M
	{0xEC75,	0,	6},
	{0xFFFF,	0, 	0},	//Detault unknow nand flash
};

unsigned int NANDID = 0xECD3; /* 0xECD3 is the default Nand ID, the extern function will initialise this value */
struct NAND_INFO *Nand_Info_t = NULL;

extern int jz_nand_init (void);
extern int jz_nand_erase_block (void *dev_id, unsigned int block);
extern int jz_nand_write_page (void *dev_id, int page, unsigned char *data, nand_page_info_t *info);
extern int jz_nand_write_page_info (void *dev_id, unsigned int page, nand_page_info_t *info);
extern int jz_nand_read_page (void *dev_id, int page, unsigned char *data,nand_page_info_t *info); 
extern int jz_nand_read_page_info (void *dev_id, int page, nand_page_info_t *info);
extern void jz_nand_ecc(char en);

#define  debug  0
/*************************************************
Initialize the  hardware of nandflash
*************************************************/
int nand_flash_hwr_init(void)
{
	unsigned char i;

	jz_nand_init (); 
	printf("Nand initialise:The NandID=%X\n",NANDID);
	for(i=0; ;i++)
	{
		if(NandInfo[i].NandID==NANDID||NandInfo[i].NandID==0xFFFF)
		{
			Nand_Info_t = &NandInfo[i];
			break;
		}
	}
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
		#if debug
		printf("*****>>>>>Nand erased block No. %d\n OK",block);
		#endif
		return 0;
	}
	else
	{
		printf("Nand erase block No. %d\n failed!\n",block);
		return -1;
	}
}

#ifndef CONFIG_YAFFS_NO_YAFFS1
/************************************************
Write data to blockpage;
blockpage:
data:	the first 512bytes
spare:	the next 16bytes
*************************************************/
int nand_flash_program_buf(int blockpage, unsigned char *data, unsigned char *spare)
{
        nand_page_info_t *info;
      	yaffs_Spare *spare_t=(yaffs_Spare *)spare;
        info = (nand_page_info_t *)spare;

        if(data)
        {
                if(!spare)
                {
                        printf("yaffs try to write data only, it's invalid!\n");
                        return -1;
                }
		if(spare_t->blockStatus!=0xff)
			printf("Nand writing:pageStatus=%x,blockStatus=%x\n",spare_t->pageStatus,spare_t->blockStatus);

                if(jz_nand_write_page(0, blockpage, data, info)!=0)
                {
                        printf("Nand program failed!\n");
                        return -1;
                }
                else
                {
                        #if debug
                        printf("Nand program page No. %d ok\n",blockpage);
                        #endif
                }
        }

        else if(spare)
        {
        	if(jz_nand_write_page_info(0, blockpage, info)!=0)
                {
                        printf("Nand write oob failed!\n");
                        return -1;
                }
                else
                {
                        #if debug
                	printf("Nand write oob of page No. %d ok\n",blockpage);
                        #endif
                }
		if(spare_t->blockStatus!=0xff)
			printf("Nand oob writing:pageStatus=%x,blockStatus=%x\n",spare_t->pageStatus,spare_t->blockStatus);
        }

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
        unsigned char buf[YAFFS_BYTES_PER_SPARE];
        unsigned char dbuf[YAFFS_BYTES_PER_CHUNK];

        nand_page_info_t *info=(nand_page_info_t *)buf;
        yaffs_Spare *spare_t=(yaffs_Spare *)spare;

        memset(buf,0xff,YAFFS_BYTES_PER_SPARE);

        if(data)
        {
                memset(dbuf,0xff,YAFFS_BYTES_PER_CHUNK); //512
                if(jz_nand_read_page (0, blockpage, dbuf, info)!=0)
                {
                        printf("Nand read failed!\n");
                        return -1;
                }
                else
                {
                        #if debug
                        printf("Nand read page No. %d ok\n",blockpage);
                        #endif
                }

                memcpy((void *)data,(void *)dbuf,YAFFS_BYTES_PER_CHUNK);        //512
                if(spare)
                {
                        memcpy((void *)spare,(void *)info,YAFFS_BYTES_PER_SPARE);
                        return 0;
                }
        }

        if(spare)
        {
                if(jz_nand_read_page_info(0, blockpage,info)!=0)
                {
                        printf("Nand read oob failed!\n");
                        return -1;
                }
                else
                {
                        #if debug
                        printf("Nand read oob of page No. %d ok\n",blockpage);
                        #endif
                }

                memcpy((void *)spare,(void *)info, YAFFS_BYTES_PER_SPARE);
		if(spare_t->blockStatus!=0xff)
			printf("Nand reading:pageStatus=%x,blockStatus=%x\n",spare_t->pageStatus,spare_t->blockStatus);
        }
        return 0;
}


/*
   update ucos.bin
   program ucos.bin on nand flash , begin offset block
   unsigned char *buf: data;
   int offset: offset blocks;
   size:        the size of data;

*/

#define ucos_partition_size (1024*1024*2)
int update_ucos(unsigned char *buf, int offset, int size)
{
        int write_blocks = size/YAFFS_BYTES_PER_BLOCK + ((size%YAFFS_BYTES_PER_BLOCK)!=0?1:0);
        unsigned char *p=buf;
        unsigned char pbuf[YAFFS_BYTES_PER_SPARE];
        unsigned char data[YAFFS_BYTES_PER_CHUNK];
        nand_page_info_t *info=pbuf;
        yaffs_Spare *spare_t=(yaffs_Spare *)pbuf;
        int block, cnt = size;
        int j;

//        printf("------update ucos. ucos size=%d,write_blocks=%d,\n",size,write_blocks);
	/* enable hard ecc */
	jz_nand_ecc(1);

        for(block=offset; block<offset+write_blocks; block++)
        {
		jz_nand_read_page_info(0, block*YAFFS_CHUNKS_PER_BLOCK, info);

		if(spare_t->blockStatus!=0xff)
		{
			/* skip the bad block */
			printf("Skip the bad block No.%d\n",block);
			write_blocks++;
			if(write_blocks>(ucos_partition_size/YAFFS_BYTES_PER_BLOCK))
				return -1;	
			continue;
		}
	//	printf("block %d is ok\n",block);
                jz_nand_erase_block(0,block);
                for(j=0; j<YAFFS_CHUNKS_PER_BLOCK; j++)
                {
                        if(cnt>=YAFFS_BYTES_PER_CHUNK)
                        {
                                jz_nand_write_page(0,block*YAFFS_CHUNKS_PER_BLOCK+j, p, info);
                        }
                        else if(cnt>0)
                        {
				memset(data,0xff,YAFFS_BYTES_PER_CHUNK);
                                memcpy(data,p,cnt);
                                jz_nand_write_page(0,block*YAFFS_CHUNKS_PER_BLOCK+j, data, info);
				jz_nand_ecc(0);
                                return 0;
                        }
                        else
                        {
				jz_nand_ecc(0);
                                return 0;
                        }
                        p+=YAFFS_BYTES_PER_CHUNK;
                        cnt-=YAFFS_BYTES_PER_CHUNK;
                }
        }

	jz_nand_ecc(0);
	return -1;
}
#else	 /* for 2k large page */

void translate(nand_page_info_t *info,yaffs_PackedTags2 *spare_t)
{
        memcpy(&info->yaffs_PackedTags2[0], &spare_t->t.sequenceNumber,4);
        memcpy(&info->yaffs_PackedTags2[4], &spare_t->t.objectId,4);
        memcpy(&info->yaffs_PackedTags2[8], &spare_t->t.chunkId, 4);
        memcpy(&info->yaffs_PackedTags2[12], &spare_t->t.byteCount, 4);

	info->yaffs_PackedTags2[16]=(spare_t->ecc.colParity);

        memcpy(&info->yaffs_PackedTags2[17], &spare_t->ecc.lineParity, 4);
        memcpy(&info->yaffs_PackedTags2[21], &spare_t->ecc.lineParityPrime, 4);

}

void detranslate(yaffs_PackedTags2 *spare_t,nand_page_info_t *info)
{
        memcpy(&spare_t->t.sequenceNumber, &info->yaffs_PackedTags2[0],4);
        memcpy(&spare_t->t.objectId, &info->yaffs_PackedTags2[4], 4);
        memcpy(&spare_t->t.chunkId, &info->yaffs_PackedTags2[8], 4);
        memcpy(&spare_t->t.byteCount, &info->yaffs_PackedTags2[12], 4);

        spare_t->ecc.colParity = info->yaffs_PackedTags2[16];

        memcpy(&spare_t->ecc.lineParity, &info->yaffs_PackedTags2[17], 4);
        memcpy(&spare_t->ecc.lineParityPrime, &info->yaffs_PackedTags2[21], 4);
}

/************************************************
Write data to blockpage;
blockpage:
data:	the first 2048bytes
spare:	the next 25bytes
*************************************************/
int nand_flash_program_buf(int blockpage, unsigned char *data, unsigned char *spare)
{
	int i;
	unsigned char buf[64];
	nand_page_info_t *info=(nand_page_info_t *)buf;
	yaffs_PackedTags2 *spare_t=(yaffs_PackedTags2 *)spare;

	memset(buf,0xff,64);

	if(data)
	{
		if(spare)
		{
			translate(info,spare_t);
		}
		else
		{	
			printf("yaffs try to write data only, it's invalid!\n");
			return -1;
		}

		if(jz_nand_write_page(0, blockpage, data, info)!=0)
		{
			printf("Nand program failed!\n");
			return -1;
		}
		else
		{
			#if debug
			printf("Nand program page No. %d ok\n",blockpage);
			#endif
		}
	}

	else if(spare)
	{
		if(jz_nand_read_page_info(0, blockpage,info)!=0)
		{
			printf("Nand Read oob failed!\n");
			return -1;
		}
		else
		{
			translate(info,spare_t);
			if(jz_nand_write_page_info(0, blockpage, info)!=0)		
			{
				printf("Nand write oob failed!\n");
				return -1;
			}	
			else
			{
				#if debug
				printf("Nand write oob of page No. %d ok\n",blockpage);
				#endif
			}
		}
	}
	return 0;
}



/************************************************
Read data to blockpage;
blockpage:
data:	the first 2048 bytes
spare:	the next 25 bytes
*************************************************/

int nand_flash_read_buf(int blockpage, unsigned char *data,unsigned char *spare)
{
	unsigned char buf[64];
	unsigned char dbuf[CONFIG_SSFDC_NAND_PAGE_SIZE];

	nand_page_info_t *info=(nand_page_info_t *)buf;
	yaffs_PackedTags2 *spare_t=(yaffs_PackedTags2 *)spare;

	memset(buf,0xff,64);

	if(data)
	{
		memset(dbuf,0xff,CONFIG_SSFDC_NAND_PAGE_SIZE);	//2048
		if(jz_nand_read_page (0, blockpage, dbuf, info)!=0)		
		{
			printf("Nand read failed!\n");
			return -1;
		}
		else	
		{		
			#if debug
			printf("Nand read page No. %d ok\n",blockpage);
			#endif
		}

		memcpy((void *)data,(void *)dbuf,CONFIG_SSFDC_NAND_PAGE_SIZE);	//2048
		if(spare) 
		{
			detranslate(spare_t,info);
			return 0;
		}
	}

	if(spare)
	{
		if(jz_nand_read_page_info(0, blockpage,info)!=0)		
		{
			printf("Nand read oob failed!\n");
			return -1;
		}	
		else
		{
			#if debug
			printf("Nand read oob of page No. %d ok\n",blockpage);
			#endif
		}

		detranslate(spare_t,info);
	}

	return 0;
}

int update_ucos(unsigned char *buf, int offset, int size)
{

}
#endif 

/* yaffs2 mark bad block */
int nand_flash_mark_badblock(int blockNo)
{
	unsigned char retval=0;
	unsigned char buf[64];
        nand_page_info_t *info=(nand_page_info_t *)buf;

//	printf("the page position of bad block =%d\n",Nand_Info_t->PosBlock);
	jz_nand_read_page_info(0, blockNo * YAFFS_CHUNKS_PER_BLOCK + Nand_Info_t->PosBlock, info);
	info->block_status=0;	/* zero means mark the block to be bad */
	retval=jz_nand_write_page_info(0, blockNo * YAFFS_CHUNKS_PER_BLOCK + Nand_Info_t->PosBlock, info);
	if(retval!=0)
		printf("Nand Mark bad block failed\n");

	return retval;
}

/* yaffs2 query block status */
int nand_flash_query_block(int blockNo)
{
	unsigned char buf[64];
        nand_page_info_t *info=(nand_page_info_t *)buf;

//	printf("the page position of bad block =%d\n",Nand_Info_t->PosBlock);
	jz_nand_read_page_info(0, blockNo * YAFFS_CHUNKS_PER_BLOCK + Nand_Info_t->PosBlock, info);
//	printf("Nand query %dth block status=%x\n",blockNo,info->block_status);
	/*	info->block_status != 0xFF  mean bad block 	*/
#ifdef YAFFS1
	return info->block_status; /* return 0xFF mean good block */
#else
	return info->block_status - 0xFF; /* return 0 mean good block */
#endif
}


