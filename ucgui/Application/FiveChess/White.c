/*
  C-file generated by �C/GUI-BitmapConvert V2.30c, compiled Jul 23 2002, 12:06:33

  (c) 2002  Micrium, Inc.
  www.micrium.com

  (c) 1998-2002  Segger
  Microcontroller Systeme GmbH
  www.segger.com

  Source file: white
  Dimensions:  21 * 21
  NumColors:   8
*/

#include "stdlib.h"

#include "GUI.h"

/*   Palette
The following are the entries of the palette table.
Every entry is a 32-bit value (of which 24 bits are actually used)
the lower   8 bits represent the Red component,
the middle  8 bits represent the Green component,
the highest 8 bits (of the 24 bits used) represent the Blue component
as follows:   0xBBGGRR
*/

const GUI_COLOR Colorswhite[] = {
     0x000000,0xD4DCE4,0xFCFCFC,0xB4B4BC
    ,0x549CCC,0x8494AC,0x3C84C4,0x646C74
};

const GUI_LOGPALETTE Palwhite = {
  8,	/* number of entries */
  1, 	/* Has transparency */
  &Colorswhite[0]
};

const unsigned char acwhite[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x12, 0x21, 0x11, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x03, 0x11, 0x21, 0x21, 0x13, 0x13, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x11, 0x12, 0x22, 0x12, 0x11, 0x31, 0x40, 0x00, 0x00,
  0x00, 0x01, 0x22, 0x22, 0x21, 0x21, 0x11, 0x23, 0x34, 0x00, 0x00,
  0x00, 0x31, 0x22, 0x22, 0x22, 0x11, 0x22, 0x31, 0x33, 0x50, 0x00,
  0x00, 0x12, 0x22, 0x22, 0x21, 0x22, 0x11, 0x13, 0x23, 0x20, 0x00,
  0x01, 0x12, 0x22, 0x22, 0x22, 0x21, 0x21, 0x31, 0x33, 0x55, 0x00,
  0x01, 0x22, 0x21, 0x21, 0x22, 0x22, 0x21, 0x15, 0x22, 0x26, 0x00,
  0x02, 0x22, 0x22, 0x22, 0x11, 0x11, 0x21, 0x31, 0x33, 0x56, 0x00,
  0x01, 0x21, 0x22, 0x21, 0x22, 0x22, 0x12, 0x23, 0x22, 0x56, 0x00,
  0x01, 0x22, 0x12, 0x21, 0x11, 0x21, 0x12, 0x31, 0x33, 0x26, 0x00,
  0x01, 0x12, 0x22, 0x12, 0x22, 0x12, 0x13, 0x13, 0x36, 0x56, 0x00,
  0x03, 0x22, 0x12, 0x12, 0x12, 0x31, 0x31, 0x23, 0x34, 0x27, 0x00,
  0x00, 0x13, 0x22, 0x11, 0x12, 0x13, 0x13, 0x25, 0x24, 0x20, 0x00,
  0x00, 0x31, 0x31, 0x22, 0x31, 0x22, 0x23, 0x34, 0x56, 0x70, 0x00,
  0x00, 0x03, 0x23, 0x23, 0x15, 0x23, 0x53, 0x44, 0x67, 0x00, 0x00,
  0x00, 0x00, 0x23, 0x33, 0x22, 0x32, 0x34, 0x26, 0x70, 0x00, 0x00,
  0x00, 0x00, 0x03, 0x35, 0x33, 0x24, 0x22, 0x67, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x05, 0x54, 0x54, 0x57, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const GUI_BITMAP bmwhite = {
 21, /* XSize */
 21, /* YSize */
 11, /* BytesPerLine */
 4, /* BitsPerPixel */
 acwhite,  /* Pointer to picture data (indices) */
 &Palwhite  /* Pointer to palette */
};

/* *** End of file *** */
