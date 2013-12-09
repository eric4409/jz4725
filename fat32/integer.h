/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef _INTEGER
//#include "ccc.h"
#if 0
#include <windows.h>
#else

#if 1
/* These types must be 16-bit, 32-bit or larger integer */
typedef int		INT;
typedef unsigned int	UINT;

/* These types must be 8-bit integer */
typedef signed char		CHAR;
typedef unsigned char	UCHAR;
#ifndef BYTE
typedef unsigned char	BYTE;
#endif

/* These types must be 16-bit integer */
typedef short			SHORT;
typedef unsigned short	USHORT;
#endif
#ifndef WORD
typedef unsigned short	WORD;
#endif

#ifndef WCHAR
typedef unsigned short	WCHAR;
#endif

/* These types must be 32-bit integer */
#ifndef LOGN
typedef long			LONG;
#endif

typedef unsigned long	ULONG;
#ifndef DWORD
typedef unsigned long	DWORD;
#endif

/* Boolean type */
#ifndef BOOL
typedef enum { FALSE = 0, TRUE } BOOL;
#endif
#endif

#define _INTEGER
#endif
