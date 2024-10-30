#include "tests.h"
#include "common.h"
#include "piece.h"
#include "board.h"
#include "move.h"
#include "generate.h"

#include <stdio.h>


void test(void) {
    test_wrapper(test_common);
    test_wrapper(test_piece);
    test_wrapper(test_board);
    test_wrapper(test_move);
    test_wrapper(test_generate);

    arena_reset(&arena);
}
