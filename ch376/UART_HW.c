#include	"HAL.h"

void    xWriteCH376Cmd( UINT8 mCmd );
void    xWriteCH376Data( UINT8 mData );

//#define CH376_INT_WIRE			INT0	/* �ٶ�CH376��INT#����,���δ������ôҲ����ͨ����ѯ�����ж�״̬��ʵ�� */
//#define	UART_WORK_BAUDRATE	57600

void	CH376_PORT_INIT( void )  /* ����ʹ���첽���ڶ�дʱ��,���Խ��г�ʼ�� */
{
	serial_init (2000000, 8, 0, 0);
//	serial_init (460800, 8, 0, 0);
}

#ifdef	UART_WORK_BAUDRATE
void	SET_WORK_BAUDRATE( void )  /* ����Ƭ���л�����ʽͨѶ������ */
{
	xWriteCH376Cmd(CMD21_SET_BAUDRATE);
	xWriteCH376Data(0x03);
	xWriteCH376Data(0x98);
	serial_init (UART_WORK_BAUDRATE, 8, 0, 0);
}
#endif

void	xWriteCH376Cmd( UINT8 mCmd )  /* ��CH376д���� */
{
	serial_put(SER_SYNC_CODE1);
	serial_put(SER_SYNC_CODE2);
	serial_put(mCmd);
}

void	xWriteCH376Data( UINT8 mData )  /* ��CH376д���� */
{
	serial_put(mData);
}

UINT8	xReadCH376Data( void )  /* ��CH376������ */
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

/* ��ѯCH376�ж�(INT#�͵�ƽ) */
UINT8	Query376Interrupt( void )
{
#ifdef	CH376_INT_WIRE
	return( CH376_INT_WIRE ? FALSE : TRUE );  /* ���������CH376���ж�������ֱ�Ӳ�ѯ�ж����� */
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

/* ��ʼ��CH376 */
UINT8	mInitCH376Host( void )  
{
	UINT8	res;
	static UINT8 openflag = 0;
	if (openflag)
		return USB_INT_SUCCESS;

	mdelay(50);
	CH376_PORT_INIT( );  /* �ӿ�Ӳ����ʼ�� */

	xWriteCH376Cmd( CMD11_CHECK_EXIST );  /* ���Ե�Ƭ����CH376֮���ͨѶ�ӿ� */
	xWriteCH376Data( 0x65 );
	mdelay(1);
	res = xReadCH376Data( );
//	printf("376Host2 res:%x\n",res);
	if ( res != 0x9A ) 
		return( ERR_USB_UNKNOWN );  /* ͨѶ�ӿڲ�����,����ԭ����:�ӿ������쳣,�����豸Ӱ��(Ƭѡ��Ψһ),���ڲ�����,һֱ�ڸ�λ,���񲻹��� */
#ifdef	UART_WORK_BAUDRATE  /* ���ô���ͨѶ������ */
/*	xWriteCH376Cmd( CMD21_SET_BAUDRATE );
#if		UART_WORK_BAUDRATE >= 6000000/256
	xWriteCH376Data( 0x03 );
	xWriteCH376Data( 256 - 6000000/UART_WORK_BAUDRATE );
#else
	xWriteCH376Data( 0x02 );
	xWriteCH376Data( 256 - 750000/UART_WORK_BAUDRATE );
#endif  */
	SET_WORK_BAUDRATE( );  /* ����Ƭ���л�����ʽͨѶ������ */
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
		return( ERR_USB_UNKNOWN );  /* ͨѶ�������л�ʧ��,����ͨ��Ӳ����λCH376������ */
#endif
	xWriteCH376Cmd( CMD11_SET_USB_MODE );  /* �豸USB����ģʽ */
	xWriteCH376Data( 0x06 );
	mdelay(1);
	res = xReadCH376Data( );
//	printf("376Host4  res:%x\n",res);
	openflag = 1;
	if ( res == CMD_RET_SUCCESS ) 
		return( USB_INT_SUCCESS );
	else 
		return( ERR_USB_UNKNOWN );  /* ����ģʽ���� */

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
