target_sources(${kernel_target} PRIVATE head.S main.c tools.S)

add_subdirectory(boot)
