#ifndef _wavplay_h_
#define _wavplay_h_ 

#include "wavfile.h"

/*
 * Default lowest sampling rate unless overriden by the Makefile:
 */
#ifndef DSP_MIN
#define DSP_MIN		4000			/* Lowest acceptable sample rate */
#endif

/*
 * Default maximum sampling rate unless overrided by the Makefile:
 */
#ifndef DSP_MAX
#define DSP_MAX		48000			/* Highest acceptable sample rate */
#endif

/*
 * Default pathname of the audio device, unless overrided by the Makefile:
 */
#ifndef AUDIODEV
#define AUDIODEV	"/dev/dsp"		/* Default pathname for audio device */
#endif

/*
 * Types internal to wavplay, in an attempt to isolate ourselves from
 * a dependance on a particular platform.
 */
typedef unsigned char Byte;
typedef short Int16;
typedef long Int32;
typedef unsigned long UInt32;
typedef unsigned short UInt16;

/*
 * This value sets buffer sizes for temporary buffers that sprintf()
 * uses, and for copying pathnames around in. You probably don't want
 * to mess with this.
 */
#define MAXTXTLEN	2048	/* Must allow for long pathnames */

/*
 * This enumerated type, selects between monophonic sound and
 * stereo sound (1 or 2 channels).
 */
typedef enum {
        Mono,                                   /* Monophonic sound (1 channel) */
        Stereo                                  /* Stereo sound (2 channels) */
} Chan;

/*
 * These values represent values found in/or destined for a
 * WAV file.
 */
typedef struct {
        UInt32  SamplingRate;                   /* Sampling rate in Hz */
        Chan    Channels;                       /* Mono or Stereo */
        UInt32  Samples;                        /* Sample count */
        UInt16  DataBits;                       /* Sample bit size (8/12/16) */
        UInt32  DataStart;                      /* Offset to wav data */
        UInt32  DataBytes;                      /* Data bytes in current chunk */
        char    bOvrSampling;                   /* True if sampling_rate overrided */
        char    bOvrMode;                       /* True if chan_mode overrided */
        char    bOvrBits;                       /* True if data_bits is overrided */
} WAVINF;
                                                                                                               
/*
 * This structure manages an open WAV file.
 */
typedef struct {
        char    *Pathname;                      /* Pathname of wav file */
        int     fd;                             /* Open file descriptor or -1 */
        WAVINF  wavinfo;                        /* WAV file hdr info */
        UInt32  num_samples;                    /* Total number of samples */
        UInt32  StartSample;                    /* First sample to play */
} WAVFILE;
                                                                                                               
/*
 * This macro is used to return the system file descriptor
 * associated with the open WAV file, given a (WAVFILE *).
 */
#define WAV_FD(wfile) (wfile->fd)               /* Return file descriptor */
                                                                                                               
/*
 * This structure manages an opened DSP device.
 */
typedef struct {
        int     fd;                             /* Open fd of /dev/dsp */
        int     dspblksiz;                      /* Size of the DSP buffer */
        char    *dspbuf;                        /* The buffer */
} DSPFILE;

char *env_AUDIODEV;                      /* Default compiled in audio device */

WAVFILE *WavOpenForRead(const char *Pathname);
int WavClose(WAVFILE *wfile);

DSPFILE *OpenDSP(WAVFILE *wfile,int omode);
int PlayDSP(DSPFILE *dfile,WAVFILE *wfile);
int CloseDSP(DSPFILE *dfile);

int wavplay(char *pathname);

#endif /* _wavplay_h_ */

