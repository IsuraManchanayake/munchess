#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#include "engine.h"
#include "board.h"
#include "generate.h"
#include "common.h"
#include "move.h"
#include "piece.h"
#include "tests.h"
#include "utils.h"

Engine *engine_create(void) {
    Engine *engine = (Engine *) arena_allocate(&arena, sizeof(Engine));
    engine->state = ENGINE_NOT_STARTED;
	engine->board = NULL;
	engine->moves = dai32_create();
    return engine;
}

void engine_start(Engine *engine) {
    if (engine->state == ENGINE_NOT_STARTED) {
        srand((unsigned)getpid());
		// srand(time(NULL));
        // Load engine related data
        engine->state = ENGINE_READY;
    } else {
        // assert(0);
    }
}

int64_t evaluate_board(Engine *engine) {
	static const int piece_vals[] = {
		[PAWN] = 100,
		[KNIGHT] = 300,
		[BISHOP] = 350,
		[ROOK] = 500,
		[QUEEN] = 900,
		[KING] = 0,
		[NONE] = 0,
	};
	int64_t eval = 0;
	DAi32 *moves = dai32_create();
	generate_moves(engine->board, moves);
	if (moves->size == 0) {
		volatile size_t idx = king_idx(engine->board, engine->board->to_move);
		volatile size_t x = king_in_check(engine->board, idx);
		if (x > 0) {
			return 1000000;
		} else {
			return 0;
		}
	}
	for (size_t i = 0; i < 64; ++i) {
		Piece *piece = engine->board->pieces + i;
		int fac = (piece->data != 0) * (2  * (piece->color != engine->board->to_move) - 1);
		eval += fac * piece_vals[piece->type];
	}
	dai32_free(moves);
	return eval;
}

Move engine_best_move(Engine *engine, Board *board) {
	engine->board = board;
    if (engine->state != ENGINE_READY) {
        return (Move) {0};
    }
    engine->state = ENGINE_BUSY;
	engine->moves->size = 0;
    generate_moves(engine->board, engine->moves);

	DAi32 *best_moves = dai32_create();
	int64_t best_eval = INT_MIN;
	for (size_t i = 0; i < engine->moves->size; ++i) {
		Move move = move_data_create(engine->moves->data[i]);
		apply_move(engine->board, move);
		int eval = evaluate_board(engine);

		// DA *da = da_create();
		// char *buf = move_buf_write(move, da);
		// printf("%s %d\n", buf, eval);
		// da_free(da);

		if (eval == best_eval) {
			dai32_push(best_moves, move.data);
		} else if (eval > best_eval) {
			best_moves->size = 0;
			best_eval = eval;
			dai32_push(best_moves, move.data);
		}

		undo_last_move(engine->board);
	}
    
    size_t random_move_idx = rand_lim(best_moves->size);
    Move move = move_data_create(best_moves->data[random_move_idx]);
	dai32_free(best_moves);

    engine->state = ENGINE_READY;
    return move;
}

/*

position startpos moves b2b4 b7b5 g1h3 a7a6 c1a3 h7h5 h3f4 h8h7 f4h5 h7h5 g2g3 h5h2 h1h2 g8f6 c2c4 b5c4 d1c2 c4c3 c2c3 g7g5 c3f6 e7f6 h2g2 f8b4 a3b4 a6a5 b4a5 a8a5 d2d3 a5a2 a1a2 e8e7 e1d2 c8a6 a2a6 b8a6 d2e1 g5g4 f2f3 g4f3 e2f3 d8c8 g2d2 c8f8 f3f4 e7d8 e1f2 d7d6 f1h3 f8g8 d2e2 g8g3 f2g3 a6c5 b1c3 c5d3 e2c2 d3f4 g3f4 d8e8 h3g2 e8d8 f4g4 c7c6 g2c6 d8c8 c3a2 c8b8 c2c4 d6d5 c6d5 b8a7 d5f7 a7a8 g4g3 a8b7 g3h2 b7a6 f7e8 a6a7 h2h3 a7b6 e8h5 b6a6 h5g6 a6b7 g6d3 f6f5 d3f5 b7a8 h3g2 a8b7 g2f2 b7a6 f5e6 a6a7 c4c2 a7b6 f2f1 b6a6 c2g2 a6b7 g2g3 b7c7 f1e1 c7b7 a2c3 b7b8 c3e4 b8a8 e1f2 a8a7 g3b3 a7a8 f2e2 a8a7 e4d6 a7a8

*/

// ================================

void test_move_sequence(void) {
    do {} while (0);
}

void test_engine(void) {
	test_wrapper(test_move_sequence);
}
