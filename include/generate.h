#pragma once

#include <stdbool.h>

#include "board.h"

void generate_attacked(Board *board, Color color, uint8_t attacked[64], size_t *king_idx);

bool king_in_check_base(Board *board, Color color, size_t *checked_by);

bool king_in_check(Board *board);

void validate_and_push_move(Board *board, DAi32 *moves, Move move);

void generate_pawn_moves(Board *board, size_t idx, DAi32 *moves);

void generate_bishop_moves(Board *board, size_t idx, DAi32 *moves);

void generate_rook_moves(Board *board, size_t idx, DAi32 *moves);

void generate_queen_moves(Board *board, size_t idx, DAi32 *moves);

void generate_knight_moves(Board *board, size_t idx, DAi32 *moves);

void generate_king_moves(Board *board, size_t idx, DAi32 *moves);

void generate_moves(Board *board, DAi32 *moves);

// ==================================

void test_generate_initial_moves(void);

void test_generate(void);
