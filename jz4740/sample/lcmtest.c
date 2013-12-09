

#define  LCD_DISPLAY_ON          0x01
#define  LCD_DISPLAY_OFF         0x02
#define  LCD_RESET               0x03
#define  LCD_DISPLAY_LINE_START  0x04
#define  LCD_WRITE_DATA          0x05
#define  LCD_READ_DATA           0x06
#define  LCD_SET_PAGE            0x07
#define  LCD_SET_ADDRESS         0x08
#define  LCD_READ_STATE          0x09

unsigned char dian[] = {
   0x00, 0xf8, 0x48, 0x48, 0x48, 0x48, 0xff, 0x48, 0x48, 0x48, 0x48, 0xfc, 0x08, 0x00, 0x00, 0x00,
   0x00, 0x07, 0x02, 0x02, 0x02, 0x02, 0x3f, 0x42, 0x42, 0x42, 0x42, 0x47, 0x40, 0x70, 0x00, 0x00
};

void LCMTest(void)
{
	unsigned int cs;
    	int i,j,k;

	InitializeLCD();

	cs = 1;
   	lcd_ioctl(LCD_RESET, &cs);

   	printf("LCD_DISPLAY_ON...\n");

   	cs = 1;
   	lcd_ioctl(LCD_DISPLAY_ON, &cs);

   	for(k=0; k<1; k++) {
        /* write line */
        	for(i=0; i<8; i++) {
                	cs = 0;
                	cs += i;
                	if (lcd_ioctl(LCD_SET_PAGE, &cs)) {
                        	printf("error:ioctl(LCD_SET_PAGE)\n");
                        	return;
                	}

                	cs = 0;
                	if (lcd_ioctl(LCD_SET_ADDRESS, &cs)) {
                        	printf("error:ioctl(LCD_SET_ADDRESS)\n");
                        	return;
                	}
//                	cs = 0xff;
//                	lcd_ioctl(LCD_WRITE_DATA, cs);

                	for (j=0; j<128; j++) {
				if(j%2)
	                        	cs = 0xaa;
				else	cs = 0x55;

                        	lcd_ioctl(LCD_WRITE_DATA, &cs);
                	}
//                	cs = 0xff;
//                	lcd_ioctl(LCD_WRITE_DATA, cs);
        	}
	}
	printf("finished display\n");
}
