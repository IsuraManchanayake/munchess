#include <assert.h>

#include "result.h"
#include "common.h"
#include "piece.h"
#include "tests.h"
#include "generate.h"

Result result_create(ResultType type, Color winner) {
    Result result = (Result) {0};
    result.type = type;
    result.winner = winner;
    assert(result.winner == NO_COLOR 
        || result.type == MATE 
        || result.type == RESIGNATION 
        || result.type == TIMEOUT);
    return result;
}

Result evaluate_result(Board *board) {
    uint8_t attacked[64] = {0};
    size_t king_idx = 64;

    DAi32 *moves = dai32_create();
    generate_moves(board, moves);
    if (moves->size == 0) {
        generate_attacked(board, board->to_move, attacked, &king_idx);
        assert(king_idx < 64);
        if (attacked[king_idx] > 0) {
            return result_create(MATE, op_color(board->to_move));
        } else {
            return result_create(DRAW, NO_COLOR);
        }
    }
    return result_create(NO_RESULT, NO_COLOR);
}

// ======================

void test_result_size(void) {
    Result result = result_create(MATE, BLACK);
    assert(sizeof(result) == sizeof(result.data));
}

void test_result(void) {
    test_wrapper(test_result_size);
}
