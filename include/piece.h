#pragma once

#include <stdint.h>
#include <stdbool.h>

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

Piece piece_create(Color color, PieceType type);

Piece piece_data_create(uint8_t data);

bool is_piece_null(Piece piece);

char piece_type_repr(PieceType type);

PieceType char_to_piece_type(char c);

char piece_repr_base(Color color, PieceType type);

char piece_repr(Piece piece);

char color_repr(Color color);

// ===============================

void test_piece_size(void);
void test_piece(void);
