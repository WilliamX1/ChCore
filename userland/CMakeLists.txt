cmake_minimum_required(VERSION 3.14)
project(ChCoreUserLand C ASM)

include(CommonTools)
include(UserTools)

set(_libchcore_install_dir ${CMAKE_CURRENT_SOURCE_DIR}/../libchcore/_install)

add_compile_options(-fPIE -nostdinc -ffreestanding)
include_directories(${_libchcore_install_dir}/include
                    ${_libchcore_install_dir}/include/libc)
add_link_options(-static -no-pie -nostdlib -nostartfiles -e _start)
link_libraries(${_libchcore_install_dir}/lib/crt0.o
               ${_libchcore_install_dir}/lib/libchcore.a)

set(_ramdisk_dir ${CMAKE_CURRENT_BINARY_DIR}/ramdisk)
file(REMOVE_RECURSE ${_ramdisk_dir})
file(MAKE_DIRECTORY ${_ramdisk_dir})
add_custom_target(
    ramdisk.cpio ALL
    WORKING_DIRECTORY ${_ramdisk_dir}
    COMMAND find . | cpio -o -H newc > ${CMAKE_CURRENT_BINARY_DIR}/ramdisk.cpio)

function(chcore_copy_files_to_ramdisk)
    file(COPY ${ARGN} DESTINATION ${_ramdisk_dir})
endfunction()

function(chcore_copy_target_to_ramdisk _target)
    add_custom_command(
        TARGET ${_target}
        POST_BUILD
        COMMAND cp $<TARGET_FILE:${_target}> ${_ramdisk_dir})
    add_dependencies(ramdisk.cpio ${_target})
    set_property(GLOBAL PROPERTY ${_target}_INSTALLED TRUE)
endfunction()

function(chcore_copy_all_targets_to_ramdisk)
    set(_targets)
    chcore_get_all_targets(_targets)
    foreach(_target ${_targets})
        get_property(_installed GLOBAL PROPERTY ${_target}_INSTALLED)
        if(${_installed})
            continue()
        endif()
        get_target_property(_target_type ${_target} TYPE)
        if(${_target_type} STREQUAL SHARED_LIBRARY OR ${_target_type} STREQUAL
                                                      EXECUTABLE)
            chcore_copy_target_to_ramdisk(${_target})
        endif()
    endforeach()
endfunction()

add_subdirectory(servers)
add_subdirectory(apps)
