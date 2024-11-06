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
    uci_move_str[4] = '\0';
}

Move uci_to_move(char* uci, Board *board) {
    size_t from = COORD_TO_IDX(uci);
    size_t to = COORD_TO_IDX(uci + 2);
    Piece* piece = &board->pieces[from];
    uint8_t move_type_mask = NORMAL;
    PieceType promoted_type = NONE;
    PieceType captured_type = board->pieces[to].type;

    size_t len = strlen(uci);
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
            promoted_type = char_to_piece_type(uci[4]);
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
    fprintf(uci->log_fp, "<  %s ", buffer);
    vfprintf(uci->log_fp, fmt, args_2);
    fprintf(uci->log_fp, "\n");
    fflush(uci->log_fp);
    va_start(args_2, end);
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

void store_board(UCI *uci, char *fen) {
    fen_to_board(fen, uci->board);
}


void process_position_command(UCI *uci, char *input) {
    char *token = strtok(input, " ");
    token = strtok(NULL, " ");
    
    if (match_cmd(token, "fen")) {
        char fen[100] = {0};
        token = strtok(NULL, " ");
        while (token && !match_cmd(token, "moves")) {
            strcat(fen, token);
            strcat(fen, " ");
            token = strtok(NULL, " ");
        }
        if (strlen(fen) > 0) {
            fen[strlen(fen) - 1] = '\0';
            store_board(uci, fen);
        }
    } else if (match_cmd(token, "startpos")) {
        store_board(uci, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    }
    
    token = strtok(NULL, " ");
    while (token != NULL) {
        if (strlen(token) >= 4) {
            token[4] = '\0';  // Ensure the move string is properly terminated
            Move move = uci_to_move(token, uci->board);
            apply_move(uci->board, move);
        }
        token = strtok(NULL, " ");
    }
}

void send_best_move(UCI *uci) {
    Move move = engine_best_move(uci->engine, uci->board);
    char uci_move_str[6] = {0};
    uci->last_move = move;
    move_to_uci(move, uci_move_str);
    apply_move(uci->board, move);
    send_message(uci, "bestmove %s", uci_move_str);
}

void log_input(UCI *uci, const char *input) {
    time_t timer;
    char buffer[26];
    struct tm* tm_info;
    timer = time(NULL);
    tm_info = localtime(&timer);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(uci->log_fp, " > %s %s", buffer, input);
    fflush(uci->log_fp);
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
            process_position_command(uci, input);

            // char *token = strtok(input, " ");
            // token = strtok(NULL, " ");
            // if (match_cmd(token, "fen")) {
            //     char fen[100];
            //     fen[0] = '\0';
                
            //     token = strtok(NULL, " ");
            //     while (token) {
            //         strcat(fen, token);
            //         strcat(fen, " ");
            //         token = strtok(NULL, " ");
            //     }
                
            //     fen[strlen(fen) - 1] = '\0';
            //     store_board(uci, fen);
            // } else if (match_cmd(token, "startpos")) {
            //     store_board(uci, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            // }
            // // }
            // else {
            //     assert(0);
            // }
            // continue;
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
