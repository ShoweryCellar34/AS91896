cmake_minimum_required(VERSION 3.20)
project(AS91896 LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

set(CMAKE_C_STANDARD          11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS        OFF)

# Set debug build type if no build type is set 
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Export the compiler commands for LSP programs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Check if Inter Procedural Optimization is supported
include(CheckIPOSupported)
check_ipo_supported()

# Ensure necessary tooling is present
find_program(CCACHE_PROGRAM     ccache               REQUIRED)
find_program(CLANG_TIDY_PROGRAM clang-tidy           REQUIRED)
find_program(IWYU_PROGRAM       include-what-you-use REQUIRED)
find_program(CPPCHECK_PROGRAM   cppcheck             REQUIRED)
find_program(LCOV_PROGRAM       lcov                 REQUIRED)
find_program(GENHTML_PROGRAM    genhtml              REQUIRED)

find_package(Doxygen REQUIRED)

# Enable caching of unchanged objects during building
set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})

# Get dependencies
include(FetchContent)

# Define a dependency helper function
function(fetch_dependency dependencyName gitRepo gitTag)
    FetchContent_Declare(${dependencyName}
        GIT_REPOSITORY ${gitRepo}
        GIT_TAG        ${gitTag}
        GIT_PROGRESS   TRUE
        GIT_SHALLOW    TRUE
        EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(${dependencyName})
endfunction()

# Configure options for dependencies

# Declare dependencies
fetch_dependency(raylib https://github.com/raysan5/raylib.git dbce4e4bc87481de311516c25395fd1b121b3ea1)

# Set target name
set(TARGET_NAME "mazeTest")

# Create target
add_executable(${TARGET_NAME}
    source/main.cpp
)

# Set target include directories
target_include_directories(${TARGET_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/headers>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
    ${stb_SOURCE_DIR}
)

# Set target link libraries
target_link_libraries(${TARGET_NAME}
    PRIVATE raylib
)

target_compile_definitions(${TARGET_NAME} PRIVATE DEBUG_BUILD=$<CONFIG:Debug>)

# Generate configure file for target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/headers/projectData.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/projectData.h"
)

# Enable IPO for target
set_property(TARGET ${TARGET_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

# Enable all warnings and treat them as errors
target_compile_options(${TARGET_NAME} PRIVATE
    -Wall -Wextra -Wpedantic -Werror
    -Wshadow -Wconversion -Wsign-conversion
    -Wcast-align -Wnull-dereference -Wdouble-promotion
    -Wformat=2 -Wstrict-prototypes
    -Wmissing-prototypes # Not very good for macros, disable if causing problems
)
set_target_properties(${TARGET_NAME} PROPERTIES
    C_CLANG_TIDY           "${CLANG_TIDY_PROGRAM}"
    C_INCLUDE_WHAT_YOU_USE "${IWYU_PROGRAM}"
)

# Copy new resources and compile shaders
add_custom_command(
    TARGET  ${TARGET_NAME} POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E echo "Removing old resources from build folder"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:${TARGET_NAME}>/resources"

    COMMAND ${CMAKE_COMMAND} -E echo "Copying new resources to build folder"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/resources" "$<TARGET_FILE_DIR:${TARGET_NAME}>/resources"
)

# Create the spell check target
add_custom_target(cppcheck
    COMMAND           ${CPPCHECK_PROGRAM} --enable=all --std=c++20 --project=${CMAKE_BINARY_DIR}/compile_commands.json
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Create the coverage target
add_custom_target(coverage
    COMMAND           ${LCOV_PROGRAM} --capture --directory . --output-file coverage.info
    COMMAND           ${GENHTML_PROGRAM} coverage.info --output-directory coverage_html
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT           "Report: ${CMAKE_BINARY_DIR}/coverage_html/index.html"
)

# Create the doxygen target
doxygen_add_docs(docs ${CMAKE_SOURCE_DIR}/src COMMENT "Generating docs")
