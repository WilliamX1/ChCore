#pragma once

/* Leave 8K space to kernel stack */
#define KERNEL_STACK_SIZE (8192)
#define IDLE_STACK_SIZE   (8192)
#define STACK_ALIGNMENT   16

//#include <arch/mmu.h>
/* can be different in different architectures */
#ifndef KBASE
#define KBASE 0xffffff0000000000
#endif
