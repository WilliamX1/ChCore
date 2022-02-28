# CMake toolchain for building ChCore kernel.

if(NOT DEFINED CHCORE_PROJECT_DIR)
    message(FATAL_ERROR "CHCORE_PROJECT_DIR is not defined")
else()
    message(STATUS "CHCORE_PROJECT_DIR: ${CHCORE_PROJECT_DIR}")
endif()

# Set toolchain executables
set(CMAKE_ASM_COMPILER "${CHCORE_CROSS_COMPILE}gcc")
set(CMAKE_C_COMPILER "${CHCORE_CROSS_COMPILE}gcc")
set(CMAKE_AR "${CHCORE_CROSS_COMPILE}ar")
set(CMAKE_NM "${CHCORE_CROSS_COMPILE}nm")
set(CMAKE_OBJCOPY "${CHCORE_CROSS_COMPILE}objcopy")
set(CMAKE_OBJDUMP "${CHCORE_CROSS_COMPILE}objdump")
set(CMAKE_RANLIB "${CHCORE_CROSS_COMPILE}ranlib")
set(CMAKE_STRIP "${CHCORE_CROSS_COMPILE}strip")

# Set build type
if(CHCORE_KERNEL_DEBUG)
    set(CMAKE_BUILD_TYPE "Debug")
else()
    set(CMAKE_BUILD_TYPE "Release")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/_common.cmake)

# Set the target system (automatically set CMAKE_CROSSCOMPILING to true)
set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_SYSTEM_PROCESSOR ${CHCORE_ARCH})
