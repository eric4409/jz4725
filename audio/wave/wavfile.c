#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "wavplay.h"

#define		BUFFERSIZE   		1024
#define		PCM_WAVE_FORMAT   	1

#define		TRUE			1
#define		FALSE			0

typedef  struct
{	u_long     dwSize ;
	u_short    wFormatTag ;
	u_short    wChannels ;
	u_long     dwSamplesPerSec ;
	u_long     dwAvgBytesPerSec ;
	u_short    wBlockAlign ;
	u_short    wBitsPerSample ;
} WAVEFORMAT ;

typedef  struct
{	char    	RiffID [4] ;
	u_long    	RiffSize ;
	char    	WaveID [4] ;
	char    	FmtID  [4] ;
	u_long    	FmtSize ;
	u_short   	wFormatTag ;
	u_short   	nChannels ;
	u_long		nSamplesPerSec ;
	u_long		nAvgBytesPerSec ;
	u_short		nBlockAlign ;
	u_short		wBitsPerSample ;
	char		DataID [4] ;
	u_long		nDataBytes ;
} WAVE_HEADER ;

/*=================================================================================================*/

char*  findchunk (char* s1, char* s2, size_t n) ;

/*=================================================================================================*/

int  WaveReadHeader  (int wavefile, int* channels, unsigned long* samplerate, int* samplebits, unsigned long * samples, unsigned long* datastart)
{	static  WAVEFORMAT  waveformat ;
	static	char   buffer [ BUFFERSIZE ] ;		/* Function is not reentrant.*/
	char*   ptr ;
	u_long  databytes ;

	if (yaffs_lseek (wavefile, 0, SEEK_SET)) {
		return  WR_BADSEEK ;
	}

	yaffs_read (wavefile, buffer, BUFFERSIZE) ;
	if (findchunk (buffer, "RIFF", BUFFERSIZE) != buffer) {
		printf("Bad format: Cannot find RIFF file marker");	/* wwg: Report error */
		return  WR_BADRIFF ;
	}

	if (! findchunk (buffer, "WAVE", BUFFERSIZE)) {
		printf("Bad format: Cannot find WAVE file marker");	/* wwg: report error */
		return  WR_BADWAVE ;
	}

	ptr = findchunk (buffer, "fmt ", BUFFERSIZE) ;

	if (! ptr) {
		printf("Bad format: Cannot find 'fmt' file marker");	/* wwg: report error */
		return  WR_BADFORMAT ;
	}

	ptr += 4 ;	/* Move past "fmt ".*/
	memcpy (&waveformat, ptr, sizeof (WAVEFORMAT)) ;

	if (waveformat.dwSize < (sizeof (WAVEFORMAT) - sizeof (u_long))) {
		printf("Bad format: Bad fmt size");			/* wwg: report error */
		return  WR_BADFORMATSIZE ;
	}

	if (waveformat.wFormatTag != PCM_WAVE_FORMAT) {
		printf("Only supports PCM wave format");			/* wwg: report error */
		return  WR_NOTPCMFORMAT ;
	}

	ptr = findchunk (buffer, "data", BUFFERSIZE) ;

	if (! ptr) {
		printf("Bad format: unable to find 'data' file marker");	/* wwg: report error */
		return  WR_NODATACHUNK ;
	}

	ptr += 4 ;	/* Move past "data".*/
	memcpy (&databytes, ptr, sizeof (u_long)) ;

	/* Everything is now cool, so fill in output data.*/

	*channels   = waveformat.wChannels ;
	*samplerate = waveformat.dwSamplesPerSec ;
	*samplebits = waveformat.wBitsPerSample ;
	*samples    = databytes / waveformat.wBlockAlign ;
	
	*datastart  = ((u_long) (ptr + 4)) - ((u_long) (&(buffer[0]))) ;

	if (waveformat.dwSamplesPerSec != waveformat.dwAvgBytesPerSec / waveformat.wBlockAlign) {
		printf("Bad file format");			/* wwg: report error */
		return  WR_BADFORMATDATA ;
	}

	if (waveformat.dwSamplesPerSec != waveformat.dwAvgBytesPerSec / waveformat.wChannels / ((waveformat.wBitsPerSample == 16) ? 2 : 1)) {
		printf("Bad file format");			/* wwg: report error */
		return  WR_BADFORMATDATA ;
	}

  	return  0 ;
} ; /* WaveReadHeader*/

/*===========================================================================================*/

char* findchunk  (char* pstart, char* fourcc, size_t n)
{	char	*pend ;
	int		k, test ;

	pend = pstart + n ;

	while (pstart < pend)
	{ 	if (*pstart == *fourcc)       /* found match for first char*/
		{	test = TRUE ;
			for (k = 1 ; fourcc [k] != 0 ; k++)
				test = (test ? ( pstart [k] == fourcc [k] ) : FALSE) ;
			if (test)
				return  pstart ;
			} ; /* if*/
		pstart ++ ;
		} ; /* while lpstart*/

	return  NULL ;
} ; /* findchuck*/

