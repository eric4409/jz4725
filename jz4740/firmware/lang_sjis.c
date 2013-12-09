/*
*	SJIS ����֧��
*/

#include "ccc.h"
#include "locale.h"
#include <stdlib.h>
#include <string.h>

static unsigned short *ToUCSTable;
static int ToUCSTableSize;
/*
������Ԫ��Shift_JISʹ��һ���ֽ�����ʾ��

ASCII�ַ� (0x20-0x7E)������\����������(U+0x203E)ȡ��
ASCII�����ַ� (0x00-0x1F��0x7F)
JIS X 0201��׼�ڵİ�Ǳ�㼰Ƭ����(0xA1-0xDF)
�ڲ��ֲ���ϵͳ�У�0xA0�������á������пո񡱡�

������Ԫ��Shift_JISʹ�������ֽ�����ʾ��

JIS X 0208�ּ��������ַ�
����һλ�ֽڡ�ʹ��0x81-0x9F��0xE0-0xEF (��47��)
���ڶ�λ�ֽڡ�ʹ��0x40-0x7E��0x80-0xFC (��188��)

ʹ���߶�����
����һλ�ֽڡ�ʹ��0xF0-0xFC (��3��)
���ڶ�λ�ֽڡ�ʹ��0x40-0x7E��0x80-0xFC (��188��)

��Shift_JIS������У���δʹ��0xFD��0xFE��0xFF��

��΢��IBM���������ϵͳ�У���0xFA��0xFB��0xFC�����ֽ����򣬼�����388��JIS X 0208û����¼�ķ��źͺ��֡�
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

//ȡ�ı���һ���ַ����ַ�����
//Text - ��ѯ�ı�
//Dots - �����ŵĻ�����
//DotsSize - [IN]���������� [OUT]ʵ�ʵĵ����ֽ���
//ByteCount - ��һ���ı���ռ���ֽ���
//[RET] ��һ���ַ���λ�ã����ı��Ѿ�������Ϊ�գ����򷵻� NULL
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
	else if(x>0xA0 && x<0xE0) //JIS X 0201��׼�ڵİ�Ǳ�㼰Ƭ����(0xA1-0xDF)
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
