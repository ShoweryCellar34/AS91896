# Require at least CMake version 3.20 and define the project
cmake_minimum_required(VERSION 3.20)
project(AS91896 LANGUAGES CXX VERSION 1.0.0)

# Force the use of C++20 with no compiler extensions
set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# Force the use of C11 with no compiler extensions for raylib to compile
set(CMAKE_C_STANDARD          11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS        OFF)

# Set debug build type if no build type is set 
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Fetch raylib
include(FetchContent)
FetchContent_Declare(raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        6.0
    GIT_PROGRESS   TRUE
    GIT_SHALLOW    TRUE
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(raylib)

# Create maze executable
add_executable(maze-maker-and-solver main.cpp)

# Link raylib
target_link_libraries(maze-maker-and-solver PRIVATE raylib)