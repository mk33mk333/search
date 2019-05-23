cmake_minimum_required(VERSION 3.10)
project(search C)

set(CMAKE_C_STANDARD 99)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)  

INCLUDE_DIRECTORIES(  
    include  
) 

add_executable(search src/main.c src/commander.c src/puzzle.c)