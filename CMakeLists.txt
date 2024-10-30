cmake_minimum_required(VERSION 3.5)
project (chess)

# set(CMAKE_C_COMPILER "gcc-13")
set(CMAKE_C_STANDARD 17)

include_directories(${chess_SOURCE_DIR}/include)
add_subdirectory(src)

file(GLOB SOURCES "src/*.c")
add_executable(chess ${SOURCES}
        include/board.h
        include/common.h
        include/generate.h
        include/move.h
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