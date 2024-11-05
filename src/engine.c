#include "engine.h"
#include "generate.h"
#include "common.h"
#include "utils.h"

Move best_move(Board *board) {
	DAi32* moves = dai32_create();
	generate_moves(board, moves);
	
	size_t random_move_idx = rand_lim(moves->size);
	Move move = move_data_create(moves->data[random_move_idx]);

	dai32_free(moves);
	return move;
}
