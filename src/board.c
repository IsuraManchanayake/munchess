#include <assert.h>
#include <string.h>

#if _WIN32
#include <intrin.h>
#pragma intrinsic(_mm_popcnt_u64)
#endif

#include "board.h"
#include "common.h"
#include "defs.h"
#include "move.h"
#include "piece.h"
#include "utils.h"
#include "tests.h"

bool idx_is_safe(size_t idx) {
    return idx < 64;
}

bool yx_is_safe(size_t y, size_t x) {
    return y < 8 && x < 8;
}

Piece board_safe_at(const Board *board, size_t idx) {
    if (idx_is_safe(idx)) {
        return board->pieces[idx];
    }
    return (Piece) {.data=0};
}

void clear_square(Board *board, size_t idx) {
    if (board->pieces[idx].type == KING) {
        board->king_bb[board->pieces[idx].color] &= ~(1ULL << idx);
    }
    Piece piece = board->pieces[idx];
    board->bb[piece.type][piece.color] &= ~(1ULL << idx);
    board->pieces[idx].data = 0;
}

void set_piece_with(Board *board, size_t idx, Color color, PieceType type) {
    if (type == KING) {
        board->king_bb[color] = 1ULL << idx;
    }
    clear_square(board, idx);
    board->bb[type][color] |= 1ULL << idx;
    board->pieces[idx].data = 0;
    board->pieces[idx].color = color;
    board->pieces[idx].type = type;
}

void set_piece(Board *board, size_t idx, Piece piece) {
    set_piece_with(board, idx, piece.color, piece.type);
}

void board_reset(Board *board) {
    for (size_t i = 0; i < 64; ++i) {
        clear_square(board, i);
    }
    dai32_free(board->moves);
    board->moves = dai32_create();
    board->first_king_move[WHITE] = 0;
    board->first_king_move[BLACK] = 0;
    board->first_king_rook_move[WHITE] = 0;
    board->first_king_rook_move[BLACK] = 0;
    board->first_queen_rook_move[WHITE] = 0;
    board->first_queen_rook_move[BLACK] = 0;
    board->last_pawn_move[WHITE] = 0;
    board->last_pawn_move[BLACK] = 0;
    board->last_capture_move[WHITE] = 0;
    board->last_capture_move[BLACK] = 0;
    board->to_move = WHITE;
    board->initial_half_move_clock = 0;
    board->half_move_counter = 0;
    board->king_bb[WHITE] = 0;
    board->king_bb[BLACK] = 0;

    for (size_t i = 0; i < sizeof(board->bb) / sizeof(*board->bb); ++i) {
        for (size_t j = 0; j < 2; ++j) {
            board->bb[i][j] = 0;
        }
    }

    board->time_to_generate_last_move_us = 0;
    board->attacked = 0;
    board->attacked_evaluated = false;
}

Board *board_create(void) {
    Board *board = (Board *) arena_allocate(&arena, sizeof(Board));
    board->moves = dai32_create();
    board_reset(board);
    return board;
}

bool is_attacked(Board *board, size_t idx) {
    assert(board->attacked_evaluated);
    return board->attacked & (1ULL << idx);
}

void set_attacked(Board *board, size_t idx) {
    board->attacked |= (1ULL << idx);
}

void place_initial_pieces(Board *board) {
    for(size_t i = 0; i < 8; ++i) {
        set_piece_with(board, YX_TO_IDX(1, i), WHITE, PAWN);
        set_piece_with(board, YX_TO_IDX(6, i), BLACK, PAWN);
    }

    set_piece_with(board, COORD_TO_IDX("a1"), WHITE, ROOK);
    set_piece_with(board, COORD_TO_IDX("b1"), WHITE, KNIGHT);
    set_piece_with(board, COORD_TO_IDX("c1"), WHITE, BISHOP);
    set_piece_with(board, COORD_TO_IDX("d1"), WHITE, QUEEN);
    set_piece_with(board, COORD_TO_IDX("e1"), WHITE, KING);
    set_piece_with(board, COORD_TO_IDX("f1"), WHITE, BISHOP);
    set_piece_with(board, COORD_TO_IDX("g1"), WHITE, KNIGHT);
    set_piece_with(board, COORD_TO_IDX("h1"), WHITE, ROOK);

    set_piece_with(board, COORD_TO_IDX("a8"), BLACK, ROOK);
    set_piece_with(board, COORD_TO_IDX("b8"), BLACK, KNIGHT);
    set_piece_with(board, COORD_TO_IDX("c8"), BLACK, BISHOP);
    set_piece_with(board, COORD_TO_IDX("d8"), BLACK, QUEEN);
    set_piece_with(board, COORD_TO_IDX("e8"), BLACK, KING);
    set_piece_with(board, COORD_TO_IDX("f8"), BLACK, BISHOP);
    set_piece_with(board, COORD_TO_IDX("g8"), BLACK, KNIGHT);
    set_piece_with(board, COORD_TO_IDX("h8"), BLACK, ROOK);

    board->to_move = WHITE;
}

size_t get_king_idx(Board *board, Color color) {
#if _WIN32
    unsigned long index;
    unsigned char non_zero = _BitScanForward64(&index, board->king_bb[color]);
    assert(non_zero > 0);
    return index;
#else
    return __builtin_ctzll(board->king_bb[color]);
#endif
}

size_t count_pieces(Board *board, PieceType type, Color color) {
    uint64_t bb = board->bb[type][color];
#if _WIN32
    return _mm_popcnt_u64(bb);
#else
    return __builtin_popcountll(bb);
#endif
}

int move_direction(Color color) {
    return (int) 2 * (color == WHITE) - 1;
}

void apply_move_base(Board *board, Move move, bool invalidate_attacked) {
    if (move.piece_color != board->to_move) {
        assert(0);
    }
    // size_t move_n = board->moves->size + 1;
    Color color = move.piece_color;
    if (move_is_type_of(move, CASTLE)) {
        size_t rank = IDX_TO_RANK(move.from);
        char file = IDX_TO_FILE(move.to);
        if (file == 'g') {
            Piece rook = ATfr(board, 'h', rank);
            assert(rook.type == ROOK);
            set_piece(board, FR_TO_IDX('f', rank), rook);
            clear_square(board, FR_TO_IDX('h', rank));
        } else if (file == 'c') {
            Piece rook = ATfr(board, 'a', rank);
            assert(rook.type == ROOK);
            set_piece(board, FR_TO_IDX('d', rank), rook);
            clear_square(board, FR_TO_IDX('a', rank));
        } else {
            assert(0);
        }
    } else if (move_is_type_of(move, PROMOTION)) {
        move.piece_type = move.promoted_type;
    } else if (move_is_type_of(move, EN_PASSANT)) {
        int dir = move_direction(color);
        size_t to_y = IDX_Y(move.to);
        size_t to_x = IDX_X(move.to);
        clear_square(board, YX_TO_IDX(to_y - dir, to_x));
    }
    clear_square(board, move.from);
    set_piece_with(board, move.to, color, move.piece_type);
    if (move.piece_type == KING 
        && board->first_king_move[color] == 0) {
        board->first_king_move[color] = 1 + board->half_move_counter;
        if (move_is_type_of(move, CASTLE)) {
            char to_file = IDX_TO_FILE(move.to);
            if (to_file == 'g') {
                board->first_king_rook_move[color] = 1 + board->half_move_counter;
            } else if (to_file == 'c') {
                board->first_queen_rook_move[color] = 1 + board->half_move_counter;
            } else {
                assert(0);
            }
        }
    } else if (move.piece_type == ROOK) {
        size_t r = color == WHITE ? 1 : 8;
        if (board->first_king_rook_move[color] == 0 
            && move.from == FR_TO_IDX('H', r)) {
            board->first_king_rook_move[color] = 1 + board->half_move_counter;
        } else if (board->first_queen_rook_move[color] == 0 
            && move.from == FR_TO_IDX('A', r)) {
            board->first_queen_rook_move[color] = 1 + board->half_move_counter;
        }
    } else if (move.piece_type == PAWN) {
        board->last_pawn_move[color] = 1 + board->half_move_counter;
    }
    if (move_is_type_of(move, CAPTURE)) {
        board->last_capture_move[color] = 1 + board->half_move_counter;

        if (move.captured_type == ROOK) {
            size_t to = move.to;
            size_t last_y = move.piece_color == WHITE ? 7 : 0;
            Color op = op_color(move.piece_color);
            if (to == YX_TO_IDX(last_y, 0) && board->first_king_rook_move[op] == 0) {
                // King side rook capture
                board->first_king_rook_move[op] = 1 + board->half_move_counter;
            } else if (to == YX_TO_IDX(last_y, 7) && board->first_queen_rook_move[op] == 0) {
                // Queen side rook capture
                board->first_queen_rook_move[op] = 1 + board->half_move_counter;
            }
        }
    }
    board->to_move = op_color(color);
    ++board->half_move_counter;
    if (invalidate_attacked) {
        board->attacked_evaluated = false;
        board->attacked = 0;
    }
    dai32_push(board->moves, move.data);
}

void apply_move(Board *board, Move move) {
    apply_move_base(board, move, true);
}

void undo_last_move_base(Board *board, bool invalidate_attacked) {
    assert(board->moves->size > 0);
    Move move = move_data_create(board->moves->data[board->moves->size - 1]);
    const Color color = move.piece_color;
    if (move_is_type_of(move, CASTLE)) {
        size_t rank = IDX_TO_RANK(move.from);
        char file = IDX_TO_FILE(move.to);
        if (file == 'g') {
            Piece rook = ATfr(board, 'f', rank);
            assert(rook.type == ROOK);
            set_piece(board, FR_TO_IDX('h', rank), rook);
            clear_square(board, FR_TO_IDX('f', rank));
        } else if (file == 'c') {
            Piece rook = ATfr(board, 'd', rank);
            assert(rook.type == ROOK);
            set_piece(board, FR_TO_IDX('a', rank), rook);
            clear_square(board, FR_TO_IDX('d', rank));
        } else {
            assert(0);
        }
    } else if (move_is_type_of(move, PROMOTION)) {
        move.piece_type = PAWN;
    } else if (move_is_type_of(move, EN_PASSANT)) {
        int dir = move_direction(color);
        // unsigned from_y = IDX_Y(move.from);
        size_t to_y = IDX_Y(move.to);
        size_t to_x = IDX_X(move.to);
        set_piece_with(board, YX_TO_IDX(to_y - dir, to_x), op_color(color), PAWN);
    }
    set_piece_with(board, move.from, color, move.piece_type);
    if (move_is_type_of(move, CAPTURE)) {
        set_piece_with(board, move.to, op_color(color), move.captured_type);        bool found = false;
        for (size_t i = board->moves->size - 1; i-- > 0;) {
            Move tmove = move_data_create(board->moves->data[i]);
            if (tmove.piece_color == color && move_is_type_of(tmove, CAPTURE)) {
                found = true;
                board->last_capture_move[color] = board->half_move_counter - 1 - (board->moves->size - 1 - i);
                break;
            }
        }
        if (!found) {
            board->last_capture_move[color] = board->half_move_counter - board->moves->size - board->initial_half_move_clock;
        }

        if (move.captured_type == ROOK) {
            size_t to = move.to;
            size_t last_y = move.piece_color == WHITE ? 7 : 0;
            Color op = op_color(move.piece_color);

            if (to == YX_TO_IDX(last_y, 0) && board->first_king_rook_move[op] == board->half_move_counter) {
                board->first_king_rook_move[op] = 0;
            } else if (to == YX_TO_IDX(last_y, 7) && board->first_queen_rook_move[op] == board->half_move_counter) {
                board->first_queen_rook_move[op] = 0;
            }
        }
    } else {
        clear_square(board, move.to);
    }
    if (move.piece_type == KING) {
        if (board->first_king_move[color] == board->half_move_counter) {
            board->first_king_move[color] = 0;
            if (move_is_type_of(move, CASTLE)) {
                char to_file = IDX_TO_FILE(move.to);
                if (to_file == 'g') {
                    board->first_king_rook_move[color] = 0;
                } else if (to_file == 'c') {
                    board->first_queen_rook_move[color] = 0;
                }
            }
        }
    } else if (move.piece_type == ROOK) {
        size_t r = color == WHITE ? 1 : 8;
        if (move.from == FR_TO_IDX('H', r) && board->first_king_rook_move[color] == board->half_move_counter) {
            board->first_king_rook_move[color] = 0;
        } else if (move.from == FR_TO_IDX('A', r) && board->first_queen_rook_move[color] == board->half_move_counter) {
            board->first_queen_rook_move[color] = 0;
        }
    }
    if (move.piece_type == PAWN) {
        bool found = false;
        for (size_t i = board->moves->size - 1; i-- > 0;) {
            Move tmove = move_data_create(board->moves->data[i]);
            if (tmove.piece_color == color && tmove.piece_type == PAWN) {
                found = true;
                board->last_pawn_move[color] = board->half_move_counter - (board->moves->size - 1 - i);
                break;
            }
        }
        if (!found) {
            board->last_pawn_move[color] = board->half_move_counter - board->moves->size - board->initial_half_move_clock;
        }
    }
    board->to_move = color;
    --board->half_move_counter;
    if (invalidate_attacked) {
        board->attacked = 0;
        board->attacked_evaluated = false;
    }
    (void) dai32_pop(board->moves);
}

void undo_last_move(Board *board) {
    undo_last_move_base(board, true);
}

size_t n_moves_since_last_pawn_or_capture_move(Board *board) {
    size_t last_pawn_move = max(board->last_pawn_move[WHITE], board->last_pawn_move[BLACK]);
    size_t last_capture_move = max(board->last_capture_move[WHITE], board->last_capture_move[BLACK]);
    size_t last_pawn_or_capture_move = max(last_pawn_move, last_capture_move);
    size_t moves_since_last_pawn_or_capture_move = board->half_move_counter - last_pawn_or_capture_move;
    return moves_since_last_pawn_or_capture_move;
}

char *board_to_fen(Board *board, DA *da) {
    for (size_t y = 8; y-- > 0;) {
        size_t null_counter = 0;
        for (size_t x = 0; x < 8; ++x) {
            Piece piece = ATyx(board, y, x);
            if (!is_piece_null(piece)) {
                if (null_counter > 0) {
                    buf_printf(da, "%zu", null_counter);
                    null_counter = 0;
                }
                buf_printf(da, "%c", piece_repr(piece));
            } else {
                ++null_counter;
            }
        }
        if (null_counter > 0) {
            buf_printf(da, "%zu", null_counter);
        }
        if (y > 0) {
            buf_printf(da, "/", 0);
        }
    }
    buf_printf(da, " %c ", color_repr(board->to_move));
    bool no_castling = true;
    if (board->first_king_move[WHITE] == 0) {
        if (board->first_king_rook_move[WHITE] == 0) {
            buf_printf(da, "K", 0);
            no_castling = false;
        }
        if (board->first_queen_rook_move[WHITE] == 0) {
            buf_printf(da, "Q", 0);
            no_castling = false;
        }
    }
    if (board->first_king_move[BLACK] == 0) {
        if (board->first_king_rook_move[BLACK] == 0) {
            buf_printf(da, "k", 0);
            no_castling = false;
        }
        if (board->first_queen_rook_move[BLACK] == 0) {
            buf_printf(da, "q", 0);
            no_castling = false;
        }
    }
    if (no_castling) {
        buf_printf(da, "-", 0);
    }
    buf_printf(da, " ", 0);
    bool en_passant = false;
    if (board->moves->size > 0) {
        Move move = move_data_create(*dai32_last_elem(board->moves));
        if (move.piece_type == PAWN) {
            size_t start_y = IDX_Y(move.from);
            size_t dest_y = IDX_Y(move.to);
            size_t x = IDX_X(move.from);
            int dir = move_direction(move.piece_color);
            if (dest_y == 2 * dir + start_y) {
                en_passant = true;
                size_t en_passant_target_y = start_y + dir;
                const char en_passant_target_sq[] = YX_TO_COORD(en_passant_target_y, x);
                buf_printf(da, "%s", en_passant_target_sq);
            }
        }
    }
    if (!en_passant) {
        buf_printf(da, "-", 0);
    }
    
    size_t moves_since_last_pawn_or_capture_move = n_moves_since_last_pawn_or_capture_move(board);
    size_t full_move_counter = 1 + board->half_move_counter / 2;

    buf_printf(da, " %zu %zu", moves_since_last_pawn_or_capture_move, full_move_counter);
    return (char *) da->data;
}

const char *fen_to_board(const char *fen, Board *board) {
    board_reset(board);
    size_t idx = 0;
    while (*fen) {
        if ('1' <= *fen && *fen <= '8') {
            size_t n = (*fen - '0');
            for (size_t i = 0; i < n; ++i) {
                clear_square(board, (7 - idx / 8) * 8 + idx % 8);
                ++idx;
            }
        } else if (*fen == '/') {
            assert(idx % 8 == 0);
        } else {
            Piece piece = char_to_piece(*fen);
            if (piece.type != NONE) {
                set_piece(board, (7 - idx / 8) * 8 + idx % 8, piece);
                ++idx;
            } else {
                assert(0);
            }
        }
        ++fen;
        if (idx >= 64) {
            break;
        }
    }
    while (*fen == ' ') {
        ++fen;
    }
    if (*fen == 'b') {
        board->to_move = BLACK;
    } else if (*fen == 'w') {
        board->to_move = WHITE;
    } else {
        assert(0);
    }
    ++fen;
    while (*fen == ' ') {
        ++fen;
    }
    bool w_king_castle = false;
    bool w_queen_castle = false;
    bool b_king_castle = false;
    bool b_queen_castle = false;
    const char *start = fen;
    while (*fen && *fen != ' ') {
        ++fen;
    }
    for (const char *c = start; c != fen; ++c) {
        if (*c == 'K') {
            w_king_castle = true;
        } else if (*c == 'Q') {
            w_queen_castle = true;
        } else if (*c == 'k') {
            b_king_castle = true;
        } else if (*c == 'q') {
            b_queen_castle = true;
        } else if (*c == '-') {
            continue;
        } else {
            assert(0);
        }
    }
    while (*fen == ' ') {
        ++fen;
    }
    if (*fen == '-') {
        ++fen;
    } else if ('a' <= *fen && *fen <= 'h') {
        ++fen;
        if ('1' <= *fen && *fen <= '8') {
            size_t f = *(fen - 1);
            int dir = move_direction(board->to_move);
            size_t r = *fen - '0';
            size_t from = FR_TO_IDX(f, r + dir);
            size_t to = FR_TO_IDX(f, r - dir);
            Piece piece = piece_create(op_color(board->to_move), PAWN);
            Move move = move_create(piece, from, to, NORMAL, NONE, NONE);
            dai32_push(board->moves, move.data);
            ++fen;
        } else {
            assert(0);
        }
    } else {
        assert(0);
    }
    while (*fen == ' ') {
        ++fen;
    }
    size_t half_move_clock = 0;
    while (*fen && '0' <= *fen && *fen <= '9') {
        half_move_clock = half_move_clock * 10 + (*fen - '0');
        ++fen;
    }
    while (*fen == ' ') {
        ++fen;
    }
    size_t full_move_counter = 0;
    while (*fen && '0' <= *fen && *fen <= '9') {
        full_move_counter = full_move_counter * 10 + (*fen - '0');
        ++fen; 
    }

    board->initial_half_move_clock = half_move_clock;
    board->half_move_counter = (full_move_counter - 1) * 2 + (board->to_move == BLACK ? 1 : 0);
    board->first_king_move[WHITE] = (w_king_castle || w_queen_castle) ? 0 : board->half_move_counter;
    board->first_king_move[BLACK] = b_king_castle ? 0 : board->half_move_counter;
    board->first_king_rook_move[WHITE] = w_king_castle ? 0 : board->half_move_counter;
    board->first_king_rook_move[BLACK] = b_king_castle ? 0 : board->half_move_counter;
    board->first_queen_rook_move[WHITE] = w_queen_castle ? 0 : board->half_move_counter;
    board->first_queen_rook_move[BLACK] = b_queen_castle ? 0 : board->half_move_counter;
    board->last_pawn_move[WHITE] = board->half_move_counter - board->initial_half_move_clock;
    board->last_pawn_move[BLACK] = board->half_move_counter - board->initial_half_move_clock;
    board->last_capture_move[WHITE] = board->half_move_counter - board->initial_half_move_clock;
    board->last_capture_move[BLACK] = board->half_move_counter - board->initial_half_move_clock;
    return fen;
}

void generate_moves(Board *board, DAi32 *moves);

Move san_notation_to_move(const char *notation, Board *board) {
    // TODO: Improve notation_to_move performance
    DAi32 *moves = dai32_create();
    Move rmove = (Move) {0};
    Color color = board->to_move;

    generate_moves(board, moves);
    // printf("Time for generating moves: %zu us\n", board->time_to_generate_last_move_us);

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
            if (move.piece_color == color
                && move.piece_type == KING
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
            if (move.piece_color == color
                && move.piece_type == KING
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
            if (move.piece_color == color 
                && move.piece_type == KING 
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
            if (move.piece_color != color) {
                continue;
            }
            if (move.piece_type != piece_type) {
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
            char file = c[0];
            size_t dest = COORD_TO_IDX(c + len - 2);
            for (size_t i = 0; i < moves->size; ++i) {
                Move move = move_data_create(moves->data[i]);
                if (move.piece_color == color
                    && move.piece_type ==  PAWN
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

Move uci_notation_to_move(const char *move_str, Board* board) {
    size_t from = COORD_TO_IDX(move_str);
    size_t to = COORD_TO_IDX(move_str + 2);
    Piece* piece = &board->pieces[from];
    uint8_t move_type_mask = NORMAL;
    PieceType promoted_type = NONE;
    PieceType captured_type = board->pieces[to].type;

    size_t len = strlen(move_str);
    if (!is_piece_null(board->pieces[to])) {
        move_type_mask |= CAPTURE;
    }
    int dir = move_direction(board->to_move);
    if (piece->type == PAWN) {
        if (board->moves->size > 0) {
            Move last_move = move_data_create(board->moves->data[board->moves->size - 1]);
            if (last_move.piece_type == PAWN) {
                size_t last_move_from_y = IDX_Y(last_move.from);
                size_t last_move_to_y = IDX_Y(last_move.to);
                size_t last_move_x = IDX_X(last_move.from);
                if (last_move_to_y + 2 * dir == last_move_from_y
                    && YX_TO_IDX(last_move_from_y - dir, last_move_x) == to) {
                    move_type_mask |= EN_PASSANT;
                }
            }
        }
        // Promotion
        if (len == 5) {
            move_type_mask |= PROMOTION;
            promoted_type = char_to_piece_type(move_str[4]);
            assert(IDX_Y(to) == (7 + 7 * dir) / 2);
        }
    }
    if (piece->type == KING) {
        if (from - to == 2 || to - from == 2) {
            move_type_mask |= CASTLE;
            assert(IDX_Y(from) == IDX_Y(to));
            assert(IDX_Y(from) == (7 - 7 * dir) / 2);
        }
    }
    return move_create(
        *piece,
        from,
        to,
        move_type_mask,
        promoted_type,
        captured_type
    );
}

char *board_buf_write(Board *board, DA *da) {
    buf_printf(da, "   +", 0);
    for(size_t i = 0; i < 8; ++i) {
        buf_printf(da, "---+", 0);
    }
    for(size_t i = 8; i-- > 0;) {
        buf_printf(da, "\n %zu |", i + 1);
        for (size_t j = 0; j < 8; ++j) {
            buf_printf(da, " %c |", piece_repr(ATyx(board, i, j)));
        }
        buf_printf(da, "\n   +", 0);
        for (size_t j = 0; j < 8; ++j) {
            buf_printf(da, "---+", 0);
        }
    }
    buf_printf(da, "\n    ", 0);
    for(size_t i = 0; i < 8; ++i) {
        buf_printf(da, " %c  ", i + 'a');
    }
    buf_printf(da, "\n    ", 0);
    for(size_t i = 0; i < 8; ++i) {
        buf_printf(da, "    ", 0);
    }
    return (char *)da->data;
}

void print_board(Board *board) {
    DA *da = da_create();
    char *board_str = board_buf_write(board, da);
    printf("%s\n", board_str);
    da_free(da);
}

void print_bb(Board *board, PieceType type, Color color) {
    uint64_t bb = type == KING ? board->king_bb[color] : board->bb[type][color];
    for (size_t i = 0; i < 64; ++i) {
        size_t y = IDX_Y(i);
        size_t x = IDX_X(i);
        uint8_t pos = YX_TO_IDX(7 - y, x);
        uint8_t bit = (bb >> pos) & 1;
        char c = bit ? piece_repr_base(color, type) : '-';
        char e = (x == 7 ? '\n' : ' ');
        printf("%c%c", c, e);
    }
    printf("\n");
}

void print_fen(Board* board) {
    DA *da = da_create();
    char *fen = board_to_fen(board, da);
    printf("%s\n", fen);
    da_free(da);
}

// ===========================================

void test_board_initial(void) {
    Board *board = board_create();
    place_initial_pieces(board);
    assert(ATyx(board, 0, 0).color == WHITE);
    assert(ATyx(board, 0, 0).type == ROOK);
    assert(ATcoord(board, "C2").type == PAWN);
    assert(ATcoord(board, "C2").color == WHITE);
    assert(ATcoord(board, "E8").type == KING);
    assert(ATcoord(board, "E8").color == BLACK);
    assert(is_piece_null(ATcoord(board, "C6")));
    
    assert(count_pieces(board, PAWN, WHITE) == 8);
    assert(count_pieces(board, PAWN, BLACK) == 8);
    assert(count_pieces(board, BISHOP, WHITE) == 2);
    assert(count_pieces(board, BISHOP, BLACK) == 2);
    assert(count_pieces(board, KNIGHT, WHITE) == 2);
    assert(count_pieces(board, KNIGHT, BLACK) == 2);
    assert(count_pieces(board, ROOK, WHITE) == 2);
    assert(count_pieces(board, ROOK, BLACK) == 2);
    assert(count_pieces(board, QUEEN, WHITE) == 1);
    assert(count_pieces(board, QUEEN, BLACK) == 1);
    assert(count_pieces(board, KING, WHITE) == 1);
    assert(count_pieces(board, KING, BLACK) == 1);
}

void test_board_display(void) {
    Board *board = board_create();
    place_initial_pieces(board);
    DA *da = da_create();
    char *repr = board_buf_write(board, da);
    da_free(da);
    // printf("%s\n", repr);
    (void) repr;
}

void test_apply_move(void) {
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
    Move move_5 = move_create(ATcoord(board, "F3"), COORD_TO_IDX("F3"), COORD_TO_IDX("E5"), CAPTURE, NONE, NONE);
    apply_move(board, move_5);
    Move move_6 = move_create(ATcoord(board, "C6"), COORD_TO_IDX("C6"), COORD_TO_IDX("E5"), CAPTURE, NONE, NONE);
    apply_move(board, move_6);

    DA *da = da_create();
    char *repr = board_buf_write(board, da);
    // printf("%s\n", repr);
    const char* const expected = 
"   +---+---+---+---+---+---+---+---+\n"
" 8 | r |   | b | q | k | b | n | r |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 7 | p | p | p | p |   | p | p | p |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 6 |   |   |   |   |   |   |   |   |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 5 |   |   |   |   | n |   |   |   |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 4 |   |   |   |   | P |   |   |   |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 3 |   |   |   |   |   |   |   |   |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 2 | P | P | P | P |   | P | P | P |\n"
"   +---+---+---+---+---+---+---+---+\n"
" 1 | R | N | B | Q | K | B |   | R |\n"
"   +---+---+---+---+---+---+---+---+\n"
"     a   b   c   d   e   f   g   h  \n"
"                                    ";
    assert(strcmp(repr, expected) == 0);
    da_free(da);
}

void test_undo_last_move(void) {
    Board *board = board_create();
    place_initial_pieces(board);

    const Move moves[] = {
        move_create(ATcoord(board, "E2"), COORD_TO_IDX("E2"), COORD_TO_IDX("E4"), NORMAL, NONE, NONE),
        move_create(ATcoord(board, "C7"), COORD_TO_IDX("C7"), COORD_TO_IDX("C5"), NORMAL, NONE, NONE),
        move_create(ATcoord(board, "G1"), COORD_TO_IDX("G1"), COORD_TO_IDX("F3"), NORMAL, NONE, NONE),
    };

    for (size_t i = 0; i < sizeof(moves) / sizeof(moves[0]); ++i) {
        apply_move(board, moves[i]);
    }

    const char *fens[] = {
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2",
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    };

    for (size_t i = 0; i < sizeof(moves) / sizeof(moves[0]); ++i) {
        undo_last_move(board);
        DA *da = da_create();
        char *fen = board_to_fen(board, da);
        // debugs(fen);
        assert(strcmp(fen, fens[i]) == 0);
    }
}

void test_board_to_fen(void) {
    Board *board = board_create();
    place_initial_pieces(board);
    DA *da_1 = da_create();
    char *fen_1 = board_to_fen(board, da_1);
    // printf("%s\n", fen_1);
    const char *expected_1 = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    assert(strcmp(expected_1, fen_1) == 0);
    
    Move move_1 = move_create(ATcoord(board, "E2"), COORD_TO_IDX("E2"), COORD_TO_IDX("E4"), NORMAL, NONE, NONE);
    apply_move(board, move_1);
    DA *da_2 = da_create();
    char *fen_2 = board_to_fen(board, da_2);
    // printf("%s\n", fen_2);
    const char *expected_2 = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
    assert(strcmp(expected_2, fen_2) == 0);

    Move move_2 = move_create(ATcoord(board, "C7"), COORD_TO_IDX("C7"), COORD_TO_IDX("C5"), NORMAL, NONE, NONE);
    apply_move(board, move_2);
    DA *da_3 = da_create();
    char *fen_3 = board_to_fen(board, da_3);
    // printf("%s\n", fen_3);
    const char *expected_3 = "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2";
    assert(strcmp(expected_3, fen_3) == 0);

    Move move_3 = move_create(ATcoord(board, "G1"), COORD_TO_IDX("G1"), COORD_TO_IDX("F3"), NORMAL, NONE, NONE);
    apply_move(board, move_3);
    DA *da_4 = da_create();
    char *fen_4 = board_to_fen(board, da_4);
    // printf("%s\n", fen_4);
    const char *expected_4 = "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2";
    assert(strcmp(expected_4, fen_4) == 0);

    da_free(da_1);
    da_free(da_2);
    da_free(da_3);
    da_free(da_4);
}

void test_fen_to_board(void) {
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
    DA *fens_da = da_create();
    for (size_t i = 0; i < sizeof(moves) / sizeof(moves[0]); ++i) {
        Move move = san_notation_to_move(moves[i], board);
        apply_move(board, move);
        DA *fen_da = da_create();
        board_to_fen(board, fen_da);
        da_push(fens_da, (void *) strdup((char *)fen_da->data));
        da_free(fen_da);
    }

    for (size_t i = 8; i < fens_da->size; ++i) {
        char *original_fen = (char *) fens_da->data[i];
        // debugs(original_fen);
        fen_to_board(original_fen, board);

        DA *fen_da =  da_create();
        char *derived_fen = board_to_fen(board, fen_da);
        // debugs(derived_fen);
        assert(strcmp(original_fen, derived_fen) == 0);
        da_free(fen_da);

        for (size_t j = i + 1; j < sizeof(moves) / sizeof(moves[0]); ++j) {
            Move move = san_notation_to_move(moves[j], board);
            apply_move(board, move);
        }
        
        DA *fen_da_2 =  da_create();
        char *derived_fen_2 = board_to_fen(board, fen_da_2);
        // debugs((char *) fens_da->data[fens_da->size - 1]);
        // debugs(derived_fen_2);
        assert(strcmp(fens_da->data[fens_da->size - 1], derived_fen_2) == 0);
        da_free(fen_da_2);
    }

    da_free(fens_da);
    
    assert(count_pieces(board, PAWN, WHITE) == 8);
    assert(count_pieces(board, PAWN, BLACK) == 8);
    assert(count_pieces(board, BISHOP, WHITE) == 2);
    assert(count_pieces(board, BISHOP, BLACK) == 2);
    assert(count_pieces(board, KNIGHT, WHITE) == 2);
    assert(count_pieces(board, KNIGHT, BLACK) == 2);
    assert(count_pieces(board, ROOK, WHITE) == 2);
    assert(count_pieces(board, ROOK, BLACK) == 2);
    assert(count_pieces(board, QUEEN, WHITE) == 1);
    assert(count_pieces(board, QUEEN, BLACK) == 1);
    assert(count_pieces(board, KING, WHITE) == 1);
    assert(count_pieces(board, KING, BLACK) == 1);
}

void test_san_notation_to_move(void) {
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
        Move move = san_notation_to_move(moves[i], board);
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

void test_uci_notation_to_move(void) {
    char* seq[] = {
        "e2e3", "a7a6", "h2h4", "b7b5", "g2g3", "b8c6", "h1h2", "e7e5", "d1f3",
        "a8a7", "f3f6", "c6e7", "c2c4", "b5c4", "f6d6", "g7g6", "d6c5", "h7h6",
        "c5c7", "h8h7", "g1f3", "d7d6", "b1a3", "h7g7", "c7a7", "d8c7", "d2d4",
        "e5e4", "f3e5", "a6a5", "e5c4", "e7c6"
    };
    Board* board = board_create();
    place_initial_pieces(board);
    for (size_t i = 0; i < sizeof(seq) / sizeof(seq[0]); ++i) {
        Move move = uci_notation_to_move(seq[i], board);
        apply_move(board, move);

        //DA *board_da = da_create();
        //char *board_buf = board_buf_write(board, board_da);
        //printf("%s\n", board_buf);
        //da_free(board_da);
    }

    DAi32* moves = dai32_create();
    generate_moves(board, moves);

    //print_board(board);
    //print_fen(board);

    //for (size_t i = 0; i < moves->size; ++i) {
    //	Move move = move_data_create(moves->data[i]);

    //    print_move(move);
    //}
    assert(moves->size == 35);

    (void)seq;
}

void test_board(void) {
    test_wrapper(test_board_initial);
    test_wrapper(test_board_display);
    test_wrapper(test_apply_move);
    test_wrapper(test_undo_last_move);
    test_wrapper(test_board_to_fen);
    test_wrapper(test_fen_to_board);
    test_wrapper(test_san_notation_to_move);
    test_wrapper(test_uci_notation_to_move);
}
