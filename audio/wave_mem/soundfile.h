
#include "yaffsfs.h"
int sound_open(char *buf, int mode);
int sound_close(int fd);
int sound_lseek(int fd, int offset, int whence);
int sound_read(int fd, char *buf, unsigned int nbyte);

