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

char piece_repr(Piece piece) {
    if (piece.type == NONE) {
        return ' ';
    }
    char res = piece_type_repr(piece.type);
    return (res != ' ' && piece.color == WHITE) ? res ^ 32 : res;
}

// ===============================

void test_piece_size() {
    Piece piece = piece_create(WHITE, KING);
    assert(sizeof(piece) == sizeof(piece.data));
}

void test_piece() {
    test_wrapper(test_piece_size);
}