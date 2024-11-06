#pragma once
#pragma once

#include <stdlib.h>

extern const char *stream;
extern size_t line_n;
extern size_t col_n;

bool is_whitespace(char c);

bool is_alpha(char c);

bool is_numeric(char c);

bool is_promotable_piece(char c);

void parse_error_exit(const char *msg);

void start_parsing(const char *str);

void next_char(void);

void advance(size_t d);

bool soft_expect(char expected);

void expect(char expected);

bool soft_expect_range(char begin, char end);

void expect_range(char begin, char end);

bool soft_expect_fn(bool (*f)(char));

void expect_fn(bool (*f)(char));

bool soft_expect_str(const char *str);

void expect_str(const char *str);

void skip_whitespace(void);

void scan_quoted_str(void);

void expect_square(void);

void scan_numeric(void);
