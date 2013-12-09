/*
*	CP949 朝鲜文／韩文的支持
*/

#include "ccc.h"
#include "locale.h"
#include <stdlib.h>
#include <string.h>
#include "exfun.h"
#include "utils.h"

static unsigned short *ToUCSTable;
static int ToUCSTableSize;

static unsigned short *ToJohabTable=NULL;

#define	table1_index(hi,lo)	((hi-0xB0)*(0xFF-0xA1)+(lo-0xA1))

#define tbl2c	(20*4)
#define tbl1c	(0x4860/2-tbl2c)

unsigned short ToJohab(BYTE hi, BYTE lo)
{
	int idx, wc;
	if(ToJohabTable==NULL) return 0;
	idx = table1_index(hi,lo);
	if ( idx>=0 && (idx<tbl1c) )
		return ToJohabTable[idx];
	else
	{
		wc = (hi<<8) | lo;
		for( idx=0; idx<tbl2c; )
		{
			if ( ToJohabTable[idx+tbl1c] == wc )
				return ToJohabTable[idx+1+tbl1c];
			idx += 2;
		}
	}
	return 0;
}

#define   NTYPES1   8
#define   NTYPES2   4
#define   NTYPES3   4
#define   NITEM1	19
#define   NITEM2	21
#define   NITEM3	27

int	GetHanImage(TFontLib *FontLib, int johab_code, BYTE *bitmap)
{
	BYTE Dots[32];
	static char idxtbl1[] = {
		0,  0,  1,  2,  3,  4,  5,  6,
			7,  8,  9, 10, 11, 12, 13, 14,
			15, 16, 17, 18, 19,  0,  0,  0,
			0,  0,  0,  0,  0,  0,  0,  0
	};
	static char idxtbl2[] = {
		0,  0,  0,  1,  2,  3,  4,  5,
			0,  0,  6,  7,  8,  9, 10, 11,
			0,  0, 12, 13, 14, 15, 16, 17,
			0,  0, 18, 19, 20, 21,  0,  0
	};
	static char idxtbl3[] = {
		0,  0,  1,  2,  3,  4,  5,  6,
			7,  8,  9, 10, 11, 12, 13, 14,
			15, 16,  0, 17, 18, 19, 20, 21,
			22, 23, 24, 25, 26, 27,  0,  0
	};

	static char type1tbl_no[]  = {
		0, 0, 0, 0, 0, 0, 0, 0,
			0, 1, 3, 3, 3, 1, 2, 4,
			4, 4, 2, 1, 3, 0, 0, 0
	};
	static char type1tbl_yes[] = {
		5, 5, 5, 5, 5, 5, 5, 5,
			5, 6, 7, 7, 7, 6, 6, 7,
			7, 7, 6, 6, 7, 5, 0, 0
	};
	static char type3tbl[] = {
		0, 0, 2, 0, 2, 1, 2, 1,
			2, 3, 0, 2, 1, 3, 3, 1,
			2, 1, 3, 3, 1, 1, 0, 0
	};

	unsigned short h1, h2, h3, type1, type2, type3;
	
	if(NULL==FontLib)
		return 0;
	h1 = (johab_code>>10) & 0x1f;
	h2 = (johab_code>>5)  & 0x1f;
	h3 = (johab_code)     & 0x1f;

	h1 = idxtbl1[h1];
	h2 = idxtbl2[h2];
	h3 = idxtbl3[h3];

	type1 = h3 ? type1tbl_yes[h2] : type1tbl_no[h2];
	type2 = ((h1 == 0 || h1 == 1 || h1 == 16) ? 0 : 1) + (h3 ? 2 : 0);
	type3 = type3tbl[h2];

	if (h1)	FullFontDots16(FontLib, type1*NITEM1+h1-1, bitmap);
	if (h2) 
	{
		FullFontDots16(FontLib, type2*NITEM2+h2-1+NTYPES1*NITEM1, Dots);
		memor((char*)bitmap, (char*)Dots, 32);
	}
	if (h3) 
	{
		FullFontDots16(FontLib, type3*NITEM3+h3-1+NTYPES1*NITEM1+NTYPES2*NITEM2, Dots);
		memor((char*)bitmap, (char*)Dots, 32);
	}
	return TRUE;
}

int ConvertUnicodeCP949Index(char *Text)
{
	unsigned char ch1, ch2, row, col;
	ch1=*Text++;
	if(ch1<=0x80 || ch1==0xC9)
		return -1;
	if(ch1<0xC9)
		row=ch1-0x81;
	else 
		row=ch1-0x81-1;
	ch2=*Text;
	if(ch1<0xC7)
	{
		col=ch2-0x40;
		return row*(0xff-0x40)+col;
	}
	else
	{
		col=ch2-0xA1;
		return (0xC7-0x80)*(0xFF-0x41)+(row-0xC7)*(0xFF-0xA1)+col;
	}
}

//取文本第一个字符的字符点阵
//Text - 查询文本
//Dots - 点阵存放的缓冲区
//DotsSize - [IN]缓冲区长度 [OUT]实际的点阵字节数
//ByteCount - 第一个文本所占的字节数
//[RET] 下一个字符的位置，若文本已经结束（为空），则返回 NULL
char* GetTextDots_CP949(void *BasedLangDriver, char *Text, char *Dots, int *DotsSize, int *ByteCount)
{
	int index=ConvertUnicodeCP949Index(Text);
	unsigned char x=*(unsigned char *)Text++;
	PLangDriver LangDriver=BasedLangDriver;	
	memset(Dots,0,32);
	if(index>=0)	//CP949 Characters
	{
		if(LangDriver->FontLib)
		{
			if(LangDriver->FontLib->codeid==LID_CP949)
			{
				FullFontDots16(LangDriver->FontLib, index, (BYTE*)Dots);
			}
			else if(ToUCSTable && LangDriver->FontLib->codeid==LID_UNICODE2)
			{
				unsigned short Unicode=ToUCSTable[index];
				FullFontDots16(LangDriver->FontLib, Unicode, (BYTE*)Dots);
			}
			else if(ToJohabTable && LangDriver->FontLib->codeid==LID_JOHAB)
				GetHanImage(LangDriver->FontLib, ToJohab(x, (BYTE)*Text), (BYTE*)Dots);
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

PLangDriver CreateLanguage_CP949(int LangID, char *FontName, int FontSize)
{
	TLangDriver *LangDriver=CreateLanguage_Default(LangID);
	LangDriver->GetNextTextFun=GetNextText_EUC;
	LangDriver->GetTextDotsFun=GetTextDots_CP949;
	if(ToJohabTable==NULL)
		ToJohabTable=(unsigned short *)LoadFile("JOHAB.CD", NULL);
	if(ToUCSTable==NULL)
		ToUCSTable=(unsigned short*)LoadFile("KOR.UNI",&ToUCSTableSize);
	LangDriver->FontLib=(TFontLib*)malloc(sizeof(TFontLib));
	if(LoadFontLib("KOR.FT", LangDriver->FontLib))
	{
		LangDriver->FontLib->codeid=LID_KOR;
	}
	else if(ToUCSTable && LoadFontLib("UNI2.FT", LangDriver->FontLib))
	{
		LangDriver->FontLib->codeid=LID_UNICODE2;
	}
	else if(ToJohabTable && LoadFontLib("JOHAB.FT", LangDriver->FontLib))
	{
		LangDriver->FontLib->codeid=LID_JOHAB;
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
