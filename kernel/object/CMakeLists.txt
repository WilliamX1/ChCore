target_sources(
    ${kernel_target}
    PRIVATE cap_group.c
            capability.c
            memory.c
            set_thread_env.c
            thread.c)
