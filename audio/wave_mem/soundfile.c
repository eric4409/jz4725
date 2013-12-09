#include "soundfile.h"

unsigned char *wavbuf=NULL;
int position;

int sound_open(char *buf, int mode)
{
//	printf(" Opening wav file at 0x%p\n",buf);
	wavbuf=(unsigned char *)buf;
	position = 0;
	return 1;
}

int sound_close(int fd)
{
	wavbuf = NULL;
	position = 0;
	return 0;
}

int sound_lseek(int fd, int offset, int whence)
{
	int pos = -1;

	if(whence == SEEK_SET)
        {
                if(offset >= 0)
                {
                        pos= offset;
                }
        }
        else if(whence == SEEK_CUR)
        {
                if( (position + offset) >= 0)
                {
                        pos = (position + offset);
                }
        }
        else if(whence == SEEK_END)
	{

	}

	if(pos >= 0)
        {
                position = pos;
        }
        else
        {
                // todo error
        }
//	printf("sound_lseek:the position =%d\n",position);
	return pos;
}

int sound_read(int fd, char *buf, unsigned int nbyte)
{
	int nRead = nbyte;

//	printf("sound_read: reading data from %p,position=%d,nRead=%d\n",wavbuf+position,position,nRead);
	if(nRead>0)
	{
		memcpy(buf, wavbuf+position, nRead);
		position += nRead;
	}
	else
		nRead = 0;
//	printf("sound_read: position=%d\n",position);
//	printf("buf=%x %x %x %x %x\n",buf[0],buf[1],buf[2],buf[3],buf[4]);
        return (nRead >= 0) ? nRead : -1;

}




