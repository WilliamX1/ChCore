#[[
Dump cache variables from `CMakeCache.txt` to `.config`.

This script is intended to be used as -C option of
cmake command.
#]]

include(${CMAKE_CURRENT_LIST_DIR}/Modules/CommonTools.cmake)

set(_config_lines)

macro(chcore_config _config_name _config_type _default _description)
    # Dump config lines in definition order
    list(APPEND _config_lines
         "${_config_name}:${_config_type}=${${_config_name}}")
endmacro()

include(${CMAKE_SOURCE_DIR}/config.cmake)

string(REPLACE ";" "\n" _config_str "${_config_lines}")

file(WRITE ${CMAKE_SOURCE_DIR}/.config "${_config_str}\n")
