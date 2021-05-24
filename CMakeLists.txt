cmake_minimum_required(VERSION 3.17)
project(chip8_emulator)

set(CMAKE_CXX_STANDARD 14)

add_library(chip8 src/chip8.cpp)
target_include_directories(chip8 PUBLIC inc)

add_executable(chip8_interp app/main.cpp)
target_link_libraries(chip8_interp chip8)

#add_executable(chip8_emulator main.cpp chip8.h chip8.cpp)
#target_compile_options(chip8_emulator PRIVATE -Wall -Wextra -Wpedantic)
