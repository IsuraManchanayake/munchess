cmake_minimum_required(VERSION 3.5)
project (chess)

set(CMAKE_C_STANDARD 17)

include_directories(${chess_SOURCE_DIR}/include)
cmake_minimum_required(VERSION 3.5)
project(chess)

if(MSVC)
    set(CMAKE_C_STANDARD 17)
else()
    set(CMAKE_C_STANDARD 23)
endif()

include_directories(${chess_SOURCE_DIR}/include)

add_library(chess_lib STATIC
    src/board.c
    src/common.c
    src/engine.c
    src/generate.c
    src/move.c
    src/parser.c
    src/pgn.c
    src/piece.c
    src/uci.c
    src/utils.c
)

set(HEADER_FILES
    include/board.h
    include/common.h
    include/engine.h
    include/defs.h
    include/generate.h
    include/move.h
    include/parser.h
    include/pgn.h
    include/piece.h
    include/tests.h
    include/uci.h
    include/utils.h
)

target_sources(chess_lib PRIVATE ${HEADER_FILES})

add_executable(munchess
    src/munchess.c
)

add_executable(tests
    src/tests.c
)

target_link_libraries(munchess PRIVATE chess_lib)
target_link_libraries(tests PRIVATE chess_lib)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

function(set_compiler_flags target)
    if(MSVC)
        # Debug flags
        target_compile_options(${target} PRIVATE 
            $<$<CONFIG:Debug>:/Od /Zi /RTC1 /W4>
        )
        
        # Release flags
        target_compile_options(${target} PRIVATE 
            $<$<CONFIG:Release>:/O2 /Oi /GL>
        )
    else() # Linux - GCC/Clang
        # Debug flags
        target_compile_options(${target} PRIVATE 
            $<$<CONFIG:Debug>:-O0 -g3 -Wall -Wextra -Wpedantic -Werror>
        )
        
        # Release flags
        target_compile_options(${target} PRIVATE 
            $<$<CONFIG:Release>:-O3 -march=native -DNDEBUG>
        )
    endif()
endfunction()

set_compiler_flags(chess_lib)
set_compiler_flags(munchess)
set_compiler_flags(tests)

set(RESOURCE_FILES
    ${CMAKE_SOURCE_DIR}/res/tests/game-1.pgn
)

source_group("Resource Files" FILES ${RESOURCE_FILES})
target_sources(chess_lib PRIVATE ${RESOURCE_FILES})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT tests)
