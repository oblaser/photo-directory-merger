
# author        Oliver Blaser
# date          11.02.2023
# copyright     GNU GPLv3 - Copyright (c) 2023 Oliver Blaser

cmake_minimum_required(VERSION 3.13)

project(phodime)

set(EXE phodime)

#add_compile_definitions(_DEBUG)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

include_directories(../../src/)

set(SOURCES
../../src/application/cliarg.cpp
../../src/application/processor.cpp
../../src/middleware/util.cpp
../../src/main.cpp
)

include_directories(../../sdk/omw/include)
link_directories(../../sdk/omw/lib)

add_executable(${EXE} ${SOURCES})

target_link_libraries(${EXE} omw)
