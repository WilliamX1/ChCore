#pragma once

struct process_metadata {
        u64 phdr_addr;
        u64 phentsize;
        u64 phnum;
        u64 flags;
        u64 entry;
};

#define ENV_MAGIC   0x43484f53
#define ENV_SIZE    0x1000
#define ENV_NO_CAPS (-1)
