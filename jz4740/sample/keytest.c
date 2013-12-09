#include <bsp.h>
#include <jz4740.h>
//#include <key.h>

/*
extern PFN_KEYHANDLE UpKey;
extern PFN_KEYHANDLE DownKey;

void UpKeyTest(int key)
{
	printf("UpKeyTest 0x%x\r\n",key);
}
void DownKeyTest(int key)
{
	printf("DownKeyTest 0x%x\r\n",key);
}


void KeyTest()
{
    int i;  
	UpKey = (PFN_KEYHANDLE)UpKeyTest;
	DownKey =(PFN_KEYHANDLE)DownKeyTest;
	KeyInit();
	
}
*/

void KeyTest(void)
{
	int key;

	KeyInit();
#if 1
	while(1)
	{
	//	printf("getting key ... \n");
		key=GetKey();
		if(key>=0)
			printf("Key value=%x\n",key);
	//	OSTimeDlyHMSM(0, 0, 1, 0);     /* Wait one second */
	}
#endif
}
