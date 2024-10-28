#define simple(c) ((char)((c) | 32))
#define capital(c) ((char)(c) & ~32))
#define dtoc(d) ((char)((d) + '0'))
#define ctod(c) ((c) - '0')

#define IDX(y, x) (((y) << 3) + (x))
#define IDX_Y(idx) (((size_t)(idx)) >> 3)
#define IDX_X(idx) ((idx) % 8)
#define IDX_TO_RANK(idx) (IDX_Y(idx) + 1)
#define IDX_TO_FILE(idx) (IDX_X(idx) + 'a')

// #define WITHIN_BOARD(x, y) ((0 <= (x)) && ((x) < 8) && (0 <= (y)) && ((y) < 8))

// position in the array - idx (e.g. 4)
// coordinate as string (null-terminated array of length 3) - coord (e.g. E1)
// coordinate as xy tuple - y,x (e.g. {0, 4})
// coordinate as fr tuple - f,r (e.g. {'E', 1})
#define IDX_TO_COORD(idx) {IDX_TO_FILE(idx), '0' + IDX_TO_RANK(idx), 0}
#define IDX_TO_YX(idx) {IDX_Y(idx), IDX_X(idx)}
#define IDX_TO_FR(idx) {IDX_TO_FILE(idx), IDX_TO_RANK(idx)}
#define COORD_TO_IDX(coord) IDX((coord)[1] - '1', simple((coord)[0]) - 'a')
#define COORD_TO_YX(coord) {(coord)[1] - '1', simple((coord)[0]) - 'a'}
#define COORD_TO_FR(coord) {simple((coord)[0]), (coord)[1] - '1'}
#define YX_TO_IDX(y, x) (IDX(y, x))
#define YX_TO_COORD(y, x) {y + 'a', x + '1', 0}
#define YX_TO_FR(y, x) {y + 'a',x + 1}
#define FR_TO_IDX(f, r) IDX(r - 1, simple(f) - 'a')
#define FR_TO_COORD(f, r) {f, dtoc(r), 0}
#define FR_TO_YX(f, r) {r - 1, simple(f) - 'a'}
