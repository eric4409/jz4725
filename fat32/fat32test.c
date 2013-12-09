#if DEVTYPE == 2
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "ff.h"
#include "diskio.h"

#define ATA             0
#define MMC             1
#define USB             2

static FATFS file_system;

int fatfile_Setup(void)
{
	if(MMC_DetectStatus()!=0)
		return -1;
	FRESULT err;
	err = f_mount(1, &file_system);
	if(err != FR_OK)
		return -1;

	err = f_mount(2, &file_system);
	if(err != FR_OK)

//	err = f_mkfs(0, 0, 512);
//	ASSERT(err == FR_OK);
	
//	int space=0;
//	f_getfree("1:root", &space,&file_system);
//	printf("%s free space:%d\n",__func__,space);
	return 0;
}

int fatfile_TearDown(void)
{
	FRESULT err;
	err = f_mount(1, 0);

	if(err != FR_OK)
		return -1;

	err = f_mount(2, 0);

	if(err != FR_OK)
		return -1;
	return 0;
}

#if 0
int fatfile_testRun(void)
{
	FRESULT fat_err;
	FIL file_handler;
	const int SIZE = 10;
	int write[SIZE], read[SIZE];
	unsigned int count;
	int i;

//	f_mkdir("1:newdir");
	fat_err = f_open(&file_handler, "1:foo.txt", FA_READ|FA_WRITE | FA_CREATE_ALWAYS);

	printf(" f_open id:%d, err:%d\n",file_handler.id, fat_err);

	for (i = 0; i < SIZE; ++i)
	{
		write[i] = i;
		fat_err=f_write(&file_handler, &i, sizeof(int), &count);
		printf(" f_write count:%d, err:%d\n",count, fat_err);
		
	} 
	f_sync(&file_handler);

	f_lseek (&file_handler, 0);
	/* test error function */

	for (i = 0; i < SIZE; ++ i)
	{
		f_read(&file_handler, &read[i], sizeof(int), &count);
		/* check for correctness */
		printf("read[%d]:%d\n",i, read[i]);
		if(read[i] != write[i])
			printf("ERROR !!!\n");
	}
//	int tmp;
//	fat_err=f_read(&file_handler, &tmp, sizeof(int), &count);
//
//	printf("tmp:%d, count:%d err:%d\n", tmp, count, fat_err);
//	if (count < sizeof(int))
		//ASSERT(f_error(&file_handler.id) == FR_DENIED);
//		printf("fread error\n");
//	f_truncate(&file_handler.id);


	int err = 0;
	err = f_close(&file_handler);

#if 0
	fat_err = f_open(&file_handler, "1:foo.txt", FA_READ);
	ASSERT(fat_err == FR_OK);

	for (i = 0; i < SIZE; ++ i)
	{
		f_read(&file_handler, &read[i], sizeof(int), &count);
		ASSERT(count == sizeof(int));
		/* check for correctness */
		printf("read[%d]:%d\n",i, read[i]);
		if(read[i] != write[i])
			printf("ERROR !!!\n");
	}
#endif
	/* test kfile_seek() */
//	ASSERT(f_seek(&file_handler.id, -(sizeof(int) * SIZE * 2), KSM_SEEK_CUR) == 0);
//	ASSERT(f_seek(&file_handler.id, sizeof(int), KSM_SEEK_END) == EOF);


//	f_open(&file_handler, "MMC:foo.txt", FA_READ | FA_WRITE);
//	f_seek(&file_handler.id, sizeof(int));
//	ASSERT(f_seek(&file_handler.id, -SIZE, KSM_SEEK_SET) == 0);

	return 0;
}


#define CNT	200

int fatfile_test(void)
{
	FRESULT fat_err;
	FIL file_handler;
	FILINFO finfo;

       uint8_t Name[20]="1:testfat.txt";
#define BUFSIZE (1024*512)
        int i;
        uint8_t buffer[BUFSIZE];
        uint8_t buf[BUFSIZE];
	int count;
	int fsize=0;
	int readok=0;


//        strcpy(Name, "HELP__CNPDF");


	fat_err = f_open(&file_handler, Name, FA_READ);
	if(fat_err!=0)
	{
		printf("fat error\n");
		return -1;
	}

	if( f_stat(Name, &finfo) ==0)
		fsize = finfo.fsize;
	printf("%s size:%d\n", Name,fsize);

	memset(buffer, 0, BUFSIZE);
	fat_err=f_read(&file_handler, buffer, fsize, &count);
	if( (fat_err==0)&&(fsize==count) )
	{
		printf("read %s ok\n",Name);
		readok=1;
	}
	else
		printf("read %s failed\n",Name);

	f_close(&file_handler);

//	printf("%s\n",buffer);

	if((readok==0) && (fsize<=0) )
		return;
	
	printf("comparing %d files ...\n",CNT);
	for(i=0;i<CNT; i++)
	{
		sprintf(Name,"1:fat%03d.txt",i);
		fat_err = f_open(&file_handler, Name, FA_READ);
		if(fat_err!=0)
		{
			continue;
		}

		if( f_stat(Name, &finfo) ==0)
			fsize = finfo.fsize;
		//printf("%s size:%d\n", Name,fsize);

		count=0;
		memset(buf, 0, BUFSIZE);
		fat_err=f_read(&file_handler, buf, fsize, &count);
		if( (fat_err==0)&&(fsize==count) )
		{
		//	printf("read %s ok\n",Name);
			if(memcmp(buf, buffer, fsize)==0)
				printf("%s(fsize:%d) compared ok\n",Name,count);
			else
				printf("%s(fsize:%d) compared failed---------------\n",Name,count);
		}
		else
			printf("read %s(fsize:%d) failed\n",Name,count);

		f_close(&file_handler);
	}

	if((readok==0) && (fsize<=0) )
		return;
	printf("writing %d files ...\n",CNT);
	for(i=0;i<CNT; i++)
	{
		count=0;

		//strcpy(Name, "1:copyfat.txt");
		sprintf(Name,"1:fat%03d.txt",i);
		//	f_mkdir("1:newdir");
		f_unlink(Name);
		fat_err = f_open(&file_handler, Name, FA_WRITE|FA_CREATE_NEW);	
		if(fat_err!=0)
		{
			printf("%s open error\n",Name);
			continue;
		}

		fat_err=f_write(&file_handler, buffer, fsize, &count);
		if( (fat_err==0)&&(fsize==count) )
			printf("write %s(fsize:%d) ok\n",Name,count);
		else
			printf("write %s(fsize:%d) failed---------------------\n",Name,count); 

		f_sync(&file_handler);
		f_close(&file_handler);
	
	}
}


int fatfile_pic(void)
{
	FRESULT fat_err;
	FIL file_handler;
	FILINFO finfo;

       uint8_t Name[20]="1:testfat.jpg";
#define BUFSIZE (1024*512)
        int i;
        uint8_t buffer[BUFSIZE];
        uint8_t buf[BUFSIZE];
	int count;
	int fsize=0;
	int readok=0;


//        strcpy(Name, "HELP__CNPDF");


	fat_err = f_open(&file_handler, Name, FA_READ);
	if(fat_err!=0)
	{
		printf("fat error\n");
		return -1;
	}

	if( f_stat(Name, &finfo) ==0)
		fsize = finfo.fsize;
	printf("%s size:%d\n", Name,fsize);

	memset(buffer, 0, BUFSIZE);
	fat_err=f_read(&file_handler, buffer, fsize, &count);
	if( (fat_err==0)&&(fsize==count) )
	{
		printf("read %s ok\n",Name);
		readok=1;
	}
	else
		printf("read %s failed\n",Name);

	f_close(&file_handler);

//	printf("%s\n",buffer);
	if((readok==0) && (fsize<=0) )
		return;

	printf("comparing %d files ...\n",CNT);
	for(i=0;i<CNT; i++)
	{
		sprintf(Name,"1:vfat%03d.jpg",i);
		fat_err = f_open(&file_handler, Name, FA_READ);
		if(fat_err!=0)
		{
			continue;
		}

		if( f_stat(Name, &finfo) ==0)
			fsize = finfo.fsize;
		//printf("%s size:%d\n", Name,fsize);

		count=0;
		memset(buf, 0, BUFSIZE);
		fat_err=f_read(&file_handler, buf, fsize, &count);
		if( (fat_err==0)&&(fsize==count) )
		{
		//	printf("read %s ok\n",Name);
			if(memcmp(buf, buffer, fsize)==0)
				printf("%s(fsize:%d) compared ok\n",Name,count);
			else
				printf("%s(fsize:%d) compared failed---------------\n",Name,count);
		}
		else
			printf("read %s(fsize:%d) failed\n",Name,count);

		f_close(&file_handler);
	}

	if((readok==0) && (fsize<=0) )
		return;
	printf("writing %d files ...\n",CNT);
	for(i=0;i<CNT; i++)
	{
		count=0;

		//strcpy(Name, "1:copyfat.txt");
		sprintf(Name,"1:vfat%03d.jpg",i);
		//	f_mkdir("1:newdir");
		f_unlink(Name);
		fat_err = f_open(&file_handler, Name, FA_WRITE|FA_CREATE_NEW);	
		if(fat_err!=0)
		{
			printf("%s open error\n",Name);
			continue;
		}

		fat_err=f_write(&file_handler, buffer, fsize, &count);
		if( (fat_err==0)&&(fsize==count) )
			printf("write %s(fsize:%d) ok\n",Name,count);
		else
			printf("write %s(fsize:%d) failed---------------------\n",Name,count); 

		f_sync(&file_handler);
		f_close(&file_handler);
	
	}
}

int fatfile_dir(void)
{
        FRESULT fat_err;
        FIL file_handler;
        FILINFO finfo;
       uint8_t Name[20]="1:testfat.jpg";
#define BUFSIZE (1024*512)
        int i;
        uint8_t buffer[BUFSIZE];
        uint8_t buf[BUFSIZE];
	int count;
	int fsize=0;
	int readok=0;


//        strcpy(Name, "HELP__CNPDF");


	fat_err = f_open(&file_handler, Name, FA_READ);
	if(fat_err!=0)
	{
		printf("fat error\n");
		return -1;
	}

	if( f_stat(Name, &finfo) ==0)
		fsize = finfo.fsize;
	printf("%s size:%d\n", Name,fsize);

	memset(buffer, 0, BUFSIZE);
	fat_err=f_read(&file_handler, buffer, fsize, &count);
	if( (fat_err==0)&&(fsize==count) )
	{
		printf("read %s ok\n",Name);
		readok=1;
	}
	else
		printf("read %s failed\n",Name);

	f_close(&file_handler);

//	printf("%s\n",buffer);
	if((readok==0) && (fsize<=0) )
		return;

	f_mkdir("1:newdir");

	strcpy(Name,"1:newdir/testfat.jpg");
	f_unlink(Name);
	fat_err = f_open(&file_handler, Name, FA_WRITE|FA_CREATE_NEW);
	if(fat_err!=0)
	{
		printf("%s open error\n",Name);
		return;
	}

	fat_err=f_write(&file_handler, buffer, fsize, &count);
	if( (fat_err==0)&&(fsize==count) )
		printf("write %s(fsize:%d) ok\n",Name,count);
	else
		printf("write %s(fsize:%d) failed---------------------\n",Name,count);

	f_sync(&file_handler);
	f_close(&file_handler);
	
}

int fatfile_attlog(void)
{
        FRESULT fat_err;
        FIL file_handler;
        FILINFO finfo;

       uint8_t Name[20]="1:fatatt.dat";
#define BUFSIZE (1024)
        int i;
        uint8_t buffer[BUFSIZE]="2010-02-05 15:48:30\n";
        int count;
        int fsize=0;
        int readok=0;


//        strcpy(Name, "HELP__CNPDF");


	f_unlink(Name);
        fat_err = f_open(&file_handler, Name, FA_WRITE|FA_CREATE_NEW);
        if(fat_err!=0)
        {
                printf("fat error\n");
                return -1;
        }
	
	for(i=0; i<100; i++)
	{
		sprintf(buffer,"2010-02-05 15:48:%02d\n",i%60);
		fsize=strlen(buffer);
		fat_err=f_write(&file_handler, buffer, fsize, &count);
		if( (fat_err!=0)&&(fsize!=count) )
			printf("write %s(fsize:%d) failed---------------------\n",Name,count);
	}
	
	printf("write %s ok\n",Name);
        f_sync(&file_handler);
        f_close(&file_handler);
	

}

int fatfile(void)
{
	fatfile_Setup();
	fatfile_attlog();
	fatfile_dir();
	//fatfile_testRun();
	fatfile_test();
	fatfile_pic();
	fatfile_TearDown();
}
#endif
#endif
