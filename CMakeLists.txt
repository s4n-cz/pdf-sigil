cmake_minimum_required(VERSION 3.9)
project(pdf_sigil)

set(CMAKE_C_STANDARD 11)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic")

# header files
include_directories(include)

file(GLOB LIB_SRC "lib/*.c")
set (TEST_SRC "test/test.c")

# build both static and shared library
add_library(pdfsigil_static STATIC ${LIB_SRC})
add_library(pdfsigil SHARED ${LIB_SRC})

target_link_libraries(pdfsigil_static crypto)
target_link_libraries(pdfsigil crypto)

# build selftest executable
add_executable(selftest ${TEST_SRC})
target_link_libraries(selftest pdfsigil)

#build pdf-sigil - PoC command-line application
add_executable(pdf-sigil src/pdf-sigil.c)
target_link_libraries(pdf-sigil pdfsigil)

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

# generating the documentation with a Doxygen
find_package(Doxygen)
if (DOXYGEN_FOUND)
    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile})

    add_custom_target(doc ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)

else (DOXYGEN_FOUND)
    message("Install Doxygen for generation of the documentation")
endif (DOXYGEN_FOUND)
