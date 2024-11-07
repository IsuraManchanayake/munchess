#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "defs.h"

typedef enum Color UNDERLYING(uint8_t) {
    WHITE=0,
    BLACK=1,
} Color;

typedef enum PieceType UNDERLYING(uint8_t) {
    NONE=0,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
} PieceType;

typedef union Piece {
    struct {
        ENUM_BITS(Color, color, 1);
        ENUM_BITS(PieceType, type, 3);
    };
    uint8_t data;
} Piece;

Piece piece_create(Color color, PieceType type);

Piece piece_data_create(uint8_t data);

bool is_piece_null(Piece piece);

char piece_type_repr(PieceType type);

PieceType char_to_piece_type(char c);

Piece char_to_piece(char c);

char piece_repr_base(Color color, PieceType type);

char piece_repr(Piece piece);

char color_repr(Color color);

// ===============================

void test_piece_size(void);
void test_piece(void);
