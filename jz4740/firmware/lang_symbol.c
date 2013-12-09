/*
*	符号 字库
*/

#include "ccc.h"
#include "locale.h"
#include "exfun.h"
#include <stdlib.h>
#include <string.h>

BYTE *SymbolDots=NULL;
#define MAX_ROM_CHAR 132
#define MIN_ROM_CHAR 23

static char ROM8x8Dots[MAX_ROM_CHAR-MIN_ROM_CHAR+1][8]=
#include "chardots.txt"

PLangDriver LangDriver=NULL;
//取文本的像素高度
int GetTextHeight_ROM(void *BasedLangDriver, char *Text)
{
        return 8; 
}

//取文本的像素高度
int GetTextWidth_ROM(void *BasedLangDriver, char *Text)
{
        return strlen(Text)*6;
}

//取超出设定宽度的下一段文本位置
char* GetNextText_ROM(void *BasedLangDriver, char *Text, int Width)
{
        if((int)strlen(Text)*6>Width)
                return Text+Width/6;
        else
                return NULL;
}

char* GetTextDots_ROM(void *BasedLangDriver, char *Text, char *Dots, int *DotsSize, int *ByteCount)
{
        unsigned char x=*(unsigned char *)Text++;

        if(x<MIN_ROM_CHAR||x>MAX_ROM_CHAR)
                memcpy(Dots, ROM8x8Dots[0],8);
        else
                memcpy(Dots, ROM8x8Dots[x-MIN_ROM_CHAR],8);
        *DotsSize=8;
        *ByteCount=x>0;
        return Text;
}

char* GetTextDots_SYM(void *BasedLangDriver, char *Text, char *Dots, int *DotsSize, int *ByteCount)
{
	unsigned char x=*(unsigned char *)Text++;
	PLangDriver LangDriver=BasedLangDriver;	
	*DotsSize=16;
	*ByteCount=1;
	memset(Dots, 0, 32);
	if(x==0x12)	//双字节的菜单制表符
	{
		x=*Text++-0x12;
		if(x<6)
		{
			if(SymbolDots)
				memcpy(Dots, SymbolDots+(x*2+102)*16, 32);
		}
		*DotsSize=32;
		*ByteCount=2;
	}
	else if(x>0x12 && x<=0x12+6) //单字节的菜单制表符
	{
		x-=0x13;
		if(SymbolDots)
			memcpy(Dots, SymbolDots+(x+114)*16, 16);
	}
	else if(x==254)
	{
		if(SymbolDots)
		{
			x=*Text++;
			memcpy(Dots, SymbolDots+(x-161)*32, 32);
			*DotsSize=32;
			*ByteCount=2;
		}
		else if(LangDriver && LangDriver->FontLib && LangDriver->FontLib->codeid==LID_GB23122)
		{
			int y,l,index=0;
			y=*(unsigned char*)Text++-161;
			index=((x-161-6)*(254-161+1)+y);
			l=index+63*(255-161);
			memcpy(Dots,LangDriver->FontLib->bits+l*32,32);
			*DotsSize=32;
			*ByteCount=2;
		}
		else if(gLangDriver->FontLib && gLangDriver->FontLib->codeid==LID_GB23122)
		{
			gLangDriver->GetTextDotsFun(gLangDriver, Text-1, Dots, DotsSize, ByteCount);
		}
	}
	else if(x==0)
	{
		*ByteCount=0;
	}
	return Text;
}

//取超出设定宽度的下一段文本位置
char* GetNextText_SYM(void *BasedLangDriver, char *Text, int Width)
{
	while(1)
	{
		unsigned char ch=*Text;
		if(ch)
		{
			if(!(ch==12 || ch==254))
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

PLangDriver CreateLanguage_SYM(int LangID, char *FontName, int FontSize)
{
	if(LangDriver==NULL)
		LangDriver=(PLangDriver)malloc(sizeof(TLangDriver));
	else
		return LangDriver;
	memset(LangDriver, 0, sizeof(TLangDriver));
	LangDriver->CharHeight=16;
	LangDriver->CharWidth=8;
	LangDriver->GetNextTextFun=GetNextText_SYM;
	LangDriver->GetTextDotsFun=GetTextDots_SYM;
	LangDriver->GetTextHeightFun=GetTextHeight_Default;
	LangDriver->GetTextWidthFun=GetTextWidth_Default;
	LangDriver->LanguageID=LangID;
	if(!(SymbolDots=LoadFile("SYM.FT", &FontSize)))
		LangDriver->FontLib=LoadOldFont();
	return LangDriver;
}
//GetTextWidth_ROM

PLangDriver CreateLanguage_ROM(int LangID)
{
        PLangDriver LangDriver=(PLangDriver)malloc(sizeof(TLangDriver));
        memset(LangDriver, 0, sizeof(TLangDriver));
        LangDriver->CharHeight=8;
        LangDriver->CharWidth=8;
        LangDriver->GetNextTextFun=GetNextText_ROM;
        LangDriver->GetTextDotsFun=GetTextDots_ROM;
        LangDriver->GetTextHeightFun=GetTextHeight_ROM;
        LangDriver->GetTextWidthFun=GetTextWidth_ROM;
        LangDriver->LanguageID=LangID;
        return LangDriver;
}

