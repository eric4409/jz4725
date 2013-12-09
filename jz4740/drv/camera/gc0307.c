#include "gc0307.h"

static int m_sensor_fd=-1;
static int m_sensor_opt = 0;
static int m_sensitivity = 0;
static int m_image_reverse=1;
static int m_sensitivity1=1;
static int m_image_symmetry=1;

/* integration time */
static unsigned int integration_time = 35; /* unit: ms */

/* master clock and video clock */
static unsigned int mclk_hz = 25000000;    /* 25 MHz */
static unsigned int vclk_div = 2;          /* VCLK = MCLK/vclk_div: 2,4,8,16,32 */

extern int sensor_write_reg(unsigned char reg, unsigned char val);
extern int sensor_read_reg(unsigned char reg);

void sensor_power_down_GC307(void)
{
#if 0
	unsigned char v;
	sensor_write_reg(0x43, 0x00); 
	v = sensor_read_reg(0x44); 
	sensor_write_reg(0x44, v&~(0x40));
#endif 
}

void sensor_power_on_GC307(void)
{
	unsigned char v;
#if 0
	sensor_write_reg(0x43, 0x40);
	v = sensor_read_reg(0x44); 
	sensor_write_reg(0x44, v|0x40); 
#endif
}

void set_sensitivity_GC3071()
{
	int exposH = 0x0250;

	//æ— è†œæŒ‡çº¹å¤?	
	if(m_image_reverse==1)
	{
		switch(m_sensitivity1)
		{
			case 0://low    //é€‚åˆå¹²æ‰‹æŒ?				//é’ˆå¯¹0307å°æŒ‡çº¹å¤´
				if(m_image_symmetry==1)
					exposH = 0x07D0;
				else
					exposH = 0x08D0;
				break;
			case 1://middle //å¹²æ¹¿æ‰‹æŒ‡å¹¶ç”¨çš„æƒ…å†?				//é’ˆå¯¹0307å°æŒ‡çº¹å¤´
				if(m_image_symmetry==1)
					exposH = 0x050;
				else
					exposH = 0x0250;
				break;
			case 2://high   //é€‚åˆæ¹¿æ‰‹æŒ?				
				if(m_image_symmetry==1)
					exposH = 0x0100;
				else
					exposH = 0x01C0;
				break;
			default:
				if(m_image_symmetry==1)
					exposH = 0x0150;
				else
					exposH = 0x0250;
				break;
		}
	}
	//æœ‰è†œæŒ‡çº¹å¤´ï¼Œæš‚æ—¶æ²¡æœ‰ç”?0100727
	else if(m_image_reverse==0)
	{
		exposH = 0xe0;
	}
	set_EXPOS_GC307(exposH);
	printf("exposH=%d\n",exposH);
}
/*
void set_sensitivity_GC3071()
{
	int exposH = 0x400;//0x0250;
	//if(m_sensor_opt!=OPT_FLAG_GC0307)
		//printf(ZKPRINTF_MODE1, "%s() doesn't support 0307 m_sensor_opt=%d\n", __FUNCTION__, m_sensor_opt);

	//ÎÞÄ¤Ö¸ÎÆÍ·
	if(m_image_reverse==1)
	{
		switch(m_sensitivity1)
		{
		case 0://low	//ÊÊºÏ¸ÉÊÖÖ¸
			exposH = 0x300;//0x08D0;
			break;
		case 1://middle	//¸ÉÊªÊÖÖ¸²¢ÓÃµÄÇé¿ö
			exposH = 0x400;
			break;
		case 2://high	//ÊÊºÏÊªÊÖÖ¸
			exposH = 0x500;
			break;
		default:
			exposH = 0x400;//0x0250;
			//printf(ZKPRINTF_MODE1, "%s() Error: Default m_sensitivity=%d\n", __FUNCTION__, m_sensitivity);
			break;
		}
	}
	//ÓÐÄ¤Ö¸ÎÆÍ·£¬ÔÝÊ±Ã»ÓÐÓÃ20100727
	else if(m_image_reverse==0)
	{
		exposH = 0x400;//0x0250;
	}
	//printf(ZKPRINTF_MODE1, "%s() exposH=0x%x\n", __FUNCTION__, exposH);
	set_EXPOS_GC307(exposH);

	printf("exposH=%d\n",exposH);
//`	set_RGBGain_GCC307(0, 0, 0, 0, 0x18);

}*/
//è®¾ç½®å›¾åƒæ˜¯å¦éœ€è¦å·¦å³é•œåƒå¤„ç?
void set_symmetry_GC307(int symmetry)
{
	if(symmetry==1)
		sensor_write_reg(0x0F  ,0x30);//å›¾åƒéœ€è¦åž‚ç›´ç¿»è½?	
	else
		sensor_write_reg(0x0F  ,0x20);//å›¾åƒä¸éœ€è¦åž‚ç›´ç¿»è½?
}
static void set_window_GC307(int l, int t, int w, int h)
{
    //    l=(640 - w)/2*2;
    //    t=(480 - h)/2*2;

        /* Set the row start address */
        sensor_write_reg(GC0307_ROW_H, (t >> 8)&0x03);
        sensor_write_reg(GC0307_ROW_L, t&0xff);

        /* Set the column start address */
        sensor_write_reg(GC0307_COL_H, (l >> 8)&0x03);
        sensor_write_reg(GC0307_COL_L, l&0xff);

        /* Set the image window width*/
        sensor_write_reg(GC0307_WINW_L, w&0xff);
        sensor_write_reg(GC0307_WINW_H, (w >> 8)&0x03);

        /* Set the image window height*/
        sensor_write_reg(GC0307_WINH_L, (h)&0xff);
        sensor_write_reg(GC0307_WINH_H, (h>> 8)&0x01);

/*	int v = sensor_read_reg(0x09);
	int v1 = sensor_read_reg(0x0a);
	printf("-----+---- h:%02x, %02x\n",v, v1);

	v = sensor_read_reg(GC0307_WINW_H);
	v1 = sensor_read_reg(GC0307_WINW_L);
	printf("--------- w:%02x, %02x\n",v, v1);
*/
}

/* VCLK = MCLK/div */
static void set_sensor_clock_gc307(int div)
{
#define ABLC_EN (0xC0)
        /* ABLC enable */
        switch (div) {
        case 2:
                sensor_write_reg(GC0307_SCTRA, ABLC_EN | 0x00);       // DCF=MCLK
                break;
        case 4:
                sensor_write_reg(GC0307_SCTRA, ABLC_EN | 0x04);       // DCF=MCLK/2
                break;
        case 8:
                sensor_write_reg(GC0307_SCTRA, ABLC_EN | 0x08);       // DCF=MCLK/4
                break;
        case 16:
                sensor_write_reg(GC0307_SCTRA, ABLC_EN | 0x0C);       // DCF=MCLK/8
                break;
        default:
                break;
        }
}

void set_EXPOS_GC307(int exposH)
{
        sensor_write_reg(GC0307_EXP_L, exposH&0xff);
        sensor_write_reg(GC0307_EXP_H, (exposH>>8)&0xff);
}

static void set_sensitivity_GC307()
{
        int exposH=0;
        //expose
        if(m_sensor_opt & 0x01)
        {
                switch(m_sensitivity)
                {
                case 0://low
                        exposH = 0x02;
                        break;
                case 2://high
                        exposH = 0x0F;
                        break;
                case 1://middle
                default:
                        exposH = 0x00;
                        break;
                }
                set_EXPOS_GC307(exposH);
        }else
        {
                set_EXPOS_GC307(0x02);
        }
}

void _initgc307(void)
{
   	sensor_write_reg(0xf0  ,0x00);
   	sensor_write_reg(0x43  ,0x00);
	sensor_write_reg(0x44  ,0xa2);

   	sensor_write_reg(0x03  ,0x00);
	sensor_write_reg(0x04  ,0x90);
	//========= close some functions
	// open them after configure their parmameters
	sensor_write_reg(0x40  ,0x10);
	sensor_write_reg(0x41  ,0x00);
	sensor_write_reg(0x42  ,0x10);
	sensor_write_reg(0x47  ,0x00);//mode1,
	//sensor_write_reg(,0x48  ,0xc3);//0xc3);//mode2,
	sensor_write_reg(0x49  ,0x00);//dither_mode
	sensor_write_reg(0x4a  ,0x00);//clock_gating_en
	sensor_write_reg(0x4b  ,0x00);//mode_reg3
	sensor_write_reg(0x4E  ,0x06);//sync mode
	sensor_write_reg(0x4F  ,0x01);//AWB, AEC, every N frame

	//========= frame timing
	sensor_write_reg(0x01  ,0x6a);//HB
	sensor_write_reg(0x02  ,0x25); //VB
	sensor_write_reg(0x1C  ,0x00);//Vs_st
	sensor_write_reg(0x1D  ,0x00);//Vs_et
	sensor_write_reg(0x10  ,0x00);//high 4 bits of VB, HB
	sensor_write_reg(0x11  ,0x05);//row_tail,  AD_pipe_number


	//========= windowing
	sensor_write_reg(0x05  ,0x00);//row_start
	sensor_write_reg(0x06  ,0x00);
	sensor_write_reg(0x07  ,0x00);//col start
	sensor_write_reg(0x08  ,0x00);
	sensor_write_reg(0x09  ,0x01);//win height
	sensor_write_reg(0x0A  ,0xE8);
	sensor_write_reg(0x0B  ,0x02);//win width, pixel array only 640
	sensor_write_reg(0x0C  ,0x80);
//	sensor_write_reg(,0x0F  ,0x32);

	//========= analog
	sensor_write_reg(0x0D  ,0x22);//rsh_width
	sensor_write_reg(0x0E  ,0x02);//CISCTL mode2, zsliu for zem800
	//sensor_write_reg(,0x0E  ,0x42);//CISCTL mode2,


	sensor_write_reg(0x12  ,0x70);//7 hrst, 6_4 darsg,
	sensor_write_reg(0x13  ,0x00);//7 CISCTL_restart, 0 apwd
	sensor_write_reg(0x14  ,0x00);//NA
	sensor_write_reg(0x15  ,0xba);//7_4 vref
	sensor_write_reg(0x16  ,0x13);//5to4 _coln_r,  __1to0__da18
	sensor_write_reg(0x17  ,0x52);//opa_r, ref_r, sRef_r
	sensor_write_reg(0x18  ,0xc0);//analog_mode, best case for left band.

	sensor_write_reg(0x1E  ,0x0d);//tsp_width
	sensor_write_reg(0x1F  ,0x32);//sh_delay

	//========= offset
	sensor_write_reg(0x47  ,0x00); //7__test_image, __6__fixed_pga, __5__auto_DN, __4__CbCr_fix,

	sensor_write_reg(0x19  ,0x06); //pga_o
	sensor_write_reg(0x1a  ,0x06); //pga_e

	sensor_write_reg(0x31  ,0x00); //4	//pga_oFFset ,	 high 8bits of 11bits
	sensor_write_reg(0x3B  ,0x00); //global_oFFset, low 8bits of 11bits

	sensor_write_reg(0x59  ,0x0f); //offset_mode
	sensor_write_reg(0x58  ,0x88); //DARK_VALUE_RATIO_G,  DARK_VALUE_RATIO_RB
	sensor_write_reg(0x57  ,0x08); //DARK_CURRENT_RATE
	sensor_write_reg(0x56  ,0x77); //PGA_OFFSET_EVEN_RATIO, PGA_OFFSET_ODD_RATIO

	//========= blk
	sensor_write_reg(0x35  ,0xd8); //blk_mode

	sensor_write_reg(0x36  ,0x40);

	sensor_write_reg(0x3C  ,0x00);
	sensor_write_reg(0x3D  ,0x00);
	sensor_write_reg(0x3E  ,0x00);
	sensor_write_reg(0x3F  ,0x00);

	sensor_write_reg(0xb5  ,0x70);
	sensor_write_reg(0xb6  ,0x40);
	sensor_write_reg(0xb7  ,0x00);
	sensor_write_reg(0xb8  ,0x38);
	sensor_write_reg(0xb9  ,0xc3);
	sensor_write_reg(0xba  ,0x0f);

	sensor_write_reg(0x7e  ,0x45);
	sensor_write_reg(0x7f  ,0x66);

	sensor_write_reg(0x5c  ,0x48);//78
	sensor_write_reg(0x5d  ,0x58);//88


	//========= manual_gain
	sensor_write_reg(0x61  ,0x80);//manual_gain_g1
	sensor_write_reg(0x63  ,0x80); //manual_gain_r
	sensor_write_reg(0x65  ,0x98); //manual_gai_b, 0xa0=1.25, 0x98=1.1875
	sensor_write_reg(0x67  ,0x80);//manual_gain_g2
	sensor_write_reg(0x68  ,0x18);//global_manual_gain	 2.4bits

	//=========CC _R
	sensor_write_reg(0x69  ,0x58); //54
	sensor_write_reg(0x6A  ,0xf6); //ff
	sensor_write_reg(0x6B  ,0xfb); //fe
	sensor_write_reg(0x6C  ,0xf4); //ff
	sensor_write_reg(0x6D  ,0x5a); //5f
	sensor_write_reg(0x6E  ,0xe6); //e1

	sensor_write_reg(0x6f  ,0x00);

	//=========lsc
	sensor_write_reg(0x70  ,0x14);
	sensor_write_reg(0x71  ,0x1c);
	sensor_write_reg(0x72  ,0x20);

	sensor_write_reg(0x73  ,0x10);
	sensor_write_reg(0x74  ,0x3c);
	sensor_write_reg(0x75  ,0x52);

	//=========dn
	sensor_write_reg(0x7d  ,0x2f); //dn_mode
	sensor_write_reg(0x80  ,0x0c);//when auto_dn, check 7e,7f
	sensor_write_reg(0x81  ,0x0c);
	sensor_write_reg(0x82  ,0x44);

	//dd
	sensor_write_reg(0x83  ,0x18); //DD_TH1
	sensor_write_reg(0x84  ,0x18); //DD_TH2
	sensor_write_reg(0x85  ,0x04); //DD_TH3
	sensor_write_reg(0x87  ,0x34); //32 b DNDD_low_range X16,  DNDD_low_range_C_weight_center


	//=========intp-ee
	sensor_write_reg(0x88  ,0x04);
	sensor_write_reg(0x89  ,0x01);
	sensor_write_reg(0x8a  ,0x50);//60
	sensor_write_reg(0x8b  ,0x50);//60
	sensor_write_reg(0x8c  ,0x07);

	sensor_write_reg(0x50  ,0x0c);
	sensor_write_reg(0x5f  ,0x3c);

	sensor_write_reg(0x8e  ,0x02);
	sensor_write_reg(0x86  ,0x02);

	sensor_write_reg(0x51  ,0x20);
	sensor_write_reg(0x52  ,0x08);
	sensor_write_reg(0x53  ,0x00);


	//========= YCP
	//contrast_center
	sensor_write_reg(0x77  ,0x80);//contrast_center
	sensor_write_reg(0x78  ,0x00);//fixed_Cb
	sensor_write_reg(0x79  ,0x00);//fixed_Cr
	sensor_write_reg(0x7a  ,0x00);//luma_offset
	sensor_write_reg(0x7b  ,0x40);//hue_cos
	sensor_write_reg(0x7c  ,0x00);//hue_sin

	//saturation
	sensor_write_reg(0xa0  ,0x40);//global_saturation
	sensor_write_reg(0xa1  ,0x40);//luma_contrast
	sensor_write_reg(0xa2  ,0x34);//saturation_Cb
	sensor_write_reg(0xa3  ,0x34);//saturation_Cr

	sensor_write_reg(0xa4  ,0xc8);
	sensor_write_reg(0xa5  ,0x02);
	sensor_write_reg(0xa6  ,0x28);
	sensor_write_reg(0xa7  ,0x02);

	//skin
	sensor_write_reg(0xa8  ,0xee);
	sensor_write_reg(0xa9  ,0x12);
	sensor_write_reg(0xaa  ,0x01);
	sensor_write_reg(0xab  ,0x20);
	sensor_write_reg(0xac  ,0xf0);
	sensor_write_reg(0xad  ,0x10);

	//========= ABS
	sensor_write_reg(0xae  ,0x18);
	sensor_write_reg(0xaf  ,0x74);
	sensor_write_reg(0xb0  ,0xe0);
	sensor_write_reg(0xb1  ,0x20);
	sensor_write_reg(0xb2  ,0x6c);
	sensor_write_reg(0xb3  ,0xc0); 
	sensor_write_reg(0xb4  ,0x04);

	//========= AWB
	sensor_write_reg(0xbb  ,0x42);
	sensor_write_reg(0xbc  ,0x60);
	sensor_write_reg(0xbd  ,0x50);
	sensor_write_reg(0xbe  ,0x50);

	sensor_write_reg(0xbf  ,0x0c);
	sensor_write_reg(0xc0  ,0x06);
	sensor_write_reg(0xc1  ,0x60);
	sensor_write_reg(0xc2  ,0xf1); //f1
	sensor_write_reg(0xc3  ,0x40);
	sensor_write_reg(0xc4  ,0x1c);//18//20
	sensor_write_reg(0xc5  ,0x56); //33
	sensor_write_reg(0xc6  ,0x1d);

	sensor_write_reg(0xca  ,0x56); //70
	sensor_write_reg(0xcb  ,0x52); //70
	sensor_write_reg(0xcc  ,0x66); //78

	sensor_write_reg(0xcd  ,0x80);//R_ratio
	sensor_write_reg(0xce  ,0x80);//G_ratio  , cold_white white
	sensor_write_reg(0xcf  ,0x80);//B_ratio

	//=========  aecT
	sensor_write_reg(0x20  ,0x06);//0x02
	sensor_write_reg(0x21  ,0xc0);
	sensor_write_reg(0x22  ,0x60);
	sensor_write_reg(0x23  ,0x88);
	sensor_write_reg(0x24  ,0x96);
	sensor_write_reg(0x25  ,0x30);
	sensor_write_reg(0x26  ,0xd0);
	sensor_write_reg(0x27  ,0x00);

	sensor_write_reg(0x28  ,0x02); //AEC_exp_level_1bit11to8   
	sensor_write_reg(0x29  ,0x58); //AEC_exp_level_1bit7to0	  
	sensor_write_reg(0x2a  ,0x02); //AEC_exp_level_2bit11to8   
	sensor_write_reg(0x2b  ,0x58); //AEC_exp_level_2bit7to0			 
	sensor_write_reg(0x2c  ,0x02); //AEC_exp_level_3bit11to8   659 - 8FPS,  8ca - 6FPS  //	 
	sensor_write_reg(0x2d  ,0x58); //AEC_exp_level_3bit7to0			 
	sensor_write_reg(0x2e  ,0x05); //AEC_exp_level_4bit11to8   4FPS 
	sensor_write_reg(0x2f  ,0xdc); //AEC_exp_level_4bit7to0	 

	sensor_write_reg(0x30  ,0x20);
	sensor_write_reg(0x31  ,0x00);
	sensor_write_reg(0x32  ,0x1c);
	sensor_write_reg(0x33  ,0x90);
	sensor_write_reg(0x34  ,0x10);

	sensor_write_reg(0xd0  ,0x34);

	sensor_write_reg(0xd1  ,0x50);//AEC_target_Y
	sensor_write_reg(0xd2  ,0x61);//0xf2
	sensor_write_reg(0xd4  ,0x96);
	sensor_write_reg(0xd5  ,0x01);// william 0318
	sensor_write_reg(0xd6  ,0x96);//antiflicker_step
	sensor_write_reg(0xd7  ,0x03);//AEC_exp_time_min ,william 20090312
	sensor_write_reg(0xd8  ,0x02);

	sensor_write_reg(0xdd  ,0x22);//0x12

	//========= measure window
	sensor_write_reg(0xe0  ,0x03);
	sensor_write_reg(0xe1  ,0x02);
	sensor_write_reg(0xe2  ,0x27);
	sensor_write_reg(0xe3  ,0x1e);
	sensor_write_reg(0xe8  ,0x3b);
	sensor_write_reg(0xe9  ,0x6e);
	sensor_write_reg(0xea  ,0x2c);
	sensor_write_reg(0xeb  ,0x50);
	sensor_write_reg(0xec  ,0x73);

	//========= close_frame
	sensor_write_reg(0xed  ,0x00);//close_frame_num1 ,can be use to reduce FPS
	sensor_write_reg(0xee  ,0x00);//close_frame_num2
	sensor_write_reg(0xef  ,0x00);//close_frame_num

	// page1
	sensor_write_reg(0xf0  ,0x01);//select page1

	sensor_write_reg(0x00  ,0x20);
	sensor_write_reg(0x01  ,0x20);
	sensor_write_reg(0x02  ,0x20);
	sensor_write_reg(0x03  ,0x20);
	sensor_write_reg(0x04  ,0x78);
	sensor_write_reg(0x05  ,0x78);
	sensor_write_reg(0x06  ,0x78);
	sensor_write_reg(0x07  ,0x78);



	sensor_write_reg(0x10  ,0x04);
	sensor_write_reg(0x11  ,0x04);
	sensor_write_reg(0x12  ,0x04);
	sensor_write_reg(0x13  ,0x04);
	sensor_write_reg(0x14  ,0x01);
	sensor_write_reg(0x15  ,0x01);
	sensor_write_reg(0x16  ,0x01);
	sensor_write_reg(0x17  ,0x01);


	sensor_write_reg(0x20  ,0x00);
	sensor_write_reg(0x21  ,0x00);
	sensor_write_reg(0x22  ,0x00);
	sensor_write_reg(0x23  ,0x00);
	sensor_write_reg(0x24  ,0x00);
	sensor_write_reg(0x25  ,0x00);
	sensor_write_reg(0x26  ,0x00);
	sensor_write_reg(0x27  ,0x00);

	sensor_write_reg(0x40  ,0x11);

	//=============================lscP
	sensor_write_reg(0x45  ,0x06);
	sensor_write_reg(0x46  ,0x06);
	sensor_write_reg(0x47  ,0x05);

	sensor_write_reg(0x48  ,0x04);
	sensor_write_reg(0x49  ,0x03);
	sensor_write_reg(0x4a  ,0x03);


	sensor_write_reg(0x62  ,0xd8);
	sensor_write_reg(0x63  ,0x24);
	sensor_write_reg(0x64  ,0x24);
	sensor_write_reg(0x65  ,0x24);
	sensor_write_reg(0x66  ,0xd8);
	sensor_write_reg(0x67  ,0x24);

	sensor_write_reg(0x5a  ,0x00);
	sensor_write_reg(0x5b  ,0x00);
	sensor_write_reg(0x5c  ,0x00);
	sensor_write_reg(0x5d  ,0x00);
	sensor_write_reg(0x5e  ,0x00);
	sensor_write_reg(0x5f  ,0x00);


	//============================= ccP

	sensor_write_reg(0x69  ,0x03);//cc_mode

	//CC_G
	sensor_write_reg(0x70  ,0x5d);
	sensor_write_reg(0x71  ,0xed);
	sensor_write_reg(0x72  ,0xff);
	sensor_write_reg(0x73  ,0xe5);
	sensor_write_reg(0x74  ,0x5f);
	sensor_write_reg(0x75  ,0xe6);

	sensor_write_reg(0x76  ,0x41);
	sensor_write_reg(0x77  ,0xef);
	sensor_write_reg(0x78  ,0xff);
	sensor_write_reg(0x79  ,0xff);
	sensor_write_reg(0x7a  ,0x5f);
	sensor_write_reg(0x7b  ,0xfa);


	//============================= AGP

	sensor_write_reg(0x7e  ,0x00);
	sensor_write_reg(0x7f  ,0x00);
	sensor_write_reg(0x80  ,0xc8);
	sensor_write_reg(0x81  ,0x06);
	sensor_write_reg(0x82  ,0x08);

	sensor_write_reg(0x83  ,0x23);
	sensor_write_reg(0x84  ,0x38);
	sensor_write_reg(0x85  ,0x4F);
	sensor_write_reg(0x86  ,0x61);
	sensor_write_reg(0x87  ,0x72);
	sensor_write_reg(0x88  ,0x80);
	sensor_write_reg(0x89  ,0x8D);
	sensor_write_reg(0x8a  ,0xA2);
	sensor_write_reg(0x8b  ,0xB2);
	sensor_write_reg(0x8c  ,0xC0);
	sensor_write_reg(0x8d  ,0xCA);
	sensor_write_reg(0x8e  ,0xD3);
	sensor_write_reg(0x8f  ,0xDB);
	sensor_write_reg(0x90  ,0xE2);
	sensor_write_reg(0x91  ,0xED);
	sensor_write_reg(0x92  ,0xF6);
	sensor_write_reg(0x93  ,0xFD);

	//about gamma1 is hex r oct
	sensor_write_reg(0x94  ,0x04);
	sensor_write_reg(0x95  ,0x0E);
	sensor_write_reg(0x96  ,0x1B);
	sensor_write_reg(0x97  ,0x28);
	sensor_write_reg(0x98  ,0x35);
	sensor_write_reg(0x99  ,0x41);
	sensor_write_reg(0x9a  ,0x4E);
	sensor_write_reg(0x9b  ,0x67);
	sensor_write_reg(0x9c  ,0x7E);
	sensor_write_reg(0x9d  ,0x94);
	sensor_write_reg(0x9e  ,0xA7);
	sensor_write_reg(0x9f  ,0xBA);
	sensor_write_reg(0xa0  ,0xC8);
	sensor_write_reg(0xa1  ,0xD4);
	sensor_write_reg(0xa2  ,0xE7);
	sensor_write_reg(0xa3  ,0xF4);
	sensor_write_reg(0xa4  ,0xFA);

	//========= open functions
	sensor_write_reg(0xf0  ,0x00);//set back to page0
	sensor_write_reg(0x40  ,0x18);//1c
	sensor_write_reg(0x41  ,0x24);//0x00);

	//sensor_write_reg(,0x0F  ,0x02);//0xb2);//CISCTL mode1
	sensor_write_reg(0x0F  ,0x22);//Í¼ÏñÐèÒª´¹Ö±·­×ª zsliu
	sensor_write_reg(0x45  ,0x26);
	sensor_write_reg(0x47  ,0x28);


	sensor_write_reg(0x43,0x40);	//zem800
	sensor_write_reg(0x44  ,0xf1); //zyh zem800  0xc0

}
void select_page_gc307(int page)
{
	sensor_write_reg(0xF0,page);
	
}

int sensor_init_GC307(int left, int top, int width, int height)
{
	unsigned char i;
	_initgc307();
	select_page_gc307(0);
	set_window_GC307(left, top, width, height+8);
	set_sensor_clock_gc307(16);

//	set_EXPOS_GC307(0x70);

	sensor_power_on_GC307();
//	set_symmetry_GC307(1);
//	set_sensitivity_GC3071();
	unsigned short data;
	Read24WC02(24, &data, 2);
	printf("data=0x%04x ", data);
	if (data&0x8000)
		m_image_reverse = 1;
	else
		m_image_reverse = 0;
	i2c_set_addr(0x42);
	select_page_gc307(0);
	set_sensitivity_GC3071();
	set_symmetry_GC307(m_image_reverse);
	printf("m_image_reverse=%d\n",m_image_reverse);
	printf("GC307 init finished\n");
	return 0;
}

