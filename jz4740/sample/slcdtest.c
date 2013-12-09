/* jz4740/sample/slcdtest.c 
 *Test smart lcd; use generic dma with descriptor
 *Author:zyliu@ingenic.cn
 */

#include "slcdc.h"
#include <jz4740.h>
# include "slcdc.h"

static int dma_chan = 0;
void slcd_test()
{
	if (jzfb_slcd.bpp == 16) {
		unsigned short *frame;
		unsigned int i;
		
		jzlcd_init();
		lcd_enter_frame();
		frame = (unsigned short *)lcd_get_current_frame();
		printf("slcd frame = 0x%x\r\n",frame);
		for (i = 0;i < 400 * 240;i++) {
			frame[i] = 0xf81f;
		}
		printf("16-bpp, data = 0x%08x\n",frame[0]);

		lcd_change_frame();
		frame = (unsigned short *)lcd_get_current_frame();
		printf("slcd frame = 0x%x\r\n",frame);
		for (i = 0;i < 400 * 240/2;i++) {
			frame[i] = 0x07ff;
		}
		printf("16-bpp, data = 0x%08x\n", frame[0]);
		for (;i < 400 * 240;i++) {
			frame[i] = 0xffe0;
		}
		printf("16-bpp, data = 0x%08x\n", frame[i-1]);
		
		lcd_change_frame();
		frame = (unsigned short *)lcd_get_current_frame();
		
		for (i = 0;i < 400 * 80;i++) {
			frame[i] = 0x001f;
		}
		printf("16-bpp, data = 0x%08x\n", frame[i-1]);
		for (;i < 400 * 160;i++) {
			frame[i] = 0x07e0;
		}
		printf("16-bpp, data = 0x%08x\n", frame[i-1]);
		for (;i < 400 * 240;i++) {
			frame[i] = 0xf800;
		}
		printf("16-bpp, data = 0x%08x\n", frame[i-1]);
		lcd_change_frame();
//		lcd_reset_frame();
		while (1) {
//			mdelay(1000);
			lcd_change_frame();
			
		}
	}
	else { /*bpp =18*/
		unsigned int *frame;
		unsigned int i;
		
		jzlcd_init();
		frame = (unsigned int *)jzfb_slcd.frame;
		
		printf("slcd test = 0x%x 0x%x 0x%x\r\n",frame,jzfb_slcd.frame,&jzfb_slcd);
		while(1) {
			for (i = 0;i < 400 * 240;i++) {
				
				frame[i] = 0x000000ff;
//				frame[i] = 0x0000ff00;
//				frame[i] = 0x00ff0000;
			}
			__dcache_writeback_all();
			printf("18-bpp, data = 0x%08x\n", frame[0]);
#if 0			/*enable it if use single refresh smart lcd*/
			while (REG_SLCD_STATE & SLCD_STATE_BUSY);
			__dmac_channel_set_doorbell(dma_chan);
			mdelay(100);
			dump_jz_dma_channel(dma_chan);
#endif
			mdelay(2000);
			for (i = 0;i < 400 * 240;i++) {
//				frame[i] = 0x00ff0000;
				frame[i] = 0x0000ff00;
//				frame[i] = 0x000000ff;

			}
			__dcache_writeback_all();
			printf("18-bpp, data = 0x%08x\n", frame[0]);
#if 0			/*enable it if use single refresh smart lcd*/
			while (REG_SLCD_STATE & SLCD_STATE_BUSY);
			__dmac_channel_set_doorbell(dma_chan);
			mdelay(100);
			dump_jz_dma_channel(dma_chan);
#endif
			mdelay(2000);
			for(i = 0;i < 400 * 240;i++)
			{
//				frame[i] = 0x00ff0000;
//				frame[i] = 0x0000ff00;
				frame[i] = 0x000000ff;

			}
			__dcache_writeback_all();
			printf("18-bpp, data = 0x%08x\n", frame[0]);
#if 0			/*enable it if use single refresh smart lcd*/
			while (REG_SLCD_STATE & SLCD_STATE_BUSY);
			__dmac_channel_set_doorbell(dma_chan);
			mdelay(100);
			dump_jz_dma_channel(dma_chan);
#endif
			mdelay(2000);
		}
	}
}
#if 0
void slcd_test()
{
	if (jzfb_slcd.bpp == 16) {
		unsigned short *frame;
		unsigned int i;
		
		jzlcd_init();
		frame = (unsigned short *)jzfb_slcd.frame;
		
		printf("slcd test = 0x%x 0x%x 0x%x\r\n",frame,jzfb_slcd.frame,&jzfb_slcd);
		while(1) {
			for (i = 0;i < 400 * 240;i++) {
				
				frame[i] = 0xf81f;
			}
			__dcache_writeback_all();
			printf("16-bpp, data = 0x%08x\n", frame[0]);
#if 0			/*enable it if use single refresh smart lcd*/
			while (REG_SLCD_STATE & SLCD_STATE_BUSY);
			__dmac_channel_set_doorbell(dma_chan);
			mdelay(100);
			dump_jz_dma_channel(dma_chan);
#endif
			mdelay(2000);
			for (i = 0;i < 400 * 240;i++) {
				frame[i] = 0x07e0;
			}
			__dcache_writeback_all();
			printf("16-bpp, data = 0x%08x\n", frame[0]);
#if 0			/*enable it if use single refresh smart lcd*/
			while (REG_SLCD_STATE & SLCD_STATE_BUSY);
			__dmac_channel_set_doorbell(dma_chan);
			mdelay(100);
			dump_jz_dma_channel(dma_chan);
#endif

			mdelay(2000);
			for (i = 0;i < 400 * 240;i++) {
			frame[i] = 0x00ff;
			}
			__dcache_writeback_all();
			printf("16-bpp, data = 0x%08x\n", frame[0]);
#if 0			/*enable it if use single refresh smart lcd*/
			while (REG_SLCD_STATE & SLCD_STATE_BUSY);
			__dmac_channel_set_doorbell(dma_chan);
			mdelay(100);
			dump_jz_dma_channel(dma_chan);
#endif
			mdelay(2000);
			for(i = 0;i < 400 * 240;i++)
			{
				frame[i] = 0xffe0;
			}
			__dcache_writeback_all();
			printf("16-bpp, data = 0x%08x\n", frame[0]);
#if 0			/*enable it if use single refresh smart lcd*/
			while (REG_SLCD_STATE & SLCD_STATE_BUSY);
			__dmac_channel_set_doorbell(dma_chan);
			dump_jz_dma_channel(dma_chan);
#endif
			printf("REG_DMAC_DMADBR = 0x%08x\n", REG_DMAC_DMADBR);
			mdelay(2000);
		}
	}
	else { /*bpp =18*/
		unsigned int *frame;
		unsigned int i;
		
		jzlcd_init();
		frame = (unsigned int *)jzfb_slcd.frame;
		
		printf("slcd test = 0x%x 0x%x 0x%x\r\n",frame,jzfb_slcd.frame,&jzfb_slcd);
		while(1) {
			for (i = 0;i < 400 * 240;i++) {
				
				frame[i] = 0x000000ff;
//				frame[i] = 0x0000ff00;
//				frame[i] = 0x00ff0000;
			}
			__dcache_writeback_all();
			printf("18-bpp, data = 0x%08x\n", frame[0]);
#if 0			/*enable it if use single refresh smart lcd*/
			while (REG_SLCD_STATE & SLCD_STATE_BUSY);
			__dmac_channel_set_doorbell(dma_chan);
			mdelay(100);
			dump_jz_dma_channel(dma_chan);
#endif
			mdelay(2000);
			for (i = 0;i < 400 * 240;i++) {
//				frame[i] = 0x00ff0000;
				frame[i] = 0x0000ff00;
//				frame[i] = 0x000000ff;

			}
			__dcache_writeback_all();
			printf("18-bpp, data = 0x%08x\n", frame[0]);
#if 0			/*enable it if use single refresh smart lcd*/
			while (REG_SLCD_STATE & SLCD_STATE_BUSY);
			__dmac_channel_set_doorbell(dma_chan);
			mdelay(100);
			dump_jz_dma_channel(dma_chan);
#endif
			mdelay(2000);
			for(i = 0;i < 400 * 240;i++)
			{
//				frame[i] = 0x00ff0000;
//				frame[i] = 0x0000ff00;
				frame[i] = 0x000000ff;

			}
			__dcache_writeback_all();
			printf("18-bpp, data = 0x%08x\n", frame[0]);
#if 0			/*enable it if use single refresh smart lcd*/
			while (REG_SLCD_STATE & SLCD_STATE_BUSY);
			__dmac_channel_set_doorbell(dma_chan);
			mdelay(100);
			dump_jz_dma_channel(dma_chan);
#endif
			mdelay(2000);
		}
	}
}
#endif

