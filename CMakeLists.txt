cmake_minimum_required(VERSION 3.18)

project(BM25-Multilingual VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_PYTHON_BINDINGS "Build the pybind11 Python extension" OFF)

file(GLOB_RECURSE CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/bm25.cpp")

add_library(search_core STATIC ${CORE_SOURCES})
set_target_properties(search_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(search_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/snowball/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cppjieba/include
)
target_compile_definitions(search_core PRIVATE
    JIEBA_DICT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/third_party/cppjieba/dict")
    
target_link_libraries(search_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/snowball/libstemmer.a
)

add_executable(BM25-Multilingual src/bm25.cpp)
target_link_libraries(BM25-Multilingual PRIVATE search_core)

if(BUILD_PYTHON_BINDINGS)
    set(PYBIND11_FINDPYTHON ON)
    find_package(Python 3.8 REQUIRED COMPONENTS Interpreter Development.Module)
    find_package(pybind11 CONFIG REQUIRED)

    pybind11_add_module(multilingual_search bindings/bindings.cpp)
    target_link_libraries(multilingual_search PRIVATE search_core)

    set_target_properties(multilingual_search PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/python")
endif()