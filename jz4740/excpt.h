#ifndef __EXCPT_H__
#define __EXCPT_H__
#include <mipsregs.h>
extern unsigned int Process_RA;
extern unsigned int Process_SP;

#define SAVE_PROCESS_REGISTER() \
    do{ unsigned int sr    ;    \
    	sr = read_c0_status();    \
    	write_c0_status((sr&(~1))); \
			__asm__ __volatile__(			\
			"sw $31,0x00(%0)\n\t"			\
			"sw $29,0x00(%1)\n\t"			\
            :										\
			: "r" (&Process_RA),"r" (&Process_SP)); \
       write_c0_status(sr);     \
    }while(0)
#define RESTORE_PROCESS_REGISTER() \
			__asm__ __volatile__(			\
			"lw $31,0x00(%0)\n\t"			\
			"lw $29,0x00(%1)\n\t"			\
            :										\
			: "r" (&Process_RA),"r" (&Process_SP))
				
inline static void excpt_exit(x) \
{
	  unsigned int sr;
	  sr = read_c0_status();
	  write_c0_status(sr & (~1));
		__asm__ __volatile__("lw $2,0x00(%0)\n\t" :: "r" (&x));
		RESTORE_PROCESS_REGISTER();		
    write_c0_status(sr);
    __asm__ __volatile__(
		"jr $31\n\t"
		"nop\n\t"
		);
}				
#endif

