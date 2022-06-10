#include <stdlib.h>
#include <algorithm> // used for stable sort
#include <cmath>
#include <time.h> // used for setting rand seed
#include <unistd.h> // grants sleep function used for testing
#include <iostream> // reading in the board from .py

typedef uint64_t board_t;
typedef uint16_t row_t;

#define MAX_DEPTH 4
// These are used since we can only use ctypes in python, which does not include bool
#define TRUE 1
#define FALSE 0

// Intuition behind the various constant values used
#define ROW_SIZE 4
#define BOARD_SIZE ROW_SIZE*ROW_SIZE
#define SQUARE_BITS 4
#define ROW_BITS ROW_SIZE*SQUARE_BITS
#define BOARD_BITS ROW_BITS*ROW_SIZE
#define MOVE_DIRECTIONS 4

// Note in the following, 'table' is different to 'board'. Table stores data on possible row states
#define TABLE_SIZE 65536 // 2^16, 2 possible bit states, 16 bits, per row
#define MAXIMUM_TILE 32768

// This can be &'d with any 16bit row to output the number that represents the rightmost square (last 4 digits)
#define SQUARE_MASK 0xF

// These can be &'d with the board to output a 64bit number representing just the desired column
#define COLUMN_MASK 0x000F000F000F000FULL

// Can be &'d with any board to extract bottom row
#define ROW_MASK 0xFFFF

#define BOARD_SEPERATOR "================================"
#define ROW_SEPERATOR   "--------------------------------"

// Lookup table functions
bool left_shift_comp(const row_t a, const row_t b);
bool right_shift_comp(const row_t a, const row_t b);
static float score_board(board_t board);

void play_game();
board_t init_board();
static inline board_t transpose_board(board_t board);
board_t insert_rand_square(board_t board, board_t new_square);
board_t get_new_square();
int count_empty_squares(board_t board);

static inline board_t play_move(int move, board_t board);
static inline board_t play_move_right(board_t board);
static inline board_t play_move_left(board_t board);
static inline board_t play_move_up(board_t board);
static inline board_t play_move_down(board_t board);

void print_bitboard(board_t board);

void instantiate_tables();
float get_move_tree_score(board_t board, int depth, int is_max, float cprob);
int select_move(board_t board);
