#include "common.h"

typedef enum {
    WHITE=0,
    BLACK=1
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

typedef struct Piece {
    Color color;
    PieceType type;
} Piece;

Piece *piece_create(Arena *arena, Color color, PieceType type) {
    Piece *piece = (Piece *)arena_allocate(arena, sizeof(Piece));
    piece->color = color;
    piece->type = type;
    return piece;
}

char piece_repr(Piece *piece) {
    if(piece == NULL) {
        return ' ';
    }
    char res = ' ';
    switch(piece->type) {
        case PAWN:      res = 'p'; break;
        case KNIGHT:    res = 'n'; break;
        case BISHOP:    res = 'b'; break;
        case ROOK:      res = 'r'; break;
        case QUEEN:     res = 'q'; break;
        case KING:      res = 'k'; break;
        default:        res = ' ';
    }
    return piece->color == WHITE ? res ^ 32 : res;
}