cmake_minimum_required(VERSION 3.9)
project(pdf_sigil)

set(CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")

# header files
include_directories(include)


file(GLOB LIB_SRC "lib/*.c")
set (TEST_SRC "test/test.c")

# build both static and shared library
add_library(pdf-sigil_static STATIC ${LIB_SRC})
add_library(pdf-sigil_shared SHARED ${LIB_SRC})

# build selftest executable
add_executable(selftest ${TEST_SRC})
target_link_libraries(selftest pdf-sigil_static)

# running selftest
add_custom_target(run_tests ALL
    COMMAND selftest
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(run_tests_verbose
    COMMAND selftest --verbose
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(run_tests_quiet
    COMMAND selftest --quiet
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)