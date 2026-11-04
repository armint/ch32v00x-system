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
set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CXX_DIALECT gnu)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_DIALECT gnu)

# Chip family of the build. It selects the directory src/families/<family>.
# The device macro (CH32V003, CH32V20x_D8W, ...) is still set by the consumer.
set(CH32_FAMILY "CH32V00X" CACHE STRING "CH32 family to build for (CH32V00X, CH32V20X)")
set_property(CACHE CH32_FAMILY PROPERTY STRINGS CH32V00X CH32V20X)
if (NOT CH32_FAMILY MATCHES "^(CH32V00X|CH32V20X)$")
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
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_features(ch32v00x-system PUBLIC cxx_std_23)

# If a central build configuration target exists in the superproject,
# link it so this OBJECT library is compiled with the same flags/defines.
if (TARGET art-build-config)
    # Only affect this target's compilation in the build tree; do not appear in installed export
    target_link_libraries(ch32v00x-system PRIVATE $<BUILD_INTERFACE:art-build-config>)
endif ()

# Ensure the exported interface does not pull non-exported targets.
set_property(TARGET ch32v00x-system PROPERTY INTERFACE_LINK_LIBRARIES "")

install(TARGETS ch32v00x-system
        EXPORT ch32v00x-system-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
# Headers: the shared tree, then the headers of the selected family. Both go
# flat into the include directory, as the build-tree include path expects.
install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
        PATTERN "families" EXCLUDE)
install(DIRECTORY ${FAMILY_DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h")

install(EXPORT ch32v00x-system-targets
        NAMESPACE art::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ch32v00x-system)

# Generate a Config file so consumers can `find_package(ch32v00x-system)`
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ch32v00x-systemConfigVersion.cmake
        VERSION ${GIT_VERSION}
        COMPATIBILITY SameMajorVersion)

configure_package_config_file(
        ${CMAKE_CURRENT_LIST_DIR}/cmake/ch32v00x-systemConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/ch32v00x-systemConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ch32v00x-system)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ch32v00x-systemConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ch32v00x-systemConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ch32v00x-system)

