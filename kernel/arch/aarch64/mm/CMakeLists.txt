target_sources(
    ${kernel_target}
    PRIVATE cache.c
            memcpy.S
            memmove.S
            # remove memset.S since it requires FPU mm/memset.S
            memset.c
            page_table.c
            page_table.S
            tlb.c
            uaccess.c
            vmspace.c)
