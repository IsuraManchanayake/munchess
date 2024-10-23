#pragma once

#include "tests.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b));
#endif
#ifndef max
#define max(a, b) ((a) < (b) ? (b) : (a));
#endif

typedef struct {
    void **data;
    size_t size;
    size_t capacity;
} DA;  // Dynamic array

DA *da_create(void) {
    DA *da = (DA *) malloc(sizeof(DA));
    da->data = NULL;
    da->size = 0;
    da->capacity = 0;
    return da;
}

void da_free(DA *da, bool free_data) {
    if (da->data && free_data) {
        free(da->data);
    }
    free(da);
}

void da_alloc(DA *da, size_t capacity) {
    da->data = malloc(capacity * sizeof(void *));
    da->capacity = capacity;
}

void da_grow(DA *da, size_t new_capacity) {
    da->data = (void **) realloc(da->data, new_capacity * sizeof(void *));
    da->capacity = new_capacity;
}

void da_push(DA *da, void *elem) {
    if (da->capacity == 0) {
        da->data = (void **) malloc(sizeof(void *));
        da->capacity = 1;
    } else if (da->size == da->capacity) {
        size_t new_capacity = 2 * da->capacity;
        da_grow(da, new_capacity);
    }
    da->data[da->size++] = elem;
}

void **da_last_elem(DA *da) {
    return &(da->data[da->size - 1]);
}

#define REGION_SIZE 1024

typedef struct Region Region;

typedef struct Region {
    // Region *next;
    size_t size;
    size_t capacity;
    void *data;
} Region;

typedef struct {
    DA *regions;
} Arena;

Region *region_create(size_t size) {
    size_t capacity = max(size, REGION_SIZE);
    Region *region = (Region *) malloc(sizeof(Region));
    region->data = malloc(capacity);
    region->size = size;
    region->capacity = capacity;
    return region;
}

void region_free(Region *region) {
    free(region->data);
}

Arena *arena_create() {
    Arena *arena = (Arena *) malloc(sizeof(Arena));
    arena->regions = da_create();
    return arena;
}

void *arena_allocate(Arena *arena, size_t size) {
    if(arena->regions->size == 0) {
        Region *region = region_create(size);
        da_push(arena->regions, region);
        return region->data;
    }
    Region *last_region = (Region *) *da_last_elem(arena->regions);
    if(last_region->size + size <= last_region->capacity) {
        void *res = (char *) last_region->data + last_region->size;
        last_region->size += size;
        return res;
    } else {
        Region *new_region = region_create(size);
        da_push(arena->regions, new_region);
        return new_region->data;
    }
}

void arena_free(Arena *arena) {
    for(size_t i = 0, s = arena->regions->size; i < s; ++i) {
        region_free(arena->regions->data[i]);
    }
    da_free(arena->regions, false);
    free(arena);
}

void _buf_printf(DA *da, const char* fmt, ...) {
    va_list args;

    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args) + 1;
    va_end(args);

    size_t curr = da->size;
    size_t new = curr + len;
    if (da->capacity < new) {
        da_grow(da, new + 1);
        *(da->data + new) = 0;
    }

    va_start(args, fmt);
    vsnprintf((char *) da->data + curr, len, fmt, args);
    da->size += len - 1;
    va_end(args);
}

#define buf_printf(da, fmt, ...) \
    do { \
        _buf_printf((da), (fmt), ##__VA_ARGS__); \
    } while(0)

void test_da() {
    DA *da = da_create();
    for(size_t i = 0; i < 10; ++i) {
        int *v = (int *) malloc(sizeof(int));
        *v = (int) i;
        da_push(da, v);
        assert(da->capacity >= da->size);
        assert(da->size = (i + 1));
        assert(*(int *)*da_last_elem(da) == i);
        // printf("Capacity: %zu ", da->capacity);
        // printf("Size: %zu ", da->size);
        // printf("Last element: %d\n", *(int *)da_last_elem(da));
    }
    for(size_t i = 0; i < 10; ++i) {
        int value = *(int *)(da->data[i]);
        assert(value == i);
        // printf("Value at index %zu is %d\n", i, value);
    }
    da_free(da, true);
}

void test_basic_allocation() {
    Arena *arena = arena_create();
    void *block1 = arena_allocate(arena, 64);
    void *block2 = arena_allocate(arena, 32);
    assert(block1 != NULL);
    assert(block2 != NULL);
    assert(block1 != block2);
    arena_free(arena);
}

void test_allocation_over_region_size() {
    Arena *arena = arena_create();
    void *block1 = arena_allocate(arena, 200);  // Should span across multiple regions
    assert(block1 != NULL);
    arena_free(arena);
}


void test_multiple_regions() {
    Arena *arena = arena_create();
    void *block1 = arena_allocate(arena, 64);  // Fits in one region
    void *block2 = arena_allocate(arena, 80);  // New region should be created
    assert(block1 != NULL);
    assert(block2 != NULL);
    assert(block1 != block2);
    arena_free(arena);
}

void test_region_capacity_boundary() {
    Arena *arena = arena_create();
    void *block1 = arena_allocate(arena, REGION_SIZE);  // Should exactly fit one region
    void *block2 = arena_allocate(arena, 1);  // Should cause a new region allocation
    assert(block1 != NULL);
    assert(block2 != NULL);
    assert(block1 != block2);
    Region *region1 = arena->regions->data[0];
    Region *region2 = arena->regions->data[1];
    assert(region1->data == block1);
    assert(region2->data == block2);
    arena_free(arena);
}

void test_arena_small_size_allocs() {
    Arena *arena = arena_create();
    size_t block1_size = sizeof(int);
    size_t block2_size = sizeof(double);
    void *block1 = arena_allocate(arena, block1_size);
    void *block2 = arena_allocate(arena, block2_size);
    assert(block1 != NULL);
    assert(block2 != NULL);
    assert(block1 != block2);
    assert((char *)block2 - (char *)block1 == block1_size);
    Region *last_region = (Region *) *da_last_elem(arena->regions);
    assert(block1 == last_region->data);
    assert(block1_size + block2_size == last_region->size);
    arena_free(arena);
}

void test_buf_printf() {
    DA *da = da_create();
    buf_printf(da, "hello %s %zu\n", "there", 23ULL);
    buf_printf(da, "mr. %s\n", "ben");
    char *str = "hello there 23\nmr. ben\n";
    assert(strcmp(str, (char *) da->data) == 0);
//    printf("%s", (char *)da->data);
}

void test_common() {
    test_wrapper(test_da);
    test_wrapper(test_basic_allocation);
    test_wrapper(test_allocation_over_region_size);
    test_wrapper(test_multiple_regions);
    test_wrapper(test_region_capacity_boundary);
    test_wrapper(test_arena_small_size_allocs);
    test_wrapper(test_buf_printf);
}