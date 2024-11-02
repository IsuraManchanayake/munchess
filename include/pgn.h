#pragma once

#include "string.h"

#include "common.h"
#include "tests.h"
#include "utils.h"

extern const char *c;
extern size_t line_n;
extern size_t col_n;

bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool is_alpha(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool is_numeric(char c) {
    return '0' <= c && c <= '9';
}

bool is_promotable_piece(char c) {
    return c == 'Q' || c == 'N' || c == 'B' || c == 'R';
}

void parse_error_exit(const char *msg) {
    printf("line: %zu:%zu | %s\n", line_n, col_n, msg);
    error_exit(-1);
}

void start_parsing(const char *str) {
    c = str;
    line_n = 1;
    col_n = 1;
}

void next_char(void) {
    if (*c == '\0') {
        printf("line: %zu:%zu | Reached EOF while parsing\n", line_n, col_n);
        error_exit(-1);
    }
    if (*c == '\n') {
        ++line_n;
        col_n = 0;
    }
    ++c;
    ++col_n;
}

bool soft_expect(char expected) {
    if (*c != expected) {
        return false;
    }
    next_char();
    return true;
}

void expect(char expected) {
    if (!soft_expect(expected)) {
        printf("line: %zu:%zu | Expected %c, found '%c'\n", line_n, col_n, expected, *c);
        error_exit(-1);
    }
}

bool soft_expect_range(char begin, char end) {
    for (char val = begin; val <= end; ++val) {
        if (soft_expect(val)) {
            return true;
        }
    }
    return false;
}

void expect_range(char begin, char end) {
    if (!soft_expect_range(begin, end)) {
        printf("line: %zu:%zu | Expected [%c, %c], found '%c'\n", line_n, col_n, begin, end, *c);
        error_exit(-1);
    }
}

bool soft_expect_fn(bool (*f)(char)) {
    if (f(*c)) {
        next_char();
        return true;
    }
    return false;
}

void expect_fn(bool (*f)(char)) {
    if (!soft_expect_fn(f)) {
        printf("line: %zu:%zu | %c does not satisfy the function\n", line_n, col_n, *c);
        error_exit(-1);
    }
}

bool soft_expect_str(const char *str) {
    const char *ct = c;
    while (*str) {
        if (*ct != *str) {
            return false;
        }
        ++ct;
        ++str;
    }
    c = ct;
    return true;
}

void skip_whitespace(void) {
    while (soft_expect_fn(is_whitespace)) {
    }
}

void scan_quoted_str(void) {
    expect('"');
    while(!soft_expect('"')) {
        if (soft_expect('\\')) {
            next_char();
        }
        next_char();
    }
}

void expect_square(void) {
    expect_range('a', 'h');
    expect_range('1', '8');
}

void scan_numeric(void) {
    expect_range('0', '9');
    while (soft_expect_range('0', '9'));
}

size_t scan_move_repr(void) {
    const char *start = c;
    if (soft_expect('O')) {
        expect('-');
        expect('O');
        if (soft_expect('-')) {
            expect('O');
        }
    } else if (soft_expect('K') || soft_expect('Q') 
            || soft_expect('R') || soft_expect('B') 
            || soft_expect('N')) {
        if (soft_expect('x')) {
            expect_square();  // Nxf2
        } else if (soft_expect_range('1', '8')) {
            soft_expect('x');
            expect_square();  // N1f2, N1xf2
        } else {
            expect_range('a', 'h');
            if (soft_expect_range('1', '8')) {
                if (soft_expect_range('a', 'h')) {
                    expect_range('1', '8');  // Ng4f2
                } else if (soft_expect('x')) {
                    expect_square();  // Ng4xf2
                } else {
                    // Nf2
                }
            } else {
                soft_expect('x');
                expect_square();  // Ngf2, Ngxf2
            }
        }
    } else if (soft_expect_range('a', 'h')) {
        if (soft_expect('x')) {
            expect_square();
        } else {
            expect_range('1', '8');
        }
        if (soft_expect('=')) {
            expect_fn(is_promotable_piece);
        }
    } else {
        parse_error_exit("Unexpected character");
    }
    if (!soft_expect('+')) {
        soft_expect('#');
    }
    return c - start;
}

void parse_pgn(const char *path, DA *move_strs) {
    char *str = read_file(path);
    start_parsing(str);
    
    // Tag Section
    while(true) {
        skip_whitespace();
        if (!soft_expect('[')) {
            break;
        }
        expect_fn(is_alpha);
        while (soft_expect_fn(is_alpha));
        skip_whitespace();
        scan_quoted_str();
        expect(']');
    }

    bool draw = false;
    Color winning_color;
    while (true) {
        scan_numeric();
        skip_whitespace();
        if (!soft_expect('.')) {
            break;
        }
        skip_whitespace();
        size_t moves_per_line = 2;
        bool game_over = false;
        while(moves_per_line-- > 0) {
            size_t move_len = scan_move_repr();
            char *move_str = strndup(c - move_len, move_len);
            // debugs(move_str);
            // free(move_str);
            da_push(move_strs, (void *) move_str);
            
            skip_whitespace();

            if (soft_expect_str("1-0")) {
                game_over = true;
                winning_color = WHITE;
                break;
            } else if (soft_expect_str("0-1")) {
                game_over = true;
                winning_color = BLACK;
                break;
            } else if (soft_expect_str("1/2-1/2")) {
                game_over = true;
                draw = true;
                break;
            }
        }
        if (game_over) {
            break;
        }
    }

    if (draw) {
        printf("DRAW\n");
    } else {
        if (winning_color == WHITE) {
            printf("WHITE won\n");
        } else {
            printf("BLACK won\n");
        }
    }

    free((void *)str);
}

// ==========================

void test_read_pgn(void) {
    DA *move_strs = da_create();
    parse_pgn("../res/tests/game-1.pgn", move_strs);

    Board *board = board_create();
    place_initial_pieces(board);
    for (size_t i = 0; i < move_strs->size; ++i) {
        // if (i == 91) {
        //     volatile int m = 0;
        //     ++m;
        // }
        const char *move_str = (const char *) move_strs->data[i];
        Move move = notation_to_move(move_str, board);
        if (!is_move_null(move)) {
            apply_move(board, move);
        } else {
            assert(0);
        }
    }
    
    DA *board_da = da_create();
    board_buf_write(board, board_da);
    printf("%s\n", (char *) board_da->data);

    da_free(board_da);
    da_free(move_strs);
}

void test_pgn(void) {
    test_wrapper(test_read_pgn);
}
