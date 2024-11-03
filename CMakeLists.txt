cmake_minimum_required(VERSION 3.5)
project (chess)

set(CMAKE_C_STANDARD 17)

include_directories(${chess_SOURCE_DIR}/include)
cmake_minimum_required(VERSION 3.5)
project(chess)

set(CMAKE_C_STANDARD 17)

include_directories(${chess_SOURCE_DIR}/include)

add_library(chess_lib STATIC
    src/board.c
    src/common.c
    src/generate.c
    src/move.c
    src/pgn.c
    src/piece.c
    src/utils.c
)

set(HEADER_FILES
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

target_sources(chess_lib PRIVATE ${HEADER_FILES})

add_executable(engine
    src/main.c
)

add_executable(tests
    src/tests.c
)

target_link_libraries(engine PRIVATE chess_lib)
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
set_compiler_flags(engine)
set_compiler_flags(tests)

set(RESOURCE_FILES
    ${CMAKE_SOURCE_DIR}/res/tests/game-1.pgn
)

source_group("Resource Files" FILES ${RESOURCE_FILES})
target_sources(chess_lib PRIVATE ${RESOURCE_FILES})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT tests)
