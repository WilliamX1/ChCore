#pragma once

extern void flush_dcache_area(u64 addr, u64 size);
extern void enable_irq(void);
extern void disable_irq(void);
extern void put32(u64 addr, u32 data);
extern unsigned int get32(u64 addr);
