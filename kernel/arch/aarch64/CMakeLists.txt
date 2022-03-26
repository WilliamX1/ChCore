target_sources(${kernel_target} PRIVATE head.S main.c tools.S)

add_subdirectory(boot)

add_subdirectory(irq)
add_subdirectory(machine)
add_subdirectory(mm)
add_subdirectory(sched)
add_subdirectory(sync)
