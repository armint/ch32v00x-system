cmake_minimum_required(VERSION 3.20)
# Extract git revision
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_version.cmake")

project(ch32v00x-system
        LANGUAGES C CXX ASM
        VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}"
)


# Provides CMAKE_INSTALL_*DIR for the install() calls below.
include(GNUInstallDirs)

# Add our CMAKE modules folder
list(APPEND CMAKE_MODULE_PATH ../../cmake)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_version.cmake")

# Compiler settings
# The public interface needs C++17 only (cxx_std_17 below). The consumer
# selects the standard: CMAKE_CXX_STANDARD of the superproject applies here
# too. Only a standalone build of this library, without a value from the
# command line, defaults to C++23.
if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR AND NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 23)
endif ()
set(CXX_DIALECT gnu)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_DIALECT gnu)

# Chip family of the build. It selects the directory src/families/<family>.
# The device macro (CH32V003, CH32V20x_D8W, ...) is still set by the consumer.
set(CH32_FAMILY "CH32V00X" CACHE STRING "CH32 family to build for (CH32V00X, CH32V20X, CH32V30X)")
set_property(CACHE CH32_FAMILY PROPERTY STRINGS CH32V00X CH32V20X CH32V30X)
if (NOT CH32_FAMILY MATCHES "^(CH32V00X|CH32V20X|CH32V30X)$")
    message(FATAL_ERROR "Unknown CH32_FAMILY '${CH32_FAMILY}'")
endif ()
string(TOLOWER ${CH32_FAMILY} FAMILY_LOWER)
set(FAMILY_DIR "${PROJECT_SOURCE_DIR}/src/families/${FAMILY_LOWER}")
if (NOT IS_DIRECTORY "${FAMILY_DIR}")
    message(FATAL_ERROR "No source directory for ${CH32_FAMILY}: ${FAMILY_DIR}")
endif ()

# Gather all project sources, then take the family files out and add back
# only the files of the selected family.
file(GLOB_RECURSE PROJECT_SOURCES
        "${PROJECT_SOURCE_DIR}/src/*.c"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
)
file(GLOB_RECURSE ALL_FAMILY_SOURCES
        "${PROJECT_SOURCE_DIR}/src/families/*.c"
        "${PROJECT_SOURCE_DIR}/src/families/*.cpp"
)
list(REMOVE_ITEM PROJECT_SOURCES ${ALL_FAMILY_SOURCES})
file(GLOB_RECURSE ACTIVE_FAMILY_SOURCES
        "${FAMILY_DIR}/*.c"
        "${FAMILY_DIR}/*.cpp"
)
list(APPEND PROJECT_SOURCES ${ACTIVE_FAMILY_SOURCES})

# Shared drivers that are not ported to CH32V30x yet. Remove an entry when its
# port exists in src/families/ch32v30x/.
if (CH32_FAMILY STREQUAL "CH32V30X")
    foreach (unported peripherals/Dma.cpp peripherals/Exti.cpp peripherals/Flash.cpp
                      peripherals/I2C.cpp peripherals/Timer.cpp peripherals/Usart.cpp)
        list(REMOVE_ITEM PROJECT_SOURCES "${PROJECT_SOURCE_DIR}/src/${unported}")
    endforeach ()
endif ()


file(GLOB_RECURSE PROJECT_HEADERS
        "src/*.h"
        "src/*.hpp")

file(GLOB PROJECT_INCLUDE_DIRS
        ${CMAKE_CURRENT_BINARY_DIR}
        src
)

# Detect if we are crosscompiling, or compiling for our native system.
IF (CMAKE_CROSSCOMPILING)
    IF (NOT CMAKE_BUILD_TYPE)
        SET(CMAKE_BUILD_TYPE Release)
        MESSAGE(STATUS "CMAKE_BUILD_TYPE not set, using default: " ${CMAKE_BUILD_TYPE})
    ENDIF ()

    MESSAGE(STATUS "Building ch32v00x-system version " ${FIRMWARE_VERSION} " (" ${CMAKE_BUILD_TYPE} ")")

else ()
endif ()

add_library(ch32v00x-system OBJECT ${PROJECT_SOURCES})

# The family directory is flat in the installed package, like src/.
target_include_directories(
        ch32v00x-system
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${FAMILY_DIR}>
        )

target_compile_features(ch32v00x-system PUBLIC cxx_std_17)

# If a central build configuration target exists in the superproject,
# link it so this OBJECT library is compiled with the same flags/defines.
if (TARGET art-build-config)
    # Only affect this target's compilation in the build tree; do not appear in installed export
    target_link_libraries(ch32v00x-system PRIVATE $<BUILD_INTERFACE:art-build-config>)
endif ()

# Keep the build interface free of linked targets, so consumers get no extra link dependencies.
set_property(TARGET ch32v00x-system PROPERTY INTERFACE_LINK_LIBRARIES "")

# hw::span (src/utils/Span.h) is std::span from C++20 on. Below C++20 the
# standard library has no std::span, so link the span-compat header library
# and make hw::span the span_compat::span. This link comes after the line
# above, because consumers that include Flash.h also need the span-compat
# headers. The C++20 and later build links nothing extra.
# A superproject that already provides span-compat::span-compat keeps its
# target, so the whole build has one span type. The installed package needs
# an installed span-compat package: cmake/ch32v00x-systemConfig.cmake.in
# finds it and links it to the imported target.
# An unset CMAKE_CXX_STANDARD (the consumer sets the standard elsewhere, or
# uses the compiler default) also takes the span-compat branch: with
# SPAN_COMPAT_USE_STD_SPAN=OFF, span-compat is correct at every standard.
if (NOT DEFINED CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 20)
    set(CH32V00X_SYSTEM_USE_SPAN_COMPAT ON)
    if (NOT TARGET span-compat::span-compat)
        # A C++17 build of this library can be part of a C++20 or C++23
        # build. With AUTO, the C++23 translation units would see std::span
        # and the C++17 ones the own span: two span types in one program.
        # OFF gives the own span at every standard. A value that the
        # superproject already set is kept.
        if (NOT DEFINED SPAN_COMPAT_USE_STD_SPAN)
            set(SPAN_COMPAT_USE_STD_SPAN OFF CACHE STRING "Span type: AUTO (by __cpp_lib_span), ON (std::span) or OFF (own)")
        endif ()
        include(FetchContent)
        FetchContent_Declare(
                span_compat
                GIT_REPOSITORY https://github.com/armint/span-compat.git
                GIT_TAG 15b7f429d1ba45d1d5e092d8c0c4d97c0bc4968c
        )
        FetchContent_GetProperties(span_compat)
        if (NOT span_compat_POPULATED)
            FetchContent_MakeAvailable(span_compat)
        endif ()
    endif ()
    target_link_libraries(ch32v00x-system PUBLIC $<BUILD_INTERFACE:span-compat::span-compat>)
    target_compile_definitions(ch32v00x-system PUBLIC CH32V00X_SYSTEM_USE_SPAN_COMPAT)
else ()
    set(CH32V00X_SYSTEM_USE_SPAN_COMPAT OFF)
endif ()

# The installed package is header-only. The library is an OBJECT library, and
# install(TARGETS) does not install its objects, so no compiled code is exported.
# Consumers build the sources with add_subdirectory(). See README.md.
# Headers: the shared tree, then the headers of the selected family. Both go
# flat into the include directory, as the build-tree include path expects.
install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
        PATTERN "families" EXCLUDE)
install(DIRECTORY ${FAMILY_DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h")

# Generate a Config file so consumers can `find_package(ch32v00x-system)`
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ch32v00x-systemConfigVersion.cmake
        VERSION ${GIT_VERSION}
        COMPATIBILITY SameMajorVersion)

configure_package_config_file(
        ${CMAKE_CURRENT_LIST_DIR}/cmake/ch32v00x-systemConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/ch32v00x-systemConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ch32v00x-system
        PATH_VARS CMAKE_INSTALL_INCLUDEDIR)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ch32v00x-systemConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ch32v00x-systemConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ch32v00x-system)

