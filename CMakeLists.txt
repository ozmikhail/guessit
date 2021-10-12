cmake_minimum_required(VERSION 3.20)
project(guessit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(core)

add_executable(guessit main.cpp)
target_link_libraries(guessit PRIVATE guessit_core)
target_compile_options(guessit PRIVATE -Wall -Wextra -Wpedantic)

find_library(READLINE_LIB readline
    HINTS /opt/homebrew/opt/readline/lib /usr/local/opt/readline/lib)
find_path(READLINE_INCLUDE_DIR readline/readline.h
    HINTS /opt/homebrew/opt/readline/include /usr/local/opt/readline/include)

if(READLINE_LIB AND READLINE_INCLUDE_DIR)
    target_include_directories(guessit PRIVATE ${READLINE_INCLUDE_DIR})
    target_link_libraries(guessit PRIVATE ${READLINE_LIB})
    target_compile_definitions(guessit PRIVATE HAVE_READLINE=1)
    message(STATUS "readline: ${READLINE_LIB}")
else()
    message(STATUS "readline not found — tab completion disabled (brew install readline)")
endif()
