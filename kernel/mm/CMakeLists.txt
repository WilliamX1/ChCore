target_sources(
    ${kernel_target}
    PRIVATE buddy.c
            kmalloc.c
            mm_check.c
            mm.c
            pgfault_handler.c
            slab.c
            vmregion.c)
