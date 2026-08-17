# Set required cmake version and define project
cmake_minimum_required(VERSION 3.15)
project(ExampleProjectName LANGUAGES C VERSION 0.0.0)

# Set the C Standard Settings 
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Define all source files and add executable
set(SOURCES
    source/main.c
)
add_executable(${PROJECT_NAME} ${SOURCES})

# Set compile flags and enable static linking
set(BUILD_SHARED_LIBS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic
        $<$<CONFIG:Debug>:-g3 -ggdb -Og -fno-omit-frame-pointer -Wshadow $<$<PLATFORM_ID:Windows>:-fsanitize=address,undefined>>
        $<$<CONFIG:Release>:-O3 -march=native -flto -DNDEBUG -D_FORTIFY_SOURCE=3 -fstack-protector-strong -fstack-clash-protection -fPIE -pie -Wl,-z,relro,-z,now -Werror -s>
    )

    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address,undefined>
        $<$<CONFIG:Release>:-static>
    )
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /analyze
        $<$<CONFIG:Debug>:/Od /Zi /RTC1 /MDd /GS /JMC /fsanitize=address /D_DEBUG>
        $<$<CONFIG:Release>:/O2 /GL /Zi /GS /DNDEBUG>
    )

    target_link_options(${PROJECT_NAME} PRIVATE /INCREMENTAL:NO
        $<$<CONFIG:Release>:/LTCG /OPT:REF,ICF>
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "_CRT_SECURE_NO_WARNINGS"
    )

    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Include fetch content 
# include(FetchContent)

# Example dependancy
# FetchContent_Declare(
#     example
#     GIT_REPOSITORY https://example.com/example.git
#     GIT_TAG        exampleTag
# )
# FetchContent_MakeAvailable(example)

# Link dependencies to executable
# target_link_libraries(${PROJECT_NAME} PRIVATE)

# Set the include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/headers>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
)

# Generate a header with cmake project infomation
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/headers/projectData.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/projectData.h"
)

# Delete old resources and copy new ones
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/resources" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
    COMMENT "Removing old resources and copying new ones for ${CMAKE_BUILD_TYPE} build..."
)
