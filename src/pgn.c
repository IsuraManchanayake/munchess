#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "pgn.h"
#include "parser.h"
#include "board.h"
#include "utils.h"
#include "tests.h"

PGNGame *pgn_game_create_empty() {
    PGNGame *pgn_game = (PGNGame *) arena_allocate(&arena, sizeof(PGNGame));
    pgn_game->move_strs = da_create();
    pgn_game->draw = false;
    pgn_game->winning_color = WHITE;
    return pgn_game;
}

size_t scan_move_repr(void) {
    const char *start = stream;
    if (soft_expect('O')) {
        expect('-');
        expect('O');
        if (soft_expect('-')) {
            expect('O');
        }
    } else if (soft_expect('K') || soft_expect('Q') 
            || soft_expect('R') || soft_expect('B') 
            || soft_expect('N')) {
        if (soft_expect('x')) {
            expect_square();  // Nxf2
        } else if (soft_expect_range('1', '8')) {
            soft_expect('x');
            expect_square();  // N1f2, N1xf2
        } else {
            expect_range('a', 'h');
            if (soft_expect_range('1', '8')) {
                if (soft_expect_range('a', 'h')) {
                    expect_range('1', '8');  // Ng4f2
                } else if (soft_expect('x')) {
                    expect_square();  // Ng4xf2
                } else {
                    // Nf2
                }
            } else {
                soft_expect('x');
                expect_square();  // Ngf2, Ngxf2
            }
        }
    } else if (soft_expect_range('a', 'h')) {
        if (soft_expect('x')) {
            expect_square();
        } else {
            expect_range('1', '8');
        }
        if (soft_expect('=')) {
            expect_fn(is_promotable_piece);
        }
    } else {
        parse_error_exit("Unexpected character");
    }
    if (!soft_expect('+')) {
        soft_expect('#');
    }
    return stream - start;
}

PGNGame *parse_pgn(const char *path) {
    char *str = read_file(path);
    start_parsing(str);
    
    // Tag Section
    while(true) {
        skip_whitespace();
        if (!soft_expect('[')) {
            break;
        }
        expect_fn(is_alpha);
        while (soft_expect_fn(is_alpha));
        skip_whitespace();
        scan_quoted_str();
        expect(']');
    }

    PGNGame *pgn_game = pgn_game_create_empty();
    while (true) {
        scan_numeric();
        skip_whitespace();
        if (!soft_expect('.')) {
            break;
        }
        skip_whitespace();
        size_t moves_per_line = 2;
        bool game_over = false;
        while(moves_per_line-- > 0) {
            size_t move_len = scan_move_repr();
            char *move_str = strndup(stream - move_len, move_len);
            // debugs(move_str);
            // free(move_str);
            da_push(pgn_game->move_strs, (void *) move_str);
            
            skip_whitespace();

            if (soft_expect_str("1-0")) {
                game_over = true;
                pgn_game->winning_color = WHITE;
                break;
            } else if (soft_expect_str("0-1")) {
                game_over = true;
                pgn_game->winning_color = BLACK;
                break;
            } else if (soft_expect_str("1/2-1/2")) {
                game_over = true;
                pgn_game->draw = true;
                break;
            }
        }
        if (game_over) {
            break;
        }
    }

    //if (draw) {
    //    printf("DRAW\n");
    //} else {
    //    if (winning_color == WHITE) {
    //        printf("WHITE won\n");
    //    } else {
    //        printf("BLACK won\n");
    //    }
    //}

    free((void *)str);
    return pgn_game;
}

// ==========================

void test_read_pgn(void) {
    PGNGame *pgn_game = parse_pgn("../res/tests/game-1.pgn");

    Board *board = board_create();
    place_initial_pieces(board);
    for (size_t i = 0; i < pgn_game->move_strs->size; ++i) {
        // if (i == 91) {
        //     volatile int m = 0;
        //     ++m;
        // }
        const char *move_str = (const char *) pgn_game->move_strs->data[i];
        Move move = san_notation_to_move(move_str, board);
        if (!is_move_null(move)) {
            apply_move(board, move);
        } else {
            assert(0);
        }
    }

    DA *fen_da_1 = da_create();
    char *fen_1 = board_to_fen(board, fen_da_1);

    assert(strcmp(fen_1, "8/nk2bp2/p3b1p1/Pp1pPpP1/1P1P1P2/1KBN4/2N5/8 w - - 0 44") == 0);

    for (size_t i = 0, s = board->moves->size; i < s; ++i) {
        // DA *move_da = da_create();
        // move_buf_write(move_data_create(board->moves->data[board->moves->size - 1]), move_da);
        // printf("%s\n", (char *) move_da->data);
        // da_free(move_da);
        
        undo_last_move(board);

        // DA *board_da = da_create();
        // board_buf_write(board, board_da);
        // printf("%s\n", (char *) board_da->data);
        // da_free(board_da);
        
        // getchar();
    }
    
    // DA *board_da = da_create();
    // board_buf_write(board, board_da);
    // printf("%s\n", (char *) board_da->data);
    
    DA *fen_da_2 = da_create();
    char *fen_2 = board_to_fen(board, fen_da_2);
    // debugs(fen_2);
    assert(strcmp(fen_2, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") == 0);

    da_free(fen_da_2);
    da_free(fen_da_1);
    // da_free(board_da);
}

void test_pgn_with_fen(void) {
    Board *board = board_create();
    place_initial_pieces(board);

    PGNGame *pgn_game = parse_pgn("../res/tests/game-1.pgn");

    DA *fens_da = da_create();
    for (size_t i = 0; i < pgn_game->move_strs->size; ++i) {
        Move move = san_notation_to_move(pgn_game->move_strs->data[i], board);
        apply_move(board, move);
        DA *fen_da = da_create();
        board_to_fen(board, fen_da);
        da_push(fens_da, (void *) strdup((char *)fen_da->data));
        da_free(fen_da);
    }

    for (size_t i = 8; i < fens_da->size; ++i) {
        char *original_fen = (char *) fens_da->data[i];
        fen_to_board(original_fen, board);

        DA *fen_da =  da_create();
        char *derived_fen = board_to_fen(board, fen_da);
        assert(strcmp(original_fen, derived_fen) == 0);
        da_free(fen_da);

        for (size_t j = i + 1; j < pgn_game->move_strs->size; ++j) {
            Move move = san_notation_to_move(pgn_game->move_strs->data[j], board);
            apply_move(board, move);
        }
        
        DA *fen_da_2 =  da_create();
        char *derived_fen_2 = board_to_fen(board, fen_da_2);
        assert(strcmp(fens_da->data[fens_da->size - 1], derived_fen_2) == 0);
        da_free(fen_da_2);
    }

    da_free(fens_da);
}

void test_pgn(void) {
    test_wrapper(test_read_pgn);
    test_wrapper(test_pgn_with_fen);
}
