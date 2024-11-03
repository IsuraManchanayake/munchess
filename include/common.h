#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define REGION_SIZE 1024

typedef struct Region Region;

typedef struct Region {
    Region *next;
    size_t size;
    size_t capacity;
    void *data;
} Region;

typedef struct Arena {
    Region *begin;
    Region *end;
} Arena;

Region *region_create(size_t size);

void region_free(Region *region);

Arena *arena_create(void);

void *arena_allocate(Arena *arena, size_t size);

void arena_free(Arena *arena);

void arena_reset(Arena *arena);

size_t n_regions(Arena *arena);

extern Arena arena;

typedef struct DA {
    void **data;
    size_t size;
    size_t capacity;
} DA;  // Dynamic array

typedef struct DAi32 {
    uint32_t *data;
    size_t size;
    size_t capacity;
} DAi32;  // Dynamic array uint32_t

#define DA_INITIAL_CAPACITY 1

DA *da_create(void);

DAi32 *dai32_create(void);

void da_free(DA *da);

void dai32_free(DAi32 *da);

void da_resize(DA *da, size_t new_capacity);

void dai32_resize(DAi32 *da, size_t new_capacity);

void da_push(DA *da, void *elem);

void dai32_push(DAi32 *da, uint32_t elem);

void **da_pop(DA *da);

uint32_t *dai32_pop(DAi32 *da);

void **da_last_elem(DA *da);

uint32_t *dai32_last_elem(DAi32 *da);

void _buf_printf(DA *da, const char* fmt, ...);

#define buf_printf(da, fmt, ...) \
    do { \
        _buf_printf((da), (fmt), __VA_ARGS__); \
    } while(0)

// ======================================

void test_da(void);
void test_basic_allocation(void);
void test_allocation_over_region_size(void);
void test_multiple_regions(void);
void test_region_capacity_boundary(void);
void test_arena_small_size_allocs(void);
void test_buf_printf(void);
void test_common(void);
