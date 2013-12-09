#include <stdio.h>


int DumpData(unsigned char *data, int dataSize)
{
        static char Hex[17]="0123456789ABCDEF";
        int i, linec=0;
        char line[20];
        printf("Dump Data: Size=%d\n", dataSize);
        for(i=0; i<dataSize;i++)
        {
                char HexData[3];
                HexData[0]=Hex[data[i]>>4]; HexData[1]=Hex[data[i]&0x0F]; HexData[2]=0;
                if(linec==0) printf("  ");
                printf("%s ", HexData);
                if(data[i]>=0x20 && data[i]<0xFF)
                        line[linec]=data[i];
                else
                        line[linec]='.';
                if(++linec>=16)
                {
                        line[linec]=0;
                        printf("  %s\n", line);
                        linec=0;
                }
        }
        if(linec)
        {
                for(i=0;i<16-linec;i++) printf("   ");
                line[linec]=0;
                printf("  %s\n",line);
        }
        return 0;
}


