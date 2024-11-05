#pragma once

#include "move.h"
#include "board.h"

void move_to_uci(Move move, char* uci);

Move uci_to_move(char* uci, Board* board);

void start_uci(void);
