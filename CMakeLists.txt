cmake_minimum_required(VERSION 3.14)
project(chml C)

set(CMAKE_C_STANDARD 11)

if(NOT DEFINED CHML_BUILD_VERSION)
    set(CHML_BUILD_VERSION 0)
endif()

option(CHML_NATIVE "Enable native hardware optimizations" OFF)

if(NOT MSVC)
    add_compile_options(-O3 -Wall -Wextra -Werror)
    if(CHML_NATIVE)
        add_compile_options(-march=native)
    endif()
    add_definitions(-D_GNU_SOURCE)
else()
    add_compile_options(/O2 /W3 /WX)
endif()

set_source_files_properties(src/libchml.c PROPERTIES
    COMPILE_DEFINITIONS KC_CHML_BUILD_VERSION=${CHML_BUILD_VERSION}
)

# Main executable
add_executable(chml
    src/chml.c
    src/libchml.c
)

target_include_directories(chml PRIVATE src)

if(UNIX AND NOT ANDROID)
    target_link_libraries(chml pthread m)
endif()

# Static library
add_library(chml_static STATIC src/libchml.c)

set_target_properties(chml_static PROPERTIES OUTPUT_NAME "chml")

target_include_directories(chml_static PRIVATE src)

# Shared library
add_library(chml_shared SHARED src/libchml.c)

set_target_properties(chml_shared PROPERTIES OUTPUT_NAME "chml")

target_include_directories(chml_shared PRIVATE src)

if(UNIX AND NOT ANDROID)
    target_link_libraries(chml_shared pthread m)
endif()
