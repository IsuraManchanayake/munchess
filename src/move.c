#include <stdbool.h>
#include <string.h>

#include "move.h"
#include "common.h"
#include "defs.h"
#include "tests.h"

bool move_is_type_of(Move move, MoveType type) {
    return move.move_type_mask & type;
}

Move move_create(Piece piece, 
                    unsigned from, 
                    unsigned to, 
                    unsigned move_type_mask, 
                    PieceType promoted_type, 
                    PieceType captured_type) {
    return (Move) {
        .piece_color=piece.color, 
        .piece_type=piece.type,
        .move_type_mask=move_type_mask, 
        .from=from, 
        .to=to, 
        .promoted_type=promoted_type,
        .captured_type=captured_type
    };
}

Move move_data_create(uint32_t data) {
    return (Move) {.data=data};
}

bool is_move_null(Move move) {
    return move.data == 0;
}

char *move_buf_write(Move move, DA *da) {
    if (is_move_null(move)) {
        buf_printf(da, "[NULL MOVE]", 0);
        return (char *) da->data;
    }
    if (move_is_type_of(move, CASTLE)) {
        buf_printf(da, "%c ", piece_repr_base(move.piece_color, move.piece_type));
        volatile char to_file = IDX_TO_FILE(move.to);
        if (to_file == 'g') {
            buf_printf(da, "O-O", 0);
        } else if (to_file == 'c') {
            buf_printf(da, "O-O-O", 0);
        } else {
            assert(0);
        }
    } else {
        const char from[3] = IDX_TO_COORD(move.from);
        const char to[3] = IDX_TO_COORD(move.to);
        buf_printf(da, "%c ", piece_repr_base(move.piece_color, move.piece_type));
        buf_printf(da, "%s", from);
        char mid = '-';
        if (move_is_type_of(move, CAPTURE)) {
            mid = 'x';
        } else if (move_is_type_of(move, EN_PASSANT)) {
            mid = 'e';
        }
        buf_printf(da, "%c", mid);
        buf_printf(da, "%s", to);
        if (move_is_type_of(move, PROMOTION)) {
            char promoted = piece_type_repr(move.promoted_type);
            promoted = move.piece_color == WHITE ? promoted ^ 32 : promoted;
            buf_printf(da, "=%c", promoted);
        }
    }
    return (char *) da->data;
}

// ============================================

void test_move_size(void) {
    Move move = (Move) {0};
    move.promoted_type = 7;
    move.move_type_mask = 15;
    move.from = 63;
    move.to = 63;
    move.captured_type = 7;
    move.piece_color = 1;
    move.piece_type = 7;
    // debugzu((size_t)move.data);
    // 00001111 00111111 11111111 11111111

    // debugzu(sizeof(move));
    // debugzu(sizeof(move.data));
    assert(sizeof(move) == sizeof(move.data));
}

void test_move_create(void) {
    Piece piece = piece_create(WHITE, KING);
    Move move = move_create(piece, COORD_TO_IDX("E1"), COORD_TO_IDX("G1"), CASTLE, NONE, NONE);
    assert(move.from == 4);
    assert(move.to == 6);
    assert(move.from == COORD_TO_IDX("e1"));
    assert(move.to == COORD_TO_IDX("g1"));
    assert(move.piece_color == WHITE);
    assert(move.piece_type == KING);
}

void test_move_data_create(void) {
    Piece piece = piece_create(BLACK, KING);
    Move move = move_create(piece, COORD_TO_IDX("E8"), COORD_TO_IDX("G8"), CASTLE, NONE, NONE);
    Move copied_move = move_data_create(move.data);
    assert(copied_move.from == COORD_TO_IDX("e8"));
    assert(copied_move.to == COORD_TO_IDX("g8"));
    assert(copied_move.piece_color == BLACK);
    assert(copied_move.piece_type == KING);
}

void test_move_buf_write(void) {
    Piece piece_1 = piece_create(WHITE, KING);
    Piece piece_2 = piece_create(BLACK, QUEEN);
    Piece piece_3 = piece_create(WHITE, PAWN);
    Move move_1 = move_create(piece_1, COORD_TO_IDX("e4"), COORD_TO_IDX("e5"), NORMAL, NONE, NONE);
    Move move_2 = move_create(piece_2, COORD_TO_IDX("a1"), COORD_TO_IDX("a8"), CAPTURE, NONE, NONE);
    Move move_3 = move_create(piece_3, COORD_TO_IDX("a7"), COORD_TO_IDX("A8"), PROMOTION, ROOK, NONE);
    Move move_4 = move_create(piece_1, COORD_TO_IDX("E1"), COORD_TO_IDX("G1"), CASTLE, NONE, NONE);
    DA *da_1 = da_create();
    DA *da_2 = da_create();
    DA *da_3 = da_create();
    DA *da_4 = da_create();
    char *move_1_str = move_buf_write(move_1, da_1);
    char *move_2_str = move_buf_write(move_2, da_2);
    char *move_3_str = move_buf_write(move_3, da_3);
    char *move_4_str = move_buf_write(move_4, da_4);
    assert(strcmp(move_1_str, "K e4-e5") == 0);
    assert(strcmp(move_2_str, "q a1xa8") == 0);
    assert(strcmp(move_3_str, "P a7-a8=R") == 0);
    assert(strcmp(move_4_str, "K O-O") == 0);
    // debugs(move_1_str);
    // debugs(move_2_str);
    // debugs(move_3_str);
    // debugs(move_4_str);
    da_free(da_1);
    da_free(da_2);
    da_free(da_3);
    da_free(da_4);
}

void test_move(void) {
    test_wrapper(test_move_size);
    test_wrapper(test_move_create);
    test_wrapper(test_move_data_create);
    test_wrapper(test_move_buf_write);
}
