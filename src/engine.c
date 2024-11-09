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
	size_t n_moves = moves->size;
	dai32_free(moves);
	if (n_moves == 0) {
		if (is_king_in_check(engine->board)) {
			return 1000000;
		} else {
			return 0;
		}
	}
	size_t half_move_clock = n_moves_since_last_pawn_or_capture_move(engine->board);
	if (half_move_clock >= 50) {
		return 0;
	}
	for (size_t i = 0; i < 64; ++i) {
		Piece *piece = engine->board->pieces + i;
		int fac = (piece->data != 0) * (2  * (piece->color != engine->board->to_move) - 1);
		eval += fac * piece_vals[piece->type];
	}
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

// ================================

void test_move_sequence(void) {
    do {} while (0);
}

void test_engine(void) {
	test_wrapper(test_move_sequence);
}
