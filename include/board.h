#pragma once

#include "common.h"
#include "piece.h"
#include "move.h"

typedef struct Board {
    Piece pieces[64];
    DAi32 *moves;
    size_t first_king_move[2];
    size_t first_king_rook_move[2];
    size_t first_queen_rook_move[2];
    size_t last_pawn_move[2];
    size_t last_capture_move[2];
    size_t initial_half_move_clock;
    size_t half_move_counter;
    Color to_move;
} Board;

bool idx_is_safe(size_t idx);

bool yx_is_safe(size_t y, size_t x);

Piece board_safe_at(const Board *board, size_t idx);

void set_piece_null(Piece *piece);

Board *board_create(void);

void place_initial_pieces(Board *board);

int move_direction(Color color);

void apply_move(Board *board, Move move);

void set_square_empty(Board *board, size_t idx);

void undo_last_move(Board *board);

char *board_to_fen(Board *board, DA *da);

Move notation_to_move(const char *notation, Board *board);

void fen_to_board(char *fen, Board *board);

char *board_buf_write(Board *board, DA *da);

// ===========================================

void test_board_initial(void);
void test_board_display(void);
void test_apply_move(void);
void test_undo_last_move(void);
void test_board_to_fen(void);
void test_fen_to_board(void);
void test_notation_to_move(void);
void test_board(void);
