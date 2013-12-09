/**************************************************************************
 * FreeDOS 32 FAT Driver                                                  *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2001-2005, Salvatore Isaja                               *
 *                                                                        *
 * This is "fat.h" - Driver's constants, structures and prototypes        *
 *                                                                        *
 *                                                                        *
 * This file is part of the FreeDOS 32 FAT Driver.                        *
 *                                                                        *
 * The FreeDOS 32 FAT Driver is free software; you can redistribute it    *
 * and/or modify it under the terms of the GNU General Public License     *
 * as published by the Free Software Foundation; either version 2 of the  *
 * License, or (at your option) any later version.                        *
 *                                                                        *
 * The FreeDOS 32 FAT Driver is distributed in the hope that it will be   *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with the FreeDOS 32 FAT Driver; see the file COPYING;            *
 * if not, write to the Free Software Foundation, Inc.,                   *
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA                *
 **************************************************************************/

#ifndef __FD32_FAT_H
#define __FD32_FAT_H


#include <nls.h>
#include <lfn.h>
/* Use the following defines to add features to the FAT driver */
/* TODO: The FAT driver currently doesn't work with buffers disabled! */
/* TODO: The name cache is totally broken! */
/* TODO: Sharing support can't be currently enabled */
#define FATBUFFERS   /* Uncomment this to use the buffered I/O        */
#define FATLFN       /* Define this to use Long File Names            */
#define FATWRITE     /* Define this to enable writing facilities      */


/* FAT 32-byte Directory Entry structure */
typedef struct
{
  BYTE  Name[11];
  BYTE  Attr;
  BYTE  NTRes;
  BYTE  CrtTimeTenth;
  WORD  CrtTime;
  WORD  CrtDate;
  WORD  LstAccDate;
  WORD  FstClusHI;
  WORD  WrtTime;
  WORD  WrtDate;
  WORD  FstClusLO;
  DWORD FileSize;
}
__attribute__ ((packed)) tDirEntry;


/* LFN.C - Long file names support procedures */
int fat_expand_fcb_name  (const struct nls_operations *nls, char *Dest, const BYTE *Source, size_t size);
int fat_compare_fcb_names(BYTE *Name1, BYTE *Name2); /* was from the FS layer */

#endif /* #ifndef __FD32_FAT_H */
