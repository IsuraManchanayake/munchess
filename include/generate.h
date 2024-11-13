#pragma once

#include <stdbool.h>

#include "board.h"

void generate_attacked(Board *board, Color color, uint8_t attacked[64], size_t *king_idx);

bool is_king_in_check_base(Board *board, Color color, size_t *checked_by);

bool is_king_in_check(Board *board);

bool validate_and_push_move(Board *board, DAi32 *moves, Move move);

bool generate_pawn_moves(Board *board, size_t idx, DAi32 *moves, bool return_on_found);

bool generate_bishop_moves(Board *board, size_t idx, DAi32 *moves, bool return_on_found);

bool generate_rook_moves(Board *board, size_t idx, DAi32 *moves, bool return_on_found);

bool generate_queen_moves(Board *board, size_t idx, DAi32 *moves, bool return_on_found);

bool generate_knight_moves(Board *board, size_t idx, DAi32 *moves, bool return_on_found);

bool generate_king_moves(Board *board, size_t idx, DAi32 *moves, bool return_on_found);

void generate_moves(Board *board, DAi32 *moves);

// ==================================

void test_generate_initial_moves(void);

void test_generate(void);
