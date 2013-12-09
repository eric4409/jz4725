/*
 * Capture a frame of image
 */
#include "camera.h"

#if 0
static void convert_frame(void *buf, int size)
{
	unsigned char *ptr = (unsigned char *)buf;
	unsigned short *ptr2 = (unsigned short *)buf;
	int i;

	for (i = 0; i < size/2; i++) {
		*ptr = (*ptr2) & 0xff;
		ptr++;
		ptr2++;
	}
}
#endif

void capture(void)
{
	int framesize;
	char filename[256];
	unsigned char framebuf[IMG_WIDTH * IMG_HEIGHT+1024];
	int i,j;
	int tmp;
	int size;
	int captured=0;
	unsigned int sum,sum1;

	framesize = IMG_WIDTH * IMG_HEIGHT;

	OSSchedLock();

	camera_open(); /* open devices */
	
	printf("begin camera reading\n");
	sum=OSTimeGet();

	JZ_StopTicker();
	for (i = 0; i < 50; i++)
	{
		memset(framebuf,0,IMG_WIDTH*IMG_HEIGHT+1024);
		size=camera_read(framebuf);
		if(size>0)
		{
	//		printf("the size of frame =%d\n",size);
			captured++;
			if(captured>6) break;
		}
	}
	JZ_StartTicker(OS_TICKS_PER_SEC);
	
	sum1=OSTimeGet();
	printf("time=%d,cnt=%d\n",sum1-sum,captured);
	
	if(captured<6)
	{
		printf("capture failed\n");
		return;
	}
	sum=0;

	printf("****** begin print picture ********\n");
	
	serial_put(0xab);
	serial_put(0xcd);
	serial_put(0xef);

	for(i=0;i<IMG_HEIGHT;i++)
	{
		for(j=0;j<IMG_WIDTH;j++)
		{
			serial_put(framebuf[j+i*IMG_WIDTH]);
			sum+=framebuf[j+i*IMG_WIDTH];
		}
	}

	serial_put(sum&0xff);
	serial_put((sum>>8)&0xff);
	serial_put((sum>>16)&0xff);
	serial_put((sum>>24)&0xff);

	printf("frame size =%d\n",framesize);
	printf("********finished print picture ********\n");

	camera_close(); /* close devices */
	OSSchedUnlock();

	return;
}
