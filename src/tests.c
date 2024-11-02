#include "tests.h"
#include "common.h"
#include "piece.h"
#include "board.h"
#include "move.h"
#include "generate.h"
#include "pgn.h"

#include <stdio.h>

int test_depth = 0;

const char *c = NULL;
size_t line_n = 1;
size_t col_n = 1;

void test(void) {
    test_wrapper(test_common);
    test_wrapper(test_piece);
    test_wrapper(test_board);
    test_wrapper(test_move);
    test_wrapper(test_generate);
    test_wrapper(test_pgn);

    arena_reset(&arena);
}
