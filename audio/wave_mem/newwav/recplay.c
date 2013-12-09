#include <stdio.h>
//#include <stdlib.h>
//#include <malloc.h>
#include <string.h>
//#include <fcntl.h>
#include <sys/stat.h>
#include <linux/soundcard.h>
#include "yaffsfs.h"
#include "pcm.h"
#include "wavplay.h"

WAVFILE *g_wfile = NULL;			/* Opened wav file */
DSPFILE *g_dfile = NULL;			/* Opened /dev/dsp device */

int WavInit(void)
{

}
/*
 * Play a series of WAV files:
 */
int wavplay(char *pathname)
{

#if 0
	const char *Pathname;			/* Pathname of the open WAV file */

	if ( (Pathname = *argv) == NULL )
		Pathname = "-";			/* Standard input */
	else	
		Pathname = *(++argv);		/* Point to first pathname on command line */
#endif

//	printf("wavplay Pathname:\t%s\n",pathname);
	/*
	 * Play each Pathname:
	 */
	//do
	//{
		/*
		 * Open the wav file for read, unless its stdin:
		 */
		if ( (g_wfile = WavOpenForRead(pathname)) == NULL )
			goto errxit;


//		printf("after wavopenforread\n");
		/*
		 * Report the file details, unless in quiet mode:
		 */
		//printf("Device:\t\t%s\n",env_AUDIODEV);
//		printf("Sampling Rate:\t%lu Hz\n",(unsigned long)g_wfile->wavinfo.SamplingRate);
//		printf("Mode:\t\t%s\n",g_wfile->wavinfo.Channels == Mono ? "Mono" : "Stereo");
//		printf("Samples:\t%lu\n",(unsigned long)g_wfile->wavinfo.Samples);
//		printf("Bits:\t\t%u\n\n",(unsigned)g_wfile->wavinfo.DataBits);

		/*
		 * If not -i mode, play the file:
		 */
		if ( (g_dfile = OpenDSP(g_wfile,O_RDWR)) == NULL )
			goto errxit;
//		printf("after opendsp\n");

		if ( PlayDSP(g_dfile,g_wfile) )
			goto errxit;
//		printf("after playdsp*****\n");

                if ( CloseDSP(g_dfile) ) 
		{  /* Close /dev/dsp */
                        g_dfile = NULL;           /* Mark it as closed */
                       goto errxit;
                }
//		printf("after close dsp\n");
                g_dfile = NULL;   

                /* Mark it as closed */
                if ( WavClose(g_wfile) )          /* Close the wav file */
                        g_wfile = NULL; 	  /* Mark the file as closed */
                g_wfile = NULL;                   /* Mark the file as closed */
//		printf("after end playwav\n");

	//} while ( (Pathname = *++argv) != NULL );

	return 0;

	/*
	 * Error exit:
	 */
errxit:	
//	printf("error exit playwav\n");
	if ( g_wfile != NULL )
		WavClose(g_wfile);
//	printf("error wavclose\n");
	if ( g_dfile != NULL )
		CloseDSP(g_dfile);
//	printf("error CloseDSP\n");
	return -1;
}

