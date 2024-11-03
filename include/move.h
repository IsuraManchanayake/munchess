#pragma once

#include "piece.h"
#include "common.h"

#include <stdbool.h>

typedef UCHAR_ENUM {
    NORMAL=0,
    CAPTURE=1<<0,
    EN_PASSANT=1<<1,
    CASTLE=1<<2,
    PROMOTION=1<<3,
} MoveType;

typedef union Move {
    struct {
        ENUM_BITS(PieceType, promoted_type, 3);
        uint8_t move_type_mask : 4;
        uint8_t from : 6;
        uint8_t to : 6;
        ENUM_BITS(Color, piece_color, 1);
        ENUM_BITS(PieceType, piece_type, 3);
        ENUM_BITS(PieceType, captured_type, 3);
    };
    uint32_t data;
} Move;

bool move_is_type_of(Move move, MoveType type);

Move move_create(Piece piece, 
                    unsigned from, 
                    unsigned to, 
                    unsigned move_type_mask, 
                    PieceType promoted_type, 
                    PieceType captured_type);

Move move_data_create(uint32_t data);

bool is_move_null(Move move);

// typedef struct Board Board;

char *move_buf_write(Move move, DA *da);

// ============================================

void test_move_size(void);
void test_move_create(void);
void test_move_data_create(void);
void test_move_buf_write(void);
void test_move(void);
