
int i2c_read(unsigned char device, unsigned char *buf,
             unsigned char offset, int count);

int i2c_write(unsigned char device, unsigned char *buf,
              unsigned char offset, int count);
