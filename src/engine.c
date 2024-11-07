#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "engine.h"
#include "board.h"
#include "generate.h"
#include "common.h"
#include "move.h"
#include "tests.h"
#include "utils.h"

Engine *engine_create(void) {
    Engine *engine = (Engine *) arena_allocate(&arena, sizeof(Engine));
    engine->state = ENGINE_NOT_STARTED;
    return engine;
}

void engine_start(Engine *engine) {
    if (engine->state == ENGINE_NOT_STARTED) {
        srand((unsigned)getpid());
        // Load engine related data
        engine->state = ENGINE_READY;
    } else {
        // assert(0);
    }
}

Move engine_best_move(Engine *engine, Board *board) {
    if (engine->state != ENGINE_READY) {
        return (Move) {0};
    }
    engine->state = ENGINE_BUSY;
    DAi32 *moves = dai32_create();
    generate_moves(board, moves);
    
    size_t random_move_idx = rand_lim(moves->size);
    Move move = move_data_create(moves->data[random_move_idx]);

    dai32_free(moves);
    engine->state = ENGINE_READY;
    return move;
}

// ================================

void test_move_sequence(void) {
    do {} while (0);
}

void test_engine(void) {
	test_wrapper(test_move_sequence);
}
