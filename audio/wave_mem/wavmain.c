#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "wavplay.h"

char *env_AUDIODEV = AUDIODEV;				/* Default compiled in audio device */

/*
 * Main program:
 */
int
PlayWavFile(char *wavname) {
	int rc;					/* Return code */
	
	rc = wavplay(wavname); /* Play samples */
	return rc;
}

