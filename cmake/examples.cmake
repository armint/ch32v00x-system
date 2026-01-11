
function(add_example name ADDITIONAL_SOURCES)
    file(GLOB_RECURSE EXAMPLE_SOURCES
            "*.c"
            "*.cpp"
            "*.S"
    )

    add_executable(${name}.elf
            ${EXAMPLE_SOURCES}
            ${SYSTEM_SOURCES}
            ${ADDITIONAL_SOURCES}
    )


    # Central build-config interface: include and attach to ensure all deps build with the same flags/defines
    include("${CMAKE_SOURCE_DIR}/cmake/BuildConfig.cmake")
    art_setup_build_config(art-build-config)
    target_link_libraries(${name}.elf PUBLIC art-build-config)

    target_link_libraries(${name}.elf PRIVATE etl::etl)
    target_link_libraries(${name}.elf PUBLIC art::core)

    target_compile_options(${name}.elf PUBLIC "${COMMON_OPTIONS}")

    target_link_options(${name}.elf PUBLIC -Tmem-ch32v003.ld)

    target_link_options(${name}.elf PUBLIC -flto -Wl,-gc-sections -L${linkscript_dir} -Tsections.ld -Tnoheap.ld -nostartfiles -Wl,-Map,${name}.map --specs=nano.specs --specs=nosys.specs)
    target_link_options(${name}.elf PUBLIC ${LINKER_OPTIONS})
    target_link_options(${name}.elf PUBLIC ${COMMON_OPTIONS})

    # For GCC 12, we disable the linker warning about executable LOAD segments
    #target_link_options(${name}.elf PRIVATE "LINKER:--no-warn-rwx-segments")

    ADD_DUMP_TARGET(${name}.elf)
    PRINT_SIZE_OF_TARGETS(${name}.elf)

    set(HEX_FILE ${name}-${BOARD}-${CMAKE_BUILD_TYPE}-${FIRMWARE_VERSION}.hex)
    set(BIN_FILE ${name}-${BOARD}-${CMAKE_BUILD_TYPE}-${FIRMWARE_VERSION}.bin)

    add_custom_command(TARGET ${name}.elf POST_BUILD
            COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${name}.elf> ${name}.hex
            COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${name}.elf> ${name}.bin
            COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${name}.elf> ${HEX_FILE}
            COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${name}.elf> ${BIN_FILE}
            COMMENT "Building ${HEX_FILE} \nBuilding ${BIN_FILE}")

endfunction()
