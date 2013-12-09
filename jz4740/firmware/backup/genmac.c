
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <asm/io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>



#define CONFIG_KEY1    0x0BADFACE
#define CONFIG_KEY2    0xDEADDEAD



unsigned char mac[6]={0x00, 0x0A, 0x5D};
static struct _config {
    unsigned long key1;
    unsigned char config_data[1024-(3*4)];
    unsigned long key2;
    unsigned long cksum;
} config;

unsigned long  _fconfig_cksum(unsigned long *buf, int len);
int   generate_mac();


int generate_mac()
{
  int fd = -1;
  unsigned char mac_lsb[7];
  unsigned int value = 0;
  
  fd=open("mac.res",O_RDWR);
  
  if(fd<0){
	  printf("mac.res not found!\n");
	  return -1;
  }
  read(fd,mac_lsb,6);
  mac_lsb[6] = '\0';
  value = atoi(mac_lsb);
  
  mac[3] = (value >> 16) & 0xFF;
  mac[4] = (value >> 8) & 0xFF;
  mac[5] = value & 0xFF;
  value += 1;
  sprintf(mac_lsb,"%6.6d",value);
  lseek(fd,0,SEEK_SET);
  write(fd,mac_lsb,6);
  close(fd);
  return 0;
  
}


unsigned long  _fconfig_cksum(unsigned long *buf, int len)
{
    unsigned long cksum = 0;
    int shift = 0;

    // Round 'len' up to multiple of longwords
    len = (len + (sizeof(unsigned long)-1)) / sizeof(unsigned long);   
    while (len-- > 0) {
        cksum ^= (*buf | (*buf << shift));
        buf++;
        if (++shift == 16) shift = 0;
    }
    return cksum;
}


int main(int argc , char ** argv)
{
   int fd = -1, len = 0, i = 0 ;

   fd = open("config.img",O_RDWR);
   
   if( fd < 0){
	printf("config image not found!\n");
	return -1;
   }
  len=read(fd,&config,sizeof(config));
  
  if(len != sizeof(config)){
       printf("read config data failed\n");
       return -1;
  }
  if(generate_mac()<0)  {
        printf("generate mac failed\n");
        close(fd);
	return -1;
  }
  for(;i <6;i++) config.config_data[i]=mac[i];
  config.key1 = CONFIG_KEY1;
  config.key2 = CONFIG_KEY2;
  config.cksum = _fconfig_cksum((unsigned long *)&config,sizeof(config)-4);
  lseek(fd,0,SEEK_SET);
  len = write(fd,&config,sizeof(config));
  if(len == sizeof(config)) { 
      printf("set mac ok,new mac is  ");
      for(i = 0;i < 6;i++){
	  printf("%2.2X",config.config_data[i]);
	  if(i != 5) putchar(':');
	  else  putchar('\n');
      }
  }
  else  printf("Modify mac failed\n"); 
  close(fd);
  return 0;
	  
}
