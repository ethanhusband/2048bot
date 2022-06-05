#include <stdlib.h>
#include <algorithm>
#include <cmath>

typedef uint64_t board_t;
typedef uint16_t row_t;

// Intuition behind the various constant values used
#define ROW_SIZE 4
#define BOARD_SIZE ROW_SIZE*ROW_SIZE
#define SQUARE_BITS 4
#define ROW_BITS ROW_SIZE*SQUARE_BITS
#define BOARD_BITS ROW_BITS*ROW_SIZE

// Note in the following, 'table' is different to 'board'. Table stores data on possible row states
#define TABLE_SIZE 65536 // 2^16, 2 possible bit states, 16 bits, per row
#define MAXIMUM_TILE 32768

// This can be &'d with any 16bit row to output the number that represents the rightmost square (last 4 digits)
#define SQUARE_MASK 0xF

// These can be &'d with the board to output a 64bit number representing just the desired column
#define COL0_MASK 0xF000F000F000F000ULL
#define COL1_MASK 0x0F000F000F000F00ULL
#define COL2_MASK 0x00F000F000F000F0ULL
#define COL3_MASK 0x000F000F000F000FULL

// Can be &'d with any board to extract bottom row
#define ROW_MASK 0xFFFF

#define BOARD_SEPERATOR "================================"
#define ROW_SEPERATOR   "--------------------------------"


void print_bitboard(board_t board);
static inline board_t play_move_right(board_t board);
static inline board_t play_move_left(board_t board);
bool row_left_shift_comp(const row_t a, const row_t b);
bool row_right_shift_comp(const row_t a, const row_t b);
void instantiate_tables();
board_t transpose_board(board_t board);