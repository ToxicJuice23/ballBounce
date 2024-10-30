# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(ballBounce VERSION 1.0)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -g")

# Find SDL2 package (requires SDL2 development files installed)
find_package(SDL2 REQUIRED)

# Include directories for SDL2
include_directories(${SDL2_INCLUDE_DIRS})

# Add the executable
add_executable(ball_sdl main.h
        ball_sdl.c
        string_functions.c
        main.c
)

# Link SDL2 and math library
target_link_libraries(ball_sdl ${SDL2_LIBRARIES} m)