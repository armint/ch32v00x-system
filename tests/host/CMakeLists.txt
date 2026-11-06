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

# hw::Timer once per family. The stubs-<family> directory gives <wch_device.h>
# with the TIM register layout of that family.
foreach(_family v00x v20x v30x)
    add_executable(test-timer-${_family}
            test_timer.cpp
            ${_src}/peripherals/Timer.cpp)
    target_include_directories(test-timer-${_family} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/stubs-${_family}
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${_src})
    target_compile_options(test-timer-${_family} PRIVATE -Wall -Wextra -Werror)
    add_test(NAME timer-${_family} COMMAND test-timer-${_family})
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
        ${_src}/peripherals/Usart.cpp
        ${_src}/peripherals/Rcc.cpp)
target_include_directories(test-usart-v30x PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/stubs-usart-v30x
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${_src}/families/ch32v30x
        ${_src})
target_compile_options(test-usart-v30x PRIVATE -Wall -Wextra -Werror)
add_test(NAME usart-v30x COMMAND test-usart-v30x)

# The CH32V30x ADC driver. The stubs-adc-v30x directory gives <wch_device.h>
# with the ADC and RCC registers as host variables.
add_executable(test-adc-v30x
        test_adc_v30x.cpp
        ${_src}/families/ch32v30x/Adc.cpp
        ${_src}/peripherals/Rcc.cpp)
target_include_directories(test-adc-v30x PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/stubs-adc-v30x
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${_src}/families/ch32v30x
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

# The shared Flash driver with the CH32V30x registers. The stubs-flash-v30x directory gives
# <wch_device.h> with models of the FLASH registers and RCC_CFGR0 as host variables.
add_executable(test-flash-v30x
        test_flash_v30x.cpp
        ${_src}/peripherals/Flash.cpp)
target_include_directories(test-flash-v30x PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/stubs-flash-v30x
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${_src})
target_compile_definitions(test-flash-v30x PRIVATE CH32V30x_D8C)
target_compile_options(test-flash-v30x PRIVATE -Wall -Wextra -Werror)
add_test(NAME flash-v30x COMMAND test-flash-v30x)

# The CH32V30x Rcc driver: clock enables, resets and bus clocks. The stubs-rcc-v30x directory gives
# <wch_device.h> with the RCC registers as host variables. The family directory gives Rcc_V30x.h.
add_executable(test-rcc-v30x
        test_rcc_v30x.cpp
        ${_src}/peripherals/Rcc.cpp)
target_include_directories(test-rcc-v30x PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/stubs-rcc-v30x
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${_src}/families/ch32v30x
        ${_src})
target_compile_options(test-rcc-v30x PRIVATE -Wall -Wextra -Werror)
add_test(NAME rcc-v30x COMMAND test-rcc-v30x)

# The Dma driver with the CH32V30x registers: channel addresses and interrupt flags of DMA1 and DMA2.
# The stubs-dma-v30x directory gives <wch_device.h> with the DMA types.
add_executable(test-dma-v30x
        test_dma_v30x.cpp
        ${_src}/peripherals/Dma.cpp)
target_include_directories(test-dma-v30x PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/stubs-dma-v30x
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${_src}/peripherals)
target_compile_options(test-dma-v30x PRIVATE -Wall -Wextra -Werror)
add_test(NAME dma-v30x COMMAND test-dma-v30x)

# The Spi driver with the CH32V30X bits. The stubs-spi-v30x directory gives <wch_device.h>
# with the SPI and RCC registers as host variables.
add_executable(test-spi-v30x
        test_spi_v30x.cpp
        ${_src}/peripherals/Spi.cpp
        ${_src}/peripherals/Rcc.cpp)
target_include_directories(test-spi-v30x PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/stubs-spi-v30x
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${_src}/families/ch32v30x
        ${_src})
target_compile_options(test-spi-v30x PRIVATE -Wall -Wextra -Werror)
add_test(NAME spi-v30x COMMAND test-spi-v30x)
