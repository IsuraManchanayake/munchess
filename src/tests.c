#include "tests.h"
#include "common.h"
#include "piece.h"
#include "board.h"
#include "move.h"
#include "generate.h"
#include "pgn.h"
#include "engine.h"
#include "result.h"

#ifdef _WIN32
#include <windows.h>
#endif

int test_depth = 0;

void test(void) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    test_wrapper(test_common);
    test_wrapper(test_piece);
    test_wrapper(test_board);
    test_wrapper(test_move);
    test_wrapper(test_generate);
    test_wrapper(test_pgn);
    test_wrapper(test_engine);
    test_wrapper(test_result);

    arena_reset(&arena);
}
