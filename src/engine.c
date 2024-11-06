#include <assert.h>

#include "engine.h"
#include "generate.h"
#include "common.h"
#include "utils.h"

Engine *engine_create(void) {
    Engine *engine = (Engine *) arena_allocate(&arena, sizeof(Engine));
    engine->state = ENGINE_NOT_STARTED;
    return engine;
}

void engine_start(Engine *engine) {
    if (engine->state == ENGINE_NOT_STARTED) {
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
    DAi32* moves = dai32_create();
    generate_moves(board, moves);
    
    size_t random_move_idx = rand_lim(moves->size);
    Move move = move_data_create(moves->data[random_move_idx]);

    dai32_free(moves);
    engine->state = ENGINE_READY;
    return move;
}

// To test  > 2024-11-06 20:52:46 position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 moves b1c3 b7b6 c3d5 e7e6 f2f4 c7c6 e1f2 d8e7 h2h4 g8f6 d2d3 b8a6 d5c7 a6c7 c1d2 c8b7 e2e4 d7d5 a2a3 e8d8 b2b3 g7g6 d2a5 b6a5 d3d4 f8g7 d1e1 d8d7 f1c4 d5c4 e4e5 c4b3 e1e3 b3c2 e3e4 c2c1Q e4e1 c1e1 
//  > 2024-11-06 20:54:51 position startpos moves g1h3 c7c5 f2f4 d7d5 c2c4 g8f6 d2d4 d5c4 a2a3 d8d4 b1c3 d4d1
