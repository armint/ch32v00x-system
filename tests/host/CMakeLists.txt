# Host unit tests for the parts of ch32v00x-system that do not need hardware.
# A native build, separate from the firmware build:
#   cmake -S tests/host -B build-host && cmake --build build-host && ctest --test-dir build-host
cmake_minimum_required(VERSION 3.20)
project(ch32v00x-system-host-tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(_src ${CMAKE_CURRENT_SOURCE_DIR}/../../src)

enable_testing()

# GpioPin once per family. The stubs-<family> directory gives <wch_device.h>
# with the GPIO register layout of that family.
foreach(_family v00x v20x v30x)
    if(_family STREQUAL "v00x")
        set(_pins 8)
    else()
        set(_pins 16)
    endif()
    add_executable(test-gpio-${_family}
            test_gpio.cpp
            ${_src}/peripherals/GpioPin.cpp)
    target_include_directories(test-gpio-${_family} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/stubs-${_family}
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${_src})
    target_compile_definitions(test-gpio-${_family} PRIVATE GPIO_PINS_PER_PORT=${_pins})
    target_compile_options(test-gpio-${_family} PRIVATE -Wall -Wextra -Werror)
    add_test(NAME gpio-${_family} COMMAND test-gpio-${_family})
endforeach()
