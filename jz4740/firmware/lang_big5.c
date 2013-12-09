/*
*	BIG5���ֿ�Ľ���
*/

#include "ccc.h"
#include "locale.h"
#include <stdlib.h>
#include <string.h>
#include "exfun.h"

static unsigned short *ToUCSTable;
static int ToUCSTableSize;
/*
����λ�ֹ���ʹ����0xA1-0xF9������λ�ֹ���ʹ����0x40-0x7E����0xA1-0xFE����Big5�ķօ^�У�
0xA140-0xA3BF 	���c��̖��ϣ�D��ĸ�������̖��
������0xA259-0xA261���������p�����������λ���֣����������ăņ��H��
0xA3C0-0xA3FE 	�������˅^�]���_�������օ^�á�
0xA440-0xC67E 	���Ýh�֣��Ȱ��P���ٰ���������
0xC6A1-0xC8FE 	�����oʹ�����Զ��x��Ԫ�����օ^��
0xC940-0xF9D5 	�γ��Ýh�֣������Ȱ��P���ٰ���������
0xF9D6-0xFEFE 	�����oʹ�����Զ��x��Ԫ�����օ^��
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

//ȡ�ı���һ���ַ����ַ�����
//Text - ��ѯ�ı�
//Dots - �����ŵĻ�����
//DotsSize - [IN]���������� [OUT]ʵ�ʵĵ����ֽ���
//ByteCount - ��һ���ı���ռ���ֽ���
//[RET] ��һ���ַ���λ�ã����ı��Ѿ�������Ϊ�գ����򷵻� NULL
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
