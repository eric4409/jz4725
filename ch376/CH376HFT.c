/* 2008.10.18
****************************************
**  Copyright  (C)  W.ch  1999-2009   **
**  Web:  http://www.winchiphead.com  **
****************************************
**  USB Host File Interface for CH376 **
**  TC2.0@PC, KC7.0@MCS51             **
****************************************
*/
/* CH376 �����ļ�ϵͳ�ӿ� */

/* MCS-51��Ƭ��C���Ե�U���ļ���дʾ������ */
/* ��������ʾ�ֽڶ�д,���ڽ�ADCģ���ɼ���������ӵ�U�̻���SD���ļ�MY_ADC.TXT��,
   ����ļ�������ô��������ӵ��ļ�ĩβ,����ļ���������ô�½��ļ���������� */

/* C51   CH376HFT.C */
/* LX51  CH376HFT.OBJ */
/* OHX51 CH376HFT */
#include <stdio.h>
#include <string.h>

#include "HAL.h"

UINT8	buf[64];

usbtest() {
	UINT8	s;
	UINT8	month, date, hour;
	UINT16	adc;

	mDelaymS( 100 );  /* ��ʱ100���� */

	s = mInitCH376Host( );  /* ��ʼ��CH376 */
	sprintf(buf, " usbtest %x\n", (int)s);
	LCDWriteCenterStr(3, buf);
	DelayMS(1000);
//	printf("InitCH376:%d\n", (int)s);
/* ������·��ʼ�� */

//	while ( 1 ) 
	{
		printf( "Wait Udisk/SD\n" );
		LCDWriteCenterStr(3, "wait");
		while ( CH376DiskConnect( ) != USB_INT_SUCCESS ) {  /* ���U���Ƿ�����,�ȴ�U�̲���,����SD��,�����ɵ�Ƭ��ֱ�Ӳ�ѯSD�����Ĳ��״̬���� */
		//	printf("1.wait\n");
			mDelaymS( 100 );
		}
		mDelaymS( 200 );  /* ��ʱ,��ѡ����,�е�USB�洢����Ҫ��ʮ�������ʱ */

/* ���ڼ�⵽USB�豸��,���ȴ�10*50mS */
		for ( s = 0; s < 10; s ++ ) {  /* ��ȴ�ʱ��,10*50mS */
			mDelaymS( 50 );
			printf( "Ready ?\n" );
			LCDWriteCenterStr(3, "Ready?");
			if ( CH376DiskMount( ) == USB_INT_SUCCESS ) break;  /* ��ʼ�����̲����Դ����Ƿ���� */
		}
		s = CH376ReadBlock(buf);  /* �����Ҫ,���Զ�ȡ���ݿ�CH376_CMD_DATA.DiskMountInq,���س��� */
		printf("readblock s:%d\n",s);
		if ( s == sizeof( INQUIRY_DATA ) ) {  /* U�̵ĳ��̺Ͳ�Ʒ��Ϣ */
			buf[ s ] = 0;
			printf( "UdiskInfo: %s\n", ((P_INQUIRY_DATA)buf) -> VendorIdStr );
			LCDWriteCenterStr(3, ((P_INQUIRY_DATA)buf) -> VendorIdStr );
		}
		return;
/* ���MY_ADC.TXT�ļ��Ѿ�������������ݵ�β��,������������½��ļ� */
		printf( "Open\n" );
		s = CH376FileOpen( "/MY_ADC.TXT" );  /* ���ļ�,���ļ��ڸ�Ŀ¼�� */
		if ( s == USB_INT_SUCCESS ) {  /* �ļ����ڲ����Ѿ�����,�ƶ��ļ�ָ�뵽β���Ա�������� */
			printf( "File size = %ld\n", CH376GetFileSize( ) );  /* ��ȡ��ǰ�ļ����� */
		//	printf( "Locate tail\n" );
			s = CH376ByteLocate( 0xFFFFFFFF );  /* �Ƶ��ļ���β�� */
		}
		else if ( s == ERR_MISS_FILE ) {  /* û���ҵ��ļ�,�����½��ļ� */
			printf( "Create\n" );
			s = CH376FileCreate( NULL );  /* �½��ļ�����,����ļ��Ѿ���������ɾ�������½�,�������ṩ�ļ���,�ղ��Ѿ��ṩ��CH376FileOpen */
		}
	//	printf( "Write begin\n" );
		s = sprintf( buf, "��ǰ�ļ�����= %ld �ֽ�\xd\xa", CH376GetFileSize( ) );  /* ע���ַ������Ȳ������buf,����Ӵ󻺳������߷ֶ��д�� */
		s = CH376ByteWrite( buf, s, NULL );  /* ���ֽ�Ϊ��λ���ļ�д������ */
#if 0
		printf( "Write ADC data\n" );
		TR0=1;  /* �ö�ʱ��0�ļ���ֵ����ADC���� */
		for ( hour = 8; hour != 20; hour ++  ) {  /* ��ѭ����ʽ���12������ */
			month = 5;  /* �ٶ���5�� */
			date = TL1 & 0x1F;  /* ��Ϊ���԰���û��ʵʱʱ��оƬ,�����ö�ʱ��1�ļ������������ʾ */
/*			adc = get_adc_data( ); */
			adc = ( (UINT16)TH0 << 8 ) | TL0;  /* ��Ϊ���԰���û��ADC,�����ö�ʱ��0�ļ�������ADC������ʾ */
			s = sprintf( buf, "%02d.%02d.%02d ADC=%u\xd\xa", (UINT16)month, date, (UINT16)hour, adc );  /* �����������ݸ�ʽΪһ���ַ��� */
			s = CH376ByteWrite( buf, s, NULL );  /* ���ֽ�Ϊ��λ���ļ�д������ */
/* ��ЩU�̿��ܻ�Ҫ����д���ݺ�ȴ�һ����ܼ�������,����,�����ĳЩU���з������ݶ�ʧ����,������ÿ��д�����ݺ�������ʱ�ټ��� */
			printf( "Current offset ( file point ) is %ld\n", CH376ReadVar32( VAR_CURRENT_OFFSET ) );  /* ��ȡ��ǰ�ļ�ָ�� */
		}
//		CH376ByteWrite( buf, 0, NULL );  /* ���ֽ�Ϊ��λ���ļ�д������,��Ϊ��0�ֽ�д��,����ֻ���ڸ����ļ��ĳ���,���׶���д�����ݺ�,���������ְ취�����ļ����� */
#endif
	//	printf( "Write end\n" );
		strcpy( buf, "�����ADC���ݵ��˽���\xd\xa" );
		s = CH376ByteWrite( buf, strlen( buf ), NULL );  /* ���ֽ�Ϊ��λ���ļ�д������ */
/* ���ʵ�ʲ�Ʒ����ʵʱʱ��,���Ը�����Ҫ���ļ������ں�ʱ���޸�Ϊʵ��ֵ,�ο�EXAM10��CH376DirInfoRead/CH376DirInfoSaveʵ�� */
	//	printf( "Close\n" );
		s = CH376FileClose( TRUE );  /* �ر��ļ�,�Զ������ļ�����,���ֽ�Ϊ��λд�ļ�,�����ó����ر��ļ��Ա��Զ������ļ����� */

		printf( "Take out\n" );
		while ( CH376DiskConnect( ) == USB_INT_SUCCESS ) {  /* ���U���Ƿ�����,�ȴ�U�̰γ� */
			mDelaymS( 100 );
		}
		mDelaymS( 200 );
	}
}
