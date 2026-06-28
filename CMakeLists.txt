cmake_minimum_required(VERSION 3.14)
project(chml C)

set(CMAKE_C_STANDARD 11)

if(NOT DEFINED CHML_BUILD_VERSION)
    set(CHML_BUILD_VERSION 0)
endif()

option(CHML_NATIVE "Enable native hardware optimizations" OFF)
option(CHML_BUILD_TESTS "Build chml tests" OFF)

set(CHML_TEST_EXTERNAL_ARTIFACTS OFF)
if(CHML_BUILD_TESTS AND CHML_TEST_SHARED_LIBRARY)
    set(CHML_TEST_EXTERNAL_ARTIFACTS ON)
endif()

if(NOT MSVC)
    add_compile_options(-O3 -Wall -Wextra -Werror)
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        add_compile_options(-Wno-unused-command-line-argument)
    endif()
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

if(NOT CHML_TEST_EXTERNAL_ARTIFACTS)
    add_executable(chml
        src/chml.c
        src/libchml.c
    )

    target_include_directories(chml PRIVATE src)

    if(UNIX AND NOT ANDROID AND NOT APPLE)
        target_link_libraries(chml pthread m)
    endif()

    add_library(chml_static STATIC src/libchml.c)
    set_target_properties(chml_static PROPERTIES OUTPUT_NAME "chml")
    target_include_directories(chml_static PRIVATE src)

    add_library(chml_shared SHARED src/libchml.c)
    set_target_properties(chml_shared PROPERTIES OUTPUT_NAME "chml")
    set_target_properties(chml_shared PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
    target_include_directories(chml_shared PRIVATE src)

    if(UNIX AND NOT ANDROID AND NOT APPLE)
        target_link_libraries(chml_shared pthread m)
    endif()
endif()

if(CHML_BUILD_TESTS)
    enable_testing()

    if(CHML_TEST_SHARED_LIBRARY)
        add_library(chml_test_shared SHARED IMPORTED)
        set_target_properties(chml_test_shared PROPERTIES
            IMPORTED_LOCATION "${CHML_TEST_SHARED_LIBRARY}"
        )
        if(CHML_TEST_IMPORT_LIBRARY)
            set_target_properties(chml_test_shared PROPERTIES
                IMPORTED_IMPLIB "${CHML_TEST_IMPORT_LIBRARY}"
            )
        endif()
    else()
        set(CHML_TEST_SHARED_TARGET chml_shared)
    endif()

    if(TARGET chml_test_shared)
        set(CHML_TEST_SHARED_TARGET chml_test_shared)
    endif()

    add_executable(chml_contract_test src/test.c)
    target_include_directories(chml_contract_test PRIVATE src)
    target_link_libraries(chml_contract_test PRIVATE ${CHML_TEST_SHARED_TARGET})

    if(UNIX AND NOT ANDROID AND NOT APPLE)
        target_link_libraries(chml_contract_test PRIVATE m)
    endif()

    add_custom_command(TARGET chml_contract_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${CHML_TEST_SHARED_TARGET}>
            $<TARGET_FILE_DIR:chml_contract_test>
    )

    add_test(NAME library-version COMMAND chml_contract_test version)
    add_test(NAME library-options-default COMMAND chml_contract_test options-default)
    add_test(NAME library-options-load-env COMMAND chml_contract_test options-load-env)
    add_test(NAME library-options-free COMMAND chml_contract_test options-free)
    add_test(NAME library-open-close COMMAND chml_contract_test open-close)
    add_test(NAME library-stop COMMAND chml_contract_test stop)
    add_test(NAME library-get-role COMMAND chml_contract_test get-role)
    add_test(NAME library-format-from-name COMMAND chml_contract_test format-from-name)
    add_test(NAME library-render COMMAND chml_contract_test render)
    add_test(NAME library-on-signal COMMAND chml_contract_test on-signal)
    add_test(NAME library-raise-signal COMMAND chml_contract_test raise-signal)
    add_test(NAME library-listen-signals COMMAND chml_contract_test listen-signals)
    add_test(NAME library-listen-signal COMMAND chml_contract_test listen-signal)
    add_test(NAME library-signal-listener COMMAND chml_contract_test signal-listener)
    add_test(NAME library-multictx COMMAND chml_contract_test multictx)
endif()
