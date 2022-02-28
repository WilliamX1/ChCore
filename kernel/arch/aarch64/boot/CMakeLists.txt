# Add bootloader source files
add_subdirectory(${CHCORE_PLAT})
message(STATUS "init_objects: ${init_objects}")
string(REGEX REPLACE ";" "\n" init_objects "${init_objects}")

# Prepare and set linker script
configure_file(linker.tpl.ld linker.ld.S)
add_custom_target(
    linker.ld ALL
    COMMAND
        ${CMAKE_C_COMPILER} -E -x c
        -I${CMAKE_CURRENT_LIST_DIR}/${CHCORE_PLAT}/include
        ${CMAKE_CURRENT_BINARY_DIR}/linker.ld.S | grep -v "^#" >
        ${CMAKE_CURRENT_BINARY_DIR}/linker.ld)
add_dependencies(${kernel_target} linker.ld)
target_link_options(${kernel_target} PRIVATE -T
                    ${CMAKE_CURRENT_BINARY_DIR}/linker.ld)
