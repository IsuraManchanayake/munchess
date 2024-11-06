#pragma once

#include <stdio.h>

#include "move.h"
#include "board.h"
#include "defs.h"
#include "engine.h"

typedef enum UCIState UNDERLYING(uint8_t) {
    UCI_NOT_READY=0,
    UCI_READY,
    UCI_IN_GAME,
    UCI_ENDED
} UCIState;

typedef struct UCI {
    UCIState state;
    Engine *engine;
    Board *board;
    Move last_move;
    FILE *log_fp;
} UCI;

UCI *uci_create(void);

// UCIState engine_uci_state(void);

void update_uci_state(void);

void move_to_uci(Move move, char* uci);

Move uci_to_move(char* uci, Board* board);

bool match_cmd(char *input, const char *cmd);

void send_uci_ok(UCI *uci);

void send_is_ready(UCI *uci);

void store_board(UCI *uci, char *fen);

void send_best_move(UCI *uci);

void start_uci(void);
