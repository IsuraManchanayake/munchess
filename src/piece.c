#include "piece.h"
#include "defs.h"
#include "tests.h"

Piece piece_create(Color color, PieceType type) {
    return (Piece) {.color=color, .type=type};
}

Piece piece_data_create(uint8_t data) {
    return (Piece) {.data=data};
}

bool is_piece_null(Piece piece) {
    return piece.type == NONE;
}

char piece_type_repr(PieceType type) {
    if (NONE <= type && type <= KING) {
        return " pnbrqk"[(size_t)type];
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

Piece char_to_piece(char c) {
    switch(c) {
        case 'k': return piece_create(BLACK, KING);
        case 'q': return piece_create(BLACK, QUEEN);
        case 'r': return piece_create(BLACK, ROOK);
        case 'b': return piece_create(BLACK, BISHOP);
        case 'n': return piece_create(BLACK, KNIGHT);
        case 'p': return piece_create(BLACK, PAWN);
        case 'K': return piece_create(WHITE, KING);
        case 'Q': return piece_create(WHITE, QUEEN);
        case 'R': return piece_create(WHITE, ROOK);
        case 'B': return piece_create(WHITE, BISHOP);
        case 'N': return piece_create(WHITE, KNIGHT);
        case 'P': return piece_create(WHITE, PAWN);
        default: return piece_create(WHITE, NONE);
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

char color_repr(Color color) {
    return color == WHITE ? 'w' : 'b';
}

// ===============================

void test_piece_size(void) {
    Piece piece = piece_create(WHITE, KING);
    assert(sizeof(piece) == sizeof(piece.data));
}

void test_piece(void) {
    test_wrapper(test_piece_size);
}
