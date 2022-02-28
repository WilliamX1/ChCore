target_include_directories(${kernel_target} PRIVATE include)

list(
    APPEND
    _init_sources
    init/start.S
    init/mmu.c
    init/tools.S
    init/init_c.c
    peripherals/uart.c)
chcore_target_sources_out_objects(${kernel_target} PRIVATE init_objects
                                  ${_init_sources})

set(init_objects
    ${init_objects}
    PARENT_SCOPE)

chcore_objcopy_binary(${kernel_target} kernel8.img)

# Note: `$basedir` is a shell variable in `emulate.tpl.sh`
chcore_generate_emulate_sh(
    "qemu-system-aarch64"
    "-machine raspi3b -nographic -serial null -serial mon:stdio -m size=1G -kernel \$basedir/kernel.img"
)
