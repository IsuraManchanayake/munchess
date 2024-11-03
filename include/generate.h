#pragma once

#include "board.h"


void generate_attacked(Board *board, Color color, uint8_t attacked[64], size_t *king_idx) {
    int dir = move_direction(color);

    for (size_t i = 0; i < 64; ++i) {
        if (is_piece_null(board->pieces[i])) {
            continue;
        }
        if (board->pieces[i].color == color) {
            if (board->pieces[i].type == KING) {
                *king_idx = i;
            }
            continue;
        }
        int x = IDX_X(i);
        int y = IDX_Y(i);
        // Pawns
        if (board->pieces[i].type == PAWN) {
            if (yx_is_safe(y - dir, x + 1)) {
                attacked[YX_TO_IDX(y - dir, x + 1)] = i + 1;
            }
            if (yx_is_safe(y - dir, x + 1)) {
                attacked[YX_TO_IDX(y - dir, x - 1)] = i + 1;
            }
        } else if (board->pieces[i].type == BISHOP
            || board->pieces[i].type == ROOK
            || board->pieces[i].type == QUEEN) {

            int dirs[8][2] = {
                // Bishop
                {1, 1},
                {1, -1},
                {-1, 1},
                {-1, -1},

                // Rook
                {1, 0},
                {-1, 0},
                {0, 1},
                {0, -1},
            };
            size_t dir_idxs[8][2] = {
                [BISHOP] = {0, 4},
                [ROOK] = {4, 8},
                [QUEEN] = {0, 8},
            };
            size_t start_dir_idx = dir_idxs[board->pieces[i].type][0];
            size_t end_dir_idx = dir_idxs[board->pieces[i].type][1];
            for (size_t j = start_dir_idx; j < end_dir_idx; ++j) {
                int dir_x = dirs[j][0];
                int dir_y = dirs[j][1];
                for (size_t k = 1; k < 8; ++k) {
                    size_t dest = YX_TO_IDX(y + k * dir_y, x + k * dir_x);
                    if (!yx_is_safe(y + k * dir_y, x + k * dir_x)) {
                        break;
                    }
                    attacked[dest] = i + 1;
                    if (!is_piece_null(board->pieces[dest])) {
                        break;
                    }
                }
            }
        } else if (board->pieces[i].type == KNIGHT) {
            int dirs[8][2] = {
                {1, 2},
                {1, -2},
                {-1, 2},
                {-1, -2},
                {2, 1},
                {2, -1},
                {-2, 1},
                {-2, -1}
            };
            for (size_t j = 0; j < 8; ++j) {
                int dir_x = dirs[j][0];
                int dir_y = dirs[j][1];
                size_t dest = YX_TO_IDX(y + dir_y, x + dir_x);
                if (yx_is_safe(y + dir_y, x + dir_x)) {
                    attacked[dest] = i + 1;
                }
            }
        } else if (board->pieces[i].type == KING) {
            int dirs[8][2] = {
                {1, 1},
                {1, -1},
                {-1, 1},
                {-1, -1},
                {1, 0},
                {-1, 0},
                {0, 1},
                {0, -1},
            };
            for (size_t j = 0; j < 8; ++j) {
                int dir_x = dirs[j][0];
                int dir_y = dirs[j][1];
                size_t dest = YX_TO_IDX(y + dir_y, x + dir_x);
                if (yx_is_safe(y + dir_y, x + dir_x)) {
                    attacked[dest] = i + 1;
                }
            }
        } else {
            assert(0);
        }
    }
}

size_t king_in_check(Board *board, size_t king_idx) {
    int x = IDX_X(king_idx);
    int y = IDX_Y(king_idx);
    Piece king_piece = board->pieces[king_idx];
    Color color = king_piece.color;

    // Pawn
    int dir = move_direction(color);
    size_t dest;
    dest = YX_TO_IDX(y + dir, x + 1);
    if (yx_is_safe(y + dir, x + 1)) {
        if (board->pieces[dest].color != color 
            && board->pieces[dest].type == PAWN) {
            return dest + 1;
        }
    }
    dest = YX_TO_IDX(y + dir, x - 1);
    if (yx_is_safe(y + dir, x - 1)) {
        if (board->pieces[dest].color != color 
            && board->pieces[dest].type == PAWN) {
            return dest + 1;
        }
    }

    int dirs[8][2] = {
        // Bishop, Queen
        {1, 1},
        {1, -1},
        {-1, 1},
        {-1, -1},

        // Rook, Queen
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1},
    };
    // Bishop or Queen
    for (size_t i = 0; i < 4; ++i) {
        int dir_x = dirs[i][0];
        int dir_y = dirs[i][1];
        for (size_t k = 1; k < 8; ++k) {
            dest = YX_TO_IDX(y + k * dir_y, x + k * dir_x);
            if (!yx_is_safe(y + k * dir_y, x + k * dir_x)) {
                break;
            }
            if (!is_piece_null(board->pieces[dest])) {
                if (board->pieces[dest].color != color
                    && (board->pieces[dest].type == BISHOP || board->pieces[dest].type == QUEEN)) {
                    return dest + 1;
                } else {
                    break;
                }
            }
        }
    }

    // Rook or Queen
    for (size_t i = 4; i < 8; ++i) {
        int dir_x = dirs[i][0];
        int dir_y = dirs[i][1];
        for (size_t k = 1; k < 8; ++k) {
            dest = YX_TO_IDX(y + k * dir_y, x + k * dir_x);
            if (!yx_is_safe(y + k * dir_y, x + k * dir_x)) {
                break;
            }
            if (!is_piece_null(board->pieces[dest])) {
                if (board->pieces[dest].color != color
                    && (board->pieces[dest].type == ROOK || board->pieces[dest].type == QUEEN)) {
                    return dest + 1;
                } else {
                    break;
                }
            }
        }
    }

    // Knight
    int knight_dirs[8][2] = {
        {1, 2},
        {1, -2},
        {-1, 2},
        {-1, -2},
        {2, 1},
        {2, -1},
        {-2, 1},
        {-2, -1}
    };
    for (size_t j = 0; j < 8; ++j) {
        int dir_x = knight_dirs[j][0];
        int dir_y = knight_dirs[j][1];
        dest = YX_TO_IDX(y + dir_y, x + dir_x);
        if (yx_is_safe(y + dir_y, x + dir_x)) {
            if (!is_piece_null(board->pieces[dest]) 
                && board->pieces[dest].color != color 
                && board->pieces[dest].type == KNIGHT) {
                return dest + 1;
            }
        }
    }

    int king_dirs[8][2] = {
        {1, 1},
        {1, -1},
        {-1, 1},
        {-1, -1},
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1},
    };
    for (size_t j = 0; j < 8; ++j) {
        int dir_x = king_dirs[j][0];
        int dir_y = king_dirs[j][1];
        dest = YX_TO_IDX(y + dir_y, x + dir_x);
        if (yx_is_safe(y + dir_y, x + dir_x)) {
            if (!is_piece_null(board->pieces[dest]) 
                && board->pieces[dest].color != color 
                && board->pieces[dest].type == KING) {
                return dest + 1;
            }
        }
    }

    return 0;
}

void validate_and_push_move(Board *board, DAi32 *moves, Move move, size_t king_idx) {
    apply_move(board, move);
    if (king_in_check(board, king_idx) == 0) {
        dai32_push(moves, move.data);
    }
    undo_last_move(board);
}

void generate_pawn_moves(Board *board, size_t idx, DAi32 *moves, size_t king_idx) {
    Piece piece = board->pieces[idx];
    assert(piece.type == PAWN);

    int dir = move_direction(piece.color);
    size_t y = IDX_Y(idx);
    size_t x = IDX_X(idx);

    PieceType possible_promotions[] = {
        KNIGHT,
        BISHOP,
        ROOK,
        QUEEN
    };
    
    // Straight moves
    size_t one_step = YX_TO_IDX(y + dir, x);
    if (yx_is_safe(y + dir, x) && is_piece_null(board->pieces[one_step])) {
        // Promotion
        if ((y == 6 && piece.color == WHITE) || (y == 1 && piece.color == BLACK)) {
            for (size_t i = 0; i < sizeof(possible_promotions) / sizeof(possible_promotions[0]); ++i) {
                Move promotion_move = move_create(piece, idx, one_step, PROMOTION, possible_promotions[i], NONE);
                validate_and_push_move(board, moves, promotion_move, king_idx);
            }
        } else {
            // One step move
            Move one_step_move = move_create(piece, idx, one_step, NORMAL, NONE, NONE);
            validate_and_push_move(board, moves, one_step_move, king_idx);
            size_t two_steps = YX_TO_IDX(y + 2 * dir, x);
            // Two steps move
            if ((y == 1 && piece.color == WHITE) || (y == 6 && piece.color == BLACK)) {
                if (is_piece_null(board_safe_at(board, two_steps))) {
                    Move two_steps_move = move_create(piece, idx, two_steps, NORMAL, NONE, NONE);      
                    validate_and_push_move(board, moves, two_steps_move, king_idx);
                }
            }
        }
    }

    // Capture moves
    size_t x_dirs[] = {
        x + 1,
        x - 1,
    };
    for (size_t i = 0; i < 2; ++i) {
        size_t dest = YX_TO_IDX(y + dir, x_dirs[i]);
        if (yx_is_safe(y + dir, x_dirs[i])
                && !is_piece_null(board->pieces[dest])
                && board->pieces[dest].color != piece.color 
                && board->pieces[dest].type != KING) {
            size_t dest_y= IDX_Y(dest);
            if ((dest_y == 7 && piece.color == WHITE) || (dest_y == 0 && piece.color == BLACK)) {
                for (size_t i = 0; i < sizeof(possible_promotions) / sizeof(possible_promotions[0]); ++i) {
                    Move promotion_move = move_create(piece, idx, dest, CAPTURE | PROMOTION, possible_promotions[i], board->pieces[dest].type);
                    validate_and_push_move(board, moves, promotion_move, king_idx);
                }
            } else {
                Move move = move_create(piece, idx, dest, CAPTURE, NONE, board->pieces[dest].type);
                validate_and_push_move(board, moves, move, king_idx);
            }
        }
    }

    // En passant moves
    if ((y == 4 && piece.color == WHITE) && (y == 3 && piece.color == BLACK)) {
        Move last_move = move_data_create(*dai32_last_elem(board->moves));
        if (!is_move_null(last_move) && last_move.piece.type == PAWN) {
            unsigned last_move_from_x = IDX_X(last_move.from);
            unsigned last_move_to_x = IDX_X(last_move.to);
            if (last_move_from_x == last_move_to_x 
                    && ((last_move_from_x == x + 1) || (last_move_from_x + 1 == x))) {
                unsigned last_move_from_y = IDX_Y(last_move.from);
                unsigned last_move_to_y = IDX_Y(last_move.to);
                if (last_move_from_y == 2 * dir + last_move_to_y) {
                    size_t dest = YX_TO_IDX(last_move_to_y + dir, last_move_to_x);
                    Move move = move_create(piece, idx, dest, CAPTURE | EN_PASSANT, NONE, PAWN);
                    validate_and_push_move(board, moves, move, king_idx);
                }
            }
        }
    }
}

void generate_bishop_moves(Board *board, size_t idx, DAi32 *moves, size_t king_idx) {
    Piece piece = board->pieces[idx];
    assert(piece.type == BISHOP);

    size_t y = IDX_Y(idx);
    size_t x = IDX_X(idx);

    int dirs[4][2] = {
        {1, 1},
        {1, -1},
        {-1, 1},
        {-1, -1}
    };
    for (size_t j = 0; j < 4; ++j) {
        int dir_x = dirs[j][0];
        int dir_y = dirs[j][1];
        for (size_t i = 1; i < 8; ++i) {
            size_t dest = YX_TO_IDX(y + i * dir_y, x + i * dir_x);
            if (!yx_is_safe(y + i * dir_y, x + i * dir_x)) {
                break;
            }
            if (is_piece_null(board->pieces[dest])) {
                Move move = move_create(piece, idx, dest, NORMAL, NONE, NONE);
                validate_and_push_move(board, moves, move, king_idx);
            } else {
                if (board->pieces[dest].color != piece.color && board->pieces[dest].type != KING) {
                    Move move = move_create(piece, idx, dest, CAPTURE, NONE, board->pieces[dest].type);
                    validate_and_push_move(board, moves, move, king_idx);
                }
                break;
            }
        }
    }
}

void generate_rook_moves(Board *board, size_t idx, DAi32 *moves, size_t king_idx) {
    Piece piece = board->pieces[idx];
    assert(piece.type == ROOK);

    size_t y = IDX_Y(idx);
    size_t x = IDX_X(idx);

    int dirs[4][2] = {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1}
    };
    for (size_t j = 0; j < 4; ++j) {
        int dir_x = dirs[j][0];
        int dir_y = dirs[j][1];
        for (size_t i = 1; i < 8; ++i) {
            size_t dest = YX_TO_IDX(y + i * dir_y, x + i * dir_x);
            if (!yx_is_safe(y + i * dir_y, x + i * dir_x)) {
                break;
            }
            if (is_piece_null(board->pieces[dest])) {
                Move move = move_create(piece, idx, dest, NORMAL, NONE, NONE);
                validate_and_push_move(board, moves, move, king_idx);
            } else {
                if (board->pieces[dest].color != piece.color && board->pieces[dest].type != KING) {
                    Move move = move_create(piece, idx, dest, CAPTURE, NONE, board->pieces[dest].type);
                    validate_and_push_move(board, moves, move, king_idx);
                }
                break;
            }
        }
    }
}

void generate_queen_moves(Board *board, size_t idx, DAi32 *moves, size_t king_idx) {
    Piece piece = board->pieces[idx];
    assert(piece.type == QUEEN);

    size_t y = IDX_Y(idx);
    size_t x = IDX_X(idx);

    int dirs[8][2] = {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1},
        {1, 1},
        {1, -1},
        {-1, 1},
        {-1, -1}
    };
    for (size_t j = 0; j < 8; ++j) {
        int dir_x = dirs[j][0];
        int dir_y = dirs[j][1];
        for (size_t i = 1; i < 8; ++i) {
            size_t dest = YX_TO_IDX(y + i * dir_y, x + i * dir_x);
            if (!yx_is_safe(y + i * dir_y, x + i * dir_x)) {
                break;
            }
            if (is_piece_null(board->pieces[dest])) {
                Move move = move_create(piece, idx, dest, NORMAL, NONE, NONE);
                validate_and_push_move(board, moves, move, king_idx);
            } else {
                if (board->pieces[dest].color != piece.color && board->pieces[dest].type != KING) {
                    Move move = move_create(piece, idx, dest, CAPTURE, NONE, board->pieces[dest].type);
                    validate_and_push_move(board, moves, move, king_idx);
                }
                break;
            }
        }
    }
}

void generate_knight_moves(Board *board, size_t idx, DAi32 *moves, size_t king_idx) {
    Piece piece = board->pieces[idx];
    assert(piece.type == KNIGHT);

    size_t y = IDX_Y(idx);
    size_t x = IDX_X(idx);

    int dirs[8][2] = {
        {1, 2},
        {1, -2},
        {-1, 2},
        {-1, -2},
        {2, 1},
        {2, -1},
        {-2, 1},
        {-2, -1}
    };
    for (size_t j = 0; j < 8; ++j) {
        int dir_x = dirs[j][0];
        int dir_y = dirs[j][1];
        if (!yx_is_safe(y + dir_y, x + dir_x)) {
            continue;
        }
        size_t dest = YX_TO_IDX(y + dir_y, x + dir_x);
        if (is_piece_null(board->pieces[dest])) {
            Move move = move_create(piece, idx, dest, NORMAL, NONE, NONE);
            validate_and_push_move(board, moves, move, king_idx);
        } else {
            if (board->pieces[dest].color != piece.color && board->pieces[dest].type != KING) {
                Move move = move_create(piece, idx, dest, CAPTURE, NONE, board->pieces[dest].type);
                validate_and_push_move(board, moves, move, king_idx);
            }
        }
    }
}

void generate_king_moves(Board *board, size_t idx, DAi32 *moves) {
    Piece piece = board->pieces[idx];
    assert(piece.type == KING);

    size_t y = IDX_Y(idx);
    size_t x = IDX_X(idx);

    uint8_t attacked[64] = {0};
    size_t king_idx;
    generate_attacked(board, piece.color, attacked, &king_idx);
    assert(king_idx == idx);

    // if (board->moves->size == 91) {
    //     DA *da = da_create();
    //     for(size_t i = 0; i < 64; ++i) {
    //         const char pos[] = IDX_TO_COORD(i);
    //         const char from[] = IDX_TO_COORD(attacked[i] - 1);
    //         Piece attacking_piece = board->pieces[attacked[i] - 1];
    //         if (!is_piece_null(attacking_piece)) {
    //             buf_printf(da, "%s %s %c\n", pos, from, piece_repr(attacking_piece));
    //         } else {
    //             buf_printf(da, "%s [SAFE]\n", pos);
    //         }
    //     }
    //     printf("%s", (char *) da->data);
    //     da_free(da);
    // }
    
    // Normal & captures
    int dirs[8][2] = {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1},
        {1, 1},
        {1, -1},
        {-1, 1},
        {-1, -1}
    };
    for (size_t j = 0; j < sizeof(dirs) / sizeof(dirs[0]); ++j) {
        int dir_x = dirs[j][0];
        int dir_y = dirs[j][1];
        size_t dest = YX_TO_IDX(y + dir_y, x + dir_x);
        if (!yx_is_safe(y + dir_y, x + dir_x)) {
            continue;
        }
        if (attacked[dest]) {
            continue;
        }
        if (is_piece_null(board->pieces[dest])) {
            Move move = move_create(piece, idx, dest, NORMAL, NONE, NONE);
            validate_and_push_move(board, moves, move, dest);
        } else {
            if (board->pieces[dest].color != piece.color && board->pieces[dest].type != KING) {
                Move move = move_create(piece, idx, dest, CAPTURE, NONE, board->pieces[dest].type);
                validate_and_push_move(board, moves, move, dest);
            }
        }
    }

    // Castle
    if (board->first_king_move[piece.color] == 0) {
        if (board->first_king_rook_move[piece.color] == 0) {
            size_t sq_1 = YX_TO_IDX(y, x);
            size_t sq_2 = YX_TO_IDX(y, x + 1);
            size_t sq_3 = YX_TO_IDX(y, x + 2);
            if (!attacked[sq_1]
                && !attacked[sq_2]
                && !attacked[sq_3]
                && is_piece_null(board->pieces[sq_2])
                && is_piece_null(board->pieces[sq_3])) {
                Move move = move_create(piece, idx, sq_3, CASTLE, NONE, NONE);
                validate_and_push_move(board, moves, move, sq_3);
            }
        }
        if (board->first_queen_rook_move[piece.color] == 0) {
            size_t sq_1 = YX_TO_IDX(y, x);
            size_t sq_2 = YX_TO_IDX(y, x - 1);
            size_t sq_3 = YX_TO_IDX(y, x - 2);
            size_t sq_4 = YX_TO_IDX(y, x - 3);
            if (!attacked[sq_1]
                && !attacked[sq_2]
                && !attacked[sq_3]
                && is_piece_null(board->pieces[sq_2])
                && is_piece_null(board->pieces[sq_3])
                && is_piece_null(board->pieces[sq_4])) {
                Move move = move_create(piece, idx, sq_3, CASTLE, NONE, NONE);
                validate_and_push_move(board, moves, move, sq_3);
            }
        }
    }
}

void generate_moves(Board *board, DAi32 *moves) {
    uint8_t attacked[64] = {0};
    size_t king_idx;
    generate_attacked(board, board->to_move, attacked, &king_idx);
    for(size_t i = 0; i < 64; ++i) {
        Piece piece = board->pieces[i];
        if (is_piece_null(piece) || piece.type == NONE || piece.color != board->to_move) {
            continue;
        }
        switch (piece.type) {
            case PAWN:   generate_pawn_moves(board, i, moves, king_idx); break;
            case BISHOP: generate_bishop_moves(board, i, moves, king_idx); break;
            case ROOK:   generate_rook_moves(board, i, moves, king_idx); break;
            case QUEEN:  generate_queen_moves(board, i, moves, king_idx); break;
            case KNIGHT: generate_knight_moves(board, i, moves, king_idx); break;
            case KING:   generate_king_moves(board, i, moves); break;
            default: assert(0);
        }
    }
}

Move notation_to_move(const char *notation, Board *board) {
    // TODO: Improve notation_to_move performance
    DAi32 *moves = dai32_create();
    Move rmove = (Move) {0};
    Color color = board->to_move;

    time_t start_time = time_now();
    generate_moves(board, moves);
    time_t end_time = time_now();
    double diff_ms = (end_time - start_time) / 1000.0;
    printf("Time for generating moves: %f ms\n", diff_ms);

    const char *c = notation;
    size_t len = strlen(c);
    bool capture = false;
    bool promotion = false;
    PieceType promoted_type = NONE;
    if (c[len - 1] == '+' || c[len - 1] == '#') {
        --len;
    }
    if (len == 3 && strncmp(c, "O-O", len) == 0) {
        for (size_t i = 0; i < moves->size; ++i) {
            Move move = move_data_create(moves->data[i]);
            if (move.piece.color == color
                && move.piece.type == KING
                && move_is_type_of(move, CASTLE)
                && IDX_X(move.to) == 6) {
                rmove = move;
                goto finalize;
            }
        }
        goto finalize;
    } else if (len == 5 && strncmp(c, "O-O-O", len) == 0) {
        for (size_t i = 0; i < moves->size; ++i) {
            Move move = move_data_create(moves->data[i]);
            if (move.piece.color == color
                && move.piece.type == KING
                && move_is_type_of(move, CASTLE)
                && IDX_X(move.to) == 2) {
                rmove = move;
                goto finalize;
            }
        }
        goto finalize;
    }
    if (len >= 2 && c[len - 2] == '=') {
        promoted_type = char_to_piece_type(c[len - 1]);
        len -= 2;
    }
    if (len >= 3 && c[len - 3] == 'x') {
        capture = true;
    }
    if (c[0] == 'K') {
        size_t dest = COORD_TO_IDX(c + len - 2);
        for (size_t i = 0; i < moves->size; ++i) {
            Move move = move_data_create(moves->data[i]);
            if (move.piece.color == color 
                && move.piece.type == KING 
                && move.to == dest) {
                rmove = move;
                goto finalize;
            }
        }
        goto finalize;
    } else if (c[0] == 'Q' || c[0] == 'R' || c[0] == 'B' || c[0] == 'N') {
        PieceType piece_type = char_to_piece_type(c[0]);
        size_t dest = COORD_TO_IDX(c + len - 2);
        char rank_key = 0;
        char file_key = 0;
        if ((!capture && len == 4) || (capture && len == 5)) {
            if (('a' <= c[1] && c[1] <= 'h') || ('A' <= c[1] && c[1] <= 'H')) {
                file_key = c[1];
            } else if (('1' <= c[1] && c[1] <= '8')) {
                rank_key = c[1] - '0';
            } else {
                assert(0);
            }
            ++c;
        } else if ((!capture && len == 5) || (capture && len == 6)) {
            file_key = c[1];
            rank_key = c[2] - '0';
            c += 2;
        } else if ((!capture && len != 3) || (capture && len != 4)) {
            assert(0);
        }
        // char dest_file = *(++c);
        // size_t dest_rank = *(++c) - '0';
        // size_t dest = FR_TO_IDX(dest_file, dest_rank);
        for (size_t i = 0; i < moves->size; ++i) {
            Move move = move_data_create(moves->data[i]);
            if (move.piece.color != color) {
                continue;
            }
            if (move.piece.type != piece_type) {
                continue;
            }
            char from_file = IDX_TO_FILE(move.from);
            char from_rank = IDX_TO_RANK(move.from);
            if (move.to == dest 
                && (!rank_key || (from_rank == rank_key)) 
                && (!file_key || (from_file == file_key))) {
                rmove = move;
                goto finalize;
            }
        }
        return (Move) {0};
    } else if (('a' <= c[0] && c[0] <= 'h') ||('A' <= c[0] && c[0] <= 'H')) {
        if (len == 2 || len == 4) { // e4 or exd4
            size_t file = c[0];
            size_t dest = COORD_TO_IDX(c + len - 2);
            for (size_t i = 0; i < moves->size; ++i) {
                Move move = move_data_create(moves->data[i]);
                if (move.piece.color == color
                    && move.piece.type ==  PAWN
                    && move.to == dest
                    && IDX_TO_FILE(move.from) == file) {
                    rmove = move;
                    goto finalize;
                }
            }
            goto finalize;
        } else {
            assert(0);
        }
    }
    (void) capture;
    (void) promotion;
    (void) promoted_type;
    finalize:
    dai32_free(moves);
    return rmove;
}

// ==================================

void test_generate_initial_moves(void) {
    Board *board = board_create();
    place_initial_pieces(board);
    Move move_1 = move_create(ATcoord(board, "E2"), COORD_TO_IDX("E2"), COORD_TO_IDX("E4"), NORMAL, NONE, NONE);
    apply_move(board, move_1);
    Move move_2 = move_create(ATcoord(board, "E7"), COORD_TO_IDX("E7"), COORD_TO_IDX("E5"), NORMAL, NONE, NONE);
    apply_move(board, move_2);
    Move move_3 = move_create(ATcoord(board, "G1"), COORD_TO_IDX("G1"), COORD_TO_IDX("F3"), NORMAL, NONE, NONE);
    apply_move(board, move_3);
    Move move_4 = move_create(ATcoord(board, "B8"), COORD_TO_IDX("B8"), COORD_TO_IDX("C6"), NORMAL, NONE, NONE);
    apply_move(board, move_4);
    DAi32 *moves = dai32_create();
    generate_moves(board, moves);
    assert(moves->size == 27);
    // debugzu(moves->size);
    // for (size_t i = 0; i < moves->size; ++i) {
    //     DA *da = da_create();
    //     move_buf_write(move_data_create(moves->data[i]), da);
    //     printf("%zu. %s\n", i, (char *) da->data);
    //     // debugs((char *)da->data);
    // }
    // Move move_5 = move_data_create(moves->data[23]);
    // apply_move(board, move_5);

    DA *da = da_create();
    char *repr = board_buf_write(board, da);
    char *expected = 
"   +---+---+---+---+---+---+---+---+\n"
" 8 | r |   | b | q | k | b | n | r |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 7 | p | p | p | p |   | p | p | p |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 6 |   |   | n |   |   |   |   |   |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 5 |   |   |   |   | p |   |   |   |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 4 |   |   |   |   | P |   |   |   |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 3 |   |   |   |   |   | N |   |   |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 2 | P | P | P | P |   | P | P | P |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 1 | R | N | B | Q | K | B |   | R |\n"
"   +---+---+---+---+---+---+---+---+\n"
"     a   b   c   d   e   f   g   h  \n"
"                                    ";
    // printf("%s\n", repr);
    assert(strcmp(repr, expected) == 0);
}

void test_notation_to_move(void) {
    Board *board = board_create();
    place_initial_pieces(board);

    const char *moves[] = {
        "e4", "e5",
        "Nf3", "Nc6",
        "Bc4", "Bc5",
        "O-O", "d6",
        "d3", "Be6",
        "Bd2", "Qd7",
        "Nc3", "O-O-O",
    };
    for (size_t i = 0; i < sizeof(moves) / sizeof(moves[0]); ++i) {
        Move move = notation_to_move(moves[i], board);
        assert(!is_move_null(move));
        apply_move(board, move);

        // DA *da_1 = da_create();
        // char *repr = move_buf_write(move, da_1);

        // DA *da_2 = da_create();
        // char *fen = board_to_fen(board, da_2);

        // DA *da_3 = da_create();
        // char *board_buf = board_buf_write(board, da_3);

        // debugs(repr);
        // debugs(fen);
        // printf("%s\n", board_buf);

        // da_free(da_1);
        // da_free(da_2);
        // da_free(da_3);

        // printf("--\n");
    }
    DA *da_1 = da_create();
    char *fen_1 = board_to_fen(board, da_1);
    // debugs(fen_1);
    const char *expected_1 = "2kr2nr/pppq1ppp/2npb3/2b1p3/2B1P3/2NP1N2/PPPB1PPP/R2Q1RK1 w - - 5 8";
    assert(strcmp(fen_1, expected_1) == 0);
    da_free(da_1);


    for (size_t i = 0; i < sizeof(moves) / sizeof(moves[0]); ++i) {
        undo_last_move(board);
    }

    DA *da_2 = da_create();
    char *fen_2 = board_to_fen(board, da_2);
    // debugs(fen_2);
    const char *expected_2 = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    assert(strcmp(fen_2, expected_2) == 0);
    da_free(da_2);
}

void test_generate(void) {
    test_wrapper(test_generate_initial_moves);
    test_wrapper(test_notation_to_move);
}
