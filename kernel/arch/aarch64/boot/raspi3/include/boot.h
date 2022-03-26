#pragma once

extern void el1_mmu_activate(void);
extern void init_boot_pt(void);

extern void start_kernel(void *boot_flag);
extern void secondary_cpu_boot(int cpuid);

extern char _bss_start;
extern char _bss_end;

#define PLAT_CPU_NUMBER 4

#define ALIGN(n) __attribute__((__aligned__(n)))
