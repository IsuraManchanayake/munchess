#pragma once

#include <stdio.h>
#include <assert.h>

extern int test_depth;

#define test_wrapper(f) do { \
    ++test_depth; \
    f(); \
    for(int i = 1; i < test_depth; ++i) {printf(" ");} \
    printf("✓ %s\n", #f); \
    --test_depth; \
} while(0)

void test(void);
