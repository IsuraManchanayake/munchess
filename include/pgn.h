#pragma once

#include "common.h"
#include "piece.h"

typedef struct PGNGame {
    DA *move_strs;
    bool draw;
    Color winning_color;
} PGNGame;

PGNGame *pgn_game_create_empty();

size_t scan_move_repr(void);

PGNGame *parse_pgn(const char *path);

// ==========================

void test_read_pgn(void);
void test_pgn_with_fen(void);
void test_pgn(void);
