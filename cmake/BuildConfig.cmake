# Central build configuration helper
# Creates an INTERFACE target with the passed NAME and attaches common
# compile definitions/options and link options based on variables defined
# by the including project (e.g., PROJECT_DEFINES, COMMON_OPTIONS, etc.).

function(art_setup_build_config NAME)
    if (NOT NAME)
        message(FATAL_ERROR "art_setup_build_config(NAME): NAME is required")
    endif ()

    if (TARGET ${NAME})
        return()
    endif ()

    add_library(${NAME} INTERFACE)

    # Compile definitions
    if (DEFINED PROJECT_DEFINES)
        target_compile_definitions(${NAME} INTERFACE ${PROJECT_DEFINES})
    endif ()
    if (DEFINED DEBUG_DEFINES)
        target_compile_definitions(${NAME} INTERFACE "$<$<CONFIG:Debug>:${DEBUG_DEFINES}>")
    endif ()

    # Compile options
    if (DEFINED COMMON_OPTIONS)
        target_compile_options(${NAME} INTERFACE ${COMMON_OPTIONS})
    endif ()
    if (DEFINED CXX_OPTIONS)
        target_compile_options(${NAME} INTERFACE "$<$<COMPILE_LANGUAGE:CXX>:${CXX_OPTIONS}>")
    endif ()
    if (DEFINED C_OPTIONS)
        target_compile_options(${NAME} INTERFACE "$<$<COMPILE_LANGUAGE:C>:${C_OPTIONS}>")
    endif ()
    if (DEFINED DEBUG_OPTIONS)
        target_compile_options(${NAME} INTERFACE "$<$<CONFIG:Debug>:${DEBUG_OPTIONS}>")
    endif ()

    # Linker options (take effect on consumers that perform linking)
    if (DEFINED LINKER_OPTIONS)
        target_link_options(${NAME} INTERFACE ${LINKER_OPTIONS})
    endif ()

    # Optional includes to propagate
    if (DEFINED PROJECT_INCLUDE_DIRS OR DEFINED ADDITIONAL_INCLUDE_DIRS)
        target_include_directories(${NAME} INTERFACE
                ${PROJECT_INCLUDE_DIRS}
                ${ADDITIONAL_INCLUDE_DIRS}
        )
    endif ()
endfunction()

# Apply the INTERFACE usage requirements of a build-config interface target
# directly onto one or more concrete targets as PRIVATE properties, without
# creating a link dependency on the interface target itself. This avoids
# leaking the interface target into install(EXPORT ...) sets.
# Usage:
#   art_apply_build_config(<iface-target> <tgt1> [<tgt2> ...])
function(art_apply_build_config IFACE)
    if (NOT TARGET ${IFACE})
        message(FATAL_ERROR "art_apply_build_config: Interface target '${IFACE}' does not exist")
    endif ()
    if (ARGC LESS 2)
        message(FATAL_ERROR "art_apply_build_config: Provide at least one consumer target after the interface target")
    endif ()

    # Collect interface properties from the config target
    get_target_property(_cfg_defs   ${IFACE} INTERFACE_COMPILE_DEFINITIONS)
    get_target_property(_cfg_opts   ${IFACE} INTERFACE_COMPILE_OPTIONS)
    get_target_property(_cfg_incs   ${IFACE} INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(_cfg_links  ${IFACE} INTERFACE_LINK_LIBRARIES)
    get_target_property(_cfg_lopts  ${IFACE} INTERFACE_LINK_OPTIONS)

    foreach(_tgt IN LISTS ARGN)
        if (NOT TARGET ${_tgt})
            continue()
        endif ()
        if (_cfg_defs)
            target_compile_definitions(${_tgt} PRIVATE ${_cfg_defs})
        endif ()
        if (_cfg_opts)
            target_compile_options(${_tgt} PRIVATE ${_cfg_opts})
        endif ()
        if (_cfg_incs)
            target_include_directories(${_tgt} PRIVATE ${_cfg_incs})
        endif ()
        if (_cfg_lopts)
            # Link options only affect linkable targets; harmless for OBJECT libs
            target_link_options(${_tgt} PRIVATE ${_cfg_lopts})
        endif ()
        # Do NOT copy INTERFACE_LINK_LIBRARIES to avoid re-introducing dependencies
    endforeach()
endfunction()
