cmake_minimum_required(VERSION 3.14)
project(LibChCore C ASM)

add_compile_options(-Wall -Werror)
add_compile_options(-fPIC -nostdinc -ffreestanding)

add_library(chcore_headers INTERFACE)
target_include_directories(chcore_headers INTERFACE include/arch/${CHCORE_ARCH})
target_include_directories(chcore_headers INTERFACE include)
target_include_directories(chcore_headers INTERFACE include/libc)

file(GLOB_RECURSE _sources src/*.c)
add_library(chcore_objects OBJECT ${_sources})
target_link_libraries(chcore_objects PUBLIC chcore_headers)

file(
    GENERATE
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/objects.txt
    CONTENT "$<JOIN:$<TARGET_OBJECTS:chcore_objects>, >")

add_library(chcore STATIC)
target_link_libraries(chcore PRIVATE chcore_objects)

install(TARGETS chcore LIBRARY DESTINATION lib)
install(
    DIRECTORY include/chcore include/arch/${CHCORE_ARCH}/chcore include/libc include/sync
    DESTINATION include
    FILES_MATCHING
    PATTERN "*.h")

add_custom_target(
    chcore_crt0 ALL
    COMMAND
        ${CMAKE_C_COMPILER} -c
        -I${CMAKE_CURRENT_SOURCE_DIR}/include/arch/${CHCORE_ARCH}
        -I${CMAKE_CURRENT_SOURCE_DIR}/include -o
        ${CMAKE_CURRENT_BINARY_DIR}/crt0.o
        ${CMAKE_CURRENT_SOURCE_DIR}/crt/crt0.c)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/crt0.o DESTINATION lib)
