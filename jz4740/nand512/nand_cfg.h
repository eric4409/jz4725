/********************** BEGIN LICENSE BLOCK ************************************
 *
 * JZ4740  mobile_tv  Project  V1.0.0
 * INGENIC CONFIDENTIAL--NOT FOR DISTRIBUTION IN SOURCE CODE FORM
 * Copyright (c) Ingenic Semiconductor Co. Ltd 2005. All rights reserved.
 * 
 * This file, and the files included with this file, is distributed and made 
 * available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER 
 * EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS ALL SUCH WARRANTIES, 
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS 
 * FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. 
 * 
 * http://www.ingenic.cn 
 *
 ********************** END LICENSE BLOCK **************************************
 *
 *  Author:  <dsqiu@ingenic.cn>  <jgao@ingenic.cn> <xyzhang@ingenic.cn> 
 *
 *  Create:   2008-06-26, by dsqiu
 *            
 *  Maintain: 2008-06-26, by jgao
 *            
 *  Maintain: 2008-08-27, by xyzhang
 *
 *******************************************************************************
 */


#ifndef __NAND_CFG_H__
#define __NAND_CFG_H__
static const NAND_INFO	g_NandInfo[] = 
{
//	{NandID,    NandExtID,	PlaneNum	Tals	Talh	Trp	Twp	Trhw	Twhr	dwPageSize,	dwBlockSize, dwPageAddressCycle,dwMinValidBlocks,dwMaxValidBlocks}
	{0xECD7,			0,		1,		12,		5,		12,	12,	100, 	60,		4096,	512 * 1024,	    	3,				7992,			8192		},//SAMSUNG_K9LBG08U0M
	{0xEC76,			0,		1,		21,		5,		21,	21,	100, 	60,		512,	16 * 1024,			3,				4016,			4096		},//SAMSUNG_K9F1208U0C
	{0xEC75,			0,		1,		0,		10,		25,	25,	100, 	60,		512,	16 * 1024,			3,				2013,			2048		},//SAMSUNG_K9F5608U0D
	{0xAD76,                        0,              1,              15,             5,              15,     15,     100,    60,             512,    16 * 1024,                      4,                              4016,                   4096            },//HYNIX_H27U518S2C
	{0xAD75,                        0,              1,              0,              10,             25,     25,     100,    50,             512,    16 * 1024,                      3,                              2008,                   2048            },//HYNIX_HY27US08561A
};
//Note: NandID = Manufacture code + device code; and if your chip is not mutil cs,you must set NandExtId = 0,whether it has or no.

#endif // __NAND_CFG_H__
