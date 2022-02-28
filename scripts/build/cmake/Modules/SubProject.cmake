# A simple wrapper to the built-in ExternalProject module.

include(ExternalProject)

macro(chcore_add_subproject)
    # Note: may encounter problem when need to forward empty arguments
    ExternalProject_Add(${ARGN})
endmacro()
