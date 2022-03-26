#pragma once

/*
 * Magic prefix for stack env page, the value is ASCII for "CHOS".
 * Keep it 4 bytes for compatibility with 32 bit architectures.
 */
#define ENV_MAGIC   0x43484f53
#define ENV_SIZE    0x1000
#define ENV_NO_CAPS (-1)

#include <chcore/internal/mem_layout_arch.h>
