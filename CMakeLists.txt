cmake_minimum_required(VERSION 3.0.0)
project(CppBase64
    VERSION 0.1.0)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

# include(CTest)
# enable_testing()

add_subdirectory(Tests)