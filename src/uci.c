#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include "uci.h"
#include "board.h"
#include "common.h"
#include "engine.h"
#include "defs.h"
#include "move.h"
#include "piece.h"
#include "parser.h"

char *position_parser = NULL;

UCI *uci_create(void) {
    UCI *uci = (UCI *) arena_allocate(&arena, sizeof(UCI));
    uci->state = UCI_NOT_READY;
    uci->engine = engine_create();
    uci->board = board_create();
    uci->last_move = move_data_create(0);
    uci->log_fp = fopen("/Users/imanchanayak/munchess/logs.txt", "a");
    return uci;
}

void move_to_uci(Move move, char* uci_move_str) {
    char from[] = IDX_TO_COORD(move.from);
    char to[] = IDX_TO_COORD(move.to);
    uci_move_str[0] = from[0];
    uci_move_str[1] = from[1];
    uci_move_str[2] = to[0];
    uci_move_str[3] = to[1];
    if (move_is_type_of(move, PROMOTION)) {
        uci_move_str[4] = piece_type_repr(move.promoted_type);
        uci_move_str[5] = 0;
    } else {
        uci_move_str[4] = 0;
    }
}

Move uci_to_move(const char* uci_move_str, Board *board) {
    size_t from = COORD_TO_IDX(uci_move_str);
    size_t to = COORD_TO_IDX(uci_move_str + 2);
    Piece* piece = &board->pieces[from];
    uint8_t move_type_mask = NORMAL;
    PieceType promoted_type = NONE;
    PieceType captured_type = board->pieces[to].type;

    size_t len = strlen(uci_move_str);
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
            promoted_type = char_to_piece_type(uci_move_str[4]);
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

bool match_cmd(char *input, const char *cmd) {
    return strncmp(input, cmd, strlen(cmd)) == 0;
}

void curr_time(char *buffer) {
    time_t timer;
    struct tm* tm_info;
    timer = time(NULL);
    tm_info = localtime(&timer);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
}

void _uci_log_base(UCI *uci, const char *prefix, const char*fmt, va_list args) {
    char buffer[26];
    curr_time(buffer);

    fprintf(uci->log_fp, "%.2s %s ", prefix, buffer);
    vfprintf(uci->log_fp, fmt, args);
    fprintf(uci->log_fp, "\n");
    fflush(uci->log_fp);
}

void uci_log(UCI *uci, const char *prefix, const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    _uci_log_base(uci, prefix, fmt, args);
    va_end(args);
}

void send_message(UCI *uci, const char *fmt, ...) {
    va_list args_1;

    va_start(args_1, fmt);
    vprintf(fmt, args_1);
    printf("\n");
    fflush(stdout);
    va_end(args_1);

    va_list args_2;

    char buffer[26];
    curr_time(buffer);

    va_start(args_2, fmt);
    _uci_log_base(uci, "< ", fmt, args_2);
    va_end(args_2);
}

void log_input(UCI *uci, const char *input) {
    uci_log(uci, "> ", "%s", input);
}

void send_uci_ok(UCI *uci) {
    send_message(uci, "id name %s", ENGINE_NAME);
    send_message(uci, "id author %s", ENGINE_AUTHOR);
    send_message(uci, "uciok");
}

void send_is_ready(UCI *uci) {
    engine_start(uci->engine);
    if (uci->engine->state == ENGINE_READY) {
        uci->state = UCI_READY;
        send_message(uci, "readyok");
    }
}

const char *uci_store_board(UCI *uci, const char *fen) {
    board_reset(uci->board);
    return fen_to_board(fen, uci->board);
}

void parse_position_command(UCI *uci, const char *input) {
    start_parsing(input);

    expect_str("position");
    skip_whitespace();

    if (soft_expect_str("fen")) {
        skip_whitespace();
        const char* current = uci_store_board(uci, stream);
        advance(current - stream);
    } else if (soft_expect_str("startpos")) {
        uci_store_board(uci, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    } else {
        uci_log(uci, "##", "Expected fen or startpos but found %s", input);
        exit(-1);
    }
    skip_whitespace();
    if (soft_expect_str("moves")) {
        skip_whitespace();
        while (*stream) {
            const char *start = stream;
            expect_range('a', 'h');
            expect_range('1', '8');
            expect_range('a', 'h');
            expect_range('1', '8');
            soft_expect_fn(is_promotable_piece);
            char uci_move_str[6] = {0};
            memcpy(uci_move_str, start, stream - start);

            Move move = uci_to_move(uci_move_str, uci->board);
            apply_move(uci->board, move);

            skip_whitespace();
            if (*stream == 0) {
                break;
            }
        }
    }
    DA *da = da_create();
    char *fen = board_to_fen(uci->board, da);
    uci_log(uci, "**", "fen = %s", fen);
    da_free(da);
}

void send_best_move(UCI *uci) {
    Move move = engine_best_move(uci->engine, uci->board);
    char uci_move_str[6] = {0};
    uci->last_move = move;
    move_to_uci(move, uci_move_str);
    apply_move(uci->board, move);
    send_message(uci, "bestmove %s", uci_move_str);
}

void start_uci(void) {
    UCI *uci = uci_create();

    char input[2048];
    while (true) {
        if (!fgets(input, sizeof(input), stdin)) {
            break;  // Handle EOF gracefully
        }
        log_input(uci, input);
        input[strcspn(input, "\n")] = 0;

        if (match_cmd(input, "uci")) {
            send_uci_ok(uci);
        } else if (match_cmd(input, "isready")) {
            send_is_ready(uci);
        } else if (match_cmd(input, "ucinewgame")) {
            continue;
        } else if (match_cmd(input, "position")) {
            parse_position_command(uci, input);
        } else if (match_cmd(input, "go")) {
            send_best_move(uci);
        } else if (match_cmd(input, "stop")) {
            char uci_move_str[6] = {0};
            move_to_uci(uci->last_move, uci_move_str);
            send_message(uci, "bestmove %s", uci_move_str);
            // When mult-threading, send the current best move.
            continue;
        } else if (match_cmd(input, "quit")) {
            break;
        }
        fflush(stdout);
    }
    send_message(uci, "Exiting.");
    fclose(uci->log_fp);
}
