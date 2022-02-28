#[[
Load config values from `.config` and check if all
cache variables defined in `config.cmake` are set,
if not, abort.

This script is intended to be used as -C option of
cmake command.
#]]

macro(chcore_config _config_name _config_type _default _description)
    if(NOT DEFINED ${_config_name})
        message(FATAL_ERROR "${_config_name} is not set")
    endif()
endmacro()

include(${CMAKE_CURRENT_LIST_DIR}/LoadConfig.cmake)
