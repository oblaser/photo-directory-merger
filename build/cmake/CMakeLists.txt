
# author        Oliver Blaser
# date          11.02.2023
# copyright     GNU GPLv3 - Copyright (c) 2023 Oliver Blaser

cmake_minimum_required(VERSION 3.13)

project(phodime)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)



#
# SDK
#

# omw
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../sdk/omw/build/cmake/libomw.a/ ${CMAKE_CURRENT_LIST_DIR}/../../sdk/omw/build/cmake/libomw.a/)
include_directories(../../sdk/omw/include)



#
# the application
#

set(BINNAME phodime)

if(_DEBUG)
    add_definitions(-D_DEBUG)
endif()

include_directories(../../src/)

set(SOURCES
../../src/application/cliarg.cpp
../../src/application/processor.cpp
../../src/middleware/util.cpp
../../src/main.cpp
)



add_executable(${BINNAME} ${SOURCES})

target_link_libraries(${BINNAME} omw)

target_compile_options(${BINNAME} PRIVATE
    -Wall
    -Werror=format
    -Werror=overflow
    -Werror=return-type
    -Wdouble-promotion

    $<$<COMPILE_LANGUAGE:C>:
        -Werror=discarded-qualifiers
        -Werror=implicit
        -Werror=incompatible-pointer-types
        -Werror=int-conversion
        -Werror=pointer-sign
    >

    $<$<COMPILE_LANGUAGE:CXX>:
        -Werror=reorder
        -Werror=switch
        -Wno-psabi
    >
)
