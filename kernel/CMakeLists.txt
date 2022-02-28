cmake_minimum_required(VERSION 3.14)
project(ChCoreKernel C ASM)

include(CommonTools)
include(KernelTools)

chcore_dump_cmake_vars()

# Create kernel.img target
set(kernel_target "kernel.img")
add_executable(${kernel_target})
install(TARGETS ${kernel_target} DESTINATION ${CMAKE_INSTALL_PREFIX})

# Set warning level
list(APPEND _compile_options -Wall)
# list(APPEND _compile_options -Werror)

# Generic compile settings
list(APPEND _compile_definitions)
list(APPEND _compile_options -nostdinc -ffreestanding -fPIC)
list(APPEND _asm_compile_definitions __ASM__)
list(APPEND _asm_compile_options)
list(APPEND _c_compile_definitions)
list(APPEND _c_compile_options)

# Arch-specific compile settings
if(CHCORE_ARCH STREQUAL "aarch64")
    list(APPEND _compile_options -march=armv8-a+nofp)
endif()

# Set compile settings to target
target_compile_definitions(${kernel_target} PRIVATE ${_compile_definitions})
target_compile_options(${kernel_target} PRIVATE ${_compile_options})
target_compile_definitions(
    ${kernel_target}
    PRIVATE $<$<COMPILE_LANGUAGE:ASM>:${_asm_compile_definitions}>)
target_compile_options(
    ${kernel_target} PRIVATE $<$<COMPILE_LANGUAGE:ASM>:${_asm_compile_options}>)
target_compile_definitions(
    ${kernel_target} PRIVATE $<$<COMPILE_LANGUAGE:C>:${_c_compile_definitions}>)
target_compile_options(${kernel_target}
                       PRIVATE $<$<COMPILE_LANGUAGE:C>:${_c_compile_options}>)

# Linker options
target_link_options(${kernel_target} PRIVATE -no-pie -nostdlib -nostartfiles
                    -Wl,--build-id=none)

# Build-config-specific options
target_compile_definitions(${kernel_target}
                           PRIVATE $<$<CONFIG:Debug>:LOG_LEVEL=2>)
target_compile_definitions(${kernel_target}
                           PRIVATE $<$<CONFIG:Release>:LOG_LEVEL=1>)

# Include directories
target_include_directories(${kernel_target} PRIVATE include)
target_include_directories(${kernel_target} PRIVATE include/arch/${CHCORE_ARCH})
target_include_directories(
    ${kernel_target} PRIVATE include/arch/${CHCORE_ARCH}/plat/${CHCORE_PLAT})

# Add source code directories
add_subdirectory(arch/${CHCORE_ARCH})
add_subdirectory(arch/${CHCORE_ARCH}/plat/${CHCORE_PLAT})
add_subdirectory(lib)
