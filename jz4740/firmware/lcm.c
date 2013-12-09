/*************************************************
                                           
 ZEM 200                                          
                                                    
 lcm.c LCD output functions support multi-language                               
                                                      
 Copyright (C) 2003-2006, ZKSoftware Inc.      		
                                                      
*************************************************/
/*
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>*/
#include "arca.h"
#include "lcm.h"
#include "utils.h"
#include "options.h"
#include "ccc.h"
#include "locale.h"

#define BIDI

//COMMAND
#define  LCD_DISPLAY_ON          0x01
#define  LCD_DISPLAY_OFF         0x02
#define  LCD_RESET               0x03
#define  LCD_DISPLAY_LINE_START  0x04
#define  LCD_WRITE_DATA          0x05
#define  LCD_READ_DATA           0x06
#define  LCD_SET_PAGE            0x07
#define  LCD_SET_ADDRESS         0x08
#define  LCD_READ_STATE          0x09

static int LCDBuffered=LCD_BUFFER_OFF;

static BYTE **LCDBuffer=NULL;

int gLCDWidth=128, gLCDHeight=64, gLCDCharWidth=16, gLCDRowCount=4, gRowHeight=16;

static int LCD1_WIDTH=0;

void ExGetLCDSize(int *lcdWidth, int *lcdheight)
{
	if(gOptions.TwoLineLCM)
	{
		*lcdWidth=122;
		*lcdheight=32;
	}
	else
	{
		*lcdWidth=128;
		*lcdheight=64;
	}
}

void LCD_Final(void)
{
	int RowCount;
	RowCount=gLCDHeight/8;
	if(LCDBuffer && (RowCount>0)) 
		free(LCDBuffer);
	LCDBuffer=NULL;
}

int LCDInit(void)
{
	int i, RowCount,LCDWidth, LCDHeight;
	LCD_Final();

	ExGetLCDSize(&LCDWidth, &LCDHeight);
	
	gLCDWidth=LCDWidth;
	gLCDHeight=LCDHeight;
	gRowHeight=DEFAULT_ROW_HEIGHT;
	gLCDRowCount=LCDHeight/DEFAULT_ROW_HEIGHT;
	gLCDCharWidth=LCDWidth/8;
	RowCount=LCDHeight/8;
	LCDBuffer=(BYTE**)malloc(RowCount*sizeof(BYTE*)+RowCount*2*LCDWidth);
	LCDBuffer[0]=(BYTE*)LCDBuffer+RowCount*sizeof(BYTE*);
	memset(LCDBuffer[0],0,RowCount*LCDWidth*2);
	for(i=1;i<RowCount;i++)
	{
		LCDBuffer[i]=LCDBuffer[i-1]+2*LCDWidth;
	}
//	ExClearLCD();
	return TRUE;
}
                                                                                                                                                         
//Open LCM
int ExLCDOpen(void)
{
	/* Initialize the port of lcd for hardware */
//	InitializeLCD();
	/* Initialize the parameter for application */
	oled_init();
	LCDInit();
	return TRUE;
}

void DrawProgbar(int pos)
{
	int i, j;
	int start = 7;

	for (i = start; i < 8; i++) 
	{
		memset(LCDBuffer[i], 0, 128);
		for (j = 0; j < pos*2; j++)
			LCDBuffer[i][j] = 0xff;
	}
	LCDInvalid();
}

//Close LCM
void ExLCDClose(void)
{
	LCD_Final();
}

void ExClearLCD(void)
{
/*	int i, cs, k, j;
    
	for(k=0; k<2; k++)
	{
		for(i=0; i<gLCDHeight/8; i++)
		{
			//ROW
			cs = 0x00010000 + (k<<16);
			cs += i;
			lcd_ioctl(LCD_SET_PAGE, &cs); 
			//COL
			cs = 0x00010000 + (k<<16);
			lcd_ioctl(LCD_SET_ADDRESS, &cs);      
			for(j=0; j<gLCDWidth/2; j++)
			{
				cs = 0x00010000 + (k<<16);
				lcd_ioctl(LCD_WRITE_DATA, &cs);
			}  
		}
	}
*/
	char buf[4];
        buf[0]=1;buf[1]=1;buf[2]=1;buf[3]=1;
        //ExEnableClock(FALSE);
        //RESETEXCMD;

  //      if(gOLED_En != 1)       ExCommand(2,buf,4,10);
  //      else
        OLED_Clear();

}

void ExPutPixelBuffer16H(int row, int col, unsigned char *Buffer)
{
	/*int i, cs, j;
	int lcd_1_2;
	int sign;
	
	//if(row>=gLCDRowCount*2) return;
	if(!LCD1_WIDTH) LCD1_WIDTH=gLCDWidth/2;
	sign=0;	
	for(j=0; j<2; j++)
	{	
		col += j;
		for(i=0; i<8; i++)
		{
			if((col*8+i)>(gLCDWidth-1)) break;
			if((col*8+i)>=LCD1_WIDTH)
				lcd_1_2 = 0x00010000 + (1<<16);
			else	
				lcd_1_2 = 0x00010000 + (0<<16);			
			if(sign!=lcd_1_2)
			{
				//set page
				cs = lcd_1_2 + row + j;
				lcd_ioctl(LCD_SET_PAGE, &cs);
				//set address
				if ((col*8+i)>=LCD1_WIDTH)
					cs = lcd_1_2 + col*8 + i - LCD1_WIDTH;
				else
					cs = lcd_1_2 + col*8 + i;
				lcd_ioctl(LCD_SET_ADDRESS, &cs);
				sign=lcd_1_2;
			}
			//write data
			cs = lcd_1_2 | ((int)Buffer[i+j*8]);
			lcd_ioctl(LCD_WRITE_DATA, &cs);
		}
	}*/
	char Buf[20];
        int i;

        unsigned int    r,c;

        r = row;
        c = col;
        if(row>7) return;
        if(col>15) return;
        Buf[3]=col+row*16;
        memcpy(Buf+4, Buffer, 4*4);
//      for(i=0;i<4;i++)
//              ((U32*)(Buf+4))[i]=((U32*)Buffer)[i];
        //i=(gLCDRowCount<4)?7:(MCUVersion>3)?4:(MCUVersion>1)?5:7;
        //CHECKDELAY(i)
//      ExCommand(11, Buf+3, 17, i);
        //if(gOLED_En != 1)
         //       ExCommand(11, Buf+3, 17, i);
        //else{
                OLED_ShowImg16_H(r, c*8, Buf+4);
                //DebugOutput("show OLed\n");
        //}
}

//在LCM的row行col列显示Buffer指向的半角字符(16Bytes 16Dot*8Dot) 8*8 verical
void ExPutPixelBuffer16(int row, int col, unsigned char *Buffer)
{
/*
        int i, cs, j;
        int lcd_1_2;
	int sign;

	//if(row>=gLCDRowCount) return;
	if(!LCD1_WIDTH) LCD1_WIDTH=gLCDWidth/2;        
	for(j=0; j<2; j++)
        {
		sign=0;
                for(i=0; i<8; i++)
                {
                        if((col*8+i)>(gLCDWidth-1)) break;
                        if((col*8+i)>=LCD1_WIDTH)
                                lcd_1_2 = 0x00010000 + (1<<16);
                        else
                                lcd_1_2 = 0x00010000 + (0<<16);
			if(sign!=lcd_1_2)
			{
                        	//set page
                        	cs = lcd_1_2 + row + j;
                        	lcd_ioctl(LCD_SET_PAGE, &cs);
                        	//set address
                        	if ((col*8+i)>=LCD1_WIDTH)
                                	cs = lcd_1_2 + col*8 + i - LCD1_WIDTH;
                        	else
                                	cs = lcd_1_2 + col*8 + i;
                        	lcd_ioctl(LCD_SET_ADDRESS, &cs);
				sign=lcd_1_2;
			}
                        //write data
                        cs = lcd_1_2 | ((int)Buffer[i+j*8]);
                        lcd_ioctl(LCD_WRITE_DATA, &cs);
                }
        }
*/
        char Buf[20];
        int i;
        unsigned int    r,c;

        r = row;
        c = col;
        if(row>7) return;
        if(col>15) return;      
        Buf[3]=col+row*16;
        memcpy(Buf+4, Buffer, 4*4);
//      for(i=0;i<4;i++)
//              ((U32*)(Buf+4))[i]=((U32*)Buffer)[i];
        i=8;
        //CHECKDELAY(i)

        //if(gOLED_En != 1)ExCommand(12, Buf+3, 17, i);
        //else       {
                OLED_ShowImg16_V(r, c*8, Buf+4); 
        //      DebugOutput("show OLed\n");
        //}

}

//在LCM的row行col列显示Buffer指向的汉字或者全角字符(32Bytes 16Dot*16Dot)
void ExPutPixelBuffer32(int row, int col, unsigned char *Buffer)
{
    char Buf[16];
  
    memcpy(Buf, Buffer+8, 8);
    memcpy(Buffer+8, Buffer+16, 8);
    memcpy(Buf+8, Buffer+24, 8);

    ExPutPixelBuffer16(row*2, col, Buffer);
    ExPutPixelBuffer16(row*2, col+1, Buf);
}

void LCD_ShowImg(int row, int col, int bytes, unsigned char *Buffer)
{
/*
	int i, cs;
	int lcd_1_2;

	if(!LCD1_WIDTH) LCD1_WIDTH=gLCDWidth/2;
	
	if((col*8+1) > LCD1_WIDTH)
		lcd_1_2 = 0x00010000 + (1<<16);
	else
		lcd_1_2 = 0x00010000 + (0<<16);
	//set page
	cs = lcd_1_2 + row;
	lcd_ioctl(LCD_SET_PAGE, &cs);
	//set address
	if((col*8+1) > LCD1_WIDTH)
		cs = lcd_1_2 + col*8 - LCD1_WIDTH;
	else
		cs = lcd_1_2 + col*8;
	lcd_ioctl(LCD_SET_ADDRESS, &cs);
   
	for(i=0; i<bytes; i++)
	{
		cs = lcd_1_2 | ((int)Buffer[i]);
		lcd_ioctl(LCD_WRITE_DATA, &cs);
	}
*/
}

void ExLCDShowDot(BOOL b_Second)
{
	// ':'
	BYTE Dot32[]={
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x0c,0x1e,0x1e,0x0c,0x00,0x00,
		0x00,0x00,0x00,0x00,0x80,0xc0,0xc0,0x80,0x00,0x00,
		0x00,0x00,0x00,0x00,0x01,0x03,0x03,0x01,0};
    
	if(b_Second)
	{
		LCD_ShowImg(3, 7, 8, (Dot32+8-2));
		LCD_ShowImg(4, 7, 8, (Dot32+16));
		LCD_ShowImg(5, 7, 8, (Dot32+24+2));
		LCD_ShowImg(3, 8, 2, (Dot32+8-2+8));
		LCD_ShowImg(4, 8, 2, (Dot32+16+8));
		LCD_ShowImg(5, 8, 2, (Dot32+24+2+8));
	}
	else
	{
		BYTE *buf=Dot32;
 
		LCD_ShowImg(3,7,8,buf);buf=Dot32;
		LCD_ShowImg(4,7,8,buf);buf=Dot32;
		LCD_ShowImg(5,7,8,buf);buf=Dot32;
		LCD_ShowImg(3,8,8,buf);buf=Dot32;
		LCD_ShowImg(4,8,8,buf);buf=Dot32;
		LCD_ShowImg(5,8,8,buf);
	}
}

void LCDBufferStart(int OnOff)	//开始缓冲区操作，直到LCDInvalid调用才显示
{
	if(LCDBuffered!=OnOff)
	{
		LCDBuffered=OnOff;
		if(OnOff==LCD_BUFFER_OFF)	
			LCDInvalid();
	}
}

void LCDClear(void)		//直接清除缓冲区和LCD
{
	int i;
	LCD_Clear();
	for(i=0;i<gLCDHeight/8;i++)
	{
		memset(LCDBuffer[i], 0, 2*gLCDWidth);
	}
	ExClearLCD();
}

void LCDInvalid(void)	//重新绘制缓冲区数据，使得LCD的显示反映缓冲区真实内容
{
	int i, j, LCDCharWidth, RowCount;
//	BYTE **LCDGrid;
	BYTE *LCDGrid[160];

	if(gOptions.LCDModify && gLCDWidth==128)
	{
		for(i=128-8;i<128;i++)
			LCDBuffer[7][i]&=0x7F;
	}

	LCDCharWidth=(gLCDWidth+7)/8;
	RowCount=gLCDHeight/8;
//	LCDGrid=(BYTE**)malloc(RowCount*sizeof(BYTE*)+LCDCharWidth*RowCount);
	LCDGrid[0]=(BYTE*)LCDGrid+RowCount*sizeof(BYTE*);
	for(i=1;i<RowCount;i++)
		LCDGrid[i]=LCDGrid[i-1]+LCDCharWidth;
	memset(LCDGrid[0],0,LCDCharWidth*RowCount);
	for(i=0;i<RowCount;i++)
	{
		for(j=0;j<gLCDWidth;j++)
			if(LCDBuffer[i][j]!=LCDBuffer[i][j+gLCDWidth]) LCDGrid[i][j/8]=1;
	}
	for(i=0;i<(RowCount)/2;i++)
	{
		for(j=0;j<gLCDWidth/8-1;j++)
		{
			//if(1 || (LCDGrid[i*2][j] && LCDGrid[i*2+1][j+1]) ||
			if((LCDGrid[i*2][j] && LCDGrid[i*2+1][j+1]) ||
				(LCDGrid[i*2][j+1] && LCDGrid[i*2+1][j]))
			{
				char Dots[32];
				memcpy(Dots, LCDBuffer[i*2]+j*8, 16);
				memcpy(Dots+16, LCDBuffer[i*2+1]+j*8, 16);
				ExPutPixelBuffer32(i,j,Dots);
				LCDGrid[i*2][j]=0;
				LCDGrid[i*2][j+1]=0;
				LCDGrid[i*2+1][j]=0;
				LCDGrid[i*2+1][j+1]=0;
			}
		}
	}
	for(i=0;i<RowCount-1;i++)
	{
		for(j=0;j<LCDCharWidth;j++)
		{
			if(LCDGrid[i][j] && LCDGrid[i+1][j])
			{
				char Dots[32];
				memcpy(Dots, LCDBuffer[i]+j*8, 8);
				memcpy(Dots+8, LCDBuffer[i+1]+j*8, 8);
				ExPutPixelBuffer16(i,j,Dots);
				LCDGrid[i][j]=0;
				LCDGrid[i+1][j]=0;
			}
		}
	}
	for(i=0;i<RowCount;i++)
	{
		for(j=0;j<LCDCharWidth-1;j++)
		{
			if(LCDGrid[i][j] || (((j==LCDCharWidth-2)||(j==LCDCharWidth/2-2)) && LCDGrid[i][j+1]))
			{
				ExPutPixelBuffer16H(i,j,(char*)LCDBuffer[i]+j*8);
				LCDGrid[i][j]=0;
				LCDGrid[i][j+1]=0;
			}
		}
	}
//	free(LCDGrid);
	for(i=0;i<RowCount;i++)
		memcpy(LCDBuffer[i]+gLCDWidth, LCDBuffer[i], gLCDWidth);
}

int LCD_OutDotsX(int Row, int x, BYTE *Dots, int Width)
{
	if(Row>=0 && Row<gLCDHeight/8 && x>=0)
	{
		int c=gLCDWidth-x;
		if(c>Width) c=Width;
		if(c>0)
		{
			memcpy(LCDBuffer[Row]+x,Dots,c);
			return TRUE;
		}
	} 
	return FALSE;	
}

int LCD_OutDots16(int Row, int x, BYTE *Dots, int CharWidth)
{
	if(LCD_OutDotsX(Row, x, Dots, CharWidth))
		return LCD_OutDotsX(Row+1, x, Dots+CharWidth, CharWidth);
	else
		return FALSE;
}
/*
int LCD_Line(int x1, int y1, int x2, int y2)
{
	const int p=100;
	const int pHalf=p / 2;
	int   i,j,dx,dy,d,di,dj,Result;

	di = y2 - y1;
	dj = x2 - x1;

	Result = 0;
	if((di == 0) && (dj == 0)) return Result;

	if(abs(dj) > abs(di))
	{
		if(dj>0) dx=1; else dx=-1;
		d = di * p / abs(dj);
		i = y1;
		dy = 0;
		j = x1;
		while(j != x2)
		{
			LCD_SetPixel(j,i);
			Result++;
			dy += d;
			if(dy >= pHalf) { dy -= p; i++; }
			if(dy <= -pHalf) { dy += p; i--; }
			j += dx;
		}
	}
	else
	{
		if(di>0) dy=1; else dy=-1;
		d = dj * p / abs(di);
		j = x1;
		dx = 0;
		i = y1;
		while(i != y2)
		{
			LCD_SetPixel(j,i);
			Result++;
			dx += d;
			if(dx >= pHalf) { dx -= p; j++; }
			if(dx <= -pHalf) { dx += p; j--; }
			i += dy;
		}
	}
	return Result;
}
*/
//new LCD_LINE 2006.08

#define SetPixelLng(x,y)        \
        if(gLangDriver && gLangDriver->RightToLeft)\
                LCD_SetPixel(gLCDWidth-(x),y); \
        else\
                LCD_SetPixel(x,y)\

int LCD_Line(int x1, int y1, int x2, int y2)
{
        const int p=100;
        const int pHalf=p / 2;
        int   i,j,dx,dy,d,di,dj,Result; 
        di = y2 - y1;
        if(di<0)
        {
                di=y2;y2=y1;y1=di;di=y2-y1;         }
        dj = x2 - x1;
        if(dj<0)
        {
                dj=x2;x2=x1;x1=dj;dj=x2-x1;
        }

        Result = 0;
        if((di == 0) && (dj == 0)) return Result;

        if(abs(dj) > abs(di))
        {
                if(dj>0) dx=1; else dx=-1;
                d = di * p / abs(dj);
                i = y1;
                dy = 0;
                j = x1;
                while(j <= x2)
                {
                        SetPixelLng(j,i);
                        Result++;
                        dy += d;
                        if(dy >= pHalf) { dy -= p; i++; }
                        if(dy <= -pHalf) { dy += p; i--; }
                        j += dx;
                }
        }
        else
        {
                if(di>0) dy=1; else dy=-1;
                d = dj * p / abs(di);
                j = x1;
                dx = 0;
                i = y1;
                while(i <= y2)
                {
                        SetPixelLng(j,i);
                        Result++;
                        dx += d;
                        if(dx >= pHalf) { dx -= p; j++; }
                        if(dx <= -pHalf) { dx += p; j--; }
                        i += dy;
                }
        }
        return Result;
}

int LCD_Rectangle(int x1, int y1, int x2, int y2)
{
	LCD_Line(x1,y1,x2,y1);
	LCD_Line(x2,y1,x2,y2);
	LCD_Line(x2,y2,x1,y2);
	LCD_Line(x1,y2,x1,y1);
	return TRUE;
}

void LCDFullALine(int row, char *hint)
{
	int c=0;
	char p1[MAX_CHAR_WIDTH], *p2=NULL;
	strcpy(p1,hint);
	TrimStr(p1);
	c=strlen(p1);
	while(c--)
	{
		char x=hint[c];
		if(x==0x20) 
		{
			p2=p1+c+1;
			break;
		}
	}
	if(p2==NULL)
		LCDWriteCenterStr(row, hint);
	else
	{
		char p3[MAX_CHAR_WIDTH];
		p1[c]=0;
		strcpy(p3,p2);
		PadRightStrStr(p1, p1, p3, gLCDCharWidth);
		LCDWriteCenterStr(row, p1);
	}
}

void fourPoint(int cx, int cy, int x, int y)
{
	LCD_SetPixel(cx+x,cy+y);
	LCD_SetPixel(cx+x,cy-y);
	LCD_SetPixel(cx-x,cy+y);
	LCD_SetPixel(cx-x,cy-y);
}

void circlePoint(int cx, int cy, int x, int y)
{
	fourPoint(cx, cy, x, y);
	fourPoint(cx, cy, y, x);
}

int LCD_Circle(int cx, int cy, int r)
{
	int d,x,y;
	if(r<=0) return FALSE;
	d=1-r;
	x=0;
	y=r;
	while(x<=y)
	{
		circlePoint(cx,cy,x,y);
		if(d<=0)
			d+=2*x+3;
		else
		{
			d+=2*(x-y)+5;
			y--;
		}
		x++;
	}
	return TRUE;
}

int LCD_Triangle(int TopX, int TopY, int Height, int Direction)
{
	int i, startx, starty, endx, endy, dx, dy;
	switch(Direction) 
	{
	case TriDir_Left:	dx=-1;dy=0;	break;
	case TriDir_Right:	dx=1;dy=0;	break;
	case TriDir_Top	:	dx=0;dy=-1;	break;
	default:			dx=0;dy=1;	break;
	}
	startx=TopX; starty=TopY; endx=TopX; endy=TopY;
	for(i=0;i<Height;i++)
	{
		int x=startx, y=starty;
		while(1)
		{
			LCD_SetPixel(x,y); if(x==endx && y==endy) break;
			y-=dx; x-=dy;
		}
		startx+=dx+dy;endx+=dx-dy;
		starty+=dx+dy;endy+=dy-dx;
	}
	return TRUE;
}

int LCD_Ellipse(int x1, int y1, int x2, int y2)
{
	int d1,d2;
	int x=0,y;
	int a,b,cx,cy;
	cx=(x1+x2)/2;
	cy=(y1+y2)/2;
	a=abs(x2-x1)/2;
	b=abs(y2-y1)/2;
	y=b;
	d1=4*b*b+a*a*(1-4*b);
	fourPoint(cx,cy,x,y);
	while( b*b*(x+1)*2 < a*a*(2*y-1) )
	{
		if(d1<=0)
		{
			d1+=4*b*b*(2*x+3);
			x++;
		}
		else
		{
			d1+=4*b*b*(2*x+3)+4*a*a*(2-2*y);
			x++;
			y--;
		}
		fourPoint(cx,cy,x,y);
	}
	d2=b*b*(4*x+1)*(4*x+1)+16*a*a*(y-1)*(y-1)-16*a*a*b*b;
	while(y>0)
	{
		if(d2<=0)
		{
			d2+=16*b*b*(2*x+2)+16*a*a*(3-2*y);
			x++;
			y--;
		}
		else
		{
			d2+=16*a*a*(3-2*y);
			y--;
		}
		fourPoint(cx,cy,x,y);
	}

	return TRUE;
}

void LCD_Clear(void)
{
	int i;
	for(i=0;i<gLCDHeight/8;i++)
	{
		memset(LCDBuffer[i],0,gLCDWidth);
	}
}

int XY2Index(int x, int y)
{
	int row=y/8;
	y-=row*8;
	x=(row*gLCDWidth*2+x)*8+y;
	return x;
}

int LCD_SetPixel(int x, int y)
{
	int Index=XY2Index(x,y);
	if(Index>=0 && Index<gLCDWidth*gLCDHeight*2)
	{
		SetBit(LCDBuffer[0], Index);
		return TRUE;
	}
	else
		return FALSE;
}

int LCD_ClearPixel(int x, int y)
{
	int Index=XY2Index(x,y);
	if(Index>=0 && Index<gLCDWidth*gLCDHeight*2)
	{
		ClearBit(LCDBuffer[0], Index);
		return TRUE;
	}
	else
		return FALSE;
}

void LCDClearLine(int line)
{
	LCDWriteLine(line, "");
}

void LCDWriteStrID(int row, int col, int StrID)
{
	LCDWriteStr(row,col,LoadStrByID(StrID),0);
}

void LCDWriteCenterStrIDNew(int row, int StrID, int flag)
{
	LCDWriteCenterStrNew(row, LoadStrByID(StrID), flag);
}

void LCDWriteCenterStrID(int row, int StrID)
{
	LCDWriteCenterStr(row, LoadStrByID(StrID));
}

void LCDWriteLineStrID(int row, int StrID)
{
	char buf[100];
	char *tmp=LoadStrByID(StrID);
	if(tmp)
		sprintf(buf, "%-80s", tmp);
	else
		sprintf(buf, "%-80s", " ");		
	LCDWriteStr(row, 0, buf, 0);
}

void LCDWriteLine(int row, char *Str)
{
	char buf[100];
	sprintf(buf,"%-80s",Str);
	LCDWriteStr(row,0,buf,0);
}

void LCDWriteCenterStrNew(int row, char *str, int flag)
{
	LCDClearLine(row);
	if(str) LCDWriteCenterNew(row, str, flag);
}

void LCDWriteCenterStr(int row, char *str)
{
	LCDClearLine(row);
	if(str) LCDWriteCenter(row, str);
}

void LCDWriteStr(int row, int col, char *s, int flag)
{
	LCDWriteStrLng(gLangDriver, row, col, s, flag);
}

int LCDPutPixelBuffer8(int row, int col, char *cps, int flag, int CharWidth)
{
	int i;
	if(flag & LCD_BOTTOM_LINE)
	{
		for(i=0;i<CharWidth;i++)
			cps[i]|=0x80;
	}
	if(flag & LCD_TOP_LINE)
	{
		for(i=0;i<CharWidth;i++)
			cps[i]|=1;
	}
	if(flag & LCD_RIGHT_LINE)
	{
		cps[CharWidth-1]=(char)0xff;
	}
	if(flag & LCD_LEFT_LINE)
	{
		cps[0]=(char)0xff;
	}
	if(flag & LCD_HIGH_LIGHT)
	{
		for(i=0;i<8;i++)
			cps[i]=~cps[i];
	}
	LCD_OutDotsX(row, col*CharWidth, (BYTE*)cps, CharWidth);
	return 1;
}

int LCDPutPixelBuffer(int row, int col, char *cps, int flag, int CharWidth)
{
	int i;
	if(flag & LCD_BOTTOM_LINE)
	{
		for(i=CharWidth*2;i<CharWidth*4;i++)
			cps[i]|=0x80;
	}
	if(flag & LCD_TOP_LINE)
	{
		for(i=0;i<CharWidth*2;i++)
			cps[i]|=1;
	}
	if(flag & LCD_RIGHT_LINE)
	{
		cps[CharWidth*2-1]=(char)0xff;cps[CharWidth*4-1]=(char)0xff;
	}
	if(flag & LCD_LEFT_LINE)
	{
		cps[0]=(char)0xff;cps[CharWidth*2]=(char)0xff;
	}
	if(flag & LCD_HIGH_LIGHT)
	{
		for(i=0;i<CharWidth*4;i++)
			cps[i]=~cps[i];
	}
	LCD_OutDots16(row, col*CharWidth, (BYTE*)cps, CharWidth*2);
	return 1;
}

int LCDPutPixelBuffer16(int row, int col, char *cps, int flag, int CharWidth)
{
	int i;
	if(flag & LCD_BOTTOM_LINE)
	{
		for(i=CharWidth;i<CharWidth*2;i++)
			cps[i]|=0x80;
	}
	if(flag & LCD_TOP_LINE)
	{
		for(i=0;i<CharWidth;i++)
			cps[i]|=1;
	}
	if(flag & LCD_RIGHT_LINE)
	{
		cps[CharWidth-1]=(char)0xff;cps[CharWidth*2-1]=(char)0xff;
	}
	if(flag & LCD_LEFT_LINE)
	{
		cps[0]=(char)0xff;cps[CharWidth]=(char)0xff;
	}
	if(flag & LCD_HIGH_LIGHT)
	{
		for(i=0;i<CharWidth*2;i++)
			cps[i]=~cps[i];
	}
	LCD_OutDots16(row, col*CharWidth, (BYTE*)cps, CharWidth);
	return 1;
}

void _LCDWriteStrLngDelay(PLangDriver Lng, int row, int col, char *string, int flag)
{
	int Theflag, r, mcol;
	BYTE cps[32];
	char S[MAX_CHAR_WIDTH], *s=S,*p=s;
	unsigned short *ucs=NULL;

	if(Lng==NULL) return ;
        strncpy(S,string,MAX_CHAR_WIDTH);

	if(gLCDHeight/Lng->CharHeight<=row) return;

//      if(0) //尝试用镜像法解决从左到右的显示问题
        if(Lng->RightToLeft)
        {
                int len;
                p=Lng->GetNextTextFun(Lng, s, gLCDWidth-col*Lng->CharWidth);
                if(p)
                {
                        int len=p-s;
                        char *ps=string+len-1, *sp=s;
                        *p=0;
                        while(p>sp)
                        {
                                if(' '==*ps--)
                                        *sp++=' ';
                                else
                                        break;
                        }
                        ps=string;
                        while(ps<string+len)
                                if(*ps!=' ')
                                        break;
                                else
                                        ps++;
                        while((sp<p) && (ps<string+len))
                        {
                                *sp++=*ps++;
                        }
                        while(sp<p)
                                *sp++=' ';
                }
                p=s;

                len=Lng->GetTextWidthFun(Lng, s)/Lng->CharWidth;
                col=gLCDWidth/Lng->CharWidth-col-len;
        }

	if(Lng->GetTextDotsFun==NULL)
	{//To UCS2
		unsigned short *tmpUCS=StrToUCS2(Lng->LanguageID, s);
		unsigned short *bidi_l2v(const unsigned short *uscbuf, int orientation);
		if(Lng->Bidi)
		{
			ucs=bidi_l2v(tmpUCS,1);
		}
		else
			ucs=tmpUCS;
	}

	r=row*Lng->CharHeight/8;
	while(1)
	{
		int DotsSize=32; 
		int ByteCount;

		//左边线只在第一个字符显示，右边线只在最后一个字符显示
		Theflag=flag & ~(LCD_LEFT_LINE | LCD_RIGHT_LINE);
		if(s==p)
			Theflag=Theflag | (LCD_LEFT_LINE & flag);
		if(s[1]==0)
			Theflag=Theflag | (LCD_RIGHT_LINE & flag);
		if(ucs)
			ucs=GetTextDots_UCS2(ucs, (char*)cps, &DotsSize, &ByteCount);
		else
			s=Lng->GetTextDotsFun(Lng, s, (char*)cps, &DotsSize, &ByteCount);
		if(ByteCount<=0) break;
		DotsSize/=16;
//		if(Lng->Bidi) 
//			mcol=gLCDCharWidth-1-col; 
//		else
			mcol=col;
		if(DotsSize==0)
		{
			LCDPutPixelBuffer8(r,mcol,(char*)cps, Theflag, Lng->CharWidth);
			col++;
		}
		else if(DotsSize==1)
		{
			LCDPutPixelBuffer16(r,mcol,(char*)cps, Theflag, Lng->CharWidth);
			col++;
		}
		else
		{
			LCDPutPixelBuffer(r,mcol,(char*)cps, Theflag, Lng->CharWidth);
			col+=2;
		}
/*		if(col>=gLCDCharWidth)
		{
			if(!(flag & LCD_WRAP))break;
			col=0;
			row++;
			r=row*Lng->CharHeight/8;
			if(row==gLCDRowCount)break;
		}
*/
	        if(col>=gLCDWidth/Lng->CharWidth)
                {
                        if(!(flag & LCD_WRAP))break;
                        col=0;
                        row++;
                        r=row*Lng->CharHeight/8;
                        if(row==gLCDHeight/Lng->CharHeight)break;
                }
	}
}

void LCDWriteStrLng(PLangDriver Lng, int row, int col, char *s, int flag)
{
	_LCDWriteStrLngDelay(Lng, row, col, s, flag);
	if(LCDBuffered==LCD_BUFFER_OFF)	LCDInvalid();
}

void LCDWriteCenterNew(int Row, char *Text, int flag)
{
	if(gLangDriver)
	{
		int i=gLangDriver->GetTextWidthFun(gLangDriver, Text)/gLangDriver->CharWidth;
		i=(gLCDCharWidth-2-i)/2+1;
		LCDWriteStr(Row, i, Text, flag);
	}
}

void LCDWriteCenter(int Row, char *Text)
{
	if(gLangDriver)
	{
		int i=gLangDriver->GetTextWidthFun(gLangDriver, Text)/gLangDriver->CharWidth;
		i=(gLCDCharWidth-2-i)/2+1;
		LCDWriteStr(Row, i, Text, 0);
	}
}

char* PadMidStrStr(char *buf, char *Str, char *Value, int Width)
{
        char *p, vbuf[40];
        int vwidth;
        int lwidth;
        if(Value) strcpy(vbuf,Value);
        vwidth=gLangDriver->GetTextWidthFun(gLangDriver,TrimRightStr(vbuf))/gLangDriver->CharWidth;
        sprintf(buf, "%s%20s", Str, " ");
        lwidth=Width-vwidth;
        if(lwidth<0) lwidth=0;
        p=gLangDriver->GetNextTextFun(gLangDriver, buf, lwidth*gLangDriver->CharWidth);
        if(p && Value)
                sprintf(p,"%s", vbuf);
        return buf;
}


char* PadRightStrStr(char *buf, char *Str, char *Value, int Width)
{
        if(gLangDriver)
        {
                if(!gLangDriver->RightToLeft)
                        PadMidStrStr(buf, Str, Value, Width);
                else
                        PadMidStrStr(buf, Value, Str, Width);
        }
        return buf;

/*

	if(gLangDriver)
	{
		char *p, vbuf[40];
		int vwidth;
		int lwidth;
		if(Value) strcpy(vbuf,Value);
		vwidth=gLangDriver->GetTextWidthFun(gLangDriver,TrimRightStr(vbuf))/gLangDriver->CharWidth;
		sprintf(buf, "%s%20s", Str, " ");
		lwidth=Width-vwidth;
		if(lwidth<0) lwidth=0;
		p=gLangDriver->GetNextTextFun(gLangDriver, buf, lwidth*gLangDriver->CharWidth);
		if(p && Value)
			sprintf(p,"%s", Value);
	}
	return buf;
*/
}

char* PadRightStrSID(char *buf, int StrID, char *Value, int Width)
{
	return PadRightStrStr(buf, LoadStrByID(StrID), Value, Width);
}

char* PadRightIntSID(char *buf, int StrID, int Value, int Width)
{
	char StrValue[100];
	sprintf(StrValue, "%d", Value);
	return PadRightStrSID(buf, StrID, StrValue, Width);
}

//Display black/white image according to threshold
void DrawImage(char *image, int width, int height, int WhiteThreshold)
{
	BYTE ibuf[128*64/8], pbuf[32];
	int w,h,i,j, hist[256], row;

	//calculate the threshold
	memset(hist, 0, sizeof(int)*256);
	memset(ibuf, 0xff, sizeof(ibuf));
	h=0;
	for(i=0;i<height;i++)
		for(j=0;j<width;j++)
			hist[(BYTE)image[h++]]++;
	h=0;w=width*height;
	for(i=0;i<256;i++)
	{
		h+=hist[i];
		if(h*100>w*WhiteThreshold)
		{
			WhiteThreshold=i;
			break;
		}
	}

	//calculate the real size
	w=gLCDWidth/2;
	h=gLCDHeight;
	if(w*height<h*width)
		h=height*w/width;
	else
		w=width*h/height;

	//take a black/white image
	for(i=0;i<h;i++)
	{
		int k,W;
		BYTE *p=ibuf, *img=(BYTE*)image;
		int H=i*height/h;
		p+=i*gLCDWidth/8;
		img+=width*H;
		for(j=0;j<(w+7)/8;j++)
		{
			*p=0;
			for(k=0;k<8;k++)
			{
				W=(j*8+k)*width/w;
				if(W<width)
				{
					if(img[W]>WhiteThreshold)
						*p|=(0x80>>k);
				}
				else
					*p|=(0x80>>k);
			}
			p++;
		}
	}

	//render the image to LCD
	for(row=0;row<(h+15)/16;row++)
	{
		int x=0,CharIndex;
		BYTE *(p[8]), b;
		for(i=0;i<8;i++)
			((int *)pbuf)[i]=0;
		for(CharIndex=0;CharIndex<(w+15)/16;CharIndex++)
		{
			for(i=0;i<8;i++)
				p[i] = ibuf+(i+row*16)*gLCDWidth/8+x;
			for(i=0;i<8;i++)
			{
				b = 0x80 >> i;
				pbuf[i] = 0;
				for(j=0;j<8;j++)
					if(0==(b & *(p[j])))
						pbuf[i] |= (1 << j);
				pbuf[i+8] = 0;
				for(j=0;j<8;j++)
					if(0==(*(p[j]+1) & b))
						pbuf[i+8] |= (1 << j);
			}
			for(i=0;i<8;i++)
				p[i] = ibuf+(i+row*16+8)*gLCDWidth/8+x;
			for(i=0;i<8;i++)
			{
				b = 0x80 >> i;
				pbuf[i+16] = 0;
				for(j=0;j<8;j++)
					if((b & *p[j])==0)
						pbuf[i+16] |= (1 << j);
				pbuf[i+24] = 0;
				for(j=0;j<8;j++)
					if(0==(*(p[j]+1) & b))
						pbuf[i+24] |= (1 << j);
			}
			ExPutPixelBuffer32(row*2, x, (char*)pbuf);
			x+=2;
		}
	}
}

char ICON_MF[8]={0x18,0x18,0x00,0x18,0x00,0x3C,0x81,0x7E};                                                                                                               
char ICON_NET[8]={0x2F,0x29,0x39,0x2F,0x40,0x9E,0xF2,0x9E};

//Display device icon MIFARE AND ETHERNET                                                                                                               
void ExShowDevIcon(int Row, int Col)
{
        extern int gMFOpened;
	//extern int gEthOpened;
        char Buf[50];
        memset(Buf, 0, 16);
        if(gMFOpened) memcpy(Buf, ICON_MF, 8);
        memcpy(Buf+8, ICON_NET, 8);
        ExPutPixelBuffer16(Row*2, Col, Buf);
}
//use for new mainform
int LCD_Bar(int x1, int y1, int x2, int y2)
{
        while(y1<=y2)
        {
                LCD_Line(x1,y1,x2,y1); y1++;
        }
        return TRUE;
}

int LCD_ClearBar(int x1, int y1, int x2, int y2)
{
        int x,y;
        if(gLangDriver && gLangDriver->RightToLeft)
        {
                x=x1;
                x1=gLCDWidth-x2;
                x2=gLCDWidth-x;
        }

        for(x=x1;x<x2;x++)
        for(y=y1;y<y2;y++)
        {
                LCD_ClearPixel(x,y);
        }
        return TRUE;
}

int _LCD_OutBMP1Bit(int StartX, int StartY, BYTE *BMPData, int x1, int y1, int Width, int Height, int Reverse)
{
        BYTE *data=BMPData+BMPData[0xa]+256*BMPData[0xb];
        int bmpWidth, bmpHeight, bmpRowByte, x, y;
        bmpWidth=BMPData[0x12]+256*BMPData[0x13];
        bmpHeight=BMPData[0x16]+256*BMPData[0x17];
        if(Width==-1) Width=bmpWidth;
        if(Height==-1) Height=bmpHeight;
        bmpRowByte=((bmpWidth+7)/8+3)/4*4;
        if(BMPData[2]+256*BMPData[3]!=BMPData[0xa]+256*BMPData[0xb]+
                bmpRowByte*bmpHeight) return FALSE;
        for(y=0;y<Height;y++)
        for(x=0;x<Width;x++)
        {
                int i,j;
                i=(x+x1)/8+(bmpHeight-1-(y+y1))*bmpRowByte;
                j=7-((x+x1)%8);
                if(((0!=(data[i] & (1<<j))) && !Reverse) ||((0==(data[i] & (1<<j))) && Reverse))
                        LCD_ClearPixel(StartX+x, StartY+y);
                else
                        LCD_SetPixel(StartX+x, StartY+y);
        }
        return TRUE;
}
int LCD_OutBMP1Bit(int StartX, int StartY, BYTE *BMPData, int x1, int y1, int Width, int Height, int Reverse)
{

        if(gLangDriver && gLangDriver->RightToLeft)
                StartX=gLCDWidth+1-StartX-(Width==-1?32:Width);
        return _LCD_OutBMP1Bit(StartX, StartY, BMPData, x1, y1, Width, Height, Reverse);
}

int LCD_DrawProgress(int x, int y, int width, int height, int count, int progress, int showBox)
{
        int barWidth=(width-3)/count-1;
        int i;
        width=barWidth*count+3;
        LCD_Rectangle(x,y,x+width-1,y+height-1);
        x+=2;
        for(i=0;i<count;i++)
        {
                if(i<progress)
                        LCD_Bar(x, y+2, x+barWidth-2, y+height-3);
                else if(showBox)
                        LCD_Rectangle(x, y+2, x+barWidth-2, y+height-3);
                x+=barWidth;
        }
        return TRUE;
}
