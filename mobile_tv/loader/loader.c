/*
 *  JZ4740  mobile_tv  Project  V1.0.0
 *  Copyright (C) 2006 - 2007 Ingenic Semiconductor Inc.
 *  Author: <xliu@ingenic.cn>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  2007-12-27, xliu
 *  Created.
 */

#include <ucos_ii.h>
//#include "../../jz4740/nand/ssfdc_nftl.h"
#include "../../jz4740/nand/header.h"
#define CONFIG_SSFDC_NAND_PAGE_PER_BLOCK 128
#define CONFIG_SSFDC_NAND_PAGE_SIZE 2048


//#define DEBUG
#undef DEBUG

//#ifndef DEBUG
//#define dprintf(x, args...)
//#else
#define dprintf(x, args...)  printf("________line %d: ",__LINE__);printf(x,##args);printf("\n")
//#endif

#define BLOCK_MAX            (1024 * 1)
#define PAGE_PER_BLOCK       CONFIG_SSFDC_NAND_PAGE_PER_BLOCK
#define PAGE_SIZE            CONFIG_SSFDC_NAND_PAGE_SIZE
#define BLOCK_SIZE           (PAGE_PER_BLOCK * PAGE_SIZE)
unsigned int dnand = 0;
#define read_nand_info(page,info)        jz_nand_read_page_info(&dnand,page,info)
#define read_nand_data(page,data,info)   jz_nand_read_page(&dnand,page,data,info)
#define block_to_page(i)            (i * PAGE_PER_BLOCK)
#define page_len(len)               ((len + PAGE_SIZE - 1) / PAGE_SIZE)
#define page_to_size(page)          (page * PAGE_SIZE)
#define block_to_size(block)        (block * PAGE_PER_BLOCK * PAGE_SIZE)


int ReadPage(int page_addr, int page_count, unsigned char* buffer)
{
    int ret;
    int cur_page = page_addr;
    int cur_block = page_addr / PAGE_PER_BLOCK;
    int read_page_count = 0;
    //struct nand_page_info_t info;
    nand_page_info_t info;
    int i;
    for(i=0; i<BLOCK_MAX; i++)
    {
        // 判断cur_page所在的块是否坏块
        int cur_block_first_page = block_to_page(cur_block);
        //dprintf("cur_block_first_page = %d\n", cur_block_first_page);
        int cur_block_last_page  = cur_block_first_page + PAGE_PER_BLOCK - 1;
        //dprintf("cur_block_last_page  = %d\n", cur_block_last_page);
        cur_block ++;

        ret = read_nand_info(cur_block_first_page, &info);
        if(0 == info.block_status){
            cur_block ++;
            cur_page = cur_block_last_page + 1;
            dprintf("bad block.");
            continue;
        }
		ret = read_nand_info(cur_block_last_page, &info);
        if(0 == info.block_status){
            cur_block ++;
            cur_page = cur_block_last_page + 1;
            dprintf("bad block.");
			continue;
        }

        while(cur_page <= cur_block_last_page && read_page_count < page_count)
        {
            //dprintf("cur_page(read in page) = %d\n", cur_page);
            //dprintf("buffer address = 0x%08X\n", buffer);
	        //if(0 != read_nand_data(cur_page, buffer, &info)){
            int t = read_nand_data(cur_page, buffer, &info);
            //dprintf("t = %d\n", t);
            if(0 != t){
                dprintf("Read nand data error!\n");
		        return FALSE;
	        }
            buffer += PAGE_SIZE;
	        read_page_count ++;
            cur_page ++;
            if(read_page_count == page_count){
                dprintf("OK, read in %d pages.\n", read_page_count);
                return TRUE;
            }           
        }
    }

    dprintf("Too many bad blocks!\n");
    return FALSE;
}

typedef unsigned int COLORREF;
#define LOWORD(l)           ((unsigned short)((COLORREF)(l) & 0xffff))
#define HIWORD(l)           ((unsigned short)((COLORREF)(l) >> 16))
// 剪裁矩形区域
void inline ClipRect(int x, int y, int bmp_w, int bmp_h, int *w, int *h)
{
    if((*w + x) > bmp_w)
        *w = bmp_w - x;
    if((*h + y) > bmp_h)
        *h = bmp_h - y;
}
// 位图结构
typedef struct _BITMAP
{
    unsigned short w;
    unsigned short h;
    COLORREF* buffer;
} BITMAP;
// 位图绘制(不支持alpha)
int BitBlt(BITMAP* pbmp_dest, int x, int y, int w, int h, BITMAP* pbmp_orig, int ox, int oy)
{
	if(x < 0){
        w  += x;    // 注意x是负的！
        ox -= x;
        x   = 0;
    }
    if(y < 0){
        h  += y;
        oy -= y;
        y   = 0;
    }
    int dest_w = pbmp_dest->w;
    int dest_h = pbmp_dest->h;
    int orig_w = pbmp_orig->w;
    int orig_h = pbmp_orig->h;
    //dprintf("__________dest_w=%d, dest_h=%d, orig_w=%d, orig_h=%d\n",dest_w,dest_h,orig_w,orig_h);
    if(w <= 0 || h <= 0 || x >= dest_w || y >= dest_h){
        return 0;
    }
    ClipRect(x, y, dest_w, dest_h, &w, &h);
    ClipRect(ox, oy, orig_w, orig_h, &w, &h);
    if(w <= 0 || h <= 0){
        return 0;
    }
    //dprintf("__________BitBlt:x:%d,y:%d,w:%d,h:%d,ox:%d,oy:%d\n",x,y,w,h,ox,oy);
    COLORREF* dest_buf = pbmp_dest->buffer + y  * dest_w + x;
    COLORREF* orig_buf = pbmp_orig->buffer + oy * orig_w + ox;
    int i;
    for(i=0; i<h; i++){
        memcpy(dest_buf,orig_buf,w*sizeof(COLORREF));
        dest_buf += dest_w;
        orig_buf += orig_w;
    }
    return 1;
}

// 配置信息在Nand中的位置
#define CONFIG_ADDR     256 // 配置信息
#define CONFIG_LENGTH   1

// 配置信息：位置(页号) 长度(页)
static int config[page_to_size(CONFIG_LENGTH) / 4] __attribute__ ((aligned (16)));   // 配置信息
static int index = 6;
static int getx()    { return config[index];   }
static int gety()    { return config[index+1]; }
static int getw()    { return config[index+2]; }
static int geth()    { return config[index+3]; }
static int getox()   { return config[index+4]; }
static int getoy()   { return config[index+5]; }
static int getdelay(){ return config[index+6]; }
static int next(){
    index += 7;
    if(index>=page_to_size(CONFIG_LENGTH)/4)
        index = 6;
}

// 图片资源：位置(页号) 长度(页) 读入固定位置，最长2M
//           config[0]  config[1]
// 应用程序：位置(页号) 长度(页)   读入位置   入口地址(字节)
//           config[2]  config[3]  config[4]  config[5]
// 动画脚本：从config[6]开始格式如下：
//           x, y, w, h, ox, oy, delay(单位10ms)
//           x, y, w, h, ox, oy, delay(单位10ms)
//           ......

#define  DISPLAY_TASK_PRI  11          // 动画优先级
#define  LOADER_TASK_PRI   10          // 工作优先级
#define  TASK_STK_SIZE     1024 * 1    // 栈尺寸
#define  DISPLAY_TASK_ID   1
#define  LOADER_TASK_ID    2
static   OS_STK   DisplayTaskStk[TASK_STK_SIZE];  //size is DWORD
static   OS_STK   LoaderTaskStk[TASK_STK_SIZE];   //size is DWORD
static   OS_EVENT *psem_quit_display;
static   int  bDisplay = 1;

// frame buffer
//static unsigned char* pframe_buffer = (unsigned char*)0x83F00000; // 使用绝对地址
static   BITMAP   bmpFB;
static   BITMAP   bmpImage;
#define  IMAGE_RES_SIZE    (1024 * 1024 * 2)
static   COLORREF image_memory[IMAGE_RES_SIZE / sizeof(COLORREF)] __attribute__ ((aligned (16)));

// 应用程序入口函数
static void (* motv_entry)();

static void StartLoaderTask(void);
static void StartDisplayTask(void);
static void LoaderTask(void *data);
static void DisplayTask(void *data);
unsigned char os_malloc_completed = 0;
unsigned char os_init_completed = 0;
void LoaderInit(void)
{
    jz_nand_init();
}
void StartLoaderTask(void)
{
    
    OS_STK *ptos = &LoaderTaskStk[TASK_STK_SIZE - 1];
    OS_STK *pbos = &LoaderTaskStk[0];
    INT32U  size = TASK_STK_SIZE;
    OSTaskCreateExt(LoaderTask,
                    (void *)0,
                    ptos,
                    LOADER_TASK_PRI,
                    LOADER_TASK_ID,
                    pbos,
                    size,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
}
void StartDisplayTask(void)
{
    OS_STK *ptos = &DisplayTaskStk[TASK_STK_SIZE - 1];
    OS_STK *pbos = &DisplayTaskStk[0];
    INT32U  size = TASK_STK_SIZE;
    OSTaskCreateExt(DisplayTask,
                    (void *)0,
                    ptos,
                    DISPLAY_TASK_PRI,
                    DISPLAY_TASK_ID,
                    pbos,
                    size,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
}
void LoaderTask(void *data)
{
	unsigned int c;
    LoaderInit();

    int page_addr  = CONFIG_ADDR;
    int page_count = CONFIG_LENGTH;

    int index = 5;

    // 初始化OS嘀嗒
    JZ_StartTicker(OS_TICKS_PER_SEC);
    //OSStatInit();                     // Initialize uC/OS-II's statistics

    // 读入配置信息，内含动画脚本、动画资源信息、应用程序信息
    dprintf("begin to read in config information.");
    if(FALSE == ReadPage(page_addr, page_count, (unsigned char*)config)){
        dprintf("Read config info failure!\n");
        while(1);
    }
	// 将图片资源读入内存
    dprintf("begin to read in image resource.");
    page_addr  = config[0];
    page_count = config[1];
    dprintf("page_addr=%d, page_count=%d\n",page_addr, page_count);

    if(IMAGE_RES_SIZE < page_to_size(page_count)){
        dprintf("Config error: image size=%d; Can not support!\n", page_to_size(page_count));
        while(1);
    }
    if(FALSE == ReadPage(page_addr, page_count, image_memory)){
        dprintf("Read image resource failure!\n");
        while(1);
    }
    int size = image_memory[0];
    bmpImage.w = LOWORD(size);
    bmpImage.h = HIWORD(size);
    bmpImage.buffer = &image_memory[1];

    // 启动动画线程
    dprintf("start display task.\n");
    dprintf("image_w=%d, image_h=%d\n", bmpImage.w, bmpImage.h);
    psem_quit_display = OSSemCreate(0);
    StartDisplayTask();

    // 将应用程序读入内存
#ifdef DEBUG
    dprintf("________Delay 10 seconds.\n");
    OSTimeDly(2000);  // 10 seconds
#else
    dprintf("begin to read in application module.\n");
    page_addr  = config[2];
    page_count = config[3];
    dprintf("page_addr=%d, page_count=%d, buffer=0x%08X\n",page_addr, page_count, config[4]);

    if(FALSE == ReadPage(page_addr, page_count, (unsigned char*)config[4])){
        printf("Read application module failure!\n");
        while(1);
    }
#endif
    //test:等待动画显示完成
    //OSTimeDly(100);  // 10 seconds

    // 停止动画线程
    printf("stop the display thread.\n");
    bDisplay = FALSE;
    OSSemPost(psem_quit_display);

    // 关闭显示
    //lcdstop();
    
    // 跳转至应用程序入口
    printf("go go go! goto 0x%08X\n", config[5]);
    SetLoadInit();
    motv_entry = config[5];
    cli();
    REG_INTC_IMSR = -1;
		c = REG_INTC_IPR;
		__dcache_writeback_all();
		__icache_invalidate_all();
    motv_entry();
}
void DisplayTask(void *data)
{
    unsigned char err;

	// 初始化Frame Buffer
    jzlcd_init();
    bmpFB.w = lcd_get_width();
    bmpFB.h = lcd_get_height();
    dprintf("bmpFB.w=%d, bmpFB.h=%d\n", bmpFB.w, bmpFB.h);
    bmpFB.buffer = lcd_get_frame();

    bDisplay = TRUE;
    while(bDisplay)
    {
        //dprintf("x=%d,y=%d,w=%d,h=%d,ox=%d,oy=%d,buffer=0x%08X\n",
        //    getx(),gety(),getw(),geth(),getox(),getoy(),bmpImage.buffer);

        BitBlt(&bmpFB,getx(),gety(),getw(),geth(),&bmpImage,getox(),getoy());
        // 超时唤醒或者信号唤醒
        OSSemPend(psem_quit_display, getdelay(), &err);
        next();
    }
    OSTaskDel(DISPLAY_TASK_ID);
}

// 主程序
void APP_vMain(void)
{
    OSInit();
    SetLoadData(0);
    StartLoaderTask();
    OSStart();
    while(1);
}
// END
