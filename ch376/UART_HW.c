#include	"HAL.h"

void    xWriteCH376Cmd( UINT8 mCmd );
void    xWriteCH376Data( UINT8 mData );

//#define CH376_INT_WIRE			INT0	/* 假定CH376的INT#引脚,如果未连接那么也可以通过查询串口中断状态码实现 */
//#define	UART_WORK_BAUDRATE	57600

void	CH376_PORT_INIT( void )  /* 由于使用异步串口读写时序,所以进行初始化 */
{
	serial_init (2000000, 8, 0, 0);
//	serial_init (460800, 8, 0, 0);
}

#ifdef	UART_WORK_BAUDRATE
void	SET_WORK_BAUDRATE( void )  /* 将单片机切换到正式通讯波特率 */
{
	xWriteCH376Cmd(CMD21_SET_BAUDRATE);
	xWriteCH376Data(0x03);
	xWriteCH376Data(0x98);
	serial_init (UART_WORK_BAUDRATE, 8, 0, 0);
}
#endif

void	xWriteCH376Cmd( UINT8 mCmd )  /* 向CH376写命令 */
{
	serial_put(SER_SYNC_CODE1);
	serial_put(SER_SYNC_CODE2);
	serial_put(mCmd);
}

void	xWriteCH376Data( UINT8 mData )  /* 向CH376写数据 */
{
	serial_put(mData);
}

UINT8	xReadCH376Data( void )  /* 从CH376读数据 */
{
	int timeout=800000;
	while (!serial_tstc()&&timeout--);
	if(timeout<=0)
	{
	//	printf("Read rs232 failed\n");
		return 0;
	}
	return serial_getc();
}

/* 查询CH376中断(INT#低电平) */
UINT8	Query376Interrupt( void )
{
#ifdef	CH376_INT_WIRE
	return( CH376_INT_WIRE ? FALSE : TRUE );  /* 如果连接了CH376的中断引脚则直接查询中断引脚 */
#else
	UINT8 s=0;
	if(serial_tstc())
	{
		s=serial_getc();
	//	printf("Interrupt:%02x\n",s);
	}
	return s; 
//	return serial_tstc();
#endif
}

/* 初始化CH376 */
UINT8	mInitCH376Host( void )  
{
	UINT8	res;
	static UINT8 openflag = 0;
	if (openflag)
		return USB_INT_SUCCESS;

	mdelay(50);
	CH376_PORT_INIT( );  /* 接口硬件初始化 */

	xWriteCH376Cmd( CMD11_CHECK_EXIST );  /* 测试单片机与CH376之间的通讯接口 */
	xWriteCH376Data( 0x65 );
	mdelay(1);
	res = xReadCH376Data( );
//	printf("376Host2 res:%x\n",res);
	if ( res != 0x9A ) 
		return( ERR_USB_UNKNOWN );  /* 通讯接口不正常,可能原因有:接口连接异常,其它设备影响(片选不唯一),串口波特率,一直在复位,晶振不工作 */
#ifdef	UART_WORK_BAUDRATE  /* 设置串口通讯波特率 */
/*	xWriteCH376Cmd( CMD21_SET_BAUDRATE );
#if		UART_WORK_BAUDRATE >= 6000000/256
	xWriteCH376Data( 0x03 );
	xWriteCH376Data( 256 - 6000000/UART_WORK_BAUDRATE );
#else
	xWriteCH376Data( 0x02 );
	xWriteCH376Data( 256 - 750000/UART_WORK_BAUDRATE );
#endif  */
	SET_WORK_BAUDRATE( );  /* 将单片机切换到正式通讯波特率 */
//	int i;
//	for(i=0;i<100;i++)	
	{
		mdelay(1);
		res = xReadCH376Data( );
	//	if ( res == CMD_RET_SUCCESS )
	//		break;
	}
//	printf("376Host3  res:%x\n",res);
	if ( res != CMD_RET_SUCCESS ) 
		return( ERR_USB_UNKNOWN );  /* 通讯波特率切换失败,建议通过硬件复位CH376后重试 */
#endif
	xWriteCH376Cmd( CMD11_SET_USB_MODE );  /* 设备USB工作模式 */
	xWriteCH376Data( 0x06 );
	mdelay(1);
	res = xReadCH376Data( );
//	printf("376Host4  res:%x\n",res);
	openflag = 1;
	if ( res == CMD_RET_SUCCESS ) 
		return( USB_INT_SUCCESS );
	else 
		return( ERR_USB_UNKNOWN );  /* 设置模式错误 */

	return 0;
}

int InitCH376(void)
{
	UINT8 s=0;
	UINT8 buf[64];

	if(mInitCH376Host()!=USB_INT_SUCCESS)
	{
		printf("mInitCH376Host error\n");
		return -1;
	}

	printf( "Wait Udisk/SD\n" );
        for ( s = 0; s < 10; s ++ ) {
                if ( CH376DiskConnect( ) == USB_INT_SUCCESS ) 
			break;
                mDelaymS( 100 );
        }
	
	if(s>=10)
	{
		printf("Udisk connect error!\n");
		return -1;
	}
	else
		printf("uDisk connect OK!\n");

        mDelaymS( 200 );

        for ( s = 0; s < 10; s ++ ) {
                printf( "Ready ?\n" );
                if ( CH376DiskMount( ) == USB_INT_SUCCESS ) 
			break;
                mDelaymS(50);
        }

	if(s>=10)
	{
		printf("Udisk mount error!\n");
		return -1;
	}
	else
		printf("Udisk mount OK!\n");
        s = CH376ReadBlock(buf);
//      printf("ReadBlock: %02x\n",s);
        if ( s == sizeof( INQUIRY_DATA ) )
        {
                buf[ s ] = 0;
                printf( "UdiskInfo: %s\n", ((P_INQUIRY_DATA)buf) -> VendorIdStr );
        }
	
	return 0;
}

#if 0
void ch376test(void)
{
	unsigned char buff[512];
	unsigned char rbuf[512];
	mInitCH376Host();
/*
	if(getUSBConnect()!=USB_INT_CONNECT)
	{
		printf("USB DISK not connect\n");
		return;
	}
	UINT8 r=0;
	r=CH376DiskConnect();
	printf("usb connect:%02x\n",r);
	r= CH376DiskMount();
	printf("usb mount:%02x\n",r);
*/
	UINT8 s,r=0;
	UINT8 buf[64];
	printf( "Wait Udisk/SD\n" );
	while ( CH376DiskConnect( ) != USB_INT_SUCCESS ) {
		mDelaymS( 100 );
	}
	mDelaymS( 200 );

	for ( s = 0; s < 10; s ++ ) {
		mDelaymS( 50 );
		printf( "Ready ?\n" );
		if ( CH376DiskMount( ) == USB_INT_SUCCESS ) break;
	}
	s = CH376ReadBlock(buf);
//	printf("ReadBlock: %02x\n",s);
	if ( s == sizeof( INQUIRY_DATA ) ) 
	{
		buf[ s ] = 0;
		printf( "UdiskInfo: %s\n", ((P_INQUIRY_DATA)buf) -> VendorIdStr );
	}


	UINT32   DiskCap=0;
	UINT32   DiskQ=0;
	CH376DiskCapacity(&DiskCap);
	CH376DiskQuery(&DiskQ);
	printf("Whole sector :%ld,free:%ldx\n",DiskCap,DiskQ);
/*
	s=CH376FileOpen("/test.txt");
	if ( s == USB_INT_SUCCESS )
	{
	printf("open file:%02x\n",r);

	}
	else
	{
		r=CH376FileCreate(NULL);
		printf("create file:%02x\n",r);
	}
	UINT8 rcount=0;
	r=CH376ByteWrite("123456789",9,NULL);
	printf("write file:%02x,c:502x\n",r,rcount);
	r = CH376FileClose( TRUE );

	return; */
	
	int i=0;
	for(i=0; i<512; i++)
		buff[i]=i;
	printf("write sector\n");
	printf("%02x %02x %02x %02x %02x",buff[0],buff[1],buff[2],buff[3],buff[4]);
	if(CH376DiskWriteSec(buff, 500, 1)!=USB_INT_SUCCESS)
	{
		printf("write sector failed\n");
	}
	memset(rbuf, 0, 512);
	if(CH376DiskReadSec(rbuf, 500, 1)!=USB_INT_SUCCESS)
	{
		printf("read sector failed\n");
		return;
	}

	printf("read sector\n");
	printf("%02x %02x %02x %02x %02x ",rbuf[0],rbuf[1],rbuf[2],rbuf[3],rbuf[4]);

	/*
	   UINT8 buf[64];
	   UINT8 s;
	   printf( "Wait Udisk/SD\n" );
	   while ( CH376DiskConnect( ) != USB_INT_SUCCESS ) {
	   mDelaymS( 100 );
	   }
	   mDelaymS( 200 );

	   for ( s = 0; s < 10; s ++ ) {
	   mDelaymS( 50 );
	   printf( "Ready ?\n" );
	   if ( CH376DiskMount( ) == USB_INT_SUCCESS ) break;
	   }
	   s = CH376ReadBlock( buf );
	   if ( s == sizeof( INQUIRY_DATA ) ) {
	   buf[ s ] = 0;
	   printf( "UdiskInfo: %s\n", ((P_INQUIRY_DATA)buf) -> VendorIdStr );
	   }
	 */
}
#endif
int USB_DetectStatus(void)
{
	UINT8 s=0;
	UINT8 buf[64];

	if(mInitCH376Host()!=USB_INT_SUCCESS)
	{
		printf("mInitCH376Host error\n");
		return -1;
	}

	printf( "Wait Udisk/SD\n" );
	for ( s = 0; s < 3; s ++ ) {
		if ( CH376DiskConnect( ) == USB_INT_SUCCESS )
			break;
		mDelaymS( 100 );
	}

	if(s>=3)
	{
		printf("Udisk connect error!\n");
		return -1;
	}

	return 0;
}
