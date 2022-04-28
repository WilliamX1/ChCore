file(GLOB _sources "*.c")
add_executable(shell.srv ${_sources})

if(CHCORE_SHELL_TEST)
    target_compile_options(shell.srv PRIVATE "-DSHELL_TEST")
endif()

if(CHCORE_FSM_TEST)
    target_compile_options(shell.srv PRIVATE "-DFSM_TEST")
endif()


