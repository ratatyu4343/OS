cmake_minimum_required(VERSION 3.16)
project(Lab1 C)

set(CMAKE_C_STANDARD 11)

include_directories("./library/include")
include_directories("./include")

set(
        SOURCE_FILES
        main.c
        library/trialfuncs.c
        library/include/trialfuncs.h
        library/include/compfuncs.h
        src/manager.c
        include/manager.h
        src/fgfunctions.c
        include/fgfunctions.h
        include/ffunction.h
        include/gfunction.h
        src/ffunction.c
        src/gfunction.c
        include/bfunction.h src/bfunction.c)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Llibrary/ -llab1")

add_executable(Lab1 ${SOURCE_FILES})
