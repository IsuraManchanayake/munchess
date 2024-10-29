#pragma once

#include "piece.h"
#include "core.h"
#include "utils.h"

typedef enum {
    NORMAL=0,
    CAPTURE=1<<0,
    EN_PASSANT=1<<1,
    CASTLE=1<<2,
    PROMOTION=1<<3,
} MoveType;

typedef union Move {
    struct {
        PieceType promoted_type:3;
        unsigned move_type_mask:4;
        unsigned from:6;
        unsigned to:6;
        PieceType captured_type:3;
#ifdef _MSC_VER
    #pragma pack(push, 1)
#endif
        struct {
            Color color:1;
            PieceType type:3;
        }
#ifdef __GNUC__
    __attribute__((packed)) // For GCC and Clang
#endif
#ifdef _MSC_VER
    #pragma pack(pop)
#endif
        piece;
    };
    uint32_t data;
} Move;

/*
00000000000001111111111111111111
00000000000001111111111111111000 - promoted_type
00000000000001111111111110000000 - move_type_mask
00000000000001111110000000000000 - from
00000000000000000000000000000000 - to
00000000000000000000000000000000 - piece.color
00000000000000000000000000000000 - piece.type
*/

bool move_is_type_of(Move move, MoveType type) {
    return move.move_type_mask & type;
}

Move move_create(Piece piece, unsigned from, unsigned to, unsigned move_type_mask, PieceType promoted_type) {
    return (Move) {
        .piece={.color=piece.color, .type=piece.type},
        .move_type_mask=move_type_mask, 
        .from=from, 
        .to=to, 
        .promoted_type=promoted_type
    };
}

Move move_data_create(uint32_t data) {
    return (Move) {.data=data};
}

bool is_move_null(Move move) {
    return move.data == 0;
}

typedef struct Board Board;

char *move_buf_write(Move move, DA *da) {
    if (move_is_type_of(move, CASTLE)) {
        buf_printf(da, "%c ", piece_repr_base(move.piece.color, move.piece.type));
        volatile char f = IDX_TO_FILE(move.to);
        if (IDX_TO_FILE(move.to) == 'g') {
            buf_printf(da, "O-O");
        } else if (IDX_TO_FILE(move.to) == 'c') {
            buf_printf(da, "O-O-O");
        } else {
            assert(0);
        }
    } else {
        const char from[3] = IDX_TO_COORD(move.from);
        const char to[3] = IDX_TO_COORD(move.to);
        buf_printf(da, "%c ", piece_repr_base(move.piece.color, move.piece.type));
        buf_printf(da, from);
        char mid = '-';
        if (move_is_type_of(move, CAPTURE)) {
            mid = 'x';
        } else if (move_is_type_of(move, EN_PASSANT)) {
            mid = 'e';
        }
        buf_printf(da, "%c", mid);
        buf_printf(da, to);
        if (move_is_type_of(move, PROMOTION)) {
            char promoted = piece_type_repr(move.promoted_type);
            promoted = move.piece.color == WHITE ? promoted ^ 32 : promoted;
            buf_printf(da, "=%c", promoted);
        }
    }
    return (char *) da->data;
}

// ============================================

void test_move_size() {
    // Piece piece = piece_create(WHITE, KING);
    // Move move = move_create(piece, COORD_TO_IDX("E1"), COORD_TO_IDX("G1"), CASTLE, NONE);
    unsigned d = 0;
    Move move = *(Move *)(&d);
    // move.promoted_type = 7;
    // move.move_type_mask = 15;
    // move.from = 63;
    // move.to = 63;
    move.piece.color = 1;
    move.piece.type = 7;
    // move.piece.captured_type = 7;
    debugzu((size_t)move.data);
    debugzu(sizeof(move));
    debugzu(sizeof(move.data));
    assert(sizeof(move) == sizeof(move.data));
}

void test_move_create() {
    Piece piece = piece_create(WHITE, KING);
    Move move = move_create(piece, COORD_TO_IDX("E1"), COORD_TO_IDX("G1"), CASTLE, NONE);
    assert(move.from == 4);
    assert(move.to == 6);
    assert(move.from == COORD_TO_IDX("e1"));
    assert(move.to == COORD_TO_IDX("g1"));
    assert(move.piece.color == WHITE);
    assert(move.piece.type == KING);
}

void test_move_data_create() {
    Piece piece = piece_create(BLACK, KING);
    Move move = move_create(piece, COORD_TO_IDX("E8"), COORD_TO_IDX("G8"), CASTLE, NONE);
    Move copied_move = move_data_create(move.data);
    assert(copied_move.from == COORD_TO_IDX("e8"));
    assert(copied_move.to == COORD_TO_IDX("g8"));
    assert(copied_move.piece.color == BLACK);
    assert(copied_move.piece.type == KING);
}

void test_move_buf_write() {
    Piece piece_1 = piece_create(WHITE, KING);
    Piece piece_2 = piece_create(BLACK, QUEEN);
    Piece piece_3 = piece_create(WHITE, PAWN);
    Move move_1 = move_create(piece_1, COORD_TO_IDX("e4"), COORD_TO_IDX("e5"), NORMAL, NONE);
    Move move_2 = move_create(piece_2, COORD_TO_IDX("a1"), COORD_TO_IDX("a8"), CAPTURE, NONE);
    Move move_3 = move_create(piece_3, COORD_TO_IDX("a7"), COORD_TO_IDX("A8"), PROMOTION, ROOK);
    Move move_4 = move_create(piece_1, COORD_TO_IDX("E1"), COORD_TO_IDX("G1"), CASTLE, NONE);
    DA *da_1 = da_create();
    DA *da_2 = da_create();
    DA *da_3 = da_create();
    DA *da_4 = da_create();
    char *move_1_str = move_buf_write(move_1, da_1);
    char *move_2_str = move_buf_write(move_2, da_2);
    char *move_3_str = move_buf_write(move_3, da_3);
    char *move_4_str = move_buf_write(move_4, da_4);
    assert(strcmp(move_1_str, "K e4-e5") == 0);
    assert(strcmp(move_2_str, "q a1xa8") == 0);
    assert(strcmp(move_3_str, "P a7-a8=R") == 0);
    assert(strcmp(move_4_str, "K O-O") == 0);
    // debugs(move_1_str);
    // debugs(move_2_str);
    // debugs(move_3_str);
    // debugs(move_4_str);
    // da_free(da_1, true);
    // da_free(da_2, true);
    // da_free(da_3, true);
    // da_free(da_4, true);
}

void test_move() {
    test_wrapper(test_move_size);
    test_wrapper(test_move_create);
    test_wrapper(test_move_data_create);
    test_wrapper(test_move_buf_write);
}
