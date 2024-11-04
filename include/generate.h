#pragma once

#include "board.h"


void generate_attacked(Board *board, Color color, uint8_t attacked[64], size_t *king_idx);

size_t king_in_check(Board *board, size_t king_idx);

void validate_and_push_move(Board *board, DAi32 *moves, Move move, size_t king_idx);

void generate_pawn_moves(Board *board, size_t idx, DAi32 *moves, size_t king_idx);

void generate_bishop_moves(Board *board, size_t idx, DAi32 *moves, size_t king_idx);

void generate_rook_moves(Board *board, size_t idx, DAi32 *moves, size_t king_idx);

void generate_queen_moves(Board *board, size_t idx, DAi32 *moves, size_t king_idx);

void generate_knight_moves(Board *board, size_t idx, DAi32 *moves, size_t king_idx);

void generate_king_moves(Board *board, size_t idx, DAi32 *moves);

void generate_moves(Board *board, DAi32 *moves);

// ==================================

void test_generate_initial_moves(void);

void test_generate(void);
