#include "zklogo.h"

#define bitvalue(s,c) (((c) & (1<<(s))) ? 1:0)
extern int lcm_write_cmd(unsigned char cmd);
extern int lcm_write_data(unsigned char data);
unsigned char page_count=64/8;
unsigned char lcdwidth=128;

static void lcdm_cog_putscrn(unsigned char *buf)
{
        int  x, y, i;
        unsigned char c;
        unsigned char *s = buf;
	int lcdconvert = getLCDType();

        for (x = 0; x < page_count; x++) {
                lcm_write_cmd(0xb0|x);
                y = 1;
                lcm_write_cmd(0x10 |((y&0xf0) >> 4));
		if (lcdconvert)
	                lcm_write_cmd(0x00 |((y+2)&0x0f));
		else	
			lcm_write_cmd(0x00 |(y&0x0f));
                for (; y <= lcdwidth; y++) {
                        for (c=0,i=0;i<8;i++) {
                                c <<= 1;
                                c |= bitvalue(7-((y-1)%8), s[(x*8+7-i)*16+(y-1)/8]);
                        }
                        lcm_write_data(c);
                }
        }
}

void LCDShowLogo(void)
{
	InitializeLCD();
        lcdm_cog_putscrn(logo_buf);
	/* wdt will reboot system after 60s */
	wdt_initialize(60);	
	wdt_enable(1);
//	printf("Finished show logo .. \n");
}



