#[[
Load config values from `.config` and default values
from `config.cmake`.

This script is intended to be used as -C option of
cmake command.
#]]

macro(chcore_config _config_name _config_type _default _description)
    if(NOT DEFINED ${_config_name})
        # config is not in `.config`, set default value
        set(${_config_name}
            ${_default}
            CACHE ${_config_type} ${_description})
    endif()
endmacro()

include(${CMAKE_CURRENT_LIST_DIR}/LoadConfig.cmake)
