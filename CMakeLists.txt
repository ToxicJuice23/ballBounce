cmake_minimum_required(VERSION 3.10)
project(ballBounce C)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O3")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")  # Debug build

# SDL2 configuration
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Include source directory for headers
include_directories(src)

# Define source files
set(SRC_FILES src/ball_sdl.c src/string_functions.c src/main.c)
set(TEST_FILES tests/check_ball.c ${SRC_FILES})
set(HEADERS src/main.h)

# Target: ballBounce (optimized)
add_executable(ballBounce ${SRC_FILES} ${HEADERS})
target_link_libraries(ballBounce ${SDL2_LIBRARIES} m)

# Target: test_runner (unit tests with Check library)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
find_package(Check REQUIRED)
add_executable(test_runner ${TEST_FILES})
target_link_libraries(test_runner ${CHECK_LIBRARIES} ${SDL2_LIBRARIES} m)

# Debug target: ball_debug (Separate build type for debugging)
add_executable(ball_debug ${SRC_FILES} ${HEADERS})
target_link_libraries(ball_debug ${SDL2_LIBRARIES} m)
set_target_properties(ball_debug PROPERTIES COMPILE_FLAGS "-g")

# Custom build directories
set_target_properties(ballBounce PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build")
set_target_properties(test_runner PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set_target_properties(ball_debug PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build_debug")

# Custom commands for building each target
add_custom_target(debug_build DEPENDS ball_debug)
add_custom_target(all_build DEPENDS ballBounce test_runner)

# Clean target
add_custom_target(clean_all
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/build ${CMAKE_BINARY_DIR}/build_debug test_runner
)