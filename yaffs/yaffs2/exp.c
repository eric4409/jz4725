/***********************************************************************************
Author:			Andy.Wu
Date:			2006.6.8
Description:	For test yaffs
************************************************************************************/

#include "yaffsfs.h"

extern int nand_flash_hwr_init(void);
extern int nand_flash_program_buf(int blockpage, unsigned char *data, unsigned char *spare);
extern int nand_flash_read_buf(int blockpage, unsigned char *data,unsigned char *spare);
#if 0
void ListDir(const char *DirName)
{
	yaffs_DIR *d;
	yaffs_dirent *de;
	struct yaffs_stat s;
	char str[100];
			
	d = yaffs_opendir(DirName);
	
	if(!d)
	{
		printf("opendir failed\n");
	}
	else
	{
		printf("%s\n", DirName);
		while((de = yaffs_readdir(d)) != NULL)
		{
			sprintf(str,"%s/%s",DirName,de->d_name);
			yaffs_lstat(str,&s);
			printf("%20s        %8d     %5d    ", de->d_name, s.st_size, s.st_mode);
			
			switch(s.st_mode & S_IFMT)
			{
				case S_IFREG: printf("data file"); break;
				case S_IFDIR: printf("directory"); break;
				case S_IFLNK: printf("symlink");   break;
				default: printf("unknown"); break;
			}
		
			printf("\n");		
		}
		
		yaffs_closedir(d);
	}
	
	printf("FreeSpace: %d\n\n", yaffs_freespace(DirName));
}
#endif

#define Nfile	50

void yaffstest(void)
{
	int i = 0,j;
	int f = -1,f1=-1;
	int fd[Nfile];
#if 1
	char file[20];
	char buf[2048];
	char buffer[2048];
#endif
	struct yaffs_stat s;
	int ret;
	unsigned int block;
	unsigned int page;
	int size;

#if 1
	unsigned char buffer0[16];
	unsigned char buffer1[2048];
	unsigned char buffer2[64];
	unsigned char buffer3[2048];
	

	//jz_nand_init ();
	nand_flash_hwr_init();
	

//	for(i=129;i<2047;i++)
//		nand_flash_erase_block(i);

for(i=129;i<2047;i++)
//if(1)
{
	nand_flash_erase_block(i);
	block=i;
	printf("block=%d ok\n",block);
	for(j=0;j<32;j++){
//	if(1){

	page=block*32+j;
	memset(buffer0,j,16);
//	if((page%32)==0||(page%32)==1)
		buffer0[5]=0xff;
	memset(buffer1,j+1,512);

	memset(buffer2,0xff,16);
	memset(buffer3,0xff,512);
	

	nand_flash_program_buf(page, buffer1,buffer0);//writeing data
//	nand_flash_program_buf(page, NULL, buffer0);//writeing spare
	if(nand_flash_read_buf(page, buffer3,buffer2)!=0)
		printf("*********read nand failed\n");
	if(nand_flash_read_buf(page, NULL,buffer2)!=0)
		printf("*********read nand oob failed\n");

//	ret=memcmp(buffer1+512+256+128,buffer3+512+256+128,64);
	ret=memcmp(buffer1,buffer3,512);
	if(ret!=0)
	{
		printf("!!!!!Error\n");
		printf("ret=%d\n",ret);
		break;
	}
	ret=memcmp(buffer2,buffer0,16);
	if(ret!=0)
	{
		printf("!!!!!Error\n");
		printf("ret=%d\n \r\n",ret); 
		break;
	}
	}
	
	if(i==2046)	i=128;
}
/*
	for(i=0;i<16;i++)
		printf("***data[%d]=%x,spare[%d]=%x\n",i,buffer3[i],i,buffer2[i]);
	for(i=896+64;i>896;i--)
		printf("*** buffer3[%d]=%x,buffer1[%d]=%x\n",i,buffer3[i],i,buffer1[i]); 
*/
#else
#if 1
	yaffs_StartUp();
	yaffs_mount("/mnt");
//	yaffs_mount("/d");
//	return;
//	ListDir("/mnt");
	yaffs_mkdir("/mnt/mtdblock", 0);
	yaffs_mkdir("/mnt/mtdblock/font", 0);
//	yaffs_mkdir("/c/data1", 0);
	ListDir("/mnt/mtdblock");
	ListDir("/mnt/mtdblock/font");

#endif
#if 0
	printf("testing serial\n\n");
	Updatefile();
	yaffs_sync("/mnt");
//	ReadOptions("/mnt/mtdblock/test1");
//	ReadOptions("/mnt/mtdblock/options.cfg");
//	ReadOptions("/mnt/mtdblock/font/LANGUAGE.E");
	ListDir("/mnt/mtdblock");
	ListDir("/mnt/mtdblock/font");

	return;
#else
	/* testing memory */
	return;
#endif


#if 0
//	f = yaffs_open("/mnt/mtdblock/bigfile", O_CREAT | O_RDWR , S_IREAD | S_IWRITE);
	f = yaffs_open("/mnt/mtdblock/options.cfg", O_CREAT | O_RDWR , S_IREAD | S_IWRITE);
//	f = yaffs_open("/mnt/mtdblock/options.cfg", O_RDWR , S_IREAD | S_IWRITE);
	if(f<0)
		printf("Yaffs Open File Failed\n");
	#if 1
	for(i=0;i<12000;i++)
//	for(i=0;i<2;i++)
	{
		
		yaffs_lseek(f,0,SEEK_SET);
		yaffs_read(f,buffer,50);
		memset(buffer,i%255,2048);
		size=yaffs_lseek(f,0,SEEK_END);
		ret=yaffs_write(f,buffer,2048);
	//	ret=yaffs_write(f,buffer,100);
	//	printf("the result write  = %d,No.%d\n",ret,i);
		yaffs_lseek(f,0,SEEK_SET);
		yaffs_read(f,buffer,50);
	}
	#endif
	yaffs_lseek(f,0,SEEK_SET);
	size=yaffs_lseek(f,0,SEEK_END);

	yaffs_close(f);

	printf("the length of file =%d,it should be %d\n",size,i*2048);
	ListDir("/mnt/mtdblock");
	yaffs_sync("/mnt");
//	yaffs_unmount("/mnt");
//	yaffs_unlink("/c/data1/bigfile");
//	ListDir("/c/data1");
	
#else
	for(j=0;;j++){
	for(i=0;i<Nfile ;i++)
	{
		sprintf(file,"/mnt/mtdblock/file%d",i);
		printf("will open file %s\n",file);		
	//	fd[i] = yaffs_open(file, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
		fd[i] = yaffs_open(file, O_CREAT | O_RDWR , S_IREAD | S_IWRITE);
	//	printf("**************fd[%d]=%d\n",i,fd[i]);

		memset(buffer,0x5b+i,2048);
#if 1

		size=yaffs_lseek(fd[i],0,SEEK_END);
		ret=yaffs_write(fd[i],buffer,2048);
		ret=yaffs_write(fd[i],buffer,2048);
		ret=yaffs_write(fd[i],buffer,2048);
		ret=yaffs_write(fd[i],buffer,2048);
		ret=yaffs_write(fd[i],buffer,2048);
		ret=yaffs_write(fd[i],buffer,2048);
		ret=yaffs_write(fd[i],buffer,2048);
		ret=yaffs_write(fd[i],buffer,2048);
		ret=yaffs_write(fd[i],buffer,2048);
		ret=yaffs_write(fd[i],buffer,2048);
	//	printf("the result of yaffs write file %s = %d\n",file,ret);

/*		for(j=0;j<2;j++)
		{
			printf("buffer[%i]=%x\n",j,buffer[j]);
		}*/

		yaffs_lseek(fd[i],0,SEEK_SET);
#endif
		size=yaffs_lseek(fd[i],0,SEEK_END);
		printf("the length of file %s=%d\n",file,size);
		yaffs_lseek(fd[i],-2048,SEEK_END);
#if 1
		memset(buf,0,2048);
		ret=yaffs_read(fd[i],buf,2048);
	//	printf("the result of yaffs read file %s ==%d\n",file,ret);
/*
		for(j=0;j<2;j++)
		{
			printf("buf[%i]=%x\n",j,buf[j]);
		} */

		ret=memcmp(buffer,buf,2048);
	//	printf("the result of compare read*write file %s = %d\n",file,ret);
		printf("the result of compare read*write file = %d\n",ret);
	//	ListDir("/mnt/mtdblock");
#endif
	

		yaffs_close(fd[i]);
		//yaffs_sync("/mnt");
	}
	ListDir("/mnt/mtdblock");
		if((j%10)==0)
			for(i=0;i<Nfile;i++)
			{
				sprintf(file,"/mnt/mtdblock/file%d",i);
				yaffs_unlink(file);
			}
	}
#endif
#endif
}
