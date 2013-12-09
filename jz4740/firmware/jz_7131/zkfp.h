#ifndef _ZKFINGER_H_
#define _ZKFINGER_H_

#ifdef UNDER_CE
#include <windows.h>
#endif

//Force C (not C++) names for API functions
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ZK_FP_DLL
	#ifdef ZKFINGER_EXPORTS
		#define BIOKEY_API __declspec(dllexport)
	#else
		#define BIOKEY_API __declspec(dllimport)
	#endif
#else
	#define BIOKEY_API
#endif
#ifndef BYTE
#define BYTE unsigned char
#endif
#ifndef WORD
#define WORD unsigned short
#endif

/*
#ifndef DWORD
#define DWORD unsigned int
#endif

#ifndef BOOL
#define BOOL unsigned int
#endif
*/

#ifndef HANDLE
typedef void * HANDLE;
#endif

typedef int FPHANDLE;

#define SPEED_LOW 0
#define SPEED_HIGH 1
#define THRESHOLD_HIGH 80
#define THRESHOLD_MIDDLE 72
#define THRESHOLD_LOW 47

#define EXTRACT_FOR_ENROLLMENT		0
#define EXTRACT_FOR_IDENTIFICATION	1
#define EXTRACT_FOR_VERIFICATION	2

#define DEF_MTHRESHOLD	55
#define DEF_MAXNOISETHR	100
#define DEF_MINMINUTIAE	12
#define DEF_MAXTEMPLEN	508

#define CORRECT_ROTATION	2
#define CORRECT_REVERSE		1
#define CORRECT_NONE		0


#define ZKFP_PARAM_CODE_USERCOUNT	5001
#define ZKFP_PARAM_CODE_USERIDS		5002
#define ZKFP_PARAM_CODE_FINGERCOUNT	5003
#define ZKFP_PARAM_CODE_USERFINGERCOUNT	5004	//��ȡָ��������û�ָ������ֻȡ�ú���ָ�ƣ���ZKFP_PARAM_CODE_USERFINGERCOUNT_EX������
#define ZKFP_PARAM_CODE_REG_FP_FORMAT		5005	//�Ǽ�ģ���ʽ��ģʽ1ʹ����õ�1öָ�ƣ�ģʽ2ʹ����õ�2öָ��
	#define ZKFP_REG_FP_TEMPLATE_STD	1
	#define ZKFP_REG_FP_TEMPLATE_LONG	2

#define ZKFP_PARAM_CODE_DBNAME	5006	//����ʹ�õ����ݿ�


#define ZKFP_PARAM_CODE_DBMM	5007	//ָ��ģ�����ݿ����ʽ
	#define ZKFP_DBMM_INTERNAL	1		//�ڲ�����
	#define ZKFP_DBMM_EXTERNAL	2		//�ⲿ����

#define ZKFP_PARAM_CODE_USERFINGERCOUNT_EX	5008	//��ȡָ���û�������ָ��,����ȡ����PIN | (fingerindex<<16)������ָ�ơ���ZKFP_PARAM_CODE_USERFINGERCOUNT������

#define ZKFP_PARAM_CODE_MAXNUMBERFINGER		5009	//֧�ֵ����ָ������

#define ZKFPV10_MAX_TEMPLEN	32*1024		//maxinum size fo multi templates

#define ZKFPV10_MAX_TEMP_SIZE	1664	//maximum size of single template

//�������л�������ʼ��ָ��ʶ��ϵͳ������Ƕ��ʽϵͳ��
//�����˽��й�ѧ����/�ֱ���У����ʵʱ��鰴ѹ��ָ�Ĳ�����
// Param - Ϊһָ����ֲ�����ָ�룬��Щ�����ڵ���ǰ�뱻��ʼ��
// ImageBuffer - Ϊָ��ɼ�ָ�Ƶ���������ָ�룬�ô������ԭʼ��ָ��ͼ��
BIOKEY_API HANDLE BIOKEY_INIT(int License, WORD *isize, BYTE *Params, BYTE *Buffer, int ImageFlag);

//����һ���ָ��ͼ�������ʼ��ָ��ʶ��ϵͳ
// width - ָ��ͼ��Ŀ��
// height - ָ��ͼ��ĸ߶�
// ImageBuffer - ָ��ͼ������������������Ӧ���ڵ���ǰ��ʼ��
BIOKEY_API HANDLE BIOKEY_INIT_SIMPLE(int License, int width, int height, BYTE *Buffer);

//�ر�ָ��ʶ��ϵͳ
BIOKEY_API int BIOKEY_CLOSE(HANDLE Handle);

//��ȡ��ǰ��������ָ��ͼ�������ģ�壬��������Template��ָ�Ļ�������
//Template�������Ĵ�С��Ԥ��Ϊ2K
//����ȡ�ɹ�������ָ��ģ���ʵ�ʴ�С�����򷵻�ֵ<=0
BIOKEY_API int BIOKEY_EXTRACT(HANDLE Handle, BYTE* PixelsBuffer, BYTE *Template, int PurposeMode);
BIOKEY_API int BIOKEY_EXTRACT_SIMPLE(HANDLE Handle, BYTE** PixelsBuffer, BYTE *Template, int PurposeMode);
//ȡ���ָ��ͼ�������
BIOKEY_API int BIOKEY_GETLASTQUALITY(HANDLE Handle);

//��3��ָ������ģ������һ���Ǽ�ģ��
BIOKEY_API int BIOKEY_GENTEMPLATE(HANDLE Handle, BYTE *Templates[], int TmpCount, BYTE *GTemplate);

//�ȶ���������ģ�壬�������ǵ����Ƴ̶ȣ�0��ʾ��ȫ��ͬ��100��ʾ��ȫ��ͬ
BIOKEY_API int BIOKEY_VERIFY(HANDLE Handle,  BYTE *Template1, BYTE *Template2);

BIOKEY_API int BIOKEY_MATCHINGPARAM(HANDLE Handle,  int speed, int threshold);
////////////////////////////////////////////
//
//  Identification ��غ���
//
//������ݿ���ָ��ģ��
BIOKEY_API int BIOKEY_DB_CLEAR(HANDLE Handle);

//��ָ��ģ��������ݿ�
// TID - ��ָ��ģ��ı�ʶ�š�ÿ��ָ��ģ����һ��Ψһ�ı�ʶ��
// Template - ָ��ģ��
//�ɹ�ʱ����1, ʧ��ʱ����0
BIOKEY_API int BIOKEY_DB_ADD(HANDLE Handle,  int TID, int TempLength, BYTE *Template);

//�����ݿ����Ƴ�һ��ָ��ģ��
// TID - ��Ҫ�Ƴ���ָ��ģ��ı�ʶ�š�
// Template - ָ��ģ��
//�ɹ�ʱ����1, ʧ��ʱ����0
BIOKEY_API int BIOKEY_DB_DEL(HANDLE Handle,  int TID);

//Ӧ��һ�����˺����������ݿ⣬ֻ�иù��˺������ط���ʱ����TID��Ӧ��ָ��ģ��Ż����Identify
typedef int (*FilteFun)(int TID);
BIOKEY_API int BIOKEY_DB_FILTERID(HANDLE Handle,  FilteFun Filter);
BIOKEY_API int BIOKEY_DB_FILTERID_ALL(HANDLE Handle);
BIOKEY_API int BIOKEY_DB_FILTERID_NONE(HANDLE Handle);

//ʶ��ָ��
// Template - Ҫʶ���ָ��ģ��
// TID - ʶ��Ľ������֮��ƥ������ݿ��е�ָ������Ӧ�ı�ʶ��
// Score - ��Ϊ�����������ʾ����ȷ�ϵ�ƥ�����ƶȣ�
//         ��Ϊ�������ʾTemplate��TID��Ӧָ�Ƶ�ʵ�����ƶ�
//         ΪNULLʱ���Ը���
// ʶ��ɹ�����1�����򷵻�0
BIOKEY_API int BIOKEY_IDENTIFYTEMP(HANDLE Handle, BYTE *Template, int *TID, int *Score);

//ʶ��ǰ�ɼ�ָ��ͼ��
// TID - ʶ��Ľ������֮��ƥ������ݿ��е�ָ������Ӧ�ı�ʶ��
// Score - ��Ϊ�����������ʾ����ȷ�ϵ�ƥ�����ƶȣ�
//         ��Ϊ�������ʾTemplate��TID��Ӧָ�Ƶ�ʵ�����ƶ�
//         ΪNULLʱ���Ը���
// ʶ��ɹ�����1�����򷵻�0
BIOKEY_API int BIOKEY_IDENTIFY(HANDLE Handle, BYTE *ImageBuffer, int *TID, int *Score);
BIOKEY_API int BIOKEY_IDENTIFY_SIMPLE(HANDLE Handle, BYTE **ImageBuffer, int *TID, int *Score);

////////////////////////////////////////////
//
//  Ƕ��ʽϵͳ���
//
//ʹ��Ĭ�ϲ�����ʼ������ָ��⹦��
BIOKEY_API int BIOKEY_TEST_INIT(HANDLE Handle);

//���Ե�ǰ�������Ƿ���ָ��ͼ��
//��ͼ�񷵻�1����ͼ�񷵻�0
BIOKEY_API int BIOKEY_TESTFINGER(HANDLE Handle, BYTE *ImageBuffer);

//������ǰ��������ָ�ƣ����ػ���У�����ָ��ͼ��
BIOKEY_API void BIOKEY_GETFINGERLINEAR(HANDLE Handle, BYTE *ImageBuffer, BYTE *Finger);

//ȡָ�ƴ���ϵͳ����
BIOKEY_API int BIOKEY_GETPARAM(HANDLE Handle, int *DPI, int *width, int *height);

BIOKEY_API int BIOKEY_SETNOISETHRESHOLD(HANDLE Handle, int NoiseValue, int MinMinutiae, int MaxTempLen, int ExtractScaleDPI);
BIOKEY_API int BIOKEY_TEMPLATELEN(BYTE *Template);
BIOKEY_API int BIOKEY_SETTEMPLATELEN(BYTE *Template, int NewLength);
BIOKEY_API int IdentifyInThread(HANDLE Handle, BYTE *Template, int *TID, int *Score,int ThreadNumber);

BIOKEY_API int BIOKEY_GETVERSION(int *Major,int *Minor);
BIOKEY_API int BIOKEY_DB_GET_TEMPLATE(int UID,int FingerIndex,BYTE *Template, int *Length);
BIOKEY_API int BIOKEY_SET_PARAMETER(HANDLE Handle, int ParameterCode, int ParameterValue);
BIOKEY_API int BIOKEY_GET_PARAMETER(HANDLE Handle, int ParameterCode, int *ParameterValue);
BIOKEY_API int BIOKEY_DB_ADDEX(HANDLE Handle,  int TID, int TempLength, BYTE *Template);
BIOKEY_API int BIOKEY_GET_CUSTOMDATA(HANDLE Handle, int UID, char *CustomData,int *Length);
BIOKEY_API int BIOKEY_SET_CUSTOMDATA(HANDLE Handle, int UID, char *CustomData,int Length);
BIOKEY_API int BIOKEY_MERGE_TEMPLATE(BYTE *Templates[],int FingerCount,BYTE *TemplateDest);
BIOKEY_API int BIOKEY_SPLIT_TEMPLATE(BYTE *Template,BYTE *Templates[], int *FingerCount,int *FingerSizes);
#ifdef __cplusplus
}
#endif

#endif

