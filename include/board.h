#pragma once

#include "piece.h"
#include "utils.h"

typedef struct Board {
    Piece *pieces[64];
} Board;

#define ATyx(board, y, x) (board->pieces[(y) * 8 + (x)])
#define AT(board, coord) (ATyx(board, (coord)[1] - '1', (char)((coord)[0] | 32) - 'a'))

Board *board_create(Arena *arena) {
    Board *board = (Board *)arena_allocate(arena, sizeof(Board));
    for(size_t i = 0; i < 8; ++i) {
        for(size_t j = 0; j < 8; ++j) {
            ATyx(board, i, j) = NULL;
        }
    }
    return board;
}

void place_initial_pieces(Arena *arena, Board *board) {
    for(size_t i = 0; i < 8; ++i) {
        ATyx(board, 1, i) = piece_create(arena, WHITE, PAWN);
        ATyx(board, 6, i) = piece_create(arena, BLACK, PAWN);
    }

    AT(board, "A1") = piece_create(arena, WHITE, ROOK);
    AT(board, "B1") = piece_create(arena, WHITE, KNIGHT);
    AT(board, "C1") = piece_create(arena, WHITE, BISHOP);
    AT(board, "D1") = piece_create(arena, WHITE, QUEEN);
    AT(board, "E1") = piece_create(arena, WHITE, KING);
    AT(board, "F1") = piece_create(arena, WHITE, BISHOP);
    AT(board, "G1") = piece_create(arena, WHITE, KNIGHT);
    AT(board, "H1") = piece_create(arena, WHITE, ROOK);

    AT(board, "A8") = piece_create(arena, BLACK, ROOK);
    AT(board, "B8") = piece_create(arena, BLACK, KNIGHT);
    AT(board, "C8") = piece_create(arena, BLACK, BISHOP);
    AT(board, "D8") = piece_create(arena, BLACK, QUEEN);
    AT(board, "E8") = piece_create(arena, BLACK, KING);
    AT(board, "F8") = piece_create(arena, BLACK, BISHOP);
    AT(board, "G8") = piece_create(arena, BLACK, KNIGHT);
    AT(board, "H8") = piece_create(arena, BLACK, ROOK);
}

char *board_buf_write(Arena *arena, Board *board) {
    DA *da = da_create();
    buf_printf(da, "+");
    for(size_t i = 0; i < 8; ++i) {
        buf_printf(da, "---+");
    }
    for(size_t i = 0; i < 8; ++i) {
        buf_printf(da, "\n|");
        for (size_t j = 0; j < 8; ++j) {
            buf_printf(da, "   |");
        }
        buf_printf(da, "\n|");
        for (size_t j = 0; j < 8; ++j) {
            buf_printf(da, " %c |", piece_repr(ATyx(board, i, j)));
        }
        buf_printf(da, "\n|");
        for (size_t j = 0; j < 8; ++j) {
            buf_printf(da, "   |");
        }
        buf_printf(da, "\n+");
        for (size_t j = 0; j < 8; ++j) {
            buf_printf(da, "---+");
        }
    }
    return (char *)da->data;
}

void test_board_initial() {
    Arena *arena = arena_create();
    Board *board = board_create(arena);
    place_initial_pieces(arena, board);
    assert(ATyx(board, 0, 0)->color == WHITE);
    assert(ATyx(board, 0, 0)->type == ROOK);
    assert(AT(board, "C2")->type == PAWN);
    assert(AT(board, "C2")->color == WHITE);
    assert(AT(board, "E8")->type == KING);
    assert(AT(board, "E8")->color == BLACK);
    assert(AT(board, "C6") == NULL);
    arena_free(arena);
}

void test_board_display() {
    Arena *arena = arena_create();
    Board *board = board_create(arena);
    place_initial_pieces(arena, board);
    char *repr = board_buf_write(arena, board);
//    printf("%s\n", repr);
    arena_free(arena);
}

void test_board() {
    test_wrapper(test_board_initial);
    test_wrapper(test_board_display);
}