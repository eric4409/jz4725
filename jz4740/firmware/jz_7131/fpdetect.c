/*************************************************
                                           
 ZEM 200                                          
                                                    
 sensor.c
                                                      
 Copyright (C) 2003-2006, ZKSoftware Inc.      		
                                                      
*************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "options.h"

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#define CORRECT_NONE    	0
#define CORRECT_REVERSE 	1
#define CORRECT_ROTATION 	2
#define MARGIN 			8	

//unsigned int img_bpp = 8; /* 8/16/32 */

#define BYTE	unsigned char
int gNewFPReader=0;
int CalcVar(BYTE *img, int width, int height, int *var, int *mean, int *whiteSum, int FrameWidth);

static void ReverseImage(BYTE *image,int w, int h)
{
	static int i;
	BYTE *p=image;
	for(i=0; i<w*h; i++)
	{
		*p=255-*p;
		p++;
	}
}

static void RegionDivideAdaptive(BYTE* lpDIBBits, int lmageWidth, int lmageHeight, int adjustValue, int *AvgPixel)
{
        static int i;  //循环变量
        static int j;  //循环变量
        // 指向源图像的指针
        static unsigned char*  lpSrc;
        //像素值
        static unsigned char pixeltemp;
        // 子图像灰度的平均值
        static int nAvg ;
        nAvg = 0 ; //初始化为0
        // 对左下图像逐点扫描：
        for(j = 0; j <lmageHeight; j++)
        {
                for(i = 0;i <lmageWidth; i++)
                {
                        // 指向源图像倒数第j行，第i个象素的指针                 
                        lpSrc = (unsigned char *)lpDIBBits + lmageWidth * j + i;
                        //取得当前指针处的像素值
                        pixeltemp = (unsigned char)*lpSrc;
                        //灰度的累计
                        nAvg = (int)pixeltemp+nAvg;
                }
        }
        // 计算均值
        // adjustValue is best for blank image,because the diff is very small on blank image,it can filter many invalid data.
        //it is -12 for upek,-16 for glass that have cover
        nAvg = nAvg /((lmageHeight) * (lmageWidth))+adjustValue ;
        *AvgPixel = 255-nAvg;
        // 对左下图像逐点扫描：
        for(j = 0; j <lmageHeight; j++)
        {
                for(i = 0;i <lmageWidth; i++)
                {
                        // 指向源图像倒数第j行，第i个象素的指针                 
                        lpSrc = (unsigned char *)lpDIBBits + lmageWidth * j + i;
                        //取得当前指针处的像素值
                        pixeltemp = (unsigned char)*lpSrc;
                        //目标图像中灰度值小于门限则设置为黑点
                        if(pixeltemp <= nAvg)
                        {
                                *lpSrc=(unsigned char)0;
                        }
                        else    //否则设置为白点
                        {
                                *lpSrc=(unsigned char)255;
                        }
                }
        }
}


/*
Cut a area for detect from original image
*/
static int CutDetectArea(BYTE* SrcImgBuf,BYTE* DstImgBuf,int X,int Y,int SrcWidth,int SrcHeight, int Width,int Height)
{
        static int i,j;
        if(Width>SrcWidth || Height>SrcHeight || !SrcImgBuf || !DstImgBuf)
        {
                return FALSE;
        }

        for(j=Y;j<Y+Height;j++)
        {
                for(i=X;i<X+Width;i++)
                {
                        *DstImgBuf= *(SrcImgBuf+SrcWidth * j +i);
                        DstImgBuf++; 
                }
        }
        return TRUE;
}


#define DETECT_WIDTH            128
#define DETECT_HEIGHT           64
#define DETECT_IMG_SIZE         (DETECT_WIDTH*DETECT_HEIGHT)


BYTE prev_fp[DETECT_IMG_SIZE]={0};
BYTE diff_fp[DETECT_IMG_SIZE]={0};
BYTE cur_fp[DETECT_IMG_SIZE]={0};

int DetectFP(BYTE *ImgBuf,int Width,int Height, int HasFingerThreshold,int NoFingerThreshold,int Reverse,int DetectCount,int IsSingle)
{
	int v,avg_cur,avg_cur_div,avg_diff,m1,m2,size,pixel_diff,whiteSum=0,iTemp=0;
	int avgPixel=0;
/*	int ImgSize= Width*Height;

	BYTE *prev_fp = ImgBuf + (ImgSize),
		 *diff_fp = ImgBuf + (ImgSize) + DETECT_IMG_SIZE,
		 *cur_fp  = ImgBuf + (ImgSize) + (DETECT_IMG_SIZE <<1); */
//	printf(" ********** DetectFP ImgBuf=%p,prev_fp=%p,diff_fp=%p,cur_fp=%p \n",ImgBuf,prev_fp,diff_fp,cur_fp);
	static int LeaveFinger=0,
		   validCount=0,
		   pre_avg_cur_div=0;


	if(!CutDetectArea(ImgBuf,cur_fp,(Width>>1)-64 ,(Height>>1),Width,Height,DETECT_WIDTH,DETECT_HEIGHT))
	{
		return 0;
	}
	if(Reverse)
	{
		ReverseImage(cur_fp,DETECT_WIDTH,DETECT_HEIGHT);
	}
#ifdef DEBUGME
	WriteBitmap(cur_fp,DETECT_WIDTH,DETECT_HEIGHT,"fp_cut.bmp");
	WriteBitmap(ImgBuf,Width,Height,"fp_cut_src.bmp");
#endif
	CalcVar(cur_fp,DETECT_WIDTH,DETECT_HEIGHT,&avg_cur,&m1,&whiteSum,0);
	RegionDivideAdaptive((BYTE*)cur_fp,DETECT_WIDTH,DETECT_HEIGHT,-12,&avgPixel);
	size = DETECT_IMG_SIZE;
	for(v=0;v<size;v++)
	{
		if(cur_fp[v]==prev_fp[v])
			pixel_diff=255;
		else
			pixel_diff=0;
		diff_fp[v]=(BYTE)pixel_diff;
	}
	CalcVar(cur_fp,DETECT_WIDTH,DETECT_HEIGHT,&avg_cur_div,&m1,&iTemp,0);
	CalcVar(diff_fp,DETECT_WIDTH, DETECT_HEIGHT,&avg_diff,&m2,&iTemp,0);
	
#ifdef DEBUGME
	WriteBitmap(cur_fp, DETECT_WIDTH, DETECT_HEIGHT,"cur.bmp");
	WriteBitmap( prev_fp, DETECT_WIDTH, DETECT_HEIGHT,"prev.bmp");
	WriteBitmap( diff_fp,DETECT_WIDTH, DETECT_HEIGHT,"diff.bmp");
#endif
	if(!IsSingle && !LeaveFinger && (avg_cur_div<NoFingerThreshold || avg_diff>NoFingerThreshold))
	{
		LeaveFinger=TRUE;
		memcpy(prev_fp,cur_fp,DETECT_WIDTH*DETECT_HEIGHT);
		validCount=0; 
		pre_avg_cur_div=0;
	//	printf("No finger\n"); 
		return 0;
	}else if (IsSingle && ((avg_cur_div>HasFingerThreshold ) ||			//正常探测
			(avgPixel>180 && avg_cur_div>HasFingerThreshold-1200 ) || //适当考虑过湿指纹
			(avgPixel<80 &&  avg_cur_div>HasFingerThreshold+800 )))
	{
		return TRUE;
		
	}else if(LeaveFinger && ((avg_cur_div>HasFingerThreshold && avg_diff>NoFingerThreshold) ||			//正常探测
			(avgPixel>180 && avg_cur_div>HasFingerThreshold-1200 && avg_diff>NoFingerThreshold-1200) || //适当考虑过湿指纹
			(avgPixel<80 &&  avg_cur_div>HasFingerThreshold+800 && avg_diff>NoFingerThreshold+800)))	//适当过滤残留指纹
	{
	//	printf("debug me avg_cur_div=%d,count=%d,avgPixel=%d\n",avg_cur_div,validCount,avgPixel);
		validCount++;
		if(validCount==DetectCount)
		{
			//filter black background
			if(avgPixel>216 || (avg_cur<696 && avgPixel>136 && whiteSum<104) || (avg_cur<480 && avgPixel>120))
			{
				validCount=0;
			//	DBPRINTF("a fake finger\n");
				//write_bitmap("/mnt/ramdisk/finger_fake.bmp",ImgBuf,Width,Height);
				return FALSE;
			}
			//write_bitmap("/mnt/ramdisk/finger.bmp",ImgBuf,Width,Height);
					
			LeaveFinger=FALSE;
			memcpy(prev_fp,cur_fp,DETECT_IMG_SIZE);
#ifdef DEBUGME 	//get a best finger image
			if(validCount>1 && pre_avg_cur_div> avg_cur_div)
				memcpy(ImgBuf,ImgBuf+ImgSize,ImgSize);
#endif
			pre_avg_cur_div = 0;
			validCount=0;

			return TRUE;
		}else
		{
			pre_avg_cur_div = avg_cur_div;
#ifdef DEBUGME
			memcpy(ImgBuf+ImgSize,ImgBuf,ImgSize);
#endif
			return 0;
		}
	}else
	{
		//_RPT3(0,"else debug me avg_cur_div=%d,avg_diff=%d,avgPixel=%d\n",avg_cur_div,avg_diff,avgPixel);
	//	printf("else debug me avg_cur_div=%d,avg_diff=%d,avgPixel=%d\n",avg_cur_div,avg_diff,avgPixel);
		validCount=0;
		pre_avg_cur_div=0;
		if(LeaveFinger && avgPixel>180)
			return FALSE;// -1;
		else
			return FALSE;
	}
}


#define BSIZE 16

int CalcVar(BYTE *img, int width, int height, int *var, int *mean, int *whiteSum, int FrameWidth)
{
	int msum, vsum, i, j, bc, sum, m, n, v,t, bsize;
	BYTE *p;
	bsize=BSIZE*BSIZE;
	msum=0;bc=0;vsum=0;
	width-=FrameWidth*2;
	height-=FrameWidth*2;
	*whiteSum=0;
	for(i=0;i<height/BSIZE;i++)
	for(j=0;j<width/BSIZE;j++)
	{
		sum=0;
		for(m=i*BSIZE;m<i*BSIZE+BSIZE;m++)
		{
			p=img+FrameWidth+(m+FrameWidth)*(width+FrameWidth*2)+j*BSIZE;
			for(n=0;n<BSIZE;n++)
			{
				if((*p)>136)
					(*whiteSum)++;
				sum+=(int)*p++;
			}
		}
		sum=(sum+bsize)/bsize;
		msum+=sum;
		v=0;
		for(m=i*BSIZE;m<i*BSIZE+BSIZE;m++)
		{
			p=img+FrameWidth+(m+FrameWidth)*(width+FrameWidth*2)+j*BSIZE;
			for(n=0;n<BSIZE;n++)
			{
				t=(int)*p++-sum;
				t=t*t;
				v+=t;
			}
		}
		v=(v+bsize)/bsize;
		vsum+=v;
		bc++;
	}
	*var=(vsum+bc/2)/bc;
	if(gOptions.NewFPReader & CORRECT_REVERSE)
		*mean=255-(msum+bc/2)/bc;
	else
		*mean=(msum+bc/2)/bc;
	
	return 1;
}

