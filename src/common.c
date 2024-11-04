#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#include "common.h"
#include "tests.h"
#include "defs.h"

Arena arena = { 0 };

Region *region_create(size_t size) {
    size_t capacity = max(size, REGION_SIZE);
    Region *region = (Region *) malloc(sizeof(Region));
    region->data = malloc(capacity);
    region->size = size;
    region->capacity = capacity;
    region->next = NULL;
    return region;
}

void region_free(Region *region) {
    free(region->data);
}

Arena *arena_create(void) {
    Arena *arena = (Arena *) malloc(sizeof(Arena));
    arena->begin = NULL;
    arena->end = NULL;
    return arena;
}

void *arena_allocate(Arena *arena, size_t size) {
    if (arena->begin == NULL) {
        arena->begin = region_create(size);
        arena->end = arena->begin;
        return arena->end->data;
    }
    if (arena->end->size + size <= arena->end->capacity) {
        void *res = (char *) arena->end->data + arena->end->size;
        arena->end->size += size;
        return res;
    } else {
        Region *new_region = region_create(size);
        arena->end->next = new_region;
        arena->end = new_region;
        return arena->end->data;
    }
}

void arena_free(Arena *arena) {
    Region *region = arena->begin;
    while (region != NULL) {
        free(region->data);
        Region *next = region->next;
        free(region);
        region = next;
    }
    free(arena);
}

void arena_reset(Arena *arena) {
    Region *region = arena->begin;
    while (region != NULL) {
        free(region->data);
        Region *next = region->next;
        free(region);
        region = next;
    }
    arena->begin = NULL;
    arena->end = NULL;
}

size_t n_regions(Arena *arena) {
    size_t c = 0;
    Region *region = arena->begin;
    while (region != NULL) {
        region = region->next;
        ++c;
    }
    return c;
}

DA *da_create(void) {
    DA *da = arena_allocate(&arena, sizeof(DA));
    da->data = NULL;
    da->size = 0;
    da->capacity = 0;
    return da;
}

DAi32 *dai32_create(void) {
    DAi32 *da = arena_allocate(&arena, sizeof(DAi32));
    da->data = NULL;
    da->size = 0;
    da->capacity = 0;
    return da;
}

// void da_reset(DA *da) {
//     da->size = 0;
// }

void da_free(DA *da) {
    if (da->data) {
        free(da->data);
    }
    // free(da);
}

void dai32_free(DAi32 *da) {
    if (da->data) {
        free(da->data);
    }
    // free(da);
}

// void da_alloc(DA *da, size_t capacity) {
//     da->data = malloc(capacity * sizeof(void *));
//     da->capacity = capacity;
// }

void da_resize(DA *da, size_t new_capacity) {
    da->data = (void **) realloc(da->data, new_capacity * sizeof(void *));
    da->capacity = new_capacity;
}

void dai32_resize(DAi32 *da, size_t new_capacity) {
    da->data = (uint32_t*) realloc(da->data, new_capacity * sizeof(uint32_t));
    da->capacity = new_capacity;
}

void da_push(DA *da, void *elem) {
    if (da->capacity == 0) {
        da->data = (void **) malloc(DA_INITIAL_CAPACITY * sizeof(void *));
        da->capacity = DA_INITIAL_CAPACITY;
    } else if (da->size == da->capacity) {
        size_t new_capacity = 2 * da->capacity;
        da_resize(da, new_capacity);
    }
    da->data[da->size++] = elem;
}

void dai32_push(DAi32 *da, uint32_t elem) {
    if (da->capacity == 0) {
        da->data = (uint32_t *) malloc(DA_INITIAL_CAPACITY * sizeof(uint32_t));
        da->capacity = DA_INITIAL_CAPACITY;
    } else if (da->size == da->capacity) {
        size_t new_capacity = 2 * da->capacity;
        dai32_resize(da, new_capacity);
    }
    da->data[da->size++] = elem;
}

void **da_pop(DA *da) {
    if (da->size == 0) {
        return NULL;
    } else if (da->size < da->capacity / 4) {
        size_t new_capacity = da->capacity / 4;
        da_resize(da, new_capacity);
    }
    return &da->data[--da->size];
}

uint32_t *dai32_pop(DAi32 *da) {
    if (da->size == 0) {
        return NULL;
    } else if (da->size < da->capacity / 4) {
        size_t new_capacity = da->capacity / 4;
        dai32_resize(da, new_capacity);
    }
    return &(da->data[--da->size]);
}

void **da_last_elem(DA *da) {
    return &(da->data[da->size - 1]);
}

uint32_t *dai32_last_elem(DAi32 *da) {
    return &(da->data[da->size - 1]);
}

void _buf_printf(DA *da, const char* fmt, ...) {
    va_list args;

    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args) + 1;
    va_end(args);

    size_t curr = da->size;
    size_t new = curr + len;
    if (da->capacity < new) {
        da_resize(da, new + 1);
        *(da->data + new) = 0;
    }

    va_start(args, fmt);
    vsnprintf((char *) da->data + curr, len, fmt, args);
    da->size += len - 1;
    va_end(args);
}

// ======================================

void test_da(void) {
    DA *da = da_create();
    for(size_t i = 0; i < 10; ++i) {
        int *v = (int *) malloc(sizeof(int));
        *v = (int) i;
        da_push(da, v);
        assert(da->capacity >= da->size);
        assert(da->size == (i + 1));
        assert(*(int *)*da_last_elem(da) == (int) i);
        // printf("Capacity: %zu ", da->capacity);
        // printf("Size: %zu ", da->size);
        // printf("Last element: %d\n", *(int *)da_last_elem(da));
    }
    for(size_t i = 0; i < 10; ++i) {
        int value = *(int *)(da->data[i]);
        assert(value == (int) i);
        // printf("Value at index %zu is %d\n", i, value);
    }
    da_free(da);
}

void test_basic_allocation(void) {
    arena_reset(&arena);
    void *block1 = arena_allocate(&arena, 64);
    void *block2 = arena_allocate(&arena, 32);
    assert(block1 != NULL);
    assert(block2 != NULL);
    assert(block1 != block2);
}

void test_allocation_over_region_size(void) {
    arena_reset(&arena);
    void *block1 = arena_allocate(&arena, 200);
    assert(block1 != NULL);
}


void test_multiple_regions(void) {
    arena_reset(&arena);
    void *block1 = arena_allocate(&arena, 64);  // Fits in one region
    void *block2 = arena_allocate(&arena, 80);  // New region should be created
    assert(block1 != NULL);
    assert(block2 != NULL);
    assert(block1 != block2);
}

void test_region_capacity_boundary(void) {
    arena_reset(&arena);
    void *block1 = arena_allocate(&arena, REGION_SIZE);  // Should exactly fit one region
    void *block2 = arena_allocate(&arena, 1);  // Should cause a new region allocation
    assert(block1 != NULL);
    assert(block2 != NULL);
    assert(block1 != block2);
    assert(n_regions(&arena) == 2);
    Region *region1 = (Region *) arena.begin;
    Region *region2 = (Region *) arena.end;
    assert(region1->data == block1);
    assert(region2->data == block2);
    // arena_free(arena);
}

void test_arena_small_size_allocs(void) {
    arena_reset(&arena);
    size_t block1_size = sizeof(int);
    size_t block2_size = sizeof(double);
    void *block0 = arena_allocate(&arena, REGION_SIZE);
    void *block1 = arena_allocate(&arena, block1_size);
    void *block2 = arena_allocate(&arena, block2_size);
    (void) block0;
    assert(block1 != NULL);
    assert(block2 != NULL);
    assert(block1 != block2);
    assert((char *)block2 - (char *)block1 == (long) block1_size);
    Region *last_region = (Region *) arena.end;
    assert(block1 == last_region->data);
    assert(block1_size + block2_size == last_region->size);
}

void test_buf_printf(void) {
    DA *da = da_create();
    buf_printf(da, "hello %s %zu\n", "there", 23ULL);
    buf_printf(da, "mr. %s\n", "ben");
    char *str = "hello there 23\nmr. ben\n";
    assert(strcmp(str, (char *) da->data) == 0);
//    printf("%s", (char *)da->data);
    da_free(da);
}

void test_common(void) {
    test_wrapper(test_da);
    test_wrapper(test_basic_allocation);
    test_wrapper(test_allocation_over_region_size);
    test_wrapper(test_multiple_regions);
    test_wrapper(test_region_capacity_boundary);
    test_wrapper(test_arena_small_size_allocs);
    test_wrapper(test_buf_printf);
}
