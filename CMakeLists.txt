cmake_minimum_required(VERSION 3.10)
project(stateless-sync-protocol LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Werror)

add_executable(encoder encoder.cpp)
add_executable(decoder decoder.cpp
        encoder.cpp
        encoder.h
        decoder.cpp
        decoder.h)
target_include_directories(encoder PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(decoder PRIVATE ${PROJECT_SOURCE_DIR})
