#ifndef _wavfile_h
#define _wavfile_h 

#define WW_BADOUTPUTFILE	1
#define WW_BADWRITEHEADER	2

#define WR_BADALLOC		3
#define WR_BADSEEK		4
#define WR_BADRIFF		5
#define WR_BADWAVE		6
#define WR_BADFORMAT		7
#define WR_BADFORMATSIZE	8

#define WR_NOTPCMFORMAT		9
#define WR_NODATACHUNK		10
#define WR_BADFORMATDATA	11

int WaveReadHeader(int wavefile,int *channels,unsigned long *samplerate,int *samplebits,unsigned long *samples,unsigned long  *datastart);

#endif /* _wavfile_h_ */

