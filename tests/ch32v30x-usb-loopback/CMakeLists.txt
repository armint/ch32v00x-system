cmake_minimum_required(VERSION 3.20)

# Hardware test of hw::UsbDevice on CH32V307. A standalone project: it adds
# the library from the repository root. See README.md in this directory.
project(ch32v30x-usb-loopback LANGUAGES C CXX ASM)

# C++20 gives std::span, so the library fetches no span-compat.
set(CMAKE_CXX_STANDARD 20)
set(CH32_FAMILY "CH32V30X" CACHE STRING "" FORCE)
add_subdirectory(../.. ch32v00x-system)

add_executable(usb-loopback.elf main.cpp)
target_link_libraries(usb-loopback.elf PRIVATE ch32v00x-system)

set(LDSCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/../../ldscripts")
target_link_options(usb-loopback.elf PRIVATE
        -L${LDSCRIPTS} -Tmem-ch32v307.ld -Tsections.ld -Tnoheap.ld
        -nostartfiles -Wl,-gc-sections -Wl,-Map,usb-loopback.map
        --specs=nano.specs --specs=nosys.specs)

add_custom_command(TARGET usb-loopback.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:usb-loopback.elf> usb-loopback.hex
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:usb-loopback.elf>)
