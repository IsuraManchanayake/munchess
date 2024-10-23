#pragma once

#include "piece.h"


typedef struct Move {
    Piece *piece;
    size_t from;
    size_t to;
} Move;