#pragma once

#include <common/types.h>
#include <arch/machine/registers.h>

/* size in registers.h (to be used in asm) */
typedef struct arch_exec_cont {
        u64 reg[REG_NUM];
} arch_exec_cont_t;
