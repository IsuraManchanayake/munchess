#pragma once

#include "board.h"
#include "move.h"

typedef enum EngineState UNDERLYING(uint8_t) {
    ENGINE_NOT_STARTED=0,
    ENGINE_READY,
    ENGINE_BUSY,
} EngineState;

typedef struct Engine {
    EngineState state;
    // Other options
} Engine;

Engine *engine_create(void);

void engine_start(Engine *engine);

Move engine_best_move(Engine *engine, Board* board);

// ================================

void test_move_sequence(void);
void test_engine(void);
