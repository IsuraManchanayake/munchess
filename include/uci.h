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
extern UCI *uci;

UCI *uci_create(void);

void move_to_uci(Move move, char *uci);

Move uci_notation_to_move(const char *uci, Board *board);

bool match_cmd(char *input, const char *cmd);

void curr_time(char *buffer);

void uci_log(const char *prefix, const char *fmt, ...);

void send_message(const char *fmt, ...);

void send_uci_ok();

void send_is_ready();

void send_info_score_cp(int64_t cp);

const char *uci_store_board(const char *fen);

void parse_position_command(const char *input);

void send_best_move();

void start_uci(void);
