#pragma once

#include "piece.h"
#include "utils.h"
#include "core.h"
#include "move.h"

typedef struct Board {
    Piece pieces[64];
    DAi32 *moves;
    size_t first_king_move[2];
    size_t first_king_rook_move[2];
    size_t first_queen_rook_move[2];
    size_t last_pawn_move[2];
} Board;

bool idx_is_safe(size_t idx) {
    return idx < 64;
}

bool yx_is_safe(size_t y, size_t x) {
    return 0 <= y && y < 8 && 0 <= x && x < 8;
}

Piece board_safe_at(const Board *board, size_t idx) {
    if (idx_is_safe(idx)) {
        return board->pieces[idx];
    }
    return (Piece) {.data=0};
}

void set_piece_null(Piece *piece) {
    piece->data = 0;
}

#define ATyx(board, y, x) ((board)->pieces[IDX((y), (x))])
#define ATcoord(board, coord) ((board)->pieces[COORD_TO_IDX(coord)])
#define ATidx(board, pos) ((board)->pieces[pos])
#define ATfr(board, file, rank) ATyx((board), (rank) - 1, simple(file) - 'a')

Board *board_create() {
    Board *board = (Board *)arena_allocate(&arena, sizeof(Board));
    for(size_t i = 0; i < 8; ++i) {
        for(size_t j = 0; j < 8; ++j) {
            set_piece_null(&ATyx(board, i, j));
        }
    }
    board->moves = dai32_create();
    board->first_king_move[WHITE] = 0;
    board->first_king_move[BLACK] = 0;
    board->first_king_rook_move[WHITE] = 0;
    board->first_king_rook_move[BLACK] = 0;
    board->first_queen_rook_move[WHITE] = 0;
    board->first_queen_rook_move[BLACK] = 0;
    board->last_pawn_move[WHITE] = 0;
    board->last_pawn_move[WHITE] = 0;
    return board;
}

void place_initial_pieces(Board *board) {
    for(size_t i = 0; i < 8; ++i) {
        ATyx(board, 1, i) = piece_create(WHITE, PAWN);
        ATyx(board, 6, i) = piece_create(BLACK, PAWN);
    }

    ATcoord(board, "A1") = piece_create(WHITE, ROOK);
    ATcoord(board, "B1") = piece_create(WHITE, KNIGHT);
    ATcoord(board, "C1") = piece_create(WHITE, BISHOP);
    ATcoord(board, "D1") = piece_create(WHITE, QUEEN);
    ATcoord(board, "E1") = piece_create(WHITE, KING);
    ATcoord(board, "F1") = piece_create(WHITE, BISHOP);
    ATcoord(board, "G1") = piece_create(WHITE, KNIGHT);
    ATcoord(board, "H1") = piece_create(WHITE, ROOK);

    ATcoord(board, "A8") = piece_create(BLACK, ROOK);
    ATcoord(board, "B8") = piece_create(BLACK, KNIGHT);
    ATcoord(board, "C8") = piece_create(BLACK, BISHOP);
    ATcoord(board, "D8") = piece_create(BLACK, QUEEN);
    ATcoord(board, "E8") = piece_create(BLACK, KING);
    ATcoord(board, "F8") = piece_create(BLACK, BISHOP);
    ATcoord(board, "G8") = piece_create(BLACK, KNIGHT);
    ATcoord(board, "H8") = piece_create(BLACK, ROOK);
}

int move_direction(Color color) {
    return (int) 2 * (color == WHITE) - 1;
}

void apply_move(Board *board, Move move) {
    size_t move_n = board->moves->size + 1;
    if(move_is_type_of(move, CASTLE)) {
        size_t rank = IDX_TO_RANK(move.from);
        char file = IDX_TO_FILE(move.to);
        if (file == 'g') {
            Piece rook = ATfr(board, 'h', rank);
            assert(rook.type == ROOK);
            ATfr(board, 'f', rank) = rook;
        } else if (file == 'c') {
            Piece rook = ATfr(board, 'a', rank);
            assert(rook.type == ROOK);
            ATfr(board, 'd', rank) = rook;
        } else {
            assert(0);
        }
    } else if (move_is_type_of(move, PROMOTION)) {
        move.piece.type = move.promoted_type;
    } else if (move_is_type_of(move, EN_PASSANT)) {
        int dir = move_direction(move.piece.color);
        size_t to_y = IDX_Y(move.to);
        size_t to_x = IDX_X(move.to);
        set_piece_null(&ATyx(board, to_y - dir, to_x));
    }
    set_piece_null(&ATidx(board, move.from));
    Piece *piece_to = &ATidx(board, move.to);
    piece_to->color = move.piece.color;
    piece_to->type = move.piece.type;
    if (move.piece.type == KING 
        && board->first_king_move[move.piece.color] == 0) {
        board->first_king_move[move.piece.color] = move_n;
    } else if (move.piece.type == ROOK) {
        size_t r = move.piece.color == WHITE ? 1 : 8;
        if (board->first_king_rook_move[move.piece.color] == 0 
            && move.from == FR_TO_IDX('H', r)) {
            board->first_king_rook_move[move.piece.color] = move_n;
        } else if (board->first_queen_rook_move[move.piece.color] == 0 
            && move.from == FR_TO_IDX('A', r)) {
            board->first_queen_rook_move[move.piece.color] = move_n;
        }
    } else if (move.piece.type == PAWN) {
        board->last_pawn_move[move.piece.color] = move_n;
    }
    dai32_push(board->moves, move.data);
}

void undo_last_move(Board *board) {
    size_t move_n = board->moves->size;
    Move move = move_data_create(board->moves->data[move_n - 1]);
    if (move_is_type_of(move, CASTLE)) {
        size_t rank = IDX_TO_RANK(move.from);
        char file = IDX_TO_FILE(move.to);
        if (file == 'g') {
            Piece rook = ATfr(board, 'f', rank);
            assert(rook.type == ROOK);
            ATfr(board, 'h', rank) = rook;
        } else if (file == 'c') {
            Piece rook = ATfr(board, 'd', rank);
            assert(rook.type == ROOK);
            ATfr(board, 'a', rank) = rook;
        } else {
            assert(0);
        }
    } else if (move_is_type_of(move, PROMOTION)) {
        move.piece.type = PAWN;
    } else if (move_is_type_of(move, EN_PASSANT)) {
        int dir = move_direction(move.piece.color);
        unsigned from_y = IDX_Y(move.from);
        size_t to_y = IDX_Y(move.to);
        size_t to_x = IDX_X(move.to);
        ATyx(board, to_y - dir, to_x) = piece_create(op_color(move.piece.color), PAWN);
    }
    ATidx(board, move.from) = piece_create(move.piece.color, move.piece.type);
    if (move_is_type_of(move, CAPTURE)) {
        ATidx(board, move.to) = piece_create(op_color(move.piece.color), move.captured_type);
    } else {
        set_piece_null(&ATidx(board, move.to));
    }
    if (move.piece.type == KING) {
        if (board->first_king_move[move.piece.color] == move_n) {
            board->first_king_move[move.piece.color] = 0;
        }
    } else if (move.piece.type == ROOK) {
        size_t r = move.piece.color == WHITE ? 1 : 8;
        if (move.from == FR_TO_IDX('H', r) && board->first_king_rook_move[move.piece.color] == move_n) {
            board->first_king_rook_move[move.piece.color] = 0;
        } else if (move.from == FR_TO_IDX('A', r) && board->first_queen_rook_move[move.piece.color] == move_n) {
            board->first_queen_rook_move[move.piece.color] = 0;
        }
    }
    (void) dai32_pop(board->moves);
}

char *board_buf_write(Board *board, DA *da) {
    buf_printf(da, "   +");
    for(size_t i = 0; i < 8; ++i) {
        buf_printf(da, "---+");
    }
    for(int i = 7; i >= 0; --i) {
        buf_printf(da, "\n %zu |", i + 1);
        for (size_t j = 0; j < 8; ++j) {
            buf_printf(da, " %c |", piece_repr(ATyx(board, i, j)));
        }
        buf_printf(da, "\n   +");
        for (size_t j = 0; j < 8; ++j) {
            buf_printf(da, "---+");
        }
    }
    buf_printf(da, "\n    ");
    for(size_t i = 0; i < 8; ++i) {
        buf_printf(da, " %c  ", i + 'a');
    }
    buf_printf(da, "\n    ");
    for(size_t i = 0; i < 8; ++i) {
        buf_printf(da, "    ");
    }
    return (char *)da->data;
}

// ===========================================

void test_board_initial() {
    Board *board = board_create();
    place_initial_pieces(board);
    assert(ATyx(board, 0, 0).color == WHITE);
    assert(ATyx(board, 0, 0).type == ROOK);
    assert(ATcoord(board, "C2").type == PAWN);
    assert(ATcoord(board, "C2").color == WHITE);
    assert(ATcoord(board, "E8").type == KING);
    assert(ATcoord(board, "E8").color == BLACK);
    assert(is_piece_null(ATcoord(board, "C6")));
}

void test_board_display() {
    Board *board = board_create();
    place_initial_pieces(board);
    DA *da = da_create();
    char *repr = board_buf_write(board, da);
    // da_free(da, true);
    // printf("%s\n", repr);
}

void test_move_apply() {
    Board *board = board_create();
    place_initial_pieces(board);
    Move move_1 = move_create(ATcoord(board, "E2"), COORD_TO_IDX("E2"), COORD_TO_IDX("E4"), NORMAL, NONE);
    apply_move(board, move_1);
    Move move_2 = move_create(ATcoord(board, "E7"), COORD_TO_IDX("E7"), COORD_TO_IDX("E5"), NORMAL, NONE);
    apply_move(board, move_2);
    Move move_3 = move_create(ATcoord(board, "G1"), COORD_TO_IDX("G1"), COORD_TO_IDX("F3"), NORMAL, NONE);
    apply_move(board, move_3);
    Move move_4 = move_create(ATcoord(board, "B8"), COORD_TO_IDX("B8"), COORD_TO_IDX("C6"), NORMAL, NONE);
    apply_move(board, move_4);
    Move move_5 = move_create(ATcoord(board, "F3"), COORD_TO_IDX("F3"), COORD_TO_IDX("E5"), CAPTURE, NONE);
    apply_move(board, move_5);
    Move move_6 = move_create(ATcoord(board, "C6"), COORD_TO_IDX("C6"), COORD_TO_IDX("E5"), CAPTURE, NONE);
    apply_move(board, move_6);

    DA *da = da_create();
    char *repr = board_buf_write(board, da);
    // printf("%s\n", repr);
    const char expected[] = 
"   +---+---+---+---+---+---+---+---+\n"
" 8 | r |   | b | q | k | b | n | r |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 7 | p | p | p | p |   | p | p | p |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 6 |   |   |   |   |   |   |   |   |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 5 |   |   |   |   | n |   |   |   |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 4 |   |   |   |   | P |   |   |   |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 3 |   |   |   |   |   |   |   |   |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 2 | P | P | P | P |   | P | P | P |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 1 | R | N | B | Q | K | B |   | R |\n"
"   +---+---+---+---+---+---+---+---+\n"
"     a   b   c   d   e   f   g   h  \n"
"                                    ";
    assert(strcmp(repr, expected) == 0);
}

void test_board() {
    test_wrapper(test_board_initial);
    test_wrapper(test_board_display);
    test_wrapper(test_move_apply);
}