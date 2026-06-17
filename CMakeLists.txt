cmake_minimum_required(VERSION 4.3.2)
project(malphine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(main 
    src/main.cpp
    src/board/board.cpp
    src/board/fen.cpp
    src/move/move.cpp
    src/movegen/movegen.cpp
    src/tools/magics.cpp
    src/evaluate/evaluate.cpp
    src/search/search.cpp
    src/uci/uci.cpp
)
add_executable(magics_generator
    src/tools/magics_generator.cpp
    src/tools/magics.cpp
)

target_include_directories(magics_generator PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/src)
