function(chcore_install_target _target)
    install(TARGETS ${_target} DESTINATION ${CMAKE_INSTALL_PREFIX})
    set_property(GLOBAL PROPERTY ${_target}_INSTALLED TRUE)
endfunction()

# Get all "build system targets" defined in the current source dir,
# recursively.
function(chcore_get_all_targets _out_var)
    set(_targets)
    _get_all_targets_recursive(_targets ${CMAKE_CURRENT_SOURCE_DIR})
    set(${_out_var}
        ${_targets}
        PARENT_SCOPE)
endfunction()

macro(_get_all_targets_recursive _targets _dir)
    get_property(
        _subdirectories
        DIRECTORY ${_dir}
        PROPERTY SUBDIRECTORIES)
    foreach(_subdir ${_subdirectories})
        _get_all_targets_recursive(${_targets} ${_subdir})
    endforeach()

    get_property(
        _current_targets
        DIRECTORY ${_dir}
        PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND ${_targets} ${_current_targets})
endmacro()

function(chcore_install_all_targets)
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
            chcore_install_target(${_target})
        endif()
    endforeach()
endfunction()
