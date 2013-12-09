/*************************************************
                                           
 ZEM 200                                          
                                                    
 options.c all function for options                               
                                                      
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
                                                      
 Author: Richard Chen
	 
 Modified by David Lee for JFFS2 FS 2004.12.12	 
 
 $Log: options.c,v $
 Revision 5.19  2006/03/04 17:30:09  david
 Add multi-language function

 Revision 5.18  2005/12/22 08:54:23  david
 Add workcode and PIN2 support

 Revision 5.17  2005/11/06 02:41:34  david
 Fixed RTC Bug(Synchronize time per hour)

 Revision 5.16  2005/09/19 10:01:59  david
 Add AuthServer Function

 Revision 5.15  2005/08/18 07:16:56  david
 Fixed firmware update flash error

 Revision 5.14  2005/08/13 13:26:14  david
 Fixed some minor bugs and Modify schedule bell

 Revision 5.13  2005/08/04 15:42:53  david
 Add Wiegand 26 Output&Fixed some minor bug

 Revision 5.12  2005/08/02 16:07:51  david
 Add Mifare function&Duress function

 Revision 5.11  2005/07/14 16:59:53  david
 Add update firmware by SDK and U-disk

 Revision 5.10  2005/07/07 08:09:02  david
 Fixed AuthServer&Add remote register

 Revision 5.9  2005/06/29 20:21:43  david
 Add MultiAuthServer Support

 Revision 5.8  2005/06/16 23:27:51  david
 Add AuthServer function

 Revision 5.7  2005/06/10 17:11:01  david
 support tcp connection

 Revision 5.6  2005/06/02 20:11:12  david
 Fixed SMS bugs and Add Power Button Control function

 Revision 5.5  2005/04/27 00:15:37  david
 Fixed Some Bugs

 Revision 5.4  2005/04/24 11:11:26  david
 Add advanced access control function

 Revision 5.3  2005/04/07 17:01:45  davidacc
 Modify to support A&C and 2 row LCD
	 
*************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
//#include <sys/stat.h>
//#include <sys/ioctl.h>
#include <unistd.h>
//#include <fcntl.h>
#include "arca.h"
#include "options.h"
#include "utils.h"
#include "sensor.h"
#include "finger.h"
#include "ccc.h"
#include "mainmenu.h"
#include "lcm.h"
#include "kb.h"
//#include "netspeed.h"
#include "serial.h"
#include "exfun.h"
#include "exvoice.h"
#include <sys/vfs.h>

//options file handle
static int fdOptions = -1;
static int fdLanguage = -1;
static char CurLanguage = ' ';

extern int fd_sensor;

char *DateFormats[]={"YY-MM-DD","YY/MM/DD","YY.MM.DD", "MM-DD-YY","MM/DD/YY","MM.DD.YY","DD-MM-YY","DD/MM/YY","DD.MM.YY","YYYYMMDD"};

int FormatDate(char *buf, int index, int y, int m, int d)
{
	index=index%10;
	if(index==9)
		sprintf(buf, "%04d%02d%02d", y,m,d);
	else
	{
		char ss;
		if(index<3) sprintf(buf, "%02d-%02d-%02d", y%100,m,d);
		else if(index<6) sprintf(buf, "%02d-%02d-%02d", m,d,y%100);
		else sprintf(buf, "%02d-%02d-%02d", d,m,y%100);
		if(index%3==0) ss='-';
		else if(index%3==1) ss='/';
		else ss='.';
		buf[2]=ss;buf[5]=ss;
	}
	return 8;
}

int FormatDate2(char *buf, int index, int m, int d)
{
	index=index%10;
	if(index==9)
		sprintf(buf, "%02d%02d", m,d);
	else
	{
		char ss;
		if(index<3) sprintf(buf, "%02d-%02d", m,d);
		else if(index<6) sprintf(buf, "%02d-%02d", m,d);
		else sprintf(buf, "%02d-%02d", d,m);
		if(index%3==0) ss='-';
		else if(index%3==1) ss='/';
		else ss='.';
		buf[2]=ss;
	}
	return 5;
}
//this function is only used for options.cfg, it is different with ReadOneLine
U32 PackStrBuffer(char *Buffer, const char *name, int size)
{
	char c, *cp, *namep, *valuep,*TheName;
	int i, isname, OriSize;
	char tmp[VALUE_BUFFERLEN];
	int offset=0;
	
	OriSize=size;
	TheName=(char*)malloc(size);
	namep=Buffer;
	valuep=namep;
	cp=Buffer;
	
	while(cp<(Buffer+size)) 
	{
		if(('='==*cp) && (valuep<=namep))
		{
			valuep=cp++;
			offset++;
		}
		else if((('\n'==*cp) || ('\r'==*cp)) && (cp>namep))
		{
			cp++;offset++;
			if (('\n'==*cp) || ('\r'==*cp)){cp++; offset++;}
			i=0;isname=1;
			while(1)
			{	
				c=namep[i];
				if(c=='=')
				{	
					TheName[i]=0;
					if(isname && name[i]) isname=0;
					break;
				}
				else
				{
					TheName[i]=c;
					if(c!=name[i]) isname=0;
				}
				i++;
			}
			if (isname || (LoadStrFromFile(fdOptions, TheName, tmp, TRUE, offset)!=-1)) 
			{ 	//delete this name and value
				memmove(namep,cp,size-(cp-Buffer));
				size-=cp-namep;
				memset(Buffer+size, 0, OriSize-size);
				cp=namep;
			}
			namep=cp;
		}
		else
		{
			cp++;
			offset++;
		}
		if('\0'==*cp) break;
	}
	free(TheName);
	return cp-Buffer;
}

//The strings are of the form name = value.
void CombineNameAndValue(const char *name, const char *value, int SaveTrue, char *processedStr)
{
	sprintf(processedStr, "%s=%s\n", name, value);
}

//support two format XXX=YYY OR "XXX=YYY" for compatible with zem100 language file format.
BOOL ReadOneLine(int fd, char *dest, int *size)
{
       char c;
       
       *size=0;
       while(TRUE)
       {
	       if (read(fd, &c, 1) == 1)
	       {
		       if((c == '\n') || (c == '\r') || (c == '"'))
		       {
			       if(*size==0)
				       continue;
			       else
				       break;
		       }
		       dest[*size] = c;
		       *size = *size + 1;
	       }
	       else 
		       break;
       }
       if (*size > 0)
       {
	       dest[*size] = '\0';
       }
       return(*size > 0); 
}

int GetFileCurPos(int fd)
{
       return lseek(fd, 0, SEEK_CUR);
}
	
void SplitByChar(char *buffer, char *name, char * value, char DeliChar)
{
       int cnt;
       char *p;
    
       p=buffer;
       cnt=0;
       while(*p)
       {
	       if (*p==DeliChar) break;
	       cnt++;
	       p++;
       }
       memcpy(name, buffer, cnt);
       name[cnt]='\0';
       if ((cnt+1)<strlen(buffer))
	       memcpy(value, buffer+cnt+1, strlen(buffer)-cnt-1);
       value[strlen(buffer)-cnt-1]='\0';
}

//return -1 mean can not find string by name	
int LoadStrFromFile(int fd, const char *name, char *value, BOOL ExitSign, int offset) 
{
       char name1[128], value1[VALUE_BUFFERLEN];
       char buffer[VALUE_BUFFERLEN];
       int size;
       int position;
    
       position=-1;
       lseek(fd, offset, SEEK_SET); 
       while(TRUE){
	   if(ReadOneLine(fd, buffer, &size)){
	       SplitByChar(buffer, name1, value1, '=');
	       if(strcmp(name1, name)==0){
		   strcpy(value, value1);
		   position = GetFileCurPos(fd);
		   if (ExitSign) break;
	       }
	   }else
	       break;
       }
       return position;
}

BOOL LoadStr(const char *name, char *value)
{
	return (LoadStrFromFile(fdOptions, name, value, FALSE, 0)!=-1?TRUE:FALSE);
}

void ExecuteActionForOption(const char *name, const char *value)
{
	if (strcmp(name, "RS485On")==0)
	{
		if (gOptions.RS485On) RS485_setmode(FALSE); 
	}
	else if (strcmp(name, "VOLUME")==0)
		SetAudioVol(gOptions.AudioVol);
}

BOOL SaveStr(const char *name, const char *value, int SaveTrue)
{
	char buffer[VALUE_BUFFERLEN]; 
	int len;
	
	len=strlen(value);
	if (LoadStr(name, buffer))
	{
		//the value is the same as old value, then return.
		if (0==strcmp(value, buffer)) return TRUE;
	}
	//check language item whether can be setup or not
	if(!gOptions.MultiLanguage)
	{
		if(strcmp(name, "Language")==0) return TRUE;
	}	
	CombineNameAndValue(name, value, SaveTrue, buffer);	
	len=lseek(fdOptions, 0, SEEK_END);
	if (len>=MAX_OPTION_SIZE)
	{ 
	    ClearOptionItem("NONE");
	    len=lseek(fdOptions, 0, SEEK_END);
	}
	if (len<MAX_OPTION_SIZE)
	    write(fdOptions, buffer, strlen(buffer));
	
	ExecuteActionForOption(name, value);
	yaffs_flush(fdOptions);
	return ((len<MAX_OPTION_SIZE)?TRUE:FALSE);
}

char *GetCardKeyStr(char *Buffer, BYTE *Key)
{
	int i;
	BYTE *tmp=(BYTE *)Buffer;
        memcpy(tmp,Key,6);
        tmp[6]=0;
        for(i=5;i>=0;i--)
                if(tmp[i]==0xff) tmp[i]=0;
        return Buffer;
}

static char ln[40];

int LoadInteger(const char *Name, int DefaultValue)
{
	char tmp[VALUE_BUFFERLEN];
	char *buf;
	int v,n=1,d,c;
	
	buf=tmp;
	if(LoadStr(Name, buf))
	{
		if(*buf)
		{
			if('-'==*buf)
			{
				n=-1;
				buf++;
			}
			v=0;c=0;
			do{
				d=buf[c];
				if(d==0) break;
				if(d<'0' || d>'9')
				{
					return DefaultValue;
				}
				v=v*10+(d-'0');
				c++;
			}while(1);
			if(c)
				return n*v;
		}
	}
	return DefaultValue;
}

int SaveInteger(const char *Name, int Value)
{
	char Buf[20];
	sprintf(Buf,"%d",Value);
	if (SaveStr(Name, Buf, FALSE))
		return 0;
	else
		return 1;
}

TOptions gOptions;

char* SaveOptionItem(char *buf, const char *name, const char *value)
{
	char *p=buf;
	while(*name) *p++=*name++;
	*p++='=';
	while(*value) *p++=*value++;
	*p++=0;
	return p;
}

int SaveDefaultOptions(char *buffer)
{
	char *p=buffer;
	return p-buffer;
}

int InitOptions(void)
{
	char Buffer[80];
	struct statfs s;
	int sel;
	
	GetEnvFilePath("OPTIONPATH", "options.cfg", Buffer);
	fdOptions=open(Buffer, O_RDWR|O_CREAT, S_IREAD|S_IWRITE);
//	printf("fdoptions=%d,path=%s\n",fdOptions,Buffer);

	LoadOptions(&gOptions);
	if(LoadStr("BCIIKeyLayouts", Buffer))
	{
		SetKeyLayouts(Buffer);
	}	
	if(gOptions.AttLogExtendFormat&&!gOptions.IClockFunOn && (statfs("/mnt/mtdblock/data", &s)==-1)) 
	{
		if(!gOptions.IsOnlyRFMachine)
			gOptions.MaxAttLogCount=(gOptions.MaxAttLogCount>3?3:gOptions.MaxAttLogCount);
		else
			gOptions.MaxAttLogCount=(gOptions.MaxAttLogCount>8?8:gOptions.MaxAttLogCount);
        }	
	if(!gOptions.IsOnlyRFMachine)
	{
		if(gOptions.MaxUserCount>100) gOptions.MaxUserCount=100;
	}
        if ((sel=LoadInteger("ChangeVersion",0))!=0)
        {
                SaveInteger("~ZKFPVersion", sel);
                gOptions.ZKFPVersion=sel;
                SaveInteger("ChangeVersion", 0);
        }
	
	if(gOptions.PlayTZVoice) LoadVoiceInfo();
	//ClearOptionItem("NONE");	
	return 1;
}

void TruncOptionAndSaveAs(char *buffer, int size)
{
	char tmp[80];
	
	GetEnvFilePath("OPTIONPATH", "options.cfg", tmp);
	
	if (fdOptions > 0) close(fdOptions); 
    	fdOptions = open(tmp, O_RDWR|O_TRUNC);
	if (buffer!=NULL)
	    write(fdOptions, buffer, size);
	close(fdOptions);
	fdOptions = open(tmp, O_RDWR|O_CREAT);
	//flush the cached data to disk
	yaffs_flush(fdOptions);
	yaffs_sync("/mnt");
}

static char gBuffer[VALUE_BUFFERCACHE+1];
static int gPosition=0;

char *strCache(char *value)
{
       char *p;
       int len;
    
       len=strlen(value);
       p=gBuffer;
       if ((gPosition+len)>=VALUE_BUFFERCACHE) gPosition=0;
       p+=gPosition;
       memcpy(p, value, len+1);
       gPosition+=len+1;
       return p;
}

char *LoadStrOld(const char *name)
{
       char tmp[VALUE_BUFFERLEN];
    
       return (LoadStr(name, tmp)?strCache(tmp):NULL);
}

char* LoadStrByID(int ID)
{	
	char tmp[VALUE_BUFFERLEN];
	char *p;
       
       	ln[0]=CurLanguage;
       	sprintf(ln+1,"/_%d_",ID);

       	if (LoadStrFromFile(fdLanguage, ln, tmp, FALSE, 0) != -1)
		p = strCache(tmp);
	else
		p = NULL;
	if (p==NULL)
	{
                if(ln[1]=='/')
                {
        	       SelectLanguage(LanguageEnglish);
                        ln[0]='E';
       			if (LoadStrFromFile(fdLanguage, ln, tmp, FALSE, 0) != -1)
				p = strCache(tmp);
			else
				p = NULL;
        		SelectLanguage(gOptions.Language);
                }
                if(p==NULL)
		{
       			if (LoadStrFromFile(fdLanguage, ln+2, tmp, FALSE, 0) != -1)
				p = strCache(tmp);
			else
				p = NULL;
			
		}
                if(p==NULL) p=(char*)ln;
	}		
	return p;       

//       return ((LoadStrFromFile(fdLanguage, ln, tmp, FALSE, 0)!=-1)?strCache(tmp):NULL);
}

char* LoadStrByIDPad(int ID, int Len)
{
	char *p;
	int i;
	
	p=LoadStrByID(ID);
	memset(ln,' ',Len); ln[Len]=0;
	if(p) 
	{
		for(i=0;i<Len;i++)
		{
			if(p[i]==0) break;
			ln[i]=p[i];
		}
	}
	return ln;
}

char* GetYesNoName(int Yes)
{
	if(Yes) return LoadStrByID(HID_YES); else return LoadStrByID(HID_NO);
}

static char SMSBuf[100];
                                                                                                               
char *GetSMS(int UserID)
{
        int i, id;
        char *p;
        for(i=0;i<=100;i++)
        {
                sprintf(SMSBuf, "SMS%d", i);
                p=LoadStrOld(SMSBuf);
                if(p && *p)
                {
                        id=(Hex2Char(p)<<12)+(Hex2Char(p+1)<<8)+(Hex2Char(p+2)<<4)+Hex2Char(p+3);
                        if(UserID==id)
                        {
                                memset(SMSBuf, 0, 100);
                                return nstrcpy(SMSBuf, p+5, 100);
                        }
                }
        }
        return NULL;
}

int ClearAllACOpt(int All)
{
	char name[20], *Buffer;
	int i, oldsize, size;
	char p[1024];
	
	size=lseek(fdOptions, 0, SEEK_END);
	Buffer=(char*)malloc(size);
	lseek(fdOptions, 0, SEEK_SET);
	if (read(fdOptions, Buffer, size)!=size) 
	{
	    free(Buffer);
	    return FALSE;
	}
	
	size=PackStrBuffer(Buffer, "NONE", size);
	oldsize=size;	
	//清除分组设置
	for(i=1;i<10;i++)
	{
		sprintf(name,"GRP%d", i);
		size=PackStrBuffer(Buffer, name, size);
	}
	
	//清除开锁组合
	if(All)
		size=PackStrBuffer(Buffer, "ULG", size);
	
	//清除时间段设置
	if(All)
	    for(i=1;i<=50;i++)
	    {
	        sprintf(name,"TZ%d", i);
		 size=PackStrBuffer(Buffer, name, size);
	    }
	
	//清除组时间段设置
	if(All)
	    for(i=1;i<=50;i++)
	    {
	       sprintf(name,"GTZ%d", i);
		size=PackStrBuffer(Buffer, name, size);
	    }
	
	//Clear 用户时间段设置
	for(i=1;i<65535;i++)
	{
		sprintf(name,"UTZ%d", i);
		if(LoadStr(name, p))
			size=PackStrBuffer(Buffer, name, size);		
	}
	if(oldsize!=size)
	{
		TruncOptionAndSaveAs(Buffer, size);
	}
	free(Buffer);
	return TRUE;
}

int ClearOptionItem(char *name)
{
       int size, orisize;
       char *Buffer;
       
       size=lseek(fdOptions, 0, SEEK_END);
       Buffer=(char*)malloc(size);
       lseek(fdOptions, 0, SEEK_SET);
       if (read(fdOptions, Buffer, size)!=size)
       {
	       free(Buffer);
	       return FALSE;
       }
       orisize=size;
	
       size=PackStrBuffer(Buffer, name, size);	
	
       if(orisize!=size)
       {
	       TruncOptionAndSaveAs(Buffer, size);
       }
       free(Buffer);
       return TRUE;	   
}

//Language 
void SelectLanguage(char Language)
{
	char buffer[128];
	char *tmp;
 
	if ((tmp=LoadStrOld("FONTFILEPATH"))!=NULL)
		sprintf(buffer, "%s%s.%c", tmp, "LANGUAGE", Language);
	else
		sprintf(buffer, "%s.%c", "/mnt/mtdblock/LANGUAGE", Language);	 
//	printf(">>>>language path=%s\n",buffer);
	if (Language!=CurLanguage)
	{
		if (fdLanguage > 0) close(fdLanguage);
		fdLanguage = open(buffer, O_RDWR, S_IREAD|S_IWRITE);
		CurLanguage = Language;
	}
}

int GetLocaleID(int fd, int LngID)
{
	char *p, buf[]="E/_0_";
	char tmp[VALUE_BUFFERLEN];
	
	buf[0]=LngID;
	p=((LoadStrFromFile(fd, buf, tmp, FALSE, 0)!=-1)?strCache(tmp):NULL);
	if(p)
                return str2int(p,LID_INVALID);
	else
		return -2;
}

int GetDefaultLocaleID(void)
{
        return GetLocaleID(fdLanguage, gOptions.Language);
}

char *GetLangName(char LngID)
{
        char *p, buf[]="E/_0_";
	int fdTmp;
	char path[128];
	char value[VALUE_BUFFERLEN];	
	char *tmp;

	buf[0]=LngID;        
	//该资源的语言与当前的系统语言一致,则取本地化的语言名称，否则取英语名称	
	if(CurLanguage==LngID)
	{
                buf[3]='1';
		p=((LoadStrFromFile(fdLanguage, buf, value, FALSE, 0)!=-1)?strCache(value):NULL);
		//English name
		if(p==NULL)
		{
			buf[3]='2';
			p=((LoadStrFromFile(fdLanguage, buf, value, FALSE, 0)!=-1)?strCache(value):NULL);
		}
	}
	else
	{
		buf[3]='2';
		if ((tmp=LoadStrOld("FONTFILEPATH"))!=NULL)
			sprintf(path, "%s%s.%c", tmp, "LANGUAGE", LngID);
		else
			sprintf(path, "%s.%c", "/mnt/mtdblock/LANGUAGE", LngID);
		fdTmp=open(path, O_RDONLY);
		if(fdTmp==-1) 
			p=NULL; 
		else
		{
			p=((LoadStrFromFile(fdTmp, buf, value, FALSE, 0)!=-1)?strCache(value):NULL);
			if(p==NULL)
			{
				buf[3]='1';
				p=((LoadStrFromFile(fdTmp, buf, value, FALSE, 0)!=-1)?strCache(value):NULL);
			}
			close(fdTmp);
		}
	}
	return p;
}

int GetSupportedLang(int *LngID, int MaxLngCnt)
{
	DIR *dir;
        struct dirent *entry;
	char *filename;
	int LngCnt=0;
	char path[128];
	char *tmp;
	
	if ((tmp=LoadStrOld("FONTFILEPATH"))!=NULL)
		sprintf(path, "%s", tmp);
	else
		sprintf(path, "./");
        dir=opendir(path);
        if(dir)
        {
		while((LngCnt<MaxLngCnt)&&((entry=readdir(dir))!=NULL))
		{
			filename=entry->d_name;
			if((strlen(filename)==10)&&(strncmp(filename, "LANGUAGE.", 9)==0))
			{
				LngID[LngCnt++]=filename[9];
			}
		}
		closedir(dir);
		dir=0;
	}
        return LngCnt;
}

int GetSupportedLangByYaffs(int *LngID, int MaxLngCnt)
{
	yaffs_DIR *dir;
	yaffs_dirent *entry;

        char path[128];
	char *tmp;
	char *filename;
	int LngCnt=0;

        if ((tmp=LoadStrOld("FONTFILEPATH"))!=NULL)
		sprintf(path, "%s", tmp);
	else
		sprintf(path, "./");

	dir=yaffs_opendir(path);
	if(dir)
	{
		while((LngCnt<MaxLngCnt) && ((entry=yaffs_readdir(dir))!=NULL))
		{
			filename=entry->d_name;
			if((strlen(filename)==10) && (strncmp(filename,"LANGUAGE.",9)==0))
			{
				LngID[LngCnt++]=filename[9];
			}
		}
		yaffs_closedir(dir);
		dir=0;
	}
	return LngCnt;
}

BOOL UpdateNetworkInfoByDHCP(char *dhcp)
{
	FILE *fp;
	char buffer[1024];
	char tmp[128];
	char *name, *value;
	int len, i;
	char OpName[128];
	BOOL bSign=FALSE;
	
	if((fp=fopen(dhcp, "rb"))==NULL) return FALSE;
	while(!feof(fp))
	{
		memset(buffer, 0, 1024);
		if(!fgets(buffer, 1024, fp)) break;
		i=0;
		name=buffer;
		value=NULL;
		while(buffer[i])
		{
			if(buffer[i]=='=')
			{
				buffer[i]='\0';
				value=buffer+i+1;
				//trunc the CR
				i=0;
				while(value[i])
				{
					if((value[i]=='\r')||(value[i]=='\n'))
					{
						value[i]='\0';
						break;
					}
					i++;
				}	
				TrimRightStr(value);
				break;
			}
			i++;
		}
		//OK, we get a valid line
		if(value)
		{
			memset(OpName, 0, 128);
			if(strcmp(name, "ip")==0)
			{
				strcpy(OpName, "IPAddress");
				str2ip(value, gOptions.IPAddress);
			}
			else if(strcmp(name, "router")==0)
			{
				strcpy(OpName, "GATEIPAddress");
				str2ip(value, gOptions.GATEIPAddress);
			}
			else if(strcmp(name, "subnet")==0)
			{
				strcpy(OpName, "NetMask");
				str2ip(value, gOptions.NetMask);
			}
			if(OpName[0])
			{
				//Check OpName
				if(LoadStr(OpName, tmp))
				{
					//the value is the same as old value, then return.
					if (0==strcmp(value, tmp)) continue;
				}
				CombineNameAndValue(OpName, value, TRUE, tmp);
				len=lseek(fdOptions, 0, SEEK_END);
				if (len>=MAX_OPTION_SIZE)
				{ 
					ClearOptionItem("NONE");
					len=lseek(fdOptions, 0, SEEK_END);
				}
				if (len<MAX_OPTION_SIZE)
					write(fdOptions, tmp, strlen(tmp));
				bSign=TRUE;
			}
		}	
	}
	fclose(fp);
	return bSign;
}

#ifndef URU
int TestEEPROM(BYTE *data, int size)
{
        BYTE Buffer[1024];
        memset(Buffer, 0, 1024);
        if(0==Read24WC02(0,Buffer,size))
        if(0==Read24WC02(0,Buffer,size))
        if(0==Read24WC02(0,Buffer,size))
                return -1;
        if(nmemcmp(Buffer, data, size))
                return 1;
        else	
                return 0;
}

int CheckSensorData(unsigned short *data)
{
        int i, sum;
        sum=1;
        for(i=0;i<14;i++) sum+=data[i];
        if(data[14]!=(sum & 0x7FFF)) return FALSE;
        if((data[2]<50) || (data[3]<50) ||(data[2]>CMOS_WIDTH) || (data[3]>CMOS_HEIGHT) ||
                  ((data[12]&0x7FFF)<50) || (data[13]<50)) //是否合法的数据
                  return FALSE;
        return TRUE;
}

int ReadSensorOptions(POptions opts)
{
	extern BOOL GC0303_NoCoat;
        unsigned short data[15];
        int i=10;
        while(i--)
        if(Read24WC02(0,(BYTE*)data,sizeof(data)))
	{
		DBPRINTF("Read data from EEPROM OK!\n");
		for(i=0;i<14;i++)
			DBPRINTF("data[%d]=%d\n", i, data[i]);
                if(!CheckSensorData(data))
                {
                        //不合法的数据则重写
			printf("Error Chedking data from eeprom\n");
			fd_sensor = 0;
                        return 0;//WriteSensorOptions(opts, TRUE);
                }
                else
                {
                        opts->OLeftLine         =data[0];
                        opts->OTopLine         	=data[1];
                        opts->OImageWidth       =data[2];
                        opts->OImageWidth 	=((opts->OImageWidth+2)/4)*4;
                        opts->OImageHeight     	=data[3];
			
			opts->ZF_WIDTH          =data[12]&0xFFF;
                        opts->ZF_WIDTH		=((opts->ZF_WIDTH+2)/4)*4;
                        opts->ZF_HEIGHT         =data[13]&0xFFF;
			gOptions.NewFPReader	=((data[12]&0x7FFF)>>12) + ((data[13]>>12)<<4);			
			if(gOptions.NewFPReader&0x01)
				GC0303_NoCoat=TRUE;
			else
				GC0303_NoCoat=FALSE;
		//	if(gOptions.NewFPReader)
			if(0)
			{
				opts->CPY[0]    =data[11];
				opts->CPY[1]	=data[10];
				opts->CPY[2]    =data[9];
				opts->CPY[3]    =data[8];
				
				opts->CPX[0] 	=data[4];
				opts->CPX[1]    =data[5];
				opts->CPX[2]    =data[6];
				opts->CPX[3]    =data[7];
			}
			else
			{
				opts->CPY[0]    =data[8];
				opts->CPY[1]    =data[9];
				opts->CPY[2]    =data[10];
				opts->CPY[3]    =data[11];
				
				opts->CPX[0] 	=data[5];
				opts->CPX[1]    =data[4];
				opts->CPX[2]    =data[7];
				opts->CPX[3]    =data[6];
			}				
                        return 1;
                }
        }
        else
	{
		fd_sensor = 0;
		printf("Read EEPROM Error!\n");
                return 0;
	}
}

int EEPROMWriteOpt(BYTE * data, int size, int Rewrite)
{
        int i;
        if(Rewrite)
                i=1;
        else
                i=TestEEPROM(data, size);
        if(i==-1)
                return FALSE;
        else if(i==0)
                return TRUE;
        else
        {
                i=10;
                while(i--)
                {
                        if(Write24WC02(0, data, size))
                        {
                                if(TestEEPROM(data, size)==0)
                                        return TRUE;
                        }
                }
                return FALSE;
        }
}

int WriteSensorOptions(POptions opts, int Rewrite)
{
	short data[15];
        int i, sum;
        data[0] =opts->OLeftLine;
        data[1] =opts->OTopLine;
        data[2] =opts->OImageWidth;
        data[3] =opts->OImageHeight;
        data[5] =opts->CPX[0];
        data[4] =opts->CPX[1];
        data[7] =opts->CPX[2];
        data[6] =opts->CPX[3];
        data[9] =opts->CPY[0];
        data[8] =opts->CPY[1];
        data[11]=opts->CPY[2];
        data[10]=opts->CPY[3];
        data[12]=((gOptions.NewFPReader&0x0F)<<12)+opts->ZF_WIDTH;
        data[13]=(((gOptions.NewFPReader&0xF0)>>4)<<12)+opts->ZF_HEIGHT;
        sum=1;
        for(i=0;i<14;i++) sum+=data[i];
        data[14]=(sum & 0x07FFF);
	DBPRINTF("Write data to EEPROM OK!\n");
	for(i=0;i<14;i++)
		DBPRINTF("data[%d]=%d\n", i, data[i]);
        return EEPROMWriteOpt((BYTE*)data, sizeof(data), Rewrite);
}
#endif

char * macformat(char *str, BYTE *value)
{
	sprintf(str,"%02X:%02X:%02X:%02X:%02X:%02X", value[0],value[1],value[2],value[3],value[4],value[5]);
	return str;
}

char * ipformat(char *str, BYTE *value)
{
	sprintf(str,"%d.%d.%d.%d",value[0],value[1],value[2],value[3]);
	return str;
}

TOptionsResStr OptionsResStr[]={	
	//配置名称	长度	缺省值				是否需要恢复出厂设置
	{"MAC",		6,	{0x00,0x17,0x61,0x09,0x11,0x23},0,	optoffset(MAC),		str2mac,	macformat	},
	{"CardKey",	6,	{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},0,	optoffset(CardKey),	str2cardkey,	GetCardKeyStr	},
	{"IPAddress",	4,	{192,168,1,201},		1,	optoffset(IPAddress),	str2ip,		ipformat	},
	{"GATEIPAddress",4,	{0,0,0,0},			1,	optoffset(GATEIPAddress),str2ip,	ipformat	},
	{"NetMask",	4,	{255,255,255,0},		1,	optoffset(NetMask),	str2ip,		ipformat	},
	{"AuthServerIP",4,	{0,0,0,0},			1,	optoffset(AuthServerIP),str2ip,		ipformat	},
	{"WebServerIP",	4,	{0,0,0,0},			1,	optoffset(WebServerIP),	str2ip,		ipformat	},
	{"TimeServerIP",4,	{0,0,0,0},			1,	optoffset(TimeServerIP),str2ip,		ipformat	},
	{"ProxyServerIP",4,	{0,0,0,0},			1,	optoffset(ProxyServerIP),str2ip,	ipformat	}
};

TOptionsResInt OptionsResInt[]={
	//配置名称		缺省值			是否需要恢复出厂设置		菜单项资源 		最大值		最小值
	{"~ML",			1,			0,	optoffset(MultiLanguage),	0,			1,		0 	},
	{"Language",		LanguageSimplifiedChinese,0,	optoffset(Language),		MID_OS_LANGUAGE,	255,		32+1 	},
	{"DeviceID",		0x01,			1,	optoffset(DeviceID),		MID_OS_DEVNUMBER,	255,		1 	},
	{"MThreshold",		55,			1,	optoffset(MThreshold),								},
	{"EThreshold",		45,			0,	optoffset(EThreshold),								},
	{"VThreshold",		35,			1,	optoffset(VThreshold),								},
	{"LastAttLo",		0,			1,	optoffset(LastAttLog),								},
	{"UDPPort",		0x1112,			0,	optoffset(UDPPort),								},	
	{"TCPPort",		0x1110,			0,	optoffset(TCPPort),								},
	{"OImageWidth",		404,			0,	optoffset(OImageWidth),		0,			CMOS_WIDTH,	200 	},
	{"OImageHeight",	300,			0,	optoffset(OImageHeight),	0,			CMOS_HEIGHT,	200 	},
	{"OTopLine",		40,			0,	optoffset(OTopLine),		0,			CMOS_HEIGHT,	0 	},
	{"OLeftLine",		144,			0,	optoffset(OLeftLine),		0,			CMOS_WIDTH,	0 	},
	{"CPX0",		377,			0,	optoffset(CPX[0]), 								},
	{"CPX1",		28,			0,	optoffset(CPX[1]),								},
	{"CPX2",		424,			0,	optoffset(CPX[2]),								},
	{"CPX3",		-20,			0,	optoffset(CPX[3]),								},
	{"CPY0",		300,			0,	optoffset(CPY[0]),								},
	{"CPY1",		300,			0,	optoffset(CPY[1]),								},
	{"CPY2",		0,			0,	optoffset(CPY[2]),								},
	{"CPY3",		0,			0,	optoffset(CPY[3]),								},
	{"ZF_WIDTH",		276,			0,	optoffset(ZF_WIDTH),								},
	{"ZF_HEIGHT",		294,			0,	optoffset(ZF_HEIGHT),								},
	{"MSpeed",		MSPEED_AUTO,		0,	optoffset(MSpeed),								},
	{"AttState",		0,			1,	optoffset(AttState),								},
	{"~MaxUserCount",	30,			0,	optoffset(MaxUserCount),							},
	{"~MaxAttLogCount",	3,			0,	optoffset(MaxAttLogCount),							},
	{"~MaxFingerCount",	8,			0,	optoffset(MaxFingerCount),							},
	{"LockOn",		150,			1,	optoffset(LockOn),		MID_OS_LOCK,		500,		0 	},
	{"AlarmAttLog",		99,			1,	optoffset(AlarmAttLog),								},
	{"AlarmOpLog",		99,			1,	optoffset(AlarmOpLog),								},
	{"AlarmReRec",		0,			1,	optoffset(AlarmReRec),								},
	{"RS232BaudRate",	115200,			1,	optoffset(RS232BaudRate),	0,			115200,		9600	},
	{"RS232CRC",		0,			0,	optoffset(RS232CRC),								},
	{"RS232Stop",		1,			0,	optoffset(RS232Stop),								},
	{"WEBPort",		80,			0,	optoffset(WEBPort),								},
	{"~ShowState",		0,			0,	optoffset(ShowState),								},
	{"~KeyLayout",		KeyLayout_BioClockIII,	0,	optoffset(KeyLayout),								},
	{"VoiceOn",		1,			1,	optoffset(VoiceOn),		HMID_VOICEON,		1,		0	},
	{"AutoPowerOff",	0xFFFF,			1,	optoffset(AutoPowerOff),	MID_OSA_POWEROFF				},
	{"AutoPowerOn",		0xFFFF,			1,	optoffset(AutoPowerOn),		MID_OSA_POWERON					},
	{"AutoPowerSuspend",	0xFFFF,			1,	optoffset(AutoPowerSuspend),	MID_OSA_SUSPEND					},
	{"AutoAlarm1",		0xFFFF,			1,	optoffset(AutoAlarm[0]),							},
	{"AutoAlarm2",		0xFFFF,			1,	optoffset(AutoAlarm[1]),							},
	{"AutoAlarm3",		0xFFFF,			1,	optoffset(AutoAlarm[2]),							},
	{"AutoAlarm4",		0xFFFF,			1,	optoffset(AutoAlarm[3]),							},
	{"AutoAlarm5",		0xFFFF,			1,	optoffset(AutoAlarm[4]),							},
	{"AutoAlarm6",		0xFFFF,			1,	optoffset(AutoAlarm[5]),							},
	{"AutoAlarm7",		0xFFFF,			1,	optoffset(AutoAlarm[6]),							},
	{"AutoAlarm8",		0xFFFF,			1,	optoffset(AutoAlarm[7]),							},
        {"IdlePower",		HID_SUSPEND,		1,	optoffset(IdlePower),		MID_OSA_IDLE,		HID_SUSPEND,	HID_POWEROFF},
        {"IdleMinute",		0,			1,	optoffset(IdleMinute),		MID_OSA_IDLETIME				},
        {"ShowScore",		0,			1,	optoffset(ShowScore),		HMID_SHOWSCORE,		1,		0	},
        {"NetworkOn",		1,			1,	optoffset(NetworkOn),		MID_OC_NETOFF,		1,		0	},
        {"RS232On",		1,			0,	optoffset(RS232On),		MID_OC_RS232OFF,	1,		0	},
        {"RS485On",		0,			0,	optoffset(RS485On),		MID_OC_RS485OFF,	1,		0	},
	{"~NetworkFunOn",	1,			0,	optoffset(NetworkFunOn),	MID_OI_NET,		1,		0	},
	{"~LockFunOn",		1,			0,	optoffset(LockFunOn),								},
	{"~RFCardOn",		1,			0,	optoffset(RFCardFunOn),								},
	{"~One2OneFunOn",	1,			0,	optoffset(One2OneFunOn),							},
	{"~PowerMngFunOn",	1,			0,	optoffset(PowerMngFunOn),	MID_OI_POWERMNG,	1,		0	},
	{"~NewFPReader",	0,			0,	optoffset(NewFPReader),								},
	{"~ShowName",		1,			0,	optoffset(ShowName),		0,			1,		0	},
	{"UnlockPerson",	1,			1,	optoffset(UnlockPerson),							},
	{"ShowCheckIn",		0,			0,	optoffset(ShowCheckIn),								},
	{"OnlyPINCard",		1,			1,	optoffset(OnlyPINCard),		MID_OC_PINCARD,		1,		0	},
	{"~IsTestMachine",	0,			1,	optoffset(IsTestMachine),							},
	{"~MustChoiceInOut",	0,			0,	optoffset(MustChoiceInOut),							},
	{"HiSpeedNet",		8,			1,	optoffset(HiSpeedNet),								},
	{"~MenuStyle",		1,			0,	optoffset(MenuStyle),								},
	{"CCCKey",		1,			0,	optoffset(CanChangeCardKey),							},
	{"Must1To1",		0,			1,	optoffset(Must1To1),		MID_OS_MUST1TO1,	1,		0	},
	{"LCDM",		0,			0,	optoffset(LCDModify),								},
	{"COMKey",		0,			1,	optoffset(ComKey),								},
	{"MustEnroll",		1,			1,	optoffset(MustEnroll),		MID_OC_MUSTENROLL				},
	{"TOMenu",		60,			0,	optoffset(TimeOutMenu),		0,			65535*32768,	10	},
	{"TOPin",		10,			0,	optoffset(TimeOutPin),		0,			65535,		5	},
	{"TOState",		10,			0,	optoffset(TimeOutState),							},
	{"SaveAttLog",		1,			0,	optoffset(SaveAttLog),								},	
	{"RS232Fun",		1,			0,	optoffset(RS232Fun),								},	
        {"~IsModule",		0,			0,	optoffset(IsModule),								},
	{"~ShowSecond",		0,			0,	optoffset(ShowSecond),								},
	{"~RFSStart",		0,			0,	optoffset(RFCardSecStart),							},
	{"~RFSLen",		10,			0,	optoffset(RFCardSecLen),							},	
	{"~RFFPC",		1,			0,	optoffset(RFCardFPC),								}, 	
	{"~PIN2Width",		5,			0,	optoffset(PIN2Width),		0,			10,		5	}, 	
	{"DtFmt",		0,			1,	optoffset(DateFormat),								}, 	
	{"~OPLM1",		-1,			0,	optoffset(OPLogMask1),								},
	{"~OPLM2",		0,			0,	optoffset(OPLogMask2),								},
	{"AS1",			-1,			1,	optoffset(AutoState[0]),							},
	{"AS2",			-1,			1,	optoffset(AutoState[1]),							},
	{"AS3",			-1,			1,	optoffset(AutoState[2]),							},
	{"AS4",			-1,			1,	optoffset(AutoState[3]),							},
	{"AS5",			-1,			1,	optoffset(AutoState[4]),							},
	{"AS6",			-1,			1,	optoffset(AutoState[5]),							},
	{"AS7",			-1,			1,	optoffset(AutoState[6]),							},
	{"AS8",			-1,			1,	optoffset(AutoState[7]),							},
	{"AS9",			-1,			1,	optoffset(AutoState[8]),							},
	{"AS10",		-1,			1,	optoffset(AutoState[9]),							},
	{"AS11",		-1,			1,	optoffset(AutoState[10]),							},
	{"AS12",		-1,			1,	optoffset(AutoState[11]),							},
	{"AS13",		-1,			1,	optoffset(AutoState[12]),							},
	{"AS14",		-1,			1,	optoffset(AutoState[13]),							},
	{"AS15",		-1,			1,	optoffset(AutoState[14]),							},
	{"AS16",		-1,			1,	optoffset(AutoState[15]),							},
	{"~DC",			3,			0,	optoffset(DelayCount),								},
	{"~IncThr",		14,			0,	optoffset(IncThr),								},
	{"~TopThr",		50,			0,	optoffset(TopThr),								},
	{"~MinThr",		30,			0,	optoffset(MinThr),								},
	{"NoiseThreshold",	100,			0,	optoffset(MaxNoiseThr),								},
	{"~MinM",		12,			0,	optoffset(MinMinutiae),								},
	{"~MaxTL",		MAXVALIDTMPSIZE,	0,	optoffset(MaxTempLen),								},
	{"AdminCnt",		1,			0,	optoffset(AdminCnt),								},
	{"ODD",			10,			1,	optoffset(OpenDoorDelay),							},
	{"DSM",			2,			1,	optoffset(DoorSensorMode),							},
	{"ECnt",		3,			0,	optoffset(EnrollCount),		0,			10,		3	},
	{"~AAFO",		0,			0,	optoffset(AutoAlarmFunOn),							},
	{"AADelay",		10,			1,	optoffset(AutoAlarmDelay),							},
	{"~ASFO",		0,			0,	optoffset(AutoStateFunOn),							},
	{"DUHK",		0,			1,	optoffset(DuressHelpKeyOn),	MID_AD_DURESSHELP,	1,		0	},
	{"DU11",		0,			1,	optoffset(Duress1To1),		MID_AD_DURESS11,	1,		0	},
	{"DU1N",		0,			1,	optoffset(Duress1ToN),		MID_AD_DURESS1N,	1,		0	},
	{"DUPWD",		0,			1,	optoffset(DuressPwd),		MID_AD_DURESSPWD,	1,		0	},
	{"DUAD",		10,			1,	optoffset(DuressAlarmDelay),							},
	{"LockPWRButton",	0,			1,	optoffset(LockPowerButton),	0,			1,		0	},
	{"SUN",			3,			0,	optoffset(StartUpNotify),							},
	{"I1NFrom",		0,			1,	optoffset(I1ToNFrom),								},
	{"I1NTo",		0,			1,	optoffset(I1ToNTo),								},
	{"I1H",			0,			1,	optoffset(I1ToH),		MID_OS_1TOH		,1		,0	},
	{"I1G",			0,			1,	optoffset(I1ToG),		MID_OS_1TOG		,1		,0	},
	{"~MaxUserFingerCount",	10,			0,	optoffset(MaxUserFingerCount),							},
	{"~MIFARE",		0,			0,	optoffset(IsSupportMF),								},
	{"~FlashLed",		1,			0,	optoffset(IsFlashLed),								},
	{"~IsInit",		1,			0,	optoffset(IsInit),								},
	{"CMOSGC",		0,			0,	optoffset(CMOSGC),		0,			255,		0	},
	{"~ADMATCH",		0,			0,	optoffset(AdvanceMatch),							},
	{"ERRTimes",		0,			1,	optoffset(ErrTimes),		MID_AD_ERRPRESS,	255,		0       },
	{"~IsOnlyOneSensor",	1,			1,	optoffset(IsOnlyOneSensor),	0,			1,		0	},
	{"AuthServerEnabled",	0,			1,	optoffset(AuthServerEnabled),							},
	{"ConnectMODEM",	0,			1,	optoffset(IsConnectModem),	0,			1,		0	},
	{"AATimes",		2,			1,	optoffset(AutoAlarmTimes),							},
	{"DNSCheckTime",	0,			1,	optoffset(DNSCheckInterval),							},
	{"AutoUPLogTime",	0,			1,	optoffset(AutoUploadAttlog),							},
	{"DisableUser",		0,			1,	optoffset(DisableNormalUser),	0,			1,		0 	},
	{"KeyPadBeep",		0,			1,	optoffset(KeyPadBeep),		0, 			1,		0	},
	{"WorkCode",		0,			1,	optoffset(WorkCode),		0,			10,		0	},
	{"VOLUME",		67,			1,	optoffset(AudioVol), 		0,			99,		0 	},
	{"AAVOLUME",		67,			1,	optoffset(AutoAlarmAudioVol),	0,			99,		1 	},
	{"DHCP",		0,			1,	optoffset(DHCP),		0,			1,		0	},
	{"AutoSyncTime",	0xFFFF,			1,	optoffset(AutoSyncTime),							},
	{"~IsOnlyRFMachine",	0,			0,	optoffset(IsOnlyRFMachine),	0,			1, 		0 	},
	{"~OS",			1,			0,	optoffset(OS),			0,			255, 		0 	},
	{"~IsWiegandKeyPad",	0,			0,	optoffset(IsWiegandKeyPad),	0,			1, 		0 	},
	{"~SMS",		0,			0,	optoffset(IsSupportSMS),	0,			1, 		0 	},
	{"~USBDisk",		1,			0,	optoffset(IsSupportUSBDisk),	0,			1, 		0 	},
	{"~MODEM",		0,			0,	optoffset(IsSupportModem),	0,			1, 		0 	},
	{"~AuthServer",		0,			0,	optoffset(IsSupportAuthServer),	0,			1, 		0 	},
	{"~ACWiegand",		0,			0,	optoffset(IsACWiegand),		0,			1, 		0 	},
	{"~ExtendFmt",		0,			0,	optoffset(AttLogExtendFormat),	0,			1, 		0 	},
	{"~DRPass",		0,			0,	optoffset(DisableRootPassword),	0,			1, 		0 	},
	{"~MP3",		0,			0,	optoffset(IsSupportMP3),	0,			1, 		0 	},
	{"~MIFAREID",		0,			0,	optoffset(MifareAsIDCard),	0,			1, 		0 	},
	{"~GroupVoice",		0,			0,	optoffset(PlayGroupVoice),	0,			1, 		0 	},
	{"~TZVoice",		0,			0,	optoffset(PlayTZVoice),		0,			1, 		0 	},
	{"~ASTFO",		0,			0,	optoffset(AutoSyncTimeFunOn),	0,			1, 		0 	},
	{"~CFO",		0,			0,	optoffset(CameraFunOn),		0,			1, 		0 	},
	{"~SaveBitmap",		0,			0,	optoffset(SaveBitmap),		0,			1, 		0 	},
	{"~ProcessImage",	0,			0,	optoffset(ProcessImage),	0,			1, 		0 	},
	{"ASTimeOut",		10,			0,	optoffset(AuthServerTimeOut),	0,			30, 		0 	},
	{"~TLLCM",		0,			0,	optoffset(TwoLineLCM),		0,			0, 		0 	},
	{"~UserExtFmt",		0,			0,	optoffset(UserExtendFormat),	0,			1, 		0 	},
	{"RefreshUserData",	0,			1,	optoffset(RefreshUserData),							},
	{"~DisableAU",		0,			0,	optoffset(DisableAdminUser),							},
	{"~ICFO",		0,			0,	optoffset(IClockFunOn),								},
	{"ProxyServerPort",	0,			1,	optoffset(ProxyServerPort),							},
	{"~C2",			0,			0,	optoffset(IsSupportC2),								},
	{"EnableProxyServer",	0,			1,	optoffset(EnableProxyServer),							},
	{"~WCFO",		0,			0,	optoffset(WorkCodeFunOn),							},
	{"~VALF",		0,			0,	optoffset(ViewAttlogFunOn),	0,			10,		0	},
	{"~DHCPFunOn",		0,			0,	optoffset(DHCPFunOn),		0,			1,		0 	},
	{"~OutSensorFunOn",	0,			0,	optoffset(OutSensorFunOn),	0,			1,		0 	},
	{"SaveAuthServerLog", 	0,			1,	optoffset(SaveAuthServerLog),	0,			1,		0 	},
	{"SetGatewayWaitCount", 30,			1,	optoffset(SetGatewayWaitCount),	0,			65535*32768,	0 	},
	{"~HID", 		0,			0,	optoffset(IsSupportHID),	0,			1,		0 	},
	{"DoorSensorTimeout", 	30,			1,	optoffset(DoorSensorTimeout),	0,			99,		0 	},
	{"DAM", 		0,			1,	optoffset(DoorAlarmMode),							},
	{"~ASDewarpedImage",0,                          0,     optoffset(ASDewarpedImage),      0,
		   1,		   0},
	{"Nideka",		0,	0,		 	optoffset(Nideka), 		0,		1,		0},
        {"~PrinterFunOn",       0,            		 0,      optoffset(PrinterFunOn),0,0},
        {"PrinterOn",           0,             		 1,      optoffset(PrinterOn)            ,MID_PRINTERON,10,0},
	{"CheckOutWc",		0,	0,			optoffset(CheckOutWc), 0,
		1,	0},
	{"SyncTmAuthServer",    30,      0,  			optoffset(SyncTimeFromAuthServer),},
        {"FPOpenRelay", 1,                      0,      optoffset(FPOpenRelay),0,                 1,              0},
        {"AutoOpenRelay",       0,                      0,      optoffset(AutoOpenRelay),0,
                1,              0},
        {"AutoOpenRelayTimes", 5,                       0,      optoffset(AutoOpenRelayTimes),0,
                20,             1},
        {"~AutoOpenRelayFunOn", 0,                      0,      optoffset(AutoOpenRelayFunOn),0,
                1,              0},
	{"DefaultGroup",       1,			0,	optoffset(DefaultGroup), 0,
		5,		1},
	{"GroupFpLimit",       200,			0,	optoffset(GroupFpLimit), 0,
		0,		0},
	{"LimitFpCount",	3200,			0,	optoffset(LimitFpCount),0,
		0,	0},
	{"WireLessBell",	0,			0,	optoffset(WireLessBell),0,
		0,	0},
	{"DevID",	0,			0,	optoffset(DevID),0, 99,	0},
	{"RunMode",	0,			0,	optoffset(RunMode),0, 10, 0},
	{"~ZKFPVersion", 10,                      0,      optoffset(ZKFPVersion),0, 0, 0},
	{"~MulAlgVer",0,                      0,      optoffset(MulAlgVer),0, 0, 0},
	{"VryFailWait", 1, 1, optoffset(VryFailWait), MID_VRYFAIL_WAIT, 1, 0},
	{"VryFailWaitCnt", 10, 1, optoffset(VryFailWaitCnt), MID_VRYFAIL_WAITCNT, 99, 3},
	{"VryBind", 1, 1, optoffset(VryBind), MID_VRY_BIND, 2, 0},
	{"NorOpenOn", 1, 1, optoffset(NorOpenOn), MID_NOR_OPEN_ON, 1, 0},
};

POptionsResInt QueryOptResByOffset(int Offset)
{
	int i;
	for(i=0;i<OPTIONSRESINTCOUNT;i++)
	{
		if(OptionsResInt[i].Offset==Offset) 
			return OptionsResInt+i;
	}
	return NULL;
}

//for option that not saved in flash
char *GetDefaultOption(const char *name,char *value)
{
        extern int gWGFailedID;
        extern int gWGDuressID;
        extern int gWGSiteCode;
        extern int gWGPulseWidth;
        extern int gWGPulseInterval;
        char *s=NULL;
        int i;
        for(i=0;i<OPTIONSRESINTCOUNT;i++)
        {
                if(strcmp(name,OptionsResInt[i].OptionName)==0)
                {
                        int v=LoadInteger(OptionsResInt[i].OptionName, OptionsResInt[i].DefaultValue);
                        if(v!=-1)
                        if(OptionsResInt[i].MaxValue>OptionsResInt[i].MinValue)
                        {
                                if(OptionsResInt[i].MaxValue<v)
                                        v=OptionsResInt[i].MaxValue;
                                else if(OptionsResInt[i].MinValue>v)
                                        v=OptionsResInt[i].MinValue;
                        }
                        sprintf(value,"%d",v);
                        break;
                }
        }
        if(strlen(value)==0)
        {
                if(strcmp(name, "WiegandFmt")==0)
                {
                        LoadStr("WiegandFmt",s);
                        if(s)
                                sprintf(value,"%s",s);
                        else
                                sprintf(value,"%d",26);
                }
                else if(strcmp(name, "WGFailedID")==0)
                        sprintf(value,"%d",gWGFailedID );
                else if(strcmp(name, "WGDuressID")==0)
                        sprintf(value,"%d",gWGDuressID );
                else if(strcmp(name, "WGSiteCode")==0)
                        sprintf(value,"%d",gWGSiteCode);
                else if(strcmp(name,"WGPulseWidth")==0)
                        sprintf(value,"%d",gWGPulseWidth);
                else if(strcmp(name,"WGPulseInterval")==0)
                        sprintf(value,"%d",gWGPulseInterval);
                else if(strcmp(name,"~RFSStart")==0)
                        sprintf(value,"%d",gOptions.RFCardSecStart);
                else if(strcmp(name,"~RFSLen")==0)
                        sprintf(value,"%d",gOptions.RFCardSecLen );
                else if(strcmp(name,"~RFFPC")==0)
                        sprintf(value,"%d",gOptions.RFCardFPC);
        }
        return value;
}





POptions GetDefaultOptions(POptions opts)
{
	int i=0;
	//Get common default value
	for(i=0;i<OPTIONSRESSTRCOUNT;i++)
	{
		if(OptionsResStr[i].IsNeedRestoreFactory)
			memcpy(((char*)opts)+OptionsResStr[i].Offset, OptionsResStr[i].DefaultValue, OptionsResStr[i].OptionLong);
	}
	
	for(i=0;i<OPTIONSRESINTCOUNT;i++)
	{
		if(OptionsResInt[i].IsNeedRestoreFactory)
			memcpy(((char*)opts)+OptionsResInt[i].Offset, &(OptionsResInt[i].DefaultValue), 4);
	}

#ifdef OEM_CMI
	opts->MaxNoiseThr=124;
	opts->RS232On =0;
	opts->RS485On =0;
#endif
	//special options
	if(LOCKFUN_ADV & LoadInteger("~LockFunOn",0))
	{
		opts->MThreshold=65;
		opts->VThreshold=55;
	}	
	opts->Saved =1;
	return opts;
}

POptions LoadOptions(POptions opts)
{
	int i;
#ifndef URU
	static BOOL LoadSign=FALSE;
#endif	
	char name1[128], value1[VALUE_BUFFERLEN];
	char buffer[VALUE_BUFFERLEN];
	int size;
	BOOL exitsign;
	int value;
       
	//setting default value
	for(i=0;i<OPTIONSRESSTRCOUNT;i++)
	{
		memcpy(((char*)opts)+OptionsResStr[i].Offset, OptionsResStr[i].DefaultValue, OptionsResStr[i].OptionLong);
	}
	for(i=0;i<OPTIONSRESINTCOUNT;i++)
	{
		memcpy(((char*)opts)+OptionsResInt[i].Offset, &(OptionsResInt[i].DefaultValue), 4);
	}
	//Read option from options.cfg	
	lseek(fdOptions, 0, SEEK_SET); 
	while(TRUE)
	{
		if(ReadOneLine(fdOptions, buffer, &size))
		{
			exitsign=FALSE;
			SplitByChar(buffer, name1, value1, '=');
			for(i=0;i<OPTIONSRESSTRCOUNT;i++)
			{
				if(strcmp(name1, OptionsResStr[i].OptionName)==0)
				{
					if(OptionsResStr[i].Convertor)
						OptionsResStr[i].Convertor(value1, ((BYTE*)opts)+OptionsResStr[i].Offset);
					else
						strcpy(((char*)opts)+OptionsResStr[i].Offset, value1);
					exitsign=TRUE;
					break;
				}
			}
			if(!exitsign)
			{
				for(i=0;i<OPTIONSRESINTCOUNT;i++)
				{
					if(strcmp(name1, OptionsResInt[i].OptionName)==0)
					{
						value=str2int(value1, OptionsResInt[i].DefaultValue);
						if(OptionsResInt[i].MaxValue>OptionsResInt[i].MinValue)
						{
							if(OptionsResInt[i].MaxValue<value) 
								value=OptionsResInt[i].MaxValue;
							else if(OptionsResInt[i].MinValue>value) 
								value=OptionsResInt[i].MinValue;
						}
						memcpy(((char*)opts)+OptionsResInt[i].Offset, &value, 4);
						break;
					}
				}				
			}			
		}
		else
			break;
	}
#ifndef URU
	//Read from sensor EEPROM
	if(!(LoadSign||gOptions.IsOnlyRFMachine))
	{
		ReadSensorOptions(opts);
		LoadSign=TRUE;
	}
#endif
	return opts;
}

char GetLangFileType(char Language)
{
	char ctype;

	switch (Language)
	{
		case 'S':
			ctype = 'C';
			break;
		default:
			ctype = 'E';
			break;
	}

	return ctype;
}

BOOL SaveStrIgnoreCheck(const char *name, const char *value)
{
	char buffer[VALUE_BUFFERLEN]; 
	int len;
	
	len=strlen(value);
	//check language item whether can be setup or not
	if(!gOptions.MultiLanguage)
	{
		if(strcmp(name, "Language")==0) return TRUE;
	}	
	CombineNameAndValue(name, value, TRUE, buffer);	
	len=lseek(fdOptions, 0, SEEK_END);
	if (len>=MAX_OPTION_SIZE)
	{	 
		ClearOptionItem("NONE");
		len=lseek(fdOptions, 0, SEEK_END);
	}
	if (len<MAX_OPTION_SIZE)
		write(fdOptions, buffer, strlen(buffer));
	yaffs_flush(fdOptions);
	ExecuteActionForOption(name, value);
	
	return ((len<MAX_OPTION_SIZE)?TRUE:FALSE);
}

POptions SaveOptions(POptions opts)
{
	int i;
	TOptions OldOpt;
	char Buf[20];
	char buffer[1000]={0};
	int value;

	LoadOptions(&OldOpt);

/*	if(gMachineState==STA_MENU) //正在进行设置
	{
		for(i=0;i<sizeof(OldOpt)/sizeof(int)-2;i++)
			if(((int*)&OldOpt)[i]!=((int*)opts)[i])
				FDB_AddOPLog(ADMINPIN, OP_CHG_OPTION, i,0,0,0);	

	}*/
	for(i=0;i<OPTIONSRESSTRCOUNT;i++)
	{
		if(memcmp((((char*)opts)+OptionsResStr[i].Offset), (((char*)&OldOpt)+OptionsResStr[i].Offset), OptionsResStr[i].OptionLong))
		{
			if(OptionsResStr[i].Formator)
				OptionsResStr[i].Formator(buffer, (BYTE*)(((char*)opts)+OptionsResStr[i].Offset));
			else
				nstrcpy(buffer,(((char*)opts)+OptionsResStr[i].Offset), OptionsResStr[i].OptionLong);
			SaveStrIgnoreCheck(OptionsResStr[i].OptionName, buffer);
		}
	}
	for(i=0;i<OPTIONSRESINTCOUNT;i++)
	{
		//DBPRINTF("Name=%s New=%d old=%d\n", OptionsResInt[i].OptionName, *(int*)(((char*)opts)+OptionsResInt[i].Offset), *(int*)(((char*)&OldOpt)+OptionsResInt[i].Offset));
		if(memcmp((((char*)opts)+OptionsResInt[i].Offset), (((char*)&OldOpt)+OptionsResInt[i].Offset), 4))
		{
			memcpy(&value, ((char*)opts)+OptionsResInt[i].Offset, 4);
			sprintf(Buf, "%d", value);
			SaveStrIgnoreCheck(OptionsResInt[i].OptionName, Buf);
		}
	}
	
#ifndef URU
	//WriteSensorOptions(opts, TRUE);
#endif	
			
	//flush the cached data to disk
	yaffs_flush(fdOptions);
//	yaffs_sync("/mnt");
	opts->Saved=TRUE;
	return opts;
}
