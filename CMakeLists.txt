
cmake_minimum_required(VERSION 2.7)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#set(LIBS_PATH "${PROJECT_SOURCE_DIR}/libs")

include_directories ("${PROJECT_SOURCE_DIR}/includes")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -D_FILE_OFFSET_BITS=64")

add_definitions( -D__DEBUG__ )

add_subdirectory("src")
