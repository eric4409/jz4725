//多国语言支持
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>
#include "ccc.h"
#include "locale.h"
#include "exfun.h"
#include "lcm.h"
#include "utils.h"

TLangDriver *gLangDriver=NULL;
TLangDriver *gSymbolDriver=NULL;
TLangDriver *gROMDriver=NULL;

static int ExpandFontOffset(TFontLib *fontlib)
{
	int i;
	int offset=1;
	if(fontlib->size && fontlib->offset)
	{
		int c=0, last=0, cc=0;
		for(i=0;i<fontlib->size;i++) //测试字体偏移量是否压缩
		{
			if(fontlib->offset[i]==last) 
			{
				if(c++>20) return 1;	//未压缩
			}
			else
				c=0;
			last=fontlib->offset[i]+1;
			if(last==2) if(cc++>20) break; //已压缩
		}
		for(i=0;i<fontlib->size;i++) //展开
		{
			int curoffset=fontlib->offset[i];
			if(curoffset==1)
			{
				fontlib->offset[i]=offset++;
			}
			else if(curoffset==0)
			{
			}
			else
			{
				fontlib->offset[i]=fontlib->offset[curoffset-fontlib->firstchar-2];
			}
		}
	}
	return 0;
}

BYTE *LoadFontLib(char *FontName, TFontLib *FontLib)
{
	U32 CodeSize;
	BYTE *FontStart=(void*)LoadFile(FontName, NULL);
	if(FontStart)
	{
		BYTE *p;
		FontLib->buffer=FontStart;
		memcpy(&CodeSize,FontStart,4);
		if(CodeSize>65535*2) return NULL; //没有大于65535的字体编码表
		p=FontStart+CodeSize+4;
		FontLib->width=p[0]+p[1]*256; p+=2;
		FontLib->height=p[0]+p[1]*256; p+=2;
		FontLib->firstchar=p[0]+p[1]*256; p+=2;
		if(FontLib->firstchar==0xcccc) FontLib->firstchar=0;
		FontLib->size=p[0]+p[1]*256; p+=2;
		FontLib->symbolcount=p[0]+p[1]*256; p+=2;
		if(FontLib->size)
			FontLib->offset=(unsigned short *)p;
		else
			FontLib->offset=NULL;
		FontLib->bits=p+FontLib->size*2;
		ExpandFontOffset(FontLib);
		return FontStart+4;
	}
	return NULL;
}

//取文本的像素宽度
int GetTextWidth_Default(void *BasedLangDriver, char *Text)
{
	return strlen(Text)*8;
}


//取文本的像素高度
int GetTextHeight_Default(void *BasedLangDriver, char *Text)
{
	return 16;
}

//取文本的字符数
int GetTextCharCount_Default(void *BasedLangDriver, char *Text)
{
	return strlen(Text);
}

//取超出设定宽度的下一段文本位置
char* GetNextText_Default(void *BasedLangDriver, char *Text, int Width)
{
	if((int)strlen(Text)*8>Width)
		return Text+Width/8;
	else
		return NULL;
}

char* GetNextText_EUC(void *BasedLangDriver, char *Text, int Width)
{
	while(1)
	{
		unsigned char ch=*Text;
		if(ch)
		{
			if(ch<=0x7F) 
			{
				Width-=8;
				if(Width<0) break;
				Text++;
			}
			else
			{
				Width-=16;
				if(Width<0) break;
				Text+=2;
			}
		}
		else
			break;
	}
	return Text;
}

//设置当前系统语言
PLangDriver CreateLanguageDriver(int LangID, char *FontName, int FontSize)
{
	switch(LangID)
	{
	case LID_UTF8:
		return CreateLanguage_UTF8(LangID, FontName, FontSize);
	case LID_ISO8859_1:	
	case LID_ISO8859_2:
	case LID_ISO8859_3:
	case LID_ISO8859_4:	
	case LID_ISO8859_5:
	case LID_ISO8859_6:	
	case LID_ISO8859_7:
	case LID_ISO8859_8:	
	case LID_ISO8859_9:	
	case LID_ISO8859_10:
	case LID_ISO8859_11:	
	case LID_ISO8859_13:	
	case LID_ISO8859_14:	
	case LID_ISO8859_15:	
	case LID_ISO8859_16:
	case LID_KOI8_R:
	case LID_CP1250:
	case LID_CP1251:
	case LID_CP1252:
	case LID_CP1253:
	case LID_CP1254:
	case LID_CP1255:
	case LID_CP1256:
	case LID_CP1257:
	case LID_CP1258:
		{
			return CreateLanguage_LT(LangID, FontName, FontSize);
		}
	case LID_SYMBOL:
		{
			return CreateLanguage_SYM(LangID, FontName, FontSize);
		}
	case LID_ROM:
	              {
		                   return CreateLanguage_ROM(LangID);
	               }
	case LID_BIG5:
		{
			return CreateLanguage_BIG5(LangID, FontName, FontSize);
		}
	case LID_SJIS:
		{
			return CreateLanguage_SJIS(LangID, FontName, FontSize);
		}
	case LID_KOR:
		{
			return CreateLanguage_CP949(LangID, FontName, FontSize);
		}
	default:
		{
		//	LoadLatinCodeLib(LID_UNICODE2);	//treckle
			return CreateLanguage_CN(LangID, FontName, FontSize);
		}
	}
}

//取当前系统语言设置
int GetDefaultLanguage(void)
{
	return gLangDriver->LanguageID;
}

void FreeLanguageDriver(PLangDriver LangDriver)
{
	if(LangDriver->FontLib)
	{
		free(LangDriver->FontLib->buffer);
		free(LangDriver->FontLib);
	}
	free(LangDriver);
}

int SetDefaultLanguage(int LocaleID, int RowHeight)
{
	if(gLangDriver)
	{
                if(gLangDriver!=gROMDriver)
			FreeLanguageDriver(gLangDriver);
		gLangDriver=NULL;
	}
	if(gSymbolDriver==NULL)
		gSymbolDriver=CreateLanguageDriver(LID_SYMBOL, NULL, 16);
        if(gROMDriver==NULL)
                gROMDriver=CreateLanguageDriver(LID_ROM, NULL, 8);

	gLangDriver=CreateLanguageDriver(LocaleID, NULL, RowHeight);
	if(gLangDriver)
	{
		gRowHeight=gLangDriver->CharHeight;
		gLCDRowCount=gLCDHeight/gRowHeight;
		gLCDCharWidth=gLCDWidth/gLangDriver->CharWidth;
	}
	return (int)gLangDriver;
}

static BYTE *zk2=NULL;
static int zk2len=0;

TFontLib *LoadOldFont(void)
{
	if(zk2==NULL)
	{
		if(zk2len==0)
		{
			zk2=(BYTE*)LoadFile("hz2.dat", &zk2len);
			if(zk2==NULL)
				zk2len=-1;
		}
	}
	if(zk2)
	{
		TFontLib *FontLib=(TFontLib*)malloc(sizeof(TFontLib));
		FontLib->buffer=zk2;
		FontLib->bits=zk2;
		FontLib->height=16;
		FontLib->width=8;
		FontLib->firstchar=0xa1a1;
		FontLib->size=(WORD)0xFEFE - (WORD)0xA1A1;
		FontLib->offset=NULL;
		FontLib->symbolcount=FontLib->size;
		FontLib->codeid=LID_GB23122;
		return FontLib;
	}
	return NULL;
}

PLangDriver CreateLanguage_Default(int LangID)
{
	TLangDriver *LangDriver=(PLangDriver)malloc(sizeof(TLangDriver));
	memset(LangDriver,0,sizeof(TLangDriver));
	LangDriver->GetTextHeightFun=GetTextHeight_Default;
	LangDriver->GetNextTextFun=GetNextText_Default;
	LangDriver->GetTextWidthFun=GetTextWidth_Default;
	LangDriver->LanguageID=LangID;
	LangDriver->CharHeight=16;
	LangDriver->CharWidth=8;
	return LangDriver;
}

BYTE *LoadFile(char *FileName, int *FileSize)
{
	int fp;
	char buf[64], tmp[64];
	BYTE *DataBuf=NULL;
	int len=0;
	sprintf(tmp, "%s", FileName);	//treckle
//	if((fp=open(GetEnvFilePath("USERDATAPATH", tmp, buf), O_RDWR, S_IREAD|S_IWRITE))==-1) 
	if((fp=open(GetEnvFilePath("OPTIONPATH", tmp, buf), O_RDWR, S_IREAD|S_IWRITE))==-1) 
		return NULL;
//	printf("end read font %s\n",tmp);//treckle
	len=lseek(fp, 0, SEEK_END);
	if(len)
	{
		DataBuf=malloc(len);
		lseek(fp, 0, SEEK_SET);
		if(read(fp, DataBuf, len)!=len)
		{
			free(DataBuf);
			DataBuf=NULL;
		}
	}
	close(fp);
	if(FileSize) *FileSize=len;
        return DataBuf;
}
