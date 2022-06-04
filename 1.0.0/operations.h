#include <stdlib.h>
#include <algorithm>

typedef uint64_t board_t;
typedef uint16_t row_t;

// Intuition behind the various constant values used
#define ROW_SIZE 4
#define BOARD_SIZE ROW_SIZE*ROW_SIZE
#define SQUARE_BITS 4
#define ROW_BITS ROW_SIZE*SQUARE_BITS

// Note in the following, 'table' is different to 'board'. Table stores data on possible row states
#define TABLE_SIZE 65536 // 2^16, 2 possible bit states, 16 bits, per row
#define MAXIMUM_TILE 32768

// This can be &'d with any 16bit row to output the number that represents the rightmost square (last 4 digits)
#define SQUARE_MASK 0xf