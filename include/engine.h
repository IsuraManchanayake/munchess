#pragma once

#include "board.h"
#include "move.h"

typedef void (*on_score_event_f)(Move move, size_t depth, int64_t cp);

typedef enum EngineState UNDERLYING(uint8_t) {
    ENGINE_NOT_STARTED=0,
    ENGINE_READY,
    ENGINE_BUSY,
} EngineState;

typedef struct Engine {
    EngineState state;
    Board *board;
    DAi32 *moves;

    on_score_event_f on_score;
} Engine;

Engine *engine_create(on_score_event_f on_score);

void engine_start(Engine *engine);

Move engine_best_move(Engine *engine, Board *board);

// ================================

void test_move_sequence(void);
void test_engine(void);
