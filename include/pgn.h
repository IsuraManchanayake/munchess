#pragma once

#include "common.h"
#include "piece.h"

extern const char *stream;
extern size_t line_n;
extern size_t col_n;

typedef struct PGNGame {
    DA *move_strs;
    bool draw;
    Color winning_color;
} PGNGame;

PGNGame *pgn_game_create_empty();

bool is_whitespace(char c);

bool is_alpha(char c);

bool is_numeric(char c);

bool is_promotable_piece(char c);

void parse_error_exit(const char *msg);

void start_parsing(const char *str);

void next_char(void);

bool soft_expect(char expected);

void expect(char expected);

bool soft_expect_range(char begin, char end);

void expect_range(char begin, char end);

bool soft_expect_fn(bool (*f)(char));

void expect_fn(bool (*f)(char));

bool soft_expect_str(const char *str);

void skip_whitespace(void);

void scan_quoted_str(void);

void expect_square(void);

void scan_numeric(void);

size_t scan_move_repr(void);

PGNGame *parse_pgn(const char *path);

// ==========================

void test_read_pgn(void);
void test_pgn_with_fen(void);
void test_pgn(void);
