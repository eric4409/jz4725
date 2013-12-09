/*
 *	二级汉字库的解码
 */

#include "ccc.h"
#include "locale.h"
#include <stdlib.h>
#include <string.h>
#include "exfun.h"

static unsigned short *ToUCSTable;
static int ToUCSTableSize;

//取文本第一个字符的字符点阵
//Text - 查询文本
//Dots - 点阵存放的缓冲区
//DotsSize - [IN]缓冲区长度 [OUT]实际的点阵字节数
//ByteCount - 第一个文本所占的字节数
//[RET] 下一个字符的位置，若文本已经结束（为空），则返回 NULL
char* GetTextDots_CN(void *BasedLangDriver, char *Text, char *Dots, int *DotsSize, int *ByteCount)
{
	unsigned char x=*(unsigned char *)Text++;
	PLangDriver LangDriver=BasedLangDriver;	
	memset(Dots,0,32);
	if(x>=161)	//GB2312 Characters
	{
		int y, l;
		if(LangDriver->FontLib)
		{
			int index=0;
			y=*(unsigned char*)Text++-161;
			if(x>=0xB0)
				index=((x-161-6)*(254-161+1)+y);
			else
				index=((x-161)*(254-161+1)+y);
			
			if(LangDriver->FontLib->codeid==LID_GB23122)
			{
				if(x>=0xB0)
					l=index+63*(255-161);
				else
					l=index+146*(255-161);
				if(y>=0 && l<454208/32)
				{
					memcpy(Dots,LangDriver->FontLib->bits+l*32,32);
				}
			}
			else if(LangDriver->FontLib->codeid==LID_GB2312)
			{
				FullFontDots16(LangDriver->FontLib, index, (BYTE*)Dots);
			}
/*
			else if(ToUCSTable && FontLib->codeid==LID_BIG5)
			{
				unsigned short Unicode=ToUCSTable[index];
				index=QueryBig5Unicode(Unicode);
				if(index>0)
					FullFontDots16(FontLib, index, Dots);	
			}
*/
			else if(ToUCSTable && LangDriver->FontLib->codeid==LID_UNICODE2)
			{
				unsigned short Unicode=ToUCSTable[index];
				FullFontDots16(LangDriver->FontLib, Unicode, (BYTE*)Dots);
			}
		}
		*DotsSize=32;
		*ByteCount=2;
	}
	else if(x<0x80)
	{
		unsigned short Unicode=x;
		GetTextDots_UCS2(&Unicode, Dots, DotsSize, ByteCount);
	//	printf("lang_cn.c Dots: %x %x %x %x %x \n",Dots[0],Dots[1],Dots[2],Dots[3],Dots[4]);//treckle
	}
	else if(x)
	{
		*DotsSize=16;
		*ByteCount=1;
	}
	else
	{
		*ByteCount=0;
	}
	return Text;
}


PLangDriver CreateLanguage_CN(int LangID, char *FontName, int FontSize)
{
	TLangDriver *LangDriver=CreateLanguage_Default(LangID);
	LangDriver->GetNextTextFun=GetNextText_EUC;
	LangDriver->GetTextDotsFun=GetTextDots_CN;
	if(ToUCSTable==NULL)
		ToUCSTable=(unsigned short *)LoadFile("GB2312.UNI",&ToUCSTableSize);
	LangDriver->FontLib=LoadOldFont();
	if(LangDriver->FontLib==NULL)
	{
		LangDriver->FontLib=(TFontLib*)malloc(sizeof(TFontLib));
		if(LoadFontLib("GB2312.FT", LangDriver->FontLib))
		{
			LangDriver->FontLib->codeid=LID_GB2312;
		}
		else if(ToUCSTable && LoadFontLib("UNI2.FT", LangDriver->FontLib))
		{
			LangDriver->FontLib->codeid=LID_UNICODE2;
		}
		else
		{
			free(LangDriver->FontLib->buffer);
			free(LangDriver->FontLib);
			LangDriver->FontLib=NULL;
			free(LangDriver);
			LangDriver=NULL;			
		}
	}
	return LangDriver;
}

