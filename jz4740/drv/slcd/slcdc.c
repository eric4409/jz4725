/*
 * JZ4740 Smart LCD Driver Routines
 * Copyright (C) 2008 Ingenic Semiconductor Inc.
 * Author: <zyliu@ingenic.cn>
 */

#include <bsp.h>
#include <jz4740.h>
#include <slcdc.h>
#include <dm.h>


#undef dprintf
#define dprintf(x...)

#ifdef FRAME_BUFF_ADDRESS
//#if 0
static u8 *lcd_heap = (u8 *)FRAME_BUFF_ADDRESS;
static struct jz_dma_desc *lcd_frame_desc2_room;
#else
static u8 lcd_heap[4096 + (480 * 272 * 4 + 4095) / 4096 * 4096 * FRAMEBUF_NUM] __attribute__((aligned(4096))); // 4Kb align

static struct jz_dma_desc lcd_frame_desc2_room[FRAMEBUF_NUM] __attribute__ ((aligned (32)));

#endif


static struct jz_dma_desc lcd_palette_desc __attribute__ ((aligned (32)));
struct jz_dma_desc *lcd_frame_desc2;
struct jz_dma_desc *cur_lcd_frame_desc,*old_lcd_frame_desc;


//static unsigned int frm_size;
static int dma_chan = 5;
static unsigned int n_doorbell = 0;
static unsigned int non_link_desp = 0, link_desp = 0;

struct jzfb_slcd_info jzfb_slcd = {
#if SLCDTYPE == 1 /*CONFIG_JZ_SLCD_LGDP4551)*/
	SLCD_CFG_CS_ACTIVE_LOW | SLCD_CFG_RS_CMD_LOW | SLCD_CFG_TYPE_PARALLEL,
	400, 240, 16, 8, 16000000   /* 16bpp 8bus */
//	400, 240, 18, 8, 16000000   /* 18bpp 8bus */
//	400, 240, 18, 18, 16000000   /* 18bpp 18bus */
#endif

#if SLCDTYPE == 2 /*CONFIG_JZ_SLCD_SPFD5420A)*/
	SLCD_CFG_CS_ACTIVE_LOW | SLCD_CFG_RS_CMD_LOW | SLCD_CFG_TYPE_PARALLEL,
//	400, 240, 16, 8, 16000000   /* 16bpp 8bus */
//	400, 240, 18, 8, 16000000   /* 18bpp 8bus */
//	400, 240, 18, 18, 16000000   /* 18bpp 18bus */
	400, 240, 16, 16, 16000000   /* 16bpp 16bus */
//	400, 240, 18, 16, 4000000   /* 16bpp 16bus */
#endif

#if SLCDTYPE == 3 /*CONFIG_JZ_SLCD_SPFD5408A)*/
	SLCD_CFG_CS_ACTIVE_LOW | SLCD_CFG_RS_CMD_LOW | SLCD_CFG_TYPE_PARALLEL,
	320, 240, 16, 16, 16000000   /* 16bpp 16bus */
#endif
};

u32 lcd_get_width(void) {return jzfb_slcd.w;}
u32 lcd_get_height(void) {return jzfb_slcd.h;}
u32 lcd_get_bpp(void) {
	if(jzfb_slcd.bpp > 16)
		return 32;
	return jzfb_slcd.bpp;
}

#if 0 /*for 480x272, 18bpp resoure*/
u32 lcd_get_width(void) {return 480;}
u32 lcd_get_height(void) {return 272;}
u32 lcd_get_bpp(void) {
//	if(jzfb_slcd.bpp > 16)
		return 32;
//	return jzfb_slcd.bpp;
}
#endif

/*The interface functions of framebuffer*/
u8* lcd_get_frame(void) {return jzfb_slcd.frame;}
u8* lcd_get_cframe(void) {return jzfb_slcd.cframe;}

u8* lcd_get_change_frame(void) {return (u8*)LCD_UNCACHED(old_lcd_frame_desc->dsadr);}
u8* lcd_get_change_cframe(void) {return (u8*)(old_lcd_frame_desc->dsadr + 0x80000000);}
u8* lcd_get_change_phyframe(void){/*printf("old_lcd_frame_desc->dsadr = %x\n",old_lcd_frame_desc->dsadr);*/return (u8*)old_lcd_frame_desc->dsadr;}

u8* lcd_get_current_frame(void) {return (u8*)LCD_UNCACHED(cur_lcd_frame_desc->dsadr);}
u8* lcd_get_current_cframe(void) {return (u8*)(cur_lcd_frame_desc->dsadr + 0x80000000);}
u8* lcd_get_current_phyframe(void){return (u8*)cur_lcd_frame_desc->dsadr;}

void lcd_flush_frame(int x,int y,int w,int h)
{
	int i,j;
	for(i = 0; i < FRAMEBUF_NUM;i++)
	{
				for(j = 0; j < h; j++)
					memset((jzfb_slcd.cframe2[i] + (x + (j + y) * jzfb_slcd.w) * 4),0, w * 4);
	}
}
void lcd_clean_frame_other() 
{
	int i; 
	for(i = 0; i < FRAMEBUF_NUM;i++)
	{
		if((unsigned int)(jzfb_slcd.cframe2[i]) != (unsigned int)lcd_get_current_cframe())
			memset(jzfb_slcd.cframe2[i],0,
			       lcd_get_width() * lcd_get_height() * lcd_get_bpp() / 8
		);
	}
}
void lcd_clean_frame_all() 
{ 
	int i; 
	for(i = 0; i < FRAMEBUF_NUM;i++)
		memset(jzfb_slcd.cframe2[i],0,
			lcd_get_width() * lcd_get_height() * lcd_get_bpp() / 8
		);
}
void lcd_flush_frame_all() {__dcache_writeback_all();}

/*Change buffer descriptor*/
void lcd_change_frame(void){
    int i;
#if 1
//	n_doorbell++;
    //printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX old_lcd_frame_desc: %x\n", old_lcd_frame_desc );
	if (n_doorbell == 0) {
		n_doorbell++;
//		dump_jz_dma_channel(dma_chan);
	    REG_DMAC_DDA(dma_chan) = PHYS(old_lcd_frame_desc);
		__dmac_channel_set_doorbell(dma_chan);

	}
	else
		n_doorbell++;
#endif
//	printf("Change: n_doorbell = %d\n", n_doorbell);
//	printf("%d\n", n_doorbell);
//	old_lcd_frame_desc->ddadr = ((PHYS(old_lcd_frame_desc) >> 4) << 24) | (frm_size & 0xffffff);
//	cur_lcd_frame_desc->ddadr = ((PHYS(old_lcd_frame_desc) >> 4) << 24) | (frm_size & 0xffffff);
//	old_lcd_frame_desc->ddadr = ((PHYS(old_lcd_frame_desc) >> 4) << 24) |
//		( old_lcd_frame_desc->ddadr & 0xffffff);
//	cur_lcd_frame_desc->ddadr = ((PHYS(old_lcd_frame_desc) >> 4) << 24) |
//		( old_lcd_frame_desc->ddadr & 0xffffff);
	cur_lcd_frame_desc = old_lcd_frame_desc;
	old_lcd_frame_desc++;
	if((unsigned int)old_lcd_frame_desc > (unsigned int)&lcd_frame_desc2[FRAMEBUF_NUM - 1])
		old_lcd_frame_desc = &lcd_frame_desc2[0];
#if 0
	printf("old_desc= 0x%08x\n", old_lcd_frame_desc);
	printf("cur_desc= 0x%08x\n", cur_lcd_frame_desc);    
	for(i = 0;i < FRAMEBUF_NUM;i++)
	{
        printf("[%d].dsadr= 0x%08x\n", i, lcd_frame_desc2_room[i].dsadr);
        printf("[%d].dtadr= 0x%08x\n", i, lcd_frame_desc2_room[i].dtadr);
        printf("[%d].ddadr= 0x%08x\n", i, lcd_frame_desc2_room[i].ddadr);
        printf("[%d].dcmd= 0x%08x\n", i, lcd_frame_desc2_room[i].dcmd);
     //   printf("\n");
	}
#endif
}

/* Enter frame will Change Descriptor to Non-link and Enable Interrupt */
void lcd_enter_frame()
{
    
	int i;
//	printf(" 11 SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS: %d\n", dma_chan);
//	non_link_desp = 1;
#if 1
//	printf(" 22 SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS%d\n", dma_chan);
	REG_DMAC_DCMD(dma_chan)	|= DMAC_DCMD_TIE;
    REG_DMAC_DCMD(dma_chan) &= ~DMAC_DCMD_LINK;

//	printf(" 33 SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSs\n");
	for(i = 0;i < FRAMEBUF_NUM;i++)
	{

		lcd_frame_desc2_room[i].dcmd &= ~DMAC_DCMD_LINK;
		lcd_frame_desc2_room[i].dcmd |=  DMAC_DCMD_TIE;
	}
	__dcache_writeback_all();
#endif
	for(i = 0;i < FRAMEBUF_NUM;i++)
	{
        printf("[%d].dsadr= 0x%08x\n", i, lcd_frame_desc2_room[i].dsadr);
        printf("[%d].dtadr= 0x%08x\n", i, lcd_frame_desc2_room[i].dtadr);
        printf("[%d].ddadr= 0x%08x\n", i, lcd_frame_desc2_room[i].ddadr);
        printf("[%d].dcmd= 0x%08x\n", i, lcd_frame_desc2_room[i].dcmd);
        printf("\n");
	}
		    printf(" 44 SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSs\n"); 
	printf("%s\n",__FUNCTION__);
}

void lcd_reset_frame()
{
	int i;
	if((int)cur_lcd_frame_desc != (int)&lcd_frame_desc2[0])
	{
		old_lcd_frame_desc = &lcd_frame_desc2[0];
//		old_lcd_frame_desc->ddadr = ((PHYS(old_lcd_frame_desc) >> 4) << 24) | (frm_size & 0xffffff);
//		cur_lcd_frame_desc->ddadr = ((PHYS(old_lcd_frame_desc) >> 4) << 24) | (frm_size & 0xffffff);
		old_lcd_frame_desc->ddadr = ((PHYS(old_lcd_frame_desc) >> 4) << 24) |
			(old_lcd_frame_desc->ddadr & 0xffffff);
		cur_lcd_frame_desc->ddadr = ((PHYS(old_lcd_frame_desc) >> 4) << 24) |
			(old_lcd_frame_desc->ddadr & 0xffffff);
	}
#if 1
	for(i = 0;i < FRAMEBUF_NUM;i++)
	{
		lcd_frame_desc2_room[i].dcmd &= ~DMAC_DCMD_TIE;
		lcd_frame_desc2_room[i].dcmd |= DMAC_DCMD_LINK;
	}
	__dcache_writeback_all();
	if (n_doorbell == 0) {
		REG_DMAC_DCMD(dma_chan) &= ~DMAC_DCMD_TIE;
		__dmac_channel_set_doorbell(dma_chan);
	}
	else 
		link_desp = 1;
#endif		
}

static __inline__ unsigned int __cpm_divisor_encode(unsigned int n)
{
	unsigned int encode[10] = {1,2,3,4,6,8,12,16,24,32};
	int i;
	for (i=0;i<10;i++)
		if (n < encode[i])
			break;
	return i;
}

static int jzfb_setcolreg(u32 regno, u8 red, u8 green, u8 blue)
{
	u16 *ptr, ctmp;

	if (regno >= NR_PALETTE)
		return 1;

	red	&= 0xff;
	green	&= 0xff;
	blue	&= 0xff;
	
	jzfb_slcd.palette[regno].red		= red ;
	jzfb_slcd.palette[regno].green	= green;
	jzfb_slcd.palette[regno].blue	= blue;
	
	if (jzfb_slcd.bpp <= 8) {
		if (((jzfb_slcd.cfg & MODE_MASK) == MODE_STN_MONO_SINGLE) ||
		    ((jzfb_slcd.cfg & MODE_MASK) == MODE_STN_MONO_DUAL)) {
			ctmp = (77L * red + 150L * green + 29L * blue) >> 8;
			ctmp = ((ctmp >> 3) << 11) | ((ctmp >> 2) << 5) |
				(ctmp >> 3);
		} else {
			/* RGB 565 */
			if (((red >> 3) == 0) && ((red >> 2) != 0))
				red = 1 << 3;
			if (((blue >> 3) == 0) && ((blue >> 2) != 0))
				blue = 1 << 3;
			ctmp = ((red >> 3) << 11) 
				| ((green >> 2) << 5) | (blue >> 3);
		}

		ptr = (u16 *)jzfb_slcd.pal;
		ptr[regno] = ctmp;
		REG_LCD_DA0 = PHYS(&lcd_palette_desc);
	} else
		printf("No palette used.\n");

	return 0;
}

/*
 * Map screen memory
 */
static int fb_malloc(void)
{
	struct page * map = NULL;
	u8 *tmp;
	u32 page_shift, needroom;//, t;
	u32 i;

	needroom = ((lcd_get_width() * lcd_get_bpp() + 7) >> 3) * lcd_get_height();
	printf("1frame buff addr = %x %d\n",lcd_heap,needroom);

#ifdef FRAME_BUFF_ADDRESS
//#if 0
	jzfb_slcd.cpal = (u8*) lcd_heap;
	jzfb_slcd.cframe = (u8 *)((u32)lcd_heap);
	printf("2frame buff addr = %x\n",jzfb_slcd.cframe);
	lcd_frame_desc2_room = (struct jz_dma_desc *)((((unsigned int)lcd_heap + FRAMEBUF_NUM * needroom) + 31) & (~31));
#else
	jzfb_slcd.cpal = (u8 *)(((u32)lcd_heap) & ~0xfff);
	jzfb_slcd.cframe = (u8 *)((u32)jzfb_slcd.cpal + 0x1000);
	for(i = 0; i < FRAMEBUF_NUM;i++)  
		jzfb_slcd.cframe2[i] = (u8 *)((u32)jzfb_slcd.cframe + ((needroom + 0x1000 - 1)/ 0x1000 * 0x1000 * i));
  
#endif 
	for(i = 0; i < FRAMEBUF_NUM;i++)  
	{
		jzfb_slcd.cframe2[i] = (u8 *)((u32)jzfb_slcd.cframe + needroom * i);
		if(!GetLcdInit())
			memset(jzfb_slcd.cframe2[i],0,needroom);
	}
	jzfb_slcd.pal = (u8 *)LCD_UNCACHED(jzfb_slcd.cpal);
	jzfb_slcd.frame = (u8 *)LCD_UNCACHED(jzfb_slcd.cframe);
	for(i = 0; i < FRAMEBUF_NUM;i++) 
		jzfb_slcd.frame2[i] = (u8 *)LCD_UNCACHED(jzfb_slcd.cframe2[i]);
	memset(jzfb_slcd.cpal, 0, 512);
	
	return 0;
}

/*DMA Interrupt Function*/
static void slcd_dma_irq(unsigned int irq)
//static int slcd_dma_irq(unsigned int irq)
{
	int i;
	static unsigned int doormark=0; 
	if (__dmac_channel_transmit_end_detected(dma_chan)) {
//		printf("DMA TT\n");
		__dmac_channel_clear_transmit_end(dma_chan);
#if 0
		if (non_link_desp == 1) {
			for(i = 0;i < FRAMEBUF_NUM;i++)
			{
				lcd_frame_desc2_room[i].dcmd &= ~DMAC_DCMD_LINK;
				lcd_frame_desc2_room[i].dcmd |=  DMAC_DCMD_TIE;
			}
			non_link_desp = 0;
//			return 0;
			return;
		}
#endif
		if (link_desp == 1) {
			REG_DMAC_DCMD(dma_chan) &= ~DMAC_DCMD_TIE;
			__dmac_channel_set_doorbell(dma_chan);
			link_desp = 0;
			return;
		}
		//printf("IRQ: n_doorbell = %d\n", n_doorbell);
		//printf("IRQ: doormark = %d\n", doormark);
//		dump_jz_dma_channel(dma_chan);

		if (doormark == 1) { /* set doorbell by irq */
			if (n_doorbell > 1) {
				n_doorbell = 1;
				doormark = 0;
			}
			else {
				n_doorbell = 0;
				doormark = 0;
//				return 0;
				return;
			}
		}
		else
// 			n_doorbell--;
 			n_doorbell = 0;
		if (n_doorbell > 0) {
		    doormark = 1;
			REG_DMAC_DDA(dma_chan) = PHYS(old_lcd_frame_desc);
//			REG_DMAC_DDA(dma_chan) = PHYS(cur_lcd_frame_desc);
			__dmac_channel_set_doorbell(dma_chan); 
//			dump_jz_dma_channel(dma_chan);
//			return 0;
			return;
		}
	}
#if 1
	if (__dmac_channel_transmit_halt_detected(dma_chan)) {
		printf("DMA HALT\n");
		__dmac_channel_clear_transmit_halt(dma_chan);
	}

	if (__dmac_channel_address_error_detected(dma_chan)) {
		printf("DMA ADDR ERROR\n");
		__dmac_channel_clear_address_error(dma_chan);
	}

	if (__dmac_channel_descriptor_invalid_detected(dma_chan)) {
		printf("DMA DESC INVALID\n");
		__dmac_channel_clear_descriptor_invalid(dma_chan);
	}

	if (__dmac_channel_count_terminated_detected(dma_chan)) {
		printf("**DMA CT\n");
		dump_jz_dma_channel(dma_chan);
		__dmac_channel_clear_count_terminated(dma_chan);
	}
#endif
}
/*Set registers of smart lcd*/
void Mcupanel_RegSet(unsigned int cmd, unsigned int data)
{
	switch (jzfb_slcd.bus) {
	case 8:
		while (REG_SLCD_STATE & SLCD_STATE_BUSY);
		REG_SLCD_DATA = SLCD_DATA_RS_COMMAND | ((cmd&0xff00) >> 8);
		while (REG_SLCD_STATE & SLCD_STATE_BUSY);
		REG_SLCD_DATA = SLCD_DATA_RS_COMMAND | ((cmd&0xff) >> 0);
		while (REG_SLCD_STATE & SLCD_STATE_BUSY);
		REG_SLCD_DATA = SLCD_DATA_RS_DATA | (data&0xffff);
		break;
	case 9:
		data = ((data & 0xff) << 1) | ((data & 0xff00) << 2);
		data = ((data << 6) & 0xfc0000) | ((data << 4) & 0xfc00) | ((data << 2) & 0xfc);
		while (REG_SLCD_STATE & SLCD_STATE_BUSY);
		REG_SLCD_DATA = SLCD_DATA_RS_COMMAND | ((cmd&0xff00) >> 8);
		while (REG_SLCD_STATE & SLCD_STATE_BUSY);
		REG_SLCD_DATA = SLCD_DATA_RS_COMMAND | ((cmd&0xff) >> 0);
		while (REG_SLCD_STATE & SLCD_STATE_BUSY);
		REG_SLCD_DATA = SLCD_DATA_RS_DATA | data;
		break;
	case 16:
		while (REG_SLCD_STATE & SLCD_STATE_BUSY);
		REG_SLCD_DATA = SLCD_DATA_RS_COMMAND | (cmd&0xffff);
		while (REG_SLCD_STATE & SLCD_STATE_BUSY);
		REG_SLCD_DATA = SLCD_DATA_RS_DATA | (data&0xffff);
		break;
	case 18:
		cmd = ((cmd & 0xff) << 1) | ((cmd & 0xff00) << 2); 	
 		data = ((data & 0xff) << 1) | ((data & 0xff00) << 2);
 		while (REG_SLCD_STATE & SLCD_STATE_BUSY);
		REG_SLCD_DATA = SLCD_DATA_RS_COMMAND | cmd;
		while (REG_SLCD_STATE & SLCD_STATE_BUSY);
		REG_SLCD_DATA = SLCD_DATA_RS_DATA | ((data<<6)&0xfc0000)|((data<<4)&0xfc00) | ((data<<2)&0xfc);
		break;
	default:
		printf("Don't support %d bit Bus\n", jzfb_slcd.bus );
		break;
	}
}

/* Sent a command without data */
void Mcupanel_Command(unsigned int cmd) {
	switch (jzfb_slcd.bus) {
	case 8:
	case 9:
		while (REG_SLCD_STATE & SLCD_STATE_BUSY);
		REG_SLCD_DATA = SLCD_DATA_RS_COMMAND | ((cmd&0xff00) >> 8);
		while (REG_SLCD_STATE & SLCD_STATE_BUSY);
		REG_SLCD_DATA = SLCD_DATA_RS_COMMAND | ((cmd&0xff) >> 0);
		break;
	case 16:
		while (REG_SLCD_STATE & SLCD_STATE_BUSY);
		REG_SLCD_DATA = SLCD_DATA_RS_COMMAND | (cmd&0xffff);
		break;
	case 18:
		while (REG_SLCD_STATE & SLCD_STATE_BUSY);
		REG_SLCD_DATA = SLCD_DATA_RS_COMMAND | ((cmd&0xff00) << 2) | ((cmd&0xff) << 1);
		break;
	default:
		printf("Don't support %d bit Bus\n", jzfb_slcd.bus );
		break;
	}
}

/*prepare descriptor in memory*/
static void lcd_descriptor_init(void)
{
	int i;
	unsigned int pal_size;
	unsigned int frm_size;
	struct jz_dma_desc *pal_desc, *frame_desc0, *frame_desc1;
	lcd_frame_desc2 = (struct jz_dma_desc *)LCD_UNCACHED(lcd_frame_desc2_room);
	
	pal_desc	= &lcd_palette_desc;
	frame_desc0	= &lcd_frame_desc2[0];
	frame_desc1	= &lcd_frame_desc2[0];
	
	
	i = jzfb_slcd.bpp;
	if (i == 15)
		i = 16;
	if( i == 18)
		i = 32;
	frm_size = (jzfb_slcd.w*jzfb_slcd.h*i)>>3;

	switch (jzfb_slcd.bpp) {
	case 1:
		pal_size = 4;
		break;
	case 2:
		pal_size = 8;
		break;
	case 4:
		pal_size = 32;
		break;
	case 8:
	default:
		pal_size = 512;
		break;
	}

	/* Prepare Palette Descriptor */
	pal_desc->dcmd = DMAC_DCMD_SAI | DMAC_DCMD_RDIL_IGN | DMAC_DCMD_SWDH_32
		| DMAC_DCMD_DWDH_32 | DMAC_DCMD_DS_16BYTE | DMAC_DCMD_TM | DMAC_DCMD_DES_V 
//		| DMAC_DCMD_DES_VIE | DMAC_DCMD_TIE; /*refresh according event*/
		| DMAC_DCMD_DES_VIE | DMAC_DCMD_LINK; /*refresh according always*/
	switch (lcd_palette_desc.dcmd & DMAC_DCMD_DS_MASK) {
	case DMAC_DCMD_DS_32BYTE:
		pal_size /= 32;
		break;
	case DMAC_DCMD_DS_16BYTE:
		pal_size /= 16;
		break;
	case DMAC_DCMD_DS_32BIT:
		pal_size /= 4;
		break;
	case DMAC_DCMD_DS_16BIT:
		pal_size /= 2;
		break;
	case DMAC_DCMD_DS_8BIT:
	default:
		break;
	}
	pal_desc->dsadr = PHYS(jzfb_slcd.pal);    /* DMA source address */
	pal_desc->dtadr = PHYS(SLCD_FIFO);    /* DMA target address */
	pal_desc->ddadr = ((PHYS(frame_desc0) >> 4) << 24) | (pal_size & 0xffffff);    /* offset and size*/
	/*Prepare Frame Descriptor in memory*/
	switch (jzfb_slcd.bpp) {
	case 8 ... 16:
		frame_desc0->dcmd = DMAC_DCMD_SAI | DMAC_DCMD_RDIL_IGN | DMAC_DCMD_SWDH_32
			| DMAC_DCMD_DWDH_16 | DMAC_DCMD_DS_16BYTE | DMAC_DCMD_TM | DMAC_DCMD_DES_V
//			| DMAC_DCMD_DES_VIE | DMAC_DCMD_TIE; /*refresh slcd single*/
			| DMAC_DCMD_DES_VIE | DMAC_DCMD_LINK;/*refresh slcd always*/
	break;

	case 17 ... 32:
		frame_desc0->dcmd = DMAC_DCMD_SAI | DMAC_DCMD_RDIL_IGN | DMAC_DCMD_SWDH_32
			| DMAC_DCMD_DWDH_32 | DMAC_DCMD_DS_32BYTE | DMAC_DCMD_TM | DMAC_DCMD_DES_V
//			| DMAC_DCMD_DES_VIE | DMAC_DCMD_TIE; /*refresh slcd single*/
			| DMAC_DCMD_DES_VIE | DMAC_DCMD_LINK;/*refresh slcd always*/

		break;
	}
	switch (frame_desc0->dcmd & DMAC_DCMD_DS_MASK) {
	case DMAC_DCMD_DS_32BYTE:
		frm_size /= 32;
		break;
	case DMAC_DCMD_DS_16BYTE:
		frm_size /= 16;
		break;
	case DMAC_DCMD_DS_32BIT:
		frm_size /= 4;
		break;
	case DMAC_DCMD_DS_16BIT:
		frm_size /= 2;
		break;
	case DMAC_DCMD_DS_8BIT:
	default:
		break;
	}
	frame_desc0->dsadr = PHYS(jzfb_slcd.frame);   /* DMA source address */
	frame_desc0->dtadr = PHYS(SLCD_FIFO);    /* DMA target address */
	frame_desc0->ddadr = ((PHYS(frame_desc0) >> 4) << 24) | (frm_size & 0xffffff);
	dprintf("frame_desc0->dcmd = 0x%08x\n", frame_desc0->dcmd);
	dprintf("frame_desc0->dsadr = 0x%08x\n", frame_desc0->dsadr);
	dprintf("frame_desc0->dtadr = 0x%08x\n", frame_desc0->dtadr);
	dprintf("frame_desc0->ddadr = 0x%08x\n", frame_desc0->ddadr);

	/* Frame Descriptor 1 */
	for(i = 0;i < FRAMEBUF_NUM;i++)
	{
		frame_desc1->dcmd	= frame_desc0->dcmd;
		frame_desc1->dsadr	= PHYS(jzfb_slcd.frame2[i]);
		frame_desc1->dtadr	= PHYS(SLCD_FIFO);
		frame_desc1->ddadr	= ((PHYS(frame_desc1) >> 4) << 24) | (frm_size & 0xffffff);
		frame_desc1++;
	}


	if(FRAMEBUF_NUM > 1)
	{	
		old_lcd_frame_desc = &lcd_frame_desc2[1];
		//printf("old_lcd_frame_desc->buf = %x\n",old_lcd_frame_desc->buf);
	}
	cur_lcd_frame_desc = &lcd_frame_desc2[0];	
}

/* Descriptor transfer */
static int slcd_dma_init(void)
{
	/* Request DMA channel and setup irq handler */
	dma_stop(dma_chan);
	dma_request(dma_chan, slcd_dma_irq, 0, 0, DMAC_DRSR_RS_SLCD);

	/*Init the SLCD DMA and Enable*/
	REG_DMAC_DRSR(dma_chan) = DMAC_DRSR_RS_SLCD;
	REG_DMAC_DMACR = DMAC_DMACR_DMAE;
	REG_DMAC_DCCSR(dma_chan) =  DMAC_DCCSR_EN; /*Descriptor Transfer*/
	if (jzfb_slcd.bpp <= 8)
		REG_DMAC_DDA(dma_chan) = PHYS(&lcd_palette_desc);
	else
		REG_DMAC_DDA(dma_chan) = PHYS(&lcd_frame_desc2[0]);

	/* DMA doorbell set -- start DMA now ... */
	__dmac_channel_set_doorbell(dma_chan); /*enable this if refresh screen always*/
	return 0;
}

/*SLCD controller initialization*/
static int controller_init(void)
{
	unsigned int val, pclk;
	int pll_div;

	REG_LCD_CFG &= ~LCD_CFG_LCDPIN_MASK;
	REG_LCD_CFG |= LCD_CFG_LCDPIN_SLCD;

	if ((jzfb_slcd.bpp == 18) | (jzfb_slcd.bpp == 24))
		jzfb_slcd.bpp = 32;

	/* Configure SLCD module for initialize smart lcd registers*/
	switch (jzfb_slcd.bus) {
	case 8:
		REG_SLCD_CFG = SLCD_CFG_BURST_8_WORD | SLCD_CFG_DWIDTH_8_x2 
			| SLCD_CFG_CWIDTH_8BIT | SLCD_CFG_CS_ACTIVE_LOW 
			| SLCD_CFG_RS_CMD_LOW | SLCD_CFG_CLK_ACTIVE_FALLING 
			| SLCD_CFG_TYPE_PARALLEL;
		__gpio_as_slcd_8bit();
		break;
	case 9:
		REG_SLCD_CFG = SLCD_CFG_BURST_8_WORD | SLCD_CFG_DWIDTH_8_x2
			| SLCD_CFG_CWIDTH_8BIT | SLCD_CFG_CS_ACTIVE_LOW 
			| SLCD_CFG_RS_CMD_LOW | SLCD_CFG_CLK_ACTIVE_FALLING 
			| SLCD_CFG_TYPE_PARALLEL;
		__gpio_as_slcd_9bit();
		break;
	case 16:
		REG_SLCD_CFG = SLCD_CFG_BURST_8_WORD | SLCD_CFG_DWIDTH_16
			| SLCD_CFG_CWIDTH_16BIT | SLCD_CFG_CS_ACTIVE_LOW
			| SLCD_CFG_RS_CMD_LOW | SLCD_CFG_CLK_ACTIVE_FALLING
			| SLCD_CFG_TYPE_PARALLEL;
		__gpio_as_slcd_16bit();
		break;
	case 18:
		REG_SLCD_CFG = SLCD_CFG_BURST_8_WORD | SLCD_CFG_DWIDTH_18
			| SLCD_CFG_CWIDTH_18BIT | SLCD_CFG_CS_ACTIVE_LOW 
			| SLCD_CFG_RS_CMD_LOW | SLCD_CFG_CLK_ACTIVE_FALLING 
			| SLCD_CFG_TYPE_PARALLEL;
		__gpio_as_slcd_18bit();
		break;
	default:
		printf("Error: Don't support BUS %d!\n", jzfb_slcd.bus);
		break;
	}

	REG_SLCD_CTRL = SLCD_CTRL_DMA_EN;
	__cpm_stop_lcd();
	pclk = jzfb_slcd.pclk;
	pll_div = ( REG_CPM_CPCCR & CPM_CPCCR_PCS ); /* clock source,0:pllout/2 1: pllout */
	pll_div = pll_div ? 1 : 2 ;
	val = ( __cpm_get_pllout()/pll_div ) / pclk;
	val--;
	if ( val > 0x1ff ) {
		printf("CPM_LPCDR too large, set it to 0x1ff\n");
		val = 0x1ff;
	}
	__cpm_set_pixdiv(val);

	REG_CPM_CPCCR |= CPM_CPCCR_CE ; /* update divide */

	printf("SLCDC: PixClock:%d LcdClock:%d\n",
	        __cpm_get_pixclk(), __cpm_get_lcdclk());

	__cpm_start_lcd();
	udelay(1000);
	slcd_board_init();
	/* Configure SLCD module for transfer data to smart lcd GRAM*/
	switch (jzfb_slcd.bus) {
	case 8:
		switch (jzfb_slcd.bpp) {
		case 8:
			REG_SLCD_CFG &= ~SLCD_CFG_DWIDTH_MASK;
			REG_SLCD_CFG |= SLCD_CFG_DWIDTH_8_x1;
			break;
		case 15:
		case 16:
			REG_SLCD_CFG &= ~SLCD_CFG_DWIDTH_MASK;
			REG_SLCD_CFG |= SLCD_CFG_DWIDTH_8_x2;
			break;
		case 17 ... 32:
			REG_SLCD_CFG &= ~SLCD_CFG_DWIDTH_MASK;
			REG_SLCD_CFG |= SLCD_CFG_DWIDTH_8_x3;
			break;
		default:
			printf("The BPP %d is not supported\n", jzfb_slcd.bpp);
			break;
		}
		break;
	case 9:
		switch (jzfb_slcd.bpp) {
		case 18:
			REG_SLCD_CFG &= ~SLCD_CFG_DWIDTH_MASK;
			REG_SLCD_CFG |= SLCD_CFG_DWIDTH_9_x2;
			break;
		default:
			printf("The BPP %d is not supported\n", jzfb_slcd.bpp);
			break;
		}
		break;
	case 16:
		switch (jzfb_slcd.bpp) {
		case 15 ... 16:
			REG_SLCD_CFG &= ~SLCD_CFG_DWIDTH_MASK;
			REG_SLCD_CFG |= SLCD_CFG_DWIDTH_16;
			break;
		case 17 ... 32:
			REG_SLCD_CFG &= ~SLCD_CFG_DWIDTH_MASK;
			REG_SLCD_CFG |= SLCD_CFG_DWIDTH_9_x2;
//			REG_SLCD_CFG |= SLCD_CFG_DWIDTH_16;
			break;
		default:
			printf("The BPP %d is not supported\n", jzfb_slcd.bpp);
			break;
		}
		break;
	case 18:
		switch (jzfb_slcd.bpp) {
		case 17 ... 32:
			REG_SLCD_CFG &= ~SLCD_CFG_DWIDTH_MASK;
			REG_SLCD_CFG |= SLCD_CFG_DWIDTH_18;
			break;
		default:
			printf("The BPP %d is not supported\n", jzfb_slcd.bpp);
			jzfb_slcd.bpp = 18;
			break;
		}
		break;
	default:
		printf("Error: The BUS %d is not supported\n", jzfb_slcd.bus);
		break;
	}
	dprintf("SLCD_CFG=0x%x\n", REG_SLCD_CFG);
	return 0;
}

/*The initialization of smart lcd*/
int jzlcd_init(void)
{
	int err = 0;
	printf("SLCD Init!\n");

	err = fb_malloc();
	if (err) 
		goto failed;

	err = controller_init();
	if (err)
		goto failed;

	lcd_descriptor_init();
	__dcache_writeback_all();
	err = slcd_dma_init();
	if (err)
		goto failed;
	__slcd_set_backlight_level(80);
	return 0;

failed:
	return err;

}

#if (DM==1)
int  lcd_poweron(void)
{
}
int  lcd_poweroff(void)
{
}
int lcd_preconvert(void)
{
//	__cpm_stop_lcd();

	return 1;
}
int  lcd_convert(void)
{

}
void  mng_init_lcd(void)
{
	struct dm_jz4740_t lcd_dm;
	lcd_dm.name = "SLCD driver";	
	lcd_dm.init = jzlcd_init;  
	lcd_dm.poweron = lcd_poweron;
	lcd_dm.poweroff = lcd_poweroff;
	lcd_dm.convert = lcd_convert;
	lcd_dm.preconvert = lcd_preconvert;
	dm_register(0,&lcd_dm);
}
void lcdstop()
{
	__slcd_close_backlight();
	__dmac_disable_channel(dma_chan);
	__slcd_dma_disable(); 
	__slcd_special_off(); 
	__cpm_stop_lcd();

}
void lcdstart()
{
	controller_init();
	__slcd_set_backlight_level(80);
}
#endif
