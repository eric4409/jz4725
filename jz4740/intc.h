
void enable_irq(unsigned int irq);
void disable_irq(unsigned int irq);
void ack_irq(unsigned int irq);
void intc_init(void);
int request_irq(unsigned int irq, void (*handler)(unsigned int), unsigned arg);
void free_irq(unsigned int irq);
//void C_vINTHandler(CP0_tstREGS *pstC0);
void cli(void);
unsigned int mips_get_sr(void);
void sti(void);
unsigned int spin_lock_irqsave(void);
void spin_unlock_irqrestore(unsigned int val);

