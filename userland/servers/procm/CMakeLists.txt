add_executable(lab4.bin lab4.c elf.c launch.c spawn.c) # lab4
add_executable(procm.srv main.c elf.c launch.c spawn.c)

macro(_incbin _binary_name _dep_target _binary_path)
    set(binary_name ${_binary_name})
    set(binary_path ${_binary_path})
    configure_file(incbin.tpl.S incbin_${_binary_name}.S)
    unset(binary_name)
    unset(binary_path)
    target_sources(lab4.bin PRIVATE incbin_${_binary_name}.S) # lab4
    add_dependencies(lab4.bin ${_dep_target}) # lab4
    target_sources(procm.srv PRIVATE incbin_${_binary_name}.S)
    add_dependencies(procm.srv ${_dep_target})
endmacro()

# lab4-specific (do not have fs)
_incbin(userproc_elf user.bin ${CMAKE_CURRENT_BINARY_DIR}/../../apps/lab4/user.bin)
_incbin(ipc_client_elf ipc_client.bin ${CMAKE_CURRENT_BINARY_DIR}/../../apps/lab4/ipc_client.bin)

chcore_install_target(lab4.bin) # lab4

# lab5-specific
_incbin(fsm_elf fsm.srv ${CMAKE_CURRENT_BINARY_DIR}/../fsm/fsm.srv)
_incbin(tmpfs_elf tmpfs.srv ${CMAKE_CURRENT_BINARY_DIR}/../tmpfs/tmpfs.srv)
_incbin(shell_elf shell.srv ${CMAKE_CURRENT_BINARY_DIR}/../shell/shell.srv)
_incbin(fakefs_elf fakefs.srv ${CMAKE_CURRENT_BINARY_DIR}/../fakefs/fakefs.srv)

chcore_install_target(procm.srv)
