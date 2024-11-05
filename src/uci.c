#include <stdio.h>
#include <assert.h>

#include "uci.h"
#include "board.h"
#include "engine.h"
#include "defs.h"

void move_to_uci(Move move, char* uci) {
	char from[] = IDX_TO_COORD(move.from);
	char to[] = IDX_TO_COORD(move.to);
	uci[0] = from[0];
	uci[1] = from[1];
	uci[2] = to[0];
	uci[3] = to[1];
	uci[4] = '\0';
}

Move uci_to_move(char* uci, Board *board) {
	size_t from = COORD_TO_IDX(uci);
	size_t to = COORD_TO_IDX(uci + 2);
	Piece* piece = &board->pieces[from];
	uint8_t move_type_mask = NORMAL;
	PieceType promoted_type = NONE;
	PieceType captured_type = board->pieces[to].type;

	size_t len = strlen(uci);
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
			promoted_type = char_to_piece_type(uci[4]);
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

void start_uci(void) {
	Board *board = board_create();

	char input[256];
	while (true) {
		fgets(input, 256, stdin);
		if (strncmp(input, "uci", 3) == 0) {
			printf("id name %s\n", ENGINE_NAME);
			printf("id author %s\n", ENGINE_AUTHOR);
			printf("uciok\n");
		}
		else if (strncmp(input, "isready", 7) == 0) {
			printf("readyok\n");
		}
		else if (strncmp(input, "ucinewgame", 10) == 0) {
			board = board_create();
		}
		else if (strncmp(input, "position startpos", 17) == 0) {
			//board = board_create();
			//char* token = strtok(input, " ");
			//while (token != NULL) {
			//	if (strncmp(token, "moves", 5) == 0) {
			//		token = strtok(NULL, " ");
			//		while (token != NULL) {
			//			Move move = move_data_create(token);
			//			board_make_move(&board, move);
			//			token = strtok(NULL, " ");
			//		}
			//	}
			//	token = strtok(NULL, " ");
			//}
		}
		else if (strncmp(input, "go", 2) == 0) {
			Move move = best_move(&board);
			apply_move(board, move);
			char uci_move[6];
			move_to_uci(move, uci_move);
			printf("bestmove %s\n", uci_move);
		}
		else if (strncmp(input, "quit", 4) == 0) {
			break;
		}
	}
}
