/*
*	SJIS 日文支持
*/

#include "ccc.h"
#include "locale.h"
#include <stdlib.h>
#include <string.h>

static unsigned short *ToUCSTable;
static int ToUCSTableSize;
/*
以下字元在Shift_JIS使用一个字节来表示。

ASCII字符 (0x20-0x7E)，但“\”被“￥”(U+0x203E)取代
ASCII控制字符 (0x00-0x1F、0x7F)
JIS X 0201标准内的半角标点及片假名(0xA1-0xDF)
在部分操作系统中，0xA0用来放置“不换行空格”。

以下字元在Shift_JIS使用两个字节来表示。

JIS X 0208字集的所有字符
“第一位字节”使用0x81-0x9F、0xE0-0xEF (共47个)
“第二位字节”使用0x40-0x7E、0x80-0xFC (共188个)

使用者定义区
“第一位字节”使用0xF0-0xFC (共3个)
“第二位字节”使用0x40-0x7E、0x80-0xFC (共188个)

在Shift_JIS编码表中，并未使用0xFD、0xFE及0xFF。

在微软及IBM的日语电脑系统中，在0xFA、0xFB及0xFC的两字节区域，加入了388个JIS X 0208没有收录的符号和汉字。
*/

int ConvertUnicodeSJISIndex(char *Text)
{
	unsigned char ch1, ch2, row, col;
	ch1=*Text++;
	if(ch1<=0x80 || ch1==0x85 || ch1==0x86 || ch1==0x87 || 
		(ch1>0x9F && ch1<0xE0) || ch1>0xEA)
		return -1;
	if(ch1<0x88)
		row=ch1-0x81;
	else if(ch1<0xE0)
		row=ch1-0x81-3;
	else
		row=ch1-0x81-3-64;
	ch2=*Text;
	if(ch2<0x40)
		return -1;
	else if(ch2<0x7F) 
		col=ch2-0x40;
	else if(ch2<0x80)
		return -1;
	else if(ch2<0xFD)
		col=ch2-0x40-1;
	else
		return -1;
	return row*(0x7f-0x40+0xFD-0x80)+col;
}

//取文本第一个字符的字符点阵
//Text - 查询文本
//Dots - 点阵存放的缓冲区
//DotsSize - [IN]缓冲区长度 [OUT]实际的点阵字节数
//ByteCount - 第一个文本所占的字节数
//[RET] 下一个字符的位置，若文本已经结束（为空），则返回 NULL
char* GetTextDots_SJIS(void *BasedLangDriver, char *Text, char *Dots, int *DotsSize, int *ByteCount)
{
	int index=ConvertUnicodeSJISIndex(Text);
	unsigned char x=*(unsigned char *)Text++;
	PLangDriver LangDriver=BasedLangDriver;	
	memset(Dots,0,32);
	if(index>=0)	//SJIS Characters
	{
		if(LangDriver->FontLib)
		{
			if(LangDriver->FontLib->codeid==LID_SJIS)
			{
				FullFontDots16(LangDriver->FontLib, index, (BYTE*)Dots);
			}
			else if(ToUCSTable && LangDriver->FontLib->codeid==LID_UNICODE2)
			{
				unsigned short Unicode=ToUCSTable[index];
				FullFontDots16(LangDriver->FontLib, Unicode,(BYTE*)Dots);
			}
		}
		Text++;
		*DotsSize=32;
		*ByteCount=2;
	}
	else if(x>0xA0 && x<0xE0) //JIS X 0201标准内的半角标点及片假名(0xA1-0xDF)
	{
		unsigned short Unicode=0xFF60+x-0xA0;
		GetTextDots_UCS2(&Unicode, Dots, DotsSize, ByteCount);
	}
	else if(x<0x80)
	{
		unsigned short Unicode=x;
		GetTextDots_UCS2(&Unicode, Dots, DotsSize, ByteCount);
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

PLangDriver CreateLanguage_SJIS(int LangID, char *FontName, int FontSize)
{
	TLangDriver *LangDriver=CreateLanguage_Default(LangID);
	LangDriver->GetNextTextFun=GetNextText_EUC;
	LangDriver->GetTextDotsFun=GetTextDots_SJIS;
	if(ToUCSTable==NULL)
		ToUCSTable=(unsigned short*)LoadFile("SJIS.UNI",&ToUCSTableSize);
	LangDriver->FontLib=(TFontLib*)malloc(sizeof(TFontLib));
	if(LoadFontLib("SJIS.FT", LangDriver->FontLib))
	{
		LangDriver->FontLib->codeid=LID_SJIS;
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
	return LangDriver;
}
