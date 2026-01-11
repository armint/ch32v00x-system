cmake_minimum_required(VERSION 3.20)
# Extract git revision
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_version.cmake")

project(ch32v00x-system
        LANGUAGES C CXX ASM
        VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}"
)


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

file(GLOB_RECURSE PROJECT_SOURCES
        "${PROJECT_SOURCE_DIR}/src/*.c"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
)


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

add_library(ch32v00x-system OBJECT ${PROJECT_SOURCES} ${STM32_SOURCES})

target_include_directories(
        ch32v00x-system
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)


target_include_directories(ch32v00x-system
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Export CMSIS headers to dependents as part of the public interface
target_include_directories(ch32v00x-system
        PUBLIC
        $<BUILD_INTERFACE:${cmsis_device_l0_SOURCE_DIR}/Include>
        $<BUILD_INTERFACE:${cmsis54st_SOURCE_DIR}/Core/Include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/cmsis_device_l0/Include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/cmsis_core/Core/Include>
)

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
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install the CMSIS headers so consumers can include them from the installed package
install(DIRECTORY ${cmsis_device_l0_SOURCE_DIR}/Include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cmsis_device_l0/Include)
install(DIRECTORY ${cmsis54st_SOURCE_DIR}/Core/Include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cmsis_core/Core/Include)

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

