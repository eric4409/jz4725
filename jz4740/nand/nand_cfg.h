//
// Copyright (c) Ingenic Semiconductor Co., Ltd. 2007.
//

#ifndef __NAND_CFG_H__
#define __NAND_CFG_H__
static const NAND_INFO	g_NandInfo[] = 
{
//	{NandID,Tals	Talh	Trp	Twp	Trhw	Twhr	dwPageSize,	dwPageAddressCycle,dwMinValidBlocks,dwMaxValidBlocks}
	{0xECD3,	15,		5,		15,	15,	100,	60,		2048,		256 * 1024, 	3,	3996,	4096	}, //SAMSUNG_K9G8G08U0M 
	{0xECF1,	15,		5,		15,	15,	100,	60,		2048,		128 * 1024, 	3,	1004,	1024	},
	{0xECDC,	12,		5,		12,	12,	100,	60,		2048,		128 * 1024, 	3,	4016,	4096	},//SAMSUNG_K9F4G08U0A
	{0xADF1,	5,		15,		25,	40,	60,	60,		2048,		128 * 1024,	3,	1004,	1024	},//HYNIX_HY27UF081G2M
	{0xECDA,	12,		5,		12,	12,	100,	60,		2048,		128 * 1024,	3,	2008,	2048   },//SAMSUNG K9F2G08U0A

};


#endif // __NAND_CFG_H__
