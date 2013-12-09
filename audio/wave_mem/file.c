#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <linux/soundcard.h>
#include "soundfile.h"
#include "pcm.h"
#include "wavplay.h"

extern WAVFILE *g_wfile;  /* Opened wav file */
extern DSPFILE *g_dfile;  /* Opened /dev/dsp device */ 

/*
 * Internal routine to allocate WAVFILE structure:
 */
static WAVFILE *wavfile_alloc(const char *Pathname) 
{

	
	WAVFILE *wfile = (WAVFILE *) mem_malloc(sizeof (WAVFILE));

	if ( wfile == NULL ) 
	{
		printf("Allocating WAVFILE structure\n");
		return NULL;
	}

	memset(wfile,0,sizeof(*wfile));
	wfile->Pathname=Pathname;

	wfile->fd = -1;				/* Initialize fd as not open */
	wfile->wavinfo.Channels = Mono;
	wfile->wavinfo.DataBits = 8;

	return wfile;
}

/*
 * Internal routine to release WAVFILE structure:
 * No errors reported.
 */
static void wavfile_free(WAVFILE *wfile)
{
	mem_free(wfile);
	wfile = NULL;
}

/*
 * Open a WAV file for reading: returns (WAVFILE *)
 *
 * The opened file is positioned at the first byte of WAV file data, or
 * NULL is returned if the open is unsuccessful.
 */
WAVFILE *WavOpenForRead(const char *Pathname) 
{
	WAVFILE *wfile;
	UInt32 offset;				/* File offset */
	Byte ubuf[4];				/* 4 byte buffer */
	UInt32 dbytes;				/* Data byte count */
						/* wavfile.c values : */
	int channels;				/* Channels recorded in this wav file */
	u_long samplerate;			/* Sampling rate */
	int sample_bits;			/* data bit size (8/12/16) */
	u_long samples;				/* The number of samples in this file */
	u_long datastart;			/* The offset to the wav data */

	if(g_wfile) WavClose(g_wfile); 

	wfile = wavfile_alloc(Pathname);

	if ( wfile == NULL )
		return NULL;			/* Insufficient memory (class B msg) */

	/*
	 * Open the file for reading:
	 */
//	printf("Opening WAV file %p\n", wfile->Pathname);
	if ( (wfile->fd = sound_open(wfile->Pathname,O_RDONLY)) < 0 ) 
	{
		printf("Opening WAV file %p failed\n", wfile->Pathname);
		goto errxit;
	}

	if ( sound_lseek(wfile->fd,0,SEEK_SET) != 0 ) 
	{
		printf("Rewinding WAV file %p\n",wfile->Pathname);
		goto errxit;		/* Wav file must be seekable device */
	}

	if ( WaveReadHeader(wfile->fd,&channels,&samplerate,&sample_bits,&samples,&datastart) != 0 ) 
	{
		printf("Reading WAV header from %p", wfile->Pathname);
		goto errxit;
	}

	/*
	 * Copy WAV data over to WAVFILE struct:
	 */
	if ( channels == 2 )
		wfile->wavinfo.Channels = Stereo;
	else	wfile->wavinfo.Channels = Mono;

	wfile->wavinfo.SamplingRate = (UInt32) samplerate;
	wfile->wavinfo.Samples = (UInt32) samples;
	wfile->wavinfo.DataBits = (UInt16) sample_bits;
	wfile->wavinfo.DataStart = (UInt32) datastart;
        wfile->num_samples = wfile->wavinfo.Samples;

	offset = wfile->wavinfo.DataStart - 4;

	/*
	 * Seek to byte count and read dbytes:
	 */
	if ( sound_lseek(wfile->fd,offset,SEEK_SET) != offset ) 
	{
		printf("Seeking to WAV data in %p",wfile->Pathname);
		goto errxit;			/* Seek failure */
	}

	if ( sound_read(wfile->fd,ubuf,4) != 4 ) 
	{
		printf("Reading dbytes from %s",wfile->Pathname);
		goto errxit;
	}

	/*
	 * Put little endian value into 32 bit value:
	 */
	dbytes = ubuf[3];
	dbytes = (dbytes << 8) | ubuf[2];
	dbytes = (dbytes << 8) | ubuf[1];
	dbytes = (dbytes << 8) | ubuf[0];

	wfile->wavinfo.DataBytes = dbytes;

	/*
	 * Open succeeded:
	 */
	return wfile;				/* Return open descriptor */

	/*
	 * Return error after failed open:
	  WavClose*/
errxit:	
	mem_free(wfile);				/* Dispose of WAVFILE struct */
	return NULL;				/* Return error indication */
}

/*
 * Close a WAVFILE
 */
int WavClose(WAVFILE *wfile) 
{
	if ( wfile == NULL ) 
	{
		printf("WAVFILE pointer is NULL!\n");
		return -1;
	}

	if ( sound_close(wfile->fd) < 0 ) 
	{
		printf("Closing WAV file\n");
	}

	wavfile_free(wfile);			/* Release WAVFILE structure */

	return 0;				/* Successful exit */
}

/*
 * Open /dev/dsp for reading or writing:
 */
DSPFILE *OpenDSP(WAVFILE *wfile,int omode)
{
	int t;					/* Work int */
	unsigned long ul;			/* Work unsigned long */
	DSPFILE *dfile;

	if(g_dfile) CloseDSP(g_dfile);

	dfile = (DSPFILE *) mem_malloc(sizeof (DSPFILE));
	if ( dfile == NULL ) 
	{
		printf("Opening DSP device\n");
		return NULL;
	}

	memset(dfile,0,sizeof *dfile);
	dfile->dspbuf = NULL;
	dfile->fd=-1;

	dfile->dspblksiz = 8*1024;
 
        /*
         * Check the range on the buffer sizes:
         */
        /* Minimum was 4096 but es1370 returns 1024 for 44.1kHz, 16 bit */
        /* and 64 for 8130Hz, 8 bit */
        if ( dfile->dspblksiz < 32 || dfile->dspblksiz > 65536 )
	{
                printf("Audio block size (%d bytes)", (int)dfile->dspblksiz);
                goto errxit;
        }
 
        /*
         * Allocate a buffer to do the I/O through:
         */
        if ( (dfile->dspbuf = (char *) mem_malloc(dfile->dspblksiz)) == NULL )
	{
                printf("For DSP I/O buffer\n");
                goto errxit;
        }

	/* Reset the I2S codec */
	 pcm_ioctl(PCM_RESET, 0);	

	/*
	 * Set the data bit size:
	 */
	t = wfile->wavinfo.DataBits;
	if (t==8)
		pcm_ioctl(PCM_SET_FORMAT, 8);
	else
		pcm_ioctl(PCM_SET_FORMAT,  16);
	/*
	 * Set the mode to be Stereo or Mono:
	 */
	t = wfile->wavinfo.Channels == Stereo ? 2 : 1;
	pcm_ioctl(PCM_SET_CHANNEL, t);
      
	/*
	 * Set the sampling rate:
	 */
	ul = wfile->wavinfo.SamplingRate;
	pcm_ioctl(PCM_SET_SAMPLE_RATE,ul);

	/*
	 * Return successfully opened device:
	 */
	return dfile;				/* Return file descriptor */

	/*
	 * Failed to open/initialize properly:
	 */
errxit:	
	if ( dfile->fd >= 0 )
		close(dfile->fd);		/* Close device */
	if ( dfile->dspbuf != NULL )
		mem_free(dfile->dspbuf);
	mem_free(dfile);
	return NULL;				/* Return error indication */
}

/*
 * Close the DSP device:
 */
int CloseDSP(DSPFILE *dfile)
{
	int fd;

	if ( dfile == NULL ) 
	{
		printf("DSPFILE is not open\n");
		return -1;
	}

	if ( dfile->dspbuf != NULL )
		mem_free(dfile->dspbuf);
	mem_free(dfile);
	
	dfile = NULL;

	return 0;
}

/*
 * Play DSP from WAV file:
 */
int PlayDSP(DSPFILE *dfile, WAVFILE *wfile)
{
	UInt32 byte_count = (UInt32) wfile->wavinfo.Samples;
	int bytes;
	int n;
	int byte_modulo;
        int total_bytes;

	//First determine how many bytes are required for each channel's sample:
	switch ( wfile->wavinfo.DataBits ) 
	{
	case 8 :
		byte_count = 1;
		break;
	case 16 :
		byte_count = 2;
		break;
	default :
		printf("Cannot process %u bit samples\n", (unsigned)wfile->wavinfo.DataBits);
		return -1;
	}

	byte_modulo = byte_count;				/* This many bytes per sample */
	byte_count  = wfile->wavinfo.Samples * byte_modulo;	/* Total bytes to process */
        total_bytes = byte_count;

        /* Seek to requested start sample */
        sound_lseek(wfile->fd,wfile->StartSample*byte_modulo,SEEK_CUR);
 
	for (; byte_count > 0 && wfile->wavinfo.DataBytes > 0; byte_count -= (UInt32) n ) 
	{
		bytes = (int) ( byte_count > dfile->dspblksiz ? dfile->dspblksiz : byte_count );
		if ( bytes > wfile->wavinfo.DataBytes )	/* Size bigger than data chunk? */
			bytes = wfile->wavinfo.DataBytes;	/* Data chunk only has this much left */

	//	printf("play databytes:%d bytes:%d n:%d\n",wfile->wavinfo.DataBytes,bytes,n);

		if ( (n = sound_read(wfile->fd,dfile->dspbuf,bytes)) != bytes ) 
		{
			if ( n >= 0 )
				printf("Unexpected EOF reading samples from WAV file\n");
			else	
				printf("Reading samples from WAV file\n");
			goto errxit;
		}
	//	printf("pcm writeing data\n");//treckle
		if (pcm_write(dfile->dspbuf,n) <= 0)
		{
			printf("Writing samples data failed\n");
			goto errxit;
		}
	//	printf("end pcm writeing data\n");//treckle
		wfile->wavinfo.DataBytes -= (UInt32) bytes;	/* We have fewer bytes left to read */
		//printf("dspblksize=%d bytes=%d DataBytes=%d\n", dfile->dspblksiz, bytes, wfile->wavinfo.DataBytes);

        }
	return 0;	/* All samples played successfully */

	errxit:	return -1;	/* Indicate error return */
}

void SetAudioVol(int vol)
{
	unsigned char c, val=0;

	/* vol 0 - 99 it's persation */
	if(vol>4)
		c = vol/20;
	else
		c = val;	

//	printf("the vol=%d c=%d\n",vol,c);//treckle
        /* Set the volume 0 - 31 */
	switch(c)
	{
		case 0:
			val=0;	
			break;
		case 1:
			val=15;	
			break;
		case 2:
			val=20;	
			break;
		case 3:
			val=25;	
			break;
		case 4:
			val=30;	
			break;
		default:
			val=25;
			break;
	}
//	printf("val=%d\n",val);//treckle
        pcm_ioctl(PCM_SET_HP_VOL,val);
//	pcm_ioctl(PCM_SET_VOL,vol);
	return;
}
