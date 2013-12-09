/* 单字节字符集字库支持
	UTF8 的例程也只支持 半宽字体
 */

#include "ccc.h"
#include "locale.h"
#include "exfun.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

static BYTE *LatinCode=NULL;			//字库缓冲区
static TFontLib *UCS2FontLib=NULL;
static int FontLoaded=0;

// ISO8859_6 0xEB-0xF2是组合符号 Arabic
#define ComboReg4Start  0x610
#define ComboReg4End	0x615
#define ComboReg5Start	0x64B
#define ComboReg5End	0x65e
#define ComboReg6Start  0x6d6
#define ComboReg6End	0x6ed

// ISO8859_11 0xD1、0xD4-0xDA、0xE7-0xEE是组合符号 Thai
#define ComboReg1Start	0xE34
#define ComboReg1End	0xE3A
#define ComboReg2Start	0xE47
#define ComboReg2End	0xE4E
#define ComboReg3		0xE31

unsigned short ComboRangs[][2]={
	{0x610,0x615},
	{0x64b,0x65e},
	{0x6d6,0x6ed},
	{0xe31,0xe31},
	{0xe34,0xe3a},
	{0xe47,0xe4e},
};

#define ComboRangCount (sizeof(ComboRangs)/sizeof(ComboRangs[0]))

/* Decode 1 UTF-8 char and return a pointer to the next char. */
char* UTF8ToUCS2(char *utf8, unsigned short *ucs)
{
	unsigned char c = *(unsigned char *)utf8++;
	unsigned long code;
	int tail = 0;

	if ((c <= 0x7f) || (c >= 0xc2)) 
	{
		/* Start of new character. */
		if (c < 0x80) /* U-00000000 - U-0000007F, 1 byte */
			code = c;
		else if (c < 0xe0) 
		{ /* U-00000080 - U-000007FF, 2 bytes */
			tail = 1;
			code = c & 0x1f;		
		} 
		else if (c < 0xf0) 
		{ /* U-00000800 - U-0000FFFF, 3 bytes */
			tail = 2;
			code = c & 0x0f;	
		} 
		else if (c < 0xf5) 
		{ /* U-00010000 - U-001FFFFF, 4 bytes */
			tail = 3;
			code = c & 0x07;
		} 
		else 
		{ /* Invalid size. */
			code = 0xffff;
		}

		while (tail-- && ((c = *utf8++) != 0)) 
		{
			if ((c & 0xc0) == 0x80) 
			{ /* Valid continuation character. */
				code = (code << 6) | (c & 0x3f);
			} 
			else 
			{ /* Invalid continuation char */
				code = 0xffff;
				utf8--;
				break;
			}
		}
	} 
	else 
	{	/* Invalid UTF-8 char */
		code = 0xffff;
	}
	/* currently we don't support chars above U-FFFF */
	*ucs = (code < 0x10000) ? code : 0xffff;
	return utf8;
}

unsigned short GetCharUCS2(int LangID, unsigned short ch)
{
	unsigned short *ToUCSTable=NULL;	//当前的代码转换表
	unsigned short StartChar=0xa0;
	switch(LangID)
	{
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
		{
			int ISOCode=LangID-LID_ISO8859_1+1;

			if(ISOCode<12)
				ISOCode=(ISOCode-2)*(0x100-0xA0);
			else if(ISOCode>12)
				ISOCode=(ISOCode-3)*(0x100-0xA0);
			ToUCSTable=(unsigned short *)(LatinCode+2*ISOCode); 
			break;
		}
	case LID_KOI8_R:
		StartChar=0x80;
		ToUCSTable=(unsigned short *)(LatinCode+2*(14-1)*(0x100-0xA0)); 
		break;
	case LID_CP1250:
	case LID_CP1251:
	case LID_CP1252:
	case LID_CP1253:
	case LID_CP1254:
	case LID_CP1255:
	case LID_CP1256:
	case LID_CP1257:
	case LID_CP1258:
		StartChar=0x80;
		ToUCSTable=(unsigned short *)(LatinCode+2*(14-1)*(0x100-0xA0)+2*(LangID-LID_CP1250+1)*(0x100-0x80)); 
		break;		
	default: //LID_ISO8859_1
		return ch;
		break;
	}
	if(ch>=StartChar)
	{
		return ToUCSTable[ch-StartChar];
	}
	return ch;
}

unsigned short *StrToUCS2(int LangID, char *str)
{
	static unsigned short tmpUcs[260];
	int i=0;
	tmpUcs[259]=0;
	while(i<259)
	{
		char *Text;
		unsigned short ucs2;
		if(LID_UTF8==LangID) 
			Text=UTF8ToUCS2(str, &ucs2);
		else
		{
			ucs2=GetCharUCS2(LangID, *(BYTE*)str);
			Text=str+1;
		}
		tmpUcs[i]=ucs2;
		if(ucs2==0) break;
		i++;
		str=Text;
	}
	return tmpUcs;
}

TFontLib *LoadLatinCodeLib(int LangID)
{
	int i;
	char buffer[32];
	TFontLib *FontLib=(TFontLib*)malloc(sizeof(TFontLib));
	sprintf(buffer, "B8_%X.FT",LangID);	//单字符集 字库
	if(LoadFontLib(buffer, FontLib))
	{
		FontLib->codeid=LangID;	
		return FontLib;
	}
	LatinCode=LoadFile("LATIN.UNI", &i); //字符编码表
	if(LatinCode)
	{
		if(LoadFontLib("B8.FT", FontLib)) //Unicode 的半角字符字库
		{
			FontLib->codeid=LID_UNICODE2;
			return FontLib;
		}
	}
	else
	{
		LatinCode=LoadFontLib("ffiso.dat", FontLib); //字符编码表与 ISO10464 半角字符字库
	//	printf("Loaded ffiso.data LatinCode=%p,FontLib=%p\n",LatinCode,FontLib->buffer);//treckle
		if(LatinCode)
		{
			UCS2FontLib=FontLib;
			FontLoaded=TRUE;
		}
	}
	if(LatinCode==NULL)
	{
		FontLib=LoadOldFont();
		if(UCS2FontLib==NULL) UCS2FontLib=FontLib;
		FontLoaded=TRUE;
		return FontLib;
	}
	else
	{
		FontLib->codeid=LID_UNICODE2;
		return FontLib;
	}
	free(FontLib);
	FontLib=NULL;
	return FontLib;
}

static unsigned char *QueryFontDots(TFontLib *FontLib, unsigned short code)
{
	if(FontLib==NULL)
	{
//		printf("<<<<<<<<<<<<<<<<<<<<< QueryFontDots NULL code=%x\n",code);//treckle
		return NULL;
	}
	else if(FontLib->offset)
	{
		if(code<FontLib->size)
		{
			int Offset;
			Offset=FontLib->offset[code];
			if(Offset)
				return FontLib->bits+(Offset-1)*FontLib->width*((FontLib->height+7)/8);
		}
//		else
//			printf("<<<<<<<<<<<<Code is over flow>>>>>>>>>>>>>>> %x\n",code);//treckle
	}
	else 
	{
		
		return FontLib->bits+2*FontLib->width*(code);
	}
//	printf(">>>>>>>>>>>>>>>>>>>>> QueryFontDots NULL %x\n",code);//treckle
	return NULL;
}

unsigned char *FullFontDots(TFontLib *FontLib, unsigned short code, unsigned char *Dots, int DotsWidth)
{
	BYTE *p;
	p=QueryFontDots(FontLib, code);
	memset(Dots,0,DotsWidth*2);
	if(p)
	{
		int Width=DotsWidth;
		if(FontLib->width<DotsWidth) 
		{
			int Offs=(DotsWidth-FontLib->width+1)/2;
			Width=FontLib->width;
			memcpy(Dots+Offs, p, Width);
			memcpy(Dots+Offs+DotsWidth, p+FontLib->width, Width);
		//	if(code>='0'&&code<='9')
		//		printf("ooooooooooo Dots in font lib %x, %x\n",*p,*(p+1));	//treckle
		}
		else
		{
			memcpy(Dots, p, Width);
			memcpy(Dots+DotsWidth, p+FontLib->width, Width);
		}
	}
	return Dots;
}

unsigned char *FullFontDots8(TFontLib *FontLib, unsigned short code, unsigned char *Dots)
{
	return FullFontDots(FontLib, code, Dots, 8);
}

unsigned char *FullFontDots16(TFontLib *FontLib, unsigned short code, unsigned char *Dots)
{
	return FullFontDots(FontLib, code, Dots, 16);
}

int IsComboSymbol(unsigned short LastUCSCode, unsigned short UCSCode, int LID)
{
	int i;
	if(LID==LID_THAI)
		UCSCode+=0xE34-0xD4;
	else if(LID==LID_ISO8859_6 || LID==LID_CP1256)
		UCSCode+=0x64B-0xEB;
	for(i=0;i<ComboRangCount;i++)
		if(UCSCode>=ComboRangs[i][0] && UCSCode<=ComboRangs[i][1])
			return TRUE;
	if(LastUCSCode==0x0644)
		if(UCSCode>=0x0622 && UCSCode<=0x0627) return TRUE;
//		return (UCSCode>=ComboReg1Start && UCSCode<=ComboReg1End) ||
//			(UCSCode>=ComboReg2Start && UCSCode<=ComboReg2End) ||
//			(UCSCode>=ComboReg4Start && UCSCode<=ComboReg4End) ||
//			(UCSCode==ComboReg3);
	return FALSE;
}

typedef char *(*FunToUCS2)(char *Text, unsigned short *ucs2);

char* GetTextDotsUCS2(char *Text, char *Dots, int *DotsSize, int *WordCount, FunToUCS2 ucs2fun)
{
	TFontLib *FontLib=UCS2FontLib;
	unsigned short c2, UCSCode;
	char *Text1;
#if 0
	/* testting treckle */  
	char number;
	number=0;
	if(Text[0]>='0'&&Text[0]<='9')
	{
		printf(" 8888888  Text=%s, UCS2FontLib=%p\n",Text, UCS2FontLib); //treckle
		number=1;
	}
	//end testing
#endif
	Text=ucs2fun(Text, &UCSCode);
	//printf(" 777777  Text=%x,UCSCode=%d\n",*Text,UCSCode); //treckle 
	if(UCSCode)
	{
		memset(Dots, 0, 16);
		*DotsSize=16;
		*WordCount=1;
		if(FontLib && FontLib->codeid==LID_UNICODE2)
		{
			FullFontDots8(FontLib, UCSCode-FontLib->firstchar, (BYTE*)Dots);
		//	if(number)	//treckle
			//	printf(" 11111111  FontLib=%p,FontLib->width=%d,UCSCode-FontLib->firstchar=0x%x, UCSCode=0x%x, Dots=%x, %x, %x %x, %x\n",FontLib,FontLib->width,UCSCode-FontLib->firstchar,UCSCode,Dots[0],Dots[1],Dots[2],Dots[3],Dots[4]); //treckle
			//检查是否泰语、阿拉伯语的组合字符
			c2=UCSCode;
			Text1=ucs2fun(Text, &UCSCode);
			if(UCSCode)
			{
				if(IsComboSymbol(c2, UCSCode, LID_UNICODE2))
				{
					char AppDots[16];
					FullFontDots8(FontLib, UCSCode-FontLib->firstchar, (BYTE*)AppDots);
					memor(Dots, AppDots, 16);
					Text=Text1;
					c2=UCSCode;
					Text1=ucs2fun(Text, &UCSCode);
					*WordCount=2;
					//第二个组合字符
					if(IsComboSymbol(c2, UCSCode, LID_UNICODE2))
					{
						FullFontDots8(FontLib, UCSCode-FontLib->firstchar, (BYTE*)AppDots);
						memor(Dots, AppDots, 16);
						Text=Text1;
						*WordCount=3;
					}
				}
			}
		}
		else if(FontLib && UCSCode<0x80 && FontLib->codeid==LID_GB23122)
		{
			//printf(" 222222 Text=%s\n",Text); //treckle
			int y, l=(248-176)*(254-161+1)*32, x=UCSCode;
			x-=0x21;
			y=x / 2;
			l=l+y*32;
			l+=32*(254-161+1)*(247-176+1);
			if( x>=0)
			{
				if(y*2==x)
				{
					memcpy(Dots,FontLib->bits+l,8);
					memcpy(Dots+8,FontLib->bits+l+16,8);
				}
				else
					{
					memcpy(Dots,FontLib->bits+l+8,8);
					memcpy(Dots+8,FontLib->bits+l+24,8);
				}
			}
		}
	}
	else
	{
		*WordCount=0;
	}
	return Text;
}

char *UCS2ToUCS2(char *Text, unsigned short *ucs2)
{
	*ucs2=((BYTE*)Text)[0]+((BYTE*)Text)[1]*256;
	return Text+2;
}

unsigned short * GetTextDots_UCS2(unsigned short *Text, char *Dots, int *DotsSize, int *WordCount)
{
	if(!FontLoaded) LoadLatinCodeLib(LID_UNICODE2); //treckle
	return (unsigned short *)GetTextDotsUCS2((char*)Text, Dots, DotsSize, WordCount,UCS2ToUCS2);
}


//取文本的像素宽度
int GetTextWidth_CB(void *BasedLangDriver, char *Text)
{
	unsigned short Unicode, c2=0;
	int width=0;
	PLangDriver LangDriver=BasedLangDriver;
	while(1)
	{
		BYTE x=*(BYTE*)Text++;
		Unicode=GetCharUCS2(LangDriver->LanguageID,x);
		if(x==0) break;
		if(!IsComboSymbol(c2, Unicode, LangDriver->FontLib->codeid)) 
			width++;
		c2=Unicode;
	}
	return width*8;
}

//取超出设定宽度的下一段文本位置
char* GetNextText_CB(void *BasedLangDriver, char *Text, int Width)
{
	unsigned short Unicode, c2=0;
	 PLangDriver LangDriver=BasedLangDriver;
	while(1)
	{
		BYTE x=*(BYTE*)Text;
		Unicode=GetCharUCS2(LangDriver->LanguageID, x);
		if(x==0) return NULL;
		if(!IsComboSymbol(c2, Unicode, LangDriver->FontLib->codeid))
		{
			Width-=8;
			if(Width<0) break;
		}
		Text++;
		c2=Unicode;
	}
	return Text;
}


PLangDriver CreateLanguage_LT(int LangID, char *FontName, int FontSize)
{
	TLangDriver *LangDriver=CreateLanguage_Default(LangID);
	LangDriver->FontLib=LoadLatinCodeLib(LangID);
	if(LangDriver->FontLib)
	{
		if(LangID==LID_ISO8859_6 || LangID==LID_ISO8859_8 || LangID==LID_CP1256 || LangID==LID_CP1255)
		{
			LangDriver->Bidi=TRUE;
			LangDriver->RightToLeft=TRUE;
		}
		if((LangID==LID_ISO8859_11||LangID==LID_CP1256||LangID==LID_ISO8859_6)) //泰语和阿拉伯语有组合字符
		{
			LangDriver->GetNextTextFun=GetNextText_CB;
			LangDriver->GetTextWidthFun=GetTextWidth_CB;
		}
		return LangDriver;
	}
	else
	{
		free(LangDriver);
		return NULL;
	}
}

//取文本的像素宽度
int GetTextWidth_UCS2(unsigned short *Text)
{
	unsigned short Unicode, c2=0;
	int width=0;
	while(1)
	{
		int charwidth=1;
		Unicode=*Text++;
		if(Unicode==0) break;
		if(IsComboSymbol(c2, Unicode, LID_UNICODE2)) 
			charwidth=0;
		else if(Unicode>=0x3000 && Unicode<0xFB00) //Full Width Char
			charwidth=2;
		width+=charwidth;
		c2=Unicode;
	}
	return width*8;
}

int GetTextWidth_UTF8(void *BasedLangDriver, char *Text)
{
	return GetTextWidth_UCS2(StrToUCS2(LID_UTF8, Text));
}

//取超出设定宽度的下一段文本位置
char* GetNextText_UTF8(void *BasedLangDriver, char *Text, int Width)
{
	unsigned short Unicode, c2=0;
	while(1)
	{
		char *Text1=UTF8ToUCS2(Text, &Unicode);
		if(Unicode==0) return NULL;
		if(!IsComboSymbol(c2, Unicode, LID_UNICODE2))
		{
			if(Unicode>=0x3000 && Unicode<0xFB00) //Full Width Char
				Width-=16;
			else
				Width-=8;
			if(Width<0) break;
		}
		Text=Text1;
		c2=Unicode;
	}
	return Text;
}

char * GetTextDots_UTF8(PLangDriver LangDriver, char *Text, char *Dots, int *DotsSize, int *WordCount)
{
	return GetTextDotsUCS2((char*)Text, Dots, DotsSize, WordCount, UTF8ToUCS2);
}

PLangDriver CreateLanguage_UTF8(int LangID, char *FontName, int FontSize)
{
	TLangDriver *LangDriver=CreateLanguage_Default(LangID);
	
	LangDriver->FontLib=LoadLatinCodeLib(LangID);
	if(LangDriver->FontLib)
	{
//		LangDriver->GetTextDotsFun=GetTextDots_UTF8;
		LangDriver->GetNextTextFun=GetNextText_UTF8;
		LangDriver->GetTextWidthFun=GetTextWidth_UTF8;
		return LangDriver;
	}
	else
	{
		free(LangDriver);
		return NULL;
	}
}
