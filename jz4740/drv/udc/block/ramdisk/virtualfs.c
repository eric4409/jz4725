#include <stdio.h>
#include "options.h"
#include "arca.h"
#define VFAT
#ifdef VFAT
typedef struct _vf{
	const char *name;
	const char *ext;
	const char *fname;
	int  fd;
	int startCluster;
	int size;
}VFEntry;

#if 0
#define VF_ENTRY_COUNT	3
#else
#define VF_ENTRY_COUNT	2
#endif
#define SECTOR_SIZE	512
#define SIZE_DUMMY	(SECTOR_SIZE*3)		//the size of dummy is 1536 bytes
#define SIZE_MANUAL	(SECTOR_SIZE*1224)	//the size of manual is 626314 bytes

#if 0
char yaffs_file_name[][50] = {
	{ },
	{"/mnt/mtdblock/help_cn.pdf"},
	{"/mnt/mtdblock/guide_cn.pdf"},
};
#else
char yaffs_file_name[][50] = {
	{ },
	{"/mnt/mtdblock/help_cn.pdf"},
};
#endif

static char nameh[24];

VFEntry fEntry[VF_ENTRY_COUNT];

int loadVFile(int index, const char *name, const char *ext, const char *fname)
{
	struct yaffs_stat s;
	
	memset(&fEntry[index], 0, sizeof(VFEntry));
	/* the first file is the dummy file used for communication only */

	fEntry[index].fd = open(fname, O_RDWR, S_IREAD|S_IWRITE);
	if(fEntry[index].fd<0)
		printf("Open %s error\n",name);
	else
	{
		yaffs_stat(fname, &s);
		fEntry[index].size = s.yst_size;
	}
		
	fEntry[index].name = name;
	fEntry[index].ext = ext;
	fEntry[index].fname = fname;

	if(index==0)
		fEntry[index].size = SIZE_DUMMY;

//	if(index==1)
//		fEntry[index].size = SIZE_MANUAL;

	printf("loadFile: name=%s, extname=%s,filesize=%d\n",
		fEntry[index].name,
		fEntry[index].ext,
		fEntry[index].size);
//	close(fEntry[index].fd);
	return index;
}

int createFAT(unsigned char *fat, unsigned char *fdt);
int initVFEntry(unsigned char *fat, unsigned char *fdt)
{
#if 0
	loadVFile(0, "TEMPLATE","DAT", "/flash/template.dat");
	loadVFile(1, "USER    ","DAT", "/flash/user.dat");
	loadVFile(2, "ATTLOG  ","DAT", "/flash/attlog.dat");
	loadVFile(3, "OPTIONS ","CFG", "/mnt/mtdblock/options.cfg");
	loadVFile(4, "FFISO   ","DAT", "/mnt/mtdblock/ffiso.dat");
	loadVFile(5, "S_7     ","WAV", "/mnt/mtdblock/S_7.wav");
#endif
	char ctype = GetLangFileType(gOptions.Language);
#if 0
	yaffs_file_name[1][strlen(yaffs_file_name[1])-6]=tolower(ctype);
	yaffs_file_name[2][strlen(yaffs_file_name[2])-6]=tolower(ctype);
	loadVFile(0, "DUMMY   ", "   ", &yaffs_file_name[0][0]);
        sprintf(name, "HELP_%cN ", ctype);
	loadVFile(1, name, "PDF", &yaffs_file_name[1][0]);
	sprintf(name, "GUIDE_%cN ", ctype);
	loadVFile(2, name, "PDF", &yaffs_file_name[2][0]);
#else
	yaffs_file_name[1][strlen(yaffs_file_name[1])-6]=tolower(ctype);
	loadVFile(0, "DUMMY   ", "   ", &yaffs_file_name[0][0]);
        sprintf(nameh, "HELP_%cN ", ctype);
	loadVFile(1, nameh, "PDF", &yaffs_file_name[1][0]);
#endif	
	createFAT(fat, fdt);
	return 0;
}

//*************************************************************************
//* 32字节记录结构的时间结构（占用2两字节） 
//*------------------------------------------------------------------------
typedef struct _Time 
{ 
	unsigned char Seconds : 5; //* 2秒为单位 
	unsigned char Minute : 6; //* 分 
	unsigned char Hour : 5; //* 小时 
}FAT_TIME; 
//*************************************************************************
//* 32字节记录结构的日期结构（占用2两字节） 
//*------------------------------------------------------------------------
 
typedef struct _Date 
{ 
	unsigned char Day : 5; //* 
	unsigned char Month : 4; //* 
	unsigned char Year : 7; //* 
}FAT_DATE; 
#define FAT_NAME_LEN 8
#define FAT_EXT_LEN 3
typedef struct _FAT32DirEntry 
{ 
	char Name[FAT_NAME_LEN]; //* 文件名 
	char Extension[FAT_EXT_LEN]; //* 文件扩展名 
	unsigned char Attribute; //* 属性字节 
	unsigned char Reserved[8]; //* 保留给NT使用 	//the reserved byte should be 10.
	FAT_TIME CrtTime; //* 文件创建时间 
	FAT_DATE CrtDate; //* 文件创建日期 
	unsigned char Cluster[2]; //* 记录的第一簇的高16位（31..16） 
	unsigned int FileSize; //* 文件长度，以字节为单位（31..00） 
} __attribute__((__packed__)) FAT16_DIR_ENTRY;

void createFDTEntry(FAT16_DIR_ENTRY * fdt, int i)
{
	memset(fdt, 0, sizeof(FAT16_DIR_ENTRY));
	strcpy(fdt->Name, fEntry[i].name);
	strcpy(fdt->Extension, fEntry[i].ext);
	if(i==0)
		fdt->Attribute = 0x22;
	else if(i==1 || i==2)
		fdt->Attribute = 0x21;
	else
		fdt->Attribute = 0x20;
	fdt->CrtTime.Seconds = 0;
	fdt->CrtTime.Minute = 0;
	fdt->CrtTime.Hour = 18;
	fdt->CrtDate.Day = 24;;
	fdt->CrtDate.Month = 11;;
	fdt->CrtDate.Year = 2008-1980;;
	memcpy(fdt->Reserved, "\x18\xA8\x44\x88\x7B\x39\x7B\x39", 8);
	fdt->FileSize = fEntry[i].size;
//	printf("createEFTEntry:fdt[0] = %x\n",*(unsigned char *)fdt);
	if( *(unsigned char *)fdt==0xE5)
		*(unsigned char *)fdt = 0x05;
/*	printf("createEFTEntry: name=%s, extname=%s,filesize=%d\n",
		fdt->Name,
		fdt->Extension,
		fdt->FileSize); */
}

int SET_FAT_ENTRY(unsigned char *fat, int cluster, int next)
{
	if(cluster%2==0)
	{
		fat[cluster*3/2]=next&0xFF;
		fat[cluster*3/2+1]=((next>>8)&0x0F)|(fat[cluster*3/2+1]&0xF0);
	}
	else
	{
		fat[cluster*3/2]=(fat[cluster*3/2]&0x0f)|((next<<4)&0xf0);
		fat[cluster*3/2+1]=next>>4;
	}
	return 0;
}

int createFAT(unsigned char *fat, unsigned char *fdt)
{
	int i, startCluster=2;

	/* Create fdt */
	//0x4F, 0x50, 0x54, 0x49, 0x4F, 0x4E, 0x53, 0x20, 0x43, 0x46, 0x47, 0x20, 0x18, 0x8B, 0x6E, 0xA8,
	//0x7A, 0x39, 0x7A, 0x39, 0x00, 0x00, 0x4C, 0xA8, 0x7A, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	for(i=0;i<VF_ENTRY_COUNT;i++){
		FAT16_DIR_ENTRY fdt_entry;
		createFDTEntry(&fdt_entry, i);
		memcpy(fdt+i*32, &fdt_entry, 32);
	}

	/* Create fat */
	//  00 00 00 FF 4F 00 05 60 00 07 80 00 FF 0F 00 00   ....O..`..€.....
	memset(fat, 0, 128);
	SET_FAT_ENTRY(fat, 0, 0);
	SET_FAT_ENTRY(fat, 1, 0);
	for(i=0;i<VF_ENTRY_COUNT;i++)
	{
		int j,cluster=startCluster;
		fEntry[i].startCluster=startCluster;
		startCluster=(fEntry[i].size+511)/512 + startCluster;
	//	printf("startcluster: %d. filesize=%d\n",fEntry[i].startCluster,fEntry[i].size);
		if(fEntry[i].size>0)
		{
			*(fdt+32*i+0x1A)=fEntry[i].startCluster & 0xff; //The first byte of cluster
			*(fdt+32*i+0x1B)=fEntry[i].startCluster >> 8;	//the second byte of cluster
			for(j=0; j<startCluster-fEntry[i].startCluster-1; j++)
			{
				SET_FAT_ENTRY(fat, cluster, cluster+1);
				cluster++;
			}
			SET_FAT_ENTRY(fat, cluster, 0xFFF);
		}
	}
//	DumpData(fdt, VF_ENTRY_COUNT*sizeof(FAT16_DIR_ENTRY));
//	DumpData(fat, 512);
	return 0;
}

int searchFDByCluster(int cluster, int *start)
{
	int StartLogicluster=cluster-64+2;
	int i;
	for(i=0;i<VF_ENTRY_COUNT;i++)
	{
		
		if(fEntry[i].size>0 && fEntry[i].startCluster>StartLogicluster)
		{
		//	printf("find entry: %s,%d\n", fEntry[i-1].name, i-1); 
			*start=(StartLogicluster-fEntry[i-1].startCluster)*512;
			return i-1;
		}
	}
	if(StartLogicluster*512<=fEntry[VF_ENTRY_COUNT-1].startCluster*512+fEntry[VF_ENTRY_COUNT-1].size)
	{
	//	printf("find entry: %s,%d\n", fEntry[VF_ENTRY_COUNT-1].name, VF_ENTRY_COUNT-1); 
		*start=(StartLogicluster-fEntry[VF_ENTRY_COUNT-1].startCluster)*512;
		return VF_ENTRY_COUNT-1;
	}
	return -1;
}

void FlushfEntry(unsigned char *fdt, int index)
{
	/* update the file size */
	fEntry[index].size = *(int *)(fdt+index*32+28);
	return;
}

int readVFile(unsigned char *buffer, int cluster, int count)
{
	int start;
	
	int index=searchFDByCluster(cluster, &start);
	if(index>=0)
	{
	//	printf("entry: %s, from: %d\n", fEntry[index].name, start); 
		lseek(fEntry[index].fd, start, SEEK_SET);
		return read(fEntry[index].fd, buffer, count*512);
	}
	else
		printf("readVFile: %s not found file\n",fEntry[index].name);
	return -1;
}

extern unsigned char disk_fdt[512];
int writeVFile(unsigned char *buffer, int cluster, int count)
{
	int start,index;

	index=searchFDByCluster(cluster, &start);
	FlushfEntry(disk_fdt, index);
	if(index>=0)
	{
		printf("writeVFile: filename %s, from: %d filesize:%d\n", 
								fEntry[index].name, 
								start, 
								fEntry[index].size);
		if(fEntry[index].fd>=0)
		{
			close(fEntry[index].fd);
			fEntry[index].fd = open(&yaffs_file_name[index][0], O_RDWR|O_CREAT|O_TRUNC, 
										S_IREAD|S_IWRITE);
		}	
		/* Please use the new file size from fdt */
		if(write(fEntry[index].fd, buffer, fEntry[index].size)==fEntry[index].size)
			printf("writeVFile: writing file ok\n");
		yaffs_flush(fEntry[index].fd);
	}
	else
		printf("writeVFile: not fond file\n");

	return 0;
}
#endif
