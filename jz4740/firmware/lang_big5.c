/*
*	BIG5汉字库的解码
*/

#include "ccc.h"
#include "locale.h"
#include <stdlib.h>
#include <string.h>
#include "exfun.h"

static unsigned short *ToUCSTable;
static int ToUCSTableSize;
/*
「高位字節」使用了0xA1-0xF9，「低位字節」使用了0x40-0x7E，及0xA1-0xFE。在Big5的分區中：
0xA140-0xA3BF 	標點符號、希臘字母及特殊符號，
另外在0xA259-0xA261，安放了雙音節度量衡單位用字：兙兛兞兝兡兣嗧瓩糎。
0xA3C0-0xA3FE 	保留。此區沒有開放作造字區用。
0xA440-0xC67E 	常用漢字，先按筆劃再按部首排序。
0xC6A1-0xC8FE 	保留給使用者自定義字元（造字區）
0xC940-0xF9D5 	次常用漢字，亦是先按筆劃再按部首排序。
0xF9D6-0xFEFE 	保留給使用者自定義字元（造字區）
*/

int BIG5Index(char *Text)
{
	unsigned char ch1, ch2, row, col;
	ch1=*Text++;
	if(ch1<0xA1 || ch1>0xF9 || ch1==0xC8)
		return -1;
	if(ch1>0xC8)
		row=ch1-0xa1-1;
	else
		row=ch1-0xa1;
	ch2=*Text;
	if(ch2<0x40) return -1;
	if(ch2>=0x7F && ch2<0xA1) return -1;
	if(ch2<0xa1)
		col=ch2-0x40;
	else
		col=ch2-0xa1+(0x7f-0x40);
	return row*(0x7F-0x40+0xFF-0xA1)+col;
}

int QueryUnicode(unsigned short *UnicodeTable, unsigned short Unicode, int TableCount)
{
	while(TableCount--)
	{
		if(UnicodeTable[TableCount]==Unicode) return TableCount;
	}
	return -1;
}

int QueryBig5Unicode(unsigned short Unicode)
{
	if(NULL==ToUCSTable)
	{
		ToUCSTable=(unsigned short*)LoadFile("BIG5.UNI",&ToUCSTableSize);
	}
	return QueryUnicode(ToUCSTable, Unicode, ToUCSTableSize/2);
}

//取文本第一个字符的字符点阵
//Text - 查询文本
//Dots - 点阵存放的缓冲区
//DotsSize - [IN]缓冲区长度 [OUT]实际的点阵字节数
//ByteCount - 第一个文本所占的字节数
//[RET] 下一个字符的位置，若文本已经结束（为空），则返回 NULL
char* GetTextDots_BIG5(void *BasedLangDriver, char *Text, char *Dots, int *DotsSize, int *ByteCount)
{
	int index=BIG5Index(Text);
	unsigned char x=*(unsigned char *)Text++;
	PLangDriver LangDriver=BasedLangDriver;
	memset(Dots,0,32);
	if(index>=0)	//BIG5 Characters
	{
		if(LangDriver->FontLib)
		{
			if(LangDriver->FontLib->codeid==LID_BIG5)
			{
				FullFontDots16(LangDriver->FontLib, index, (BYTE*)Dots);
			}
			else if(ToUCSTable && LangDriver->FontLib->codeid==LID_UNICODE2)
			{
				unsigned short Unicode=ToUCSTable[index];
				FullFontDots16(LangDriver->FontLib, Unicode, (BYTE*)Dots);
			}
		}
		Text++;
		*DotsSize=32;
		*ByteCount=2;
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


PLangDriver CreateLanguage_BIG5(int LangID, char *FontName, int FontSize)
{
	TLangDriver *LangDriver=CreateLanguage_Default(LangID);
	LangDriver->GetNextTextFun=GetNextText_EUC;
	LangDriver->GetTextDotsFun=GetTextDots_BIG5;
	if(ToUCSTable) 
		ToUCSTable=(unsigned short*)LoadFile("BIG5.UNI",&ToUCSTableSize);
	LangDriver->FontLib=(TFontLib*)malloc(sizeof(TFontLib));
	if(LoadFontLib("BIG5.FT", LangDriver->FontLib))
	{
		LangDriver->FontLib->codeid=LID_BIG5;
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
