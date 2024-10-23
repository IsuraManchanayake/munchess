#pragma once

static int test_depth = 0;

#define test_wrapper(f) do { \
    ++test_depth; \
    f(); \
    for(size_t i = 1; i < test_depth; ++i) {printf(" ");} \
    printf("✓ %s\n", #f); \
    --test_depth; \
} while(0)

void test();
