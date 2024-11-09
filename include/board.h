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
    uint64_t king_bb[2];

    uint64_t attacked;
    bool attacked_evaluated;

    size_t time_to_generate_last_move_us;
} Board;

bool idx_is_safe(size_t idx);

bool yx_is_safe(size_t y, size_t x);

Piece board_safe_at(const Board *board, size_t idx);

void clear_square(Board *board, size_t idx);

void set_piece_with(Board *board, size_t idx, Color color, PieceType type);

void set_piece(Board *board, size_t idx, Piece);

void board_reset(Board *board);

Board *board_create(void);

bool is_attacked(Board *board, size_t idx);

void set_attacked(Board *board, size_t idx);

void place_initial_pieces(Board *board);

size_t get_king_idx(Board *board, Color color);

int move_direction(Color color);

void apply_move_base(Board *board, Move move, bool invalidate_attacked);

void apply_move(Board *board, Move move);

void undo_last_move_base(Board *board, bool invalidate_attacked);

void undo_last_move(Board *board);

size_t n_moves_since_last_pawn_or_capture_move(Board* board);

char *board_to_fen(Board *board, DA *da);

Move san_notation_to_move(const char *notation, Board *board);

Move uci_notation_to_move(const char* move_str, Board* board);

const char *fen_to_board(const char *fen, Board *board);

char *board_buf_write(Board *board, DA *da);

void print_board(Board *board);

void print_fen(Board* board);

// ===========================================

void test_board_initial(void);
void test_board_display(void);
void test_apply_move(void);
void test_undo_last_move(void);
void test_board_to_fen(void);
void test_fen_to_board(void);
void test_san_notation_to_move(void);
void test_uci_notation_to_move(void);
void test_board(void);
