cmake_minimum_required(VERSION 3.10)

project(BM25-Multilingual VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(BM25-Multilingual)

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
)

target_sources( BM25-Multilingual PRIVATE
    ${SOURCES}
)


target_include_directories(
    BM25-Multilingual
    PRIVATE
    src
)

