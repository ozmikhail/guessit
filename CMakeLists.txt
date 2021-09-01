cmake_minimum_required(VERSION 3.20)
project(guessit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(core)

add_executable(guessit main.cpp)
target_link_libraries(guessit PRIVATE guessit_core)
target_compile_options(guessit PRIVATE -Wall -Wextra -Wpedantic)
