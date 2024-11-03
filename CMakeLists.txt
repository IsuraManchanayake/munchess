cmake_minimum_required(VERSION 3.5)
project (chess)

if(NOT MSVC)
  set(CMAKE_C_COMPILER "gcc-13")
endif()
set(CMAKE_C_STANDARD 23)

include_directories(${chess_SOURCE_DIR}/include)
add_subdirectory(src)

file(GLOB SOURCES "src/*.c")
add_executable(chess ${SOURCES}
        include/board.h
        include/common.h
        include/core.h
        include/generate.h
        include/move.h
        include/pgn.h
        include/piece.h
        include/tests.h
        include/utils.h
)

if(MSVC)
  target_compile_options(chess PRIVATE /W4 /WX)
else()
  target_compile_options(chess PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")