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
    srand(time(NULL));
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

#include <string.h>
Move to_move(char* uci_move_str, Board *board) {
    size_t from = COORD_TO_IDX(uci_move_str);
    size_t to = COORD_TO_IDX(uci_move_str + 2);
    Piece* piece = &board->pieces[from];
    uint8_t move_type_mask = NORMAL;
    PieceType promoted_type = NONE;
    PieceType captured_type = board->pieces[to].type;

    size_t len = strlen(uci_move_str);
	debugzu(len);
    if (!is_piece_null(board->pieces[to])) {
        move_type_mask |= CAPTURE;
    }
    int dir = move_direction(board->to_move);
    if (piece->type == PAWN) {
        if (board->moves->size > 0) {
            Move last_move = move_data_create(board->moves->data[board->moves->size - 1]);
            if (last_move.piece_type == PAWN) {
                size_t last_move_from_y = IDX_Y(last_move.from);
                size_t last_move_to_y = IDX_Y(last_move.to);
                size_t last_move_x = IDX_X(last_move.from);
                if (last_move_to_y + 2 * dir == last_move_from_y 
                    && YX_TO_IDX(last_move_from_y - dir, last_move_x) == to) {
                    move_type_mask |= EN_PASSANT;
                }
            }
        }
        // Promotion
        if (len == 5) {
			move_type_mask |= PROMOTION;
            promoted_type = char_to_piece_type(uci_move_str[4]);
            assert(IDX_Y(to) == (7 + 7 * dir) / 2);
        }
    }
    if (piece->type == KING) {
        if (from - to == 2 || to - from == 2) {
            move_type_mask |= CASTLE;
            assert(IDX_Y(from) == IDX_Y(to));
            assert(IDX_Y(from) == (7 - 7 * dir) / 2);
        }
    }
    return move_create(
        *piece,
        from,
        to,
        move_type_mask,
        promoted_type,
        captured_type
    );
}

void test_move_sequence(void) {
	char *seq[] = {
		"e2e3",
"a7a6",
"h2h4",
"b7b5",
"g2g3",
"b8c6",
"h1h2",
"e7e5",
"d1f3",
"a8a7",
"f3f6",
"c6e7",
"c2c4",
"b5c4",
"f6d6",
"g7g6",
"d6c5",
"h7h6",
"c5c7",
"h8h7",
"g1f3",
"d7d6",
"b1a3",
"h7g7",
"c7a7",
"d8c7",
"d2d4",
"e5e4",
"f3e5",
"a6a5",
"e5c4",
"e7c6"
	};
	Board *board = board_create();
	place_initial_pieces(board);
	for (size_t i = 0; i < sizeof(seq) / sizeof(seq[0]); ++i) {
		Move move = to_move(seq[i], board);
		apply_move(board, move);

		DA *board_da = da_create();
		char *board_buf = board_buf_write(board, board_da);
		printf("%s\n", board_buf);
		da_free(board_da);
	}

	DAi32 *moves = dai32_create();
	generate_moves(board, moves);
	debugzu(moves->size);
	for (size_t i = 0; i < moves->size; ++i) {
		Move move = move_data_create(moves->data[i]);

		DA *move_da = da_create();
		char *move_buf = move_buf_write(move, move_da);
		printf("%s\n", move_buf);
		da_free(move_da);
	}

	(void) seq;
}

void test_engine(void) {
	test_wrapper(test_move_sequence);
}
