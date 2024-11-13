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

#define NEG_INF -10000000LL
#define POS_INF 10000000LL

Engine *engine_create(on_score_event_f on_score) {
    Engine *engine = (Engine *) arena_allocate(&arena, sizeof(Engine));
    engine->state = ENGINE_NOT_STARTED;
	engine->board = NULL;
	engine->moves = dai32_create();
    engine->on_score = on_score;
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

typedef struct MoveH {
    uint32_t move_data;
    int32_t score;
} MoveH;

int cmp_moveh(const void *a, const void *b) {
    const MoveH *arg1 = (const MoveH *) a;
    const MoveH *arg2 = (const MoveH *) b;
    if (arg1->score > arg2->score) {
        return -1;
    } else if (arg1->score < arg2->score) {
        return 1;
    } else {
        return 0;
    }
}

void sort_moves(DAi32 *moves) {
	static const int64_t piece_vals[] = {
		[PAWN] = 100LL,
		[KNIGHT] = 300LL,
		[BISHOP] = 350LL,
		[ROOK] = 500LL,
		[QUEEN] = 900LL,
		[KING] = 0LL,
		[NONE] = 0LL,
	};
    MoveH *movehs = arena_allocate(&arena, sizeof(MoveH) * moves->size);
    for (size_t i = 0; i < moves->size; ++i) {
        Move move = move_data_create(moves->data[i]);
        movehs[i].move_data = moves->data[i];
        movehs[i].score = 0;
        if (move_is_type_of(move, CAPTURE)) {
            movehs[i].score += piece_vals[move.captured_type];
        }
        if (move_is_type_of(move, PROMOTION)) {
            movehs[i].score += piece_vals[move.promoted_type] - piece_vals[PAWN];
        }
    }
    qsort(movehs, moves->size, sizeof(movehs[0]), cmp_moveh);

    for (size_t i = 0; i < moves->size; ++i) {
        moves->data[i] = movehs[i].move_data;
    }
}

int64_t evaluate_board(Engine *engine, size_t n_moves) {
	static const int64_t piece_vals[] = {
		[PAWN] = 100LL,
		[KNIGHT] = 300LL,
		[BISHOP] = 350LL,
		[ROOK] = 500LL,
		[QUEEN] = 900LL,
		[KING] = 0LL,
		[NONE] = 0LL,
	};
	if (n_moves == 0) {
		if (is_king_in_check(engine->board)) {
            //for (size_t i = 0; i < engine->board->moves->size; ++i) {
            //    print_move(move_data_create(engine->board->moves->data[i]));
            //    printf(" ");
            //}
            //printf("\n");
			return -1000000LL;
		} else {
			return 0LL;
		}
	}
	size_t half_move_clock = n_moves_since_last_pawn_or_capture_move(engine->board);
	if (half_move_clock >= 50) {
		return 0LL;
	}
	int64_t eval = 0LL;
	for (size_t i = 0; i < 64; ++i) {
		Piece *piece = engine->board->pieces + i;
		int fac = (piece->data != 0) * (2  * (piece->color == engine->board->to_move) - 1);
		eval += fac * piece_vals[piece->type];
	}
	return eval;
}

int64_t alphabeta(Engine *engine, size_t depth, int64_t alpha, int64_t beta, bool is_root_color) {
    DAi32 *moves = dai32_create();
    generate_moves(engine->board, moves);
    //sort_moves(moves);
    
    if (depth == 0 || moves->size == 0) {
        int64_t eval = evaluate_board(engine, moves->size);
        dai32_free(moves);
        return eval;
    }
    
    int64_t value = NEG_INF;
    
    for (size_t i = 0; i < moves->size; ++i) {
        Move move = move_data_create(moves->data[i]);
        apply_move(engine->board, move);
        
        value = max(value, -alphabeta(engine, depth - 1, -beta, -alpha, !is_root_color));
        
        undo_last_move(engine->board);
        
        alpha = max(alpha, value);
        if (alpha >= beta) {
            break;
        }
    }
    
    dai32_free(moves);
    return value;
}

Move engine_best_move(Engine *engine, Board *board) {
	engine->board = board;
    if (engine->state != ENGINE_READY) {
        return (Move) {0};
    }
    engine->state = ENGINE_BUSY;
	engine->moves->size = 0;
    generate_moves(engine->board, engine->moves);
    //sort_moves(engine->moves);

    DAi32 *best_moves = dai32_create();
    int64_t best_eval = NEG_INF;
    
    int64_t alpha = NEG_INF;
	int64_t beta = POS_INF;
    
    size_t depth = 4;
    
    for (size_t i = 0; i < engine->moves->size; ++i) {
        Move move = move_data_create(engine->moves->data[i]);
        apply_move(engine->board, move);
        
        int64_t eval = -alphabeta(engine, depth - 1, -beta, -alpha, false);
        
		if (eval > best_eval) {
			best_moves->size = 0;
			best_eval = eval;
			dai32_push(best_moves, move.data);
		} else if (eval == best_eval) {
			dai32_push(best_moves, move.data);
		}
        
        undo_last_move(engine->board);
    }
    
    Move best_move = (Move) {0};
    if (best_moves->size > 0) {
        size_t random_move_idx = rand_lim(best_moves->size);
        best_move = move_data_create(best_moves->data[random_move_idx]);
    }

    if (engine->on_score != NULL) {
        engine->on_score(best_move, depth, best_eval);
    }
    
	dai32_free(best_moves);
    engine->state = ENGINE_READY;
    return best_move;
}

// ================================

void test_move_sequence(void) {
    const char *fen = "3rr1k1/1bq2p1p/p5p1/6Pn/2pQ2N1/3BR2P/5P2/6K1 w - - 0 28";
    Board *board = board_create();
    (void) fen_to_board(fen, board);

    Engine *engine = engine_create(NULL);
    engine_start(engine);
    time_t start = time_now();
    engine_best_move(engine, board);
    time_t end = time_now();
    printf("%f ms\n", (end - start) / 1000.0);
    (void) end;
    (void) start;
}

void test_engine(void) {
	test_wrapper(test_move_sequence);
}
