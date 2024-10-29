#pragma once

#include "common.h"
#include "tests.h"

typedef enum {
    WHITE,
    BLACK,
} Color;

typedef enum {
    NONE=0,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
} PieceType;

#ifdef _MSC_VER
    #pragma pack(push, 1)
#endif

typedef union Piece {
    struct {
        Color color:1;
        PieceType type:3; 
    }
#ifdef __GNUC__
    __attribute__((packed)) // For GCC and Clang
#endif
;
    uint8_t data;
} Piece;

#ifdef _MSC_VER
    #pragma pack(pop)
#endif

Piece piece_create(Color color, PieceType type) {
    return (Piece) {.color=color, .type=type};
}

bool is_piece_null(Piece piece) {
    return piece.type == NONE;
}

char piece_type_repr(PieceType type) {
    switch(type) {
        case PAWN:      return 'p';
        case KNIGHT:    return 'n';
        case BISHOP:    return 'b';
        case ROOK:      return 'r';
        case QUEEN:     return 'q';
        case KING:      return 'k';
        case NONE: {}
    }
    return ' ';
}

PieceType char_to_piece_type(char c) {
    c = simple(c);
    switch(c) {
        case 'k': return KING;
        case 'q': return QUEEN;
        case 'r': return ROOK;
        case 'b': return BISHOP;
        case 'n': return KNIGHT;
        case 'p': return PAWN;
        default: return NONE;
    }
}

char piece_repr_base(Color color, PieceType type) {
    if (type == NONE) {
        return ' ';
    }
    char res = piece_type_repr(type);
    return (res != ' ' && color == WHITE) ? res ^ 32 : res;
}

char piece_repr(Piece piece) {
    return piece_repr_base(piece.color, piece.type);
}

// ===============================

void test_piece_size() {
    Piece piece = piece_create(WHITE, KING);
    assert(sizeof(piece) == sizeof(piece.data));
}

void test_piece() {
    test_wrapper(test_piece_size);
}