#pragma once

#include "piece.h"
#include "common.h"

#include <stdbool.h>

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
