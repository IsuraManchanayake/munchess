#define simple(c) ((char)((c) | 32))
#define capital(c) ((char)(c) & ~32))
#define dtoc(d) ((char)((d) + '0'))
#define ctod(c) ((c) - '0')

#define op_color(c) ((Color)(1 - (c)))

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
#define YX_TO_COORD(y, x) {x + 'a', y + '1', 0}
#define YX_TO_FR(y, x) {x + 'a',y + 1}
#define FR_TO_IDX(f, r) IDX(r - 1, simple(f) - 'a')
#define FR_TO_COORD(f, r) {f, dtoc(r), 0}
#define FR_TO_YX(f, r) {r - 1, simple(f) - 'a'}

#define ATyx(board, y, x) ((board)->pieces[IDX((y), (x))])
#define ATcoord(board, coord) ((board)->pieces[COORD_TO_IDX(coord)])
#define ATidx(board, pos) ((board)->pieces[pos])
#define ATfr(board, file, rank) ATyx((board), (rank) - 1, simple(file) - 'a')

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b));
#endif
#ifndef max
#define max(a, b) ((a) < (b) ? (b) : (a));
#endif

#if (defined(__GNUC__) || defined(__clang__)) && (__STDC_VERSION__ >= 201710L)
#define UNDERLYING(Type) :uint8_t
#define ENUM_BITS(Type, name, bits) Type name: bits
#else
#define UNDERLYING(Type) 
#define ENUM_BITS(Type, name, bits) uint8_t name: bits
#endif

#define debugzu(x) (printf("%s = %zu\n", #x, (x)))
#define debugi(x) (printf("%s = %d\n", #x, (x)))
#define debugc(x) (printf("%s = %c\n", #x, (x)))
#define debugs(x) (printf("%s = %s\n", #x, (x)))
