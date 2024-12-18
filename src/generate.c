#include <string.h>

#include "generate.h"
#include "utils.h"
#include "defs.h"
#include "board.h"
#include "tests.h"

void generate_attacked(Board *board, Color color, uint8_t attacked[64], size_t *king_idx) {
#define mark_attacked(idx, from) \
    do { \
        set_attacked(board, (idx)); \
        attacked[(idx)] = (from) + 1; \
    } while (0)
    board->attacked = 0;
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
                size_t idx = YX_TO_IDX(y - dir, x + 1);
                mark_attacked(idx, i);
            }
            if (yx_is_safe(y - dir, x - 1)) {
                size_t idx = YX_TO_IDX(y - dir, x - 1);
                mark_attacked(idx, i);
            }
        } else if (board->pieces[i].type == BISHOP
            || board->pieces[i].type == ROOK
            || board->pieces[i].type == QUEEN) {

            static const int dirs[8][2] = {
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
            static const size_t dir_idxs[8][2] = {
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
                    mark_attacked(dest, i);
                    if (!is_piece_null(board->pieces[dest])) {
                        break;
                    }
                }
            }
        } else if (board->pieces[i].type == KNIGHT) {
            static const int dirs[8][2] = {
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
                    mark_attacked(dest, i);
                }
            }
        } else if (board->pieces[i].type == KING) {
            static const int dirs[8][2] = {
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
                    mark_attacked(dest, i);
                }
            }
        } else {
            assert(0);
        }
    }
    board->attacked_evaluated = true;
#undef mark_attacked
}

bool _return_king_in_check(size_t *checked_by, size_t idx) {
    *checked_by = idx;
    return true;
}

bool is_king_in_check_base(Board *board, Color color, size_t *checked_by) {
    size_t king_idx = get_king_idx(board, color);
    int x = IDX_X(king_idx);
    int y = IDX_Y(king_idx);
    // Piece king_piece = board->pieces[king_idx];

    // Pawn
    int dir = move_direction(color);
    size_t dest;
    dest = YX_TO_IDX(y + dir, x + 1);
    if (yx_is_safe(y + dir, x + 1)) {
        if (board->pieces[dest].color != color 
            && board->pieces[dest].type == PAWN) {
            return _return_king_in_check(checked_by, dest);
        }
    }
    dest = YX_TO_IDX(y + dir, x - 1);
    if (yx_is_safe(y + dir, x - 1)) {
        if (board->pieces[dest].color != color 
            && board->pieces[dest].type == PAWN) {
            return _return_king_in_check(checked_by, dest);
        }
    }

    static const int dirs[8][2] = {
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
                    return _return_king_in_check(checked_by, dest);
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
                    return _return_king_in_check(checked_by, dest);
                } else {
                    break;
                }
            }
        }
    }

    // Knight
    static const int knight_dirs[8][2] = {
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
                return _return_king_in_check(checked_by, dest);
            }
        }
    }

    static const int king_dirs[8][2] = {
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
                return _return_king_in_check(checked_by, dest);
            }
        }
    }

    *checked_by = 0;
    return false;
}


bool is_king_in_check(Board *board) {
    size_t checked_by = 0;
    return is_king_in_check_base(board, board->to_move, &checked_by);
}

bool validate_and_push_move(Board *board, DAi32 *moves, Move move) {
    bool is_valid = false;
    apply_move_base(board, move, false);
    size_t _checked_by = 0;
    if (!is_king_in_check_base(board, move.piece_color, &_checked_by)) {
        dai32_push(moves, move.data);
        is_valid = true;
    }
    undo_last_move_base(board, false);
    return is_valid;
}

bool generate_pawn_moves(Board *board, size_t idx, DAi32 *moves, bool return_on_found) {
    (void) return_on_found;
    Piece piece = board->pieces[idx];
    assert(piece.type == PAWN);

    int dir = move_direction(piece.color);
    size_t y = IDX_Y(idx);
    size_t x = IDX_X(idx);

    static const PieceType possible_promotions[] = {
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
                validate_and_push_move(board, moves, promotion_move);
            }
        } else {
            // One step move
            Move one_step_move = move_create(piece, idx, one_step, NORMAL, NONE, NONE);
            validate_and_push_move(board, moves, one_step_move);
            size_t two_steps = YX_TO_IDX(y + 2 * dir, x);
            // Two steps move
            if ((y == 1 && piece.color == WHITE) || (y == 6 && piece.color == BLACK)) {
                if (is_piece_null(board_safe_at(board, two_steps))) {
                    Move two_steps_move = move_create(piece, idx, two_steps, NORMAL, NONE, NONE);      
                    validate_and_push_move(board, moves, two_steps_move);
                }
            }
        }
    }

    // Capture moves
    const size_t x_dirs[] = {
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
                for (size_t j = 0; j < sizeof(possible_promotions) / sizeof(possible_promotions[0]); ++j) {
                    Move promotion_move = move_create(piece, idx, dest, CAPTURE | PROMOTION, possible_promotions[j], board->pieces[dest].type);
                    validate_and_push_move(board, moves, promotion_move);
                }
            } else {
                Move move = move_create(piece, idx, dest, CAPTURE, NONE, board->pieces[dest].type);
                validate_and_push_move(board, moves, move);
            }
        }
    }

    // En passant moves
    if ((y == 4 && piece.color == WHITE) && (y == 3 && piece.color == BLACK)) {
        Move last_move = move_data_create(*dai32_last_elem(board->moves));
        if (!is_move_null(last_move) && last_move.piece_type == PAWN) {
            unsigned last_move_from_x = IDX_X(last_move.from);
            unsigned last_move_to_x = IDX_X(last_move.to);
            if (last_move_from_x == last_move_to_x 
                    && ((last_move_from_x == x + 1) || (last_move_from_x + 1 == x))) {
                unsigned last_move_from_y = IDX_Y(last_move.from);
                unsigned last_move_to_y = IDX_Y(last_move.to);
                if (last_move_from_y == 2 * dir + last_move_to_y) {
                    size_t dest = YX_TO_IDX(last_move_to_y + dir, last_move_to_x);
                    Move move = move_create(piece, idx, dest, CAPTURE | EN_PASSANT, NONE, PAWN);
                    validate_and_push_move(board, moves, move);
                }
            }
        }
    }
    return false;
}

bool generate_bishop_moves(Board *board, size_t idx, DAi32 *moves, bool return_on_found) {
    (void) return_on_found;
    Piece piece = board->pieces[idx];
    assert(piece.type == BISHOP);

    size_t y = IDX_Y(idx);
    size_t x = IDX_X(idx);

    static const int dirs[4][2] = {
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
                validate_and_push_move(board, moves, move);
            } else {
                if (board->pieces[dest].color != piece.color && board->pieces[dest].type != KING) {
                    Move move = move_create(piece, idx, dest, CAPTURE, NONE, board->pieces[dest].type);
                    validate_and_push_move(board, moves, move);
                }
                break;
            }
        }
    }
    return false;
}

bool generate_rook_moves(Board *board, size_t idx, DAi32 *moves, bool return_on_found) {
    (void) return_on_found;
    Piece piece = board->pieces[idx];
    assert(piece.type == ROOK);

    size_t y = IDX_Y(idx);
    size_t x = IDX_X(idx);

    static const int dirs[4][2] = {
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
                validate_and_push_move(board, moves, move);
            } else {
                if (board->pieces[dest].color != piece.color && board->pieces[dest].type != KING) {
                    Move move = move_create(piece, idx, dest, CAPTURE, NONE, board->pieces[dest].type);
                    validate_and_push_move(board, moves, move);
                }
                break;
            }
        }
    }
    return false;
}

bool generate_queen_moves(Board *board, size_t idx, DAi32 *moves, bool return_on_found) {
    (void) return_on_found;
    Piece piece = board->pieces[idx];
    assert(piece.type == QUEEN);

    size_t y = IDX_Y(idx);
    size_t x = IDX_X(idx);

    static const int dirs[8][2] = {
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
                validate_and_push_move(board, moves, move);
            } else {
                if (board->pieces[dest].color != piece.color && board->pieces[dest].type != KING) {
                    Move move = move_create(piece, idx, dest, CAPTURE, NONE, board->pieces[dest].type);
                    validate_and_push_move(board, moves, move);
                }
                break;
            }
        }
    }
    return false;
}

bool generate_knight_moves(Board *board, size_t idx, DAi32 *moves, bool return_on_found) {
    (void) return_on_found;
    Piece piece = board->pieces[idx];
    assert(piece.type == KNIGHT);

    size_t y = IDX_Y(idx);
    size_t x = IDX_X(idx);

    static const int dirs[8][2] = {
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
            validate_and_push_move(board, moves, move);
        } else {
            if (board->pieces[dest].color != piece.color && board->pieces[dest].type != KING) {
                Move move = move_create(piece, idx, dest, CAPTURE, NONE, board->pieces[dest].type);
                validate_and_push_move(board, moves, move);
            }
        }
    }
    return false;
}

bool generate_king_moves(Board *board, size_t idx, DAi32 *moves, bool return_on_found) {
    (void) return_on_found;
    Piece piece = board->pieces[idx];
    assert(piece.type == KING);

    size_t y = IDX_Y(idx);
    size_t x = IDX_X(idx);

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
    static const int dirs[8][2] = {
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
        // if (attacked[dest]) {
        //     continue;
        // }
        if (is_piece_null(board->pieces[dest])) {
            Move move = move_create(piece, idx, dest, NORMAL, NONE, NONE);
            validate_and_push_move(board, moves, move);
        } else {
            if (board->pieces[dest].color != piece.color && board->pieces[dest].type != KING) {
                Move move = move_create(piece, idx, dest, CAPTURE, NONE, board->pieces[dest].type);
                validate_and_push_move(board, moves, move);
            }
        }
    }

    // Castle
    bool castling_maybe_available = (board->first_king_move[piece.color] == 0)
        && ((board->first_king_rook_move[piece.color] == 0) || (board->first_queen_rook_move[piece.color] == 0));
    if (castling_maybe_available) {
        uint8_t attacked[64] = {0};
        size_t king_idx;
        generate_attacked(board, piece.color, attacked, &king_idx);
        assert(king_idx == idx);
        // if (board->first_king_move[piece.color] == 0) { // Always true since 
        if (board->first_king_rook_move[piece.color] == 0) {       
            size_t sq_1 = idx;
            size_t sq_2 = idx + 1;
            size_t sq_3 = idx + 2;
            size_t sq_4 = idx + 3;
            Piece rook = board->pieces[sq_4];
            if (rook.color == piece.color 
                && rook.type == ROOK 
                && !is_attacked(board, sq_1)
                && !is_attacked(board, sq_2)
                && !is_attacked(board, sq_3)
                && is_piece_null(board->pieces[sq_2])
                && is_piece_null(board->pieces[sq_3])) {
                Move move = move_create(piece, idx, sq_3, CASTLE, NONE, NONE);
                validate_and_push_move(board, moves, move);
            }
        }
        if (board->first_queen_rook_move[piece.color] == 0) {
            size_t sq_1 = idx;
            size_t sq_2 = idx - 1;
            size_t sq_3 = idx - 2;
            size_t sq_4 = idx - 3;
            size_t sq_5 = idx - 4;
            Piece rook = board->pieces[sq_5];
            if (rook.color == piece.color
                && rook.type == ROOK
                && !is_attacked(board, sq_1)
                && !is_attacked(board, sq_2)
                && !is_attacked(board, sq_3)
                && is_piece_null(board->pieces[sq_2])
                && is_piece_null(board->pieces[sq_3])
                && is_piece_null(board->pieces[sq_4])) {
                Move move = move_create(piece, idx, sq_3, CASTLE, NONE, NONE);
                validate_and_push_move(board, moves, move);
            }
        }
        // }
    }
    return false;
}

void generate_moves(Board *board, DAi32 *moves) {
    time_t start_time = time_now();

#if 1
    bool (*gen_funcs[6])(Board *, size_t, DAi32 *, bool) = {
        [PAWN] = generate_pawn_moves,
        [BISHOP] = generate_bishop_moves,
        [KNIGHT] = generate_knight_moves,
        [ROOK] = generate_rook_moves,
        [QUEEN] = generate_queen_moves,
    };
    PieceType piece_types[] = {
        PAWN,
        BISHOP,
        KNIGHT,
        ROOK,
        QUEEN
    };

    for (size_t i = 0; i < sizeof(piece_types) / sizeof(*piece_types); ++i) {
        PieceType piece_type = piece_types[i];
        
        uint64_t piece_bb = board->bb[piece_type][board->to_move];
        uint8_t piece_idx = 0;
        while (piece_bb) {
            uint8_t piece_idx_offset = next_piece_idx(piece_bb);
            piece_idx += piece_idx_offset;
            gen_funcs[piece_type](board, piece_idx, moves, false);
            piece_bb >>= piece_idx_offset + 1;
            piece_bb &= -(64ULL > (piece_idx_offset + 1));
            ++piece_idx;
        }
    }
#else   
    uint64_t pawn_bb = board->bb[PAWN][board->to_move];
    uint8_t pawn_idx = 0;
    while (pawn_bb) {
        uint8_t pawn_idx_offset = next_piece_idx(pawn_bb);
        pawn_idx += pawn_idx_offset;
        generate_pawn_moves(board, pawn_idx, moves, false);
        pawn_bb >>= pawn_idx_offset + 1;
        pawn_bb &= -(63ULL > pawn_idx_offset);
        ++pawn_idx;
    }
    uint64_t bishop_bb = board->bb[BISHOP][board->to_move];
    uint8_t bishop_idx = 0;
    while (bishop_bb) {
        uint8_t bishop_idx_offset = next_piece_idx(bishop_bb);
        bishop_idx += bishop_idx_offset;
        generate_bishop_moves(board, bishop_idx, moves, false);
        bishop_bb >>= bishop_idx_offset + 1;
        bishop_bb &= -(63ULL > bishop_idx_offset);
        ++bishop_idx;
    }
    uint64_t knight_bb = board->bb[KNIGHT][board->to_move];
    uint8_t knight_idx = 0;
    while (knight_bb) {
        uint8_t knight_idx_offset = next_piece_idx(knight_bb);
        knight_idx += knight_idx_offset;
        generate_knight_moves(board, knight_idx, moves, false);
        knight_bb >>= knight_idx_offset + 1;
        knight_bb &= -(63ULL > knight_idx_offset);
        ++knight_idx;
    }
    uint64_t rook_bb = board->bb[ROOK][board->to_move];
    uint8_t rook_idx = 0;
    while (rook_bb) {
        uint8_t rook_idx_offset = next_piece_idx(rook_bb);
        rook_idx += rook_idx_offset;
        generate_rook_moves(board, rook_idx, moves, false);
        rook_bb >>= rook_idx_offset + 1ULL;
        rook_bb &= -(63ULL > rook_idx_offset);
        ++rook_idx;
    }
    uint64_t queen_bb = board->bb[QUEEN][board->to_move];
    uint8_t queen_idx = 0;
    while (queen_bb) {
        uint8_t queen_idx_offset = next_piece_idx(queen_bb);
        queen_idx += queen_idx_offset;
        generate_queen_moves(board, queen_idx, moves, false);
        queen_bb >>= queen_idx_offset + 1;
        queen_bb &= -(63ULL > queen_idx_offset);
        ++queen_idx;
    }
#endif

    uint64_t king_bb = board->king_bb[board->to_move];
    uint8_t king_idx = next_piece_idx(king_bb);
    generate_king_moves(board, king_idx, moves, false);

    time_t end_time = time_now();
    board->time_to_generate_last_move_us = end_time - start_time;
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

void test_generate(void) {
    test_wrapper(test_generate_initial_moves);
}
