cmake_minimum_required(VERSION 3.15)

project(BM25-Multilingual VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)

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
    ${CMAKE_SOURCE_DIR}/third_party/snowball/include
    ${CMAKE_SOURCE_DIR}/third_party/cppjieba/include

)

target_link_libraries(BM25-Multilingual PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/snowball/libstemmer.a
)