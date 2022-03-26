#pragma once

#include <common/types.h>

struct lock_impl {
        volatile u32 owner;
        volatile u32 next;
};
