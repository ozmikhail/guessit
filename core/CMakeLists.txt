add_library(guessit_core
    src/gradebook.cpp
    src/grades.cpp
)

target_include_directories(guessit_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_options(guessit_core PRIVATE -Wall -Wextra -Wpedantic)
