cmake_minimum_required(VERSION 3.9)
project(pdf_sigil)

set(CMAKE_C_STANDARD 11)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic")

# add OpenSSL
#set (OPENSSL_USE_STATIC_LIBS TRUE)
#include(FindOpenSSL)
#find_package(OpenSSL REQUIRED)

# header files
include_directories(include)
#include_directories(${OPENSSL_INCLUDE_DIR})

file(GLOB LIB_SRC "lib/*.c")
set (TEST_SRC "test/test.c")

# build both static and shared library
add_library(pdf-sigil_static STATIC ${LIB_SRC})
add_library(pdf-sigil_shared SHARED ${LIB_SRC})

#target_link_libraries(pdf-sigil_static ${OPENSSL_LIBRARIES} dl pthread)
#target_link_libraries(pdf-sigil_shared ${OPENSSL_LIBRARIES} dl pthread)
target_link_libraries(pdf-sigil_static crypto)
target_link_libraries(pdf-sigil_shared crypto)

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