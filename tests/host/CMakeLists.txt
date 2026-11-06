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
# with the GPIO register layout of that family. stubs-gpio gives a critical
# section that does nothing.
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
            ${CMAKE_CURRENT_SOURCE_DIR}/stubs-gpio
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${_src})
    target_compile_definitions(test-gpio-${_family} PRIVATE GPIO_PINS_PER_PORT=${_pins})
    target_compile_options(test-gpio-${_family} PRIVATE -Wall -Wextra -Werror)
    add_test(NAME gpio-${_family} COMMAND test-gpio-${_family})
endforeach()

# The CH32V307 USBHS device driver. The stubs-usb-v307 directory gives
# <wch_device.h> with the USBHS and RCC registers as host variables.
add_executable(test-usb-v307
        test_usb_v307.cpp
        ${_src}/families/ch32v30x/UsbDevice.cpp)
target_include_directories(test-usb-v307 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/stubs-usb-v307
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${_src})
target_compile_options(test-usb-v307 PRIVATE -Wall -Wextra -Werror)
add_test(NAME usb-v307 COMMAND test-usb-v307)

# The baud rate of the CH32V30x Usart with APB prescalers. The stubs-usart-v30x
# directory gives <wch_device.h> with RCC_CFGR0 and the USART registers as host variables.
add_executable(test-usart-v30x
        test_usart_v30x.cpp
        ${_src}/peripherals/Usart.cpp)
target_include_directories(test-usart-v30x PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/stubs-usart-v30x
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${_src})
target_compile_options(test-usart-v30x PRIVATE -Wall -Wextra -Werror)
add_test(NAME usart-v30x COMMAND test-usart-v30x)

# The CH32V30x ADC driver. The stubs-adc-v30x directory gives <wch_device.h>
# with the ADC and RCC registers as host variables.
add_executable(test-adc-v30x
        test_adc_v30x.cpp
        ${_src}/families/ch32v30x/Adc_V30x.cpp)
target_include_directories(test-adc-v30x PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/stubs-adc-v30x
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${_src})
target_compile_definitions(test-adc-v30x PRIVATE CH32V30x_D8C)
target_compile_options(test-adc-v30x PRIVATE -Wall -Wextra -Werror)
add_test(NAME adc-v30x COMMAND test-adc-v30x)

# The CH32V30X Afio, Exti and GpioInterrupt. The stubs-exti-v30x directory
# gives <wch_device.h> with the AFIO, EXTI and RCC registers as host
# variables. The real utils/Interrupt.h is used: the test defines the
# functions that it declares.
add_executable(test-exti-afio-v30x
        test_exti_afio_v30x.cpp
        ${_src}/families/ch32v30x/Afio.cpp
        ${_src}/families/ch32v30x/Exti_V30x.cpp
        ${_src}/families/ch32v30x/GpioInterrupt.cpp)
target_include_directories(test-exti-afio-v30x PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/stubs-exti-v30x
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${_src}/families/ch32v30x
        ${_src})
target_compile_options(test-exti-afio-v30x PRIVATE -Wall -Wextra -Werror)
add_test(NAME exti-afio-v30x COMMAND test-exti-afio-v30x)

# The SysTick time base of CH32V30x (free-running counter, compare moved by the handler). The
# stubs-systick-v30x directory gives <wch_device.h> with the SysTick registers as a host variable.
add_executable(test-systick-v30x
        test_systick_v30x.cpp
        ${_src}/Timekeeping.cpp)
target_include_directories(test-systick-v30x PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/stubs-systick-v30x
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${_src})
target_compile_definitions(test-systick-v30x PRIVATE CH32V30x_D8C)
target_compile_options(test-systick-v30x PRIVATE -Wall -Wextra -Werror)
add_test(NAME systick-v30x COMMAND test-systick-v30x)
