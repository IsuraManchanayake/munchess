"""
    uint64_t rook_bb = board->bb[ROOK][board->to_move];
    uint8_t rook_idx = 0;
    while (rook_bb) {
        uint8_t rook_idx_offset = next_piece_idx(rook_bb);
        rook_idx += rook_idx_offset;
        generate_rook_moves(board, rook_idx, moves, false);
        rook_bb >>= rook_idx_offset + 1;
        ++rook_idx;
    }
"""

def ctzll(x):
    if x == 0:
        raise ValueError("Input must be non-zero")
    return (x & -x).bit_length() - 1


rook_bb = 9223372036854775808
rook_idx = 0
while rook_bb != 0:
	rook_idx_offset = ctzll(rook_bb)
	rook_idx += rook_idx_offset
	print(rook_idx)
	rook_bb >>= rook_idx_offset + 1
	rook_idx += 1