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
    int pid;  // Process id
} UCI;

extern char *position_parser;

UCI *uci_create(void);

void update_uci_state(void);

void move_to_uci(Move move, char *uci);

Move uci_notation_to_move(const char *uci, Board *board);

bool match_cmd(char *input, const char *cmd);

void curr_time(char *buffer);

void uci_log(UCI *uci, const char *prefix, const char *fmt, ...);

void send_message(UCI *uci, const char *fmt, ...);

void send_uci_ok(UCI *uci);

void send_is_ready(UCI *uci);

const char *uci_store_board(UCI *uci, const char *fen);

void parse_position_command(UCI *uci, const char *input);

void send_best_move(UCI *uci);

void start_uci(void);
