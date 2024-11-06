#include <assert.h>
#include <string.h>

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

void set_piece_null(Piece *piece) {
    piece->data = 0;
}

Board *board_create(void) {
    Board *board = (Board *)arena_allocate(&arena, sizeof(Board));
    for(size_t i = 0; i < 8; ++i) {
        for(size_t j = 0; j < 8; ++j) {
            set_piece_null(&ATyx(board, i, j));
        }
    }
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
    return board;
}

void place_initial_pieces(Board *board) {
    for(size_t i = 0; i < 8; ++i) {
        ATyx(board, 1, i) = piece_create(WHITE, PAWN);
        ATyx(board, 6, i) = piece_create(BLACK, PAWN);
    }

    ATcoord(board, "A1") = piece_create(WHITE, ROOK);
    ATcoord(board, "B1") = piece_create(WHITE, KNIGHT);
    ATcoord(board, "C1") = piece_create(WHITE, BISHOP);
    ATcoord(board, "D1") = piece_create(WHITE, QUEEN);
    ATcoord(board, "E1") = piece_create(WHITE, KING);
    ATcoord(board, "F1") = piece_create(WHITE, BISHOP);
    ATcoord(board, "G1") = piece_create(WHITE, KNIGHT);
    ATcoord(board, "H1") = piece_create(WHITE, ROOK);

    ATcoord(board, "A8") = piece_create(BLACK, ROOK);
    ATcoord(board, "B8") = piece_create(BLACK, KNIGHT);
    ATcoord(board, "C8") = piece_create(BLACK, BISHOP);
    ATcoord(board, "D8") = piece_create(BLACK, QUEEN);
    ATcoord(board, "E8") = piece_create(BLACK, KING);
    ATcoord(board, "F8") = piece_create(BLACK, BISHOP);
    ATcoord(board, "G8") = piece_create(BLACK, KNIGHT);
    ATcoord(board, "H8") = piece_create(BLACK, ROOK);

    board->to_move = WHITE;
}

int move_direction(Color color) {
    return (int) 2 * (color == WHITE) - 1;
}

void apply_move(Board *board, Move move) {
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
            ATfr(board, 'f', rank) = rook;
            set_piece_null(&ATfr(board, 'h', rank));
        } else if (file == 'c') {
            Piece rook = ATfr(board, 'a', rank);
            assert(rook.type == ROOK);
            ATfr(board, 'd', rank) = rook;
            set_piece_null(&ATfr(board, 'a', rank));
        } else {
            assert(0);
        }
    } else if (move_is_type_of(move, PROMOTION)) {
        move.piece_type = move.promoted_type;
    } else if (move_is_type_of(move, EN_PASSANT)) {
        int dir = move_direction(color);
        size_t to_y = IDX_Y(move.to);
        size_t to_x = IDX_X(move.to);
        set_piece_null(&ATyx(board, to_y - dir, to_x));
    }
    set_piece_null(&ATidx(board, move.from));
    Piece *piece_to = &ATidx(board, move.to);
    piece_to->color = color;
    piece_to->type = move.piece_type;
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
    }
    board->to_move = op_color(color);
    ++board->half_move_counter;
    dai32_push(board->moves, move.data);
}

void set_square_empty(Board *board, size_t idx) {
    set_piece_null(board->pieces + idx);
}

void undo_last_move(Board *board) {
    assert(board->moves->size > 0);
    Move move = move_data_create(board->moves->data[board->moves->size - 1]);
    const Color color = move.piece_color;
    if (move_is_type_of(move, CASTLE)) {
        size_t rank = IDX_TO_RANK(move.from);
        char file = IDX_TO_FILE(move.to);
        if (file == 'g') {
            Piece rook = ATfr(board, 'f', rank);
            assert(rook.type == ROOK);
            ATfr(board, 'h', rank) = rook;
            set_square_empty(board, FR_TO_IDX('f', rank));
        } else if (file == 'c') {
            Piece rook = ATfr(board, 'd', rank);
            assert(rook.type == ROOK);
            ATfr(board, 'a', rank) = rook;
            set_square_empty(board, FR_TO_IDX('d', rank));
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
        ATyx(board, to_y - dir, to_x) = piece_create(op_color(color), PAWN);
    }
    ATidx(board, move.from) = piece_create(color, move.piece_type);
    if (move_is_type_of(move, CAPTURE)) {
        ATidx(board, move.to) = piece_create(op_color(color), move.captured_type);
        bool found = false;
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
    } else {
        set_square_empty(board, move.to);
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
    (void) dai32_pop(board->moves);
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
    size_t last_pawn_move = max(board->last_pawn_move[WHITE], board->last_pawn_move[BLACK]);
    size_t last_capture_move = max(board->last_capture_move[WHITE], board->last_capture_move[WHITE]);
    size_t last_pawn_or_capture_move = max(last_pawn_move, last_capture_move);
    size_t moves_since_last_pawn_or_capture_move = board->half_move_counter - last_pawn_or_capture_move;
   
    size_t full_move_counter = 1 + board->half_move_counter / 2;
    buf_printf(da, " %zu %zu", moves_since_last_pawn_or_capture_move, full_move_counter);
    return (char *) da->data;
}

void fen_to_board(char *fen, Board *board) {
    board->moves->size = 0;
    size_t idx = 0;
    while (*fen) {
        if ('1' <= *fen && *fen <= '8') {
            size_t n = (*fen - '0');
            for (size_t i = 0; i < n; ++i) {
                set_piece_null(board->pieces + (7 - idx / 8) * 8 + idx % 8);
                ++idx;
            }
        } else if (*fen == '/') {
            assert(idx % 8 == 0);
        } else {
            Piece piece = char_to_piece(*fen);
            if (piece.type != NONE) {
                board->pieces[(7 - idx / 8) * 8 + idx % 8] = piece;
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
    char *start = fen;
    while (*fen && *fen != ' ') {
        ++fen;
    }
    for (char *c = start; c != fen; ++c) {
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
}

void generate_moves(Board *board, DAi32 *moves);

Move notation_to_move(const char *notation, Board *board) {
    // TODO: Improve notation_to_move performance
    DAi32 *moves = dai32_create();
    Move rmove = (Move) {0};
    Color color = board->to_move;

    time_t start_time = time_now();
    generate_moves(board, moves);
    time_t end_time = time_now();
    double diff_ms = (end_time - start_time) / 1000.0;
    (void) diff_ms;
    //printf("Time for generating moves: %f ms\n", diff_ms);

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
    const char expected[] = 
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

    Move moves[] = {
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
        Move move = notation_to_move(moves[i], board);
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
            Move move = notation_to_move(moves[j], board);
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

void test_board(void) {
    test_wrapper(test_board_initial);
    test_wrapper(test_board_display);
    test_wrapper(test_apply_move);
    test_wrapper(test_undo_last_move);
    test_wrapper(test_board_to_fen);
    test_wrapper(test_fen_to_board);
    test_wrapper(test_notation_to_move);
}
