#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "parser.h"
#include "utils.h"

const char *stream = NULL;
size_t line_n = 1;
size_t col_n = 1;

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
    stream = str;
    line_n = 1;
    col_n = 1;
}

void next_char(void) {
    if (*stream == '\0') {
        printf("line: %zu:%zu | Reached EOF while parsing\n", line_n, col_n);
        error_exit(-1);
    }
    if (*stream == '\n') {
        ++line_n;
        col_n = 0;
    }
    ++stream;
    ++col_n;
}

void advance(size_t d) {
    while (d-- > 0) {
        next_char();
    }
}

bool soft_expect(char expected) {
    if (*stream != expected) {
        return false;
    }
    next_char();
    return true;
}

void expect(char expected) {
    if (!soft_expect(expected)) {
        printf("line: %zu:%zu | Expected %c, found '%c'\n", line_n, col_n, expected, *stream);
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
        printf("line: %zu:%zu | Expected [%c, %c], found '%c'\n", line_n, col_n, begin, end, *stream);
        error_exit(-1);
    }
}

bool soft_expect_fn(bool (*f)(char)) {
    if (f(*stream)) {
        next_char();
        return true;
    }
    return false;
}

void expect_fn(bool (*f)(char)) {
    if (!soft_expect_fn(f)) {
        printf("line: %zu:%zu | %c does not satisfy the function\n", line_n, col_n, *stream);
        error_exit(-1);
    }
}

bool soft_expect_str(const char *str) {
    const char *ct = stream;
    while (*str) {
        if (*ct != *str) {
            return false;
        }
        ++ct;
        ++str;
    }
    stream = ct;
    return true;
}

void expect_str(const char *str) {
    if (!soft_expect_str(str)) {
        int len = (int) strlen(str);
        printf("line: %zu:%zu | Expected %s, found %.*s\n", line_n, col_n, str, len, stream);
        error_exit(-1);
    }
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
