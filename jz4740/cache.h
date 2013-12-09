

void __icache_invalidate_all(void);
void __dcache_invalidate_all(void);
void __dcache_writeback_all(void);
void dma_cache_wback_inv(unsigned long addr, unsigned long size);
