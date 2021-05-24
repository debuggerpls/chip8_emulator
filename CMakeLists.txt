cmake_minimum_required(VERSION 3.17)
project(chip8_emulator)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(3rdparty/SDL2-2.0.14)

add_library(chip8 src/chip8.cpp src/display.cpp)
target_include_directories(chip8 PUBLIC inc)
target_link_libraries(chip8 PUBLIC SDL2main SDL2-static)

add_executable(chip8_interp app/main.cpp)
target_link_libraries(chip8_interp chip8)
