#include "lcdc.h"
struct jzfb_info jzfb = {
#if LCDTYPE == 1
	 MODE_TFT_18BIT | MODE_TFT_GEN | HSYNC_N | VSYNC_N,
	480, 272, 18, 60, 41, 10, 2, 2, 2, 2
#endif
#if LCDTYPE == 2
	MODE_8BIT_SERIAL_TFT | PCLK_N | HSYNC_N | VSYNC_N,
	320, 240, 32, 60, 1, 1, 10, 50, 10, 13
#endif
#if LCDTYPE == 4
	MODE_TFT_GEN | MODE_TFT_18BIT | HSYNC_N | VSYNC_N,
	480, 272, 18, 60, 39, 10, 8, 4, 4, 2 
#endif
};
