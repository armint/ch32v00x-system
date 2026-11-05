# CH32V307 test image: links the library with a minimal main() and the linker
# scripts of this repository. Built only on request, see
# CH32V00X_SYSTEM_BUILD_TEST_IMAGE in the top-level CMakeLists.txt.
add_executable(test-image-v307 main.cpp)
target_link_libraries(test-image-v307 PRIVATE ch32v00x-system)
target_link_options(test-image-v307 PRIVATE
    -nostartfiles
    --specs=nano.specs
    --specs=nosys.specs
    -Wl,--gc-sections
    -Wl,--no-warn-rwx-segments
    -Wl,-Map=test-image-v307.map
    -L${PROJECT_SOURCE_DIR}/ldscripts
    -Tmem-ch32v307.ld
    -Tsections.ld
    -Tnoheap.ld)
add_custom_command(TARGET test-image-v307 POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:test-image-v307>)
