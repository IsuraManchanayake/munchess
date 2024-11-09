#pragma once

#include <stdint.h>

#include "defs.h"
#include "piece.h"
#include "board.h"

typedef enum ResultType UNDERLYING(uint8_t) {
    NO_RESULT=0,
    DRAW,
    MATE,
    RESIGNATION,
    TIMEOUT,
} ResultType;

typedef union Result {
    struct {
        ENUM_BITS(ResultType, type, 3);
        ENUM_BITS(Color, winner, 2);  // 2 bits to include NO_COLOR
    };
    uint8_t data;
} Result;

Result result_create(ResultType type, Color winner);

Result evaluate_result(Board *board);

// ==================

void test_result_size(void);
void test_result(void);
