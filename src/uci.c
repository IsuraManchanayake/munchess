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
#include "utils.h"

char *position_parser = NULL;
UCI *uci = NULL;

UCI *uci_create(void) {
    UCI *uci = (UCI *) arena_allocate(&arena, sizeof(UCI));
    uci->state = UCI_NOT_READY;
    uci->engine = engine_create(send_info_score_cp);
    uci->board = board_create();
    uci->last_move = move_data_create(0);
    uci->log_fp = fopen("logs.txt", "a");
    uci->pid = getpid();
    return uci;
}

void move_to_uci(Move move, char *uci_move_str) {
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

bool match_cmd(char *input, const char *cmd) {
    return strncmp(input, cmd, strlen(cmd)) == 0;
}

void curr_time(char *buffer) {
    time_t timer;
    struct tm *tm_info;
    timer = time(NULL);
    tm_info = localtime(&timer);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
}

void _uci_log_base(const char *prefix, const char*fmt, va_list args) {
    char buffer[26];
    curr_time(buffer);

    fprintf(uci->log_fp, "%.2s %d %s ", prefix, getpid(), buffer);
    vfprintf(uci->log_fp, fmt, args);
    fprintf(uci->log_fp, "\n");
    fflush(uci->log_fp);
}

void uci_log(const char *prefix, const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    _uci_log_base(prefix, fmt, args);
    va_end(args);
}

void send_message(const char *fmt, ...) {
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
    _uci_log_base("< ", fmt, args_2);
    va_end(args_2);
}

void log_input(const char* input) {
    uci_log("> ", "%s\n", input);
}

void send_uci_ok() {
    send_message("id name %s", ENGINE_NAME);
    send_message("id author %s", ENGINE_AUTHOR);
    send_message("uciok");
}

void send_is_ready() {
    engine_start(uci->engine);
    if (uci->engine->state == ENGINE_READY) {
        uci->state = UCI_READY;
        send_message("readyok");
    }
}

void send_info_score_cp(Move move, size_t depth, int64_t cp) {
    char uci_move_str[6] = { 0 };
    move_to_uci(move, uci_move_str);
    send_message("info depth %zu score cp %d pv %s", depth, (int) cp, uci_move_str);
}

const char *uci_store_board(const char *fen) {
    dai32_free(uci->board->moves);
    board_reset(uci->board);
    return fen_to_board(fen, uci->board);
}

void parse_position_command(const char *input) {
    start_parsing(input);

    expect_str("position");
    skip_whitespace();

    if (soft_expect_str("fen")) {
        skip_whitespace();
        const char *current = uci_store_board(stream);
        advance(current - stream);
    }
    else if (soft_expect_str("startpos")) {
        uci_store_board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    }
    else {
        uci_log("##", "Expected fen or startpos but found %s", input);
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
            char uci_move_str[6] = { 0 };
            memcpy(uci_move_str, start, stream - start);

            Move move = uci_notation_to_move(uci_move_str, uci->board);
            apply_move(uci->board, move);

            skip_whitespace();
            if (*stream == 0) {
                break;
            }
        }
    }
    DA *da = da_create();
    char *fen = board_to_fen(uci->board, da);
    uci_log("**", "fen = %s", fen);
    da_free(da);
}

void send_best_move() {
    Move move = engine_best_move(uci->engine, uci->board);
    char uci_move_str[6] = { 0 };
    uci->last_move = move;
    move_to_uci(move, uci_move_str);
    apply_move(uci->board, move);
    send_message("bestmove %s", uci_move_str);

    uci_log("**", "elapsed = %zu us", uci->board->time_to_generate_last_move_us);
}

void start_uci(void) {
    uci = uci_create();

    char *input;
    while (true) {
        if ((input = read_line(stdin)) == NULL) {
            break;
        }
        log_input(input);

        if (match_cmd(input, "uci")) {
            send_uci_ok(uci);
        } else if (match_cmd(input, "isready")) {
            send_is_ready(uci);
        } else if (match_cmd(input, "ucinewgame")) {
            continue;
        } else if (match_cmd(input, "position")) {
            parse_position_command(input);
        } else if (match_cmd(input, "go")) {
            send_best_move(uci);
        } else if (match_cmd(input, "stop")) {
            char uci_move_str[6] = {0};
            move_to_uci(uci->last_move, uci_move_str);
            send_message("bestmove %s", uci_move_str);
            // When mult-threading, send the current best move.
            continue;
        } else if (match_cmd(input, "quit")) {
            break;
        }
        fflush(stdout);
        free(input);
    }
    send_message("Exiting.");
    fclose(uci->log_fp);
}
